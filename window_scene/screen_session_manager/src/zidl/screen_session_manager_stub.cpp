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

#include "zidl/screen_session_manager_stub.h"

#include "common/rs_rect.h"
#include "dm_common.h"
#include <ipc_skeleton.h>
#include "transaction/rs_marshalling_helper.h"

#include "marshalling_helper.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_DISPLAY, "ScreenSessionManagerStub" };
const static uint32_t MAX_SCREEN_SIZE = 32;
const static int32_t ERR_INVALID_DATA = -1;
const static int32_t MAX_BUFF_SIZE = 100;
}

int32_t ScreenSessionManagerStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    WLOGFD("OnRemoteRequest code is %{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WLOGFE("InterfaceToken check failed");
        return ERR_TRANSACTION_FAILED;
    }
    DisplayManagerMessage msgId = static_cast<DisplayManagerMessage>(code);
    switch (msgId) {
        case DisplayManagerMessage::TRANS_ID_GET_DEFAULT_DISPLAY_INFO: {
            auto info = GetDefaultDisplayInfo();
            reply.WriteParcelable(info);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_REGISTER_DISPLAY_MANAGER_AGENT: {
            auto agent = iface_cast<IDisplayManagerAgent>(data.ReadRemoteObject());
            if (agent == nullptr) {
                return ERR_INVALID_DATA;
            }
            auto type = static_cast<DisplayManagerAgentType>(data.ReadUint32());
            DMError ret = RegisterDisplayManagerAgent(agent, type);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_UNREGISTER_DISPLAY_MANAGER_AGENT: {
            auto agent = iface_cast<IDisplayManagerAgent>(data.ReadRemoteObject());
            if (agent == nullptr) {
                return ERR_INVALID_DATA;
            }
            auto type = static_cast<DisplayManagerAgentType>(data.ReadUint32());
            DMError ret = UnregisterDisplayManagerAgent(agent, type);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_WAKE_UP_BEGIN: {
            PowerStateChangeReason reason = static_cast<PowerStateChangeReason>(data.ReadUint32());
            reply.WriteBool(WakeUpBegin(reason));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_WAKE_UP_END: {
            reply.WriteBool(WakeUpEnd());
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SUSPEND_BEGIN: {
            PowerStateChangeReason reason = static_cast<PowerStateChangeReason>(data.ReadUint32());
            reply.WriteBool(SuspendBegin(reason));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SUSPEND_END: {
            reply.WriteBool(SuspendEnd());
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_INTERNAL_SCREEN_ID: {
            reply.WriteUint64(GetInternalScreenId());
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_POWER_BY_ID: {
            ScreenId screenId = data.ReadUint64();
            ScreenPowerState state = static_cast<ScreenPowerState>(data.ReadUint32());
            PowerStateChangeReason reason = static_cast<PowerStateChangeReason>(data.ReadUint32());
            reply.WriteBool(SetScreenPowerById(screenId, state, reason));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_DISPLAY_STATE: {
            DisplayState state = static_cast<DisplayState>(data.ReadUint32());
            reply.WriteBool(SetDisplayState(state));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SPECIFIED_SCREEN_POWER: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint32());
            ScreenPowerState state = static_cast<ScreenPowerState>(data.ReadUint32());
            PowerStateChangeReason reason = static_cast<PowerStateChangeReason>(data.ReadUint32());
            reply.WriteBool(SetSpecifiedScreenPower(screenId, state, reason));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_POWER_FOR_ALL: {
            ScreenPowerState state = static_cast<ScreenPowerState>(data.ReadUint32());
            PowerStateChangeReason reason = static_cast<PowerStateChangeReason>(data.ReadUint32());
            reply.WriteBool(SetScreenPowerForAll(state, reason));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_STATE: {
            DisplayState state = GetDisplayState(data.ReadUint64());
            static_cast<void>(reply.WriteUint32(static_cast<uint32_t>(state)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_EVENT: {
            DisplayEvent event = static_cast<DisplayEvent>(data.ReadUint32());
            NotifyDisplayEvent(event);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER: {
            ScreenId dmsScreenId;
            if (!data.ReadUint64(dmsScreenId)) {
                WLOGFE("fail to read dmsScreenId.");
                return ERR_INVALID_DATA;
            }
            static_cast<void>(reply.WriteUint32(static_cast<uint32_t>(GetScreenPower(dmsScreenId))));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_TRY_TO_CANCEL_SCREEN_OFF: {
            reply.WriteBool(TryToCancelScreenOff());
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_BRIGHTNESS: {
            uint64_t screenId = data.ReadUint64();
            uint32_t level = data.ReadUint64();
            reply.WriteBool(SetScreenBrightness(screenId, level));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_SCREEN_BRIGHTNESS: {
            uint64_t screenId = data.ReadUint64();
            reply.WriteUint32(GetScreenBrightness(screenId));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_BY_ID: {
            DisplayId displayId = data.ReadUint64();
            auto info = GetDisplayInfoById(displayId);
            reply.WriteParcelable(info);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_BY_SCREEN: {
            ScreenId screenId = data.ReadUint64();
            auto info = GetDisplayInfoByScreen(screenId);
            reply.WriteParcelable(info);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_ALL_DISPLAYIDS: {
            std::vector<DisplayId> allDisplayIds = GetAllDisplayIds();
            static_cast<void>(reply.WriteUInt64Vector(allDisplayIds));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_SCREEN_INFO_BY_ID: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            auto screenInfo = GetScreenInfoById(screenId);
            reply.WriteStrongParcelable(screenInfo);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_ALL_SCREEN_INFOS: {
            std::vector<sptr<ScreenInfo>> screenInfos;
            DMError ret  = GetAllScreenInfos(screenInfos);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            if (!MarshallingHelper::MarshallingVectorParcelableObj<ScreenInfo>(reply, screenInfos)) {
                WLOGE("fail to marshalling screenInfos in stub.");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_GAMUTS: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            std::vector<ScreenColorGamut> colorGamuts;
            DMError ret = GetScreenSupportedColorGamuts(screenId, colorGamuts);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            if (ret != DMError::DM_OK) {
                break;
            }
            MarshallingHelper::MarshallingVectorObj<ScreenColorGamut>(reply, colorGamuts,
                [](Parcel& parcel, const ScreenColorGamut& color) {
                    return parcel.WriteUint32(static_cast<uint32_t>(color));
                }
            );
            break;
        }
        case DisplayManagerMessage::TRANS_ID_CREATE_VIRTUAL_SCREEN: {
            std::string name = data.ReadString();
            uint32_t width = data.ReadUint32();
            uint32_t height = data.ReadUint32();
            float density = data.ReadFloat();
            int32_t flags = data.ReadInt32();
            bool isForShot = data.ReadBool();
            std::vector<uint64_t> missionIds;
            data.ReadUInt64Vector(&missionIds);
            bool isSurfaceValid = data.ReadBool();
            sptr<Surface> surface = nullptr;
            if (isSurfaceValid) {
                sptr<IRemoteObject> surfaceObject = data.ReadRemoteObject();
                sptr<IBufferProducer> bp = iface_cast<IBufferProducer>(surfaceObject);
                surface = Surface::CreateSurfaceAsProducer(bp);
            }
            sptr<IRemoteObject> virtualScreenAgent = data.ReadRemoteObject();
            VirtualScreenOption virScrOption = {
                .name_ = name,
                .width_ = width,
                .height_ = height,
                .density_ = density,
                .surface_ = surface,
                .flags_ = flags,
                .isForShot_ = isForShot,
                .missionIds_ = missionIds
            };
            ScreenId screenId = CreateVirtualScreen(virScrOption, virtualScreenAgent);
            static_cast<void>(reply.WriteUint64(static_cast<uint64_t>(screenId)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SURFACE: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            bool isSurfaceValid = data.ReadBool();
            sptr<IBufferProducer> bp = nullptr;
            if (isSurfaceValid) {
                sptr<IRemoteObject> surfaceObject = data.ReadRemoteObject();
                bp = iface_cast<IBufferProducer>(surfaceObject);
            }
            DMError result = SetVirtualScreenSurface(screenId, bp);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(result)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_CANVAS_ROTATION: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            bool autoRotate = data.ReadBool();
            DMError result = SetVirtualMirrorScreenCanvasRotation(screenId, autoRotate);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(result)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SCALE_MODE: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            ScreenScaleMode scaleMode = static_cast<ScreenScaleMode>(data.ReadUint32());
            DMError result = SetVirtualMirrorScreenScaleMode(screenId, scaleMode);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(result)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_DESTROY_VIRTUAL_SCREEN: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            DMError result = DestroyVirtualScreen(screenId);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(result)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR: {
            ScreenId mainScreenId = static_cast<ScreenId>(data.ReadUint64());
            std::vector<ScreenId> mirrorScreenId;
            if (!data.ReadUInt64Vector(&mirrorScreenId)) {
                WLOGE("fail to receive mirror screen in stub. screen:%{public}" PRIu64"", mainScreenId);
                break;
            }
            ScreenId screenGroupId = INVALID_SCREEN_ID;
            DMError ret = MakeMirror(mainScreenId, mirrorScreenId, screenGroupId);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            static_cast<void>(reply.WriteUint64(static_cast<uint64_t>(screenGroupId)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR_WITH_REGION: {
            ScreenId mainScreenId = static_cast<ScreenId>(data.ReadUint64());
            ScreenId mirrorScreenId = static_cast<ScreenId>(data.ReadUint64());
            int32_t posX = data.ReadInt32();
            int32_t posY = data.ReadInt32();
            uint32_t width = data.ReadUint32();
            uint32_t height = data.ReadUint32();
            DMRect mainScreenRegion = { posX, posY, width, height };
            ScreenId screenGroupId = INVALID_SCREEN_ID;
            DMError ret = MakeMirror(mainScreenId, mirrorScreenId, mainScreenRegion, screenGroupId);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            static_cast<void>(reply.WriteUint64(static_cast<uint64_t>(screenGroupId)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_MULTI_SCREEN_MODE_SWITCH: {
            ScreenId mainScreenId = static_cast<ScreenId>(data.ReadUint64());
            ScreenId secondaryScreenId = static_cast<ScreenId>(data.ReadUint64());
            MultiScreenMode screenMode = static_cast<MultiScreenMode>(data.ReadUint32());
            DMError ret = SetMultiScreenMode(mainScreenId, secondaryScreenId, screenMode);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_MULTI_SCREEN_POSITION: {
            uint64_t mainScreenId = data.ReadUint64();
            uint32_t mainScreenX = data.ReadUint32();
            uint32_t mainScreenY = data.ReadUint32();
            uint64_t secondaryScreenId = data.ReadUint64();
            uint32_t secondaryScreenX = data.ReadUint32();
            uint32_t secondaryScreenY = data.ReadUint32();
            MultiScreenPositionOptions mainScreenOptions = {
                .screenId_ = mainScreenId,
                .startX_ = mainScreenX,
                .startY_ = mainScreenY,
            };
            MultiScreenPositionOptions secondScreenOption = {
                .screenId_ = secondaryScreenId,
                .startX_ = secondaryScreenX,
                .startY_ = secondaryScreenY,
            };
            DMError ret = SetMultiScreenRelativePosition(mainScreenOptions, secondScreenOption);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_STOP_MIRROR: {
            std::vector<ScreenId> mirrorScreenIds;
            if (!data.ReadUInt64Vector(&mirrorScreenIds)) {
                WLOGE("fail to receive mirror screens in stub.");
                break;
            }
            DMError ret = StopMirror(mirrorScreenIds);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_DISABLE_MIRROR: {
            DMError ret = DisableMirror(data.ReadBool());
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_EXPAND: {
            std::vector<ScreenId> screenId;
            if (!data.ReadUInt64Vector(&screenId)) {
                WLOGE("fail to receive expand screen in stub.");
                break;
            }
            std::vector<Point> startPoint;
            if (!MarshallingHelper::UnmarshallingVectorObj<Point>(data, startPoint, [](Parcel& parcel, Point& point) {
                    return parcel.ReadInt32(point.posX_) && parcel.ReadInt32(point.posY_);
                })) {
                WLOGE("fail to receive startPoint in stub.");
                break;
            }
            ScreenId screenGroupId = INVALID_SCREEN_ID;
            DMError ret = MakeExpand(screenId, startPoint, screenGroupId);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            static_cast<void>(reply.WriteUint64(static_cast<uint64_t>(screenGroupId)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_STOP_EXPAND: {
            std::vector<ScreenId> expandScreenIds;
            if (!data.ReadUInt64Vector(&expandScreenIds)) {
                WLOGE("fail to receive expand screens in stub.");
                break;
            }
            DMError ret = StopExpand(expandScreenIds);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_SCREEN_GROUP_INFO_BY_ID: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            auto screenGroupInfo = GetScreenGroupInfoById(screenId);
            reply.WriteStrongParcelable(screenGroupInfo);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_FROM_SCREEN_GROUP: {
            std::vector<ScreenId> screenId;
            if (!data.ReadUInt64Vector(&screenId)) {
                WLOGE("fail to receive screens in stub.");
                break;
            }
            RemoveVirtualScreenFromGroup(screenId);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT: {
            DisplayId displayId = data.ReadUint64();
            DmErrorCode errCode = DmErrorCode::DM_OK;
            std::shared_ptr<Media::PixelMap> displaySnapshot = GetDisplaySnapshot(displayId, &errCode);
            reply.WriteParcelable(displaySnapshot == nullptr ? nullptr : displaySnapshot.get());
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(errCode)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_SNAPSHOT_BY_PICKER: {
            ProcGetSnapshotByPicker(reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_ACTIVE_MODE: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            uint32_t modeId = data.ReadUint32();
            DMError ret = SetScreenActiveMode(screenId, modeId);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            float virtualPixelRatio = data.ReadFloat();
            DMError ret = SetVirtualPixelRatio(screenId, virtualPixelRatio);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            float virtualPixelRatio = data.ReadFloat();
            DMError ret = SetVirtualPixelRatioSystem(screenId, virtualPixelRatio);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_RESOLUTION: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            uint32_t width = data.ReadUint32();
            uint32_t height = data.ReadUint32();
            float virtualPixelRatio = data.ReadFloat();
            DMError ret = SetResolution(screenId, width, height, virtualPixelRatio);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DENSITY_IN_CURRENT_RESOLUTION: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            float virtualPixelRatio;
            DMError ret = GetDensityInCurResolution(screenId, virtualPixelRatio);
            reply.WriteFloat(virtualPixelRatio);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_GAMUT: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            ScreenColorGamut colorGamut;
            DMError ret = GetScreenColorGamut(screenId, colorGamut);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            if (ret != DMError::DM_OK) {
                break;
            }
            static_cast<void>(reply.WriteUint32(static_cast<uint32_t>(colorGamut)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_GAMUT: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            int32_t colorGamutIdx = data.ReadInt32();
            DMError ret = SetScreenColorGamut(screenId, colorGamutIdx);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_GAMUT_MAP: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            ScreenGamutMap gamutMap;
            DMError ret = GetScreenGamutMap(screenId, gamutMap);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            if (ret != DMError::DM_OK) {
                break;
            }
            static_cast<void>(reply.WriteInt32(static_cast<uint32_t>(gamutMap)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_SET_GAMUT_MAP: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            ScreenGamutMap gamutMap = static_cast<ScreenGamutMap>(data.ReadUint32());
            DMError ret = SetScreenGamutMap(screenId, gamutMap);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_TRANSFORM: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            DMError ret = SetScreenColorTransform(screenId);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_PIXEL_FORMAT: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            GraphicPixelFormat pixelFormat;
            DMError ret = GetPixelFormat(screenId, pixelFormat);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            if (ret != DMError::DM_OK) {
                break;
            }
            static_cast<void>(reply.WriteInt32(static_cast<uint32_t>(pixelFormat)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_SET_PIXEL_FORMAT: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            GraphicPixelFormat pixelFormat = static_cast<GraphicPixelFormat>(data.ReadUint32());
            DMError ret = SetPixelFormat(screenId, pixelFormat);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_HDR_FORMAT: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            std::vector<ScreenHDRFormat> hdrFormats;
            DMError ret = GetSupportedHDRFormats(screenId, hdrFormats);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            if (ret != DMError::DM_OK) {
                break;
            }
            MarshallingHelper::MarshallingVectorObj<ScreenHDRFormat>(reply, hdrFormats,
                [](Parcel& parcel, const ScreenHDRFormat& hdrFormat) {
                    return parcel.WriteUint32(static_cast<uint32_t>(hdrFormat));
                }
            );
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_HDR_FORMAT: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            ScreenHDRFormat hdrFormat;
            DMError ret = GetScreenHDRFormat(screenId, hdrFormat);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            if (ret != DMError::DM_OK) {
                break;
            }
            static_cast<void>(reply.WriteInt32(static_cast<uint32_t>(hdrFormat)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_SET_HDR_FORMAT: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            int32_t modeIdx = data.ReadInt32();
            DMError ret = SetScreenHDRFormat(screenId, modeIdx);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_SPACE: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            std::vector<GraphicCM_ColorSpaceType> colorSpaces;
            DMError ret = GetSupportedColorSpaces(screenId, colorSpaces);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            if (ret != DMError::DM_OK) {
                break;
            }
            MarshallingHelper::MarshallingVectorObj<GraphicCM_ColorSpaceType>(reply, colorSpaces,
                [](Parcel& parcel, const GraphicCM_ColorSpaceType& color) {
                    return parcel.WriteUint32(static_cast<uint32_t>(color));
                }
            );
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_SPACE: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            GraphicCM_ColorSpaceType colorSpace;
            DMError ret = GetScreenColorSpace(screenId, colorSpace);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            if (ret != DMError::DM_OK) {
                break;
            }
            static_cast<void>(reply.WriteInt32(static_cast<uint32_t>(colorSpace)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_SPACE: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            GraphicCM_ColorSpaceType colorSpace = static_cast<GraphicCM_ColorSpaceType>(data.ReadUint32());
            DMError ret = SetScreenColorSpace(screenId, colorSpace);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_ORIENTATION: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            Orientation orientation = static_cast<Orientation>(data.ReadUint32());
            DMError ret = SetOrientation(screenId, orientation);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED: {
            bool isLocked = static_cast<bool>(data.ReadBool());
            DMError ret = SetScreenRotationLocked(isLocked);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED_FROM_JS: {
            bool isLocked = static_cast<bool>(data.ReadBool());
            DMError ret = SetScreenRotationLockedFromJs(isLocked);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_IS_SCREEN_ROTATION_LOCKED: {
            bool isLocked = false;
            DMError ret = IsScreenRotationLocked(isLocked);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            reply.WriteBool(isLocked);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_CUTOUT_INFO: {
            DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
            sptr<CutoutInfo> cutoutInfo = GetCutoutInfo(displayId);
            reply.WriteParcelable(cutoutInfo);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_HAS_PRIVATE_WINDOW: {
            DisplayId id = static_cast<DisplayId>(data.ReadUint64());
            bool hasPrivateWindow = false;
            DMError ret = HasPrivateWindow(id, hasPrivateWindow);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            reply.WriteBool(hasPrivateWindow);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_CONVERT_SCREENID_TO_RSSCREENID: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            ScreenId rsId = SCREEN_ID_INVALID;
            bool ret = ConvertScreenIdToRsScreenId(screenId, rsId);
            reply.WriteBool(ret);
            static_cast<void>(reply.WriteUint64(rsId));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_HAS_IMMERSIVE_WINDOW: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            bool immersive = false;
            DMError ret = HasImmersiveWindow(screenId, immersive);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            reply.WriteBool(immersive);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_DUMP_ALL_SCREEN: {
            std::string dumpInfo;
            DumpAllScreensInfo(dumpInfo);
            static_cast<void>(reply.WriteString(dumpInfo));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_DUMP_SPECIAL_SCREEN: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            std::string dumpInfo;
            DumpSpecialScreenInfo(screenId, dumpInfo);
            static_cast<void>(reply.WriteString(dumpInfo));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_DEVICE_IS_CAPTURE: {
            reply.WriteBool(IsCaptured());
            break;
        }
        //Fold Screen
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE: {
            FoldDisplayMode displayMode = static_cast<FoldDisplayMode>(data.ReadUint32());
            SetFoldDisplayMode(displayMode);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_FOLD_DISPLAY_MODE_FROM_JS: {
            FoldDisplayMode displayMode = static_cast<FoldDisplayMode>(data.ReadUint32());
            DMError ret = SetFoldDisplayModeFromJs(displayMode);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_LOCK_FOLD_DISPLAY_STATUS: {
            bool lockDisplayStatus = static_cast<bool>(data.ReadUint32());
            SetFoldStatusLocked(lockDisplayStatus);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_LOCK_FOLD_DISPLAY_STATUS_FROM_JS: {
            bool lockDisplayStatus = static_cast<bool>(data.ReadUint32());
            DMError ret = SetFoldStatusLockedFromJs(lockDisplayStatus);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_DISPLAY_SCALE: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            auto scaleX = data.ReadFloat();
            auto scaleY = data.ReadFloat();
            auto pivotX = data.ReadFloat();
            auto pivotY = data.ReadFloat();
            SetDisplayScale(screenId, scaleX, scaleY, pivotX, pivotY);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE: {
            FoldDisplayMode displayMode = GetFoldDisplayMode();
            static_cast<void>(reply.WriteUint32(static_cast<uint32_t>(displayMode)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_IS_FOLDABLE: {
            reply.WriteBool(IsFoldable());
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_STATUS: {
            static_cast<void>(reply.WriteUint32(static_cast<uint32_t>(GetFoldStatus())));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_CURRENT_FOLD_CREASE_REGION: {
            reply.WriteStrongParcelable(GetCurrentFoldCreaseRegion());
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN: {
            std::vector<ScreenId> uniqueScreenIds;
            uint32_t size = data.ReadUint32();
            if (size > MAX_SCREEN_SIZE) {
                WLOGFE("screenIds size is bigger than %{public}u", MAX_SCREEN_SIZE);
                break;
            }
            if (!data.ReadUInt64Vector(&uniqueScreenIds)) {
                WLOGFE("failed to receive unique screens in stub");
                break;
            }
            DMError ret = MakeUniqueScreen(uniqueScreenIds);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_CLIENT: {
            auto remoteObject = data.ReadRemoteObject();
            auto clientProxy = iface_cast<IScreenSessionManagerClient>(remoteObject);
            if (clientProxy == nullptr) {
                WLOGFE("clientProxy is null");
                break;
            }
            SetClient(clientProxy);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_SCREEN_PROPERTY: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            if (!RSMarshallingHelper::Marshalling(reply, GetScreenProperty(screenId))) {
                WLOGFE("Write screenProperty failed");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_NODE: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            auto displayNode = GetDisplayNode(screenId);
            if (!displayNode || !displayNode->Marshalling(reply)) {
                WLOGFE("Write displayNode failed");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_DIRECTION_INFO: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            auto screenComponentRotation = data.ReadFloat();
            auto rotation = data.ReadFloat();
            UpdateScreenDirectionInfo(screenId, screenComponentRotation, rotation);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_ROTATION_PROPERTY: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            RRect bounds;
            if (!RSMarshallingHelper::Unmarshalling(data, bounds)) {
                WLOGFE("Read bounds failed");
                break;
            }
            auto rotation = data.ReadFloat();
            auto screenPropertyChangeType = static_cast<ScreenPropertyChangeType>(data.ReadUint32());
            UpdateScreenRotationProperty(screenId, bounds, rotation, screenPropertyChangeType);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_CURVED_SCREEN_COMPRESSION_AREA: {
            auto area = GetCurvedCompressionArea();
            static_cast<void>(reply.WriteUint32(area));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_PHY_SCREEN_PROPERTY: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            if (!RSMarshallingHelper::Marshalling(reply, GetPhyScreenProperty(screenId))) {
                WLOGFE("Write screenProperty failed");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_CHANGE_INFO: {
            sptr<DisplayChangeInfo> info = DisplayChangeInfo::Unmarshalling(data);
            if (!info) {
                WLOGFE("Read DisplayChangeInfo failed");
                return ERR_INVALID_DATA;
            }
            NotifyDisplayChangeInfoChanged(info);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_STATE: {
            auto hasPrivate = data.ReadBool();
            SetScreenPrivacyState(hasPrivate);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREENID_PRIVACY_STATE: {
            DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
            auto hasPrivate = data.ReadBool();
            SetPrivacyStateByDisplayId(displayId, hasPrivate);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_LIST: {
            DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
            std::vector<std::string> privacyWindowList;
            data.ReadStringVector(&privacyWindowList);
            SetScreenPrivacyWindowList(displayId, privacyWindowList);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_RESIZE_VIRTUAL_SCREEN: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            uint32_t width = data.ReadUint32();
            uint32_t height = data.ReadUint32();
            DMError ret = ResizeVirtualScreen(screenId, width, height);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_UPDATE_AVAILABLE_AREA: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            int32_t posX = data.ReadInt32();
            int32_t posY = data.ReadInt32();
            uint32_t width = data.ReadUint32();
            uint32_t height = data.ReadUint32();
            DMRect area = {posX, posY, width, height};
            UpdateAvailableArea(screenId, area);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_OFF_DELAY_TIME: {
            int32_t delay = data.ReadInt32();
            int32_t ret = SetScreenOffDelayTime(delay);
            static_cast<void>(reply.WriteInt32(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_AVAILABLE_AREA: {
            ProcGetAvailableArea(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_FOLD_TO_EXPAND_COMPLETION: {
            bool foldToExpand = data.ReadBool();
            NotifyFoldToExpandCompletion(foldToExpand);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_VIRTUAL_SCREEN_FLAG: {
            ProcGetVirtualScreenFlag(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_FLAG: {
            ProcSetVirtualScreenFlag(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DEVICE_SCREEN_CONFIG: {
            if (!RSMarshallingHelper::Marshalling(reply, GetDeviceScreenConfig())) {
                TLOGE(WmsLogTag::DMS, "Write deviceScreenConfig failed");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_REFRESH_RATE: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            uint32_t refreshInterval = data.ReadUint32();
            DMError ret = SetVirtualScreenRefreshRate(screenId, refreshInterval);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SWITCH_USER: {
            SwitchUser();
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_BLACK_LIST: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            std::vector<uint64_t> windowIdList;
            if (!data.ReadUInt64Vector(&windowIdList)) {
                TLOGE(WmsLogTag::DMS, "Failed to receive windowIdList in stub");
                break;
            }
            SetVirtualScreenBlackList(screenId, windowIdList);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_DISABLE_POWEROFF_RENDER_CONTROL: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            DisablePowerOffRenderControl(screenId);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_PROXY_FOR_FREEZE: {
            ProcProxyForFreeze(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_RESET_ALL_FREEZE_STATUS: {
            DMError ret = ResetAllFreezeStatus();
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_HOOK_INFO: {
            int32_t uid = data.ReadInt32();
            bool enable = data.ReadBool();
            DMHookInfo hookInfo;
            hookInfo.width_ = data.ReadUint32();
            hookInfo.height_ = data.ReadUint32();
            hookInfo.density_ = data.ReadFloat();
            hookInfo.rotation_ = data.ReadUint32();
            hookInfo.enableHookRotation_ = data.ReadBool();
            UpdateDisplayHookInfo(uid, enable, hookInfo);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_ALL_PHYSICAL_DISPLAY_RESOLUTION: {
            ProcGetAllDisplayPhysicalResolution(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_STATUS: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            VirtualScreenStatus screenStatus = static_cast<VirtualScreenStatus>(data.ReadInt32());
            bool res = SetVirtualScreenStatus(screenId, screenStatus);
            reply.WriteBool(res);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SECURITY_EXEMPTION: {
            ProcSetVirtualScreenSecurityExemption(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_MAX_REFRESHRATE: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            uint32_t refreshRate = data.ReadUint32();
            uint32_t actualRefreshRate;
            DMError ret = SetVirtualScreenMaxRefreshRate(screenId, refreshRate, actualRefreshRate);
            reply.WriteUint32(actualRefreshRate);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPTURE: {
            ProcGetScreenCapture(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_PRIMARY_DISPLAY_INFO: {
            auto info = GetPrimaryDisplayInfo();
            reply.WriteParcelable(info);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION: {
            ProcGetDisplaySnapshotWithOption(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_CAMERA_STATUS: {
            int32_t cameraStatus = data.ReadInt32();
            int32_t cameraPosition = data.ReadInt32();
            SetCameraStatus(cameraStatus, cameraPosition);
            break;
        }
        default:
            WLOGFW("unknown transaction code");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_NONE;
}

void ScreenSessionManagerStub::ProcGetAvailableArea(MessageParcel& data, MessageParcel& reply)
{
    DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
    DMRect area;
    DMError ret = GetAvailableArea(displayId, area);
    static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
    static_cast<void>(reply.WriteInt32(area.posX_));
    static_cast<void>(reply.WriteInt32(area.posY_));
    static_cast<void>(reply.WriteUint32(area.width_));
    static_cast<void>(reply.WriteUint32(area.height_));
}

void ScreenSessionManagerStub::ProcGetSnapshotByPicker(MessageParcel& reply)
{
    DmErrorCode errCode = DmErrorCode::DM_OK;
    Media::Rect imgRect { 0, 0, 0, 0 };
    std::shared_ptr<Media::PixelMap> snapshot = GetSnapshotByPicker(imgRect, &errCode);
    reply.WriteParcelable(snapshot == nullptr ? nullptr : snapshot.get());
    static_cast<void>(reply.WriteInt32(static_cast<int32_t>(errCode)));
    static_cast<void>(reply.WriteInt32(imgRect.left));
    static_cast<void>(reply.WriteInt32(imgRect.top));
    static_cast<void>(reply.WriteInt32(imgRect.width));
    static_cast<void>(reply.WriteInt32(imgRect.height));
}

void ScreenSessionManagerStub::ProcSetVirtualScreenFlag(MessageParcel& data, MessageParcel& reply)
{
    ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
    VirtualScreenFlag screenFlag = static_cast<VirtualScreenFlag>(data.ReadUint32());
    DMError setRet = SetVirtualScreenFlag(screenId, screenFlag);
    static_cast<void>(reply.WriteInt32(static_cast<int32_t>(setRet)));
}

void ScreenSessionManagerStub::ProcGetVirtualScreenFlag(MessageParcel& data, MessageParcel& reply)
{
    ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
    VirtualScreenFlag screenFlag = GetVirtualScreenFlag(screenId);
    static_cast<void>(reply.WriteUint32(static_cast<uint32_t>(screenFlag)));
}

void ScreenSessionManagerStub::ProcProxyForFreeze(MessageParcel& data, MessageParcel& reply)
{
    std::set<int32_t> pidList;
    int32_t size = data.ReadInt32();
    if (size > MAX_BUFF_SIZE) {
        WLOGFE("pid List size invalid: %{public}d", size);
        size = MAX_BUFF_SIZE;
    }
    for (int32_t i = 0; i < size; i++) {
        pidList.insert(data.ReadInt32());
    }
    bool isProxy = data.ReadBool();
    DMError ret = ProxyForFreeze(pidList, isProxy);
    static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
}

void ScreenSessionManagerStub::ProcGetAllDisplayPhysicalResolution(MessageParcel& data, MessageParcel& reply)
{
    auto physicalInfos = GetAllDisplayPhysicalResolution();
    size_t infoSize = physicalInfos.size();
    bool writeRet = reply.WriteInt32(static_cast<int32_t>(infoSize));
    if (!writeRet) {
        WLOGFE("write physical size error");
        return;
    }
    for (const auto &physicalItem : physicalInfos) {
        writeRet = reply.WriteUint32(static_cast<uint32_t>(physicalItem.foldDisplayMode_));
        if (!writeRet) {
            WLOGFE("write display mode error");
            break;
        }
        writeRet = reply.WriteUint32(physicalItem.physicalWidth_);
        if (!writeRet) {
            WLOGFE("write physical width error");
            break;
        }
        writeRet = reply.WriteUint32(physicalItem.physicalHeight_);
        if (!writeRet) {
            WLOGFE("write physical height error");
            break;
        }
    }
}

void ScreenSessionManagerStub::ProcSetVirtualScreenSecurityExemption(MessageParcel& data, MessageParcel& reply)
{
    ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
    uint32_t pid = data.ReadUint32();
    std::vector<uint64_t> windowIdList;
    data.ReadUInt64Vector(&windowIdList);
    DMError ret = SetVirtualScreenSecurityExemption(screenId, pid, windowIdList);
    static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
}

void ScreenSessionManagerStub::ProcGetScreenCapture(MessageParcel& data, MessageParcel& reply)
{
    CaptureOption option;
    option.displayId_ = static_cast<DisplayId>(data.ReadUint64());
    option.isNeedNotify_ = static_cast<bool>(data.ReadBool());
    option.isNeedPointer_ = static_cast<bool>(data.ReadBool());
    DmErrorCode errCode = DmErrorCode::DM_OK;
    std::shared_ptr<Media::PixelMap> capture = GetScreenCapture(option, &errCode);
    reply.WriteParcelable(capture == nullptr ? nullptr : capture.get());
    static_cast<void>(reply.WriteInt32(static_cast<int32_t>(errCode)));
}

void ScreenSessionManagerStub::ProcGetDisplaySnapshotWithOption(MessageParcel& data, MessageParcel& reply)
{
    CaptureOption option;
    option.displayId_ = static_cast<DisplayId>(data.ReadUint64());
    option.isNeedNotify_ = static_cast<bool>(data.ReadBool());
    option.isNeedPointer_ = static_cast<bool>(data.ReadBool());
    DmErrorCode errCode = DmErrorCode::DM_OK;
    std::shared_ptr<Media::PixelMap> capture = GetDisplaySnapshotWithOption(option, &errCode);
    reply.WriteParcelable(capture == nullptr ? nullptr : capture.get());
    static_cast<void>(reply.WriteInt32(static_cast<int32_t>(errCode)));
}
} // namespace OHOS::Rosen
