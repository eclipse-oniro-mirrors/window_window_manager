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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SCREEN_PROPERTY_H
#define OHOS_ROSEN_WINDOW_SCENE_SCREEN_PROPERTY_H

#include "common/rs_rect.h"
#include "dm_common.h"
#include "class_var_definition.h"
#include "screen_info.h"

namespace OHOS::Rosen {

enum class ScreenPropertyChangeReason : uint32_t {
    UNDEFINED = 0,
    ROTATION,
    CHANGE_MODE,
    FOLD_SCREEN_EXPAND,
    SCREEN_CONNECT,
    SCREEN_DISCONNECT,
    FOLD_SCREEN_FOLDING,
    VIRTUAL_SCREEN_RESIZE,
    RELATIVE_POSITION_CHANGE,
    SUPER_FOLD_STATUS_CHANGE,
    VIRTUAL_PIXEL_RATIO_CHANGE,
    ACCESS_INFO_CHANGE,
    FOLD_SCREEN_EXPAND_SWITCH_USER,
    FOLD_SCREEN_FOLDING_SWITCH_USER,
    SCREEN_MODE_CHANGE,
    ACTIVE_MODE_CHANGE,
    RESOLUTION_EFFECT_CHANGE,
};
class ScreenProperty {
public:
    ScreenProperty() = default;
    ~ScreenProperty() = default;

    void SetRotation(float rotation);
    float GetRotation() const;

    void SetBounds(const RRect& bounds);
    RRect GetBounds() const;

    void SetPhyBounds(const RRect& phyBounds);
    RRect GetPhyBounds() const;

    void SetFakeBounds(const RRect& fakeBounds);
    RRect GetFakeBounds() const;

    void SetIsFakeInUse(bool isFakeInUse);
    bool GetIsFakeInUse() const;

    void SetIsDestroyDisplay(bool isPreFakeInUse);
    bool GetIsDestroyDisplay() const;

    void SetScaleX(float scaleX);
    float GetScaleX() const;

    void SetScaleY(float scaleY);
    float GetScaleY() const;

    void SetPivotX(float pivotX);
    float GetPivotX() const;

    void SetPivotY(float pivotY);
    float GetPivotY() const;

    void SetTranslateX(float translateX);
    float GetTranslateX() const;

    void SetTranslateY(float translateY);
    float GetTranslateY() const;

    float GetDensity();
    float GetDefaultDensity();
    void SetDefaultDensity(float defaultDensity);

    float GetDensityInCurResolution() const;
    void SetDensityInCurResolution(float densityInCurResolution);

    void SetPhyWidth(uint32_t phyWidth);
    int32_t GetPhyWidth() const;

    void SetPhyHeight(uint32_t phyHeight);
    int32_t GetPhyHeight() const;

    void SetDpiPhyBounds(uint32_t phyWidth, uint32_t phyHeight);

    void SetRefreshRate(uint32_t refreshRate);
    uint32_t GetRefreshRate() const;

    void SetRsId(ScreenId rsId);
    ScreenId GetRsId() const;

    void SetPropertyChangeReason(ScreenPropertyChangeReason propertyChangeReason);
    ScreenPropertyChangeReason GetPropertyChangeReason() const;

    void SetDefaultDeviceRotationOffset(uint32_t defaultRotationOffset);
    uint32_t GetDefaultDeviceRotationOffset() const;

    void UpdateVirtualPixelRatio(const RRect& bounds);
    void SetVirtualPixelRatio(float virtualPixelRatio);
    float GetVirtualPixelRatio() const;

    void SetScreenDensityProperties(float screenDpi);

    void SetScreenRotation(Rotation rotation);
    void SetRotationAndScreenRotationOnly(Rotation rotation);
    Rotation GetScreenRotation() const;
    void UpdateScreenRotation(Rotation rotation);

    Rotation GetDeviceRotation() const;
    void UpdateDeviceRotation(Rotation rotation);

    void SetOrientation(Orientation orientation);
    Orientation GetOrientation() const;

    void SetDisplayState(DisplayState displayState);
    DisplayState GetDisplayState() const;

    void SetDisplayOrientation(DisplayOrientation displayOrientation);
    DisplayOrientation GetDisplayOrientation() const;
    void CalcDefaultDisplayOrientation();

    void SetDeviceOrientation(DisplayOrientation displayOrientation);
    DisplayOrientation GetDeviceOrientation() const;

    void SetPhysicalRotation(float rotation);
    float GetPhysicalRotation() const;

    void SetScreenComponentRotation(float rotation);
    float GetScreenComponentRotation() const;

    float GetXDpi() const;
    float GetYDpi() const;

    void SetOffsetX(int32_t offsetX);
    int32_t GetOffsetX() const;

    void SetOffsetY(int32_t offsetY);
    int32_t GetOffsetY() const;

    void SetOffset(int32_t offsetX, int32_t offsetY);

    void SetStartX(uint32_t startX);
    uint32_t GetStartX() const;

    void SetStartY(uint32_t startY);
    uint32_t GetStartY() const;

    void SetValidHeight(uint32_t validHeight);
    uint32_t GetValidHeight() const;

    void SetValidWidth(uint32_t validWidth);
    uint32_t GetValidWidth() const;

    void SetStartPosition(uint32_t startX, uint32_t startY);

    void SetScreenType(ScreenType type);
    ScreenType GetScreenType() const;

    void SetScreenRequestedOrientation(Orientation orientation);
    Orientation GetScreenRequestedOrientation() const;

    void SetScreenShape(ScreenShape screenShape);
    ScreenShape GetScreenShape() const;

    void SetX(int32_t x);
    int32_t GetX() const;

    void SetY(int32_t y);
    int32_t GetY() const;

    void SetXYPosition(int32_t x, int32_t y);

    DMRect GetAvailableArea()
    {
        return availableArea_;
    }

    void SetAvailableArea(DMRect area)
    {
        availableArea_ = area;
    }

    DMRect GetExpandAvailableArea()
    {
        return expandAvailableArea_;
    }

    void SetExpandAvailableArea(DMRect area)
    {
        expandAvailableArea_ = area;
    }

    DMRect GetCreaseRect()
    {
        return creaseRect_;
    }

    void SetCreaseRect(DMRect creaseRect)
    {
        creaseRect_ = creaseRect;
    }

    FoldDisplayMode GetDisplayMode() const;
    void SetDisplayMode(FoldDisplayMode mode);
    RRect GetPhysicalTouchBounds() const;

    void SetPhysicalTouchBounds(Rotation rotationOffset);
    void SetPhysicalTouchBoundsDirectly(RRect physicalTouchBounds);

    int32_t GetInputOffsetX() const;

    int32_t GetInputOffsetY() const;

    void SetInputOffset(int32_t x, int32_t y);
    void SetMirrorWidth(uint32_t mirrorWidth);
    uint32_t GetMirrorWidth() const;
    void SetMirrorHeight(uint32_t mirrorHeight);
    uint32_t GetMirrorHeight() const;

    float CalculatePPI();
    uint32_t CalculateDPI();

    void SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents changeEvent)
    {
        changeEvent_ = changeEvent;
    }
    SuperFoldStatusChangeEvents GetSuperFoldStatusChangeEvent() const
    {
        return changeEvent_;
    }

    void SetCurrentValidHeight(uint32_t currentValidHeight)
    {
        currentValidHeight_ = currentValidHeight;
    }
    uint32_t GetCurrentValidHeight() const
    {
        return currentValidHeight_;
    }

    void SetIsKeyboardOn(bool isKeyboardOn)
    {
        isKeyboardOn_ = isKeyboardOn;
    }

    bool GetIsKeyboardOn() const
    {
        return isKeyboardOn_;
    }

    void SetFoldStatus(SuperFoldStatus status)
    {
        foldStatus_ = status;
    }

    SuperFoldStatus GetFoldStatus() const
    {
        return foldStatus_;
    }

    // OffScreenRender
    void SetCurrentOffScreenRendering(bool enable) { isCurrentOffScreenRendering_ = enable; }
    bool GetCurrentOffScreenRendering() { return isCurrentOffScreenRendering_; }
    void SetScreenRealWidth(uint32_t width) { screenRealWidth_ = width; }
    uint32_t GetScreenRealWidth() { return screenRealWidth_; }
    void SetScreenRealHeight(uint32_t height) { screenRealHeight_ = height; }
    uint32_t GetScreenRealHeight() { return screenRealHeight_; }
    void SetScreenRealPPI() { screenRealPPI_ = CalculatePPI(); }
    float GetScreenRealPPI() { return screenRealPPI_; }
    void SetScreenRealDPI() { screenRealDPI_ = CalculateDPI(); }
    uint32_t GetScreenRealDPI() { return screenRealDPI_; }

    void SetPointerActiveWidth(uint32_t pointerActiveWidth);
    uint32_t GetPointerActiveWidth() const;
    void SetPointerActiveHeight(uint32_t pointerActiveHeight);
    uint32_t GetPointerActiveHeight() const;

    // displayInfo
    void SetDisplayGroupId(DisplayGroupId displayGroupId) { displayGroupId_ = displayGroupId; }
    DisplayGroupId GetDisplayGroupId() const { return displayGroupId_; }
    void SetMainDisplayIdOfGroup(ScreenId screenId) { mainDisplayIdOfGroup_ = screenId; }
    ScreenId GetMainDisplayIdOfGroup() const { return mainDisplayIdOfGroup_; }
    void SetScreenAreaOffsetX(uint32_t screenAreaOffsetX) { screenAreaOffsetX_ = screenAreaOffsetX; }
    uint32_t GetScreenAreaOffsetX() const { return screenAreaOffsetX_; }
    void SetScreenAreaOffsetY(uint32_t screenAreaOffsetY) { screenAreaOffsetY_ = screenAreaOffsetY; }
    uint32_t GetScreenAreaOffsetY() const { return screenAreaOffsetY_; }
    void SetScreenAreaWidth(uint32_t screenAreaWidth) { screenAreaWidth_ = screenAreaWidth; }
    uint32_t GetScreenAreaWidth() const { return screenAreaWidth_; }
    void SetScreenAreaHeight(uint32_t screenAreaHeight) { screenAreaHeight_ = screenAreaHeight; }
    uint32_t GetScreenAreaHeight() const { return screenAreaHeight_; }

private:
    SuperFoldStatusChangeEvents changeEvent_ {SuperFoldStatusChangeEvents::UNDEFINED};
    static inline bool IsVertical(Rotation rotation)
    {
        return (rotation == Rotation::ROTATION_0 || rotation == Rotation::ROTATION_180);
    }
    DisplayGroupId displayGroupId_ = DISPLAY_GROUP_ID_INVALID;
    ScreenId mainDisplayIdOfGroup_ = SCREEN_ID_INVALID;
    float rotation_ { 0.0f };
    float physicalRotation_ { 0.0f };
    float screenComponentRotation_ { 0.0f };
    RRect bounds_;
    RRect phyBounds_;
    RRect fakeBounds_;
    bool isFakeInUse_ = false;  // is fakeBounds can be used
    bool isDestroyDisplay_ = false;  // is fakeBounds can be used

    float scaleX_ { 1.0f };
    float scaleY_ { 1.0f };
    float pivotX_ { 0.5f };
    float pivotY_ { 0.5f };
    float translateX_ { 0.0f };
    float translateY_ { 0.0f };

    uint32_t phyWidth_ { UINT32_MAX };
    uint32_t phyHeight_ { UINT32_MAX };

    uint32_t dpiPhyWidth_ { UINT32_MAX };
    uint32_t dpiPhyHeight_ { UINT32_MAX };

    uint32_t refreshRate_ { 0 };
    uint32_t defaultDeviceRotationOffset_ { 0 };

    ScreenId rsId_ = SCREEN_ID_INVALID;

    ScreenPropertyChangeReason propertyChangeReason_;

    float virtualPixelRatio_ { 1.0f };
    float defaultDensity_ { 1.0f };
    float densityInCurResolution_ { 1.0f };

    Orientation orientation_ { Orientation::UNSPECIFIED };
    DisplayOrientation displayOrientation_ { DisplayOrientation::UNKNOWN };
    DisplayOrientation deviceOrientation_ { DisplayOrientation::UNKNOWN };
    Rotation screenRotation_ { Rotation::ROTATION_0 };
    Rotation deviceRotation_ { Rotation::ROTATION_0 };
    Orientation screenRequestedOrientation_ { Orientation::UNSPECIFIED };
    DisplayState displayState_ { DisplayState::UNKNOWN };

    float xDpi_ { 0.0f };
    float yDpi_ { 0.0f };

    int32_t offsetX_ { 0 };
    int32_t offsetY_ { 0 };

    uint32_t startX_ { 0 };
    uint32_t startY_ { 0 };

    int32_t x_ { 0 };
    int32_t y_ { 0 };

    uint32_t validWidth_ { UINT32_MAX };
    uint32_t validHeight_ { UINT32_MAX };

    int32_t currentValidHeight_ { UINT32_MAX };
    bool isKeyboardOn_ { false };

    uint32_t pointerActiveWidth_ { 0 };
    uint32_t pointerActiveHeight_ { 0 };

    ScreenShape screenShape_ { ScreenShape::RECTANGLE };
    SuperFoldStatus foldStatus_ { SuperFoldStatus::UNKNOWN };

    ScreenType type_ { ScreenType::REAL };

    void UpdateXDpi();
    void UpdateYDpi();
    void CalculateXYDpi(uint32_t phyWidth, uint32_t phyHeight);
    DMRect availableArea_;  // can be used for all devices
    DMRect expandAvailableArea_;  // only used for 2in1 device
    DMRect creaseRect_;

    RRect physicalTouchBounds_;
    int32_t inputOffsetX_ { 0 };
    int32_t inputOffsetY_ { 0 };

    // OffScreenRender
    bool isCurrentOffScreenRendering_ { false };
    uint32_t screenRealWidth_ { UINT32_MAX };
    uint32_t screenRealHeight_ { UINT32_MAX };
    float screenRealPPI_ { 0.0f };
    uint32_t screenRealDPI_ { 0 };

    // screenArea
    uint32_t screenAreaOffsetX_ { 0 };
    uint32_t screenAreaOffsetY_ { 0 };
    uint32_t screenAreaWidth_ { 0 };
    uint32_t screenAreaHeight_ { 0 };

    uint32_t mirrorWidth_ { 0 };
    uint32_t mirrorHeight_ { 0 };

    FoldDisplayMode displayMode_;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SCREEN_PROPERTY_H
