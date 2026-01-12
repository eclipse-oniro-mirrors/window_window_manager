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
    WMError RequestFocusStatusBySA(int32_t persistentId, bool isFocused = true,
        bool byForeground = true, FocusChangeReason reason = FocusChangeReason::SA_REQUEST) override;
    WSError RaiseWindowToTop(int32_t persistentId) override;

    /*
     * Sub Window
     */
    WMError SetParentWindow(int32_t subWindowId, int32_t newParentWindowId) override;

    WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError SetGestureNavigationEnabled(bool enable) override;
    void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId = DEFAULT_DISPLAY_ID) override;
    void GetFocusWindowInfoByAbilityToken(FocusChangeInfo& focusInfo,
        const sptr<IRemoteObject>& abilityToken) override;
    void GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                         std::vector<sptr<FocusChangeInfo>>& allFocusInfoList) override {};
    WSError SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label) override;
    WSError SetSessionIcon(const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon) override;
    WSError IsValidSessionIds(const std::vector<int32_t>& sessionIds, std::vector<bool>& results) override;
    WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) override;
    WMError ConvertToRelativeCoordinateExtended(const Rect& rect, Rect& newRect, DisplayId& newDisplayId) override;
    WMError GetUnreliableWindowInfo(int32_t windowId, std::vector<sptr<UnreliableWindowInfo>>& infos) override;
    WSError PendingSessionToForeground(const sptr<IRemoteObject>& token,
        int32_t windowMode = DEFAULT_INVALID_WINDOW_MODE) override;
    WSError PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
        bool shouldBackToCaller = true) override;
    WMError GetSessionSnapshotById(int32_t persistentId, SessionSnapshot& snapshot) override;
    WMError GetSnapshotByWindowId(int32_t persistentId, std::shared_ptr<Media::PixelMap>& pixelMap) override;
    WSError GetFocusSessionToken(sptr<IRemoteObject>& token, DisplayId displayId = DEFAULT_DISPLAY_ID) override;
    WSError GetFocusSessionElement(AppExecFwk::ElementName& element, DisplayId displayId = DEFAULT_DISPLAY_ID) override;
    WMError CheckWindowId(int32_t windowId, int32_t& pid) override;

    WSError RegisterSessionListener(const sptr<ISessionListener>& listener) override;
    WSError UnRegisterSessionListener(const sptr<ISessionListener>& listener) override;
    WSError GetSessionInfos(const std::string& deviceId, int32_t numMax,
                            std::vector<SessionInfoBean>& sessionInfos) override;
    WSError GetSessionInfo(const std::string& deviceId, int32_t persistentId, SessionInfoBean& sessionInfo) override;
    WSError GetSessionInfoByContinueSessionId(const std::string& continueSessionId,
        SessionInfoBean& sessionInfo) override;

    WSError DumpSessionAll(std::vector<std::string>& infos) override;
    WSError DumpSessionWithId(int32_t persistentId, std::vector<std::string>& infos) override;
    WSError SetSessionContinueState(const sptr<IRemoteObject>& token, const ContinueState& continueState) override;
    WSError TerminateSessionNew(
        const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker = false) override;
    WSError GetSessionDumpInfo(const std::vector<std::string>& params, std::string& info) override;
    void NotifyDumpInfoResult(const std::vector<std::string>& info) override;
    WSError UpdateSessionAvoidAreaListener(int32_t persistentId, bool haveListener) override;
    WSError UpdateSessionTouchOutsideListener(int32_t& persistentId, bool haveListener) override;
    WSError UpdateSessionWindowVisibilityListener(int32_t persistentId, bool haveListener) override;
    WMError UpdateSessionOcclusionStateListener(int32_t persistentId, bool haveListener) override;
    WSError GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
                               SessionSnapshot& snapshot, bool isLowResolution) override;
    WSError GetUIContentRemoteObj(int32_t persistentId, sptr<IRemoteObject>& uiContentRemoteObj) override;
    WMError GetRootUIContentRemoteObj(DisplayId displayId, sptr<IRemoteObject>& uiContentRemoteObj) override;
    WSError LockSession(int32_t persistentId) override;
    WSError UnlockSession(int32_t persistentId) override;
    WSError MoveSessionsToForeground(const std::vector<int32_t>& sessionIds, int32_t topSessionId) override;
    WSError MoveSessionsToBackground(const std::vector<int32_t>& sessionIds, std::vector<int32_t>& result) override;
    WSError ClearSession(int32_t persistentId) override;
    WSError ClearAllSessions() override;
    WSError RegisterIAbilityManagerCollaborator(int32_t type,
        const sptr<AAFwk::IAbilityManagerCollaborator>& impl) override;
    WSError UnregisterIAbilityManagerCollaborator(int32_t type) override;
    WSError NotifyWindowExtensionVisibilityChange(int32_t pid, int32_t uid, bool visible) override;
    WMError GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId) override;
    WMError NotifyWatchGestureConsumeResult(int32_t keyCode, bool isConsumed) override;
    WMError NotifyWatchFocusActiveChange(bool isActive) override;
    WMError GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId) override;
    WMError ListWindowInfo(const WindowInfoOption& windowInfoOption, std::vector<sptr<WindowInfo>>& infos) override;
    WMError GetAllWindowLayoutInfo(DisplayId displayId, std::vector<sptr<WindowLayoutInfo>>& infos) override;
    WMError GetAllMainWindowInfo(std::vector<sptr<MainWindowInfo>>& infos) override;
    WMError GetMainWindowSnapshot(const std::vector<int32_t>& windowIds, const WindowSnapshotConfiguration& config,
        const sptr<IRemoteObject>& callback) override;
    WMError GetGlobalWindowMode(DisplayId displayId, GlobalWindowMode& globalWinMode) override;
    WMError GetTopNavDestinationName(int32_t windowId, std::string& topNavDestName) override;
    WMError SetWatermarkImageForApp(const std::shared_ptr<Media::PixelMap>& pixelMap,
        std::string& watermarkName) override;
    WMError RecoverWatermarkImageForApp(const std::string& watermarkName) override;
    WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) override;
    WSError ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId) override;
    WSError SetSpecificWindowZIndex(WindowType windowType, int32_t zIndex) override;
    WSError ResetSpecificWindowZIndex(int32_t pid) override;
    void AddExtensionWindowStageToSCB(const sptr<ISessionStage>& sessionStage,
        const sptr<IRemoteObject>& token, uint64_t surfaceNodeId, int64_t startModalExtensionTimeStamp,
        bool isConstrainedModal = false) override;
    void RemoveExtensionWindowStageFromSCB(const sptr<ISessionStage>& sessionStage,
        const sptr<IRemoteObject>& token, bool isConstrainedModal = false) override;
    void UpdateModalExtensionRect(const sptr<IRemoteObject>& token, Rect rect) override;
    void ProcessModalExtensionPointDown(const sptr<IRemoteObject>& token, int32_t posX, int32_t posY) override;
    WSError AddOrRemoveSecureSession(int32_t persistentId, bool shouldHide) override;
    WSError UpdateExtWindowFlags(const sptr<IRemoteObject>& token, uint32_t extWindowFlags,
        uint32_t extWindowActions) override;
    WSError GetHostWindowRect(int32_t hostWindowId, Rect& rect) override;
    WSError GetHostGlobalScaledRect(int32_t hostWindowId, Rect& globalScaledRect) override;
    WSError GetFreeMultiWindowEnableState(bool& enable) override;
    WMError GetCallingWindowWindowStatus(uint32_t callingWindowId, WindowStatus& windowStatus) override;
    WMError GetCallingWindowRect(uint32_t callingWindowId, Rect& rect) override;
    WMError MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId = 0) override;
    WMError ToggleShownStateForAllAppWindows() override;
    WMError GetWindowModeType(WindowModeType& windowModeType) override;
    WMError GetWindowStyleType(WindowStyleType& windowStyleType) override;
    WMError GetProcessSurfaceNodeIdByPersistentId(const int32_t pid,
        const std::vector<int32_t>& persistentIds, std::vector<uint64_t>& surfaceNodeIds) override;
    WMError SkipSnapshotForAppProcess(int32_t pid, bool skip) override;
    WMError SkipSnapshotByUserIdAndBundleNames(int32_t userId,
        const std::vector<std::string>& bundleNameList) override;
    WMError SetProcessWatermark(int32_t pid, const std::string& watermarkName, bool isEnabled) override;
    WMError GetWindowIdsByCoordinate(DisplayId displayId, int32_t windowNumber,
        int32_t x, int32_t y, std::vector<int32_t>& windowIds) override;
    WMError UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease) override;
    WMError AddSkipSelfWhenShowOnVirtualScreenList(const std::vector<int32_t>& persistentIds) override;
    WMError RemoveSkipSelfWhenShowOnVirtualScreenList(const std::vector<int32_t>& persistentIds) override;
    WMError SetScreenPrivacyWindowTagSwitch(
        uint64_t screenId, const std::vector<std::string>& privacyWindowTags, bool enable) override;
    WMError IsPcWindow(bool& isPcWindow) override;
    WMError IsFreeMultiWindow(bool& isPcWindow) override;
    WMError IsPcOrPadFreeMultiWindowMode(bool& isPcOrPadFreeMultiWindowMode) override;
    WMError IsWindowRectAutoSave(const std::string& key, bool& enabled, int persistentId) override;
    WMError SetImageForRecent(uint32_t imgResourceId, ImageFit ImageFit, int32_t persistentId) override;
    WMError SetImageForRecentPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap, ImageFit ImageFit,
        int32_t persistentId) override;
    WMError RemoveImageForRecent(int32_t persistentId) override;
    WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap) override;
    WMError SetGlobalDragResizeType(DragResizeType dragResizeType) override;
    WMError GetGlobalDragResizeType(DragResizeType& dragResizeType) override;
    WMError SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType) override;
    WMError GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType) override;
    WMError SetAppKeyFramePolicy(const std::string& bundleName, const KeyFramePolicy& keyFramePolicy) override;
    WMError ShiftAppWindowPointerEvent(int32_t sourcePersistentId, int32_t targetPersistentId,
        int32_t fingerId) override;
    WMError NotifyScreenshotEvent(ScreenshotEventType type) override;
    WMError SetStartWindowBackgroundColor(
        const std::string& moduleName, const std::string& abilityName, uint32_t color, int32_t uid) override;
    WMError MinimizeByWindowId(const std::vector<int32_t>& windowIds) override;
    WMError SetForegroundWindowNum(uint32_t windowNum) override;
    WSError UseImplicitAnimation(int32_t hostWindowId, bool useImplicit) override;
    WMError RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError RecoverWindowPropertyChangeFlag(uint32_t observedFlags, uint32_t interestedFlags) override;
    WMError AnimateTo(int32_t windowId, const WindowAnimationProperty& animationProperty,
        const WindowAnimationOption& animationOption) override;
    WMError CreateUIEffectController(const sptr<IUIEffectControllerClient>& controllerClient,
        sptr<IUIEffectController>& controller, int32_t& controllerId) override;
    WMError AddSessionBlackList(const std::unordered_set<std::string>& bundleNames,
        const std::unordered_set<std::string>& privacyWindowTags) override;
    WMError RemoveSessionBlackList(const std::unordered_set<std::string>& bundleNames,
        const std::unordered_set<std::string>& privacyWindowTags) override;
    WMError GetPiPSettingSwitchStatus(bool& switchStatus) override;
    WMError GetIsPipEnabled(bool& isPipEnabled) override;
    WMError UpdateOutline(const sptr<IRemoteObject>& remoteObject, const OutlineParams& outlineParams) override;
    WMError NotifySupportRotationRegistered() override;

private:
    template<typename T>
    WSError GetParcelableInfos(MessageParcel& reply, std::vector<T>& parcelableInfos);
    static inline BrokerDelegator<SceneSessionManagerProxy> delegator_;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_PROXY_H
