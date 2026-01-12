/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "display_manager_lite_proxy.h"

#include <cinttypes>
#include <ipc_types.h>
#include <message_option.h>
#include <message_parcel.h>

#include "display_manager_interface_code.h"
#include "dm_common.h"
#include "marshalling_helper.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
DMError DisplayManagerLiteProxy::ConvertToDMError(ErrCode errCode, int32_t dmError)
{
    if (FAILED(errCode)) {
        TLOGE(WmsLogTag::DMS, "ConvertToDMError errCode: %{public}d, dmError: %{public}d", errCode, dmError);
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(dmError);
}

DMError DisplayManagerLiteProxy::RegisterDisplayManagerAgent(
    const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    TLOGD(WmsLogTag::DMS, "enter");
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (displayManagerAgent == nullptr) {
        TLOGE(WmsLogTag::DMS, "IDisplayManagerAgent is null");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (!data.WriteRemoteObject(displayManagerAgent->AsObject())) {
        TLOGE(WmsLogTag::DMS, "Write IDisplayManagerAgent failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        TLOGE(WmsLogTag::DMS, "Write DisplayManagerAgent type failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REGISTER_DISPLAY_MANAGER_AGENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
#else
    int32_t dmError;
    ErrCode errCode = RegisterDisplayManagerAgent(displayManagerAgent, static_cast<uint32_t>(type), dmError);
    return ConvertToDMError(errCode, dmError);
#endif
}

DMError DisplayManagerLiteProxy::UnregisterDisplayManagerAgent(
    const sptr<IDisplayManagerAgent>& displayManagerAgent, DisplayManagerAgentType type)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    TLOGD(WmsLogTag::DMS, "enter");
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (displayManagerAgent == nullptr) {
        TLOGE(WmsLogTag::DMS, "IDisplayManagerAgent is null");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (!data.WriteRemoteObject(displayManagerAgent->AsObject())) {
        TLOGE(WmsLogTag::DMS, "Write IWindowManagerAgent failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        TLOGE(WmsLogTag::DMS, "Write DisplayManagerAgent type failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UNREGISTER_DISPLAY_MANAGER_AGENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
#else
    int32_t dmError;
    ErrCode errCode = UnregisterDisplayManagerAgent(displayManagerAgent, static_cast<uint32_t>(type), dmError);
    return ConvertToDMError(errCode, dmError);
#endif
}

FoldDisplayMode DisplayManagerLiteProxy::GetFoldDisplayMode()
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return FoldDisplayMode::UNKNOWN;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken Failed");
        return FoldDisplayMode::UNKNOWN;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "Send TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE request failed");
        return FoldDisplayMode::UNKNOWN;
    }
    return static_cast<FoldDisplayMode>(reply.ReadUint32());
#else
    return FoldDisplayMode::UNKNOWN;
#endif
}

void DisplayManagerLiteProxy::SetFoldDisplayMode(const FoldDisplayMode displayMode)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken Failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(displayMode))) {
        TLOGE(WmsLogTag::DMS, "Write displayMode failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "Send TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE request failed");
    }
#endif
}

void DisplayManagerLiteProxy::SetFoldDisplayModeAsync(const FoldDisplayMode displayMode)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken Failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(displayMode))) {
        TLOGE(WmsLogTag::DMS, "Write displayMode failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "Send TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE request failed");
    }
#endif
}

bool DisplayManagerLiteProxy::IsFoldable()
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "IsFoldable WriteInterfaceToken failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_IS_FOLDABLE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return false;
    }
    return reply.ReadBool();
#else
    return false;
#endif
}

FoldStatus DisplayManagerLiteProxy::GetFoldStatus()
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return FoldStatus::UNKNOWN;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return FoldStatus::UNKNOWN;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return FoldStatus::UNKNOWN;
    }
    return static_cast<FoldStatus>(reply.ReadUint32());
#else
    return FoldStatus::UNKNOWN;
#endif
}

sptr<DisplayInfo> OHOS::Rosen::DisplayManagerLiteProxy::GetDefaultDisplayInfo(int32_t userId)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return nullptr;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return nullptr;
    }
    if (!data.WriteInt32(userId)) {
        TLOGE(WmsLogTag::DMS, "Write userId failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DEFAULT_DISPLAY_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return nullptr;
    }
    sptr<DisplayInfo> info = reply.ReadParcelable<DisplayInfo>();
    if (info == nullptr) {
        TLOGW(WmsLogTag::DMS, "read display info failed, info is nullptr.");
    }
    return info;
#else
    sptr<DisplayInfo> displayInfo;
    ErrCode errCode = GetDefaultDisplayInfo(displayInfo);
    if (FAILED(errCode) || displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetDefaultDisplayInfo failed, errCode: %{public}d, displayInfo: %{public}s", errCode,
            displayInfo == nullptr ? "null" : "not null");
        return nullptr;
    }
    return displayInfo;
#endif
}

sptr<DisplayInfo> DisplayManagerLiteProxy::GetDisplayInfoById(DisplayId displayId)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return nullptr;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return nullptr;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGW(WmsLogTag::DMS, "WriteUint64 displayId failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_BY_ID),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return nullptr;
    }
    sptr<DisplayInfo> info = reply.ReadParcelable<DisplayInfo>();
    if (info == nullptr) {
        TLOGW(WmsLogTag::DMS, "SendRequest nullptr.");
        return nullptr;
    }
    return info;
#else
    sptr<DisplayInfo> displayInfo;
    ErrCode errCode = GetDisplayInfoById(displayId, displayInfo);
    if (FAILED(errCode) || displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetDisplayInfo failed, displayId: %{public}" PRIu64 ", errCode: %{public}d"
            ", displayInfo: %{public}s", displayId, errCode, displayInfo == nullptr ? "null" : "not null");
        return nullptr;
    }
    return displayInfo;
#endif
}

sptr<CutoutInfo> DisplayManagerLiteProxy::GetCutoutInfo(DisplayId displayId)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return nullptr;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "failed");
        return nullptr;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS, "write displayId failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_CUTOUT_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "failed");
        return nullptr;
    }
    sptr<CutoutInfo> info = reply.ReadParcelable<CutoutInfo>();
    return info;
#else
    sptr<CutoutInfo> cutoutInfo;
    ErrCode errCode = GetCutoutInfo(displayId, cutoutInfo);
    if (FAILED(errCode) || cutoutInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetCutoutInfo failed, displayId: %{public}" PRIu64 ", errCode: %{public}d"
            ", cutoutInfo: %{public}s", displayId, errCode, cutoutInfo == nullptr ? "null" : "not null");
        return nullptr;
    }
    return cutoutInfo;
#endif
}

VirtualScreenFlag DisplayManagerLiteProxy::GetVirtualScreenFlag(ScreenId screenId)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return VirtualScreenFlag::DEFAULT;
    }
    if (screenId == SCREEN_ID_INVALID) {
        return VirtualScreenFlag::DEFAULT;
    }
    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel reply;
    MessageParcel data;
    TLOGE(WmsLogTag::DMS, "MessageParcel definded");
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "failed");
        return VirtualScreenFlag::DEFAULT;
    }
    TLOGE(WmsLogTag::DMS, "WriteInterfaceToken success");
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write failed");
        return VirtualScreenFlag::DEFAULT;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_VIRTUAL_SCREEN_FLAG),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return VirtualScreenFlag::DEFAULT;
    }
    return static_cast<VirtualScreenFlag>(reply.ReadUint32());
#else
    return VirtualScreenFlag::DEFAULT;
#endif
}

/*
 * used by powermgr
 */
bool DisplayManagerLiteProxy::WakeUpBegin(PowerStateChangeReason reason)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]WakeUpBegin remote is nullptr");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]WakeUpBegin: WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]WakeUpBegin: Write PowerStateChangeReason failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_WAKE_UP_BEGIN),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "[UL_POWER]WakeUpBegin: SendRequest failed");
        return false;
    }
    return reply.ReadBool();
#else
    bool isSucc = false;
    WakeUpBegin(static_cast<uint32_t>(reason), isSucc);
    return isSucc;
#endif
}

bool DisplayManagerLiteProxy::WakeUpEnd()
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]WakeUpEnd remote is nullptr");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]WakeUpEnd: WriteInterfaceToken failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_WAKE_UP_END),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "[UL_POWER]WakeUpEnd: SendRequest failed");
        return false;
    }
    return reply.ReadBool();
#else
    bool isSucc = false;
    WakeUpEnd(isSucc);
    return isSucc;
#endif
}

bool DisplayManagerLiteProxy::SuspendBegin(PowerStateChangeReason reason)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]SuspendBegin remote is nullptr");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]SuspendBegin: WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]SuspendBegin: Write PowerStateChangeReason failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SUSPEND_BEGIN),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "[UL_POWER]SuspendBegin: SendRequest failed");
        return false;
    }
    return reply.ReadBool();
#else
    bool isSucc = false;
    SuspendBegin(static_cast<uint32_t>(reason), isSucc);
    return isSucc;
#endif
}

bool DisplayManagerLiteProxy::SuspendEnd()
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]SuspendEnd remote is nullptr");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]SuspendEnd: WriteInterfaceToken failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SUSPEND_END),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "[UL_POWER]SuspendEnd: SendRequest failed");
        return false;
    }
    return reply.ReadBool();
#else
    bool isSucc = false;
    SuspendEnd(isSucc);
    return isSucc;
#endif
}

ScreenId DisplayManagerLiteProxy::GetInternalScreenId()
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]GetInternalScreenId remote is nullptr");
        return SCREEN_ID_INVALID;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]GetInternalScreenId: WriteInterfaceToken failed");
        return SCREEN_ID_INVALID;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_INTERNAL_SCREEN_ID),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "[UL_POWER]GetInternalScreenId: SendRequest failed");
        return SCREEN_ID_INVALID;
    }
    return reply.ReadUint64();
#else
    return SCREEN_ID_INVALID;
#endif
}

bool DisplayManagerLiteProxy::SetScreenPowerById(ScreenId screenId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]SetScreenPowerById remote is nullptr");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]Write ScreenId failed");
        return false;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(state))) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]Write ScreenPowerState failed");
        return false;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]Write PowerStateChangeReason failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_POWER_BY_ID),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "[UL_POWER]SendRequest failed");
        return false;
    }
    return reply.ReadBool();
#else
    return false;
#endif
}

bool DisplayManagerLiteProxy::SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]SetSpecifiedScreenPower remote is nullptr");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]Write ScreenId failed");
        return false;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(state))) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]Write ScreenPowerState failed");
        return false;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]Write PowerStateChangeReason failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SPECIFIED_SCREEN_POWER),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "[UL_POWER]SendRequest failed");
        return false;
    }
    return reply.ReadBool();
#else
    bool isSucc = false;
    SetSpecifiedScreenPower(screenId, static_cast<uint32_t>(state), static_cast<uint32_t>(reason), isSucc);
    return isSucc;
#endif
}

bool DisplayManagerLiteProxy::SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]SetScreenPowerForAll remote is nullptr");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(state))) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]Write ScreenPowerState failed");
        return false;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]Write PowerStateChangeReason failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_POWER_FOR_ALL),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "[UL_POWER]SendRequest failed");
        return false;
    }
    return reply.ReadBool();
#else
    bool isSucc = false;
    SetScreenPowerForAll(static_cast<uint32_t>(state), static_cast<uint32_t>(reason), isSucc);
    return isSucc;
#endif
}

ScreenPowerState DisplayManagerLiteProxy::GetScreenPower(ScreenId dmsScreenId)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return ScreenPowerState::INVALID_STATE;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return ScreenPowerState::INVALID_STATE;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(dmsScreenId))) {
        TLOGE(WmsLogTag::DMS, "Write dmsScreenId failed");
        return ScreenPowerState::INVALID_STATE;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return ScreenPowerState::INVALID_STATE;
    }
    return static_cast<ScreenPowerState>(reply.ReadUint32());
#else
    uint32_t screenPowerState;
    ErrCode errCode = GetScreenPower(dmsScreenId, screenPowerState);
    if (FAILED(errCode)) {
        TLOGE(WmsLogTag::DMS, "GetScreenPower failed, dmsScreenId: %{public}" PRIu64 ", errCode: %{public}d",
            dmsScreenId, errCode);
        return ScreenPowerState::INVALID_STATE;
    }
    return static_cast<ScreenPowerState>(screenPowerState);
#endif
}

ScreenPowerState DisplayManagerLiteProxy::GetScreenPower()
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return ScreenPowerState::INVALID_STATE;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return ScreenPowerState::INVALID_STATE;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER_AUTO),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return ScreenPowerState::INVALID_STATE;
    }
    return static_cast<ScreenPowerState>(reply.ReadUint32());
#else
    return ScreenPowerState::INVALID_STATE;
#endif
}

void DisplayManagerLiteProxy::SyncScreenPowerState(ScreenPowerState state)
{
#ifdef SCENE_BOARD_ENABLED
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
    if (!data.WriteUint32(static_cast<uint32_t>(state))) {
        TLOGE(WmsLogTag::DMS, "Write power state failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SYNC_SCREEN_POWER_STATE),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
    TLOGI(WmsLogTag::DMS, "Sync power state success");
#endif
}

bool DisplayManagerLiteProxy::SetDisplayState(DisplayState state)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]SetDisplayState remote is nullptr");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(state))) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]Write DisplayState failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_DISPLAY_STATE),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "[UL_POWER]SendRequest failed");
        return false;
    }
    return reply.ReadBool();
#else
    bool isSucc = false;
    SetDisplayState(static_cast<uint32_t>(state), isSucc);
    return isSucc;
#endif
}

DisplayState DisplayManagerLiteProxy::GetDisplayState(DisplayId displayId)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return DisplayState::UNKNOWN;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DisplayState::UNKNOWN;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS, "Write displayId failed");
        return DisplayState::UNKNOWN;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_STATE),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DisplayState::UNKNOWN;
    }
    return static_cast<DisplayState>(reply.ReadUint32());
#else
    uint32_t displayState;
    ErrCode errCode = GetDisplayState(displayId, displayState);
    if (FAILED(errCode)) {
        TLOGE(WmsLogTag::DMS, "GetDisplayState failed, displayId: %{public}" PRIu64 ", errCode: %{public}d",
            displayId, errCode);
        return DisplayState::UNKNOWN;
    }
    return static_cast<DisplayState>(displayState);
#endif
}

bool DisplayManagerLiteProxy::TryToCancelScreenOff()
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]TryToCancelScreenOff remote is nullptr");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]TryToCancelScreenOff: WriteInterfaceToken failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_TRY_TO_CANCEL_SCREEN_OFF),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "[UL_POWER]TryToCancelScreenOff: SendRequest failed");
        return false;
    }
    return reply.ReadBool();
#else
    bool isSucc = false;
    TryToCancelScreenOff(isSucc);
    return isSucc;
#endif
}

bool DisplayManagerLiteProxy::SetScreenBrightness(uint64_t screenId, uint32_t level)
{
#ifdef SCENE_BOARD_ENABLED
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
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return false;
    }
    if (!data.WriteUint64(level)) {
        TLOGE(WmsLogTag::DMS, "Write level failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_BRIGHTNESS),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return false;
    }
    return reply.ReadBool();
#else
    bool isSucc = false;
    SetScreenBrightness(screenId, level, isSucc);
    return isSucc;
#endif
}

uint32_t DisplayManagerLiteProxy::GetScreenBrightness(uint64_t screenId)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return 0;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return 0;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return 0;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_BRIGHTNESS),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return 0;
    }
    return reply.ReadUint32();
#else
    uint32_t level = 0;
    ErrCode errCode = GetScreenBrightness(screenId, level);
    if (FAILED(errCode)) {
        TLOGE(WmsLogTag::DMS, "GetScreenBrightness failed, screenId: %{public}" PRIu64 ", errCode: %{public}d",
            screenId, errCode);
        return 0;
    }
    return level;
#endif
}

std::vector<DisplayId> DisplayManagerLiteProxy::GetAllDisplayIds(int32_t userId)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]remote is nullptr");
        return {};
    }
    std::vector<DisplayId> allDisplayIds;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return allDisplayIds;
    }
    if (!data.WriteInt32(userId)) {
        TLOGE(WmsLogTag::DMS, "Write userId failed");
        return allDisplayIds;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_DISPLAYIDS),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return allDisplayIds;
    }
    reply.ReadUInt64Vector(&allDisplayIds);
    return allDisplayIds;
#else
    std::vector<DisplayId> displayIds;
    GetAllDisplayIds(displayIds);
    return displayIds;
#endif
}

DMError DisplayManagerLiteProxy::GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_SCREEN_INFOS),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    static_cast<void>(MarshallingHelper::UnmarshallingVectorParcelableObj<ScreenInfo>(reply, screenInfos));
    return ret;
#else
    int32_t dmError;
    ErrCode errCode = GetAllScreenInfos(screenInfos, dmError);
    return ConvertToDMError(errCode, dmError);
#endif
}

DMError DisplayManagerLiteProxy::GetPhysicalScreenIds(std::vector<ScreenId>& screenIds)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PHYSICAL_SCREEN_IDS),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    DMError ret = static_cast<DMError>(reply.ReadInt32());
    if (ret != DMError::DM_OK) {
        return ret;
    }

    bool res = MarshallingHelper::UnmarshallingVectorObj<ScreenId>(reply, screenIds,
        [](Parcel& parcel, ScreenId& screenId) {
            uint64_t value;
            bool res = parcel.ReadUint64(value);
            screenId = static_cast<ScreenId>(value);
            return res;
        }
    );
    if (!res) {
        TLOGW(WmsLogTag::DMS, "read physical screen ids failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return ret;
}

DMError DisplayManagerLiteProxy::SetSystemKeyboardStatus(bool isTpKeyboardOn)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteBool(isTpKeyboardOn)) {
        TLOGE(WmsLogTag::DMS, "Write isTpKeyboardOn failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SYSTEM_KEYBOARD_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
#else
    return DMError::DM_ERROR_UNKNOWN;
#endif
}

bool DisplayManagerLiteProxy::IsOnboardDisplay(DisplayId displayId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "write interface token failed");
        return false;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS, "write displayId failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_IS_ON_BOARD_DISPLAY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "send request failed");
        return false;
    }
    bool res = reply.ReadBool();
    TLOGI(WmsLogTag::DMS, "res %{public}s", res ? "true" : "false");
    return res;
}

sptr<ScreenInfo> DisplayManagerLiteProxy::GetScreenInfoById(ScreenId screenId)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return nullptr;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return nullptr;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_INFO_BY_ID),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return nullptr;
    }

    sptr<ScreenInfo> info = reply.ReadStrongParcelable<ScreenInfo>();
    if (info == nullptr) {
        TLOGW(WmsLogTag::DMS, "SendRequest nullptr.");
        return nullptr;
    }
    for (auto& mode : info->GetModes()) {
        TLOGI(WmsLogTag::DMS,
            "info modes is id: %{public}u, width: %{public}u, height: %{public}u, refreshRate: %{public}u",
            mode->id_, mode->width_, mode->height_, mode->refreshRate_);
    }
    return info;
#else
    sptr<ScreenInfo> screenInfo;
    ErrCode errCode = GetScreenInfoById(screenId, screenInfo);
    if (FAILED(errCode) || screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetScreenInfo failed, screenId: %{public}" PRIu64 ", errCode: %{public}d"
            ", screenInfo: %{public}s", screenId, errCode, screenInfo == nullptr ? "null" : "not null");
        return nullptr;
    }
    return screenInfo;
#endif
}

bool DisplayManagerLiteProxy::GetKeyboardState()
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return false;
    }

    MessageParcel reply;
    MessageParcel data;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_KEYBOARD_STATE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return false;
    }
    return reply.ReadBool();
#else
    return false;
#endif
}

bool DisplayManagerLiteProxy::SynchronizePowerStatus(ScreenPowerState state)
{
#ifdef SCENE_BOARD_ENABLED
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return false;
    }

    MessageParcel reply;
    MessageParcel data;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(state))) {
        TLOGE(WmsLogTag::DMS, "Write state failed");
        return false;
    }

    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SYNCHRONIZED_POWER_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return false;
    }

    bool res = false;
    if (!reply.ReadBool(res)) {
        TLOGE(WmsLogTag::DMS, "ReadBool failed");
        return false;
    }
    return res;
#else
    return false;
#endif
}

DMError DisplayManagerLiteProxy::SetResolution(ScreenId screenId, uint32_t width, uint32_t height,
    float virtualPixelRatio)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(screenId) || !data.WriteUint32(width) ||
        !data.WriteUint32(height) || !data.WriteFloat(virtualPixelRatio)) {
        TLOGE(WmsLogTag::DMS, "write screenId/width/height/virtualPixelRatio failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_RESOLUTION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}
} // namespace OHOS::Rosen
