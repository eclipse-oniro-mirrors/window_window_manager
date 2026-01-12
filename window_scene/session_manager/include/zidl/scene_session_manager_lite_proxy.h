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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_PROXY_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_PROXY_H

#include <iremote_proxy.h>

#include "session_manager/include/zidl/scene_session_manager_lite_interface.h"

namespace OHOS::Rosen {
class SceneSessionManagerLiteProxy : public IRemoteProxy<ISceneSessionManagerLite> {
public:
    explicit SceneSessionManagerLiteProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ISceneSessionManagerLite>(impl) {}
    virtual ~SceneSessionManagerLiteProxy() = default;

    WSError SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label) override;
    WSError SetSessionIcon(const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon) override;
    WSError SetSessionIconForThirdParty(
        const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon) override;
    WSError IsValidSessionIds(const std::vector<int32_t>& sessionIds, std::vector<bool>& results) override;
    WSError PendingSessionToForeground(const sptr<IRemoteObject>& token,
        int32_t windowMode = DEFAULT_INVALID_WINDOW_MODE) override;
    WSError PendingSessionToBackground(const sptr<IRemoteObject>& token, const BackgroundParams& params) override;
    WSError PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
        bool shouldBackToCaller = true) override;
    WSError GetFocusSessionToken(sptr<IRemoteObject>& token, DisplayId displayId = DEFAULT_DISPLAY_ID) override;
    WSError GetFocusSessionElement(AppExecFwk::ElementName& element, DisplayId displayId = DEFAULT_DISPLAY_ID) override;
    WSError IsFocusWindowParent(const sptr<IRemoteObject>& token, bool& isParent) override;
    WSError RegisterSessionListener(const sptr<ISessionListener>& listener, bool isRecover = false) override;
    WSError UnRegisterSessionListener(const sptr<ISessionListener>& listener) override;
    WSError GetSessionInfos(const std::string& deviceId, int32_t numMax,
                            std::vector<SessionInfoBean>& sessionInfos) override;
    /**
     * @brief Obtains main window state list by pid
     * @caller SA
     * @permission SA permission
     *
     * @param pid Target pid
     * @param windowStates Window state list
     */
    WSError GetMainWindowStatesByPid(int32_t pid, std::vector<MainWindowState>& windowStates) override;
    WSError GetSessionInfo(const std::string& deviceId, int32_t persistentId, SessionInfoBean& sessionInfo) override;
    WSError GetSessionInfoByContinueSessionId(const std::string& continueSessionId,
        SessionInfoBean& sessionInfo) override;
    WSError SetSessionContinueState(const sptr<IRemoteObject>& token, const ContinueState& continueState) override;
    WSError TerminateSessionNew(
        const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker = false) override;
    WSError GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
                               SessionSnapshot& snapshot, bool isLowResolution) override;
    WSError LockSession(int32_t persistentId) override;
    WSError UnlockSession(int32_t persistentId) override;
    WSError MoveSessionsToForeground(const std::vector<int32_t>& sessionIds, int32_t topSessionId) override;
    WSError MoveSessionsToBackground(const std::vector<int32_t>& sessionIds, std::vector<int32_t>& result) override;
    WSError ClearSession(int32_t persistentId) override;
    WSError ClearAllSessions() override;
    WMError GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId) override;
    WMError UpdateWindowModeByIdForUITest(int32_t windowId, int32_t updateMode) override;
    void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId = DEFAULT_DISPLAY_ID) override;
    void GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                         std::vector<sptr<FocusChangeInfo>>& allFocusInfoList) override;
    WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError CheckWindowId(int32_t windowId, int32_t& pid) override;
    WMError CheckUIExtensionCreation(int32_t windowId, uint32_t tokenId, const AppExecFwk::ElementName& element,
        AppExecFwk::ExtensionAbilityType extensionAbilityType, int32_t& pid) override;
    WMError SetGlobalDragResizeType(DragResizeType dragResizeType) override;
    WMError GetGlobalDragResizeType(DragResizeType& dragResizeType) override;
    WMError SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType) override;
    WMError GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType) override;
    WMError SetAppKeyFramePolicy(const std::string& bundleName, const KeyFramePolicy& keyFramePolicy) override;
    WMError ListWindowInfo(const WindowInfoOption& windowInfoOption, std::vector<sptr<WindowInfo>>& infos) override;
    WMError RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError RecoverWindowPropertyChangeFlag(uint32_t observedFlags, uint32_t interestedFlags) override;
    WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) override;
    WMError UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease) override;
    WMError GetWindowModeType(WindowModeType& windowModeType) override;
    WMError GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo) override;
    WMError UpdateAnimationSpeedWithPid(pid_t pid, float speed) override;
    WMError GetCallingWindowInfo(CallingWindowInfo& callingWindowInfo) override;
    WMError GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos) override;
    WMError GetMainWindowInfoByToken(const sptr<IRemoteObject>& abilityToken, MainWindowInfo& windowInfo) override;
    WMError ClearMainSessions(const std::vector<int32_t>& persistentIds, std::vector<int32_t>& clearFailedIds) override;
    WSError RaiseWindowToTop(int32_t persistentId) override;
    WSError RegisterIAbilityManagerCollaborator(int32_t type,
        const sptr<AAFwk::IAbilityManagerCollaborator>& impl) override;
    WSError UnregisterIAbilityManagerCollaborator(int32_t type) override;
    WMError GetWindowStyleType(WindowStyleType& windowModeType) override;
    WMError TerminateSessionByPersistentId(int32_t persistentId) override;
    WMError CloseTargetFloatWindow(const std::string& bundleName) override;
    WMError CloseTargetPiPWindow(const std::string& bundleName) override;
    WMError GetCurrentPiPWindowInfo(std::string& bundleName) override;
    WMError GetRootMainWindowId(int32_t persistentId, int32_t& hostWindowId) override;
    WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) override;
    WSError NotifyAppUseControlList(ControlAppType type, int32_t userId,
        const std::vector<AppUseControlInfo>& controlList) override;
    WMError MinimizeMainSession(const std::string& bundleName, int32_t appIndex, int32_t userId) override;
    WMError LockSessionByAbilityInfo(const AbilityInfoBase& abilityInfo, bool isLock) override;
    WMError HasFloatingWindowForeground(const sptr<IRemoteObject>& abilityToken,
        bool& hasOrNot) override;
    WMError RegisterSessionLifecycleListenerByIds(const sptr<ISessionLifecycleListener>& listener,
        const std::vector<int32_t>& persistentIdList) override;
    WMError RegisterSessionLifecycleListenerByBundles(const sptr<ISessionLifecycleListener>& listener,
        const std::vector<std::string>& bundleNameList) override;
    WMError UnregisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener) override;
    WSError GetRecentMainSessionInfoList(std::vector<RecentSessionInfo>& recentSessionInfoList) override;
    WMError GetRouterStackInfo(int32_t persistentId, const sptr<ISessionRouterStackListener>& listener) override;
    WSError PendingSessionToBackgroundByPersistentId(const int32_t persistentId,
        bool shouldBackToCaller = true) override;
    WMError CreateNewInstanceKey(const std::string& bundleName, std::string& instanceKey) override;
    WMError RemoveInstanceKey(const std::string& bundleName, const std::string& instanceKey) override;
    WMError TransferSessionToTargetScreen(const TransferSessionInfo& info) override;
    WMError SetPipEnableByScreenId(int32_t screenId, bool enabled) override;
    WMError UnsetPipEnableByScreenId(int32_t screenId) override;
    WMError RegisterPipChgListenerByScreenId(int32_t screenId, const sptr<IPipChangeListener>& listener) override;
    WMError UnregisterPipChgListenerByScreenId(int32_t screenId) override;
    WMError UpdateKioskAppList(const std::vector<std::string>& kioskAppList) override;
    WMError EnterKioskMode(const sptr<IRemoteObject>& token) override;
    WMError ExitKioskMode(const sptr<IRemoteObject>& token) override;
    WSError SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) override;
    WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap) override;
private:
    template<typename T>
    WSError GetParcelableInfos(MessageParcel& reply, std::vector<T>& parcelableInfos);
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_PROXY_H
