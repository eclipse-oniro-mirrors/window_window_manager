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

#include "session/host/include/extension_session.h"

#include "ipc_skeleton.h"

#include "ui_extension/host_data_handler.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "ExtensionSession" };
std::unordered_set<int32_t> g_extensionPersistentIdSet;
std::mutex g_extensionPersistentIdMutex;
constexpr uint32_t EXTENSION_ID_FLAG = 0x40000000;
constexpr uint32_t PID_LENGTH = 18;
constexpr uint32_t PID_MASK = (1 << PID_LENGTH) - 1;
constexpr uint32_t PERSISTENTID_LENGTH = 12;
constexpr uint32_t PERSISTENTID_MASK = (1 << PERSISTENTID_LENGTH) - 1;

void TryUpdateExtensionPersistentId(int32_t& persistentId)
{
    std::lock_guard lock(g_extensionPersistentIdMutex);
    if (g_extensionPersistentIdSet.count(persistentId) == 0) {
        g_extensionPersistentIdSet.insert(persistentId);
        return;
    }
    uint32_t assembledPersistentId = (static_cast<uint32_t>(getpid()) & PID_MASK) << PERSISTENTID_LENGTH;
    uint32_t persistentIdValue = assembledPersistentId | EXTENSION_ID_FLAG;
    int32_t min = static_cast<int32_t>(persistentIdValue);
    int32_t max = static_cast<int32_t>(persistentIdValue | PERSISTENTID_MASK);
    uint32_t count = 0;
    while (g_extensionPersistentIdSet.count(persistentId)) {
        persistentId++;
        if (persistentId > max) {
            persistentId = min;
        }
        count++;
        if (count > PERSISTENTID_MASK) {
            persistentId = INVALID_SESSION_ID;
            TLOGE(WmsLogTag::WMS_UIEXT, "can't generate Id");
            return;
        }
    }
    g_extensionPersistentIdSet.insert(persistentId);
}

void RemoveExtensionPersistentId(int32_t persistentId)
{
    std::lock_guard lock(g_extensionPersistentIdMutex);
    g_extensionPersistentIdSet.erase(persistentId);
}
} // namespace

bool IsExtensionSessionInvalid(int32_t persistentId)
{
    std::lock_guard lock(g_extensionPersistentIdMutex);
    return g_extensionPersistentIdSet.count(persistentId) == 0;
}

void WindowEventChannelListener::SetTransferKeyEventForConsumedParams(int32_t keyEventId, bool isPreImeEvent,
    const std::shared_ptr<std::promise<bool>>& isConsumedPromise, const std::shared_ptr<WSError>& retCode)
{
    std::lock_guard<std::mutex> lock(transferKeyEventForConsumedMutex_);
    keyEventId_ = keyEventId;
    isPreImeEvent_ = isPreImeEvent;
    retCode_ = retCode;
    isConsumedPromise_ = isConsumedPromise;
}

void WindowEventChannelListener::ResetTransferKeyEventForConsumedParams()
{
    std::lock_guard<std::mutex> lock(transferKeyEventForConsumedMutex_);
    retCode_ = nullptr;
    isConsumedPromise_ = nullptr;
}

void WindowEventChannelListener::ResetTransferKeyEventForConsumedParams(bool isConsumed, WSError retCode)
{
    std::lock_guard<std::mutex> lock(transferKeyEventForConsumedMutex_);
    if (retCode_ != nullptr) {
        *retCode_ = retCode;
        retCode_ = nullptr;
    }
    if (isConsumedPromise_ != nullptr) {
        isConsumedPromise_->set_value(isConsumed);
        isConsumedPromise_ = nullptr;
    }
}

void WindowEventChannelListener::OnTransferKeyEventForConsumed(int32_t keyEventId, bool isPreImeEvent, bool isConsumed,
    WSError retCode)
{
    std::lock_guard<std::mutex> lock(transferKeyEventForConsumedMutex_);
    if (keyEventId_ != keyEventId || isPreImeEvent_ != isPreImeEvent) {
        TLOGW(WmsLogTag::WMS_EVENT, "The event has been processed at PreIme:%{public}d id:%{public}d.",
            isPreImeEvent, keyEventId);
        return;
    }
    if (isConsumedPromise_ == nullptr || retCode_ == nullptr) {
        TLOGW(WmsLogTag::WMS_EVENT, "Promise or ret is null at PreIme:%{public}d id:%{public}d.",
            isPreImeEvent, keyEventId);
        return;
    }

    *retCode_ = retCode;
    retCode_ = nullptr;
    isConsumedPromise_->set_value(isConsumed);
    isConsumedPromise_ = nullptr;
}

int32_t WindowEventChannelListener::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TLOGE(WmsLogTag::WMS_EVENT, "InterfaceToken check failed");
        return ERR_TRANSACTION_FAILED;
    }

    auto msgId = static_cast<WindowEventChannelListenerMessage>(code);
    switch (msgId) {
        case WindowEventChannelListenerMessage::TRANS_ID_ON_TRANSFER_KEY_EVENT_FOR_CONSUMED_ASYNC: {
            int32_t keyEventId = 0;
            bool isPreImeEvent = false;
            bool isConsumed = false;
            int32_t intRetCode = 0;
            if (!data.ReadInt32(keyEventId) || !data.ReadBool(isPreImeEvent) || !data.ReadBool(isConsumed) ||
                !data.ReadInt32(intRetCode)) {
                TLOGE(WmsLogTag::WMS_EVENT, "Read keyEvent info failed");
                return ERR_TRANSACTION_FAILED;
            }
            WSError retCode = static_cast<WSError>(intRetCode);
            OnTransferKeyEventForConsumed(keyEventId, isPreImeEvent, isConsumed, retCode);
            break;
        }
        default:
            TLOGE(WmsLogTag::WMS_EVENT, "unknown transaction code %{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_NONE;
}

void ChannelDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    if (wptrDeath == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "wptrDeath is null");
        return;
    }

    sptr<IRemoteObject> object = wptrDeath.promote();
    if (!object) {
        TLOGE(WmsLogTag::WMS_UIEXT, "object is null");
        return;
    }

    if (listener_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "listener_ is null");
        return;
    }
    TLOGD(WmsLogTag::WMS_UIEXT, "Died");
    listener_->ResetTransferKeyEventForConsumedParams(false, WSError::WS_ERROR_IPC_FAILED);
}

ExtensionSession::ExtensionSession(const SessionInfo& info) : Session(info)
{
    GeneratePersistentId(true, info.persistentId_);
    TryUpdateExtensionPersistentId(persistentId_);
    dataHandler_ = std::make_shared<Extension::HostDataHandler>();
    TLOGD(WmsLogTag::WMS_UIEXT, "Create, bundle:%{public}s, module:%{public}s, ability:%{public}s, id:%{public}d.",
        info.bundleName_.c_str(), info.moduleName_.c_str(), info.abilityName_.c_str(), persistentId_);
}

ExtensionSession::~ExtensionSession()
{
    TLOGI(WmsLogTag::WMS_UIEXT, "id=%{public}d", persistentId_);
    RemoveExtensionPersistentId(persistentId_);
    if (windowEventChannel_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "window event channel is null");
        return;
    }
    sptr<IRemoteObject> remoteObject = windowEventChannel_->AsObject();
    if (remoteObject == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remoteObject is null");
        return;
    }
    remoteObject->RemoveDeathRecipient(channelDeath_);
    channelListener_ = nullptr;
    channelDeath_ = nullptr;
}

std::shared_ptr<IDataHandler> ExtensionSession::GetExtensionDataHandler() const
{
    return dataHandler_;
}

void ExtensionSession::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler>& handler,
    const std::shared_ptr<AppExecFwk::EventHandler>& exportHandler)
{
    Session::SetEventHandler(handler, exportHandler);
    dataHandler_->SetEventHandler(handler);
}

WSError ExtensionSession::ConnectInner(
    const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
    sptr<WindowSessionProperty> property, sptr<IRemoteObject> token, int32_t pid, int32_t uid,
    const std::string& identityToken)
{
    if (pid == INVALID_PID || uid == INVALID_UID) {
        TLOGE(WmsLogTag::WMS_UIEXT, "invalid pid or uid");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    auto task = [weakThis = wptr(this), sessionStage, eventChannel, surfaceNode,
        &systemConfig, property, token, pid, uid]() NO_THREAD_SAFETY_ANALYSIS {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }

        if (eventChannel != nullptr) {
            sptr<IRemoteObject> remoteObject = eventChannel->AsObject();
            if (remoteObject == nullptr) {
                TLOGNE(WmsLogTag::WMS_UIEXT, "remoteObject is null");
                return WSError::WS_ERROR_DESTROYED_OBJECT;
            }

            session->channelListener_ = sptr<WindowEventChannelListener>::MakeSptr();
            session->channelDeath_ = sptr<ChannelDeathRecipient>::MakeSptr(session->channelListener_);
            if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(session->channelDeath_)) {
                TLOGNE(WmsLogTag::WMS_UIEXT, "Failed to add death recipient");
                return WSError::WS_ERROR_INTERNAL_ERROR;
            }
        }

        session->dataHandler_->SetRemoteProxyObject(sessionStage->AsObject());
        if (session->IsTransparentUIExtension()) {
            sessionStage->SetUIExtensionTransparent();
        }
        return session->Session::ConnectInner(
            sessionStage, eventChannel, surfaceNode, systemConfig, property, token, pid, uid);
    };
    return PostSyncTask(task, "ConnectInner");
}

WSError ExtensionSession::Connect(
    const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
    sptr<WindowSessionProperty> property, sptr<IRemoteObject> token,
    const std::string& identityToken)
{
    // Get pid and uid before posting task.
    int32_t pid = IPCSkeleton::GetCallingRealPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    return ConnectInner(sessionStage, eventChannel, surfaceNode, systemConfig,
        property, token, pid, uid, identityToken);
}

WSError ExtensionSession::TransferAbilityResult(uint32_t resultCode, const AAFwk::Want& want)
{
    if (extSessionEventCallback_ != nullptr &&
        extSessionEventCallback_->transferAbilityResultFunc_ != nullptr) {
        extSessionEventCallback_->transferAbilityResultFunc_(resultCode, want);
    }
    return WSError::WS_OK;
}

int32_t ExtensionSession::TransferExtensionData(const AAFwk::WantParams& wantParams)
{
    TLOGNI(WmsLogTag::WMS_UIEXT, "uec recv");
    if (extSessionEventCallback_ != nullptr &&
        extSessionEventCallback_->transferExtensionDataFunc_ != nullptr) {
        extSessionEventCallback_->transferExtensionDataFunc_(wantParams);
    }
    return ERR_NONE;
}

WSError ExtensionSession::TransferComponentData(const AAFwk::WantParams& wantParams)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "id: %{public}d", GetPersistentId());
    if (!IsSessionValid()) {
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    sessionStage_->NotifyTransferComponentData(wantParams);
    return WSError::WS_OK;
}

WSErrorCode ExtensionSession::TransferComponentDataSync(const AAFwk::WantParams& wantParams,
                                                        AAFwk::WantParams& reWantParams)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "id: %{public}d", GetPersistentId());
    if (!IsSessionValid()) {
        return WSErrorCode::WS_ERROR_TRANSFER_DATA_FAILED;
    }
    return sessionStage_->NotifyTransferComponentDataSync(wantParams, reWantParams);
}

void ExtensionSession::NotifySyncOn()
{
    if (extSessionEventCallback_ != nullptr &&
        extSessionEventCallback_->notifySyncOnFunc_ != nullptr) {
        extSessionEventCallback_->notifySyncOnFunc_();
    }
}

void ExtensionSession::NotifyAsyncOn()
{
    if (extSessionEventCallback_ != nullptr &&
        extSessionEventCallback_->notifyAsyncOnFunc_ != nullptr) {
        extSessionEventCallback_->notifyAsyncOnFunc_();
    }
}

WSError ExtensionSession::NotifyDensityFollowHost(bool isFollowHost, float densityValue)
{
    if (!IsSessionValid()) {
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "session stage is null!");
        return WSError::WS_ERROR_NULLPTR;
    }

    return sessionStage_->NotifyDensityFollowHost(isFollowHost, densityValue);
}

WSError ExtensionSession::UpdateSessionViewportConfig(const SessionViewportConfig& config)
{
    if (!IsSessionValid()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "session is invalid");
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (sessionStage_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "sessionStage_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "winId: %{public}d, isDensityFollowHost_:%{public}d, "
        "displayId:%{public}" PRIu64", density:%{public}f, orientation:%{public}d.",
        GetPersistentId(), config.isDensityFollowHost_, config.displayId_, config.density_, config.orientation_);
    return sessionStage_->UpdateSessionViewportConfig(config);
}

void ExtensionSession::TriggerBindModalUIExtension()
{
    if (isFirstTriggerBindModal_ && extSessionEventCallback_ != nullptr &&
        extSessionEventCallback_->notifyBindModalFunc_ != nullptr) {
        WLOGFD("Start calling bind modal func.");
        extSessionEventCallback_->notifyBindModalFunc_();
        isFirstTriggerBindModal_ = false;
    }
}

void ExtensionSession::RegisterExtensionSessionEventCallback(
    const sptr<ExtensionSessionEventCallback>& extSessionEventCallback)
{
    extSessionEventCallback_ = extSessionEventCallback;
}

WSError ExtensionSession::TransferKeyEventForConsumed(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed,
    bool& isTimeout, bool isPreImeEvent)
{
    if (windowEventChannel_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "windowEventChannel_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (keyEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "KeyEvent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (channelListener_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "Created channelListener_ is nullptr.");
        return WSError::WS_ERROR_NULLPTR;
    }
    int32_t keyEventId = keyEvent->GetId();
    TLOGI(WmsLogTag::WMS_EVENT, "In with isPreImeEvent:%{public}d, id:%{public}d", isPreImeEvent, keyEventId);

    auto isConsumedPromise = std::make_shared<std::promise<bool>>();
    std::shared_ptr<WSError> retCode = std::make_shared<WSError>(WSError::WS_OK);
    channelListener_->SetTransferKeyEventForConsumedParams(keyEventId, isPreImeEvent, isConsumedPromise, retCode);
    auto ret = windowEventChannel_->TransferKeyEventForConsumedAsync(keyEvent, isPreImeEvent, channelListener_);
    // if UiExtension was died, return transferKeyEvent before wait for timeout.
    if (ret != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_EVENT, "transfer keyEvent failed with %{public}d at PreIme:%{public}d id:%{public}d.",
            ret, isPreImeEvent, keyEventId);
        return ret;
    }

    // Timeout cannot exceed APP_INPUT_BLOCK
    constexpr int64_t TRANSFER_KEY_EVENT_TIMEOUT_TIME_MS = 4000;
    auto isConsumedFuture = isConsumedPromise->get_future().share();
    if (isConsumedFuture.wait_for(std::chrono::milliseconds(TRANSFER_KEY_EVENT_TIMEOUT_TIME_MS)) ==
            std::future_status::timeout) {
        // Prevents the pointer from being used by a remote ipc after its lifetime has ended.
        channelListener_->ResetTransferKeyEventForConsumedParams();
        isTimeout = true;
    } else {
        // Prevents the pointer from being used by a death recipient after its lifetime has ended.
        channelListener_->ResetTransferKeyEventForConsumedParams();
        isTimeout = false;
        isConsumed = isConsumedFuture.get();
        ret = *retCode;
    }
    TLOGI(WmsLogTag::WMS_EVENT, "isConsumed:%{public}d Timeout:%{public}d ret:%{public}d at PreIme:%{public}d "
        "id:%{public}d.", isConsumed, isTimeout, ret, isPreImeEvent, keyEventId);
    return ret;
}

WSError ExtensionSession::TransferKeyEventAsync(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool isPreImeEvent)
{
    if (windowEventChannel_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "windowEventChannel_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (keyEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "KeyEvent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }

    TLOGI(WmsLogTag::WMS_EVENT, "In with isPreImeEvent(%{public}d), id:%{public}d", isPreImeEvent, keyEvent->GetId());
    channelListener_->ResetTransferKeyEventForConsumedParams();
    auto ret = windowEventChannel_->TransferKeyEventForConsumedAsync(keyEvent, isPreImeEvent, channelListener_);
    TLOGI(WmsLogTag::WMS_EVENT, "ret is %{public}d in id:%{public}d.", ret, keyEvent->GetId());
    return ret;
}

sptr<ExtensionSession::ExtensionSessionEventCallback> ExtensionSession::GetExtensionSessionEventCallback()
{
    if (extSessionEventCallback_ == nullptr) {
        extSessionEventCallback_ = sptr<ExtensionSessionEventCallback>::MakeSptr();
    }
    return extSessionEventCallback_;
}

WSError ExtensionSession::TransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
    int64_t uiExtensionIdLevel)
{
    NotifyTransferAccessibilityEvent(info, uiExtensionIdLevel);
    return WSError::WS_OK;
}

WSError ExtensionSession::TransferAccessibilityHoverEvent(
    float pointX, float pointY, int32_t sourceType, int32_t eventType, int64_t timeMs)
{
    if (!windowEventChannel_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowEventChannel_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return windowEventChannel_->TransferAccessibilityHoverEvent(pointX, pointY, sourceType, eventType, timeMs);
}

WSError ExtensionSession::TransferAccessibilityChildTreeRegister(
    uint32_t windowId, int32_t treeId, int64_t accessibilityId)
{
    if (!windowEventChannel_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowEventChannel_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return windowEventChannel_->TransferAccessibilityChildTreeRegister(windowId, treeId, accessibilityId);
}

WSError ExtensionSession::TransferAccessibilityChildTreeUnregister()
{
    if (!windowEventChannel_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowEventChannel_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return windowEventChannel_->TransferAccessibilityChildTreeUnregister();
}

WSError ExtensionSession::TransferAccessibilityDumpChildInfo(
    const std::vector<std::string>& params, std::vector<std::string>& info)
{
    if (!windowEventChannel_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowEventChannel_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return windowEventChannel_->TransferAccessibilityDumpChildInfo(params, info);
}

WSError ExtensionSession::UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    if (!IsSessionValid()) {
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    return sessionStage_->UpdateAvoidArea(avoidArea, type);
}

AvoidArea ExtensionSession::GetAvoidAreaByType(AvoidAreaType type, const WSRect& rect, int32_t apiVersion)
{
    Rosen::AvoidArea avoidArea;
    if (extSessionEventCallback_ != nullptr && extSessionEventCallback_->notifyGetAvoidAreaByTypeFunc_ != nullptr) {
        avoidArea = extSessionEventCallback_->notifyGetAvoidAreaByTypeFunc_(type, apiVersion);
    }
    return avoidArea;
}

WSError ExtensionSession::Background(bool isFromClient, const std::string& identityToken)
{
    SessionState state = GetSessionState();
    TLOGI(WmsLogTag::WMS_LIFE, "Background ExtensionSession, id: %{public}d, state: %{public}" PRIu32"",
        GetPersistentId(), static_cast<uint32_t>(state));
    if (state == SessionState::STATE_ACTIVE && GetWindowType() == WindowType::WINDOW_TYPE_UI_EXTENSION) {
        UpdateSessionState(SessionState::STATE_INACTIVE);
        state = SessionState::STATE_INACTIVE;
        isActive_ = false;
    }
    if (state == SessionState::STATE_BACKGROUND) {
        TLOGI(WmsLogTag::WMS_LIFE, "ExtensionSession %{public}d state already backgrounded.",  GetPersistentId());
        return WSError::WS_OK;
    }
    if (state != SessionState::STATE_INACTIVE) {
        WLOGFW("[WMSLife] Background state invalid! state:%{public}u", state);
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    UpdateSessionState(SessionState::STATE_BACKGROUND);
    NotifyBackground();
    return WSError::WS_OK;
}

void ExtensionSession::NotifyExtensionEventAsync(uint32_t notifyEvent)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "notifyEvent: %{public}d", notifyEvent);
    if (extSessionEventCallback_ != nullptr && extSessionEventCallback_->notifyExtensionEventFunc_ != nullptr) {
        extSessionEventCallback_->notifyExtensionEventFunc_(notifyEvent);
    }
}

WSError ExtensionSession::NotifyDumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "persistenId: %{public}d", GetPersistentId());
    if (!IsSessionValid()) {
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "session stage is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->NotifyDumpInfo(params, info);
}

int32_t ExtensionSession::GetStatusBarHeight()
{
    TLOGI(WmsLogTag::WMS_UIEXT, "in");
    if (extSessionEventCallback_ != nullptr && extSessionEventCallback_->getStatusBarHeightFunc_ != nullptr) {
        return extSessionEventCallback_->getStatusBarHeightFunc_();
    }
    return 0;
}

WSError ExtensionSession::SendExtensionData(MessageParcel& data, MessageParcel& reply,
                                            [[maybe_unused]] MessageOption& option)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "id=%{public}d", GetPersistentId());
    dataHandler_->NotifyDataConsumer(data, reply);
    return WSError::WS_OK;
}
} // namespace OHOS::Rosen
