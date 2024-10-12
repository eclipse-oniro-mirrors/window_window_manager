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
    int HandleIsValidSessionIds(MessageParcel& data, MessageParcel& reply);
    int HandlePendingSessionToForeground(MessageParcel& data, MessageParcel& reply);
    int HandlePendingSessionToBackgroundForDelegator(MessageParcel& data, MessageParcel& reply);
    int HandleGetFocusSessionToken(MessageParcel& data, MessageParcel& reply);
    int HandleGetFocusSessionElement(MessageParcel& data, MessageParcel& reply);
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
    //for window manager service
    int HandleGetFocusSessionInfo(MessageParcel& data, MessageParcel& reply);
    int HandleRegisterWindowManagerAgent(MessageParcel &data, MessageParcel &reply);
    int HandleUnregisterWindowManagerAgent(MessageParcel &data, MessageParcel &reply);
    int HandleCheckWindowId(MessageParcel &data, MessageParcel &reply);
    int HandleGetVisibilityWindowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleGetWindowModeType(MessageParcel& data, MessageParcel& reply);
    int HandleRaiseWindowToTop(MessageParcel& data, MessageParcel& reply);
    int HandleGetMainWinodowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleGetAllMainWindowInfos(MessageParcel& data, MessageParcel& reply);
    int HandleClearMainSessions(MessageParcel& data, MessageParcel& reply);
    int HandleRegisterCollaborator(MessageParcel& data, MessageParcel& reply);
    int HandleUnregisterCollaborator(MessageParcel& data, MessageParcel& reply);
    int HandleGetWindowStyleType(MessageParcel& data, MessageParcel& reply);

    int ProcessRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option);
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_LITE_STUB_H