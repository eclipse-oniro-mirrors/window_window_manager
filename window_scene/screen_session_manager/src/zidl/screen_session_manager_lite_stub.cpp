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

#include "zidl/screen_session_manager_lite_stub.h"
#include "dm_common.h"
#include <ipc_skeleton.h>

#include "marshalling_helper.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {

int32_t ScreenSessionManagerLiteStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    TLOGD(WmsLogTag::DMS, "OnRemoteRequest code is %{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TLOGE(WmsLogTag::DMS, "InterfaceToken check failed");
        return ERR_TRANSACTION_FAILED;
    }
    ScreenManagerLiteMessage msgId = static_cast<ScreenManagerLiteMessage>(code);
    switch (msgId) {
        case ScreenManagerLiteMessage::TRANS_ID_REGISTER_DISPLAY_MANAGER_AGENT: {
            HandleRegisterDisplayManagerAgent(data, reply);
            break;
        }
        case ScreenManagerLiteMessage::TRANS_ID_UNREGISTER_DISPLAY_MANAGER_AGENT: {
            HandleUnRegisterDisplayManagerAgent(data, reply);
            break;
        }
        case ScreenManagerLiteMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE: {
            HandleGetFoldDisplayMode(data, reply);
            break;
        }
        case ScreenManagerLiteMessage::TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE: {
            HandleSetFoldDisplayMode(data, reply);
            break;
        }
        case ScreenManagerLiteMessage::TRANS_ID_SCENE_BOARD_IS_FOLDABLE: {
            HandleIsFoldable(data, reply);
            break;
        }
        case ScreenManagerLiteMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_STATUS: {
            HandleGetFoldStatus(data, reply);
            break;
        }
        case ScreenManagerLiteMessage::TRANS_ID_GET_DEFAULT_DISPLAY_INFO: {
            HandleGetDefaultDisplayInfo(data, reply);
            break;
        }
        case ScreenManagerLiteMessage::TRANS_ID_GET_DISPLAY_BY_ID: {
            HandleGetDisplayById(data, reply);
            break;
        }
        case ScreenManagerLiteMessage::TRANS_ID_GET_CUTOUT_INFO: {
            HandleGetCutoutInfo(data, reply);
            break;
        }
        default:
            TLOGW(WmsLogTag::DMS, "unknown transaction code");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return 0;
}

int ScreenSessionManagerLiteStub::HandleRegisterDisplayManagerAgent(MessageParcel &data, MessageParcel &reply)
{
    TLOGD(WmsLogTag::DMS, "run HandleRegisterDisplayManagerAgent!");
    auto agent = iface_cast<IDisplayManagerAgent>(data.ReadRemoteObject());
    if (agent == nullptr) {
        return ERR_INVALID_DATA;
    }
    auto type = static_cast<DisplayManagerAgentType>(data.ReadUint32());
    DMError ret = RegisterDisplayManagerAgent(agent, type);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int ScreenSessionManagerLiteStub::HandleUnRegisterDisplayManagerAgent(MessageParcel &data, MessageParcel &reply)
{
    TLOGD(WmsLogTag::DMS, "run HandleUnRegisterDisplayManagerAgent!");
    auto agent = iface_cast<IDisplayManagerAgent>(data.ReadRemoteObject());
    if (agent == nullptr) {
        return ERR_INVALID_DATA;
    }
    auto type = static_cast<DisplayManagerAgentType>(data.ReadUint32());
    DMError ret = UnregisterDisplayManagerAgent(agent, type);
    reply.WriteInt32(static_cast<int32_t>(ret));
    return ERR_NONE;
}

int ScreenSessionManagerLiteStub::HandleGetFoldDisplayMode(MessageParcel &data, MessageParcel &reply)
{
    TLOGD(WmsLogTag::DMS, "run HandleGetFoldDisplayMode!");
    FoldDisplayMode displayMode = GetFoldDisplayMode();
    reply.WriteUint32(static_cast<uint32_t>(displayMode));
    return ERR_NONE;
}

int ScreenSessionManagerLiteStub::HandleSetFoldDisplayMode(MessageParcel &data, MessageParcel &reply)
{
    TLOGD(WmsLogTag::DMS, "run HandleSetFoldDisplayMode!");
    FoldDisplayMode displayMode = static_cast<FoldDisplayMode>(data.ReadUint32());
    SetFoldDisplayMode(displayMode);
    return ERR_NONE;
}

int ScreenSessionManagerLiteStub::HandleIsFoldable(MessageParcel &data, MessageParcel &reply)
{
    TLOGD(WmsLogTag::DMS, "run HandleIsFoldable!");
    reply.WriteBool(IsFoldable());
    return ERR_NONE;
}

int ScreenSessionManagerLiteStub::HandleGetFoldStatus(MessageParcel &data, MessageParcel &reply)
{
    TLOGD(WmsLogTag::DMS, "run HandleGetFoldStatus!");
    reply.WriteUint32(static_cast<uint32_t>(GetFoldStatus()));
    return ERR_NONE;
}

int ScreenSessionManagerLiteStub::HandleGetDefaultDisplayInfo(MessageParcel &data, MessageParcel &reply)
{
    TLOGD(WmsLogTag::DMS, "run HandleGetDefaultDisplayInfo!");
    auto info = GetDefaultDisplayInfo();
    reply.WriteParcelable(info);
    return ERR_NONE;
}

int ScreenSessionManagerLiteStub::HandleGetDisplayById(MessageParcel &data, MessageParcel &reply)
{
    TLOGD(WmsLogTag::DMS, "run HandleGetDisplayById!");
    DisplayId displayId = data.ReadUint64();
    auto info = GetDisplayInfoById(displayId);
    reply.WriteParcelable(info);
    return ERR_NONE;
}

int ScreenSessionManagerLiteStub::HandleGetCutoutInfo(MessageParcel &data, MessageParcel &reply)
{
    TLOGD(WmsLogTag::DMS, "run HandleGetCutoutInfo!");
    DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
    sptr<CutoutInfo> cutoutInfo = GetCutoutInfo(displayId);
    reply.WriteParcelable(cutoutInfo);
    return ERR_NONE;
}
} // namespace OHOS::Rosen
