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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_INTERFACE_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_INTERFACE_H

#include <iremote_broker.h>
#include "common/include/window_session_property.h"
#include "iability_manager_collaborator.h"
#include "interfaces/include/ws_common.h"
#include "interfaces/include/ws_common_inner.h"
#include "mission_info.h"
#include "mission_listener_interface.h"
#include "mission_snapshot.h"
#include "session_info.h"
#include "zidl/window_manager_lite_interface.h"
#include "session_lifecycle_listener_interface.h"
#include "session_router_stack_listener.h"
#include "pip_change_listener.h"

namespace OHOS::Media {
class PixelMap;
} // namespace OHOS::Media

namespace OHOS::Rosen {
using ISessionListener = AAFwk::IMissionListener;
using SessionInfoBean = AAFwk::MissionInfo;
using SessionSnapshot = AAFwk::MissionSnapshot;
class ISceneSessionManagerLite : public OHOS::Rosen::IWindowManagerLite {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ISceneSessionManagerLite");

    enum class SceneSessionManagerLiteMessage : uint32_t {
        TRANS_ID_SET_SESSION_LABEL,
        TRANS_ID_SET_SESSION_ICON,
        TRANS_ID_IS_VALID_SESSION_IDS,
        TRANS_ID_PENDING_SESSION_TO_FOREGROUND,
        TRANS_ID_PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR,
        TRANS_ID_GET_FOCUS_SESSION_TOKEN,
        TRANS_ID_GET_FOCUS_SESSION_ELEMENT,
        TRANS_ID_REGISTER_SESSION_LISTENER,
        TRANS_ID_UNREGISTER_SESSION_LISTENER,
        TRANS_ID_GET_MISSION_INFOS,
        TRANS_ID_GET_MISSION_INFO_BY_ID,
        TRANS_ID_GET_SESSION_INFO_BY_CONTINUE_SESSION_ID,
        TRANS_ID_TERMINATE_SESSION_NEW,
        TRANS_ID_GET_SESSION_SNAPSHOT,
        TRANS_ID_SET_SESSION_CONTINUE_STATE,
        TRANS_ID_CLEAR_SESSION,
        TRANS_ID_CLEAR_ALL_SESSIONS,
        TRANS_ID_LOCK_SESSION,
        TRANS_ID_UNLOCK_SESSION,
        TRANS_ID_MOVE_MISSIONS_TO_FOREGROUND,
        TRANS_ID_MOVE_MISSIONS_TO_BACKGROUND,
        //window manager message
        TRANS_ID_GET_FOCUS_SESSION_INFO,
        TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT = 22,
        TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT,
        TRANS_ID_GET_WINDOW_INFO,
        TRANS_ID_GET_ALL_GROUP_INFO,
        TRANS_ID_CHECK_WINDOW_ID,
        TRANS_ID_SET_GLOBAL_DRAG_RESIZE_TYPE,
        TRANS_ID_GET_GLOBAL_DRAG_RESIZE_TYPE,
        TRANS_ID_SET_APP_DRAG_RESIZE_TYPE,
        TRANS_ID_GET_APP_DRAG_RESIZE_TYPE,
        TRANS_ID_SET_APP_KEY_FRAME_POLICY,
        TRANS_ID_LIST_WINDOW_INFO,
        TRANS_ID_REGISTER_WINDOW_PROPERTY_CHANGE_AGENT,
        TRANS_ID_UNREGISTER_WINDOW_PROPERTY_CHANGE_AGENT,
        TRANS_ID_RECOVER_WINDOW_PROPERTY_CHANGE_FLAG,
        TRANS_ID_GET_VISIBILITY_WINDOW_INFO_ID,
        TRANS_ID_UPDATE_SESSION_SCREEN_LOCK,
        TRANS_ID_GET_WINDOW_MODE_TYPE,
        TRANS_ID_GET_TOPN_MAIN_WINDOW_INFO,
        TRANS_ID_GET_ALL_MAIN_WINDOW_INFO,
        TRANS_ID_CLEAR_MAIN_SESSIONS,
        TRANS_ID_RAISE_WINDOW_TO_TOP,
        TRANS_ID_REGISTER_COLLABORATOR,
        TRANS_ID_UNREGISTER_COLLABORATOR,
        TRANS_ID_GET_WINDOW_STYLE_TYPE,
        TRANS_ID_TERMINATE_SESSION_BY_PERSISTENT_ID,
        TRANS_ID_CLOSE_TARGET_FLOAT_WINDOW,
        TRANS_ID_CLOSE_TARGET_PIP_WINDOW,
        TRANS_ID_GET_CURRENT_PIP_WINDOW_INFO,
        TRANS_ID_GET_MAIN_WINDOW_STATES_BY_PID,
        TRANS_ID_GET_ROOT_MAIN_WINDOW_ID,
        TRANS_ID_UI_EXTENSION_CREATION_CHECK,
        TRANS_ID_NOTIFY_APP_USE_CONTROL_LIST,
        TRANS_ID_MINIMIZE_MAIN_SESSION,
        TRANS_ID_LOCK_SESSION_BY_ABILITY_INFO,
        TRANS_ID_HAS_FLOAT_FOREGROUND,
        TRANS_ID_GET_CALLING_WINDOW_INFO,
        TRANS_ID_REGISTER_SESSION_LIFECYCLE_LISTENER_BY_IDS,
        TRANS_ID_REGISTER_SESSION_LIFECYCLE_LISTENER_BY_BUNDLES,
        TRANS_ID_UNREGISTER_SESSION_LIFECYCLE_LISTENER,
        TRANS_ID_GET_RECENT_MAIN_SESSION_INFO_LIST,
        TRANS_ID_PENDING_SESSION_TO_BACKGROUND_BY_PERSISTENTID,
        TRANS_ID_CREATE_NEW_INSTANCE_KEY,
        TRANS_ID_GET_ROUTER_STACK_INFO,
        TRANS_ID_GET_DISPLAYID_BY_WINDOWID,
        TRANS_ID_REMOVE_INSTANCE_KEY,
        TRANS_ID_TRANSFER_SESSION_TO_TARGET_SCREEN,
        TRANS_ID_PENDING_SESSION_TO_BACKGROUND,
        TRANS_ID_UPDATE_KIOSK_APP_LIST,
        TRANS_ID_ENTER_KIOSK_MODE,
        TRANS_ID_EXIT_KIOSK_MODE,
        TRANS_ID_UPDATE_WINDOW_MODE_BY_ID_FOR_UI_TEST,
        TRANS_ID_SEND_POINTER_EVENT_FOR_HOVER,
        TRANS_ID_SET_START_WINDOW_BACKGROUND_COLOR,
        TRANS_IS_SET_IMAGE_FOR_RECENT,
        TRANS_ID_IS_FOCUS_WINDOW_PARENT,
        TRANS_ID_SET_PIP_ENABLED_BY_SCREENID,
        TRANS_ID_UNSET_PIP_ENABLED_BY_SCREENID,
        TRANS_ID_REGISTER_PIP_CHG_LISTENER,
        TRANS_ID_UNREGISTER_PIP_CHG_LISTENER,
        TRANS_ID_UPDATE_ANIMATION_SPEED_WITH_PID,
        TRANS_ID_GET_PARENT_WINDOW_ID,
        TRANS_ID_SET_SESSION_ICON_FOR_THIRD_PARTY,
        TRANS_ID_GET_MAIN_WINDOW_INFO_BY_TOKEN,
    };

    /*
     * Window Layout
     */
    virtual WMError UpdateWindowModeByIdForUITest(int32_t windowId, int32_t updateMode) { return WMError::WM_OK; }

    /*
     * Window Lifecycle
     */
    virtual WSError PendingSessionToForeground(const sptr<IRemoteObject>& token,
        int32_t windowMode = DEFAULT_INVALID_WINDOW_MODE) = 0;
    virtual WSError PendingSessionToBackground(const sptr<IRemoteObject>& token, const BackgroundParams& params) = 0;
    virtual WSError PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
        bool shouldBackToCaller = true) = 0;
    virtual WSError MoveSessionsToForeground(const std::vector<std::int32_t>& sessionIds, int32_t topSessionId) = 0;
    virtual WSError MoveSessionsToBackground(const std::vector<std::int32_t>& sessionIds,
        std::vector<std::int32_t>& result) = 0;
    virtual WSError TerminateSessionNew(
        const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker = false) = 0;
    virtual WSError ClearSession(int32_t persistentId) = 0;
    virtual WSError ClearAllSessions() = 0;
    virtual WMError GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId) = 0;
    virtual WSError SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label) = 0;
    virtual WSError SetSessionIcon(const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon) = 0;
    virtual WSError SetSessionIconForThirdParty(
        const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon) = 0;
    virtual WSError RegisterIAbilityManagerCollaborator(int32_t type,
        const sptr<AAFwk::IAbilityManagerCollaborator>& impl) = 0;
    virtual WSError UnregisterIAbilityManagerCollaborator(int32_t type) = 0;
    virtual WSError RegisterSessionListener(const sptr<ISessionListener>& listener, bool isRecover = false) = 0;
    virtual WSError UnRegisterSessionListener(const sptr<ISessionListener>& listener) = 0;
    virtual WSError GetSessionInfos(const std::string& deviceId,
                                    int32_t numMax, std::vector<SessionInfoBean>& sessionInfos) = 0;
    virtual WSError GetSessionInfo(const std::string& deviceId, int32_t persistentId, SessionInfoBean& sessionInfo) = 0;
    virtual WSError GetSessionInfoByContinueSessionId(const std::string& continueSessionId,
        SessionInfoBean& sessionInfo) = 0;
    virtual WSError SetSessionContinueState(const sptr<IRemoteObject>& token, const ContinueState& continueState) = 0;
    virtual WSError IsValidSessionIds(const std::vector<int32_t>& sessionIds, std::vector<bool>& results) = 0;
    virtual WSError GetFocusSessionToken(sptr<IRemoteObject>& token, DisplayId displayId = DEFAULT_DISPLAY_ID) = 0;
    virtual WSError GetFocusSessionElement(AppExecFwk::ElementName& element,
        DisplayId displayId = DEFAULT_DISPLAY_ID) = 0;
    virtual WSError IsFocusWindowParent(const sptr<IRemoteObject>& token, bool& isParent) { return WSError::WS_OK; }
    virtual WSError GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
                                       SessionSnapshot& snapshot, bool isLowResolution) = 0;
    virtual WSError LockSession(int32_t sessionId) = 0;
    virtual WSError UnlockSession(int32_t sessionId) = 0;
    virtual WSError RaiseWindowToTop(int32_t persistentId) = 0;
    virtual WMError GetWindowStyleType(WindowStyleType& windowStyleType) = 0;
    virtual WMError SetGlobalDragResizeType(DragResizeType dragResizeType) { return WMError::WM_OK; }
    virtual WMError GetGlobalDragResizeType(DragResizeType& dragResizeType) { return WMError::WM_OK; }
    virtual WMError SetAppDragResizeType(const std::string& bundleName,
        DragResizeType dragResizeType) { return WMError::WM_OK; }
    virtual WMError GetAppDragResizeType(const std::string& bundleName,
        DragResizeType& dragResizeType) { return WMError::WM_OK; }
    virtual WMError SetAppKeyFramePolicy(const std::string& bundleName,
        const KeyFramePolicy& keyFramePolicy) { return WMError::WM_OK; }
    virtual WMError ListWindowInfo(const WindowInfoOption& windowInfoOption,
        std::vector<sptr<WindowInfo>>& infos) = 0;

    /**
     * @brief Application Control SA Notification Window Control Application Information
     *
     * The application control SA notifies whether the window application is controlled or not.
     * When the window main program starts, the application control information is fully notified,
     * and only incremental information is notified after full notification.
     *
     * @param type controls the application type (caller), such as application lock
     * @param userId User ID
     * @param controlList Control Application Information List
     * @return Successful call returns WSError: WS-OK, otherwise it indicates failure
     * @permission application requires SA permission and ohos.permission.WRITE_APP_LOCK permission
     */
    virtual WSError NotifyAppUseControlList(
        ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList) = 0;

    /**
     * @brief Obtains main window state list by pid
     * @caller SA
     * @permission SA permission
     *
     * @param pid Target pid
     * @param windowStates Window state list
     */
    virtual WSError GetMainWindowStatesByPid(int32_t pid, std::vector<MainWindowState>& windowStates) = 0;

    /**
     * @brief Minimize All Main Sessions of An Application.
     *
     * This function is used to minimize the main sessions of the application with the same bundleName and appIndex.
     * The invoker must be an SA or SystemApp and have the related permission.
     *
     * @param bundleName bundle name of the application that need to be minimized.
     * @param appIndex appIndex of the target application
     * @param userId User ID
     * @return Successful call returns WMError: WS-OK, otherwise it indicates failure
     * @permission application requires SA permission or SystemApp permission
     */
    virtual WMError MinimizeMainSession(const std::string& bundleName, int32_t appIndex, int32_t userId) = 0;

    /**
     * @brief Lock or unlock a session in recent tasks.
     *
     * This function lock or unlock the session in recent tasks.
     * The invoker must be an SA or SystemApp and have the ohos.permission.MANAGE_MISSIONS permission.
     *
     * @param AbilityInfoBase abilityInfo of the session that needed to be locked or locked.
     * @param isLock isLock of the session that needed to be locked or unlocked.
     * @return Successful call returns WMError: WS-OK, otherwise it indicates failure
     * @permission application requires ohos.permission.MANAGE_MISSIONS permission and
     * SA permission or SystemApp permission
     */
    virtual WMError LockSessionByAbilityInfo(const AbilityInfoBase& abilityInfo, bool isLock) = 0;

    /**
     * @brief Query if there is float type window foreground of an abilityToken
     *
     * This function is used to query if there is float type window foreground of an ability
     *
     * @caller SA
     * @permission SA permission
     *
     * @param abilityToken token of ability
     * @param hasOrNot result for output
     */
    virtual WMError HasFloatingWindowForeground(const sptr<IRemoteObject>& abilityToken,
        bool& hasOrNot) = 0;

    /**
     * @brief Register a main session lifecycle listener for specific persistentIds
     *
     * This function is used to register a main session lifecycle listener for a list of specific persistentIds.
     * The listener will be notified when lifecycle events occur for the specified persistentId
     *
     * @caller SA
     * @permission SA permission
     *
     * @param listener The session lifecycle listener to be registered
     * @param persistentIdList The list of persistentId for which the listener should be registered
     * @return Successful call returns WMError: WM-OK, otherwise it indicates failure
     */
    virtual WMError RegisterSessionLifecycleListenerByIds(const sptr<ISessionLifecycleListener>& listener,
        const std::vector<int32_t>& persistentIdList) = 0;

    /**
     * @brief Register a session lifecycle listener for specific bundles
     *
     * This function is used to register a session lifecycle listener for a list of specific bundles.
     * The listener will be notified when lifecycle events occur for the specified bundles
     *
     * @caller SA
     * @permission SA permission
     *
     * @param listener The session lifecycle listener to be registered
     * @param bundleNameList The list of bundle for which the listener should be registered
     * @return Successful call returns WMError: WM-OK, otherwise it indicates failure
     */
    virtual WMError RegisterSessionLifecycleListenerByBundles(const sptr<ISessionLifecycleListener>& listener,
        const std::vector<std::string>& bundleNameList) = 0;

    /**
     * @brief Unregister a session lifecycle listener
     *
     * This function is used to unregister a session lifecycle listener.
     * The unregistered listener will no longer receive notifications about session lifecycle events.
     *
     * @caller SA
     * @permission SA permission
     *
     * @param listener The session lifecycle listener to be unregistered
     * @return Successful call returns WMError: WS-OK, otherwise it indicates failure
     */
    virtual WMError UnregisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener) = 0;

    /**
     * @brief Get an ordered recent main session info list
     *
     * This function is used to get an ordered recent main session info list
     *
     * @caller SA or SystemApp
     * @permission SA permission or SystemApp permission
     *
     * @param recentSessionInfoList the sessionInfo list of recent main sessions
     * @return Successful call returns WSError: WS-OK, otherwise it indicates failure
     */
    virtual WSError GetRecentMainSessionInfoList(std::vector<RecentSessionInfo>& recentSessionInfoList) = 0;

    /**
     * @brief pending session to background
     *
     * This function is used to request session to background by persistentid
     *
     * @caller SA or SystemApp
     * @permission application requires ohos.permission.MANAGE_MISSIONS permission and
     * SA permission or SystemApp permission
     *
     * @param persistentId the session of persistentId
     * @param shouldBackToCaller should back to caller
     * @return Successful call returns WSError: WS-OK, otherwise it indicates failure
     */
    virtual WSError PendingSessionToBackgroundByPersistentId(const int32_t persistentId,
        bool shouldBackToCaller = true) { return WSError::WS_OK; };

    /**
     * @brief Create a new instanceKey of a specific bundle
     *
     * This function is used to create a new instanceKey
     * If the number of instanceKey reaches max limit, then return the last created instanceKey
     *
     * @caller SA or SystemApp
     * @permission SA permission or SystemApp permission
     *
     * @param bundleName the bundleName of the new instanceKey needs to be created
     * @param instanceKey will be assigned the instanceKey just created
     * @return Successful call returns WSError: WM-OK, otherwise it indicates failure
     */
    virtual WMError CreateNewInstanceKey(const std::string& bundleName, std::string& instanceKey) = 0;

    /**
     * @brief Get the router stack by persistentId
     *
     * This function is used to get the router stack by persistentId from arkui
     *
     * @caller SA or SystemApp
     * @permission SA permission or SystemApp permission
     *
     * @param persistentId the id of session
     * @param listener the callback when get router stack from arkui
     * @return Successful call returns WSError: WM-OK, otherwise it indicates failure
     */
    virtual WMError GetRouterStackInfo(int32_t persistentId, const sptr<ISessionRouterStackListener>& listener) = 0;

    /**
     * @brief Remove a instanceKey of a specific bundle
     *
     * This function is used to remove a instanceKey of a specific bundle
     *
     * @caller SA or SystemApp
     * @permission SA permission or SystemApp permission
     *
     * @param bundleName the bundleName of the instanceKey to be removed
     * @param instanceKey the instanceKey that needs to be removed
     * @return Successful call returns WSError: WM-OK, otherwise it indicates failure
     */
    virtual WMError RemoveInstanceKey(const std::string& bundleName, const std::string& instanceKey) = 0;

    /**
     * @brief Transfer a session to the target screen
     *
     * This function is used to transfer a session to the target screen
     *
     * @caller SA or SystemApp
     * @permission SA permission or SystemApp permission
     *
     * @param info The session infomation to be transferred
     * @return Successful call returns WMError: WM-OK, otherwise it indicates failure
     */
    virtual WMError TransferSessionToTargetScreen(const TransferSessionInfo& info) = 0;
    
    /**
     * @brief Update the list of apps which can be used in kiosk mode
     *
     * This function is used to update the list of apps which can be used in kiosk mode
     *
     * @caller SA or SystemApp
     * @permission SA permission or SystemApp permission
     *
     * @param kioskAppList the list of apps which can be used in kiosk mode
     * @return Successful call returns WMError: WM-OK, otherwise it indicates failure
     */
    virtual WMError UpdateKioskAppList(const std::vector<std::string>& kioskAppList) { return WMError::WM_OK; }

    /**
     * @brief Notify that mission enters kiosk mode
     *
     * This function is used to notify that mission enters kiosk mode
     *
     * @caller SA or SystemApp
     * @permission SA permission or SystemApp permission
     *
     * @param token the abilitytoken of the mission entered kiosk mode
     * @return Successful call returns WMError: WM-OK, otherwise it indicates failure
     */
    virtual WMError EnterKioskMode(const sptr<IRemoteObject>& token) { return WMError::WM_OK; }

    /**
     * @brief Notify exit kiosk mode
     *
     * This function is used to notify exit kiosk mode
     *
     * @caller SA or SystemApp
     * @permission SA permission or SystemApp permission
     *
     * @return Successful call returns WMError: WM-OK, otherwise it indicates failure
     */
    virtual WMError ExitKioskMode(const sptr<IRemoteObject>& token) { return WMError::WM_OK; }

    /**
     * @brief Send pointer event for hover.
     *
     * This function is used to send pointer event for hover
     *
     * @caller SA
     * @permission SA permission
     *
     * @param pointerEvent The pointer event for hover
     * @return Successful call returns WSError: WS-OK, otherwise it indicates failure
     */
    virtual WSError SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
        { return WSError::WS_OK; }
    
    virtual WMError SetPipEnableByScreenId(int32_t screenId, bool enabled) { return WMError::WM_OK; }
    virtual WMError UnsetPipEnableByScreenId(int32_t screenId) { return WMError::WM_OK; }
    virtual WMError RegisterPipChgListenerByScreenId(int32_t screenId, const sptr<IPipChangeListener>& listener)
    {
        return WMError::WM_OK;
    }
    virtual WMError UnregisterPipChgListenerByScreenId(int32_t screenId) { return WMError::WM_OK; }
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_INTERFACE_H
