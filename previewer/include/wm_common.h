/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_WM_COMMON_H
#define OHOS_ROSEN_WM_COMMON_H

#include <cmath>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <variant>

#include <float.h>

#include <parcel.h>
#include "dm_common.h"
#include "securec.h"
#include "wm_animation_common.h"
#include "wm_type.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t DEFAULT_SPACING_BETWEEN_BUTTONS = 12;
constexpr uint32_t DEFAULT_BUTTON_BACKGROUND_SIZE = 28;
constexpr uint32_t DEFAULT_CLOSE_BUTTON_RIGHT_MARGIN = 20;
constexpr uint32_t DEFAULT_BUTTON_ICON_SIZE = 20;
constexpr uint32_t DEFAULT_BUTTON_BACKGROUND_CORNER_RADIUS = 4;
constexpr int32_t DEFAULT_COLOR_MODE = -1;
constexpr int32_t MIN_COLOR_MODE = -1;
constexpr int32_t MAX_COLOR_MODE = 1;
constexpr int32_t LIGHT_COLOR_MODE = 0;
constexpr int32_t DARK_COLOR_MODE = 1;
constexpr uint32_t MIN_SPACING_BETWEEN_BUTTONS = 8;
constexpr uint32_t MAX_SPACING_BETWEEN_BUTTONS = 24;
constexpr uint32_t MIN_BUTTON_BACKGROUND_SIZE = 20;
constexpr uint32_t MAX_BUTTON_BACKGROUND_SIZE = 40;
constexpr uint32_t MIN_CLOSE_BUTTON_RIGHT_MARGIN = 6;
constexpr uint32_t MAX_CLOSE_BUTTON_RIGHT_MARGIN = 22;
constexpr uint32_t MIN_BUTTON_ICON_SIZE = 16;
constexpr uint32_t MAX_BUTTON_ICON_SIZE = 24;
constexpr uint32_t MIN_BUTTON_BACKGROUND_CORNER_RADIUS = 4;
constexpr uint32_t MAX_BUTTON_BACKGROUND_CORNER_RADIUS = 8;
constexpr int32_t MINIMUM_Z_LEVEL = -10000;
constexpr int32_t MAXIMUM_Z_LEVEL = 10000;
constexpr int32_t SPECIFIC_ZINDEX_INVALID = -1;
constexpr double POS_ZERO = 0.001f;
}

/**
 * @brief Enumerates type of window
 */
enum class WindowType : uint32_t {
    APP_WINDOW_BASE = 1,
    APP_MAIN_WINDOW_BASE = APP_WINDOW_BASE,
    WINDOW_TYPE_APP_MAIN_WINDOW = APP_MAIN_WINDOW_BASE,
    APP_MAIN_WINDOW_END,

    APP_SUB_WINDOW_BASE = 1000,
    WINDOW_TYPE_MEDIA = APP_SUB_WINDOW_BASE,
    WINDOW_TYPE_APP_SUB_WINDOW,
    WINDOW_TYPE_APP_COMPONENT,
    APP_SUB_WINDOW_END,
    APP_WINDOW_END = APP_SUB_WINDOW_END,

    SYSTEM_WINDOW_BASE = 2000,
    BELOW_APP_SYSTEM_WINDOW_BASE = SYSTEM_WINDOW_BASE,
    WINDOW_TYPE_WALLPAPER = SYSTEM_WINDOW_BASE,
    WINDOW_TYPE_DESKTOP,
    BELOW_APP_SYSTEM_WINDOW_END,

    ABOVE_APP_SYSTEM_WINDOW_BASE = 2100,
    WINDOW_TYPE_APP_LAUNCHING = ABOVE_APP_SYSTEM_WINDOW_BASE,
    WINDOW_TYPE_DOCK_SLICE,
    WINDOW_TYPE_INCOMING_CALL,
    WINDOW_TYPE_SEARCHING_BAR,
    WINDOW_TYPE_SYSTEM_ALARM_WINDOW,
    WINDOW_TYPE_INPUT_METHOD_FLOAT,
    WINDOW_TYPE_FLOAT,
    WINDOW_TYPE_TOAST,
    WINDOW_TYPE_STATUS_BAR,
    WINDOW_TYPE_PANEL,
    WINDOW_TYPE_KEYGUARD,
    WINDOW_TYPE_VOLUME_OVERLAY,
    WINDOW_TYPE_NAVIGATION_BAR,
    WINDOW_TYPE_DRAGGING_EFFECT,
    WINDOW_TYPE_POINTER,
    WINDOW_TYPE_LAUNCHER_RECENT,
    WINDOW_TYPE_LAUNCHER_DOCK,
    WINDOW_TYPE_BOOT_ANIMATION,
    WINDOW_TYPE_FREEZE_DISPLAY,
    WINDOW_TYPE_VOICE_INTERACTION,
    WINDOW_TYPE_FLOAT_CAMERA,
    WINDOW_TYPE_PLACEHOLDER,
    WINDOW_TYPE_DIALOG,
    WINDOW_TYPE_SCREENSHOT,
    WINDOW_TYPE_INPUT_METHOD_STATUS_BAR,
    WINDOW_TYPE_GLOBAL_SEARCH,
    WINDOW_TYPE_SYSTEM_TOAST,
    WINDOW_TYPE_SYSTEM_FLOAT,
    WINDOW_TYPE_PIP,
    WINDOW_TYPE_THEME_EDITOR,
    WINDOW_TYPE_NAVIGATION_INDICATOR,
    WINDOW_TYPE_HANDWRITE,
    WINDOW_TYPE_SCENE_BOARD,
    WINDOW_TYPE_KEYBOARD_PANEL,
    WINDOW_TYPE_WALLET_SWIPE_CARD,
    WINDOW_TYPE_SCREEN_CONTROL,
    WINDOW_TYPE_FLOAT_NAVIGATION,
    WINDOW_TYPE_MUTISCREEN_COLLABORATION,
    WINDOW_TYPE_DYNAMIC,
    WINDOW_TYPE_MAGNIFICATION,
    WINDOW_TYPE_MAGNIFICATION_MENU,
    WINDOW_TYPE_SELECTION,
    WINDOW_TYPE_FB,
    ABOVE_APP_SYSTEM_WINDOW_END,

    SYSTEM_SUB_WINDOW_BASE = 2500,
    WINDOW_TYPE_SYSTEM_SUB_WINDOW = SYSTEM_SUB_WINDOW_BASE,
    SYSTEM_SUB_WINDOW_END,

    SYSTEM_WINDOW_END = SYSTEM_SUB_WINDOW_END,

    WINDOW_TYPE_UI_EXTENSION = 3000
};

/**
 * @brief Enumerates state of window.
 */
enum class WindowState : uint32_t {
    STATE_INITIAL,
    STATE_CREATED,
    STATE_SHOWN,
    STATE_HIDDEN,
    STATE_FROZEN,
    STATE_UNFROZEN,
    STATE_DESTROYED,
    STATE_BOTTOM = STATE_DESTROYED // Add state type after STATE_DESTROYED is not allowed.
};

/**
 * @brief Enumerates blur style of window.
 */
enum class WindowBlurStyle : uint32_t {
    WINDOW_BLUR_OFF = 0,
    WINDOW_BLUR_THIN,
    WINDOW_BLUR_REGULAR,
    WINDOW_BLUR_THICK
};

/**
 * @brief Enumerates mode supported of window.
 */
enum WindowModeSupport : uint32_t {
    WINDOW_MODE_SUPPORT_FULLSCREEN = 1 << 0,
    WINDOW_MODE_SUPPORT_FLOATING = 1 << 1,
    WINDOW_MODE_SUPPORT_SPLIT_PRIMARY = 1 << 2,
    WINDOW_MODE_SUPPORT_SPLIT_SECONDARY = 1 << 3,
    WINDOW_MODE_SUPPORT_PIP = 1 << 4,
    WINDOW_MODE_SUPPORT_FB = 1 << 5,
    WINDOW_MODE_SUPPORT_ALL = WINDOW_MODE_SUPPORT_FLOATING |
                              WINDOW_MODE_SUPPORT_FULLSCREEN |
                              WINDOW_MODE_SUPPORT_SPLIT_PRIMARY |
                              WINDOW_MODE_SUPPORT_SPLIT_SECONDARY |
                              WINDOW_MODE_SUPPORT_PIP |
                              WINDOW_MODE_SUPPORT_FB
};

/**
 * @brief Enumerates mode of window.
 */
enum class WindowMode : uint32_t {
    WINDOW_MODE_UNDEFINED = 0,
    WINDOW_MODE_FULLSCREEN = 1,
    WINDOW_MODE_SPLIT_PRIMARY = 100,
    WINDOW_MODE_SPLIT_SECONDARY,
    WINDOW_MODE_FLOATING,
    WINDOW_MODE_PIP,
    WINDOW_MODE_FB,
};

/**
 * @brief Enumerates global mode of window.
 */
enum class GlobalWindowMode : uint32_t {
    UNKNOWN = 0,
    FULLSCREEN = 1,
    SPLIT = 1 << 1,
    FLOAT = 1 << 2,
    PIP = 1 << 3,
    ALL = FULLSCREEN | SPLIT | FLOAT | PIP
};

inline GlobalWindowMode operator|(GlobalWindowMode lhs, GlobalWindowMode rhs)
{
    return static_cast<GlobalWindowMode>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

/**
 * @brief Enumerates status of window.
 */
enum class WindowStatus : uint32_t {
    WINDOW_STATUS_UNDEFINED = 0,
    WINDOW_STATUS_FULLSCREEN = 1,
    WINDOW_STATUS_MAXIMIZE,
    WINDOW_STATUS_MINIMIZE,
    WINDOW_STATUS_FLOATING,
    WINDOW_STATUS_SPLITSCREEN
};

/**
 * @brief Enumerates error code of window.
 */
enum class WMError : int32_t {
    WM_OK = 0,
    WM_DO_NOTHING,
    WM_ERROR_NO_MEM,
    WM_ERROR_DESTROYED_OBJECT,
    WM_ERROR_INVALID_WINDOW,
    WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE,
    WM_ERROR_INVALID_OPERATION,
    WM_ERROR_INVALID_PERMISSION,
    WM_ERROR_NOT_SYSTEM_APP,
    WM_ERROR_NO_REMOTE_ANIMATION,
    WM_ERROR_INVALID_DISPLAY,
    WM_ERROR_INVALID_PARENT,
    WM_ERROR_INVALID_OP_IN_CUR_STATUS,
    WM_ERROR_REPEAT_OPERATION,
    WM_ERROR_INVALID_SESSION,
    WM_ERROR_INVALID_CALLING,
    WM_ERROR_SYSTEM_ABNORMALLY,

    WM_ERROR_DEVICE_NOT_SUPPORT = 801, // the value do not change.It is defined on all system.

    WM_ERROR_NEED_REPORT_BASE = 1000, // error code > 1000 means need report.
    WM_ERROR_NULLPTR,
    WM_ERROR_INVALID_TYPE,
    WM_ERROR_INVALID_PARAM,
    WM_ERROR_SAMGR,
    WM_ERROR_IPC_FAILED,
    WM_ERROR_NEED_REPORT_END,
    WM_ERROR_START_ABILITY_FAILED,
    WM_ERROR_PIP_DESTROY_FAILED,
    WM_ERROR_PIP_STATE_ABNORMALLY,
    WM_ERROR_PIP_CREATE_FAILED,
    WM_ERROR_PIP_INTERNAL_ERROR,
    WM_ERROR_PIP_REPEAT_OPERATION,
    WM_ERROR_ILLEGAL_PARAM,
    WM_ERROR_UI_EFFECT_ERROR,
    WM_ERROR_TIMEOUT,
    WM_ERROR_FB_PARAM_INVALID,
    WM_ERROR_FB_CREATE_FAILED,
    WM_ERROR_FB_REPEAT_CONTROLLER,
    WM_ERROR_FB_REPEAT_OPERATION,
    WM_ERROR_FB_INTERNAL_ERROR,
    WM_ERROR_FB_STATE_ABNORMALLY,
    WM_ERROR_FB_INVALID_STATE,
    WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED,
    WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED,
    WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED,
    WM_ERROR_INVALID_WINDOW_TYPE,
};

/**
 * @brief Enumerates error code of window only used for js api.
 */
enum class WmErrorCode : int32_t {
    WM_OK = 0,
    WM_ERROR_NO_PERMISSION = 201,
    WM_ERROR_NOT_SYSTEM_APP = 202,
    WM_ERROR_INVALID_PARAM = 401,
    WM_ERROR_DEVICE_NOT_SUPPORT = 801,

    WM_ERROR_REPEAT_OPERATION = 1300001,
    WM_ERROR_STATE_ABNORMALLY = 1300002,
    WM_ERROR_SYSTEM_ABNORMALLY = 1300003,
    WM_ERROR_INVALID_CALLING = 1300004,
    WM_ERROR_STAGE_ABNORMALLY = 1300005,
    WM_ERROR_CONTEXT_ABNORMALLY = 1300006,
    WM_ERROR_START_ABILITY_FAILED = 1300007,
    WM_ERROR_INVALID_DISPLAY = 1300008,
    WM_ERROR_INVALID_PARENT = 1300009,
    WM_ERROR_INVALID_OP_IN_CUR_STATUS = 1300010,
    WM_ERROR_PIP_DESTROY_FAILED = 1300011,
    WM_ERROR_PIP_STATE_ABNORMALLY = 1300012,
    WM_ERROR_PIP_CREATE_FAILED = 1300013,
    WM_ERROR_PIP_INTERNAL_ERROR = 1300014,
    WM_ERROR_PIP_REPEAT_OPERATION = 1300015,
    WM_ERROR_ILLEGAL_PARAM = 1300016,
    WM_ERROR_UI_EFFECT_ERROR = 1300017,
    WM_ERROR_TIMEOUT = 1300018,
    WM_ERROR_FB_PARAM_INVALID = 1300019,
    WM_ERROR_FB_CREATE_FAILED = 1300020,
    WM_ERROR_FB_REPEAT_CONTROLLER = 1300021,
    WM_ERROR_FB_REPEAT_OPERATION = 1300022,
    WM_ERROR_FB_INTERNAL_ERROR = 1300023,
    WM_ERROR_FB_STATE_ABNORMALLY = 1300024,
    WM_ERROR_FB_INVALID_STATE = 1300025,
    WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED = 1300026,
    WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED = 1300027,
    WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED = 1300028,
    WM_ERROR_INVALID_WINDOW_TYPE = 1300029,
};

/**
 * @brief Enumerates setting flag of systemStatusBar.
 */
enum class SystemBarSettingFlag : uint32_t {
    DEFAULT_SETTING = 0,
    COLOR_SETTING = 1,
    ENABLE_SETTING = 1 << 1,
    ALL_SETTING = 0b11
};

inline SystemBarSettingFlag operator|(SystemBarSettingFlag lhs, SystemBarSettingFlag rhs)
{
    using T = std::underlying_type_t<SystemBarSettingFlag>;
    return static_cast<SystemBarSettingFlag>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline SystemBarSettingFlag& operator|=
    (SystemBarSettingFlag& lhs, SystemBarSettingFlag rhs) { return lhs = lhs | rhs; }

/**
 * @brief Enumerates flag of window.
 */
enum class WindowFlag : uint32_t {
    WINDOW_FLAG_NEED_AVOID = 1,
    WINDOW_FLAG_PARENT_LIMIT = 1 << 1,
    WINDOW_FLAG_SHOW_WHEN_LOCKED = 1 << 2,
    WINDOW_FLAG_FORBID_SPLIT_MOVE = 1 << 3,
    WINDOW_FLAG_WATER_MARK = 1 << 4,
    WINDOW_FLAG_IS_MODAL = 1 << 5,
    WINDOW_FLAG_HANDWRITING = 1 << 6,
    WINDOW_FLAG_IS_TOAST = 1 << 7,
    WINDOW_FLAG_IS_APPLICATION_MODAL = 1 << 8,
    WINDOW_FLAG_IS_TEXT_MENU = 1 << 9,
    WINDOW_FLAG_END = 1 << 10,
};

/**
 * @brief Enumerates system and app sub window avoid area options
 */
enum class AvoidAreaOption : uint32_t {
    ENABLE_SYSTEM_WINDOW = 1,
    ENABLE_APP_SUB_WINDOW = 1 << 1,
};

/**
 * @brief Enumerates flag of multiWindowUIType.
 */
enum class WindowUIType : uint8_t {
    PHONE_WINDOW = 0,
    PC_WINDOW,
    PAD_WINDOW,
    INVALID_WINDOW
};

/**
 * @brief Used to map from WMError to WmErrorCode.
 */
const std::map<WMError, WmErrorCode> WM_JS_TO_ERROR_CODE_MAP {
    {WMError::WM_OK,                                   WmErrorCode::WM_OK                             },
    {WMError::WM_DO_NOTHING,                           WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_DESTROYED_OBJECT,               WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_DEVICE_NOT_SUPPORT,             WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT       },
    {WMError::WM_ERROR_INVALID_OPERATION,              WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_INVALID_PARAM,                  WmErrorCode::WM_ERROR_INVALID_PARAM            },
    {WMError::WM_ERROR_INVALID_PERMISSION,             WmErrorCode::WM_ERROR_NO_PERMISSION            },
    {WMError::WM_ERROR_NOT_SYSTEM_APP,                 WmErrorCode::WM_ERROR_NOT_SYSTEM_APP           },
    {WMError::WM_ERROR_INVALID_TYPE,                   WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_INVALID_WINDOW,                 WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE,    WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_IPC_FAILED,                     WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY        },
    {WMError::WM_ERROR_NO_MEM,                         WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY        },
    {WMError::WM_ERROR_NO_REMOTE_ANIMATION,            WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY        },
    {WMError::WM_ERROR_INVALID_DISPLAY,                WmErrorCode::WM_ERROR_INVALID_DISPLAY          },
    {WMError::WM_ERROR_INVALID_PARENT,                 WmErrorCode::WM_ERROR_INVALID_PARENT           },
    {WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS,       WmErrorCode::WM_ERROR_INVALID_OP_IN_CUR_STATUS },
    {WMError::WM_ERROR_REPEAT_OPERATION,               WmErrorCode::WM_ERROR_REPEAT_OPERATION         },
    {WMError::WM_ERROR_NULLPTR,                        WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_SAMGR,                          WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY        },
    {WMError::WM_ERROR_START_ABILITY_FAILED,           WmErrorCode::WM_ERROR_START_ABILITY_FAILED     },
    {WMError::WM_ERROR_SYSTEM_ABNORMALLY,              WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY        },
    {WMError::WM_ERROR_TIMEOUT,                        WmErrorCode::WM_ERROR_TIMEOUT                  },
    {WMError::WM_ERROR_ILLEGAL_PARAM,                  WmErrorCode::WM_ERROR_ILLEGAL_PARAM            },
    {WMError::WM_ERROR_FB_PARAM_INVALID,               WmErrorCode::WM_ERROR_FB_PARAM_INVALID         },
    {WMError::WM_ERROR_FB_CREATE_FAILED,               WmErrorCode::WM_ERROR_FB_CREATE_FAILED         },
    {WMError::WM_ERROR_FB_REPEAT_CONTROLLER,           WmErrorCode::WM_ERROR_FB_REPEAT_CONTROLLER     },
    {WMError::WM_ERROR_FB_REPEAT_OPERATION,            WmErrorCode::WM_ERROR_FB_REPEAT_OPERATION      },
    {WMError::WM_ERROR_FB_INTERNAL_ERROR,              WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR        },
    {WMError::WM_ERROR_FB_STATE_ABNORMALLY,            WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY      },
    {WMError::WM_ERROR_FB_INVALID_STATE,               WmErrorCode::WM_ERROR_FB_INVALID_STATE         },
    {WMError::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED,  WmErrorCode::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED  },
    {WMError::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED, WmErrorCode::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED  },
    {WMError::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED,  WmErrorCode::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED  },
    {WMError::WM_ERROR_UI_EFFECT_ERROR,                WmErrorCode::WM_ERROR_UI_EFFECT_ERROR          },
    {WMError::WM_ERROR_INVALID_CALLING,                WmErrorCode::WM_ERROR_INVALID_CALLING          },
};

/**
 * @brief Enumerates window size change reason.
 */
enum class WindowSizeChangeReason : uint32_t {
    UNDEFINED = 0,
    MAXIMIZE,
    RECOVER,
    ROTATION,
    DRAG,
    DRAG_START,
    DRAG_END,
    RESIZE,
    RESIZE_WITH_ANIMATION,
    MOVE,
    MOVE_WITH_ANIMATION,
    HIDE,
    TRANSFORM,
    CUSTOM_ANIMATION_SHOW,
    FULL_TO_SPLIT,
    SPLIT_TO_FULL,
    FULL_TO_FLOATING,
    FLOATING_TO_FULL,
    PIP_START,
    PIP_SHOW,
    PIP_AUTO_START,
    PIP_RATIO_CHANGE,
    PIP_RESTORE,
    UPDATE_DPI_SYNC,
    DRAG_MOVE,
    AVOID_AREA_CHANGE,
    MAXIMIZE_TO_SPLIT,
    SPLIT_TO_MAXIMIZE,
    PAGE_ROTATION,
    SPLIT_DRAG_START,
    SPLIT_DRAG,
    SPLIT_DRAG_END,
    RESIZE_BY_LIMIT,
    MAXIMIZE_IN_IMPLICT = 32,
    RECOVER_IN_IMPLICIT = 33,
    SCREEN_RELATIVE_POSITION_CHANGE,
    SNAPSHOT_ROTATION = 37,
    SCENE_WITH_ANIMATION,
    LS_STATE_CHANGE,
    END
};

/**
 * @brief Enumerates window gravity.
 */
enum class WindowGravity : uint32_t {
    WINDOW_GRAVITY_FLOAT = 0,
    WINDOW_GRAVITY_BOTTOM
};

/**
 * @brief Enumerates window session type.
 */
enum class WindowSessionType : uint32_t {
    SCENE_SESSION = 0,
    EXTENSION_SESSION = 1
};

/**
 * @brief Enumerates window tag.
 */
enum class WindowTag : uint32_t {
    MAIN_WINDOW = 0,
    SUB_WINDOW = 1,
    SYSTEM_WINDOW = 2
};

/**
 * @brief Enumerates drag event.
 */
enum class DragEvent : uint32_t {
    DRAG_EVENT_IN  = 1,
    DRAG_EVENT_OUT,
    DRAG_EVENT_MOVE,
    DRAG_EVENT_END
};

/**
 * @brief Enumerates drag resize type.
 */
enum class DragResizeType : uint32_t {
    RESIZE_TYPE_UNDEFINED = 0,
    RESIZE_EACH_FRAME = 1,
    RESIZE_WHEN_DRAG_END = 2,
    RESIZE_KEY_FRAME = 3,
    RESIZE_MAX_VALUE,  // invalid value begin, add new value above
};

/**
 * @struct KeyFramePolicy
 *
 * @brief info for drag key frame policy.
 */
struct KeyFramePolicy : public Parcelable {
    DragResizeType dragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    uint32_t interval_ = 1000;
    uint32_t distance_ = 1000;
    uint32_t animationDuration_ = 100;
    uint32_t animationDelay_ = 100;
    bool running_ = false;
    bool stopping_ = false;

    bool enabled() const
    {
        return dragResizeType_ == DragResizeType::RESIZE_KEY_FRAME;
    }

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteUint32(static_cast<uint32_t>(dragResizeType_)) &&
            parcel.WriteUint32(interval_) && parcel.WriteUint32(distance_) &&
            parcel.WriteUint32(animationDuration_) && parcel.WriteUint32(animationDelay_) &&
            parcel.WriteBool(running_) && parcel.WriteBool(stopping_);
    }

    static KeyFramePolicy* Unmarshalling(Parcel& parcel)
    {
        KeyFramePolicy* keyFramePolicy = new KeyFramePolicy();
        uint32_t dragResizeType;
        if (!parcel.ReadUint32(dragResizeType) || !parcel.ReadUint32(keyFramePolicy->interval_) ||
            !parcel.ReadUint32(keyFramePolicy->distance_) || !parcel.ReadUint32(keyFramePolicy->animationDuration_) ||
            !parcel.ReadUint32(keyFramePolicy->animationDelay_) || !parcel.ReadBool(keyFramePolicy->running_) ||
            !parcel.ReadBool(keyFramePolicy->stopping_)) {
            delete keyFramePolicy;
            return nullptr;
        }
        if (dragResizeType >= static_cast<uint32_t>(DragResizeType::RESIZE_MAX_VALUE)) {
            delete keyFramePolicy;
            return nullptr;
        }
        keyFramePolicy->dragResizeType_ = static_cast<DragResizeType>(dragResizeType);
        return keyFramePolicy;
    }

    inline std::string ToString() const
    {
        std::ostringstream oss;
        oss << "[" << static_cast<uint32_t>(dragResizeType_) << " " << interval_ << " " << distance_;
        oss << " " << animationDuration_ << " " << animationDelay_ << "]";
        return oss.str();
    }
};

/**
 * @struct HookWindowInfo.
 *
 * @brief Configures window hook behavior based on window size ratios.
 */
struct HookWindowInfo : public Parcelable {
    bool enableHookWindow{ false };
    float widthHookRatio{ 1.0f };

    static constexpr float DEFAULT_WINDOW_SIZE_HOOK_RATIO = 1.0f;

    bool Marshalling(Parcel& parcel) const override
    {
        return WriteAllFields(parcel);
    }

    static HookWindowInfo* Unmarshalling(Parcel& parcel)
    {
        auto hookWindowInfo = std::make_unique<HookWindowInfo>();
        if (!hookWindowInfo || !ReadAllFields(parcel, *hookWindowInfo)) {
            return nullptr;
        }
        return hookWindowInfo.release();
    }

    std::string ToString() const
    {
        constexpr int precision = 6; // Print float with precision of 6 decimal places.
        std::ostringstream oss;
        oss << std::boolalpha  // For true/false instead of 1/0
            << "enableHookWindow: " << enableHookWindow
            << ", widthHookRatio: " << std::fixed << std::setprecision(precision) << widthHookRatio;
        return oss.str();
    }

private:
    bool WriteAllFields(Parcel& parcel) const
    {
        return parcel.WriteBool(enableHookWindow) &&
               parcel.WriteFloat(widthHookRatio);
    }

    static bool ReadAllFields(Parcel& parcel, HookWindowInfo& info)
    {
        return parcel.ReadBool(info.enableHookWindow) &&
               parcel.ReadFloat(info.widthHookRatio);
    }
};

/**
 * @brief Enumerates layout mode of window.
 */
enum class WindowLayoutMode : uint32_t {
    BASE = 0,
    CASCADE = BASE,
    TILE = 1,
    END,
};

namespace {
constexpr uint32_t SYSTEM_COLOR_WHITE = 0xE5FFFFFF;
constexpr uint32_t SYSTEM_COLOR_BLACK = 0x66000000;
constexpr float UNDEFINED_BRIGHTNESS = -1.0f;
constexpr float MINIMUM_BRIGHTNESS = 0.0f;
constexpr float MAXIMUM_BRIGHTNESS = 1.0f;

constexpr uint32_t INVALID_WINDOW_ID = 0;
constexpr int32_t INVALID_PID = -1;
constexpr int32_t INVALID_UID = -1;

constexpr float UNDEFINED_DENSITY = -1.0f;
constexpr float MINIMUM_CUSTOM_DENSITY = 0.5f;
constexpr float MAXIMUM_CUSTOM_DENSITY = 4.0f;
}

/**
 * @struct PointInfo.
 *
 * @brief Point info.
 */
struct PointInfo {
    int32_t x;
    int32_t y;
};

/**
@struct MainWindowInfo.
@brief topN main window info.
*/
struct MainWindowInfo : public Parcelable {
    virtual bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteInt32(pid_) && parcel.WriteString(bundleName_) &&
            parcel.WriteInt32(persistentId_) && parcel.WriteInt32(bundleType_) &&
            parcel.WriteUint64(displayId_) && parcel.WriteBool(showing_) && parcel.WriteString(label_);
    }

    static MainWindowInfo* Unmarshalling(Parcel& parcel)
    {
        MainWindowInfo* mainWindowInfo = new MainWindowInfo;
        if (!parcel.ReadInt32(mainWindowInfo->pid_) || !parcel.ReadString(mainWindowInfo->bundleName_) ||
            !parcel.ReadInt32(mainWindowInfo->persistentId_) || !parcel.ReadInt32(mainWindowInfo->bundleType_) ||
            !parcel.ReadUint64(mainWindowInfo->displayId_) || !parcel.ReadBool(mainWindowInfo->showing_) ||
            !parcel.ReadString(mainWindowInfo->label_)) {
            delete mainWindowInfo;
            return nullptr;
        }
        return mainWindowInfo;
    }

    int32_t pid_ = 0;
    std::string bundleName_ = "";
    int32_t persistentId_ = 0;
    int32_t bundleType_ = 0;
    DisplayId displayId_ = DISPLAY_ID_INVALID;
    bool showing_ = false;
    std::string label_ = "";
};

/**
 * @class Transform
 *
 * @brief parameter of transform and rotate.
 */
class Transform {
public:
    Transform()
        : pivotX_(0.5f), pivotY_(0.5f), scaleX_(1.f), scaleY_(1.f), scaleZ_(1.f), rotationX_(0.f),
          rotationY_(0.f), rotationZ_(0.f), translateX_(0.f), translateY_(0.f), translateZ_(0.f)
    {}
    ~Transform() {}

    bool operator==(const Transform& right) const
    {
        return NearZero(scaleX_ - right.scaleX_) &&
            NearZero(scaleY_ - right.scaleY_) &&
            NearZero(scaleZ_ - right.scaleZ_) &&
            NearZero(pivotX_ - right.pivotX_) &&
            NearZero(pivotY_ - right.pivotY_) &&
            NearZero(translateX_ - right.translateX_) &&
            NearZero(translateY_ - right.translateY_) &&
            NearZero(translateZ_ - right.translateZ_) &&
            NearZero(rotationX_ - right.rotationX_) &&
            NearZero(rotationY_ - right.rotationY_) &&
            NearZero(rotationZ_ - right.rotationZ_);
    }

    bool operator!=(const Transform& right) const
    {
        return !(*this == right);
    }

    static const Transform& Identity()
    {
        static Transform I;
        return I;
    }

    float pivotX_;
    float pivotY_;
    float scaleX_;
    float scaleY_;
    float scaleZ_;
    float rotationX_;
    float rotationY_;
    float rotationZ_;
    float translateX_;
    float translateY_;
    float translateZ_;

    void Unmarshalling(Parcel& parcel)
    {
        pivotX_ = parcel.ReadFloat();
        pivotY_ = parcel.ReadFloat();
        scaleX_ = parcel.ReadFloat();
        scaleY_ = parcel.ReadFloat();
        scaleZ_ = parcel.ReadFloat();
        rotationX_ = parcel.ReadFloat();
        rotationY_ = parcel.ReadFloat();
        rotationZ_ = parcel.ReadFloat();
        translateX_ = parcel.ReadFloat();
        translateY_ = parcel.ReadFloat();
        translateZ_ = parcel.ReadFloat();
    }

    bool Marshalling(Parcel& parcel) const
    {
        return parcel.WriteFloat(pivotX_) && parcel.WriteFloat(pivotY_) &&
               parcel.WriteFloat(scaleX_) && parcel.WriteFloat(scaleY_) && parcel.WriteFloat(scaleZ_) &&
               parcel.WriteFloat(rotationX_) && parcel.WriteFloat(rotationY_) && parcel.WriteFloat(rotationZ_) &&
               parcel.WriteFloat(translateX_) && parcel.WriteFloat(translateY_) && parcel.WriteFloat(translateZ_);
    }

private:
    static inline bool NearZero(float val)
    {
        return -0.001f < val && val < 0.001f;
    }
};

/**
 * @struct RectAnimationConfig
 *
 * @brief Window RectAnimationConfig
 */
struct RectAnimationConfig {
    uint32_t duration = 0; // Duartion of the animation, in milliseconds.
    float x1 = 0.0f;       // X coordinate of the first point on the Bezier curve.
    float y1 = 0.0f;       // Y coordinate of the first point on the Bezier curve.
    float x2 = 0.0f;       // X coordinate of the second point on the Bezier curve.
    float y2 = 0.0f;       // Y coordinate of the second point on the Bezier curve.
};

/**
 * @struct SystemBarPropertyFlag
 *
 * @brief Flag of system bar
 */
struct SystemBarPropertyFlag {
    bool enableFlag = false;
    bool backgroundColorFlag = false;
    bool contentColorFlag = false;
    bool enableAnimationFlag = false;
};

/**
 * @struct Rect
 *
 * @brief Window Rect.
 */
struct Rect {
    int32_t posX_;
    int32_t posY_;
    uint32_t width_;
    uint32_t height_;

    bool operator==(const Rect& a) const
    {
        return (posX_ == a.posX_ && posY_ == a.posY_ && width_ == a.width_ && height_ == a.height_);
    }

    bool operator!=(const Rect& a) const
    {
        return !this->operator==(a);
    }

    bool IsInsideOf(const Rect& a) const
    {
        return (posX_ >= a.posX_ && posY_ >= a.posY_ &&
            posX_ + width_ <= a.posX_ + a.width_ && posY_ + height_ <= a.posY_ + a.height_);
    }

    bool IsUninitializedRect() const
    {
        return (posX_ == 0 && posY_ == 0 && width_ == 0 && height_ == 0);
    }

    bool IsUninitializedSize() const
    {
        return width_ == 0 && height_ == 0;
    }

    bool IsSamePosition(int32_t x, int32_t y) const
    {
        return posX_ == x && posY_ == y;
    }

    inline std::string ToString() const
    {
        std::ostringstream oss;
        oss << "[" << posX_ << " " << posY_ << " " << width_ << " " << height_ << "]";
        return oss.str();
    }

    bool Marshalling(Parcel& parcel) const
    {
        return parcel.WriteInt32(posX_) && parcel.WriteInt32(posY_) &&
               parcel.WriteUint32(width_) && parcel.WriteUint32(height_);
    }

    void Unmarshalling(Parcel& parcel)
    {
        posX_ = parcel.ReadInt32();
        posY_ = parcel.ReadInt32();
        width_ = parcel.ReadUint32();
        height_ = parcel.ReadUint32();
    }

    static const Rect EMPTY_RECT;

    /**
     * @brief Checks whether the right-bottom corner of a rectangle stays within the valid range.
     *
     * @param x The x-coordinate of the left-top corner.
     * @param y The y-coordinate of the left-top corner.
     * @param width The rectangle's width.
     * @param height The rectangle's height.
     * @return true if right-bottom corner stays within int32_t range; false if overflow happens.
     */
    static bool IsRightBottomValid(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        int64_t right = static_cast<int64_t>(x) + static_cast<int64_t>(width);
        int64_t bottom = static_cast<int64_t>(y) + static_cast<int64_t>(height);
        return right <= INT32_MAX && bottom <= INT32_MAX;
    }
};

inline constexpr Rect Rect::EMPTY_RECT { 0, 0, 0, 0 };

/**
 * @struct SystemBarProperty
 *
 * @brief Property of system bar.
 */
struct SystemBarProperty {
    bool enable_;
    uint32_t backgroundColor_;
    uint32_t contentColor_;
    bool enableAnimation_;
    SystemBarSettingFlag settingFlag_;
    SystemBarProperty() : enable_(true), backgroundColor_(SYSTEM_COLOR_BLACK), contentColor_(SYSTEM_COLOR_WHITE),
                          enableAnimation_(false), settingFlag_(SystemBarSettingFlag::DEFAULT_SETTING) {}
    SystemBarProperty(bool enable, uint32_t background, uint32_t content, bool enableAnimation)
        : enable_(enable), backgroundColor_(background), contentColor_(content), enableAnimation_(enableAnimation),
          settingFlag_(SystemBarSettingFlag::DEFAULT_SETTING) {}
    SystemBarProperty(bool enable, uint32_t background, uint32_t content)
        : enable_(enable), backgroundColor_(background), contentColor_(content), enableAnimation_(false),
          settingFlag_(SystemBarSettingFlag::DEFAULT_SETTING) {}
    SystemBarProperty(bool enable, uint32_t background, uint32_t content,
                      bool enableAnimation, SystemBarSettingFlag settingFlag)
        : enable_(enable), backgroundColor_(background), contentColor_(content), enableAnimation_(enableAnimation),
          settingFlag_(settingFlag) {}

    bool operator == (const SystemBarProperty& a) const
    {
        return (enable_ == a.enable_ && backgroundColor_ == a.backgroundColor_ && contentColor_ == a.contentColor_ &&
            enableAnimation_ == a.enableAnimation_);
    }
};

/**
 * @struct WindowDensityInfo
 *
 * @brief Currently available density
 */
struct WindowDensityInfo {
    float systemDensity = UNDEFINED_DENSITY;
    float defaultDensity = UNDEFINED_DENSITY;
    float customDensity = UNDEFINED_DENSITY;

    std::string ToString() const
    {
        std::string str;
        constexpr int BUFFER_SIZE = 64;
        char buffer[BUFFER_SIZE] = { 0 };
        if (snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1,
            "[%f, %f, %f]", systemDensity, defaultDensity, customDensity) > 0) {
            str.append(buffer);
        }
        return str;
    }
};

/**
 * @struct WindowPropertyInfo
 *
 * @brief Currently available window property
 */
struct WindowPropertyInfo {
    Rect windowRect { 0, 0, 0, 0 };
    Rect drawableRect { 0, 0, 0, 0 };
    Rect globalDisplayRect { 0, 0, 0, 0 };
    uint32_t apiCompatibleVersion = 0;
    WindowType type = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    bool isLayoutFullScreen = false;
    bool isFullScreen = false;
    bool isTouchable = false;
    bool isFocusable = false;
    std::string name;
    bool isPrivacyMode = false;
    bool isKeepScreenOn = false;
    float brightness = 0.0f;
    bool isTransparent = false;
    bool isRoundCorner = false;
    float dimBehindValue = 0.0f;
    uint32_t id = INVALID_WINDOW_ID;
    DisplayId displayId = DISPLAY_ID_INVALID;
};

/**
 * @brief Enumerates avoid area type.
 */
enum class AvoidAreaType : uint32_t {
    TYPE_START = 0,
    TYPE_SYSTEM = TYPE_START,           // area of SystemUI
    TYPE_CUTOUT,                        // cutout of screen
    TYPE_SYSTEM_GESTURE,                // area for system gesture
    TYPE_KEYBOARD,                      // area for soft input keyboard
    TYPE_NAVIGATION_INDICATOR,          // area for navigation indicator
    TYPE_END,
};

/**
 * @brief Enumerates color space.
 */
enum class ColorSpace : uint32_t {
    COLOR_SPACE_DEFAULT = 0, // Default color space.
    COLOR_SPACE_WIDE_GAMUT,  // Wide gamut color space. The specific wide color gamut depends on the screen.
};

/**
 * @brief Enumerates occupied area type.
 */
enum class OccupiedAreaType : uint32_t {
    TYPE_INPUT, // area of input window
};

/**
 * @brief Enumerates window maximize mode.
 */
enum class MaximizeMode : uint32_t {
    MODE_AVOID_SYSTEM_BAR,
    MODE_FULL_FILL,
    MODE_RECOVER
};

/**
 * @brief Enumerates window animation.
 */
enum class WindowAnimation : uint32_t {
    NONE,
    DEFAULT,
    INPUTE,
    CUSTOM
};

/**
 * @brief Enumerates window anchor.
 */
enum class WindowAnchor : uint32_t {
    TOP_START = 0,
    TOP,
    TOP_END,
    START,
    CENTER,
    END,
    BOTTOM_START,
    BOTTOM,
    BOTTOM_END,
};

/**
 * @struct WindowAnchorInfo
 *
 * @brief Window anchor info
 */
struct WindowAnchorInfo : public Parcelable {
    bool isAnchorEnabled_ = false;
    WindowAnchor windowAnchor_ = WindowAnchor::TOP_START;
    int32_t offsetX_ = 0;
    int32_t offsetY_ = 0;

    WindowAnchorInfo() {}
    WindowAnchorInfo(bool isAnchorEnabled) : isAnchorEnabled_(isAnchorEnabled) {}
    WindowAnchorInfo(bool isAnchorEnabled, WindowAnchor windowAnchor, int32_t offsetX,
        int32_t offsetY) : isAnchorEnabled_(isAnchorEnabled),  windowAnchor_(windowAnchor),
        offsetX_(offsetX), offsetY_(offsetY) {}

    bool operator==(const WindowAnchorInfo& other) const
    {
        return isAnchorEnabled_ == other.isAnchorEnabled_ && windowAnchor_ == other.windowAnchor_ &&
            offsetX_ == other.offsetX_ && offsetY_ == other.offsetY_;
    }

    bool operator!=(const WindowAnchorInfo& other) const
    {
        return !(*this == other);
    }

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteBool(isAnchorEnabled_) && parcel.WriteUint32(static_cast<uint32_t>(windowAnchor_)) &&
            parcel.WriteInt32(offsetX_) && parcel.WriteInt32(offsetY_);
    }

    static WindowAnchorInfo* Unmarshalling(Parcel& parcel)
    {
        uint32_t windowAnchorMode = 0;
        WindowAnchorInfo* windowAnchorInfo = new(std::nothrow) WindowAnchorInfo();
        if (windowAnchorInfo == nullptr) {
            return nullptr;
        }
        if (!parcel.ReadBool(windowAnchorInfo->isAnchorEnabled_) || !parcel.ReadUint32(windowAnchorMode) ||
            !parcel.ReadInt32(windowAnchorInfo->offsetX_) || !parcel.ReadInt32(windowAnchorInfo->offsetY_)) {
            delete windowAnchorInfo;
            return nullptr;
        }
        windowAnchorInfo->windowAnchor_ = static_cast<WindowAnchor>(windowAnchorMode);
        return windowAnchorInfo;
    }
};

struct DecorButtonStyle {
    int32_t  colorMode = DEFAULT_COLOR_MODE;
    uint32_t spacingBetweenButtons = DEFAULT_SPACING_BETWEEN_BUTTONS;
    uint32_t closeButtonRightMargin = DEFAULT_CLOSE_BUTTON_RIGHT_MARGIN;
    uint32_t buttonBackgroundSize = DEFAULT_BUTTON_BACKGROUND_SIZE;
    uint32_t buttonIconSize = DEFAULT_BUTTON_ICON_SIZE;
    uint32_t buttonBackgroundCornerRadius = DEFAULT_BUTTON_BACKGROUND_CORNER_RADIUS;
};

/**
 * @class AvoidArea
 *
 * @brief Area needed to avoid.
 */
class AvoidArea : virtual public RefBase {
public:
    Rect topRect_ { 0, 0, 0, 0 };
    Rect leftRect_ { 0, 0, 0, 0 };
    Rect rightRect_ { 0, 0, 0, 0 };
    Rect bottomRect_ { 0, 0, 0, 0 };

    bool operator==(const AvoidArea& a) const
    {
        return (topRect_ == a.topRect_ && leftRect_ == a.leftRect_ &&
            rightRect_ == a.rightRect_ && bottomRect_ == a.bottomRect_);
    }

    bool operator!=(const AvoidArea& a) const
    {
        return !this->operator==(a);
    }

    static inline bool ReadParcel(Parcel& parcel, Rect& rect)
    {
        return parcel.ReadInt32(rect.posX_) && parcel.ReadInt32(rect.posY_) &&
            parcel.ReadUint32(rect.width_) && parcel.ReadUint32(rect.height_);
    }

    static inline bool WriteParcel(Parcel& parcel, const Rect& rect)
    {
        return parcel.WriteInt32(rect.posX_) && parcel.WriteInt32(rect.posY_) &&
            parcel.WriteUint32(rect.width_) && parcel.WriteUint32(rect.height_);
    }

    bool isEmptyAvoidArea() const
    {
        return topRect_.IsUninitializedRect() && leftRect_.IsUninitializedRect() &&
            rightRect_.IsUninitializedRect() && bottomRect_.IsUninitializedRect();
    }
};

/**
 * @struct VsyncCallback
 *
 * @brief Vsync callback
 */
struct VsyncCallback {
    OnCallback onCallback;
};

/**
 * @struct FrameMetrics
 *
 * @brief frame metrics info.
 */
struct FrameMetrics : public Parcelable {
    bool firstDrawFrame_ = false;
    uint64_t inputHandlingDuration_ = 0;
    uint64_t layoutMeasureDuration_ = 0;
    uint64_t vsyncTimestamp_ = 0;

    /**
     * @brief Marshalling FrameMetrics.
     *
     * @param parcel Package of FrameMetrics.
     * @return True means marshall success, false means marshall failed.
     */
    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteBool(firstDrawFrame_) &&
               parcel.WriteUint64(inputHandlingDuration_) &&
               parcel.WriteUint64(layoutMeasureDuration_) &&
               parcel.WriteUint64(vsyncTimestamp_);
    }

    /**
     * @brief Unmarshalling FrameMetrics.
     *
     * @param parcel Package of FrameMetrics.
     * @return FrameMetrics object.
     */
    static FrameMetrics* Unmarshalling(Parcel& parcel)
    {
        auto info = std::make_unique<FrameMetrics>();
        if (!parcel.ReadBool(info->firstDrawFrame_) ||
            !parcel.ReadUint64(info->inputHandlingDuration_) ||
            !parcel.ReadUint64(info->layoutMeasureDuration_) ||
            !parcel.ReadUint64(info->vsyncTimestamp_)) {
            return nullptr;
        }
        return info.release();
    }
};

/**
 * @brief Enumerates window update type.
 */
enum class WindowUpdateType : int32_t {
    WINDOW_UPDATE_ADDED = 1,
    WINDOW_UPDATE_REMOVED,
    WINDOW_UPDATE_FOCUSED,
    WINDOW_UPDATE_BOUNDS,
    WINDOW_UPDATE_ACTIVE,
    WINDOW_UPDATE_PROPERTY
};

/**
 * @brief Enumerates window size unit type.
 */
enum class PixelUnit : uint32_t {
    PX = 0, // Physical pixel
    VP = 1, // Virtual pixel
};

struct WindowLimits {
    uint32_t maxWidth_ = static_cast<uint32_t>(INT32_MAX); // The width and height are no larger than INT32_MAX.
    uint32_t maxHeight_ = static_cast<uint32_t>(INT32_MAX);
    uint32_t minWidth_ = 1; // The width and height of the window cannot be less than or equal to 0.
    uint32_t minHeight_ = 1;
    float maxRatio_ = FLT_MAX;
    float minRatio_ = 0.0f;
    float vpRatio_ = 1.0f;
    PixelUnit pixelUnit_ = PixelUnit::PX;

    WindowLimits() {}
    WindowLimits(uint32_t maxWidth, uint32_t maxHeight, uint32_t minWidth, uint32_t minHeight, float maxRatio,
        float minRatio) : maxWidth_(maxWidth), maxHeight_(maxHeight), minWidth_(minWidth), minHeight_(minHeight),
        maxRatio_(maxRatio), minRatio_(minRatio) {}
    WindowLimits(uint32_t maxWidth, uint32_t maxHeight, uint32_t minWidth, uint32_t minHeight, float maxRatio,
        float minRatio, float vpRatio) : maxWidth_(maxWidth), maxHeight_(maxHeight), minWidth_(minWidth),
        minHeight_(minHeight), maxRatio_(maxRatio), minRatio_(minRatio), vpRatio_(vpRatio) {}
    WindowLimits(uint32_t maxWidth, uint32_t maxHeight, uint32_t minWidth, uint32_t minHeight, float maxRatio,
        float minRatio, float vpRatio, PixelUnit pixelUnit) : maxWidth_(maxWidth), maxHeight_(maxHeight),
        minWidth_(minWidth), minHeight_(minHeight), maxRatio_(maxRatio), minRatio_(minRatio), vpRatio_(vpRatio),
        pixelUnit_(pixelUnit) {}

    void Clip(uint32_t clipWidth, uint32_t clipHeight)
    {
        auto safeSub = [](uint32_t base, uint32_t dec) -> uint32_t {
            return (dec >= base) ? 0 : base - dec;
        };

        minWidth_ = safeSub(minWidth_, clipWidth);
        maxWidth_ = safeSub(maxWidth_, clipWidth);
        minHeight_ = safeSub(minHeight_, clipHeight);
        maxHeight_ = safeSub(maxHeight_, clipHeight);
    }

    void Expand(uint32_t expandWidth, uint32_t expandHeight)
    {
        auto safeAdd = [](uint32_t base, uint32_t inc) -> uint32_t {
            return (base > UINT32_MAX - inc) ? UINT32_MAX : base + inc;
        };

        minWidth_ = safeAdd(minWidth_, expandWidth);
        maxWidth_ = safeAdd(maxWidth_, expandWidth);
        minHeight_ = safeAdd(minHeight_, expandHeight);
        maxHeight_ = safeAdd(maxHeight_, expandHeight);
    }

    bool IsEmpty() const
    {
        return (maxHeight_ == 0 || minHeight_ == 0 || maxWidth_ == 0 || minWidth_ == 0);
    }

    bool IsValid() const
    {
        return minWidth_ <= maxWidth_ && minHeight_ <= maxHeight_;
    }

    static const WindowLimits DEFAULT_VP_LIMITS()
    {
        return {
            static_cast<uint32_t>(INT32_MAX),  // maxWidth
            static_cast<uint32_t>(INT32_MAX),  // maxHeight
            1,                                 // minWidth
            1,                                 // minHeight
            FLT_MAX,                           // maxRatio
            0.0f,                              // minRatio
            1.0f,                              // vpRatio
            PixelUnit::VP                      // pixelUnit
        };
    }

    bool IsUninitialized() const
    {
        return (maxWidth_ == static_cast<uint32_t>(INT32_MAX) &&
                maxHeight_ == static_cast<uint32_t>(INT32_MAX) &&
                minWidth_ == 1 &&
                minHeight_ == 1);
    }

    std::string ToString() const
    {
        constexpr int precision = 6;
        std::ostringstream oss;
        oss << "[" << maxWidth_ << " " << maxHeight_ << " " << minWidth_ << " " << minHeight_
            << " " << std::fixed << std::setprecision(precision) << maxRatio_ << " " << minRatio_
            << " " << vpRatio_ << " " << static_cast<uint32_t>(pixelUnit_) << "]";
        return oss.str();
    }
};

/**
 * @struct TitleButtonRect
 *
 * @brief An area of title buttons relative to the upper right corner of the window.
 */
struct TitleButtonRect {
    int32_t posX_ = 0;
    int32_t posY_ = 0;
    uint32_t width_ = 0;
    uint32_t height_ = 0;

    bool operator==(const TitleButtonRect& a) const
    {
        return (posX_ == a.posX_ && posY_ == a.posY_ && width_ == a.width_ && height_ == a.height_);
    }

    bool operator!=(const TitleButtonRect& a) const
    {
        return !this->operator==(a);
    }

    void ResetRect()
    {
        posX_ = 0;
        posY_ = 0;
        width_ = 0;
        height_ = 0;
    }

    bool IsInsideOf(const TitleButtonRect& a) const
    {
        return (posX_ >= a.posX_ && posY_ >= a.posY_ &&
            posX_ + width_ <= a.posX_ + a.width_ && posY_ + height_ <= a.posY_ + a.height_);
    }

    bool IsUninitializedRect() const
    {
        return (posX_ == 0 && posY_ == 0 && width_ == 0 && height_ == 0);
    }
};

/**
 * @brief WindowInfo filter Option
 */
enum class WindowInfoFilterOption : WindowInfoFilterOptionDataType {
    ALL = 0,
    EXCLUDE_SYSTEM = 1,
    VISIBLE = 1 << 1,
    FOREGROUND = 1 << 2,
};

inline WindowInfoFilterOption operator|(WindowInfoFilterOption lhs, WindowInfoFilterOption rhs)
{
    return static_cast<WindowInfoFilterOption>(static_cast<WindowInfoFilterOptionDataType>(lhs) |
        static_cast<WindowInfoFilterOptionDataType>(rhs));
}

inline bool IsChosenWindowOption(WindowInfoFilterOption options, WindowInfoFilterOption option)
{
    return (static_cast<WindowInfoFilterOptionDataType>(options) &
        static_cast<WindowInfoFilterOptionDataType>(option)) != 0;
}

/**
 * @brief WindowInfo Type Option
 */
enum class WindowInfoTypeOption : WindowInfoTypeOptionDataType {
    WINDOW_UI_INFO = 1,
    WINDOW_DISPLAY_INFO = 1 << 1,
    WINDOW_LAYOUT_INFO = 1 << 2,
    WINDOW_META_INFO = 1 << 3,
    ALL = ~0,
};

inline WindowInfoTypeOption operator|(WindowInfoTypeOption lhs, WindowInfoTypeOption rhs)
{
    return static_cast<WindowInfoTypeOption>(static_cast<WindowInfoTypeOptionDataType>(lhs) |
        static_cast<WindowInfoTypeOptionDataType>(rhs));
}

inline bool IsChosenWindowOption(WindowInfoTypeOption options, WindowInfoTypeOption option)
{
    return (static_cast<WindowInfoTypeOptionDataType>(options) &
        static_cast<WindowInfoTypeOptionDataType>(option)) != 0;
}

/**
 * @enum WindowVisibilityState
 *
 * @brief Visibility state of a window
 */
enum WindowVisibilityState : uint32_t {
    START = 0,
    WINDOW_VISIBILITY_STATE_NO_OCCLUSION = START,
    WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION,
    WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION,
    WINDOW_LAYER_STATE_MAX,
    END = WINDOW_LAYER_STATE_MAX,
};

/**
 * @struct WindowUIInfo
 *
 * @brief Window UI info
 */
struct WindowUIInfo : public Parcelable {
    WindowVisibilityState visibilityState = WINDOW_LAYER_STATE_MAX;

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteUint32(static_cast<uint32_t>(visibilityState));
    }

    static WindowUIInfo* Unmarshalling(Parcel& parcel)
    {
        WindowUIInfo* windowUIInfo = new WindowUIInfo();
        uint32_t visibilityState = 0;
        if (!parcel.ReadUint32(visibilityState)) {
            delete windowUIInfo;
            return nullptr;
        }
        windowUIInfo->visibilityState = static_cast<WindowVisibilityState>(visibilityState);
        return windowUIInfo;
    }
};

/**
 * @struct WindowDisplayInfo
 *
 * @brief Window display info
 */
struct WindowDisplayInfo : public Parcelable {
    DisplayId displayId = DISPLAY_ID_INVALID;
    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteUint64(displayId);
    }

    static WindowDisplayInfo* Unmarshalling(Parcel& parcel)
    {
        WindowDisplayInfo* windowDisplayInfo = new WindowDisplayInfo();
        if (!parcel.ReadUint64(windowDisplayInfo->displayId)) {
            delete windowDisplayInfo;
            return nullptr;
        }
        return windowDisplayInfo;
    }
};

/**
 * @struct WindowLayoutInfo
 *
 * @brief Layout info for all windows on the screen.
 */
struct WindowLayoutInfo : public Parcelable {
    Rect rect = Rect::EMPTY_RECT;
    uint32_t zOrder = 0;

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteInt32(rect.posX_) && parcel.WriteInt32(rect.posY_) && parcel.WriteUint32(rect.width_) &&
               parcel.WriteUint32(rect.height_) && parcel.WriteUint32(zOrder);
    }

    static WindowLayoutInfo* Unmarshalling(Parcel& parcel)
    {
        WindowLayoutInfo* windowLayoutInfo = new WindowLayoutInfo();
        if (!parcel.ReadInt32(windowLayoutInfo->rect.posX_) || !parcel.ReadInt32(windowLayoutInfo->rect.posY_) ||
            !parcel.ReadUint32(windowLayoutInfo->rect.width_) || !parcel.ReadUint32(windowLayoutInfo->rect.height_) ||
            !parcel.ReadUint32(windowLayoutInfo->zOrder)) {
            delete windowLayoutInfo;
            return nullptr;
        }
        return windowLayoutInfo;
    }
};

/**
 * @struct WindowMetaInfo
 *
 * @brief Window meta info
 */
struct WindowMetaInfo : public Parcelable {
    int32_t windowId = 0;
    std::string windowName;
    std::string bundleName;
    std::string abilityName;
    int32_t appIndex = 0;
    WindowType windowType = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    uint32_t parentWindowId = INVALID_WINDOW_ID;
    uint64_t surfaceNodeId = 0;
    uint64_t leashWinSurfaceNodeId = 0;
    bool isPrivacyMode = false;
    WindowMode windowMode = WindowMode::WINDOW_MODE_UNDEFINED;
    bool isMidScene = false;
    bool isFocused = false;

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteInt32(windowId) && parcel.WriteString(windowName) && parcel.WriteString(bundleName) &&
               parcel.WriteString(abilityName) && parcel.WriteInt32(appIndex) &&
               parcel.WriteUint32(static_cast<uint32_t>(windowType)) && parcel.WriteUint32(parentWindowId) &&
               parcel.WriteUint64(surfaceNodeId) && parcel.WriteUint64(leashWinSurfaceNodeId) &&
               parcel.WriteBool(isPrivacyMode) && parcel.WriteBool(isMidScene) &&
               parcel.WriteBool(isFocused) && parcel.WriteUint32(static_cast<uint32_t>(windowMode));
    }

    static WindowMetaInfo* Unmarshalling(Parcel& parcel)
    {
        uint32_t windowTypeValue = 1;
        uint32_t windowModeValue = 1;
        WindowMetaInfo* windowMetaInfo = new WindowMetaInfo();
        if (!parcel.ReadInt32(windowMetaInfo->windowId) || !parcel.ReadString(windowMetaInfo->windowName) ||
            !parcel.ReadString(windowMetaInfo->bundleName) || !parcel.ReadString(windowMetaInfo->abilityName) ||
            !parcel.ReadInt32(windowMetaInfo->appIndex) || !parcel.ReadUint32(windowTypeValue) ||
            !parcel.ReadUint32(windowMetaInfo->parentWindowId) || !parcel.ReadUint64(windowMetaInfo->surfaceNodeId) ||
            !parcel.ReadUint64(windowMetaInfo->leashWinSurfaceNodeId) ||
            !parcel.ReadBool(windowMetaInfo->isPrivacyMode) || !parcel.ReadBool(windowMetaInfo->isMidScene) ||
            !parcel.ReadBool(windowMetaInfo->isFocused) || !parcel.ReadUint32(windowModeValue)) {
            delete windowMetaInfo;
            return nullptr;
        }
        windowMetaInfo->windowType = static_cast<WindowType>(windowTypeValue);
        windowMetaInfo->windowMode = static_cast<WindowMode>(windowModeValue);
        return windowMetaInfo;
    }
};

/**
 * @struct WindowInfo
 *
 * @brief Classified window info
 */
struct WindowInfo : public Parcelable {
    WindowUIInfo windowUIInfo;
    WindowDisplayInfo windowDisplayInfo;
    WindowLayoutInfo windowLayoutInfo;
    WindowMetaInfo windowMetaInfo;

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteParcelable(&windowUIInfo) && parcel.WriteParcelable(&windowDisplayInfo) &&
               parcel.WriteParcelable(&windowLayoutInfo) && parcel.WriteParcelable(&windowMetaInfo);
    }

    static WindowInfo* Unmarshalling(Parcel& parcel)
    {
        WindowInfo* windowInfo = new WindowInfo();
        sptr<WindowUIInfo> windowUIInfo = parcel.ReadParcelable<WindowUIInfo>();
        sptr<WindowDisplayInfo> windowDisplayInfo = parcel.ReadParcelable<WindowDisplayInfo>();
        sptr<WindowLayoutInfo> windowLayoutInfo = parcel.ReadParcelable<WindowLayoutInfo>();
        sptr<WindowMetaInfo> windowMetaInfo = parcel.ReadParcelable<WindowMetaInfo>();
        if (!windowUIInfo || !windowDisplayInfo || !windowLayoutInfo || !windowMetaInfo) {
            delete windowInfo;
            return nullptr;
        }
        windowInfo->windowUIInfo = *windowUIInfo;
        windowInfo->windowDisplayInfo = *windowDisplayInfo;
        windowInfo->windowLayoutInfo = *windowLayoutInfo;
        windowInfo->windowMetaInfo = *windowMetaInfo;
        return windowInfo;
    }
};

/**
 * @struct WindowInfoOption
 *
 * @brief Option of list window info
 */
struct WindowInfoOption : public Parcelable {
    WindowInfoFilterOption windowInfoFilterOption = WindowInfoFilterOption::ALL;
    WindowInfoTypeOption windowInfoTypeOption = WindowInfoTypeOption::ALL;
    DisplayId displayId = DISPLAY_ID_INVALID;
    int32_t windowId = 0;

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteUint32(static_cast<uint32_t>(windowInfoFilterOption)) &&
               parcel.WriteUint32(static_cast<uint32_t>(windowInfoTypeOption)) &&
               parcel.WriteUint64(displayId) &&
               parcel.WriteInt32(windowId);
    }

    static WindowInfoOption* Unmarshalling(Parcel& parcel)
    {
        WindowInfoOption* windowInfoOption = new WindowInfoOption();
        uint32_t windowInfoFilterOption;
        uint32_t windowInfoTypeOption;
        if (!parcel.ReadUint32(windowInfoFilterOption) || !parcel.ReadUint32(windowInfoTypeOption) ||
            !parcel.ReadUint64(windowInfoOption->displayId) || !parcel.ReadInt32(windowInfoOption->windowId)) {
            delete windowInfoOption;
            return nullptr;
        }
        windowInfoOption->windowInfoFilterOption = static_cast<WindowInfoFilterOption>(windowInfoFilterOption);
        windowInfoOption->windowInfoTypeOption = static_cast<WindowInfoTypeOption>(windowInfoTypeOption);
        return windowInfoOption;
    }
};

/**
 * Config of keyboard animation
 */
class KeyboardAnimationCurve : public Parcelable {
public:
    KeyboardAnimationCurve() = default;
    KeyboardAnimationCurve(const std::string& curveType, const std::vector<float>& curveParams, uint32_t duration)
        : curveType_(curveType), duration_(duration)
    {
        curveParams_.assign(curveParams.begin(), curveParams.end());
    }

    virtual bool Marshalling(Parcel& parcel) const override
    {
        if (!parcel.WriteString(curveType_)) {
            return false;
        }

        auto paramSize = curveParams_.size();
        if (paramSize == 4) { // 4: param size
            if (!parcel.WriteUint32(static_cast<uint32_t>(paramSize))) {
                return false;
            }
            for (auto& param : curveParams_) {
                if (!parcel.WriteFloat(param)) {
                    return false;
                }
            }
        } else {
            if (!parcel.WriteUint32(0)) {
                return false;
            }
        }

        if (!parcel.WriteUint32(duration_)) {
            return false;
        }
        return true;
    }

    static KeyboardAnimationCurve* Unmarshalling(Parcel& parcel)
    {
        KeyboardAnimationCurve* config = new KeyboardAnimationCurve;
        uint32_t paramSize = 0;
        if (!parcel.ReadString(config->curveType_) || !parcel.ReadUint32(paramSize)) {
            delete config;
            return nullptr;
        }

        if (paramSize == 4) { // 4: paramSize
            for (uint32_t i = 0; i < paramSize; i++) {
                float param = 0.0f;
                if (!parcel.ReadFloat(param)) {
                    delete config;
                    return nullptr;
                } else {
                    config->curveParams_.push_back(param);
                }
            }
        }

        if (!parcel.ReadUint32(config->duration_)) {
            delete config;
            return nullptr;
        }
        return config;
    }

    std::string curveType_ = "";
    std::vector<float> curveParams_ = {};
    uint32_t duration_ = 0;
};

struct KeyboardAnimationConfig {
    KeyboardAnimationCurve curveIn;
    KeyboardAnimationCurve curveOut;
};

struct MoveConfiguration {
    DisplayId displayId = DISPLAY_ID_INVALID;
    RectAnimationConfig rectAnimationConfig = { 0, 0.0f, 0.0f, 0.0f, 0.0f };
    std::string ToString() const
    {
        std::string str;
        constexpr int BUFFER_SIZE = 1024;
        char buffer[BUFFER_SIZE] = { 0 };
        if (snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1,
            "[displayId: %llu, rectAnimationConfig: [%u, %f, %f, %f, %f]]", displayId, rectAnimationConfig.duration,
            rectAnimationConfig.x1, rectAnimationConfig.y1, rectAnimationConfig.x2, rectAnimationConfig.y2) > 0) {
            str.append(buffer);
        }
        return str;
    }
};

enum class MaximizePresentation {
    FOLLOW_APP_IMMERSIVE_SETTING = 0,   // follow app set immersiveStateEnable
    EXIT_IMMERSIVE = 1,                 // immersiveStateEnable will be set as false
    ENTER_IMMERSIVE = 2,                // immersiveStateEnable will be set as true
    // immersiveStateEnable will be set as true, titleHoverShowEnabled and dockHoverShowEnabled will be set as false
    ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER = 3,
};

enum class ExtensionWindowAttribute : int32_t {
    SYSTEM_WINDOW = 0,
    SUB_WINDOW = 1,
    UNKNOWN = 2
};

struct SystemWindowOptions {
    int32_t windowType = -1;
};

enum class ModalityType : uint8_t {
    WINDOW_MODALITY,
    APPLICATION_MODALITY,
};

struct SubWindowOptions {
    std::string title;
    bool decorEnabled = false;
    bool isModal = false;
    bool isTopmost = false;
    bool maximizeSupported = false;
    ModalityType modalityType = ModalityType::WINDOW_MODALITY;
};

struct ExtensionWindowConfig {
    std::string windowName;
    ExtensionWindowAttribute windowAttribute = ExtensionWindowAttribute::UNKNOWN;
    Rect windowRect;
    SubWindowOptions subWindowOptions;
    SystemWindowOptions systemWindowOptions;
};

enum class BackupAndRestoreType : int32_t {
    NONE = 0,                       // no backup and restore
    CONTINUATION = 1,               // distribute
    APP_RECOVERY = 2,               // app recovery
    RESOURCESCHEDULE_RECOVERY = 3,  // app is killed due to resource schedule
};

/**
 * @brief Windowinfokey
 */
enum class WindowInfoKey : int32_t {
    NONE = 0,
    WINDOW_ID = 1,
    BUNDLE_NAME = 1 << 1,
    ABILITY_NAME = 1 << 2,
    APP_INDEX = 1 << 3,
    VISIBILITY_STATE = 1 << 4,
    DISPLAY_ID = 1 << 5,
    WINDOW_RECT = 1 << 6,
    WINDOW_MODE = 1 << 7,
    FLOATING_SCALE = 1 << 8,
    MID_SCENE = 1 << 9,
    WINDOW_GLOBAL_RECT = 1 << 10,
};

/*
 * @brief Enumerates rotation change type.
 */
enum class RotationChangeType : uint32_t {
    /**
     * rotate will begin.
     */
    WINDOW_WILL_ROTATE = 0,

    /**
     * rotate end.
     */
    WINDOW_DID_ROTATE,
};

/**
 * @brief Enumerates rect type
 */
enum class RectType : uint32_t {
    /**
     * the window rect of app relative to screen.
     */
    RELATIVE_TO_SCREEN = 0,

    /**
     * the window rect of app relative to parent window.
     */
    RELATIVE_TO_PARENT_WINDOW,
};

/**
 * @brief rotation change info to notify listener.
 */
struct RotationChangeInfo {
    RotationChangeType type_;
    uint32_t orientation_;
    DisplayId displayId_;
    Rect displayRect_;
};

/**
 * @brief rotation change result return from listener.
 */
struct RotationChangeResult {
    RectType rectType_;
    Rect windowRect_;
};

/**
 * @brief default zIndex for specific window.
 */
enum DefaultSpecificZIndex {
    MUTISCREEN_COLLABORATION = 930,
    SUPER_PRIVACY_ANIMATION = 1100,
};

/**
 * @struct ShadowsInfo
 *
 * @brief window shadows info
 */
struct ShadowsInfo : public Parcelable {
    float radius_ = 0.0f;
    std::string color_;
    float offsetX_ = 0.0f;
    float offsetY_ = 0.0f;
    bool hasRadiusValue_ = false;
    bool hasColorValue_ = false;
    bool hasOffsetXValue_ = false;
    bool hasOffsetYValue_ = false;

    ShadowsInfo() {}
    ShadowsInfo(float radius, std::string color, float offsetX, float offsetY, bool hasRadiusValue,
        bool hasColorValue, bool hasOffsetXValue, bool hasOffsetYValue) : radius_(radius), color_(color),
        offsetX_(offsetX), offsetY_(offsetY), hasRadiusValue_(hasRadiusValue), hasColorValue_(hasColorValue),
        hasOffsetXValue_(hasOffsetXValue), hasOffsetYValue_(hasOffsetYValue) {}

    bool operator==(const ShadowsInfo& other) const
    {
        return (NearEqual(radius_, other.radius_) && color_ == other.color_ &&
            NearEqual(offsetX_, other.offsetX_) && NearEqual(offsetY_, other.offsetY_));
    }

    bool operator!=(const ShadowsInfo& other) const
    {
        return !this->operator==(other);
    }

    static inline bool NearEqual(float left, float right) { return std::abs(left - right) < POS_ZERO; }

    bool Marshalling(Parcel& parcel) const override
    {
        if (!parcel.WriteBool(hasRadiusValue_)) {
            return false;
        }

        if (!parcel.WriteBool(hasColorValue_)) {
            return false;
        }

        if (!parcel.WriteBool(hasOffsetXValue_)) {
            return false;
        }

        if (!parcel.WriteBool(hasOffsetYValue_)) {
            return false;
        }

        if (hasRadiusValue_ && !parcel.WriteFloat(radius_)) {
            return false;
        }

        if (hasColorValue_ && !parcel.WriteString(color_)) {
            return false;
        }

        if (hasOffsetXValue_ && !parcel.WriteFloat(offsetX_)) {
            return false;
        }

        if (hasOffsetYValue_ && !parcel.WriteFloat(offsetY_)) {
            return false;
        }
        return true;
    }

    static ShadowsInfo* Unmarshalling(Parcel& parcel)
    {
        ShadowsInfo* shadowsInfo = new ShadowsInfo();
        if (!parcel.ReadBool(shadowsInfo->hasRadiusValue_)) {
            delete shadowsInfo;
            return nullptr;
        }

        if (!parcel.ReadBool(shadowsInfo->hasColorValue_)) {
            delete shadowsInfo;
            return nullptr;
        }

        if (!parcel.ReadBool(shadowsInfo->hasOffsetXValue_)) {
            delete shadowsInfo;
            return nullptr;
        }

        if (!parcel.ReadBool(shadowsInfo->hasOffsetYValue_)) {
            delete shadowsInfo;
            return nullptr;
        }

        if (shadowsInfo->hasRadiusValue_ && !parcel.ReadFloat(shadowsInfo->radius_)) {
            delete shadowsInfo;
            return nullptr;
        }

        if (shadowsInfo->hasColorValue_ && !parcel.ReadString(shadowsInfo->color_)) {
            delete shadowsInfo;
            return nullptr;
        }

        if (shadowsInfo->hasOffsetXValue_ && !parcel.ReadFloat(shadowsInfo->offsetX_)) {
            delete shadowsInfo;
            return nullptr;
        }

        if (shadowsInfo->hasOffsetYValue_ && !parcel.ReadFloat(shadowsInfo->offsetY_)) {
            delete shadowsInfo;
            return nullptr;
        }
        return shadowsInfo;
    }
};

/**
 * @brief Enumerates source of sub session.
 */
enum class SubWindowSource : uint32_t {
    SUB_WINDOW_SOURCE_UNKNOWN = 0,
    SUB_WINDOW_SOURCE_ARKUI = 1,
};

/**
 * @brief Screenshot event type.
 */
enum class ScreenshotEventType : int32_t {
    START = 0,

    /**
     * System screenshot.
     */
    SYSTEM_SCREENSHOT = START,

    /**
     * System screenshot abort.
     */
    SYSTEM_SCREENSHOT_ABORT = 1,

    /**
     * Scroll shot start.
     */
    SCROLL_SHOT_START = 2,

    /**
     * Scroll shot end.
     */
    SCROLL_SHOT_END = 3,

    /**
     * Scroll shot abort.
     */
    SCROLL_SHOT_ABORT = 4,

    END,
};

/**
 * @enum WaterfallResidentState
 * @brief Represents the resident (persistent) state control of the waterfall layout.
 */
enum class WaterfallResidentState : uint32_t {
    /** The resident state and the current waterfall state remain unchanged. */
    UNCHANGED = 0,

    /** Enable the resident state and enter the waterfall layout. */
    OPEN = 1,

    /** Disable the resident state and exit the waterfall layout. */
    CLOSE = 2,
};

struct StateChangeOption {
    int32_t parentPersistentId_ = 0;
    WindowState newState_ = WindowState::STATE_INITIAL;
    uint32_t reason_ = 0;
    bool withAnimation_ = false;
    bool withFocus_ = false;
    bool waitAttach_ = false;
    bool isFromInnerkits_ = false;
    bool waitDetach_ = false;

    StateChangeOption(int32_t parentPersistentId, WindowState newState)
        : parentPersistentId_(parentPersistentId), newState_(newState) {}

    StateChangeOption(int32_t parentPersistentId, WindowState newState, uint32_t reason, bool withAnimation,
        bool withFocus, bool waitAttach, bool isFromInnerkits, bool waitDetach)
        : parentPersistentId_(parentPersistentId), newState_(newState), reason_(reason),
          withAnimation_(withAnimation), withFocus_(withFocus), waitAttach_(waitAttach),
          isFromInnerkits_(isFromInnerkits), waitDetach_(waitDetach) {}
};
}
}
#endif // OHOS_ROSEN_WM_COMMON_H
