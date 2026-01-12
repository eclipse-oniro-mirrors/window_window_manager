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

#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/scene_session_manager_lite.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SceneSessionManagerLite" };
} // namespace

SceneSessionManagerLite& SceneSessionManagerLite::GetInstance()
{
    static sptr<SceneSessionManagerLite> instance(new SceneSessionManagerLite());
    return *instance;
}

WSError SceneSessionManagerLite::SetSessionContinueState(const sptr<IRemoteObject>& token,
    const ContinueState& continueState)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().SetSessionContinueState(token, continueState);
}

WSError SceneSessionManagerLite::SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().SetSessionLabel(token, label);
}

WSError SceneSessionManagerLite::SetSessionIcon(const sptr<IRemoteObject>& token,
    const std::shared_ptr<Media::PixelMap>& icon)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().SetSessionIcon(token, icon);
}

WSError SceneSessionManagerLite::SetSessionIconForThirdParty(const sptr<IRemoteObject>& token,
    const std::shared_ptr<Media::PixelMap>& icon)
{
    TLOGD(WmsLogTag::WMS_MAIN, "in");
    return SceneSessionManager::GetInstance().SetSessionIconForThirdParty(token, icon);
}

WSError SceneSessionManagerLite::IsValidSessionIds(
    const std::vector<int32_t>& sessionIds, std::vector<bool>& results)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().IsValidSessionIds(sessionIds, results);
}

WSError SceneSessionManagerLite::GetSessionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<SessionInfoBean>& sessionInfos)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().GetSessionInfos(deviceId, numMax, sessionInfos);
}

WSError SceneSessionManagerLite::GetMainWindowStatesByPid(int32_t pid, std::vector<MainWindowState>& windowStates)
{
    return SceneSessionManager::GetInstance().GetMainWindowStatesByPid(pid, windowStates);
}

WSError SceneSessionManagerLite::GetSessionInfo(const std::string& deviceId,
    int32_t persistentId, SessionInfoBean& sessionInfo)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().GetSessionInfo(deviceId, persistentId, sessionInfo);
}

WSError SceneSessionManagerLite::GetSessionInfoByContinueSessionId(
    const std::string& continueSessionId, SessionInfoBean& sessionInfo)
{
    TLOGD(WmsLogTag::WMS_MAIN, "continueSessionId: %{public}s", continueSessionId.c_str());
    return SceneSessionManager::GetInstance().GetSessionInfoByContinueSessionId(continueSessionId, sessionInfo);
}

WSError SceneSessionManagerLite::RegisterSessionListener(const sptr<ISessionListener>& listener, bool isRecover)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().RegisterSessionListener(listener);
}

WSError SceneSessionManagerLite::UnRegisterSessionListener(const sptr<ISessionListener>& listener)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().UnRegisterSessionListener(listener);
}

WSError SceneSessionManagerLite::TerminateSessionNew(
    const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().TerminateSessionNew(info, needStartCaller, isFromBroker);
}

WSError SceneSessionManagerLite::GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
    SessionSnapshot& snapshot, bool isLowResolution)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().GetSessionSnapshot(deviceId, persistentId, snapshot, isLowResolution);
}

WSError SceneSessionManagerLite::PendingSessionToForeground(const sptr<IRemoteObject>& token, int32_t windowMode)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().PendingSessionToForeground(token, windowMode);
}

WSError SceneSessionManagerLite::PendingSessionToBackground(const sptr<IRemoteObject>& token,
    const BackgroundParams& params)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().PendingSessionToBackground(token, params);
}

WSError SceneSessionManagerLite::PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
    bool shouldBackToCaller)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().PendingSessionToBackgroundForDelegator(token, shouldBackToCaller);
}

WSError SceneSessionManagerLite::GetFocusSessionToken(sptr<IRemoteObject>& token, DisplayId displayId)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().GetFocusSessionToken(token, displayId);
}

WSError SceneSessionManagerLite::GetFocusSessionElement(AppExecFwk::ElementName& element, DisplayId displayId)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().GetFocusSessionElement(element, displayId);
}

WSError SceneSessionManagerLite::IsFocusWindowParent(const sptr<IRemoteObject>& token, bool& isParent)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "in");
    return SceneSessionManager::GetInstance().IsFocusWindowParent(token, isParent);
}

WSError SceneSessionManagerLite::ClearSession(int32_t persistentId)
{
    WLOGFD("Id: %{public}d", persistentId);
    return SceneSessionManager::GetInstance().ClearSession(persistentId);
}

WSError SceneSessionManagerLite::ClearAllSessions()
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().ClearAllSessions();
}

WSError SceneSessionManagerLite::LockSession(int32_t sessionId)
{
    WLOGFD("Id: %{public}d", sessionId);
    return SceneSessionManager::GetInstance().LockSession(sessionId);
}

WSError SceneSessionManagerLite::UnlockSession(int32_t sessionId)
{
    WLOGFD("Id: %{public}d", sessionId);
    return SceneSessionManager::GetInstance().UnlockSession(sessionId);
}

WSError SceneSessionManagerLite::MoveSessionsToForeground(const std::vector<int32_t>& sessionIds, int32_t topSessionId)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().MoveSessionsToForeground(sessionIds, topSessionId);
}

WSError SceneSessionManagerLite::MoveSessionsToBackground(const std::vector<int32_t>& sessionIds,
    std::vector<int32_t>& result)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().MoveSessionsToBackground(sessionIds, result);
}

void SceneSessionManagerLite::GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId)
{
    return SceneSessionManager::GetInstance().GetFocusWindowInfo(focusInfo, displayId);
}

void SceneSessionManagerLite::GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                                              std::vector<sptr<FocusChangeInfo>>& allFocusInfoList)
{
    SceneSessionManager::GetInstance().GetAllGroupInfo(displayId2GroupIdMap, allFocusInfoList);
}

WMError SceneSessionManagerLite::RegisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    return SceneSessionManager::GetInstance().RegisterWindowManagerAgent(type, windowManagerAgent);
}

WMError SceneSessionManagerLite::UnregisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    return SceneSessionManager::GetInstance().UnregisterWindowManagerAgent(type, windowManagerAgent);
}

WMError SceneSessionManagerLite::CheckWindowId(int32_t windowId, int32_t& pid)
{
    return SceneSessionManager::GetInstance().CheckWindowId(windowId, pid);
}

WMError SceneSessionManagerLite::UpdateWindowModeByIdForUITest(int32_t windowId, int32_t updateMode)
{
    return SceneSessionManager::GetInstance().UpdateWindowModeByIdForUITest(windowId, updateMode);
}

WMError SceneSessionManagerLite::CheckUIExtensionCreation(int32_t windowId, uint32_t tokenId,
    const AppExecFwk::ElementName& element, AppExecFwk::ExtensionAbilityType extensionAbilityType, int32_t& pid)
{
    return SceneSessionManager::GetInstance().CheckUIExtensionCreation(windowId, tokenId, element, extensionAbilityType,
        pid);
}

WMError SceneSessionManagerLite::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos)
{
    return SceneSessionManager::GetInstance().GetVisibilityWindowInfo(infos);
}

WMError SceneSessionManagerLite::UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease)
{
    return SceneSessionManager::GetInstance().UpdateScreenLockStatusForApp(bundleName, isRelease);
}

WSError SceneSessionManagerLite::UpdateWindowMode(int32_t persistentId, int32_t windowMode)
{
    return SceneSessionManager::GetInstance().UpdateWindowMode(persistentId, windowMode);
}

WMError SceneSessionManagerLite::GetWindowModeType(WindowModeType& windowModeType)
{
    return SceneSessionManager::GetInstance().GetWindowModeType(windowModeType);
}

WMError SceneSessionManagerLite::GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo)
{
    return SceneSessionManager::GetInstance().GetMainWindowInfos(topNum, topNInfo);
}

WMError SceneSessionManagerLite::UpdateAnimationSpeedWithPid(pid_t pid, float speed)
{
    return SceneSessionManager::GetInstance().UpdateAnimationSpeedWithPid(pid, speed);
}

WMError SceneSessionManagerLite::GetCallingWindowInfo(CallingWindowInfo& callingWindowInfo)
{
    return SceneSessionManager::GetInstance().GetCallingWindowInfo(callingWindowInfo);
}

WMError SceneSessionManagerLite::GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos)
{
    return SceneSessionManager::GetInstance().GetAllMainWindowInfos(infos);
}

WMError SceneSessionManagerLite::GetMainWindowInfoByToken(const sptr<IRemoteObject>& abilityToken,
    MainWindowInfo& windowInfo)
{
    return SceneSessionManager::GetInstance().GetMainWindowInfoByToken(abilityToken, windowInfo);
}

WMError SceneSessionManagerLite::ClearMainSessions(const std::vector<int32_t>& persistentIds,
    std::vector<int32_t>& clearFailedIds)
{
    return SceneSessionManager::GetInstance().ClearMainSessions(persistentIds, clearFailedIds);
}

WSError SceneSessionManagerLite::RaiseWindowToTop(int32_t persistentId)
{
    return SceneSessionManager::GetInstance().RaiseWindowToTop(persistentId);
}

WSError SceneSessionManagerLite::RegisterIAbilityManagerCollaborator(int32_t type,
    const sptr<AAFwk::IAbilityManagerCollaborator>& impl)
{
    return SceneSessionManager::GetInstance().RegisterIAbilityManagerCollaborator(type, impl);
}

WSError SceneSessionManagerLite::UnregisterIAbilityManagerCollaborator(int32_t type)
{
    return SceneSessionManager::GetInstance().UnregisterIAbilityManagerCollaborator(type);
}

WMError SceneSessionManagerLite::GetWindowStyleType(WindowStyleType& windowStyletype)
{
    return SceneSessionManager::GetInstance().GetWindowStyleType(windowStyletype);
}

WMError SceneSessionManagerLite::TerminateSessionByPersistentId(int32_t persistentId)
{
    return SceneSessionManager::GetInstance().TerminateSessionByPersistentId(persistentId);
}

WMError SceneSessionManagerLite::CloseTargetFloatWindow(const std::string& bundleName)
{
    return SceneSessionManager::GetInstance().CloseTargetFloatWindow(bundleName);
}

WMError SceneSessionManagerLite::CloseTargetPiPWindow(const std::string& bundleName)
{
    return SceneSessionManager::GetInstance().CloseTargetPiPWindow(bundleName);
}

WMError SceneSessionManagerLite::GetCurrentPiPWindowInfo(std::string& bundleName)
{
    return SceneSessionManager::GetInstance().GetCurrentPiPWindowInfo(bundleName);
}

WMError SceneSessionManagerLite::GetRootMainWindowId(int32_t persistentId, int32_t& hostWindowId)
{
    return SceneSessionManager::GetInstance().GetRootMainWindowId(persistentId, hostWindowId);
}

WMError SceneSessionManagerLite::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos)
{
    return SceneSessionManager::GetInstance().GetAccessibilityWindowInfo(infos);
}

WSError SceneSessionManagerLite::NotifyAppUseControlList(
    ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList)
{
    return SceneSessionManager::GetInstance().NotifyAppUseControlList(type, userId, controlList);
}

WMError SceneSessionManagerLite::MinimizeMainSession(const std::string& bundleName, int32_t appIndex, int32_t userId)
{
    return SceneSessionManager::GetInstance().MinimizeMainSession(bundleName, appIndex, userId);
}

WMError SceneSessionManagerLite::LockSessionByAbilityInfo(const AbilityInfoBase& abilityInfo, bool isLock)
{
    return SceneSessionManager::GetInstance().LockSessionByAbilityInfo(abilityInfo, isLock);
}

WMError SceneSessionManagerLite::HasFloatingWindowForeground(const sptr<IRemoteObject>& abilityToken,
    bool& hasOrNot)
{
    return SceneSessionManager::GetInstance().HasFloatingWindowForeground(abilityToken,
        hasOrNot);
}

WMError SceneSessionManagerLite::RegisterSessionLifecycleListenerByIds(const sptr<ISessionLifecycleListener>& listener,
    const std::vector<int32_t>& persistentIdList)
{
    return SceneSessionManager::GetInstance().RegisterSessionLifecycleListener(listener, persistentIdList);
}

WMError SceneSessionManagerLite::RegisterSessionLifecycleListenerByBundles(
    const sptr<ISessionLifecycleListener>& listener, const std::vector<std::string>& bundleNameList)
{
    return SceneSessionManager::GetInstance().RegisterSessionLifecycleListener(listener, bundleNameList);
}

WMError SceneSessionManagerLite::UnregisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener)
{
    return SceneSessionManager::GetInstance().UnregisterSessionLifecycleListener(listener);
}

WMError SceneSessionManagerLite::SetGlobalDragResizeType(DragResizeType dragResizeType)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "set global drag resize lite in: %{public}d",
        static_cast<int32_t>(dragResizeType));
    return SceneSessionManager::GetInstance().SetGlobalDragResizeType(dragResizeType);
}

WMError SceneSessionManagerLite::GetGlobalDragResizeType(DragResizeType& dragResizeType)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "get global drag resize lite in");
    return SceneSessionManager::GetInstance().GetGlobalDragResizeType(dragResizeType);
}

WMError SceneSessionManagerLite::SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "set app drag resize lite in: %{public}s %{public}d",
        bundleName.c_str(), static_cast<int32_t>(dragResizeType));
    return SceneSessionManager::GetInstance().SetAppDragResizeType(bundleName, dragResizeType);
}

WMError SceneSessionManagerLite::GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "get app drag resize lite in: %{public}s", bundleName.c_str());
    return SceneSessionManager::GetInstance().GetAppDragResizeType(bundleName, dragResizeType);
}

WMError SceneSessionManagerLite::SetAppKeyFramePolicy(const std::string& bundleName,
    const KeyFramePolicy& keyFramePolicy)
{
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "set app key frame lite in: %{public}s %{public}d",
        bundleName.c_str(), keyFramePolicy.enabled());
    return SceneSessionManager::GetInstance().SetAppKeyFramePolicy(bundleName, keyFramePolicy);
}

WMError SceneSessionManagerLite::ListWindowInfo(const WindowInfoOption& windowInfoOption,
    std::vector<sptr<WindowInfo>>& infos)
{
    return SceneSessionManager::GetInstance().ListWindowInfo(windowInfoOption, infos);
}

WMError SceneSessionManagerLite::RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
    uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    return SceneSessionManager::GetInstance().RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo,
        windowManagerAgent);
}

WMError SceneSessionManagerLite::UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
    uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    return SceneSessionManager::GetInstance().UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo,
        windowManagerAgent);
}

WMError SceneSessionManagerLite::RecoverWindowPropertyChangeFlag(uint32_t observedFlags, uint32_t interestedFlags)
{
    return SceneSessionManager::GetInstance().RecoverWindowPropertyChangeFlag(observedFlags, interestedFlags);
}

WSError SceneSessionManagerLite::GetRecentMainSessionInfoList(std::vector<RecentSessionInfo>& recentSessionInfoList)
{
    return SceneSessionManager::GetInstance().GetRecentMainSessionInfoList(recentSessionInfoList);
}

WSError SceneSessionManagerLite::PendingSessionToBackgroundByPersistentId(const int32_t persistentId,
    bool shouldBackToCaller)
{
    return SceneSessionManager::GetInstance().PendingSessionToBackgroundByPersistentId(persistentId,
        shouldBackToCaller);
}

WMError SceneSessionManagerLite::CreateNewInstanceKey(const std::string& bundleName, std::string& instanceKey)
{
    return SceneSessionManager::GetInstance().CreateNewInstanceKey(bundleName, instanceKey);
}

WMError SceneSessionManagerLite::GetRouterStackInfo(
    int32_t persistentId, const sptr<ISessionRouterStackListener>& listener)
{
    return SceneSessionManager::GetInstance().GetRouterStackInfo(persistentId, listener);
}

WMError SceneSessionManagerLite::RemoveInstanceKey(const std::string& bundleName, const std::string& instanceKey)
{
    return SceneSessionManager::GetInstance().RemoveInstanceKey(bundleName, instanceKey);
}

WMError SceneSessionManagerLite::TransferSessionToTargetScreen(const TransferSessionInfo& info)
{
    return SceneSessionManager::GetInstance().NotifyTransferSessionToTargetScreen(info);
}

WMError SceneSessionManagerLite::UpdateKioskAppList(const std::vector<std::string>& kioskAppList)
{
    return SceneSessionManager::GetInstance().UpdateKioskAppList(kioskAppList);
}

WMError SceneSessionManagerLite::EnterKioskMode(const sptr<IRemoteObject>& token)
{
    return SceneSessionManager::GetInstance().EnterKioskMode(token);
}

WMError SceneSessionManagerLite::ExitKioskMode(const sptr<IRemoteObject>& token)
{
    return SceneSessionManager::GetInstance().ExitKioskMode();
}

WSError SceneSessionManagerLite::SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    return SceneSessionManager::GetInstance().SendPointerEventForHover(pointerEvent);
}

WMError SceneSessionManagerLite::SetPipEnableByScreenId(int32_t screenId, bool isEnabled)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().SetPipEnableByScreenId(screenId, isEnabled);
}

WMError SceneSessionManagerLite::UnsetPipEnableByScreenId(int32_t screenId)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().UnsetPipEnableByScreenId(screenId);
}

WMError SceneSessionManagerLite::RegisterPipChgListenerByScreenId(int32_t screenId,
    const sptr<IPipChangeListener>& listener)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().RegisterPipChgListenerByScreenId(screenId, listener);
}

WMError SceneSessionManagerLite::UnregisterPipChgListenerByScreenId(int32_t screenId)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().UnregisterPipChgListenerByScreenId(screenId);
}

WMError SceneSessionManagerLite::GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
    std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
}

WMError SceneSessionManagerLite::GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    return SceneSessionManager::GetInstance().GetParentMainWindowId(windowId, mainWindowId);
}
} // namespace OHOS::Rosen
