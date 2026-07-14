/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "abstract_display.h"

#include "abstract_screen_controller.h"
#include "display_manager_config.h"
#include "display_manager_service.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr int32_t PAD_SCREEN_WIDTH = 2560;
constexpr int32_t PHONE_SCREEN_WIDTH = 2160;
constexpr float INCH_2_MM = 25.4f;
constexpr ScreenId DEFAULT_DISPLAY_ID = 0;
}

AbstractDisplay::AbstractDisplay(DisplayId id, sptr<SupportedScreenModes>& info, sptr<AbstractScreen>& absScreen)
    : id_(id)
{
    if (info == nullptr || absScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "info or absScreen is nullptr");
        return;
    }
    screenId_ = absScreen->dmsId_;
    screenGroupId_ = absScreen->groupDmsId_;
    width_ = static_cast<int32_t>(info->width_);
    height_ = static_cast<int32_t>(info->height_);
    refreshRate_ = info->refreshRate_;
    orientation_ = absScreen->orientation_;
    name_ = absScreen->GetScreenName();
    static_cast<void>(RequestRotation(absScreen->rotation_));
    if (width_ > height_) {
        displayOrientation_ = DisplayOrientation::LANDSCAPE;
    } else {
        displayOrientation_ = DisplayOrientation::PORTRAIT;
    }
    if (info->width_ < info->height_) {
        isDefaultVertical_ = true;
    } else {
        isDefaultVertical_ = false;
    }

    CalculateXYDpi(absScreen->GetPhyWidth(), absScreen->GetPhyHeight());
    auto numbersConfig = DisplayManagerConfig::GetIntNumbersConfig();
    if (numbersConfig.count("dpi") != 0) {
        uint32_t densityDpi = static_cast<uint32_t>(numbersConfig["dpi"][0]);
        if (densityDpi >= DOT_PER_INCH_MINIMUM_VALUE && densityDpi <= DOT_PER_INCH_MAXIMUM_VALUE) {
            virtualPixelRatio_ = static_cast<float>(densityDpi) / BASELINE_DENSITY;
            absScreen->SetVirtualPixelRatio(virtualPixelRatio_);
            return;
        }
    }
    if ((info->width_ >= PHONE_SCREEN_WIDTH) || (info->height_ >= PHONE_SCREEN_WIDTH)) {
        if ((info->width_ == PAD_SCREEN_WIDTH) || (info->height_ == PAD_SCREEN_WIDTH)) {
            virtualPixelRatio_ = 2.0f; // Pad is 2.0
        } else {
            virtualPixelRatio_ = 3.0f; // Phone is 3.0
        }
    } else {
        virtualPixelRatio_ = 1.0f; // Other is 1.0
    }
    absScreen->SetVirtualPixelRatio(virtualPixelRatio_);
}

void AbstractDisplay::CalculateXYDpi(uint32_t phyWidth, uint32_t phyHeight)
{
    if (phyWidth == 0 || phyHeight == 0) {
        return;
    }

    phyWidth_ = phyWidth;
    phyHeight_ = phyHeight;
    xDpi_ = width_ * INCH_2_MM / phyWidth_;
    yDpi_ = height_ * INCH_2_MM / phyHeight_;
}

DisplayId AbstractDisplay::GetId() const
{
    return id_;
}

int32_t AbstractDisplay::GetWidth() const
{
    return width_;
}

int32_t AbstractDisplay::GetHeight() const
{
    return height_;
}

uint32_t AbstractDisplay::GetRefreshRate() const
{
    return refreshRate_;
}

float AbstractDisplay::GetVirtualPixelRatio() const
{
    return virtualPixelRatio_;
}

int32_t AbstractDisplay::GetOffsetX() const
{
    return offsetX_;
}

int32_t AbstractDisplay::GetOffsetY() const
{
    return offsetY_;
}

void AbstractDisplay::SetOffsetX(int32_t offsetX)
{
    offsetX_ = offsetX;
}

void AbstractDisplay::SetOffsetY(int32_t offsetY)
{
    offsetY_ = offsetY;
}

void AbstractDisplay::SetWidth(int32_t width)
{
    width_ = width;
    UpdateXDpi();
}

void AbstractDisplay::SetHeight(int32_t height)
{
    height_ = height;
    UpdateYDpi();
}

void AbstractDisplay::UpdateXDpi()
{
    if (phyWidth_ != UINT32_MAX) {
        xDpi_ = width_ * INCH_2_MM / phyWidth_;
    }
}

void AbstractDisplay::UpdateYDpi()
{
    if (phyHeight_ != UINT32_MAX) {
        yDpi_ = height_ * INCH_2_MM / phyHeight_;
    }
}

void AbstractDisplay::SetOffset(int32_t offsetX, int32_t offsetY)
{
    offsetX_ = offsetX;
    offsetY_ = offsetY;
}

void AbstractDisplay::SetRefreshRate(uint32_t refreshRate)
{
    refreshRate_ = refreshRate;
}

void AbstractDisplay::SetVirtualPixelRatio(float virtualPixelRatio)
{
    virtualPixelRatio_ = virtualPixelRatio;
}

void AbstractDisplay::SetId(DisplayId id)
{
    id_ = id;
}

void AbstractDisplay::SetOrientation(Orientation orientation)
{
    orientation_ = orientation;
}

void AbstractDisplay::SetDisplayOrientation(DisplayOrientation displayOrientation)
{
    displayOrientation_ = displayOrientation;
}

bool AbstractDisplay::RequestRotation(Rotation rotation)
{
    TLOGD(WmsLogTag::DMS, "request rotation from %{public}u to %{public}u, display %{public}" PRIu64"",
        rotation_, rotation, id_);
    if (rotation_ == rotation) {
        TLOGE(WmsLogTag::DMS, "rotation not change %{public}u", rotation);
        return false;
    }
    if (IsVertical(rotation) != IsVertical(rotation_)) {
        std::swap(width_, height_);
    }
    rotation_ = rotation;
    return true;
}

Rotation AbstractDisplay::GetRotation() const
{
    return rotation_;
}

Orientation AbstractDisplay::GetOrientation() const
{
    return orientation_;
}

DisplayOrientation AbstractDisplay::GetDisplayOrientation() const
{
    return displayOrientation_;
}

void AbstractDisplay::SetFreezeFlag(FreezeFlag freezeFlag)
{
    freezeFlag_ = freezeFlag;
}

FreezeFlag AbstractDisplay::GetFreezeFlag() const
{
    return freezeFlag_;
}

bool AbstractDisplay::BindAbstractScreen(sptr<AbstractScreen> abstractScreen)
{
    if (abstractScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "display bind screen error, cannot get screen. display:%{public}" PRIu64"", id_);
        return false;
    }
    ScreenId dmsScreenId = abstractScreen->dmsId_;
    sptr<SupportedScreenModes> info = abstractScreen->GetActiveScreenMode();
    if (info == nullptr) {
        TLOGE(WmsLogTag::DMS, "display bind screen error, cannot get info. display:%{public}" PRIu64", "
            "screen:%{public}" PRIu64"", id_, dmsScreenId);
        return false;
    }

    Point point = abstractScreen->GetGroup()->GetChildPosition(dmsScreenId);
    offsetX_ = point.posX_;
    offsetY_ = point.posY_;
    width_ = static_cast<int32_t>(info->width_);
    height_ = static_cast<int32_t>(info->height_);
    refreshRate_ = info->refreshRate_;
    screenId_ = dmsScreenId;
    TLOGD(WmsLogTag::DMS, "display bind to screen. display:%{public}" PRIu64", screen:%{public}" PRIu64"",
        id_, dmsScreenId);
    return true;
}

ScreenId AbstractDisplay::GetAbstractScreenId() const
{
    return screenId_;
}

ScreenId AbstractDisplay::GetAbstractScreenGroupId() const
{
    return screenGroupId_;
}

sptr<DisplayInfo> AbstractDisplay::ConvertToDisplayInfo() const
{
    sptr<DisplayInfo> displayInfo = new(std::nothrow) DisplayInfo();
    if (displayInfo == nullptr) {
        return displayInfo;
    }

    displayInfo->name_ = name_;
    displayInfo->SetDisplayId(id_);
    displayInfo->SetWidth(width_);
    displayInfo->SetHeight(height_);
    displayInfo->SetRefreshRate(refreshRate_);
    displayInfo->SetScreenId(screenId_);
    displayInfo->SetScreenGroupId(screenGroupId_);
    displayInfo->SetVirtualPixelRatio(virtualPixelRatio_);
    displayInfo->SetXDpi(xDpi_);
    displayInfo->SetYDpi(yDpi_);
    displayInfo->SetDpi(virtualPixelRatio_ * DOT_PER_INCH);
    displayInfo->SetRotation(rotation_);
    displayInfo->SetOrientation(orientation_);
    displayInfo->SetOffsetX(offsetX_);
    displayInfo->SetOffsetY(offsetY_);
    displayInfo->displayState_ = displayState_;
    displayInfo->SetWaterfallDisplayCompressionStatus(waterfallDisplayCompressionStatus_);
    displayInfo->SetDisplayOrientation(displayOrientation_);
    displayInfo->SetIsDefaultVertical(isDefaultVertical_);
    if (id_ == DEFAULT_DISPLAY_ID) {
        displayInfo->SetDisplaySourceMode(DisplaySourceMode::MAIN);
    } else {
        displayInfo->SetDisplaySourceMode(DisplaySourceMode::NONE);
    }
    return displayInfo;
}
} // namespace OHOS::Rosen