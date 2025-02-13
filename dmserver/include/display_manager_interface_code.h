/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_DMSERVER_DISPLAY_MANAGER_INTERFACE_CODE_H
#define FOUNDATION_DMSERVER_DISPLAY_MANAGER_INTERFACE_CODE_H

namespace OHOS::Rosen {
enum class DisplayManagerMessage : unsigned int {
    TRANS_ID_GET_DEFAULT_DISPLAY_INFO = 0,
    TRANS_ID_GET_DISPLAY_BY_ID,
    TRANS_ID_GET_VISIBLE_AREA_DISPLAY_INFO_BY_ID,
    TRANS_ID_GET_DISPLAY_BY_SCREEN,
    TRANS_ID_GET_DISPLAY_SNAPSHOT,
    TRANS_ID_REGISTER_DISPLAY_MANAGER_AGENT,
    TRANS_ID_UNREGISTER_DISPLAY_MANAGER_AGENT,
    TRANS_ID_WAKE_UP_BEGIN,
    TRANS_ID_WAKE_UP_END,
    TRANS_ID_SUSPEND_BEGIN,
    TRANS_ID_SUSPEND_END,
    TRANS_ID_GET_INTERNAL_SCREEN_ID,
    TRANS_ID_SET_SCREEN_POWER_BY_ID,
    TRANS_ID_SET_SPECIFIED_SCREEN_POWER,
    TRANS_ID_SET_SCREEN_POWER_FOR_ALL,
    TRANS_ID_GET_SCREEN_POWER,
    TRANS_ID_SET_DISPLAY_STATE,
    TRANS_ID_GET_DISPLAY_STATE,
    TRANS_ID_GET_ALL_DISPLAYIDS,
    TRANS_ID_NOTIFY_DISPLAY_EVENT,
    TRANS_ID_TRY_TO_CANCEL_SCREEN_OFF,
    TRANS_ID_SET_SCREEN_BRIGHTNESS,
    TRANS_ID_GET_SCREEN_BRIGHTNESS,
    TRANS_ID_SET_FREEZE_EVENT,
    TRANS_ID_SCREEN_BASE = 1000,
    TRANS_ID_CREATE_VIRTUAL_SCREEN = TRANS_ID_SCREEN_BASE,
    TRANS_ID_DESTROY_VIRTUAL_SCREEN,
    TRANS_ID_SET_VIRTUAL_SCREEN_SURFACE,
    TRANS_ID_SET_VIRTUAL_SCREEN_CANVAS_ROTATION,
    TRANS_ID_GET_SCREEN_INFO_BY_ID,
    TRANS_ID_GET_SCREEN_GROUP_INFO_BY_ID,
    TRANS_ID_SET_SCREEN_ACTIVE_MODE,
    TRANS_ID_GET_ALL_SCREEN_INFOS,
    TRANS_ID_SET_ORIENTATION,
    TRANS_ID_SET_VIRTUAL_PIXEL_RATIO,
    TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM,
    TRANS_ID_SET_RESOLUTION,
    TRANS_ID_GET_DENSITY_IN_CURRENT_RESOLUTION,
    TRANS_ID_SET_SCREEN_PRIVACY_MASKIMAGE,
    TRANS_ID_SCREENGROUP_BASE = 1100,
    TRANS_ID_SCREEN_MAKE_MIRROR = TRANS_ID_SCREENGROUP_BASE,
    TRANS_ID_SCREEN_MAKE_MIRROR_FOR_RECORD,
    TRANS_ID_SCREEN_MAKE_MIRROR_WITH_REGION,
    TRANS_ID_MULTI_SCREEN_MODE_SWITCH,
    TRANS_ID_SET_MULTI_SCREEN_POSITION,
    TRANS_ID_SCREEN_MAKE_EXPAND,
    TRANS_ID_REMOVE_VIRTUAL_SCREEN_FROM_SCREEN_GROUP,
    TRANS_ID_SCREEN_GAMUT_BASE = 1200,
    TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_GAMUTS = TRANS_ID_SCREEN_GAMUT_BASE,
    TRANS_ID_SCREEN_GET_COLOR_GAMUT,
    TRANS_ID_SCREEN_SET_COLOR_GAMUT,
    TRANS_ID_SCREEN_GET_GAMUT_MAP,
    TRANS_ID_SCREEN_SET_GAMUT_MAP,
    TRANS_ID_SCREEN_SET_COLOR_TRANSFORM,
    TRANS_ID_SCREEN_GET_PIXEL_FORMAT,
    TRANS_ID_SCREEN_SET_PIXEL_FORMAT,
    TRANS_ID_SCREEN_GET_SUPPORTED_HDR_FORMAT,
    TRANS_ID_SCREEN_GET_HDR_FORMAT,
    TRANS_ID_SCREEN_SET_HDR_FORMAT,
    TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_SPACE,
    TRANS_ID_SCREEN_GET_COLOR_SPACE,
    TRANS_ID_SCREEN_SET_COLOR_SPACE,
    TRANS_ID_IS_SCREEN_ROTATION_LOCKED,
    TRANS_ID_SET_SCREEN_ROTATION_LOCKED,
    TRANS_ID_SET_SCREEN_ROTATION_LOCKED_FROM_JS,
    TRANS_ID_HAS_PRIVATE_WINDOW,
    TRANS_ID_GET_CUTOUT_INFO,
    TRANS_ID_HAS_IMMERSIVE_WINDOW,
    TRANS_ID_ADD_SURFACE_NODE,
    TRANS_ID_REMOVE_SURFACE_NODE,
    TRANS_ID_SCREEN_STOP_MIRROR,
    TRANS_ID_SCREEN_STOP_EXPAND,
    TRANS_ID_SCREEN_DISABLE_MIRROR,
    TRANS_ID_SCENE_BOARD_SCREEN_BASE = 2000,
    TRANS_ID_SCENE_BOARD_DUMP_ALL_SCREEN,
    TRANS_ID_SCENE_BOARD_DUMP_SPECIAL_SCREEN,
    TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE,
    TRANS_ID_SET_FOLD_DISPLAY_MODE_FROM_JS,
    TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE,
    TRANS_ID_SCENE_BOARD_SET_DISPLAY_SCALE,
    TRANS_ID_SCENE_BOARD_IS_FOLDABLE,
    TRANS_ID_SCENE_BOARD_GET_FOLD_STATUS,
    TRANS_ID_SCENE_BOARD_GET_SUPER_FOLD_STATUS,
    TRANS_ID_SCENE_BOARD_GET_CURRENT_FOLD_CREASE_REGION,
    TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN,
    TRANS_ID_SCENE_BOARD_LOCK_FOLD_DISPLAY_STATUS,
    TRANS_ID_SET_LOCK_FOLD_DISPLAY_STATUS_FROM_JS,
    TRANS_ID_SET_CLIENT = 2500,
    TRANS_ID_GET_SCREEN_PROPERTY,
    TRANS_ID_GET_DISPLAY_NODE,
    TRANS_ID_UPDATE_SCREEN_DIRECTION_INFO,
    TRANS_ID_UPDATE_SCREEN_ROTATION_PROPERTY,
    TRANS_ID_UPDATE_AVAILABLE_AREA,
    TRANS_ID_UPDATE_SUPER_FOLD_AVAILABLE_AREA,
    TRANS_ID_SET_SCREEN_OFF_DELAY_TIME,
    TRANS_ID_GET_CURVED_SCREEN_COMPRESSION_AREA,
    TRANS_ID_GET_PHY_SCREEN_PROPERTY,
    TRANS_ID_NOTIFY_DISPLAY_CHANGE_INFO,
    TRANS_ID_SET_SCREEN_PRIVACY_STATE,
    TRANS_ID_SET_SCREENID_PRIVACY_STATE,
    TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_LIST,
    TRANS_ID_RESIZE_VIRTUAL_SCREEN,
    TRANS_ID_GET_AVAILABLE_AREA,
    TRANS_ID_NOTIFY_FOLD_TO_EXPAND_COMPLETION,
    TRANS_ID_CONVERT_SCREENID_TO_RSSCREENID,
    TRANS_ID_GET_VIRTUAL_SCREEN_FLAG,
    TRANS_ID_SET_VIRTUAL_SCREEN_FLAG,
    TRANS_ID_SET_VIRTUAL_SCREEN_SCALE_MODE,
    TRANS_ID_GET_DEVICE_SCREEN_CONFIG,
    TRANS_ID_SET_VIRTUAL_SCREEN_REFRESH_RATE,
    TRANS_ID_DEVICE_IS_CAPTURE,
    TRANS_ID_GET_SNAPSHOT_BY_PICKER,
    TRANS_ID_SWITCH_USER,
    TRANS_ID_SET_VIRTUAL_SCREEN_BLACK_LIST,
    TRANS_ID_DISABLE_POWEROFF_RENDER_CONTROL,
    TRANS_ID_PROXY_FOR_FREEZE,
    TRANS_ID_RESET_ALL_FREEZE_STATUS,
    TRANS_ID_NOTIFY_DISPLAY_HOOK_INFO,
    TRANS_ID_GET_ALL_PHYSICAL_DISPLAY_RESOLUTION,
    TRANS_ID_SET_VIRTUAL_SCREEN_STATUS,
    TRANS_ID_SET_VIRTUAL_SCREEN_SECURITY_EXEMPTION,
    TRANS_ID_SET_VIRTUAL_SCREEN_MAX_REFRESHRATE,
    TRANS_ID_GET_DISPLAY_CAPTURE,
    TRANS_ID_GET_PRIMARY_DISPLAY_INFO,
    TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION,
    TRANS_ID_SET_CAMERA_STATUS,
    TRANS_ID_SET_SCREEN_ON_DELAY_TIME,
    TRANS_ID_GET_SCREEN_COMBINATION,
    TRANS_ID_GET_SCREEN_POWER_AUTO,
    TRANS_ID_SET_SCREEN_SKIP_PROTECTED_WINDOW,
    TRANS_ID_IS_ORIENTATION_NEED_CHANGE,
    TRANS_ID_RECORD_EVENT_FROM_SCB,
    TRANS_ID_UPDATE_SUPER_FOLD_EXPAND_AVAILABLE_AREA,
    TRANS_ID_GET_EXPAND_AVAILABLE_AREA,
};
}
#endif // FOUNDATION_DMSERVER_DISPLAY_MANAGER_INTERFACE_CODE_H
