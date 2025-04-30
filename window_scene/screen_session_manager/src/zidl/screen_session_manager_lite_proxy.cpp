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

#include "zidl/screen_session_manager_lite_proxy.h"

#include <ipc_types.h>
#include <message_option.h>
#include <message_parcel.h>

#include "dm_common.h"
#include "marshalling_helper.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {

DMError ScreenSessionManagerLiteProxy::RegisterDisplayManagerAgent(
    const sptr<IDisplayManagerAgent>& displayManagerAgent, DisplayManagerAgentType type)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_SSM, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    TLOGD(WmsLogTag::DMS_SSM, "ScreenSessionManagerLiteProxy::RegisterDisplayManagerAgent");
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_SSM, "RegisterDisplayManagerAgent WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }

    if (displayManagerAgent == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "IDisplayManagerAgent is null");
        return DMError::DM_ERROR_INVALID_PARAM;
    }

    if (!data.WriteRemoteObject(displayManagerAgent->AsObject())) {
        TLOGE(WmsLogTag::DMS_SSM, "Write IDisplayManagerAgent failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        TLOGE(WmsLogTag::DMS_SSM, "Write DisplayManagerAgent type failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(ScreenManagerLiteMessage::TRANS_ID_REGISTER_DISPLAY_MANAGER_AGENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_SSM, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerLiteProxy::UnregisterDisplayManagerAgent(
    const sptr<IDisplayManagerAgent>& displayManagerAgent, DisplayManagerAgentType type)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_SSM, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    TLOGD(WmsLogTag::DMS_SSM, "ScreenSessionManagerLiteProxy::UnregisterDisplayManagerAgent");
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_SSM, "UnregisterDisplayManagerAgent WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }

    if (displayManagerAgent == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "IDisplayManagerAgent is null");
        return DMError::DM_ERROR_INVALID_PARAM;
    }

    if (!data.WriteRemoteObject(displayManagerAgent->AsObject())) {
        TLOGE(WmsLogTag::DMS_SSM, "Write IWindowManagerAgent failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        TLOGE(WmsLogTag::DMS_SSM, "Write DisplayManagerAgent type failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(ScreenManagerLiteMessage::TRANS_ID_UNREGISTER_DISPLAY_MANAGER_AGENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_SSM, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

FoldDisplayMode ScreenSessionManagerLiteProxy::GetFoldDisplayMode()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_SSM, "remote is null");
        return FoldDisplayMode::UNKNOWN;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_SSM, "WriteInterfaceToken Failed");
        return FoldDisplayMode::UNKNOWN;
    }
    if (remote->SendRequest(static_cast<uint32_t>(ScreenManagerLiteMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_SSM, "Send TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE request failed");
        return FoldDisplayMode::UNKNOWN;
    }
    return static_cast<FoldDisplayMode>(reply.ReadUint32());
}

void ScreenSessionManagerLiteProxy::SetFoldDisplayMode(const FoldDisplayMode displayMode)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_SSM, "remote is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_SSM, "WriteInterfaceToken Failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(displayMode))) {
        TLOGE(WmsLogTag::DMS_SSM, "Write displayMode failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(ScreenManagerLiteMessage::TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_SSM, "Send TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE request failed");
    }
}

bool ScreenSessionManagerLiteProxy::IsFoldable()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_SSM, "remote is null");
        return false;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_SSM, "IsFoldable WriteInterfaceToken failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(ScreenManagerLiteMessage::TRANS_ID_SCENE_BOARD_IS_FOLDABLE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_SSM, "SendRequest failed");
        return false;
    }
    return reply.ReadBool();
}

FoldStatus ScreenSessionManagerLiteProxy::GetFoldStatus()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_SSM, "remote is null");
        return FoldStatus::UNKNOWN;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_SSM, "WriteInterfaceToken failed");
        return FoldStatus::UNKNOWN;
    }
    if (remote->SendRequest(static_cast<uint32_t>(ScreenManagerLiteMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_SSM, "SendRequest failed");
        return FoldStatus::UNKNOWN;
    }
    return static_cast<FoldStatus>(reply.ReadUint32());
}

sptr<DisplayInfo> OHOS::Rosen::ScreenSessionManagerLiteProxy::GetDefaultDisplayInfo()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_SSM, "remote is null");
        return nullptr;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_SSM, "WriteInterfaceToken failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(ScreenManagerLiteMessage::TRANS_ID_GET_DEFAULT_DISPLAY_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS_SSM, "SendRequest failed");
        return nullptr;
    }

    sptr<DisplayInfo> info = reply.ReadParcelable<DisplayInfo>();
    if (info == nullptr) {
        TLOGW(WmsLogTag::DMS_SSM, "read display info failed, info is nullptr.");
    }
    return info;
}

sptr<DisplayInfo> ScreenSessionManagerLiteProxy::GetDisplayInfoById(DisplayId displayId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_SSM, "GetDisplayInfoById: remote is nullptr");
        return nullptr;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_SSM, "GetDisplayInfoById: WriteInterfaceToken failed");
        return nullptr;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGW(WmsLogTag::DMS_SSM, "GetDisplayInfoById: WriteUint64 displayId failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(ScreenManagerLiteMessage::TRANS_ID_GET_DISPLAY_BY_ID),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS_SSM, "GetDisplayInfoById: SendRequest failed");
        return nullptr;
    }

    sptr<DisplayInfo> info = reply.ReadParcelable<DisplayInfo>();
    if (info == nullptr) {
        TLOGW(WmsLogTag::DMS_SSM, "DisplayManagerProxy::GetDisplayInfoById SendRequest nullptr.");
        return nullptr;
    }
    return info;
}

sptr<CutoutInfo> ScreenSessionManagerLiteProxy::GetCutoutInfo(DisplayId displayId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS_SSM, "get cutout info : remote is null");
        return nullptr;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS_SSM, "get cutout info : failed");
        return nullptr;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS_SSM, "get cutout info: write displayId failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(ScreenManagerLiteMessage::TRANS_ID_GET_CUTOUT_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS_SSM, "GetCutoutInfo: GetCutoutInfo failed");
        return nullptr;
    }
    sptr<CutoutInfo> info = reply.ReadParcelable<CutoutInfo>();
    return info;
}
} // namespace OHOS::Rosen
