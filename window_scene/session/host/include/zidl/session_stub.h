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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_STUB_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_STUB_H

#include <map>

#include <iremote_stub.h>

#include "session/host/include/zidl/session_interface.h"

namespace OHOS::Rosen {
class SessionStub;

class SessionStub : public IRemoteStub<ISession> {
public:
    SessionStub() = default;
    virtual ~SessionStub() = default;

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    int HandleConnect(MessageParcel& data, MessageParcel& reply);
    int HandleForeground(MessageParcel& data, MessageParcel& reply);
    int HandleBackground(MessageParcel& data, MessageParcel& reply);
    int HandleDisconnect(MessageParcel& data, MessageParcel& reply);
    int HandleShow(MessageParcel& data, MessageParcel& reply);
    int HandleHide(MessageParcel& data, MessageParcel& reply);
    int HandleDrawingCompleted(MessageParcel& data, MessageParcel& reply);
    int HandleRemoveStartingWindow(MessageParcel& data, MessageParcel& reply);

    // scene session
    int HandleSessionEvent(MessageParcel& data, MessageParcel& reply);
    int HandleSyncSessionEvent(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateSessionRect(MessageParcel& data, MessageParcel& reply);
    int HandleGetGlobalScaledRect(MessageParcel& data, MessageParcel& reply);
    int HandleRaiseToAppTop(MessageParcel& data, MessageParcel& reply);
    int HandleBackPressed(MessageParcel& data, MessageParcel& reply);
    int HandleMarkProcessed(MessageParcel& data, MessageParcel& reply);
    int HandleSetGlobalMaximizeMode(MessageParcel& data, MessageParcel& reply);
    int HandleGetGlobalMaximizeMode(MessageParcel& data, MessageParcel& reply);
    int HandleNeedAvoid(MessageParcel& data, MessageParcel& reply);
    int HandleGetAvoidAreaByType(MessageParcel& data, MessageParcel& reply);
    int HandleGetAllAvoidAreas(MessageParcel& data, MessageParcel& reply);
    int HandleSetAspectRatio(MessageParcel& data, MessageParcel& reply);
    int HandleSetWindowAnimationFlag(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateWindowSceneAfterCustomAnimation(MessageParcel& data, MessageParcel& reply);
    int HandleRaiseAboveTarget(MessageParcel& data, MessageParcel& reply);
    int HandleRaiseAppMainWindowToTop(MessageParcel& data, MessageParcel& reply);
    int HandleChangeSessionVisibilityWithStatusBar(MessageParcel& data, MessageParcel& reply);
    int HandlePendingSessionActivation(MessageParcel& data, MessageParcel& reply);
    int HandleTerminateSession(MessageParcel& data, MessageParcel& reply);
    int HandleSessionException(MessageParcel& data, MessageParcel& reply);
    int HandleProcessPointDownSession(MessageParcel& data, MessageParcel& reply);
    int HandleSendPointerEvenForMoveDrag(MessageParcel& data, MessageParcel& reply);
    int HandleGetStartMoveFlag(MessageParcel& data, MessageParcel& reply);
    int HandleSetLandscapeMultiWindow(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateRectChangeListenerRegistered(MessageParcel& data, MessageParcel& reply);
    int HandleSetCallingSessionId(MessageParcel& data, MessageParcel& reply);
    int HandleSetCustomDecorHeight(MessageParcel& data, MessageParcel& reply);
    int HandleAdjustKeyboardLayout(MessageParcel& data, MessageParcel& reply);
    int HandleUpdatePropertyByAction(MessageParcel& data, MessageParcel& reply);
    int HandleLayoutFullScreenChange(MessageParcel& data, MessageParcel& reply);
    int HandleDefaultDensityEnabled(MessageParcel& data, MessageParcel& reply);
    int HandleTitleAndDockHoverShowChange(MessageParcel& data, MessageParcel& reply);
    int HandleRestoreMainWindow(MessageParcel& data, MessageParcel& reply);
    int HandleGetAppForceLandscapeConfig(MessageParcel& data, MessageParcel& reply);
    int HandleGetStatusBarHeight(MessageParcel& data, MessageParcel& reply);
    int HandleNotifyFrameLayoutFinish(MessageParcel& data, MessageParcel& reply);
    int HandleSetDialogSessionBackGestureEnabled(MessageParcel& data, MessageParcel& reply);
    int HandleSetSystemEnableDrag(MessageParcel& data, MessageParcel& reply);
    int HandleRequestFocus(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateClientRect(MessageParcel& data, MessageParcel& reply);
    int HandleSetGestureBackEnabled(MessageParcel& data, MessageParcel& reply);
    int HandleSessionModalTypeChange(MessageParcel& data, MessageParcel& reply);
    int HandleMainSessionModalTypeChange(MessageParcel& data, MessageParcel& reply);

    // extension extension
    int HandleTransferAbilityResult(MessageParcel& data, MessageParcel& reply);
    int HandleTransferExtensionData(MessageParcel& data, MessageParcel& reply);
    int HandleNotifySyncOn(MessageParcel& data, MessageParcel& reply);
    int HandleNotifyAsyncOn(MessageParcel& data, MessageParcel& reply);
    int HandleNotifyExtensionDied(MessageParcel& data, MessageParcel& reply);
    int HandleNotifyExtensionTimeout(MessageParcel& data, MessageParcel& reply);
    int HandleTriggerBindModalUIExtension(MessageParcel& data, MessageParcel& reply);
    int HandleTransferAccessibilityEvent(MessageParcel& data, MessageParcel& reply);
    int HandleNotifyExtensionEventAsync(MessageParcel& data, MessageParcel& reply);
    int HandleNotifyExtensionDetachToDisplay(MessageParcel& data, MessageParcel& reply);

    // PictureInPicture
    int HandleNotifyPiPWindowPrepareClose(MessageParcel& data, MessageParcel& reply);
    int HandleUpdatePiPRect(MessageParcel& data, MessageParcel& reply);
    int HandleUpdatePiPControlStatus(MessageParcel& data, MessageParcel& reply);
    int HandleSetAutoStartPiP(MessageParcel& data, MessageParcel& reply);

    // PC Window
    int HandleSetWindowRectAutoSave(MessageParcel& data, MessageParcel& reply);

    int ProcessRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option);
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_STUB_H
