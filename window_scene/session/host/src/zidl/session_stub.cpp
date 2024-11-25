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
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_ALL_AVOID_AREAS):
            return HandleGetAllAvoidAreas(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_ASPECT_RATIO):
            return HandleSetAspectRatio(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_WINDOW_ANIMATION_FLAG):
            return HandleSetWindowAnimationFlag(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_CUSTOM_ANIMATION):
            return HandleUpdateWindowSceneAfterCustomAnimation(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_LANDSCAPE_MULTI_WINDOW):
            return HandleSetLandscapeMultiWindow(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RAISE_ABOVE_TARGET):
            return HandleRaiseAboveTarget(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RAISE_APP_MAIN_WINDOW):
            return HandleRaiseAppMainWindowToTop(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR):
            return HandleChangeSessionVisibilityWithStatusBar(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_ACTIVE_PENDING_SESSION):
            return HandlePendingSessionActivation(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RESTORE_MAIN_WINDOW):
            return HandleRestoreMainWindow(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TERMINATE):
            return HandleTerminateSession(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_EXCEPTION):
            return HandleSessionException(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_PROCESS_POINT_DOWN_SESSION):
            return HandleProcessPointDownSession(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SEND_POINTEREVENT_FOR_MOVE_DRAG):
            return HandleSendPointerEvenForMoveDrag(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_START_MOVE_FLAG):
            return HandleGetStartMoveFlag(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_SYSTEM_DRAG_ENABLE):
            return HandleSetSystemEnableDrag(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_CLIENT_RECT):
            return HandleUpdateClientRect(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CALLING_SESSION_ID):
            return HandleSetCallingSessionId(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CUSTOM_DECOR_HEIGHT):
            return HandleSetCustomDecorHeight(data, reply);
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
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_LAYOUT_FULL_SCREEN_CHANGE):
            return HandleLayoutFullScreenChange(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_DEFAULT_DENSITY_ENABLED):
            return HandleDefaultDensityEnabled(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TITLE_AND_DOCK_HOVER_SHOW_CHANGE):
            return HandleTitleAndDockHoverShowChange(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_FORCE_LANDSCAPE_CONFIG):
            return HandleGetAppForceLandscapeConfig(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_STATUSBAR_HEIGHT):
            return HandleGetStatusBarHeight(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_DIALOG_SESSION_BACKGESTURE_ENABLE):
            return HandleSetDialogSessionBackGestureEnabled(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_FRAME_LAYOUT_FINISH):
            return HandleNotifyFrameLayoutFinish(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_REQUEST_FOCUS):
            return HandleRequestFocus(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_EXTENSION_EVENT_ASYNC):
            return HandleNotifyExtensionEventAsync(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_GESTURE_BACK_ENABLE):
            return HandleSetGestureBackEnabled(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_MODAL_TYPE_CHANGE):
            return HandleSessionModalTypeChange(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_MAIN_SESSION_MODAL_TYPE_CHANGE):
            return HandleMainSessionModalTypeChange(data, reply);
        case static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_RECT_AUTO_SAVE):
            return HandleSetWindowRectAutoSave(data, reply);
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
        reply.WriteBool(property->GetCompatibleModeInPc());
        reply.WriteInt32(property->GetCompatibleInPcPortraitWidth());
        reply.WriteInt32(property->GetCompatibleInPcPortraitHeight());
        reply.WriteInt32(property->GetCompatibleInPcLandscapeWidth());
        reply.WriteInt32(property->GetCompatibleInPcLandscapeHeight());
        reply.WriteBool(property->GetIsAppSupportPhoneInPc());
        reply.WriteBool(property->GetIsSupportDragInPcCompatibleMode());
        reply.WriteBool(property->GetIsPcAppInPad());
        reply.WriteBool(property->GetCompatibleModeEnableInPad());
        reply.WriteUint32(static_cast<uint32_t>(property->GetRequestedOrientation()));
        reply.WriteString(property->GetAppInstanceKey());
        reply.WriteBool(property->GetDragEnabled());
    }
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleNotifyFrameLayoutFinish(MessageParcel& data, MessageParcel& reply)
{
    bool notifyListener = data.ReadBool();
    WSRect rect = { data.ReadInt32(), data.ReadInt32(), data.ReadInt32(), data.ReadInt32() };
    NotifyFrameLayoutFinishFromApp(notifyListener, rect);
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
    TLOGD(WmsLogTag::WMS_LIFE, "Called!");
    WSError errCode = RemoveStartingWindow();
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
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
    if (eventId < static_cast<uint32_t>(SessionEvent::EVENT_MAXIMIZE) ||
        eventId > static_cast<uint32_t>(SessionEvent::EVENT_DRAG)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Invalid eventId: %{public}d", eventId);
        return ERR_INVALID_DATA;
    }
    WSError errCode = OnSessionEvent(static_cast<SessionEvent>(eventId));
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleSyncSessionEvent(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_EVENT, "In!");
    uint32_t eventId;
    if (!data.ReadUint32(eventId)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read eventId failed");
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
    TLOGD(WmsLogTag::WMS_LAYOUT, "isLayoutFullScreen: %{public}d", isLayoutFullScreen);
    WSError errCode = OnLayoutFullScreenChange(isLayoutFullScreen);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleDefaultDensityEnabled(MessageParcel& data, MessageParcel& reply)
{
    bool isDefaultDensityEnabled = false;
    if (!data.ReadBool(isDefaultDensityEnabled)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read isDefaultDensityEnabled failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_LAYOUT, "isDefaultDensityEnabled: %{public}d", isDefaultDensityEnabled);
    WSError errCode = OnDefaultDensityEnabled(isDefaultDensityEnabled);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleTitleAndDockHoverShowChange(MessageParcel& data, MessageParcel& reply)
{
    bool isTitleHoverShown = true;
    if (!data.ReadBool(isTitleHoverShown)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Read isTitleHoverShown failed.");
        return ERR_INVALID_DATA;
    }
    bool isDockHoverShown = true;
    if (!data.ReadBool(isDockHoverShown)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Read isDockHoverShown failed.");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_IMMS, "isTitleHoverShown: %{public}d, isDockHoverShown: %{public}d",
        isTitleHoverShown, isDockHoverShown);
    WSError errCode = OnTitleAndDockHoverShowChange(isTitleHoverShown, isDockHoverShown);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleRestoreMainWindow(MessageParcel& data, MessageParcel& reply)
{
    WSError errCode = OnRestoreMainWindow();
    reply.WriteUint32(static_cast<uint32_t>(errCode));
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
    WSError errCode = NotifySessionException(abilitySessionInfo);
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
    if (!data.ReadBool(abilitySessionInfo->isFromIcon)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read isFromIcon failed.");
        return ERR_INVALID_DATA;
    }
    bool hasStartWindowOption = false;
    if (!data.ReadBool(hasStartWindowOption)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read hasStartWindowOption failed.");
        return ERR_INVALID_DATA;
    }
    if (hasStartWindowOption) {
        auto startWindowOption = data.ReadParcelable<AAFwk::StartWindowOption>();
        abilitySessionInfo->startWindowOption.reset(startWindowOption);
    }
    WSError errCode = PendingSessionActivation(abilitySessionInfo);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

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
    TLOGI(WmsLogTag::WMS_LAYOUT, "rect:[%{public}d, %{public}d, %{public}u, %{public}u]", posX, posY,
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
    MoveConfiguration moveConfiguration;
    if (!data.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read displayId failed");
        return ERR_INVALID_DATA;
    }
    moveConfiguration.displayId_ = static_cast<DisplayId>(displayId);
    WSError errCode = UpdateSessionRect(rect, reason, isGlobal, isFromMoveToGlobal, moveConfiguration);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

/** @note @window.layout */
int SessionStub::HandleGetGlobalScaledRect(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "In");
    Rect tempRect;
    WMError errorCode = GetGlobalScaledRect(tempRect);
    reply.WriteInt32(tempRect.posX_);
    reply.WriteInt32(tempRect.posY_);
    reply.WriteUint32(tempRect.width_);
    reply.WriteUint32(tempRect.height_);
    reply.WriteInt32(static_cast<int32_t>(errorCode));
    return ERR_NONE;
}

/** @note @window.layout */
int SessionStub::HandleUpdateClientRect(MessageParcel& data, MessageParcel& reply)
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
    WSRect rect = { posX, posY, width, height };
    WSError errCode = UpdateClientRect(rect);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
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
        return ERR_INVALID_DATA;
    }
    AvoidAreaType type = static_cast<AvoidAreaType>(typeId);
    WLOGFD("HandleGetAvoidArea type:%{public}d", typeId);
    AvoidArea avoidArea = GetAvoidAreaByType(type);
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
    WSError errCode = TransferExtensionData(*wantParams);
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
    WSError errCode = SetAutoStartPiP(isAutoStart, priority);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleSetSystemEnableDrag(MessageParcel& data, MessageParcel& reply)
{
    bool enableDrag = data.ReadBool();
    TLOGI(WmsLogTag::WMS_LAYOUT, "handle, enableDrag: %{public}d", enableDrag);
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
    WSError errCode = SendPointEventForMoveDrag(pointerEvent);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleGetStartMoveFlag(MessageParcel& data, MessageParcel& reply)
{
    bool isMoving = false;
    WSError errCode = GetStartMoveFlag(isMoving);
    reply.WriteBool(isMoving);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
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
    TLOGD(WmsLogTag::WMS_LAYOUT, "run HandleSetCustomDecorHeight!");
    int32_t height = 0;
    if (!data.ReadInt32(height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read height error");
        return ERR_INVALID_DATA;
    }
    SetCustomDecorHeight(height);
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
    uint32_t actionValue = 0;
    if (!data.ReadUint32(actionValue)) {
        TLOGE(WmsLogTag::DEFAULT, "read action error");
        return ERR_INVALID_DATA;
    }
    if (actionValue < static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_RECT) ||
        actionValue > static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_MAIN_WINDOW_TOPMOST)) {
        TLOGE(WmsLogTag::DEFAULT, "invalid action");
        return ERR_INVALID_DATA;
    }
    auto action = static_cast<WSPropertyChangeAction>(actionValue);
    TLOGD(WmsLogTag::DEFAULT, "action:%{public}u", action);
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
    reply.WriteParcelable(&config);
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

int SessionStub::HandleSetDialogSessionBackGestureEnabled(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_DIALOG, "called");
    bool isEnabled = data.ReadBool();
    WSError ret = SetDialogSessionBackGestureEnabled(isEnabled);
    reply.WriteInt32(static_cast<int32_t>(ret));
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

int SessionStub::HandleNotifyExtensionEventAsync(MessageParcel& data, MessageParcel& reply)
{
    uint32_t notifyEvent = 0;
    if (!data.ReadUint32(notifyEvent)) {
        return ERR_TRANSACTION_FAILED;
    }
    NotifyExtensionEventAsync(notifyEvent);
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

int SessionStub::HandleSessionModalTypeChange(MessageParcel& data, MessageParcel& reply)
{
    uint32_t subWindowModalType = 0;
    if (!data.ReadUint32(subWindowModalType)) {
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_HIERARCHY, "subWindowModalType: %{public}u", subWindowModalType);
    WSError errCode = OnSessionModalTypeChange(static_cast<SubWindowModalType>(subWindowModalType));
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleMainSessionModalTypeChange(MessageParcel& data, MessageParcel& reply)
{
    bool isModal = false;
    if (!data.ReadBool(isModal)) {
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_MAIN, "isModal: %{public}d", isModal);
    WSError errCode = OnMainSessionModalTypeChange(isModal);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStub::HandleSetWindowRectAutoSave(MessageParcel& data, MessageParcel& reply)
{
    bool enabled = true;
    if (!data.ReadBool(enabled)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Read enable failed.");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_MAIN, "enabled: %{public}d", enabled);
    WSError errCode = OnSetWindowRectAutoSave(enabled);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}
} // namespace OHOS::Rosen
