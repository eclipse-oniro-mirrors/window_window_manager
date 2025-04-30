/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "zidl/display_manager_agent_proxy.h"

#include <ipc_types.h>

#include "dm_common.h"
#include "marshalling_helper.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

void DisplayManagerAgentProxy::NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(event))) {
        TLOGE(WmsLogTag::DMS_DM, "Write event failed");
        return;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(status))) {
        TLOGE(WmsLogTag::DMS_DM, "Write status failed");
        return;
    }

    if (remote->SendRequest(TRANS_ID_NOTIFY_DISPLAY_POWER_EVENT, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::NotifyDisplayStateChanged(DisplayId id, DisplayState state)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(state))) {
        TLOGE(WmsLogTag::DMS_DM, "Write DisplayState failed");
        return;
    }

    if (!data.WriteUint64(static_cast<uint64_t>(id))) {
        TLOGE(WmsLogTag::DMS_DM, "Write displayId failed");
        return;
    }

    if (remote->SendRequest(TRANS_ID_NOTIFY_DISPLAY_STATE_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::OnScreenConnect(sptr<ScreenInfo> screenInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteParcelable(screenInfo.GetRefPtr())) {
        TLOGE(WmsLogTag::DMS_DM, "Write ScreenInfo failed");
        return;
    }

    if (remote->SendRequest(TRANS_ID_ON_SCREEN_CONNECT, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::OnScreenDisconnect(ScreenId screenId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS_DM, "Write ScreenId failed");
        return;
    }

    if (remote->SendRequest(TRANS_ID_ON_SCREEN_DISCONNECT, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::OnScreenChange(const sptr<ScreenInfo>& screenInfo, ScreenChangeEvent event)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteParcelable(screenInfo.GetRefPtr())) {
        TLOGE(WmsLogTag::DMS_DM, "Write screenInfo failed");
        return;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(event))) {
        TLOGE(WmsLogTag::DMS_DM, "Write ScreenChangeEvent failed");
        return;
    }

    if (remote->SendRequest(TRANS_ID_ON_SCREEN_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::OnScreenGroupChange(const std::string& trigger,
    const std::vector<sptr<ScreenInfo>>& screenInfos, ScreenGroupChangeEvent event)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteString(trigger)) {
        TLOGE(WmsLogTag::DMS_DM, "Write trigger failed");
        return;
    }

    if (!MarshallingHelper::MarshallingVectorParcelableObj<ScreenInfo>(data, screenInfos)) {
        TLOGE(WmsLogTag::DMS_DM, "Write screenInfos failed");
        return;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(event))) {
        TLOGE(WmsLogTag::DMS_DM, "Write ScreenGroupChangeEvent failed");
        return;
    }

    if (remote->SendRequest(TRANS_ID_ON_SCREENGROUP_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::OnDisplayCreate(sptr<DisplayInfo> displayInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }
    
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteParcelable(displayInfo.GetRefPtr())) {
        TLOGE(WmsLogTag::DMS_DM, "Write DisplayInfo failed");
        return;
    }

    if (remote->SendRequest(TRANS_ID_ON_DISPLAY_CONNECT, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::OnDisplayDestroy(DisplayId displayId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS_DM, "Write DisplayId failed");
        return;
    }

    if (remote->SendRequest(TRANS_ID_ON_DISPLAY_DISCONNECT, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::OnDisplayChange(sptr<DisplayInfo> displayInfo, DisplayChangeEvent event)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteParcelable(displayInfo.GetRefPtr())) {
        TLOGE(WmsLogTag::DMS_DM, "Write DisplayInfo failed");
        return;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(event))) {
        TLOGE(WmsLogTag::DMS_DM, "Write DisplayChangeEvent failed");
        return;
    }

    if (remote->SendRequest(TRANS_ID_ON_DISPLAY_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::OnScreenshot(sptr<ScreenshotInfo> snapshotInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteParcelable(snapshotInfo.GetRefPtr())) {
        TLOGE(WmsLogTag::DMS_DM, "Write ScreenshotInfo failed");
        return;
    }
    if (remote->SendRequest(TRANS_ID_ON_SCREEN_SHOT, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::NotifyPrivateWindowStateChanged(bool hasPrivate)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(hasPrivate)) {
        TLOGE(WmsLogTag::DMS_DM, "Write private info failed");
        return;
    }
    if (remote->SendRequest(TRANS_ID_ON_PRIVATE_WINDOW, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::NotifyPrivateStateWindowListChanged(DisplayId id,
    std::vector<std::string> privacyWindowList)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(id)) {
        TLOGE(WmsLogTag::DMS_DM, "Write DisplayId failed");
        return;
    }
    if (!data.WriteStringVector(privacyWindowList)) {
        TLOGE(WmsLogTag::DMS_DM, "Write privacyWindowList failed");
        return;
    }
    if (remote->SendRequest(TRANS_ID_ON_PRIVATE_WINDOW_LIST, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::NotifyFoldStatusChanged(FoldStatus foldStatus)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(foldStatus))) {
        TLOGE(WmsLogTag::DMS_DM, "Write foldStatus failed");
        return;
    }
    if (remote->SendRequest(TRANS_ID_ON_FOLD_STATUS_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::NotifyFoldAngleChanged(std::vector<float> foldAngles)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteFloatVector(foldAngles)) {
        TLOGE(WmsLogTag::DMS_DM, "Write foldAngles failed");
        return;
    }
    if (remote->SendRequest(TRANS_ID_ON_FOLD_ANGLE_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::NotifyCaptureStatusChanged(bool isCapture)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(isCapture)) {
        TLOGE(WmsLogTag::DMS_DM, "Write isCapture failed");
        return;
    }
    if (remote->SendRequest(TRANS_ID_ON_CAPTURE_STATUS_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }
    if (!info->Marshalling(data)) {
        TLOGE(WmsLogTag::DMS_DM, "Write display change info failed");
        return;
    }
    if (remote->SendRequest(TRANS_ID_ON_DISPLAY_CHANGE_INFO_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::NotifyDisplayModeChanged(FoldDisplayMode displayMode)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(displayMode))) {
        TLOGE(WmsLogTag::DMS_DM, "Write displayMode failed");
        return;
    }
    if (remote->SendRequest(TRANS_ID_ON_DISPLAY_MODE_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::NotifyScreenMagneticStateChanged(bool isMagneticState)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(isMagneticState)) {
        TLOGE(WmsLogTag::DMS_DM, "Write isMagneticState failed");
        return;
    }
    if (remote->SendRequest(TRANS_ID_ON_SCREEN_MAGNETIC_STATE_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::NotifyAvailableAreaChanged(DMRect area, DisplayId displayId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS_DM, "Write DisplayId failed");
        return;
    }
    if (!data.WriteInt32(area.posX_) || !data.WriteInt32(area.posY_) || !data.WriteUint32(area.width_)
        ||!data.WriteUint32(area.height_)) {
        TLOGE(WmsLogTag::DMS_DM, "Write rect failed");
        return;
    }
    if (remote->SendRequest(TRANS_ID_ON_AVAILABLE_AREA_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::NotifyScreenModeChange(const std::vector<sptr<ScreenInfo>>& screenInfos)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }

    if (!MarshallingHelper::MarshallingVectorParcelableObj<ScreenInfo>(data, screenInfos)) {
        TLOGE(WmsLogTag::DMS_DM, "Write screenInfos failed");
        return;
    }

    if (remote->SendRequest(TRANS_ID_ON_SCREEN_MODE_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}

void DisplayManagerAgentProxy::NotifyAbnormalScreenConnectChange(ScreenId screenId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_DM, "remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_DM, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS_DM, "Write screenId failed");
        return;
    }
    if (remote->SendRequest(TRANS_ID_NOTIFY_ABNORMAL_SCREEN_CONNECT_CHANGED, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_DM, "SendRequest failed");
    }
}
} // namespace Rosen
} // namespace OHOS

