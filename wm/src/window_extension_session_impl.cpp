/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "window_extension_session_impl.h"

#include <bool_wrapper.h>
#include <float_wrapper.h>
#include <hitrace_meter.h>
#include <int_wrapper.h>
#include <long_wrapper.h>
#include <ipc_types.h>
#include <parameters.h>
#include <string_wrapper.h>
#include <transaction/rs_interfaces.h>
#include <transaction/rs_transaction.h>

#ifdef IMF_ENABLE
#include <input_method_controller.h>
#endif

#include "configuration.h"
#include "display_info.h"
#include "input_transfer_station.h"
#include "perform_reporter.h"
#include "rs_adapter.h"
#include "session_permission.h"
#include "singleton_container.h"
#include "sys_cap_util.h"
#include "ui_extension/provider_data_handler.h"
#include "window_adapter.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "session_helper.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowExtensionSessionImpl"};
#ifdef IMF_ENABLE
constexpr int64_t DISPATCH_KEY_EVENT_TIMEOUT_TIME_MS = 1000;
#endif // IMF_ENABLE
constexpr int32_t UIEXTENTION_ROTATION_ANIMATION_TIME = 400;
}

#define CHECK_HOST_SESSION_RETURN_IF_NULL(hostSession)                         \
    do {                                                                       \
        if ((hostSession) == nullptr) {                                        \
            TLOGE(WmsLogTag::DEFAULT, "hostSession is null");                  \
            return;                                                            \
        }                                                                      \
    } while (false)

#define CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, ret)              \
    do {                                                                       \
        if ((hostSession) == nullptr) {                                        \
            TLOGE(WmsLogTag::DEFAULT, "hostSession is null");                  \
            return ret;                                                        \
        }                                                                      \
    } while (false)

WindowExtensionSessionImpl::WindowExtensionSessionImpl(const sptr<WindowOption>& option) : WindowSessionImpl(option)
{
    if (property_->GetUIExtensionUsage() == UIExtensionUsage::MODAL ||
        SessionHelper::IsSecureUIExtension(property_->GetUIExtensionUsage())) {
        startModalExtensionTimeStamp_ = option->GetStartModalExtensionTimeStamp();
        extensionWindowFlags_.hideNonSecureWindowsFlag = true;
    }
    if ((isDensityFollowHost_ = option->GetIsDensityFollowHost())) {
        hostDensityValue_ = option->GetDensity();
    }
    TLOGNI(WmsLogTag::WMS_UIEXT, "Uiext usage=%{public}u, timeStamp=%{public}" PRId64,
        property_->GetUIExtensionUsage(), startModalExtensionTimeStamp_);
    dataHandler_ = std::make_shared<Extension::ProviderDataHandler>();
    RegisterDataConsumer();
}

WindowExtensionSessionImpl::~WindowExtensionSessionImpl()
{
    WLOGFI("[WMSCom] %{public}d, %{public}s", GetPersistentId(), GetWindowName().c_str());
}

static float GetFloatParam(const std::string& key, const AAFwk::WantParams& configParam,
    float defaultValue = 1.0f)
{
    if (AAFwk::IFloat* so = AAFwk::IFloat::Query(configParam.GetParam(key))) {
        defaultValue = AAFwk::Float::Unbox(so);
    }
    return defaultValue;
}

std::shared_ptr<IDataHandler> WindowExtensionSessionImpl::GetExtensionDataHandler() const
{
    return dataHandler_;
}

WMError WindowExtensionSessionImpl::Create(const std::shared_ptr<AbilityRuntime::Context>& context,
    const sptr<Rosen::ISession>& iSession, const std::string& identityToken, bool isModuleAbilityHookEnd)
{
    TLOGD(WmsLogTag::WMS_LIFE, "Called.");
    if (!context || !iSession) {
        TLOGE(WmsLogTag::WMS_LIFE, "context is nullptr: %{public}u or sessionToken is nullptr: %{public}u",
            context == nullptr, iSession == nullptr);
        return WMError::WM_ERROR_NULLPTR;
    }
    if (vsyncStation_ == nullptr || !vsyncStation_->IsVsyncReceiverCreated()) {
        return WMError::WM_ERROR_NULLPTR;
    }
    SetDefaultDisplayIdIfNeed();
    // Since here is init of this window, no other threads will rw it.
    hostSession_ = iSession;

    dataHandler_->SetEventHandler(handler_);
    dataHandler_->SetRemoteProxyObject(iSession->AsObject());

    context_ = context;
    if (context_) {
        abilityToken_ = context_->GetToken();
    }
    // XTS log, please do not modify
    TLOGI(WmsLogTag::WMS_UIEXT, "IsConstrainedModal: %{public}d", property_->IsConstrainedModal());
    AddExtensionWindowStageToSCB(property_->IsConstrainedModal());
    WMError ret = Connect();
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "name:%{public}s %{public}d connect fail. ret:%{public}d",
            property_->GetWindowName().c_str(), GetPersistentId(), ret);
        return ret;
    }
    MakeSubOrDialogWindowDragableAndMoveble();
    {
        std::unique_lock<std::shared_mutex> lock(windowExtensionSessionMutex_);
        GetWindowExtensionSessionSet().insert(this);
    }

    auto usage = property_->GetUIExtensionUsage();
    if ((usage == UIExtensionUsage::MODAL) || SessionHelper::IsSecureUIExtension(usage)) {
        InputTransferStation::GetInstance().AddInputWindow(this);
    }

    state_ = WindowState::STATE_CREATED;
    isUIExtensionAbilityProcess_ = true;
    property_->SetIsUIExtensionAbilityProcess(true);
    UpdateDefaultStatusBarColor();
    TLOGI(WmsLogTag::WMS_LIFE, "Created name:%{public}s %{public}d",
        property_->GetWindowName().c_str(), GetPersistentId());
    AddSetUIContentTimeoutCheck();
    return WMError::WM_OK;
}

void WindowExtensionSessionImpl::AddExtensionWindowStageToSCB(bool isConstrainedModal)
{
    if (!abilityToken_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "token is nullptr");
        return;
    }
    if (surfaceNode_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "surfaceNode_ is nullptr");
        return;
    }

    SingletonContainer::Get<WindowAdapter>().AddExtensionWindowStageToSCB(sptr<ISessionStage>(this), abilityToken_,
        surfaceNode_->GetId(), startModalExtensionTimeStamp_, isConstrainedModal);
}

void WindowExtensionSessionImpl::RemoveExtensionWindowStageFromSCB(bool isConstrainedModal)
{
    if (!abilityToken_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "token is nullptr");
        return;
    }

    SingletonContainer::Get<WindowAdapter>().RemoveExtensionWindowStageFromSCB(sptr<ISessionStage>(this),
        abilityToken_, isConstrainedModal);
}

void WindowExtensionSessionImpl::UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "notify ace extension win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
        uiContent->UpdateConfiguration(configuration);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent null, ext win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
    }
    UpdateDefaultStatusBarColor();
}

void WindowExtensionSessionImpl::UpdateConfigurationForSpecified(
    const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager)
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "notify ace extension win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
        uiContent->UpdateConfiguration(configuration, resourceManager);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent null, ext win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
    }
    if (configuration != nullptr) {
        TLOGI(WmsLogTag::WMS_IMMS, "extension win=%{public}u, colorMode=%{public}s, display=%{public}" PRIu64,
            GetWindowId(), specifiedColorMode_.c_str(), GetDisplayId());
        specifiedColorMode_ = configuration->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        UpdateDefaultStatusBarColor();
    }
}

void WindowExtensionSessionImpl::UpdateConfigurationForAll(
    const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts)
{
    std::unordered_set<std::shared_ptr<AbilityRuntime::Context>> ignoreWindowCtxSet(
        ignoreWindowContexts.begin(), ignoreWindowContexts.end());
    std::unique_lock<std::shared_mutex> lock(windowExtensionSessionMutex_);
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "extension map size: %{public}u",
        static_cast<uint32_t>(GetWindowExtensionSessionSet().size()));
    for (const auto& window : GetWindowExtensionSessionSet()) {
        if (window == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "extension window is null");
            continue;
        }
        if (ignoreWindowCtxSet.count(window->GetContext()) == 0) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "extension win=%{public}u, display=%{public}" PRIu64,
                window->GetWindowId(), window->GetDisplayId());
            window->UpdateConfiguration(configuration);
        } else {
            TLOGI(WmsLogTag::WMS_ATTRIBUTE, "skip extension win=%{public}u, display=%{public}" PRIu64,
                window->GetWindowId(), window->GetDisplayId());
        }
    }
}

void WindowExtensionSessionImpl::UpdateConfigurationSync(
    const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "notify ace extension win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
        uiContent->UpdateConfigurationSyncForAll(configuration);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent null, ext win=%{public}u, display=%{public}" PRIu64,
            GetWindowId(), GetDisplayId());
    }
}

void WindowExtensionSessionImpl::UpdateConfigurationSyncForAll(
    const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    std::unique_lock<std::shared_mutex> lock(windowExtensionSessionMutex_);
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "extension map size: %{public}u",
        static_cast<uint32_t>(GetWindowExtensionSessionSet().size()));
    for (const auto& window : GetWindowExtensionSessionSet()) {
        if (window == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "extension window is null");
            continue;
        }
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "extension win=%{public}u, display=%{public}" PRIu64,
            window->GetWindowId(), window->GetDisplayId());
        window->UpdateConfigurationSync(configuration);
    }
}

void WindowExtensionSessionImpl::UpdateDefaultStatusBarColor()
{
    if (!property_->GetIsAtomicService()) {
        TLOGD(WmsLogTag::WMS_IMMS, "win %{public}u no support", GetPersistentId());
        return;
    }
    uint32_t contentColor = 0;
    auto ret = UpdateStatusBarColorByColorMode(contentColor);
    if (ret != WMError::WM_OK) {
        TLOGD(WmsLogTag::WMS_IMMS, "win %{public}u no need update", GetPersistentId());
        return;
    }
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u, contentColor %{public}x", GetPersistentId(), contentColor);
    SetStatusBarColorForExtensionInner(contentColor);
}

WMError WindowExtensionSessionImpl::Destroy(bool needNotifyServer, bool needClearListener, uint32_t reason)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d Destroy, state:%{public}u, needNotifyServer:%{public}d, "
        "needClearListener:%{public}d", GetPersistentId(), state_, needNotifyServer, needClearListener);

    auto usage = property_->GetUIExtensionUsage();
    if ((usage == UIExtensionUsage::MODAL) || SessionHelper::IsSecureUIExtension(usage)) {
        InputTransferStation::GetInstance().RemoveInputWindow(GetPersistentId());
    }

    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (auto hostSession = GetHostSession()) {
        TLOGI(WmsLogTag::WMS_LIFE, "Disconnect with host session, id: %{public}d.", GetPersistentId());
        hostSession->Disconnect();
    }
    NotifyBeforeDestroy(GetWindowName());
    if (needClearListener) {
        ClearListenersById(GetPersistentId());
    }
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        state_ = WindowState::STATE_DESTROYED;
        requestState_ = WindowState::STATE_DESTROYED;
    }
    DestroySubWindow();
    {
        TLOGI(WmsLogTag::WMS_LIFE, "Reset state, id: %{public}d.", GetPersistentId());
        std::lock_guard<std::mutex> lock(hostSessionMutex_);
        hostSession_ = nullptr;
    }
    {
        std::unique_lock<std::shared_mutex> lock(windowExtensionSessionMutex_);
        GetWindowExtensionSessionSet().erase(this);
    }
    // Notify host window to remove rect change listeners
    AAFwk::Want want;
    auto ret = SendExtensionMessageToHost(
        static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER), want);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Send unregister host window rect change listener message failed, errCode: "
            "%{public}d", ret);
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Erase in set, id: %{public}d.", GetPersistentId());
    if (context_) {
        context_.reset();
    }
    ClearVsyncStation();
    SetUIContentComplete();
    SetUIExtensionDestroyComplete();
    RemoveExtensionWindowStageFromSCB(property_->IsConstrainedModal());
    dataHandler_->SetRemoteProxyObject(nullptr);
    TLOGI(WmsLogTag::WMS_LIFE, "Destroyed success, id: %{public}d.", GetPersistentId());
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::MoveTo(int32_t x, int32_t y,
    bool isMoveToGlobal, MoveConfiguration moveConfiguration)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "Id:%{public}d xy %{public}d %{public}d isMoveToGlobal %{public}d "
        "moveConfiguration %{public}s", property_->GetPersistentId(), x, y, isMoveToGlobal,
        moveConfiguration.ToString().c_str());
    if (IsWindowSessionInvalid()) {
        WLOGFE("Window session invalid.");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    const auto& rect = property_->GetWindowRect();
    WSRect wsRect = { x, y, rect.width_, rect.height_ };
    WSError error = UpdateRect(wsRect, SizeChangeReason::MOVE);
    return static_cast<WMError>(error);
}

WMError WindowExtensionSessionImpl::Resize(uint32_t width, uint32_t height,
    const RectAnimationConfig& rectAnimationConfig)
{
    WLOGFD("Id:%{public}d wh %{public}u %{public}u", property_->GetPersistentId(), width, height);
    if (IsWindowSessionInvalid()) {
        WLOGFE("Window session invalid.");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    const auto& rect = property_->GetWindowRect();
    WSRect wsRect = { rect.posX_, rect.posY_, width, height };
    WSError error = UpdateRect(wsRect, SizeChangeReason::RESIZE);
    return static_cast<WMError>(error);
}

WMError WindowExtensionSessionImpl::TransferAbilityResult(uint32_t resultCode, const AAFwk::Want& want)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "id: %{public}d", GetPersistentId());
    if (IsWindowSessionInvalid()) {
        WLOGFE("Window invalid.");
        return WMError::WM_ERROR_REPEAT_OPERATION;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    return static_cast<WMError>(hostSession->TransferAbilityResult(resultCode, want));
}

WMError WindowExtensionSessionImpl::TransferExtensionData(const AAFwk::WantParams& wantParams)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "id: %{public}d", GetPersistentId());
    if (IsWindowSessionInvalid()) {
        WLOGFE("Window invalid.");
        return WMError::WM_ERROR_REPEAT_OPERATION;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    auto ret = hostSession->TransferExtensionData(wantParams);
    if (ret == ERR_NONE) {
        return WMError::WM_OK;
    }
    if (context_ != nullptr) {
        std::ostringstream oss;
        oss << "TransferExtensionData from provider to host failed" << ",";
        oss << " provider bundleName: " << context_->GetBundleName() << ",";
        oss << " provider windowName: " << property_->GetWindowName() << ",";
        oss << " errorCode: " << static_cast<int32_t>(ret) << ";";
        int32_t res = WindowInfoReporter::GetInstance().ReportUIExtensionException(
            static_cast<int32_t>(WindowDFXHelperType::WINDOW_UIEXTENSION_TRANSFER_DATA_FAIL),
            getpid(), GetPersistentId(), oss.str()
        );
        if (res != 0) {
            TLOGE(WmsLogTag::WMS_UIEXT, "ReportUIExtensionException message failed, res: %{public}d", res);
        }
    }
    return WMError::WM_ERROR_IPC_FAILED;
}

void WindowExtensionSessionImpl::RegisterTransferComponentDataListener(const NotifyTransferComponentDataFunc& func)
{
    if (IsWindowSessionInvalid()) {
        WLOGFE("Window invalid.");
        return;
    }
    notifyTransferComponentDataFunc_ = std::move(func);
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_IF_NULL(hostSession);
    hostSession->NotifyAsyncOn();
}

WSError WindowExtensionSessionImpl::NotifyTransferComponentData(const AAFwk::WantParams& wantParams)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "id: %{public}d", GetPersistentId());
    if (notifyTransferComponentDataFunc_) {
        notifyTransferComponentDataFunc_(wantParams);
    }
    return WSError::WS_OK;
}

WSErrorCode WindowExtensionSessionImpl::NotifyTransferComponentDataSync(
    const AAFwk::WantParams& wantParams, AAFwk::WantParams& reWantParams)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "id: %{public}d", GetPersistentId());
    if (notifyTransferComponentDataForResultFunc_) {
        reWantParams = notifyTransferComponentDataForResultFunc_(wantParams);
        return WSErrorCode::WS_OK;
    }
    return WSErrorCode::WS_ERROR_NOT_REGISTER_SYNC_CALLBACK;
}

void WindowExtensionSessionImpl::RegisterTransferComponentDataForResultListener(
    const NotifyTransferComponentDataForResultFunc& func)
{
    if (IsWindowSessionInvalid()) {
        WLOGFE("session invalid.");
        return;
    }
    notifyTransferComponentDataForResultFunc_ = std::move(func);
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_IF_NULL(hostSession);
    hostSession->NotifySyncOn();
}

WMError WindowExtensionSessionImpl::RegisterHostWindowRectChangeListener(
    const sptr<IWindowRectChangeListener>& listener)
{
    AAFwk::Want dataToSend;
    auto ret = SendExtensionMessageToHost(
        static_cast<uint32_t>(Extension::Businesscode::REGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER), dataToSend);
    if (ret != WMError::WM_OK) {
        return ret;
    }
    {
        std::lock_guard<std::mutex> lockListener(hostWindowRectChangeListenerMutex_);
        ret = RegisterListener(hostWindowRectChangeListener_, listener);
    }
    return ret;
}

WMError WindowExtensionSessionImpl::UnregisterHostWindowRectChangeListener(
    const sptr<IWindowRectChangeListener>& listener)
{
    WMError ret = WMError::WM_OK;
    bool isHostWindowRectChangeListenerEmpty = false;
    size_t hostWindowRectChangeListenerSize = 0;
    {
        std::lock_guard<std::mutex> lockListener(hostWindowRectChangeListenerMutex_);
        ret = UnregisterListener(hostWindowRectChangeListener_, listener);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        isHostWindowRectChangeListenerEmpty = hostWindowRectChangeListener_.empty();
        hostWindowRectChangeListenerSize = hostWindowRectChangeListener_.size();
    }
    if (!isHostWindowRectChangeListenerEmpty || !rectChangeUIExtListenerIds_.empty()) {
        TLOGI(WmsLogTag::WMS_UIEXT, "No need to send message to host to unregister, size of "
            "hostWindowRectChangeListener_: %{public}zu, size of rectChangeUIExtListenerIds_: %{public}zu",
            hostWindowRectChangeListenerSize, rectChangeUIExtListenerIds_.size());
        return ret;
    }
    AAFwk::Want dataToSend;
    ret = SendExtensionMessageToHost(
        static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER), dataToSend);
    return ret;
}

WMError WindowExtensionSessionImpl::RegisterRectChangeInGlobalDisplayListener(
    const sptr<IRectChangeInGlobalDisplayListener>& listener)
{
    AAFwk::Want dataToSend;
    auto ret = SendExtensionMessageToHost(
        static_cast<uint32_t>(Extension::Businesscode::REGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER),
        dataToSend);
    if (ret != WMError::WM_OK) {
        return ret;
    }
    {
        std::lock_guard<std::mutex> lockListener(hostRectChangeInGlobalDisplayListenerMutex_);
        ret = RegisterListener(hostRectChangeInGlobalDisplayListenerList_, listener);
    }
    return ret;
}

WMError WindowExtensionSessionImpl::UnregisterRectChangeInGlobalDisplayListener(
    const sptr<IRectChangeInGlobalDisplayListener>& listener)
{
    WMError ret = WMError::WM_OK;
    bool needNotifyHost = false;
    {
        std::lock_guard<std::mutex> lockListener(hostRectChangeInGlobalDisplayListenerMutex_);
        ret = UnregisterListener(hostRectChangeInGlobalDisplayListenerList_, listener);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        needNotifyHost = hostRectChangeInGlobalDisplayListenerList_.empty() &&
            rectChangeInGlobalDisplayUIExtListenerIds_.empty();
    }
    if (needNotifyHost) {
        AAFwk::Want dataToSend;
        ret = SendExtensionMessageToHost(
            static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER),
            dataToSend);
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "No need to send message to host to unregister, size of "
        "listener: %{public}zu, size of rectChangeInGlobalDisplayUIExtListenerIds_: %{public}zu",
        hostRectChangeInGlobalDisplayListenerList_.size(), rectChangeInGlobalDisplayUIExtListenerIds_.size());
    return ret;
}

WMError WindowExtensionSessionImpl::RegisterKeyboardDidShowListener(const sptr<IKeyboardDidShowListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    AAFwk::Want want;
    auto ret = SendExtensionMessageToHost(
        static_cast<uint32_t>(Extension::Businesscode::REGISTER_KEYBOARD_DID_SHOW_LISTENER), want);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Send register listener message failed, errCode: %{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> lockListener(keyboardDidShowListenerMutex_);
    ret = RegisterListener(keyboardDidShowListenerList_, listener);
    return ret;
}

WMError WindowExtensionSessionImpl::UnregisterKeyboardDidShowListener(const sptr<IKeyboardDidShowListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    WMError ret = WMError::WM_OK;
    bool needNotifyHost = false;
    {
        std::lock_guard<std::mutex> lockListener(keyboardDidShowListenerMutex_);
        ret = UnregisterListener(keyboardDidShowListenerList_, listener);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        needNotifyHost = keyboardDidShowListenerList_.empty() && keyboardDidShowUIExtListeners_.empty();
    }
    if (needNotifyHost) {
        AAFwk::Want want;
        ret = SendExtensionMessageToHost(
            static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_KEYBOARD_DID_SHOW_LISTENER), want);
    }
    return ret;
}

WMError WindowExtensionSessionImpl::RegisterKeyboardDidHideListener(const sptr<IKeyboardDidHideListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    AAFwk::Want want;
    auto ret = SendExtensionMessageToHost(
        static_cast<uint32_t>(Extension::Businesscode::REGISTER_KEYBOARD_DID_HIDE_LISTENER), want);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Send register listener message failed, errCode: %{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> lockListener(keyboardDidHideListenerMutex_);
    ret = RegisterListener(keyboardDidHideListenerList_, listener);
    return ret;
}

WMError WindowExtensionSessionImpl::UnregisterKeyboardDidHideListener(const sptr<IKeyboardDidHideListener>& listener)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    WMError ret = WMError::WM_OK;
    bool needNotifyHost = false;
    {
        std::lock_guard<std::mutex> lockListener(keyboardDidHideListenerMutex_);
        ret = UnregisterListener(keyboardDidHideListenerList_, listener);
        if (ret != WMError::WM_OK) {
            return ret;
        }
        needNotifyHost = keyboardDidHideListenerList_.empty() && keyboardDidHideUIExtListeners_.empty();
    }
    if (needNotifyHost) {
        AAFwk::Want want;
        ret = SendExtensionMessageToHost(
            static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_KEYBOARD_DID_HIDE_LISTENER), want);
    }
    return ret;
}

void WindowExtensionSessionImpl::TriggerBindModalUIExtension()
{
    TLOGI(WmsLogTag::WMS_UIEXT, "id: %{public}d", GetPersistentId());
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_IF_NULL(hostSession);
    hostSession->TriggerBindModalUIExtension();
}

WMError WindowExtensionSessionImpl::SetPrivacyMode(bool isPrivacyMode)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "Id: %{public}u, isPrivacyMode: %{public}u", GetPersistentId(),
        isPrivacyMode);
    if (surfaceNode_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "surfaceNode is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    surfaceNode_->SetSecurityLayer(isPrivacyMode);
    RSTransactionAdapter::FlushImplicitTransaction(surfaceNode_);

    if (state_ != WindowState::STATE_SHOWN) {
        extensionWindowFlags_.privacyModeFlag = isPrivacyMode;
        return WMError::WM_OK;
    }
    if (isPrivacyMode == extensionWindowFlags_.privacyModeFlag) {
        return WMError::WM_OK;
    }

    auto updateFlags = extensionWindowFlags_;
    updateFlags.privacyModeFlag = isPrivacyMode;
    ExtensionWindowFlags actions(0);
    actions.privacyModeFlag = true;
    auto ret = UpdateExtWindowFlags(updateFlags, actions);
    if (ret == WMError::WM_OK) {
        extensionWindowFlags_ = updateFlags;
    }
    return ret;
}

WMError WindowExtensionSessionImpl::HidePrivacyContentForHost(bool needHide)
{
    auto persistentId = GetPersistentId();
    std::stringstream ss;
    ss << "ID: " << persistentId << ", needHide: " << needHide;

    if (surfaceNode_ == nullptr) {
        TLOGI(WmsLogTag::WMS_UIEXT, "surfaceNode is null, %{public}s", ss.str().c_str());
        return WMError::WM_ERROR_NULLPTR;
    }

    // Let rs guarantee the security and permissions of the interface
    auto errCode = surfaceNode_->SetHidePrivacyContent(needHide);
    TLOGI(WmsLogTag::WMS_UIEXT, "Notify Render Service client finished, %{public}s, err: %{public}u", ss.str().c_str(),
          errCode);
    if (errCode == RSInterfaceErrorCode::NONSYSTEM_CALLING) { // not system app calling
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    } else if (errCode != RSInterfaceErrorCode::NO_ERROR) { // other error
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }

    return WMError::WM_OK;
}

bool WindowExtensionSessionImpl::IsComponentFocused() const
{
    if (IsWindowSessionInvalid() || focusState_ == std::nullopt) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Session is invalid");
        return false;
    }
    TLOGD(WmsLogTag::WMS_FOCUS, "window id=%{public}d, isFocused=%{public}d",
        GetPersistentId(), static_cast<int32_t>(focusState_.value()));
    return focusState_.value();
}

void WindowExtensionSessionImpl::NotifyFocusStateEvent(bool focusState)
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        focusState ? uiContent->Focus() : uiContent->UnFocus();
    }
    if (focusState) {
        NotifyWindowAfterFocused();
    } else {
        NotifyWindowAfterUnfocused();
    }
    focusState_ = focusState;
    if (focusState_ != std::nullopt) {
        TLOGI(WmsLogTag::WMS_FOCUS, "[%{public}d,%{public}d]",
            GetPersistentId(), static_cast<int32_t>(focusState_.value()));
    }
}

void WindowExtensionSessionImpl::NotifyFocusActiveEvent(bool isFocusActive)
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SetIsFocusActive(isFocusActive);
    }
}

void WindowExtensionSessionImpl::NotifyBackpressedEvent(bool& isConsumed)
{
    if (auto uiContent = GetUIContentSharedPtr()) {
        WLOGFD("Transfer backpressed event to uiContent");
        isConsumed = uiContent->ProcessBackPressed();
    }
    WLOGFD("Backpressed event is consumed %{public}d", isConsumed);
}

void WindowExtensionSessionImpl::InputMethodKeyEventResultCallback(const std::shared_ptr<MMI::KeyEvent>& keyEvent,
    bool consumed, std::shared_ptr<std::promise<bool>> isConsumedPromise, std::shared_ptr<bool> isTimeout)
{
    if (keyEvent == nullptr) {
        WLOGFW("keyEvent is null, consumed:%{public}" PRId32, consumed);
        if (isConsumedPromise != nullptr) {
            isConsumedPromise->set_value(consumed);
        }
        return;
    }

    auto id = keyEvent->GetId();
    if (isConsumedPromise == nullptr || isTimeout == nullptr) {
        WLOGFW("Shared point isConsumedPromise or isTimeout is null, id:%{public}" PRId32, id);
        keyEvent->MarkProcessed();
        return;
    }

    if (*isTimeout) {
        WLOGFW("DispatchKeyEvent timeout id:%{public}" PRId32, id);
        keyEvent->MarkProcessed();
        return;
    }

    if (consumed) {
        isConsumedPromise->set_value(consumed);
        WLOGD("Input method has processed key event, id:%{public}" PRId32, id);
        return;
    }

    bool isConsumed = false;
    DispatchKeyEventCallback(const_cast<std::shared_ptr<MMI::KeyEvent>&>(keyEvent), isConsumed);
    isConsumedPromise->set_value(isConsumed);
}

void WindowExtensionSessionImpl::NotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed,
    bool notifyInputMethod)
{
    if (keyEvent == nullptr) {
        WLOGFE("keyEvent is nullptr");
        return;
    }
    if (property_->GetUIExtensionUsage() == UIExtensionUsage::PREVIEW_EMBEDDED) {
        TLOGD(WmsLogTag::WMS_EVENT, "Preview uiext does not handle event, eid:%{public}d", keyEvent->GetId());
        return;
    }

#ifdef IMF_ENABLE
    bool isKeyboardEvent = IsKeyboardEvent(keyEvent);
    if (isKeyboardEvent && notifyInputMethod) {
        WLOGD("Async dispatch keyEvent to input method, id:%{public}" PRId32, keyEvent->GetId());
        auto isConsumedPromise = std::make_shared<std::promise<bool>>();
        auto isConsumedFuture = isConsumedPromise->get_future().share();
        auto isTimeout = std::make_shared<bool>(false);
        auto ret = MiscServices::InputMethodController::GetInstance()->DispatchKeyEvent(keyEvent,
            [weakThis = wptr(this), isConsumedPromise, isTimeout](const std::shared_ptr<MMI::KeyEvent>& keyEvent,
                bool consumed) {
                auto window = weakThis.promote();
                if (window == nullptr) {
                    TLOGNE(WmsLogTag::WMS_UIEXT, "window is nullptr");
                    return;
                }
                window->InputMethodKeyEventResultCallback(keyEvent, consumed, isConsumedPromise, isTimeout);
            });
        if (ret != 0) {
            WLOGFW("DispatchKeyEvent failed, ret:%{public}" PRId32 ", id:%{public}" PRId32, ret, keyEvent->GetId());
            DispatchKeyEventCallback(keyEvent, isConsumed);
            return;
        }
        if (isConsumedFuture.wait_for(std::chrono::milliseconds(DISPATCH_KEY_EVENT_TIMEOUT_TIME_MS)) ==
            std::future_status::timeout) {
            *isTimeout = true;
            isConsumed = true;
            WLOGFE("DispatchKeyEvent timeout, id:%{public}" PRId32, keyEvent->GetId());
        } else {
            isConsumed = isConsumedFuture.get();
        }
        WLOGFD("Input Method DispatchKeyEvent isConsumed:%{public}" PRId32, isConsumed);
        return;
    }
#endif // IMF_ENABLE
    DispatchKeyEventCallback(keyEvent, isConsumed);
}

void WindowExtensionSessionImpl::ArkUIFrameworkSupport()
{
    uint32_t version = 0;
    if ((context_ != nullptr) && (context_->GetApplicationInfo() != nullptr)) {
        version = context_->GetApplicationInfo()->apiCompatibleVersion;
    }
    // 10 ArkUI new framework support after API10
    if (version < 10) {
        SetLayoutFullScreenByApiVersion(isIgnoreSafeArea_);
        if (!isSystembarPropertiesSet_) {
            SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, SystemBarProperty());
        }
    } else if (isIgnoreSafeAreaNeedNotify_) {
        SetLayoutFullScreenByApiVersion(isIgnoreSafeArea_);
    }
}

std::unique_ptr<Ace::UIContent> WindowExtensionSessionImpl::CreateUIContent(AppExecFwk::Ability* ability, void* env,
    EnvironmentType envType)
{
    if (envType == EnvironmentType::ANI) {
        return ability != nullptr ? Ace::UIContent::Create(ability) :
            Ace::UIContent::CreateWithAniEnv(GetContext().get(), reinterpret_cast<ani_env*>(env));
    } else {
        return ability != nullptr ? Ace::UIContent::Create(ability) :
            Ace::UIContent::Create(GetContext().get(), reinterpret_cast<NativeEngine*>(env));
    }
}
 
WMError WindowExtensionSessionImpl::NapiSetUIContent(const std::string& contentInfo, napi_env env, napi_value storage,
    BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability)
{
    return SetUIContentInner(contentInfo, env, storage, token, ability, false, EnvironmentType::NAPI);
}

WMError WindowExtensionSessionImpl::AniSetUIContent(const std::string& contentInfo, ani_env* env, ani_object storage,
    BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability)
{
    return SetUIContentInner(contentInfo, env, storage, token, ability, false, EnvironmentType::ANI);
}

WMError WindowExtensionSessionImpl::NapiSetUIContentByName(const std::string& contentName, napi_env env,
    napi_value storage, BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "name: %{public}s", contentName.c_str());
    return SetUIContentInner(contentName, env, storage, token, ability, true, EnvironmentType::NAPI);
}

WMError WindowExtensionSessionImpl::AniSetUIContentByName(const std::string& contentName, ani_env* env,
    ani_object storage, BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "name: %{public}s", contentName.c_str());
    return SetUIContentInner(contentName, env, storage, token, ability, true, EnvironmentType::ANI);
}

WMError WindowExtensionSessionImpl::SetUIContentInner(const std::string& contentInfo, void* env, void* storage,
    sptr<IRemoteObject> token, AppExecFwk::Ability* ability, bool initByName, EnvironmentType envType)
{
    WLOGFD("%{public}s state:%{public}u", contentInfo.c_str(), state_);
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->Destroy();
    }
    {
        std::unique_ptr<Ace::UIContent> uiContent = CreateUIContent(ability, static_cast<void*>(env), envType);
        if (uiContent == nullptr) {
            WLOGFE("failed, id: %{public}d", GetPersistentId());
            return WMError::WM_ERROR_NULLPTR;
        }
        uiContent->SetParentToken(token);
        auto usage = property_->GetUIExtensionUsage();
        if (usage == UIExtensionUsage::CONSTRAINED_EMBEDDED) {
            uiContent->SetUIContentType(Ace::UIContentType::SECURITY_UI_EXTENSION);
        } else if (usage == UIExtensionUsage::EMBEDDED) {
            uiContent->SetUIContentType(Ace::UIContentType::UI_EXTENSION);
        } else if (usage == UIExtensionUsage::MODAL) {
            uiContent->SetUIContentType(Ace::UIContentType::MODAL_UI_EXTENSION);
        } else if (usage == UIExtensionUsage::PREVIEW_EMBEDDED) {
            uiContent->SetUIContentType(Ace::UIContentType::PREVIEW_UI_EXTENSION);
        }
        uiContent->SetHostParams(extensionConfig_);
        if (initByName) {
            if (envType == EnvironmentType::ANI) {
                uiContent->InitializeByNameWithAniStorage(this, contentInfo, static_cast<ani_object>(storage),
                    property_->GetParentId());
            } else {
                uiContent->InitializeByName(this, contentInfo, static_cast<napi_value>(storage),
                    property_->GetParentId());
            }
        } else {
            if (envType == EnvironmentType::ANI) {
                uiContent->InitializeWithAniStorage(this, contentInfo, static_cast<ani_object>(storage),
                    property_->GetParentId());
            } else {
                uiContent->Initialize(this, contentInfo, static_cast<napi_value>(storage), property_->GetParentId());
            }
        }
        // make uiContent available after Initialize/Restore
        std::unique_lock<std::shared_mutex> lock(uiContentMutex_);
        uiContent_ = std::move(uiContent);
    }
    SetUIContentComplete();
    NotifyModalUIExtensionMayBeCovered(true);
    SetLayoutFullScreen(immersiveModeEnabled_);

    UpdateAccessibilityTreeInfo();
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "uiContent is nullptr.");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (focusState_ != std::nullopt) {
        focusState_.value() ? uiContent->Focus() : uiContent->UnFocus();
    }
    ArkUIFrameworkSupport();
    UpdateDecorEnable(true);
    if (state_ == WindowState::STATE_SHOWN) {
        // UIContent may be nullptr when show window, need to notify again when window is shown
        uiContent->Foreground();
        UpdateTitleButtonVisibility();
    }
    UpdateViewportConfig(GetRect(), WindowSizeChangeReason::UNDEFINED);
    WLOGFD("notify uiContent window size change end");
    return WMError::WM_OK;
}

WSError WindowExtensionSessionImpl::UpdateRect(const WSRect& rect, SizeChangeReason reason,
    const SceneAnimationConfig& config, const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    auto wmReason = static_cast<WindowSizeChangeReason>(reason);
    Rect wmRect = {rect.posX_, rect.posY_, rect.width_, rect.height_};
    auto preRect = GetRect();
    if (rect.width_ == static_cast<int>(preRect.width_) && rect.height_ == static_cast<int>(preRect.height_)) {
        WLOGFD("EQ [%{public}d, %{public}d, reason: %{public}d]", rect.width_,
            rect.height_, static_cast<int>(reason));
    } else {
        WLOGFI("[%{public}d, %{public}d, reason: %{public}d]", rect.width_,
            rect.height_, static_cast<int>(reason));
    }
    property_->SetWindowRect(wmRect);

    if (property_->GetUIExtensionUsage() == UIExtensionUsage::MODAL) {
        if (!abilityToken_) {
            TLOGE(WmsLogTag::WMS_UIEXT, "token is nullptr");
            return WSError::WS_ERROR_NULLPTR;
        }
        SingletonContainer::Get<WindowAdapter>().UpdateModalExtensionRect(abilityToken_, wmRect);
    }

    if (wmReason == WindowSizeChangeReason::ROTATION || wmReason == WindowSizeChangeReason::SNAPSHOT_ROTATION) {
        const std::shared_ptr<RSTransaction>& rsTransaction = config.rsTransaction_;
        UpdateRectForRotation(wmRect, preRect, wmReason, rsTransaction, avoidAreas);
    } else if (handler_ != nullptr) {
        UpdateRectForOtherReason(wmRect, wmReason, avoidAreas);
    } else {
        NotifySizeChange(wmRect, wmReason);
        UpdateViewportConfig(wmRect, wmReason, nullptr, nullptr, avoidAreas);
    }
    return WSError::WS_OK;
}

void WindowExtensionSessionImpl::UpdateRectForRotation(const Rect& wmRect, const Rect& preRect,
    WindowSizeChangeReason wmReason, const std::shared_ptr<RSTransaction>& rsTransaction,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    if (!handler_) {
        return;
    }
    auto task = [weak = wptr(this), wmReason, wmRect, preRect, rsTransaction, avoidAreas]() mutable {
        HITRACE_METER_NAME(HITRACE_TAG_WINDOW_MANAGER, "WindowExtensionSessionImpl::UpdateRectForRotation");
        auto window = weak.promote();
        if (!window) {
            return;
        }
        int32_t duration = UIEXTENTION_ROTATION_ANIMATION_TIME;
        bool needSync = false;
        if (rsTransaction && rsTransaction->GetSyncId() > 0) {
            // extract high 32 bits of SyncId as pid
            auto SyncTransactionPid = static_cast<int32_t>(rsTransaction->GetSyncId() >> 32);
            if (rsTransaction->IsOpenSyncTransaction() || SyncTransactionPid != rsTransaction->GetParentPid()) {
                needSync = true;
            }
        }

        auto rsUIContext = window->GetRSUIContext();
        if (needSync) {
            window->UpdateRotateDuration(wmReason, duration, rsTransaction);
            RSTransactionAdapter::FlushImplicitTransaction(rsUIContext);
            rsTransaction->Begin();
        }
        window->rotationAnimationCount_++;
        RSAnimationTimingProtocol protocol;
        protocol.SetDuration(duration);
        // animation curve: cubic [0.2, 0.0, 0.2, 1.0]
        auto curve = RSAnimationTimingCurve::CreateCubicCurve(0.2, 0.0, 0.2, 1.0);
        RSNode::OpenImplicitAnimation(rsUIContext, protocol, curve, [weak]() {
            auto window = weak.promote();
            if (!window) {
                return;
            }
            window->rotationAnimationCount_--;
            if (window->rotationAnimationCount_ == 0) {
                window->NotifyRotationAnimationEnd();
            }
        });
        if (wmRect != preRect) {
            window->NotifySizeChange(wmRect, wmReason);
        }
        window->UpdateViewportConfig(wmRect, wmReason, rsTransaction, nullptr, avoidAreas);
        RSNode::CloseImplicitAnimation(rsUIContext);
        if (needSync) {
            rsTransaction->Commit();
        } else {
            RSTransactionAdapter::FlushImplicitTransaction(rsUIContext);
        }
    };
    handler_->PostTask(task, "WMS_WindowExtensionSessionImpl_UpdateRectForRotation");
}

void WindowExtensionSessionImpl::UpdateRotateDuration(WindowSizeChangeReason& reason, int32_t& duration,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    if (reason == WindowSizeChangeReason::SNAPSHOT_ROTATION) {
        duration = rsTransaction->GetDuration();
        reason = WindowSizeChangeReason::ROTATION;
    } else {
        duration = rsTransaction->GetDuration() ? rsTransaction->GetDuration() : duration;
    }
}

void WindowExtensionSessionImpl::UpdateRectForOtherReason(const Rect& wmRect, WindowSizeChangeReason wmReason,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    auto task = [weak = wptr(this), wmReason, wmRect, avoidAreas] {
        auto window = weak.promote();
        if (!window) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "window is null, updateViewPortConfig failed");
            return;
        }
        window->NotifySizeChange(wmRect, wmReason);
        window->UpdateViewportConfig(wmRect, wmReason, nullptr, nullptr, avoidAreas);
    };
    if (handler_) {
        handler_->PostTask(task, "WMS_WindowExtensionSessionImpl_UpdateRectForOtherReason");
    }
}

WMError WindowExtensionSessionImpl::GetSystemViewportConfig(SessionViewportConfig& config)
{
    config.displayId_ = property_->GetDisplayId();
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(config.displayId_);
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "display is null!");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "displayInfo is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    config.density_ = GetDefaultDensity(displayInfo);
    auto rotation = ONE_FOURTH_FULL_CIRCLE_DEGREE * static_cast<uint32_t>(displayInfo->GetOriginRotation());
    auto deviceRotation = static_cast<uint32_t>(displayInfo->GetDefaultDeviceRotationOffset());
    config.transform_ = (rotation + deviceRotation) % FULL_CIRCLE_DEGREE;
    config.orientation_ = static_cast<int32_t>(displayInfo->GetDisplayOrientation());
    return WMError::WM_OK;
}

void WindowExtensionSessionImpl::UpdateSystemViewportConfig()
{
    if (!handler_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "handler_ is null");
        return;
    }
    auto task = [weak = wptr(this)]() {
        auto window = weak.promote();
        if (!window) {
            return;
        }
        if (window->isDensityCustomized_) {
            TLOGNI(WmsLogTag::WMS_UIEXT, "UpdateSystemViewportConfig: Density is customized");
            return;
        }
        if (window->isDensityFollowHost_) {
            TLOGNW(WmsLogTag::WMS_UIEXT, "UpdateSystemViewportConfig: Density is follow host");
            return;
        }
        SessionViewportConfig config;
        if (window->GetSystemViewportConfig(config) != WMError::WM_OK) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "UpdateSystemViewportConfig: Get system viewportConfig failed");
            return;
        }
        if (!MathHelper::NearZero(window->lastDensity_ - config.density_)) {
            TLOGNI(WmsLogTag::WMS_UIEXT, "UpdateSystemViewportConfig: System density is changed");
            window->UpdateSessionViewportConfig(config);
        }
    };
    handler_->PostTask(task, "UpdateSystemViewportConfig");
}

WSError WindowExtensionSessionImpl::UpdateSessionViewportConfig(const SessionViewportConfig& config)
{
    if (config.isDensityFollowHost_ && std::islessequal(config.density_, 0.0f)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "invalid density_: %{public}f", config.density_);
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!handler_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "handler_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    auto task = [weak = wptr(this), config]() {
        auto window = weak.promote();
        if (!window) {
            return;
        }
        auto viewportConfig = config;
        window->UpdateExtensionDensity(viewportConfig);

        TLOGNI(WmsLogTag::WMS_UIEXT, "UpdateSessionViewportConfig: Id:%{public}d, isDensityFollowHost_:%{public}d, "
            "displayId:%{public}" PRIu64", density:%{public}f, lastDensity:%{public}f, orientation:%{public}d, "
            "lastOrientation:%{public}d, isDensityCustomized:%{public}d",
            window->GetPersistentId(), viewportConfig.isDensityFollowHost_, viewportConfig.displayId_,
            viewportConfig.density_, window->lastDensity_, viewportConfig.orientation_, window->lastOrientation_,
            window->isDensityCustomized_);

        window->NotifyDisplayInfoChange(viewportConfig);
        window->property_->SetDisplayId(viewportConfig.displayId_);

        auto ret = window->UpdateSessionViewportConfigInner(viewportConfig);
        if (ret == WSError::WS_OK) {
            window->lastDensity_ = viewportConfig.density_;
            window->lastOrientation_ = viewportConfig.orientation_;
            window->lastDisplayId_ = viewportConfig.displayId_;
            window->lastTransform_ = viewportConfig.transform_;
        }
    };
    handler_->PostTask(task, "UpdateSessionViewportConfig");
    return WSError::WS_OK;
}

WMError WindowExtensionSessionImpl::SetUIExtCustomDensity(const float density)
{
    if (std::islessequal(density, 0.0f)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Invalid density_: %{public}f", density);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (!handler_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "handler_ is null");
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    auto task = [weak = wptr(this), density]() {
        auto window = weak.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "window is null");
            return;
        }
        TLOGNI(WmsLogTag::WMS_UIEXT, "Customize density:%{public}f", density);
        window->customizedDensity_ = density;
        window->isDensityCustomized_ = true;

        SessionViewportConfig config;
        config.isDensityFollowHost_ = false;
        config.density_ = density;
        config.orientation_ = window->lastOrientation_;
        config.displayId_ = window->lastDisplayId_;
        config.transform_ = window->lastTransform_;
        window->UpdateSessionViewportConfig(config);
    };
    handler_->PostTask(task, "SetUIExtCustomDensity");
    return WMError::WM_OK;
}

void WindowExtensionSessionImpl::UpdateExtensionDensity(SessionViewportConfig& config)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "isFollowHost:%{public}d, densityValue:%{public}f,isDensityCustomized_:%{public}d, "
        "customizedDensity:%{public}f", config.isDensityFollowHost_, config.density_, isDensityCustomized_,
        customizedDensity_);
    if (isDensityCustomized_) {
        if (config.isDensityFollowHost_) {
            hostDensityValue_ = config.density_;
        }
        config.density_ = customizedDensity_;
        return;
    }
    isDensityFollowHost_ = config.isDensityFollowHost_;
    if (config.isDensityFollowHost_) {
        hostDensityValue_ = config.density_;
        return;
    }
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(config.displayId_);
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "display is null!");
        return;
    }
    auto displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "displayInfo is null");
        return;
    }
    config.density_ = GetDefaultDensity(displayInfo);
}

void WindowExtensionSessionImpl::NotifyDisplayInfoChange(const SessionViewportConfig& config)
{
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(config.displayId_);
    if (display != nullptr) {
        auto displayInfo = display->GetDisplayInfo();
        if (displayInfo != nullptr && !MathHelper::NearZero(lastSystemDensity_ - displayInfo->GetVirtualPixelRatio())) {
            auto density = displayInfo->GetVirtualPixelRatio();
            lastSystemDensity_ = density;
            NotifySystemDensityChange(density);
        }
    }
    if (context_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "get token of window:%{public}d failed because of context is null.",
            GetPersistentId());
        return;
    }
    auto token = context_->GetToken();
    if (token == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "get token of window:%{public}d failed.", GetPersistentId());
        return;
    }
    if (config.displayId_ != lastDisplayId_) {
        NotifyDisplayIdChange(config.displayId_);
    }
    SingletonContainer::Get<WindowManager>().NotifyDisplayInfoChange(
        token, config.displayId_, config.density_, static_cast<DisplayOrientation>(config.orientation_));
}

WSError WindowExtensionSessionImpl::UpdateSessionViewportConfigInner(const SessionViewportConfig& config)
{
    if (NearEqual(lastDensity_, config.density_) && lastOrientation_ == config.orientation_ &&
        lastDisplayId_ == config.displayId_) {
        TLOGI(WmsLogTag::WMS_UIEXT, "No parameters have changed, no need to update");
        return WSError::WS_DO_NOTHING;
    }
    Ace::ViewportConfig viewportConfig;
    auto rect = GetRect();
    viewportConfig.SetSize(rect.width_, rect.height_);
    viewportConfig.SetPosition(rect.posX_, rect.posY_);
    viewportConfig.SetDensity(config.density_);
    viewportConfig.SetOrientation(config.orientation_);
    viewportConfig.SetTransformHint(config.transform_);
    viewportConfig.SetDisplayId(config.displayId_);

    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGW(WmsLogTag::WMS_UIEXT, "uiContent is null!");
        return WSError::WS_ERROR_NULLPTR;
    }
    virtualPixelRatio_ = config.density_;
    uiContent->UpdateViewportConfig(viewportConfig, WindowSizeChangeReason::UNDEFINED, nullptr, lastAvoidAreaMap_);
    return WSError::WS_OK;
}

WSError WindowExtensionSessionImpl::NotifyAccessibilityHoverEvent(float pointX, float pointY, int32_t sourceType,
    int32_t eventType, int64_t timeMs)
{
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        WLOGFE("error, no uiContent");
        return WSError::WS_ERROR_NO_UI_CONTENT_ERROR;
    }
    uiContent->HandleAccessibilityHoverEvent(pointX, pointY, sourceType, eventType, timeMs);
    return WSError::WS_OK;
}

WMError WindowExtensionSessionImpl::TransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
    int64_t uiExtensionIdLevel)
{
    if (IsWindowSessionInvalid()) {
        WLOGFE("Window session invalid.");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    return static_cast<WMError>(hostSession->TransferAccessibilityEvent(info, uiExtensionIdLevel));
}

void WindowExtensionSessionImpl::NotifySessionForeground(uint32_t reason, bool withAnimation)
{
}

void WindowExtensionSessionImpl::NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits)
{
}

void WindowExtensionSessionImpl::NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info,
    const std::shared_ptr<RSTransaction>& rsTransaction,
    const Rect& callingSessionRect, const std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    if (handler_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "handler is nullptr");
        return;
    }
    auto task = [weak = wptr(this), info, rsTransaction, callingSessionRect, avoidAreas]() {
        if (info != nullptr) {
            TLOGNI(WmsLogTag::WMS_KEYBOARD, "TextFieldPosY: %{public}f, KeyBoardHeight: %{public}d",
                info->textFieldPositionY_, info->rect_.height_);
        }
        auto window = weak.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "window is nullptr, notify occupied area info failed");
            return;
        }
        window->NotifyOccupiedAreaChange(info);
        window->occupiedAreaInfo_ = info;
        window->UpdateViewportConfig(
            window->GetRect(), WindowSizeChangeReason::OCCUPIED_AREA_CHANGE, rsTransaction, nullptr, avoidAreas);
    };
    handler_->PostTask(task, "WMS_WindowExtensionSessionImpl_NotifyOccupiedAreaChangeInfo");
}

WMError WindowExtensionSessionImpl::RegisterOccupiedAreaChangeListener(
    const sptr<IOccupiedAreaChangeListener>& listener)
{
    std::lock_guard<std::mutex> lockListener(occupiedAreaChangeListenerMutex_);
    return RegisterListener(occupiedAreaChangeListenerList_, listener);
}

WMError WindowExtensionSessionImpl::UnregisterOccupiedAreaChangeListener(
    const sptr<IOccupiedAreaChangeListener>& listener)
{
    std::lock_guard<std::mutex> lockListener(occupiedAreaChangeListenerMutex_);
    return UnregisterListener(occupiedAreaChangeListenerList_, listener);
}

WMError WindowExtensionSessionImpl::GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea,
    const Rect& rect, int32_t apiVersion)
{
    uint32_t currentApiVersion = GetTargetAPIVersionByApplicationInfo();
    apiVersion = (apiVersion == API_VERSION_INVALID) ? static_cast<int32_t>(currentApiVersion) : apiVersion;
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_NULLPTR);
    WSRect sessionRect = { rect.posX_, rect.posY_, rect.width_, rect.height_ };
    avoidArea = hostSession->GetAvoidAreaByType(type, sessionRect, apiVersion);
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}d type %{public}d api %{public}d area %{public}s",
        GetPersistentId(), type, apiVersion, avoidArea.ToString().c_str());
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    return RegisterExtensionAvoidAreaChangeListener(listener);
}

WMError WindowExtensionSessionImpl::UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    return UnregisterExtensionAvoidAreaChangeListener(listener);
}

WMError WindowExtensionSessionImpl::Show(uint32_t reason, bool withAnimation, bool withFocus)
{
    return Show(reason, withAnimation, withFocus, false);
}

WMError WindowExtensionSessionImpl::Show(uint32_t reason, bool withAnimation, bool withFocus, bool waitAttach)
{
    CheckAndAddExtWindowFlags();
    UpdateSystemViewportConfig();
    return WindowSessionImpl::Show(reason, withAnimation, withFocus, waitAttach);
}

WMError WindowExtensionSessionImpl::Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits)
{
    return Hide(reason, withAnimation, isFromInnerkits, false);
}

WMError WindowExtensionSessionImpl::Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits, bool waitDetach)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d Hide, reason:%{public}u, state:%{public}u",
        GetPersistentId(), reason, state_);
    if (IsWindowSessionInvalid()) {
        WLOGFE("session invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_ERROR_IF_NULL(hostSession, WMError::WM_ERROR_INVALID_WINDOW);
    CheckAndRemoveExtWindowFlags();
    if (state_ == WindowState::STATE_HIDDEN || state_ == WindowState::STATE_CREATED) {
        TLOGD(WmsLogTag::WMS_LIFE, "already hidden [name:%{public}s, id:%{public}d, type: %{public}u]",
            property_->GetWindowName().c_str(), GetPersistentId(), property_->GetWindowType());
        NotifyBackgroundFailed(WMError::WM_DO_NOTHING);
        return WMError::WM_OK;
    }
    WSError ret = hostSession->Background();
    WMError res = static_cast<WMError>(ret);
    if (res == WMError::WM_OK) {
        StateChangeOption option(GetPersistentId(), WindowState::STATE_HIDDEN, reason, withAnimation, false, false,
            isFromInnerkits, waitDetach);
        UpdateSubWindowStateWithOptions(option);
        state_ = WindowState::STATE_HIDDEN;
        requestState_ = WindowState::STATE_HIDDEN;
        NotifyAfterBackground();
    } else {
        TLOGD(WmsLogTag::WMS_LIFE, "window extension session Hide to Background error");
    }
    return WMError::WM_OK;
}

WSError WindowExtensionSessionImpl::NotifyDensityFollowHost(bool isFollowHost, float densityValue)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "isFollowHost:%{public}d densityValue:%{public}f", isFollowHost, densityValue);

    if (!isFollowHost && !isDensityFollowHost_) {
        TLOGI(WmsLogTag::WMS_UIEXT, "isFollowHost is false and not change");
        return WSError::WS_OK;
    }

    if (isFollowHost) {
        if (std::islessequal(densityValue, 0.0f)) {
            TLOGE(WmsLogTag::WMS_UIEXT, "densityValue is invalid");
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        if (hostDensityValue_ != std::nullopt &&
            std::abs(hostDensityValue_->load() - densityValue) < std::numeric_limits<float>::epsilon()) {
            TLOGI(WmsLogTag::WMS_UIEXT, "densityValue not change");
            return WSError::WS_OK;
        }
        hostDensityValue_ = densityValue;
    }

    isDensityFollowHost_ = isFollowHost;

    UpdateViewportConfig(GetRect(), WindowSizeChangeReason::UNDEFINED);
    return WSError::WS_OK;
}

float WindowExtensionSessionImpl::GetVirtualPixelRatio(const sptr<DisplayInfo>& displayInfo)
{
    if (isDensityCustomized_) {
        return customizedDensity_;
    }
    if (isDensityFollowHost_ && hostDensityValue_ != std::nullopt) {
        return hostDensityValue_->load();
    }
    return GetDefaultDensity(displayInfo);
}

float WindowExtensionSessionImpl::GetDefaultDensity(const sptr<DisplayInfo>& displayInfo)
{
    if (IsAdaptToSimulationScale()) {
        TLOGD(WmsLogTag::WMS_COMPAT, "id:%{public}d scale mode", GetPersistentId());
        return COMPACT_SIMULATION_SCALE_DPI;
    }
    float vpr = 1.0f;
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "displayInfo is nullptr");
        return vpr;
    }
    return displayInfo->GetVirtualPixelRatio();
}

WMError WindowExtensionSessionImpl::CheckHideNonSecureWindowsPermission(bool shouldHide)
{
    if ((property_->GetUIExtensionUsage() == UIExtensionUsage::MODAL ||
         SessionHelper::IsSecureUIExtension(property_->GetUIExtensionUsage())) && !shouldHide) {
        if (!SessionPermission::VerifyCallingPermission("ohos.permission.ALLOW_SHOW_NON_SECURE_WINDOWS")) {
            extensionWindowFlags_.hideNonSecureWindowsFlag = true;
            TLOGE(WmsLogTag::WMS_UIEXT, "Permission denied in %{public}s UIExtension.",
                property_->GetUIExtensionUsage() == UIExtensionUsage::MODAL ? "modal" : "constrained embedded");
            return WMError::WM_ERROR_INVALID_OPERATION;
        }
        if (modalUIExtensionMayBeCovered_) {
            ReportModalUIExtensionMayBeCovered(modalUIExtensionSelfLoadContent_);
        }
    }
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::HideNonSecureWindows(bool shouldHide)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "Id: %{public}u, shouldHide: %{public}u", GetPersistentId(), shouldHide);
    WMError checkRet = CheckHideNonSecureWindowsPermission(shouldHide);
    if (checkRet != WMError::WM_OK) {
        return checkRet;
    }

    if (state_ != WindowState::STATE_SHOWN) {
        extensionWindowFlags_.hideNonSecureWindowsFlag = shouldHide;
        return WMError::WM_OK;
    }
    if (shouldHide == extensionWindowFlags_.hideNonSecureWindowsFlag) {
        return WMError::WM_OK;
    }

    auto updateFlags = extensionWindowFlags_;
    updateFlags.hideNonSecureWindowsFlag = shouldHide;
    ExtensionWindowFlags actions(0);
    actions.hideNonSecureWindowsFlag = true;
    auto ret = UpdateExtWindowFlags(updateFlags, actions);
    if (ret == WMError::WM_OK) {
        extensionWindowFlags_ = updateFlags;
    }
    return ret;
}

WMError WindowExtensionSessionImpl::SetWaterMarkFlag(bool isEnable)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "Id: %{public}u, isEnable: %{public}u", GetPersistentId(), isEnable);
    if (state_ != WindowState::STATE_SHOWN) {
        extensionWindowFlags_.waterMarkFlag = isEnable;
        return WMError::WM_OK;
    }
    if (isEnable == extensionWindowFlags_.waterMarkFlag) {
        return WMError::WM_OK;
    }

    auto updateFlags = extensionWindowFlags_;
    updateFlags.waterMarkFlag = isEnable;
    ExtensionWindowFlags actions(0);
    actions.waterMarkFlag = true;
    auto ret = UpdateExtWindowFlags(updateFlags, actions);
    if (ret == WMError::WM_OK) {
        extensionWindowFlags_ = updateFlags;
    }
    return ret;
}

void WindowExtensionSessionImpl::CheckAndAddExtWindowFlags()
{
    if (extensionWindowFlags_.bitData != 0) {
        // If flag is true, make it active when foreground
        UpdateExtWindowFlags(extensionWindowFlags_, extensionWindowFlags_);
    }
}

void WindowExtensionSessionImpl::CheckAndRemoveExtWindowFlags()
{
    if (extensionWindowFlags_.bitData != 0) {
        // If flag is true, make it inactive when background
        UpdateExtWindowFlags(ExtensionWindowFlags(), extensionWindowFlags_);
    }
}

WSError WindowExtensionSessionImpl::NotifyAccessibilityChildTreeRegister(
    uint32_t windowId, int32_t treeId, int64_t accessibilityId)
{
    if (!handler_) {
        return WSError::WS_ERROR_INTERNAL_ERROR;
    }
    auto uiContentSharedPtr = GetUIContentSharedPtr();
    if (uiContentSharedPtr == nullptr) {
        accessibilityChildTreeInfo_ = {
            .windowId = windowId,
            .treeId = treeId,
            .accessibilityId = accessibilityId
        };
        TLOGD(WmsLogTag::WMS_UIEXT, "uiContent is null, save the accessibility child tree info.");
        return WSError::WS_OK;
    }
    handler_->PostTask([uiContent = uiContentSharedPtr, windowId, treeId, accessibilityId]() {
        if (uiContent == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "NotifyAccessibilityChildTreeRegister error, no uiContent");
            return;
        }
        TLOGND(WmsLogTag::WMS_UIEXT,
            "NotifyAccessibilityChildTreeRegister: %{public}d %{public}" PRId64, treeId, accessibilityId);
        uiContent->RegisterAccessibilityChildTree(windowId, treeId, accessibilityId);
    });
    return WSError::WS_OK;
}

WSError WindowExtensionSessionImpl::NotifyAccessibilityChildTreeUnregister()
{
    if (!handler_) {
        return WSError::WS_ERROR_INTERNAL_ERROR;
    }
    handler_->PostTask([uiContent = GetUIContentSharedPtr()]() {
        if (uiContent == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "NotifyAccessibilityChildTreeUnregister error, no uiContent");
            return;
        }
        uiContent->DeregisterAccessibilityChildTree();
    });
    return WSError::WS_OK;
}

WSError WindowExtensionSessionImpl::NotifyAccessibilityDumpChildInfo(
    const std::vector<std::string>& params, std::vector<std::string>& info)
{
    if (!handler_) {
        return WSError::WS_ERROR_INTERNAL_ERROR;
    }
    handler_->PostSyncTask([uiContent = GetUIContentSharedPtr(), params, &info]() {
        if (uiContent == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "NotifyAccessibilityDumpChildInfo error, no uiContent");
            return;
        }
        uiContent->AccessibilityDumpChildInfo(params, info);
    });
    return WSError::WS_OK;
}

void WindowExtensionSessionImpl::UpdateAccessibilityTreeInfo()
{
    if (accessibilityChildTreeInfo_ == std::nullopt) {
        return;
    }
    std::shared_ptr<Ace::UIContent> uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        return;
    }
    uiContent->RegisterAccessibilityChildTree(accessibilityChildTreeInfo_->windowId,
        accessibilityChildTreeInfo_->treeId, accessibilityChildTreeInfo_->accessibilityId);
    accessibilityChildTreeInfo_.reset();
}

WMError WindowExtensionSessionImpl::UpdateExtWindowFlags(const ExtensionWindowFlags& flags,
    const ExtensionWindowFlags& actions)
{
    // action is true when the corresponding flag should be updated
    if (IsWindowSessionInvalid()) {
        TLOGI(WmsLogTag::WMS_UIEXT, "session is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }

    if (!abilityToken_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "token is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }

    return SingletonContainer::Get<WindowAdapter>().UpdateExtWindowFlags(abilityToken_, flags.bitData, actions.bitData);
}

Rect WindowExtensionSessionImpl::GetHostWindowRect(int32_t hostWindowId)
{
    Rect rect;
    if (hostWindowId != property_->GetParentId()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "hostWindowId is invalid");
        return rect;
    }
    SingletonContainer::Get<WindowAdapter>().GetHostWindowRect(hostWindowId, rect);
    return rect;
}

WMError WindowExtensionSessionImpl::GetGlobalScaledRect(Rect& globalScaledRect)
{
    return SingletonContainer::Get<WindowAdapter>().GetHostGlobalScaledRect(property_->GetParentId(), globalScaledRect);
}

WMError WindowExtensionSessionImpl::GetGestureBackEnabled(bool& enable) const
{
    enable = hostGestureBackEnabled_;
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::SetGestureBackEnabled(bool enable)
{
    AAFwk::WantParams want;
    want.SetParam(Extension::ATOMICSERVICE_KEY_FUNCTION, AAFwk::String::Box("setGestureBackEnabled"));
    want.SetParam(Extension::ATOMICSERVICE_KEY_PARAM_ENABLE, AAFwk::Boolean::Box(enable));
    if (TransferExtensionData(want) != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "send failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::SetLayoutFullScreen(bool status)
{
    TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}u %{public}s] status %{public}d",
        GetWindowId(), GetWindowName().c_str(), static_cast<int32_t>(status));
    if (IsWindowSessionInvalid()) {
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (auto hostSession = GetHostSession()) {
        hostSession->OnLayoutFullScreenChange(status);
    }
    property_->SetIsLayoutFullScreen(status);
    isIgnoreSafeArea_ = status;
    isIgnoreSafeAreaNeedNotify_ = true;
    immersiveModeEnabled_ = status;
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SetIgnoreViewSafeArea(status);
    }
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::UpdateSystemBarProperties(
    const std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
    const std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags)
{
    AAFwk::WantParams want;
    want.SetParam(Extension::ATOMICSERVICE_KEY_FUNCTION, AAFwk::String::Box("setWindowSystemBarEnable"));
    want.SetParam(Extension::ATOMICSERVICE_KEY_PARAM_STATUS,
        AAFwk::Boolean::Box(systemBarProperties.at(WindowType::WINDOW_TYPE_STATUS_BAR).enable_));
    want.SetParam(Extension::ATOMICSERVICE_KEY_PARAM_NAVIGATION,
        AAFwk::Boolean::Box(systemBarProperties.at(WindowType::WINDOW_TYPE_NAVIGATION_BAR).enable_));
    if (TransferExtensionData(want) != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "send failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::UpdateHostSpecificSystemBarEnabled(const std::string& name, bool enable,
    bool enableAnimation)
{
    AAFwk::WantParams want;
    want.SetParam(Extension::ATOMICSERVICE_KEY_FUNCTION, AAFwk::String::Box("setSpecificSystemBarEnabled"));
    want.SetParam(Extension::ATOMICSERVICE_KEY_PARAM_NAME, AAFwk::String::Box(name));
    want.SetParam(Extension::ATOMICSERVICE_KEY_PARAM_ENABLE, AAFwk::Boolean::Box(enable));
    want.SetParam(Extension::ATOMICSERVICE_KEY_PARAM_ENABLEANIMATION, AAFwk::Boolean::Box(enableAnimation));
    if (TransferExtensionData(want) != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "send failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

bool WindowExtensionSessionImpl::GetImmersiveModeEnabledState() const
{
    return immersiveModeEnabled_;
}

WMError WindowExtensionSessionImpl::SetImmersiveModeEnabledState(bool enable)
{
    return SetLayoutFullScreen(enable);
}

WMError WindowExtensionSessionImpl::ExtensionSetKeepScreenOn(bool keepScreenOn)
{
    AAFwk::WantParams want;
    want.SetParam(Extension::ATOMICSERVICE_KEY_FUNCTION, AAFwk::String::Box("setWindowKeepScreenOn"));
    want.SetParam(Extension::ATOMICSERVICE_KEY_PARAM_ISKEEPSCREENON, AAFwk::Boolean::Box(keepScreenOn));
    if (TransferExtensionData(want) != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "send failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::ExtensionSetBrightness(float brightness)
{
    AAFwk::WantParams want;
    want.SetParam(Extension::ATOMICSERVICE_KEY_FUNCTION, AAFwk::String::Box("setWindowBrightness"));
    want.SetParam(Extension::ATOMICSERVICE_KEY_PARAM_BRIGHTNESS, AAFwk::Float::Box(brightness));
    if (TransferExtensionData(want) != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "send failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::GetWindowStateSnapshot(std::string& winStateSnapshotJsonStr)
{
    auto persistentId = GetPersistentId();
    nlohmann::json winStateSnapshotJson = {
        {"isPcMode", system::GetBoolParameter("persist.sceneboard.ispcmode", false)},
    };
    winStateSnapshotJsonStr = winStateSnapshotJson.dump();
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "ext winId=%{public}d, winStateSnapshot=%{public}s",
        persistentId, winStateSnapshotJsonStr.c_str());
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::SetStatusBarColorForExtension(uint32_t color)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "id=%{public}d", GetPersistentId());
    WMError ret = SetStatusBarColorForExtensionInner(color);
    if (ret == WMError::WM_OK) {
        systemBarSettingFlag_ = SystemBarSettingFlag::COLOR_SETTING;
    }
    return ret;
}

WMError WindowExtensionSessionImpl::SetStatusBarColorForExtensionInner(uint32_t color)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "id=%{public}d", GetPersistentId());
    AAFwk::WantParams want;
    want.SetParam(Extension::ATOMICSERVICE_KEY_FUNCTION, AAFwk::String::Box("setStatusBarColor"));
    want.SetParam(Extension::ATOMICSERVICE_KEY_PARAM_COLOR_NUMERIC, AAFwk::Long::Box(color));
    if (TransferExtensionData(want) != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "send failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

void WindowExtensionSessionImpl::ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "PointerEvent is nullptr, windowId: %{public}d", GetWindowId());
        return;
    }
    if (property_->GetUIExtensionUsage() == UIExtensionUsage::PREVIEW_EMBEDDED) {
        TLOGD(WmsLogTag::WMS_EVENT, "Preview uiext does not handle event, eid:%{public}d", pointerEvent->GetId());
        return;
    }
    if (hostSession_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "hostSession is nullptr, windowId: %{public}d", GetWindowId());
        pointerEvent->MarkProcessed();
        return;
    }

    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        TLOGW(WmsLogTag::WMS_EVENT, "invalid pointerEvent, windowId: %{public}d", GetWindowId());
        pointerEvent->MarkProcessed();
        return;
    }
    auto action = pointerEvent->GetPointerAction();
    bool isPointDown = (action == MMI::PointerEvent::POINTER_ACTION_DOWN ||
        action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    if ((property_->GetUIExtensionUsage() == UIExtensionUsage::MODAL) && isPointDown) {
        if (!abilityToken_) {
            TLOGE(WmsLogTag::WMS_UIEXT, "token is nullptr");
            return;
        }
        SingletonContainer::Get<WindowAdapter>().ProcessModalExtensionPointDown(abilityToken_,
            pointerItem.GetDisplayX(), pointerItem.GetDisplayY());
    }
    if (action != MMI::PointerEvent::POINTER_ACTION_MOVE) {
        TLOGI(WmsLogTag::WMS_EVENT, "eid:%{public}d,wid:%{public}u,"
            "pointId:%{public}d,sourceType:%{public}d", pointerEvent->GetId(), GetWindowId(),
            pointerEvent->GetPointerId(), pointerEvent->GetSourceType());
    }
    NotifyPointerEvent(pointerEvent);
}

void WindowExtensionSessionImpl::NotifyPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "pointerEvent is nullptr, windowId: %{public}d", GetWindowId());
        return;
    }
    WindowSessionImpl::NotifyPointerEvent(pointerEvent);
    if (!IsWindowDelayRaiseEnabled()) {
        return;
    }
    std::shared_ptr<MMI::PointerEvent> pointerEventBackup;
    if (pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
        pointerEventBackup = std::make_shared<MMI::PointerEvent>(*pointerEvent);
    } else {
        pointerEventBackup = pointerEvent;
    }
    auto uiContent = GetUIContentSharedPtr();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "uiContent is null!");
        return;
    }
    uiContent->ProcessPointerEvent(pointerEvent,
        [weakThis = wptr(this), pointerEventBackup](bool isHitTargetDraggable) mutable {
            auto window = weakThis.promote();
            if (window == nullptr) {
                TLOGNE(WmsLogTag::WMS_UIEXT, "window is null!");
                return;
            }
            window->ProcessPointerEventWithHostWindowDelayRaise(pointerEventBackup, isHitTargetDraggable);
        });
}

void WindowExtensionSessionImpl::ProcessPointerEventWithHostWindowDelayRaise(
    const std::shared_ptr<MMI::PointerEvent>& pointerEvent, bool isHitTargetDraggable) const
{
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "pointerEvent is nullptr, windowId: %{public}d", GetWindowId());
        return;
    }
    const int32_t action = pointerEvent->GetPointerAction();
    bool isPointerButtonDown = (action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    bool isPointerUp = (action == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP ||
        action == MMI::PointerEvent::POINTER_ACTION_PULL_UP);
    if (!isPointerButtonDown && !isPointerUp) {
        return;
    }
    bool needNotifyHostWindowToRaise = isPointerUp || !isHitTargetDraggable;
    if (needNotifyHostWindowToRaise) {
        // Send message to host window to raise hierarchy
        auto businessCode = Extension::Businesscode::NOTIFY_HOST_WINDOW_TO_RAISE;
        AAFwk::Want dataToSend;
        if (SendExtensionMessageToHost(static_cast<uint32_t>(businessCode), dataToSend) != WMError::WM_OK) {
            return;
        }
        TLOGI(WmsLogTag::WMS_UIEXT, "Notify host window to raise, id: %{public}d, isHitTargetDraggable: %{public}d, "
            "isPointerUp: %{public}d", GetPersistentId(), isHitTargetDraggable, isPointerUp);
        return;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "No need to notify host window to raise, id: %{public}d, isHitTargetDraggable: "
        "%{public}d, isPointerUp: %{public}d", GetPersistentId(), isHitTargetDraggable, isPointerUp);
}

bool WindowExtensionSessionImpl::PreNotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    if (keyEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "keyEvent is nullptr");
        return false;
    }
    if (property_->GetUIExtensionUsage() == UIExtensionUsage::PREVIEW_EMBEDDED) {
        TLOGD(WmsLogTag::WMS_EVENT, "Preview uiext does not handle event, eid:%{public}d", keyEvent->GetId());
        return false;
    }
    RefreshNoInteractionTimeoutMonitor();
    if (property_->GetUIExtensionUsage() == UIExtensionUsage::CONSTRAINED_EMBEDDED) {
        if (focusState_ == std::nullopt) {
            TLOGW(WmsLogTag::WMS_EVENT, "focusState is null");
            keyEvent->MarkProcessed();
            return true;
        }
        if (!focusState_.value()) {
            keyEvent->MarkProcessed();
            return true;
        }
        TLOGI(WmsLogTag::WMS_EVENT, "eid:%{public}d wid:%{public}d",
            keyEvent->GetId(), keyEvent->GetAgentWindowId());
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        TLOGD(WmsLogTag::WMS_EVENT, "Start to process keyEvent, id: %{public}d", keyEvent->GetId());
        return uiContent->ProcessKeyEvent(keyEvent, true);
    }
    return false;
}

bool WindowExtensionSessionImpl::GetFreeMultiWindowModeEnabledState()
{
    bool enable = false;
    SingletonContainer::Get<WindowAdapter>().GetFreeMultiWindowEnableState(enable);
    TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "enable=%{public}u", enable);
    return enable;
}

void WindowExtensionSessionImpl::NotifyExtensionTimeout(int32_t errorCode)
{
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_IF_NULL(hostSession);
    hostSession->NotifyExtensionTimeout(errorCode);
}

int32_t WindowExtensionSessionImpl::GetRealParentId() const
{
    return property_->GetRealParentId();
}

WindowType WindowExtensionSessionImpl::GetParentWindowType() const
{
    return property_->GetParentWindowType();
}

void WindowExtensionSessionImpl::NotifyModalUIExtensionMayBeCovered(bool byLoadContent)
{
    if (property_->GetUIExtensionUsage() != UIExtensionUsage::MODAL &&
        !SessionHelper::IsSecureUIExtension(property_->GetUIExtensionUsage())) {
        return;
    }

    modalUIExtensionMayBeCovered_ = true;
    if (byLoadContent) {
        modalUIExtensionSelfLoadContent_ = true;
    }
    if (extensionWindowFlags_.hideNonSecureWindowsFlag) {
        return;
    }
    ReportModalUIExtensionMayBeCovered(byLoadContent);
}

void WindowExtensionSessionImpl::ReportModalUIExtensionMayBeCovered(bool byLoadContent) const
{
    TLOGW(WmsLogTag::WMS_UIEXT, "Id=%{public}d", GetPersistentId());
    std::ostringstream oss;
    oss << "Modal UIExtension may be covered uid: " << getuid();
    oss << ", windowName: " << property_->GetWindowName();
    if (context_) {
        oss << ", bundleName: " << context_->GetBundleName();
    }
    auto type = byLoadContent ? WindowDFXHelperType::WINDOW_MODAL_UIEXTENSION_UICONTENT_CHECK :
        WindowDFXHelperType::WINDOW_MODAL_UIEXTENSION_SUBWINDOW_CHECK;
    SingletonContainer::Get<WindowInfoReporter>().ReportWindowException(static_cast<int32_t>(type), getpid(),
        oss.str());
}

void WindowExtensionSessionImpl::NotifyExtensionEventAsync(uint32_t notifyEvent)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "notifyEvent:%{public}d", notifyEvent);
    if (IsWindowSessionInvalid()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Window session invalid.");
        return;
    }
    auto hostSession = GetHostSession();
    CHECK_HOST_SESSION_RETURN_IF_NULL(hostSession);
    hostSession->NotifyExtensionEventAsync(notifyEvent);
}

WSError WindowExtensionSessionImpl::NotifyDumpInfo(const std::vector<std::string>& params,
    std::vector<std::string>& info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "persistentId: %{public}d", GetPersistentId());
    auto uiContentSharedPtr = GetUIContentSharedPtr();
    if (uiContentSharedPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "uiContent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    uiContentSharedPtr->DumpInfo(params, info);
    if (!SessionPermission::IsBetaVersion()) {
        TLOGW(WmsLogTag::WMS_UIEXT, "is not beta version, persistentId: %{public}d", GetPersistentId());
        info.clear();
    }
    return WSError::WS_OK;
}

bool WindowExtensionSessionImpl::IsPcWindow() const
{
    bool isPcWindow = false;
    WMError ret = SingletonContainer::Get<WindowAdapter>().IsPcWindow(isPcWindow);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "can't find isPcWindow, err: %{public}u",
            static_cast<uint32_t>(ret));
    }
    return isPcWindow;
}

bool WindowExtensionSessionImpl::IsPcOrPadFreeMultiWindowMode() const
{
    bool isPcOrPadFreeMultiWindowMode = false;
    WMError ret = SingletonContainer::Get<WindowAdapter>().IsPcOrPadFreeMultiWindowMode(isPcOrPadFreeMultiWindowMode);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "cant't find isPcOrPadFreeMultiWindowMode, err: %{public}u",
            static_cast<uint32_t>(ret));
    }
    return isPcOrPadFreeMultiWindowMode;
}

WSError WindowExtensionSessionImpl::SendExtensionData(MessageParcel& data, MessageParcel& reply,
                                                      [[maybe_unused]] MessageOption& option)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "id=%{public}d", GetPersistentId());
    dataHandler_->NotifyDataConsumer(data, reply);
    return WSError::WS_OK;
}

WindowMode WindowExtensionSessionImpl::GetWindowMode() const
{
    return property_->GetWindowMode();
}

void WindowExtensionSessionImpl::UpdateExtensionConfig(const std::shared_ptr<AAFwk::Want>& want)
{
    if (want == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "null want ptr");
        return;
    }

    const auto& configParam = want->GetParams().GetWantParams(Extension::UIEXTENSION_CONFIG_FIELD);
    auto state = configParam.GetIntParam(Extension::CROSS_AXIS_FIELD, 0);
    if (IsValidCrossState(state)) {
        crossAxisState_ = static_cast<CrossAxisState>(state);
    }
    auto waterfallModeValue = configParam.GetIntParam(Extension::WATERFALL_MODE_FIELD, 0);
    isFullScreenWaterfallMode_.store(static_cast<bool>(waterfallModeValue));
    isValidWaterfallMode_.store(true);
    hostGestureBackEnabled_ = static_cast<bool>(configParam.GetIntParam(Extension::GESTURE_BACK_ENABLED, 1));
    auto isHostWindowDelayRaiseEnabled = configParam.GetIntParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD, 0);
    property_->SetWindowDelayRaiseEnabled(static_cast<bool>(isHostWindowDelayRaiseEnabled));

    extensionConfig_ = AAFwk::WantParams(configParam);
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SetHostParams(extensionConfig_);
    }
    want->RemoveParam(Extension::UIEXTENSION_CONFIG_FIELD);
    auto rootHostWindowType =
        static_cast<WindowType>(configParam.GetIntParam(Extension::ROOT_HOST_WINDOW_TYPE_FIELD, 0));
    SetRootHostWindowType(rootHostWindowType);
    SetCompatInfo(configParam);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "CrossAxisState: %{public}d, waterfall: %{public}d, "
        "rootHostWindowType: %{public}u, isHostWindowDelayRaiseEnabled: %{public}d, winId: %{public}u",
        state, isFullScreenWaterfallMode_.load(), rootHostWindowType, isHostWindowDelayRaiseEnabled, GetWindowId());
}

WMError WindowExtensionSessionImpl::SendExtensionMessageToHost(uint32_t code, const AAFwk::Want& data) const
{
    auto dataHandler = GetExtensionDataHandler();
    if (dataHandler == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "dataHandler_ is nullptr!");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto sendResult = dataHandler->SendDataAsync(SubSystemId::ARKUI_UIEXT, code, data);
    if (sendResult == DataHandlerErr::OK) {
        return WMError::WM_OK;
    }
    TLOGE(WmsLogTag::WMS_UIEXT, "SendDataAsync failed, businessCode: %{public}u, errCode: %{public}u", code,
        sendResult);
    return WMError::WM_ERROR_IPC_FAILED;
}

WMError WindowExtensionSessionImpl::OnExtensionMessage(uint32_t code, int32_t persistentId, const AAFwk::Want& data)
{
    switch (code) {
        case static_cast<uint32_t>(Extension::Businesscode::NOTIFY_HOST_WINDOW_TO_RAISE): {
            return HandleHostWindowRaise(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::REGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER): {
            return HandleRegisterHostWindowRectChangeListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER): {
            return HandleUnregisterHostWindowRectChangeListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::REGISTER_KEYBOARD_DID_SHOW_LISTENER): {
            return HandleUIExtRegisterKeyboardDidShowListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_KEYBOARD_DID_SHOW_LISTENER): {
            return HandleUIExtUnregisterKeyboardDidShowListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::REGISTER_KEYBOARD_DID_HIDE_LISTENER): {
            return HandleUIExtRegisterKeyboardDidHideListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_KEYBOARD_DID_HIDE_LISTENER): {
            return HandleUIExtUnregisterKeyboardDidHideListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::REGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER): {
            return HandleRegisterHostRectChangeInGlobalDisplayListener(code, persistentId, data);
            break;
        }
        case static_cast<uint32_t>(Extension::Businesscode::UNREGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER): {
            return HandleUnregisterHostRectChangeInGlobalDisplayListener(code, persistentId, data);
            break;
        }
        default: {
            TLOGI(WmsLogTag::WMS_UIEXT, "Message was not processed, businessCode: %{public}u", code);
            break;
        }
    }
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::HandleHostWindowRaise(uint32_t code, int32_t persistentId, const AAFwk::Want& data)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "businessCode: %{public}u", code);
    return SendExtensionMessageToHost(code, data);
}

WMError WindowExtensionSessionImpl::HandleRegisterHostWindowRectChangeListener(uint32_t code, int32_t persistentId,
    const AAFwk::Want& data)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "businessCode: %{public}u", code);
    auto ret = SendExtensionMessageToHost(code, data);
    if (ret != WMError::WM_OK) {
        return ret;
    }
    rectChangeUIExtListenerIds_.emplace(persistentId);
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::HandleUnregisterHostWindowRectChangeListener(uint32_t code, int32_t persistentId,
    const AAFwk::Want& data)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "businessCode: %{public}u", code);
    bool isHostWindowRectChangeListenerEmpty = false;
    size_t hostWindowRectChangeListenerSize = 0;
    rectChangeUIExtListenerIds_.erase(persistentId);
    {
        std::lock_guard<std::mutex> lockListener(hostWindowRectChangeListenerMutex_);
        isHostWindowRectChangeListenerEmpty = hostWindowRectChangeListener_.empty();
        hostWindowRectChangeListenerSize = hostWindowRectChangeListener_.size();
    }
    if (!isHostWindowRectChangeListenerEmpty || !rectChangeUIExtListenerIds_.empty()) {
        TLOGI(WmsLogTag::WMS_UIEXT, "No need to send message to host to unregister, size of "
            "hostWindowRectChangeListener_: %{public}zu, size of rectChangeUIExtListenerIds_: %{public}zu",
            hostWindowRectChangeListenerSize, rectChangeUIExtListenerIds_.size());
        return WMError::WM_OK;
    }
    return SendExtensionMessageToHost(code, data);
}

WMError WindowExtensionSessionImpl::HandleRegisterHostRectChangeInGlobalDisplayListener(uint32_t code,
    int32_t persistentId, const AAFwk::Want& data)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "businessCode: %{public}u", code);
    auto ret = SendExtensionMessageToHost(code, data);
    if (ret != WMError::WM_OK) {
        return ret;
    }
    rectChangeInGlobalDisplayUIExtListenerIds_.emplace(persistentId);
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::HandleUnregisterHostRectChangeInGlobalDisplayListener(uint32_t code,
    int32_t persistentId, const AAFwk::Want& data)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "businessCode: %{public}u", code);
    bool needNotifyHost = false;
    rectChangeInGlobalDisplayUIExtListenerIds_.erase(persistentId);
    {
        std::lock_guard<std::mutex> lockListener(hostRectChangeInGlobalDisplayListenerMutex_);
        needNotifyHost = hostRectChangeInGlobalDisplayListenerList_.empty() &&
            rectChangeInGlobalDisplayUIExtListenerIds_.empty();
    }
    if (needNotifyHost) {
        return SendExtensionMessageToHost(code, data);
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "No need to send message to host to unregister, size of "
        "listener: %{public}zu, size of rectChangeUIExtListenerIds_: %{public}zu",
        hostRectChangeInGlobalDisplayListenerList_.size(), rectChangeInGlobalDisplayUIExtListenerIds_.size());
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::SetWindowMode(WindowMode mode)
{
    property_->SetWindowMode(mode);
    if (auto uiContet = GetUIContentSharedPtr()) {
        uiContet->NotifyWindowMode(mode);
    }
    TLOGNI(WmsLogTag::WMS_UIEXT, "windowMode:%{public}u", GetWindowMode());
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::OnCrossAxisStateChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply)
{
    auto state = data.GetIntParam(Extension::CROSS_AXIS_FIELD, 0);
    if (state == static_cast<int32_t>(GetCrossAxisState())) {
        return WMError::WM_OK;
    }
    if (!IsValidCrossState(state)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "invalid CrossAxisState:%{public}d", state);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    crossAxisState_ = static_cast<CrossAxisState>(state);
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SendUIExtProprty(static_cast<uint32_t>(Extension::Businesscode::SYNC_CROSS_AXIS_STATE),
            data, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "CrossAxisState:%{public}d", state);
    auto windowCrossAxisListeners = GetListeners<IWindowCrossAxisListener>();
    for (const auto& listener : windowCrossAxisListeners) {
        if (listener != nullptr) {
            listener->OnCrossAxisChange(static_cast<CrossAxisState>(state));
        }
    }
    return WMError::WM_OK;
}

CrossAxisState WindowExtensionSessionImpl::GetCrossAxisState()
{
    return crossAxisState_.load();
}

WMError WindowExtensionSessionImpl::OnWaterfallModeChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply)
{
    bool isWaterfallMode = data.GetBoolParam(Extension::WATERFALL_MODE_FIELD, false);
    if (isWaterfallMode == isFullScreenWaterfallMode_) {
        return WMError::WM_OK;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "prev: %{public}d, curr: %{public}d, winId: %{public}u",
        isFullScreenWaterfallMode_.load(), isWaterfallMode, GetWindowId());
    isFullScreenWaterfallMode_.store(isWaterfallMode);
    isValidWaterfallMode_.store(true);
    if (auto uiContent = GetUIContentSharedPtr()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "send uiext winId: %{public}u", GetWindowId());
        uiContent->SendUIExtProprty(static_cast<uint32_t>(Extension::Businesscode::SYNC_HOST_WATERFALL_MODE),
            data, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    auto waterfallModeChangeListeners = GetWaterfallModeChangeListeners();
    for (const auto& listener : waterfallModeChangeListeners) {
        if (listener != nullptr) {
            listener->OnWaterfallModeChange(isWaterfallMode);
        }
    }
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::OnResyncExtensionConfig(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply)
{
    const auto& configParam = data.GetParams().GetWantParams(Extension::UIEXTENSION_CONFIG_FIELD);
    AAFwk::Want axisWant;
    axisWant.SetParam(Extension::CROSS_AXIS_FIELD,
        static_cast<int32_t>(configParam.GetIntParam(Extension::CROSS_AXIS_FIELD, 0)));
    OnCrossAxisStateChange(std::move(axisWant), reply);
    AAFwk::Want waterfallWant;
    waterfallWant.SetParam(Extension::WATERFALL_MODE_FIELD,
        static_cast<bool>(configParam.GetIntParam(Extension::WATERFALL_MODE_FIELD, 0)));
    OnWaterfallModeChange(std::move(waterfallWant), reply);
    AAFwk::Want windowDelayRaiseWant;
    windowDelayRaiseWant.SetParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD,
        static_cast<bool>(configParam.GetIntParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD, 0)));
    OnHostWindowDelayRaiseStateChange(std::move(windowDelayRaiseWant), reply);
    AAFwk::Want compatWant;
    compatWant.SetParam(Extension::COMPAT_IS_SIMULATION_SCALE_FIELD,
        static_cast<bool>(configParam.GetIntParam(Extension::COMPAT_IS_SIMULATION_SCALE_FIELD, 0)));
    compatWant.SetParam(Extension::COMPAT_IS_PROPORTION_SCALE_FIELD,
        static_cast<bool>(configParam.GetIntParam(Extension::COMPAT_IS_PROPORTION_SCALE_FIELD, 0)));
    compatWant.SetParam(Extension::COMPAT_SCALE_X_FIELD, GetFloatParam(Extension::COMPAT_SCALE_X_FIELD, configParam));
    compatWant.SetParam(Extension::COMPAT_SCALE_Y_FIELD, GetFloatParam(Extension::COMPAT_SCALE_Y_FIELD, configParam));
    OnHostWindowCompatInfoChange(std::move(compatWant), reply);
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::OnGestureBackEnabledChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply)
{
    auto enable = data.GetBoolParam(Extension::GESTURE_BACK_ENABLED, true);
    if (enable == hostGestureBackEnabled_) {
        return WMError::WM_OK;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "gestureBackEnabled:%{public}d", enable);
    hostGestureBackEnabled_ = enable;
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SendUIExtProprty(static_cast<uint32_t>(Extension::Businesscode::SYNC_HOST_GESTURE_BACK_ENABLED),
            data, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::OnHostWindowDelayRaiseStateChange(AAFwk::Want&& data,
    std::optional<AAFwk::Want>& reply)
{
    bool isHostWindowDelayRaiseEnabled = data.GetBoolParam(Extension::HOST_WINDOW_DELAY_RAISE_STATE_FIELD, false);
    if (isHostWindowDelayRaiseEnabled == IsWindowDelayRaiseEnabled()) {
        return WMError::WM_OK;
    }
    property_->SetWindowDelayRaiseEnabled(isHostWindowDelayRaiseEnabled);
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SendUIExtProprty(static_cast<uint32_t>(Extension::Businesscode::SYNC_HOST_WINDOW_DELAY_RAISE_STATE),
            data, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "isHostWindowDelayRaiseEnabled: %{public}d", isHostWindowDelayRaiseEnabled);
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::OnHostWindowRectChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply)
{
    auto rectX = data.GetIntParam(Extension::RECT_X, 0);
    auto rectY = data.GetIntParam(Extension::RECT_Y, 0);
    auto rectWidth = data.GetIntParam(Extension::RECT_WIDTH, 0);
    auto rectHeight = data.GetIntParam(Extension::RECT_HEIGHT, 0);
    auto reason = static_cast<WindowSizeChangeReason>(data.GetIntParam(Extension::RECT_CHANGE_REASON, 0));
    Rect rect {rectX, rectY, rectWidth, rectHeight};
    TLOGI(WmsLogTag::WMS_UIEXT, "Host window rect change: %{public}s, reason: %{public}u",
        rect.ToString().c_str(), reason);
    {
        std::lock_guard<std::mutex> lockListener(hostWindowRectChangeListenerMutex_);
        for (const auto& listener : hostWindowRectChangeListener_) {
            if (listener != nullptr) {
                listener->OnRectChange(rect, reason);
            }
        }
    }
    auto uiContent = GetUIContentSharedPtr();
    if (!rectChangeUIExtListenerIds_.empty() && uiContent != nullptr) {
        uiContent->SendUIExtProprtyByPersistentId(
            static_cast<uint32_t>(Extension::Businesscode::NOTIFY_HOST_WINDOW_RECT_CHANGE), data,
            rectChangeUIExtListenerIds_, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::OnHostRectChangeInGlobalDisplay(AAFwk::Want&& data,
    std::optional<AAFwk::Want>& reply)
{
    auto rectX = data.GetIntParam(Extension::RECT_X, 0);
    auto rectY = data.GetIntParam(Extension::RECT_Y, 0);
    auto rectWidth = data.GetIntParam(Extension::RECT_WIDTH, 0);
    auto rectHeight = data.GetIntParam(Extension::RECT_HEIGHT, 0);
    auto reason = static_cast<WindowSizeChangeReason>(data.GetIntParam(Extension::RECT_CHANGE_REASON, 0));
    Rect rect {rectX, rectY, rectWidth, rectHeight};
    TLOGI(WmsLogTag::WMS_UIEXT, "Host rect change in global display: %{public}s, reason: %{public}u",
        rect.ToString().c_str(), reason);
    {
        std::lock_guard<std::mutex> lockListener(hostRectChangeInGlobalDisplayListenerMutex_);
        for (const auto& listener : hostRectChangeInGlobalDisplayListenerList_) {
            if (listener != nullptr) {
                listener->OnRectChangeInGlobalDisplay(rect, reason);
            }
        }
    }
    auto uiContent = GetUIContentSharedPtr();
    if (!rectChangeInGlobalDisplayUIExtListenerIds_.empty() && uiContent != nullptr) {
        uiContent->SendUIExtProprtyByPersistentId(
            static_cast<uint32_t>(Extension::Businesscode::NOTIFY_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY), data,
            rectChangeInGlobalDisplayUIExtListenerIds_, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::OnScreenshot(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "in");
    NotifyScreenshot();
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::OnExtensionSecureLimitChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply)
{
    bool limit = data.GetBoolParam(Extension::EXTENSION_SECURE_LIMIT_CHANGE, false);
    TLOGD(WmsLogTag::WMS_UIEXT, "limit: %{public}d", limit);
    NotifyExtensionSecureLimitChange(limit);
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::OnKeyboardDidShow(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "in");
    KeyboardPanelInfo info;
    ReadKeyboardInfoFromWant(data, info);
    NotifyKeyboardDidShow(info);
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::OnKeyboardDidHide(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "in");
    KeyboardPanelInfo info;
    ReadKeyboardInfoFromWant(data, info);
    NotifyKeyboardDidHide(info);
    return WMError::WM_OK;
}

void WindowExtensionSessionImpl::RegisterConsumer(Extension::Businesscode code,
    const std::function<WMError(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply)>& func)
{
    auto consumer = [func](SubSystemId id, uint32_t customId, AAFwk::Want&& data,
                                     std::optional<AAFwk::Want>& reply) {
        return static_cast<int32_t>(func(std::move(data), reply));
    };
    dataConsumers_.emplace(static_cast<uint32_t>(code), std::move(consumer));
}

void WindowExtensionSessionImpl::RegisterDataConsumer()
{
    auto windowModeConsumer = [this](SubSystemId id, uint32_t customId, AAFwk::Want&& data,
                                     std::optional<AAFwk::Want>& reply) -> int32_t {
        auto windowMode = data.GetIntParam(Extension::WINDOW_MODE_FIELD, 0);
        if (windowMode < static_cast<int32_t>(WindowMode::WINDOW_MODE_UNDEFINED) ||
            windowMode > static_cast<int32_t>(WindowMode::END)) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "invalid window mode, windowMode:%{public}d", windowMode);
            return static_cast<int32_t>(DataHandlerErr::INVALID_PARAMETER);
        }

        static_cast<void>(SetWindowMode(static_cast<WindowMode>(windowMode)));
        return static_cast<int32_t>(DataHandlerErr::OK);
    };
    dataConsumers_.emplace(static_cast<uint32_t>(Extension::Businesscode::SYNC_HOST_WINDOW_MODE),
                           std::move(windowModeConsumer));
    RegisterConsumer(Extension::Businesscode::SYNC_CROSS_AXIS_STATE,
        std::bind(&WindowExtensionSessionImpl::OnCrossAxisStateChange,
        this, std::placeholders::_1, std::placeholders::_2));
    RegisterConsumer(Extension::Businesscode::SYNC_HOST_WATERFALL_MODE,
        std::bind(&WindowExtensionSessionImpl::OnWaterfallModeChange,
        this, std::placeholders::_1, std::placeholders::_2));
    RegisterConsumer(Extension::Businesscode::SYNC_WANT_PARAMS,
        std::bind(&WindowExtensionSessionImpl::OnResyncExtensionConfig,
        this, std::placeholders::_1, std::placeholders::_2));
    RegisterConsumer(Extension::Businesscode::SYNC_HOST_GESTURE_BACK_ENABLED,
        std::bind(&WindowExtensionSessionImpl::OnGestureBackEnabledChange,
        this, std::placeholders::_1, std::placeholders::_2));
    RegisterConsumer(Extension::Businesscode::SYNC_COMPAT_INFO,
        std::bind(&WindowExtensionSessionImpl::OnHostWindowCompatInfoChange,
        this, std::placeholders::_1, std::placeholders::_2));
    RegisterConsumer(Extension::Businesscode::SYNC_HOST_WINDOW_DELAY_RAISE_STATE,
        std::bind(&WindowExtensionSessionImpl::OnHostWindowDelayRaiseStateChange,
        this, std::placeholders::_1, std::placeholders::_2));
    RegisterConsumer(Extension::Businesscode::NOTIFY_HOST_WINDOW_RECT_CHANGE,
        std::bind(&WindowExtensionSessionImpl::OnHostWindowRectChange,
        this, std::placeholders::_1, std::placeholders::_2));
    RegisterConsumer(Extension::Businesscode::NOTIFY_SCREENSHOT,
        std::bind(&WindowExtensionSessionImpl::OnScreenshot,
        this, std::placeholders::_1, std::placeholders::_2));
    RegisterConsumer(Extension::Businesscode::NOTIFY_EXTENSION_SECURE_LIMIT_CHANGE,
        std::bind(&WindowExtensionSessionImpl::OnExtensionSecureLimitChange,
        this, std::placeholders::_1, std::placeholders::_2));
    RegisterConsumer(Extension::Businesscode::NOTIFY_KEYBOARD_DID_SHOW,
        std::bind(&WindowExtensionSessionImpl::OnKeyboardDidShow,
        this, std::placeholders::_1, std::placeholders::_2));
    RegisterConsumer(Extension::Businesscode::NOTIFY_KEYBOARD_DID_HIDE,
        std::bind(&WindowExtensionSessionImpl::OnKeyboardDidHide,
            this, std::placeholders::_1, std::placeholders::_2));
    RegisterConsumer(Extension::Businesscode::SYNC_HOST_STATUS_BAR_CONTENT_COLOR,
        std::bind(&WindowExtensionSessionImpl::OnHostStatusBarContentColorChange,
        this, std::placeholders::_1, std::placeholders::_2));
    RegisterConsumer(Extension::Businesscode::NOTIFY_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY,
        std::bind(&WindowExtensionSessionImpl::OnHostRectChangeInGlobalDisplay,
        this, std::placeholders::_1, std::placeholders::_2));

    auto consumersEntry = [weakThis = wptr(this)](SubSystemId id, uint32_t customId, AAFwk::Want&& data,
                                                  std::optional<AAFwk::Want>& reply) -> int32_t {
        auto window = weakThis.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "window is nullptr");
            return static_cast<int32_t>(DataHandlerErr::NULL_PTR);
        }
        auto itr = window->dataConsumers_.find(customId);
        if (itr == window->dataConsumers_.end()) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "no consumer for %{public}u", customId);
            return static_cast<int32_t>(DataHandlerErr::NO_CONSUME_CALLBACK);
        }

        const auto& func = itr->second;
        if (!func) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "not callable for %{public}u", customId);
            return static_cast<int32_t>(DataHandlerErr::INVALID_CALLBACK);
        }

        auto ret = func(id, customId, std::move(data), reply);
        TLOGND(WmsLogTag::WMS_UIEXT, "customId:%{public}u, ret:%{public}d", customId, ret);
        return static_cast<int32_t>(DataHandlerErr::OK);
    };
    dataHandler_->RegisterDataConsumer(SubSystemId::WM_UIEXT, std::move(consumersEntry));
}

WMError WindowExtensionSessionImpl::UseImplicitAnimation(bool useImplicit)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "WindowId: %{public}u", GetWindowId());
    SingletonContainer::Get<WindowAdapter>().UseImplicitAnimation(property_->GetParentId(), useImplicit);
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::SetCompatInfo(const AAFwk::WantParams& configParam)
{
    bool isAdaptToSimulationScale =
        static_cast<bool>(configParam.GetIntParam(Extension::COMPAT_IS_SIMULATION_SCALE_FIELD, 0));
    bool isAdaptToProportionalScale =
        static_cast<bool>(configParam.GetIntParam(Extension::COMPAT_IS_PROPORTION_SCALE_FIELD, 0));
    auto compatibleModeProperty = property_->GetCompatibleModeProperty();
    if (compatibleModeProperty == nullptr) {
        if (!isAdaptToSimulationScale && !isAdaptToProportionalScale) {
            TLOGD(WmsLogTag::WMS_COMPAT, "id:%{public}d not scale mode", GetPersistentId());
            return WMError::WM_DO_NOTHING;
        }
        compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
        property_->SetCompatibleModeProperty(compatibleModeProperty);
    }
    compatibleModeProperty->SetIsAdaptToSimulationScale(isAdaptToSimulationScale);
    compatibleModeProperty->SetIsAdaptToProportionalScale(isAdaptToProportionalScale);
    compatScaleX_ = GetFloatParam(Extension::COMPAT_SCALE_X_FIELD, configParam);
    compatScaleY_ = GetFloatParam(Extension::COMPAT_SCALE_Y_FIELD, configParam);
    TLOGI(WmsLogTag::WMS_COMPAT, "id:%{public}d compatScaleX:%{public}f compatScaleY:%{public}f",
        GetPersistentId(), compatScaleX_, compatScaleY_);
    return WMError::WM_OK;
}

WMError WindowExtensionSessionImpl::OnHostWindowCompatInfoChange(AAFwk::Want&& data,
    std::optional<AAFwk::Want>& reply)
{
    bool isAdaptToSimulationScale = data.GetBoolParam(Extension::COMPAT_IS_SIMULATION_SCALE_FIELD, false);
    bool isAdaptToProportionalScale = data.GetBoolParam(Extension::COMPAT_IS_PROPORTION_SCALE_FIELD, false);
    auto compatibleModeProperty = property_->GetCompatibleModeProperty();
    if (compatibleModeProperty == nullptr) {
        if (!isAdaptToSimulationScale && !isAdaptToProportionalScale) {
            TLOGD(WmsLogTag::WMS_COMPAT, "id:%{public}d not scale mode", GetPersistentId());
            return WMError::WM_DO_NOTHING;
        }
        compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
        property_->SetCompatibleModeProperty(compatibleModeProperty);
    }
    compatibleModeProperty->SetIsAdaptToSimulationScale(isAdaptToSimulationScale);
    compatibleModeProperty->SetIsAdaptToProportionalScale(isAdaptToProportionalScale);
    compatScaleX_ = data.GetFloatParam(Extension::COMPAT_SCALE_X_FIELD, 1.0f);
    compatScaleY_ = data.GetFloatParam(Extension::COMPAT_SCALE_Y_FIELD, 1.0f);
    TLOGI(WmsLogTag::WMS_COMPAT, "id:%{public}d compatScaleX:%{public}f compatScaleY:%{public}f",
        GetPersistentId(), compatScaleX_, compatScaleY_);
    if (auto uiContent = GetUIContentSharedPtr()) {
        TLOGD(WmsLogTag::WMS_COMPAT, "send uiext winId: %{public}u", GetWindowId());
        uiContent->SendUIExtProprty(static_cast<uint32_t>(Extension::Businesscode::SYNC_COMPAT_INFO),
            data, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
    return WMError::WM_OK;
}

void WindowExtensionSessionImpl::NotifyKeyboardDidShow(const KeyboardPanelInfo& keyboardPanelInfo)
{
    std::lock_guard<std::mutex> lockListener(keyboardDidShowListenerMutex_);
    for (const auto& listener : keyboardDidShowListenerList_) {
        if (listener != nullptr) {
            listener->OnKeyboardDidShow(keyboardPanelInfo);
        }
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        AAFwk::Want want;
        WriteKeyboardInfoToWant(want, keyboardPanelInfo);
        uiContent->SendUIExtProprtyByPersistentId(
            static_cast<uint32_t>(Extension::Businesscode::NOTIFY_KEYBOARD_DID_SHOW), want,
            keyboardDidShowUIExtListenerIds_, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
}

void WindowExtensionSessionImpl::NotifyKeyboardDidHide(const KeyboardPanelInfo& keyboardPanelInfo)
{
    std::lock_guard<std::mutex> lockListener(keyboardDidHideListenerMutex_);
    for (const auto& listener : keyboardDidHideListenerList_) {
        if (listener != nullptr) {
            listener->OnKeyboardDidHide(keyboardPanelInfo);
        }
    }
    if (auto uiContent = GetUIContentSharedPtr()) {
        AAFwk::Want want;
        WriteKeyboardInfoToWant(want, keyboardPanelInfo);
        uiContent->SendUIExtProprtyByPersistentId(
            static_cast<uint32_t>(Extension::Businesscode::NOTIFY_KEYBOARD_DID_HIDE), want,
            keyboardDidHideUIExtListenerIds_, static_cast<uint8_t>(SubSystemId::WM_UIEXT));
    }
}

WMError WindowExtensionSessionImpl::OnHostStatusBarContentColorChange(AAFwk::Want&& data,
    std::optional<AAFwk::Want>& reply)
{
    auto contentColor = static_cast<uint32_t>(data.GetIntParam(Extension::HOST_STATUS_BAR_CONTENT_COLOR, 0));
    TLOGI(WmsLogTag::WMS_UIEXT, "contentColor: %{public}u", contentColor);
    hostStatusBarContentColor_ = contentColor;
    return WMError::WM_OK;
}

uint32_t WindowExtensionSessionImpl::GetHostStatusBarContentColor() const
{
    return hostStatusBarContentColor_;
}

void WindowExtensionSessionImpl::NotifyOccupiedAreaChange(sptr<OccupiedAreaChangeInfo> info)
{
    std::lock_guard<std::mutex> lockListener(occupiedAreaChangeListenerMutex_);
    for (auto& listener : occupiedAreaChangeListenerList_) {
        if (listener != nullptr) {
            listener->OnSizeChange(info);
        }
    }
}
} // namespace Rosen
} // namespace OHOS
