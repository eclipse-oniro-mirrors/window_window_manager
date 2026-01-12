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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_STUB_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_STUB_H

#include <map>
#include <iremote_stub.h>

#include "session_manager/include/zidl/scene_session_manager_interface.h"

namespace OHOS::Rosen {
class SceneSessionManagerStub;

class SceneSessionManagerStub : public IRemoteStub<ISceneSessionManager> {
public:
    SceneSessionManagerStub() = default;
    virtual ~SceneSessionManagerStub() = default;

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    int HandleGetAccessibilityWindowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleConvertToRelativeCoordinateExtended(MessageParcel& data, MessageParcel& reply);
    int HandleGetUnreliableWindowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleRequestFocusStatus(MessageParcel& data, MessageParcel& reply);
    int HandleRequestFocusStatusBySA(MessageParcel& data, MessageParcel& reply);
    int HandleCreateAndConnectSpecificSession(MessageParcel& data, MessageParcel& reply);
    int HandleRecoverAndConnectSpecificSession(MessageParcel& data, MessageParcel& reply);
    int HandleRecoverAndReconnectSceneSession(MessageParcel& data, MessageParcel& reply);
    int HandleDestroyAndDisconnectSpcificSession(MessageParcel& data, MessageParcel& reply);
    int HandleDestroyAndDisconnectSpcificSessionWithDetachCallback(MessageParcel& data, MessageParcel& reply);
    int HandleRegisterWindowManagerAgent(MessageParcel& data, MessageParcel& reply);
    int HandleRegisterWindowPropertyChangeAgent(MessageParcel& data, MessageParcel& reply);
    int HandleUnregisterWindowPropertyChangeAgent(MessageParcel& data, MessageParcel& reply);
    int HandleRecoverWindowPropertyChangeFlag(MessageParcel& data, MessageParcel& reply);
    int HandleUnregisterWindowManagerAgent(MessageParcel& data, MessageParcel& reply);
    int HandleGetFocusSessionInfo(MessageParcel& data, MessageParcel& reply);
    int HandleGetFocusWindowInfoByAbilityToken(MessageParcel& data, MessageParcel& reply);
    int HandleSetSessionLabel(MessageParcel& data, MessageParcel& reply);
    int HandleSetSessionIcon(MessageParcel& data, MessageParcel& reply);
    int HandleIsValidSessionIds(MessageParcel& data, MessageParcel& reply);
    int HandlePendingSessionToForeground(MessageParcel& data, MessageParcel& reply);
    int HandlePendingSessionToBackgroundForDelegator(MessageParcel& data, MessageParcel& reply);
    int HandleGetFocusSessionToken(MessageParcel& data, MessageParcel& reply);
    int HandleGetFocusSessionElement(MessageParcel& data, MessageParcel& reply);
    int HandleSetGestureNavigationEnabled(MessageParcel& data, MessageParcel& reply);
    int HandleCheckWindowId(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateWindowModeByIdForUITest(MessageParcel& data, MessageParcel& reply);

    int HandleRegisterSessionListener(MessageParcel& data, MessageParcel& reply);
    int HandleUnRegisterSessionListener(MessageParcel& data, MessageParcel& reply);
    int HandleGetSessionInfos(MessageParcel& data, MessageParcel& reply);
    int HandleGetSessionInfo(MessageParcel& data, MessageParcel& reply);
    int HandleGetSessionInfoByContinueSessionId(MessageParcel& data, MessageParcel& reply);

    /*
     * Sub Window
     */
    int HandleSetParentWindow(MessageParcel& data, MessageParcel& reply);

    int HandleDumpSessionAll(MessageParcel& data, MessageParcel& reply);
    int HandleDumpSessionWithId(MessageParcel& data, MessageParcel& reply);
    int HandleSetSessionContinueState(MessageParcel& data, MessageParcel& reply);
    int HandleTerminateSessionNew(MessageParcel& data, MessageParcel& reply);
    int HandleGetSessionDump(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateSessionAvoidAreaListener(MessageParcel& data, MessageParcel& reply);
    int HandleGetSessionSnapshot(MessageParcel& data, MessageParcel& reply);
    int HandleGetSessionSnapshotById(MessageParcel& data, MessageParcel& reply);
    int HandleGetUIContentRemoteObj(MessageParcel& data, MessageParcel& reply);
    int HandleGetRootUIContentRemoteObj(MessageParcel& data, MessageParcel& reply);
    int HandleBindDialogTarget(MessageParcel& data, MessageParcel& reply);
    int HandleNotifyDumpInfoResult(MessageParcel& data, MessageParcel& reply);
    int HandleClearSession(MessageParcel& data, MessageParcel& reply);
    int HandleClearAllSessions(MessageParcel& data, MessageParcel& reply);
    int HandleLockSession(MessageParcel& data, MessageParcel& reply);
    int HandleUnlockSession(MessageParcel& data, MessageParcel& reply);
    int HandleMoveSessionsToForeground(MessageParcel& data, MessageParcel& reply);
    int HandleMoveSessionsToBackground(MessageParcel& data, MessageParcel& reply);
    int HandleRegisterCollaborator(MessageParcel& data, MessageParcel& reply);
    int HandleUnregisterCollaborator(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateSessionTouchOutsideListener(MessageParcel& data, MessageParcel& reply);
    int HandleRaiseWindowToTop(MessageParcel& data, MessageParcel& reply);
    int HandleNotifyWindowExtensionVisibilityChange(MessageParcel& data, MessageParcel& reply);
    int HandleGetTopWindowId(MessageParcel& data, MessageParcel& reply);
    int HandleWatchGestureConsumeResult(MessageParcel& data, MessageParcel& reply);
    int HandleWatchFocusActiveChange(MessageParcel& data, MessageParcel& reply);
    int HandleGetParentMainWindowId(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateSessionWindowVisibilityListener(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateSessionOcclusionStateListener(MessageParcel& data, MessageParcel& reply);
    int HandleShiftAppWindowFocus(MessageParcel& data, MessageParcel& reply);
    int HandleListWindowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleGetAllWindowLayoutInfo(MessageParcel& data, MessageParcel& reply);
    int HandleGetAllMainWindowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleGetMainWindowSnapshot(MessageParcel& data, MessageParcel& reply);
    int HandleGetGlobalWindowMode(MessageParcel& data, MessageParcel& reply);
    int HandleGetTopNavDestinationName(MessageParcel& data, MessageParcel& reply);
    int HandleSetWatermarkImageForApp(MessageParcel& data, MessageParcel& reply);
    int HandleRecoverWatermarkImageForApp(MessageParcel& data, MessageParcel& reply);
    int HandleGetVisibilityWindowInfo(MessageParcel& data, MessageParcel& reply);
    int HandleAddExtensionWindowStageToSCB(MessageParcel& data, MessageParcel& reply);
    int HandleRemoveExtensionWindowStageFromSCB(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateModalExtensionRect(MessageParcel& data, MessageParcel& reply);
    int HandleProcessModalExtensionPointDown(MessageParcel& data, MessageParcel& reply);
    int HandleAddOrRemoveSecureSession(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateExtWindowFlags(MessageParcel& data, MessageParcel& reply);
    int HandleGetHostWindowRect(MessageParcel& data, MessageParcel& reply);
    int HandleGetHostGlobalScaledRect(MessageParcel& data, MessageParcel& reply);
    int HandleGetFreeMultiWindowEnableState(MessageParcel& data, MessageParcel& reply);
    int HandleGetCallingWindowWindowStatus(MessageParcel& data, MessageParcel& reply);
    int HandleGetCallingWindowRect(MessageParcel& data, MessageParcel& reply);
    int HandleGetWindowModeType(MessageParcel& data, MessageParcel& reply);
    int HandleGetWindowStyleType(MessageParcel& data, MessageParcel& reply);
    int HandleGetProcessSurfaceNodeIdByPersistentId(MessageParcel& data, MessageParcel& reply);
    int HandleSkipSnapshotForAppProcess(MessageParcel& data, MessageParcel& reply);
    int HandleSkipSnapshotByUserIdAndBundleNames(MessageParcel& data, MessageParcel& reply);
    int HandleSetProcessWatermark(MessageParcel& data, MessageParcel& reply);
    int HandleGetWindowIdsByCoordinate(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateSessionScreenLock(MessageParcel& data, MessageParcel& reply);
    int HandleAddSkipSelfWhenShowOnVirtualScreenList(MessageParcel& data, MessageParcel& reply);
    int HandleRemoveSkipSelfWhenShowOnVirtualScreenList(MessageParcel& data, MessageParcel& reply);
    int HandleSetScreenPrivacyWindowTagSwitch(MessageParcel& data, MessageParcel& reply);
    int HandleIsPcWindow(MessageParcel& data, MessageParcel& reply);
    int HandleIsFreeMultiWindow(MessageParcel& data, MessageParcel& reply);
    int HandleIsPcOrPadFreeMultiWindowMode(MessageParcel& data, MessageParcel& reply);
    int HandleIsWindowRectAutoSave(MessageParcel& data, MessageParcel& reply);
    int HandleSetImageForRecent(MessageParcel& data, MessageParcel& reply);
    int HandleSetImageForRecentPixelMap(MessageParcel& data, MessageParcel& reply);
    int HandleRemoveImageForRecent(MessageParcel& data, MessageParcel& reply);
    int HandleGetDisplayIdByWindowId(MessageParcel& data, MessageParcel& reply);
    int HandleSetGlobalDragResizeType(MessageParcel& data, MessageParcel& reply);
    int HandleGetGlobalDragResizeType(MessageParcel& data, MessageParcel& reply);
    int HandleSetAppDragResizeType(MessageParcel& data, MessageParcel& reply);
    int HandleGetAppDragResizeType(MessageParcel& data, MessageParcel& reply);
    int HandleSetAppKeyFramePolicy(MessageParcel& data, MessageParcel& reply);
    int HandleShiftAppWindowPointerEvent(MessageParcel& data, MessageParcel& reply);
    int HandleNotifyScreenshotEvent(MessageParcel& data, MessageParcel& reply);
    int HandleSetStartWindowBackgroundColor(MessageParcel& data, MessageParcel& reply);
    int HandleAnimateTo(MessageParcel& data, MessageParcel& reply);

    int ProcessRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int HandleMinimizeByWindowId(MessageParcel& data, MessageParcel& reply);
    int HandleSetForegroundWindowNum(MessageParcel& data, MessageParcel& reply);
    int HandleUseImplicitAnimation(MessageParcel& data, MessageParcel& reply);
    int HandleCreateUIEffectController(MessageParcel& data, MessageParcel& reply);
    int HandleAddSessionBlackList(MessageParcel& data, MessageParcel& reply);
    int HandleRemoveSessionBlackList(MessageParcel& data, MessageParcel& reply);
    int HandleGetPiPSettingSwitchStatus(MessageParcel& data, MessageParcel& reply);
    int HandleGetIsPipEnabled(MessageParcel& data, MessageParcel& reply);
    int HandleMinimizeAllAppWindows(MessageParcel& data, MessageParcel& reply);
    int HandleUpdateOutline(MessageParcel& data, MessageParcel& reply);
    int HandleSetSpecificWindowZIndex(MessageParcel& data, MessageParcel& reply);
    int HandleResetSpecificWindowZIndex(MessageParcel& data, MessageParcel& reply);
    int HandleNotifySupportRotationRegistered(MessageParcel& data, MessageParcel& reply);
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_MANAGER_STUB_H