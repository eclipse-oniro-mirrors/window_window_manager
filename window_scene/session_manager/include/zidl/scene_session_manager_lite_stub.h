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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_STUB_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_STUB_H

#include <map>
#include <iremote_stub.h>

#include "session_manager/include/zidl/scene_session_manager_lite_interface.h"

namespace OHOS::Rosen {
class SceneSessionManagerLiteStub;

class SceneSessionManagerLiteStub : public IRemoteStub<ISceneSessionManagerLite> {
public:
    SceneSessionManagerLiteStub() = default;
    virtual ~SceneSessionManagerLiteStub() = default;

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    int HandleSetSessionLabel(MessageParcel& data, MessageParcel& reply);
    int HandleSetSessionIcon(MessageParcel& data, MessageParcel& reply);
    int HandleSetSessionIconForThirdParty(MessageParcel& data, MessageParcel& reply);
    int HandleIsValidSessionIds(MessageParcel& data, MessageParcel& reply);
    int HandlePendingSessionToForeground(MessageParcel& data, MessageParcel& reply);
    int HandlePendingSessionToBackground(MessageParcel& data, MessageParcel& reply);
    int HandlePendingSessionToBackgroundForDelegator(MessageParcel& data, MessageParcel& reply);
    int HandleGetFocusSessionToken(MessageParcel& data, MessageParcel& reply);
    int HandleGetFocusSessionElement(MessageParcel& data, MessageParcel& reply);
    int HandleIsFocusWindowParent(MessageParcel& data, MessageParcel& reply);
    int HandleRegisterSessionListener(MessageParcel& data, MessageParcel& reply);
    int HandleUnRegisterSessionListener(MessageParcel& data, MessageParcel& reply);
    int HandleGetSessionInfos(MessageParcel& data, MessageParcel& reply);
    int HandleGetMainWindowStatesByPid(MessageParcel& data, MessageParcel& reply);
    int HandleGetSessionInfo(MessageParcel& data, MessageParcel& reply);
    int HandleGetSessionInfoByContinueSessionId(MessageParcel& data, MessageParcel& reply);
    int HandleSetSessionContinueState(MessageParcel& data, MessageParcel& reply);
    int HandleTerminateSessionNew(MessageParcel& data, MessageParcel& reply);
    int HandleGetSessionSnapshot(MessageParcel& data, MessageParcel& reply);
    int HandleClearSession(MessageParcel& data, MessageParcel& reply);
    int HandleClearAllSessions(MessageParcel& data, MessageParcel& reply);
    int HandleLockSession(MessageParcel& data, MessageParcel& reply);
    int HandleUnlockSession(MessageParcel& data, MessageParcel& reply);
    int HandleMoveSessionsToForeground(MessageParcel& data, MessageParcel& reply);
    int HandleMoveSessionsToBackground(MessageParcel& data, MessageParcel& reply);
    int HandleGetParentMainWindowId(MessageParcel& data, MessageParcel& reply);
    //for window manager service
    int HandleGetFocusSessionInfo(MessageParcel& data, MessageParcel& reply);
    int HandleGetAllGroupInfo(MessageParcel& data, MessageParcel& reply);
    int HandleRegisterWindowManagerAgent(MessageParcel& data, MessageParcel& reply);
    int HandleUnregisterWindowManagerAgent(MessageParcel& data, MessageParcel& reply);
    int HandleCheckWindowId(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateWindowModeByIdForUITest(MessageParcel& data, MessageParcel& reply);
    int HandleCheckUIExtensionCreation(MessageParcel& data, MessageParcel& reply);
    int HandleSetGlobalDragResizeType(MessageParcel& data, MessageParcel& reply);
    int HandleGetGlobalDragResizeType(MessageParcel& data, MessageParcel& reply);
    int HandleSetAppDragResizeType(MessageParcel& data, MessageParcel& reply);
    int HandleGetAppDragResizeType(MessageParcel& data, MessageParcel& reply);
    int HandleSetAppKeyFramePolicy(MessageParcel& data, MessageParcel& reply);
    int HandleListWindowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleRegisterWindowPropertyChangeAgent(MessageParcel& data, MessageParcel& reply);
    int HandleUnregisterWindowPropertyChangeAgent(MessageParcel& data, MessageParcel& reply);
    int HandleRecoverWindowPropertyChangeFlag(MessageParcel& data, MessageParcel& reply);
    int HandleGetVisibilityWindowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateSessionScreenLock(MessageParcel& data, MessageParcel& reply);
    int HandleGetWindowModeType(MessageParcel& data, MessageParcel& reply);
    int HandleGetMainWinodowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateAnimationSpeedWithPid(MessageParcel& data, MessageParcel& reply);
    int HandleGetCallingWindowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleGetAllMainWindowInfos(MessageParcel& data, MessageParcel& reply);
    int HandleGetMainWindowInfoByToken(MessageParcel& data, MessageParcel& reply);
    int HandleClearMainSessions(MessageParcel& data, MessageParcel& reply);
    int HandleRaiseWindowToTop(MessageParcel& data, MessageParcel& reply);
    int HandleRegisterCollaborator(MessageParcel& data, MessageParcel& reply);
    int HandleUnregisterCollaborator(MessageParcel& data, MessageParcel& reply);
    int HandleGetWindowStyleType(MessageParcel& data, MessageParcel& reply);
    int HandleTerminateSessionByPersistentId(MessageParcel& data, MessageParcel& reply);
    int HandleCloseTargetFloatWindow(MessageParcel& data, MessageParcel& reply);
    int HandleCloseTargetPiPWindow(MessageParcel& data, MessageParcel& reply);
    int HandleGetCurrentPiPWindowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleGetRootMainWindowId(MessageParcel& data, MessageParcel& reply);
    int HandleGetAccessibilityWindowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleNotifyAppUseControlList(MessageParcel& data, MessageParcel& reply);
    int HandleMinimizeMainSession(MessageParcel& data, MessageParcel& reply);
    int HandleLockSessionByAbilityInfo(MessageParcel& data, MessageParcel& reply);
    int HandleHasFloatingWindowForeground(MessageParcel& data, MessageParcel& reply);
    int HandleRegisterSessionLifecycleListenerByIds(MessageParcel& data, MessageParcel& reply);
    int HandleRegisterSessionLifecycleListenerByBundles(MessageParcel& data, MessageParcel& reply);
    int HandleUnregisterSessionLifecycleListener(MessageParcel& data, MessageParcel& reply);
    int HandleGetRecentMainSessionInfoList(MessageParcel& data, MessageParcel& reply);
    int HandlePendingSessionToBackgroundByPersistentId(MessageParcel& data, MessageParcel& reply);
    int HandleCreateNewInstanceKey(MessageParcel& data, MessageParcel& reply);
    int HandleGetRouterStackInfo(MessageParcel& data, MessageParcel& reply);
    int HandleRemoveInstanceKey(MessageParcel& data, MessageParcel& reply);
    int HandleTransferSessionToTargetScreen(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateKioskAppList(MessageParcel& data, MessageParcel& reply);
    int HandleEnterKioskMode(MessageParcel& data, MessageParcel& reply);
    int HandleExitKioskMode(MessageParcel& data, MessageParcel& reply);
    int HandleSendPointerEventForHover(MessageParcel& data, MessageParcel& reply);
    int HandleSetPipEnableByScreenId(MessageParcel& data, MessageParcel& reply);
    int HandleUnsetPipEnableByScreenId(MessageParcel& data, MessageParcel& reply);
    int HandleRegisterPipChgListener(MessageParcel& data, MessageParcel& reply);
    int HandleUnRegisterPipChgListener(MessageParcel& data, MessageParcel& reply);
    int HandleGetDisplayIdByWindowId(MessageParcel& data, MessageParcel& reply);

    int ProcessRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option);
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_STUB_H