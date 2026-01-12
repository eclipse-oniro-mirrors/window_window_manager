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
#include "session_manager/include/scene_session_manager.h"
#include "dms_global_mutex.h"
#include "marshalling_helper.h"

namespace OHOS::Rosen {
namespace {
const static uint32_t MAX_SCREEN_SIZE = 32;
const static int32_t ERR_INVALID_DATA = -1;
const static int32_t MAX_BUFF_SIZE = 100;
const static float INVALID_DEFAULT_DENSITY = 1.0f;
const static uint32_t PIXMAP_VECTOR_SIZE = 2;
constexpr uint32_t  MAX_CREASE_REGION_SIZE = 20;
constexpr uint32_t MAP_SIZE_MAX_NUM = 100;
}

int32_t ScreenSessionManagerStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    DmUtils::HoldLock callbackLock;
    int32_t result = OnRemoteRequestInner(code, data, reply, option);
    return result;
}

int32_t ScreenSessionManagerStub::OnRemoteRequestInner(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    TLOGD(WmsLogTag::DMS, "OnRemoteRequest code is %{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TLOGE(WmsLogTag::DMS, "InterfaceToken check failed");
        return ERR_TRANSACTION_FAILED;
    }
    DisplayManagerMessage msgId = static_cast<DisplayManagerMessage>(code);
    switch (msgId) {
        case DisplayManagerMessage::TRANS_ID_GET_DEFAULT_DISPLAY_INFO: {
            int32_t userId = CONCURRENT_USER_ID_DEFAULT;
            if (!data.ReadInt32(userId)) {
                TLOGD(WmsLogTag::DMS, "Read userId failed");
            }
            auto info = GetDefaultDisplayInfo(userId);
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
                TLOGE(WmsLogTag::DMS, "fail to read dmsScreenId.");
                return ERR_INVALID_DATA;
            }
            static_cast<void>(reply.WriteUint32(static_cast<uint32_t>(GetScreenPower(dmsScreenId))));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER_AUTO: {
            reply.WriteUint32(static_cast<uint32_t>(GetScreenPower()));
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
        case DisplayManagerMessage::TRANS_ID_GET_VISIBLE_AREA_DISPLAY_INFO_BY_ID: {
            DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
            auto info = GetVisibleAreaDisplayInfoById(displayId);
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
            int32_t userId = CONCURRENT_USER_ID_DEFAULT;
            if (!data.ReadInt32(userId)) {
                TLOGD(WmsLogTag::DMS, "Read userId failed");
            }
            TLOGD(WmsLogTag::DMS, "case TRANS_ID_GET_ALL_DISPLAYIDS get userId %{public}u", userId);
            std::vector<DisplayId> allDisplayIds = GetAllDisplayIds(userId);
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
                TLOGE(WmsLogTag::DMS, "fail to marshalling screenInfos in stub.");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_PHYSICAL_SCREEN_IDS: {
            std::vector<ScreenId> screenIds;
            DMError ret  = GetPhysicalScreenIds(screenIds);
            reply.WriteInt32(static_cast<int32_t>(ret));
            bool res = MarshallingHelper::MarshallingVectorObj<ScreenId>(reply, screenIds,
                [](Parcel& parcel, const ScreenId& screenId) {
                    return parcel.WriteUint64(static_cast<uint64_t>(screenId));
                }
            );
            if (!res) {
                TLOGE(WmsLogTag::DMS, "fail to marshalling screenIds in stub.");
                break;
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
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_SCREEN_BRIGHTNESS_INFO: {
            DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
            ScreenBrightnessInfo brightnessInfo;
            DMError ret = GetBrightnessInfo(displayId, brightnessInfo);
            if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
                TLOGE(WmsLogTag::DMS, "write ret failed!");
                break;
            }
            if (ret != DMError::DM_OK) {
                break;
            }
            if (!reply.WriteFloat(brightnessInfo.currentHeadroom)) {
                TLOGE(WmsLogTag::DMS, "write currentHeadroom failed!");
                break;
            }
            if (!reply.WriteFloat(brightnessInfo.maxHeadroom)) {
                TLOGE(WmsLogTag::DMS, "write maxHeadroom failed!");
                break;
            }
            if (!reply.WriteFloat(brightnessInfo.sdrNits)) {
                TLOGE(WmsLogTag::DMS, "write sdrNits failed!");
                break;
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTS_INPUT: {
            DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
            bool supportsInput;
            DMError ret = GetSupportsInput(displayId, supportsInput);
            if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
                TLOGE(WmsLogTag::DMS, "write ret failed!");
                break;
            }
            if (ret != DMError::DM_OK) {
                break;
            }
            if (!reply.WriteFloat(supportsInput)) {
                TLOGE(WmsLogTag::DMS, "write supportsInput failed!");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_SET_SUPPORTS_INPUT: {
            DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
            bool supportsInput = data.ReadBool();
            DMError ret = SetSupportsInput(displayId, supportsInput);
            if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
                TLOGE(WmsLogTag::DMS, "write ret failed!");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_GET_BUNDLE_NAME: {
            DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
            std::string bundleName;
            DMError ret = GetBundleName(displayId, bundleName);
            if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
                TLOGE(WmsLogTag::DMS, "write ret failed!");
                break;
            }
            if (!reply.WriteString(bundleName)) {
                TLOGE(WmsLogTag::DMS, "write bundleName failed!");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_ROUNDED_CORNER: {
            DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
            int radius = 0;
            DMError ret = GetRoundedCorner(displayId, radius);
            if (!reply.WriteInt32(radius)) {
                TLOGE(WmsLogTag::DMS, "write radius failed!");
                ret = DMError::DM_ERROR_IPC_FAILED;
            }
            if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
                TLOGE(WmsLogTag::DMS, "write ret failed!");
            }
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
            VirtualScreenType virtualScreenType = static_cast<VirtualScreenType>(data.ReadUint32());
            bool isSecurity = data.ReadBool();
            VirtualScreenFlag virtualScreenFlag = static_cast<VirtualScreenFlag>(data.ReadUint32());
            bool supportsFocus = data.ReadBool();
            bool supportsInput = data.ReadBool();
            std::string bundleName = data.ReadString();
            std::string serialNumber = data.ReadString();
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
                .missionIds_ = missionIds,
                .virtualScreenType_ = virtualScreenType,
                .isSecurity_ = isSecurity,
                .virtualScreenFlag_ = virtualScreenFlag,
                .supportsFocus_ = supportsFocus,
                .supportsInput_ = supportsInput,
                .bundleName_ = bundleName,
                .serialNumber_ = serialNumber
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
        case DisplayManagerMessage::TRANS_ID_ADD_VIRTUAL_SCREEN_BLOCK_LIST: {
            uint64_t size = 0;
            if (!data.ReadUint64(size)) {
                TLOGE(WmsLogTag::DMS, "Read size failed.");
                reply.WriteInt32(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM));
                break;
            }
            static constexpr uint64_t MAX_SIZE = 100;
            if (size > MAX_SIZE) {
                TLOGE(WmsLogTag::DMS, "Size too large.");
                reply.WriteInt32(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM));
                break;
            }
            std::vector<int32_t> persistentIds;
            for (uint64_t i = 0; i < size; i++) {
                int32_t persistentId = 0;
                if (!data.ReadInt32(persistentId)) {
                    TLOGE(WmsLogTag::DMS, "Read persistentId failed.");
                    break;
                }
                persistentIds.push_back(persistentId);
            }
            DMError errCode = AddVirtualScreenBlockList(persistentIds);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_BLOCK_LIST: {
            uint64_t size = 0;
            if (!data.ReadUint64(size)) {
                TLOGE(WmsLogTag::DMS, "Read size failed.");
                reply.WriteInt32(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM));
                break;
            }
            static constexpr uint64_t MAX_SIZE = 100;
            if (size > MAX_SIZE) {
                TLOGE(WmsLogTag::DMS, "Size too large.");
                reply.WriteInt32(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM));
                break;
            }
            std::vector<int32_t> persistentIds;
            for (uint64_t i = 0; i < size; i++) {
                int32_t persistentId = 0;
                if (!data.ReadInt32(persistentId)) {
                    TLOGE(WmsLogTag::DMS, "Read persistentId failed.");
                    break;
                }
                persistentIds.push_back(persistentId);
            }
            DMError errCode = RemoveVirtualScreenBlockList(persistentIds);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_ADD_VIRTUAL_SCREEN_WHITE_LIST: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            std::vector<uint64_t> missionIds;
            if (!data.ReadUInt64Vector(&missionIds)) {
                TLOGE(WmsLogTag::DMS, "AddWhitelist::fail to receive missionIds from proxy");
                reply.WriteInt32(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM));
                break;
            }
            DMError result = AddVirtualScreenWhiteList(screenId, missionIds);
            reply.WriteUint32(static_cast<uint32_t>(result));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_WHITE_LIST: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            std::vector<uint64_t> missionIds;
            if (!data.ReadUInt64Vector(&missionIds)) {
                TLOGE(WmsLogTag::DMS, "RemoveWhitelist::fail to receive missionIds from proxy");
                reply.WriteInt32(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM));
                break;
            }
            DMError result = RemoveVirtualScreenWhiteList(screenId, missionIds);
            reply.WriteUint32(static_cast<uint32_t>(result));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_IS_ON_BOARD_DISPLAY: {
            DisplayId displayId = data.ReadUint64();
            bool res = IsOnboardDisplay(displayId);
            if (!reply.WriteBool(res)) {
                TLOGE(WmsLogTag::DMS, "write res failed");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_MASKIMAGE: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            std::shared_ptr<Media::PixelMap> privacyMaskImg{nullptr};
            bool isPrivacyMaskImgValid = data.ReadBool();
            if (isPrivacyMaskImgValid) {
                privacyMaskImg = std::shared_ptr<Media::PixelMap>(data.ReadParcelable<Media::PixelMap>());
                DMError result = SetScreenPrivacyMaskImage(screenId, privacyMaskImg);
                reply.WriteInt32(static_cast<int32_t>(result));
            } else {
                DMError result = SetScreenPrivacyMaskImage(screenId, nullptr);
                reply.WriteInt32(static_cast<int32_t>(result));
            }
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
                TLOGE(WmsLogTag::DMS, "fail to receive mirror screen in stub. screen:%{public}" PRIu64"", mainScreenId);
                break;
            }
            ScreenId screenGroupId = INVALID_SCREEN_ID;
            Rotation rotation = static_cast<Rotation>(data.ReadUint32());
            bool needSetRotation = data.ReadBool();
            RotationOption rotationOption = {rotation, needSetRotation};
            DMError ret = MakeMirror(mainScreenId, mirrorScreenId, screenGroupId, rotationOption);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            static_cast<void>(reply.WriteUint64(static_cast<uint64_t>(screenGroupId)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR_FOR_RECORD: {
            std::vector<ScreenId> mainScreenIds;
            if (!data.ReadUInt64Vector(&mainScreenIds)) {
                TLOGE(WmsLogTag::DMS, "fail to receive main screen in stub.");
                break;
            }
            std::vector<ScreenId> mirrorScreenIds;
            if (!data.ReadUInt64Vector(&mirrorScreenIds)) {
                TLOGE(WmsLogTag::DMS, "fail to receive mirror screen in stub.");
                break;
            }
            ScreenId screenGroupId = INVALID_SCREEN_ID;
            DMError ret = MakeMirrorForRecord(mainScreenIds, mirrorScreenIds, screenGroupId);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            static_cast<void>(reply.WriteUint64(static_cast<uint64_t>(screenGroupId)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR_WITH_REGION: {
            ScreenId mainScreenId = static_cast<ScreenId>(data.ReadUint64());
            std::vector<ScreenId> mirrorScreenId;
            if (!data.ReadUInt64Vector(&mirrorScreenId)) {
                TLOGE(WmsLogTag::DMS, "fail to receive mirror screen in stub. screen:%{public}" PRIu64"", mainScreenId);
                break;
            }
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
                TLOGE(WmsLogTag::DMS, "fail to receive mirror screens in stub.");
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
                TLOGE(WmsLogTag::DMS, "fail to receive expand screen in stub.");
                break;
            }
            std::vector<Point> startPoint;
            if (!MarshallingHelper::UnmarshallingVectorObj<Point>(data, startPoint, [](Parcel& parcel, Point& point) {
                    return parcel.ReadInt32(point.posX_) && parcel.ReadInt32(point.posY_);
                })) {
                TLOGE(WmsLogTag::DMS, "fail to receive startPoint in stub.");
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
                TLOGE(WmsLogTag::DMS, "fail to receive expand screens in stub.");
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
                TLOGE(WmsLogTag::DMS, "fail to receive screens in stub.");
                break;
            }
            RemoveVirtualScreenFromGroup(screenId);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT: {
            DisplayId displayId = data.ReadUint64();
            DmErrorCode errCode = DmErrorCode::DM_OK;
            bool isUseDma = data.ReadBool();
            bool isCaptureFullOfScreen = data.ReadBool();
            std::shared_ptr<Media::PixelMap> displaySnapshot = GetDisplaySnapshot(displayId, &errCode, isUseDma,
                isCaptureFullOfScreen);
            reply.WriteParcelable(displaySnapshot == nullptr ? nullptr : displaySnapshot.get());
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(errCode)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HDR_SNAPSHOT: {
            DisplayId displayId = DISPLAY_ID_INVALID;
            bool isUseDma = false;
            bool isCaptureFullOfScreen = false;
            DmErrorCode errCode = DmErrorCode::DM_OK;
            if (!data.ReadUint64(displayId)) {
                TLOGE(WmsLogTag::DMS, "Read displayId failed");
                return ERR_INVALID_DATA;
            }
            if (!data.ReadBool(isUseDma)) {
                TLOGE(WmsLogTag::DMS, "Read isUseDma failed");
                return ERR_INVALID_DATA;
            }
            if (!data.ReadBool(isCaptureFullOfScreen)) {
                TLOGE(WmsLogTag::DMS, "Read isCaptureFullOfScreen failed");
                return ERR_INVALID_DATA;
            }
            std::vector<std::shared_ptr<Media::PixelMap>> displaySnapshotVec = GetDisplayHDRSnapshot(
                displayId, errCode, isUseDma, isCaptureFullOfScreen);
            if (displaySnapshotVec.size() != PIXMAP_VECTOR_SIZE) {
                TLOGE(WmsLogTag::DMS, "Dail to receive displaySnapshotVec in stub.");
                reply.WriteParcelable(nullptr);
                reply.WriteParcelable(nullptr);
            } else {
                TLOGI(WmsLogTag::DMS, "WriteParcelable to receive displaySnapshotVec in stub.");
                reply.WriteParcelable(displaySnapshotVec[0] == nullptr ? nullptr : displaySnapshotVec[0].get());
                reply.WriteParcelable(displaySnapshotVec[1] == nullptr ? nullptr : displaySnapshotVec[1].get());
            }
            reply.WriteInt32(static_cast<int32_t>(errCode));
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
        case DisplayManagerMessage::TRANS_ID_SET_DEFAULT_DENSITY_DPI: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            float virtualPixelRatio = data.ReadFloat();
            DMError ret = SetDefaultDensityDpi(screenId, virtualPixelRatio);
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
            bool isFromNapi = data.ReadBool();
            DMError ret = SetOrientation(screenId, orientation, isFromNapi);
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
        case DisplayManagerMessage::TRANS_ID_GET_CUTOUT_INFO_WITH_ROTATION: {
            DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
            int32_t width = 0;
            if (!data.ReadInt32(width)) {
                TLOGE(WmsLogTag::DMS, "Read width failed");
                return ERR_INVALID_DATA;
            }
            int32_t height = 0;
            if (!data.ReadInt32(height)) {
                TLOGE(WmsLogTag::DMS, "Read height failed");
                return ERR_INVALID_DATA;
            }
            Rotation rotation = static_cast<Rotation>(data.ReadUint32());
            sptr<CutoutInfo> cutoutInfo = GetCutoutInfo(displayId, width, height, rotation);
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
            std::string reason = data.ReadString();
            DMError ret = SetFoldDisplayModeFromJs(displayMode, reason);
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
        case DisplayManagerMessage::TRANS_ID_SET_TARGET_FOLD_STATUS_AND_LOCK: {
            FoldStatus targetFoldStatus = static_cast<FoldStatus>(data.ReadUint32());
            DMError ret = ForceSetFoldStatusAndLock(targetFoldStatus);
            if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
                TLOGE(WmsLogTag::DMS, "Write result failed");
                return ERR_INVALID_DATA;
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_UNLOCK_TARGET_FOLD_STATUS: {
            DMError ret = RestorePhysicalFoldStatus();
            if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
                TLOGE(WmsLogTag::DMS, "Write result failed");
                return ERR_INVALID_DATA;
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_FOLD_STATUS_EXPAND_AND_LOCKED: {
            bool lockDisplayStatus = static_cast<bool>(data.ReadUint32());
            SetFoldStatusExpandAndLocked(lockDisplayStatus);
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
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_SUPER_FOLD_STATUS: {
            static_cast<void>(reply.WriteUint32(static_cast<uint32_t>(GetSuperFoldStatus())));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_SUPER_ROTATION: {
            static_cast<void>(reply.WriteFloat(GetSuperRotation()));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_LANDSCAPE_LOCK_STATUS: {
            bool isLocked = data.ReadBool();
            SetLandscapeLockStatus(isLocked);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_EXTEND_SCREEN_CONNECT_STATUS: {
            static_cast<void>(reply.WriteUint32(static_cast<uint32_t>(GetExtendScreenConnectStatus())));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_CURRENT_FOLD_CREASE_REGION: {
            reply.WriteStrongParcelable(GetCurrentFoldCreaseRegion());
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_LIVE_CREASE_REGION: {
            FoldCreaseRegion region;
            DMError ret = GetLiveCreaseRegion(region);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            if (ret != DMError::DM_OK) {
                break;
            }
            static_cast<void>(reply.WriteUint64(region.GetDisplayId()));
            const auto& creaseRects = region.GetCreaseRects();
            uint32_t size = static_cast<uint32_t>(creaseRects.size());
            if (size > MAX_CREASE_REGION_SIZE) {
                TLOGE(WmsLogTag::DMS, "CreaseRects size exceeds max limit");
                break;
            }
            static_cast<void>(reply.WriteUint32(size));
            for (const auto& rect : creaseRects) {
                static_cast<void>(reply.WriteInt32(rect.posX_));
                static_cast<void>(reply.WriteInt32(rect.posY_));
                static_cast<void>(reply.WriteUint32(rect.width_));
                static_cast<void>(reply.WriteUint32(rect.height_));
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN: {
            std::vector<ScreenId> uniqueScreenIds;
            uint32_t size = data.ReadUint32();
            if (size > MAX_SCREEN_SIZE) {
                TLOGE(WmsLogTag::DMS, "screenIds size is bigger than %{public}u", MAX_SCREEN_SIZE);
                break;
            }
            if (!data.ReadUInt64Vector(&uniqueScreenIds)) {
                TLOGE(WmsLogTag::DMS, "failed to receive unique screens in stub");
                break;
            }
            std::vector<DisplayId> displayIds;
            UniqueScreenRotationOptions rotationOptions;
            if (!data.ReadBool(rotationOptions.isRotationLocked_)) {
                TLOGE(WmsLogTag::DMS, "failed to receive rotationOptions isRotationLocked in stub");
                break;
            }
            if (!data.ReadInt32(rotationOptions.rotation_)) {
                TLOGE(WmsLogTag::DMS, "failed to receive rotationOptions rotation in stub");
                break;
            }
            TLOGD(WmsLogTag::DMS,
                  "IPC Stub received unique screen lock parameters, isRotationLocked: %{public}d, rotation: %{public}d",
                  rotationOptions.isRotationLocked_, rotationOptions.rotation_);
            DMError ret = MakeUniqueScreen(uniqueScreenIds, displayIds, rotationOptions);
            reply.WriteUInt64Vector(displayIds);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_CLIENT: {
            auto remoteObject = data.ReadRemoteObject();
            auto clientProxy = iface_cast<IScreenSessionManagerClient>(remoteObject);
            if (clientProxy == nullptr) {
                TLOGE(WmsLogTag::DMS, "clientProxy is null");
                break;
            }
            SetClient(clientProxy);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_SCREEN_PROPERTY: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            if (!RSMarshallingHelper::Marshalling(reply, GetScreenProperty(screenId))) {
                TLOGE(WmsLogTag::DMS, "Write screenProperty failed");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_NODE: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            auto displayNode = GetDisplayNode(screenId);
            if (!displayNode || !displayNode->Marshalling(reply)) {
                TLOGE(WmsLogTag::DMS, "Write displayNode failed");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_SCREEN_COMBINATION: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            auto screenCombination = GetScreenCombination(screenId);
            reply.WriteUint32(static_cast<uint32_t>(screenCombination));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_DIRECTION_INFO: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            auto screenComponentRotation = data.ReadFloat();
            auto rotation = data.ReadFloat();
            auto phyRotation = data.ReadFloat();
            auto screenPropertyChangeType = static_cast<ScreenPropertyChangeType>(data.ReadUint32());
            RRect bounds;
            if (!RSMarshallingHelper::Unmarshalling(data, bounds)) {
                TLOGE(WmsLogTag::DMS, "Read bounds failed");
                break;
            }
            ScreenDirectionInfo directionInfo;
            directionInfo.screenRotation_ = screenComponentRotation;
            directionInfo.rotation_ = rotation;
            directionInfo.phyRotation_ = phyRotation;
            UpdateScreenDirectionInfo(screenId, directionInfo, screenPropertyChangeType, bounds);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_ROTATION_PROPERTY: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            RRect bounds;
            if (!RSMarshallingHelper::Unmarshalling(data, bounds)) {
                TLOGE(WmsLogTag::DMS, "Read bounds failed");
                break;
            }
            auto rotation = data.ReadFloat();
            auto screenPropertyChangeType = static_cast<ScreenPropertyChangeType>(data.ReadUint32());
            bool isSwitchUser = data.ReadBool();
            UpdateScreenRotationProperty(screenId, bounds, rotation, screenPropertyChangeType, isSwitchUser);
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
                TLOGE(WmsLogTag::DMS, "Write screenProperty failed");
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_CHANGE_INFO: {
            sptr<DisplayChangeInfo> info = DisplayChangeInfo::Unmarshalling(data);
            if (!info) {
                TLOGE(WmsLogTag::DMS, "Read DisplayChangeInfo failed");
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
            std::unordered_map<DisplayId, bool> privacyBundleDisplayId;
            uint32_t mapSize = 0;
            if (!data.ReadUint32(mapSize) || mapSize > MAP_SIZE_MAX_NUM) {
                TLOGE(WmsLogTag::DMS, "Failed to read mapSize");
                return ERR_INVALID_DATA;
            }
            for (uint32_t i = 0; i < mapSize; i++) {
                DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
                auto hasPrivate = data.ReadBool();
                privacyBundleDisplayId[displayId] = hasPrivate;
            }
            SetPrivacyStateByDisplayId(privacyBundleDisplayId);
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
        case DisplayManagerMessage::TRANS_ID_UPDATE_SUPER_FOLD_AVAILABLE_AREA: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            int32_t bPosX = data.ReadInt32();
            int32_t bPosY = data.ReadInt32();
            uint32_t bWidth = data.ReadUint32();
            uint32_t bHeight = data.ReadUint32();
            DMRect bArea = {bPosX, bPosY, bWidth, bHeight};
            int32_t cPosX = data.ReadInt32();
            int32_t cPosY = data.ReadInt32();
            uint32_t cWidth = data.ReadUint32();
            uint32_t cHeight = data.ReadUint32();
            DMRect cArea = {cPosX, cPosY, cWidth, cHeight};
            UpdateSuperFoldAvailableArea(screenId, bArea, cArea);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_UPDATE_SUPER_FOLD_EXPAND_AVAILABLE_AREA: {
            auto screenId = static_cast<ScreenId>(data.ReadUint64());
            int32_t posX = data.ReadInt32();
            int32_t posY = data.ReadInt32();
            uint32_t width = data.ReadUint32();
            uint32_t height = data.ReadUint32();
            DMRect area = {posX, posY, width, height};
            UpdateSuperFoldExpandAvailableArea(screenId, area);
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
        case DisplayManagerMessage::TRANS_ID_GET_EXPAND_AVAILABLE_AREA: {
            DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
            DMRect area;
            DMError ret = GetExpandAvailableArea(displayId, area);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            static_cast<void>(reply.WriteInt32(area.posX_));
            static_cast<void>(reply.WriteInt32(area.posY_));
            static_cast<void>(reply.WriteUint32(area.width_));
            static_cast<void>(reply.WriteUint32(area.height_));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_FOLD_TO_EXPAND_COMPLETION: {
            bool foldToExpand = data.ReadBool();
            NotifyFoldToExpandCompletion(foldToExpand);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_SCREEN_CONNECT_COMPLETION: {
            ScreenId screenId = SCREEN_ID_INVALID;
            if (!data.ReadUint64(screenId)) {
                TLOGE(WmsLogTag::DMS, "Read screenId failed");
                return ERR_INVALID_DATA;
            }
            NotifyScreenConnectCompletion(static_cast<ScreenId>(screenId));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_AOD_OP_COMPLETION: {
            uint32_t op;
            int32_t result;
            if (!data.ReadUint32(op)) {
                TLOGE(WmsLogTag::DMS, "Read aod operation failed");
                return ERR_INVALID_DATA;
            }
            if (!data.ReadInt32(result)) {
                TLOGE(WmsLogTag::DMS, "Read aod result failed");
                return ERR_INVALID_DATA;
            }
            NotifyAodOpCompletion(static_cast<AodOP>(op), result);
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
            std::vector<uint64_t> surfaceIdList;
            if (!data.ReadUInt64Vector(&surfaceIdList)) {
                TLOGE(WmsLogTag::DMS, "Failed to receive surfaceIdList in stub");
                break;
            }
            std::vector<uint8_t> typeBlackList;
            if (!data.ReadUInt8Vector(&typeBlackList)) {
                TLOGE(WmsLogTag::DMS, "Failed to receive typeBlackList in stub");
                break;
            }
            SetVirtualScreenBlackList(screenId, windowIdList, surfaceIdList, typeBlackList);
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
            hookInfo.displayOrientation_ = data.ReadUint32();
            hookInfo.enableHookDisplayOrientation_ = data.ReadBool();
            hookInfo.isFullScreenInForceSplit_ = data.ReadBool();
            UpdateDisplayHookInfo(uid, enable, hookInfo);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HOOK_INFO: {
            int32_t uid = data.ReadInt32();
            DMHookInfo hookInfo;
            GetDisplayHookInfo(uid, hookInfo);
            if (!reply.WriteUint32(hookInfo.width_) || !reply.WriteUint32(hookInfo.height_) ||
                !reply.WriteFloat(hookInfo.density_) || !reply.WriteUint32(hookInfo.rotation_) ||
                !reply.WriteBool(hookInfo.enableHookRotation_)) {
                TLOGE(WmsLogTag::DMS, "read reply hookInfo failed!");
            }
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
        case DisplayManagerMessage::TRANS_ID_GET_PRIMARY_DISPLAY_ID: {
            auto info = GetPrimaryDisplayId();
            reply.WriteUint64(info);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION: {
            ProcGetDisplaySnapshotWithOption(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HDR_SNAPSHOT_WITH_OPTION: {
            ProcGetDisplayHDRSnapshotWithOption(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_CAMERA_STATUS: {
            int32_t cameraStatus = data.ReadInt32();
            int32_t cameraPosition = data.ReadInt32();
            SetCameraStatus(cameraStatus, cameraPosition);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_ON_DELAY_TIME: {
            int32_t delay = data.ReadInt32();
            int32_t ret = SetScreenOnDelayTime(delay);
            static_cast<void>(reply.WriteInt32(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_SKIP_PROTECTED_WINDOW: {
            ProcSetScreenSkipProtectedWindow(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_RECORD_EVENT_FROM_SCB: {
            std::string description = data.ReadString();
            bool needRecordEvent = data.ReadBool();
            RecordEventFromScb(description, needRecordEvent);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_IS_REAL_SCREEN: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            reply.WriteBool(GetIsRealScreen(screenId));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPABILITY: {
            std::string capabilitInfo;
            DMError ret = GetDisplayCapability(capabilitInfo);
            static_cast<void>(reply.WriteInt32(static_cast<int32_t>(ret)));
            reply.WriteString(capabilitInfo);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SYSTEM_KEYBOARD_STATUS: {
            bool isTpKeyboardOn = static_cast<bool>(data.ReadBool());
            DMError ret = SetSystemKeyboardStatus(isTpKeyboardOn);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_DISPLAY_MUTE_FLAG: {
            ScreenId screenId = static_cast<ScreenId>(data.ReadUint64());
            bool muteFlag = data.ReadBool();
            SetVirtualDisplayMuteFlag(screenId, muteFlag);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SCENE_BOARD_FORCE_CLOSE_HDR: {
            ScreenId screenId = SCREEN_ID_INVALID;
            if (!data.ReadUint64(screenId)) {
                TLOGE(WmsLogTag::DMS, "Read screenId failed");
                return ERR_INVALID_DATA;
            }
            bool isForceCloseHdr = false;
            if (!data.ReadBool(isForceCloseHdr)) {
                TLOGE(WmsLogTag::DMS, "Read isForceCloseHdr failed");
                return ERR_INVALID_DATA;
            }
            SetForceCloseHdr(screenId, isForceCloseHdr);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_DEFAULT_MODE_WHEN_SWITCH_USER: {
            SetDefaultMultiScreenModeWhenSwitchUser();
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_EXTEND_SCREEN_CREATE_FINISH: {
            NotifyExtendScreenCreateFinish();
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_EXTEND_SCREEN_DESTROY_FINISH: {
            NotifyExtendScreenDestroyFinish();
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_SCREEN_MASK_APPEAR: {
            NotifyScreenMaskAppear();
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_KEYBOARD_STATE: {
            bool isKeyboardOn = GetKeyboardState();
            reply.WriteBool(isKeyboardOn);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_GET_SCREEN_AREA_OF_DISPLAY_AREA: {
            ProcGetScreenAreaOfDisplayArea(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_PRIMARY_DISPLAY_SYSTEM_DPI: {
            ProcSetPrimaryDisplaySystemDpi(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_AUTO_ROTATION: {
            ProcSetVirtualScreenAutoRotation(data, reply);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_TAG_SWITCH: {
            ScreenId screenId = SCREEN_ID_INVALID;
            if (!data.ReadUint64(screenId)) {
                TLOGE(WmsLogTag::DMS, "Read screenId failed");
                return ERR_INVALID_DATA;
            }
            std::vector<std::string> privacyWindowTag;
            if (!data.ReadStringVector(&privacyWindowTag)) {
                TLOGE(WmsLogTag::DMS, "Read privacyWindowTag failed");
                return ERR_INVALID_DATA;
            }
            bool enable = false;
            if (!data.ReadBool(enable)) {
                TLOGE(WmsLogTag::DMS, "Read enable failed");
                return ERR_INVALID_DATA;
            }
            DMError ret = SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTag, enable);
            if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
                TLOGE(WmsLogTag::DMS, "Write reault failed");
                return ERR_INVALID_DATA;
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SYNCHRONIZED_POWER_STATUS: {
            uint32_t stateTemp = 0;
            if (!data.ReadUint32(stateTemp)) {
                TLOGE(WmsLogTag::DMS, "Read state failed");
                return ERR_INVALID_DATA;
            }
            ScreenPowerState state = static_cast<ScreenPowerState>(stateTemp);
            bool res = SynchronizePowerStatus(state);
            if (!reply.WriteBool(res)) {
                TLOGE(WmsLogTag::DMS, "Write res failed");
                return ERR_INVALID_DATA;
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_SWITCH_USER_ANIMATION_FINISH: {
            NotifySwitchUserAnimationFinish();
            break;
        }
        case DisplayManagerMessage::TRANS_ID_NOTIFY_IS_FULL_SCREEN_IN_FORCE_SPLIT: {
            int32_t uid = 0;
            if (!data.ReadInt32(uid)) {
                TLOGE(WmsLogTag::DMS, "Read uid failed");
                return ERR_INVALID_DATA;
            }
            bool isFullScreen = false;
            if (!data.ReadBool(isFullScreen)) {
                TLOGE(WmsLogTag::DMS, "Read isFullScreen failed");
                return ERR_INVALID_DATA;
            }
            NotifyIsFullScreenInForceSplitMode(uid, isFullScreen);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_PROPERTY_BY_FOLD_STATE_CHANGE: {
            ScreenId screenId = SCREEN_ID_INVALID;
            if (!data.ReadUint64(screenId)) {
                TLOGE(WmsLogTag::DMS, "Read screenId failed");
                return ERR_INVALID_DATA;
            }
            ScreenProperty screenProperty;
            if (!RSMarshallingHelper::Unmarshalling(data, screenProperty)) {
                TLOGE(WmsLogTag::DMS, "read screenSession failed");
                return ERR_INVALID_DATA;
            }
            DMError ret = SyncScreenPropertyChangedToServer(screenId, screenProperty);
            if (!reply.WriteInt32(static_cast<int32_t>(ret))) {
                TLOGE(WmsLogTag::DMS, "write result failed");
                return ERR_INVALID_DATA;
            }
            break;
        }
        case DisplayManagerMessage::TRANS_ID_SYNC_SCREEN_POWER_STATE: {
            uint32_t ret = 0;
            if (!data.ReadUint32(ret)) {
                TLOGE(WmsLogTag::DMS, "Read power state failed");
                return ERR_INVALID_DATA;
            }
            ScreenPowerState state = static_cast<ScreenPowerState>(ret);
            SyncScreenPowerState(state);
            break;
        }
        case DisplayManagerMessage::TRANS_ID_REGISTER_DISPLAY_ATTRIBUTE_AGENT: {
            TLOGI(WmsLogTag::DMS, "called");
            sptr<IRemoteObject> remoteObj = data.ReadRemoteObject();
            auto agent = iface_cast<IDisplayManagerAgent>(remoteObj);
            if (agent == nullptr) {
                return ERR_INVALID_DATA;
            }
            std::vector<std::string> attributes;
            if (!data.ReadStringVector(&attributes)) {
                TLOGE(WmsLogTag::DMS, "Read attributes failed");
                return ERR_INVALID_DATA;
            }
 
            DMError ret = RegisterDisplayAttributeAgent(attributes, agent);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        case DisplayManagerMessage::TRANS_ID_UNREGISTER_DISPLAY_ATTRIBUTE: {
            TLOGI(WmsLogTag::DMS, "called");
            sptr<IRemoteObject> remoteObj = data.ReadRemoteObject();
            auto agent = iface_cast<IDisplayManagerAgent>(remoteObj);
            if (agent == nullptr) {
                return ERR_INVALID_DATA;
            }
            std::vector<std::string> attributes;
            if (!data.ReadStringVector(&attributes)) {
                TLOGE(WmsLogTag::DMS, "Read attributes failed");
                return ERR_INVALID_DATA;
            }
 
            DMError ret = UnRegisterDisplayAttribute(attributes, agent);
            reply.WriteInt32(static_cast<int32_t>(ret));
            break;
        }
        default:
            TLOGW(WmsLogTag::DMS, "unknown transaction code");
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
        TLOGE(WmsLogTag::DMS, "pid List size invalid: %{public}d", size);
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
        TLOGE(WmsLogTag::DMS, "write physical size error");
        return;
    }
    for (const auto &physicalItem : physicalInfos) {
        writeRet = reply.WriteUint32(static_cast<uint32_t>(physicalItem.foldDisplayMode_));
        if (!writeRet) {
            TLOGE(WmsLogTag::DMS, "write display mode error");
            break;
        }
        writeRet = reply.WriteUint32(physicalItem.physicalWidth_);
        if (!writeRet) {
            TLOGE(WmsLogTag::DMS, "write physical width error");
            break;
        }
        writeRet = reply.WriteUint32(physicalItem.physicalHeight_);
        if (!writeRet) {
            TLOGE(WmsLogTag::DMS, "write physical height error");
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
    if (!data.ReadUint64(option.displayId_)) {
        TLOGE(WmsLogTag::DMS, "Read displayId failed");
        return;
    }
    if (!data.ReadBool(option.isNeedNotify_)) {
        TLOGE(WmsLogTag::DMS, "Read isNeedNotify failed");
        return;
    }
    if (!data.ReadBool(option.isNeedPointer_)) {
        TLOGE(WmsLogTag::DMS, "Read isCaptureFullOfScreen failed");
        return;
    }
    if (!data.ReadUInt64Vector(&option.blackWindowIdList_)) {
        TLOGE(WmsLogTag::DMS, "Read node blackWindowIdList failed");
        return;
    }
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
    option.isCaptureFullOfScreen_ = static_cast<bool>(data.ReadBool());
    if (!data.ReadUInt64Vector(&option.surfaceNodesList_)) {
        TLOGE(WmsLogTag::DMS, "Read node surfaceNodesList failed");
        return;
    }
    if (!data.ReadFloat(option.scaleX_) || !data.ReadFloat(option.scaleY_)) {
        TLOGE(WmsLogTag::DMS, "Read scale failed");
        return;
    }
    if (!data.ReadInt32(option.rect.posX_) || !data.ReadInt32(option.rect.posY_) ||
        !data.ReadUint32(option.rect.width_) || !data.ReadUint32(option.rect.height_)) {
        TLOGE(WmsLogTag::DMS, "Read rect failed");
        return;
    }
    DmErrorCode errCode = DmErrorCode::DM_OK;
    std::shared_ptr<Media::PixelMap> capture = GetDisplaySnapshotWithOption(option, &errCode);
    reply.WriteParcelable(capture == nullptr ? nullptr : capture.get());
    static_cast<void>(reply.WriteInt32(static_cast<int32_t>(errCode)));
}

void ScreenSessionManagerStub::ProcGetDisplayHDRSnapshotWithOption(MessageParcel& data, MessageParcel& reply)
{
    CaptureOption option;
    option.displayId_ = DISPLAY_ID_INVALID;
    option.isNeedNotify_ = false;
    option.isCaptureFullOfScreen_ = false;
    if (!data.ReadUint64(option.displayId_)) {
        TLOGE(WmsLogTag::DMS, "Read displayId failed");
        return;
    }
    if (!data.ReadBool(option.isNeedNotify_)) {
        TLOGE(WmsLogTag::DMS, "Read isNeedNotify failed");
        return;
    }
    if (!data.ReadBool(option.isCaptureFullOfScreen_)) {
        TLOGE(WmsLogTag::DMS, "Read isCaptureFullOfScreen failed");
        return;
    }
    if (!data.ReadUInt64Vector(&option.surfaceNodesList_)) {
        TLOGE(WmsLogTag::DMS, "Read surfaceNodesList failed");
        return;
    }
    DmErrorCode errCode = DmErrorCode::DM_OK;
    std::vector<std::shared_ptr<Media::PixelMap>> captureVec = GetDisplayHDRSnapshotWithOption(option, errCode);
    if (captureVec.size() != PIXMAP_VECTOR_SIZE) {
        TLOGE(WmsLogTag::DMS, "captureVec size: %{public}zu", captureVec.size());
        reply.WriteParcelable(nullptr);
        reply.WriteParcelable(nullptr);
    } else {
        reply.WriteParcelable(captureVec[0] == nullptr ? nullptr : captureVec[0].get());
        reply.WriteParcelable(captureVec[1] == nullptr ? nullptr : captureVec[1].get());
    }
    static_cast<void>(reply.WriteInt32(static_cast<int32_t>(errCode)));
}

void ScreenSessionManagerStub::ProcSetScreenSkipProtectedWindow(MessageParcel& data, MessageParcel& reply)
{
    std::vector<ScreenId> screenIds;
    if (!data.ReadUInt64Vector(&screenIds)) {
        TLOGE(WmsLogTag::DMS, "Read screenIds failed");
        return;
    }
    bool isEnable = static_cast<bool>(data.ReadBool());
    DMError ret = SetScreenSkipProtectedWindow(screenIds, isEnable);
    reply.WriteInt32(static_cast<int32_t>(ret));
}

void ScreenSessionManagerStub::ProcGetScreenAreaOfDisplayArea(MessageParcel& data, MessageParcel& reply)
{
    DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
    int32_t posX = data.ReadInt32();
    int32_t posY = data.ReadInt32();
    uint32_t width = data.ReadUint32();
    uint32_t height = data.ReadUint32();
    DMRect displayArea = { posX, posY, width, height };
    DMRect screenArea = DMRect::NONE();
    ScreenId screenId = 0;
    DMError ret = GetScreenAreaOfDisplayArea(displayId, displayArea, screenId, screenArea);
    reply.WriteInt32(static_cast<int32_t>(ret));
    static_cast<void>(reply.WriteUint64(screenId));
    static_cast<void>(reply.WriteInt32(screenArea.posX_));
    static_cast<void>(reply.WriteInt32(screenArea.posY_));
    static_cast<void>(reply.WriteUint32(screenArea.width_));
    static_cast<void>(reply.WriteUint32(screenArea.height_));
}

void ScreenSessionManagerStub::ProcSetPrimaryDisplaySystemDpi(MessageParcel& data, MessageParcel& reply)
{
    float dpi = INVALID_DEFAULT_DENSITY;
    if (!data.ReadFloat(dpi)) {
        TLOGE(WmsLogTag::DMS, "Read dpi failed.");
        return;
    }
    DMError ret = SetPrimaryDisplaySystemDpi(dpi);
    reply.WriteInt32(static_cast<int32_t>(ret));
}

void ScreenSessionManagerStub::ProcSetVirtualScreenAutoRotation(MessageParcel& data, MessageParcel& reply)
{
    ScreenId screenId = 0;
    if (!data.ReadUint64(screenId)) {
        TLOGE(WmsLogTag::DMS, "Read screenId failed.");
        return;
    }
    bool enable = false;
    if (!data.ReadBool(enable)) {
        TLOGE(WmsLogTag::DMS, "Read enable failed.");
        return;
    }
    DMError ret = SetVirtualScreenAutoRotation(screenId, enable);
    reply.WriteInt32(static_cast<int32_t>(ret));
}
} // namespace OHOS::Rosen
