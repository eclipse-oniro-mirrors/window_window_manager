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

WSError SceneSessionManagerLite::PendingSessionToForeground(const sptr<IRemoteObject>& token)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().PendingSessionToForeground(token);
}

WSError SceneSessionManagerLite::PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
    bool shouldBackToCaller)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().PendingSessionToBackgroundForDelegator(token, shouldBackToCaller);
}

WSError SceneSessionManagerLite::GetFocusSessionToken(sptr<IRemoteObject>& token)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().GetFocusSessionToken(token);
}

WSError SceneSessionManagerLite::GetFocusSessionElement(AppExecFwk::ElementName& element)
{
    WLOGFD("in");
    return SceneSessionManager::GetInstance().GetFocusSessionElement(element);
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
    const AppExecFwk::ElementName& element, int32_t& pid)
{
    return SceneSessionManager::GetInstance().CheckUIExtensionCreation(windowId, tokenId, element, pid);
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

WMError SceneSessionManagerLite::GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo)
{
    return SceneSessionManager::GetInstance().GetMainWindowInfos(topNum, topNInfo);
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
} // namespace OHOS::Rosen
