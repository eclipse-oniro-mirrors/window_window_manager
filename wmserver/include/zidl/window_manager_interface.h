/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_WINDOW_MANAGER_INTERFACE_H
#define OHOS_WINDOW_MANAGER_INTERFACE_H

#include <iremote_broker.h>
#include <rs_iwindow_animation_finished_callback.h>

#include "common/include/window_session_property.h"
#include "interfaces/include/ws_common_inner.h"
#include "pixel_map.h"
#include "session/container/include/zidl/session_stage_interface.h"
#include "session/container/include/zidl/window_event_channel_interface.h"
#include "window_property.h"
#include "window_transition_info.h"
#include "mission_snapshot.h"
#include "zidl/window_interface.h"
#include "zidl/window_manager_agent_interface.h"
#include "interfaces/include/ws_common.h"

namespace OHOS {
namespace Rosen {
class RSIWindowAnimationController;
class RSSurfaceNode;

class IWindowManager : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.IWindowManager");

    enum class WindowManagerMessage : uint32_t {
        TRANS_ID_CREATE_WINDOW,
        TRANS_ID_ADD_WINDOW,
        TRANS_ID_REMOVE_WINDOW,
        TRANS_ID_DESTROY_WINDOW,
        TRANS_ID_REQUEST_FOCUS,
        TRANS_ID_REGISTER_FOCUS_CHANGED_LISTENER,
        TRANS_ID_UNREGISTER_FOCUS_CHANGED_LISTENER,
        TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT,
        TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT,
        TRANS_ID_GET_AVOID_AREA,
        TRANS_ID_GET_TOP_WINDOW_ID,
        TRANS_ID_PROCESS_POINT_DOWN,
        TRANS_ID_PROCESS_POINT_UP,
        TRANS_ID_MINIMIZE_ALL_APP_WINDOWS,
        TRANS_ID_TOGGLE_SHOWN_STATE_FOR_ALL_APP_WINDOWS,
        TRANS_ID_SET_BACKGROUND_BLUR,
        TRANS_ID_SET_ALPHA,
        TRANS_ID_UPDATE_LAYOUT_MODE,
        TRANS_ID_UPDATE_PROPERTY,
        TRANS_ID_GET_ACCESSIBILITY_WINDOW_INFO_ID,
        TRANS_ID_GET_WINDOW_LAYOUT_INFO,
        TRANS_ID_GET_VISIBILITY_WINDOW_INFO_ID,
        TRANS_ID_ANIMATION_SET_CONTROLLER,
        TRANS_ID_GET_SYSTEM_CONFIG,
        TRANS_ID_NOTIFY_WINDOW_TRANSITION,
        TRANS_ID_GET_FULLSCREEN_AND_SPLIT_HOT_ZONE,
        TRANS_ID_GET_ANIMATION_CALLBACK,
        TRANS_ID_UPDATE_AVOIDAREA_LISTENER,
        TRANS_ID_UPDATE_RS_TREE,
        TRANS_ID_BIND_DIALOG_TARGET,
        TRANS_ID_NOTIFY_READY_MOVE_OR_DRAG,
        TRANS_ID_SET_ANCHOR_AND_SCALE,
        TRANS_ID_SET_ANCHOR_OFFSET,
        TRANS_ID_OFF_WINDOW_ZOOM,
        TRANS_ID_RAISE_WINDOW_Z_ORDER,
        TRANS_ID_GET_SNAPSHOT,
        TRANS_ID_GESTURE_NAVIGATION_ENABLED,
        TRANS_ID_SET_WINDOW_GRAVITY,
        TRANS_ID_DISPATCH_KEY_EVENT,
        TRANS_ID_NOTIFY_DUMP_INFO_RESULT,
        TRANS_ID_GET_WINDOW_ANIMATION_TARGETS,
        TRANS_ID_SET_MAXIMIZE_MODE,
        TRANS_ID_GET_MAXIMIZE_MODE,
        TRANS_ID_GET_FOCUS_WINDOW_INFO,
        TRANS_ID_ADD_EXTENSION_WINDOW_STAGE_TO_SCB,
        TRANS_ID_UPDATE_MODALEXTENSION_RECT_TO_SCB,
        TRANS_ID_PROCESS_MODALEXTENSION_POINTDOWN_TO_SCB,
        TRANS_ID_UPDATE_EXTENSION_WINDOW_FLAGS,
        TRANS_ID_GET_HOST_WINDOW_RECT,
        TRANS_ID_GET_UNRELIABLE_WINDOW_INFO_ID,
        TRANS_ID_GET_FREE_MULTI_WINDOW_ENABLE_STATE,
    };
    virtual WMError CreateWindow(sptr<IWindow>& window, sptr<WindowProperty>& property,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        uint32_t& windowId, sptr<IRemoteObject> token) = 0;
    virtual WMError AddWindow(sptr<WindowProperty>& property) = 0;
    virtual WMError RemoveWindow(uint32_t windowId, bool isFromInnerkits) = 0;
    virtual WMError DestroyWindow(uint32_t windowId, bool onlySelf = false) = 0;
    virtual WMError RequestFocus(uint32_t windowId) = 0;
    virtual AvoidArea GetAvoidAreaByType(uint32_t windowId, AvoidAreaType type) = 0;
    virtual WMError GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId) = 0;
    virtual void NotifyServerReadyToMoveOrDrag(uint32_t windowId, sptr<WindowProperty>& windowProperty,
        sptr<MoveDragProperty>& moveDragProperty) = 0;
    virtual void ProcessPointDown(uint32_t windowId, bool isPointDown) = 0;
    virtual void ProcessPointUp(uint32_t windowId) = 0;
    virtual WMError MinimizeAllAppWindows(DisplayId displayId) = 0;
    virtual WMError ToggleShownStateForAllAppWindows() = 0;
    virtual WMError SetWindowLayoutMode(WindowLayoutMode mode) = 0;
    virtual WMError UpdateProperty(sptr<WindowProperty>& windowProperty, PropertyChangeAction action,
        bool isAsyncTask = false) = 0;
    virtual WMError SetWindowGravity(uint32_t windowId, WindowGravity gravity, uint32_t percent) = 0;
    virtual WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) = 0;
    virtual WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) = 0;
    virtual WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) = 0;
    virtual WMError GetUnreliableWindowInfo(int32_t windowId, std::vector<sptr<UnreliableWindowInfo>>& infos) = 0;
    virtual WMError GetAllWindowLayoutInfo(DisplayId displayId,
        std::vector<sptr<WindowLayoutInfo>>& infos) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) = 0;
    virtual WMError SetWindowAnimationController(const sptr<RSIWindowAnimationController>& controller) = 0;
    virtual WMError GetSystemConfig(SystemConfig& systemConfig) = 0;
    virtual WMError NotifyWindowTransition(sptr<WindowTransitionInfo>& from, sptr<WindowTransitionInfo>& to,
        bool isFromClient = false) = 0;
    virtual WMError GetModeChangeHotZones(DisplayId displayId, ModeChangeHotZones& hotZones) = 0;
    virtual void MinimizeWindowsByLauncher(std::vector<uint32_t> windowIds, bool isAnimated,
        sptr<RSIWindowAnimationFinishedCallback>& finishCallback) = 0;
    virtual WMError UpdateAvoidAreaListener(uint32_t windowId, bool haveListener) = 0;
    virtual WMError UpdateRsTree(uint32_t windowId, bool isAdd) = 0;
    virtual WMError BindDialogTarget(uint32_t& windowId, sptr<IRemoteObject> targetToken) = 0;
    virtual void SetAnchorAndScale(int32_t x, int32_t y, float scale) = 0;
    virtual void SetAnchorOffset(int32_t deltaX, int32_t deltaY) = 0;
    virtual void OffWindowZoom() = 0;
    virtual WMError RaiseToAppTop(uint32_t windowId) = 0;
    virtual std::shared_ptr<Media::PixelMap> GetSnapshot(int32_t windowId) = 0;
    virtual WMError SetGestureNavigaionEnabled(bool enable) = 0;
    virtual void DispatchKeyEvent(uint32_t windowId, std::shared_ptr<MMI::KeyEvent> event) = 0;
    virtual void NotifyDumpInfoResult(const std::vector<std::string>& info) {};
    virtual WSError DumpSessionAll(std::vector<std::string> &infos) { return WSError::WS_OK; }
    virtual WSError DumpSessionWithId(int32_t persistentId, std::vector<std::string> &infos) { return WSError::WS_OK; }
    virtual WSError GetUIContentRemoteObj(int32_t persistentId, sptr<IRemoteObject>& uiContentRemoteObj)
    {
        return WSError::WS_OK;
    }
    virtual WMError GetWindowAnimationTargets(std::vector<uint32_t> missionIds,
        std::vector<sptr<RSWindowAnimationTarget>>& targets) = 0;
    virtual void SetMaximizeMode(MaximizeMode maximizeMode) = 0;
    virtual MaximizeMode GetMaximizeMode() = 0;
    virtual void GetFocusWindowInfo(FocusChangeInfo& focusInfo) = 0;
    virtual WMError CheckWindowId(int32_t windowId, int32_t& pid) { return WMError::WM_OK; }
    virtual WSError UpdateSessionAvoidAreaListener(int32_t& persistentId, bool haveListener) { return WSError::WS_OK; }
    virtual WSError UpdateSessionTouchOutsideListener(int32_t& persistentId, bool haveListener)
    {
        return WSError::WS_OK;
    }
    virtual WSError NotifyWindowExtensionVisibilityChange(int32_t pid, int32_t uid, bool visible)
    {
        return WSError::WS_OK;
    }
    virtual WSError RaiseWindowToTop(int32_t persistentId) { return WSError::WS_OK; }
    virtual WSError UpdateSessionWindowVisibilityListener(int32_t persistentId, bool haveListener)
    {
        return WSError::WS_OK;
    }
    virtual WSError ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId)
    {
        return WSError::WS_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WSError CreateAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<WindowSessionProperty> property, int32_t& persistentId, sptr<ISession>& session,
        SystemSessionConfig& systemConfig, sptr<IRemoteObject> token = nullptr) { return WSError::WS_OK; }
    virtual WSError RecoverAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<WindowSessionProperty> property, sptr<ISession>& session, sptr<IRemoteObject> token = nullptr)
    {
        return WSError::WS_OK;
    }
    virtual WSError DestroyAndDisconnectSpecificSession(const int32_t persistentId) { return WSError::WS_OK; }
    virtual WSError DestroyAndDisconnectSpecificSessionWithDetachCallback(const int32_t persistentId,
        const sptr<IRemoteObject>& callback) { return WSError::WS_OK; }
    virtual WSError RecoverAndReconnectSceneSession(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<ISession>& session, sptr<WindowSessionProperty> property, sptr<IRemoteObject> token = nullptr)
    {
        return WSError::WS_OK;
    }
    virtual WSError BindDialogSessionTarget(uint64_t persistentId, sptr<IRemoteObject> targetToken)
    {
        return WSError::WS_OK;
    }
    virtual WSError SetSessionGravity(int32_t persistentId, SessionGravity gravity, uint32_t percent)
    {
        return WSError::WS_OK;
    }
    virtual WMError RequestFocusStatus(int32_t persistentId, bool isFocused, bool byForeground = true,
        FocusChangeReason reason = FocusChangeReason::DEFAULT)
    {
        return WMError::WM_OK;
    }
    virtual WMError GetSnapshotByWindowId(int32_t persistentId, std::shared_ptr<Media::PixelMap>& pixelMap)
    {
        return WMError::WM_OK;
    }
    virtual void AddExtensionWindowStageToSCB(const sptr<ISessionStage>& sessionStage,
        const sptr<IRemoteObject>& token, uint64_t surfaceNodeId, bool isConstrainedModal = false) {}
    virtual void RemoveExtensionWindowStageFromSCB(const sptr<ISessionStage>& sessionStage,
        const sptr<IRemoteObject>& token, bool isConstrainedModal = false) {}
    virtual void UpdateModalExtensionRect(const sptr<IRemoteObject>& token, Rect rect) {}
    virtual void ProcessModalExtensionPointDown(const sptr<IRemoteObject>& token, int32_t posX, int32_t posY) {}
    virtual WSError AddOrRemoveSecureSession(int32_t persistentId, bool shouldHide)
    {
        return WSError::WS_OK;
    }
    virtual WSError UpdateExtWindowFlags(const sptr<IRemoteObject>& token, uint32_t extWindowFlags,
        uint32_t extWindowActions)
    {
        return WSError::WS_OK;
    }
    virtual WSError GetHostWindowRect(int32_t hostWindowId, Rect& rect)
    {
        return WSError::WS_OK;
    }
    virtual WSError GetFreeMultiWindowEnableState(bool& enable)
    {
        return WSError::WS_OK;
    }
    virtual WMError GetCallingWindowWindowStatus(int32_t persistentId, WindowStatus& windowStatus)
    {
        return WMError::WM_OK;
    }
    virtual WMError GetCallingWindowRect(int32_t persistentId, Rect& rect)
    {
        return WMError::WM_OK;
    }
    virtual WMError GetWindowModeType(WindowModeType& windowModeType)
    {
        return WMError::WM_OK;
    };
    virtual WMError GetWindowStyleType(WindowStyleType& windowStyleType)
    {
        return WMError::WM_OK;
    };
    virtual WMError IsWindowRectAutoSave(const std::string& key, bool& enabled) { return WMError::WM_OK; }
    virtual WMError GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId) { return WMError::WM_OK; }
    virtual WMError GetWindowIdsByCoordinate(DisplayId displayId, int32_t windowNumber,
        int32_t x, int32_t y, std::vector<int32_t>& windowIds) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError ReleaseForegroundSessionScreenLock() { return WMError::WM_OK; }
    virtual WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap) { return WMError::WM_OK; }
    virtual WMError IsPcWindow(bool& isPcWindow) { return WMError::WM_OK; }
    virtual WMError IsPcOrPadFreeMultiWindowMode(bool& isPcOrPadFreeMultiWindowMode) { return WMError::WM_OK; }
    virtual WMError SetGlobalDragResizeType(DragResizeType dragResizeType) { return WMError::WM_OK; }
    virtual WMError GetGlobalDragResizeType(DragResizeType& dragResizeType) { return WMError::WM_OK; }
    virtual WMError SetAppDragResizeType(const std::string& bundleName,
        DragResizeType dragResizeType) { return WMError::WM_OK; }
    virtual WMError GetAppDragResizeType(const std::string& bundleName,
        DragResizeType& dragResizeType) { return WMError::WM_OK; }
    virtual WMError HasFloatingWindowForeground(const sptr<IRemoteObject>& abilityToken,
        bool& hasOrNot) { return WMError::WM_OK; }
};
}
}
#endif // OHOS_WINDOW_MANAGER_INTERFACE_H
