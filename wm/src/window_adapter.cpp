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
#include <system_ability_definition.h>
#include <rs_window_animation_target.h>
#include "window_manager.h"
#include "window_manager_proxy.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "scene_board_judgement.h"
#include "session_manager.h"
#include "focus_change_info.h"
#include <unistd.h>
#include "window_session_impl.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowAdapter"};
}
WM_IMPLEMENT_SINGLE_INSTANCE(WindowAdapter)

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

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (windowManagerAgentMap_.find(type) == windowManagerAgentMap_.end()) {
            windowManagerAgentMap_[type] = std::set<sptr<IWindowManagerAgent>>();
        }
        windowManagerAgentMap_[type].insert(windowManagerAgent);
    }

    return wmsProxy->RegisterWindowManagerAgent(type, windowManagerAgent);
}

WMError WindowAdapter::UnregisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    auto ret = wmsProxy->UnregisterWindowManagerAgent(type, windowManagerAgent);

    std::lock_guard<std::mutex> lock(mutex_);
    if (windowManagerAgentMap_.find(type) == windowManagerAgentMap_.end()) {
        WLOGFW("WindowManagerAgentType = %{public}d not found", type);
        return ret;
    }

    auto& agentSet = windowManagerAgentMap_[type];
    auto agent = std::find(agentSet.begin(), agentSet.end(), windowManagerAgent);
    if (agent == agentSet.end()) {
        WLOGFW("Cannot find agent,  type = %{public}d", type);
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

WMError WindowAdapter::GetUnreliableWindowInfo(int32_t windowId,
    std::vector<sptr<UnreliableWindowInfo>>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetUnreliableWindowInfo(windowId, infos);
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

WMError WindowAdapter::GetAvoidAreaByType(uint32_t windowId, AvoidAreaType type, AvoidArea& avoidArea)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    avoidArea = wmsProxy->GetAvoidAreaByType(windowId, type);
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

WMError WindowAdapter::MinimizeAllAppWindows(DisplayId displayId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->MinimizeAllAppWindows(displayId);
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
    TLOGI(WmsLogTag::WMS_RECOVER, "persistentId = %{public}d", persistentId);
    std::lock_guard<std::mutex> lock(mutex_);
    sessionRecoverCallbackFuncMap_[persistentId] = callbackFunc;
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
    WLOGFI("RegisterWMSConnectionChangedListener in");
    return SessionManager::GetInstance().RegisterWMSConnectionChangedListener(callbackFunc);
}

void WindowAdapter::WindowManagerAndSessionRecover()
{
    ClearWindowAdapter();
    if (!InitSSMProxy()) {
        TLOGE(WmsLogTag::WMS_RECOVER, "InitSSMProxy failed");
        return;
    }

    ReregisterWindowManagerAgent();

    std::map<int32_t, SessionRecoverCallbackFunc> sessionRecoverCallbackFuncMap;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sessionRecoverCallbackFuncMap = sessionRecoverCallbackFuncMap_;
    }
    for (const auto& it : sessionRecoverCallbackFuncMap) {
        TLOGD(WmsLogTag::WMS_RECOVER, "Session recover callback, persistentId = %{public}" PRId32, it.first);
        auto ret = it.second();
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_RECOVER, "Session recover callback, persistentId = %{public}" PRId32 " is error",
                it.first);
            return;
        }
    }
}

void WindowAdapter::ReregisterWindowManagerAgent()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!windowManagerServiceProxy_ || !windowManagerServiceProxy_->AsObject()) {
        TLOGE(WmsLogTag::WMS_RECOVER, "proxy is null");
        return;
    }
    for (const auto& it : windowManagerAgentMap_) {
        TLOGI(WmsLogTag::WMS_RECOVER, "Window manager agent type = %{public}" PRIu32 ", size = %{public}" PRIu64,
            it.first, static_cast<uint64_t>(it.second.size()));
        for (auto& agent : it.second) {
            if (windowManagerServiceProxy_->RegisterWindowManagerAgent(it.first, agent) != WMError::WM_OK) {
                TLOGE(WmsLogTag::WMS_RECOVER, "failed");
            }
        }
    }
}

void WindowAdapter::OnUserSwitch()
{
    TLOGI(WmsLogTag::WMS_MULTI_USER, "User switched");
    ClearWindowAdapter();
    InitSSMProxy();
    ReregisterWindowManagerAgent();
}

bool WindowAdapter::InitSSMProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!isProxyValid_) {
        windowManagerServiceProxy_ = SessionManager::GetInstance().GetSceneSessionManagerProxy();
        if (!windowManagerServiceProxy_ || !windowManagerServiceProxy_->AsObject()) {
            WLOGFE("Failed to get system scene session manager services");
            return false;
        }
        wmsDeath_ = new (std::nothrow) WMSDeathRecipient();
        if (!wmsDeath_) {
            WLOGFE("Failed to create death Recipient ptr WMSDeathRecipient");
            return false;
        }
        sptr<IRemoteObject> remoteObject = windowManagerServiceProxy_->AsObject();
        if (remoteObject->IsProxyObject() && !remoteObject->AddDeathRecipient(wmsDeath_)) {
            WLOGFE("Failed to add death recipient");
            return false;
        }
        if (!recoverInitialized_) {
            SessionManager::GetInstance().RegisterWindowManagerRecoverCallbackFunc(
                [this] { this->WindowManagerAndSessionRecover(); });
            recoverInitialized_ = true;
        }
        // U0 system user needs to subscribe OnUserSwitch event
        int32_t clientUserId = GetUserIdByUid(getuid());
        if (clientUserId == SYSTEM_USERID && !isRegisteredUserSwitchListener_) {
            SessionManager::GetInstance().RegisterUserSwitchListener([this]() { this->OnUserSwitch(); });
            isRegisteredUserSwitchListener_ = true;
        }
        isProxyValid_ = true;
    }
    return true;
}

void WindowAdapter::ClearWindowAdapter()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ((windowManagerServiceProxy_ != nullptr) && (windowManagerServiceProxy_->AsObject() != nullptr)) {
        windowManagerServiceProxy_->AsObject()->RemoveDeathRecipient(wmsDeath_);
    }
    isProxyValid_ = false;
    windowManagerServiceProxy_ = nullptr;
}

void WMSDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    if (wptrDeath == nullptr) {
        WLOGFE("wptrDeath is null");
        return;
    }

    sptr<IRemoteObject> object = wptrDeath.promote();
    if (!object) {
        WLOGFE("object is null");
        return;
    }
    WLOGI("wms OnRemoteDied");
    SingletonContainer::Get<WindowAdapter>().ClearWindowAdapter();
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

WMError WindowAdapter::SetGestureNavigaionEnabled(bool enable)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetGestureNavigaionEnabled(enable);
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

void WindowAdapter::GetFocusWindowInfo(FocusChangeInfo& focusInfo)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->GetFocusWindowInfo(focusInfo);
}

WMError WindowAdapter::UpdateSessionAvoidAreaListener(int32_t& persistentId, bool haveListener)
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

WMError WindowAdapter::ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(
        wmsProxy->ShiftAppWindowFocus(sourcePersistentId, targetPersistentId));
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
        TLOGE(WmsLogTag::WMS_RECOVER, "failed, ret = %{public}d", ret);
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

void WindowAdapter::AddExtensionWindowStageToSCB(const sptr<ISessionStage>& sessionStage,
    const sptr<IRemoteObject>& token, uint64_t surfaceNodeId)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->AddExtensionWindowStageToSCB(sessionStage, token, surfaceNodeId);
}

void WindowAdapter::RemoveExtensionWindowStageFromSCB(const sptr<ISessionStage>& sessionStage,
    const sptr<IRemoteObject>& token)
{
    INIT_PROXY_CHECK_RETURN();

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->RemoveExtensionWindowStageFromSCB(sessionStage, token);
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

WMError WindowAdapter::GetFreeMultiWindowEnableState(bool& enable)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->GetFreeMultiWindowEnableState(enable));
}

WMError WindowAdapter::GetCallingWindowWindowStatus(int32_t persistentId, WindowStatus& windowStatus)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->GetCallingWindowWindowStatus(persistentId, windowStatus));
}

WMError WindowAdapter::GetCallingWindowRect(int32_t persistentId, Rect& rect)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return static_cast<WMError>(wmsProxy->GetCallingWindowRect(persistentId, rect));
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

WMError WindowAdapter::ReleaseForegroundSessionScreenLock()
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return wmsProxy->ReleaseForegroundSessionScreenLock();
}

WMError WindowAdapter::IsWindowRectAutoSave(const std::string& key, bool& enabled)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->IsWindowRectAutoSave(key, enabled);
}

WMError WindowAdapter::GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
    std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
}

WMError WindowAdapter::IsPcOrPadFreeMultiWindowMode(bool& isPcOrPadFreeMultiWindowMode)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->IsPcOrPadFreeMultiWindowMode(isPcOrPadFreeMultiWindowMode);
}
} // namespace Rosen
} // namespace OHOS
