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

#include "session/host/include/zidl/session_stub.h"

#include "ability_start_setting.h"
#include <ipc_types.h>
#include <transaction/rs_transaction.h>
#include <feature/window_keyframe/rs_window_keyframe_node.h>
#include <ui/rs_surface_node.h>
#include "want.h"
#include "pointer_event.h"
#include "key_event.h"

#include "parcel/accessibility_event_info_parcel.h"
#include "process_options.h"
#include "start_window_option.h"
#include "session/host/include/zidl/session_ipc_interface_code.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS::Accessibility {
class AccessibilityEventInfo;
}
namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SessionStub" };
constexpr int32_t MAX_ABILITY_SESSION_INFOS = 4;

int ReadBasicAbilitySessionInfo(MessageParcel& data, sptr<AAFwk::SessionInfo> abilitySessionInfo)
{
    sptr<AAFwk::Want> localWant = data.ReadParcelable<AAFwk::Want>();
    if (localWant == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "localWant is nullptr");
        return ERR_INVALID_DATA;
    }
    abilitySessionInfo->want = *localWant;
    if (!data.ReadInt32(abilitySessionInfo->requestCode)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read requestCode failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadInt32(abilitySessionInfo->persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read persistentId failed.");
        return ERR_INVALID_DATA;
    }
    int32_t state = 0;
    if (!data.ReadInt32(state)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read state failed.");
        return ERR_INVALID_DATA;
    }
    abilitySessionInfo->state = static_cast<AAFwk::CallToState>(state);
    if (!data.ReadInt64(abilitySessionInfo->uiAbilityId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read uiAbilityId failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadUint32(abilitySessionInfo->callingTokenId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read callingTokenId failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadInt32(abilitySessionInfo->requestId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read requestId failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadInt32(abilitySessionInfo->specifiedReason)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read specifiedReason failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(abilitySessionInfo->reuse)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read reuse failed.");
        return ERR_INVALID_DATA;
    }
    abilitySessionInfo->processOptions.reset(data.ReadParcelable<AAFwk::ProcessOptions>());
    return ERR_NONE;
}
} // namespace

int SessionStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    WLOGFD("Scene session on remote request!, code: %{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WLOGFE("Failed to check interface token!");
        return ERR_TRANSACTION_FAILED;
    }

    return ProcessRemoteRequest(code, data, reply, option);
}

int SessionStub::ProcessRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    switch (code) {
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CONNECT):
            return HandleConnect(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_FOREGROUND):
            return HandleForeground(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_BACKGROUND):
            return HandleBackground(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_DISCONNECT):
            return HandleDisconnect(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SHOW):
            return HandleShow(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_HIDE):
            return HandleHide(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_DRAWING_COMPLETED):
            return HandleDrawingCompleted(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_APP_REMOVE_STARTING_WINDOW):
            return HandleRemoveStartingWindow(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_RECTCHANGE_LISTENER_REGISTERED):
            return HandleUpdateRectChangeListenerRegistered(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SESSION_EVENT):
            return HandleSessionEvent(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SYNC_SESSION_EVENT):
            return HandleSyncSessionEvent(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_SESSION_RECT):
            return HandleUpdateSessionRect(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_GLOBAL_SCALED_RECT):
            return HandleGetGlobalScaledRect(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RAISE_TO_APP_TOP):
            return HandleRaiseToAppTop(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_BACKPRESSED):
            return HandleBackPressed(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_MARK_PROCESSED):
            return HandleMarkProcessed(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_MAXIMIZE_MODE):
            return HandleSetGlobalMaximizeMode(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_MAXIMIZE_MODE):
            return HandleGetGlobalMaximizeMode(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NEED_AVOID):
            return HandleNeedAvoid(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_AVOID_AREA):
            return HandleGetAvoidAreaByType(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_AVOID_AREA_IGNORING_VISIBILITY):
            return HandleGetAvoidAreaByTypeIgnoringVisibility(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_ALL_AVOID_AREAS):
            return HandleGetAllAvoidAreas(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_TARGET_ORIENTATION_CONFIG_INFO):
            return HandleGetTargetOrientationConfigInfo(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CONVERT_ORIENTATION_AND_ROTATION):
            return HandleConvertOrientationAndRotation(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_ASPECT_RATIO):
            return HandleSetAspectRatio(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CONTENT_ASPECT_RATIO):
            return HandleSetContentAspectRatio(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_WINDOW_ANIMATION_FLAG):
            return HandleSetWindowAnimationFlag(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_CUSTOM_ANIMATION):
            return HandleUpdateWindowSceneAfterCustomAnimation(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_LANDSCAPE_MULTI_WINDOW):
            return HandleSetLandscapeMultiWindow(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_IS_MID_SCENE):
            return HandleGetIsMidScene(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RAISE_ABOVE_TARGET):
            return HandleRaiseAboveTarget(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RAISE_MAIN_WINDOW_ABOVE_TARGET):
            return HandleRaiseMainWindowAboveTarget(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RAISE_APP_MAIN_WINDOW):
            return HandleRaiseAppMainWindowToTop(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR):
            return HandleChangeSessionVisibilityWithStatusBar(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_ACTIVE_PENDING_SESSION):
            return HandlePendingSessionActivation(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_BATCH_ACTIVE_PENDING_SESSION):
            return HandleBatchPendingSessionsActivation(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RESTORE_MAIN_WINDOW):
            return HandleRestoreMainWindow(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RESTORE_FLOAT_MAIN_WINDOW):
            return HandleRestoreFloatMainWindow(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TERMINATE):
            return HandleTerminateSession(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_EXCEPTION):
            return HandleSessionException(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_PROCESS_POINT_DOWN_SESSION):
            return HandleProcessPointDownSession(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SEND_POINTEREVENT_FOR_MOVE_DRAG):
            return HandleSendPointerEvenForMoveDrag(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_IS_START_MOVING):
            return HandleIsStartMoving(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_SYSTEM_DRAG_ENABLE):
            return HandleSetSystemEnableDrag(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_CLIENT_RECT):
            return HandleUpdateClientRect(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CALLING_SESSION_ID):
            return HandleSetCallingSessionId(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CUSTOM_DECOR_HEIGHT):
            return HandleSetCustomDecorHeight(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_DECOR_VISIBLE):
            return HandleSetDecorVisible(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_SESSION_PROPERTY):
            return HandleUpdatePropertyByAction(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_ADJUST_KEYBOARD_LAYOUT):
            return HandleAdjustKeyboardLayout(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TRANSFER_ABILITY_RESULT):
            return HandleTransferAbilityResult(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TRANSFER_EXTENSION_DATA):
            return HandleTransferExtensionData(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_ASYNC_ON):
            return HandleNotifyAsyncOn(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_SYNC_ON):
            return HandleNotifySyncOn(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_EXTENSION_DIED):
            return HandleNotifyExtensionDied(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_EXTENSION_TIMEOUT):
            return HandleNotifyExtensionTimeout(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TRIGGER_BIND_MODAL_UI_EXTENSION):
            return HandleTriggerBindModalUIExtension(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_REPORT_ACCESSIBILITY_EVENT):
            return HandleTransferAccessibilityEvent(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_PIP_WINDOW_PREPARE_CLOSE):
            return HandleNotifyPiPWindowPrepareClose(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_PIP_RECT):
            return HandleUpdatePiPRect(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_PIP_CONTROL_STATUS):
            return HandleUpdatePiPControlStatus(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_AUTOSTART_PIP):
            return HandleSetAutoStartPiP(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_PIP_TEMPLATE_INFO):
            return HandleUpdatePiPTemplateInfo(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_PIP_PARENT_WINDOWID):
            return HandleSetPipParentWindowId(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_IS_PIP_ACTIVE):
            return HandleIsPiPActive(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_FLOATING_BALL):
            return HandleUpdateFloatingBall(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_FLOATING_BALL_PREPARE_CLOSE):
            return HandleStopFloatingBall(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_START_FLOATING_BALL_MAIN_WINDOW):
            return HandleStartFloatingBallMainWindow(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_FLOATING_BALL_WINDOW_ID):
            return HandleGetFloatingBallWindowId(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_LAYOUT_FULL_SCREEN_CHANGE):
            return HandleLayoutFullScreenChange(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_DEFAULT_DENSITY_ENABLED):
            return HandleDefaultDensityEnabled(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_COLOR_MODE):
            return HandleUpdateColorMode(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TITLE_AND_DOCK_HOVER_SHOW_CHANGE):
            return HandleTitleAndDockHoverShowChange(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_FORCE_LANDSCAPE_CONFIG):
            return HandleGetAppForceLandscapeConfig(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_FORCE_LANDSCAPE_CONFIG_ENABLE):
            return HandleGetAppForceLandscapeConfigEnable(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_HOOK_WINDOW_INFO):
            return HandleGetAppHookWindowInfoFromServer(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_WINDOW_STATUS_AFTER_SHOW_WINDOW):
            return HandleNotifyWindowStatusDidChangeAfterShowWindow(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_DIALOG_SESSION_BACKGESTURE_ENABLE):
            return HandleSetDialogSessionBackGestureEnabled(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_STATUSBAR_HEIGHT):
            return HandleGetStatusBarHeight(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_FRAME_LAYOUT_FINISH):
            return HandleNotifyFrameLayoutFinish(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_SNAPSHOT_UPDATE):
            return HandleSnapshotUpdate(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_EXTENSION_EVENT_ASYNC):
            return HandleNotifyExtensionEventAsync(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_REQUEST_FOCUS):
            return HandleRequestFocus(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_GESTURE_BACK_ENABLE):
            return HandleSetGestureBackEnabled(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SUB_MODAL_TYPE_CHANGE):
            return HandleNotifySubModalTypeChange(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_MAIN_MODAL_TYPE_CHANGE):
            return HandleNotifyMainModalTypeChange(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_RECT_AUTO_SAVE):
            return HandleSetWindowRectAutoSave(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_EXTENSION_DETACH_TO_DISPLAY):
            return HandleNotifyExtensionDetachToDisplay(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_SUPPORT_WINDOW_MODES):
            return HandleSetSupportedWindowModes(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SEND_EXTENSION_DATA):
            return HandleExtensionProviderData(data, reply, option);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_SESSION_LABEL_AND_ICON):
            return HandleSetSessionLabelAndIcon(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CHANGE_KEYBOARD_VIEW_MODE):
            return HandleChangeKeyboardEffectOption(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_CORNER_RADIUS):
            return HandleSetWindowCornerRadius(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_START_MOVING_WITH_COORDINATE):
            return HandleStartMovingWithCoordinate(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_CROSS_AXIS_STATE):
            return HandleGetCrossAxisState(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_WATERFALL_MODE):
            return HandleGetWaterfallMode(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_MAIN_WINDOW_FULL_SCREEN_ACROSS_DISPLAYS):
            return HandleIsMainWindowFullScreenAcrossDisplays(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_USE_IMPLICT_ANIMATION):
            return HandleUseImplicitAnimation(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CONTAINER_MODAL_EVENT):
            return HandleContainerModalEvent(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_WINDOW_ATTACH_STATE_LISTENER_REGISTERED):
            return HandleNotifyWindowAttachStateListenerRegistered(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_FOLLOW_PARENT_LAYOUT_ENABLED):
            return HandleSetFollowParentWindowLayoutEnabled(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_ANCHOR_INFO):
            return HandleSetWindowAnchorInfo(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_TRANSITION_ANIMATION):
            return HandleSetWindowTransitionAnimation(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_GLOBAL_DISPLAY_RECT):
            return HandleUpdateGlobalDisplayRectFromClient(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_FOLLOW_PARENT_MULTI_SCREEN_POLICY):
            return HandleNotifyFollowParentMultiScreenPolicy(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_KEY_FRAME_ANIMATE_END):
            return HandleKeyFrameAnimateEnd(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_KEY_FRAME_CLONE_NODE):
            return HandleUpdateKeyFrameCloneNode(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_DRAG_KEY_FRAME_POLICY):
            return HandleSetDragKeyFramePolicy(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_KEYBOARD_WILL_SHOW_REGISTERED):
            return HandleNotifyKeyboardWillShowRegistered(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_KEYBOARD_WILL_HIDE_REGISTERED):
            return HandleNotifyKeyboardWillHideRegistered(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_KEYBOARD_DID_SHOW_REGISTERED):
            return HandleNotifyKeyboardDidShowRegistered(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_KEYBOARD_DID_HIDE_REGISTERED):
            return HandleNotifyKeyboardDidHideRegistered(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_FLAG):
            return HandleUpdateFlag(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_ROTATION_CHANGE):
            return HandleUpdateRotationChangeListenerRegistered(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_SCREEN_SHOT_APP_EVENT_REGISTERED):
            return HandleUpdateScreenshotAppEventRegistered(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_ACROSS_DISPLAYS_REGISTERED):
            return HandleUpdateAcrossDisplaysChangeRegistered(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_IS_HIGHLIGHTED):
            return HandleGetIsHighlighted(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_DISABLE_DELEGATOR_CHANGE):
            return HandleNotifyDisableDelegatorChange(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_SHADOWS):
            return HandleSetWindowShadows(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_SUBWINDOW_SOURCE):
            return HandleSetSubWindowSource(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_FRAMERECT_FOR_PARTIAL_ZOOMIN):
            return HandleSetFrameRectForPartialZoomIn(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_IS_FULL_SCREEN_IN_FORCE_SPLIT):
            return HandleNotifyIsFullScreenInForceSplitMode(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_COMPATIBLE_MODE_CHANGE):
            return HandleNotifyCompatibleModeChange(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RESTART_APP):
            return HandleRestartApp(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SEND_COMMAND_EVENT):
            return HandleSendCommonEvent(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_REMOVE_PRELAUNCH_STARTING_WINDOW):
            return HandleRemovePrelaunchStartingWindow(data, reply);
        default:
            WLOGFE("Failed to find function handler!");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int SessionStub::HandleSetWindowAnimationFlag(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleSetWindowAnimationFlag!");
    bool isNeedWindowAnimationFlag = data.ReadBool();
    WSError errCode = UpdateWindowAnimationFlag(isNeedWindowAnimationFlag);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleForeground(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("[WMSCom] Foreground!");
    sptr<WindowSessionProperty> property = nullptr;
    if (data.ReadBool()) {
        property = data.ReadStrongParcelable<WindowSessionProperty>();
        if (property == nullptr) {
            return ERR_INVALID_DATA;
        }
    } else {
        WLOGFW("[WMSCom] Property not exist!");
        property = sptr<WindowSessionProperty>::MakeSptr();
    }
    bool isFromClient = data.ReadBool();
    std::string identityToken;
    if (!data.ReadString(identityToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read identityToken failed.");
        return ERR_INVALID_DATA;
    }
    const WSError errCode = Foreground(property, true, identityToken);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleBackground(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("[WMSCom] Background!");
    bool isFromClient = data.ReadBool();
    std::string identityToken;
    if (!data.ReadString(identityToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read identityToken failed.");
        return ERR_INVALID_DATA;
    }
    const WSError errCode = Background(true, identityToken);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleDisconnect(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("Disconnect!");
    bool isFromClient = data.ReadBool();
    std::string identityToken;
    if (!data.ReadString(identityToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read identityToken failed.");
        return ERR_INVALID_DATA;
    }
    WSError errCode = Disconnect(true, identityToken);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleShow(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("Show!");
    sptr<WindowSessionProperty> property = nullptr;
    if (data.ReadBool()) {
        property = data.ReadStrongParcelable<WindowSessionProperty>();
        if (property == nullptr) {
            return ERR_INVALID_DATA;
        }
    } else {
        WLOGFW("Property not exist!");
        property = sptr<WindowSessionProperty>::MakeSptr();
    }
    WSError errCode = Show(property);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleHide(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("Hide!");
    WSError errCode = Hide();
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleConnect(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In");
    sptr<IRemoteObject> sessionStageObject = data.ReadRemoteObject();
    sptr<ISessionStage> sessionStage = iface_cast<ISessionStage>(sessionStageObject);
    sptr<IRemoteObject> eventChannelObject = data.ReadRemoteObject();
    sptr<IWindowEventChannel> eventChannel = iface_cast<IWindowEventChannel>(eventChannelObject);
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Unmarshalling(data);
    if (sessionStage == nullptr || eventChannel == nullptr || surfaceNode == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to read scene session stage object or event channel object!");
        return ERR_INVALID_DATA;
    }
    // LCOV_EXCL_START
    bool hasWindowSessionProperty = false;
    if (!data.ReadBool(hasWindowSessionProperty)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read hasWindowSessionProperty failed.");
        return ERR_INVALID_DATA;
    }
    sptr<WindowSessionProperty> property = nullptr;
    if (hasWindowSessionProperty) {
        property = data.ReadStrongParcelable<WindowSessionProperty>();
        if (property == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "Property is nullptr.");
            return ERR_INVALID_DATA;
        }
    } else {
        TLOGW(WmsLogTag::WMS_LIFE, "Property not exist!");
    }
    sptr<IRemoteObject> token = nullptr;
    if (property && property->GetTokenState()) {
        token = data.ReadRemoteObject();
        if (token == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "Token is nullptr.");
            return ERR_INVALID_DATA;
        }
    }
    std::string identityToken;
    if (!data.ReadString(identityToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read identityToken failed.");
        return ERR_INVALID_DATA;
    }
    SystemSessionConfig systemConfig;
    WSError errCode = Connect(sessionStage, eventChannel, surfaceNode, systemConfig, property, token,
        identityToken);
    reply.WriteParcelable(&systemConfig);
    if (property) {
        reply.WriteInt32(property->GetPersistentId());
        reply.WriteUint64(property->GetDisplayId());
        bool needUpdate = property->GetIsNeedUpdateWindowMode();
        reply.WriteBool(needUpdate);
        if (needUpdate) {
            reply.WriteUint32(static_cast<uint32_t>(property->GetWindowMode()));
        }
        property->SetIsNeedUpdateWindowMode(false);
        Rect winRect = property->GetWindowRect();
        reply.WriteInt32(winRect.posX_);
        reply.WriteInt32(winRect.posY_);
        reply.WriteUint32(winRect.width_);
        reply.WriteUint32(winRect.height_);
        reply.WriteInt32(property->GetCollaboratorType());
        reply.WriteBool(property->GetFullScreenStart());
        std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes;
        property->GetSupportedWindowModes(supportedWindowModes);
        auto size = supportedWindowModes.size();
        if (size > 0 && size <= WINDOW_SUPPORT_MODE_MAX_SIZE) {
            reply.WriteUint32(static_cast<uint32_t>(size));
            for (decltype(size) i = 0; i < size; i++) {
                reply.WriteInt32(static_cast<int32_t>(supportedWindowModes[i]));
            }
        } else {
            reply.WriteUint32(0);
        }
        WindowSizeLimits windowSizeLimits = property->GetWindowSizeLimits();
        reply.WriteUint32(windowSizeLimits.maxWindowWidth);
        reply.WriteUint32(windowSizeLimits.minWindowWidth);
        reply.WriteUint32(windowSizeLimits.maxWindowHeight);
        reply.WriteUint32(windowSizeLimits.minWindowHeight);
        reply.WriteBool(property->GetIsAppSupportPhoneInPc());
        reply.WriteBool(property->GetIsPcAppInPad());
        reply.WriteUint32(static_cast<uint32_t>(property->GetRequestedOrientation()));
        reply.WriteUint32(static_cast<uint32_t>(property->GetUserRequestedOrientation()));
        reply.WriteString(property->GetAppInstanceKey());
        reply.WriteInt32(property->GetAppIndex());
        reply.WriteBool(property->GetDragEnabled());
        reply.WriteBool(property->GetIsAtomicService());
        reply.WriteBool(property->GetIsAbilityHook());
        reply.WriteBool(property->GetPcAppInpadCompatibleMode());
        reply.WriteBool(property->GetPcAppInpadSpecificSystemBarInvisible());
        reply.WriteBool(property->GetPcAppInpadOrientationLandscape());
        reply.WriteBool(property->GetMobileAppInPadLayoutFullScreen());
        reply.WriteParcelable(property->GetCompatibleModeProperty());
        reply.WriteBool(property->GetUseControlState());
        reply.WriteString(property->GetAncoRealBundleName());
        MissionInfo missionInfo = property->GetMissionInfo();
        reply.WriteParcelable(&missionInfo);
        reply.WriteBool(property->GetIsShowDecorInFreeMultiWindow());
        reply.WriteBool(property->IsPrelaunch());
        reply.WriteInt32(property->GetFrameNum());
    }
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
    // LCOV_EXCL_STOP
}

// LCOV_EXCL_START
int SessionStub::HandleNotifyFrameLayoutFinish(MessageParcel& data, MessageParcel& reply)
{
    bool notifyListener = data.ReadBool();
    WSRect rect = { data.ReadInt32(), data.ReadInt32(), data.ReadInt32(), data.ReadInt32() };
    NotifyFrameLayoutFinishFromApp(notifyListener, rect);
    return ERR_NONE;
}

int SessionStub::HandleSnapshotUpdate(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PATTERN, "In");
    WMError errCode = NotifySnapshotUpdate();
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PATTERN, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleRemovePrelaunchStartingWindow(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PATTERN, "In");
    WMError errCode = NotifyRemovePrelaunchStartingWindow();
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PATTERN, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleDrawingCompleted(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "Called!");
    const WSError errCode = DrawingCompleted();
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleRemoveStartingWindow(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_STARTUP_PAGE, "Called!");
    WSError errCode = RemoveStartingWindow();
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}
// LCOV_EXCL_STOP

bool ReadEventParam(MessageParcel& data, SessionEvent event, SessionEventParam& param)
{
    if (event == SessionEvent::EVENT_MAXIMIZE) {
        if (!data.ReadUint32(param.waterfallResidentState)) {
            TLOGE(WmsLogTag::WMS_EVENT, "Failed to read waterfallResidentState");
            return false;
        }
    } else if (event == SessionEvent::EVENT_SWITCH_COMPATIBLE_MODE) {
        if (!data.ReadUint32(param.compatibleStyleMode)) {
            TLOGE(WmsLogTag::WMS_EVENT, "Failed to read compatibleStyleMode");
            return false;
        }
    }
    return true;
}

int SessionStub::HandleSessionEvent(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_EVENT, "In!");
    uint32_t eventId = 0;
    if (!data.ReadUint32(eventId)) {
        TLOGE(WmsLogTag::WMS_EVENT, "read eventId failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_EVENT, "eventId: %{public}d", eventId);
    auto event = static_cast<SessionEvent>(eventId);
    if (event < SessionEvent::EVENT_MAXIMIZE || event >= SessionEvent::EVENT_END) {
        TLOGE(WmsLogTag::WMS_EVENT, "Invalid eventId: %{public}d", eventId);
        return ERR_INVALID_DATA;
    }
    SessionEventParam param;
    if (!ReadEventParam(data, event, param)) {
        return ERR_INVALID_DATA;
    }
    WSError errCode = OnSessionEvent(event, param);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleSyncSessionEvent(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_EVENT, "In!");
    uint32_t eventId;
    if (!data.ReadUint32(eventId)) {
        TLOGE(WmsLogTag::WMS_EVENT, "read eventId failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_EVENT, "eventId: %{public}d", eventId);
    WSError errCode = SyncSessionEvent(static_cast<SessionEvent>(eventId));
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleLayoutFullScreenChange(MessageParcel& data, MessageParcel& reply)
{
    bool isLayoutFullScreen = data.ReadBool();
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "isLayoutFullScreen: %{public}d", isLayoutFullScreen);
    WSError errCode = OnLayoutFullScreenChange(isLayoutFullScreen);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleUpdateColorMode(MessageParcel& data, MessageParcel& reply)
{
    std::string colorMode;
    if (!data.ReadString(colorMode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read colorMode failed.");
        return ERR_INVALID_DATA;
    }
    bool hasDarkRes = false;
    if (!data.ReadBool(hasDarkRes)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read hasDarkRes failed.");
        return ERR_INVALID_DATA;
    }
    OnUpdateColorMode(colorMode, hasDarkRes);
    return ERR_NONE;
}

int SessionStub::HandleDefaultDensityEnabled(MessageParcel& data, MessageParcel& reply)
{
    bool isDefaultDensityEnabled = false;
    if (!data.ReadBool(isDefaultDensityEnabled)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read isDefaultDensityEnabled failed.");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT, "isDefaultDensityEnabled: %{public}d", isDefaultDensityEnabled);
    WSError errCode = OnDefaultDensityEnabled(isDefaultDensityEnabled);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleTitleAndDockHoverShowChange(MessageParcel& data, MessageParcel& reply)
{
    bool isTitleHoverShown = true;
    if (!data.ReadBool(isTitleHoverShown)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Read isTitleHoverShown failed.");
        return ERR_INVALID_DATA;
    }
    bool isDockHoverShown = true;
    if (!data.ReadBool(isDockHoverShown)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Read isDockHoverShown failed.");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "isTitleHoverShown: %{public}d, isDockHoverShown: %{public}d",
        isTitleHoverShown, isDockHoverShown);
    WSError errCode = OnTitleAndDockHoverShowChange(isTitleHoverShown, isDockHoverShown);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleRestoreMainWindow(MessageParcel& data, MessageParcel& reply)
{
    uint32_t errCode = static_cast<uint32_t>(OnRestoreMainWindow());
    TLOGD(WmsLogTag::WMS_MAIN, "Handle restore main window readResult: %{public}u", errCode);
    if (!reply.WriteUint32(errCode)) {
        TLOGE(WmsLogTag::WMS_MAIN, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleRestoreFloatMainWindow(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "HandleRestoreFloatMainWindow In");
    std::shared_ptr<AAFwk::WantParams> wantParams(data.ReadParcelable<AAFwk::WantParams>());
    if (wantParams == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "wantParams is nullptr");
        return ERR_INVALID_VALUE;
    }
    WMError ret = RestoreFloatMainWindow(wantParams);
    if (!reply.WriteUint32(static_cast<uint32_t>(ret))) {
        TLOGE(WmsLogTag::WMS_LIFE, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleTerminateSession(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In");
    std::shared_ptr<AAFwk::Want> localWant(data.ReadParcelable<AAFwk::Want>());
    if (localWant == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "localWant is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilitySessionInfo->want = *localWant;
    bool hasCallerToken = false;
    if (!data.ReadBool(hasCallerToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read hasCallerToken failed.");
        return ERR_INVALID_DATA;
    }
    if (hasCallerToken) {
        abilitySessionInfo->callerToken = data.ReadRemoteObject();
    }
    if (!data.ReadInt32(abilitySessionInfo->resultCode)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read resultCode failed.");
        return ERR_INVALID_DATA;
    }
    WSError errCode = TerminateSession(abilitySessionInfo);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleSessionException(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In");
    std::shared_ptr<AAFwk::Want> localWant(data.ReadParcelable<AAFwk::Want>());
    if (localWant == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "localWant is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilitySessionInfo->want = *localWant;
    bool hasCallerToken = false;
    if (!data.ReadBool(hasCallerToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read hasCallerToken failed.");
        return ERR_INVALID_DATA;
    }
    if (hasCallerToken) {
        abilitySessionInfo->callerToken = data.ReadRemoteObject();
    }
    if (!data.ReadInt32(abilitySessionInfo->persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read persistentId failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadInt32(abilitySessionInfo->errorCode)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read errorCode failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadString(abilitySessionInfo->errorReason)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read errorReason failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadString(abilitySessionInfo->identityToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read identityToken failed.");
        return ERR_INVALID_DATA;
    }
    ExceptionInfo exceptionInfo;
    if (!data.ReadBool(exceptionInfo.needRemoveSession)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read needRemoveSession failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(exceptionInfo.needClearCallerLink)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read needClearCallerLink failed.");
        return ERR_INVALID_DATA;
    }
    exceptionInfo.needClearCallerLink =
        exceptionInfo.needRemoveSession ? true : exceptionInfo.needClearCallerLink;
    WSError errCode = NotifySessionException(abilitySessionInfo, exceptionInfo);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleChangeSessionVisibilityWithStatusBar(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In");
    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    int32_t readResult = ReadBasicAbilitySessionInfo(data, abilitySessionInfo);
    if (readResult == ERR_INVALID_DATA) {
        return ERR_INVALID_DATA;
    }
    bool hasCallerToken = false;
    if (!data.ReadBool(hasCallerToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read hasCallerToken failed.");
        return ERR_INVALID_DATA;
    }
    if (hasCallerToken) {
        abilitySessionInfo->callerToken = data.ReadRemoteObject();
    }
    bool hasStartSetting = false;
    if (!data.ReadBool(hasStartSetting)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read hasStartSetting failed.");
        return ERR_INVALID_DATA;
    }
    if (hasStartSetting) {
        abilitySessionInfo->startSetting.reset(data.ReadParcelable<AAFwk::AbilityStartSetting>());
    }
    bool visible = false;
    if (!data.ReadBool(visible)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read visible failed.");
        return ERR_INVALID_DATA;
    }
    WSError errCode = ChangeSessionVisibilityWithStatusBar(abilitySessionInfo, visible);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandlePendingSessionActivation(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In!");
    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    int32_t readResult = ReadBasicAbilitySessionInfo(data, abilitySessionInfo);
    if (readResult == ERR_INVALID_DATA) {
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(abilitySessionInfo->canStartAbilityFromBackground)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read canStartAbilityFromBackground failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(abilitySessionInfo->isAtomicService)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read isAtomicService failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(abilitySessionInfo->isBackTransition)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read isBackTransition failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(abilitySessionInfo->needClearInNotShowRecent)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read needClearInNotShowRecent failed.");
        return ERR_INVALID_DATA;
    }
    bool hasCallerToken = false;
    if (!data.ReadBool(hasCallerToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read hasCallerToken failed.");
        return ERR_INVALID_DATA;
    }
    if (hasCallerToken) {
        abilitySessionInfo->callerToken = data.ReadRemoteObject();
    }
    bool hasStartSetting = false;
    if (!data.ReadBool(hasStartSetting)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read hasStartSetting failed.");
        return ERR_INVALID_DATA;
    }
    if (hasStartSetting) {
        abilitySessionInfo->startSetting.reset(data.ReadParcelable<AAFwk::AbilityStartSetting>());
    }
    if (!data.ReadString(abilitySessionInfo->instanceKey)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read instanceKey failed.");
        return ERR_INVALID_DATA;
    }
    bool hasStartWindowOption = false;
    if (!data.ReadBool(hasStartWindowOption)) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "Read hasStartWindowOption failed.");
        return ERR_INVALID_DATA;
    }
    if (hasStartWindowOption) {
        auto startWindowOption = data.ReadParcelable<AAFwk::StartWindowOption>();
        abilitySessionInfo->startWindowOption.reset(startWindowOption);
    }
    uint32_t size = data.ReadUint32();
    if (size > 0 && size <= WINDOW_SUPPORT_MODE_MAX_SIZE) {
        abilitySessionInfo->supportWindowModes.reserve(size);
        for (uint32_t i = 0; i < size; i++) {
            abilitySessionInfo->supportWindowModes.push_back(
                static_cast<AppExecFwk::SupportWindowMode>(data.ReadInt32()));
        }
    }
    if (!data.ReadString(abilitySessionInfo->specifiedFlag)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read specifiedFlag failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(abilitySessionInfo->reuseDelegatorWindow)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read reuseDelegatorWindow failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(abilitySessionInfo->hideStartWindow)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read hideStartWindow failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadInt32(abilitySessionInfo->scenarios)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read scenarios failed.");
        return ERR_INVALID_DATA;
    }
    abilitySessionInfo->windowCreateParams.reset(data.ReadParcelable<WindowCreateParams>());
    if (!data.ReadBool(abilitySessionInfo->isPrelaunch)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read isPrelaunch failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadInt32(abilitySessionInfo->frameNum)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read frameNum failed.");
        return ERR_INVALID_DATA;
    }
    WSError errCode = PendingSessionActivation(abilitySessionInfo);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

// LCOV_EXCL_START
int SessionStub::HandleBatchPendingSessionsActivation(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "In!");
    int32_t size = 0;
    if (!data.ReadInt32(size) || size < 0 || size > MAX_ABILITY_SESSION_INFOS) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read ability session info size failed");
        return ERR_INVALID_DATA;
    }
    std::vector<sptr<AAFwk::SessionInfo>> abilitySessionInfos;
    for (int32_t i = 0; i < size; i++) {
        sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
        int readRet = ReadOneAbilitySessionInfo(data, abilitySessionInfo);
        if (readRet != ERR_NONE) {
            return readRet;
        }
        abilitySessionInfos.emplace_back(abilitySessionInfo);
    }

    size = 0;
    if (!data.ReadInt32(size) || size < 0 || size > MAX_ABILITY_SESSION_INFOS) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read ability config size failed");
        return ERR_INVALID_DATA;
    }
    std::vector<PendingSessionActivationConfig> configs;
    for (int32_t i = 0; i < size; i++) {
        PendingSessionActivationConfig config;
        int readRet = ReadOnePendingSessionActivationConfig(data, config);
        if (readRet != ERR_NONE) {
            return readRet;
        }
        configs.emplace_back(config);
    }
    
    WSError errCode = BatchPendingSessionsActivation(abilitySessionInfos, configs);
    if (!reply.WriteUint32(static_cast<uint32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write errCode failed");
    }
    return ERR_NONE;
}

int SessionStub::ReadOneAbilitySessionInfo(MessageParcel& data, sptr<AAFwk::SessionInfo> abilitySessionInfo)
{
    int32_t readResult = ReadBasicAbilitySessionInfo(data, abilitySessionInfo);
    if (readResult == ERR_INVALID_DATA) {
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(abilitySessionInfo->canStartAbilityFromBackground)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read canStartAbilityFromBackground failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(abilitySessionInfo->isAtomicService)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read isAtomicService failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(abilitySessionInfo->isBackTransition)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read isBackTransition failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(abilitySessionInfo->needClearInNotShowRecent)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read needClearInNotShowRecent failed.");
        return ERR_INVALID_DATA;
    }
    bool hasCallerToken = false;
    if (!data.ReadBool(hasCallerToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read hasCallerToken failed.");
        return ERR_INVALID_DATA;
    }
    if (hasCallerToken) {
        abilitySessionInfo->callerToken = data.ReadRemoteObject();
    }
    bool hasStartSetting = false;
    if (!data.ReadBool(hasStartSetting)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read hasStartSetting failed.");
        return ERR_INVALID_DATA;
    }
    if (hasStartSetting) {
        abilitySessionInfo->startSetting.reset(data.ReadParcelable<AAFwk::AbilityStartSetting>());
    }
    bool hasStartWindowOption = false;
    if (!data.ReadBool(hasStartWindowOption)) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "Read hasStartWindowOption failed.");
        return ERR_INVALID_DATA;
    }
    if (hasStartWindowOption) {
        auto startWindowOption = data.ReadParcelable<AAFwk::StartWindowOption>();
        abilitySessionInfo->startWindowOption.reset(startWindowOption);
    }
    if (!data.ReadString(abilitySessionInfo->instanceKey)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read instanceKey failed.");
        return ERR_INVALID_VALUE;
    }
    uint32_t size = data.ReadUint32();
    if (size > 0 && size <= WINDOW_SUPPORT_MODE_MAX_SIZE) {
        abilitySessionInfo->supportWindowModes.reserve(size);
        for (uint32_t i = 0; i < size; i++) {
            abilitySessionInfo->supportWindowModes.push_back(
                static_cast<AppExecFwk::SupportWindowMode>(data.ReadInt32()));
        }
    }
    if (!data.ReadString(abilitySessionInfo->specifiedFlag)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read specifiedFlag failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(abilitySessionInfo->reuseDelegatorWindow)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read reuseDelegatorWindow failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}
// LCOV_EXCL_STOP

/** @note @window.layout */
int SessionStub::HandleUpdateSessionRect(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "In");
    int32_t posX = 0;
    int32_t posY = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    if (!data.ReadInt32(posX) || !data.ReadInt32(posY) || !data.ReadUint32(width) || !data.ReadUint32(height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read rect failed");
        return ERR_INVALID_DATA;
    }
    WSRect rect = {posX, posY, width, height};
    TLOGD(WmsLogTag::WMS_LAYOUT, "rect:[%{public}d, %{public}d, %{public}u, %{public}u]", posX, posY,
        width, height);
    uint32_t changeReason = 0;
    if (!data.ReadUint32(changeReason)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read changeReason failed");
        return ERR_INVALID_DATA;
    }
    if (changeReason < static_cast<uint32_t>(SizeChangeReason::UNDEFINED) ||
        changeReason > static_cast<uint32_t>(SizeChangeReason::END)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Unknown reason");
        return ERR_INVALID_DATA;
    }
    SizeChangeReason reason = static_cast<SizeChangeReason>(changeReason);
    bool isGlobal = false;
    if (!data.ReadBool(isGlobal)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read isGlobal failed");
        return ERR_INVALID_DATA;
    }
    auto isFromMoveToGlobal = false;
    if (!data.ReadBool(isFromMoveToGlobal)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read isFromMoveToGlobal failed");
        return ERR_INVALID_DATA;
    }
    uint64_t displayId = DISPLAY_ID_INVALID;
    if (!data.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read displayId failed");
        return ERR_INVALID_DATA;
    }
    MoveConfiguration moveConfiguration;
    moveConfiguration.displayId = static_cast<DisplayId>(displayId);
    RectAnimationConfig rectAnimationConfig;
    if (reason == SizeChangeReason::MOVE_WITH_ANIMATION || reason == SizeChangeReason::RESIZE_WITH_ANIMATION) {
        if (!data.ReadUint32(rectAnimationConfig.duration) || !data.ReadFloat(rectAnimationConfig.x1) ||
            !data.ReadFloat(rectAnimationConfig.y1) || !data.ReadFloat(rectAnimationConfig.x2) ||
            !data.ReadFloat(rectAnimationConfig.y2)) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "read animation config failed");
            return ERR_INVALID_DATA;
        }
        if (reason == SizeChangeReason::MOVE_WITH_ANIMATION) {
            moveConfiguration.rectAnimationConfig = rectAnimationConfig;
        }
    }
    TLOGD(WmsLogTag::WMS_LAYOUT, "rectAnimationConfig:[%{public}u, %{public}f, %{public}f, %{public}f, %{public}f]",
        rectAnimationConfig.duration, rectAnimationConfig.x1, rectAnimationConfig.y1, rectAnimationConfig.x2,
        rectAnimationConfig.y2);
    WSError errCode = UpdateSessionRect(rect, reason, isGlobal, isFromMoveToGlobal, moveConfiguration,
        rectAnimationConfig);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

/** @note @window.layout */
int SessionStub::HandleGetGlobalScaledRect(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "In");
    Rect globalScaledRect;
    WMError errorCode = GetGlobalScaledRect(globalScaledRect);
    if (!reply.WriteInt32(globalScaledRect.posX_) || !reply.WriteInt32(globalScaledRect.posY_) ||
        !reply.WriteUint32(globalScaledRect.width_) || !reply.WriteUint32(globalScaledRect.height_) ||
        !reply.WriteInt32(static_cast<int32_t>(errorCode))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

/** @note @window.layout */
int SessionStub::HandleUpdateClientRect(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "In");
    int32_t posX = 0;
    int32_t posY = 0;
    int32_t width = 0;
    int32_t height = 0;
    if (!data.ReadInt32(posX) || !data.ReadInt32(posY) || !data.ReadInt32(width) || !data.ReadInt32(height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read rect failed");
        return ERR_INVALID_DATA;
    }
    WSRect rect = { posX, posY, width, height };
    WSError errCode = UpdateClientRect(rect);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleRaiseToAppTop(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("RaiseToAppTop!");
    WSError errCode = RaiseToAppTop();
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleRaiseAboveTarget(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_HIERARCHY, "In");
    uint32_t subWindowId = 0;
    if (!data.ReadUint32(subWindowId)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "read subWindowId failed");
        return ERR_INVALID_DATA;
    }
    WSError errCode = RaiseAboveTarget(subWindowId);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleRaiseMainWindowAboveTarget(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_HIERARCHY, "In");
    int32_t targetId = 0;
    if (!data.ReadInt32(targetId)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "read targetId failed");
        return ERR_INVALID_DATA;
    }
    WSError errCode = RaiseMainWindowAboveTarget(targetId);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleRaiseAppMainWindowToTop(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("RaiseAppMainWindowToTop!");
    WSError errCode = RaiseAppMainWindowToTop();
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleBackPressed(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleBackPressed!");
    bool needMoveToBackground = false;
    if (!data.ReadBool(needMoveToBackground)) {
        WLOGFE("Read needMoveToBackground from parcel failed!");
        return ERR_INVALID_DATA;
    }
    WSError errCode = RequestSessionBack(needMoveToBackground);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleMarkProcessed(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleMarkProcessed!");
    int32_t eventId = 0;
    if (!data.ReadInt32(eventId)) {
        WLOGFE("Read eventId from parcel failed!");
        return ERR_INVALID_DATA;
    }
    WSError errCode = MarkProcessed(eventId);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleSetGlobalMaximizeMode(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleSetGlobalMaximizeMode!");
    uint32_t mode = 0;
    if (!data.ReadUint32(mode) || mode >= static_cast<uint32_t>(MaximizeMode::MODE_END)) {
        return ERR_INVALID_DATA;
    }
    WSError errCode = SetGlobalMaximizeMode(static_cast<MaximizeMode>(mode));
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleGetGlobalMaximizeMode(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleGetGlobalMaximizeMode!");
    MaximizeMode mode = MaximizeMode::MODE_FULL_FILL;
    WSError errCode = GetGlobalMaximizeMode(mode);
    reply.WriteUint32(static_cast<uint32_t>(mode));
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleNeedAvoid(MessageParcel& data, MessageParcel& reply)
{
    bool status = false;
    if (!data.ReadBool(status)) {
        return ERR_INVALID_DATA;
    }
    WLOGFD("HandleNeedAvoid status:%{public}d", static_cast<int32_t>(status));
    WSError errCode = OnNeedAvoid(status);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleGetAvoidAreaByType(MessageParcel& data, MessageParcel& reply)
{
    uint32_t typeId = 0;
    if (!data.ReadUint32(typeId) ||
        typeId >= static_cast<uint32_t>(AvoidAreaType::TYPE_END)) {
        TLOGE(WmsLogTag::WMS_IMMS, "read typeId error");
        return ERR_INVALID_DATA;
    }
    WSRect rect {};
    if (!data.ReadInt32(rect.posX_) || !data.ReadInt32(rect.posY_) ||
        !data.ReadInt32(rect.width_) || !data.ReadInt32(rect.height_)) {
        TLOGE(WmsLogTag::WMS_IMMS, "read rect error");
        return ERR_INVALID_DATA;
    }
    int32_t apiVersion = API_VERSION_INVALID;
    if (!data.ReadInt32(apiVersion)) {
        TLOGE(WmsLogTag::WMS_IMMS, "read api version error");
        return ERR_INVALID_DATA;
    }
    AvoidAreaType type = static_cast<AvoidAreaType>(typeId);
    WLOGFD("HandleGetAvoidArea type:%{public}d", typeId);
    AvoidArea avoidArea = GetAvoidAreaByType(type, rect, apiVersion);
    reply.WriteParcelable(&avoidArea);
    return ERR_NONE;
}

int SessionStub::HandleGetAvoidAreaByTypeIgnoringVisibility(MessageParcel& data, MessageParcel& reply)
{
    uint32_t typeId = 0;
    if (!data.ReadUint32(typeId) ||
        typeId >= static_cast<uint32_t>(AvoidAreaType::TYPE_END)) {
        TLOGE(WmsLogTag::WMS_IMMS, "read typeId error");
        return ERR_INVALID_DATA;
    }
    WSRect rect {};
    if (!data.ReadInt32(rect.posX_) || !data.ReadInt32(rect.posY_) ||
        !data.ReadInt32(rect.width_) || !data.ReadInt32(rect.height_)) {
        TLOGE(WmsLogTag::WMS_IMMS, "read rect error");
        return ERR_INVALID_DATA;
    }
    AvoidAreaType type = static_cast<AvoidAreaType>(typeId);
    AvoidArea avoidArea = GetAvoidAreaByTypeIgnoringVisibility(type, rect);
    reply.WriteParcelable(&avoidArea);
    return ERR_NONE;
}

int SessionStub::HandleGetAllAvoidAreas(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_IMMS, "in");
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    WSError errCode = GetAllAvoidAreas(avoidAreas);
    reply.WriteUint32(avoidAreas.size());
    for (const auto& [type, avoidArea] : avoidAreas) {
        reply.WriteUint32(static_cast<uint32_t>(type));
        reply.WriteParcelable(&avoidArea);
    }
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleGetTargetOrientationConfigInfo(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "in");
    Orientation targetOrientation = static_cast<Orientation>(data.ReadUint32());
    std::map<Rosen::WindowType, Rosen::SystemBarProperty> targetProperties;
    std::map<Rosen::WindowType, Rosen::SystemBarProperty> currentProperties;
    uint32_t targetPropertiesSize = 0;
    if (!data.ReadUint32(targetPropertiesSize)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read targetPropertiesSize error");
        return ERR_INVALID_DATA;
    }
    constexpr uint32_t WINDOW_TYPE_MAX_SIZE = 100;
    if (targetPropertiesSize > WINDOW_TYPE_MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "size is invalid");
        return ERR_INVALID_DATA;
    }
    for (uint32_t i = 0; i < targetPropertiesSize; i++) {
        uint32_t type = 0;
        if (!data.ReadUint32(type)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read type error");
            return ERR_INVALID_DATA;
        }
        if (type < static_cast<uint32_t>(WindowType::APP_WINDOW_BASE) ||
            type > static_cast<uint32_t>(WindowType::WINDOW_TYPE_UI_EXTENSION)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "type is invalid");
            return ERR_INVALID_DATA;
        }
        bool enable = false;
        uint32_t backgroundColor = 0;
        uint32_t contentColor = 0;
        bool enableAnimation = false;
        uint32_t settingFlagNumber = 0;
        if (!data.ReadBool(enable)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read enable error");
            return ERR_INVALID_DATA;
        }
        if (!data.ReadUint32(backgroundColor)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read backgroundColor error");
            return ERR_INVALID_DATA;
        }
        if (!data.ReadUint32(contentColor)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read contentColor error");
            return ERR_INVALID_DATA;
        }
        if (!data.ReadBool(enableAnimation)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read enableAnimation error");
            return ERR_INVALID_DATA;
        }
        if (!data.ReadUint32(settingFlagNumber)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read settingFlagNumber error");
            return ERR_INVALID_DATA;
        }
        SystemBarSettingFlag settingFlag = static_cast<SystemBarSettingFlag>(settingFlagNumber);
        SystemBarProperty property = { enable, backgroundColor, contentColor, enableAnimation, settingFlag };
        targetProperties[static_cast<WindowType>(type)] = property;
    }

    uint32_t currentPropertiesSize = 0;
    if (!data.ReadUint32(currentPropertiesSize)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read currentPropertiesSize error");
        return ERR_INVALID_DATA;
    }
    if (currentPropertiesSize > WINDOW_TYPE_MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "currentPropertiesSize is invalid");
        return ERR_INVALID_DATA;
    }
    for (uint32_t i = 0; i < currentPropertiesSize; i++) {
        uint32_t type = 0;
        if (!data.ReadUint32(type)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read current type error");
            return ERR_INVALID_DATA;
        }
        if (type < static_cast<uint32_t>(WindowType::APP_WINDOW_BASE) ||
            type > static_cast<uint32_t>(WindowType::WINDOW_TYPE_UI_EXTENSION)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "current type is invalid");
            return ERR_INVALID_DATA;
        }
        bool enable = false;
        uint32_t backgroundColor = 0;
        uint32_t contentColor = 0;
        bool enableAnimation = false;
        uint32_t settingFlagNumber = 0;
        if (!data.ReadBool(enable)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read current enable error");
            return ERR_INVALID_DATA;
        }
        if (!data.ReadUint32(backgroundColor)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read current backgroundColor error");
            return ERR_INVALID_DATA;
        }
        if (!data.ReadUint32(contentColor)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read current contentColor error");
            return ERR_INVALID_DATA;
        }
        if (!data.ReadBool(enableAnimation)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read current enableAnimation error");
            return ERR_INVALID_DATA;
        }
        if (!data.ReadUint32(settingFlagNumber)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read current settingFlagNumber error");
            return ERR_INVALID_DATA;
        }
        SystemBarSettingFlag settingFlag = static_cast<SystemBarSettingFlag>(settingFlagNumber);
        SystemBarProperty property = { enable, backgroundColor, contentColor, enableAnimation, settingFlag };
        currentProperties[static_cast<WindowType>(type)] = property;
    }
    WSError errCode = GetTargetOrientationConfigInfo(targetOrientation, targetProperties, currentProperties);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleConvertOrientationAndRotation(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "in");
    uint32_t from;
    uint32_t to;
    int32_t value;
    int32_t convertedValue;
    if (!data.ReadUint32(from) || !data.ReadUint32(to) || !data.ReadInt32(value)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read value failed");
        return ERR_INVALID_DATA;
    }
    RotationInfoType fromRotationInfoType = static_cast<RotationInfoType>(from);
    if (fromRotationInfoType < RotationInfoType::WINDOW_ORIENTATION ||
        fromRotationInfoType > RotationInfoType::DISPLAY_ROTATION) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Invalid fromRotationInfoType: %{public}u", fromRotationInfoType);
        return ERR_INVALID_DATA;
    }
    RotationInfoType toRotationInfoType = static_cast<RotationInfoType>(to);
    if (toRotationInfoType < RotationInfoType::WINDOW_ORIENTATION ||
        toRotationInfoType > RotationInfoType::DISPLAY_ROTATION) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Invalid toRotationInfoType: %{public}u", toRotationInfoType);
        return ERR_INVALID_DATA;
    }
    WSError errCode = ConvertOrientationAndRotation(fromRotationInfoType, toRotationInfoType, value, convertedValue);
    if (!reply.WriteInt32(convertedValue)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Write failed");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ROTATION, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

/** @note @window.layout */
int SessionStub::HandleSetAspectRatio(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "In");
    float ratio = 0.0f;
    if (!data.ReadFloat(ratio)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read ratio failed");
        return ERR_INVALID_DATA;
    }
    WSError errCode = SetAspectRatio(ratio);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

/** @note @window.layout */
int SessionStub::HandleSetContentAspectRatio(MessageParcel& data, MessageParcel& reply)
{
    float ratio = 0.0f;
    if (!data.ReadFloat(ratio)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to read ratio");
        return ERR_INVALID_DATA;
    }
    bool isPersistent = true;
    if (!data.ReadBool(isPersistent)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to read isPersistent");
        return ERR_INVALID_DATA;
    }
    bool needUpdateRect = true;
    if (!data.ReadBool(needUpdateRect)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to read needUpdateRect");
        return ERR_INVALID_DATA;
    }
    WSError ret = SetContentAspectRatio(ratio, isPersistent, needUpdateRect);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SessionStub::HandleUpdateWindowSceneAfterCustomAnimation(MessageParcel& data, MessageParcel& reply)
{
    WLOGD("HandleUpdateWindowSceneAfterCustomAnimation!");
    bool isAdd = data.ReadBool();
    WSError errCode = UpdateWindowSceneAfterCustomAnimation(isAdd);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleSetLandscapeMultiWindow(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "HandleSetLandscapeMultiWindow!");
    bool isLandscapeMultiWindow = data.ReadBool();
    const WSError errCode = SetLandscapeMultiWindow(isLandscapeMultiWindow);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleGetIsMidScene(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "in");
    bool isMidScene = false;
    const WSError errCode = GetIsMidScene(isMidScene);
    if (!reply.WriteBool(isMidScene)) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Write isMidScene failed");
        return ERR_INVALID_DATA;
    }
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleTransferAbilityResult(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleTransferAbilityResult!");
    uint32_t resultCode = 0;
    if (!data.ReadUint32(resultCode)) {
        WLOGFE("Failed to read resultCode!");
        return ERR_TRANSACTION_FAILED;
    }
    std::shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        WLOGFE("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    WSError errCode = TransferAbilityResult(resultCode, *want);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleTransferExtensionData(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleTransferExtensionData!");
    std::shared_ptr<AAFwk::WantParams> wantParams(data.ReadParcelable<AAFwk::WantParams>());
    if (wantParams == nullptr) {
        WLOGFE("wantParams is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto errCode = TransferExtensionData(*wantParams);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleNotifySyncOn(MessageParcel& data, MessageParcel& reply)
{
    NotifySyncOn();
    return ERR_NONE;
}

int SessionStub::HandleNotifyAsyncOn(MessageParcel& data, MessageParcel& reply)
{
    NotifyAsyncOn();
    return ERR_NONE;
}

int SessionStub::HandleNotifyExtensionDied(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("called");
    NotifyExtensionDied();
    return ERR_NONE;
}

int SessionStub::HandleNotifyExtensionTimeout(MessageParcel& data, MessageParcel& reply)
{
    int32_t errorCode = 0;
    if (!data.ReadInt32(errorCode)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read eventId from parcel failed!");
        return ERR_INVALID_DATA;
    }
    NotifyExtensionTimeout(errorCode);
    return ERR_NONE;
}

int SessionStub::HandleTriggerBindModalUIExtension(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("called");
    TriggerBindModalUIExtension();
    return ERR_NONE;
}

int SessionStub::HandleTransferAccessibilityEvent(MessageParcel& data, MessageParcel& reply)
{
    sptr<Accessibility::AccessibilityEventInfoParcel> infoPtr =
        data.ReadStrongParcelable<Accessibility::AccessibilityEventInfoParcel>();
    if (infoPtr == nullptr) {
        return ERR_INVALID_DATA;
    }
    int64_t uiExtensionIdLevel = 0;
    if (!data.ReadInt64(uiExtensionIdLevel)) {
        WLOGFE("read uiExtensionIdLevel error");
        return ERR_INVALID_DATA;
    }
    NotifyTransferAccessibilityEvent(*infoPtr, uiExtensionIdLevel);
    return ERR_NONE;
}

int SessionStub::HandleNotifyPiPWindowPrepareClose(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PIP, "HandleNotifyPiPWindowPrepareClose");
    NotifyPiPWindowPrepareClose();
    return ERR_NONE;
}

int SessionStub::HandleUpdatePiPRect(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PIP, "HandleUpdatePiPRect!");
    int32_t posX = 0;
    int32_t posY = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t reason = 0;
    if (!data.ReadInt32(posX)) {
        TLOGE(WmsLogTag::WMS_PIP, "read posX error");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadInt32(posY)) {
        TLOGE(WmsLogTag::WMS_PIP, "read posY error");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadUint32(width)) {
        TLOGE(WmsLogTag::WMS_PIP, "read width error");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadUint32(height)) {
        TLOGE(WmsLogTag::WMS_PIP, "read height error");
        return ERR_INVALID_DATA;
    }
    Rect rect = {posX, posY, width, height};
    if (!data.ReadUint32(reason)) {
        TLOGE(WmsLogTag::WMS_PIP, "read reason error");
        return ERR_INVALID_DATA;
    }
    if (reason > static_cast<uint32_t>(SizeChangeReason::END)) {
        TLOGE(WmsLogTag::WMS_PIP, "Unknown reason");
        return ERR_INVALID_DATA;
    }
    WSError errCode = UpdatePiPRect(rect, static_cast<SizeChangeReason>(reason));
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

// LCOV_EXCL_START
int SessionStub::HandleUpdatePiPControlStatus(MessageParcel& data, MessageParcel& reply)
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    uint32_t controlType = 0;
    int32_t status = 0;
    if (data.ReadUint32(controlType) && data.ReadInt32(status)) {
        if (controlType > static_cast<uint32_t>(WsPiPControlType::END)) {
            TLOGE(WmsLogTag::WMS_PIP, "Unknown controlType");
            return ERR_INVALID_DATA;
        }
        if (status > static_cast<int32_t>(WsPiPControlStatus::PLAY) ||
            status < static_cast<int32_t>(WsPiPControlStatus::DISABLED)) {
            TLOGE(WmsLogTag::WMS_PIP, "Unknown status");
            return ERR_INVALID_DATA;
        }
        WSError errCode = UpdatePiPControlStatus(static_cast<WsPiPControlType>(controlType),
            static_cast<WsPiPControlStatus>(status));
        reply.WriteInt32(static_cast<int32_t>(errCode));
        return ERR_NONE;
    } else {
        return ERR_INVALID_DATA;
    }
}

int SessionStub::HandleSetAutoStartPiP(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PIP, "in");
    bool isAutoStart = false;
    if (!data.ReadBool(isAutoStart)) {
        TLOGE(WmsLogTag::WMS_PIP, "read isAutoStart error");
        return ERR_INVALID_DATA;
    }
    uint32_t priority = 0;
    if (!data.ReadUint32(priority)) {
        TLOGE(WmsLogTag::WMS_PIP, "read priority error");
        return ERR_INVALID_DATA;
    }
    uint32_t width = 0;
    if (!data.ReadUint32(width)) {
        TLOGE(WmsLogTag::WMS_PIP, "read width error");
        return ERR_INVALID_DATA;
    }
    uint32_t height = 0;
    if (!data.ReadUint32(height)) {
        TLOGE(WmsLogTag::WMS_PIP, "read height error");
        return ERR_INVALID_DATA;
    }
    WSError errCode = SetAutoStartPiP(isAutoStart, priority, width, height);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleUpdatePiPTemplateInfo(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PIP, "in");
    sptr<PiPTemplateInfo> pipTemplateInfo = data.ReadParcelable<PiPTemplateInfo>();
    if (pipTemplateInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "read pipTemplateInfo error");
        return ERR_INVALID_DATA;
    }
    WSError errCode = UpdatePiPTemplateInfo(*pipTemplateInfo);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PIP, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    TLOGI(WmsLogTag::WMS_PIP, "in HandleUpdatePiPTemplateInfo");
    return ERR_NONE;
}

int SessionStub::HandleSetPipParentWindowId(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PIP, "in");
    uint32_t pipParentWindowId = 0;
    if (!data.ReadUint32(pipParentWindowId)) {
        TLOGE(WmsLogTag::WMS_PIP, "read pipParentWindowId error");
        return ERR_INVALID_DATA;
    }
    WSError errCode = SetPipParentWindowId(pipParentWindowId);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PIP, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleIsPiPActive(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PIP, "in");
    bool status = false;
    WMError errCode = IsPiPActive(status);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_PIP, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteBool(status)) {
        TLOGE(WmsLogTag::WMS_PIP, "write status fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleUpdateFloatingBall(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "HandleUpdateFloatingBall");
    sptr<FloatingBallTemplateInfo> fbTemplateInfo = data.ReadParcelable<FloatingBallTemplateInfo>();
    if (fbTemplateInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "read fbTemplateInfo failed");
        return ERR_INVALID_DATA;
    }
    WMError errCode = UpdateFloatingBall(*fbTemplateInfo);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "write errCode fail");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleStopFloatingBall(MessageParcel& data, MessageParcel& reply)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "HandleStopFloatingBall");
    WSError errCode = StopFloatingBall();
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "write errCode fail");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleStartFloatingBallMainWindow(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "HandleStartFloatingBallMainWindow");
    std::shared_ptr<AAFwk::Want> want = std::shared_ptr<AAFwk::Want>(data.ReadParcelable<AAFwk::Want>());
    if (!want) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "read want error");
        return ERR_INVALID_DATA;
    }
    WMError errCode = RestoreFbMainWindow(want);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "write errCode fail");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleGetFloatingBallWindowId(MessageParcel& data, MessageParcel& reply)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "HandleGetFloatingBallWindowId");
    uint32_t windowId = 0;
    WMError errCode = GetFloatingBallWindowId(windowId);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "write errCode fail");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteUint32(windowId)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "write windowId fail");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}
// LCOV_EXCL_STOP

int SessionStub::HandleSetSystemEnableDrag(MessageParcel& data, MessageParcel& reply)
{
    bool enableDrag = false;
    if (!data.ReadBool(enableDrag)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read enableDrag failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT, "enableDrag: %{public}d", enableDrag);
    WMError errcode = SetSystemWindowEnableDrag(enableDrag);
    reply.WriteInt32(static_cast<int32_t>(errcode));
    return ERR_NONE;
}

int SessionStub::HandleProcessPointDownSession(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_EVENT, "called");
    int32_t posX = 0;
    int32_t posY = 0;
    if (!data.ReadInt32(posX) || !data.ReadInt32(posY)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Read failed!");
        return ERR_INVALID_DATA;
    }
    WSError errCode = ProcessPointDownSession(posX, posY);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleSendPointerEvenForMoveDrag(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleSendPointerEvenForMoveDrag!");
    auto pointerEvent = MMI::PointerEvent::Create();
    if (!pointerEvent) {
        TLOGE(WmsLogTag::WMS_EVENT, "create pointer event failed");
        return ERR_INVALID_DATA;
    }
    if (!pointerEvent->ReadFromParcel(data)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Read pointer event failed");
        return ERR_INVALID_DATA;
    }
    bool isExecuteDelayRaise = false;
    if (!data.ReadBool(isExecuteDelayRaise)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Read isExecuteDelayRaise failed");
        return ERR_INVALID_DATA;
    }
    WSError errCode = SendPointEventForMoveDrag(pointerEvent, isExecuteDelayRaise);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleIsStartMoving(MessageParcel& data, MessageParcel& reply)
{
    bool isMoving = IsStartMoving();
    if (!reply.WriteBool(isMoving)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write isMoving failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleUpdateRectChangeListenerRegistered(MessageParcel& data, MessageParcel& reply)
{
    bool isRegister = data.ReadBool();
    WSError errCode = UpdateRectChangeListenerRegistered(isRegister);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleSetCallingSessionId(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "run HandleSetCallingSessionId!");
    uint32_t callingSessionId = INVALID_WINDOW_ID;
    if (!data.ReadUint32(callingSessionId)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "callingSessionId read failed.");
        return ERR_INVALID_DATA;
    }
    SetCallingSessionId(callingSessionId);
    reply.WriteInt32(static_cast<int32_t>(WSError::WS_OK));
    return ERR_NONE;
}

int SessionStub::HandleSetCustomDecorHeight(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_DECOR, "In");
    int32_t height = 0;
    if (!data.ReadInt32(height)) {
        TLOGE(WmsLogTag::WMS_DECOR, "read height error");
        return ERR_INVALID_DATA;
    }
    SetCustomDecorHeight(height);
    return ERR_NONE;
}

int SessionStub::HandleSetDecorVisible(MessageParcel& data, MessageParcel& reply)
{
    bool isVisible = false;
    if (!data.ReadBool(isVisible)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to read isVisible");
        return ERR_INVALID_DATA;
    }
    SetDecorVisible(isVisible);
    return ERR_NONE;
}

int SessionStub::HandleAdjustKeyboardLayout(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "run HandleAdjustKeyboardLayout!");
    sptr<KeyboardLayoutParams> keyboardLayoutParams = data.ReadParcelable<KeyboardLayoutParams>();
    if (keyboardLayoutParams == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboardLayoutParams is nullptr.");
        return ERR_INVALID_DATA;
    }
    WSError ret = AdjustKeyboardLayout(*keyboardLayoutParams);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SessionStub::HandleUpdatePropertyByAction(MessageParcel& data, MessageParcel& reply)
{
    uint64_t actionValue = 0;
    if (!data.ReadUint64(actionValue)) {
        TLOGE(WmsLogTag::DEFAULT, "read action error");
        return ERR_INVALID_DATA;
    }
    if (actionValue < static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_RECT) ||
        actionValue > static_cast<uint64_t>(WSPropertyChangeAction::ACTION_UPDATE_END)) {
        TLOGE(WmsLogTag::DEFAULT, "invalid action");
        return ERR_INVALID_DATA;
    }
    auto action = static_cast<WSPropertyChangeAction>(actionValue);
    TLOGD(WmsLogTag::DEFAULT, "action: %{public}" PRIu64, action);
    sptr<WindowSessionProperty> property = nullptr;
    if (data.ReadBool()) {
        property = sptr<WindowSessionProperty>::MakeSptr();
        if (property != nullptr) {
            property->Read(data, action);
        }
    } else {
        TLOGW(WmsLogTag::DEFAULT, "Property not exist!");
    }
    const WMError ret = UpdateSessionPropertyByAction(property, action);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SessionStub::HandleGetAppForceLandscapeConfig(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DEFAULT, "called");
    AppForceLandscapeConfig config;
    WMError ret = GetAppForceLandscapeConfig(config);
    if (!reply.WriteParcelable(&config)) {
        TLOGE(WmsLogTag::DEFAULT, "write config failed");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
        TLOGE(WmsLogTag::DEFAULT, "write ret failed");
        return ERR_INVALID_DATA;
    }
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SessionStub::HandleGetAppForceLandscapeConfigEnable(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DEFAULT, "called");
    bool enableForceSplit = false;
    WMError ret = GetAppForceLandscapeConfigEnable(enableForceSplit);
    if (!reply.WriteBool(enableForceSplit)) {
        TLOGE(WmsLogTag::DEFAULT, "write enableForceSplit failed");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
        TLOGE(WmsLogTag::DEFAULT, "write ret failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleGetAppHookWindowInfoFromServer(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    HookWindowInfo hookWindowInfo{};
    WMError ret = GetAppHookWindowInfoFromServer(hookWindowInfo);
    if (!reply.WriteParcelable(&hookWindowInfo)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "write hookWindowInfo failed");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "write ret failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleNotifyWindowStatusDidChangeAfterShowWindow(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    NotifyWindowStatusDidChangeAfterShowWindow();
    return ERR_NONE;
}

int SessionStub::HandleSetDialogSessionBackGestureEnabled(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_DIALOG, "called");
    bool isEnabled = data.ReadBool();
    WSError ret = SetDialogSessionBackGestureEnabled(isEnabled);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SessionStub::HandleGetStatusBarHeight(MessageParcel& data, MessageParcel& reply)
{
    int32_t height = GetStatusBarHeight();
    TLOGD(WmsLogTag::WMS_IMMS, "StatusBarVectorHeight is %{public}d", height);
    reply.WriteInt32(height);
    return ERR_NONE;
}

int SessionStub::HandleNotifyExtensionEventAsync(MessageParcel& data, MessageParcel& reply)
{
    uint32_t notifyEvent = 0;
    if (!data.ReadUint32(notifyEvent)) {
        return ERR_TRANSACTION_FAILED;
    }
    NotifyExtensionEventAsync(notifyEvent);
    return ERR_NONE;
}

int SessionStub::HandleNotifyExtensionDetachToDisplay(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "in");
    NotifyExtensionDetachToDisplay();
    return ERR_NONE;
}

int SessionStub::HandleExtensionProviderData(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "in");
    static_cast<void>(SendExtensionData(data, reply, option));
    return ERR_NONE;
}

int SessionStub::HandleRequestFocus(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "in");
    bool isFocused = false;
    if (!data.ReadBool(isFocused)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "read isFocused failed");
        return ERR_INVALID_DATA;
    }
    WSError ret = RequestFocus(isFocused);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SessionStub::HandleSetGestureBackEnabled(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_IMMS, "in");
    bool isEnabled;
    if (!data.ReadBool(isEnabled)) {
        return ERR_INVALID_DATA;
    }
    WMError ret = SetGestureBackEnabled(isEnabled);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int SessionStub::HandleNotifySubModalTypeChange(MessageParcel& data, MessageParcel& reply)
{
    uint32_t subWindowModalType = 0;
    if (!data.ReadUint32(subWindowModalType)) {
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_HIERARCHY, "subWindowModalType: %{public}u", subWindowModalType);
    if (subWindowModalType > static_cast<uint32_t>(SubWindowModalType::END)) {
        return ERR_INVALID_DATA;
    }
    NotifySubModalTypeChange(static_cast<SubWindowModalType>(subWindowModalType));
    return ERR_NONE;
}

int SessionStub::HandleNotifyMainModalTypeChange(MessageParcel& data, MessageParcel& reply)
{
    bool isModal = false;
    if (!data.ReadBool(isModal)) {
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_MAIN, "isModal: %{public}d", isModal);
    NotifyMainModalTypeChange(isModal);
    return ERR_NONE;
}

int SessionStub::HandleSetWindowRectAutoSave(MessageParcel& data, MessageParcel& reply)
{
    bool enabled = true;
    if (!data.ReadBool(enabled)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Read enable failed.");
        return ERR_INVALID_DATA;
    }
    bool isSaveBySpecifiedFlag = false;
    if (!data.ReadBool(isSaveBySpecifiedFlag)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Read isSaveBySpecifiedFlag failed.");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_MAIN, "enabled: %{public}d, isSaveBySpecifiedFlag: %{public}d",
        enabled, isSaveBySpecifiedFlag);
    OnSetWindowRectAutoSave(enabled, isSaveBySpecifiedFlag);
    return ERR_NONE;
}

int SessionStub::HandleSetSupportedWindowModes(MessageParcel& data, MessageParcel& reply)
{
    uint32_t size = 0;
    if (!data.ReadUint32(size)) {
        return ERR_INVALID_DATA;
    }
    std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes;
    if (size > 0 && size <= WINDOW_SUPPORT_MODE_MAX_SIZE) {
        supportedWindowModes.reserve(size);
        for (uint32_t i = 0; i < size; i++) {
            supportedWindowModes.push_back(
                static_cast<AppExecFwk::SupportWindowMode>(data.ReadInt32()));
        }
    }
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "size: %{public}u", size);
    NotifySupportWindowModesChange(supportedWindowModes);
    return ERR_NONE;
}

int SessionStub::HandleSetSessionLabelAndIcon(MessageParcel& data, MessageParcel& reply)
{
    std::string label;
    if (!data.ReadString(label)) {
        TLOGE(WmsLogTag::WMS_MAIN, "read label failed");
        return ERR_INVALID_DATA;
    }
    std::shared_ptr<Media::PixelMap> icon(data.ReadParcelable<Media::PixelMap>());
    if (icon == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "read icon failed");
        return ERR_INVALID_DATA;
    }
    WSError errCode = SetSessionLabelAndIcon(label, icon);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_MAIN, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleChangeKeyboardEffectOption(MessageParcel& data, MessageParcel& reply)
{
    sptr<KeyboardEffectOption> effectOption = data.ReadStrongParcelable<KeyboardEffectOption>();
    if (effectOption == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Invalid data");
        return ERR_INVALID_DATA;
    }
    if (effectOption->viewMode_ >= KeyboardViewMode::VIEW_MODE_END ||
        effectOption->flowLightMode_ >= KeyboardFlowLightMode::END ||
        effectOption->gradientMode_ >= KeyboardGradientMode::END) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Invalid keyboard effectOption: %{public}s",
            effectOption->ToString().c_str());
        return ERR_INVALID_DATA;
    }
    ChangeKeyboardEffectOption(*effectOption);
    return ERR_NONE;
}

int SessionStub::HandleSetWindowCornerRadius(MessageParcel& data, MessageParcel& reply)
{
    float cornerRadius = 0.0f;
    if (!data.ReadFloat(cornerRadius)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read cornerRadius failed.");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "cornerRadius: %{public}f", cornerRadius);
    SetWindowCornerRadius(cornerRadius);
    return ERR_NONE;
}

int SessionStub::HandleSetWindowShadows(MessageParcel& data, MessageParcel& reply)
{
    sptr<ShadowsInfo> shadowsInfo = data.ReadParcelable<ShadowsInfo>();
    if (shadowsInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "read shadowsInfo error");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_ANIMATION, "shadow radius is %{public}f, color is %{public}s, "
        "offsetX is %{public}f, offsetY is %{public}f", shadowsInfo->radius_, shadowsInfo->color_.c_str(),
        shadowsInfo->offsetX_, shadowsInfo->offsetY_);
    WSError errCode = SetWindowShadows(*shadowsInfo);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    TLOGI(WmsLogTag::WMS_ANIMATION, "HandleSetWindowShadows end");
    return ERR_NONE;
}

int SessionStub::HandleSetFollowParentWindowLayoutEnabled(MessageParcel& data, MessageParcel& reply)
{
    bool isFollow = false;
    if (!data.ReadBool(isFollow)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read cornerRadius failed.");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_SUB, "isFollow: %{public}d", isFollow);
    WSError errCode = SetFollowParentWindowLayoutEnabled(isFollow);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_MAIN, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleSetWindowTransitionAnimation(MessageParcel& data, MessageParcel& reply)
{
    uint32_t type = 0;
    if (!data.ReadUint32(type)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Read type failed");
        return ERR_INVALID_DATA;
    }
    std::shared_ptr<TransitionAnimation> animation =
        std::shared_ptr<TransitionAnimation>(data.ReadParcelable<TransitionAnimation>());
    if (animation == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Read animation failed");
        return ERR_INVALID_DATA;
    }
    WSError errCode = SetWindowTransitionAnimation(static_cast<WindowTransitionType>(type), *animation);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write errCode fail");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleSetWindowAnchorInfo(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_SUB, "run");
    sptr<WindowAnchorInfo> windowAnchorInfo = data.ReadParcelable<WindowAnchorInfo>();
    if (windowAnchorInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "windowAnchorInfo is nullptr.");
        return ERR_INVALID_DATA;
    }
    WSError errCode = SetWindowAnchorInfo(*windowAnchorInfo);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_SUB, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleKeyFrameAnimateEnd(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "In");
    WSError errCode = KeyFrameAnimateEnd();
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleUpdateKeyFrameCloneNode(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "In");
    auto rsKeyFrameNode = RSWindowKeyFrameNode::ReadFromParcel(data);
    if (rsKeyFrameNode == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "fail get rsKeyFrameNode");
        return ERR_INVALID_DATA;
    }
    std::shared_ptr<RSTransaction> tranaction(data.ReadParcelable<RSTransaction>());
    if (!tranaction) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "fail get tranaction");
        return ERR_INVALID_DATA;
    }
    WSError errCode = UpdateKeyFrameCloneNode(rsKeyFrameNode, tranaction);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleSetDragKeyFramePolicy(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "In");
    sptr<KeyFramePolicy> keyFramePolicy = data.ReadParcelable<KeyFramePolicy>();
    if (keyFramePolicy == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read keyFramePolicy failed.");
        return ERR_INVALID_DATA;
    }
    WSError errCode = SetDragKeyFramePolicy(*keyFramePolicy);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleStartMovingWithCoordinate(MessageParcel& data, MessageParcel& reply)
{
    int32_t offsetX;
    if (!data.ReadInt32(offsetX)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Read offsetX failed!");
        return ERR_INVALID_DATA;
    }
    int32_t offsetY;
    if (!data.ReadInt32(offsetY)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Read offsetY failed!");
        return ERR_INVALID_DATA;
    }
    int32_t pointerPosX;
    if (!data.ReadInt32(pointerPosX)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Read pointerPosX failed!");
        return ERR_INVALID_DATA;
    }
    int32_t pointerPosY;
    if (!data.ReadInt32(pointerPosY)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Read pointerPosY failed!");
        return ERR_INVALID_DATA;
    }
    uint64_t displayId = DISPLAY_ID_INVALID;
    if (!data.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Read displayId failed!");
        return ERR_INVALID_DATA;
    }
    WSError errCode = StartMovingWithCoordinate(offsetX, offsetY, pointerPosX, pointerPosY, displayId);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleGetCrossAxisState(MessageParcel& data, MessageParcel& reply)
{
    CrossAxisState state = CrossAxisState::STATE_INVALID;
    GetCrossAxisState(state);
    if (!reply.WriteUint32(static_cast<uint32_t>(state))) {
        TLOGE(WmsLogTag::WMS_MAIN, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleGetWaterfallMode(MessageParcel& data, MessageParcel& reply)
{
    bool isWaterfallMode = false;
    GetWaterfallMode(isWaterfallMode);
    if (!reply.WriteBool(isWaterfallMode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleIsMainWindowFullScreenAcrossDisplays(MessageParcel& data, MessageParcel& reply)
{
    bool isAcrossDisplays = false;
    WMError ret = IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
    if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write ret fail.");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteBool(isAcrossDisplays)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write isAcrossDisplays fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleUseImplicitAnimation(MessageParcel& data, MessageParcel& reply)
{
    bool useImplicit = false;
    if (!data.ReadBool(useImplicit)) {
        TLOGE(WmsLogTag::WMS_PC, "Read useImplicit failed.");
        return ERR_INVALID_DATA;
    }
    UseImplicitAnimation(useImplicit);
    return ERR_NONE;
}

int SessionStub::HandleContainerModalEvent(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_EVENT, "In");
    std::string eventName;
    if (!data.ReadString(eventName)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Read eventName failed.");
        return ERR_INVALID_DATA;
    }
    std::string eventValue;
    if (!data.ReadString(eventValue)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Read eventValue failed.");
        return ERR_INVALID_DATA;
    }
    OnContainerModalEvent(eventName, eventValue);
    return ERR_NONE;
}

int SessionStub::HandleNotifyFollowParentMultiScreenPolicy(MessageParcel& data, MessageParcel& reply)
{
    bool enabled = false;
    if (!data.ReadBool(enabled)) {
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_SUB, "enabled: %{public}d", enabled);
    NotifyFollowParentMultiScreenPolicy(enabled);
    return ERR_NONE;
}

int SessionStub::HandleNotifyWindowAttachStateListenerRegistered(MessageParcel& data, MessageParcel& reply)
{
    bool registered = false;
    if (!data.ReadBool(registered)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "read registered failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_PATTERN, "registered: %{public}d", registered);
    NotifyWindowAttachStateListenerRegistered(registered);
    return ERR_NONE;
}

int SessionStub::HandleNotifyKeyboardDidShowRegistered(MessageParcel& data, MessageParcel& reply)
{
    bool registered = false;
    if (!data.ReadBool(registered)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "read registered failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_KEYBOARD, "registered: %{public}d", registered);
    NotifyKeyboardDidShowRegistered(registered);
    return ERR_NONE;
}

int SessionStub::HandleNotifyKeyboardDidHideRegistered(MessageParcel& data, MessageParcel& reply)
{
    bool registered = false;
    if (!data.ReadBool(registered)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "read registered failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_KEYBOARD, "registered: %{public}d", registered);
    NotifyKeyboardDidHideRegistered(registered);
    return ERR_NONE;
}

int SessionStub::HandleNotifyKeyboardWillShowRegistered(MessageParcel& data, MessageParcel& reply)
{
    bool registered = false;
    if (!data.ReadBool(registered)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "read registered failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_KEYBOARD, "registered: %{public}d", registered);
    NotifyKeyboardWillShowRegistered(registered);
    return ERR_NONE;
}

int SessionStub::HandleNotifyKeyboardWillHideRegistered(MessageParcel& data, MessageParcel& reply)
{
    bool registered = false;
    if (!data.ReadBool(registered)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "read registered failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_KEYBOARD, "registered: %{public}d", registered);
    NotifyKeyboardWillHideRegistered(registered);
    return ERR_NONE;
}

int SessionStub::HandleUpdateFlag(MessageParcel& data, MessageParcel& reply)
{
    std::string flag;
    if (!data.ReadString(flag)) {
        TLOGE(WmsLogTag::WMS_MAIN, "read flag failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_MAIN, "specifiedFlag: %{public}s", flag.c_str());
    UpdateFlag(flag);
    return ERR_NONE;
}

int SessionStub::HandleUpdateRotationChangeListenerRegistered(MessageParcel& data, MessageParcel& reply)
{
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read persistentId failed");
        return ERR_INVALID_DATA;
    }
    bool isRegister = false;
    if (!data.ReadBool(isRegister)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read isRegister failed");
        return ERR_INVALID_DATA;
    }
    WSError errCode = UpdateRotationChangeRegistered(persistentId, isRegister);
    TLOGD(WmsLogTag::WMS_ROTATION, "persistentId: %{public}d, register: %{public}d", persistentId, isRegister);
    return ERR_NONE;
}

int SessionStub::HandleUpdateScreenshotAppEventRegistered(MessageParcel& data, MessageParcel& reply)
{
    int32_t persistentId = 0;
    if (!data.ReadInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read persistentId failed");
        return ERR_INVALID_DATA;
    }
    bool isRegister = false;
    if (!data.ReadBool(isRegister)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read isRegister failed");
        return ERR_INVALID_DATA;
    }
    WMError errCode = UpdateScreenshotAppEventRegistered(persistentId, isRegister);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleUpdateAcrossDisplaysChangeRegistered(MessageParcel& data, MessageParcel& reply)
{
    bool isRegister = false;
    if (!data.ReadBool(isRegister)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read isRegister failed");
        return ERR_INVALID_DATA;
    }
    WMError errCode = UpdateAcrossDisplaysChangeRegistered(isRegister);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleGetIsHighlighted(MessageParcel& data, MessageParcel& reply)
{
    bool isHighlighted = false;
    GetIsHighlighted(isHighlighted);
    if (!reply.WriteBool(isHighlighted)) {
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleNotifyDisableDelegatorChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    WMError ret = NotifyDisableDelegatorChange();
    if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleSetSubWindowSource(MessageParcel& data, MessageParcel& reply)
{
    uint32_t sourceType = 0;
    if (!data.ReadUint32(sourceType)) {
        TLOGE(WmsLogTag::WMS_SUB, "Read sourceType failed.");
        return ERR_INVALID_DATA;
    }
    SubWindowSource source = static_cast<SubWindowSource>(sourceType);
    TLOGD(WmsLogTag::WMS_SUB, "source: %{public}d", source);
    WSError errCode = SetSubWindowSource(source);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_SUB, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleSetFrameRectForPartialZoomIn(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ANIMATION, "In");
    int32_t posX = 0;
    int32_t posY = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    if (!data.ReadInt32(posX) || !data.ReadInt32(posY) || !data.ReadUint32(width) || !data.ReadUint32(height)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "read frame rect failed");
        return ERR_INVALID_DATA;
    }
    Rect frameRect = { posX, posY, width, height };
    WSError ret = SetFrameRectForPartialZoomIn(frameRect);
    if (!reply.WriteUint32(static_cast<uint32_t>(ret))) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "write ret failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleUpdateGlobalDisplayRectFromClient(MessageParcel& data, MessageParcel& reply)
{
    int32_t posX = 0;
    int32_t posY = 0;
    int32_t width = 0;
    int32_t height = 0;
    if (!data.ReadInt32(posX) || !data.ReadInt32(posY) || !data.ReadInt32(width) || !data.ReadInt32(height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to read rect");
        return ERR_INVALID_DATA;
    }
    WSRect globalDisplayRect = { posX, posY, width, height };

    uint32_t reasonValue = 0;
    if (!data.ReadUint32(reasonValue)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to read reason");
        return ERR_INVALID_DATA;
    }
    SizeChangeReason reason = static_cast<SizeChangeReason>(reasonValue);
    if (reason < SizeChangeReason::UNDEFINED || reason >= SizeChangeReason::END) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Invalid reason: %{public}u", reasonValue);
        return ERR_INVALID_DATA;
    }
    UpdateGlobalDisplayRectFromClient(globalDisplayRect, reason);
    return ERR_NONE;
}

int SessionStub::ReadOnePendingSessionActivationConfig(MessageParcel& data, PendingSessionActivationConfig& config)
{
    if (!data.ReadBool(config.forceStart)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read forceStart failed.");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadBool(config.forceNewWant)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read forceNewWant failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleNotifyIsFullScreenInForceSplitMode(MessageParcel& data, MessageParcel& reply)
{
    bool isFullScreen = false;
    if (!data.ReadBool(isFullScreen)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Read isFullScreen failed.");
        return ERR_INVALID_DATA;
    }
    WSError errCode = NotifyIsFullScreenInForceSplitMode(isFullScreen);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_COMPAT, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleNotifyCompatibleModeChange(MessageParcel& data, MessageParcel& reply)
{
    int32_t mode = 0;
    if (!data.ReadInt32(mode)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Read mode failed.");
        return ERR_INVALID_DATA;
    }
    WSError errCode = NotifyCompatibleModeChange(static_cast<CompatibleStyleMode>(mode));
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_COMPAT, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleNotifyAppForceLandscapeConfigEnableUpdated(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "in");
    NotifyAppForceLandscapeConfigEnableUpdated();
    return ERR_NONE;
}

int SessionStub::HandleRestartApp(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    std::shared_ptr<AAFwk::Want> want = std::shared_ptr<AAFwk::Want>(data.ReadParcelable<AAFwk::Want>());
    if (!want) {
        TLOGE(WmsLogTag::WMS_LIFE, "read want error");
        return ERR_INVALID_DATA;
    }
    WSError errCode = RestartApp(want);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LIFE, "write errCode fail.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStub::HandleSendCommonEvent(MessageParcel& data, MessageParcel& reply)
{
    int32_t command = 0;
    if (!data.ReadInt32(command)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Read size failed");
        return ERR_INVALID_DATA;
    }

    std::vector<int32_t> parameters;
    int32_t length = 0;
    if (!data.ReadInt32(length)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Read size failed");
        return ERR_INVALID_DATA;
    }
    if (length > COMMON_EVENT_COMMAND_MAX_LENGTH || length < 0) {
        TLOGE(WmsLogTag::WMS_EVENT, "Out of range.");
        return ERR_INVALID_DATA;
    }
    parameters.emplace_back(length);

    int32_t info = 0;
    for (int i = 0; i < length; i++) {
        if (!data.ReadInt32(info)) {
            TLOGE(WmsLogTag::WMS_EVENT, "Read size failed");
            return ERR_INVALID_DATA;
        }
        parameters.emplace_back(info);
    }

    WMError ret = WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    switch (command) {
        case static_cast<uint32_t>(CommonEventCommand::LOCK_CURSOR):
            ret = LockCursor(parameters);
            break;
        case static_cast<uint32_t>(CommonEventCommand::UNLOCK_CURSOR):
            ret = UnlockCursor(parameters);
            break;
        case static_cast<uint32_t>(CommonEventCommand::SET_RECEIVE_DRAG_EVENT):
            ret = SetReceiveDragEventEnabled(parameters);
            break;
        case static_cast<uint32_t>(CommonEventCommand::SET_WINDOW_SEPARATION_TOUCH_ENABLED):
            ret = SetSeparationTouchEnabled(parameters);
            break;
        default:
            ret = WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
            break;
    }

    if (!reply.WriteUint32(static_cast<uint32_t>(ret))) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write errCode failed.");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}
} // namespace OHOS::Rosen
