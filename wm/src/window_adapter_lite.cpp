/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the Licenses at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is be distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "window_adapter_lite.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "scene_board_judgement.h"
#include "session_manager_lite.h"
#include "focus_change_info.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowAdapterLite"};
}
std::unordered_map<int32_t, sptr<WindowAdapterLite>> WindowAdapterLite::windowAdapterLiteMap_ = {};
std::mutex WindowAdapterLite::windowAdapterLiteMapMutex_;

#define INIT_PROXY_CHECK_RETURN(ret)        \
    do {                                    \
        if (!InitSSMProxy()) {              \
            WLOGFE("InitSSMProxy failed!"); \
            return ret;                     \
        }                                   \
    } while (false)

#define CHECK_PROXY_RETURN_ERROR_IF_NULL(proxy, ret)                      \
    do {                                                                  \
        if ((proxy) == nullptr) {                                         \
            TLOGE(WmsLogTag::DEFAULT, "window manager proxy is nullptr"); \
            return ret;                                                   \
        }                                                                 \
    } while(false)

#define CHECK_PROXY_RETURN_IF_NULL(proxy)                                 \
    do {                                                                  \
        if ((proxy) == nullptr) {                                         \
            TLOGE(WmsLogTag::DEFAULT, "window manager proxy is nullptr"); \
            return;                                                       \
        }                                                                 \
    } while(false)

WindowAdapterLite::~WindowAdapterLite()
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

WindowAdapterLite::WindowAdapterLite(const int32_t userId) : userId_(userId) {}

WindowAdapterLite& WindowAdapterLite::GetInstance()
{
    static auto instance = sptr<WindowAdapterLite>::MakeSptr();
    return *instance;
}

WindowAdapterLite& WindowAdapterLite::GetInstance(const int32_t userId)
{
    if (userId <= INVALID_USER_ID) {
        return GetInstance();
    }
    // multi-instance mode
    std::lock_guard<std::mutex> lock(windowAdapterLiteMapMutex_);
    auto iter = windowAdapterLiteMap_.find(userId);
    if (iter != windowAdapterLiteMap_.end() && iter->second) {
        return *iter->second;
    }
    auto instance = sptr<WindowAdapterLite>::MakeSptr(userId);
    windowAdapterLiteMap_.insert({ userId, instance });
    TLOGI(WmsLogTag::WMS_MULTI_USER, "get new instance, userId: %{public}d", userId);
    return *windowAdapterLiteMap_[userId];
}

WMError WindowAdapterLite::RegisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);

    {
        std::lock_guard<std::mutex> lock(windowManagerLiteAgentMapMutex_);
        if (windowManagerLiteAgentMap_.find(type) == windowManagerLiteAgentMap_.end()) {
            windowManagerLiteAgentMap_[type] = std::set<sptr<IWindowManagerAgent>>();
        }
        windowManagerLiteAgentMap_[type].insert(windowManagerAgent);
    }

    return wmsProxy->RegisterWindowManagerAgent(type, windowManagerAgent);
}

WMError WindowAdapterLite::UnregisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    auto ret = wmsProxy->UnregisterWindowManagerAgent(type, windowManagerAgent);

    std::lock_guard<std::mutex> lock(windowManagerLiteAgentMapMutex_);
    if (windowManagerLiteAgentMap_.find(type) == windowManagerLiteAgentMap_.end()) {
        TLOGW(WmsLogTag::WMS_MULTI_USER, "WindowManagerAgentType=%{public}d not found", type);
        return ret;
    }
    auto& agentSet = windowManagerLiteAgentMap_[type];
    auto agent = std::find(agentSet.begin(), agentSet.end(), windowManagerAgent);
    if (agent == agentSet.end()) {
        TLOGW(WmsLogTag::WMS_MULTI_USER, "Cannot find agent,  type=%{public}d", type);
        return ret;
    }
    agentSet.erase(agent);
    return ret;
}

WMError WindowAdapterLite::RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
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
        std::lock_guard<std::mutex> lock(windowManagerLiteAgentMapMutex_);
        windowManagerLiteAgentMap_[type].insert(windowManagerAgent);
    } else {
        TLOGE(WmsLogTag::WMS_SCB, "failed due to proxy, type: %{public}d", type);
    }
    return ret;
}

WMError WindowAdapterLite::UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
    uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    auto ret = wmsProxy->UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);

    observedFlags_ &= ~(static_cast<uint32_t>(windowInfoKey));
    interestedFlags_ &= ~interestInfo;

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY;
    std::lock_guard<std::mutex> lock(windowManagerLiteAgentMapMutex_);
    if (windowManagerLiteAgentMap_.find(type) == windowManagerLiteAgentMap_.end()) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "WINDOW_MANAGER_AGENT_TYPE_PROPERTY not found");
        return ret;
    }

    auto& agentSet = windowManagerLiteAgentMap_[type];
    auto agent = std::find(agentSet.begin(), agentSet.end(), windowManagerAgent);
    if (agent == agentSet.end()) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "Cannot find WINDOW_MANAGER_AGENT_TYPE_PROPERTY");
        return ret;
    }
    agentSet.erase(agent);

    return ret;
}

WMError WindowAdapterLite::RecoverWindowPropertyChangeFlag()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!windowManagerServiceProxy_ || !windowManagerServiceProxy_->AsObject()) {
        TLOGE(WmsLogTag::WMS_RECOVER, "proxy is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto ret = windowManagerServiceProxy_->RecoverWindowPropertyChangeFlag(observedFlags_, interestedFlags_);
    TLOGI(WmsLogTag::WMS_RECOVER, "ret=%{public}d", static_cast<int32_t>(ret));
    return ret;
}

// called after InitSSMProxy()
void WindowAdapterLite::ReregisterWindowManagerLiteAgent()
{
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);

    std::lock_guard<std::mutex> lock(windowManagerLiteAgentMapMutex_);
    for (const auto& it : windowManagerLiteAgentMap_) {
        TLOGI(WmsLogTag::WMS_MULTI_USER, "Window manager agent type=%{public}" PRIu32 ", size=%{public}" PRIu64,
            it.first, static_cast<uint64_t>(it.second.size()));
        for (auto& agent : it.second) {
            if (wmsProxy->RegisterWindowManagerAgent(it.first, agent) != WMError::WM_OK) {
                TLOGW(WmsLogTag::WMS_MULTI_USER, "Reregister window manager agent failed");
            }
        }
    }
}

WMError WindowAdapterLite::CheckWindowId(int32_t windowId, int32_t& pid)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->CheckWindowId(windowId, pid);
}

WMError WindowAdapterLite::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetVisibilityWindowInfo(infos);
}

WMError WindowAdapterLite::UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_DO_NOTHING);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_DO_NOTHING);
    return wmsProxy->UpdateScreenLockStatusForApp(bundleName, isRelease);
}

bool WindowAdapterLite::InitSSMProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (isProxyValid_) {
        return true;
    }
    windowManagerServiceProxy_ = SessionManagerLite::GetInstance(userId_).GetSceneSessionManagerLiteProxy();
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
    // U0 system user needs to subscribe OnUserSwitch event
    int32_t clientUserId = GetUserIdByUid(getuid());
    if (clientUserId == SYSTEM_USERID && !isRegisteredUserSwitchListener_) {
        SessionManagerLite::GetInstance(userId_).RegisterUserSwitchListener([weakThis = wptr(this)] {
            auto windowAdapter = weakThis.promote();
            if (!windowAdapter) {
                TLOGE(WmsLogTag::WMS_SCB, "window adapter is null");
                return;
            }
            windowAdapter->OnUserSwitch();
        });
        isRegisteredUserSwitchListener_ = true;
    }
    isProxyValid_ = true;
    return true;
}

void WindowAdapterLite::OnUserSwitch()
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "User switched lite");
    ClearWindowAdapter();
    InitSSMProxy();
    ReregisterWindowManagerLiteAgent();
    RecoverWindowPropertyChangeFlag();
}

void WindowAdapterLite::ClearWindowAdapter()
{
    WLOGD("ClearWindowAdapter");
    std::lock_guard<std::mutex> lock(mutex_);
    if (windowManagerServiceProxy_ != nullptr && windowManagerServiceProxy_->AsObject() != nullptr) {
        windowManagerServiceProxy_->AsObject()->RemoveDeathRecipient(wmsDeath_);
    }
    isProxyValid_ = false;
    windowManagerServiceProxy_ = nullptr;
}

WMSDeathRecipient::WMSDeathRecipient(int32_t userId) : userId_(userId) {}

void WMSDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    if (wptrDeath == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "wptrDeath is null");
        return;
    }
    sptr<IRemoteObject> object = wptrDeath.promote();
    if (!object) {
        TLOGE(WmsLogTag::WMS_SCB, "object is null");
        return;
    }
    TLOGI(WmsLogTag::WMS_SCB, "wms lite OnRemoteDied");
    WindowAdapterLite::GetInstance(userId_).ClearWindowAdapter();
    SessionManagerLite::GetInstance(userId_).ClearSessionManagerProxy();
}

void WindowAdapterLite::GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId)
{
    INIT_PROXY_CHECK_RETURN();
    WLOGFD("use Foucus window info proxy");

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        wmsProxy->GetFocusWindowInfo(focusInfo, displayId);
    } else {
        wmsProxy->GetFocusWindowInfo(focusInfo);
    }
}

void WindowAdapterLite::GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                                        std::vector<sptr<FocusChangeInfo>>& allFocusInfoList)
{
    INIT_PROXY_CHECK_RETURN();
    TLOGD(WmsLogTag::WMS_FOCUS, "request on adapter lite");
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_IF_NULL(wmsProxy);
    wmsProxy->GetAllGroupInfo(displayId2GroupIdMap, allFocusInfoList);
}

WMError WindowAdapterLite::GetWindowModeType(WindowModeType& windowModeType)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    WLOGFD("get window mode type");

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetWindowModeType(windowModeType);
}

WMError WindowAdapterLite::GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    TLOGD(WmsLogTag::WMS_MAIN, "get top main window info");

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetMainWindowInfos(topNum, topNInfo);
}

WMError WindowAdapterLite::GetMainWindowInfoByToken(const sptr<IRemoteObject>& abilityToken,
    MainWindowInfo& windowInfo)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    TLOGD(WmsLogTag::WMS_MAIN, "get main window info by token");

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetMainWindowInfoByToken(abilityToken, windowInfo);
}

WMError WindowAdapterLite::UpdateAnimationSpeedWithPid(pid_t pid, float speed)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    TLOGD(WmsLogTag::WMS_ANIMATION, "update animation speed with pid");
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->UpdateAnimationSpeedWithPid(pid, speed);
}

WMError WindowAdapterLite::GetCallingWindowInfo(CallingWindowInfo& callingWindowInfo)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    TLOGD(WmsLogTag::WMS_KEYBOARD, "get calling window info");

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetCallingWindowInfo(callingWindowInfo);
}

WMError WindowAdapterLite::GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    TLOGD(WmsLogTag::WMS_MAIN, "get all main window info");

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetAllMainWindowInfos(infos);
}

WMError WindowAdapterLite::ClearMainSessions(const std::vector<int32_t>& persistentIds)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    TLOGD(WmsLogTag::WMS_MAIN, "clear main sessions.");

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    std::vector<int32_t> clearFailedIds;
    return wmsProxy->ClearMainSessions(persistentIds, clearFailedIds);
}

WMError WindowAdapterLite::ClearMainSessions(const std::vector<int32_t>& persistentIds,
    std::vector<int32_t>& clearFailedIds)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    TLOGD(WmsLogTag::WMS_MAIN, "clear main sessions with failed ids.");

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->ClearMainSessions(persistentIds, clearFailedIds);
}

WMError WindowAdapterLite::RaiseWindowToTop(int32_t persistentId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return static_cast<WMError>(wmsProxy->RaiseWindowToTop(persistentId));
}

WMError WindowAdapterLite::RegisterWMSConnectionChangedListener(const WMSConnectionChangedCallbackFunc& callbackFunc)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "register listener");
    return SessionManagerLite::GetInstance(userId_).RegisterWMSConnectionChangedListener(callbackFunc);
}

WMError WindowAdapterLite::UnregisterWMSConnectionChangedListener()
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "unregister wms connection changed listener");
    return SessionManagerLite::GetInstance(userId_).UnregisterWMSConnectionChangedListener();
}

WMError WindowAdapterLite::GetWindowStyleType(WindowStyleType& windowStyleType)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetWindowStyleType(windowStyleType);
}

sptr<IWindowManagerLite> WindowAdapterLite::GetWindowManagerServiceProxy() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return windowManagerServiceProxy_;
}

WMError WindowAdapterLite::TerminateSessionByPersistentId(int32_t persistentId)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->TerminateSessionByPersistentId(persistentId);
}

WMError WindowAdapterLite::CloseTargetFloatWindow(const std::string& bundleName)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->CloseTargetFloatWindow(bundleName);
}

WMError WindowAdapterLite::CloseTargetPiPWindow(const std::string& bundleName)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->CloseTargetPiPWindow(bundleName);
}

WMError WindowAdapterLite::GetCurrentPiPWindowInfo(std::string& bundleName)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetCurrentPiPWindowInfo(bundleName);
}

WMError WindowAdapterLite::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);

    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetAccessibilityWindowInfo(infos);
}

WMError WindowAdapterLite::SetGlobalDragResizeType(DragResizeType dragResizeType)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetGlobalDragResizeType(dragResizeType);
}

WMError WindowAdapterLite::GetGlobalDragResizeType(DragResizeType& dragResizeType)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetGlobalDragResizeType(dragResizeType);
}

WMError WindowAdapterLite::SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetAppDragResizeType(bundleName, dragResizeType);
}

WMError WindowAdapterLite::GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetAppDragResizeType(bundleName, dragResizeType);
}

WMError WindowAdapterLite::SetAppKeyFramePolicy(const std::string& bundleName, const KeyFramePolicy& keyFramePolicy)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
}

WMError WindowAdapterLite::ListWindowInfo(const WindowInfoOption& windowInfoOption,
    std::vector<sptr<WindowInfo>>& infos)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->ListWindowInfo(windowInfoOption, infos);
}

WMError WindowAdapterLite::SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return static_cast<WMError>(wmsProxy->SendPointerEventForHover(pointerEvent));
}

WMError WindowAdapterLite::GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
    std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap)
{
    INIT_PROXY_CHECK_RETURN(WMError::WM_ERROR_SAMGR);
    auto wmsProxy = GetWindowManagerServiceProxy();
    CHECK_PROXY_RETURN_ERROR_IF_NULL(wmsProxy, WMError::WM_ERROR_SAMGR);
    return wmsProxy->GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
}

} // namespace Rosen
} // namespace OHOS
