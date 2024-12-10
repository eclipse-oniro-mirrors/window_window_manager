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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_INTERFACE_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_INTERFACE_H

#include "common/include/window_session_property.h"
#include <iremote_broker.h>
#include "interfaces/include/ws_common.h"
#include "interfaces/include/ws_common_inner.h"
#include "session/container/include/zidl/session_stage_interface.h"
#include "session/container/include/zidl/window_event_channel_interface.h"
#include "session/host/include/session.h"
#include "focus_change_info.h"
#include "window_manager.h"
#include "zidl/window_manager_interface.h"
#include "session_info.h"
#include "mission_listener_interface.h"
#include "mission_info.h"
#include "mission_snapshot.h"
#include "iability_manager_collaborator.h"

namespace OHOS::Media {
class PixelMap;
} // namespace OHOS::Media

namespace OHOS::Rosen {
class RSSurfaceNode;
using ISessionListener = AAFwk::IMissionListener;
using SessionInfoBean = AAFwk::MissionInfo;
using SessionSnapshot = AAFwk::MissionSnapshot;
class ISceneSessionManager : public IWindowManager {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ISceneSessionManager");

    enum class SceneSessionManagerMessage : uint32_t {
        TRANS_ID_CREATE_AND_CONNECT_SPECIFIC_SESSION,
        TRANS_ID_DESTROY_AND_DISCONNECT_SPECIFIC_SESSION,
        TRANS_ID_UPDATE_PROPERTY,
        TRANS_ID_REQUEST_FOCUS,
        TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT,
        TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT,
        TRANS_ID_BIND_DIALOG_TARGET,
        TRANS_ID_GET_FOCUS_SESSION_INFO,
        TRANS_ID_SET_SESSION_LABEL,
        TRANS_ID_SET_SESSION_ICON,
        TRANS_ID_IS_VALID_SESSION_IDS,
        TRANS_ID_SET_GESTURE_NAVIGATION_ENABLED,
        TRANS_ID_GET_WINDOW_INFO,
        TRANS_ID_PENDING_SESSION_TO_FOREGROUND,
        TRANS_ID_PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR,
        TRANS_ID_GET_FOCUS_SESSION_TOKEN,
        TRANS_ID_GET_FOCUS_SESSION_ELEMENT,
        TRANS_ID_CHECK_WINDOW_ID,
        TRANS_ID_REGISTER_SESSION_LISTENER,
        TRANS_ID_UNREGISTER_SESSION_LISTENER,
        TRANS_ID_GET_MISSION_INFOS,
        TRANS_ID_GET_MISSION_INFO_BY_ID,
        TRANS_ID_GET_SESSION_INFO_BY_CONTINUE_SESSION_ID,
        TRANS_ID_DUMP_SESSION_ALL,
        TRANS_ID_DUMP_SESSION_WITH_ID,
        TRANS_ID_TERMINATE_SESSION_NEW,
        TRANS_ID_GET_SESSION_DUMP_INFO,
        TRANS_ID_UPDATE_AVOIDAREA_LISTENER,
        TRANS_ID_GET_SESSION_SNAPSHOT,
        TRANS_ID_GET_SESSION_SNAPSHOT_BY_ID,
        TRANS_ID_SET_SESSION_CONTINUE_STATE,
        TRANS_ID_NOTIFY_DUMP_INFO_RESULT,
        TRANS_ID_CLEAR_SESSION,
        TRANS_ID_CLEAR_ALL_SESSIONS,
        TRANS_ID_LOCK_SESSION,
        TRANS_ID_UNLOCK_SESSION,
        TRANS_ID_MOVE_MISSIONS_TO_FOREGROUND,
        TRANS_ID_MOVE_MISSIONS_TO_BACKGROUND,
        TRANS_ID_REGISTER_COLLABORATOR,
        TRANS_ID_UNREGISTER_COLLABORATOR,
        TRANS_ID_UPDATE_TOUCHOUTSIDE_LISTENER,
        TRANS_ID_RAISE_WINDOW_TO_TOP,
        TRANS_ID_NOTIFY_WINDOW_EXTENSION_VISIBILITY_CHANGE,
        TRANS_ID_RECOVER_AND_RECONNECT_SCENE_SESSION,
		TRANS_ID_RECOVER_AND_CONNECT_SPECIFIC_SESSION,
        TRANS_ID_GET_TOP_WINDOW_ID,
        TRANS_ID_GET_PARENT_MAIN_WINDOW_ID,
        TRANS_ID_GET_UI_CONTENT_REMOTE_OBJ,
        TRANS_ID_UPDATE_WINDOW_VISIBILITY_LISTENER,
        TRANS_ID_SHIFT_APP_WINDOW_FOCUS,
        TRANS_ID_GET_VISIBILITY_WINDOW_INFO_ID,
        TRANS_ID_ADD_EXTENSION_WINDOW_STAGE_TO_SCB,
        TRANS_ID_REMOVE_EXTENSION_WINDOW_STAGE_FROM_SCB,
        TRANS_ID_UPDATE_MODALEXTENSION_RECT_TO_SCB,
        TRANS_ID_PROCESS_MODALEXTENSION_POINTDOWN_TO_SCB,
        TRANS_ID_ADD_OR_REMOVE_SECURE_SESSION,
        TRANS_ID_UPDATE_EXTENSION_WINDOW_FLAGS,
        TRANS_ID_GET_HOST_WINDOW_RECT,
        TRANS_ID_DESTROY_AND_DISCONNECT_SPECIFIC_SESSION_WITH_DETACH_CALLBACK,
        TRANS_ID_GET_WINDOW_STATUS,
        TRANS_ID_GET_WINDOW_RECT,
        TRANS_ID_GET_WINDOW_MODE_TYPE,
        TRANS_ID_GET_UNRELIABLE_WINDOW_INFO,
        TRANS_ID_GET_FREE_MULTI_WINDOW_ENABLE_STATE,
        TRANS_ID_GET_WINDOW_STYLE_TYPE,
        TRANS_ID_GET_PROCESS_SURFACENODEID_BY_PERSISTENTID,
        TRANS_ID_RELEASE_SESSION_SCREEN_LOCK,
        TRANS_ID_GET_DISPLAYID_BY_WINDOWID,
        TRANS_ID_IS_WINDOW_RECT_AUTO_SAVE,
    };

    virtual WSError SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label) = 0;
    virtual WSError SetSessionIcon(const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon) = 0;
    virtual WSError IsValidSessionIds(const std::vector<int32_t>& sessionIds, std::vector<bool>& results) = 0;
    virtual WSError PendingSessionToForeground(const sptr<IRemoteObject>& token) = 0;
    virtual WSError PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
        bool shouldBackToCaller = true) = 0;
    virtual WSError GetFocusSessionToken(sptr<IRemoteObject>& token) = 0;
    virtual WSError GetFocusSessionElement(AppExecFwk::ElementName& element) = 0;

    virtual WSError RegisterSessionListener(const sptr<ISessionListener>& listener) = 0;
    virtual WSError UnRegisterSessionListener(const sptr<ISessionListener>& listener) = 0;
    virtual WSError GetSessionInfos(const std::string& deviceId,
                                    int32_t numMax, std::vector<SessionInfoBean>& sessionInfos) = 0;
    virtual WSError GetSessionInfo(const std::string& deviceId, int32_t persistentId, SessionInfoBean& sessionInfo) = 0;
    virtual WSError GetSessionInfoByContinueSessionId(const std::string& continueSessionId,
        SessionInfoBean& sessionInfo) = 0;
    virtual WSError DumpSessionAll(std::vector<std::string>& infos) override { return WSError::WS_OK; }
    virtual WSError DumpSessionWithId(int32_t persistentId, std::vector<std::string>& infos) override
    {
        return WSError::WS_OK;
    }
    virtual WSError SetSessionContinueState(const sptr<IRemoteObject>& token, const ContinueState& continueState) = 0;

    virtual WSError TerminateSessionNew(
        const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker = false) = 0;
    virtual WSError GetSessionDumpInfo(const std::vector<std::string>& params, std::string& info) = 0;
    virtual WSError GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
                                       SessionSnapshot& snapshot, bool isLowResolution) = 0;
    virtual WSError ClearSession(int32_t persistentId) = 0;
    virtual WSError ClearAllSessions() = 0;
    virtual WSError LockSession(int32_t sessionId) = 0;
    virtual WSError UnlockSession(int32_t sessionId) = 0;
    virtual WSError MoveSessionsToForeground(const std::vector<std::int32_t>& sessionIds, int32_t topSessionId) = 0;
    virtual WSError MoveSessionsToBackground(const std::vector<std::int32_t>& sessionIds,
        std::vector<std::int32_t>& result) = 0;
    virtual WMError GetSessionSnapshotById(int32_t persistentId, SessionSnapshot& snapshot)
    {
        return WMError::WM_OK;
    }
    virtual WSError NotifyWindowExtensionVisibilityChange(int32_t pid, int32_t uid, bool visible) override
    {
        return WSError::WS_OK;
    }

    virtual WSError RegisterIAbilityManagerCollaborator(int32_t type,
        const sptr<AAFwk::IAbilityManagerCollaborator>& impl) = 0;
    virtual WSError UnregisterIAbilityManagerCollaborator(int32_t type) = 0;
    // interfaces of IWindowManager
    WMError CreateWindow(sptr<IWindow>& window, sptr<WindowProperty>& property,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        uint32_t& windowId, sptr<IRemoteObject> token) override { return WMError::WM_OK; }
    WMError AddWindow(sptr<WindowProperty>& property) override { return WMError::WM_OK; }
    WMError RemoveWindow(uint32_t windowId, bool isFromInnerkits) override { return WMError::WM_OK; }
    WMError DestroyWindow(uint32_t windowId, bool onlySelf = false) override { return WMError::WM_OK; }
    WMError RequestFocus(uint32_t windowId) override { return WMError::WM_OK; }
    AvoidArea GetAvoidAreaByType(uint32_t windowId, AvoidAreaType type) override { return {}; }
    WMError GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId) override { return WMError::WM_OK; }
    // only main window,sub window and dialog window can use
    WMError GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId) override { return WMError::WM_OK; }
    void NotifyServerReadyToMoveOrDrag(uint32_t windowId, sptr<WindowProperty>& windowProperty,
        sptr<MoveDragProperty>& moveDragProperty) override {}
    void ProcessPointDown(uint32_t windowId, bool isPointDown) override {}
    void ProcessPointUp(uint32_t windowId) override {}
    WMError MinimizeAllAppWindows(DisplayId displayId) override { return WMError::WM_OK; }
    WMError ToggleShownStateForAllAppWindows() override { return WMError::WM_OK; }
    WMError SetWindowLayoutMode(WindowLayoutMode mode) override { return WMError::WM_OK; }
    WMError UpdateProperty(sptr<WindowProperty>& windowProperty, PropertyChangeAction action,
        bool isAsyncTask = false) override { return WMError::WM_OK; }
    WMError SetWindowGravity(uint32_t windowId, WindowGravity gravity, uint32_t percent) override
    {
        return WMError::WM_OK;
    }
    WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override { return WMError::WM_OK; }
    WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override { return WMError::WM_OK; }
    WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) override
    {
        return WMError::WM_OK;
    }
    WMError GetUnreliableWindowInfo(int32_t windowId, std::vector<sptr<UnreliableWindowInfo>>& infos) override
    {
        return WMError::WM_OK;
    }
    WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) override { return WMError::WM_OK; }
    WMError SetWindowAnimationController(const sptr<RSIWindowAnimationController>& controller) override
    {
        return WMError::WM_OK;
    }
    WMError GetSystemConfig(SystemConfig& systemConfig) override { return WMError::WM_OK; }
    WMError NotifyWindowTransition(sptr<WindowTransitionInfo>& from, sptr<WindowTransitionInfo>& to,
        bool isFromClient = false) override { return WMError::WM_OK; }
    WMError GetModeChangeHotZones(DisplayId displayId, ModeChangeHotZones& hotZones) override { return WMError::WM_OK; }
    void MinimizeWindowsByLauncher(std::vector<uint32_t> windowIds, bool isAnimated,
        sptr<RSIWindowAnimationFinishedCallback>& finishCallback) override {}
    WMError UpdateAvoidAreaListener(uint32_t windowId, bool haveListener) override { return WMError::WM_OK; }
    WMError UpdateRsTree(uint32_t windowId, bool isAdd) override { return WMError::WM_OK; }
    WMError BindDialogTarget(uint32_t& windowId, sptr<IRemoteObject> targetToken) override { return WMError::WM_OK; }
    void SetAnchorAndScale(int32_t x, int32_t y, float scale) override {}
    void SetAnchorOffset(int32_t deltaX, int32_t deltaY) override {}
    void OffWindowZoom() override {}
    WMError RaiseToAppTop(uint32_t windowId) override { return WMError::WM_OK; }
    std::shared_ptr<Media::PixelMap> GetSnapshot(int32_t windowId) override { return nullptr; }
    WMError SetGestureNavigaionEnabled(bool enable) override { return WMError::WM_OK; }
    void DispatchKeyEvent(uint32_t windowId, std::shared_ptr<MMI::KeyEvent> event) override {}
    void NotifyDumpInfoResult(const std::vector<std::string>& info) override {};
    WMError GetWindowAnimationTargets(std::vector<uint32_t> missionIds,
        std::vector<sptr<RSWindowAnimationTarget>>& targets) override { return WMError::WM_OK; }
    void SetMaximizeMode(MaximizeMode maximizeMode) override {}
    MaximizeMode GetMaximizeMode() override { return MaximizeMode::MODE_AVOID_SYSTEM_BAR; }
    void GetFocusWindowInfo(FocusChangeInfo& focusInfo) override {}
    WSError RaiseWindowToTop(int32_t persistentId) override { return WSError::WS_OK; }
    WSError ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId) override
    {
        return WSError::WS_OK;
    }
    void AddExtensionWindowStageToSCB(const sptr<ISessionStage>& sessionStage,
        const sptr<IRemoteObject>& token, uint64_t surfaceNodeId) override {}
    void RemoveExtensionWindowStageFromSCB(const sptr<ISessionStage>& sessionStage,
        const sptr<IRemoteObject>& token) override {}
    void UpdateModalExtensionRect(const sptr<IRemoteObject>& token, Rect rect) override {}
    void ProcessModalExtensionPointDown(const sptr<IRemoteObject>& token, int32_t posX, int32_t posY) override {}
    WSError AddOrRemoveSecureSession(int32_t persistentId, bool shouldHide) override
    {
        return WSError::WS_OK;
    }
    WSError UpdateExtWindowFlags(const sptr<IRemoteObject>& token, uint32_t extWindowFlags,
        uint32_t extWindowActions) override
    {
        return WSError::WS_OK;
    }
    WSError GetHostWindowRect(int32_t hostWindowId, Rect& rect) override
    {
        return WSError::WS_OK;
    }
    WSError GetFreeMultiWindowEnableState(bool& enable) override
    {
        return WSError::WS_OK;
    }
    WMError GetCallingWindowWindowStatus(int32_t persistentId, WindowStatus& windowStatus) override
    {
        return WMError::WM_OK;
    }
    WMError GetCallingWindowRect(int32_t persistentId, Rect& rect) override
    {
        return WMError::WM_OK;
    }
    WMError GetWindowModeType(WindowModeType& windowModeType) override { return WMError::WM_OK; }

    WMError GetWindowStyleType(WindowStyleType& windowStyleType) override { return WMError::WM_OK; }

    virtual WMError GetProcessSurfaceNodeIdByPersistentId(const int32_t pid,
        const std::vector<int32_t>& persistentIds, std::vector<uint64_t>& surfaceNodeIds) = 0;

    WMError ReleaseForegroundSessionScreenLock() override { return WMError::WM_OK; }

    WMError IsWindowRectAutoSave(const std::string& key, bool& enabled) override { return WMError::WM_OK; }

    WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap) override { return WMError::WM_OK; }
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_INTERFACE_H
