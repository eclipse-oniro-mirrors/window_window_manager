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

#include "zidl/screen_session_manager_proxy.h"

#include "marshalling_helper.h"

namespace OHOS::Rosen {
namespace {
constexpr uint32_t  MAX_CREASE_REGION_SIZE = 20;
}

sptr<DisplayInfo> OHOS::Rosen::ScreenSessionManagerProxy::GetDefaultDisplayInfo(int32_t userId)
{
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
}

DMError ScreenSessionManagerProxy::SetScreenActiveMode(ScreenId screenId, uint32_t modeId)
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
    if (!data.WriteUint64(screenId) || !data.WriteUint32(modeId)) {
        TLOGE(WmsLogTag::DMS, "write screenId/modeId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ACTIVE_MODE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio)
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
    if (!data.WriteUint64(screenId) || !data.WriteFloat(virtualPixelRatio)) {
        TLOGE(WmsLogTag::DMS, "write screenId/modeId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio)
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
    if (!data.WriteUint64(screenId) || !data.WriteFloat(virtualPixelRatio)) {
        TLOGE(WmsLogTag::DMS, "write screenId/modeId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::SetDefaultDensityDpi(ScreenId screenId, float virtualPixelRatio)
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
    if (!data.WriteUint64(screenId) || !data.WriteFloat(virtualPixelRatio)) {
        TLOGE(WmsLogTag::DMS, "write screenId/virtualPixelRatio failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_DEFAULT_DENSITY_DPI),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::SetResolution(ScreenId screenId, uint32_t width, uint32_t height,
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

DMError ScreenSessionManagerProxy::GetDensityInCurResolution(ScreenId screenId, float& virtualPixelRatio)
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
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "write screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DENSITY_IN_CURRENT_RESOLUTION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    virtualPixelRatio = reply.ReadFloat();
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGW(WmsLogTag::DMS, "WriteUint64 uint64_t failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_GAMUT),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    if (ret != DMError::DM_OK) {
        return ret;
    }
    colorGamut = static_cast<ScreenColorGamut>(reply.ReadUint32());
    return ret;
}

DMError ScreenSessionManagerProxy::SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId)) || !data.WriteInt32(colorGamutIdx)) {
        TLOGW(WmsLogTag::DMS, "Write failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_GAMUT),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGW(WmsLogTag::DMS, "WriteUint64 screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_GAMUT_MAP),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    if (ret != DMError::DM_OK) {
        return ret;
    }
    gamutMap = static_cast<ScreenGamutMap>(reply.ReadUint32());
    return ret;
}

DMError ScreenSessionManagerProxy::SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId)) || !data.WriteUint32(static_cast<uint32_t>(gamutMap))) {
        TLOGW(WmsLogTag::DMS, "Write failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_GAMUT_MAP),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::SetScreenColorTransform(ScreenId screenId)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGW(WmsLogTag::DMS, "WriteUint64 screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_TRANSFORM),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::GetPixelFormat(ScreenId screenId, GraphicPixelFormat& pixelFormat)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGW(WmsLogTag::DMS, "WriteUint64 uint64_t failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_PIXEL_FORMAT),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    if (ret != DMError::DM_OK) {
        return ret;
    }
    pixelFormat = static_cast<GraphicPixelFormat>(reply.ReadUint32());
    return ret;
}

DMError ScreenSessionManagerProxy::SetPixelFormat(ScreenId screenId, GraphicPixelFormat pixelFormat)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId)) || !data.WriteInt32(pixelFormat)) {
        TLOGW(WmsLogTag::DMS, "WriteUint64 screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_PIXEL_FORMAT),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::GetSupportedHDRFormats(ScreenId screenId, std::vector<ScreenHDRFormat>& hdrFormats)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGW(WmsLogTag::DMS, "WriteUint64 screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_HDR_FORMAT),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    if (ret != DMError::DM_OK) {
        return ret;
    }
    MarshallingHelper::UnmarshallingVectorObj<ScreenHDRFormat>(reply, hdrFormats,
        [](Parcel& parcel, ScreenHDRFormat& hdrFormat) {
            uint32_t value;
            bool res = parcel.ReadUint32(value);
            hdrFormat = static_cast<ScreenHDRFormat>(value);
            return res;
        }
    );
    return ret;
}

DMError ScreenSessionManagerProxy::GetScreenHDRFormat(ScreenId screenId, ScreenHDRFormat& hdrFormat)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGW(WmsLogTag::DMS, "WriteUint64 uint64_t failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_HDR_FORMAT),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    if (ret != DMError::DM_OK) {
        return ret;
    }
    hdrFormat = static_cast<ScreenHDRFormat>(reply.ReadUint32());
    return ret;
}

DMError ScreenSessionManagerProxy::SetScreenHDRFormat(ScreenId screenId, int32_t modeIdx)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId)) || !data.WriteInt32(modeIdx)) {
        TLOGW(WmsLogTag::DMS, "WriteUint64 screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_HDR_FORMAT),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::GetSupportedColorSpaces(ScreenId screenId,
    std::vector<GraphicCM_ColorSpaceType>& colorSpaces)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGW(WmsLogTag::DMS, "WriteUint64 screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_SPACE),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    if (ret != DMError::DM_OK) {
        return ret;
    }
    MarshallingHelper::UnmarshallingVectorObj<GraphicCM_ColorSpaceType>(reply, colorSpaces,
        [](Parcel& parcel, GraphicCM_ColorSpaceType& color) {
            uint32_t value;
            bool res = parcel.ReadUint32(value);
            color = static_cast<GraphicCM_ColorSpaceType>(value);
            return res;
        }
    );
    return ret;
}

DMError ScreenSessionManagerProxy::GetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType& colorSpace)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGW(WmsLogTag::DMS, "WriteUint64 screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_SPACE),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    if (ret != DMError::DM_OK) {
        return ret;
    }
    colorSpace = static_cast<GraphicCM_ColorSpaceType>(reply.ReadUint32());
    return ret;
}

DMError ScreenSessionManagerProxy::SetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType colorSpace)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId)) || !data.WriteInt32(colorSpace)) {
        TLOGW(WmsLogTag::DMS, "Write failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_SPACE),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::RegisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
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
}

DMError ScreenSessionManagerProxy::RegisterDisplayAttributeAgent(std::vector<std::string>& attributes,
    const sptr<IDisplayManagerAgent>& displayManagerAgent)
{
    TLOGI(WmsLogTag::DMS, "called");
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
 
    if (displayManagerAgent == nullptr) {
        TLOGE(WmsLogTag::DMS, "IDisplayManagerAgent is null");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
 
    if (!data.WriteRemoteObject(displayManagerAgent->AsObject())) {
        TLOGE(WmsLogTag::DMS, "Write IDisplayManagerAgent failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteStringVector(attributes)) {
        TLOGE(WmsLogTag::DMS, "Write attributes failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
 
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REGISTER_DISPLAY_ATTRIBUTE_AGENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::UnRegisterDisplayAttribute(const std::vector<std::string>& attributes,
    const sptr<IDisplayManagerAgent>& displayManagerAgent)
{
    TLOGI(WmsLogTag::DMS, "called");
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
 
    if (displayManagerAgent == nullptr) {
        TLOGE(WmsLogTag::DMS, "IDisplayManagerAgent is null");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
 
    if (!data.WriteRemoteObject(displayManagerAgent->AsObject())) {
        TLOGE(WmsLogTag::DMS, "Write IDisplayManagerAgent failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteStringVector(attributes)) {
        TLOGE(WmsLogTag::DMS, "Write attributes failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
 
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UNREGISTER_DISPLAY_ATTRIBUTE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::UnregisterDisplayManagerAgent(const sptr<IDisplayManagerAgent>& displayManagerAgent,
    DisplayManagerAgentType type)
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
}

bool OHOS::Rosen::ScreenSessionManagerProxy::WakeUpBegin(PowerStateChangeReason reason)
{
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
}

bool OHOS::Rosen::ScreenSessionManagerProxy::WakeUpEnd()
{
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
}

bool OHOS::Rosen::ScreenSessionManagerProxy::SuspendBegin(PowerStateChangeReason reason)
{
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
}

bool OHOS::Rosen::ScreenSessionManagerProxy::SuspendEnd()
{
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
}

ScreenId OHOS::Rosen::ScreenSessionManagerProxy::GetInternalScreenId()
{
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
}

bool OHOS::Rosen::ScreenSessionManagerProxy::SetScreenPowerById(ScreenId screenId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
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
}

bool OHOS::Rosen::ScreenSessionManagerProxy::SetDisplayState(DisplayState state)
{
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
}

bool OHOS::Rosen::ScreenSessionManagerProxy::SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state, PowerStateChangeReason reason)
{
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
}

bool OHOS::Rosen::ScreenSessionManagerProxy::SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason)
{
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
}

DisplayState OHOS::Rosen::ScreenSessionManagerProxy::GetDisplayState(DisplayId displayId)
{
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
}

void OHOS::Rosen::ScreenSessionManagerProxy::NotifyDisplayEvent(DisplayEvent event)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]NotifyDisplayEvent remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(event))) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]Write DisplayEvent failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_EVENT),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "[UL_POWER]SendRequest failed");
        return;
    }
}

ScreenPowerState OHOS::Rosen::ScreenSessionManagerProxy::GetScreenPower(ScreenId dmsScreenId)
{
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
}

ScreenPowerState OHOS::Rosen::ScreenSessionManagerProxy::GetScreenPower()
{
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
}

bool OHOS::Rosen::ScreenSessionManagerProxy::TryToCancelScreenOff()
{
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
}

ScreenId ScreenSessionManagerProxy::CreateVirtualScreen(VirtualScreenOption virtualOption,
                                                        const sptr<IRemoteObject>& displayManagerAgent)
{
    TLOGI(WmsLogTag::DMS, "SCB: ENTER");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: remote is nullptr");
        return SCREEN_ID_INVALID;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "SCB: WriteInterfaceToken failed");
        return SCREEN_ID_INVALID;
    }
    bool res = data.WriteString(virtualOption.name_) && data.WriteUint32(virtualOption.width_) &&
        data.WriteUint32(virtualOption.height_) && data.WriteFloat(virtualOption.density_) &&
        data.WriteInt32(virtualOption.flags_) && data.WriteBool(virtualOption.isForShot_) &&
        data.WriteUInt64Vector(virtualOption.missionIds_) &&
        data.WriteUint32(static_cast<uint32_t>(virtualOption.virtualScreenType_)) &&
        data.WriteBool(virtualOption.isSecurity_) &&
        data.WriteUint32(static_cast<uint32_t>(virtualOption.virtualScreenFlag_)) &&
        data.WriteBool(virtualOption.supportsFocus_) &&
        data.WriteBool(virtualOption.supportsInput_) &&
        data.WriteString(virtualOption.bundleName_) &&
        data.WriteString(virtualOption.serialNumber_);
    if (virtualOption.surface_ != nullptr && virtualOption.surface_->GetProducer() != nullptr) {
        res = res &&
            data.WriteBool(true) &&
            data.WriteRemoteObject(virtualOption.surface_->GetProducer()->AsObject());
    } else {
        TLOGW(WmsLogTag::DMS, "SCB: surface is nullptr");
        res = res && data.WriteBool(false);
    }
    if (displayManagerAgent != nullptr) {
        res = res &&
            data.WriteRemoteObject(displayManagerAgent);
    }
    if (!res) {
        TLOGE(WmsLogTag::DMS, "SCB: ScreenSessionManagerProxy::Write data failed");
        return SCREEN_ID_INVALID;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_CREATE_VIRTUAL_SCREEN),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SCB: SendRequest failed");
        return SCREEN_ID_INVALID;
    }

    ScreenId screenId = static_cast<ScreenId>(reply.ReadUint64());
    return screenId;
}

DMError ScreenSessionManagerProxy::SetVirtualScreenSurface(ScreenId screenId, sptr<IBufferProducer> surface)
{
    TLOGW(WmsLogTag::DMS, "SCB: ENTER");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: remote is nullptr");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "SCB: WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    bool res = data.WriteUint64(static_cast<uint64_t>(screenId));
    if (surface != nullptr) {
        res = res &&
            data.WriteBool(true) &&
            data.WriteRemoteObject(surface->AsObject());
    } else {
        TLOGW(WmsLogTag::DMS, "SCB: surface is nullptr");
        res = res && data.WriteBool(false);
    }
    if (!res) {
        TLOGW(WmsLogTag::DMS, "SCB: Write screenId/surface failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SURFACE),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SCB: SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::AddVirtualScreenBlockList(const std::vector<int32_t>& persistentIds)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "write token failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(persistentIds.size())) {
        TLOGE(WmsLogTag::DMS, "write size failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    for (const auto persistentId : persistentIds) {
        if (!data.WriteInt32(persistentId)) {
            TLOGE(WmsLogTag::DMS, "write persistentId failed");
            return DMError::DM_ERROR_IPC_FAILED;
        }
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_ADD_VIRTUAL_SCREEN_BLOCK_LIST),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::RemoveVirtualScreenBlockList(const std::vector<int32_t>& persistentIds)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "write token failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(persistentIds.size())) {
        TLOGE(WmsLogTag::DMS, "write size failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    for (const auto persistentId : persistentIds) {
        if (!data.WriteInt32(persistentId)) {
            TLOGE(WmsLogTag::DMS, "write persistentId failed");
            return DMError::DM_ERROR_IPC_FAILED;
        }
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_BLOCK_LIST),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::AddVirtualScreenWhiteList(ScreenId screenId,
    const std::vector<uint64_t>& missionIds)
{
    return SendVirtualScreenWhiteListRequest(screenId, missionIds,
        DisplayManagerMessage::TRANS_ID_ADD_VIRTUAL_SCREEN_WHITE_LIST);
}

DMError ScreenSessionManagerProxy::RemoveVirtualScreenWhiteList(ScreenId screenId,
    const std::vector<uint64_t>& missionIds)
{
    return SendVirtualScreenWhiteListRequest(screenId, missionIds,
        DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_WHITE_LIST);
}

DMError ScreenSessionManagerProxy::SendVirtualScreenWhiteListRequest(ScreenId screenId,
    const std::vector<uint64_t>& missionIds, DisplayManagerMessage transId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterface Token failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    bool resWriteScreenId = data.WriteUint64(static_cast<uint64_t>(screenId));
    if (!resWriteScreenId) {
        TLOGE(WmsLogTag::DMS, "write screenId:%{public}" PRIu64 " failed", screenId);
        return DMError::DM_ERROR_IPC_FAILED;
    }
    bool resWriteMissionIds = data.WriteUInt64Vector(missionIds);
    if (!resWriteMissionIds) {
        TLOGE(WmsLogTag::DMS, "write missionIds failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    int32_t errCode = remote->SendRequest(static_cast<uint32_t>(transId), data, reply, option);
    if (errCode != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed, transId:%{public}d", static_cast<uint32_t>(transId));
        return DMError::DM_ERROR_IPC_FAILED;
    }
    int32_t rawErrorCode = 0;
    if (!reply.ReadInt32(rawErrorCode)) {
        TLOGE(WmsLogTag::DMS, "read result failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(rawErrorCode);
}

bool ScreenSessionManagerProxy::IsOnboardDisplay(DisplayId displayId)
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

DMError ScreenSessionManagerProxy::SetScreenPrivacyMaskImage(ScreenId screenId,
    const std::shared_ptr<Media::PixelMap>& privacyMaskImg)
{
    TLOGW(WmsLogTag::DMS, "SCB: ENTER");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: remote is nullptr");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "SCB: WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (privacyMaskImg != nullptr) {
        if (!data.WriteBool(true) || !data.WriteParcelable(privacyMaskImg.get())) {
            TLOGW(WmsLogTag::DMS, "SCB: Write privacyMaskImg failed");
            return DMError::DM_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            TLOGW(WmsLogTag::DMS, "SCB: failed");
            return DMError::DM_ERROR_IPC_FAILED;
        }
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGW(WmsLogTag::DMS, "SCB: Write screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_MASKIMAGE),
        data, reply, option) != ERR_NONE) {
    TLOGW(WmsLogTag::DMS, "SCB: SendRequest failed");
    return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::SetVirtualMirrorScreenCanvasRotation(ScreenId screenId, bool canvasRotation)
{
    TLOGW(WmsLogTag::DMS, "SCB: ENTER");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: remote is nullptr");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "SCB: WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    bool res = data.WriteUint64(static_cast<uint64_t>(screenId)) && data.WriteBool(canvasRotation);
    if (!res) {
        TLOGW(WmsLogTag::DMS, "SCB:Write screenId/bufferRotation failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_CANVAS_ROTATION),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SCB:SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::SetVirtualMirrorScreenScaleMode(ScreenId screenId, ScreenScaleMode scaleMode)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    if (screenId == SCREEN_ID_INVALID) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    bool res = data.WriteUint64(static_cast<uint64_t>(screenId)) && data.WriteUint32(static_cast<uint32_t>(scaleMode));
    if (!res) {
        TLOGE(WmsLogTag::DMS, "Write screenId/scaleMode failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SCALE_MODE),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::ResizeVirtualScreen(ScreenId screenId, uint32_t width, uint32_t height)
{
    TLOGI(WmsLogTag::DMS, "ENTER");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "WriteUnit64 screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(width)) {
        TLOGE(WmsLogTag::DMS, "WriteUnit32 width failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(height)) {
        TLOGE(WmsLogTag::DMS, "WriteUnit32 height failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_RESIZE_VIRTUAL_SCREEN),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_NULLPTR;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::DestroyVirtualScreen(ScreenId screenId)
{
    TLOGW(WmsLogTag::DMS, "SCB: ENTER");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: remote is nullptr");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "SCB: WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGW(WmsLogTag::DMS, "SCB: WriteUint64 screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_DESTROY_VIRTUAL_SCREEN),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SCB: SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
    ScreenId& screenGroupId, const RotationOption& rotationOption, bool forceMirror)
{
    TLOGW(WmsLogTag::DMS, "SCB: ENTER");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "SCB: WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    bool res = data.WriteUint64(static_cast<uint64_t>(mainScreenId)) &&
        data.WriteUInt64Vector(mirrorScreenIds) && data.WriteUint32(static_cast<uint32_t>(rotationOption.rotation_)) &&
        data.WriteBool(rotationOption.needSetRotation_);
    if (!res) {
        TLOGE(WmsLogTag::DMS, "SCB: data write failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SCB: SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    screenGroupId = static_cast<ScreenId>(reply.ReadUint64());
    return ret;
}

DMError ScreenSessionManagerProxy::MakeMirrorForRecord(const std::vector<ScreenId>& mainScreenIds,
    std::vector<ScreenId>& mirrorScreenIds, ScreenId& screenGroupId)
{
    TLOGW(WmsLogTag::DMS, "SCB: ENTER");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "SCB: WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    bool res = data.WriteUInt64Vector(mainScreenIds) &&
        data.WriteUInt64Vector(mirrorScreenIds);
    if (!res) {
        TLOGE(WmsLogTag::DMS, "SCB: data write failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR_FOR_RECORD),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SCB: SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    screenGroupId = static_cast<ScreenId>(reply.ReadUint64());
    return ret;
}

DMError ScreenSessionManagerProxy::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
                                              DMRect mainScreenRegion, ScreenId& screenGroupId)
{
    TLOGW(WmsLogTag::DMS, "ENTER");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    bool res = data.WriteUint64(static_cast<uint64_t>(mainScreenId)) &&
        data.WriteUInt64Vector(mirrorScreenIds);
    if (!res) {
        TLOGE(WmsLogTag::DMS, "write screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(mainScreenRegion.posX_) || !data.WriteInt32(mainScreenRegion.posY_) ||
        !data.WriteUint32(mainScreenRegion.width_) || !data.WriteUint32(mainScreenRegion.height_)) {
        TLOGE(WmsLogTag::DMS, "Write mainScreenRegion failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR_WITH_REGION),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    screenGroupId = static_cast<ScreenId>(reply.ReadUint64());
    return ret;
}

DMError ScreenSessionManagerProxy::SetMultiScreenMode(ScreenId mainScreenId, ScreenId secondaryScreenId,
    MultiScreenMode screenMode)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    bool res = data.WriteUint64(static_cast<uint64_t>(mainScreenId)) &&
        data.WriteUint64(static_cast<uint64_t>(secondaryScreenId)) &&
        data.WriteUint32(static_cast<uint32_t>(screenMode));
    if (!res) {
        TLOGE(WmsLogTag::DMS, "data write failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_MULTI_SCREEN_MODE_SWITCH),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    return ret;
}

DMError ScreenSessionManagerProxy::SetMultiScreenRelativePosition(MultiScreenPositionOptions mainScreenOptions,
    MultiScreenPositionOptions secondScreenOption)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    bool res = data.WriteUint64(mainScreenOptions.screenId_) &&
        data.WriteUint32(mainScreenOptions.startX_) && data.WriteUint32(mainScreenOptions.startY_) &&
        data.WriteUint64(secondScreenOption.screenId_) &&
        data.WriteUint32(secondScreenOption.startX_) && data.WriteUint32(secondScreenOption.startY_);
    if (!res) {
        TLOGE(WmsLogTag::DMS, "data write failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_MULTI_SCREEN_POSITION),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    return ret;
}

DMError ScreenSessionManagerProxy::StopMirror(const std::vector<ScreenId>& mirrorScreenIds)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "StopMirror fail: remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "StopMirror fail: WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    bool res = data.WriteUInt64Vector(mirrorScreenIds);
    if (!res) {
        TLOGE(WmsLogTag::DMS, "StopMirror fail: data write failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_STOP_MIRROR),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "StopMirror fail: SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::DisableMirror(bool disableOrNot)
{
    TLOGI(WmsLogTag::DMS, "SCB: ScreenSessionManagerProxy::DisableMirror %{public}d", disableOrNot);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "DisableMirror fail: remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "DisableMirror fail: WriteinterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteBool(disableOrNot)) {
        TLOGE(WmsLogTag::DMS, "DisableMirror fail: data write failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_DISABLE_MIRROR),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "DisableMirror fail: SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::MakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoint,
                                              ScreenId& screenGroupId)
{
    TLOGW(WmsLogTag::DMS, "SCB: ENTER");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "SCB: WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUInt64Vector(screenId)) {
        TLOGE(WmsLogTag::DMS, "SCB: write screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!MarshallingHelper::MarshallingVectorObj<Point>(data, startPoint, [](Parcel& parcel, const Point& point) {
            return parcel.WriteInt32(point.posX_) && parcel.WriteInt32(point.posY_);
        })) {
        TLOGE(WmsLogTag::DMS, "SCB: write startPoint failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_EXPAND),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SCB: SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    screenGroupId = static_cast<ScreenId>(reply.ReadUint64());
    return ret;
}

DMError ScreenSessionManagerProxy::StopExpand(const std::vector<ScreenId>& expandScreenIds)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    bool res = data.WriteUInt64Vector(expandScreenIds);
    if (!res) {
        TLOGE(WmsLogTag::DMS, "data write failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_STOP_EXPAND),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

sptr<ScreenGroupInfo> ScreenSessionManagerProxy::GetScreenGroupInfoById(ScreenId screenId)
{
    TLOGW(WmsLogTag::DMS, "SCB:ENTER!");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB:remote is nullptr");
        return nullptr;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "SCB: WriteInterfaceToken failed");
        return nullptr;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "SCB: Write screenId failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_GROUP_INFO_BY_ID),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SCB: SendRequest failed");
        return nullptr;
    }

    sptr<ScreenGroupInfo> info = reply.ReadStrongParcelable<ScreenGroupInfo>();
    if (info == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: SendRequest nullptr.");
        return nullptr;
    }
    return info;
}

void ScreenSessionManagerProxy::RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens)
{
    TLOGW(WmsLogTag::DMS, "SCB: ENTER!");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: remote is null");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "SCB: WriteInterfaceToken failed");
        return;
    }
    bool res = data.WriteUInt64Vector(screens);
    if (!res) {
        TLOGE(WmsLogTag::DMS, "SCB: write screens failed.");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_FROM_SCREEN_GROUP),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SCB:SendRequest failed");
    }
}

std::shared_ptr<Media::PixelMap> ScreenSessionManagerProxy::GetDisplaySnapshot(DisplayId displayId,
    DmErrorCode* errorCode, bool isUseDma, bool isCaptureFullOfScreen)
{
    TLOGD(WmsLogTag::DMS, "SCB: enter");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: remote is nullptr");
        return nullptr;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "SCB: WriteInterfaceToken failed");
        return nullptr;
    }

    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS, "SCB: Write displayId failed");
        return nullptr;
    }

    if (!data.WriteBool(isUseDma)) {
        TLOGE(WmsLogTag::DMS, "isUseDma fail: data write failed");
        return nullptr;
    }

    if (!data.WriteBool(isCaptureFullOfScreen)) {
        TLOGE(WmsLogTag::DMS, "isCaptureFullOfScreen fail: data write failed");
        return nullptr;
    }

    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SCB: SendRequest failed");
        return nullptr;
    }

    std::shared_ptr<Media::PixelMap> pixelMap(reply.ReadParcelable<Media::PixelMap>());
    DmErrorCode replyErrorCode = static_cast<DmErrorCode>(reply.ReadInt32());
    if (errorCode) {
        *errorCode = replyErrorCode;
    }
    if (pixelMap == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: SendRequest nullptr.");
        return nullptr;
    }
    return pixelMap;
}

std::vector<std::shared_ptr<Media::PixelMap>> ScreenSessionManagerProxy::GetDisplayHDRSnapshot(DisplayId displayId,
    DmErrorCode& errorCode, bool isUseDma, bool isCaptureFullOfScreen)
{
    TLOGD(WmsLogTag::DMS, "enter");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "SCB: remote is nullptr");
        return { nullptr, nullptr };
    }
 
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "SCB: WriteInterfaceToken failed");
        return { nullptr, nullptr };
    }
 
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS, "SCB: Write displayId failed");
        return { nullptr, nullptr };
    }
 
    if (!data.WriteBool(isUseDma)) {
        TLOGE(WmsLogTag::DMS, "isUseDma fail: data write failed");
        return { nullptr, nullptr };
    }
 
    if (!data.WriteBool(isCaptureFullOfScreen)) {
        TLOGE(WmsLogTag::DMS, "isCaptureFullOfScreen fail: data write failed");
        return { nullptr, nullptr };
    }
 
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HDR_SNAPSHOT),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SCB: SendRequest failed");
        return { nullptr, nullptr };
    }
    TLOGI(WmsLogTag::DMS, "reply pixelMap and hdrPixelMap");
    std::shared_ptr<Media::PixelMap> sdrpixelMap(reply.ReadParcelable<Media::PixelMap>());
    std::shared_ptr<Media::PixelMap> hdrPixelMap(reply.ReadParcelable<Media::PixelMap>());
    errorCode = static_cast<DmErrorCode>(reply.ReadInt32());
    if (sdrpixelMap == nullptr) {
        TLOGW(WmsLogTag::DMS, "SendRequest sdrpixelMap is nullptr.");
        return { nullptr, nullptr };
    }
    return { sdrpixelMap, hdrPixelMap };
}

std::shared_ptr<Media::PixelMap> ScreenSessionManagerProxy::GetSnapshotByPicker(Media::Rect &rect,
    DmErrorCode* errorCode)
{
    TLOGD(WmsLogTag::DMS, "enter");
    sptr<IRemoteObject> remote = Remote();
    *errorCode = DmErrorCode::DM_ERROR_SYSTEM_INNORMAL;
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return nullptr;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SNAPSHOT_BY_PICKER),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> pixelMap(reply.ReadParcelable<Media::PixelMap>()); // need't to check nullptr
    *errorCode = static_cast<DmErrorCode>(reply.ReadInt32());
    rect.left = reply.ReadInt32();
    rect.top = reply.ReadInt32();
    rect.width = reply.ReadInt32();
    rect.height = reply.ReadInt32();
    return pixelMap;
}

sptr<DisplayInfo> ScreenSessionManagerProxy::GetDisplayInfoById(DisplayId displayId)
{
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
        TLOGD(WmsLogTag::DMS, "SendRequest nullptr.");
        return nullptr;
    }
    return info;
}

sptr<DisplayInfo> ScreenSessionManagerProxy::GetVisibleAreaDisplayInfoById(DisplayId displayId)
{
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
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_VISIBLE_AREA_DISPLAY_INFO_BY_ID),
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
}

sptr<DisplayInfo> ScreenSessionManagerProxy::GetDisplayInfoByScreen(ScreenId screenId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
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
        TLOGW(WmsLogTag::DMS, "WriteUint64 displayId failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_BY_SCREEN),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return nullptr;
    }

    sptr<DisplayInfo> info = reply.ReadParcelable<DisplayInfo>();
    if (info == nullptr) {
        TLOGW(WmsLogTag::DMS, "SendRequest null");
        return nullptr;
    }
    return info;
}

std::vector<DisplayId> ScreenSessionManagerProxy::GetAllDisplayIds(int32_t userId)
{
    std::vector<DisplayId> allDisplayIds;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return allDisplayIds;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return allDisplayIds;
    }
    if (!data.WriteInt32(userId)) {
        TLOGE(WmsLogTag::DMS, "WriteInt32 userId failed");
        return allDisplayIds;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_DISPLAYIDS),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return allDisplayIds;
    }
    reply.ReadUInt64Vector(&allDisplayIds);
    return allDisplayIds;
}

sptr<ScreenInfo> ScreenSessionManagerProxy::GetScreenInfoById(ScreenId screenId)
{
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
        TLOGD(WmsLogTag::DMS, "info modes is id: %{public}u, width: %{public}u, height: %{public}u, refreshRate: %{public}u",
            mode->id_, mode->width_, mode->height_, mode->refreshRate_);
    }
    return info;
}

DMError ScreenSessionManagerProxy::GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos)
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
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_SCREEN_INFOS),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    MarshallingHelper::UnmarshallingVectorParcelableObj<ScreenInfo>(reply, screenInfos);
    return ret;
}

DMError ScreenSessionManagerProxy::GetScreenSupportedColorGamuts(ScreenId screenId,
    std::vector<ScreenColorGamut>& colorGamuts)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGW(WmsLogTag::DMS, "WriteUint64 screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_GAMUTS),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    if (ret != DMError::DM_OK) {
        return ret;
    }
    MarshallingHelper::UnmarshallingVectorObj<ScreenColorGamut>(reply, colorGamuts,
        [](Parcel& parcel, ScreenColorGamut& color) {
            uint32_t value;
            bool res = parcel.ReadUint32(value);
            color = static_cast<ScreenColorGamut>(value);
            return res;
        }
    );
    return ret;
}

DMError ScreenSessionManagerProxy::GetBrightnessInfo(DisplayId displayId, ScreenBrightnessInfo& brightnessInfo)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(displayId))) {
        TLOGW(WmsLogTag::DMS, "Write displayId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SCREEN_BRIGHTNESS_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::DMS, "Read ret failed!");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (static_cast<DMError>(ret) != DMError::DM_OK) {
        return static_cast<DMError>(ret);
    }
    if (!reply.ReadFloat(brightnessInfo.currentHeadroom)) {
        TLOGE(WmsLogTag::DMS, "Read currentHeadroom failed!");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadFloat(brightnessInfo.maxHeadroom)) {
        TLOGE(WmsLogTag::DMS, "Read maxHeadroom failed!");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadFloat(brightnessInfo.sdrNits)) {
        TLOGE(WmsLogTag::DMS, "Read sdrNits failed!");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return DMError::DM_OK;
}

DMError ScreenSessionManagerProxy::GetSupportsInput(DisplayId displayId, bool& supportsInput)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(displayId))) {
        TLOGW(WmsLogTag::DMS, "Write displayId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTS_INPUT),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::DMS, "Read ret failed!");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (static_cast<DMError>(ret) != DMError::DM_OK) {
        return static_cast<DMError>(ret);
    }
    if (!reply.ReadBool(supportsInput)) {
        TLOGE(WmsLogTag::DMS, "Read supportsInput failed!");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return DMError::DM_OK;
}

DMError ScreenSessionManagerProxy::SetSupportsInput(DisplayId displayId, bool supportsInput)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(displayId))) {
        TLOGW(WmsLogTag::DMS, "Write displayId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(supportsInput)) {
        TLOGW(WmsLogTag::DMS, "Write supportsInput failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_SUPPORTS_INPUT),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::DMS, "Read ret failed!");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (static_cast<DMError>(ret) != DMError::DM_OK) {
        return static_cast<DMError>(ret);
    }
    return DMError::DM_OK;
}

DMError ScreenSessionManagerProxy::GetBundleName(DisplayId displayId, std::string& bundleName)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(displayId))) {
        TLOGW(WmsLogTag::DMS, "Write displayId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_BUNDLE_NAME),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::DMS, "Read ret failed!");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    bundleName = reply.ReadString();
    return static_cast<DMError>(ret);
}

DMError ScreenSessionManagerProxy::GetRoundedCorner(DisplayId displayId, int& radius)
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
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(displayId))) {
        TLOGW(WmsLogTag::DMS, "Write displayId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ROUNDED_CORNER),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadInt32(radius)) {
        TLOGE(WmsLogTag::DMS, "Read radius failed!");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::DMS, "Read ret failed!");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(ret);
}

DMError ScreenSessionManagerProxy::GetPhysicalScreenIds(std::vector<ScreenId>& screenIds)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGW(WmsLogTag::DMS, "WriteInterfaceToken failed");
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
        TLOGE(WmsLogTag::DMS, "fail to unmarshalling screenIds in stub.");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return ret;
}

DMError OHOS::Rosen::ScreenSessionManagerProxy::SetOrientation(ScreenId screenId,
    Orientation orientation, bool isFromNapi)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGW(WmsLogTag::DMS, "Write screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(orientation))) {
        TLOGW(WmsLogTag::DMS, "Write orientation failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isFromNapi)) {
        TLOGW(WmsLogTag::DMS, "Write isFromNapi failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_ORIENTATION),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError OHOS::Rosen::ScreenSessionManagerProxy::SetScreenRotationLocked(bool isLocked)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteBool(isLocked)) {
        TLOGE(WmsLogTag::DMS, "write isLocked failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError OHOS::Rosen::ScreenSessionManagerProxy::SetScreenRotationLockedFromJs(bool isLocked)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteBool(isLocked)) {
        TLOGE(WmsLogTag::DMS, "write isLocked failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED_FROM_JS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError OHOS::Rosen::ScreenSessionManagerProxy::IsScreenRotationLocked(bool& isLocked)
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
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_IS_SCREEN_ROTATION_LOCKED),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    isLocked = reply.ReadBool();
    return ret;
}

sptr<CutoutInfo> ScreenSessionManagerProxy::GetCutoutInfo(DisplayId displayId)
{
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
        TLOGW(WmsLogTag::DMS, "GetCutoutInfo failed");
        return nullptr;
    }
    sptr<CutoutInfo> info = reply.ReadParcelable<CutoutInfo>();
    return info;
}

sptr<CutoutInfo> ScreenSessionManagerProxy::GetCutoutInfo(DisplayId displayId, int32_t width,
                                                          int32_t height, Rotation rotation)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null!");
        return nullptr;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "with rotation : failed!");
        return nullptr;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS, "write displayId failed!");
        return nullptr;
    }
    if (!data.WriteInt32(width)) {
        TLOGE(WmsLogTag::DMS, "write width failed!");
        return nullptr;
    }
    if (!data.WriteInt32(height)) {
        TLOGE(WmsLogTag::DMS, "write height failed!");
        return nullptr;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(rotation))) {
        TLOGE(WmsLogTag::DMS, "write rotation failed!");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_CUTOUT_INFO_WITH_ROTATION),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "GetCutoutInfoWithRotation failed");
        return nullptr;
    }
    sptr<CutoutInfo> info = reply.ReadParcelable<CutoutInfo>();
    return info;
}

DMError ScreenSessionManagerProxy::HasImmersiveWindow(ScreenId screenId, bool& immersive)
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
    if (!data.WriteUint64(screenId)) {
        return DMError::DM_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_HAS_IMMERSIVE_WINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    immersive = reply.ReadBool();
    return ret;
}

bool OHOS::Rosen::ScreenSessionManagerProxy::ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId)
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
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }

    if (!data.WriteUint64(screenId)) {
        return false;
    }

    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_CONVERT_SCREENID_TO_RSSCREENID),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return false;
    }
    bool ret = reply.ReadBool();
    rsScreenId = static_cast<ScreenId>(reply.ReadUint64());
    return ret;
}

DMError OHOS::Rosen::ScreenSessionManagerProxy::HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow)
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

    if (!data.WriteUint64(displayId)) {
        return DMError::DM_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_HAS_PRIVATE_WINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    hasPrivateWindow = reply.ReadBool();
    return ret;
}

void ScreenSessionManagerProxy::DumpAllScreensInfo(std::string& dumpInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_DUMP_ALL_SCREEN),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "failed");
        return;
    }
    dumpInfo = reply.ReadString();
}

void ScreenSessionManagerProxy::DumpSpecialScreenInfo(ScreenId id, std::string& dumpInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "failed");
        return;
    }
    if (!data.WriteUint64(id)) {
        TLOGE(WmsLogTag::DMS, "write ScreenId failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_DUMP_SPECIAL_SCREEN),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "failed");
        return;
    }
    dumpInfo = reply.ReadString();
}

//Fold Screen
void ScreenSessionManagerProxy::SetFoldDisplayMode(const FoldDisplayMode displayMode)
{
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
}

void ScreenSessionManagerProxy::SetFoldDisplayModeAsync(const FoldDisplayMode displayMode)
{
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
}

//SetFoldDisplayModeFromJs add DMError to return DMErrorCode for js
DMError ScreenSessionManagerProxy::SetFoldDisplayModeFromJs(const FoldDisplayMode displayMode, std::string reason)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken Failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(displayMode))) {
        TLOGE(WmsLogTag::DMS, "Write displayMode failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(reason)) {
        TLOGE(WmsLogTag::DMS, "Write reason failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_FOLD_DISPLAY_MODE_FROM_JS),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "Send request failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    DMError ret = static_cast<DMError>(reply.ReadInt32());
    return ret;
}

void ScreenSessionManagerProxy::SetDisplayScale(ScreenId screenId, float scaleX, float scaleY, float pivotX,
    float pivotY)
{
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
    if (!(data.WriteUint64(static_cast<uint64_t>(screenId)) && data.WriteFloat(scaleX) && data.WriteFloat(scaleY) &&
        data.WriteFloat(pivotX) && data.WriteFloat(pivotY))) {
        TLOGE(WmsLogTag::DMS, "Write screen scale info failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_DISPLAY_SCALE),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "Send TRANS_ID_SCENE_BOARD_SET_DISPLAY_SCALE request failed");
    }
}

void ScreenSessionManagerProxy::SetFoldStatusLocked(bool locked)
{
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
    if (!data.WriteUint32(static_cast<uint32_t>(locked))) {
        TLOGE(WmsLogTag::DMS, "Write lock fold display status failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
                            DisplayManagerMessage::TRANS_ID_SCENE_BOARD_LOCK_FOLD_DISPLAY_STATUS),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "Send TRANS_ID_SCENE_BOARD_LOCK_FOLD_DISPLAY_STATUS request failed");
    }
}

// SetFoldStatusLockedFromJs add DMError to return DMErrorCode for js
DMError ScreenSessionManagerProxy::SetFoldStatusLockedFromJs(bool locked)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken Failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(locked))) {
        TLOGE(WmsLogTag::DMS, "Write lock fold display status failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
                            DisplayManagerMessage::TRANS_ID_SET_LOCK_FOLD_DISPLAY_STATUS_FROM_JS),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "Send request failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    DMError ret = static_cast<DMError>(reply.ReadInt32());
    return ret;
}

DMError ScreenSessionManagerProxy::ForceSetFoldStatusAndLock(FoldStatus targetFoldStatus)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    if (targetFoldStatus < FoldStatus::EXPAND ||
            targetFoldStatus > FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_HALF_FOLDED) {
        TLOGE(WmsLogTag::DMS, "Invalid targetFoldStatus: %{public}d, IPC stop", static_cast<int>(targetFoldStatus));
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken Failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(targetFoldStatus))) {
        TLOGE(WmsLogTag::DMS, "Write lock fold status failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
                            DisplayManagerMessage::TRANS_ID_SET_TARGET_FOLD_STATUS_AND_LOCK),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "Send TRANS_ID_SET_TARGET_FOLD_STATUS_AND_LOCK failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        TLOGE(WmsLogTag::DMS, "Failed to read error code from reply");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(errorCode);
    return ret;
}

DMError ScreenSessionManagerProxy::RestorePhysicalFoldStatus()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken Failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
                            DisplayManagerMessage::TRANS_ID_UNLOCK_TARGET_FOLD_STATUS),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "Send TRANS_ID_UNLOCK_TARGET_FOLD_STATUS failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        TLOGE(WmsLogTag::DMS, "Failed to read error code from reply");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(errorCode);
    return ret;
}

void ScreenSessionManagerProxy::SetFoldStatusExpandAndLocked(bool locked)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken Failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(locked))) {
        TLOGE(WmsLogTag::DMS, "Write lock fold display status failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
                            DisplayManagerMessage::TRANS_ID_SET_FOLD_STATUS_EXPAND_AND_LOCKED),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "Send TRANS_ID_SET_FOLD_STATUS_EXPAND_AND_LOCKED request failed");
    }
}

FoldDisplayMode ScreenSessionManagerProxy::GetFoldDisplayMode()
{
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
}

bool ScreenSessionManagerProxy::IsFoldable()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return false;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_IS_FOLDABLE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return false;
    }
    return reply.ReadBool();
}

bool ScreenSessionManagerProxy::IsCaptured()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return false;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_DEVICE_IS_CAPTURE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return false;
    }
    return reply.ReadBool();
}

FoldStatus ScreenSessionManagerProxy::GetFoldStatus()
{
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
}

SuperFoldStatus ScreenSessionManagerProxy::GetSuperFoldStatus()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return SuperFoldStatus::UNKNOWN;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return SuperFoldStatus::UNKNOWN;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_SUPER_FOLD_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return SuperFoldStatus::UNKNOWN;
    }
    return static_cast<SuperFoldStatus>(reply.ReadUint32());
}

float ScreenSessionManagerProxy::GetSuperRotation()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return -1.f;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return -1.f;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_SUPER_ROTATION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return -1.f;
    }
    return reply.ReadFloat();
}

void ScreenSessionManagerProxy::SetLandscapeLockStatus(bool isLocked)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(isLocked)) {
        TLOGE(WmsLogTag::DMS, "Write isLocked failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_LANDSCAPE_LOCK_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::SetForceCloseHdr(ScreenId screenId, bool isForceCloseHdr)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!(data.WriteUint64(static_cast<uint64_t>(screenId)))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteBool(isForceCloseHdr)) {
        TLOGE(WmsLogTag::DMS, "Write isForceCloseHdr failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_FORCE_CLOSE_HDR),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

ExtendScreenConnectStatus ScreenSessionManagerProxy::GetExtendScreenConnectStatus()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return ExtendScreenConnectStatus::UNKNOWN;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return ExtendScreenConnectStatus::UNKNOWN;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_EXTEND_SCREEN_CONNECT_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return ExtendScreenConnectStatus::UNKNOWN;
    }
    return static_cast<ExtendScreenConnectStatus>(reply.ReadUint32());
}

sptr<FoldCreaseRegion> ScreenSessionManagerProxy::GetCurrentFoldCreaseRegion()
{
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
    if (remote->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_CURRENT_FOLD_CREASE_REGION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return nullptr;
    }
    return reply.ReadStrongParcelable<FoldCreaseRegion>();
}

DMError ScreenSessionManagerProxy::GetLiveCreaseRegion(FoldCreaseRegion& region)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_LIVE_CREASE_REGION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    if (ret != DMError::DM_OK) {
        return ret;
    }
    DisplayId displayId;
    if (!reply.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::DMS, "Read displayId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    uint32_t size = 0;
    if (!reply.ReadUint32(size) || size > MAX_CREASE_REGION_SIZE) {
        TLOGE(WmsLogTag::DMS, "Invalid creaseRects size");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    std::vector<DMRect> creaseRects;
    for (uint32_t i = 0; i < size; i++) {
        int32_t posX;
        int32_t posY;
        uint32_t width;
        uint32_t height;
        if (!reply.ReadInt32(posX) || !reply.ReadInt32(posY) ||
        !reply.ReadUint32(width) || !reply.ReadUint32(height)) {
            TLOGE(WmsLogTag::DMS, "Failed to read crease rect");
            return DMError::DM_ERROR_IPC_FAILED;
        }
        creaseRects.emplace_back(DMRect{posX, posY, width, height});
    }
    region = FoldCreaseRegion(displayId, creaseRects);
    return ret;
}

DMError ScreenSessionManagerProxy::MakeUniqueScreen(const std::vector<ScreenId>& screenIds,
    std::vector<DisplayId>& displayIds, const UniqueScreenRotationOptions& rotationOptions)
{
    TLOGI(WmsLogTag::DMS, "enter");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "writeInterfaceToken failed");
        return DMError::DM_ERROR_NULLPTR;
    }
    if (!data.WriteUint32(screenIds.size())) {
        TLOGE(WmsLogTag::DMS, "write screenIds size failed");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    bool res = data.WriteUInt64Vector(screenIds);
    if (!res) {
        TLOGE(WmsLogTag::DMS, "write screens failed");
        return DMError::DM_ERROR_NULLPTR;
    }
    if (!data.WriteBool(rotationOptions.isRotationLocked_)) {
        TLOGE(WmsLogTag::DMS, "write isRotationLocked failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (!data.WriteInt32(rotationOptions.rotation_)) {
        TLOGE(WmsLogTag::DMS, "write rotation failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    TLOGD(WmsLogTag::DMS,
          "IPC Proxy sending parameters, isRotationLocked: %{public}d, rotation: %{public}d",
          rotationOptions.isRotationLocked_, rotationOptions.rotation_);

    if (remote->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_NULLPTR;
    }
    reply.ReadUInt64Vector(&displayIds);
    return static_cast<DMError>(reply.ReadInt32());
}

void ScreenSessionManagerProxy::SetClient(const sptr<IScreenSessionManagerClient>& client)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!client || !data.WriteRemoteObject(client->AsObject())) {
        TLOGE(WmsLogTag::DMS, "WriteRemoteObject failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_CLIENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::SwitchUser()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SWITCH_USER),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

ScreenProperty ScreenSessionManagerProxy::GetScreenProperty(ScreenId screenId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return {};
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return {};
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return {};
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_PROPERTY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return {};
    }
    ScreenProperty screenProperty;
    if (!RSMarshallingHelper::Unmarshalling(reply, screenProperty)) {
        TLOGE(WmsLogTag::DMS, "Read screenProperty failed");
        return {};
    }
    return screenProperty;
}

std::shared_ptr<RSDisplayNode> ScreenSessionManagerProxy::GetDisplayNode(ScreenId screenId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return nullptr;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return nullptr;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_NODE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return nullptr;
    }

    auto displayNode = RSDisplayNode::Unmarshalling(reply);
    if (!displayNode) {
        TLOGE(WmsLogTag::DMS, "displayNode is null");
        return nullptr;
    }
    return displayNode;
}

ScreenCombination ScreenSessionManagerProxy::GetScreenCombination(ScreenId screenId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return ScreenCombination::SCREEN_ALONE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return ScreenCombination::SCREEN_ALONE;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return ScreenCombination::SCREEN_ALONE;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_COMBINATION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return ScreenCombination::SCREEN_ALONE;
    }
    return static_cast<ScreenCombination>(reply.ReadUint32());
}

void ScreenSessionManagerProxy::UpdateScreenDirectionInfo(ScreenId screenId, const ScreenDirectionInfo& directionInfo,
    ScreenPropertyChangeType screenPropertyChangeType, const RRect& bounds)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
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
    if (!data.WriteFloat(directionInfo.screenRotation_)) {
        TLOGE(WmsLogTag::DMS, "Write screenComponentRotation failed");
        return;
    }
    if (!data.WriteFloat(directionInfo.rotation_)) {
        TLOGE(WmsLogTag::DMS, "Write rotation failed");
        return;
    }
    if (!data.WriteFloat(directionInfo.phyRotation_)) {
        TLOGE(WmsLogTag::DMS, "Write phyRotation failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(screenPropertyChangeType))) {
        TLOGE(WmsLogTag::DMS, "Write screenPropertyChangeType failed");
        return;
    }
    if (!RSMarshallingHelper::Marshalling(data, bounds)) {
        TLOGE(WmsLogTag::DMS, "Write bounds failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_DIRECTION_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::UpdateScreenRotationProperty(ScreenId screenId, const RRect& bounds, float rotation,
    ScreenPropertyChangeType screenPropertyChangeType, bool isSwitchUser)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
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
    if (!RSMarshallingHelper::Marshalling(data, bounds)) {
        TLOGE(WmsLogTag::DMS, "Write bounds failed");
        return;
    }
    if (!data.WriteFloat(rotation)) {
        TLOGE(WmsLogTag::DMS, "Write rotation failed");
        return;
    }
    if (!data.WriteBool(isSwitchUser)) {
        TLOGE(WmsLogTag::DMS, "Write isSwitchUser failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(screenPropertyChangeType))) {
        TLOGE(WmsLogTag::DMS, "Write screenPropertyChangeType failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_ROTATION_PROPERTY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

uint32_t ScreenSessionManagerProxy::GetCurvedCompressionArea()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return 0;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return 0;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_CURVED_SCREEN_COMPRESSION_AREA),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return 0;
    }

    return reply.ReadUint32();
}

ScreenProperty ScreenSessionManagerProxy::GetPhyScreenProperty(ScreenId screenId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return {};
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return {};
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return {};
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PHY_SCREEN_PROPERTY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return {};
    }
    ScreenProperty screenProperty;
    if (!RSMarshallingHelper::Unmarshalling(reply, screenProperty)) {
        TLOGE(WmsLogTag::DMS, "Read screenProperty failed");
        return {};
    }
    return screenProperty;
}

void ScreenSessionManagerProxy::NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageOption option(MessageOption::TF_ASYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!info->Marshalling(data)) {
        TLOGE(WmsLogTag::DMS, "Write display change info failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_CHANGE_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::SetScreenPrivacyState(bool hasPrivate)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(hasPrivate)) {
        TLOGE(WmsLogTag::DMS, "Write hasPrivate failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_STATE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::SetPrivacyStateByDisplayId(std::unordered_map<DisplayId, bool>& privacyBundleDisplayId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(privacyBundleDisplayId.size())) {
        TLOGE(WmsLogTag::DMS, "Write privacyBundleDisplayId size failed");
        return;
    }
    for (const auto& iter : privacyBundleDisplayId) {
        if (!data.WriteUint64(iter.first)) {
            TLOGE(WmsLogTag::DMS, "Write DisplayId failed");
            return;
        }
        if (!data.WriteBool(iter.second)) {
            TLOGE(WmsLogTag::DMS, "Write hasPrivate failed");
            return;
        }
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREENID_PRIVACY_STATE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::SetScreenPrivacyWindowList(DisplayId id, std::vector<std::string> privacyWindowList)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(id)) {
        TLOGE(WmsLogTag::DMS, "Write DisplayId failed");
        return;
    }
    if (!data.WriteStringVector(privacyWindowList)) {
        TLOGE(WmsLogTag::DMS, "Write privacyWindowList failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_LIST),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::UpdateAvailableArea(ScreenId screenId, DMRect area)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageOption option(MessageOption::TF_ASYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteInt32(area.posX_) || !data.WriteInt32(area.posY_) || !data.WriteUint32(area.width_) ||
        !data.WriteInt32(area.height_)) {
        TLOGE(WmsLogTag::DMS, "Write area failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_AVAILABLE_AREA),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::UpdateSuperFoldAvailableArea(ScreenId screenId, DMRect bArea, DMRect cArea)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageOption option(MessageOption::TF_ASYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteInt32(bArea.posX_) || !data.WriteInt32(bArea.posY_) || !data.WriteUint32(bArea.width_) ||
        !data.WriteUint32(bArea.height_)) {
        TLOGE(WmsLogTag::DMS, "Write bArea failed");
        return;
    }
    if (!data.WriteInt32(cArea.posX_) || !data.WriteInt32(cArea.posY_) || !data.WriteUint32(cArea.width_) ||
        !data.WriteUint32(cArea.height_)) {
        TLOGE(WmsLogTag::DMS, "Write cArea failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SUPER_FOLD_AVAILABLE_AREA),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::UpdateSuperFoldExpandAvailableArea(ScreenId screenId, DMRect area)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageOption option(MessageOption::TF_ASYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteInt32(area.posX_) || !data.WriteInt32(area.posY_) || !data.WriteUint32(area.width_) ||
        !data.WriteUint32(area.height_)) {
        TLOGE(WmsLogTag::DMS, "Write area failed");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SUPER_FOLD_EXPAND_AVAILABLE_AREA),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

int32_t ScreenSessionManagerProxy::SetScreenOffDelayTime(int32_t delay)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return 0;
    }

    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return 0;
    }
    if (!data.WriteInt32(delay)) {
        TLOGE(WmsLogTag::DMS, "Write delay failed");
        return 0;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_OFF_DELAY_TIME),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return 0;
    }
    return reply.ReadInt32();
}

void ScreenSessionManagerProxy::SetCameraStatus(int32_t cameraStatus, int32_t cameraPosition)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteInt32(cameraStatus)) {
        TLOGE(WmsLogTag::DMS, "Write cameraStatus failed");
        return;
    }
    if (!data.WriteInt32(cameraPosition)) {
        TLOGE(WmsLogTag::DMS, "Write cameraPosition failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_CAMERA_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

DMError ScreenSessionManagerProxy::GetAvailableArea(DisplayId displayId, DMRect& area)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    MessageOption option;
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS, "Write displayId failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_AVAILABLE_AREA),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    int32_t posX = reply.ReadInt32();
    int32_t posY = reply.ReadInt32();
    uint32_t width = reply.ReadUint32();
    uint32_t height = reply.ReadUint32();
    area = {posX, posY, width, height};
    return ret;
}

DMError ScreenSessionManagerProxy::GetExpandAvailableArea(DisplayId displayId, DMRect& area)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    MessageOption option;
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DMS, "Write displayId failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_EXPAND_AVAILABLE_AREA),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    int32_t posX = reply.ReadInt32();
    int32_t posY = reply.ReadInt32();
    uint32_t width = reply.ReadUint32();
    uint32_t height = reply.ReadUint32();
    area = {posX, posY, width, height};
    return ret;
}

void ScreenSessionManagerProxy::NotifyFoldToExpandCompletion(bool foldToExpand)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageOption option(MessageOption::TF_ASYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return ;
    }
    if (!data.WriteBool(foldToExpand)) {
        TLOGE(WmsLogTag::DMS, "Write foldToExpand failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_FOLD_TO_EXPAND_COMPLETION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::NotifyScreenConnectCompletion(ScreenId screenId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageOption option(MessageOption::TF_ASYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return ;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_SCREEN_CONNECT_COMPLETION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::NotifyAodOpCompletion(AodOP operation, int32_t result)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageOption option(MessageOption::TF_ASYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return ;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(operation))) {
        TLOGE(WmsLogTag::DMS, "Write operation failed");
        return;
    }
    if (!data.WriteInt32(result)) {
        TLOGE(WmsLogTag::DMS, "Write operation result failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_AOD_OP_COMPLETION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::RecordEventFromScb(std::string description, bool needRecordEvent)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageOption option(MessageOption::TF_ASYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return ;
    }
    if (!data.WriteString(description)) {
        TLOGE(WmsLogTag::DMS, "Write description failed");
        return;
    }
    if (!data.WriteBool(needRecordEvent)) {
        TLOGE(WmsLogTag::DMS, "Write needRecordEvent failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_RECORD_EVENT_FROM_SCB),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

VirtualScreenFlag ScreenSessionManagerProxy::GetVirtualScreenFlag(ScreenId screenId)
{
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
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return VirtualScreenFlag::DEFAULT;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return VirtualScreenFlag::DEFAULT;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_VIRTUAL_SCREEN_FLAG),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return VirtualScreenFlag::DEFAULT;
    }
    return static_cast<VirtualScreenFlag>(reply.ReadUint32());
}

DMError ScreenSessionManagerProxy::SetVirtualScreenFlag(ScreenId screenId, VirtualScreenFlag screenFlag)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    if (screenId == SCREEN_ID_INVALID) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (screenFlag < VirtualScreenFlag::DEFAULT || screenFlag >= VirtualScreenFlag::MAX) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(screenId) || !data.WriteUint32(static_cast<uint32_t>(screenFlag))) {
        TLOGE(WmsLogTag::DMS, "Write screenId or screenFlag failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_FLAG),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DeviceScreenConfig ScreenSessionManagerProxy::GetDeviceScreenConfig()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return {};
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return {};
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DEVICE_SCREEN_CONFIG),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return {};
    }
    DeviceScreenConfig deviceScreenConfig;
    if (!RSMarshallingHelper::Unmarshalling(reply, deviceScreenConfig)) {
        TLOGE(WmsLogTag::DMS, "Read deviceScreenConfig failed");
        return {};
    }
    return deviceScreenConfig;
}

DMError ScreenSessionManagerProxy::SetVirtualScreenRefreshRate(ScreenId screenId, uint32_t refreshInterval)
{
    TLOGI(WmsLogTag::DMS, "ENTER");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(screenId) || !data.WriteUint32(refreshInterval)) {
        TLOGE(WmsLogTag::DMS, "Write screenId or refreshInterval failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_REFRESH_RATE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

void ScreenSessionManagerProxy::SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList,
    std::vector<uint64_t> surfaceIdList, std::vector<uint8_t> typeBlackList)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "Remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteUInt64Vector(windowIdList)) {
        TLOGE(WmsLogTag::DMS, "Write windowIdList failed");
        return;
    }
    if (!data.WriteUInt64Vector(surfaceIdList)) {
        TLOGE(WmsLogTag::DMS, "Write surfaceIdList failed");
        return;
    }
    if (!data.WriteUInt8Vector(typeBlackList)) {
        TLOGE(WmsLogTag::DMS, "Write typeBlackList failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_BLACK_LIST),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::SetVirtualDisplayMuteFlag(ScreenId screenId, bool muteFlag)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "Remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (!data.WriteBool(muteFlag)) {
        TLOGE(WmsLogTag::DMS, "Write muteFlag failed");
        return;
    }

    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_DISPLAY_MUTE_FLAG),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::DisablePowerOffRenderControl(ScreenId screenId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "Remote is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_DISABLE_POWEROFF_RENDER_CONTROL),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

DMError ScreenSessionManagerProxy::ProxyForFreeze(const std::set<int32_t>& pidList, bool isProxy)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "Remote is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    MessageParcel reply;
    MessageParcel data;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "proxy for freeze: failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteInt32(pidList.size())) {
        TLOGE(WmsLogTag::DMS, "proxy for freeze write date: failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    for (auto it = pidList.begin(); it != pidList.end(); it++) {
        if (!data.WriteInt32(*it)) {
            TLOGE(WmsLogTag::DMS, "proxy for freeze write date: failed");
            return DMError::DM_ERROR_WRITE_DATA_FAILED;
        }
    }
    if (!data.WriteBool(isProxy)) {
        TLOGE(WmsLogTag::DMS, "proxy for freeze write date: failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_PROXY_FOR_FREEZE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "proxy for freeze send request: failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::ResetAllFreezeStatus()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "Remote is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    MessageParcel reply;
    MessageParcel data;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_RESET_ALL_FREEZE_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

void OHOS::Rosen::ScreenSessionManagerProxy::UpdateDisplayHookInfo(int32_t uid, bool enable, const DMHookInfo& hookInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageOption option(MessageOption::TF_ASYNC);
    MessageParcel reply;
    MessageParcel data;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(uid)) {
        TLOGE(WmsLogTag::DMS, "Write uid failed");
        return;
    }

    if (!data.WriteBool(enable)) {
        TLOGE(WmsLogTag::DMS, "Write enable failed");
        return;
    }

    if (!data.WriteUint32(hookInfo.width_) || !data.WriteUint32(hookInfo.height_) ||
        !data.WriteFloat(hookInfo.density_) || !data.WriteUint32(hookInfo.rotation_) ||
        !data.WriteBool(hookInfo.enableHookRotation_) || !data.WriteUint32(hookInfo.displayOrientation_) ||
        !data.WriteBool(hookInfo.enableHookDisplayOrientation_) ||
        !data.WriteBool(hookInfo.isFullScreenInForceSplit_)) {
        TLOGE(WmsLogTag::DMS, "Write hookInfo failed");
        return;
    }

    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_HOOK_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::GetDisplayHookInfo(int32_t uid, DMHookInfo& hookInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return;
    }

    MessageOption option;
    MessageParcel reply;
    MessageParcel data;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(uid)) {
        TLOGE(WmsLogTag::DMS, "Write uid failed");
        return;
    }

    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HOOK_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
    if (!reply.ReadUint32(hookInfo.width_) || !reply.ReadUint32(hookInfo.height_) ||
        !reply.ReadFloat(hookInfo.density_) || !reply.ReadUint32(hookInfo.rotation_)||
        !reply.ReadBool(hookInfo.enableHookRotation_)) {
        TLOGE(WmsLogTag::DMS, "reply write hookInfo failed!");
    }
}

void ScreenSessionManagerProxy::NotifyIsFullScreenInForceSplitMode(int32_t uid, bool isFullScreen)
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
    if (!data.WriteInt32(uid)) {
        TLOGE(WmsLogTag::DMS, "Write uid failed");
        return;
    }
    if (!data.WriteBool(isFullScreen)) {
        TLOGE(WmsLogTag::DMS, "Write isFullScreen failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_IS_FULL_SCREEN_IN_FORCE_SPLIT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

DMError ScreenSessionManagerProxy::SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid,
    std::vector<uint64_t>& windowIdList)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "Remote is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    MessageParcel reply;
    MessageParcel data;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "proxy for freeze: failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "write date: failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (!data.WriteUint32(pid)) {
        TLOGE(WmsLogTag::DMS, "write date: failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }

    if (!data.WriteUInt64Vector(windowIdList)) {
        TLOGE(WmsLogTag::DMS, "write date: failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SECURITY_EXEMPTION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "send request: failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

std::vector<DisplayPhysicalResolution> ScreenSessionManagerProxy::GetAllDisplayPhysicalResolution()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return std::vector<DisplayPhysicalResolution> {};
    }
    MessageOption option;
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return std::vector<DisplayPhysicalResolution> {};
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_PHYSICAL_DISPLAY_RESOLUTION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return std::vector<DisplayPhysicalResolution> {};
    }
    std::vector<DisplayPhysicalResolution> allPhysicalSize;
    int32_t displayInfoSize = 0;
    bool readRet = reply.ReadInt32(displayInfoSize);
    if (!readRet || displayInfoSize <= 0) {
        return std::vector<DisplayPhysicalResolution> {};
    }
    for (int32_t i = 0; i < displayInfoSize; i++) {
        DisplayPhysicalResolution physicalItem;
        physicalItem.foldDisplayMode_ = static_cast<FoldDisplayMode>(reply.ReadUint32());
        physicalItem.physicalWidth_ = reply.ReadUint32();
        physicalItem.physicalHeight_ = reply.ReadUint32();
        allPhysicalSize.emplace_back(physicalItem);
    }
    return allPhysicalSize;
}

DMError ScreenSessionManagerProxy::GetDisplayCapability(std::string& capabilitInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPABILITY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    DMError ret = static_cast<DMError>(reply.ReadInt32());
    capabilitInfo = reply.ReadString();
    return ret;
}

bool ScreenSessionManagerProxy::SetVirtualScreenStatus(ScreenId screenId, VirtualScreenStatus screenStatus)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return false;
    }

    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }

    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return false;
    }

    if (!data.WriteInt32(static_cast<int32_t>(screenStatus))) {
        TLOGE(WmsLogTag::DMS, "Write screenStatus failed");
        return false;
    }

    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return false;
    }

    return reply.ReadBool();
}

DMError ScreenSessionManagerProxy::SetVirtualScreenMaxRefreshRate(ScreenId id, uint32_t refreshRate,
    uint32_t& actualRefreshRate)
{
    TLOGI(WmsLogTag::DMS, "ENTER");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(id))) {
        TLOGE(WmsLogTag::DMS, "WriteUnit64 screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(refreshRate)) {
        TLOGE(WmsLogTag::DMS, "WriteUnit32 width failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_MAX_REFRESHRATE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_NULLPTR;
    }
    actualRefreshRate = reply.ReadUint32();
    return static_cast<DMError>(reply.ReadInt32());
}

std::shared_ptr<Media::PixelMap> ScreenSessionManagerProxy::GetScreenCapture(const CaptureOption& captureOption,
    DmErrorCode* errorCode)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return nullptr;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return nullptr;
    }
    if (!data.WriteUint64(captureOption.displayId_) ||
        !data.WriteBool(captureOption.isNeedNotify_) || !data.WriteBool(captureOption.isNeedPointer_) ||
        !data.WriteUInt64Vector(captureOption.blackWindowIdList_)) {
        TLOGE(WmsLogTag::DMS, "Write displayId or isNeedNotify or isNeedPointer failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPTURE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> pixelMap(reply.ReadParcelable<Media::PixelMap>());
    DmErrorCode replyErrorCode = static_cast<DmErrorCode>(reply.ReadInt32());
    if (errorCode) {
        *errorCode = replyErrorCode;
    }
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::DMS, "Send pixelMap nullptr.");
        return nullptr;
    }
    return pixelMap;
}

sptr<DisplayInfo> ScreenSessionManagerProxy::GetPrimaryDisplayInfo()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "GetPrimaryDisplayInfo: remote is nullptr");
        return nullptr;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PRIMARY_DISPLAY_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return nullptr;
    }

    sptr<DisplayInfo> info = reply.ReadParcelable<DisplayInfo>();
    if (info == nullptr) {
        TLOGE(WmsLogTag::DMS, "get display info.");
    }
    return info;
}

DisplayId ScreenSessionManagerProxy::GetPrimaryDisplayId()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return SCREEN_ID_INVALID;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return SCREEN_ID_INVALID;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PRIMARY_DISPLAY_ID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return SCREEN_ID_INVALID;
    }

    DisplayId id = static_cast<DisplayId>(reply.ReadUint64());
    return id;
}

std::shared_ptr<Media::PixelMap> ScreenSessionManagerProxy::GetDisplaySnapshotWithOption(
    const CaptureOption& captureOption, DmErrorCode* errorCode)
{
    TLOGD(WmsLogTag::DMS, "enter");
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
    if (!data.WriteUint64(captureOption.displayId_) ||
        !data.WriteBool(captureOption.isNeedNotify_) || !data.WriteBool(captureOption.isNeedPointer_) ||
        !data.WriteBool(captureOption.isCaptureFullOfScreen_) ||
        !data.WriteUInt64Vector(captureOption.surfaceNodesList_)) {
        TLOGE(WmsLogTag::DMS, "Write displayId or isNeedNotify or isNeedPointer failed");
        return nullptr;
    }
    if (!data.WriteFloat(captureOption.scaleX_) || !data.WriteFloat(captureOption.scaleY_) ||
        !data.WriteInt32(captureOption.rect.posX_) || !data.WriteInt32(captureOption.rect.posY_) ||
        !data.WriteUint32(captureOption.rect.width_) || !data.WriteUint32(captureOption.rect.height_)) {
        TLOGE(WmsLogTag::DMS, "Write scale or rect failed");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION),
        data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return nullptr;
    }

    std::shared_ptr<Media::PixelMap> pixelMap(reply.ReadParcelable<Media::PixelMap>());
    DmErrorCode replyErrorCode = static_cast<DmErrorCode>(reply.ReadInt32());
    if (errorCode) {
        *errorCode = replyErrorCode;
    }
    if (pixelMap == nullptr) {
        TLOGW(WmsLogTag::DMS, "SendRequest nullptr.");
        return nullptr;
    }
    return pixelMap;
}

std::vector<std::shared_ptr<Media::PixelMap>> ScreenSessionManagerProxy::GetDisplayHDRSnapshotWithOption(
    const CaptureOption& captureOption, DmErrorCode& errorCode)
{
    TLOGD(WmsLogTag::DMS, "enter");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGW(WmsLogTag::DMS, "remote is nullptr");
        return { nullptr, nullptr };
    }
 
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return { nullptr, nullptr };
    }
    if (!data.WriteUint64(captureOption.displayId_) || !data.WriteBool(captureOption.isNeedNotify_) ||
        !data.WriteBool(captureOption.isCaptureFullOfScreen_) ||
        !data.WriteUInt64Vector(captureOption.surfaceNodesList_)) {
        TLOGE(WmsLogTag::DMS, "Write displayId or isNeedNotify or isNeedPointer failed");
        return { nullptr, nullptr };
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HDR_SNAPSHOT_WITH_OPTION), data, reply, option) != ERR_NONE) {
        TLOGW(WmsLogTag::DMS, "SendRequest failed");
        return { nullptr, nullptr };
    }
 
    std::shared_ptr<Media::PixelMap> sdrpixelMap(reply.ReadParcelable<Media::PixelMap>());
    std::shared_ptr<Media::PixelMap> hdrPixelMap(reply.ReadParcelable<Media::PixelMap>());
    errorCode = static_cast<DmErrorCode>(reply.ReadInt32());
    if (sdrpixelMap == nullptr) {
        TLOGW(WmsLogTag::DMS, "SendRequest sdrpixelMap is nullptr.");
        return { nullptr, nullptr };
    }
    return { sdrpixelMap, hdrPixelMap };
}

int32_t ScreenSessionManagerProxy::SetScreenOnDelayTime(int32_t delay)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "SetScreenOnDelayTime: remote is null");
        return 0;
    }

    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return 0;
    }
    if (!data.WriteInt32(delay)) {
        TLOGE(WmsLogTag::DMS, "Write delay failed");
        return 0;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ON_DELAY_TIME),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return 0;
    }
    return reply.ReadInt32();
}

DMError ScreenSessionManagerProxy::SetScreenSkipProtectedWindow(const std::vector<ScreenId>& screenIds, bool isEnable)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_REMOTE_CREATE_FAILED;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUInt64Vector(screenIds)) {
        TLOGE(WmsLogTag::DMS, "Write screenIds failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (!data.WriteBool(isEnable)) {
        TLOGE(WmsLogTag::DMS, "Write isEnable failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_SKIP_PROTECTED_WINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

bool ScreenSessionManagerProxy::GetIsRealScreen(ScreenId screenId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return false;
    }
    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_IS_REAL_SCREEN),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return false;
    }
    return reply.ReadBool();
}

void ScreenSessionManagerProxy::SetDefaultMultiScreenModeWhenSwitchUser()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageParcel reply;
    MessageParcel data;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_DEFAULT_MODE_WHEN_SWITCH_USER),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
    return;
}

void ScreenSessionManagerProxy::NotifyExtendScreenCreateFinish()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageParcel reply;
    MessageParcel data;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_EXTEND_SCREEN_CREATE_FINISH),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::NotifyExtendScreenDestroyFinish()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageParcel reply;
    MessageParcel data;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_EXTEND_SCREEN_DESTROY_FINISH),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

void ScreenSessionManagerProxy::NotifyScreenMaskAppear()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }

    MessageParcel reply;
    MessageParcel data;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_SCREEN_MASK_APPEAR),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

DMError ScreenSessionManagerProxy::GetScreenAreaOfDisplayArea(DisplayId displayId, const DMRect& displayArea,
    ScreenId& screenId, DMRect& screenArea)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is nullptr");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(displayId))) {
        TLOGE(WmsLogTag::DMS, "Write displayId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(displayArea.posX_) || !data.WriteInt32(displayArea.posY_) ||
        !data.WriteUint32(displayArea.width_) || !data.WriteUint32(displayArea.height_)) {
        TLOGE(WmsLogTag::DMS, "Write displayArea failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_AREA_OF_DISPLAY_AREA),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    DMError ret = static_cast<DMError>(reply.ReadInt32());
    screenId = static_cast<ScreenId>(reply.ReadUint64());
    int32_t posX = reply.ReadInt32();
    int32_t posY = reply.ReadInt32();
    uint32_t width = reply.ReadUint32();
    uint32_t height = reply.ReadUint32();
    screenArea = { posX, posY, width, height };
    return ret;
}

DMError ScreenSessionManagerProxy::SetPrimaryDisplaySystemDpi(float dpi)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    MessageParcel reply;
    MessageParcel data;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteFloat(dpi)) {
        TLOGE(WmsLogTag::DMS, "write dpi failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_PRIMARY_DISPLAY_SYSTEM_DPI),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return DMError::DM_OK;
}

DMError ScreenSessionManagerProxy::SetVirtualScreenAutoRotation(ScreenId screenId, bool enable)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    MessageParcel reply;
    MessageParcel data;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(enable)) {
        TLOGE(WmsLogTag::DMS, "Write enable failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_AUTO_ROTATION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

DMError ScreenSessionManagerProxy::SetScreenPrivacyWindowTagSwitch(ScreenId screenId,
    const std::vector<std::string>& privacyWindowTag, bool enable)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }

    MessageParcel reply;
    MessageParcel data;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (!data.WriteStringVector(privacyWindowTag)) {
        TLOGE(WmsLogTag::DMS, "Write privacyWindowTag failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (!data.WriteBool(enable)) {
        TLOGE(WmsLogTag::DMS, "Write enable failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_TAG_SWITCH),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(reply.ReadInt32());
}

void ScreenSessionManagerProxy::NotifySwitchUserAnimationFinish()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return;
    }
    MessageParcel reply;
    MessageParcel data;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_SWITCH_USER_ANIMATION_FINISH),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return;
    }
}

DMError ScreenSessionManagerProxy::SyncScreenPropertyChangedToServer(ScreenId screenId,
    const ScreenProperty& screenProperty)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DMS, "remote is null");
        return DMError::DM_ERROR_IPC_FAILED;
    }
    MessageParcel reply;
    MessageParcel data;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DMS, "WriteInterfaceToken failed");
        return DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(screenId))) {
        TLOGE(WmsLogTag::DMS, "Write screenId failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (!RSMarshallingHelper::Marshalling(data, screenProperty)) {
        TLOGE(WmsLogTag::DMS, "Write screenSession failed");
        return DMError::DM_ERROR_WRITE_DATA_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_PROPERTY_BY_FOLD_STATE_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DMS, "SendRequest failed");
        return DMError::DM_ERROR_IPC_FAILED;
            }
    int32_t ret = -1;
    if (!reply.ReadInt32(ret)) {
        return DMError::DM_ERROR_IPC_FAILED;
    }
    return static_cast<DMError>(ret);
}
} // namespace OHOS::Rosen
