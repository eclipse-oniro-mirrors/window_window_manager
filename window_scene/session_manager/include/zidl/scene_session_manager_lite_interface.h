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
        TRANS_ID_CHECK_WINDOW_ID,
        TRANS_ID_GET_VISIBILITY_WINDOW_INFO_ID,
        TRANS_ID_GET_WINDOW_MODE_TYPE,
        TRANS_ID_RAISE_WINDOW_TO_TOP,
        TRANS_ID_GET_TOPN_MAIN_WINDOW_INFO,
        TRANS_ID_REGISTER_COLLABORATOR,
        TRANS_ID_UNREGISTER_COLLABORATOR,
        TRANS_ID_GET_ALL_MAIN_WINDOW_INFO,
        TRANS_ID_CLEAR_MAIN_SESSIONS,
        TRANS_ID_GET_WINDOW_STYLE_TYPE,
        TRANS_ID_TERMINATE_SESSION_BY_PERSISTENT_ID,
        TRANS_ID_GET_MAIN_WINDOW_STATES_BY_PID,
        TRANS_ID_CLOSE_TARGET_FLOAT_WINDOW,
        TRANS_ID_CLOSE_TARGET_PIP_WINDOW,
        TRANS_ID_GET_CURRENT_PIP_WINDOW_INFO,
        TRANS_ID_NOTIFY_APP_USE_CONTROL_LIST,
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
    virtual WSError SetSessionContinueState(const sptr<IRemoteObject>& token, const ContinueState& continueState) = 0;
    virtual WSError TerminateSessionNew(
        const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker = false) = 0;
    virtual WSError GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
                                       SessionSnapshot& snapshot, bool isLowResolution) = 0;
    virtual WSError ClearSession(int32_t persistentId) = 0;
    virtual WSError ClearAllSessions() = 0;
    virtual WSError LockSession(int32_t sessionId) = 0;
    virtual WSError UnlockSession(int32_t sessionId) = 0;
    virtual WSError MoveSessionsToForeground(const std::vector<std::int32_t>& sessionIds, int32_t topSessionId) = 0;
    virtual WSError MoveSessionsToBackground(const std::vector<std::int32_t>& sessionIds,
        std::vector<std::int32_t>& result) = 0;
    virtual WSError RaiseWindowToTop(int32_t persistentId) = 0;
    virtual WSError RegisterIAbilityManagerCollaborator(int32_t type,
        const sptr<AAFwk::IAbilityManagerCollaborator>& impl) = 0;
    virtual WSError UnregisterIAbilityManagerCollaborator(int32_t type) = 0;
    virtual WMError GetWindowStyleType(WindowStyleType& windowStyleType) = 0;

    /**
     * @brief Application Control SA Notification Window Control Application Information
     *
     * The application control SA notifies whether the window application is controlled or not.\n
     * When the window main program starts, the application control information is fully notified,\n
     * and only incremental information is notified after full notification.\n
     *
     * @param type controls the application type (caller), such as application lock
     * @param userId User ID
     * @param controlList Control Application Information List
     * @return Successful call returns WSError: WS-OK, otherwise it indicates failure
     * @permission application requires SA permission
     */
    virtual WSError NotifyAppUseControlList(
        ControlAppType type, int32_t userId, const std::vector<ControlAppInfo>& controlList) = 0;

    /**
     * @brief Obtains main window state list by pid
     * @caller SA
     * @permission SA permission
     *
     * @param pid Target pid
     * @param windowStates Window state list
     */
    virtual WSError GetMainWindowStatesByPid(int32_t pid, std::vector<MainWindowState>& windowStates) = 0;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_INTERFACE_H
