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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_PROXY_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_PROXY_H

#include <iremote_proxy.h>

#include "session_manager/include/zidl/scene_session_manager_interface.h"

namespace OHOS::Rosen {
class SceneSessionManagerProxy : public IRemoteProxy<ISceneSessionManager> {
public:
    explicit SceneSessionManagerProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ISceneSessionManager>(impl) {}
    virtual ~SceneSessionManagerProxy() = default;

    WSError CreateAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<WindowSessionProperty> property, int32_t& persistentId, sptr<ISession>& session,
        SystemSessionConfig& systemConfig, sptr<IRemoteObject> token = nullptr) override;
    WSError RecoverAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<WindowSessionProperty> property, sptr<ISession>& session, sptr<IRemoteObject> token = nullptr) override;
    WSError RecoverAndReconnectSceneSession(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<ISession>& session, sptr<WindowSessionProperty> property = nullptr,
        sptr<IRemoteObject> token = nullptr) override;
    WSError DestroyAndDisconnectSpecificSession(const int32_t persistentId) override;
    WSError DestroyAndDisconnectSpecificSessionWithDetachCallback(const int32_t persistentId,
        const sptr<IRemoteObject>& callback) override;
    WSError BindDialogSessionTarget(uint64_t persistentId, sptr<IRemoteObject> targetToken) override;
    WMError RequestFocusStatus(int32_t persistentId, bool isFocused, bool byForeground = true,
        FocusChangeReason reason = FocusChangeReason::DEFAULT) override;
    WSError RaiseWindowToTop(int32_t persistentId) override;

    WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError SetGestureNavigaionEnabled(bool enable) override;
    void GetFocusWindowInfo(FocusChangeInfo& focusInfo) override;
    WSError SetSessionLabel(const sptr<IRemoteObject> &token, const std::string& label) override;
    WSError SetSessionIcon(const sptr<IRemoteObject> &token, const std::shared_ptr<Media::PixelMap> &icon) override;
    WSError IsValidSessionIds(const std::vector<int32_t> &sessionIds, std::vector<bool> &results) override;
    WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) override;
    WMError GetUnreliableWindowInfo(int32_t windowId, std::vector<sptr<UnreliableWindowInfo>>& infos) override;
    WSError PendingSessionToForeground(const sptr<IRemoteObject> &token) override;
    WSError PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
        bool shouldBackToCaller = true) override;
    WMError GetSessionSnapshotById(int32_t persistentId, SessionSnapshot& snapshot) override;
    WMError GetSnapshotByWindowId(int32_t persistentId, std::shared_ptr<Media::PixelMap>& pixelMap) override;
    WSError GetFocusSessionToken(sptr<IRemoteObject> &token) override;
    WSError GetFocusSessionElement(AppExecFwk::ElementName& element) override;
    WMError CheckWindowId(int32_t windowId, int32_t& pid) override;

    WSError RegisterSessionListener(const sptr<ISessionListener>& listener) override;
    WSError UnRegisterSessionListener(const sptr<ISessionListener>& listener) override;
    WSError GetSessionInfos(const std::string& deviceId, int32_t numMax,
                            std::vector<SessionInfoBean>& sessionInfos) override;
    WSError GetSessionInfo(const std::string& deviceId, int32_t persistentId, SessionInfoBean& sessionInfo) override;
    WSError GetSessionInfoByContinueSessionId(const std::string& continueSessionId,
        SessionInfoBean& sessionInfo) override;

    WSError DumpSessionAll(std::vector<std::string> &infos) override;
    WSError DumpSessionWithId(int32_t persistentId, std::vector<std::string> &infos) override;
    WSError SetSessionContinueState(const sptr<IRemoteObject> &token, const ContinueState& continueState) override;
    WSError TerminateSessionNew(
        const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker = false) override;
    WSError GetSessionDumpInfo(const std::vector<std::string>& params, std::string& info) override;
    void NotifyDumpInfoResult(const std::vector<std::string>& info) override;
    WSError UpdateSessionAvoidAreaListener(int32_t& persistentId, bool haveListener) override;
    WSError UpdateSessionTouchOutsideListener(int32_t& persistentId, bool haveListener) override;
    WSError UpdateSessionWindowVisibilityListener(int32_t persistentId, bool haveListener) override;
    WSError GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
                               SessionSnapshot& snapshot, bool isLowResolution) override;
    WSError GetUIContentRemoteObj(int32_t persistentId, sptr<IRemoteObject>& uiContentRemoteObj) override;
    WSError LockSession(int32_t persistentId) override;
    WSError UnlockSession(int32_t persistentId) override;
    WSError MoveSessionsToForeground(const std::vector<int32_t>& sessionIds, int32_t topSessionId) override;
    WSError MoveSessionsToBackground(const std::vector<int32_t>& sessionIds, std::vector<int32_t>& result) override;
    WSError ClearSession(int32_t persistentId) override;
    WSError ClearAllSessions() override;
    WSError RegisterIAbilityManagerCollaborator(int32_t type,
        const sptr<AAFwk::IAbilityManagerCollaborator> &impl) override;
    WSError UnregisterIAbilityManagerCollaborator(int32_t type) override;
    WSError NotifyWindowExtensionVisibilityChange(int32_t pid, int32_t uid, bool visible) override;
    WMError GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId) override;
    WMError GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId) override;
    WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) override;
    WSError ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId) override;
    void AddExtensionWindowStageToSCB(const sptr<ISessionStage>& sessionStage,
        const sptr<IRemoteObject>& token, uint64_t surfaceNodeId) override;
    void RemoveExtensionWindowStageFromSCB(const sptr<ISessionStage>& sessionStage,
        const sptr<IRemoteObject>& token) override;
    void UpdateModalExtensionRect(const sptr<IRemoteObject>& token, Rect rect) override;
    void ProcessModalExtensionPointDown(const sptr<IRemoteObject>& token, int32_t posX, int32_t posY) override;
    WSError AddOrRemoveSecureSession(int32_t persistentId, bool shouldHide) override;
    WSError UpdateExtWindowFlags(const sptr<IRemoteObject>& token, uint32_t extWindowFlags,
        uint32_t extWindowActions) override;
    WSError GetHostWindowRect(int32_t hostWindowId, Rect& rect) override;
    WMError GetCallingWindowWindowStatus(int32_t persistentId, WindowStatus& windowStatus) override;
    WMError GetCallingWindowRect(int32_t persistentId, Rect& rect) override;
    WMError GetWindowModeType(WindowModeType& windowModeType) override;
    WMError GetWindowStyleType(WindowStyleType& windowStyleType) override;
    WMError GetProcessSurfaceNodeIdByPersistentId(const int32_t pid,
        const std::vector<int32_t>& persistentIds, std::vector<uint64_t>& surfaceNodeIds) override;

private:
    template<typename T>
    WSError GetParcelableInfos(MessageParcel& reply, std::vector<T>& parcelableInfos);
    static inline BrokerDelegator<SceneSessionManagerProxy> delegator_;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_PROXY_H
