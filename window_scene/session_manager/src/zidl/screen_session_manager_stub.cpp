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
#include <ipc_skeleton.h>
#include "transaction/rs_marshalling_helper.h"

#include "marshalling_helper.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "ScreenSessionManagerStub"};
const static uint32_t MAX_SCREEN_SIZE = 32;
}

int32_t ScreenSessionManagerStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    WLOGFD("OnRemoteRequest code is %{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WLOGFE("InterfaceToken check failed");
        return -1;
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
            auto type = static_cast<DisplayManagerAgentType>(data.ReadUint32());
            DMError ret = RegisterDisplayManagerAgent(agent, type);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_UNREGISTER_DISPLAY_MANAGER_AGENT: {
            auto agent = iface_cast<IDisplayManagerAgent>(data.ReadRemoteObject());
            auto type = static_cast<DisplayManagerAgentType>(data.ReadUint32());
            DMError ret = UnregisterDisplayManagerAgent(agent, type);
            reply.WriteInt32(static_cast<int32_t>(ret));
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
            reply.WriteUint32(static_cast<uint32_t>(state));
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
                return -1;
            }
            reply.WriteUint32(static_cast<uint32_t>(GetScreenPower(dmsScreenId)));
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
            reply.WriteUInt64Vector(allDisplayIds);
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
            reply.WriteInt32(static_cast<int32_t>(ret));
            if (!MarshallingHelper::MarshallingVectorParcelableObj<ScreenInfo>(reply, screenInfos)) {
                WLOGE("fail to marshalling screenInfos in stub.");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_GAMUTS: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            std::vector<ScreenColorGamut> colorGamuts;
            DMError ret = GetScreenSupportedColorGamuts(screenId, colorGamuts);
            reply.WriteInt32(static_cast<int32_t>(ret));
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
                .isForShot_ = isForShot
            };
            ScreenId screenId = CreateVirtualScreen(virScrOption, virtualScreenAgent);
            reply.WriteUint64(static_cast<uint64_t>(screenId));
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
            reply.WriteInt32(static_cast<int32_t>(result));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_BUFFER_ROTATION: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            bool autoRotate = data.ReadBool();
            DMError result = SetVirtualMirrorScreenBufferRotation(screenId, autoRotate);
            reply.WriteInt32(static_cast<int32_t>(result));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_DESTROY_VIRTUAL_SCREEN: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            DMError result = DestroyVirtualScreen(screenId);
            reply.WriteInt32(static_cast<int32_t>(result));
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
            reply.WriteInt32(static_cast<int32_t>(ret));
            reply.WriteUint64(static_cast<uint64_t>(screenGroupId));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_STOP_MIRROR: {
            std::vector<ScreenId> mirrorScreenIds;
            if (!data.ReadUInt64Vector(&mirrorScreenIds)) {
                WLOGE("fail to receive mirror screens in stub.");
                break;
            }
            DMError ret = StopMirror(mirrorScreenIds);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_DISABLE_MIRROR: {
            DMError ret = DisableMirror(data.ReadBool());
            reply.WriteInt32(static_cast<int32_t>(ret));
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
            reply.WriteInt32(static_cast<int32_t>(ret));
            reply.WriteUint64(static_cast<uint64_t>(screenGroupId));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_STOP_EXPAND: {
            std::vector<ScreenId> expandScreenIds;
            if (!data.ReadUInt64Vector(&expandScreenIds)) {
                WLOGE("fail to receive expand screens in stub.");
                break;
            }
            DMError ret = StopExpand(expandScreenIds);
            reply.WriteInt32(static_cast<int32_t>(ret));
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
            std::shared_ptr<Media::PixelMap> displaySnapshot = GetDisplaySnapshot(displayId);
            reply.WriteParcelable(displaySnapshot == nullptr ? nullptr : displaySnapshot.get());
            break;
        }
        case DisplayManagerMessage::TRANS_ID_DISABLE_DISPLAY_SNAPSHOT: {
            DMError ret = DisableDisplaySnapshot(data.ReadBool());
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_ACTIVE_MODE: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            uint32_t modeId = data.ReadUint32();
            DMError ret = SetScreenActiveMode(screenId, modeId);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            float virtualPixelRatio = data.ReadFloat();
            DMError ret = SetVirtualPixelRatio(screenId, virtualPixelRatio);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_RESOLUTION: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            uint32_t width = data.ReadUint32();
            uint32_t height = data.ReadUint32();
            float virtualPixelRatio = data.ReadFloat();
            DMError ret = SetResolution(screenId, width, height, virtualPixelRatio);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_GAMUT: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            ScreenColorGamut colorGamut;
            DMError ret = GetScreenColorGamut(screenId, colorGamut);
            reply.WriteInt32(static_cast<int32_t>(ret));
            if (ret != DMError::DM_OK) {
                break;
            }
            reply.WriteUint32(static_cast<uint32_t>(colorGamut));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_GAMUT: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            int32_t colorGamutIdx = data.ReadInt32();
            DMError ret = SetScreenColorGamut(screenId, colorGamutIdx);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_GAMUT_MAP: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            ScreenGamutMap gamutMap;
            DMError ret = GetScreenGamutMap(screenId, gamutMap);
            reply.WriteInt32(static_cast<int32_t>(ret));
            if (ret != DMError::DM_OK) {
                break;
            }
            reply.WriteInt32(static_cast<uint32_t>(gamutMap));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_SET_GAMUT_MAP: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            ScreenGamutMap gamutMap = static_cast<ScreenGamutMap>(data.ReadUint32());
            DMError ret = SetScreenGamutMap(screenId, gamutMap);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_TRANSFORM: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            DMError ret = SetScreenColorTransform(screenId);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_ORIENTATION: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            Orientation orientation = static_cast<Orientation>(data.ReadUint32());
            DMError ret = SetOrientation(screenId, orientation);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED: {
            bool isLocked = static_cast<bool>(data.ReadBool());
            DMError ret = SetScreenRotationLocked(isLocked);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_IS_SCREEN_ROTATION_LOCKED: {
            bool isLocked = false;
            DMError ret = IsScreenRotationLocked(isLocked);
            reply.WriteInt32(static_cast<int32_t>(ret));
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
            reply.WriteInt32(static_cast<int32_t>(ret));
            reply.WriteBool(hasPrivateWindow);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_HAS_IMMERSIVE_WINDOW: {
            bool immersive = false;
            DMError ret = HasImmersiveWindow(immersive);
            reply.WriteInt32(static_cast<int32_t>(ret));
            reply.WriteBool(immersive);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_DUMP_ALL_SCREEN: {
            std::string dumpInfo;
            DumpAllScreensInfo(dumpInfo);
            reply.WriteString(dumpInfo);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_DUMP_SPECIAL_SCREEN: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            std::string dumpInfo;
            DumpSpecialScreenInfo(screenId, dumpInfo);
            reply.WriteString(dumpInfo);
            break;
        }
        //Fold Screen
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE: {
            FoldDisplayMode displayMode = static_cast<FoldDisplayMode>(data.ReadUint32());
            SetFoldDisplayMode(displayMode);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_LOCK_FOLD_DISPLAY_STATUS: {
            bool lockDisplayStatus = static_cast<bool>(data.ReadUint32());
            LockFoldDisplayStatus(lockDisplayStatus);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE: {
            FoldDisplayMode displayMode = GetFoldDisplayMode();
            reply.WriteUint32(static_cast<uint32_t>(displayMode));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_IS_FOLDABLE: {
            reply.WriteBool(IsFoldable());
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_STATUS: {
            reply.WriteUint32(static_cast<uint32_t>(GetFoldStatus()));
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
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_CLIENT: {
            auto remoteObject = data.ReadRemoteObject();
            auto clientProxy = iface_cast<IScreenSessionManagerClient>(remoteObject);
            if (!clientProxy) {
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
        case DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_ROTATION_PROPERTY: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            RRect bounds;
            if (!RSMarshallingHelper::Unmarshalling(data, bounds)) {
                WLOGFE("Read bounds failed");
                break;
            }
            auto rotation = data.ReadFloat();
            UpdateScreenRotationProperty(screenId, bounds, rotation);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_CURVED_SCREEN_COMPRESSION_AREA: {
            auto area = GetCurvedCompressionArea();
            reply.WriteUint32(area);
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
                return -1;
            }
            NotifyDisplayChangeInfoChanged(info);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_STATE: {
            auto hasPrivate = data.ReadBool();
            SetScreenPrivacyState(hasPrivate);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_RESIZE_VIRTUAL_SCREEN: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            uint32_t width = data.ReadUint32();
            uint32_t height = data.ReadUint32();
            DMError ret = ResizeVirtualScreen(screenId, width, height);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        default:
            WLOGFW("unknown transaction code");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return 0;
}
} // namespace OHOS::Rosen
