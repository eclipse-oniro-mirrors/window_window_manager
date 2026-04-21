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

#ifndef OHOS_ROSEN_WINDOW_SCENE_MAIN_SESSION_H
#define OHOS_ROSEN_WINDOW_SCENE_MAIN_SESSION_H

#include "session/host/include/scene_session.h"

namespace OHOS::Rosen {
class MainSession : public SceneSession {
public:
    MainSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback);
    ~MainSession();

    void OnFirstStrongRef(const void* objectId) override;

    WSError Reconnect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode, sptr<WindowSessionProperty> property = nullptr,
        sptr<IRemoteObject> token = nullptr, int32_t pid = -1, int32_t uid = -1) override;
    WSError ProcessPointDownSession(int32_t posX, int32_t posY) override;
    void NotifyForegroundInteractiveStatus(bool interactive) override;
    WSError TransferKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    void RectCheck(float curWidth, float curHeight, const ScreenMetrics& screenMetrics) override;

    /*
     * Window Hierarchy
     */
    WSError SetTopmost(bool topmost) override;
    bool IsTopmost() const override;
    WSError SetMainWindowTopmost(bool isTopmost) override;
    bool IsMainWindowTopmost() const override;

    void SetExitSplitOnBackground(bool isExitSplitOnBackground) override;
    bool IsExitSplitOnBackground() const override;

    /*
     * Window Focus
     */
    WSError UpdateFocus(bool isFocused) override;

    /*
     * PC Window
     */
    WSError OnTitleAndDockHoverShowChange(bool isTitleHoverShown = true,
        bool isDockHoverShown = true) override;
    WSError OnRestoreMainWindow() override;
    WSError OnSetWindowRectAutoSave(bool enabled, bool isSaveBySpecifiedFlag) override;
    bool IsModal() const override;
    bool IsApplicationModal() const override;
    WSError NotifyMainModalTypeChange(bool isModal) override;
    WSError NotifySupportWindowModesChange(
        const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes) override;
    WSError UpdateFlag(const std::string& flag) override;

    /*
     * Window LifeCycle
     */
    WSError SetSessionLabelAndIcon(const std::string& label, const std::shared_ptr<Media::PixelMap>& icon) override;
    void SetUpdateSessionLabelAndIconListener(NofitySessionLabelAndIconUpdatedFunc&& func) override;
    void RegisterSessionLockStateChangeCallback(NotifySessionLockStateChangeCallback&& callback) override;
    void NotifySessionLockStateChange(bool isLockedState) override;
    void SetSessionLockState(bool isLockedState);
    bool GetSessionLockState() const;
    void SetSceneSessionDestructNotificationFunc(NotifySceneSessionDestructFunc&& func) override;
    void SetIsUserRequestedExit(bool isUserRequestedExit) override;
    WMError GetRouterStackInfo(std::string& routerStackInfo) const override;
    void SetRecentSessionState(RecentSessionInfo& info, const SessionState& state) override;

    /*
     * Compatible Mode
     */
    WSError NotifyIsFullScreenInForceSplitMode(bool isFullScreen) override;
    void RegisterForceSplitFullScreenChangeCallback(ForceSplitFullScreenChangeCallback&& callback) override;
    bool IsFullScreenInForceSplit() override;
    void RegisterCompatibleModeChangeCallback(CompatibleModeChangeCallback&& callback) override;
    WSError NotifyCompatibleModeChange(CompatibleStyleMode mode) override;
    void RegisterPageEnableCallback(PageEnableCallback&& callback) override;
    void RegisterSetSelectModeCallback(SetSelectModeCallback&& callback) override;
    WSError NotifyPageEnable(const std::string& action, const std::string& message) override;
    WSError UpdateAppHookWindowInfo(const HookWindowInfo& hookWindowInfo) override;
    WSError UpdateHookWindowInfo(const HookWindowInfo& hookWindowInfo) override;
    WSError SetForceSplitEnable(bool isForceSplitEnabled, bool needUpdateViewport, SelectMode selectMode) override;
    WMError NotifySplitRatioChanged(float newRatio) override;

    /*
     * Window Pattern
     */
    void RecoverSnapshotPersistence(const SessionInfo& info) override;
    void ClearSnapshotPersistence() override;

    /*
     * Prelaunch check
     */
    void RemovePrelaunchStartingWindow() override;
    void SetPrelaunch() override;
    bool IsPrelaunch() const override;

    /*
     * Window Layout
     */
    /**
     * @brief Main window implementation: update own limits and propagate to all children
     *
     * Main window updates its own attached window limits and then requests all attached
     * child windows to update their limits as well. This ensures limits are propagated
     * throughout the window hierarchy.
     *
     * @param sourcePersistentId the persistentId of the window providing the limits
     * @param attachedWindowLimits the other window's limits
     * @param isIntersectedHeightLimit whether to limit height with attached window's limits
     * @param isIntersectedWidthLimit whether to limit width with attached window's limits
     * @return Returns WSError::WS_OK if success, otherwise failed.
     */
    WSError RequestUpdateAttachedWindowLimits(int32_t sourcePersistentId,
        const WindowLimits& attachedWindowLimits, bool isIntersectedHeightLimit = true,
        bool isIntersectedWidthLimit = true, int32_t excludePersistentId = INVALID_SESSION_ID) override;

    /**
     * @brief Main window implementation: remove own limits and propagate to all children
     *
     * Main window removes its own attached window limits and then requests all attached
     * child windows to remove their limits as well.
     *
     * @param sourcePersistentId the persistentId of the source window whose limits should be removed
     * @param excludePersistentId the persistentId of child window to exclude from notification
     * @return Returns WSError::WS_OK if success, otherwise failed.
     */
    WSError RequestRemoveAttachedWindowLimits(int32_t sourcePersistentId,
        int32_t excludePersistentId = INVALID_SESSION_ID) override;

protected:
    void UpdatePointerArea(const WSRect& rect) override;
    bool CheckPointerEventDispatch(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const override;
    void NotifyClientToUpdateInteractive(bool interactive) override;
    bool isClientInteractive_ = true;

private:
    /*
     * Window LifeCycle
     */
    WSError SetSessionLabelAndIconInner(const std::string& label, const std::shared_ptr<Media::PixelMap>& icon);
    NotifySessionLockStateChangeCallback onSessionLockStateChangeCallback_;
    bool isLockedState_ = false;
    NotifySceneSessionDestructFunc notifySceneSessionDestructFunc_;
    bool isUserRequestedExit_ = false;
    bool GetSessionBoundedSystemTray(int32_t callingPid, uint32_t callingToken, const std::string &instanceKey) const;

    /*
     * Window Layout
     */
    void NotifySubAndDialogFollowRectChange(const WSRect& rect, bool isGlobal, bool needFlush) override;
    void SetSubWindowBoundsDuringCross(const WSRect& parentRect, bool isGlobal, bool needFlush) override;
    void NotifySubSessionRectChangeByAnchor(const WSRect& parentRect,
        SizeChangeReason reason = SizeChangeReason::UNDEFINED, DisplayId displayId = DISPLAY_ID_INVALID) override;
    void HandleSubSessionSurfaceNodeByWindowAnchor(SizeChangeReason reason, DisplayId displayId) override;
    bool RestoreAspectRatio(float ratio);

    /*
     * Compatible Mode
     */
    ForceSplitFullScreenChangeCallback forceSplitFullScreenChangeCallback_;
    std::atomic_bool isFullScreenInForceSplit_ { false };
    CompatibleModeChangeCallback compatibleModeChangeCallback_;
    PageEnableCallback pageEnableCallback_;
    SetSelectModeCallback setSelectModeCallback_;

    /*
     * Prelaunch check
     */
    int64_t prelaunchStart_ = 0;
    bool prelaunchEnable_ = false;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_MAIN_SESSION_H
