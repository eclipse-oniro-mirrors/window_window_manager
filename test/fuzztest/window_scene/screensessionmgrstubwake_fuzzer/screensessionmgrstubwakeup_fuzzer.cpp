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

#include "screensessionmgrstubwakeup_fuzzer.h"

#include <iremote_stub.h>
#include <parcel.h>

#include "message_option.h"
#include "message_parcel.h"
#include "screen_session_manager_stub.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace {
constexpr size_t DATA_MIN_SIZE = 2;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    parcel.RewindRead(0);
    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_WAKE_UP_BEGIN),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_WAKE_UP_END),
        parcel, reply, option);
    return true;
}
bool DoSomethingInterestingWithMyAPI1(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();

    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_VISIBLE_AREA_DISPLAY_INFO_BY_ID),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_INTERNAL_SCREEN_ID),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_POWER_BY_ID),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SPECIFIED_SCREEN_POWER),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_TRY_TO_CANCEL_SCREEN_OFF),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_BRIGHTNESS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_BRIGHTNESS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_FREEZE_EVENT),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_ADD_VIRTUAL_SCREEN_BLOCK_LIST),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_BLOCK_LIST),
        parcel, reply, option);
    return true;
}
bool DoSomethingInterestingWithMyAPI2(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();

    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_CANVAS_ROTATION),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PHYSICAL_SCREEN_IDS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_DEFAULT_DENSITY_DPI),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_RESOLUTION),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DENSITY_IN_CURRENT_RESOLUTION),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_MASKIMAGE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR_FOR_RECORD),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR_WITH_REGION),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_PIXEL_FORMAT),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_PIXEL_FORMAT),
        parcel, reply, option);
    return true;
}
bool DoSomethingInterestingWithMyAPI3(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();

    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_HDR_FORMAT),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_HDR_FORMAT),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_HDR_FORMAT),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_SPACE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_SPACE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_SPACE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED_FROM_JS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_HAS_IMMERSIVE_WINDOW),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_ADD_SURFACE_NODE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REMOVE_SURFACE_NODE),
        parcel, reply, option);
    return true;
}
bool DoSomethingInterestingWithMyAPI4(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();

    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_DISABLE_MIRROR),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_CUTOUT_INFO_WITH_ROTATION),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SCREEN_BASE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_FOLD_DISPLAY_MODE_FROM_JS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_DISPLAY_SCALE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_SUPER_FOLD_STATUS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_LANDSCAPE_LOCK_STATUS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_LOCK_FOLD_DISPLAY_STATUS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_LOCK_FOLD_DISPLAY_STATUS_FROM_JS),
        parcel, reply, option);
    return true;
}
bool DoSomethingInterestingWithMyAPI5(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();

    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_FORCE_CLOSE_HDR),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_SUPER_ROTATION),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_CLIENT),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_PROPERTY),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_NODE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_DIRECTION_INFO),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_ROTATION_PROPERTY),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_AVAILABLE_AREA),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SUPER_FOLD_AVAILABLE_AREA),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_OFF_DELAY_TIME),
        parcel, reply, option);
    return true;
}
bool DoSomethingInterestingWithMyAPI6(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();

    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_CURVED_SCREEN_COMPRESSION_AREA),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PHY_SCREEN_PROPERTY),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_CHANGE_INFO),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_STATE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_LIST),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_RESIZE_VIRTUAL_SCREEN),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_AVAILABLE_AREA),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_FOLD_TO_EXPAND_COMPLETION),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_CONVERT_SCREENID_TO_RSSCREENID),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_VIRTUAL_SCREEN_FLAG),
        parcel, reply, option);
    return true;
}
bool DoSomethingInterestingWithMyAPI7(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();

    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_FLAG),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SCALE_MODE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DEVICE_SCREEN_CONFIG),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_REFRESH_RATE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_DEVICE_IS_CAPTURE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SNAPSHOT_BY_PICKER),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SWITCH_USER),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_BLACK_LIST),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_DISABLE_POWEROFF_RENDER_CONTROL),
        parcel, reply, option);
    return true;
}
bool DoSomethingInterestingWithMyAPI8(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();

    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_PROXY_FOR_FREEZE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_RESET_ALL_FREEZE_STATUS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_HOOK_INFO),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_HOOK_INFO),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_PHYSICAL_DISPLAY_RESOLUTION),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_STATUS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SECURITY_EXEMPTION),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_MAX_REFRESHRATE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPTURE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PRIMARY_DISPLAY_INFO),
        parcel, reply, option);
    return true;
}

bool DoSomethingInterestingWithMyAPI9(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();

    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_CAMERA_STATUS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ON_DELAY_TIME),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_COMBINATION),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER_AUTO),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_SKIP_PROTECTED_WINDOW),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_RECORD_EVENT_FROM_SCB),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_UPDATE_SUPER_FOLD_EXPAND_AVAILABLE_AREA),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_EXPAND_AVAILABLE_AREA),
        parcel, reply, option);
    return true;
}

bool DoSomethingInterestingWithMyAPI10(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();

    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_IS_REAL_SCREEN),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_EXTEND_SCREEN_CONNECT_STATUS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPABILITY),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SYSTEM_KEYBOARD_STATUS),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_DISPLAY_MUTE_FLAG),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_DEFAULT_MODE_WHEN_SWITCH_USER),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_EXTEND_SCREEN_CREATE_FINISH),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_EXTEND_SCREEN_DESTROY_FINISH),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_FOLD_STATUS_EXPAND_AND_LOCKED),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_SCREEN_MASK_APPEAR),
        parcel, reply, option);
    return true;
}

bool DoSomethingInterestingWithMyAPI11(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return false;
    }

    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;

    parcel.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    parcel.WriteBuffer(data, size);

    std::shared_ptr<ScreenSessionManagerStub> screenStub = std::make_shared<ScreenSessionManagerStub>();

    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_KEYBOARD_STATE),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_AREA_OF_DISPLAY_AREA),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_PRIMARY_DISPLAY_SYSTEM_DPI),
        parcel, reply, option);
    parcel.RewindRead(0);
    screenStub->OnRemoteRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_AUTO_ROTATION),
        parcel, reply, option);
    return true;
}

} // namespace.OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    OHOS::DoSomethingInterestingWithMyAPI1(data, size);
    return 0;
}

extern "C" int LLVMFuzzerTestTwoInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI2(data, size);
    OHOS::DoSomethingInterestingWithMyAPI3(data, size);
    return 0;
}

extern "C" int LLVMFuzzerTestThreeInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI4(data, size);
    OHOS::DoSomethingInterestingWithMyAPI5(data, size);
    return 0;
}

extern "C" int LLVMFuzzerTestFourInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI6(data, size);
    OHOS::DoSomethingInterestingWithMyAPI7(data, size);
    return 0;
}

extern "C" int LLVMFuzzerTestFiveInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI8(data, size);
    OHOS::DoSomethingInterestingWithMyAPI9(data, size);
    return 0;
}

extern "C" int LLVMFuzzerTestSixInput(const uint8_t* data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI10(data, size);
    OHOS::DoSomethingInterestingWithMyAPI11(data, size);
    return 0;
}