/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "window_adapter.h"
#include <iservice_registry.h>
#include <key_event.h>
#include <rs_window_animation_target.h>
#include <system_ability_definition.h>
#include <unistd.h>
#include "focus_change_info.h"
#include "scene_board_judgement.h"
#include "session_manager.h"
#include "window_manager.h"
#include "window_manager_hilog.h"
#include "window_manager_proxy.h"
#include "window_session_impl.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowAdapter"};
}
std::unordered_map<int32_t, sptr<WindowAdapter>> WindowAdapter::windowAdapterMap_ = {};
std::mutex WindowAdapter::windowAdapterMapMutex_;

#define INIT_PROXY_CHECK_RETURN(ret)                             \
    do {                                                         \
        if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) { \
            if (!InitSSMProxy()) {                               \
                WLOGFE("InitSSMProxy failed!");                  \
                return ret;                                      \
            }                                                    \
        } else {                                                 \
            if (!InitWMSProxy()) {                               \
                WLOGFE("InitWMSProxy failed!");                  \
                return ret;                                      \
            }                                                    \
        }                                                        \
    } while (false)

#define CHECK_PROXY_RETURN_ERROR_IF_NULL(proxy, ret)                      \
    do {                                                                  \
        if ((proxy) == nullptr) {                                         \
            TLOGE(WmsLogTag::DEFAULT, "window manager proxy is nullptr"); \
            return ret;                                                   \
        }                                                                 \
    } while (false)

#define CHECK_PROXY_RETURN_IF_NULL(proxy)                                 \
    do {                                                                  \
        if ((proxy) == nullptr) {                                         \
            TLOGE(WmsLogTag::DEFAULT, "window manager proxy is nullptr"); \
            return;                                                       \
        }                                                                 \
    } while (false)

WMSDeathRecipient::WMSDeathRecipient(const int32_t userId) : userId_(userId) {}

void WMSDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "wms died");
    sptr<IRemoteObject> object = wptrDeath.promote();
    if (object == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "invalid user: %d", userId_);
        return;
    }
    WindowAdapter::GetInstance(userId_).ClearWindowAdapter();
}

WindowAdapter::WindowAdapter(const int32_t userId) : userId_(userId) {}

WindowAdapter& WindowAdapter::GetInstance()
{
    static auto instance = sptr<WindowAdapter>::MakeSptr();
    return *instance;
}

WindowAdapter& WindowAdapter::GetInstance(const int32_t userId)
{
    if (userId <= INVALID_USER_ID) {
        return GetInstance();
    }
    // multi-instance mode
    std::lock_guard<std::mutex> lock(windowAdapterMapMutex_);
    auto iter = windowAdapterMap_.find(userId);
    if (iter != windowAdapterMap_.end() && iter->second) {
        return *iter->second;
    }
    auto instance = sptr<WindowAdapter>::MakeSptr(userId);
    windowAdapterMap_.insert({ userId, instance });
    TLOGI(WmsLogTag::WMS_MULTI_USER, "get new instance, userId: %{public}d", userId);
    return *windowAdapterMap_[userId];
}

WindowAdapter::~WindowAdapter()
{
    sptr<IRemoteObject> remoteObject = nullptr;
    if (windowManagerServiceProxy_) {
        remoteObject = windowManagerServiceProxy_->AsObject();
    }
    if (remoteObject) {
        remoteObject->RemoveDeathRecipient(wmsDeath_);
    }
    TLOGI(WmsLogTag::WMS_SCB, "destroyed, userId: %{public}d", userId_);
}

WMError WindowAdapter::CreateWindow(sptr<IWindow>& window, sptr<WindowProperty>& windowProperty,
    std::shared_ptr<RSSurfaceNode> surfaceNode, uint32_t& windowId, const sptr<IRemoteObject>& token)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->CreateWindow(window, windowProperty, surfaceNode, windowId, token);
}

WMError WindowAdapter::AddWindow(sptr<WindowProperty>& windowProperty)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->AddWindow(windowProperty);
}

WMError WindowAdapter::RemoveWindow(uint32_t windowId, bool isFromInnerkits)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->RemoveWindow(windowId, isFromInnerkits);
}

WMError WindowAdapter::DestroyWindow(uint32_t windowId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->DestroyWindow(windowId);
}

WMError WindowAdapter::SetParentWindow(int32_t subWindowId, int32_t newParentWindowId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetParentWindow(subWindowId, newParentWindowId);
}

WMError WindowAdapter::RequestFocus(uint32_t windowId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->RequestFocus(windowId);
}

WMError WindowAdapter::RegisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);

    auto ret = wmsProxy->RegisterWindowManagerAgent(type, windowManagerAgent);
    if (ret == WMError::WM_OK) {
        std::lock_guard<std::mutex> lock(wmAgentMapMutex_);
        windowManagerAgentMap_[type].insert(windowManagerAgent);
    } else {
        TLOGE(WmsLogTag::WMS_SCB, "failed due to proxy, type: %{public}d", type);
    }
    return ret;
}

void WindowAdapter::RegisterWindowManagerAgentWhenSCBFault(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    // Note: sceneboard is restarting due to a failure, the listener will be re-registered
    // during the "independent recovery" process.
    std::lock_guard<std::mutex> lock(wmAgentMapMutex_);
    windowManagerAgentFaultMap_[type].insert(windowManagerAgent);
}

WMError WindowAdapter::UnregisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    auto ret = wmsProxy->UnregisterWindowManagerAgent(type, windowManagerAgent);
    if (ret != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_SCB, "unregister failed due to proxy, ret: %{public}d", ret);
        return ret;
    }

    std::lock_guard<std::mutex> lock(wmAgentMapMutex_);
    if (windowManagerAgentMap_[type].erase(windowManagerAgent)) {
        TLOGI(WmsLogTag::WMS_SCB, "erase success, type: %{public}d", type);
    } else {
        TLOGW(WmsLogTag::WMS_SCB, "agent not found, type: %{public}d", type);
    }
    return ret;
}

WMError WindowAdapter::RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
    uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);

    auto type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY;
    // Flags only used to independent recovery.
    observedFlags_ |= static_cast<uint32_t>(windowInfoKey);
    interestedFlags_ |= interestInfo;

    auto ret = wmsProxy->RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    if (ret == WMError::WM_OK) {
        std::lock_guard<std::mutex> lock(wmAgentMapMutex_);
        windowManagerAgentMap_[type].insert(windowManagerAgent);
    } else {
        TLOGE(WmsLogTag::WMS_SCB, "failed due to proxy, type: %{public}d", type);
    }
    return ret;
}

WMError WindowAdapter::UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
    uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    auto ret = wmsProxy->UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);

    observedFlags_ &= ~(static_cast<uint32_t>(windowInfoKey));
    interestedFlags_ &= ~interestInfo;

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY;
    std::lock_guard<std::mutex> lock(wmAgentMapMutex_);
    if (windowManagerAgentMap_.find(type) == windowManagerAgentMap_.end()) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "WINDOW_MANAGER_AGENT_TYPE_PROPERTY not found");
        return ret;
    }

    auto& agentSet = windowManagerAgentMap_[type];
    auto agent = std::find(agentSet.begin(), agentSet.end(), windowManagerAgent);
    if (agent == agentSet.end()) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "Cannot find WINDOW_MANAGER_AGENT_TYPE_PROPERTY");
        return ret;
    }
    agentSet.erase(agent);

    return ret;
}

WMError WindowAdapter::CheckWindowId(int32_t windowId, int32_t& pid)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->CheckWindowId(windowId, pid);
}

WMError WindowAdapter::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetAccessibilityWindowInfo(infos);
}

WMError WindowAdapter::ConvertToRelativeCoordinateExtended(const Rect& rect, Rect& newRect, DisplayId& newDisplayId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return wmsProxy->ConvertToRelativeCoordinateExtended(rect, newRect, newDisplayId);
}

WMError WindowAdapter::GetUnreliableWindowInfo(int32_t windowId,
    std::vector<sptr<UnreliableWindowInfo>>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetUnreliableWindowInfo(windowId, infos);
}

WMError WindowAdapter::ListWindowInfo(const WindowInfoOption& windowInfoOption, std::vector<sptr<WindowInfo>>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->ListWindowInfo(windowInfoOption, infos);
}

WMError WindowAdapter::GetAllWindowLayoutInfo(DisplayId displayId, std::vector<sptr<WindowLayoutInfo>>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetAllWindowLayoutInfo(displayId, infos);
}

WMError WindowAdapter::GetAllMainWindowInfo(std::vector<sptr<MainWindowInfo>>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetAllMainWindowInfo(infos);
}
 
WMError WindowAdapter::GetMainWindowSnapshot(const std::vector<int32_t>& windowIds,
    const WindowSnapshotConfiguration& config, const sptr<IRemoteObject>& callback)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetMainWindowSnapshot(windowIds, config, callback);
}

WMError WindowAdapter::GetGlobalWindowMode(DisplayId displayId, GlobalWindowMode& globalWinMode)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetGlobalWindowMode(displayId, globalWinMode);
}

WMError WindowAdapter::GetTopNavDestinationName(int32_t windowId, std::string& topNavDestName)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetTopNavDestinationName(windowId, topNavDestName);
}

WMError WindowAdapter::SetWatermarkImageForApp(const std::shared_ptr<Media::PixelMap>& pixelMap)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    auto errCode = wmsProxy->SetWatermarkImageForApp(pixelMap, appWatermarkName_);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "watermarkName: %{public}s, errCode: %{public}d",
        appWatermarkName_.c_str(), static_cast<int32_t>(errCode));
    return errCode;
}

WMError WindowAdapter::RecoverWatermarkImageForApp()
{
    if (appWatermarkName_.empty()) {
        return WMError::WM_OK;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!windowManagerServiceProxy_ || !windowManagerServiceProxy_->AsObject()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "proxy is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto errCode = windowManagerServiceProxy_->RecoverWatermarkImageForApp(appWatermarkName_);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "watermarkName: %{public}s, errCode: %{public}d",
        appWatermarkName_.c_str(), static_cast<int32_t>(errCode));
    return errCode;
}

WMError WindowAdapter::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetVisibilityWindowInfo(infos);
}

WMError WindowAdapter::SetWindowAnimationController(const sptr<RSIWindowAnimationController>& controller)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetWindowAnimationController(controller);
}

WMError WindowAdapter::GetAvoidAreaByType(uint32_t windowId, AvoidAreaType type, AvoidArea& avoidArea, const Rect& rect)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    avoidArea = wmsProxy->GetAvoidAreaByType(windowId, type, rect);
    return WMError::WM_OK;
}

void WindowAdapter::NotifyServerReadyToMoveOrDrag(uint32_t windowId, sptr<WindowProperty>& windowProperty,
    sptr<MoveDragProperty>& moveDragProperty)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->NotifyServerReadyToMoveOrDrag(windowId, windowProperty, moveDragProperty);
}

void WindowAdapter::ProcessPointDown(uint32_t windowId, bool isPointDown)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->ProcessPointDown(windowId, isPointDown);
}

void WindowAdapter::ProcessPointUp(uint32_t windowId)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->ProcessPointUp(windowId);
}

WMError WindowAdapter::MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->MinimizeAllAppWindows(displayId, excludeWindowId);
}

WMError WindowAdapter::ToggleShownStateForAllAppWindows()
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->ToggleShownStateForAllAppWindows();
}

WMError WindowAdapter::GetSystemConfig(SystemConfig& systemConfig)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetSystemConfig(systemConfig);
}

WMError WindowAdapter::GetModeChangeHotZones(DisplayId displayId, ModeChangeHotZones& hotZones)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetModeChangeHotZones(displayId, hotZones);
}

bool WindowAdapter::InitWMSProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!isProxyValid_) {
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityManager) {
            WLOGFE("Failed to get system ability mgr.");
            return false;
        }

        sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(WINDOW_MANAGER_SERVICE_ID);
        if (!remoteObject) {
            WLOGFE("Failed to get window manager service.");
            return false;
        }

        windowManagerServiceProxy_ = iface_cast<IWindowManager>(remoteObject);
        if (!windowManagerServiceProxy_ || !windowManagerServiceProxy_->AsObject()) {
            WLOGFE("Failed to get system window manager services");
            return false;
        }

        wmsDeath_ = new WMSDeathRecipient();
        if (!wmsDeath_) {
            WLOGFE("Failed to create death Recipient ptr WMSDeathRecipient");
            return false;
        }
        if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(wmsDeath_)) {
            WLOGFE("Failed to add death recipient");
            return false;
        }
        isProxyValid_ = true;
    }
    return true;
}

void WindowAdapter::RegisterSessionRecoverCallbackFunc(
    int32_t persistentId, const SessionRecoverCallbackFunc& callbackFunc)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "persistentId=%{public}d", persistentId);
    std::lock_guard<std::mutex> lock(mutex_);
    sessionRecoverCallbackFuncMap_[persistentId] = callbackFunc;
}

void WindowAdapter::RegisterUIEffectRecoverCallbackFunc(int32_t id,
    const UIEffectRecoverCallbackFunc& callbackFunc)
{
    std::lock_guard<std::mutex> lock(effectMutex_);
    uiEffectRecoverCallbackFuncMap_[id] = callbackFunc;
}
void WindowAdapter::UnregisterUIEffectRecoverCallbackFunc(int32_t id)
{
    std::lock_guard<std::mutex> lock(effectMutex_);
    uiEffectRecoverCallbackFuncMap_.erase(id);
}

WMError WindowAdapter::GetSnapshotByWindowId(int32_t windowId, std::shared_ptr<Media::PixelMap>& pixelMap)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_IPC_FAILED);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_IPC_FAILED);
    return wmsProxy->GetSnapshotByWindowId(windowId, pixelMap);
}

void WindowAdapter::UnregisterSessionRecoverCallbackFunc(int32_t persistentId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessionRecoverCallbackFuncMap_.find(persistentId);
    if (it != sessionRecoverCallbackFuncMap_.end()) {
        sessionRecoverCallbackFuncMap_.erase(it);
    }
}

WMError WindowAdapter::RegisterWMSConnectionChangedListener(const WMSConnectionChangedCallbackFunc& callbackFunc)
{
    TLOGI(WmsLogTag::WMS_MULTI_USER, "RegisterWMSConnectionChangedListener in");
    return SessionManager::GetInstance(userId_).RegisterWMSConnectionChangedListener(callbackFunc);
}

WMError WindowAdapter::UnregisterWMSConnectionChangedListener()
{
    TLOGI(WmsLogTag::WMS_MULTI_USER, "unregister wms connection changed listener");
    return SessionManager::GetInstance(userId_).UnregisterWMSConnectionChangedListener();
}

void WindowAdapter::WindowManagerAndSessionRecover()
{
    ClearWindowAdapter();
    if (!InitSSMProxy()) {
        TLOGE(WmsLogTag::WMS_RECOVER, "InitSSMProxy failed");
        return;
    }

    ReregisterWindowManagerAgent();
    RecoverWindowPropertyChangeFlag();
    RecoverWatermarkImageForApp();
    RecoverSpecificZIndexSetByApp();

    std::map<int32_t, SessionRecoverCallbackFunc> sessionRecoverCallbackFuncMap;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sessionRecoverCallbackFuncMap = sessionRecoverCallbackFuncMap_;
    }
    for (const auto& it : sessionRecoverCallbackFuncMap) {
        TLOGD(WmsLogTag::WMS_RECOVER, "Session recover callback, persistentId=%{public}" PRId32, it.first);
        auto ret = it.second();
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_RECOVER, "Session recover callback, persistentId=%{public}" PRId32 " is error",
                it.first);
        }
    }
    std::map<int32_t, UIEffectRecoverCallbackFunc> uiEffectRecoverCallbackFuncMap;
    {
        std::lock_guard<std::mutex> lock(effectMutex_);
        uiEffectRecoverCallbackFuncMap = uiEffectRecoverCallbackFuncMap_;
    }
    for (const auto& it : uiEffectRecoverCallbackFuncMap) {
        TLOGD(WmsLogTag::WMS_RECOVER, "ui effect recover callback, id: %{public}d", it.first);
        auto ret = it.second();
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_RECOVER, "ui effect create failed, id: %{public}d, reason %{public}d", it.first, ret);
        }
    }

    OutlineRecoverCallbackFunc outlineRecoverCallbackFunc;
    {
        std::lock_guard<std::mutex> lock(outlineMutex_);
        outlineRecoverCallbackFunc = outlineRecoverCallbackFunc_;
    }
    if (outlineRecoverCallbackFunc) {
        auto ret = outlineRecoverCallbackFunc();
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "Recover outline failed, ret: %{public}d.", ret);
        }
    }
}

void WindowAdapter::RecoverSpecificZIndexSetByApp()
{
    if (specificZIndexMap_.empty()) {
        return;
    }
    for (const auto& elem : specificZIndexMap_) {
        SetSpecificWindowZIndex(elem.first, elem.second, false);
        TLOGI(WmsLogTag::WMS_FOCUS, "windowType: %{public}d, zIndex: %{public}d",
            elem.first, elem.second);
    }
}

WMError  WindowAdapter::RecoverWindowPropertyChangeFlag()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!windowManagerServiceProxy_ || !windowManagerServiceProxy_->AsObject()) {
        TLOGE(WmsLogTag::WMS_RECOVER, "proxy is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto ret = windowManagerServiceProxy_->RecoverWindowPropertyChangeFlag(observedFlags_, interestedFlags_);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_RECOVER, "failed, ret=%{public}d", static_cast<int32_t>(ret));
    }
    return ret;
}

void WindowAdapter::ReregisterWindowManagerAgent()
{
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);

    std::vector<std::pair<WindowManagerAgentType, sptr<IWindowManagerAgent>>> agentsToRegister;
    {
        std::lock_guard<std::mutex> lock(wmAgentMapMutex_);
        for (const auto& [type, agents] : windowManagerAgentMap_) {
            TLOGI(WmsLogTag::WMS_RECOVER, "agent type: %{public}u, size: %{public}zu", type, agents.size());
            for (auto& agent : agents) {
                agentsToRegister.emplace_back(type, agent);
            }
        }
    }
    for (const auto& [type, agent] : agentsToRegister) {
        if (wmsProxy->RegisterWindowManagerAgent(type, agent) != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_RECOVER, "register failed due to wms proxy, type: %{public}" PRIu32, type);
        }
    }
    // Note: Recover the listener that was registered during the SCB failure.
    ReregisterWindowManagerFaultAgent(wmsProxy);
}

void WindowAdapter::ReregisterWindowManagerFaultAgent(const sptr<IWindowManager>& proxy)
{
    if (!proxy) {
        TLOGE(WmsLogTag::WMS_RECOVER, "proxy is null");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(wmAgentMapMutex_);
        if (windowManagerAgentFaultMap_.empty()) {
            TLOGI(WmsLogTag::WMS_RECOVER, "no fault agent");
            return;
        }
        TLOGI(WmsLogTag::WMS_RECOVER, "begin Re-register fault agent");
        for (auto& [type, faultAgents] : windowManagerAgentFaultMap_) {
            TLOGI(WmsLogTag::WMS_RECOVER, "agent type: %{public}" PRIu32 ", size: %{public}zu",
                type, faultAgents.size());
            for (auto it = faultAgents.begin(); it != faultAgents.end();) {
                if (proxy->RegisterWindowManagerAgent(type, *it) == WMError::WM_OK) {
                    // Re-register success, move agent from fault map to normal.
                    windowManagerAgentMap_[type].insert(*it);
                    it = faultAgents.erase(it);
                } else {
                    TLOGE(WmsLogTag::WMS_RECOVER, "Re-register agent failed");
                    ++it;
                }
            }
        }
    }
    TLOGI(WmsLogTag::WMS_RECOVER, "end");
}

void WindowAdapter::OnUserSwitch()
{
    TLOGI(WmsLogTag::WMS_MULTI_USER, "User switched");
    ClearWindowAdapter();
    InitSSMProxy();
    ReregisterWindowManagerAgent();
    RecoverWindowPropertyChangeFlag();
}

bool WindowAdapter::InitSSMProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (isProxyValid_) {
        return true;
    }
    windowManagerServiceProxy_ = SessionManager::GetInstance(userId_).GetSceneSessionManagerProxy();
    if (!windowManagerServiceProxy_) {
        TLOGE(WmsLogTag::WMS_SCB, "windowManagerServiceProxy_ is null");
        return false;
    }
    sptr<IRemoteObject> remoteObject = windowManagerServiceProxy_->AsObject();
    if (!remoteObject) {
        TLOGE(WmsLogTag::WMS_SCB, "remoteObject is null");
        return false;
    }
    wmsDeath_ = sptr<WMSDeathRecipient>::MakeSptr(userId_);
    if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(wmsDeath_)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to add death recipient");
        return false;
    }
    if (!recoverInitialized_) {
        SessionManager::GetInstance(userId_).RegisterWindowManagerRecoverCallbackFunc([weakThis = wptr(this)] {
            auto windowAdapter = weakThis.promote();
            if (!windowAdapter) {
                TLOGE(WmsLogTag::WMS_SCB, "window adapter is null");
                return;
            }
            windowAdapter->WindowManagerAndSessionRecover();
        });
        recoverInitialized_ = true;
    }
    // U0 system user needs to subscribe OnUserSwitch event
    int32_t clientUserId = GetUserIdByUid(getuid());
    if (clientUserId == SYSTEM_USERID && !isRegisteredUserSwitchListener_) {
        SessionManager::GetInstance(userId_).RegisterUserSwitchListener([weakThis = wptr(this)] {
            auto instance = weakThis.promote();
            if (!instance) {
                TLOGE(WmsLogTag::WMS_SCB, "window adapter is null");
                return;
            }
            instance->OnUserSwitch();
        });
        isRegisteredUserSwitchListener_ = true;
    }
    isProxyValid_ = true;
    return true;
}

void WindowAdapter::ClearWindowAdapter()
{
    TLOGI(WmsLogTag::WMS_RECOVER, "begin clear");
    std::lock_guard<std::mutex> lock(mutex_);
    if ((windowManagerServiceProxy_ != nullptr) && (windowManagerServiceProxy_->AsObject() != nullptr)) {
        windowManagerServiceProxy_->AsObject()->RemoveDeathRecipient(wmsDeath_);
    }
    isProxyValid_ = false;
    windowManagerServiceProxy_ = nullptr;
}

WMError WindowAdapter::GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetTopWindowId(mainWinId, topWinId);
}

WMError WindowAdapter::GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetParentMainWindowId(windowId, mainWindowId);
}

WMError WindowAdapter::SetWindowLayoutMode(WindowLayoutMode mode)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetWindowLayoutMode(mode);
}

WMError WindowAdapter::UpdateProperty(sptr<WindowProperty>& windowProperty, PropertyChangeAction action)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->UpdateProperty(windowProperty, action);
}

WMError WindowAdapter::SetWindowGravity(uint32_t windowId, WindowGravity gravity, uint32_t percent)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetWindowGravity(windowId, gravity, percent);
}

WMError WindowAdapter::NotifyWindowTransition(sptr<WindowTransitionInfo> from, sptr<WindowTransitionInfo> to)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->NotifyWindowTransition(from, to, true);
}

void WindowAdapter::MinimizeWindowsByLauncher(std::vector<uint32_t> windowIds, bool isAnimated,
    sptr<RSIWindowAnimationFinishedCallback>& finishCallback)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->MinimizeWindowsByLauncher(windowIds, isAnimated, finishCallback);
}

WMError WindowAdapter::UpdateAvoidAreaListener(uint32_t windowId, bool haveListener)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->UpdateAvoidAreaListener(windowId, haveListener);
}

WMError WindowAdapter::UpdateRsTree(uint32_t windowId, bool isAdd)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->UpdateRsTree(windowId, isAdd);
}

WMError WindowAdapter::BindDialogTarget(uint32_t& windowId, sptr<IRemoteObject> targetToken)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->BindDialogTarget(windowId, targetToken);
}

void WindowAdapter::SetAnchorAndScale(int32_t x, int32_t y, float scale)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->SetAnchorAndScale(x, y, scale);
}

void WindowAdapter::SetAnchorOffset(int32_t deltaX, int32_t deltaY)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->SetAnchorOffset(deltaX, deltaY);
}

void WindowAdapter::OffWindowZoom()
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->OffWindowZoom();
}

/** @note @window.hierarchy */
WMError WindowAdapter::RaiseToAppTop(uint32_t windowId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->RaiseToAppTop(windowId);
}

std::shared_ptr<Media::PixelMap> WindowAdapter::GetSnapshot(int32_t windowId)
{
    INIT_PROXY_CHECK_RETURN(nullptr);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, nullptr);
    return wmsProxy->GetSnapshot(windowId);
}

WMError WindowAdapter::SetGestureNavigationEnabled(bool enable)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetGestureNavigationEnabled(enable);
}

void WindowAdapter::DispatchKeyEvent(uint32_t windowId, std::shared_ptr<MMI::KeyEvent> event)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->DispatchKeyEvent(windowId, event);
}

void WindowAdapter::NotifyDumpInfoResult(const std::vector<std::string>& info)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->NotifyDumpInfoResult(info);
}

WMError WindowAdapter::DumpSessionAll(std::vector<std::string>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return static_cast<WMError>(wmsProxy->DumpSessionAll(infos));
}

WMError WindowAdapter::DumpSessionWithId(int32_t persistentId, std::vector<std::string>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return static_cast<WMError>(wmsProxy->DumpSessionWithId(persistentId, infos));
}

WMError WindowAdapter::GetUIContentRemoteObj(int32_t persistentId, sptr<IRemoteObject>& uiContentRemoteObj)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return static_cast<WMError>(wmsProxy->GetUIContentRemoteObj(persistentId, uiContentRemoteObj));
}

WMError WindowAdapter::GetRootUIContentRemoteObj(DisplayId displayId, sptr<IRemoteObject>& uiContentRemoteObj)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetRootUIContentRemoteObj(displayId, uiContentRemoteObj);
}

WMError WindowAdapter::GetWindowAnimationTargets(std::vector<uint32_t> missionIds,
    std::vector<sptr<RSWindowAnimationTarget>>& targets)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetWindowAnimationTargets(missionIds, targets);
}

void WindowAdapter::SetMaximizeMode(MaximizeMode maximizeMode)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->SetMaximizeMode(maximizeMode);
}

MaximizeMode WindowAdapter::GetMaximizeMode()
{
    INIT_PROXY_CHECK_RETURN(MaximizeMode::MODE_FULL_FILL);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, MaximizeMode::MODE_FULL_FILL);
    return wmsProxy->GetMaximizeMode();
}

void WindowAdapter::GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        wmsProxy->GetFocusWindowInfo(focusInfo, displayId);
    } else {
        wmsProxy->GetFocusWindowInfo(focusInfo);
    }
}

void WindowAdapter::GetFocusWindowInfoByAbilityToken(FocusChangeInfo& focusInfo,
    const sptr<IRemoteObject>& abilityToken)
{
    INIT_PROXY_CHECK_RETURN();
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->GetFocusWindowInfoByAbilityToken(focusInfo, abilityToken);
}

WMError WindowAdapter::UpdateSessionAvoidAreaListener(int32_t persistentId, bool haveListener)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->UpdateSessionAvoidAreaListener(persistentId, haveListener));
}

WMError WindowAdapter::UpdateSessionTouchOutsideListener(int32_t& persistentId, bool haveListener)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(
        wmsProxy->UpdateSessionTouchOutsideListener(persistentId, haveListener));
}

WMError WindowAdapter::NotifyWindowExtensionVisibilityChange(int32_t pid, int32_t uid, bool visible)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->NotifyWindowExtensionVisibilityChange(pid, uid, visible));
}

WMError WindowAdapter::RaiseWindowToTop(int32_t persistentId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->RaiseWindowToTop(persistentId));
}

WMError WindowAdapter::UpdateSessionWindowVisibilityListener(int32_t persistentId, bool haveListener)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    WSError ret = wmsProxy->UpdateSessionWindowVisibilityListener(persistentId, haveListener);
    return static_cast<WMError>(ret);
}

WMError WindowAdapter::UpdateSessionOcclusionStateListener(int32_t persistentId, bool haveListener)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return wmsProxy->UpdateSessionOcclusionStateListener(persistentId, haveListener);
}

WMError WindowAdapter::ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(
        wmsProxy->ShiftAppWindowFocus(sourcePersistentId, targetPersistentId));
}

WMError WindowAdapter::SetSpecificWindowZIndex(WindowType windowType, int32_t zIndex, bool updateMap)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    WMError ret = static_cast<WMError>(wmsProxy->SetSpecificWindowZIndex(windowType, zIndex));
    if (updateMap && ret == WMError::WM_OK) {
        specificZIndexMap_[windowType] = zIndex;
    }
    TLOGI(WmsLogTag::WMS_FOCUS, "windowType: %{public}d, zIndex: %{public}d",
        windowType, zIndex);
    return ret;
}

void WindowAdapter::CreateAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
    const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
    sptr<WindowSessionProperty> property, int32_t& persistentId, sptr<ISession>& session,
    SystemSessionConfig& systemConfig, sptr<IRemoteObject> token)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->CreateAndConnectSpecificSession(sessionStage, eventChannel,
        surfaceNode, property, persistentId, session, systemConfig, token);
}

void WindowAdapter::RecoverAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
    const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
    sptr<WindowSessionProperty> property, sptr<ISession>& session, sptr<IRemoteObject> token)
{
    INIT_PROXY_CHECK_RETURN();
    TLOGI(WmsLogTag::WMS_RECOVER, "called");

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->RecoverAndConnectSpecificSession(
        sessionStage, eventChannel, surfaceNode, property, session, token);
}

WMError WindowAdapter::DestroyAndDisconnectSpecificSession(const int32_t persistentId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->DestroyAndDisconnectSpecificSession(persistentId));
}

WMError WindowAdapter::DestroyAndDisconnectSpecificSessionWithDetachCallback(const int32_t persistentId,
    const sptr<IRemoteObject>& callback)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(
        wmsProxy->DestroyAndDisconnectSpecificSessionWithDetachCallback(persistentId, callback));
}

WMError WindowAdapter::RecoverAndReconnectSceneSession(const sptr<ISessionStage>& sessionStage,
    const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
    sptr<ISession>& session, sptr<WindowSessionProperty> property, sptr<IRemoteObject> token)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);
    TLOGI(WmsLogTag::WMS_RECOVER, "called");

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    auto ret = wmsProxy->RecoverAndReconnectSceneSession(
        sessionStage, eventChannel, surfaceNode, session, property, token);
    if (ret != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_RECOVER, "failed, ret=%{public}d", ret);
        return WMError::WM_DO_NOTHING;
    }
    return WMError::WM_OK;
}

WMError WindowAdapter::SetSessionGravity(int32_t persistentId, SessionGravity gravity, uint32_t percent)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->SetSessionGravity(persistentId, gravity, percent));
}

WMError WindowAdapter::BindDialogSessionTarget(uint64_t persistentId, sptr<IRemoteObject> targetToken)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->BindDialogSessionTarget(persistentId, targetToken));
}

WMError WindowAdapter::RequestFocusStatus(int32_t persistentId, bool isFocused)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->RequestFocusStatus(persistentId, isFocused));
}

WMError WindowAdapter::RequestFocusStatusBySA(int32_t persistentId, bool isFocused,
    bool byForeground, FocusChangeReason reason)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->RequestFocusStatusBySA(persistentId, isFocused, byForeground, reason);
}

void WindowAdapter::AddExtensionWindowStageToSCB(const sptr<ISessionStage>& sessionStage,
    const sptr<IRemoteObject>& token, uint64_t surfaceNodeId, int64_t startModalExtensionTimeStamp,
    bool isConstrainedModal)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->AddExtensionWindowStageToSCB(sessionStage, token, surfaceNodeId, startModalExtensionTimeStamp,
        isConstrainedModal);
}

void WindowAdapter::RemoveExtensionWindowStageFromSCB(const sptr<ISessionStage>& sessionStage,
    const sptr<IRemoteObject>& token, bool isConstrainedModal)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->RemoveExtensionWindowStageFromSCB(sessionStage, token, isConstrainedModal);
}

void WindowAdapter::ProcessModalExtensionPointDown(const sptr<IRemoteObject>& token, int32_t posX, int32_t posY)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->ProcessModalExtensionPointDown(token, posX, posY);
}

void WindowAdapter::UpdateModalExtensionRect(const sptr<IRemoteObject>& token, Rect rect)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->UpdateModalExtensionRect(token, rect);
}

WMError WindowAdapter::AddOrRemoveSecureSession(int32_t persistentId, bool shouldHide)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->AddOrRemoveSecureSession(persistentId, shouldHide));
}

WMError WindowAdapter::UpdateExtWindowFlags(const sptr<IRemoteObject>& token, uint32_t extWindowFlags,
    uint32_t extWindowActions)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->UpdateExtWindowFlags(token, extWindowFlags, extWindowActions));
}

WMError WindowAdapter::GetHostWindowRect(int32_t hostWindowId, Rect& rect)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->GetHostWindowRect(hostWindowId, rect));
}

WMError WindowAdapter::GetHostGlobalScaledRect(int32_t hostWindowId, Rect& globalScaledRect)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->GetHostGlobalScaledRect(hostWindowId, globalScaledRect));
}

WMError WindowAdapter::GetFreeMultiWindowEnableState(bool& enable)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->GetFreeMultiWindowEnableState(enable));
}

WMError WindowAdapter::GetCallingWindowWindowStatus(uint32_t callingWindowId, WindowStatus& windowStatus)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->GetCallingWindowWindowStatus(callingWindowId, windowStatus));
}

WMError WindowAdapter::GetCallingWindowRect(uint32_t callingWindowId, Rect& rect)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->GetCallingWindowRect(callingWindowId, rect));
}

WMError WindowAdapter::GetWindowModeType(WindowModeType& windowModeType)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    WLOGFD("get window mode type");
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetWindowModeType(windowModeType);
}

WMError WindowAdapter::GetWindowStyleType(WindowStyleType& windowStyleType)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetWindowStyleType(windowStyleType);
}

WMError WindowAdapter::GetWindowIdsByCoordinate(DisplayId displayId, int32_t windowNumber,
    int32_t x, int32_t y, std::vector<int32_t>& windowIds)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetWindowIdsByCoordinate(displayId, windowNumber, x, y, windowIds);
}

sptr<IWindowManager> WindowAdapter::GetWindowManagerServiceProxy() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return windowManagerServiceProxy_;
}

WMError WindowAdapter::SkipSnapshotForAppProcess(int32_t pid, bool skip)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return wmsProxy->SkipSnapshotForAppProcess(pid, skip);
}

WMError WindowAdapter::SetProcessWatermark(int32_t pid, const std::string& watermarkName, bool isEnabled)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetProcessWatermark(pid, watermarkName, isEnabled);
}

WMError WindowAdapter::UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return wmsProxy->UpdateScreenLockStatusForApp(bundleName, isRelease);
}

WMError WindowAdapter::IsPcWindow(bool& isPcWindow)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->IsPcWindow(isPcWindow);
}

WMError WindowAdapter::IsFreeMultiWindowMode(bool& isFreeMultiWindow)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->IsFreeMultiWindow(isFreeMultiWindow);
}

WMError WindowAdapter::IsPcOrPadFreeMultiWindowMode(bool& isPcOrPadFreeMultiWindowMode)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->IsPcOrPadFreeMultiWindowMode(isPcOrPadFreeMultiWindowMode);
}

WMError WindowAdapter::IsWindowRectAutoSave(const std::string& key, bool& enabled, int persistentId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->IsWindowRectAutoSave(key, enabled, persistentId);
}

WMError WindowAdapter::SetImageForRecent(uint32_t imgResourceId, ImageFit imageFit, int32_t persistentId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetImageForRecent(imgResourceId, imageFit, persistentId);
}

WMError WindowAdapter::SetImageForRecentPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap,
    ImageFit imageFit, int32_t persistentId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetImageForRecentPixelMap(pixelMap, imageFit, persistentId);
}

WMError WindowAdapter::RemoveImageForRecent(int32_t persistentId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->RemoveImageForRecent(persistentId);
}

WMError WindowAdapter::ShiftAppWindowPointerEvent(int32_t sourceWindowId, int32_t targetWindowId, int32_t fingerId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->ShiftAppWindowPointerEvent(sourceWindowId, targetWindowId, fingerId);
}

WMError WindowAdapter::NotifyScreenshotEvent(ScreenshotEventType type)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->NotifyScreenshotEvent(type);
}

WMError WindowAdapter::SetStartWindowBackgroundColor(
    const std::string& moduleName, const std::string& abilityName, uint32_t color, int32_t uid)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetStartWindowBackgroundColor(moduleName, abilityName, color, uid);
}

WMError WindowAdapter::GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
    std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
}

WMError WindowAdapter::SetGlobalDragResizeType(DragResizeType dragResizeType)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetGlobalDragResizeType(dragResizeType);
}

WMError WindowAdapter::GetGlobalDragResizeType(DragResizeType& dragResizeType)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetGlobalDragResizeType(dragResizeType);
}

WMError WindowAdapter::SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetAppDragResizeType(bundleName, dragResizeType);
}

WMError WindowAdapter::GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetAppDragResizeType(bundleName, dragResizeType);
}

WMError WindowAdapter::SetAppKeyFramePolicy(const std::string& bundleName, const KeyFramePolicy& keyFramePolicy)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
}

WMError WindowAdapter::NotifyWatchGestureConsumeResult(int32_t keyCode, bool isConsumed)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->NotifyWatchGestureConsumeResult(keyCode, isConsumed);
}

WMError WindowAdapter::NotifyWatchFocusActiveChange(bool isActive)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->NotifyWatchFocusActiveChange(isActive);
}

WMError WindowAdapter::MinimizeByWindowId(const std::vector<int32_t>& windowIds)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->MinimizeByWindowId(windowIds);
}

WMError WindowAdapter::SetForegroundWindowNum(uint32_t windowNum)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetForegroundWindowNum(windowNum);
}

WMError WindowAdapter::UseImplicitAnimation(int32_t hostWindowId, bool useImplicit)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->UseImplicitAnimation(hostWindowId, useImplicit));
}

WMError WindowAdapter::AnimateTo(int32_t windowId, const WindowAnimationProperty& animationProperty,
    const WindowAnimationOption& animationOption)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->AnimateTo(windowId, animationProperty, animationOption);
}

WMError WindowAdapter::CreateUIEffectController(const sptr<IUIEffectControllerClient>& controllerClient,
    sptr<IUIEffectController>& controller, int32_t& controllerId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->CreateUIEffectController(controllerClient, controller, controllerId);
}

WMError WindowAdapter::GetPiPSettingSwitchStatus(bool& switchStatus)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetPiPSettingSwitchStatus(switchStatus);
}

WMError WindowAdapter::GetIsPipEnabled(bool& isPipEnabled)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetIsPipEnabled(isPipEnabled);
}

WMError WindowAdapter::AddSessionBlackList(
    const std::unordered_set<std::string>& bundleNames, const std::unordered_set<std::string>& privacyWindowTags)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->AddSessionBlackList(bundleNames, privacyWindowTags);
}

WMError WindowAdapter::NotifySupportRotationRegistered()
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->NotifySupportRotationRegistered();
}

WMError WindowAdapter::RemoveSessionBlackList(
    const std::unordered_set<std::string>& bundleNames, const std::unordered_set<std::string>& privacyWindowTags)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->RemoveSessionBlackList(bundleNames, privacyWindowTags);
}

void WindowAdapter::RegisterOutlineRecoverCallbackFunc(const OutlineRecoverCallbackFunc& callback)
{
    std::lock_guard<std::mutex> lock(outlineMutex_);
    outlineRecoverCallbackFunc_ = callback;
}

void WindowAdapter::UnregisterOutlineRecoverCallbackFunc()
{
    std::lock_guard<std::mutex> lock(outlineMutex_);
    outlineRecoverCallbackFunc_ = nullptr;
}

WMError WindowAdapter::UpdateOutline(const sptr<IRemoteObject>& remoteObject, const OutlineParams& outlineParams)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->UpdateOutline(remoteObject, outlineParams);
}
} // namespace Rosen
} // namespace OHOS
