/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "screensessionmgrstub_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include <array>
#include <parcel.h>
#include <iremote_stub.h>
#include "message_option.h"
#include "message_parcel.h"
#include "screen_session_manager_stub.h"
#include <fuzzer/FuzzedDataProvider.h>
#include "fuzz_util.h"
using namespace OHOS::Rosen;

namespace OHOS {
constexpr size_t DATA_MIN_SIZE = 2;
const size_t MAX_BUFFER_SIZE = 18;
const int32_t MAX_ROTATION_VALUE = 270;
const int32_t MAX_SCALE_MODE_VALUE = 3;
constexpr uint32_t MAP_SIZE_MAX_NUM = 100;
constexpr size_t MAX_LENGTH_STRING = 64;
constexpr int32_t MIN_POSITION = -10000;
constexpr int32_t MAX_POSITION = 10000;
constexpr uint32_t MAX_RESOLUTION = 8192;
const int BYTES_PER_VALUE = 8;
static constexpr std::array<uint32_t, 181> kValidTransIds = {{
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DEFAULT_DISPLAY_INFO),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_BY_ID),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_VISIBLE_AREA_DISPLAY_INFO_BY_ID),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_BY_SCREEN),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HDR_SNAPSHOT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REGISTER_DISPLAY_MANAGER_AGENT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UNREGISTER_DISPLAY_MANAGER_AGENT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_WAKE_UP_BEGIN),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_WAKE_UP_END),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SUSPEND_BEGIN),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SUSPEND_END),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_INTERNAL_SCREEN_ID),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_POWER_BY_ID),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SPECIFIED_SCREEN_POWER),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_POWER_FOR_ALL),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_DISPLAY_STATE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_STATE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_DISPLAYIDS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_EVENT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_TRY_TO_CANCEL_SCREEN_OFF),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_BRIGHTNESS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_BRIGHTNESS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_FREEZE_EVENT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_BASE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_CREATE_VIRTUAL_SCREEN),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_DESTROY_VIRTUAL_SCREEN),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SURFACE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_ADD_VIRTUAL_SCREEN_BLOCK_LIST),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_BLOCK_LIST),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_ADD_VIRTUAL_SCREEN_WHITE_LIST),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_WHITE_LIST),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_CANVAS_ROTATION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_IS_ON_BOARD_DISPLAY),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_INFO_BY_ID),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_GROUP_INFO_BY_ID),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ACTIVE_MODE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_SCREEN_INFOS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PHYSICAL_SCREEN_IDS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_ORIENTATION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_DEFAULT_DENSITY_DPI),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_RESOLUTION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DENSITY_IN_CURRENT_RESOLUTION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_MASKIMAGE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_AS_DEFAULT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREENGROUP_BASE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR_FOR_RECORD),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR_WITH_REGION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_MULTI_SCREEN_MODE_SWITCH),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_MULTI_SCREEN_POSITION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_EXPAND),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_FROM_SCREEN_GROUP),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GAMUT_BASE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_GAMUTS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_GAMUT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_GAMUT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_GAMUT_MAP),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_GAMUT_MAP),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_TRANSFORM),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_PIXEL_FORMAT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_PIXEL_FORMAT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_HDR_FORMAT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_HDR_FORMAT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_HDR_FORMAT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_SPACE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_SPACE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_SPACE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_IS_SCREEN_ROTATION_LOCKED),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED_FROM_JS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_HAS_PRIVATE_WINDOW),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_CUTOUT_INFO),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_HAS_IMMERSIVE_WINDOW),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_ADD_SURFACE_NODE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REMOVE_SURFACE_NODE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_STOP_MIRROR),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_STOP_EXPAND),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_DISABLE_MIRROR),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_CUTOUT_INFO_WITH_ROTATION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SCREEN_BRIGHTNESS_INFO),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTS_INPUT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_SUPPORTS_INPUT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_BUNDLE_NAME),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ROUNDED_CORNER),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SCREEN_BASE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_DUMP_ALL_SCREEN),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_DUMP_SPECIAL_SCREEN),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_FOLD_DISPLAY_MODE_FROM_JS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_DISPLAY_SCALE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_IS_FOLDABLE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_STATUS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_SUPER_FOLD_STATUS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_CURRENT_FOLD_CREASE_REGION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_LIVE_CREASE_REGION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_LANDSCAPE_LOCK_STATUS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_LOCK_FOLD_DISPLAY_STATUS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_LOCK_FOLD_DISPLAY_STATUS_FROM_JS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_TARGET_FOLD_STATUS_AND_LOCK),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UNLOCK_TARGET_FOLD_STATUS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_FORCE_CLOSE_HDR),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_SUPER_ROTATION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_CLIENT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_PROPERTY),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_NODE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_DIRECTION_INFO),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_ROTATION_PROPERTY),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_AVAILABLE_AREA),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SUPER_FOLD_AVAILABLE_AREA),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_OFF_DELAY_TIME),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_CURVED_SCREEN_COMPRESSION_AREA),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PHY_SCREEN_PROPERTY),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_CHANGE_INFO),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_STATE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREENID_PRIVACY_STATE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_LIST),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_RESIZE_VIRTUAL_SCREEN),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_AVAILABLE_AREA),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_FOLD_TO_EXPAND_COMPLETION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_CONVERT_SCREENID_TO_RSSCREENID),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_VIRTUAL_SCREEN_FLAG),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_FLAG),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SCALE_MODE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DEVICE_SCREEN_CONFIG),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_REFRESH_RATE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_DEVICE_IS_CAPTURE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SNAPSHOT_BY_PICKER),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SWITCH_USER),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_BLACK_LIST),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_DISABLE_POWEROFF_RENDER_CONTROL),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_PROXY_FOR_FREEZE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_RESET_ALL_FREEZE_STATUS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_HOOK_INFO),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HOOK_INFO),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_PHYSICAL_DISPLAY_RESOLUTION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_STATUS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SECURITY_EXEMPTION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_MAX_REFRESHRATE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPTURE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PRIMARY_DISPLAY_INFO),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PRIMARY_DISPLAY_ID),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HDR_SNAPSHOT_WITH_OPTION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_CAMERA_STATUS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ON_DELAY_TIME),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_COMBINATION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER_AUTO),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_SKIP_PROTECTED_WINDOW),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SUPER_FOLD_EXPAND_AVAILABLE_AREA),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_EXPAND_AVAILABLE_AREA),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_RECORD_EVENT_FROM_SCB),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_IS_REAL_SCREEN),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_EXTEND_SCREEN_CONNECT_STATUS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPABILITY),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SYSTEM_KEYBOARD_STATUS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_DISPLAY_MUTE_FLAG),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_DEFAULT_MODE_WHEN_SWITCH_USER),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_EXTEND_SCREEN_CREATE_FINISH),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_EXTEND_SCREEN_DESTROY_FINISH),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_FOLD_STATUS_EXPAND_AND_LOCKED),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_SCREEN_MASK_APPEAR),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_KEYBOARD_STATE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_AREA_OF_DISPLAY_AREA),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_PRIMARY_DISPLAY_SYSTEM_DPI),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_AUTO_ROTATION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SYNCHRONIZED_POWER_STATUS),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_TAG_SWITCH),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_SCREEN_CONNECT_COMPLETION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_SWITCH_USER_ANIMATION_FINISH),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_IS_FULL_SCREEN_IN_FORCE_SPLIT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_PROPERTY_BY_FOLD_STATE_CHANGE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_AOD_OP_COMPLETION),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SYNC_SCREEN_POWER_STATE),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REGISTER_DISPLAY_ATTRIBUTE_AGENT),
    static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UNREGISTER_DISPLAY_ATTRIBUTE),
}};

sptr<IRemoteObject> GetRemote()
{
    return new CameraStandard::MockIRemoteObject();
}

void ScreenSessionManagerStubFuzzTestEnhanced(FuzzedDataProvider& fdp)
{
    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();
    sptr<IRemoteObject> remote = GetRemote();
    MessageParcel parcel;
    MessageOption option;
    MessageParcel reply;
    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    size_t index = fdp.ConsumeIntegralInRange<size_t>(0, kValidTransIds.size() - 1);
    uint32_t code = kValidTransIds[index];
    std::cout << "code: " << code << std::endl;
    switch (code) {
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DEFAULT_DISPLAY_INFO): {
            int32_t userId = fdp.ConsumeIntegral<int32_t>();
            parcel.WriteInt32(userId);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REGISTER_DISPLAY_MANAGER_AGENT):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UNREGISTER_DISPLAY_MANAGER_AGENT): {
            parcel.WriteRemoteObject(remote);
            uint32_t type = fdp.ConsumeIntegral<uint32_t>();
            parcel.WriteUint32(type);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SUSPEND_BEGIN):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_WAKE_UP_BEGIN): {
            uint32_t rawReason = fdp.ConsumeIntegralInRange<uint32_t>(
                static_cast<uint32_t>(PowerStateChangeReason::POWER_BUTTON),
                static_cast<uint32_t>(PowerStateChangeReason::STATE_CHANGE_REASON_UNKNOWN)
            );
            parcel.WriteUint32(rawReason);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_INTERNAL_SCREEN_ID):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SUSPEND_END):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_WAKE_UP_END):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER_AUTO):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_TRY_TO_CANCEL_SCREEN_OFF):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_SCREEN_INFOS):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PHYSICAL_SCREEN_IDS):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SNAPSHOT_BY_PICKER):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_IS_SCREEN_ROTATION_LOCKED):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_DUMP_ALL_SCREEN):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_DEVICE_IS_CAPTURE):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UNLOCK_TARGET_FOLD_STATUS):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_IS_FOLDABLE):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_STATUS):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_SUPER_FOLD_STATUS):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_SUPER_ROTATION):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_EXTEND_SCREEN_CONNECT_STATUS):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_CURRENT_FOLD_CREASE_REGION):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_LIVE_CREASE_REGION):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_AVAILABLE_AREA):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_VIRTUAL_SCREEN_FLAG):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_FLAG):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DEVICE_SCREEN_CONFIG):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SWITCH_USER):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_PROXY_FOR_FREEZE):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_RESET_ALL_FREEZE_STATUS):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_PHYSICAL_DISPLAY_RESOLUTION):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPTURE):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PRIMARY_DISPLAY_INFO):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PRIMARY_DISPLAY_ID):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPABILITY):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_DEFAULT_MODE_WHEN_SWITCH_USER):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_EXTEND_SCREEN_CREATE_FINISH):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_EXTEND_SCREEN_DESTROY_FINISH):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_SCREEN_MASK_APPEAR):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_KEYBOARD_STATE):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_AREA_OF_DISPLAY_AREA):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_PRIMARY_DISPLAY_SYSTEM_DPI):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_AUTO_ROTATION):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_SWITCH_USER_ANIMATION_FINISH): {
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_POWER_BY_ID): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t stateRaw = fdp.ConsumeIntegralInRange<uint32_t>(
                static_cast<uint32_t>(ScreenPowerState::POWER_ON),
                static_cast<uint32_t>(ScreenPowerState::POWER_DOZE_SUSPEND)
            );
            uint32_t reasonRaw = fdp.ConsumeIntegralInRange<uint32_t>(
                static_cast<uint32_t>(PowerStateChangeReason::POWER_BUTTON),
                static_cast<uint32_t>(PowerStateChangeReason::STATE_CHANGE_REASON_UNKNOWN)
            );

            parcel.WriteUint64(screenId);
            parcel.WriteUint32(stateRaw);
            parcel.WriteUint32(reasonRaw);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_DISPLAY_STATE): {
            uint32_t stateRaw = fdp.ConsumeIntegral<uint32_t>();
            parcel.WriteUint32(stateRaw);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SPECIFIED_SCREEN_POWER): {
            uint32_t screenId = fdp.ConsumeIntegral<uint32_t>();
            uint32_t stateRaw = fdp.ConsumeIntegral<uint32_t>();
            uint32_t reasonRaw = fdp.ConsumeIntegral<uint32_t>();

            parcel.WriteUint32(screenId);
            parcel.WriteUint32(stateRaw);
            parcel.WriteUint32(reasonRaw);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_POWER_FOR_ALL): {
            uint32_t stateRaw = fdp.ConsumeIntegral<uint32_t>();
            uint32_t reasonRaw = fdp.ConsumeIntegral<uint32_t>();

            parcel.WriteUint32(stateRaw);
            parcel.WriteUint32(reasonRaw);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_STATE):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_VISIBLE_AREA_DISPLAY_INFO_BY_ID):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SCREEN_BRIGHTNESS_INFO):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTS_INPUT):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_BUNDLE_NAME):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ROUNDED_CORNER):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_IS_ON_BOARD_DISPLAY):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_CUTOUT_INFO):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_HAS_PRIVATE_WINDOW):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_EXPAND_AVAILABLE_AREA):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_BY_ID): {
            uint64_t displayId = fdp.ConsumeIntegral<uint64_t>();
            parcel.WriteUint64(displayId);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_EVENT): {
            uint32_t event = fdp.ConsumeIntegral<uint32_t>();
            parcel.WriteUint32(event);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER): {
            uint64_t dmsScreenId = fdp.ConsumeIntegral<uint64_t>();
            parcel.WriteUint64(dmsScreenId);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_BRIGHTNESS): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t level = fdp.ConsumeIntegral<uint32_t>();

            parcel.WriteUint64(screenId);
            parcel.WriteUint32(level);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_BRIGHTNESS):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_BY_SCREEN):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_INFO_BY_ID):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_GAMUTS):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_GROUP_INFO_BY_ID):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DENSITY_IN_CURRENT_RESOLUTION):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_GAMUT_MAP):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_TRANSFORM):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_PIXEL_FORMAT):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_HDR_FORMAT):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_SPACE):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_CONVERT_SCREENID_TO_RSSCREENID):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_HAS_IMMERSIVE_WINDOW):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_DUMP_SPECIAL_SCREEN):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_PROPERTY):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_NODE):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_COMBINATION):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PHY_SCREEN_PROPERTY):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_SCREEN_CONNECT_COMPLETION):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_DISABLE_POWEROFF_RENDER_CONTROL):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_IS_REAL_SCREEN):
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_PROPERTY_BY_FOLD_STATE_CHANGE): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            parcel.WriteUint64(screenId);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_DISPLAYIDS): {
            int32_t userId = fdp.ConsumeIntegral<int32_t>();
            parcel.WriteInt32(userId);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_SUPPORTS_INPUT): {
            uint64_t displayId = fdp.ConsumeIntegral<uint64_t>();
            bool supportsInput = fdp.ConsumeBool();
            parcel.WriteUint64(displayId);
            parcel.WriteBool(supportsInput);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_CREATE_VIRTUAL_SCREEN): {
            std::string name = fdp.ConsumeRandomLengthString(64);
            uint32_t width = fdp.ConsumeIntegral<uint32_t>();
            uint32_t height = fdp.ConsumeIntegral<uint32_t>();
            float density = fdp.ConsumeFloatingPoint<float>();
            int32_t flags = fdp.ConsumeIntegral<int32_t>();
            bool isForShot = fdp.ConsumeBool();

            uint64_t missionCount = fdp.ConsumeIntegralInRange<uint64_t>(0, MAX_BUFFER_SIZE);
            parcel.WriteUint64(missionCount);
            for (uint64_t i = 0; i < missionCount; ++i) {
                parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            }

            uint32_t virtualScreenType = fdp.ConsumeIntegral<uint32_t>();
            bool isSecurity = fdp.ConsumeBool();
            uint32_t virtualScreenFlag = fdp.ConsumeIntegral<uint32_t>();
            bool supportsFocus = fdp.ConsumeBool();
            bool supportsInput = fdp.ConsumeBool();

            std::string serialNumber = fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING);
            std::string bundleName = fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING);

            bool isSurfaceValid = fdp.ConsumeBool();
            parcel.WriteBool(isSurfaceValid);
            if (isSurfaceValid) {
                parcel.WriteRemoteObject(remote);
            }
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteString(name);
            parcel.WriteUint32(width);
            parcel.WriteUint32(height);
            parcel.WriteFloat(density);
            parcel.WriteInt32(flags);
            parcel.WriteBool(isForShot);
            parcel.WriteUint32(virtualScreenType);
            parcel.WriteBool(isSecurity);
            parcel.WriteUint32(virtualScreenFlag);
            parcel.WriteBool(supportsFocus);
            parcel.WriteBool(supportsInput);
            parcel.WriteString(serialNumber);
            parcel.WriteString(bundleName);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SURFACE): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            bool isSurfaceValid = fdp.ConsumeBool();

            parcel.WriteUint64(screenId);
            parcel.WriteBool(isSurfaceValid);
            if (isSurfaceValid) {
                parcel.WriteRemoteObject(remote);
            }

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_ADD_VIRTUAL_SCREEN_BLOCK_LIST): {
            uint64_t size = fdp.ConsumeIntegralInRange<uint64_t>(0, MAP_SIZE_MAX_NUM);
            parcel.WriteUint64(size);
            for (uint64_t i = 0; i < size; ++i) {
                parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
            }

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_BLOCK_LIST): {
            uint64_t size = fdp.ConsumeIntegralInRange<uint64_t>(0, MAP_SIZE_MAX_NUM);
            parcel.WriteUint64(size);
            for (uint64_t i = 0; i < size; ++i) {
                parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
            }

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_ADD_VIRTUAL_SCREEN_WHITE_LIST): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            parcel.WriteUint64(screenId);
            uint64_t missionCount = fdp.ConsumeIntegralInRange<uint64_t>(0, MAX_BUFFER_SIZE);
            parcel.WriteUint64(missionCount);
            for (uint64_t i = 0; i < missionCount; ++i) {
                parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            }
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_WHITE_LIST): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            parcel.WriteUint64(screenId);

            uint64_t missionCount = fdp.ConsumeIntegralInRange<uint64_t>(0, MAX_BUFFER_SIZE);
            parcel.WriteUint64(missionCount);
            for (uint64_t i = 0; i < missionCount; ++i) {
                parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            }

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_MASKIMAGE): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            bool isPrivacyMaskImgValid = fdp.ConsumeBool();
            parcel.WriteUint64(static_cast<uint64_t>(screenId));
            parcel.WriteBool(isPrivacyMaskImgValid);
            if (isPrivacyMaskImgValid) {
                int32_t width = fdp.ConsumeIntegral<int32_t>();
                int32_t height = fdp.ConsumeIntegral<int32_t>();
                parcel.WriteInt32(width);
                parcel.WriteInt32(height);
            }
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_CANVAS_ROTATION): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            bool autoRotate = fdp.ConsumeBool();
            parcel.WriteUint64(screenId);
            parcel.WriteBool(autoRotate);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SCALE_MODE): {
            // ProcSetVirtualScreenScaleMode read ：screenId + scaleMode
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t scaleMode = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SCALE_MODE_VALUE);
            parcel.WriteUint64(screenId);
            parcel.WriteUint32(scaleMode);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_DESTROY_VIRTUAL_SCREEN): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            parcel.WriteUint64(screenId);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR): {
            uint64_t mainScreenId = fdp.ConsumeIntegral<uint64_t>();
            size_t numMirrors = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<ScreenId> mirrorScreenId;
            for (size_t i = 0; i < numMirrors; ++i) {
                mirrorScreenId.push_back(static_cast<ScreenId>(fdp.ConsumeIntegral<uint64_t>()));
            }
            int32_t rotation = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_ROTATION_VALUE);
            bool needSetRotation = fdp.ConsumeBool();
            
            parcel.WriteUint64(mainScreenId);
            parcel.WriteUInt64Vector(mirrorScreenId);
            parcel.WriteInt32(rotation);
            parcel.WriteBool(needSetRotation);
            
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR_FOR_RECORD): {
            size_t numMain = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<ScreenId> mainScreenIds;
            for (size_t i = 0; i < numMain; ++i) {
                mainScreenIds.push_back(static_cast<ScreenId>(fdp.ConsumeIntegral<uint64_t>()));
            }
            
            size_t numMirror = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<ScreenId> mirrorScreenIds;
            for (size_t i = 0; i < numMirror; ++i) {
                mirrorScreenIds.push_back(static_cast<ScreenId>(fdp.ConsumeIntegral<uint64_t>()));
            }
            
            parcel.WriteUInt64Vector(mainScreenIds);
            parcel.WriteUInt64Vector(mirrorScreenIds);
            
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR_WITH_REGION): {
            uint64_t mainScreenId = fdp.ConsumeIntegral<uint64_t>();
            size_t numMirrors = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<ScreenId> mirrorScreenId;
            for (size_t i = 0; i < numMirrors; ++i) {
                mirrorScreenId.push_back(static_cast<ScreenId>(fdp.ConsumeIntegral<uint64_t>()));
            }
            
            int32_t posX = fdp.ConsumeIntegralInRange<int32_t>(MIN_POSITION, MAX_POSITION);
            int32_t posY = fdp.ConsumeIntegralInRange<int32_t>(MIN_POSITION, MAX_POSITION);
            uint32_t width = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_RESOLUTION);
            uint32_t height = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_RESOLUTION);
            
            parcel.WriteUint64(mainScreenId);
            parcel.WriteUInt64Vector(mirrorScreenId);
            parcel.WriteInt32(posX);
            parcel.WriteInt32(posY);
            parcel.WriteUint32(width);
            parcel.WriteUint32(height);
            
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_MULTI_SCREEN_MODE_SWITCH): {
            uint64_t mainScreenId = fdp.ConsumeIntegral<uint64_t>();
            uint64_t secondaryScreenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t screenMode = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SCALE_MODE_VALUE);
            parcel.WriteUint64(mainScreenId);
            parcel.WriteUint64(secondaryScreenId);
            parcel.WriteUint32(screenMode);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_MULTI_SCREEN_POSITION): {
            uint64_t mainScreenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t mainScreenX = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_POSITION);
            uint32_t mainScreenY = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_POSITION);
            uint64_t secondaryScreenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t secondaryScreenX = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_POSITION);
            uint32_t secondaryScreenY = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_POSITION);
            
            parcel.WriteUint64(mainScreenId);
            parcel.WriteUint32(mainScreenX);
            parcel.WriteUint32(mainScreenY);
            parcel.WriteUint64(secondaryScreenId);
            parcel.WriteUint32(secondaryScreenX);
            parcel.WriteUint32(secondaryScreenY);
            
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_STOP_MIRROR): {
            size_t numScreens = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<ScreenId> mirrorScreenIds;
            for (size_t i = 0; i < numScreens; ++i) {
                mirrorScreenIds.push_back(static_cast<ScreenId>(fdp.ConsumeIntegral<uint64_t>()));
            }
            parcel.WriteUInt64Vector(mirrorScreenIds);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_DISABLE_MIRROR): {
            bool disable = fdp.ConsumeBool();
            parcel.WriteBool(disable);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_EXPAND): {
            constexpr size_t maxAllowedScreens = MAX_BUFFER_SIZE;
            size_t numScreens = fdp.ConsumeIntegralInRange<size_t>(0, maxAllowedScreens);

            std::vector<ScreenId> screenId;
            screenId.reserve(numScreens);
            for (size_t i = 0; i < numScreens; ++i) {
                screenId.push_back(static_cast<ScreenId>(fdp.ConsumeIntegral<uint64_t>()));
            }

            std::vector<Point> startPoint;
            startPoint.reserve(numScreens);
            for (size_t i = 0; i < numScreens; ++i) {
                Point p;
                p.posX_ = fdp.ConsumeIntegralInRange<int32_t>(MIN_POSITION, MAX_POSITION);
                p.posY_ = fdp.ConsumeIntegralInRange<int32_t>(MIN_POSITION, MAX_POSITION);
                startPoint.push_back(p);
            }

            parcel.WriteUInt64Vector(screenId);
            parcel.WriteUint32(static_cast<uint32_t>(startPoint.size()));

            for (const auto& pt : startPoint) {
                parcel.WriteInt32(pt.posX_);
                parcel.WriteInt32(pt.posY_);
            }

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_STOP_EXPAND): {
            size_t numScreens = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<ScreenId> expandScreenIds;
            for (size_t i = 0; i < numScreens; ++i) {
                expandScreenIds.push_back(static_cast<ScreenId>(fdp.ConsumeIntegral<uint64_t>()));
            }
            parcel.WriteUInt64Vector(expandScreenIds);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_FROM_SCREEN_GROUP): {
            size_t numScreens = fdp.ConsumeIntegralInRange<size_t>(0, MAX_BUFFER_SIZE);
            std::vector<ScreenId> screenId;
            for (size_t i = 0; i < numScreens; ++i) {
                screenId.push_back(static_cast<ScreenId>(fdp.ConsumeIntegral<uint64_t>()));
            }
            parcel.WriteUInt64Vector(screenId);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT): {
            uint64_t displayId = fdp.ConsumeIntegral<uint64_t>();
            bool isUseDma = fdp.ConsumeBool();
            bool isCaptureFullOfScreen = fdp.ConsumeBool();

            parcel.WriteUint64(displayId);
            parcel.WriteBool(isUseDma);
            parcel.WriteBool(isCaptureFullOfScreen);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HDR_SNAPSHOT): {
            uint64_t displayId = fdp.ConsumeIntegral<uint64_t>();
            bool isUseDma = fdp.ConsumeBool();
            bool isCaptureFullOfScreen = fdp.ConsumeBool();

            parcel.WriteUint64(displayId);
            parcel.WriteBool(isUseDma);
            parcel.WriteBool(isCaptureFullOfScreen);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ACTIVE_MODE): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t modeId = fdp.ConsumeIntegral<uint32_t>();

            parcel.WriteUint64(screenId);
            parcel.WriteUint32(modeId);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            float virtualPixelRatio = fdp.ConsumeFloatingPoint<float>();

            parcel.WriteUint64(screenId);
            parcel.WriteFloat(virtualPixelRatio);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            float virtualPixelRatio = fdp.ConsumeFloatingPoint<float>();

            parcel.WriteUint64(screenId);
            parcel.WriteFloat(virtualPixelRatio);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_DEFAULT_DENSITY_DPI): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            float virtualPixelRatio = fdp.ConsumeFloatingPoint<float>();

            parcel.WriteUint64(screenId);
            parcel.WriteFloat(virtualPixelRatio);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_RESOLUTION): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t width = fdp.ConsumeIntegral<uint32_t>();
            uint32_t height = fdp.ConsumeIntegral<uint32_t>();
            float virtualPixelRatio = fdp.ConsumeFloatingPoint<float>();

            parcel.WriteUint64(screenId);
            parcel.WriteUint32(width);
            parcel.WriteUint32(height);
            parcel.WriteFloat(virtualPixelRatio);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_GAMUT): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            int32_t colorGamutIdx = fdp.ConsumeIntegral<int32_t>();

            parcel.WriteUint64(screenId);
            parcel.WriteInt32(colorGamutIdx);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_GAMUT_MAP): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t gamutMap = fdp.ConsumeIntegral<uint32_t>();

            parcel.WriteUint64(screenId);
            parcel.WriteUint32(gamutMap);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_PIXEL_FORMAT): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t pixelFormat = fdp.ConsumeIntegral<uint32_t>();

            parcel.WriteUint64(screenId);
            parcel.WriteUint32(pixelFormat);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_HDR_FORMAT): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            parcel.WriteUint64(screenId);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_HDR_FORMAT): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            int32_t modeIdx = fdp.ConsumeIntegral<int32_t>();

            parcel.WriteUint64(screenId);
            parcel.WriteInt32(modeIdx);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_SPACE): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t colorSpace = fdp.ConsumeIntegral<uint32_t>();

            parcel.WriteUint64(screenId);
            parcel.WriteUint32(colorSpace);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_ORIENTATION): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t orientation = fdp.ConsumeIntegral<uint32_t>();
            bool isFromNapi = fdp.ConsumeBool();

            parcel.WriteUint64(screenId);
            parcel.WriteUint32(orientation);
            parcel.WriteBool(isFromNapi);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED): {
            bool isLocked = fdp.ConsumeBool();
            parcel.WriteBool(isLocked);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED_FROM_JS): {
            bool isLocked = fdp.ConsumeBool();
            parcel.WriteBool(isLocked);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_CUTOUT_INFO_WITH_ROTATION): {
            uint64_t displayId = fdp.ConsumeIntegral<uint64_t>();
            int32_t width = fdp.ConsumeIntegral<int32_t>();
            int32_t height = fdp.ConsumeIntegral<int32_t>();
            uint32_t rotation = fdp.ConsumeIntegral<uint32_t>();

            parcel.WriteUint64(displayId);
            parcel.WriteInt32(width);
            parcel.WriteInt32(height);
            parcel.WriteUint32(rotation);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE): {
            uint32_t displayMode = fdp.ConsumeIntegral<uint32_t>();
            parcel.WriteUint32(displayMode);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_FOLD_DISPLAY_MODE_FROM_JS): {
            uint32_t displayMode = fdp.ConsumeIntegral<uint32_t>();
            std::string reason = fdp.ConsumeRandomLengthString();

            parcel.WriteUint32(displayMode);
            parcel.WriteString(reason);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_LOCK_FOLD_DISPLAY_STATUS): {
            bool lockDisplayStatus = fdp.ConsumeBool();
            parcel.WriteBool(lockDisplayStatus);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_LOCK_FOLD_DISPLAY_STATUS_FROM_JS): {
            bool lockDisplayStatus = fdp.ConsumeBool();
            parcel.WriteBool(lockDisplayStatus);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_TARGET_FOLD_STATUS_AND_LOCK): {
            uint32_t targetFoldStatus = fdp.ConsumeIntegral<uint32_t>();
            parcel.WriteUint32(targetFoldStatus);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_FOLD_STATUS_EXPAND_AND_LOCKED): {
            bool lockDisplayStatus = fdp.ConsumeBool();
            parcel.WriteBool(lockDisplayStatus);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_DISPLAY_SCALE): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            float scaleX = fdp.ConsumeFloatingPoint<float>();
            float scaleY = fdp.ConsumeFloatingPoint<float>();
            float pivotX = fdp.ConsumeFloatingPoint<float>();
            float pivotY = fdp.ConsumeFloatingPoint<float>();
            parcel.WriteUint64(screenId);
            parcel.WriteFloat(scaleX);
            parcel.WriteFloat(scaleY);
            parcel.WriteFloat(pivotX);
            parcel.WriteFloat(pivotY);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_LANDSCAPE_LOCK_STATUS): {
            bool isLocked = fdp.ConsumeBool();
            parcel.WriteBool(isLocked);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN): {
            uint32_t size = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_BUFFER_SIZE);
            parcel.WriteUint32(size);
            std::vector<uint64_t> screenIds;
            for (uint32_t i = 0; i < size; ++i) {
                screenIds.push_back(fdp.ConsumeIntegral<uint64_t>());
            }
            parcel.WriteUInt64Vector(screenIds);
            bool isRotationLocked = fdp.ConsumeBool();
            int32_t rotation = fdp.ConsumeIntegral<int32_t>();
            parcel.WriteBool(isRotationLocked);
            parcel.WriteInt32(rotation);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_CLIENT): {
            parcel.WriteRemoteObject(remote);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_DIRECTION_INFO): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            float screenComponentRotation = fdp.ConsumeFloatingPoint<float>();
            float rotation = fdp.ConsumeFloatingPoint<float>();
            float phyRotation = fdp.ConsumeFloatingPoint<float>();
            uint32_t changeType = fdp.ConsumeIntegral<uint32_t>();

            parcel.WriteUint64(screenId);
            parcel.WriteFloat(screenComponentRotation);
            parcel.WriteFloat(rotation);
            parcel.WriteFloat(phyRotation);
            parcel.WriteUint32(changeType);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_ROTATION_PROPERTY): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            float rotation = fdp.ConsumeFloatingPoint<float>();
            uint32_t changeType = fdp.ConsumeIntegral<uint32_t>();
            bool isSwitchUser = fdp.ConsumeBool();
            parcel.WriteUint64(screenId);
            parcel.WriteFloat(rotation);
            parcel.WriteUint32(changeType);
            parcel.WriteBool(isSwitchUser);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_CURVED_SCREEN_COMPRESSION_AREA): {
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_CHANGE_INFO): {
            constexpr uint32_t minAction = static_cast<uint32_t>(ActionType::SINGLE_START);
            constexpr uint32_t maxAction = static_cast<uint32_t>(ActionType::SINGLE_BACKGROUND);
            ActionType action = static_cast<ActionType>(
                fdp.ConsumeIntegralInRange<uint32_t>(minAction, maxAction)
            );

            uint64_t toScreenId = fdp.ConsumeIntegral<uint64_t>();
            uint64_t fromScreenId = fdp.ConsumeIntegral<uint64_t>();

            std::string abilityName = fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING);
            std::string bundleName = fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING);

            sptr<DisplayChangeInfo> info = sptr<DisplayChangeInfo>::MakeSptr();
            info->action_ = action;
            info->toScreenId_ = toScreenId;
            info->fromScreenId_ = fromScreenId;
            info->abilityName_ = abilityName;
            info->bundleName_ = bundleName;

            if (!info->Marshalling(parcel)) {
                reply.WriteInt32(static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM));
                reply.WriteBool(false);
                break;
            }

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_STATE): {
            bool hasPrivate = fdp.ConsumeBool();
            parcel.WriteBool(hasPrivate);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREENID_PRIVACY_STATE): {
            uint32_t mapSize = fdp.ConsumeIntegralInRange<uint32_t>(0, MAP_SIZE_MAX_NUM);
            std::vector<uint64_t> displayIds;
            std::vector<bool> hasPrivacies;
            for (uint32_t i = 0; i < mapSize; ++i) {
                displayIds.push_back(fdp.ConsumeIntegral<uint64_t>());
                hasPrivacies.push_back(fdp.ConsumeBool());
            }
            parcel.WriteUInt64Vector(displayIds);
            parcel.WriteBoolVector(hasPrivacies);
            bool isRotationLocked = fdp.ConsumeBool();
            int32_t rotation = fdp.ConsumeIntegral<int32_t>();
            parcel.WriteBool(isRotationLocked);
            parcel.WriteInt32(rotation);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_LIST): {
            uint64_t displayId = fdp.ConsumeIntegral<uint64_t>();
            parcel.WriteUint64(displayId);
            uint32_t vecSize = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_BUFFER_SIZE);
            std::vector<std::string> privacyWindowList;
            for (uint32_t i = 0; i < vecSize; ++i) {
                privacyWindowList.push_back(fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING));
            }
            parcel.WriteStringVector(privacyWindowList);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_RESIZE_VIRTUAL_SCREEN): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t width = fdp.ConsumeIntegral<uint32_t>();
            uint32_t height = fdp.ConsumeIntegral<uint32_t>();
            parcel.WriteUint64(screenId);
            parcel.WriteUint32(width);
            parcel.WriteUint32(height);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_AVAILABLE_AREA): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            int32_t posX = fdp.ConsumeIntegral<int32_t>();
            int32_t posY = fdp.ConsumeIntegral<int32_t>();
            uint32_t width = fdp.ConsumeIntegral<uint32_t>();
            uint32_t height = fdp.ConsumeIntegral<uint32_t>();
            parcel.WriteUint64(screenId);
            parcel.WriteInt32(posX);
            parcel.WriteInt32(posY);
            parcel.WriteUint32(width);
            parcel.WriteUint32(height);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SUPER_FOLD_AVAILABLE_AREA): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            int32_t bPosX = fdp.ConsumeIntegral<int32_t>();
            int32_t bPosY = fdp.ConsumeIntegral<int32_t>();
            uint32_t bWidth = fdp.ConsumeIntegral<uint32_t>();
            uint32_t bHeight = fdp.ConsumeIntegral<uint32_t>();
            int32_t cPosX = fdp.ConsumeIntegral<int32_t>();
            int32_t cPosY = fdp.ConsumeIntegral<int32_t>();
            uint32_t cWidth = fdp.ConsumeIntegral<uint32_t>();
            uint32_t cHeight = fdp.ConsumeIntegral<uint32_t>();
            parcel.WriteUint64(screenId);
            parcel.WriteInt32(bPosX);
            parcel.WriteInt32(bPosY);
            parcel.WriteUint32(bWidth);
            parcel.WriteUint32(bHeight);
            parcel.WriteInt32(cPosX);
            parcel.WriteInt32(cPosY);
            parcel.WriteUint32(cWidth);
            parcel.WriteUint32(cHeight);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SUPER_FOLD_EXPAND_AVAILABLE_AREA): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            int32_t posX = fdp.ConsumeIntegral<int32_t>();
            int32_t posY = fdp.ConsumeIntegral<int32_t>();
            uint32_t width = fdp.ConsumeIntegral<uint32_t>();
            uint32_t height = fdp.ConsumeIntegral<uint32_t>();
            parcel.WriteUint64(screenId);
            parcel.WriteInt32(posX);
            parcel.WriteInt32(posY);
            parcel.WriteUint32(width);
            parcel.WriteUint32(height);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_OFF_DELAY_TIME): {
            int32_t delay = fdp.ConsumeIntegral<int32_t>();
            parcel.WriteInt32(delay);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_FOLD_TO_EXPAND_COMPLETION): {
            bool foldToExpand = fdp.ConsumeBool();
            parcel.WriteBool(foldToExpand);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_AOD_OP_COMPLETION): {
            uint32_t op = fdp.ConsumeIntegral<uint32_t>();
            int32_t result = fdp.ConsumeIntegral<int32_t>();
            parcel.WriteUint32(op);
            parcel.WriteInt32(result);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_REFRESH_RATE): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t refreshInterval = fdp.ConsumeIntegral<uint32_t>();
            parcel.WriteUint64(screenId);
            parcel.WriteUint32(refreshInterval);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_BLACK_LIST): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            parcel.WriteUint64(screenId);
            uint64_t windowCount = fdp.ConsumeIntegralInRange<uint64_t>(0, MAX_BUFFER_SIZE);
            parcel.WriteUint64(windowCount);
            for (uint64_t i = 0; i < windowCount; ++i) {
                parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            }

            uint64_t surfaceCount = fdp.ConsumeIntegralInRange<uint64_t>(0, MAX_BUFFER_SIZE);
            parcel.WriteUint64(surfaceCount);
            for (uint64_t i = 0; i < surfaceCount; ++i) {
                parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            }

            uint64_t typeCount = fdp.ConsumeIntegralInRange<uint64_t>(0, MAX_BUFFER_SIZE);
            parcel.WriteUint64(typeCount);
            for (uint64_t i = 0; i < typeCount; ++i) {
                parcel.WriteUint8(fdp.ConsumeIntegral<uint8_t>());
            }
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_HOOK_INFO): {
            int32_t uid = fdp.ConsumeIntegral<int32_t>();
            bool enable = fdp.ConsumeBool();
            uint32_t width = fdp.ConsumeIntegral<uint32_t>();
            uint32_t height = fdp.ConsumeIntegral<uint32_t>();
            float density = fdp.ConsumeFloatingPoint<float>();
            uint32_t rotation = fdp.ConsumeIntegral<uint32_t>();
            bool enableHookRotation = fdp.ConsumeBool();
            uint32_t displayOrientation = fdp.ConsumeIntegral<uint32_t>();
            bool enableHookDisplayOrientation = fdp.ConsumeBool();
            bool isFullScreenInForceSplit = fdp.ConsumeBool();

            parcel.WriteInt32(uid);
            parcel.WriteBool(enable);
            parcel.WriteUint32(width);
            parcel.WriteUint32(height);
            parcel.WriteFloat(density);
            parcel.WriteUint32(rotation);
            parcel.WriteBool(enableHookRotation);
            parcel.WriteUint32(displayOrientation);
            parcel.WriteBool(enableHookDisplayOrientation);
            parcel.WriteBool(isFullScreenInForceSplit);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HOOK_INFO): {
            int32_t uid = fdp.ConsumeIntegral<int32_t>();
            parcel.WriteInt32(uid);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SECURITY_EXEMPTION): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            bool exempt = fdp.ConsumeBool();
            parcel.WriteUint64(screenId);
            parcel.WriteBool(exempt);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_STATUS): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            int32_t screenStatus = fdp.ConsumeIntegral<int32_t>();

            parcel.WriteUint64(screenId);
            parcel.WriteInt32(screenStatus);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_MAX_REFRESHRATE): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t refreshRate = fdp.ConsumeIntegral<uint32_t>();

            parcel.WriteUint64(screenId);
            parcel.WriteUint32(refreshRate);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            bool includeWindow = fdp.ConsumeBool();
            bool includeSurface = fdp.ConsumeBool();
            bool captureOpaqueOnly = fdp.ConsumeBool();

            uint32_t windowCount = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_BUFFER_SIZE);
            std::vector<uint64_t> windowIdList;
            for (uint32_t i = 0; i < windowCount; ++i) {
                windowIdList.push_back(fdp.ConsumeIntegral<uint64_t>());
            }

            float cropX = fdp.ConsumeFloatingPoint<float>();
            float cropY = fdp.ConsumeFloatingPoint<float>();
            int32_t width = fdp.ConsumeIntegralInRange<int32_t>(1, MAX_RESOLUTION);
            int32_t height = fdp.ConsumeIntegralInRange<int32_t>(1, MAX_RESOLUTION);

            parcel.WriteUint64(screenId);
            parcel.WriteBool(includeWindow);
            parcel.WriteBool(includeSurface);
            parcel.WriteBool(captureOpaqueOnly);
            parcel.WriteUInt64Vector(windowIdList);
            parcel.WriteFloat(cropX);
            parcel.WriteFloat(cropY);
            parcel.WriteInt32(width);
            parcel.WriteInt32(height);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HDR_SNAPSHOT_WITH_OPTION): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            bool enableHdr = fdp.ConsumeBool();

            parcel.WriteUint64(screenId);
            parcel.WriteBool(enableHdr);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_CAMERA_STATUS): {
            int32_t cameraStatus = fdp.ConsumeIntegral<int32_t>();
            int32_t cameraPosition = fdp.ConsumeIntegral<int32_t>();

            parcel.WriteInt32(cameraStatus);
            parcel.WriteInt32(cameraPosition);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ON_DELAY_TIME): {
            int32_t delay = fdp.ConsumeIntegral<int32_t>();

            parcel.WriteInt32(delay);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_SKIP_PROTECTED_WINDOW): {
            uint32_t count = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_BUFFER_SIZE);
            std::vector<uint64_t> screenIds;
            for (uint32_t i = 0; i < count; ++i) {
                screenIds.push_back(fdp.ConsumeIntegral<uint64_t>());
            }
            bool isEnable = fdp.ConsumeBool();

            parcel.WriteUInt64Vector(screenIds);
            parcel.WriteBool(isEnable);

            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_RECORD_EVENT_FROM_SCB): {
            std::string description = fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING);
            bool needRecordEvent = fdp.ConsumeBool();

            parcel.WriteString(description);
            parcel.WriteBool(needRecordEvent);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SYSTEM_KEYBOARD_STATUS): {
            bool isTpKeyboardOn = fdp.ConsumeBool();

            parcel.WriteBool(isTpKeyboardOn);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_DISPLAY_MUTE_FLAG): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            bool muteFlag = fdp.ConsumeBool();
            parcel.WriteUint64(screenId);
            parcel.WriteBool(muteFlag);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_FORCE_CLOSE_HDR): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            bool isForceCloseHdr = fdp.ConsumeBool();
            parcel.WriteUint64(screenId);
            parcel.WriteBool(isForceCloseHdr);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SYNCHRONIZED_POWER_STATUS): {
            constexpr uint32_t maxPowerStateValue = static_cast<uint32_t>(ScreenPowerState::POWER_DOZE_SUSPEND);
            uint32_t stateTemp = fdp.ConsumeIntegralInRange<uint32_t>(0, maxPowerStateValue);
            parcel.WriteUint32(stateTemp);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_TAG_SWITCH): {
            uint64_t screenId = fdp.ConsumeIntegral<uint64_t>();
            uint32_t vecSize = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_BUFFER_SIZE);
            std::vector<std::string> privacyWindowTag;
            for (uint32_t i = 0; i < vecSize; ++i) {
                privacyWindowTag.push_back(fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING));
            }
            bool enable = fdp.ConsumeBool();
            parcel.WriteUint64(screenId);
            parcel.WriteStringVector(privacyWindowTag);
            parcel.WriteBool(enable);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_IS_FULL_SCREEN_IN_FORCE_SPLIT): {
            int32_t uid = fdp.ConsumeIntegral<int32_t>();
            bool isFullScreen = fdp.ConsumeBool();
            parcel.WriteInt32(uid);
            parcel.WriteBool(isFullScreen);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SYNC_SCREEN_POWER_STATE): {
            constexpr uint32_t maxPowerStateValue = static_cast<uint32_t>(ScreenPowerState::POWER_DOZE_SUSPEND);
            uint32_t state = fdp.ConsumeIntegralInRange<uint32_t>(0, maxPowerStateValue);
            parcel.WriteUint32(state);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REGISTER_DISPLAY_ATTRIBUTE_AGENT): {
            parcel.WriteRemoteObject(remote);
            uint32_t vecSize = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_BUFFER_SIZE);
            std::vector<std::string> attributes;
            for (uint32_t i = 0; i < vecSize; ++i) {
                attributes.push_back(fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING));
            }
            parcel.WriteStringVector(attributes);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        case static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UNREGISTER_DISPLAY_ATTRIBUTE): {
            parcel.WriteRemoteObject(remote);
            uint32_t vecSize = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_BUFFER_SIZE);
            std::vector<std::string> attributes;
            for (uint32_t i = 0; i < vecSize; ++i) {
                attributes.push_back(fdp.ConsumeRandomLengthString(MAX_LENGTH_STRING));
            }
            parcel.WriteStringVector(attributes);
            screenStub->OnRemoteRequest(code, parcel, reply, option);
            break;
        }
        default:
            break;
    }
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
        ScreenSessionManagerStubFuzzTestEnhanced,
    });
    func(fdp);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < DATA_MIN_SIZE) {
        return 0;
    }

    FuzzedDataProvider fdp(data, size);
    Test(fdp);
    return 0;
}
}