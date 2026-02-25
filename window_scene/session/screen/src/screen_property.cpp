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

#include "session/screen/include/screen_property.h"
#include "parameters.h"
#include "fold_screen_state_internel.h"

namespace OHOS::Rosen {
namespace {
constexpr int32_t PHONE_SCREEN_WIDTH = 1344;
constexpr int32_t PHONE_SCREEN_HEIGHT = 2772;
constexpr float PHONE_SCREEN_DENSITY = 3.5f;
constexpr float ELSE_SCREEN_DENSITY = 1.5f;
constexpr float INCH_2_MM = 25.4f;
constexpr int32_t HALF_VALUE = 2;
constexpr int32_t TRUNCATE_TWO_DECIMALS = 100;
constexpr int32_t TRUNCATE_THREE_DECIMALS = 1000;
constexpr float SECONDARY_ROTATION_90 = 90.0F;
constexpr float SECONDARY_ROTATION_270 = 270.0F;
constexpr float SECONDARY_ROTATION_360 = 360.0F;
constexpr float EPSILON = 1e-6f;
constexpr float PPI_TO_DPI = 1.6f;
}

void ScreenProperty::SetRotation(float rotation)
{
    rotation_ = rotation;
}

float ScreenProperty::GetRotation() const
{
    return rotation_;
}

void ScreenProperty::SetPhysicalRotation(float rotation)
{
    physicalRotation_ = rotation;
}

float ScreenProperty::GetPhysicalRotation() const
{
    return physicalRotation_;
}

void ScreenProperty::SetScreenComponentRotation(float rotation)
{
    screenComponentRotation_ = rotation;
}

float ScreenProperty::GetScreenComponentRotation() const
{
    return screenComponentRotation_;
}

void ScreenProperty::SetBounds(const RRect& bounds)
{
    bounds_ = bounds;
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        physicalTouchBounds_.rect_.width_ = bounds_.rect_.width_;
        physicalTouchBounds_.rect_.height_ = bounds_.rect_.height_;
        physicalTouchBounds_.rect_.left_ = bounds_.rect_.left_;
        physicalTouchBounds_.rect_.top_ = bounds_.rect_.top_;
    }
    UpdateXDpi();
    UpdateYDpi();
}

RRect ScreenProperty::GetBounds() const
{
    return bounds_;
}

void ScreenProperty::SetFakeBounds(const RRect& fakeBounds)
{
    fakeBounds_ = fakeBounds;
}

RRect ScreenProperty::GetFakeBounds() const
{
    return fakeBounds_;
}

void ScreenProperty::SetIsFakeInUse(bool isFakeInUse)
{
    isFakeInUse_ = isFakeInUse;
}

bool ScreenProperty::GetIsFakeInUse() const
{
    return isFakeInUse_;
}

void ScreenProperty::SetIsDestroyDisplay(bool isPreFakeInUse)
{
    isDestroyDisplay_ = isPreFakeInUse;
}

bool ScreenProperty::GetIsDestroyDisplay() const
{
    return isDestroyDisplay_;
}

void ScreenProperty::SetScaleX(float scaleX)
{
    scaleX_ = scaleX;
}

float ScreenProperty::GetScaleX() const
{
    return scaleX_;
}

void ScreenProperty::SetScaleY(float scaleY)
{
    scaleY_ = scaleY;
}

float ScreenProperty::GetScaleY() const
{
    return scaleY_;
}

void ScreenProperty::SetPivotX(float pivotX)
{
    pivotX_ = pivotX;
}

float ScreenProperty::GetPivotX() const
{
    return pivotX_;
}

void ScreenProperty::SetPivotY(float pivotY)
{
    pivotY_ = pivotY;
}

float ScreenProperty::GetPivotY() const
{
    return pivotY_;
}

void ScreenProperty::SetTranslateX(float translateX)
{
    translateX_ = translateX;
}

float ScreenProperty::GetTranslateX() const
{
    return translateX_;
}

void ScreenProperty::SetTranslateY(float translateY)
{
    translateY_ = translateY;
}

float ScreenProperty::GetTranslateY() const
{
    return translateY_;
}

void ScreenProperty::SetPhyBounds(const RRect& phyBounds)
{
    phyBounds_ = phyBounds;
}

RRect ScreenProperty::GetPhyBounds() const
{
    return phyBounds_;
}

float ScreenProperty::GetDensity()
{
    return virtualPixelRatio_;
}

void ScreenProperty::SetScreenDensityProperties(float screenDpi)
{
    SetVirtualPixelRatio(screenDpi);
    SetDefaultDensity(screenDpi);
    SetDensityInCurResolution(screenDpi);
}

float ScreenProperty::GetDefaultDensity()
{
    return defaultDensity_;
}

void ScreenProperty::SetDefaultDensity(float defaultDensity)
{
    defaultDensity_ = defaultDensity;
}

float ScreenProperty::GetDensityInCurResolution() const
{
    return densityInCurResolution_;
}

void ScreenProperty::SetDensityInCurResolution(float densityInCurResolution)
{
    densityInCurResolution_ = densityInCurResolution;
}

void ScreenProperty::SetValidHeight(uint32_t validHeight)
{
    validHeight_ = validHeight;
}
 
uint32_t ScreenProperty::GetValidHeight() const
{
    return validHeight_;
}
 
void ScreenProperty::SetValidWidth(uint32_t validWidth)
{
    validWidth_ = validWidth;
}
 
uint32_t ScreenProperty::GetValidWidth() const
{
    return validWidth_;
}

void ScreenProperty::SetPhyWidth(uint32_t phyWidth)
{
    phyWidth_ = phyWidth;
}

int32_t ScreenProperty::GetPhyWidth() const
{
    return phyWidth_;
}

void ScreenProperty::SetPhyHeight(uint32_t phyHeight)
{
    phyHeight_ = phyHeight;
}

int32_t ScreenProperty::GetPhyHeight() const
{
    return phyHeight_;
}

void ScreenProperty::SetDpiPhyBounds(uint32_t phyWidth, uint32_t phyHeight)
{
    dpiPhyWidth_ = phyWidth;
    dpiPhyHeight_ = phyHeight;
}

void ScreenProperty::SetRefreshRate(uint32_t refreshRate)
{
    refreshRate_ = refreshRate;
}

uint32_t ScreenProperty::GetRefreshRate() const
{
    return refreshRate_;
}

void ScreenProperty::SetRsId(ScreenId rsId)
{
    rsId_ = rsId;
}

ScreenId ScreenProperty::GetRsId() const
{
    return rsId_;
}

void ScreenProperty::SetPropertyChangeReason(ScreenPropertyChangeReason propertyChangeReason)
{
    propertyChangeReason_ = propertyChangeReason;
}

ScreenPropertyChangeReason ScreenProperty::GetPropertyChangeReason() const
{
    return propertyChangeReason_;
}

void ScreenProperty::SetVirtualPixelRatio(float virtualPixelRatio)
{
    virtualPixelRatio_ = virtualPixelRatio;
}

float ScreenProperty::GetVirtualPixelRatio() const
{
    return virtualPixelRatio_;
}

void ScreenProperty::SetScreenRotation(Rotation rotation)
{
    bool enableRotation = (system::GetParameter("persist.window.rotation.enabled", "1") == "1");
    if (!enableRotation) {
        return;
    }
    if (IsVertical(rotation) != IsVertical(screenRotation_)) {
        std::swap(bounds_.rect_.width_, bounds_.rect_.height_);
        int32_t width = bounds_.rect_.width_;
        int32_t height = bounds_.rect_.height_;
        if (IsVertical(screenRotation_)) {
            bounds_.rect_.left_ -= static_cast<float>(width - height) / static_cast<float>(HALF_VALUE) -
                static_cast<float>(offsetY_);
            bounds_.rect_.top_ += static_cast<float>(width - height) / static_cast<float>(HALF_VALUE);
        } else {
            bounds_.rect_.left_ += static_cast<float>(height - width) / static_cast<float>(HALF_VALUE);
            bounds_.rect_.top_ -= static_cast<float>(height - width) / static_cast<float>(HALF_VALUE) +
                static_cast<float>(offsetY_);
        }
    }
    switch (rotation) {
        case Rotation::ROTATION_90:
            rotation_ = 90.f;
            break;
        case Rotation::ROTATION_180:
            rotation_ = 180.f;
            break;
        case Rotation::ROTATION_270:
            rotation_ = 270.f;
            break;
        default:
            rotation_ = 0.f;
            break;
    }
    screenRotation_ = rotation;
}

void ScreenProperty::SetRotationAndScreenRotationOnly(Rotation rotation)
{
    bool enableRotation = (system::GetParameter("persist.window.rotation.enabled", "1") == "1");
    if (!enableRotation) {
        return;
    }
    switch (rotation) {
        case Rotation::ROTATION_90:
            rotation_ = 90.f;
            break;
        case Rotation::ROTATION_180:
            rotation_ = 180.f;
            break;
        case Rotation::ROTATION_270:
            rotation_ = 270.f;
            break;
        default:
            rotation_ = 0.f;
            break;
    }
    screenRotation_ = rotation;
    deviceRotation_ = rotation;
}

void ScreenProperty::UpdateScreenRotation(Rotation rotation)
{
    screenRotation_ = rotation;
}

Rotation ScreenProperty::GetScreenRotation() const
{
    return screenRotation_;
}

void ScreenProperty::UpdateDeviceRotation(Rotation rotation)
{
    deviceRotation_ = rotation;
}

Rotation ScreenProperty::GetDeviceRotation() const
{
    return deviceRotation_;
}

void ScreenProperty::SetOrientation(Orientation orientation)
{
    orientation_ = orientation;
}

Orientation ScreenProperty::GetOrientation() const
{
    return orientation_;
}

void ScreenProperty::SetDisplayState(DisplayState displayState)
{
    displayState_ = displayState;
}

DisplayState ScreenProperty::GetDisplayState() const
{
    return displayState_;
}

void ScreenProperty::SetDisplayOrientation(DisplayOrientation displayOrientation)
{
    displayOrientation_ = displayOrientation;
}

DisplayOrientation ScreenProperty::GetDisplayOrientation() const
{
    return displayOrientation_;
}

void ScreenProperty::SetDeviceOrientation(DisplayOrientation displayOrientation)
{
    deviceOrientation_ = displayOrientation;
}

DisplayOrientation ScreenProperty::GetDeviceOrientation() const
{
    return deviceOrientation_;
}

void ScreenProperty::UpdateXDpi()
{
    if (dpiPhyWidth_ != UINT32_MAX) {
        int32_t width = phyBounds_.rect_.width_;
        xDpi_ = width * INCH_2_MM / dpiPhyWidth_;
        xDpi_ = std::floor(xDpi_ * TRUNCATE_THREE_DECIMALS) / TRUNCATE_THREE_DECIMALS;
    }
}

void ScreenProperty::UpdateYDpi()
{
    if (dpiPhyHeight_ != UINT32_MAX) {
        int32_t height_ = phyBounds_.rect_.height_;
        yDpi_ = height_ * INCH_2_MM / dpiPhyHeight_;
        yDpi_ = std::floor(yDpi_ * TRUNCATE_THREE_DECIMALS) / TRUNCATE_THREE_DECIMALS;
    }
}

void ScreenProperty::UpdateVirtualPixelRatio(const RRect& bounds)
{
    int32_t width = bounds.rect_.width_;
    int32_t height = bounds.rect_.height_;

    if (width == PHONE_SCREEN_WIDTH && height == PHONE_SCREEN_HEIGHT) { // telephone
        virtualPixelRatio_ = PHONE_SCREEN_DENSITY;
    } else {
        virtualPixelRatio_ = ELSE_SCREEN_DENSITY;
    }
    defaultDensity_ = virtualPixelRatio_;
}

void ScreenProperty::CalcDefaultDisplayOrientation()
{
    if (bounds_.rect_.width_ > bounds_.rect_.height_) {
        displayOrientation_ = DisplayOrientation::LANDSCAPE;
        deviceOrientation_ = DisplayOrientation::LANDSCAPE;
    } else {
        displayOrientation_ = DisplayOrientation::PORTRAIT;
        deviceOrientation_ = DisplayOrientation::PORTRAIT;
    }
}

void ScreenProperty::CalculateXYDpi(uint32_t phyWidth, uint32_t phyHeight)
{
    if (phyWidth == 0 || phyHeight == 0) {
        return;
    }

    phyWidth_ = phyWidth;
    phyHeight_ = phyHeight;
    int32_t width_ = phyBounds_.rect_.width_;
    int32_t height_ = phyBounds_.rect_.height_;
    xDpi_ = width_ * INCH_2_MM / phyWidth_;
    yDpi_ = height_ * INCH_2_MM / phyHeight_;
    xDpi_ = std::floor(xDpi_ * TRUNCATE_THREE_DECIMALS) / TRUNCATE_THREE_DECIMALS;
    yDpi_ = std::floor(yDpi_ * TRUNCATE_THREE_DECIMALS) / TRUNCATE_THREE_DECIMALS;
}

float ScreenProperty::GetXDpi() const
{
    return xDpi_;
}

float ScreenProperty::GetYDpi() const
{
    return yDpi_;
}

void ScreenProperty::SetOffsetX(int32_t offsetX)
{
    offsetX_ = offsetX;
}

int32_t ScreenProperty::GetOffsetX() const
{
    return offsetX_;
}

void ScreenProperty::SetOffsetY(int32_t offsetY)
{
    offsetY_ = offsetY;
}

int32_t ScreenProperty::GetOffsetY() const
{
    return offsetY_;
}

void ScreenProperty::SetMirrorWidth(uint32_t mirrorWidth)
{
    mirrorWidth_ = mirrorWidth;
}

uint32_t ScreenProperty::GetMirrorWidth() const
{
    return mirrorWidth_;
}

void ScreenProperty::SetMirrorHeight(uint32_t mirrorHeight)
{
    mirrorHeight_ = mirrorHeight;
}

uint32_t ScreenProperty::GetMirrorHeight() const
{
    return mirrorHeight_;
}

void ScreenProperty::SetOffset(int32_t offsetX, int32_t offsetY)
{
    offsetX_ = offsetX;
    offsetY_ = offsetY;
}

void ScreenProperty::SetStartX(uint32_t startX)
{
    startX_ = startX;
}

uint32_t ScreenProperty::GetStartX() const
{
    return startX_;
}

void ScreenProperty::SetStartY(uint32_t startY)
{
    startY_ = startY;
}

uint32_t ScreenProperty::GetStartY() const
{
    return startY_;
}

void ScreenProperty::SetStartPosition(uint32_t startX, uint32_t startY)
{
    startX_ = startX;
    startY_ = startY;
}

void ScreenProperty::SetScreenType(ScreenType type)
{
    type_ = type;
}

ScreenType ScreenProperty::GetScreenType() const
{
    return type_;
}

void ScreenProperty::SetScreenRequestedOrientation(Orientation orientation)
{
    screenRequestedOrientation_ = orientation;
}

Orientation ScreenProperty::GetScreenRequestedOrientation() const
{
    return screenRequestedOrientation_;
}

void ScreenProperty::SetDefaultDeviceRotationOffset(uint32_t defaultRotationOffset)
{
    defaultDeviceRotationOffset_ = defaultRotationOffset;
}

uint32_t ScreenProperty::GetDefaultDeviceRotationOffset() const
{
    return defaultDeviceRotationOffset_;
}

void ScreenProperty::SetScreenShape(ScreenShape screenShape)
{
    screenShape_ = screenShape;
}

ScreenShape ScreenProperty::GetScreenShape() const
{
    return screenShape_;
}

void ScreenProperty::SetX(int32_t x)
{
    x_ = x;
}

int32_t ScreenProperty::GetX() const
{
    return x_;
}

void ScreenProperty::SetY(int32_t y)
{
    y_ = y;
}

int32_t ScreenProperty::GetY() const
{
    return y_;
}

void ScreenProperty::SetXYPosition(int32_t x, int32_t y)
{
    x_ = x;
    y_ = y;
}

RRect ScreenProperty::GetPhysicalTouchBounds() const
{
    return physicalTouchBounds_;
}

void ScreenProperty::SetPhysicalTouchBounds(Rotation rotationOffset)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        physicalTouchBounds_.rect_.width_ = bounds_.rect_.width_;
        physicalTouchBounds_.rect_.height_ = bounds_.rect_.height_;
        return;
    }
    float correctionValue = rotation_ - static_cast<float>(rotationOffset) * SECONDARY_ROTATION_90 +
        SECONDARY_ROTATION_360;
    float correctionRotation = static_cast<float>(static_cast<uint32_t>(correctionValue) % 360);
    if (std::fabs(correctionRotation - SECONDARY_ROTATION_90) < FLT_EPSILON ||
        std::fabs(correctionRotation - SECONDARY_ROTATION_270) < FLT_EPSILON) {
        physicalTouchBounds_.rect_.width_ = phyBounds_.rect_.width_;
        physicalTouchBounds_.rect_.height_ = phyBounds_.rect_.height_;
    } else {
        physicalTouchBounds_.rect_.width_ = phyBounds_.rect_.height_;
        physicalTouchBounds_.rect_.height_ = phyBounds_.rect_.width_;
    }
}

void ScreenProperty::SetPhysicalTouchBoundsDirectly(RRect physicalTouchBounds)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        return;
    }
    physicalTouchBounds_ = physicalTouchBounds;
}

int32_t ScreenProperty::GetInputOffsetX() const
{
    return inputOffsetX_;
}

int32_t ScreenProperty::GetInputOffsetY() const
{
    return inputOffsetY_;
}

void ScreenProperty::SetInputOffset(int32_t x, int32_t y)
{
    inputOffsetX_ = x;
    inputOffsetY_ = y;
}

float ScreenProperty::CalculatePPI()
{
    int32_t phywidth = GetPhyWidth();
    int32_t phyHeight = GetPhyHeight();
    float phyDiagonal = std::sqrt(static_cast<float>(phywidth * phywidth + phyHeight * phyHeight));
    if (phyDiagonal < EPSILON) {
        return 0.0f;
    }
    RRect bounds = GetBounds();
    int32_t width = bounds.rect_.GetWidth();
    int32_t height = bounds.rect_.GetHeight();
    float ppi = std::sqrt(static_cast<float>(width * width + height * height)) * INCH_2_MM / phyDiagonal;
    return std::round(ppi * TRUNCATE_TWO_DECIMALS) / TRUNCATE_TWO_DECIMALS;
}

uint32_t ScreenProperty::CalculateDPI()
{
    return static_cast<uint32_t>(std::round(CalculatePPI() * PPI_TO_DPI));
}

void ScreenProperty::SetPointerActiveWidth(uint32_t pointerActiveWidth)
{
    pointerActiveWidth_ = pointerActiveWidth;
}

uint32_t ScreenProperty::GetPointerActiveWidth() const
{
    return pointerActiveWidth_;
}

void ScreenProperty::SetPointerActiveHeight(uint32_t pointerActiveHeight)
{
    pointerActiveHeight_ = pointerActiveHeight;
}

uint32_t ScreenProperty::GetPointerActiveHeight() const
{
    return pointerActiveHeight_;
}

FoldDisplayMode ScreenProperty::GetDisplayMode() const
{
    return displayMode_;
}

void ScreenProperty::SetDisplayMode(FoldDisplayMode mode)
{
    displayMode_ = mode;
}

} // namespace OHOS::Rosen
