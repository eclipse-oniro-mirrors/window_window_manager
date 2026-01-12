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

#include "session/container/include/zidl/session_stage_stub.h"
#include "session/container/include/zidl/session_stage_ipc_interface_code.h"

#include <ipc_types.h>
#include <transaction/rs_transaction.h>

#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "SessionStageStub"};
constexpr size_t MAX_PARCEL_CAPACITY = 100 * 1024 * 1024; // 100M
constexpr size_t CAPACITY_THRESHOLD = 8 * 100 * 1024; // 800k
constexpr size_t RESERVED_SPACE = 4 * 1024; // 4k

bool CalculateDumpInfoSize(const std::vector<std::string>& infos)
{
    size_t dataSize = 0;
    for (const auto& info : infos) {
        auto infoSize = info.length();
        if (MAX_PARCEL_CAPACITY - dataSize < infoSize) {
            return false;
        }

        dataSize += info.length();
    }
    return dataSize + RESERVED_SPACE <= CAPACITY_THRESHOLD;
}

bool WriteLittleStringVector(const std::vector<std::string>& infos, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "entry");
    reply.SetMaxCapacity(CAPACITY_THRESHOLD);
    if (!reply.WriteStringVector(infos)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write infos failed");
        return false;
    }
    return true;
}

bool WriteLargeStringVector(const std::vector<std::string>& infos, MessageParcel& reply)
{
    Parcel writeParcel;
    writeParcel.SetMaxCapacity(MAX_PARCEL_CAPACITY);
    if (!writeParcel.WriteInt32(static_cast<int32_t>(infos.size()))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write infosSize failed");
        return false;
    }

    for (const auto& info : infos) {
        if (!writeParcel.WriteString(info)) {
            TLOGE(WmsLogTag::WMS_UIEXT, "write info failed");
            return false;
        }
    }

    size_t dataSize = writeParcel.GetDataSize();
    TLOGD(WmsLogTag::WMS_UIEXT, "dataSize: %{public}zu", dataSize);
    if (!reply.WriteInt32(static_cast<int32_t>(dataSize))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write dataSize failed");
        return false;
    }

    if (!reply.WriteRawData(
        reinterpret_cast<uint8_t*>(writeParcel.GetData()), dataSize)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write rawData failed");
        return false;
    }

    return true;
}
}

int SessionStageStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    WLOGFD("Scene session stage on remote request!, code: %{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WLOGFE("Failed to check interface token!");
        return ERR_TRANSACTION_FAILED;
    }

    switch (code) {
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_ACTIVE):
            return HandleSetActive(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SIZE_CHANGE):
            return HandleUpdateRect(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DENSITY_CHANGE):
            return HandleUpdateDensity(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_ORIENTATION_CHANGE):
            return HandleUpdateOrientation(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_SESSION_VIEWPORT_CONFIG):
            return HandleUpdateSessionViewportConfig(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_HANDLE_BACK_EVENT):
            return HandleBackEventInner(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DESTROY):
            return HandleNotifyDestroy(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_FOCUS_CHANGE):
            return HandleUpdateFocus(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TRANSFER_COMPONENT_DATA):
            return HandleNotifyTransferComponentData(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TRANSFER_COMPONENT_DATA_SYNC):
            return HandleNotifyTransferComponentDataSync(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_OCCUPIED_AREA_CHANGE_INFO):
            return HandleNotifyOccupiedAreaChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_AVOID_AREA):
            return HandleUpdateAvoidArea(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SCREEN_SHOT):
            return HandleNotifyScreenshot(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SCREEN_SHOT_APP_EVENT):
            return HandleNotifyScreenshotAppEvent(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_DUMP_SESSSION_ELEMENT_INFO):
            return HandleDumpSessionElementInfo(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TOUCH_OUTSIDE):
            return HandleNotifyTouchOutside(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SECURE_LIMIT_CHANGE):
            return HandleNotifySecureLimitChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_WINDOW_MODE_CHANGE):
            return HandleUpdateWindowMode(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_GET_TOP_NAV_DEST_NAME):
            return HandleGetTopNavDestinationName(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_LAYOUT_FINISH_AFTER_WINDOW_MODE_CHANGE):
            return HandleNotifyLayoutFinishAfterWindowModeChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS):
            return HandleNotifyForegroundInteractiveStatus(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_MAXIMIZE_MODE_CHANGE):
            return HandleUpdateMaximizeMode(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_CLOSE_EXIST_PIP_WINDOW):
            return HandleNotifyCloseExistPipWindow(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SESSION_FOREGROUND):
            return HandleNotifySessionForeground(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SESSION_BACKGROUND):
            return HandleNotifySessionBackground(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TITLE_POSITION_CHANGE):
            return HandleUpdateTitleInTargetPos(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DENSITY_FOLLOW_HOST):
            return HandleNotifyDensityFollowHost(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_WINDOW_VISIBILITY_CHANGE):
            return HandleNotifyWindowVisibilityChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_WINDOW_OCCLUSION_STATE):
            return HandleNotifyWindowOcclusionState(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TRANSFORM_CHANGE):
            return HandleNotifyTransformChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SINGLE_HAND_TRANSFORM):
            return HandleNotifySingleHandTransformChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_GLOBAL_SCALED_RECT):
            return HandleNotifyGlobalScaledRectChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DIALOG_STATE_CHANGE):
            return HandleNotifyDialogStateChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_PIP_ACTION_EVENT):
            return HandleSetPipActionEvent(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_PIP_CONTROL_EVENT):
            return HandleSetPiPControlEvent(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DISPLAYID_CHANGE):
            return HandleUpdateDisplayId(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DISPLAY_MOVE):
            return HandleNotifyDisplayMove(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SWITCH_FREEMULTIWINDOW):
            return HandleSwitchFreeMultiWindow(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_GET_UI_CONTENT_REMOTE_OBJ):
            return HandleGetUIContentRemoteObj(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_KEYBOARD_INFO_CHANGE):
            return HandleNotifyKeyboardPanelInfoChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_PCAPPINPADNORMAL_CLOSE):
            return HandlePcAppInPadNormalClose(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_COMPATIBLE_MODE_PROPERTY_CHANGE):
            return HandleNotifyCompatibleModePropertyChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DENSITY_UNIQUE):
            return HandleSetUniqueVirtualPixelRatio(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SESSION_FULLSCREEN):
            return HandleNotifySessionFullScreen(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DUMP_INFO):
            return HandleNotifyDumpInfo(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_SPLIT_BUTTON_VISIBLE):
            return HandleSetSplitButtonVisible(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_ENABLE_DRAG_BY_SYSTEM):
            return HandleSetEnableDragBySystem(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_FULLSCREEN_WATERFALL_MODE):
            return HandleSetFullScreenWaterfallMode(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_SUPPORT_ENTER_WATERFALL_MODE):
            return HandleSetSupportEnterWaterfallMode(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SEND_EXTENSION_DATA):
            return HandleExtensionHostData(data, reply, option);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_LINK_KEYFRAME_NODE):
            return HandleLinkKeyFrameNode(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_STAGE_KEYFRAME_POLICY):
            return HandleSetStageKeyFramePolicy(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SEND_CONTAINER_MODAL_EVENT):
            return HandleSendContainerModalEvent(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_DRAG_ACTIVATED):
            return HandleSetDragActivated(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_HIGHLIGHT_CHANGE):
            return HandleNotifyHighlightChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_CROSS_AXIS):
            return HandleNotifyWindowCrossAxisChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_PIPSIZE_CHANGE):
            return HandleNotifyPipSizeChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_ACTIVE_STATUS_CHANGE):
            return HandleNotifyPiPActiveStatusChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_WINDOW_ATTACH_STATE_CHANGE):
            return HandleNotifyWindowAttachStateChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_KEYBOARD_ANIMATION_COMPLETED):
            return HandleNotifyKeyboardAnimationCompleted(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_KEYBOARD_ANIMATION_WILLBEGIN):
            return HandleNotifyKeyboardAnimationWillBegin(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_ROTATION_PROPERTY):
            return HandleNotifyRotationProperty(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_PAGE_ROTATION_ISIGNORED):
            return HandleNotifyPageRotationIsIgnored(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_ROTATION_CHANGE):
            return HandleNotifyRotationChange(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_CURRENT_ROTATION):
            return HandleSetCurrentRotation(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_APP_FORCE_LANDSCAPE_CONFIG_UPDATED):
            return HandleNotifyAppForceLandscapeConfigUpdated(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_APP_FORCE_LANDSCAPE_ENABLE_UPDATED):
            return HandleNotifyAppForceLandscapeConfigEnableUpdated(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_APP_HOOK_WINDOW_INFO_UPDATED):
            return HandleNotifyAppHookWindowInfoUpdated(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_PAUSED_STATUS):
            return HandleNotifyPausedStatus();
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_USE_CONTROL_STATUS):
            return HandleNotifyAppUseControlStatus(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_CLOSE_SPECIFIC_SCENE):
            return HandleCloseSpecificScene(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_GET_ROUTER_STACK_INFO):
            return HandleGetRouterStackInfo(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_WINDOW_MODE_FOR_UI_TEST):
            return HandleUpdateWindowModeForUITest(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_GLOBAL_DISPLAY_RECT):
            return HandleUpdateGlobalDisplayRectFromServer(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SEND_FB_ACTION_EVENT):
            return HandleSendFbActionEvent(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_UPDATE_SHOW_DECOR_IN_FREE_MULTI_WINDOW):
            return HandleUpdateIsShowDecorInFreeMultiWindow(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_ANIMATION_SPEED):
            return HandleUpdateAnimationSpeed(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_BRIGHTNESS):
            return HandleUpdateBrightness(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_ADD_SIDEBAR_BLUR):
            return HandleAddSidebarBlur(data, reply);
        case static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_SIDEBAR_BLUR_STYLE):
            return HandleSetSidebarBlurStyleWithType(data, reply);
        default:
            WLOGFE("Failed to find function handler!");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int SessionStageStub::HandleSetActive(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("SetActive!");
    bool active = data.ReadBool();
    WSError errCode = SetActive(active);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateRect(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    int32_t posX = 0;
    int32_t posY = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    if (!data.ReadInt32(posX) || !data.ReadInt32(posY) || !data.ReadUint32(width) || !data.ReadUint32(height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read rect failed");
        return -1;
    }
    WSRect rect = { posX, posY, width, height };
    uint32_t reasonType = 0;
    if (!data.ReadUint32(reasonType) || reasonType > static_cast<uint32_t>(SizeChangeReason::END)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read reasonType failed");
        return -1;
    }
    SizeChangeReason reason = static_cast<SizeChangeReason>(reasonType);
    bool hasRSTransaction = false;
    if (!data.ReadBool(hasRSTransaction)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read hasRSTransaction failed.");
        return -1;
    }
    sptr<SceneAnimationConfig> configPtr = data.ReadParcelable<SceneAnimationConfig>();
    if (configPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read SceneAnimationConfig failed");
        return -1;
    }
    bool isInnerProcess = false;
    if (!data.ReadBool(isInnerProcess)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read isInnerProcess failed.");
        return -1;
    }
    if (hasRSTransaction && !isInnerProcess) {
        std::shared_ptr<RSTransaction> transaction(data.ReadParcelable<RSTransaction>());
        if (!transaction) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "transaction unMarsh failed.");
            return -1;
        }
        configPtr->rsTransaction_ = transaction;
    }
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    uint32_t size = 0;
    if (!data.ReadUint32(size)) {
        TLOGE(WmsLogTag::WMS_IMMS, "read avoid area size failed");
        return -1;
    }
    constexpr uint32_t AVOID_AREA_TYPE_MAX_SIZE = 100;
    if (size > AVOID_AREA_TYPE_MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_IMMS, "avoid area size is invalid");
        return -1;
    }
    for (uint32_t i = 0; i < size; i++) {
        uint32_t type = data.ReadUint32();
        if (type < static_cast<uint32_t>(AvoidAreaType::TYPE_START) ||
            type >= static_cast<uint32_t>(AvoidAreaType::TYPE_END)) {
            TLOGE(WmsLogTag::WMS_IMMS, "read avoid area type failed");
            return -1;
        }
        sptr<AvoidArea> area = data.ReadParcelable<AvoidArea>();
        if (area == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "read avoid area failed");
            return -1;
        }
        avoidAreas[static_cast<AvoidAreaType>(type)] = *area;
    }
    WSError errCode = UpdateRect(rect, reason, *configPtr, avoidAreas);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateGlobalDisplayRectFromServer(MessageParcel& data, MessageParcel& reply)
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
    UpdateGlobalDisplayRectFromServer(globalDisplayRect, reason);
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateDensity(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("UpdateDensity!");
    UpdateDensity();
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateOrientation(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "HandleUpdateOrientation!");
    WSError errCode = UpdateOrientation();
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateSessionViewportConfig(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "HandleUpdateSessionViewportConfig!");
    SessionViewportConfig config;
    if (!data.ReadBool(config.isDensityFollowHost_) || !data.ReadFloat(config.density_) ||
        !data.ReadUint64(config.displayId_) || !data.ReadInt32(config.orientation_) ||
        !data.ReadUint32(config.transform_)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read HandleUpdateSessionViewportConfig data failed!");
        return ERR_INVALID_DATA;
    };
    UpdateSessionViewportConfig(config);
    return ERR_NONE;
}

int SessionStageStub::HandleBackEventInner(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleBackEventInner!");
    WSError errCode = HandleBackEvent();
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyDestroy(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("Notify Destroy");
    WSError errCode = NotifyDestroy();
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyCloseExistPipWindow(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PIP, "Notify Pip AlreadyExists");
    WSError errCode = NotifyCloseExistPipWindow();
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateFocus(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("UpdateFocus!");
    sptr<FocusNotifyInfo> focusNotifyInfo = data.ReadParcelable<FocusNotifyInfo>();
    if (focusNotifyInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to read focusNotifyInfo");
        return ERR_INVALID_DATA;
    }
    bool isFocused = false;
    if (!data.ReadBool(isFocused)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to read isFocused");
        return ERR_INVALID_DATA;
    }
    WSError errCode = UpdateFocus(focusNotifyInfo, isFocused);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyTransferComponentData(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleNotifyTransferComponentData!");
    std::shared_ptr<AAFwk::WantParams> wantParams(data.ReadParcelable<AAFwk::WantParams>());
    if (wantParams == nullptr) {
        WLOGFE("wantParams is nullptr");
        return ERR_INVALID_VALUE;
    }
    WSError errCode = NotifyTransferComponentData(*wantParams);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyTransferComponentDataSync(MessageParcel& data, MessageParcel& reply)
{
    std::shared_ptr<AAFwk::WantParams> wantParams(data.ReadParcelable<AAFwk::WantParams>());
    if (wantParams == nullptr) {
        WLOGFE("wantParams is nullptr");
        return static_cast<int>(WSErrorCode::WS_ERROR_TRANSFER_DATA_FAILED);
    }
    AAFwk::WantParams reWantParams;
    WSErrorCode errCode = NotifyTransferComponentDataSync(*wantParams, reWantParams);
    if (errCode != WSErrorCode::WS_OK) {
        return static_cast<int>(errCode);
    }
    if (!reply.WriteParcelable(&reWantParams)) {
        WLOGFE("reWantParams write failed.");
        return static_cast<int>(WSErrorCode::WS_ERROR_TRANSFER_DATA_FAILED);
    }
    return static_cast<int>(WSErrorCode::WS_OK);
}

int SessionStageStub::HandleNotifyOccupiedAreaChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "HandleNotifyOccupiedAreaChangeInfo!");
    sptr<OccupiedAreaChangeInfo> info(data.ReadParcelable<OccupiedAreaChangeInfo>());
    if (info == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Occupied info is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t posX = 0;
    int32_t posY = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    if (!(data.ReadInt32(posX) && data.ReadInt32(posY) && data.ReadUint32(width) && data.ReadUint32(height))) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Read callingSessionRect failed");
        return ERR_INVALID_VALUE;
    }
    Rect callingSessionRect = { posX, posY, width, height };
    std::map<AvoidAreaType, AvoidArea> avoidAreas = {};
    uint32_t size = 0;
    if (!data.ReadUint32(size)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Read avoid area size failed");
        return ERR_INVALID_VALUE;
    }
    constexpr uint32_t AVOID_AREA_TYPE_MAX_SIZE = 100;
    if (size > AVOID_AREA_TYPE_MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Avoid area size: %{public}d is invalid", size);
        return ERR_INVALID_VALUE;
    }
    for (uint32_t i = 0; i < size; i++) {
        uint32_t type = static_cast<uint32_t>(AvoidAreaType::TYPE_START);
        if (!data.ReadUint32(type)) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Read avoid area size failed");
            return ERR_INVALID_VALUE;
        }
        if (type < static_cast<uint32_t>(AvoidAreaType::TYPE_START) ||
            type >= static_cast<uint32_t>(AvoidAreaType::TYPE_END)) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "invalid avoid area type: %{public}d", type);
            return ERR_INVALID_VALUE;
        }
        sptr<AvoidArea> area = data.ReadParcelable<AvoidArea>();
        if (area == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Read avoid area failed");
            return ERR_INVALID_VALUE;
        }
        avoidAreas[static_cast<AvoidAreaType>(type)] = *area;
    }
    bool hasRSTransaction = data.ReadBool();
    if (hasRSTransaction) {
        std::shared_ptr<RSTransaction> transaction(data.ReadParcelable<RSTransaction>());
        if (!transaction) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "transaction unMarsh failed");
            return ERR_INVALID_VALUE;
        }
        NotifyOccupiedAreaChangeInfo(info, transaction, callingSessionRect, avoidAreas);
    } else {
        NotifyOccupiedAreaChangeInfo(info, nullptr, callingSessionRect, avoidAreas);
    }

    return ERR_NONE;
}

int SessionStageStub::HandleUpdateAvoidArea(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleUpdateAvoidArea!");
    sptr<AvoidArea> avoidArea = data.ReadStrongParcelable<AvoidArea>();
    if (!avoidArea) {
        return ERR_INVALID_VALUE;
    }
    uint32_t type = 0;
    if (!data.ReadUint32(type) ||
        type >= static_cast<uint32_t>(AvoidAreaType::TYPE_END)) {
        return ERR_INVALID_VALUE;
    }
    UpdateAvoidArea(avoidArea, static_cast<AvoidAreaType>(type));
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyScreenshot(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("Notify Screen shot!");
    NotifyScreenshot();
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyScreenshotAppEvent(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "called");
    int32_t screenshotEventType = static_cast<int32_t>(ScreenshotEventType::END);
    if (!data.ReadInt32(screenshotEventType)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read screenshotEventType failed");
        return ERR_INVALID_VALUE;
    }
    NotifyScreenshotAppEvent(static_cast<ScreenshotEventType>(screenshotEventType));
    return ERR_NONE;
}

int SessionStageStub::HandleDumpSessionElementInfo(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleDumpSessionElementInfo!");
    std::vector<std::string> params;
    if (!data.ReadStringVector(&params)) {
        WLOGFE("Fail to read params");
        return -1;
    }
    DumpSessionElementInfo(params);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyTouchOutside(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleNotifyTouchOutside!");
    NotifyTouchOutside();
    return ERR_NONE;
}

int SessionStageStub::HandleNotifySecureLimitChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "HandleNotifySecureLimitChange!");
    bool isLimit = true;
    if (!data.ReadBool(isLimit)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read isLimit failed.");
        return ERR_INVALID_DATA;
    }
    NotifyExtensionSecureLimitChange(isLimit);
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateWindowMode(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleUpdateWindowMode!");
    WindowMode mode = static_cast<WindowMode>(data.ReadUint32());
    WSError errCode = UpdateWindowMode(mode);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleGetTopNavDestinationName(MessageParcel& data, MessageParcel& reply)
{
    std::string topNavDestName;
    WSError errCode = GetTopNavDestinationName(topNavDestName);
    if (errCode != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "get stage failed, errCode=%{public}d", static_cast<int32_t>(errCode));
        return ERR_INVALID_DATA;
    }
    uint32_t size = static_cast<uint32_t>(topNavDestName.length());
    if (!reply.WriteUint32(size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write the size of stage top page name failed, size=%{public}u", size);
        return ERR_INVALID_DATA;
    }
    if (size > 0 && !reply.WriteRawData(topNavDestName.c_str(), size)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write stage top page name failed, name=%{public}s", topNavDestName.c_str());
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write stage error code failed");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyLayoutFinishAfterWindowModeChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    uint32_t mode = static_cast<uint32_t>(WindowMode::WINDOW_MODE_UNDEFINED);
    if (!data.ReadUint32(mode)) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Failed to read mode");
        return ERR_INVALID_DATA;
    }
    WSError errCode = NotifyLayoutFinishAfterWindowModeChange(static_cast<WindowMode>(mode));
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateWindowModeForUITest(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    int32_t updateMode = 0;
    if (!data.ReadInt32(updateMode)) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Failed to read updateMode");
        return ERR_INVALID_DATA;
    }
    WMError errCode = UpdateWindowModeForUITest(updateMode);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to write errCode");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyWindowVisibilityChange(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleNotifyWindowVisibilityChange!");
    bool isVisible = data.ReadBool();
    WSError errCode = NotifyWindowVisibility(isVisible);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyWindowOcclusionState(MessageParcel& data, MessageParcel& reply)
{
    uint32_t state = static_cast<uint32_t>(WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    if (!data.ReadUint32(state)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read visibility state failed");
        return ERR_INVALID_DATA;
    }
    if (state > static_cast<uint32_t>(WindowVisibilityState::END)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "invalid visibility state: %{public}u", state);
        return ERR_INVALID_DATA;
    }
    auto errCode = NotifyWindowOcclusionState(static_cast<WindowVisibilityState>(state));
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write failed: errCode=%{public}d", static_cast<int32_t>(errCode));
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyForegroundInteractiveStatus(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("NotifyForegroundInteractiveStatus!");
    bool interactive = data.ReadBool();
    NotifyForegroundInteractiveStatus(interactive);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyAppUseControlStatus(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LIFE, "called");
    bool useControlState = false;
    if (!data.ReadBool(useControlState)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to read useControlState");
        return ERR_INVALID_DATA;
    }
    NotifyAppUseControlStatus(useControlState);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyPausedStatus()
{
    TLOGD(WmsLogTag::WMS_LIFE, "called");
    NotifyLifecyclePausedStatus();
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateMaximizeMode(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleUpdateMaximizeMode!");
    MaximizeMode mode = static_cast<MaximizeMode>(data.ReadUint32());
    WSError errCode = UpdateMaximizeMode(mode);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleNotifySessionForeground(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleNotifySessionForeground");
    uint32_t reason = data.ReadUint32();
    bool withAnimation = data.ReadBool();
    NotifySessionForeground(reason, withAnimation);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifySessionFullScreen(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "called");
    bool fullScreen = data.ReadBool();
    NotifySessionFullScreen(fullScreen);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifySessionBackground(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleNotifySessionBackground");
    uint32_t reason = data.ReadUint32();
    bool withAnimation = data.ReadBool();
    bool isFromInnerkits = data.ReadBool();
    NotifySessionBackground(reason, withAnimation, isFromInnerkits);
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateTitleInTargetPos(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_DECOR, "called");
    bool isShow = data.ReadBool();
    int32_t height = data.ReadInt32();
    WSError errCode = UpdateTitleInTargetPos(isShow, height);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyTransformChange(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("HandleNotifyTransformChange!");
    Transform transform;
    transform.Unmarshalling(data);
    NotifyTransformChange(transform);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifySingleHandTransformChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    SingleHandTransform singleHandTransform;
    singleHandTransform.Unmarshalling(data);
    NotifySingleHandTransformChange(singleHandTransform);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyGlobalScaledRectChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    Rect globalScaledRect;
    globalScaledRect.Unmarshalling(data);
    NotifyGlobalScaledRectChange(globalScaledRect);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyDensityFollowHost(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "HandleNotifyDensityFollowHost");
    bool isFollowHost = data.ReadBool();
    float densityValue = data.ReadFloat();
    NotifyDensityFollowHost(isFollowHost, densityValue);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyDialogStateChange(MessageParcel& data, MessageParcel& reply)
{
    WLOGD("HandleNotifyDialogStateChange!");
    bool isForeground = data.ReadBool();
    NotifyDialogStateChange(isForeground);
    return ERR_NONE;
}

int SessionStageStub::HandleSetPipActionEvent(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PIP, "HandleSetPipActionEvent");
    std::string action = data.ReadString();
    if (action.empty()) {
        TLOGE(WmsLogTag::WMS_PIP, "SessionStageStub pip action event is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t status;
    if (!data.ReadInt32(status)) {
        return ERR_INVALID_VALUE;
    }
    SetPipActionEvent(action, status);
    return ERR_NONE;
}

int SessionStageStub::HandleSendFbActionEvent(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "HandleSendFbActionEvent");
    std::string action;
    if (!data.ReadString(action)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Read action failed.");
        reply.WriteInt32(static_cast<int32_t>(WSError::WS_ERROR_IPC_FAILED));
        return ERR_INVALID_VALUE;
    }
    auto error = SendFbActionEvent(action);
    if (!reply.WriteInt32(static_cast<int32_t>(error))) {
        return ERR_INVALID_VALUE;
    }
    return ERR_NONE;
}

int SessionStageStub::HandleSetPiPControlEvent(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PIP, "called");
    uint32_t controlType;
    if (!data.ReadUint32(controlType)) {
        return ERR_INVALID_VALUE;
    }
    int32_t status;
    if (!data.ReadInt32(status)) {
        return ERR_INVALID_VALUE;
    }
    SetPiPControlEvent(static_cast<WsPiPControlType>(controlType), static_cast<WsPiPControlStatus>(status));
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateDisplayId(MessageParcel& data, MessageParcel& reply)
{
    WLOGD("UpdateDisplayId!");
    uint64_t displayId = data.ReadUint64();
    WSError errCode = UpdateDisplayId(displayId);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyDisplayMove(MessageParcel& data, MessageParcel& reply)
{
    WLOGD("HandleNotifyDisplayMove!");
    DisplayId from = static_cast<DisplayId>(data.ReadUint64());
    DisplayId to = static_cast<DisplayId>(data.ReadUint64());
    NotifyDisplayMove(from, to);
    return ERR_NONE;
}

int SessionStageStub::HandleSwitchFreeMultiWindow(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "called!");
    bool enable = data.ReadBool();
    WSError errCode = SwitchFreeMultiWindow(enable);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleGetUIContentRemoteObj(MessageParcel& data, MessageParcel& reply)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Called");
    sptr<IRemoteObject> uiContentRemoteObj;
    WSError errCode = GetUIContentRemoteObj(uiContentRemoteObj);
    reply.WriteRemoteObject(uiContentRemoteObj);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyKeyboardPanelInfoChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "HandleNotifyKeyboardPanelInfoChange!");
    sptr<KeyboardPanelInfo> keyboardPanelInfo = data.ReadParcelable<KeyboardPanelInfo>();
    if (keyboardPanelInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboardPanelInfo is nullptr!");
        return ERR_INVALID_VALUE;
    }
    NotifyKeyboardPanelInfoChange(*keyboardPanelInfo);

    return ERR_NONE;
}

int SessionStageStub::HandlePcAppInPadNormalClose(MessageParcel& data, MessageParcel& reply)
{
    WSError errCode = PcAppInPadNormalClose();
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyCompatibleModePropertyChange(MessageParcel& data, MessageParcel& reply)
{
    sptr<CompatibleModeProperty> property = data.ReadParcelable<CompatibleModeProperty>();
    WSError errCode = NotifyCompatibleModePropertyChange(property);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int SessionStageStub::HandleSetUniqueVirtualPixelRatio(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "HandleSetUniqueVirtualPixelRatio!");
    bool useUniqueDensity = data.ReadBool();
    float densityValue = data.ReadFloat();
    SetUniqueVirtualPixelRatio(useUniqueDensity, densityValue);
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateAnimationSpeed(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ANIMATION, "HandleUpdateAnimationSpeed!");
    float speed = 0.0f;
    if (!data.ReadFloat(speed)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Read speed failed!");
        return ERR_INVALID_DATA;
    }
    UpdateAnimationSpeed(speed);
    return ERR_NONE;
}

int SessionStageStub::HandleSetSplitButtonVisible(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    bool isVisible = true;
    if (!data.ReadBool(isVisible)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read isVisible failed.");
        return ERR_INVALID_DATA;
    }
    SetSplitButtonVisible(isVisible);
    return ERR_NONE;
}

int SessionStageStub::HandleSetEnableDragBySystem(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    bool enableDrag = true;
    if (!data.ReadBool(enableDrag)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read enableDrag failed.");
        return ERR_INVALID_DATA;
    }
    SetEnableDragBySystem(enableDrag);
    return ERR_NONE;
}

int SessionStageStub::HandleSetDragActivated(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    bool dragActivated = true;
    if (!data.ReadBool(dragActivated)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read dragActivated failed.");
        return ERR_INVALID_DATA;
    }
    SetDragActivated(dragActivated);
    return ERR_NONE;
}

int SessionStageStub::HandleSetFullScreenWaterfallMode(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    bool isWaterfallMode = false;
    if (!data.ReadBool(isWaterfallMode)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read isWaterfallMode failed.");
        return ERR_INVALID_DATA;
    }
    SetFullScreenWaterfallMode(isWaterfallMode);
    return ERR_NONE;
}

int SessionStageStub::HandleSetSupportEnterWaterfallMode(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "in");
    bool isSupportEnter = false;
    if (!data.ReadBool(isSupportEnter)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Read isSupportEnter failed.");
        return ERR_INVALID_DATA;
    }
    SetSupportEnterWaterfallMode(isSupportEnter);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyDumpInfo(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "entry");
    std::vector<std::string> params;
    if (!data.ReadStringVector(&params)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to read string vector");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> infos;
    WSError errCode = NotifyDumpInfo(params, infos);
    bool isLittleSize = CalculateDumpInfoSize(infos);
    if (!reply.WriteBool(isLittleSize)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write isLittleSize failed");
        return ERR_TRANSACTION_FAILED;
    }

    bool writeResult = isLittleSize ? WriteLittleStringVector(infos, reply) :
        WriteLargeStringVector(infos, reply);
    if (!writeResult) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write data failed");
        return ERR_TRANSACTION_FAILED;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write errCode failed");
        return ERR_TRANSACTION_FAILED;
    }

    return ERR_NONE;
}

int SessionStageStub::HandleExtensionHostData(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "in");
    static_cast<void>(SendExtensionData(data, reply, option));
    return ERR_NONE;
}

int SessionStageStub::HandleLinkKeyFrameNode(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    LinkKeyFrameNode();
    return ERR_NONE;
}

int SessionStageStub::HandleSetStageKeyFramePolicy(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    sptr<KeyFramePolicy> keyFramePolicy = data.ReadParcelable<KeyFramePolicy>();
    if (!keyFramePolicy) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read keyFramePolicy failed.");
        return ERR_INVALID_DATA;
    }
    SetStageKeyFramePolicy(*keyFramePolicy);
    return ERR_NONE;
}

int SessionStageStub::HandleSendContainerModalEvent(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_EVENT, "in");
    std::string eventName = data.ReadString();
    if (eventName.empty()) {
        TLOGE(WmsLogTag::WMS_EVENT, "event name is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string eventValue = data.ReadString();
    if (eventValue.empty()) {
        TLOGE(WmsLogTag::WMS_EVENT, "event value  is nullptr");
        return ERR_INVALID_VALUE;
    }
    SendContainerModalEvent(eventName, eventValue);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyHighlightChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "called!");
    sptr<HighlightNotifyInfo> highlightNotifyInfo = data.ReadParcelable<HighlightNotifyInfo>();
    if (highlightNotifyInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to read highlightNotifyInfo");
        return ERR_INVALID_DATA;
    }
    bool isHighlight = false;
    if (!data.ReadBool(isHighlight)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Read isHighlight failed.");
        return ERR_INVALID_DATA;
    }
    NotifyHighlightChange(highlightNotifyInfo, isHighlight);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyWindowCrossAxisChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "in");
    uint32_t state = 0;
    if (!data.ReadUint32(state)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Read cross axis state failed.");
        return ERR_INVALID_DATA;
    }
    if (state >= static_cast<uint32_t>(CrossAxisState::STATE_END)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "invalid cross axis state.");
        return ERR_INVALID_DATA;
    }
    NotifyWindowCrossAxisChange(static_cast<CrossAxisState>(state));
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyPipSizeChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PIP, "in");
    double width;
    if (!data.ReadDouble(width)) {
        return ERR_INVALID_VALUE;
    }
    double height;
    if (!data.ReadDouble(height)) {
        return ERR_INVALID_VALUE;
    }
    double scale;
    if (!data.ReadDouble(scale)) {
        return ERR_INVALID_VALUE;
    }
    NotifyPipWindowSizeChange(width, height, scale);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyPiPActiveStatusChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PIP, "in");
    bool status;
    if (!data.ReadBool(status)) {
        TLOGE(WmsLogTag::WMS_PIP, "Read active status failed.");
        return ERR_INVALID_VALUE;
    }
    NotifyPiPActiveStatusChange(status);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyWindowAttachStateChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_SUB, "in");
    bool isAttach = false;
    if (!data.ReadBool(isAttach)) {
        return ERR_INVALID_DATA;
    }
    NotifyWindowAttachStateChange(isAttach);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyKeyboardAnimationCompleted(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    sptr<KeyboardPanelInfo> keyboardPanelInfo = data.ReadParcelable<KeyboardPanelInfo>();
    if (keyboardPanelInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboardPanelInfo is nullptr!");
        return ERR_INVALID_VALUE;
    }
    NotifyKeyboardAnimationCompleted(*keyboardPanelInfo);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyRotationProperty(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "in");
    uint32_t rotation = 0;
    if (!data.ReadUint32(rotation)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read rotation failed");
        return ERR_INVALID_VALUE;
    }

    Rect rect = {0, 0, 0, 0};
    if (!data.ReadInt32(rect.posX_) || !data.ReadInt32(rect.posY_) ||
        !data.ReadUint32(rect.width_) ||!data.ReadUint32(rect.height_)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read rect failed");
        return ERR_INVALID_VALUE;
    }
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    uint32_t size = 0;
    if (!data.ReadUint32(size)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read avoid area size failed");
        return ERR_INVALID_VALUE;
    }
    constexpr uint32_t AVOID_AREA_TYPE_MAX_SIZE = 100;
    if (size > AVOID_AREA_TYPE_MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "avoid area size is invalid");
        return ERR_INVALID_VALUE;
    }
    for (uint32_t i = 0; i < size; i++) {
        uint32_t type = data.ReadUint32();
        if (type < static_cast<uint32_t>(AvoidAreaType::TYPE_START) ||
            type >= static_cast<uint32_t>(AvoidAreaType::TYPE_END)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "avoid area type invalid");
            return ERR_INVALID_VALUE;
        }
        sptr<AvoidArea> area = data.ReadParcelable<AvoidArea>();
        if (area == nullptr) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read avoid area invalid");
            return ERR_INVALID_VALUE;
        }
        avoidAreas[static_cast<AvoidAreaType>(type)] = *area;
    }

    uint32_t currentRotation = 0;
    if (!data.ReadUint32(currentRotation)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read current rotation failed");
        return ERR_INVALID_VALUE;
    }

    Rect currentRect = {0, 0, 0, 0};
    if (!data.ReadInt32(currentRect.posX_) || !data.ReadInt32(currentRect.posY_) ||
        !data.ReadUint32(currentRect.width_) ||!data.ReadUint32(currentRect.height_)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read current rect failed");
        return ERR_INVALID_VALUE;
    }
    std::map<AvoidAreaType, AvoidArea> currentAvoidAreas;
    uint32_t currentSize = 0;
    if (!data.ReadUint32(currentSize)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read current avoid area size failed");
        return ERR_INVALID_VALUE;
    }
    if (currentSize > AVOID_AREA_TYPE_MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "current avoid area size is invalid");
        return ERR_INVALID_VALUE;
    }
    for (uint32_t i = 0; i < currentSize; i++) {
        uint32_t type = 0;
        if (!data.ReadUint32(type)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read current avoid area type failed");
            return ERR_INVALID_VALUE;
        }
        if (type < static_cast<uint32_t>(AvoidAreaType::TYPE_START) ||
            type >= static_cast<uint32_t>(AvoidAreaType::TYPE_END)) {
            TLOGE(WmsLogTag::WMS_ROTATION, "current avoid area type invalid");
            return ERR_INVALID_VALUE;
        }
        sptr<AvoidArea> currentArea = data.ReadParcelable<AvoidArea>();
        if (currentArea == nullptr) {
            TLOGE(WmsLogTag::WMS_ROTATION, "read current avoid area failed");
            return ERR_INVALID_VALUE;
        }
        currentAvoidAreas[static_cast<AvoidAreaType>(type)] = *currentArea;
    }

    OrientationInfo info = { rotation, rect, avoidAreas };
    OrientationInfo currentInfo = { currentRotation, currentRect, currentAvoidAreas };
    NotifyTargetRotationInfo(info, currentInfo);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyPageRotationIsIgnored(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "in");
    NotifyPageRotationIsIgnored();
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyRotationChange(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "in");
    uint32_t type = 0;
    if (!data.ReadUint32(type)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read type failed");
        return ERR_INVALID_DATA;
    }
    uint32_t orientation = 0;
    if (!data.ReadUint32(orientation)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read orientation failed");
        return ERR_INVALID_DATA;
    }
    uint64_t displayId = 0;
    if (!data.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read displayId failed");
        return ERR_INVALID_DATA;
    }

    Rect rect = { 0, 0, 0, 0 };
    if (!data.ReadInt32(rect.posX_) || !data.ReadInt32(rect.posY_) ||
        !data.ReadUint32(rect.width_) || !data.ReadUint32(rect.height_)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read rect failed");
        return ERR_INVALID_DATA;
    }
    RotationChangeInfo rotationChangeInfo = { static_cast<RotationChangeType>(type), orientation, displayId, rect };
    RotationChangeResult rotationChangeResult = NotifyRotationChange(rotationChangeInfo);
    if (rotationChangeInfo.type_ == RotationChangeType::WINDOW_DID_ROTATE) {
        TLOGI(WmsLogTag::WMS_ROTATION, "WINDOW_DID_ROTATE return");
        return ERR_NONE;
    }
    if (!reply.WriteUint32(static_cast<uint32_t>(rotationChangeResult.rectType_))) {
        TLOGE(WmsLogTag::WMS_ROTATION, "send rectType failed");
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(rotationChangeResult.windowRect_.posX_) ||
        !reply.WriteInt32(rotationChangeResult.windowRect_.posY_) ||
        !reply.WriteUint32(rotationChangeResult.windowRect_.width_) ||
        !reply.WriteUint32(rotationChangeResult.windowRect_.height_)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "send window rect failed");
        return ERR_INVALID_DATA;
    }
    TLOGI(WmsLogTag::WMS_ROTATION, "send type:%{public}d, rect: [%{public}d, %{public}d, %{public}d, %{public}d]",
        rotationChangeResult.rectType_, rotationChangeResult.windowRect_.posX_, rotationChangeResult.windowRect_.posY_,
        rotationChangeResult.windowRect_.width_, rotationChangeResult.windowRect_.height_);
    return ERR_NONE;
}

int SessionStageStub::HandleSetCurrentRotation(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "in");
    int32_t currentRotation;
    if (!data.ReadInt32(currentRotation)) {
        return ERR_INVALID_VALUE;
    }
    SetCurrentRotation(currentRotation);
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyAppForceLandscapeConfigUpdated(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    NotifyAppForceLandscapeConfigUpdated();
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyAppForceLandscapeConfigEnableUpdated(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    NotifyAppForceLandscapeConfigEnableUpdated();
    return ERR_NONE;
}
int SessionStageStub::HandleNotifyAppHookWindowInfoUpdated(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    NotifyAppHookWindowInfoUpdated();
    return ERR_NONE;
}

int SessionStageStub::HandleNotifyKeyboardAnimationWillBegin(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "in");
    sptr<KeyboardAnimationInfo> keyboardAnimationInfo = data.ReadParcelable<KeyboardAnimationInfo>();
    if (keyboardAnimationInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboardAnimationInfo is nullptr!");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<RSTransaction> transaction(data.ReadParcelable<RSTransaction>());
    NotifyKeyboardAnimationWillBegin(*keyboardAnimationInfo, transaction);
    return ERR_NONE;
}

int SessionStageStub::HandleGetRouterStackInfo(MessageParcel& data, MessageParcel& reply)
{
    std::string routerStackInfo;
    WMError errCode = GetRouterStackInfo(routerStackInfo);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteString(routerStackInfo)) {
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStageStub::HandleCloseSpecificScene(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_EVENT, "in");
    CloseSpecificScene();
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateIsShowDecorInFreeMultiWindow(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_DECOR, "called!");
    bool isShow = true;
    if (!data.ReadBool(isShow)) {
        TLOGE(WmsLogTag::WMS_DECOR, "read isShow failed");
        return ERR_INVALID_DATA;
    }
    WSError errCode = UpdateIsShowDecorInFreeMultiWindow(isShow);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int SessionStageStub::HandleUpdateBrightness(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "in");
    float brightness;
    if (!data.ReadFloat(brightness)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read brightness failed");
        return ERR_INVALID_VALUE;
    }
    WSError errCode = UpdateBrightness(brightness);
    if (!reply.WriteInt32(static_cast<int32_t>(errCode))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write errCode failed");
        return ERR_TRANSACTION_FAILED;
    }
    return ERR_NONE;
}

int SessionStageStub::HandleAddSidebarBlur(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_PC, "in");
    AddSidebarBlur();
    return ERR_NONE;
}

int SessionStageStub::HandleSetSidebarBlurStyleWithType(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::WMS_DECOR, "called!");
    uint32_t typeValue = 0;
    if (!data.ReadUint32(typeValue) || typeValue >= static_cast<uint32_t>(SidebarBlurType::END)) {
        TLOGE(WmsLogTag::WMS_PC, "read reasonType failed");
        return ERR_INVALID_DATA;
    }
    SidebarBlurType type = static_cast<SidebarBlurType>(typeValue);
    SetSidebarBlurStyleWithType(type);
    return ERR_NONE;
}
} // namespace OHOS::Rosen
