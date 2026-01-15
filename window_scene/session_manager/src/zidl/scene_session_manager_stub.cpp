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

#include "session_manager/include/zidl/scene_session_manager_stub.h"

#include <ui/rs_surface_node.h>
#include "marshalling_helper.h"
#include "rs_adapter.h"
#include "ui_effect_controller_client_interface.h"
#include "ui_effect_controller_stub.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "SceneSessionManagerStub"};
constexpr uint32_t MAX_VECTOR_SIZE = 100;
}

int SceneSessionManagerStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    WLOGFD("Scene session on remote request!, code: %{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WLOGFE("Failed to check interface token!");
        return ERR_TRANSACTION_FAILED;
    }
    return ProcessRemoteRequest(code, data, reply, option);
}

int SceneSessionManagerStub::ProcessRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    switch (code) {
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_CREATE_AND_CONNECT_SPECIFIC_SESSION):
            return HandleCreateAndConnectSpecificSession(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_RECOVER_AND_CONNECT_SPECIFIC_SESSION):
            return HandleRecoverAndConnectSpecificSession(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_RECOVER_AND_RECONNECT_SCENE_SESSION):
            return HandleRecoverAndReconnectSceneSession(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_DESTROY_AND_DISCONNECT_SPECIFIC_SESSION):
            return HandleDestroyAndDisconnectSpcificSession(data, reply);
        case static_cast<uint32_t>(
            SceneSessionManagerMessage::TRANS_ID_DESTROY_AND_DISCONNECT_SPECIFIC_SESSION_WITH_DETACH_CALLBACK):
            return HandleDestroyAndDisconnectSpcificSessionWithDetachCallback(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REQUEST_FOCUS):
            return HandleRequestFocusStatus(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REQUEST_FOCUS_STATUS_BY_SA):
            return HandleRequestFocusStatusBySA(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT):
            return HandleRegisterWindowManagerAgent(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT):
            return HandleUnregisterWindowManagerAgent(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REGISTER_WINDOW_PROPERTY_CHANGE_AGENT):
            return HandleRegisterWindowPropertyChangeAgent(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UNREGISTER_WINDOW_PROPERTY_CHANGE_AGENT):
            return HandleUnregisterWindowPropertyChangeAgent(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_INFO):
            return HandleGetFocusSessionInfo(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_INFO_BY_ABILITY_TOKEN):
            return HandleGetFocusWindowInfoByAbilityToken(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_SESSION_LABEL):
            return HandleSetSessionLabel(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_SESSION_ICON):
            return HandleSetSessionIcon(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_IS_VALID_SESSION_IDS):
            return HandleIsValidSessionIds(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_PENDING_SESSION_TO_FOREGROUND):
            return HandlePendingSessionToForeground(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR):
            return HandlePendingSessionToBackgroundForDelegator(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_TOKEN):
            return HandleGetFocusSessionToken(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_ELEMENT):
            return HandleGetFocusSessionElement(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_CHECK_WINDOW_ID):
            return HandleCheckWindowId(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_GESTURE_NAVIGATION_ENABLED):
            return HandleSetGestureNavigationEnabled(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_INFO):
            return HandleGetAccessibilityWindowInfo(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GLOBAL_COORDINATE_TO_RELATIVE_COORDINATE):
            return HandleConvertToRelativeCoordinateExtended(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_UNRELIABLE_WINDOW_INFO):
            return HandleGetUnreliableWindowInfo(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REGISTER_SESSION_LISTENER):
            return HandleRegisterSessionListener(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UNREGISTER_SESSION_LISTENER):
            return HandleUnRegisterSessionListener(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_MISSION_INFOS):
            return HandleGetSessionInfos(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_MISSION_INFO_BY_ID):
            return HandleGetSessionInfo(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_SESSION_INFO_BY_CONTINUE_SESSION_ID):
            return HandleGetSessionInfoByContinueSessionId(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_DUMP_SESSION_ALL):
            return HandleDumpSessionAll(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_DUMP_SESSION_WITH_ID):
            return HandleDumpSessionWithId(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_TERMINATE_SESSION_NEW):
            return HandleTerminateSessionNew(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UPDATE_AVOIDAREA_LISTENER):
            return HandleUpdateSessionAvoidAreaListener(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_SESSION_DUMP_INFO):
            return HandleGetSessionDump(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_SESSION_SNAPSHOT):
            return HandleGetSessionSnapshot(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_SESSION_SNAPSHOT_BY_ID):
            return HandleGetSessionSnapshotById(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_UI_CONTENT_REMOTE_OBJ):
            return HandleGetUIContentRemoteObj(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_ROOT_UI_CONTENT_REMOTE_OBJ):
            return HandleGetRootUIContentRemoteObj(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_BIND_DIALOG_TARGET):
            return HandleBindDialogTarget(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_NOTIFY_DUMP_INFO_RESULT):
            return HandleNotifyDumpInfoResult(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_SESSION_CONTINUE_STATE):
            return HandleSetSessionContinueState(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_CLEAR_SESSION):
            return HandleClearSession(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_CLEAR_ALL_SESSIONS):
            return HandleClearAllSessions(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_LOCK_SESSION):
            return HandleLockSession(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UNLOCK_SESSION):
            return HandleUnlockSession(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_MOVE_MISSIONS_TO_FOREGROUND):
            return HandleMoveSessionsToForeground(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_MOVE_MISSIONS_TO_BACKGROUND):
            return HandleMoveSessionsToBackground(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REGISTER_COLLABORATOR):
            return HandleRegisterCollaborator(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UNREGISTER_COLLABORATOR):
            return HandleUnregisterCollaborator(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UPDATE_TOUCHOUTSIDE_LISTENER):
            return HandleUpdateSessionTouchOutsideListener(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_RAISE_WINDOW_TO_TOP):
            return HandleRaiseWindowToTop(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_TOP_WINDOW_ID):
            return HandleGetTopWindowId(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_PARENT_MAIN_WINDOW_ID):
            return HandleGetParentMainWindowId(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_NOTIFY_WINDOW_EXTENSION_VISIBILITY_CHANGE):
            return HandleNotifyWindowExtensionVisibilityChange(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UPDATE_WINDOW_VISIBILITY_LISTENER):
            return HandleUpdateSessionWindowVisibilityListener(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UPDATE_SESSION_OCCLUSION_STATE_LISTENER):
            return HandleUpdateSessionOcclusionStateListener(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SHIFT_APP_WINDOW_FOCUS):
            return HandleShiftAppWindowFocus(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_LIST_WINDOW_INFO):
            return HandleListWindowInfo(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_LAYOUT_INFO):
            return HandleGetAllWindowLayoutInfo(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_ALL_MAIN_WINDOW_INFO):
            return HandleGetAllMainWindowInfo(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_MAIN_WINDOW_SNAPSHOT):
            return HandleGetMainWindowSnapshot(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_GLOBAL_WINDOW_MODE):
            return HandleGetGlobalWindowMode(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_TOP_NAV_DEST_NAME):
            return HandleGetTopNavDestinationName(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_APP_WATERMARK_IMAGE):
            return HandleSetWatermarkImageForApp(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_RECOVER_APP_WATERMARK_IMAGE):
            return HandleRecoverWatermarkImageForApp(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_VISIBILITY_WINDOW_INFO_ID):
            return HandleGetVisibilityWindowInfo(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_ADD_EXTENSION_WINDOW_STAGE_TO_SCB):
            return HandleAddExtensionWindowStageToSCB(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REMOVE_EXTENSION_WINDOW_STAGE_FROM_SCB):
            return HandleRemoveExtensionWindowStageFromSCB(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UPDATE_MODALEXTENSION_RECT_TO_SCB):
            return HandleUpdateModalExtensionRect(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_PROCESS_MODALEXTENSION_POINTDOWN_TO_SCB):
            return HandleProcessModalExtensionPointDown(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_ADD_OR_REMOVE_SECURE_SESSION):
            return HandleAddOrRemoveSecureSession(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UPDATE_EXTENSION_WINDOW_FLAGS):
            return HandleUpdateExtWindowFlags(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_HOST_WINDOW_RECT):
            return HandleGetHostWindowRect(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_HOST_GLOBAL_SCALE_RECT):
            return HandleGetHostGlobalScaledRect(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_STATUS):
            return HandleGetCallingWindowWindowStatus(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_RECT):
            return HandleGetCallingWindowRect(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_MODE_TYPE):
            return HandleGetWindowModeType(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_FREE_MULTI_WINDOW_ENABLE_STATE):
            return HandleGetFreeMultiWindowEnableState(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_STYLE_TYPE):
            return HandleGetWindowStyleType(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_PROCESS_SURFACENODEID_BY_PERSISTENTID):
            return HandleGetProcessSurfaceNodeIdByPersistentId(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_PROCESS_SNAPSHOT_SKIP):
            return HandleSkipSnapshotForAppProcess(data, reply);
        case static_cast<uint32_t>
            (SceneSessionManagerMessage::TRANS_ID_SET_SNAPSHOT_SKIP_BY_USERID_AND_BUNDLENAMES):
            return HandleSkipSnapshotByUserIdAndBundleNames(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_PROCESS_WATERMARK):
            return HandleSetProcessWatermark(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_IDS_BY_COORDINATE):
            return HandleGetWindowIdsByCoordinate(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UPDATE_SESSION_SCREEN_LOCK):
            return HandleUpdateSessionScreenLock(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_ADD_SKIP_SELF_ON_VIRTUAL_SCREEN):
            return HandleAddSkipSelfWhenShowOnVirtualScreenList(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REMOVE_SKIP_SELF_ON_VIRTUAL_SCREEN):
            return HandleRemoveSkipSelfWhenShowOnVirtualScreenList(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_TAG_SWITCH):
            return HandleSetScreenPrivacyWindowTagSwitch(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_IS_PC_WINDOW):
            return HandleIsPcWindow(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_IS_FREE_MULTI_WINDOW):
            return HandleIsFreeMultiWindow(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_IS_PC_OR_PAD_FREE_MULTI_WINDOW_MODE):
            return HandleIsPcOrPadFreeMultiWindowMode(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_DISPLAYID_BY_WINDOWID):
            return HandleGetDisplayIdByWindowId(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_IS_WINDOW_RECT_AUTO_SAVE):
            return HandleIsWindowRectAutoSave(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_GLOBAL_DRAG_RESIZE_TYPE):
            return HandleSetGlobalDragResizeType(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_GLOBAL_DRAG_RESIZE_TYPE):
            return HandleGetGlobalDragResizeType(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_APP_DRAG_RESIZE_TYPE):
            return HandleSetAppDragResizeType(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_APP_DRAG_RESIZE_TYPE):
            return HandleGetAppDragResizeType(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_APP_KEY_FRAME_POLICY):
            return HandleSetAppKeyFramePolicy(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_WATCH_GESTURE_CONSUME_RESULT):
            return HandleWatchGestureConsumeResult(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_WATCH_FOCUS_ACTIVE_CHANGE):
            return HandleWatchFocusActiveChange(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SHIFT_APP_WINDOW_POINTER_EVENT):
            return HandleShiftAppWindowPointerEvent(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_NOTIFY_SCREEN_SHOT_EVENT):
            return HandleNotifyScreenshotEvent(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_START_WINDOW_BACKGROUND_COLOR):
            return HandleSetStartWindowBackgroundColor(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_MINIMIZE_BY_WINDOW_ID):
            return HandleMinimizeByWindowId(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_PARENT_WINDOW):
            return HandleSetParentWindow(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_FOREGROUND_WINDOW_NUM):
            return HandleSetForegroundWindowNum(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_USE_IMPLICIT_ANIMATION):
            return HandleUseImplicitAnimation(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_IMAGE_FOR_RECENT):
            return HandleSetImageForRecent(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_IMAGE_FOR_RECENT_PIXELMAP):
            return HandleSetImageForRecentPixelMap(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REMOVE_IMAGE_FOR_RECENT):
            return HandleRemoveImageForRecent(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_ANIMATE_TO_WINDOW):
            return HandleAnimateTo(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_CREATE_UI_EFFECT_CONTROLLER):
            return HandleCreateUIEffectController(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_ADD_SESSION_BLACK_LIST):
            return HandleAddSessionBlackList(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REMOVE_SESSION_BLACK_LIST):
            return HandleRemoveSessionBlackList(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_PIP_SWITCH_STATUS):
            return HandleGetPiPSettingSwitchStatus(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_PIP_IS_PIP_ENABLED):
            return HandleGetIsPipEnabled(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_RECOVER_WINDOW_PROPERTY_CHANGE_FLAG):
            return HandleRecoverWindowPropertyChangeFlag(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_MINIMIZE_ALL_WINDOW):
            return HandleMinimizeAllAppWindows(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UPDATE_OUTLINE):
            return HandleUpdateOutline(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_SPECIFIC_WINDOW_ZINDEX):
            return HandleSetSpecificWindowZIndex(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SUPPORT_ROTATION_REGISTERED):
            return HandleNotifySupportRotationRegistered(data, reply);
        case static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_RESET_SPECIFIC_WINDOW_ZINDEX):
            return HandleResetSpecificWindowZIndex(data, reply);
        default:
            WLOGFE("Failed to find function handler!");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int SceneSessionManagerStub::HandleCreateAndConnectSpecificSession(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "run");
    sptr<IRemoteObject> sessionStageObject = data.ReadRemoteObject();
    sptr<ISessionStage> sessionStage = iface_cast<ISessionStage>(sessionStageObject);
    sptr<IRemoteObject> eventChannelObject = data.ReadRemoteObject();
    sptr<IWindowEventChannel> eventChannel = iface_cast<IWindowEventChannel>(eventChannelObject);
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Unmarshalling(data);
    TLOGD(WmsLogTag::WMS_SCB, "Unmarshalling RSSurfaceNode: %{public}s",
          RSAdapterUtil::RSNodeToStr(surfaceNode).c_str());
    if (sessionStage == nullptr || eventChannel == nullptr || surfaceNode == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to read scene session stage object or event channel object!");
        return ERR_INVALID_DATA;
    }
    sptr<WindowSessionProperty> property = data.ReadStrongParcelable<WindowSessionProperty>();
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "property is nullptr");
        return ERR_INVALID_DATA;
    }

    sptr<IRemoteObject> token = nullptr;
    if (property->GetTokenState()) {
        token = data.ReadRemoteObject();
    } else {
        TLOGW(WmsLogTag::WMS_LIFE, "accept token is nullptr");
    }

    auto persistentId = INVALID_SESSION_ID;
    sptr<ISession> sceneSession;
    SystemSessionConfig systemConfig;
    CreateAndConnectSpecificSession(sessionStage, eventChannel, surfaceNode,
        property, persistentId, sceneSession, systemConfig, token);
    if (sceneSession== nullptr) {
        return ERR_INVALID_STATE;
    }
    reply.WriteInt32(persistentId);
    reply.WriteRemoteObject(sceneSession->AsObject());
    reply.WriteParcelable(&systemConfig);
    reply.WriteUint32(property->GetSubWindowLevel());
    reply.WriteUint64(property->GetDisplayId());
    reply.WriteUint32(static_cast<uint32_t>(property->GetWindowType()));
    reply.WriteUint32(static_cast<uint32_t>(WSError::WS_OK));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRecoverAndConnectSpecificSession(MessageParcel& data, MessageParcel& reply)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "run!");
    sptr<IRemoteObject> sessionStageObject = data.ReadRemoteObject();
    sptr<ISessionStage> sessionStage = iface_cast<ISessionStage>(sessionStageObject);
    sptr<IRemoteObject> eventChannelObject = data.ReadRemoteObject();
    sptr<IWindowEventChannel> eventChannel = iface_cast<IWindowEventChannel>(eventChannelObject);
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Unmarshalling(data);
    if (sessionStage == nullptr || eventChannel == nullptr || surfaceNode == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "Failed to read scene session stage object or event channel object!");
        return ERR_INVALID_DATA;
    }
    bool hasProperty = false;
    if (!data.ReadBool(hasProperty)) {
        TLOGE(WmsLogTag::WMS_RECOVER, "Read hasProperty failed!");
        return ERR_TRANSACTION_FAILED;
    }
    sptr<WindowSessionProperty> property = nullptr;
    if (hasProperty) {
        property = data.ReadStrongParcelable<WindowSessionProperty>();
    } else {
        TLOGW(WmsLogTag::WMS_RECOVER, "Property not exist!");
    }

    sptr<IRemoteObject> token = nullptr;
    if (property && property->GetTokenState()) {
        token = data.ReadRemoteObject();
    } else {
        TLOGI(WmsLogTag::WMS_RECOVER, "accept token is nullptr");
    }

    sptr<ISession> sceneSession;
    auto ret = RecoverAndConnectSpecificSession(sessionStage, eventChannel, surfaceNode, property, sceneSession, token);
    if (sceneSession== nullptr) {
        return ERR_INVALID_STATE;
    }
    reply.WriteRemoteObject(sceneSession->AsObject());
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRecoverAndReconnectSceneSession(MessageParcel& data, MessageParcel& reply)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "run");
    sptr<IRemoteObject> sessionStageObject = data.ReadRemoteObject();
    sptr<ISessionStage> sessionStage = iface_cast<ISessionStage>(sessionStageObject);
    sptr<IRemoteObject> eventChannelObject = data.ReadRemoteObject();
    sptr<IWindowEventChannel> eventChannel = iface_cast<IWindowEventChannel>(eventChannelObject);
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Unmarshalling(data);
    if (sessionStage == nullptr || eventChannel == nullptr || surfaceNode == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "Failed to read scene session stage object or event channel object!");
        return ERR_INVALID_DATA;
    }
    bool hasProperty = false;
    if (!data.ReadBool(hasProperty)) {
        TLOGE(WmsLogTag::WMS_RECOVER, "Read hasProperty failed!");
        return ERR_TRANSACTION_FAILED;
    }
    sptr<WindowSessionProperty> property = nullptr;
    if (hasProperty) {
        property = data.ReadStrongParcelable<WindowSessionProperty>();
    } else {
        TLOGW(WmsLogTag::WMS_RECOVER, "Property not exist!");
    }

    sptr<IRemoteObject> token = nullptr;
    if (property && property->GetTokenState()) {
        token = data.ReadRemoteObject();
    } else {
        TLOGI(WmsLogTag::WMS_RECOVER, "accept token is nullptr");
    }

    sptr<ISession> sceneSession;
    RecoverAndReconnectSceneSession(sessionStage, eventChannel, surfaceNode, sceneSession, property, token);
    if (sceneSession == nullptr) {
        return ERR_INVALID_STATE;
    }
    reply.WriteRemoteObject(sceneSession->AsObject());
    reply.WriteUint32(static_cast<uint32_t>(WSError::WS_OK));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleDestroyAndDisconnectSpcificSession(MessageParcel& data, MessageParcel& reply)
{
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read persistentId failed");
        return ERR_TRANSACTION_FAILED;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d", persistentId);
    WSError ret = DestroyAndDisconnectSpecificSession(persistentId);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleDestroyAndDisconnectSpcificSessionWithDetachCallback(MessageParcel& data,
    MessageParcel& reply)
{
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read persistentId failed");
        return ERR_TRANSACTION_FAILED;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d", persistentId);
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    const WSError ret = DestroyAndDisconnectSpecificSessionWithDetachCallback(persistentId, callback);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetParentWindow(MessageParcel& data, MessageParcel& reply)
{
    int32_t subWindowId = INVALID_WINDOW_ID;
    if (!data.ReadInt32(subWindowId)) {
        TLOGE(WmsLogTag::WMS_SUB, "read subWindowId failed");
        return ERR_INVALID_DATA;
    }
    int32_t newParentWindowId = INVALID_WINDOW_ID;
    if (!data.ReadInt32(newParentWindowId)) {
        TLOGE(WmsLogTag::WMS_SUB, "read newParentWindowId failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_SUB, "subWindowId: %{public}d, newParentWindowId: %{public}d",
        subWindowId, newParentWindowId);
    WMError errCode = SetParentWindow(subWindowId, newParentWindowId);
    if (!reply.WriteUint32(static_cast<uint32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_SUB, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRequestFocusStatus(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "run");
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "read persistentId failed");
        return ERR_INVALID_DATA;
    }
    bool isFocused = false;
    if (!data.ReadBool(isFocused)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "read isFocused failed");
        return ERR_INVALID_DATA;
    }
    WMError ret = RequestFocusStatus(persistentId, isFocused, true, FocusChangeReason::CLIENT_REQUEST);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRequestFocusStatusBySA(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "run");
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "read persistentId failed");
        return ERR_INVALID_DATA;
    }
    bool isFocused = false;
    if (!data.ReadBool(isFocused)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "read isFocused failed");
        return ERR_INVALID_DATA;
    }
    bool byForeground = false;
    if (!data.ReadBool(byForeground)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "read byForeground failed");
        return ERR_INVALID_DATA;
    }
    int32_t reason = static_cast<int32_t>(FocusChangeReason::SA_REQUEST);
    if (!data.ReadInt32(reason)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "read reason failed");
        return ERR_INVALID_DATA;
    }
    WMError ret = RequestFocusStatusBySA(persistentId, isFocused, byForeground,
        static_cast<FocusChangeReason>(reason));
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRegisterWindowManagerAgent(MessageParcel& data, MessageParcel& reply)
{
    uint32_t typeId = 0;
    if (!data.ReadUint32(typeId) ||
        typeId < static_cast<uint32_t>(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS) ||
        typeId >= static_cast<uint32_t>(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_END)) {
        return ERR_INVALID_DATA;
    }
    WindowManagerAgentType type = static_cast<WindowManagerAgentType>(typeId);
    TLOGD(WmsLogTag::DEFAULT, "type=%{public}u", typeId);
    sptr<IRemoteObject> windowManagerAgentObject = data.ReadRemoteObject();
    sptr<IWindowManagerAgent> windowManagerAgentProxy =
        iface_cast<IWindowManagerAgent>(windowManagerAgentObject);
    WMError errCode = RegisterWindowManagerAgent(type, windowManagerAgentProxy);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUnregisterWindowManagerAgent(MessageParcel& data, MessageParcel& reply)
{
    uint32_t typeId = 0;
    if (!data.ReadUint32(typeId) ||
        typeId < static_cast<uint32_t>(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS) ||
        typeId >= static_cast<uint32_t>(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_END)) {
        return ERR_INVALID_DATA;
    }
    WindowManagerAgentType type = static_cast<WindowManagerAgentType>(typeId);
    TLOGD(WmsLogTag::DEFAULT, "type=%{public}u", typeId);
    sptr<IRemoteObject> windowManagerAgentObject = data.ReadRemoteObject();
    sptr<IWindowManagerAgent> windowManagerAgentProxy =
        iface_cast<IWindowManagerAgent>(windowManagerAgentObject);
    WMError errCode = UnregisterWindowManagerAgent(type, windowManagerAgentProxy);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRecoverWindowPropertyChangeFlag(MessageParcel& data, MessageParcel& reply)
{
    uint32_t observedFlags = 0;
    if (!data.ReadUint32(observedFlags)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read observedFlags failed");
        return ERR_TRANSACTION_FAILED;
    }

    uint32_t interestFlags = 0;
    if (!data.ReadUint32(interestFlags)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read interestFlags failed");
        return ERR_TRANSACTION_FAILED;
    }

    WMError errCode = RecoverWindowPropertyChangeFlag(observedFlags, interestFlags);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write errCode failed");
        return ERR_TRANSACTION_FAILED;
    }

    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRegisterWindowPropertyChangeAgent(MessageParcel& data, MessageParcel& reply)
{
    int32_t windowInfoKeyValue = 0;
    if (!data.ReadInt32(windowInfoKeyValue)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read window info key value failed");
        return ERR_INVALID_DATA;
    }
    WindowInfoKey windowInfoKey = static_cast<WindowInfoKey>(windowInfoKeyValue);

    uint32_t interestInfo = 0;
    if (!data.ReadUint32(interestInfo)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read interest info failed");
        return ERR_INVALID_DATA;
    }

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY;
    sptr<IRemoteObject> windowManagerAgentObject = data.ReadRemoteObject();
    sptr<IWindowManagerAgent> windowManagerAgentProxy =
        iface_cast<IWindowManagerAgent>(windowManagerAgentObject);
    WMError errCode = RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgentProxy);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUnregisterWindowPropertyChangeAgent(MessageParcel& data, MessageParcel& reply)
{
    int32_t windowInfoKeyValue = 0;
    if (!data.ReadInt32(windowInfoKeyValue)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read window info key value failed");
        return ERR_INVALID_DATA;
    }
    WindowInfoKey windowInfoKey = static_cast<WindowInfoKey>(windowInfoKeyValue);

    uint32_t interestInfo = 0;
    if (!data.ReadUint32(interestInfo)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read interest info failed");
        return ERR_INVALID_DATA;
    }

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY;
    sptr<IRemoteObject> windowManagerAgentObject = data.ReadRemoteObject();
    sptr<IWindowManagerAgent> windowManagerAgentProxy =
        iface_cast<IWindowManagerAgent>(windowManagerAgentObject);
    WMError errCode = UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgentProxy);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetFocusSessionInfo(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "Run");
    FocusChangeInfo focusInfo;
    uint64_t displayId = 0;
    if (!data.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to read displayId");
        return ERR_INVALID_DATA;
    }
    GetFocusWindowInfo(focusInfo, displayId);
    reply.WriteParcelable(&focusInfo);
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetFocusWindowInfoByAbilityToken(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "Run");
    sptr<IRemoteObject> abilityToken = data.ReadRemoteObject();
    if (!abilityToken) {
        TLOGE(WmsLogTag::WMS_FOCUS, "AbilityToken is null.");
        return ERR_INVALID_DATA;
    }
    FocusChangeInfo focusInfo;
    GetFocusWindowInfoByAbilityToken(focusInfo, abilityToken);
    reply.WriteParcelable(&focusInfo);
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetSessionLabel(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DEFAULT, "In");
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    std::string label;
    if (!data.ReadString(label)) {
        TLOGE(WmsLogTag::DEFAULT, "read label failed");
        return ERR_INVALID_DATA;
    }
    WSError errCode = SetSessionLabel(token, label);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetSessionIcon(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DEFAULT, "In");
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    std::shared_ptr<Media::PixelMap> icon(data.ReadParcelable<Media::PixelMap>());
    if (icon == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "icon is null");
        return ERR_INVALID_DATA;
    }
    WSError errCode = SetSessionIcon(token, icon);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleIsValidSessionIds(MessageParcel& data, MessageParcel& reply)
{
    WLOGFI("run HandleIsValidSessionIds!");
    std::vector<int32_t> sessionIds;
    data.ReadInt32Vector(&sessionIds);
    std::vector<bool> results;
    reply.WriteBoolVector(results);
    return ERR_NONE;
}

int SceneSessionManagerStub::HandlePendingSessionToForeground(MessageParcel& data, MessageParcel& reply)
{
    WLOGFI("run HandlePendingSessionToForeground!");
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (token == nullptr) {
        WLOGFE("token is nullptr");
        return ERR_INVALID_DATA;
    }
    int32_t windowMode = 0;
    if (!data.ReadInt32(windowMode)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read windowMode fail");
        return ERR_INVALID_DATA;
    }
    WSError errCode = PendingSessionToForeground(token, windowMode);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandlePendingSessionToBackgroundForDelegator(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "run");
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (token == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "token is nullptr");
        return ERR_INVALID_DATA;
    }
    bool shouldBackToCaller = true;
    if (!data.ReadBool(shouldBackToCaller)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read shouldBackToCaller failed");
        return ERR_INVALID_DATA;
    }
    WSError errCode = PendingSessionToBackgroundForDelegator(token, shouldBackToCaller);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRegisterSessionListener(MessageParcel& data, MessageParcel& reply)
{
    WLOGFI("run HandleRegisterSessionListener!");
    sptr<ISessionListener> listener = iface_cast<ISessionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        reply.WriteInt32(static_cast<int32_t>(WSError::WS_ERROR_INVALID_PARAM));
        WLOGFI("listener is nullptr");
        return ERR_NONE;
    }
    WSError errCode = RegisterSessionListener(listener);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUnRegisterSessionListener(MessageParcel& data, MessageParcel& reply)
{
    WLOGFI("run HandleUnRegisterSessionListener!");
    sptr<ISessionListener> listener = iface_cast<ISessionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        reply.WriteInt32(static_cast<int32_t>(WSError::WS_ERROR_INVALID_PARAM));
        WLOGFI("listener is nullptr");
        return ERR_NONE;
    }
    WSError errCode = UnRegisterSessionListener(listener);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetSessionInfos(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In!");
    std::string deviceId = Str16ToStr8(data.ReadString16());
    int32_t numMax = 0;
    if (!data.ReadInt32(numMax)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read numMax failed");
        return ERR_INVALID_DATA;
    }
    std::vector<SessionInfoBean> missionInfos;
    WSError errCode = GetSessionInfos(deviceId, numMax, missionInfos);
    reply.WriteInt32(missionInfos.size());
    for (auto& it : missionInfos) {
        if (!reply.WriteParcelable(&it)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write missionInfos error");
            return ERR_INVALID_DATA;
        }
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write result error");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetSessionInfo(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In!");
    std::string deviceId = Str16ToStr8(data.ReadString16());
    int32_t persistentId;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read persistentId failed");
        return ERR_INVALID_DATA;
    }
    SessionInfoBean info;
    WSError errCode = GetSessionInfo(deviceId, persistentId, info);
    if (!reply.WriteParcelable(&info)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write sessionInfo error");
        return ERR_INVALID_DATA;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write result error");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}


int SceneSessionManagerStub::HandleGetSessionInfoByContinueSessionId(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In!");
    SessionInfoBean info;
    std::string continueSessionId;
    if (!data.ReadString(continueSessionId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read continueSessionId failed");
        return ERR_INVALID_DATA;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "continueSessionId: %{public}s", continueSessionId.c_str());
    WSError errCode = GetSessionInfoByContinueSessionId(continueSessionId, info);
    if (!reply.WriteParcelable(&info)) {
        TLOGE(WmsLogTag::WMS_LIFE, "GetSessionInfo error");
        return ERR_INVALID_DATA;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LIFE, "GetSessionInfo result error");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleDumpSessionAll(MessageParcel& data, MessageParcel& reply)
{
    WLOGFI("run HandleDumpSessionAll!");
    std::vector<std::string> infos;
    WSError errCode = DumpSessionAll(infos);
    if (!reply.WriteStringVector(infos)) {
        WLOGFE("HandleDumpSessionAll write info failed.");
        return ERR_TRANSACTION_FAILED;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        WLOGFE("HandleDumpSessionAll write errcode failed.");
        return ERR_TRANSACTION_FAILED;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleDumpSessionWithId(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DEFAULT, "In!");
    int32_t persistentId;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::DEFAULT, "read persistentId failed.");
        return ERR_INVALID_DATA;
    }
    std::vector<std::string> infos;
    WSError errCode = DumpSessionWithId(persistentId, infos);
    if (!reply.WriteStringVector(infos)) {
        TLOGE(WmsLogTag::DEFAULT, "write info failed.");
        return ERR_TRANSACTION_FAILED;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::DEFAULT, "write errcode failed.");
        return ERR_TRANSACTION_FAILED;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleTerminateSessionNew(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    sptr<AAFwk::SessionInfo> abilitySessionInfo = data.ReadParcelable<AAFwk::SessionInfo>();
    if (abilitySessionInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "abilitySessionInfo is null");
        return ERR_INVALID_DATA;
    }
    bool needStartCaller = false;
    if (!data.ReadBool(needStartCaller)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read needStartCaller failed.");
        return ERR_INVALID_DATA;
    }
    bool isFromBroker = false;
    if (!data.ReadBool(isFromBroker)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read isFromBroker failed.");
        return ERR_INVALID_DATA;
    }
    WSError errCode = TerminateSessionNew(abilitySessionInfo, needStartCaller, isFromBroker);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetFocusSessionToken(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("run HandleGetFocusSessionToken!");
    sptr<IRemoteObject> token = nullptr;
    uint64_t displayId = 0;
    if (!data.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to read displayId");
        return ERR_INVALID_DATA;
    }
    WSError errCode = GetFocusSessionToken(token, displayId);
    reply.WriteRemoteObject(token);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetFocusSessionElement(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("run HandleGetFocusSessionElement!");
    AppExecFwk::ElementName element;
    uint64_t displayId = 0;
    if (!data.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to read displayId");
        return ERR_INVALID_DATA;
    }
    WSError errCode = GetFocusSessionElement(element, displayId);
    reply.WriteParcelable(&element);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleCheckWindowId(MessageParcel& data, MessageParcel& reply)
{
    WLOGFI("run HandleCheckWindowId!");
    int32_t windowId = INVALID_WINDOW_ID;
    if (!data.ReadInt32(windowId)) {
        WLOGE("Failed to readInt32 windowId");
        return ERR_INVALID_DATA;
    }
    int32_t pid = INVALID_PID;
    WMError errCode = CheckWindowId(windowId, pid);
    if (errCode != WMError::WM_OK) {
        WLOGE("Failed to checkWindowId(%{public}d)", pid);
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(pid)) {
        WLOGE("Failed to WriteInt32 pid");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetGestureNavigationEnabled(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DEFAULT, "Run");
    bool enable = false;
    if (!data.ReadBool(enable)) {
        return ERR_INVALID_DATA;
    }
    const WMError &ret = SetGestureNavigationEnabled(enable);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetAccessibilityWindowInfo(MessageParcel& data, MessageParcel& reply)
{
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    WMError errCode = GetAccessibilityWindowInfo(infos);
    if (!MarshallingHelper::MarshallingVectorParcelableObj<AccessibilityWindowInfo>(reply, infos)) {
        WLOGFE("Write window infos failed.");
        return ERR_TRANSACTION_FAILED;
    }
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetUnreliableWindowInfo(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DEFAULT, "run!");
    std::vector<sptr<UnreliableWindowInfo>> infos;
    int32_t windowId = INVALID_WINDOW_ID;
    if (!data.ReadInt32(windowId)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to readInt32 windowId");
        return ERR_INVALID_DATA;
    }
    WMError errCode = GetUnreliableWindowInfo(windowId, infos);
    if (!MarshallingHelper::MarshallingVectorParcelableObj<UnreliableWindowInfo>(reply, infos)) {
        TLOGE(WmsLogTag::DEFAULT, "Write unreliable window infos failed.");
        return ERR_TRANSACTION_FAILED;
    }
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetSessionContinueState(MessageParcel& data, MessageParcel& reply)
{
    WLOGFI("HandleSetSessionContinueState");
    sptr <IRemoteObject> token = data.ReadRemoteObject();
    auto continueState = static_cast<ContinueState>(data.ReadInt32());
    const WSError &ret = SetSessionContinueState(token, continueState);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetSessionDump(MessageParcel& data, MessageParcel& reply)
{
    std::vector<std::string> params;
    if (!data.ReadStringVector(&params)) {
        WLOGFE("Fail to read params");
        return ERR_INVALID_DATA;
    }
    std::string dumpInfo;
    WSError errCode = GetSessionDumpInfo(params, dumpInfo);
    uint32_t infoSize = static_cast<uint32_t>(dumpInfo.length());
    TLOGD(WmsLogTag::DEFAULT, "HandleGetSessionDump, infoSize: %{public}d", infoSize);
    reply.WriteUint32(infoSize);
    if (infoSize != 0) {
        if (!reply.WriteRawData(dumpInfo.c_str(), infoSize)) {
            WLOGFE("Fail to write dumpInfo");
            return ERR_INVALID_DATA;
        }
    }
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUpdateSessionAvoidAreaListener(MessageParcel& data, MessageParcel& reply)
{
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        return ERR_INVALID_DATA;
    }
    bool haveAvoidAreaListener = false;
    if (!data.ReadBool(haveAvoidAreaListener)) {
        return ERR_INVALID_DATA;
    }
    WSError errCode = UpdateSessionAvoidAreaListener(persistentId, haveAvoidAreaListener);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetSessionSnapshot(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "Handled!");
    std::u16string deviceIdData;
    if (!data.ReadString16(deviceIdData)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "read deviceId fail");
        return ERR_INVALID_DATA;
    }
    std::string deviceId = Str16ToStr8(deviceIdData);
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "read persistentId fail");
        return ERR_INVALID_DATA;
    }
    bool isLowResolution = false;
    if (!data.ReadBool(isLowResolution)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "read isLowResolution fail");
        return ERR_INVALID_DATA;
    }
    std::shared_ptr<SessionSnapshot> snapshot = std::make_shared<SessionSnapshot>();
    WSError ret = GetSessionSnapshot(deviceId, persistentId, *snapshot, isLowResolution);
    reply.WriteParcelable(snapshot.get());
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetSessionSnapshotById(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "Handled!");
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "read persistentId fail");
        return ERR_INVALID_DATA;
    }
    std::shared_ptr<SessionSnapshot> snapshot = std::make_shared<SessionSnapshot>();
    const WMError ret = GetSessionSnapshotById(persistentId, *snapshot);
    reply.WriteParcelable(snapshot.get());
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetUIContentRemoteObj(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "In!");
    int32_t persistentId;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to read persistentId");
        return ERR_INVALID_DATA;
    }
    sptr<IRemoteObject> uiContentRemoteObj;
    WSError ret = GetUIContentRemoteObj(persistentId, uiContentRemoteObj);
    reply.WriteRemoteObject(uiContentRemoteObj);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetRootUIContentRemoteObj(MessageParcel& data, MessageParcel& reply)
{
    DisplayId displayId = 0;
    if (!data.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read displayId failed");
        return ERR_INVALID_DATA;
    }
    sptr<IRemoteObject> uiContentRemoteObj;
    auto errCode = GetRootUIContentRemoteObj(displayId, uiContentRemoteObj);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "get uiContent failed");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteRemoteObject(uiContentRemoteObj)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write remote object failed");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleBindDialogTarget(MessageParcel& data, MessageParcel& reply)
{
    WLOGFI("run HandleBindDialogTarget!");
    uint64_t persistentId = 0;
    if (!data.ReadUint64(persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read persistentId failed");
        return ERR_TRANSACTION_FAILED;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    WSError ret = BindDialogSessionTarget(persistentId, remoteObject);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleNotifyDumpInfoResult(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DEFAULT, "Enter");
    uint32_t vectorSize;
    if (!data.ReadUint32(vectorSize)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to read vectorSize");
        return ERR_INVALID_DATA;
    }
    if (vectorSize > MAX_VECTOR_SIZE) {
        TLOGE(WmsLogTag::DEFAULT, "Vector is too big!");
        return ERR_INVALID_DATA;
    }
    std::vector<std::string> info;
    for (uint32_t i = 0; i < vectorSize; i++) {
        uint32_t curSize = data.ReadUint32();
        std::string curInfo = "";
        if (curSize != 0) {
            const char* infoPtr = nullptr;
            infoPtr = reinterpret_cast<const char*>(data.ReadRawData(curSize));
            curInfo = (infoPtr) ? std::string(infoPtr, curSize) : "";
        }
        info.emplace_back(curInfo);
        TLOGD(WmsLogTag::DEFAULT, "InfoResult count: %{public}u, infoSize: %{public}u", i, curSize);
    }
    NotifyDumpInfoResult(info);
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleClearSession(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In!");
    int32_t persistentId;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to read persistentId");
        return ERR_INVALID_DATA;
    }
    WSError ret = ClearSession(persistentId);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleClearAllSessions(MessageParcel& data, MessageParcel& reply)
{
    WLOGFI("run HandleClearAllSessions!");
    WSError ret = ClearAllSessions();
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleLockSession(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In!");
    int32_t sessionId;
    if (!data.ReadInt32(sessionId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to read sessionId");
        return ERR_INVALID_DATA;
    }
    WSError ret = LockSession(sessionId);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}
int SceneSessionManagerStub::HandleUnlockSession(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In!");
    int32_t sessionId;
    if (!data.ReadInt32(sessionId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to read sessionId");
        return ERR_INVALID_DATA;
    }
    WSError ret = UnlockSession(sessionId);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}
int SceneSessionManagerStub::HandleMoveSessionsToForeground(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In!");
    std::vector<int32_t> sessionIds;
    data.ReadInt32Vector(&sessionIds);
    int32_t topSessionId;
    if (!data.ReadInt32(topSessionId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to read topSessionId");
        return ERR_INVALID_DATA;
    }
    const WSError &ret = MoveSessionsToForeground(sessionIds, topSessionId);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}
int SceneSessionManagerStub::HandleMoveSessionsToBackground(MessageParcel& data, MessageParcel& reply)
{
    WLOGFI("run HandleMoveSessionsToBackground!");
    std::vector<int32_t> sessionIds;
    data.ReadInt32Vector(&sessionIds);
    std::vector<int32_t> result;
    data.ReadInt32Vector(&result);
    const WSError &ret = MoveSessionsToBackground(sessionIds, result);
    reply.WriteInt32Vector(result);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRegisterCollaborator(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In!");
    int32_t type;
    if (!data.ReadInt32(type)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to read type");
        return ERR_INVALID_DATA;
    }
    sptr<AAFwk::IAbilityManagerCollaborator> collaborator =
        iface_cast<AAFwk::IAbilityManagerCollaborator>(data.ReadRemoteObject());
    if (collaborator == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "collaborator is nullptr");
        return ERR_INVALID_DATA;
    }
    WSError ret = RegisterIAbilityManagerCollaborator(type, collaborator);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUnregisterCollaborator(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In!");
    int32_t type;
    if (!data.ReadInt32(type)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to read type");
        return ERR_INVALID_DATA;
    }
    WSError ret = UnregisterIAbilityManagerCollaborator(type);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUpdateSessionTouchOutsideListener(MessageParcel& data, MessageParcel& reply)
{
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "read persistentId failed");
        return ERR_INVALID_DATA;
    }
    bool haveAvoidAreaListener = false;
    if (!data.ReadBool(haveAvoidAreaListener)) {
        TLOGE(WmsLogTag::WMS_INPUT_KEY_FLOW, "read haveAvoidAreaListener fail");
        return ERR_INVALID_DATA;
    }
    WSError errCode = UpdateSessionTouchOutsideListener(persistentId, haveAvoidAreaListener);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRaiseWindowToTop(MessageParcel& data, MessageParcel& reply)
{
    auto persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "read persistentId failed");
        return ERR_INVALID_DATA;
    }
    WSError errCode = RaiseWindowToTop(persistentId);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleNotifyWindowExtensionVisibilityChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "In!");
    int32_t pid;
    if (!data.ReadInt32(pid)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read pid failed");
        return ERR_INVALID_DATA;
    }
    int32_t uid;
    if (!data.ReadInt32(uid)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read uid failed");
        return ERR_INVALID_DATA;
    }
    bool visible;
    if (!data.ReadBool(visible)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read visible failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_UIEXT, "pid:%{public}d, uid:%{public}d, visible:%{public}d", pid, uid, visible);
    WSError ret = NotifyWindowExtensionVisibilityChange(pid, uid, visible);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetTopWindowId(MessageParcel& data, MessageParcel& reply)
{
    uint32_t mainWinId = 0;
    if (!data.ReadUint32(mainWinId)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "read mainWinId failed");
        return ERR_INVALID_DATA;
    }
    uint32_t topWinId = 0;
    WMError ret = GetTopWindowId(mainWinId, topWinId);
    reply.WriteUint32(topWinId);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetParentMainWindowId(MessageParcel& data, MessageParcel& reply)
{
    int32_t windowId = INVALID_SESSION_ID;
    if (!data.ReadInt32(windowId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "read windowId failed");
        return ERR_INVALID_DATA;
    }
    int32_t mainWindowId = INVALID_SESSION_ID;
    WMError errCode = GetParentMainWindowId(windowId, mainWindowId);
    if (!reply.WriteInt32(mainWindowId)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleWatchGestureConsumeResult(MessageParcel& data, MessageParcel& reply)
{
    int32_t keyCode = 0;
    if (!data.ReadInt32(keyCode)) {
        TLOGE(WmsLogTag::WMS_EVENT, "read keyCode failed");
        return ERR_INVALID_DATA;
    }
    bool isConsumed = false;
    if (!data.ReadBool(isConsumed)) {
        TLOGE(WmsLogTag::WMS_EVENT, "read isConsumed failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_EVENT, "isConsumed:%{public}d", isConsumed);
    WMError ret = NotifyWatchGestureConsumeResult(keyCode, isConsumed);
    if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleWatchFocusActiveChange(MessageParcel& data, MessageParcel& reply)
{
    bool isActive = false;
    if (!data.ReadBool(isActive)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "read isActive failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_EVENT, "isActive:%{public}d", isActive);
    WMError ret = NotifyWatchFocusActiveChange(isActive);
    if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUpdateSessionWindowVisibilityListener(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Handled!");
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read persistentId fail");
        return ERR_INVALID_DATA;
    }
    bool haveListener = false;
    if (!data.ReadBool(haveListener)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read haveListener fail");
        return ERR_INVALID_DATA;
    }
    WSError ret = UpdateSessionWindowVisibilityListener(persistentId, haveListener);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUpdateSessionOcclusionStateListener(MessageParcel& data, MessageParcel& reply)
{
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read persistentId fail");
        return ERR_INVALID_DATA;
    }
    bool haveListener = false;
    if (!data.ReadBool(haveListener)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read haveListener fail");
        return ERR_INVALID_DATA;
    }
    auto errCode = UpdateSessionOcclusionStateListener(persistentId, haveListener);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write error code failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleShiftAppWindowFocus(MessageParcel& data, MessageParcel& reply)
{
    int32_t sourcePersistentId = 0;
    int32_t targetPersistentId = 0;
    if (!data.ReadInt32(sourcePersistentId) || !data.ReadInt32(targetPersistentId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "read sourcePersistentId or targetPersistentId failed");
        return ERR_INVALID_DATA;
    }
    WSError ret = ShiftAppWindowFocus(sourcePersistentId, targetPersistentId);
    reply.WriteUint32(static_cast<uint32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleListWindowInfo(MessageParcel& data, MessageParcel& reply)
{
    WindowInfoOption windowInfoOption;
    uint8_t windowInfoFilterOptionValue = static_cast<WindowInfoFilterOptionDataType>(WindowInfoFilterOption::ALL);
    if (!data.ReadUint8(windowInfoFilterOptionValue)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to read windowInfoFilterOption");
        return ERR_INVALID_DATA;
    }
    windowInfoOption.windowInfoFilterOption = static_cast<WindowInfoFilterOption>(windowInfoFilterOptionValue);
    uint8_t windowInfoTypeOptionValue = static_cast<WindowInfoTypeOptionDataType>(WindowInfoTypeOption::ALL);
    if (!data.ReadUint8(windowInfoTypeOptionValue)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to read windowInfoTypeOption");
        return ERR_INVALID_DATA;
    }
    windowInfoOption.windowInfoTypeOption = static_cast<WindowInfoTypeOption>(windowInfoTypeOptionValue);
    if (!data.ReadUint64(windowInfoOption.displayId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to read displayId");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadInt32(windowInfoOption.windowId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to read windowId");
        return ERR_INVALID_DATA;
    }
    std::vector<sptr<WindowInfo>> infos;
    WMError errCode = ListWindowInfo(windowInfoOption, infos);
    if (!MarshallingHelper::MarshallingVectorParcelableObj<WindowInfo>(reply, infos)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to write window info");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write errCode fail");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetAllWindowLayoutInfo(MessageParcel& data, MessageParcel& reply)
{
    uint64_t displayId = 0;
    if (!data.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to read displayId");
        return ERR_INVALID_DATA;
    }
    std::vector<sptr<WindowLayoutInfo>> infos;
    WMError errCode = GetAllWindowLayoutInfo(displayId, infos);
    if (!MarshallingHelper::MarshallingVectorParcelableObj<WindowLayoutInfo>(reply, infos)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to write window layout info");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write errCode fail");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetAllMainWindowInfo(MessageParcel& data, MessageParcel& reply)
{
    std::vector<sptr<MainWindowInfo>> infos;
    WMError errCode = GetAllMainWindowInfo(infos);
    if (!MarshallingHelper::MarshallingVectorParcelableObj<MainWindowInfo>(reply, infos)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to write main window info");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write errCode fail");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}
 
int SceneSessionManagerStub::HandleGetMainWindowSnapshot(MessageParcel& data, MessageParcel& reply)
{
    std::vector<int32_t> windowIds;
    if (!data.ReadInt32Vector(&windowIds)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read windowIds Failed");
        return ERR_INVALID_DATA;
    }
    WindowSnapshotConfiguration config;
    if (!data.ReadBool(config.useCache)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read useCache failed!");
        return ERR_INVALID_DATA;
    }
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "callback is null");
        return ERR_INVALID_DATA;
    }
    WMError errCode = GetMainWindowSnapshot(windowIds, config, callback);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write errCode fail");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetGlobalWindowMode(MessageParcel& data, MessageParcel& reply)
{
    uint64_t displayId = 0;
    if (!data.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to read displayId");
        return ERR_INVALID_DATA;
    }
    GlobalWindowMode globalWinMode = GlobalWindowMode::UNKNOWN;
    if (!reply.WriteInt32(static_cast<int32_t>(GetGlobalWindowMode(displayId, globalWinMode)))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write errCode fail");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteUint32(static_cast<uint32_t>(globalWinMode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to write global window mode");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetTopNavDestinationName(MessageParcel& data, MessageParcel& reply)
{
    int32_t windowId = 0;
    if (!data.ReadInt32(windowId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read windowId fail");
        return ERR_INVALID_DATA;
    }
    std::string topNavDestName;
    WMError errCode = GetTopNavDestinationName(windowId, topNavDestName);
    if (errCode != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "get top page name failed");
    }
    uint32_t size = static_cast<uint32_t>(topNavDestName.length());
    if (!reply.WriteUint32(size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write the size of top page name failed");
        return ERR_INVALID_DATA;
    }
    if (size > 0 && !reply.WriteRawData(topNavDestName.c_str(), size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write top page name failed");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write error code failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetWatermarkImageForApp(MessageParcel& data, MessageParcel& reply)
{
    std::shared_ptr<Media::PixelMap> pixelMap(data.ReadParcelable<Media::PixelMap>());
    std::string watermarkName;
    WMError errCode = SetWatermarkImageForApp(pixelMap, watermarkName);
    if (errCode != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "set app watermark failed");
    }
    uint32_t size = static_cast<uint32_t>(watermarkName.length());
    if (!reply.WriteUint32(size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write the size of watermark name failed");
        return ERR_INVALID_DATA;
    }
    if (size > 0 && !reply.WriteRawData(watermarkName.c_str(), size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write watermark name failed");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write error code failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRecoverWatermarkImageForApp(MessageParcel& data, MessageParcel& reply)
{
    std::string watermarkName;
    if (!data.ReadString(watermarkName)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "get watermark name failed");
        return ERR_INVALID_DATA;
    }
    WMError errCode = RecoverWatermarkImageForApp(watermarkName);
    if (errCode != WMError::WM_OK) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "recover failed, watermarkName=%{public}s", watermarkName.c_str());
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write error code failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetVisibilityWindowInfo(MessageParcel& data, MessageParcel& reply)
{
    std::vector<sptr<WindowVisibilityInfo>> infos;
    WMError errCode = GetVisibilityWindowInfo(infos);
    if (!MarshallingHelper::MarshallingVectorParcelableObj<WindowVisibilityInfo>(reply, infos)) {
        WLOGFE("Write visibility window infos failed");
        return ERR_INVALID_DATA;
    }
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleAddExtensionWindowStageToSCB(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "In!");
    sptr<IRemoteObject> sessionStageObject = data.ReadRemoteObject();
    sptr<ISessionStage> sessionStage = iface_cast<ISessionStage>(sessionStageObject);
    if (sessionStage == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "sessionStage is nullptr");
        return ERR_INVALID_DATA;
    }
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    uint64_t surfaceNodeId;
    if (!data.ReadUint64(surfaceNodeId)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read surfaceNodeId failed");
        return ERR_INVALID_DATA;
    }
    int64_t startModalExtensionTimeStamp = -1;
    if (!data.ReadInt64(startModalExtensionTimeStamp)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read startModalExtensionTimeStamp failed");
        return ERR_INVALID_DATA;
    }
    bool isConstrainedModal = false;
    if (!data.ReadBool(isConstrainedModal)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read isConstrainedModal failed");
        return ERR_INVALID_DATA;
    }
    AddExtensionWindowStageToSCB(sessionStage, token, surfaceNodeId, startModalExtensionTimeStamp,
        isConstrainedModal);
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRemoveExtensionWindowStageFromSCB(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> sessionStageObject = data.ReadRemoteObject();
    sptr<ISessionStage> sessionStage = iface_cast<ISessionStage>(sessionStageObject);
    if (sessionStage == nullptr) {
        WLOGFE("sessionStage is nullptr");
        return ERR_INVALID_DATA;
    }
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    bool isConstrainedModal = false;
    if (!data.ReadBool(isConstrainedModal)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read isConstrainedModal failed");
        return ERR_INVALID_DATA;
    }
    RemoveExtensionWindowStageFromSCB(sessionStage, token, isConstrainedModal);
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUpdateModalExtensionRect(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "In!");
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (token == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "token is nullptr");
        return ERR_INVALID_DATA;
    }
    int32_t rectX;
    if (!data.ReadInt32(rectX)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read rectX failed");
        return ERR_INVALID_DATA;
    }
    int32_t rectY;
    if (!data.ReadInt32(rectY)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read rectY failed");
        return ERR_INVALID_DATA;
    }
    int32_t rectWidth;
    if (!data.ReadInt32(rectWidth)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read rectWidth failed");
        return ERR_INVALID_DATA;
    }
    int32_t rectHeight;
    if (!data.ReadInt32(rectHeight)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read rectHeight failed");
        return ERR_INVALID_DATA;
    }
    Rect windowRect{rectX, rectY, rectWidth, rectHeight};
    UpdateModalExtensionRect(token, windowRect);
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleProcessModalExtensionPointDown(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "In!");
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (token == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "token is nullptr");
        return ERR_INVALID_DATA;
    }
    int32_t posX;
    if (!data.ReadInt32(posX)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read posX failed");
        return ERR_INVALID_DATA;
    }
    int32_t posY;
    if (!data.ReadInt32(posY)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read posY failed");
        return ERR_INVALID_DATA;
    }
    ProcessModalExtensionPointDown(token, posX, posY);
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleAddOrRemoveSecureSession(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "In!");
    int32_t persistentId;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read persistentId failed");
        return ERR_INVALID_DATA;
    }
    bool shouldHide;
    if (!data.ReadBool(shouldHide)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read shouldHide failed");
        return ERR_INVALID_DATA;
    }
    WSError ret = AddOrRemoveSecureSession(persistentId, shouldHide);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUpdateExtWindowFlags(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "In!");
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (token == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "token is nullptr");
        return ERR_INVALID_DATA;
    }
    uint32_t extWindowFlags;
    if (!data.ReadUint32(extWindowFlags)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read extWindowFlags failed");
        return ERR_INVALID_DATA;
    }
    uint32_t extWindowActions;
    if (!data.ReadUint32(extWindowActions)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read extWindowActions failed");
        return ERR_INVALID_DATA;
    }
    WSError ret = UpdateExtWindowFlags(token, extWindowFlags, extWindowActions);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetHostWindowRect(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "Handled!");
    int32_t hostWindowId = 0;
    if (!data.ReadInt32(hostWindowId)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read hostWindowId fail");
        return ERR_INVALID_DATA;
    }
    Rect rect;
    WSError ret = GetHostWindowRect(hostWindowId, rect);
    reply.WriteInt32(rect.posX_);
    reply.WriteInt32(rect.posY_);
    reply.WriteUint32(rect.width_);
    reply.WriteUint32(rect.height_);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetHostGlobalScaledRect(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "Handled!");
    int32_t hostWindowId = 0;
    if (!data.ReadInt32(hostWindowId)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read hostWindowId fail");
        return ERR_INVALID_DATA;
    }
    Rect rect;
    WSError ret = GetHostGlobalScaledRect(hostWindowId, rect);
    reply.WriteInt32(rect.posX_);
    reply.WriteInt32(rect.posY_);
    reply.WriteUint32(rect.width_);
    reply.WriteUint32(rect.height_);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetFreeMultiWindowEnableState(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "run HandleGetFreeMultiWindowEnableState!");
    bool enable = false;
    WSError ret = GetFreeMultiWindowEnableState(enable);
    reply.WriteBool(enable);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetCallingWindowWindowStatus(MessageParcel&data, MessageParcel&reply)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "In!");
    uint32_t callingWindowId;
    if (!data.ReadUint32(callingWindowId)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "read callingWindowId failed");
        return ERR_INVALID_DATA;
    }
    WindowStatus windowStatus = WindowStatus::WINDOW_STATUS_UNDEFINED;
    WMError ret = GetCallingWindowWindowStatus(callingWindowId, windowStatus);
    reply.WriteUint32(static_cast<int32_t>(ret));
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to GetCallingWindowWindowStatus(%{public}u)", callingWindowId);
        return ERR_INVALID_DATA;
    }
    reply.WriteUint32(static_cast<uint32_t>(windowStatus));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetCallingWindowRect(MessageParcel&data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "In!");
    uint32_t callingWindowId;
    if (!data.ReadUint32(callingWindowId)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "read callingWindowId failed");
        return ERR_INVALID_DATA;
    }
    Rect rect = {0, 0, 0, 0};
    WMError ret = GetCallingWindowRect(callingWindowId, rect);
    reply.WriteInt32(static_cast<int32_t>(ret));
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to GetCallingWindowRect(%{public}u)", callingWindowId);
        return ERR_INVALID_DATA;
    }
    reply.WriteInt32(rect.posX_);
    reply.WriteInt32(rect.posY_);
    reply.WriteUint32(rect.width_);
    reply.WriteUint32(rect.height_);
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetWindowModeType(MessageParcel& data, MessageParcel& reply)
{
    WindowModeType windowModeType = Rosen::WindowModeType::WINDOW_MODE_OTHER;
    WMError errCode = GetWindowModeType(windowModeType);
    WLOGFI("run HandleGetWindowModeType, windowModeType:%{public}d!", static_cast<int32_t>(windowModeType));
    if (!reply.WriteUint32(static_cast<int32_t>(windowModeType))) {
        WLOGE("Failed to WriteBool");
        return ERR_INVALID_DATA;
    }
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetWindowStyleType(MessageParcel& data, MessageParcel& reply)
{
    WindowStyleType windowStyleType = Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT;
    WMError errCode = GetWindowStyleType(windowStyleType);
    TLOGI(WmsLogTag::WMS_LIFE, "windowStyleType:%{public}d!", static_cast<int32_t>(windowStyleType));
    if (!reply.WriteUint32(static_cast<int32_t>(windowStyleType))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to WriteBool");
        return ERR_INVALID_DATA;
    }
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetProcessSurfaceNodeIdByPersistentId(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Handled!");
    int32_t pid = 0;
    if (!data.ReadInt32(pid)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to readInt32 pid");
        return ERR_INVALID_DATA;
    }
    std::vector<int32_t> persistentIds;
    if (!data.ReadInt32Vector(&persistentIds)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to readInt32Vector persistentIds");
        return ERR_INVALID_DATA;
    }
    std::vector<uint64_t> surfaceNodeIds;
    WMError errCode = GetProcessSurfaceNodeIdByPersistentId(pid, persistentIds, surfaceNodeIds);
    if (!reply.WriteUInt64Vector(surfaceNodeIds)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write surfaceNodeIds fail.");
        return ERR_INVALID_DATA;
    }
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSkipSnapshotForAppProcess(MessageParcel& data, MessageParcel& reply)
{
    int32_t pid = INVALID_PID;
    if (!data.ReadInt32(pid)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to readInt32 pid");
        return ERR_INVALID_DATA;
    }
    bool skip = false;
    if (!data.ReadBool(skip)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to readBool skip");
        return ERR_INVALID_DATA;
    }
    WMError errCode = SkipSnapshotForAppProcess(pid, skip);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSkipSnapshotByUserIdAndBundleNames(MessageParcel& data, MessageParcel& reply)
{
    int32_t userId = -1;
    if (!data.ReadInt32(userId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to readInt32 userId");
        return ERR_INVALID_DATA;
    }
    std::vector<std::string> bundleNameList;
    if (!data.ReadStringVector(&bundleNameList)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Fail to read bundleNameList");
        return ERR_INVALID_DATA;
    }
    WMError errCode = SkipSnapshotByUserIdAndBundleNames(userId, bundleNameList);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

// LCOV_EXCL_START
int SceneSessionManagerStub::HandleSetProcessWatermark(MessageParcel& data, MessageParcel& reply)
{
    int32_t pid = INVALID_PID;
    if (!data.ReadInt32(pid)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to readInt32 pid");
        return ERR_INVALID_DATA;
    }
    std::string watermarkName;
    if (!data.ReadString(watermarkName)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to readString watermarkName");
        return ERR_INVALID_DATA;
    }
    bool isEnabled = false;
    if (!data.ReadBool(isEnabled)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to readBool isEnabled");
        return ERR_INVALID_DATA;
    }
    WMError errCode = SetProcessWatermark(pid, watermarkName, isEnabled);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetWindowIdsByCoordinate(MessageParcel& data, MessageParcel& reply)
{
    uint64_t displayId;
    if (!data.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::DEFAULT, "read displayId failed");
        return ERR_INVALID_DATA;
    }
    int32_t windowNumber;
    if (!data.ReadInt32(windowNumber)) {
        TLOGE(WmsLogTag::DEFAULT, "read windowNumber failed");
        return ERR_INVALID_DATA;
    }
    int32_t x;
    int32_t y;
    if (!data.ReadInt32(x) || !data.ReadInt32(y)) {
        TLOGE(WmsLogTag::DEFAULT, "read coordinate failed");
        return ERR_INVALID_DATA;
    }
    std::vector<int32_t> windowIds;
    WMError errCode = GetWindowIdsByCoordinate(displayId, windowNumber, x, y, windowIds);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed.");
        return ERR_INVALID_DATA;
    }
    reply.WriteInt32Vector(windowIds);
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUpdateSessionScreenLock(MessageParcel& data, MessageParcel& reply)
{
    std::string bundleName;
    if (!data.ReadString(bundleName)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read bundleName failed.");
        return ERR_INVALID_DATA;
    }
    bool isRelease = false;
    if (!data.ReadBool(isRelease)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read isRelease failed.");
        return ERR_INVALID_DATA;
    }
    WMError errCode = UpdateScreenLockStatusForApp(bundleName, isRelease);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleAddSkipSelfWhenShowOnVirtualScreenList(MessageParcel& data, MessageParcel& reply)
{
    uint64_t size = 0;
    if (!data.ReadUint64(size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read size failed.");
        return ERR_INVALID_DATA;
    }
    static constexpr uint64_t MAX_SIZE = 100;
    if (size > MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Size too large.");
        return ERR_INVALID_DATA;
    }
    std::vector<int32_t> persistentIds;
    for (uint64_t i = 0; i < size; i++) {
        int32_t persistentId = 0;
        if (!data.ReadInt32(persistentId)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read persistentId failed.");
            return ERR_INVALID_DATA;
        }
        persistentIds.push_back(persistentId);
    }
    WMError errCode = AddSkipSelfWhenShowOnVirtualScreenList(persistentIds);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRemoveSkipSelfWhenShowOnVirtualScreenList(MessageParcel& data, MessageParcel& reply)
{
    uint64_t size = 0;
    if (!data.ReadUint64(size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read size failed.");
        return ERR_INVALID_DATA;
    }
    static constexpr uint64_t MAX_SIZE = 100;
    if (size > MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Size too large.");
        return ERR_INVALID_DATA;
    }
    std::vector<int32_t> persistentIds;
    for (uint64_t i = 0; i < size; i++) {
        int32_t persistentId = 0;
        if (!data.ReadInt32(persistentId)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read persistentId failed.");
            return ERR_INVALID_DATA;
        }
        persistentIds.push_back(persistentId);
    }
    WMError errCode = RemoveSkipSelfWhenShowOnVirtualScreenList(persistentIds);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetScreenPrivacyWindowTagSwitch(MessageParcel& data, MessageParcel& reply)
{
    uint64_t screenId = INVALID_SCREEN_ID;
    if (!data.ReadUint64(screenId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read screenId failed.");
        return ERR_INVALID_DATA;
    }

    uint64_t size = 0;
    if (!data.ReadUint64(size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read size failed.");
        return ERR_INVALID_DATA;
    }

    if (size > MAX_VECTOR_SIZE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Vector is too big, size is %{public}" PRIu64, size);
        return ERR_INVALID_DATA;
    }

    std::vector<std::string> privacyWindowTags;
    for (uint64_t i = 0; i < size; i++) {
        std::string privacyWidnowTag;
        if (!data.ReadString(privacyWidnowTag)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read privacyWidnowTag failed.");
            return ERR_INVALID_DATA;
        }
        privacyWindowTags.push_back(privacyWidnowTag);
    }

    bool enable;
    if (!data.ReadBool(enable)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read enable failed.");
        return ERR_INVALID_DATA;
    }

    WMError errCode = SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleIsPcWindow(MessageParcel& data, MessageParcel& reply)
{
    bool isPcWindow = false;
    WMError errCode = IsPcWindow(isPcWindow);
    if (!reply.WriteBool(isPcWindow)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write isPcWindow fail.");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleIsFreeMultiWindow(MessageParcel& data, MessageParcel& reply)
{
    bool isFreeMultiWindow = false;
    WMError errCode = IsFreeMultiWindow(isFreeMultiWindow);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PC, "Write errorCode fail.");
        return ERR_INVALID_DATA;
    }

    if (WMError::WM_OK == errCode && !reply.WriteBool(isFreeMultiWindow)) {
        TLOGE(WmsLogTag::WMS_PC, "Write isFreeMultiWindow fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleIsPcOrPadFreeMultiWindowMode(MessageParcel& data, MessageParcel& reply)
{
    bool isPcOrPadFreeMultiWindowMode = false;
    WMError errCode = IsPcOrPadFreeMultiWindowMode(isPcOrPadFreeMultiWindowMode);
    if (!reply.WriteBool(isPcOrPadFreeMultiWindowMode)) {
        TLOGE(WmsLogTag::WMS_SUB, "Write isPcOrPadFreeMultiWindowMode fail.");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_SUB, "Write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetDisplayIdByWindowId(MessageParcel& data, MessageParcel& reply)
{
    std::vector<uint64_t> windowIds;
    if (!data.ReadUInt64Vector(&windowIds)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read windowIds Failed");
        return ERR_INVALID_DATA;
    }
    std::unordered_map<uint64_t, DisplayId> windowDisplayIdMap;
    WMError errCode = GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
    if (!reply.WriteInt32(static_cast<int32_t>(windowDisplayIdMap.size()))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write windowDisplayIdMap size faild");
        return ERR_INVALID_DATA;
    }
    for (auto it = windowDisplayIdMap.begin(); it != windowDisplayIdMap.end(); ++it) {
        if (!reply.WriteUint64(it->first)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write windowId failed");
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteUint64(it->second)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write displayId failed");
            return ERR_INVALID_DATA;
        }
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}
// LCOV_EXCL_STOP

int SceneSessionManagerStub::HandleIsWindowRectAutoSave(MessageParcel& data, MessageParcel& reply)
{
    std::string key;
    if (!data.ReadString(key)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Read key failed.");
        return ERR_INVALID_DATA;
    }
    int persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Read persistentId failed.");
        return ERR_INVALID_DATA;
    }
    bool enabled = false;
    WMError errCode = IsWindowRectAutoSave(key, enabled, persistentId);
    if (!reply.WriteBool(enabled)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write enabled failed.");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteUint32(static_cast<uint32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetImageForRecent(MessageParcel& data, MessageParcel& reply)
{
    uint32_t imgResourceId = 0;
    if (!data.ReadUint32(imgResourceId)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Read imgResourceId failed.");
        return ERR_INVALID_DATA;
    }
    uint32_t imageFit = 0;
    if (!data.ReadUint32(imageFit)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Read imageFit failed.");
        return ERR_INVALID_DATA;
    }
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Read persistentId failed.");
        return ERR_INVALID_DATA;
    }
    WMError errCode = SetImageForRecent(imgResourceId, static_cast<ImageFit>(imageFit), persistentId);
    if (!reply.WriteUint32(static_cast<uint32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetImageForRecentPixelMap(MessageParcel& data, MessageParcel& reply)
{
    std::shared_ptr<Media::PixelMap> pixelMap(data.ReadParcelable<Media::PixelMap>());
    if (!pixelMap) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Read pixelMap failed.");
        return ERR_INVALID_DATA;
    }
    uint32_t imageFit = 0;
    if (!data.ReadUint32(imageFit)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Read imageFit failed.");
        return ERR_INVALID_DATA;
    }
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Read persistentId failed.");
        return ERR_INVALID_DATA;
    }
    WMError errCode = SetImageForRecentPixelMap(pixelMap, static_cast<ImageFit>(imageFit), persistentId);
    if (!reply.WriteUint32(static_cast<uint32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRemoveImageForRecent(MessageParcel& data, MessageParcel& reply)
{
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Read persistentId failed.");
        return ERR_INVALID_DATA;
    }
    WMError errCode = RemoveImageForRecent(persistentId);
    if (!reply.WriteUint32(static_cast<uint32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetGlobalDragResizeType(MessageParcel& data, MessageParcel& reply)
{
    uint32_t dragResizeType;
    if (!data.ReadUint32(dragResizeType)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read dragResizeType failed.");
        return ERR_INVALID_DATA;
    }
    if (dragResizeType >= static_cast<uint32_t>(DragResizeType::RESIZE_MAX_VALUE)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "bad dragResizeType value");
        return ERR_INVALID_DATA;
    }
    WMError errCode = SetGlobalDragResizeType(static_cast<DragResizeType>(dragResizeType));
    if (!reply.WriteUint32(static_cast<uint32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetGlobalDragResizeType(MessageParcel& data, MessageParcel& reply)
{
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    WMError errCode = GetGlobalDragResizeType(dragResizeType);
    if (!reply.WriteUint32(static_cast<uint32_t>(dragResizeType))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write dragResizeType failed.");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteUint32(static_cast<uint32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetAppDragResizeType(MessageParcel& data, MessageParcel& reply)
{
    std::string bundleName;
    if (!data.ReadString(bundleName)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read bundleName failed.");
        return ERR_INVALID_DATA;
    }
    uint32_t dragResizeType;
    if (!data.ReadUint32(dragResizeType)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read dragResizeType failed.");
        return ERR_INVALID_DATA;
    }
    if (dragResizeType >= static_cast<uint32_t>(DragResizeType::RESIZE_MAX_VALUE)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "bad dragResizeType value");
        return ERR_INVALID_DATA;
    }
    WMError errCode = SetAppDragResizeType(bundleName, static_cast<DragResizeType>(dragResizeType));
    if (!reply.WriteUint32(static_cast<uint32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetAppDragResizeType(MessageParcel& data, MessageParcel& reply)
{
    std::string bundleName;
    if (!data.ReadString(bundleName)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read bundleName failed.");
        return ERR_INVALID_DATA;
    }
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    WMError errCode = GetAppDragResizeType(bundleName, dragResizeType);
    if (!reply.WriteUint32(static_cast<uint32_t>(dragResizeType))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write dragResizeType failed.");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteUint32(static_cast<uint32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetAppKeyFramePolicy(MessageParcel& data, MessageParcel& reply)
{
    std::string bundleName;
    if (!data.ReadString(bundleName)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read bundleName failed.");
        return ERR_INVALID_DATA;
    }
    sptr<KeyFramePolicy> keyFramePolicy = data.ReadParcelable<KeyFramePolicy>();
    if (!keyFramePolicy) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read keyFramePolicy failed.");
        return ERR_INVALID_DATA;
    }
    WMError errCode = SetAppKeyFramePolicy(bundleName, *keyFramePolicy);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

// LCOV_EXCL_START
int SceneSessionManagerStub::HandleShiftAppWindowPointerEvent(MessageParcel& data, MessageParcel& reply)
{
    int32_t sourcePersistentId = INVALID_WINDOW_ID;
    if (!data.ReadInt32(sourcePersistentId)) {
        TLOGE(WmsLogTag::WMS_PC, "read sourcePersistentId failed");
        return ERR_INVALID_DATA;
    }
    int32_t targetPersistentId = INVALID_WINDOW_ID;
    if (!data.ReadInt32(targetPersistentId)) {
        TLOGE(WmsLogTag::WMS_PC, "read targetPersistentId failed");
        return ERR_INVALID_DATA;
    }
    int32_t fingerId = INVALID_FINGER_ID;
    if (!data.ReadInt32(fingerId)) {
        TLOGE(WmsLogTag::WMS_PC, "read fingerId failed");
        return ERR_INVALID_DATA;
    }
    WMError errCode = ShiftAppWindowPointerEvent(sourcePersistentId, targetPersistentId, fingerId);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleNotifyScreenshotEvent(MessageParcel& data, MessageParcel& reply)
{
    int32_t screenshotEventType = static_cast<int32_t>(ScreenshotEventType::END);
    if (!data.ReadInt32(screenshotEventType)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to read screenshotEventType");
        return ERR_INVALID_DATA;
    }
    WMError errCode = NotifyScreenshotEvent(static_cast<ScreenshotEventType>(screenshotEventType));
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write errCode fail");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetStartWindowBackgroundColor(MessageParcel& data, MessageParcel& reply)
{
    std::string moduleName;
    if (!data.ReadString(moduleName)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "read moduleName failed");
        return ERR_INVALID_DATA;
    }
    std::string abilityName;
    if (!data.ReadString(abilityName)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "read abilityName failed");
        return ERR_INVALID_DATA;
    }
    uint32_t color;
    if (!data.ReadUint32(color)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "read color failed");
        return ERR_INVALID_DATA;
    }
    int32_t uid;
    if (!data.ReadInt32(uid)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "read uid failed");
        return ERR_INVALID_DATA;
    }
    WMError errCode = SetStartWindowBackgroundColor(moduleName, abilityName, color, uid);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}
// LCOV_EXCL_STOP

int SceneSessionManagerStub::HandleMinimizeByWindowId(MessageParcel& data, MessageParcel& reply)
{
    std::vector<int32_t> windowIds;
    if (!data.ReadInt32Vector(&windowIds)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read windowIds failed");
        return ERR_INVALID_DATA;
    }
    WMError errCode = MinimizeByWindowId(windowIds);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleMinimizeAllAppWindows(MessageParcel& data, MessageParcel& reply)
{
    DisplayId displayId = 0;
    if (!data.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read displayId failed.");
        return ERR_INVALID_DATA;
    }
    int32_t excludeWindowId = 0;
    if (!data.ReadInt32(excludeWindowId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read excludeWindowId failed.");
        return ERR_INVALID_DATA;
    }
    WMError errCode = MinimizeAllAppWindows(displayId, excludeWindowId);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetForegroundWindowNum(MessageParcel& data, MessageParcel& reply)
{
    uint32_t windowNum = 0;
    if (!data.ReadUint32(windowNum)) {
        TLOGE(WmsLogTag::WMS_PC, "read windowNum failed");
        return ERR_INVALID_DATA;
    }
    WMError errCode = SetForegroundWindowNum(windowNum);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PC, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUseImplicitAnimation(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "Handled!");
    int32_t hostWindowId = 0;
    if (!data.ReadInt32(hostWindowId)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "read hostWindowId fail");
        return ERR_INVALID_DATA;
    }
    bool useImplicit = false;
    if (!data.ReadBool(useImplicit)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read useImplicit failed!");
        return ERR_INVALID_DATA;
    }
    WSError errCode = UseImplicitAnimation(hostWindowId, useImplicit);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

// LCOV_EXCL_START
int SceneSessionManagerStub::HandleAnimateTo(MessageParcel& data, MessageParcel& reply)
{
    int32_t windowId = 0;
    if (!data.ReadInt32(windowId)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Read windowId failed");
        return ERR_INVALID_DATA;
    }
    sptr<WindowAnimationProperty> animationProperty = data.ReadStrongParcelable<WindowAnimationProperty>();
    if (animationProperty == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Read animationProperty failed");
        return ERR_INVALID_DATA;
    }
    sptr<WindowAnimationOption> animationOption = data.ReadStrongParcelable<WindowAnimationOption>();
    if (animationOption == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Read animationOption failed");
        return ERR_INVALID_DATA;
    }
    WMError errCode = AnimateTo(windowId, *animationProperty, *animationOption);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write errCode failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}
int SceneSessionManagerStub::HandleCreateUIEffectController(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> controllerObject = data.ReadRemoteObject();
    sptr<IUIEffectControllerClient> controllerClient = iface_cast<IUIEffectControllerClient>(controllerObject);
    if (controllerObject == nullptr || controllerClient == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "get client controller failed");
        return ERR_INVALID_DATA;
    }
    sptr<IUIEffectController> controller;
    int32_t controllerId = -1;
    WMError errCode = CreateUIEffectController(controllerClient, controller, controllerId);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    if (errCode != WMError::WM_OK) {
        return ERR_NONE;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(controllerId))) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write id failed.");
        return ERR_INVALID_DATA;
    }
    if (controller == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "controller is nullptr.");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteRemoteObject(controller->AsObject())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write controller failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleAddSessionBlackList(MessageParcel& data, MessageParcel& reply)
{
    uint64_t size = 0;
    if (!data.ReadUint64(size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read size failed");
        return ERR_INVALID_DATA;
    }
    if (size > MAX_VECTOR_SIZE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Vector is too big, size is %{public}" PRIu64, size);
        return ERR_INVALID_DATA;
    }
    std::unordered_set<std::string> bundleNames;
    for (uint64_t i = 0; i < size; i++) {
        std::string bundleName;
        if (!data.ReadString(bundleName)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read windowId failed");
            return ERR_INVALID_DATA;
        }
        bundleNames.insert(bundleName);
    }

    size = 0;
    if (!data.ReadUint64(size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read size failed");
        return ERR_INVALID_DATA;
    }
    if (size > MAX_VECTOR_SIZE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Vector is too big, size is %{public}" PRIu64, size);
        return ERR_INVALID_DATA;
    }
    std::unordered_set<std::string> privacyWindowTags;
    for (uint64_t i = 0; i < size; i++) {
        std::string privacyWindowTag = 0;
        if (!data.ReadString(privacyWindowTag)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read privacyWindowTag failed");
            return ERR_INVALID_DATA;
        }
        privacyWindowTags.insert(privacyWindowTag);
    }
    WMError errCode = AddSessionBlackList(bundleNames, privacyWindowTags);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write errCode failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleRemoveSessionBlackList(MessageParcel& data, MessageParcel& reply)
{
    uint64_t size = 0;
    if (!data.ReadUint64(size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read size failed");
        return ERR_INVALID_DATA;
    }
    if (size > MAX_VECTOR_SIZE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Vector is too big, size is %{public}" PRIu64, size);
        return ERR_INVALID_DATA;
    }
    std::unordered_set<std::string> bundleNames;
    for (uint64_t i = 0; i < size; i++) {
        std::string bundleName;
        if (!data.ReadString(bundleName)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read bundleName failed");
            return ERR_INVALID_DATA;
        }
        bundleNames.insert(bundleName);
    }

    size = 0;
    if (!data.ReadUint64(size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read size failed");
        return ERR_INVALID_DATA;
    }
    if (size > MAX_VECTOR_SIZE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Vector is too big, size is %{public}" PRIu64, size);
        return ERR_INVALID_DATA;
    }
    std::unordered_set<std::string> privacyWindowTags;
    for (uint64_t i = 0; i < size; i++) {
        std::string privacyWindowTag = 0;
        if (!data.ReadString(privacyWindowTag)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read privacyWindowTag failed");
            return ERR_INVALID_DATA;
        }
        privacyWindowTags.insert(privacyWindowTag);
    }
    WMError errCode = RemoveSessionBlackList(bundleNames, privacyWindowTags);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write errCode failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}
// LCOV_EXCL_STOP

int SceneSessionManagerStub::HandleGetPiPSettingSwitchStatus(MessageParcel& data, MessageParcel& reply)
{
    bool switchStatus = false;
    WMError errCode = GetPiPSettingSwitchStatus(switchStatus);
    if (!reply.WriteBool(switchStatus)) {
        TLOGE(WmsLogTag::WMS_PIP, "Write switchStatus fail.");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PIP, "Write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleGetIsPipEnabled(MessageParcel& data, MessageParcel& reply)
{
    bool isPipEnabled = false;
    WMError errCode = GetIsPipEnabled(isPipEnabled);
    if (!reply.WriteBool(isPipEnabled)) {
        TLOGE(WmsLogTag::WMS_PIP, "Write isPipEnabled fail.");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PIP, "Write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleUpdateOutline(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (!remoteObject) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Read remote object failed.");
        return ERR_INVALID_DATA;
    }
    sptr<OutlineParams> outlineParams = data.ReadParcelable<OutlineParams>();
    if (!outlineParams) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Read outline params failed.");
        return ERR_INVALID_DATA;
    }
    WMError ret = UpdateOutline(remoteObject, *outlineParams);
    if (!reply.WriteUint32(static_cast<uint32_t>(ret))) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleConvertToRelativeCoordinateExtended(MessageParcel& data, MessageParcel& reply)
{
    int32_t posX_ = 0;
    int32_t posY_ = 0;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    DisplayId newDisplayId = 0;
    if (!data.ReadInt32(posX_) || !data.ReadInt32(posY_) ||
        !data.ReadUint32(width_) || !data.ReadUint32(height_) ||
        !data.ReadUint64(newDisplayId)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read window infos failed.");
        return ERR_TRANSACTION_FAILED;
    }
    Rect rect = {posX_, posY_, width_, height_};
    Rect newRect;
    WMError errCode = ConvertToRelativeCoordinateExtended(rect, newRect, newDisplayId);
    if (!reply.WriteInt32(static_cast<int32_t>(newRect.posX_)) ||
        !reply.WriteInt32(static_cast<int32_t>(newRect.posY_)) ||
        !reply.WriteUint32(static_cast<uint32_t>(newRect.width_)) ||
        !reply.WriteUint32(static_cast<uint32_t>(newRect.height_))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write rect failed.");
        return ERR_TRANSACTION_FAILED;
    }
    if (!reply.WriteUint64(static_cast<uint64_t>(newDisplayId))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write newDisplayId failed.");
        return ERR_TRANSACTION_FAILED;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write errCode failed.");
        return ERR_TRANSACTION_FAILED;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleNotifySupportRotationRegistered(MessageParcel& data, MessageParcel& reply)
{
    NotifySupportRotationRegistered();
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleSetSpecificWindowZIndex(MessageParcel& data, MessageParcel& reply)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "in");
    uint32_t windowTypeValue = 0;
    if (!data.ReadUint32(windowTypeValue)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Read windowType failed");
        return ERR_INVALID_DATA;
    }
    WindowType windowType = static_cast<WindowType>(windowTypeValue);
    int32_t zIndex = 0;
    if (!data.ReadInt32(zIndex)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "read zIndex failed");
        return ERR_INVALID_DATA;
    }
    WSError ret = SetSpecificWindowZIndex(windowType, zIndex);
    if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write errCode fail");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SceneSessionManagerStub::HandleResetSpecificWindowZIndex(MessageParcel& data, MessageParcel& reply)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "in");
    int32_t pid = 0;
    if (!data.ReadInt32(pid)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Read pid fail");
        return ERR_INVALID_DATA;
    }
    WSError ret = ResetSpecificWindowZIndex(pid);
    if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write errCode fail");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}
} // namespace OHOS::Rosen