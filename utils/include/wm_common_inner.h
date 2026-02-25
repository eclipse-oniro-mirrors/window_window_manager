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

#ifndef OHOS_ROSEN_WM_COMMON_INNER_H
#define OHOS_ROSEN_WM_COMMON_INNER_H

#include <cfloat>
#include <cinttypes>
#include <unordered_set>
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
class KeyboardAnimationCurve;

enum class LifeCycleEvent : uint32_t {
    CREATE_EVENT,
    SHOW_EVENT,
    HIDE_EVENT,
    DESTROY_EVENT,
};

enum class WindowStateChangeReason : uint32_t {
    NORMAL,
    KEYGUARD,
    TOGGLING,
    USER_SWITCH,
    ABILITY_CALL,
    ABILITY_HOOK,
};

enum class WindowUpdateReason : uint32_t {
    NEED_SWITCH_CASCADE_BASE,
    UPDATE_ALL = NEED_SWITCH_CASCADE_BASE,
    UPDATE_MODE,
    UPDATE_RECT,
    UPDATE_FLAGS,
    UPDATE_TYPE,
    UPDATE_ASPECT_RATIO,
    NEED_SWITCH_CASCADE_END,
    UPDATE_OTHER_PROPS,
    UPDATE_TRANSFORM,
};

enum class AvoidPosType : uint32_t {
    AVOID_POS_LEFT,
    AVOID_POS_TOP,
    AVOID_POS_RIGHT,
    AVOID_POS_BOTTOM,
    AVOID_POS_UNKNOWN
};

enum class WindowRootNodeType : uint32_t {
    APP_WINDOW_NODE,
    ABOVE_WINDOW_NODE,
    BELOW_WINDOW_NODE,
};

enum class PropertyChangeAction : uint32_t {
    ACTION_UPDATE_RECT = 1,
    ACTION_UPDATE_MODE = 1 << 1,
    ACTION_UPDATE_FLAGS = 1 << 2,
    ACTION_UPDATE_OTHER_PROPS = 1 << 3,
    ACTION_UPDATE_FOCUSABLE = 1 << 4,
    ACTION_UPDATE_TOUCHABLE = 1 << 5,
    ACTION_UPDATE_CALLING_WINDOW = 1 << 6,
    ACTION_UPDATE_ORIENTATION = 1 << 7,
    ACTION_UPDATE_TURN_SCREEN_ON = 1 << 8,
    ACTION_UPDATE_KEEP_SCREEN_ON = 1 << 9,
    ACTION_UPDATE_SET_BRIGHTNESS = 1 << 10,
    ACTION_UPDATE_MODE_SUPPORT_INFO = 1 << 11,
    ACTION_UPDATE_TOUCH_HOT_AREA = 1 << 12,
    ACTION_UPDATE_TRANSFORM_PROPERTY = 1 << 13,
    ACTION_UPDATE_ANIMATION_FLAG = 1 << 14,
    ACTION_UPDATE_PRIVACY_MODE = 1 << 15,
    ACTION_UPDATE_ASPECT_RATIO = 1 << 16,
    ACTION_UPDATE_MAXIMIZE_STATE = 1 << 17,
    ACTION_UPDATE_SYSTEM_PRIVACY_MODE = 1 << 18,
    ACTION_UPDATE_SNAPSHOT_SKIP = 1 << 19,
    ACTION_UPDATE_TEXTFIELD_AVOID_INFO = 1 << 20,
    ACTION_UPDATE_FOLLOW_SCREEN_CHANGE = 1 << 21,
};

struct ModeChangeHotZonesConfig {
    bool isModeChangeHotZoneConfigured_;
    uint32_t fullscreenRange_;
    uint32_t primaryRange_;
    uint32_t secondaryRange_;
};

struct SystemConfig : public Parcelable {
    bool isSystemDecorEnable_ = true;
    uint32_t decorWindowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;
    bool isStretchable_ = false;
    WindowMode defaultWindowMode_ = WindowMode::WINDOW_MODE_FULLSCREEN;
    KeyboardAnimationCurve animationIn_;
    KeyboardAnimationCurve animationOut_;
    WindowUIType windowUIType_ = WindowUIType::INVALID_WINDOW;
    bool supportTypeFloatWindow_ = false;

    virtual bool Marshalling(Parcel& parcel) const override
    {
        if (!parcel.WriteBool(isSystemDecorEnable_) || !parcel.WriteBool(isStretchable_) ||
            !parcel.WriteUint32(decorWindowModeSupportType_)) {
            return false;
        }

        if (!parcel.WriteUint32(static_cast<uint32_t>(defaultWindowMode_)) ||
            !parcel.WriteParcelable(&animationIn_) || !parcel.WriteParcelable(&animationOut_)) {
            return false;
        }

        if (!parcel.WriteUint8(static_cast<uint8_t>(windowUIType_))) {
            return false;
        }

        if (!parcel.WriteBool(supportTypeFloatWindow_)) {
            return false;
        }

        return true;
    }

    static SystemConfig* Unmarshalling(Parcel& parcel)
    {
        SystemConfig* config = new SystemConfig();
        config->isSystemDecorEnable_ = parcel.ReadBool();
        config->isStretchable_ = parcel.ReadBool();
        config->decorWindowModeSupportType_ = parcel.ReadUint32();
        config->defaultWindowMode_ = static_cast<WindowMode>(parcel.ReadUint32());
        sptr<KeyboardAnimationCurve> animationIn = parcel.ReadParcelable<KeyboardAnimationCurve>();
        if (animationIn == nullptr) {
            delete config;
            return nullptr;
        }
        config->animationIn_ = *animationIn;
        sptr<KeyboardAnimationCurve> animationOut = parcel.ReadParcelable<KeyboardAnimationCurve>();
        if (animationOut == nullptr) {
            delete config;
            return nullptr;
        }
        config->animationOut_ = *animationOut;
        config->windowUIType_ = static_cast<WindowUIType>(parcel.ReadUint8());
        config->supportTypeFloatWindow_ = parcel.ReadBool();
        return config;
    }

    bool IsPhoneWindow() const
    {
        return windowUIType_ == WindowUIType::PHONE_WINDOW;
    }

    bool IsPcWindow() const
    {
        return windowUIType_ == WindowUIType::PC_WINDOW;
    }

    bool IsPadWindow() const
    {
        return windowUIType_ == WindowUIType::PAD_WINDOW;
    }
};

struct ModeChangeHotZones {
    Rect fullscreen_;
    Rect primary_;
    Rect secondary_;
};

struct SplitRatioConfig {
    // when divider reaches this position, the top/left window will hide. Valid range: (0, 0.5)
    float exitSplitStartRatio;
    // when divider reaches this position, the bottom/right window will hide. Valid range: (0.5, 1)
    float exitSplitEndRatio;
    std::vector<float> splitRatios;
};

enum class DragType : uint32_t {
    DRAG_UNDEFINED,
    DRAG_LEFT_OR_RIGHT,
    DRAG_BOTTOM_OR_TOP,
    DRAG_LEFT_TOP_CORNER,
    DRAG_RIGHT_TOP_CORNER,
};

enum class TraceTaskId : int32_t {
    STARTING_WINDOW = 0,
    REMOTE_ANIMATION,
    CONNECT_EXTENSION,
    REMOTE_ANIMATION_HOME,
    START_WINDOW_ANIMATION,
};

enum class PersistentStorageType : uint32_t {
    UKNOWN = 0,
    ASPECT_RATIO,
    MAXIMIZE_STATE,
};

struct MoveDragProperty : public Parcelable {
    int32_t startPointPosX_;
    int32_t startPointPosY_;
    int32_t startPointerId_;
    int32_t targetDisplayId_;
    int32_t sourceType_;
    bool startDragFlag_;
    bool startMoveFlag_;
    bool pointEventStarted_;
    DragType dragType_;
    Rect startPointRect_;
    Rect startRectExceptFrame_;
    Rect startRectExceptCorner_;

    MoveDragProperty() : startPointPosX_(0), startPointPosY_(0), startPointerId_(0), targetDisplayId_(0),
        sourceType_(0), startDragFlag_(false), startMoveFlag_(false), pointEventStarted_(false),
        dragType_(DragType::DRAG_UNDEFINED)
    {
        startPointRect_ = {0, 0, 0, 0};
        startRectExceptFrame_ = {0, 0, 0, 0};
        startRectExceptCorner_ = {0, 0, 0, 0};
    }

    MoveDragProperty(int32_t startPointPosX, int32_t startPointPosY, int32_t startPointerId, int32_t targetDisplayId,
        int32_t sourceType, bool startDragFlag, bool startMoveFlag, bool pointEventStarted, DragType dragType,
        Rect startPointRect, Rect startRectExceptFrame, Rect startRectExceptCorner)
        : startPointPosX_(startPointPosX), startPointPosY_(startPointPosY), startPointerId_(startPointerId),
        targetDisplayId_(targetDisplayId), sourceType_(sourceType), startDragFlag_(startDragFlag),
        startMoveFlag_(startMoveFlag), pointEventStarted_(pointEventStarted), dragType_(dragType),
        startPointRect_(startPointRect), startRectExceptFrame_(startRectExceptFrame),
        startRectExceptCorner_(startRectExceptCorner) {}

    virtual bool Marshalling(Parcel& parcel) const override
    {
        if (!parcel.WriteInt32(startPointPosX_) || !parcel.WriteInt32(startPointPosY_) ||
            !parcel.WriteInt32(startPointerId_) || !parcel.WriteInt32(targetDisplayId_) ||
            !parcel.WriteInt32(sourceType_) || !parcel.WriteBool(startDragFlag_) ||
            !parcel.WriteBool(startMoveFlag_) || !parcel.WriteBool(pointEventStarted_) ||
            !parcel.WriteUint32(static_cast<uint32_t>(dragType_))) {
            return false;
        }

        if (!parcel.WriteInt32(startPointRect_.posX_) || !parcel.WriteInt32(startPointRect_.posY_) ||
            !parcel.WriteUint32(startPointRect_.width_) || !parcel.WriteUint32(startPointRect_.height_)) {
            return false;
        }

        if (!parcel.WriteInt32(startRectExceptFrame_.posX_) || !parcel.WriteInt32(startRectExceptFrame_.posY_) ||
            !parcel.WriteUint32(startRectExceptFrame_.width_) || !parcel.WriteUint32(startRectExceptFrame_.height_)) {
            return false;
        }

        if (!parcel.WriteInt32(startRectExceptCorner_.posX_) || !parcel.WriteInt32(startRectExceptCorner_.posY_) ||
            !parcel.WriteUint32(startRectExceptCorner_.width_) || !parcel.WriteUint32(startRectExceptCorner_.height_)) {
            return false;
        }

        return true;
    }

    static MoveDragProperty* Unmarshalling(Parcel& parcel)
    {
        MoveDragProperty* info = new MoveDragProperty();
        info->startPointPosX_ = parcel.ReadInt32();
        info->startPointPosY_ = parcel.ReadInt32();
        info->startPointerId_ = parcel.ReadInt32();
        info->targetDisplayId_ = parcel.ReadInt32();
        info->sourceType_ = parcel.ReadInt32();
        info->startDragFlag_ = parcel.ReadBool();
        info->startMoveFlag_ = parcel.ReadBool();
        info->pointEventStarted_ = parcel.ReadBool();
        info->dragType_ = static_cast<DragType>(parcel.ReadUint32());
        Rect startPointRect = { parcel.ReadInt32(), parcel.ReadInt32(),
                                parcel.ReadUint32(), parcel.ReadUint32() };
        Rect startRectExceptFrame = { parcel.ReadInt32(), parcel.ReadInt32(),
                                      parcel.ReadUint32(), parcel.ReadUint32() };
        Rect startRectExceptCorner = { parcel.ReadInt32(), parcel.ReadInt32(),
                                       parcel.ReadUint32(), parcel.ReadUint32() };
        info->startPointRect_ = startPointRect;
        info->startRectExceptFrame_ = startRectExceptFrame;
        info->startRectExceptCorner_ = startRectExceptCorner;
        return info;
    }

    void CopyFrom(const sptr<MoveDragProperty>& property)
    {
        startPointPosX_ = property->startPointPosX_;
        startPointPosY_ = property->startPointPosY_;
        startPointerId_ = property->startPointerId_;
        targetDisplayId_ = property->targetDisplayId_;
        sourceType_ = property->sourceType_;
        startDragFlag_ = property->startDragFlag_;
        startMoveFlag_ = property->startMoveFlag_;
        pointEventStarted_ = property->pointEventStarted_;
        dragType_ = property->dragType_;
        startPointRect_ = property->startPointRect_;
        startRectExceptFrame_ = property->startRectExceptFrame_;
        startRectExceptCorner_ = property->startRectExceptCorner_;
    }
};

struct AbilityInfo {
    std::string bundleName_ = "";
    std::string abilityName_ = "";
    int32_t missionId_ = -1;
};

namespace {
    constexpr float DEFAULT_SPLIT_RATIO = 0.5;
    constexpr float DEFAULT_ASPECT_RATIO = 0.67;
    constexpr float DISPLAY_ZOOM_OFF_SCALE = 1.0;
    constexpr float DISPLAY_ZOOM_MIN_SCALE = 2.0;
    constexpr float DISPLAY_ZOOM_MAX_SCALE = 8.0;
    constexpr int32_t IVALID_DIALOG_WINDOW_ID = -1;
    constexpr uint32_t DIVIDER_WIDTH = 8;
    constexpr uint32_t WINDOW_TITLE_BAR_HEIGHT = 48;
    constexpr uint32_t WINDOW_FRAME_WIDTH = 5;
    constexpr uint32_t WINDOW_FRAME_CORNER_WIDTH = 16; // the frame width of corner
    constexpr uint32_t WINDOW_FRAME_CORNER_TOUCH_WIDTH = 32; // the frame width of corner for touch
    constexpr uint32_t HOTZONE_TOUCH = 24;
    constexpr uint32_t HOTZONE_POINTER = 4;
    constexpr uint32_t MIN_FLOATING_WIDTH = 320;
    constexpr uint32_t MIN_FLOATING_HEIGHT = 240;
    constexpr uint32_t MIN_VERTICAL_SPLIT_HEIGHT = 240;
    constexpr uint32_t MIN_HORIZONTAL_SPLIT_WIDTH = 320;
    constexpr unsigned int WMS_WATCHDOG_CHECK_INTERVAL = 6; // actual check interval is 3000ms(6 * 500)
    const Rect INVALID_EMPTY_RECT = {0, 0, 0, 0};
    const Rect DEFAULT_PLACE_HOLDER_RECT = {0, 0, 512, 512};
    constexpr int32_t SNAPSHOT_TIMEOUT_MS = 300;
    const std::unordered_set<WindowType> INPUT_WINDOW_TYPE_SKIPPED {
        WindowType::WINDOW_TYPE_POINTER,
        WindowType::WINDOW_TYPE_DRAGGING_EFFECT,
    };
}
}
}
#endif // OHOS_ROSEN_WM_COMMON_INNER_H