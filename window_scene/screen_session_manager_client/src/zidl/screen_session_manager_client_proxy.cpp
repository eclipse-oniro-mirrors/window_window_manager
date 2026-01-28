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

#include "zidl/screen_session_manager_client_proxy.h"

#include "window_manager_hilog.h"

namespace OHOS::Rosen {

void ScreenSessionManagerClientProxy::OnScreenConnectionChanged(SessionOption SessionOption, ScreenEvent screenEvent)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!ScreenConnectWriteParam(SessionOption, screenEvent, data)) {
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_CONNECTION_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnTentModeChange(TentMode tentMode)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(tentMode))) {
        TLOGE(WmsLogTag::DMS, "Write tentMode failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_TENT_MODE_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

bool ScreenSessionManagerClientProxy::ScreenConnectWriteParam(const SessionOption& sessionOption,
    ScreenEvent screenEvent, MessageParcel& data)
{
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteUint64(sessionOption.rsId_)) {
        TLOGE(WmsLogTag::DMS, "Write rsId failed");
        return false;
    }
    if (!data.WriteString(sessionOption.name_)) {
        TLOGE(WmsLogTag::DMS, "Write name failed");
        return false;
    }
    if (!data.WriteBool(sessionOption.isExtend_)) {
        TLOGE(WmsLogTag::DMS, "Write isExtended failed");
        return false;
    }
    if (!data.WriteString(sessionOption.innerName_)) {
        TLOGE(WmsLogTag::DMS, "Write innerName failed");
        return false;
    }
    if (!data.WriteUint64(sessionOption.screenId_)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return false;
    }
    if (!data.WriteUint8(static_cast<uint8_t>(screenEvent))) {
        TLOGE(WmsLogTag::DMS, "Write screenEvent failed");
        return false;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(sessionOption.rotationCorrectionMap_.size()))) {
        TLOGE(WmsLogTag::DMS, "Write size failed");
        return false;
    }
    for (auto& iter : sessionOption.rotationCorrectionMap_) {
        if (!data.WriteUint32(static_cast<uint32_t>(iter.first))) {
            TLOGE(WmsLogTag::DMS, "Write displayMode failed");
            return false;
        }
        if (!data.WriteUint32(static_cast<uint32_t>(iter.second))) {
            TLOGE(WmsLogTag::DMS, "Write offset failed");
            return false;
        }
    }
    if (!data.WriteBool(sessionOption.supportsFocus_)) {
        TLOGE(WmsLogTag::DMS, "Write supportsFocus failed");
        return false;
    }
    if (!data.WriteBool(sessionOption.isRotationLocked_)) {
        TLOGE(WmsLogTag::DMS, "Write isRotationLocked failed");
        return false;
    }
    if (!data.WriteInt32(sessionOption.rotation_)) {
        TLOGE(WmsLogTag::DMS, "Write rotation failed");
        return false;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(sessionOption.rotationOrientationMap_.size()))) {
        TLOGE(WmsLogTag::DMS, "Write rotationOrientationMap size failed");
        return false;
    }
    for (const auto& pair : sessionOption.rotationOrientationMap_) {
        if (!data.WriteInt32(pair.first)) {
            TLOGE(WmsLogTag::DMS, "Write rotationOrientationMap key failed");
            return false;
        }
        if (!data.WriteInt32(pair.second)) {
            TLOGE(WmsLogTag::DMS, "Write rotationOrientationMap value failed");
            return false;
        }
    }
    return true;
}

void ScreenSessionManagerClientProxy::SwitchUserCallback(std::vector<int32_t> oldScbPids, int32_t currentScbPid)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteInt32Vector(oldScbPids)) {
        TLOGE(WmsLogTag::DMS, "Write oldScbPids failed");
        return;
    }
    if (!data.WriteInt32(currentScbPid)) {
        TLOGE(WmsLogTag::DMS, "Write currentScbPid failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_SWITCH_USER_CMD),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnScreenExtendChanged(ScreenId mainScreenId, ScreenId extendScreenId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(mainScreenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteUint64(extendScreenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenOrientation failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_EXTEND_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnPropertyChanged(ScreenId screenId,
    const ScreenProperty& property, ScreenPropertyChangeReason reason)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!RSMarshallingHelper::Marshalling(data, property)) {
        TLOGE(WmsLogTag::DMS, "Write property failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        TLOGE(WmsLogTag::DMS, "Write reason failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_PROPERTY_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

bool ScreenSessionManagerClientProxy::OnFoldPropertyChange(ScreenId screenId, const ScreenProperty& property,
    ScreenPropertyChangeReason reason, FoldDisplayMode displayMode, ScreenProperty& midProperty)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return false;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return false;
    }
    if (!RSMarshallingHelper::Marshalling(data, property)) {
        TLOGE(WmsLogTag::DMS, "Write property failed");
        return false;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        TLOGE(WmsLogTag::DMS, "Write reason failed");
        return false;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(displayMode))) {
        TLOGE(WmsLogTag::DMS, "Write displayMode failed");
        return false;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_FOLD_PROPERTY_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return false;
    }
    if (!RSMarshallingHelper::Unmarshalling(reply, midProperty)) {
        TLOGE(WmsLogTag::DMS, "read property failed");
        return false;
    }
    return true;
}

void ScreenSessionManagerClientProxy::OnPowerStatusChanged(DisplayPowerEvent event, EventStatus status,
    PowerStateChangeReason reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(event))) {
        TLOGE(WmsLogTag::DMS, "Write event failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(status))) {
        TLOGE(WmsLogTag::DMS, "Write status failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        TLOGE(WmsLogTag::DMS, "Write reason failed");
        return;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed, Remote is nullptr");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_POWER_STATUS_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnSensorRotationChanged(ScreenId screenId,
    float sensorRotation, bool isSwitchUser)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteFloat(sensorRotation)) {
        TLOGE(WmsLogTag::DMS, "Write sensorRotation failed");
        return;
    }
    if (!data.WriteBool(isSwitchUser)) {
        TLOGE(WmsLogTag::DMS, "Write isSwitchUser failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_SENSOR_ROTATION_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnHoverStatusChanged(ScreenId screenId, int32_t hoverStatus, bool needRotate)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteInt32(hoverStatus)) {
        TLOGE(WmsLogTag::DMS, "Write hoverStatus failed");
        return;
    }
    if (!data.WriteBool(needRotate)) {
        TLOGE(WmsLogTag::DMS, "Write needRotate failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_HOVER_STATUS_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnScreenOrientationChanged(ScreenId screenId, float screenOrientation)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteFloat(screenOrientation)) {
        TLOGE(WmsLogTag::DMS, "Write screenOrientation failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_ORIENTATION_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnScreenRotationLockedChanged(ScreenId screenId, bool isLocked)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteBool(isLocked)) {
        TLOGE(WmsLogTag::DMS, "Write isLocked failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_ROTATION_LOCKED_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnDisplayStateChanged(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(defaultDisplayId)) {
        TLOGE(WmsLogTag::DMS, "Write defaultDisplayId failed");
        return;
    }
    if (!data.WriteStrongParcelable(displayInfo)) {
        TLOGE(WmsLogTag::DMS, "Write displayInfo failed");
        return;
    }
    auto mapSize = static_cast<uint32_t>(displayInfoMap.size());
    if (!data.WriteUint32(mapSize)) {
        TLOGE(WmsLogTag::DMS, "Write mapSize failed");
        return;
    }
    for (auto [id, info] : displayInfoMap) {
        if (!data.WriteUint64(id)) {
            TLOGE(WmsLogTag::DMS, "Write id failed");
            return;
        }
        if (!data.WriteStrongParcelable(info)) {
            TLOGE(WmsLogTag::DMS, "Write info failed");
            return;
        }
    }
    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        TLOGE(WmsLogTag::DMS, "Write type failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_DISPLAY_STATE_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnGetSurfaceNodeIdsFromMissionIdsChanged(std::vector<uint64_t>& missionIds,
    std::vector<uint64_t>& surfaceNodeIds, const std::vector<uint32_t>& needWindowTypeList, bool isNeedForceCheck)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUInt64Vector(missionIds)) {
        TLOGE(WmsLogTag::DMS, "Write missionIds failed");
        return;
    }
    if (!data.WriteUInt64Vector(surfaceNodeIds)) {
        TLOGE(WmsLogTag::DMS, "Write surfaceNodeIds failed");
        return;
    }
    if (!data.WriteUInt32Vector(needWindowTypeList)) {
        TLOGE(WmsLogTag::DMS, "Write needWindowTypeList failed");
        return;
    }
    if (!data.WriteBool(isNeedForceCheck)) {
        TLOGE(WmsLogTag::DMS, "Write isNeedForceCheck failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        ScreenSessionManagerClientMessage::TRANS_ID_GET_SURFACENODEID_FROM_MISSIONID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
    reply.ReadUInt64Vector(&surfaceNodeIds);
}

void ScreenSessionManagerClientProxy::OnSetSurfaceNodeIdsChanged(DisplayId displayId,
    const std::vector<uint64_t>& surfaceNodeIds)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS, "Write displayId failed");
        return;
    }
    if (!data.WriteUInt64Vector(surfaceNodeIds)) {
        TLOGE(WmsLogTag::DMS, "Write surfaceNodeIds failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_SET_SURFACENODEIDS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnVirtualScreenDisconnected(DisplayId displayId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS, "Write displayId failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_VIRTUAL_SCREEN_DISCONNECTED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnUpdateFoldDisplayMode(FoldDisplayMode displayMode)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(displayMode))) {
        TLOGE(WmsLogTag::DMS, "Write displayMode failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_SET_FOLD_DISPLAY_MODE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnScreenshot(DisplayId displayId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS, "Write displayId failed");
        return;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed, Remote is nullptr");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_SHOT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnImmersiveStateChanged(ScreenId screenId, bool& immersive)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_IMMERSIVE_STATE_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
    immersive = reply.ReadBool();
}

void ScreenSessionManagerClientProxy::SetDisplayNodeScreenId(ScreenId screenId, ScreenId displayNodeScreenId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteUint64(displayNodeScreenId)) {
        TLOGE(WmsLogTag::DMS, "Write displayNodeScreenId failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_SET_DISPLAY_NODE_SCREEN_ID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId) || !data.WriteFloat(virtualPixelRatio)) {
        TLOGE(WmsLogTag::DMS, "Write screenId/virtualPixelRatio failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnFoldStatusChangedReportUE(const std::vector<std::string>& screenFoldInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteStringVector(screenFoldInfo)) {
        TLOGE(WmsLogTag::DMS, "Write screenFoldInfo failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        ScreenSessionManagerClientMessage::TRANS_ID_ON_FOLDSTATUS_CHANGED_REPORT_UE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::ScreenCaptureNotify(ScreenId mainScreenId, int32_t uid,
    const std::string& clientName)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(mainScreenId) || !data.WriteInt32(uid) || !data.WriteString(clientName)) {
        TLOGE(WmsLogTag::DMS, "Write screenId or uid or client failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_CAPTURE_NOTIFY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnCameraBackSelfieChanged(ScreenId screenId, bool isCameraBackSelfie)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteBool(isCameraBackSelfie)) {
        TLOGE(WmsLogTag::DMS, "Write isCameraBackSelfie failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_CAMERA_BACKSELFIE_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnSuperFoldStatusChanged(ScreenId screenId, SuperFoldStatus superFoldStatus)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(superFoldStatus))) {
        TLOGE(WmsLogTag::DMS, "Write superFoldStatus failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_SUPER_FOLD_STATUS_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnExtendScreenConnectStatusChanged(ScreenId screenId,
    ExtendScreenConnectStatus extendScreenConnectStatus)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(extendScreenConnectStatus))) {
        TLOGE(WmsLogTag::DMS, "Write extendScreenConnectStatus failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_EXTEND_SCREEN_CONNECT_STATUS_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnSecondaryReflexionChanged(ScreenId screenId, bool isSecondaryReflexion)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(isSecondaryReflexion))) {
        TLOGE(WmsLogTag::DMS, "Write secondaryReflexion failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_SECONDARY_REFLEXION_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

bool ScreenSessionManagerClientProxy::OnCreateScreenSessionOnly(ScreenId screenId, ScreenId rsId,
    const std::string& name, bool isExtend)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteUint64(screenId) || !data.WriteUint64(rsId) || !data.WriteString(name) ||
        !data.WriteBool(isExtend)) {
        TLOGE(WmsLogTag::DMS, "Write parameters failed");
        return false;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_CREATE_SCREEN_SESSION_ONLY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return false;
    }
    return reply.ReadBool();
}

bool ScreenSessionManagerClientProxy::OnExtendDisplayNodeChange(ScreenId firstId, ScreenId secondId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return false;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteUint64(firstId) || !data.WriteUint64(secondId)) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        ScreenSessionManagerClientMessage::TRANS_ID_ON_EXTEND_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return false;
    }
    return reply.ReadBool();
}

bool ScreenSessionManagerClientProxy::OnMainDisplayNodeChange(ScreenId mainScreenId, ScreenId extendScreenId,
    ScreenId extendRSId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return false;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteUint64(mainScreenId) || !data.WriteUint64(extendScreenId) || !data.WriteUint64(extendRSId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId/virtualPixelRatio failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        ScreenSessionManagerClientMessage::TRANS_ID_ON_MAIN_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
    }
    return reply.ReadBool();
}

void ScreenSessionManagerClientProxy::SetScreenCombination(ScreenId mainScreenId, ScreenId extendScreenId,
    ScreenCombination extendCombination)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(mainScreenId))) {
        TLOGE(WmsLogTag::DMS, "Write main screenId failed");
        return;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(extendScreenId))) {
        TLOGE(WmsLogTag::DMS, "Write extend screenId failed");
        return;
    }
    if (!data.WriteUint64(static_cast<uint32_t>(extendCombination))) {
        TLOGE(WmsLogTag::DMS, "Write combination failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        ScreenSessionManagerClientMessage::TRANS_ID_SET_SCREEN_COMBINATION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

std::string ScreenSessionManagerClientProxy::OnDumperClientScreenSessions()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return "";
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return "";
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        ScreenSessionManagerClientMessage::TRANS_ID_ON_DUMP_SCREEN_SESSION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return "";
    }
    return reply.ReadString();
}

void ScreenSessionManagerClientProxy::OnBeforeScreenPropertyChanged(FoldStatus foldStatus)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(foldStatus))) {
        TLOGE(WmsLogTag::DMS, "Write fold status failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_BEFORE_PROPERTY_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnScreenModeChanged(ScreenModeChangeEvent screenModeChangeEvent)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(screenModeChangeEvent))) {
        TLOGE(WmsLogTag::DMS, "Write screen mode change event failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_ON_SCREEN_MODE_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerClientProxy::OnAnimationFinish()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        ScreenSessionManagerClientMessage::TRANS_ID_ON_ANIMATE_FINISH_TIMEOUT), data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}
 
void ScreenSessionManagerClientProxy::SetInternalClipToBounds(ScreenId screenId, bool clipToBounds)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }
 
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteBool(clipToBounds)) {
        TLOGE(WmsLogTag::DMS, "Write clipToBounds failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(ScreenSessionManagerClientMessage::TRANS_ID_SET_INTERNAL_CLIPTOBOUNDS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}
} // namespace OHOS::Rosen
