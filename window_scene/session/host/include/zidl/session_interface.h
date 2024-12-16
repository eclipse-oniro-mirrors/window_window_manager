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
    virtual WSError OnSessionEvent(SessionEvent event) { return WSError::WS_OK; }
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
        bool isDockHoverShown = true)
    {
        return WSError::WS_OK;
    }

    /**
     * @brief Callback for processing restore main window.
     *
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError OnRestoreMainWindow() { return WSError::WS_OK; }

    virtual WSError RaiseToAppTop() { return WSError::WS_OK; }
    virtual WSError UpdateSessionRect(
        const WSRect &rect, const SizeChangeReason reason, bool isGlobal = false, bool isFromMoveToGlobal = false)
    {
        return WSError::WS_OK;
    }
    virtual WSError UpdateClientRect(const WSRect& rect) { return WSError::WS_OK; }
    virtual WMError GetGlobalScaledRect(Rect& globalScaledRect) { return WMError::WM_OK; }
    virtual WSError OnNeedAvoid(bool status) { return WSError::WS_OK; }
    virtual AvoidArea GetAvoidAreaByType(AvoidAreaType type) { return {}; }
    virtual WSError GetAllAvoidAreas(std::map<AvoidAreaType, AvoidArea>& avoidAreas) { return WSError::WS_OK; }
    virtual WSError RequestSessionBack(bool needMoveToBackground) { return WSError::WS_OK; }
    virtual WSError MarkProcessed(int32_t eventId) { return WSError::WS_OK; }
    virtual WSError SetGlobalMaximizeMode(MaximizeMode mode) { return WSError::WS_OK; }
    virtual WSError GetGlobalMaximizeMode(MaximizeMode& mode) { return WSError::WS_OK; }
    virtual WSError SetAspectRatio(float ratio) { return WSError::WS_OK; }
    virtual WSError UpdateWindowAnimationFlag(bool needDefaultAnimationFlag) { return WSError::WS_OK; }
    virtual WSError UpdateWindowSceneAfterCustomAnimation(bool isAdd) { return WSError::WS_OK; }
    virtual WSError RaiseAboveTarget(int32_t subWindowId) { return WSError::WS_OK; }
    virtual WSError RaiseAppMainWindowToTop() { return WSError::WS_OK; }
    virtual WSError PendingSessionActivation(const sptr<AAFwk::SessionInfo> abilitySessionInfo)
        { return WSError::WS_OK; }
    virtual WSError TerminateSession(const sptr<AAFwk::SessionInfo> abilitySessionInfo) { return WSError::WS_OK; }
    virtual WSError SetLandscapeMultiWindow(bool isLandscapeMultiWindow) { return WSError::WS_OK; }
    virtual WSError NotifySessionException(
        const sptr<AAFwk::SessionInfo> abilitySessionInfo, bool needRemoveSession = false) { return WSError::WS_OK; }

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
     * show pip window, update pip size and pip restore.\n
     * Make sure the caller's process is same with the process which created pip window.\n
     *
     * @param rect Indicates the {@link Rect} structure containing required size and position.
     * @param reason Indicates the {@link SizeChangeReason} reason.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError UpdatePiPRect(const Rect& rect, SizeChangeReason reason) { return WSError::WS_OK; }

    /**
     * @brief Update the pip control status to pip control panel.
     *
     * Called when the specified component's status needs to be updated.\n
     * Make sure the caller's process is same with the process which created pip window.\n
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
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError SetAutoStartPiP(bool isAutoStart, uint32_t priority) { return WSError::WS_OK; }

    virtual WSError ProcessPointDownSession(int32_t posX, int32_t posY) { return WSError::WS_OK; }
    virtual WSError SendPointEventForMoveDrag(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
    {
        return WSError::WS_OK;
    }
    virtual bool IsStartMoving() { return false; }
    virtual WSError ChangeSessionVisibilityWithStatusBar(const sptr<AAFwk::SessionInfo> abilitySessionInfo,
        bool visible) { return WSError::WS_OK; }
    virtual WSError UpdateRectChangeListenerRegistered(bool isRegister)
    {
        return WSError::WS_OK;
    }
    virtual void SetCallingSessionId(uint32_t callingSessionId) {};
    virtual void SetCustomDecorHeight(int32_t height) {};
    virtual WMError UpdateSessionPropertyByAction(const sptr<WindowSessionProperty>& property,
        WSPropertyChangeAction action) { return WMError::WM_OK; }
    virtual WMError GetAppForceLandscapeConfig(AppForceLandscapeConfig& config) { return WMError::WM_OK; }
    virtual WSError AdjustKeyboardLayout(const KeyboardLayoutParams& params) { return WSError::WS_OK; }
    virtual WSError SetDialogSessionBackGestureEnabled(bool isEnabled) { return WSError::WS_OK; }

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

    /*
     *  Gesture Back
     */
    virtual WMError SetGestureBackEnabled(bool isEnabled) { return WMError::WM_OK; }
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_INTERFACE_H
