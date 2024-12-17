/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <parcel.h>
#include <map>
#include <float.h>
#include <sstream>
#include <string>

namespace OHOS {
namespace Rosen {
using DisplayId = uint64_t;
/**
 * @brief Enumerates type of window.
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
    WINDOW_TYPE_NEGATIVE_SCREEN,
    WINDOW_TYPE_SYSTEM_TOAST,
    WINDOW_TYPE_SYSTEM_FLOAT,
    WINDOW_TYPE_PIP,
    WINDOW_TYPE_THEME_EDITOR,
    WINDOW_TYPE_NAVIGATION_INDICATOR,
    WINDOW_TYPE_HANDWRITE,
    WINDOW_TYPE_SCENE_BOARD,
    WINDOW_TYPE_KEYBOARD_PANEL,
    WINDOW_TYPE_SCB_DEFAULT,
    WINDOW_TYPE_TRANSPARENT_VIEW,
    ABOVE_APP_SYSTEM_WINDOW_END,

    SYSTEM_SUB_WINDOW_BASE = 2500,
    WINDOW_TYPE_SYSTEM_SUB_WINDOW = SYSTEM_SUB_WINDOW_BASE,
    SYSTEM_SUB_WINDOW_END,

    SYSTEM_WINDOW_END = SYSTEM_SUB_WINDOW_END,

    WINDOW_TYPE_UI_EXTENSION = 3000
};

/**
 * @struct HookInfo.
 *
 * @brief hook diaplayinfo deepending on the window size.
 */
struct HookInfo {
    uint32_t width_;
    uint32_t height_;
    float_t density_;
    uint32_t rotation_;
    bool enableHookRotation_;
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
    WINDOW_MODE_PIP
};

/**
 * @brief Enumerates modeType of window.
 */
enum class WindowModeType : uint8_t {
    WINDOW_MODE_SPLIT_FLOATING = 0,
    WINDOW_MODE_SPLIT = 1,
    WINDOW_MODE_FLOATING = 2,
    WINDOW_MODE_FULLSCREEN = 3,
    WINDOW_MODE_FULLSCREEN_FLOATING = 4,
    WINDOW_MODE_OTHER = 5
};

/**
 * @brief Enumerates modal of sub session.
 */
enum class SubWindowModalType : uint32_t {
    BEGIN = 0,
    TYPE_UNDEFINED = BEGIN,
    TYPE_NORMAL,
    TYPE_DIALOG,
    TYPE_WINDOW_MODALITY,
    TYPE_TOAST,
    TYPE_APPLICATION_MODALITY,
    END = TYPE_APPLICATION_MODALITY,
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
    WINDOW_MODE_SUPPORT_ALL = WINDOW_MODE_SUPPORT_FULLSCREEN |
                              WINDOW_MODE_SUPPORT_SPLIT_PRIMARY |
                              WINDOW_MODE_SUPPORT_SPLIT_SECONDARY |
                              WINDOW_MODE_SUPPORT_FLOATING |
                              WINDOW_MODE_SUPPORT_PIP
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
    STATE_BOTTOM = STATE_DESTROYED, // Add state type after STATE_DESTROYED is not allowed
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

    WM_ERROR_DEVICE_NOT_SUPPORT = 801, // the value do not change.It is defined on all system

    WM_ERROR_NEED_REPORT_BASE = 1000, // error code > 1000 means need report
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
};

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
 * @brief Enumerates setting flag of systemStatusBar
 */
enum class SystemBarSettingFlag : uint32_t {
    DEFAULT_SETTING = 0,
    COLOR_SETTING = 1,
    ENABLE_SETTING = 1 << 1,
    ALL_SETTING = COLOR_SETTING | ENABLE_SETTING,
    FOLLOW_SETTING = 1 << 2
};

/**
 * @brief Enumerates flag of ControlAppType.
 */
enum class ControlAppType : uint8_t {
    CONTROL_APP_TYPE_BEGIN = 0,
    APP_LOCK = 1,
    CONTROL_APP_TYPE_END,
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
    {WMError::WM_ERROR_PIP_DESTROY_FAILED,             WmErrorCode::WM_ERROR_PIP_DESTROY_FAILED       },
    {WMError::WM_ERROR_PIP_STATE_ABNORMALLY,           WmErrorCode::WM_ERROR_PIP_STATE_ABNORMALLY     },
    {WMError::WM_ERROR_PIP_CREATE_FAILED,              WmErrorCode::WM_ERROR_PIP_CREATE_FAILED        },
    {WMError::WM_ERROR_PIP_INTERNAL_ERROR,             WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR       },
    {WMError::WM_ERROR_PIP_REPEAT_OPERATION,           WmErrorCode::WM_ERROR_PIP_REPEAT_OPERATION     },
    {WMError::WM_ERROR_INVALID_CALLING,                WmErrorCode::WM_ERROR_INVALID_CALLING          },
    {WMError::WM_ERROR_INVALID_SESSION,                WmErrorCode::WM_ERROR_STATE_ABNORMALLY         },
    {WMError::WM_ERROR_SYSTEM_ABNORMALLY,              WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY        },
};

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
    WINDOW_FLAG_IS_APPLICATION_MODAL = 1 << 6,
    WINDOW_FLAG_HANDWRITING = 1 << 7,
    WINDOW_FLAG_IS_TOAST = 1 << 8,
    WINDOW_FLAG_END = 1 << 9,
};

/**
 * @brief Flag of uiextension window.
 */
union ExtensionWindowFlags {
    uint32_t bitData;
    struct {
        // Each flag should be false default, true when active
        bool hideNonSecureWindowsFlag : 1;
        bool waterMarkFlag : 1;
        bool privacyModeFlag : 1;
    };
    ExtensionWindowFlags() : bitData(0) {}
    ExtensionWindowFlags(uint32_t bits) : bitData(bits) {}
    ~ExtensionWindowFlags() {}
    void SetAllActive()
    {
        hideNonSecureWindowsFlag = true;
        waterMarkFlag = true;
        privacyModeFlag = true;
    }
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
    MOVE,
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
    END,
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

/**
 * @brief Enumerates drag event.
 */
enum class DragEvent : uint32_t {
    DRAG_EVENT_IN  = 1,
    DRAG_EVENT_OUT,
    DRAG_EVENT_MOVE,
    DRAG_EVENT_END,
};

/**
 * @brief Enumerates window tag.
 */
enum class WindowTag : uint32_t {
    MAIN_WINDOW = 0,
    SUB_WINDOW = 1,
    SYSTEM_WINDOW = 2,
};

/**
 * @brief Enumerates window session type.
 */
enum class WindowSessionType : uint32_t {
    SCENE_SESSION = 0,
    EXTENSION_SESSION = 1,
};

/**
 * @brief Enumerates window gravity.
 */
enum class WindowGravity : uint32_t {
    WINDOW_GRAVITY_FLOAT = 0,
    WINDOW_GRAVITY_BOTTOM,
    WINDOW_GRAVITY_DEFAULT,
};

/**
 * @brief Enumerates window setuicontent type.
 */
enum class WindowSetUIContentType: uint32_t {
    DEFAULT,
    DISTRIBUTE,
    BY_NAME,
    BY_ABC,
};

/**
 * @brief Enumerates window Style type.
 */
enum class WindowStyleType : uint8_t {
    WINDOW_STYLE_DEFAULT = 0,
    WINDOW_STYLE_FREE_MULTI_WINDOW = 1,
};

/**
 * @brief Disable Gesture Back Type
 */
enum class GestureBackType : uint8_t {
    GESTURE_SIDE = 0,
    GESTURE_SWIPE_UP = 1,
    GESTURE_ALL = 2,
};

/**
 * @struct PointInfo.
 *
 * @brief point Info.
 */
struct PointInfo {
    int32_t x;
    int32_t y;
};

/**
 * @struct MainWindowInfo.
 *
 * @brief topN main window info.
 */
struct MainWindowInfo : public Parcelable {
    virtual bool Marshalling(Parcel& parcel) const override
    {
        if (!parcel.WriteInt32(pid_)) {
            return false;
        }

        if (!parcel.WriteString(bundleName_)) {
            return false;
        }

        if (!parcel.WriteInt32(persistentId_)) {
            return false;
        }

        if (!parcel.WriteInt32(bundleType_)) {
            return false;
        }
        return true;
    }

    static MainWindowInfo* Unmarshalling(Parcel& parcel)
    {
        MainWindowInfo* mainWindowInfo = new MainWindowInfo;
        mainWindowInfo->pid_ = parcel.ReadInt32();
        mainWindowInfo->bundleName_ = parcel.ReadString();
        mainWindowInfo->persistentId_ = parcel.ReadInt32();
        mainWindowInfo->bundleType_ = parcel.ReadInt32();
        return mainWindowInfo;
    }

    int32_t pid_ = 0;
    std::string bundleName_ = "";
    int32_t persistentId_ = 0;
    int32_t bundleType_ = 0;
};

/**
 * @struct MainWindowState.
 *
 * @brief Main window state info.
 */
struct MainWindowState : public Parcelable {
    bool Marshalling(Parcel& parcel) const override
    {
        if (!parcel.WriteInt32(state_)) {
            return false;
        }
        if (!parcel.WriteBool(isVisible_)) {
            return false;
        }
        if (!parcel.WriteBool(isForegroundInteractive_)) {
            return false;
        }
        if (!parcel.WriteBool(isPcOrPadEnableActivation_)) {
            return false;
        }
        return true;
    }

    static MainWindowState* Unmarshalling(Parcel& parcel)
    {
        MainWindowState* mainWindowState = new MainWindowState();
        if (!mainWindowState) {
            return nullptr;
        }
        if (!parcel.ReadInt32(mainWindowState->state_) ||
            !parcel.ReadBool(mainWindowState->isVisible_) ||
            !parcel.ReadBool(mainWindowState->isForegroundInteractive_) ||
            !parcel.ReadBool(mainWindowState->isPcOrPadEnableActivation_)) {
            delete mainWindowState;
            return nullptr;
        }
        return mainWindowState;
    }

    int32_t state_ = 0;
    bool isVisible_ = false;
    bool isForegroundInteractive_ = false;
    bool isPcOrPadEnableActivation_ = false;
};

namespace {
    constexpr uint32_t SYSTEM_COLOR_WHITE = 0xE5FFFFFF;
    constexpr uint32_t SYSTEM_COLOR_BLACK = 0x66000000;
    constexpr uint32_t INVALID_WINDOW_ID = 0;
    constexpr float UNDEFINED_BRIGHTNESS = -1.0f;
    constexpr float MINIMUM_BRIGHTNESS = 0.0f;
    constexpr float MAXIMUM_BRIGHTNESS = 1.0f;
    constexpr int32_t INVALID_PID = -1;
    constexpr int32_t INVALID_UID = -1;
    constexpr int32_t INVALID_USER_ID = -1;
    constexpr int32_t SYSTEM_USERID = 0;
    constexpr int32_t BASE_USER_RANGE = 200000;
    constexpr int32_t DEFAULT_SCREEN_ID = 0;
}

inline int32_t GetUserIdByUid(int32_t uid)
{
    return uid / BASE_USER_RANGE;
}

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
        return NearZero(pivotX_ - right.pivotX_) &&
            NearZero(pivotY_ - right.pivotY_) &&
            NearZero(scaleX_ - right.scaleX_) &&
            NearZero(scaleY_ - right.scaleY_) &&
            NearZero(scaleZ_ - right.scaleZ_) &&
            NearZero(rotationX_ - right.rotationX_) &&
            NearZero(rotationY_ - right.rotationY_) &&
            NearZero(rotationZ_ - right.rotationZ_) &&
            NearZero(translateX_ - right.translateX_) &&
            NearZero(translateY_ - right.translateY_) &&
            NearZero(translateZ_ - right.translateZ_);
    }

    bool operator!=(const Transform& right) const
    {
        return !(*this == right);
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

    static const Transform& Identity()
    {
        static Transform I;
        return I;
    }

    bool Marshalling(Parcel& parcel) const
    {
        return parcel.WriteFloat(pivotX_) && parcel.WriteFloat(pivotY_) &&
               parcel.WriteFloat(scaleX_) && parcel.WriteFloat(scaleY_) && parcel.WriteFloat(scaleZ_) &&
               parcel.WriteFloat(rotationX_) && parcel.WriteFloat(rotationY_) && parcel.WriteFloat(rotationZ_) &&
               parcel.WriteFloat(translateX_) && parcel.WriteFloat(translateY_) && parcel.WriteFloat(translateZ_);
    }

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
private:
    static inline bool NearZero(float val)
    {
        return val < 0.001f && val > -0.001f;
    }
};

/**
 * @struct SystemBarProperty
 *
 * @brief Property of system bar
 */
struct SystemBarProperty {
    bool enable_;
    uint32_t backgroundColor_;
    uint32_t contentColor_;
    bool enableAnimation_;
    SystemBarSettingFlag settingFlag_;
    SystemBarProperty() : enable_(true), backgroundColor_(SYSTEM_COLOR_BLACK), contentColor_(SYSTEM_COLOR_WHITE),
                          enableAnimation_(false), settingFlag_(SystemBarSettingFlag::DEFAULT_SETTING) {}
    SystemBarProperty(bool enable, uint32_t background, uint32_t content)
        : enable_(enable), backgroundColor_(background), contentColor_(content), enableAnimation_(false),
          settingFlag_(SystemBarSettingFlag::DEFAULT_SETTING) {}
    SystemBarProperty(bool enable, uint32_t background, uint32_t content, bool enableAnimation)
        : enable_(enable), backgroundColor_(background), contentColor_(content), enableAnimation_(enableAnimation),
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
 * @brief Window Rect
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

    bool IsUninitializedRect() const
    {
        return (posX_ == 0 && posY_ == 0 && width_ == 0 && height_ == 0);
    }

    bool IsUninitializedSize() const
    {
        return width_ == 0 && height_ == 0;
    }

    bool IsInsideOf(const Rect& a) const
    {
        return (posX_ >= a.posX_ && posY_ >= a.posY_ &&
            posX_ + width_ <= a.posX_ + a.width_ && posY_ + height_ <= a.posY_ + a.height_);
    }

    inline std::string ToString() const
    {
        std::stringstream ss;
        ss << "[" << posX_ << " " << posY_ << " " << width_ << " " << height_ << "]";
        return ss.str();
    }
};

/**
 * @brief UIExtension usage
 */
enum class UIExtensionUsage : uint32_t {
    MODAL = 0,
    EMBEDDED,
    CONSTRAINED_EMBEDDED,
    UIEXTENSION_USAGE_END
};

/**
 * @brief UIExtension info for event
 */
struct ExtensionWindowEventInfo {
    int32_t persistentId  = 0;
    int32_t pid = -1;
    Rect windowRect {0, 0, 0, 0};
};

/**
 * @brief UIExtension info from ability
 */
struct ExtensionWindowAbilityInfo {
    int32_t persistentId  { 0 };
    int32_t parentId { 0 };
    UIExtensionUsage usage { UIExtensionUsage::UIEXTENSION_USAGE_END };
};

/**
 * @struct KeyboardPanelInfo
 *
 * @brief Info of keyboard panel
 */
struct KeyboardPanelInfo : public Parcelable {
    Rect rect_ = {0, 0, 0, 0};
    WindowGravity gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    bool isShowing_ = false;

    bool Marshalling(Parcel& parcel) const
    {
        return parcel.WriteInt32(rect_.posX_) && parcel.WriteInt32(rect_.posY_) &&
               parcel.WriteUint32(rect_.width_) && parcel.WriteUint32(rect_.height_) &&
               parcel.WriteUint32(static_cast<uint32_t>(gravity_)) &&
               parcel.WriteBool(isShowing_);
    }

    static KeyboardPanelInfo* Unmarshalling(Parcel& parcel)
    {
        KeyboardPanelInfo* keyboardPanelInfo = new(std::nothrow)KeyboardPanelInfo;
        if (keyboardPanelInfo == nullptr) {
            return nullptr;
        }
        bool res = parcel.ReadInt32(keyboardPanelInfo->rect_.posX_) &&
            parcel.ReadInt32(keyboardPanelInfo->rect_.posY_) && parcel.ReadUint32(keyboardPanelInfo->rect_.width_) &&
            parcel.ReadUint32(keyboardPanelInfo->rect_.height_);
        if (!res) {
            delete keyboardPanelInfo;
            return nullptr;
        }
        keyboardPanelInfo->gravity_ = static_cast<WindowGravity>(parcel.ReadUint32());
        keyboardPanelInfo->isShowing_ = parcel.ReadBool();

        return keyboardPanelInfo;
    }
};

/**
 * @brief Enumerates avoid area type.
 */
enum class AvoidAreaType : uint32_t {
    TYPE_SYSTEM,           // area of SystemUI
    TYPE_CUTOUT,           // cutout of screen
    TYPE_SYSTEM_GESTURE,   // area for system gesture
    TYPE_KEYBOARD,         // area for soft input keyboard
    TYPE_NAVIGATION_INDICATOR, // area for navigation indicator
};

/**
 * @brief Enumerates occupied area type.
 */
enum class OccupiedAreaType : uint32_t {
    TYPE_INPUT, // area of input window
};

/**
 * @brief Enumerates color space.
 */
enum class ColorSpace : uint32_t {
    COLOR_SPACE_DEFAULT = 0, // Default color space.
    COLOR_SPACE_WIDE_GAMUT,  // Wide gamut color space. The specific wide color gamut depends on the screen.
};

/**
 * @brief Enumerates window animation.
 */
enum class WindowAnimation : uint32_t {
    NONE,
    DEFAULT,
    INPUTE,
    CUSTOM,
};

/**
 * @brief Enumerates window maximize mode.
 */
enum class MaximizeMode : uint32_t {
    MODE_AVOID_SYSTEM_BAR,
    MODE_FULL_FILL,
    MODE_RECOVER,
};

/**
 * @class AvoidArea
 *
 * @brief Area needed to avoid.
 */
class AvoidArea : public Parcelable {
public:
    Rect topRect_ { 0, 0, 0, 0 };
    Rect leftRect_ { 0, 0, 0, 0 };
    Rect rightRect_ { 0, 0, 0, 0 };
    Rect bottomRect_ { 0, 0, 0, 0 };

    bool operator==(const AvoidArea& a) const
    {
        return (leftRect_ == a.leftRect_ && topRect_ == a.topRect_ &&
            rightRect_ == a.rightRect_ && bottomRect_ == a.bottomRect_);
    }

    bool operator!=(const AvoidArea& a) const
    {
        return !this->operator==(a);
    }

    bool isEmptyAvoidArea() const
    {
        return topRect_.IsUninitializedRect() && leftRect_.IsUninitializedRect() &&
            rightRect_.IsUninitializedRect() && bottomRect_.IsUninitializedRect();
    }

    static inline bool WriteParcel(Parcel& parcel, const Rect& rect)
    {
        return parcel.WriteInt32(rect.posX_) && parcel.WriteInt32(rect.posY_) &&
            parcel.WriteUint32(rect.width_) && parcel.WriteUint32(rect.height_);
    }

    static inline bool ReadParcel(Parcel& parcel, Rect& rect)
    {
        return parcel.ReadInt32(rect.posX_) && parcel.ReadInt32(rect.posY_) &&
            parcel.ReadUint32(rect.width_) && parcel.ReadUint32(rect.height_);
    }

    virtual bool Marshalling(Parcel& parcel) const override
    {
        return (WriteParcel(parcel, leftRect_) && WriteParcel(parcel, topRect_) &&
            WriteParcel(parcel, rightRect_) && WriteParcel(parcel, bottomRect_));
    }

    static AvoidArea* Unmarshalling(Parcel& parcel)
    {
        AvoidArea *avoidArea = new(std::nothrow) AvoidArea();
        if (avoidArea == nullptr) {
            return nullptr;
        }
        if (ReadParcel(parcel, avoidArea->leftRect_) && ReadParcel(parcel, avoidArea->topRect_) &&
            ReadParcel(parcel, avoidArea->rightRect_) && ReadParcel(parcel, avoidArea->bottomRect_)) {
            return avoidArea;
        }
        delete avoidArea;
        return nullptr;
    }

    std::string ToString() const
    {
        std::stringstream ss;
        if (isEmptyAvoidArea()) {
            ss << "empty";
            return ss.str();
        }
        if (!topRect_.IsUninitializedRect()) {
            ss << "top " << topRect_.ToString() << " ";
        }
        if (!bottomRect_.IsUninitializedRect()) {
            ss << "bottom " << bottomRect_.ToString() << " ";
        }
        if (!leftRect_.IsUninitializedRect()) {
            ss << "left " << leftRect_.ToString() << " ";
        }
        if (!rightRect_.IsUninitializedRect()) {
            ss << "right " << rightRect_.ToString() << " ";
        }
        return ss.str();
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
    WINDOW_UPDATE_PROPERTY,
    WINDOW_UPDATE_ALL,
};

/**
 * @brief Enumerates picture in picture window state.
 */
enum class PiPWindowState : uint32_t {
    STATE_UNDEFINED = 0,
    STATE_STARTING = 1,
    STATE_STARTED = 2,
    STATE_STOPPING = 3,
    STATE_STOPPED = 4,
    STATE_RESTORING = 5,
};

/**
 * @brief Enumerates picture in picture template type.
 */
enum class PiPTemplateType : uint32_t {
    VIDEO_PLAY = 0,
    VIDEO_CALL = 1,
    VIDEO_MEETING = 2,
    VIDEO_LIVE = 3,
    END,
};

/**
 * @brief Enumerates picture in picture control group.
 */
enum class PiPControlGroup : uint32_t {
    VIDEO_PLAY_START = 100,
    VIDEO_PREVIOUS_NEXT = 101,
    FAST_FORWARD_BACKWARD = 102,
    VIDEO_PLAY_END,

    VIDEO_CALL_START = 200,
    VIDEO_CALL_MICROPHONE_SWITCH = 201,
    VIDEO_CALL_HANG_UP_BUTTON = 202,
    VIDEO_CALL_CAMERA_SWITCH = 203,
    VIDEO_CALL_MUTE_SWITCH = 204,
    VIDEO_CALL_END,

    VIDEO_MEETING_START = 300,
    VIDEO_MEETING_HANG_UP_BUTTON = 301,
    VIDEO_MEETING_CAMERA_SWITCH = 302,
    VIDEO_MEETING_MUTE_SWITCH = 303,
    VIDEO_MEETING_MICROPHONE_SWITCH = 304,
    VIDEO_MEETING_END,

    VIDEO_LIVE_START = 400,
    VIDEO_PLAY_PAUSE = 401,
    VIDEO_LIVE_MUTE_SWITCH = 402,
    VIDEO_LIVE_END,
    END,
};

/**
 * @brief Enumerates picture in picture state.
 */
enum class PiPState : int32_t {
    ABOUT_TO_START = 1,
    STARTED = 2,
    ABOUT_TO_STOP = 3,
    STOPPED = 4,
    ABOUT_TO_RESTORE = 5,
    ERROR = 6,
};

/**
 * @brief Enumerates picture in picture control status.
 */
enum class PiPControlStatus : int32_t {
    PLAY = 1,
    PAUSE = 0,
    OPEN = 1,
    CLOSE = 0,
    ENABLED = -2,
    DISABLED = -3,
};

/**
 * @brief Enumerates picture in picture control type.
 */
enum class PiPControlType : uint32_t {
    VIDEO_PLAY_PAUSE = 0,
    VIDEO_PREVIOUS = 1,
    VIDEO_NEXT = 2,
    FAST_FORWARD = 3,
    FAST_BACKWARD = 4,
    HANG_UP_BUTTON = 5,
    MICROPHONE_SWITCH = 6,
    CAMERA_SWITCH = 7,
    MUTE_SWITCH = 8,
    END,
};

struct PiPControlStatusInfo {
    PiPControlType controlType;
    PiPControlStatus status;
};

struct PiPControlEnableInfo {
    PiPControlType controlType;
    PiPControlStatus enabled;
};

struct PiPTemplateInfo {
    uint32_t pipTemplateType;
    uint32_t priority;
    std::vector<uint32_t> controlGroup;
    std::vector<PiPControlStatusInfo> pipControlStatusInfoList;
    std::vector<PiPControlEnableInfo> pipControlEnableInfoList;
};

using OnCallback = std::function<void(int64_t, int64_t)>;

/**
 * @struct VsyncCallback
 *
 * @brief Vsync callback
 */
struct VsyncCallback {
    OnCallback onCallback;
};

struct WindowLimits {
    uint32_t maxWidth_ = UINT32_MAX;
    uint32_t maxHeight_ = UINT32_MAX;
    uint32_t minWidth_ = 0;
    uint32_t minHeight_ = 0;
    float maxRatio_ = FLT_MAX;
    float minRatio_ = 0.0f;
    float vpRatio_ = 1.0f;

    WindowLimits() {}
    WindowLimits(uint32_t maxWidth, uint32_t maxHeight, uint32_t minWidth, uint32_t minHeight, float maxRatio,
        float minRatio) : maxWidth_(maxWidth), maxHeight_(maxHeight), minWidth_(minWidth), minHeight_(minHeight),
        maxRatio_(maxRatio), minRatio_(minRatio) {}
    WindowLimits(uint32_t maxWidth, uint32_t maxHeight, uint32_t minWidth, uint32_t minHeight, float maxRatio,
        float minRatio, float vpRatio) : maxWidth_(maxWidth), maxHeight_(maxHeight), minWidth_(minWidth),
        minHeight_(minHeight), maxRatio_(maxRatio), minRatio_(minRatio), vpRatio_(vpRatio) {}

    bool IsEmpty() const
    {
        return (maxWidth_ == 0 || minWidth_ == 0 || maxHeight_ == 0 || minHeight_ == 0);
    }
};

/**
 * @struct TitleButtonRect
 *
 * @brief An area of title buttons relative to the upper right corner of the window.
 */
struct TitleButtonRect {
    int32_t posX_;
    int32_t posY_;
    uint32_t width_;
    uint32_t height_;

    bool operator==(const TitleButtonRect& a) const
    {
        return (posX_ == a.posX_ && posY_ == a.posY_ && width_ == a.width_ && height_ == a.height_);
    }

    bool operator!=(const TitleButtonRect& a) const
    {
        return !this->operator==(a);
    }

    bool IsUninitializedRect() const
    {
        return (posX_ == 0 && posY_ == 0 && width_ == 0 && height_ == 0);
    }

    bool IsInsideOf(const TitleButtonRect& a) const
    {
        return (posX_ >= a.posX_ && posY_ >= a.posY_ &&
            posX_ + width_ <= a.posX_ + a.width_ && posY_ + height_ <= a.posY_ + a.height_);
    }
};

/*
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

enum class CaseType {
    CASE_WINDOW_MANAGER = 0,
    CASE_WINDOW,
    CASE_STAGE
};

enum class MaximizePresentation {
    FOLLOW_APP_IMMERSIVE_SETTING = 0,  // follow app set immersiveStateEnable
    EXIT_IMMERSIVE = 1,                // immersiveStateEnable will be set as false
    ENTER_IMMERSIVE = 2,               // immersiveStateEnable will be set as true
    // immersiveStateEnable will be set as true, titleHoverShowEnabled and dockHoverShowEnabled will be set as false
    ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER = 3,
};

enum ForceHideState : uint32_t {
    NOT_HIDDEN = 0,
    HIDDEN_WHEN_FOCUSED,
    HIDDEN_WHEN_UNFOCUSED
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
    ModalityType modalityType = ModalityType::WINDOW_MODALITY;
};

/**
 * @class KeyboardLayoutParams
 *
 * @brief Keyboard need adjust layout
 */
class KeyboardLayoutParams : public Parcelable {
public:
    WindowGravity gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    Rect LandscapeKeyboardRect_ { 0, 0, 0, 0 };
    Rect PortraitKeyboardRect_ { 0, 0, 0, 0 };
    Rect LandscapePanelRect_ { 0, 0, 0, 0 };
    Rect PortraitPanelRect_ { 0, 0, 0, 0 };

    bool operator==(const KeyboardLayoutParams& params) const
    {
        return (gravity_ == params.gravity_ && LandscapeKeyboardRect_ == params.LandscapeKeyboardRect_ &&
            PortraitKeyboardRect_ == params.PortraitKeyboardRect_ &&
            LandscapePanelRect_ == params.LandscapePanelRect_ &&
            PortraitPanelRect_ == params.PortraitPanelRect_);
    }

    bool operator!=(const KeyboardLayoutParams& params) const
    {
        return !this->operator==(params);
    }

    bool isEmpty() const
    {
        return LandscapeKeyboardRect_.IsUninitializedRect() && PortraitKeyboardRect_.IsUninitializedRect() &&
            LandscapePanelRect_.IsUninitializedRect() && PortraitPanelRect_.IsUninitializedRect();
    }

    static inline bool WriteParcel(Parcel& parcel, const Rect& rect)
    {
        return parcel.WriteInt32(rect.posX_) && parcel.WriteInt32(rect.posY_) &&
            parcel.WriteUint32(rect.width_) && parcel.WriteUint32(rect.height_);
    }

    static inline bool ReadParcel(Parcel& parcel, Rect& rect)
    {
        return parcel.ReadInt32(rect.posX_) && parcel.ReadInt32(rect.posY_) &&
            parcel.ReadUint32(rect.width_) && parcel.ReadUint32(rect.height_);
    }

    virtual bool Marshalling(Parcel& parcel) const override
    {
        return (parcel.WriteUint32(static_cast<uint32_t>(gravity_)) &&
            WriteParcel(parcel, LandscapeKeyboardRect_) &&
            WriteParcel(parcel, PortraitKeyboardRect_) &&
            WriteParcel(parcel, LandscapePanelRect_) &&
            WriteParcel(parcel, PortraitPanelRect_));
    }

    static KeyboardLayoutParams* Unmarshalling(Parcel& parcel)
    {
        KeyboardLayoutParams *params = new(std::nothrow) KeyboardLayoutParams();
        if (params == nullptr) {
            return nullptr;
        }
        params->gravity_ = static_cast<WindowGravity>(parcel.ReadUint32());
        if (ReadParcel(parcel, params->LandscapeKeyboardRect_) &&
            ReadParcel(parcel, params->PortraitKeyboardRect_) &&
            ReadParcel(parcel, params->LandscapePanelRect_) &&
            ReadParcel(parcel, params->PortraitPanelRect_)) {
            return params;
        }
        delete params;
        return nullptr;
    }
};
}
}
#endif // OHOS_ROSEN_WM_COMMON_H
