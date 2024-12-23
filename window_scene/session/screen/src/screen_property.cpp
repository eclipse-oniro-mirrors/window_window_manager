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

namespace OHOS::Rosen {
namespace {
constexpr int32_t PHONE_SCREEN_WIDTH = 1344;
constexpr int32_t PHONE_SCREEN_HEIGHT = 2772;
constexpr float PHONE_SCREEN_DENSITY = 3.5f;
constexpr float ELSE_SCREEN_DENSITY = 1.5f;
constexpr float INCH_2_MM = 25.4f;
constexpr int32_t HALF_VALUE = 2;
constexpr int32_t TRUNCATE_THREE_DECIMALS = 1000;
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
    UpdateXDpi();
    UpdateYDpi();
}

RRect ScreenProperty::GetBounds() const
{
    return bounds_;
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
    bool enableRotation = system::GetParameter("persist.window.rotation.enabled", "1") == "1";
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

float ScreenProperty::GetXDpi()
{
    return xDpi_;
}

float ScreenProperty::GetYDpi()
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

void ScreenProperty::SetOffset(int32_t offsetX, int32_t offsetY)
{
    offsetX_ = offsetX;
    offsetY_ = offsetY;
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
} // namespace OHOS::Rosen
