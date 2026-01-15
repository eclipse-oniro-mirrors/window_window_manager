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

#include "zidl/screen_session_manager_client_stub.h"

#include "window_manager_hilog.h"

namespace OHOS::Rosen {

void ScreenSessionManagerClientStub::InitScreenChangeMap()
{
    if (HandleScreenChangeMap_.size() != 0) {
        TLOGI(WmsLogTag::DMS, "screen change map has init!");
        return;
    }
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_CONNECTION_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
            return HandleOnScreenConnectionChanged(data, reply);
        };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_PROPERTY_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnPropertyChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_FOLD_PROPERTY_CHANGED] =
            [this](MessageParcel& data, MessageParcel& reply) {
                return HandleOnFoldPropertyChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_POWER_STATUS_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnPowerStatusChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_EXTEND_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnScreenExtendChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_SENSOR_ROTATION_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnSensorRotationChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_HOVER_STATUS_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnHoverStatusChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_ORIENTATION_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnScreenOrientationChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_ROTATION_LOCKED_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnScreenRotationLockedChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_DISPLAY_STATE_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnDisplayStateChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_SHOT] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnScreenshot(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_IMMERSIVE_STATE_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnImmersiveStateChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_SET_DISPLAY_NODE_SCREEN_ID] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnSetDisplayNodeScreenId(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_GET_SURFACENODEID_FROM_MISSIONID] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnGetSurfaceNodeIdsFromMissionIdsChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_SET_SURFACENODEIDS] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnSetSurfaceNodeIdsChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_VIRTUAL_SCREEN_DISCONNECTED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnVirtualScreenDisconnected(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_SET_FOLD_DISPLAY_MODE] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnUpdateFoldDisplayMode(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_SWITCH_USER_CMD] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleSwitchUserCallback(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleSetVirtualPixelRatioSystem(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_FOLDSTATUS_CHANGED_REPORT_UE] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnFoldStatusChangedReportUE(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_CAPTURE_NOTIFY] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleScreenCaptureNotify(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_SUPER_FOLD_STATUS_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnSuperFoldStatusChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_SECONDARY_REFLEXION_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnSecondaryReflexionChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_CAMERA_BACKSELFIE_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnCameraBackSelfieChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_EXTEND_SCREEN_CONNECT_STATUS_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnExtendScreenConnectStatusChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_EXTEND_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnExtendDisplayNodeChange(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_CREATE_SCREEN_SESSION_ONLY] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnCreateScreenSessionOnly(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_MAIN_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnMainDisplayNodeChange(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_SET_SCREEN_COMBINATION] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleSyncScreenCombination(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_DUMP_SCREEN_SESSION] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnDumperClientScreenSessions(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_BEFORE_PROPERTY_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnBeforeScreenPropertyChange(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_MODE_CHANGED] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnScreenModeChanged(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_ANIMATE_FINISH_TIMEOUT] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleOnAnimationFinish(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_SET_INTERNAL_CLIPTOBOUNDS] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleSetInternalClipToBounds(data, reply);
    };
    HandleScreenChangeMap_[ScreenSessionManagerClientMessage::TRANS_ID_ON_TENT_MODE_CHANGE] =
        [this](MessageParcel& data, MessageParcel& reply) {
        return HandleTentModeChange(data, reply);
    };
}

ScreenSessionManagerClientStub::ScreenSessionManagerClientStub()
{
    InitScreenChangeMap();
}

int ScreenSessionManagerClientStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    int handleRet = ERR_INVALID_STATE;
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TLOGE(WmsLogTag::DMS, "Failed to check interface token!");
        return handleRet;
    }
    ScreenSessionManagerClientMessage msgId = static_cast<ScreenSessionManagerClientMessage>(code);
    auto handleCall = HandleScreenChangeMap_.find(msgId);
    if (handleCall != HandleScreenChangeMap_.end() && handleCall->second != nullptr) {
        auto handleFunc = handleCall->second;
        handleRet = handleFunc(data, reply);
    } else {
        TLOGE(WmsLogTag::DMS, "Failed to find function handler!");
        handleRet = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return handleRet;
}

int ScreenSessionManagerClientStub::HandleSwitchUserCallback(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    std::vector<int32_t> oldScbPids;
    data.ReadInt32Vector(&oldScbPids);
    int32_t currentScbPid = data.ReadInt32();
    SwitchUserCallback(oldScbPids, currentScbPid);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnScreenConnectionChanged(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto rsId = static_cast<ScreenId>(data.ReadUint64());
    auto name = data.ReadString();
    bool isExtend = data.ReadBool();
    auto innerName = data.ReadString();
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    auto screenEvent = static_cast<ScreenEvent>(data.ReadUint8());

    std::unordered_map<FoldDisplayMode, int32_t> rotationCorrectionMap;
    if (ReadRotationCorrectionMap(data, rotationCorrectionMap) != ERR_NONE) {
        return ERR_INVALID_DATA;
    }
    bool supportsFocus = data.ReadBool();
    UniqueScreenRotationOptions rotationOptions;
    if (ReadRotationOptions(data, rotationOptions) != ERR_NONE) {
        return ERR_INVALID_DATA;
    }
    std::map<int32_t, int32_t> rotationOrientationMap;
    if (ReadRotationOrientationMap(data, rotationOrientationMap) != ERR_NONE) {
        return ERR_INVALID_DATA;
    }

    SessionOption option = {
        .rsId_ = rsId,
        .name_ = name,
        .isExtend_ = isExtend,
        .innerName_ = innerName,
        .screenId_ = screenId,
        .rotationCorrectionMap_ = rotationCorrectionMap,
        .supportsFocus_ = supportsFocus,
        .isRotationLocked_ = rotationOptions.isRotationLocked_,
        .rotation_ = rotationOptions.rotation_,
        .rotationOrientationMap_ = rotationOrientationMap
    };
    TLOGD(WmsLogTag::DMS,
        "ClientStub received callback parameters, isRotationLocked: %{public}d, rotation: %{public}d, "
        "rotationOrientationMap: %{public}s",
        option.isRotationLocked_, option.rotation_, MapToString(option.rotationOrientationMap_).c_str());
    OnScreenConnectionChanged(option, screenEvent);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::ReadRotationCorrectionMap(MessageParcel& data,
    std::unordered_map<FoldDisplayMode, int32_t>& rotationCorrectionMap)
{
    uint64_t size = 0;
    if (!data.ReadUint64(size)) {
        TLOGE(WmsLogTag::DMS, "Failed to read size");
        return ERR_INVALID_DATA;
    }
    for (uint64_t i = 0; i < size; i++) {
        uint32_t foldDisplayMode = 0;
        if (!data.ReadUint32(foldDisplayMode)) {
            TLOGE(WmsLogTag::DMS, "Failed to read foldDisplayMode");
            return ERR_INVALID_DATA;
        }
        uint32_t offset = 0;
        if (!data.ReadUint32(offset)) {
            TLOGE(WmsLogTag::DMS, "Failed to read offset");
            return ERR_INVALID_DATA;
        }
        rotationCorrectionMap.insert({static_cast<FoldDisplayMode>(foldDisplayMode), offset});
    }
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::ReadRotationOptions(MessageParcel& data,
    UniqueScreenRotationOptions& rotationOptions)
{
    if (!data.ReadBool(rotationOptions.isRotationLocked_)) {
        TLOGE(WmsLogTag::DMS, "Failed to read isRotationLocked");
        return ERR_INVALID_DATA;
    }
    if (!data.ReadInt32(rotationOptions.rotation_)) {
        TLOGE(WmsLogTag::DMS, "Failed to read rotation");
        return ERR_INVALID_DATA;
    }
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::ReadRotationOrientationMap(MessageParcel& data,
    std::map<int32_t, int32_t>& rotationOrientationMap)
{
    uint64_t mapSize = 0;
    if (!data.ReadUint64(mapSize) || mapSize > ROTATION_NUM) {
        TLOGE(WmsLogTag::DMS, "Failed to read rotationOrientationMap size");
        return ERR_INVALID_DATA;
    }
    for (uint64_t i = 0; i < mapSize; ++i) {
        int32_t key;
        int32_t value;
        if (!data.ReadInt32(key) || !data.ReadInt32(value)) {
            TLOGE(WmsLogTag::DMS, "Failed to read rotationOrientationMap");
            return ERR_INVALID_DATA;
        }
        rotationOrientationMap[key] = value;
    }
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnPropertyChanged(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    ScreenProperty property;
    if (!RSMarshallingHelper::Unmarshalling(data, property)) {
        TLOGE(WmsLogTag::DMS, "Read property failed");
        return ERR_INVALID_DATA;
    }
    auto reason = static_cast<ScreenPropertyChangeReason>(data.ReadUint32());
    OnPropertyChanged(screenId, property, reason);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnFoldPropertyChanged(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    ScreenId screenId;
    if (!data.ReadUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Read screenId failed");
        return ERR_INVALID_DATA;
    }
    ScreenProperty property;
    if (!RSMarshallingHelper::Unmarshalling(data, property)) {
        TLOGE(WmsLogTag::DMS, "Read property failed");
        return ERR_INVALID_DATA;
    }
    auto reason = static_cast<ScreenPropertyChangeReason>(data.ReadUint32());
    auto displayMode = static_cast<FoldDisplayMode>(data.ReadUint32());
    ScreenProperty midProperty;
    bool valid = OnFoldPropertyChange(screenId, property, reason, displayMode, midProperty);
    if (!valid || !RSMarshallingHelper::Marshalling(reply, midProperty)) {
        TLOGE(WmsLogTag::DMS, "Write screenProperty failed");
        return ERR_TRANSACTION_FAILED;
    }
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnPowerStatusChanged(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto event = static_cast<DisplayPowerEvent>(data.ReadUint32());
    auto status = static_cast<EventStatus>(data.ReadUint32());
    auto reason = static_cast<PowerStateChangeReason>(data.ReadUint32());
    OnPowerStatusChanged(event, status, reason);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnSensorRotationChanged(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    auto sensorRotation = data.ReadFloat();
    auto isSwitchUser = data.ReadBool();
    OnSensorRotationChanged(screenId, sensorRotation, isSwitchUser);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnScreenExtendChanged(MessageParcel& data, MessageParcel& reply)
{
    auto mainScreenId = static_cast<ScreenId>(data.ReadUint64());
    auto extendScreenId = static_cast<ScreenId>(data.ReadUint64());
    TLOGI(WmsLogTag::DMS, "mainScreenId=%{public}" PRIu64" extendScreenId=%{public}" PRIu64, mainScreenId,
        extendScreenId);
    OnScreenExtendChanged(mainScreenId, extendScreenId);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnScreenOrientationChanged(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    auto screenOrientation = data.ReadFloat();
    OnScreenOrientationChanged(screenId, screenOrientation);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnScreenRotationLockedChanged(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    auto isLocked = data.ReadBool();
    OnScreenRotationLockedChanged(screenId, isLocked);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnDisplayStateChanged(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto defaultDisplayId = static_cast<DisplayId>(data.ReadUint64());
    auto displayInfo = data.ReadStrongParcelable<DisplayInfo>();
    auto mapSize = data.ReadUint32();
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    while (mapSize--) {
        auto displayId = data.ReadUint64();
        auto displayInfoValue = data.ReadStrongParcelable<DisplayInfo>();
        displayInfoMap.emplace(displayId, displayInfoValue);
    }
    auto type = static_cast<DisplayStateChangeType>(data.ReadUint32());
    OnDisplayStateChanged(defaultDisplayId, displayInfo, displayInfoMap, type);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnUpdateFoldDisplayMode(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto foldDisplayMode = static_cast<FoldDisplayMode>(data.ReadUint32());
    OnUpdateFoldDisplayMode(foldDisplayMode);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnGetSurfaceNodeIdsFromMissionIdsChanged(MessageParcel& data,
    MessageParcel& reply)
{
    std::vector<uint64_t> missionIds;
    data.ReadUInt64Vector(&missionIds);
    std::vector<uint64_t> surfaceNodeIds;
    data.ReadUInt64Vector(&surfaceNodeIds);
    std::vector<uint32_t> needWindowTypeList;
    data.ReadUInt32Vector(&needWindowTypeList);
    bool isNeedForceCheck = data.ReadBool();
    OnGetSurfaceNodeIdsFromMissionIdsChanged(missionIds, surfaceNodeIds, needWindowTypeList, isNeedForceCheck);
    if (!reply.WriteUInt64Vector(surfaceNodeIds)) {
        TLOGE(WmsLogTag::DMS, "Write surfaceNodeIds failed");
        return ERR_TRANSACTION_FAILED;
    }
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnSetSurfaceNodeIdsChanged(MessageParcel& data, MessageParcel& reply)
{
    auto displayId = static_cast<DisplayId>(data.ReadUint64());
    std::vector<uint64_t> surfaceNodeIds;
    data.ReadUInt64Vector(&surfaceNodeIds);
    OnSetSurfaceNodeIdsChanged(displayId, surfaceNodeIds);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnVirtualScreenDisconnected(MessageParcel& data, MessageParcel& reply)
{
    auto displayId = static_cast<DisplayId>(data.ReadUint64());
    OnVirtualScreenDisconnected(displayId);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnScreenshot(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto displayId = static_cast<DisplayId>(data.ReadUint64());
    OnScreenshot(displayId);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleTentModeChange(MessageParcel& data, MessageParcel& reply)
{
    auto tentMode = static_cast<TentMode>(data.ReadUint32());
    TLOGD(WmsLogTag::DMS, "begin, tentMode = %{public}" PRIu32, static_cast<uint32_t>(tentMode));
    OnTentModeChange(tentMode);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnImmersiveStateChanged(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    bool immersive = false;
    OnImmersiveStateChanged(screenId, immersive);
    if (!reply.WriteBool(immersive)) {
        TLOGE(WmsLogTag::DMS, "Write immersive failed");
        return ERR_TRANSACTION_FAILED;
    }
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnSetDisplayNodeScreenId(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    auto displayNodeScreenId = static_cast<ScreenId>(data.ReadUint64());
    SetDisplayNodeScreenId(screenId, displayNodeScreenId);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleSetVirtualPixelRatioSystem(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    float virtualPixelRatio = data.ReadFloat();
    SetVirtualPixelRatioSystem(screenId, virtualPixelRatio);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnFoldStatusChangedReportUE(MessageParcel& data, MessageParcel& reply)
{
    std::vector<std::string> screenFoldInfo;
    data.ReadStringVector(&screenFoldInfo);
    OnFoldStatusChangedReportUE(screenFoldInfo);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnHoverStatusChanged(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    auto hoverStatus = data.ReadInt32();
    auto needRotate = data.ReadBool();
    OnHoverStatusChanged(screenId, hoverStatus, needRotate);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleScreenCaptureNotify(MessageParcel& data, MessageParcel& reply)
{
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    auto uid = data.ReadInt32();
    auto clientName = data.ReadString();
    TLOGI(WmsLogTag::DMS, "notify scb capture screenId=%{public}" PRIu64", uid=%{public}d.", screenId, uid);
    ScreenCaptureNotify(screenId, uid, clientName);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnCameraBackSelfieChanged(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    bool isCameraBackSelfie = data.ReadBool();
    OnCameraBackSelfieChanged(screenId, isCameraBackSelfie);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnSuperFoldStatusChanged(MessageParcel& data, MessageParcel& reply)
{
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    auto superFoldStatus = static_cast<SuperFoldStatus>(data.ReadUint32());
    TLOGI(WmsLogTag::DMS, "super fold status screenId=%{public}" PRIu64", superFoldStatus=%{public}d.",
        screenId, static_cast<uint32_t>(superFoldStatus));
    OnSuperFoldStatusChanged(screenId, superFoldStatus);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnSecondaryReflexionChanged(MessageParcel& data, MessageParcel& reply)
{
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    auto isSecondaryReflexion = static_cast<bool>(data.ReadUint32());
    TLOGI(WmsLogTag::DMS, "secondary reflexion screenId=%{public}" PRIu64", isSecondaryReflexion=%{public}d.",
        screenId, static_cast<uint32_t>(isSecondaryReflexion));
    OnSecondaryReflexionChanged(screenId, isSecondaryReflexion);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnExtendScreenConnectStatusChanged(MessageParcel& data, MessageParcel& reply)
{
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    auto extendScreenConnectStatus = static_cast<ExtendScreenConnectStatus>(data.ReadUint32());
    TLOGI(WmsLogTag::DMS, "extendScreenConnectStatus screenId=%{public}" PRIu64", extendScreenConnectStatus"
        "=%{public}d.", screenId, static_cast<uint32_t>(extendScreenConnectStatus));
    OnExtendScreenConnectStatusChanged(screenId, extendScreenConnectStatus);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnExtendDisplayNodeChange(MessageParcel& data, MessageParcel& reply)
{
    ScreenId mainScreenId = static_cast<ScreenId>(data.ReadUint64());
    ScreenId extendScreenId = static_cast<ScreenId>(data.ReadUint64());
    bool changeStatus = OnExtendDisplayNodeChange(mainScreenId, extendScreenId);
    reply.WriteBool(changeStatus);
    TLOGI(WmsLogTag::DMS, "extend display change status=%{public}d", changeStatus);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnCreateScreenSessionOnly(MessageParcel& data, MessageParcel& reply)
{
    auto screenId = static_cast<ScreenId>(data.ReadUint64());
    auto rsId = static_cast<ScreenId>(data.ReadUint64());
    auto name = data.ReadString();
    bool isExtend = data.ReadBool();
    bool refreshStatus = OnCreateScreenSessionOnly(screenId, rsId, name, isExtend);
    reply.WriteBool(refreshStatus);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnMainDisplayNodeChange(MessageParcel& data, MessageParcel& reply)
{
    ScreenId mainScreenId = static_cast<ScreenId>(data.ReadUint64());
    ScreenId extendScreenId = static_cast<ScreenId>(data.ReadUint64());
    ScreenId extendRSId = static_cast<ScreenId>(data.ReadUint64());
    bool changeStatus = OnMainDisplayNodeChange(mainScreenId, extendScreenId, extendRSId);
    reply.WriteBool(changeStatus);
    TLOGI(WmsLogTag::DMS, "main node change status=%{public}d", changeStatus);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnDumperClientScreenSessions(MessageParcel& data, MessageParcel& reply)
{
    std::string screenInfos = OnDumperClientScreenSessions();
    reply.WriteString(screenInfos);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleSyncScreenCombination(MessageParcel& data, MessageParcel& reply)
{
    auto mainScreenId = static_cast<ScreenId>(data.ReadUint64());
    auto extendScreenId = static_cast<ScreenId>(data.ReadUint64());
    auto extendCombination = static_cast<ScreenCombination>(data.ReadUint32());
    SetScreenCombination(mainScreenId, extendScreenId, extendCombination);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnBeforeScreenPropertyChange(MessageParcel& data, MessageParcel& reply)
{
    uint32_t status = data.ReadUint32();
    FoldStatus foldStatus = FoldStatus::UNKNOWN;
    if (status > static_cast<uint32_t>(FoldStatus::HALF_FOLD)) {
        return ERR_INVALID_VALUE;
    }
    foldStatus = static_cast<FoldStatus>(status);
    TLOGI(WmsLogTag::DMS, "fold status %{public}d", foldStatus);
    OnBeforeScreenPropertyChanged(foldStatus);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnScreenModeChanged(MessageParcel& data, MessageParcel& reply)
{
    auto screenModeChangeEvent = static_cast<ScreenModeChangeEvent>(data.ReadUint32());
    TLOGI(WmsLogTag::DMS, "screenModeChangeEvent: %{public}d", screenModeChangeEvent);
    OnScreenModeChanged(screenModeChangeEvent);
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleOnAnimationFinish(MessageParcel& data, MessageParcel& reply)
{
    OnAnimationFinish();
    return ERR_NONE;
}

int ScreenSessionManagerClientStub::HandleSetInternalClipToBounds(MessageParcel& data, MessageParcel& reply)
{
    TLOGD(WmsLogTag::DMS, "enter");
    auto mainScreenId = static_cast<ScreenId>(data.ReadUint64());
    auto clipToBounds = data.ReadBool();
    SetInternalClipToBounds(mainScreenId, clipToBounds);
    return ERR_NONE;
}
} // namespace OHOS::Rosen
