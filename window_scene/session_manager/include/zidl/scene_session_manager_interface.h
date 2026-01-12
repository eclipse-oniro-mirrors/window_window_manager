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
        TRANS_ID_GET_ROOT_UI_CONTENT_REMOTE_OBJ,
        TRANS_ID_UPDATE_WINDOW_VISIBILITY_LISTENER,
        TRANS_ID_UPDATE_SESSION_OCCLUSION_STATE_LISTENER,
        TRANS_ID_SHIFT_APP_WINDOW_FOCUS,
        TRANS_ID_LIST_WINDOW_INFO,
        TRANS_ID_GET_WINDOW_LAYOUT_INFO,
        TRANS_ID_GET_ALL_MAIN_WINDOW_INFO,
        TRANS_ID_GET_MAIN_WINDOW_SNAPSHOT,
        TRANS_ID_GET_GLOBAL_WINDOW_MODE,
        TRANS_ID_GET_TOP_NAV_DEST_NAME,
        TRANS_ID_SET_APP_WATERMARK_IMAGE,
        TRANS_ID_RECOVER_APP_WATERMARK_IMAGE,
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
        TRANS_ID_SET_PROCESS_SNAPSHOT_SKIP,
        TRANS_ID_SET_SNAPSHOT_SKIP_BY_USERID_AND_BUNDLENAMES,
        TRANS_ID_SET_PROCESS_WATERMARK,
        TRANS_ID_GET_WINDOW_IDS_BY_COORDINATE,
        TRANS_ID_UPDATE_SESSION_SCREEN_LOCK,
        TRANS_ID_ADD_SKIP_SELF_ON_VIRTUAL_SCREEN,
        TRANS_ID_REMOVE_SKIP_SELF_ON_VIRTUAL_SCREEN,
        TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_TAG_SWITCH,
        TRANS_ID_IS_PC_WINDOW,
        TRANS_ID_IS_FREE_MULTI_WINDOW,
        TRANS_ID_IS_PC_OR_PAD_FREE_MULTI_WINDOW_MODE,
        TRANS_ID_GET_DISPLAYID_BY_WINDOWID,
        TRANS_ID_IS_WINDOW_RECT_AUTO_SAVE,
        TRANS_ID_SET_GLOBAL_DRAG_RESIZE_TYPE,
        TRANS_ID_GET_GLOBAL_DRAG_RESIZE_TYPE,
        TRANS_ID_SET_APP_DRAG_RESIZE_TYPE,
        TRANS_ID_GET_APP_DRAG_RESIZE_TYPE,
        TRANS_ID_SET_APP_KEY_FRAME_POLICY,
        TRANS_ID_WATCH_GESTURE_CONSUME_RESULT,
        TRANS_ID_WATCH_FOCUS_ACTIVE_CHANGE,
        TRANS_ID_SHIFT_APP_WINDOW_POINTER_EVENT,
        TRANS_ID_NOTIFY_SCREEN_SHOT_EVENT,
        TRANS_ID_SET_START_WINDOW_BACKGROUND_COLOR,
        TRANS_ID_REQUEST_FOCUS_STATUS_BY_SA,
        TRANS_ID_SET_PARENT_WINDOW,
        TRANS_ID_MINIMIZE_BY_WINDOW_ID,
        TRANS_ID_SET_FOREGROUND_WINDOW_NUM,
        TRANS_ID_USE_IMPLICIT_ANIMATION,
        TRANS_ID_SET_IMAGE_FOR_RECENT,
        TRANS_ID_SET_IMAGE_FOR_RECENT_PIXELMAP,
        TRANS_ID_REMOVE_IMAGE_FOR_RECENT,
        TRANS_ID_REGISTER_WINDOW_PROPERTY_CHANGE_AGENT,
        TRANS_ID_UNREGISTER_WINDOW_PROPERTY_CHANGE_AGENT,
        TRANS_ID_GET_HOST_GLOBAL_SCALE_RECT,
        TRANS_ID_ANIMATE_TO_WINDOW,
        TRANS_ID_CREATE_UI_EFFECT_CONTROLLER,
        TRANS_ID_ADD_SESSION_BLACK_LIST,
        TRANS_ID_REMOVE_SESSION_BLACK_LIST,
        TRANS_ID_GET_PIP_SWITCH_STATUS,
        TRANS_ID_GET_PIP_IS_PIP_ENABLED,
        TRANS_ID_RECOVER_WINDOW_PROPERTY_CHANGE_FLAG,
        TRANS_ID_MINIMIZE_ALL_WINDOW,
        TRANS_ID_GLOBAL_COORDINATE_TO_RELATIVE_COORDINATE,
        TRANS_ID_UPDATE_OUTLINE,
        TRANS_ID_SET_SPECIFIC_WINDOW_ZINDEX,
        TRANS_ID_SUPPORT_ROTATION_REGISTERED,
        TRANS_ID_RESET_SPECIFIC_WINDOW_ZINDEX,
        TRANS_ID_GET_FOCUS_SESSION_INFO_BY_ABILITY_TOKEN,
    };

    virtual WSError SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label) = 0;
    virtual WSError SetSessionIcon(const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon) = 0;
    virtual WSError IsValidSessionIds(const std::vector<int32_t>& sessionIds, std::vector<bool>& results) = 0;
    virtual WSError PendingSessionToForeground(const sptr<IRemoteObject>& token,
        int32_t windowMode = DEFAULT_INVALID_WINDOW_MODE) = 0;
    virtual WSError PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
        bool shouldBackToCaller = true) = 0;
    virtual WSError GetFocusSessionToken(sptr<IRemoteObject>& token, DisplayId displayId = DEFAULT_DISPLAY_ID) = 0;
    virtual WSError GetFocusSessionElement(AppExecFwk::ElementName& element,
        DisplayId displayId = DEFAULT_DISPLAY_ID) = 0;

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

    /**
     * @brief notify watch gesture event consumption results
     *
     * This function provides the ability for notifying watch gesture event consumption results
     *
     * @param keycode keyEvent codes
     * @param isConsumed consume result
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     * @permission Make sure the caller has system permission.
     */
    WMError NotifyWatchGestureConsumeResult(int32_t keyCode, bool isConsumed) override { return WMError::WM_OK; }

    /**
     * @brief notify watch focus active change
     *
     * This function provides the ability for notifying watch focus active change
     *
     * @param isActive focus status
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     * @permission Make sure the caller has system permission.
     */
    WMError NotifyWatchFocusActiveChange(bool isActive) override { return WMError::WM_OK; }

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
    WMError RequestFocusStatusBySA(int32_t persistentId, bool isFocused = true,
        bool byForeground = true, FocusChangeReason reason = FocusChangeReason::SA_REQUEST) override
    {
        return WMError::WM_OK;
    }
    AvoidArea GetAvoidAreaByType(uint32_t windowId, AvoidAreaType type,
        const Rect& rect = Rect::EMPTY_RECT) override { return {}; }

    /**
     * @brief get top window information by id of main window.
     *
     * This function provides the ability for system applications to get window information.
     *
     * @param mainWinId the id of target main window.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     * @permission Make sure the caller has system permission.
     */
    WMError GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId) override { return WMError::WM_OK; }
    // only main window,sub window and dialog window can use
    WMError GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId) override { return WMError::WM_OK; }
    void NotifyServerReadyToMoveOrDrag(uint32_t windowId, sptr<WindowProperty>& windowProperty,
        sptr<MoveDragProperty>& moveDragProperty) override {}
    void ProcessPointDown(uint32_t windowId, bool isPointDown) override {}
    void ProcessPointUp(uint32_t windowId) override {}
    WMError MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId = 0) override { return WMError::WM_OK; }
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
    WMError RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
        uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent) override { return WMError::WM_OK; }
    WMError UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
        uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent) override { return WMError::WM_OK;}
    WMError RecoverWindowPropertyChangeFlag(uint32_t observedFlags, uint32_t interestedFlags) override
    {
        return WMError::WM_OK;
    }
    WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) override
    {
        return WMError::WM_OK;
    }
    WMError GetUnreliableWindowInfo(int32_t windowId, std::vector<sptr<UnreliableWindowInfo>>& infos) override
    {
        return WMError::WM_OK;
    }
    WMError ListWindowInfo(const WindowInfoOption& windowInfoOption,
        std::vector<sptr<WindowInfo>>& infos) override { return WMError::WM_OK; }
    WMError GetAllWindowLayoutInfo(DisplayId displayId,
        std::vector<sptr<WindowLayoutInfo>>& infos) override { return WMError::WM_OK; }
    WMError GetAllMainWindowInfo(std::vector<sptr<MainWindowInfo>>& infos) override { return WMError::WM_OK; }
    WMError GetMainWindowSnapshot(const std::vector<int32_t>& windowIds, const WindowSnapshotConfiguration& config,
        const sptr<IRemoteObject>& callback) override { return WMError::WM_OK; }
    WMError GetGlobalWindowMode(DisplayId displayId,
        GlobalWindowMode& globalWinMode) override { return WMError::WM_OK; }
    WMError GetTopNavDestinationName(int32_t windowId, std::string& topNavDestName) override { return WMError::WM_OK; }
    WMError SetWatermarkImageForApp(const std::shared_ptr<Media::PixelMap>& pixelMap,
        std::string& watermarkName) override { return WMError::WM_OK; }
    WMError RecoverWatermarkImageForApp(const std::string& watermarkName) override { return WMError::WM_OK; }
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
    WMError SetGestureNavigationEnabled(bool enable) override { return WMError::WM_OK; }
    void DispatchKeyEvent(uint32_t windowId, std::shared_ptr<MMI::KeyEvent> event) override {}
    void NotifyDumpInfoResult(const std::vector<std::string>& info) override {};
    WMError GetWindowAnimationTargets(std::vector<uint32_t> missionIds,
        std::vector<sptr<RSWindowAnimationTarget>>& targets) override { return WMError::WM_OK; }
    void SetMaximizeMode(MaximizeMode maximizeMode) override {}
    MaximizeMode GetMaximizeMode() override { return MaximizeMode::MODE_AVOID_SYSTEM_BAR; }
    void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId = DEFAULT_DISPLAY_ID) override {}
    void GetFocusWindowInfoByAbilityToken(FocusChangeInfo& focusInfo,
        const sptr<IRemoteObject>& abilityToken) override {};
    void GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                         std::vector<sptr<FocusChangeInfo>>& allFocusInfoList) override {}
    WMError MinimizeByWindowId(const std::vector<int32_t>& windowIds) override { return WMError::WM_OK; }
    WMError SetForegroundWindowNum(uint32_t windowNum) override { return WMError::WM_OK; }

    /**
     * @brief Raise a window to screen top by id of window.
     *
     * This function provides the ability for system applications to raise window.
     *
     * @param persistentId the id of target window.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     * @permission Make sure the caller has system permission.
     */
    WSError RaiseWindowToTop(int32_t persistentId) override { return WSError::WS_OK; }
    WSError ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId) override
    {
        return WSError::WS_OK;
    }
    WSError SetSpecificWindowZIndex(WindowType windowType, int32_t zIndex) override { return WSError::WS_OK; }
    WSError ResetSpecificWindowZIndex(int32_t pid) override { return WSError::WS_OK; }
    void AddExtensionWindowStageToSCB(const sptr<ISessionStage>& sessionStage, const sptr<IRemoteObject>& token,
        uint64_t surfaceNodeId, int64_t startModalExtensionTimeStamp, bool isConstrainedModal) override {}
    void RemoveExtensionWindowStageFromSCB(const sptr<ISessionStage>& sessionStage,
        const sptr<IRemoteObject>& token, bool isConstrainedModal) override {}
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
    WSError GetHostGlobalScaledRect(int32_t hostWindowId, Rect& globalScaledRect) override
    {
        return WSError::WS_OK;
    }
    WSError GetFreeMultiWindowEnableState(bool& enable) override
    {
        return WSError::WS_OK;
    }
    WMError GetCallingWindowWindowStatus(uint32_t callingWindowId, WindowStatus& windowStatus) override
    {
        return WMError::WM_OK;
    }
    WMError GetCallingWindowRect(uint32_t callingWindowId, Rect& rect) override
    {
        return WMError::WM_OK;
    }
    WMError GetWindowModeType(WindowModeType& windowModeType) override { return WMError::WM_OK; }

    WMError GetWindowStyleType(WindowStyleType& windowStyleType) override { return WMError::WM_OK; }

    virtual WMError GetProcessSurfaceNodeIdByPersistentId(const int32_t pid,
        const std::vector<int32_t>& persistentIds, std::vector<uint64_t>& surfaceNodeIds) = 0;

    WMError SkipSnapshotForAppProcess(int32_t pid, bool skip) override { return WMError::WM_OK; }

    virtual WMError SkipSnapshotByUserIdAndBundleNames(int32_t userId,
        const std::vector<std::string>& bundleNameList) = 0;

    WMError SetProcessWatermark(int32_t pid, const std::string& watermarkName,
        bool isEnabled) override { return WMError::WM_OK; }
    WMError GetWindowIdsByCoordinate(DisplayId displayId, int32_t windowNumber, int32_t x, int32_t y,
        std::vector<int32_t>& windowIds) override { return WMError::WM_OK; }

    WMError UpdateScreenLockStatusForApp(const std::string& bundleName,
        bool isRelease) override { return WMError::WM_OK; }

    virtual WMError AddSkipSelfWhenShowOnVirtualScreenList(
        const std::vector<int32_t>& persistentIds) { return WMError::WM_OK; }

    virtual WMError RemoveSkipSelfWhenShowOnVirtualScreenList(
        const std::vector<int32_t>& persistentIds) { return WMError::WM_OK; }

    virtual WMError SetScreenPrivacyWindowTagSwitch(
        uint64_t screenId, const std::vector<std::string>& privacyWindowTags, bool enable) { return WMError::WM_OK; }

    WMError IsPcOrPadFreeMultiWindowMode(bool& isPcOrPadFreeMultiWindowMode) override { return WMError::WM_OK; }

    WMError IsWindowRectAutoSave(const std::string& key, bool& enabled,
        int persistentId) override { return WMError::WM_OK; }

    WMError SetImageForRecent(uint32_t imgResourceId, ImageFit imageFit,
        int32_t persistentId) override { return WMError::WM_OK; }
    WMError SetImageForRecentPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap, ImageFit imageFit,
        int32_t persistentId) override { return WMError::WM_OK; }
    WMError RemoveImageForRecent(int32_t persistentId) override { return WMError::WM_OK; }
        
    WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap) override { return WMError::WM_OK; }

    WMError SetGlobalDragResizeType(DragResizeType dragResizeType) override { return WMError::WM_OK; }
    WMError GetGlobalDragResizeType(DragResizeType& dragResizeType) override { return WMError::WM_OK; }
    WMError SetAppDragResizeType(const std::string& bundleName,
        DragResizeType dragResizeType) override { return WMError::WM_OK; }
    WMError GetAppDragResizeType(const std::string& bundleName,
        DragResizeType& dragResizeType) override { return WMError::WM_OK; }
    WMError SetAppKeyFramePolicy(const std::string& bundleName,
        const KeyFramePolicy& keyFramePolicy) override { return WMError::WM_OK; }
    WMError ShiftAppWindowPointerEvent(int32_t sourcePersistentId, int32_t targetPersistentId,
        int32_t fingerId) override { return WMError::WM_OK; }
    WMError NotifyScreenshotEvent(ScreenshotEventType type) override { return WMError::WM_OK; }
    WMError SetStartWindowBackgroundColor(const std::string& moduleName, const std::string& abilityName,
        uint32_t color, int32_t uid) override { return WMError::WM_OK; }
    WMError HasFloatingWindowForeground(const sptr<IRemoteObject>& abilityToken,
        bool& hasOrNot) override { return WMError::WM_OK; }
    WSError UseImplicitAnimation(int32_t hostWindowId, bool useImplicit) override { return WSError::WS_OK; };
    WMError AnimateTo(int32_t windowId, const WindowAnimationProperty& animationProperty,
        const WindowAnimationOption& animationOption) override { return WMError::WM_OK; }
    WMError CreateUIEffectController(const sptr<IUIEffectControllerClient>& controllerClient,
        sptr<IUIEffectController>& controller, int32_t& controllerId) override { return WMError::WM_OK; };
    WMError AddSessionBlackList(const std::unordered_set<std::string>& bundleNames,
        const std::unordered_set<std::string>& privacyWindowTags) override { return WMError::WM_OK; }
    WMError RemoveSessionBlackList(const std::unordered_set<std::string>& bundleNames,
        const std::unordered_set<std::string>& privacyWindowTags) override { return WMError::WM_OK; }
    WMError GetPiPSettingSwitchStatus(bool& switchStatus) override { return WMError::WM_OK; }
    WMError GetIsPipEnabled(bool& isPipEnabled) override { return WMError::WM_OK; }
    WMError ConvertToRelativeCoordinateExtended(
        const Rect& rect, Rect& newRect, DisplayId& newDisplayId) override
    {
        return WMError::WM_OK;
    }
    WMError UpdateOutline(const sptr<IRemoteObject>& remoteObject, const OutlineParams& outlineParams) override
    {
        return WMError::WM_OK;
    }
    WMError NotifySupportRotationRegistered() override { return WMError::WM_OK; }
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_INTERFACE_H
