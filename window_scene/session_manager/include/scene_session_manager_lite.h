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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SCENE_SESSION_MANAGER_LITE_H
#define OHOS_ROSEN_WINDOW_SCENE_SCENE_SESSION_MANAGER_LITE_H

#include <mutex>
#include <shared_mutex>
#include "session_manager/include/zidl/scene_session_manager_lite_stub.h"
#include "wm_single_instance.h"


namespace OHOS::Rosen {

class SceneSessionManagerLite : public SceneSessionManagerLiteStub {
WM_DECLARE_SINGLE_INSTANCE_BASE(SceneSessionManagerLite)
public:
    WSError SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label) override;
    WSError SetSessionIcon(const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon) override;
    WSError IsValidSessionIds(const std::vector<int32_t>& sessionIds, std::vector<bool>& results) override;
    WSError PendingSessionToForeground(const sptr<IRemoteObject>& token) override;
    WSError PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
        bool shouldBackToCaller = true) override;
    WSError GetFocusSessionToken(sptr<IRemoteObject>& token) override;
    WSError GetFocusSessionElement(AppExecFwk::ElementName& element) override;
    WSError RegisterSessionListener(const sptr<ISessionListener>& listener) override;
    WSError UnRegisterSessionListener(const sptr<ISessionListener>& listener) override;
    WSError GetSessionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<SessionInfoBean>& sessionInfos) override;
    WSError GetMainWindowStatesByPid(int32_t pid, std::vector<MainWindowState>& windowStates) override;
    WSError GetSessionInfo(const std::string& deviceId, int32_t persistentId, SessionInfoBean& sessionInfo) override;
    WSError GetSessionInfoByContinueSessionId(const std::string& continueSessionId,
        SessionInfoBean& sessionInfo) override;
    WSError TerminateSessionNew(
        const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker = false) override;
    WSError GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
                               SessionSnapshot& snapshot, bool isLowResolution) override;
    WSError SetSessionContinueState(const sptr<IRemoteObject>& token, const ContinueState& continueState) override;
    WSError ClearSession(int32_t persistentId) override;
    WSError ClearAllSessions() override;
    WSError LockSession(int32_t sessionId) override;
    WSError UnlockSession(int32_t sessionId) override;
    WSError MoveSessionsToForeground(const std::vector<int32_t>& sessionIds, int32_t topSessionId) override;
    WSError MoveSessionsToBackground(const std::vector<int32_t>& sessionIds, std::vector<int32_t>& result) override;

    void GetFocusWindowInfo(FocusChangeInfo& focusInfo) override;
    WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError CheckWindowId(int32_t windowId, int32_t &pid) override;
    WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) override;
    WSError UpdateWindowMode(int32_t persistentId, int32_t windowMode);
    WMError GetWindowModeType(WindowModeType& windowModeType) override;
    WSError RaiseWindowToTop(int32_t persistentId) override;
    WMError GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo) override;
    WMError GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos) override;
    WMError ClearMainSessions(const std::vector<int32_t>& persistentIds, std::vector<int32_t>& clearFailedIds) override;
    WSError RegisterIAbilityManagerCollaborator(int32_t type,
        const sptr<AAFwk::IAbilityManagerCollaborator>& impl) override;
    WSError UnregisterIAbilityManagerCollaborator(int32_t type) override;
    WMError GetWindowStyleType(WindowStyleType& windowStyletype) override;

protected:
    SceneSessionManagerLite() = default;
    virtual ~SceneSessionManagerLite() = default;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SCENE_SESSION_MANAGER_LITE_H
