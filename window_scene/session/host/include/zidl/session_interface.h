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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_INTERFACE_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_INTERFACE_H

#include <iremote_broker.h>
#include <session_info.h>

#include "interfaces/include/ws_common.h"
#include "common/include/window_session_property.h"
#include "session/container/include/zidl/session_stage_interface.h"
#include "session/container/include/zidl/window_event_channel_interface.h"

namespace OHOS::Accessibility {
class AccessibilityEventInfo;
}
namespace OHOS::Rosen {
class RSSurfaceNode;
class ISession : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ISession");

    virtual WSError Connect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
        sptr<WindowSessionProperty> property = nullptr, sptr<IRemoteObject> token = nullptr,
        const std::string& identityToken = "") { return WSError::WS_OK; }
    virtual WSError Foreground(
        sptr<WindowSessionProperty> property, bool isFromClient = false, const std::string& identityToken = "") = 0;
    virtual WSError Background(bool isFromClient = false, const std::string& identityToken = "") = 0;
    virtual WSError Disconnect(bool isFromClient = false, const std::string& identityToken = "") = 0;
    virtual WSError Show(sptr<WindowSessionProperty> property) = 0;
    virtual WSError Hide() = 0;
    virtual WSError DrawingCompleted() = 0;
    virtual WSError RemoveStartingWindow() = 0;

    // scene session
    /**
     * @brief Receive session event from application.
     *
     * This function provides the ability for applications to move window.
     * This interface will take effect after touch down event.
     *
     * @param event Indicates the {@link SessionEvent}
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError OnSessionEvent(SessionEvent event) { return WSError::WS_OK; }

    /**
     * @brief Receive session event from application.
     *
     * This function provides the ability for applications to move window.\n
     * This interface will take effect after touch down event.\n
     *
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError SyncSessionEvent(SessionEvent event) { return WSError::WS_OK; }

    /**
     * @brief Enables or disables system window dragging.
     *
     * This function provides the ability for system application to make system window dragable.
     *
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     * @permission Make sure the caller has system permission.
     */
    virtual WMError SetSystemWindowEnableDrag(bool enableDrag) { return WMError::WM_OK; }

    /**
     * @brief Callback for processing full-screen layout changes.
     *
     * @param isLayoutFullScreen Indicates the {@link bool}
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError OnLayoutFullScreenChange(bool isLayoutFullScreen) { return WSError::WS_OK; }

    /**
     * @brief Callback for processing set default density enabled.
     *
     * @param isDefaultDensityEnabled Indicates the {@link bool}
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError OnDefaultDensityEnabled(bool isDefaultDensityEnabled) { return WSError::WS_OK; }

    /**
     * @brief Callback for processing title and dock hover show changes.
     *
     * @param isTitleHoverShown Indicates the {@link bool}
     * @param isDockHoverShown Indicates the {@link bool}
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError OnTitleAndDockHoverShowChange(bool isTitleHoverShown = true,
        bool isDockHoverShown = true) { return WSError::WS_OK; }

    /**
     * @brief Callback for processing restore main window.
     *
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError OnRestoreMainWindow() { return WSError::WS_OK; }

    /**
     * @brief Raise the application subwindow to the top layer of the application.
     *
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     * @permission Make sure the caller has system permission.
     */
    virtual WSError RaiseToAppTop() { return WSError::WS_OK; }

    /**
     * @brief Update window size and position.
     *
     * @param rect Indicates the {@link WSRect} structure containing required size and position.
     * @param reason Indicates the {@link SizeChangeReason} reason.
     * @param isGlobal Indicates the {@link bool}.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError UpdateSessionRect(
        const WSRect& rect, SizeChangeReason reason, bool isGlobal = false,
        bool isFromMoveToGlobal = false, const MoveConfiguration& moveConfiguration = {},
        const RectAnimationConfig& rectAnimationConfig = {}) { return WSError::WS_OK; }
    virtual WSError UpdateClientRect(const WSRect& rect) { return WSError::WS_OK; }
    virtual WMError GetGlobalScaledRect(Rect& globalScaledRect) { return WMError::WM_OK; }
    virtual WSError OnNeedAvoid(bool status) { return WSError::WS_OK; }
    virtual AvoidArea GetAvoidAreaByType(AvoidAreaType type, const WSRect& rect = WSRect::EMPTY_RECT,
        int32_t apiVersion = API_VERSION_INVALID) { return {}; }
    virtual WSError GetAllAvoidAreas(std::map<AvoidAreaType, AvoidArea>& avoidAreas) { return WSError::WS_OK; }
    virtual WSError RequestSessionBack(bool needMoveToBackground) { return WSError::WS_OK; }
    virtual WSError MarkProcessed(int32_t eventId) { return WSError::WS_OK; }

    /**
     * @brief Sets the global maximization mode of window.
     *
     * @param mode Indicates the {@link MaximizeMode}.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError SetGlobalMaximizeMode(MaximizeMode mode) { return WSError::WS_OK; }

    /**
     * @brief this interface is invoked by the ACE to the native host.
     * @param eventName invoking event name, which is used to distinguish different invoking types.
     * @param eventValue used to transfer parameters.
     * @return WM_OK means get success, others means get failed.
     */
    virtual WSError OnContainerModalEvent(const std::string& eventName,
        const std::string& eventValue) { return WSError::WS_OK; }

    /**
     * @brief Obtains the global maximization mode of window.
     *
     * @param mode Indicates the {@link MaximizeMode}.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError GetGlobalMaximizeMode(MaximizeMode& mode) { return WSError::WS_OK; }

    /**
     * @brief Sets the aspect ratio of window.
     *
     * @param ratio Indicates the {@link float}
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError SetAspectRatio(float ratio) { return WSError::WS_OK; }
    virtual WSError UpdateWindowAnimationFlag(bool needDefaultAnimationFlag) { return WSError::WS_OK; }
    virtual WSError UpdateWindowSceneAfterCustomAnimation(bool isAdd) { return WSError::WS_OK; }

    /**
     * @brief Raise a subwindow above a target subwindow.
     *
     * @param subWindowId Indicates the {@link int32_t} id of the target subwindow.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     * @permission Make sure the caller has system permission.
     */
    virtual WSError RaiseAboveTarget(int32_t subWindowId) { return WSError::WS_OK; }

    /**
     * @brief Raise the application main window to the top layer of the application.
     *
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError RaiseAppMainWindowToTop() { return WSError::WS_OK; }
    virtual WSError PendingSessionActivation(const sptr<AAFwk::SessionInfo> abilitySessionInfo)
        { return WSError::WS_OK; }
    virtual WSError TerminateSession(const sptr<AAFwk::SessionInfo> abilitySessionInfo) { return WSError::WS_OK; }
    virtual WSError SetLandscapeMultiWindow(bool isLandscapeMultiWindow) { return WSError::WS_OK; }
    virtual WSError GetIsMidScene(bool& isMidScene) { return WSError::WS_OK; }
    virtual WSError NotifySessionException(
        const sptr<AAFwk::SessionInfo> abilitySessionInfo,
        const ExceptionInfo& exceptionInfo) { return WSError::WS_OK; }

    // extension session
    virtual WSError TransferAbilityResult(uint32_t resultCode, const AAFwk::Want& want) { return WSError::WS_OK; }
    virtual WSError TransferExtensionData(const AAFwk::WantParams& wantParams) { return WSError::WS_OK; }
    virtual WSError TransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
        int64_t uiExtensionIdLevel)
    {
        return WSError::WS_OK;
    }
    virtual WSError NotifyFrameLayoutFinishFromApp(bool notifyListener, const WSRect& rect)
    {
        return WSError::WS_OK;
    }
    virtual void NotifyExtensionDied() {}
    virtual void NotifyExtensionTimeout(int32_t errorCode) {}
    virtual void TriggerBindModalUIExtension() {}
    virtual void NotifySyncOn() {}
    virtual void NotifyAsyncOn() {}
    virtual void NotifyTransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
        int64_t uiExtensionIdLevel) {}
    virtual void NotifyExtensionEventAsync(uint32_t notifyEvent) {}
    virtual WSError SendExtensionData(MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return WSError::WS_OK;
    }

    /**
     * @brief Close pip window while stopPip is called.
     *
     * Notify system that pip window is stopping and execute animation.
     */
    virtual void NotifyPiPWindowPrepareClose() {}

    /**
     * @brief Update the required params to system.
     *
     * Update the required rect and reason to determine the final size of pip window. Called when start pip,
     * show pip window, update pip size and pip restore.
     * Make sure the caller's process is same with the process which created pip window.
     *
     * @param rect Indicates the {@link Rect} structure containing required size and position.
     * @param reason Indicates the {@link SizeChangeReason} reason.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError UpdatePiPRect(const Rect& rect, SizeChangeReason reason) { return WSError::WS_OK; }

    /**
     * @brief Update the pip control status to pip control panel.
     *
     * Called when the specified component's status needs to be updated.
     * Make sure the caller's process is same with the process which created pip window.
     *
     * @param controlType Indicates the {@link WsPiPControlType} component in pip control panel.
     * @param status Indicates the {@link WsPiPControlStatus} status of specified component.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError UpdatePiPControlStatus(WsPiPControlType controlType, WsPiPControlStatus status)
    {
        return WSError::WS_OK;
    }

    /**
     * @brief Update the auto start pip window status.
     *
     * @param isAutoStart Indicates the {@link bool}
     * @param priority Indicates the {@link uint32_t} priority of pip window
     * @param width Indicates the {@link uint32_t} width of the video content
     * @param height Indicates the {@link uint32_t} height of the video content
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError SetAutoStartPiP(bool isAutoStart, uint32_t priority, uint32_t width, uint32_t height)
    {
        return WSError::WS_OK;
    }

    virtual WSError ProcessPointDownSession(int32_t posX, int32_t posY) { return WSError::WS_OK; }
    virtual WSError SendPointEventForMoveDrag(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        bool isExecuteDelayRaise = false) { return WSError::WS_OK; }
    virtual bool IsStartMoving() { return false; }
    virtual WSError ChangeSessionVisibilityWithStatusBar(const sptr<AAFwk::SessionInfo> abilitySessionInfo,
        bool isShow) { return WSError::WS_OK; }

    /**
     * @brief Instruct the application to update the listening flag for registering rect changes.
     *
     * @param isRegister Indicates the {@link bool}
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError UpdateRectChangeListenerRegistered(bool isRegister)
    {
        return WSError::WS_OK;
    }
    virtual void SetCallingSessionId(uint32_t callingSessionId) {};
    virtual void NotifyKeyboardDidShowRegistered(bool registered) { };
    virtual void NotifyKeyboardDidHideRegistered(bool registered) { };
    virtual void SetCustomDecorHeight(int32_t height) {};
    virtual WMError UpdateSessionPropertyByAction(const sptr<WindowSessionProperty>& property,
        WSPropertyChangeAction action) { return WMError::WM_OK; }
    virtual WMError GetAppForceLandscapeConfig(AppForceLandscapeConfig& config) { return WMError::WM_OK; }
    virtual WSError AdjustKeyboardLayout(const KeyboardLayoutParams& params) { return WSError::WS_OK; }
    virtual WSError SetDialogSessionBackGestureEnabled(bool isEnabled) { return WSError::WS_OK; }
    virtual void NotifyExtensionDetachToDisplay() {}
    virtual int32_t GetStatusBarHeight() { return 0; }
    /**
     * @brief Request to get focus or lose focus.
     *
     * @param isFocused True means window wants to get focus, false means the opposite.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError RequestFocus(bool isFocused) { return WSError::WS_OK; }

    /**
     * @brief Callback for session modal type changes.
     *
     * @param subWindowModalType Indicates the {@link SubWindowModalType}
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError NotifySubModalTypeChange(SubWindowModalType subWindowModalType) { return WSError::WS_OK; }

    /**
     * @brief Callback for main session modal type changes.
     *
     * @param isModal Indicates the {@link bool}
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError NotifyMainModalTypeChange(bool isModal) { return WSError::WS_OK; }

    /**
     * @brief Callback for setting whether the sub window supports simultaneous display on multiple screens
     *        when the parent window is dragged to move or dragged to zoom.
     * @param enabled Indicates the {@link bool}
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError NotifyFollowParentMultiScreenPolicy(bool enabled) { return WSError::WS_OK; }

    /**
     * @brief Callback for setting to automatically save the window rect.
     *
     * @param enabled Enable the window rect auto-save if true, otherwise means the opposite.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError OnSetWindowRectAutoSave(bool enabled, bool isSaveBySpecifiedFlag) { return WSError::WS_OK; }

    /**
     * @brief Callback for setting to radius of window.
     *
     * @param cornerRadius corner radius of window.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError SetWindowCornerRadius(float cornerRadius) { return WSError::WS_OK; }

    /**
     *  Gesture Back
     */
    virtual WMError SetGestureBackEnabled(bool isEnabled) { return WMError::WM_OK; }

    /**
     * @brief Callback for setting the window support modes.
     *
     * @param supportedWindowModes Indicates the {@link AppExecFwk::SupportWindowMode}.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError NotifySupportWindowModesChange(
        const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes) { return WSError::WS_OK; }

    /**
     * @brief set session label and icon
     *
     * @param label
     * @param icon
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     * @permission ohos.permission.SET_ABILITY_INSTANCE_INFO
     * @scene 15
     */
    virtual WSError SetSessionLabelAndIcon(const std::string& label,
        const std::shared_ptr<Media::PixelMap>& icon) { return WSError::WS_OK; }

    virtual WSError ChangeKeyboardViewMode(KeyboardViewMode mode) { return WSError::WS_OK; };

    /**
     * @brief Start Moving window with coordinate.
     *
     * @param offsetX expected pointer position x-axis offset in window when start moving.
     * @param offsetY expected pointer position y-axis offset in window when start moving.
     * @param pointerPosX current pointer position x-axis offset in screen.
     * @param pointerPosY current pointer position y-axis offset in screen.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError StartMovingWithCoordinate(int32_t offsetX, int32_t offsetY,
        int32_t pointerPosX, int32_t pointerPosY) { return WSError::WS_OK; }
    virtual WSError GetCrossAxisState(CrossAxisState& state) { return WSError::WS_OK; };

    /**
     * @brief Notify the window attach state listener is registered or not.
     *
     * @param registered true means register success.
     */
    virtual void NotifyWindowAttachStateListenerRegistered(bool registered) { }
    virtual WSError SetFollowParentWindowLayoutEnabled(bool isFollow) { return WSError::WS_OK; };
    virtual WSError UpdateFlag(const std::string& flag) { return WSError::WS_OK; };
    virtual WSError GetIsHighlighted(bool& isHighlighted) { return WSError::WS_OK; }
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_INTERFACE_H
