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

WM_IMPLEMENT_SINGLE_INSTANCE(SceneSessionManagerLite)

WSError SceneSessionManagerLite::SetSessionContinueState(const sptr<IRemoteObject>& token,
    const ContinueState& continueState)
{
    WLOGFD("run SetSessionContinueState");
    return SceneSessionManager::GetInstance().SetSessionContinueState(token, continueState);
}

WSError SceneSessionManagerLite::SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label)
{
    WLOGFD("run SetSessionLabel");
    return SceneSessionManager::GetInstance().SetSessionLabel(token, label);
}

WSError SceneSessionManagerLite::SetSessionIcon(const sptr<IRemoteObject>& token,
    const std::shared_ptr<Media::PixelMap>& icon)
{
    WLOGFD("run SetSessionIcon");
    return SceneSessionManager::GetInstance().SetSessionIcon(token, icon);
}

WSError SceneSessionManagerLite::IsValidSessionIds(
    const std::vector<int32_t>& sessionIds, std::vector<bool>& results)
{
    WLOGFD("run IsValidSessionIds");
    return SceneSessionManager::GetInstance().IsValidSessionIds(sessionIds, results);
}

WSError SceneSessionManagerLite::GetSessionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<SessionInfoBean>& sessionInfos)
{
    WLOGFD("run GetSessionInfos");
    return SceneSessionManager::GetInstance().GetSessionInfos(deviceId, numMax, sessionInfos);
}

WSError SceneSessionManagerLite::GetMainWindowStatesByPid(int32_t pid, std::vector<MainWindowState>& windowStates)
{
    return SceneSessionManager::GetInstance().GetMainWindowStatesByPid(pid, windowStates);
}

WSError SceneSessionManagerLite::GetSessionInfo(const std::string& deviceId,
    int32_t persistentId, SessionInfoBean& sessionInfo)
{
    WLOGFD("run GetSessionInfo");
    return SceneSessionManager::GetInstance().GetSessionInfo(deviceId, persistentId, sessionInfo);
}

WSError SceneSessionManagerLite::GetSessionInfoByContinueSessionId(
    const std::string& continueSessionId, SessionInfoBean& sessionInfo)
{
    TLOGD(WmsLogTag::WMS_MAIN, "liteProxy -> continueSessionId: %{public}s", continueSessionId.c_str());
    return SceneSessionManager::GetInstance().GetSessionInfoByContinueSessionId(continueSessionId, sessionInfo);
}

WSError SceneSessionManagerLite::RegisterSessionListener(const sptr<ISessionListener>& listener, bool isRecover)
{
    WLOGFD("run RegisterSessionListener");
    return SceneSessionManager::GetInstance().RegisterSessionListener(listener);
}

WSError SceneSessionManagerLite::UnRegisterSessionListener(const sptr<ISessionListener>& listener)
{
    WLOGFD("run UnRegisterSessionListener");
    return SceneSessionManager::GetInstance().UnRegisterSessionListener(listener);
}

WSError SceneSessionManagerLite::TerminateSessionNew(
    const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker)
{
    WLOGFD("run TerminateSessionNew");
    return SceneSessionManager::GetInstance().TerminateSessionNew(info, needStartCaller, isFromBroker);
}

WSError SceneSessionManagerLite::GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
    SessionSnapshot& snapshot, bool isLowResolution)
{
    WLOGFD("run GetSessionSnapshot");
    return SceneSessionManager::GetInstance().GetSessionSnapshot(deviceId, persistentId, snapshot, isLowResolution);
}

WSError SceneSessionManagerLite::PendingSessionToForeground(const sptr<IRemoteObject>& token)
{
    WLOGFD("run PendingSessionToForeground");
    return SceneSessionManager::GetInstance().PendingSessionToForeground(token);
}

WSError SceneSessionManagerLite::PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
    bool shouldBackToCaller)
{
    WLOGFD("run PendingSessionToBackgroundForDelegator");
    return SceneSessionManager::GetInstance().PendingSessionToBackgroundForDelegator(token, shouldBackToCaller);
}

WSError SceneSessionManagerLite::GetFocusSessionToken(sptr<IRemoteObject>& token)
{
    WLOGFD("run GetFocusSessionToken");
    return SceneSessionManager::GetInstance().GetFocusSessionToken(token);
}

WSError SceneSessionManagerLite::GetFocusSessionElement(AppExecFwk::ElementName& element)
{
    WLOGFD("run GetFocusSessionElement");
    return SceneSessionManager::GetInstance().GetFocusSessionElement(element);
}

WSError SceneSessionManagerLite::ClearSession(int32_t persistentId)
{
    WLOGFD("run ClearSession with persistentId: %{public}d", persistentId);
    return SceneSessionManager::GetInstance().ClearSession(persistentId);
}

WSError SceneSessionManagerLite::ClearAllSessions()
{
    WLOGFD("run ClearAllSessions");
    return SceneSessionManager::GetInstance().ClearAllSessions();
}

WSError SceneSessionManagerLite::LockSession(int32_t sessionId)
{
    WLOGFD("run LockSession with persistentId: %{public}d", sessionId);
    return SceneSessionManager::GetInstance().LockSession(sessionId);
}

WSError SceneSessionManagerLite::UnlockSession(int32_t sessionId)
{
    WLOGFD("run UnlockSession with persistentId: %{public}d", sessionId);
    return SceneSessionManager::GetInstance().UnlockSession(sessionId);
}

WSError SceneSessionManagerLite::MoveSessionsToForeground(const std::vector<int32_t>& sessionIds, int32_t topSessionId)
{
    WLOGFD("run MoveSessionsToForeground");
    return SceneSessionManager::GetInstance().MoveSessionsToForeground(sessionIds, topSessionId);
}

WSError SceneSessionManagerLite::MoveSessionsToBackground(const std::vector<int32_t>& sessionIds,
    std::vector<int32_t>& result)
{
    WLOGFD("run MoveSessionsToBackground");
    return SceneSessionManager::GetInstance().MoveSessionsToBackground(sessionIds, result);
}

void SceneSessionManagerLite::GetFocusWindowInfo(FocusChangeInfo& focusInfo)
{
    return SceneSessionManager::GetInstance().GetFocusWindowInfo(focusInfo);
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

WSError SceneSessionManagerLite::UpdateWindowMode(int32_t persistentId, int32_t windowMode)
{
    return SceneSessionManager::GetInstance().UpdateWindowMode(persistentId, windowMode);
}

WMError SceneSessionManagerLite::GetWindowModeType(WindowModeType& windowModeType)
{
    return SceneSessionManager::GetInstance().GetWindowModeType(windowModeType);
}

WSError SceneSessionManagerLite::RaiseWindowToTop(int32_t persistentId)
{
    return SceneSessionManager::GetInstance().RaiseWindowToTop(persistentId);
}

WMError SceneSessionManagerLite::GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo)
{
    return SceneSessionManager::GetInstance().GetMainWindowInfos(topNum, topNInfo);
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

WMError SceneSessionManagerLite::GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos)
{
    return SceneSessionManager::GetInstance().GetAllMainWindowInfos(infos);
}

WMError SceneSessionManagerLite::ClearMainSessions(const std::vector<int32_t>& persistentIds,
    std::vector<int32_t>& clearFailedIds)
{
    return SceneSessionManager::GetInstance().ClearMainSessions(persistentIds, clearFailedIds);
}

WMError SceneSessionManagerLite::GetWindowStyleType(WindowStyleType& windowStyletype)
{
    return SceneSessionManager::GetInstance().GetWindowStyleType(windowStyletype);
}

WMError SceneSessionManagerLite::TerminateSessionByPersistentId(int32_t persistentId)
{
    return SceneSessionManager::GetInstance().TerminateSessionByPersistentId(persistentId);
}

WMError SceneSessionManagerLite::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos)
{
    return SceneSessionManager::GetInstance().GetAccessibilityWindowInfo(infos);
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
WSError SceneSessionManagerLite::NotifyAppUseControlList(
    ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList)
{
    return SceneSessionManager::GetInstance().NotifyAppUseControlList(type, userId, controlList);
}

WMError SceneSessionManagerLite::GetRootMainWindowId(int32_t persistentId, int32_t& hostWindowId)
{
    return SceneSessionManager::GetInstance().GetRootMainWindowId(persistentId, hostWindowId);
}

WMError SceneSessionManagerLite::MinimizeMainSession(const std::string& bundleName, int32_t appIndex, int32_t userId)
{
    return SceneSessionManager::GetInstance().MinimizeMainSession(bundleName, appIndex, userId);
}

WMError SceneSessionManagerLite::HasFloatingWindowForeground(const sptr<IRemoteObject>& abilityToken,
    bool& hasOrNot)
{
    return SceneSessionManager::GetInstance().HasFloatingWindowForeground(abilityToken,
        hasOrNot);
}
} // namespace OHOS::Rosen
