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

#include "screen.h"

#include <cstdint>
#include <new>
#include <refbase.h>

#include "class_var_definition.h"
#include "display_manager_adapter.h"
#include "screen_info.h"
#include "singleton_container.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
class Screen::Impl : public RefBase {
public:
    explicit Impl(sptr<ScreenInfo> info)
    {
        screenInfo_ = info;
    }
    ~Impl() = default;
    DEFINE_VAR_FUNC_GET_SET_WITH_LOCK(sptr<ScreenInfo>, ScreenInfo, screenInfo);
};

Screen::Screen(sptr<ScreenInfo> info)
    : pImpl_(new Impl(info))
{
}

Screen::~Screen()
{
}

bool Screen::IsGroup() const
{
    UpdateScreenInfo();
    return pImpl_->GetScreenInfo()->GetIsScreenGroup();
}

std::string Screen::GetName() const
{
    return pImpl_->GetScreenInfo()->GetName();
}

ScreenId Screen::GetId() const
{
    return pImpl_->GetScreenInfo()->GetScreenId();
}

uint32_t Screen::GetWidth() const
{
    auto modeId = GetModeId();
    auto modes = GetSupportedModes();
    if (modeId < 0 || modeId >= modes.size()) {
        return 0;
    }
    return modes[modeId]->width_;
}

uint32_t Screen::GetHeight() const
{
    UpdateScreenInfo();
    auto modeId = GetModeId();
    auto modes = GetSupportedModes();
    if (modeId < 0 || modeId >= modes.size()) {
        return 0;
    }
    return modes[modeId]->height_;
}

uint32_t Screen::GetVirtualWidth() const
{
    UpdateScreenInfo();
    return pImpl_->GetScreenInfo()->GetVirtualWidth();
}

uint32_t Screen::GetVirtualHeight() const
{
    UpdateScreenInfo();
    return pImpl_->GetScreenInfo()->GetVirtualHeight();
}

float Screen::GetVirtualPixelRatio() const
{
    UpdateScreenInfo();
    return pImpl_->GetScreenInfo()->GetVirtualPixelRatio();
}

Rotation Screen::GetRotation() const
{
    UpdateScreenInfo();
    return pImpl_->GetScreenInfo()->GetRotation();
}

Orientation Screen::GetOrientation() const
{
    UpdateScreenInfo();
    return pImpl_->GetScreenInfo()->GetOrientation();
}

bool Screen::IsReal() const
{
    return pImpl_->GetScreenInfo()->GetType() == ScreenType::REAL;
}

DMError Screen::SetOrientation(Orientation orientation) const
{
    TLOGD(WmsLogTag::DMS, "Orientation %{public}u", orientation);
    return SingletonContainer::Get<ScreenManagerAdapter>().SetOrientation(GetId(), orientation, false);
}

DMError Screen::SetScreenOrientation(Orientation orientation) const
{
    TLOGD(WmsLogTag::DMS, "Orientation %{public}u", orientation);
    return SingletonContainer::Get<ScreenManagerAdapter>().SetOrientation(GetId(), orientation, true);
}

DMError Screen::SetScreenOrientation(Orientation orientation, const OrientationOptions& options) const
{
    TLOGI(WmsLogTag::DMS, "Orientation %{public}u, needAnimation %{public}d, ignoreRotationLock %{public}d",
        orientation, options.needAnimation, options.ignoreRotationLock);
    return SingletonContainer::Get<ScreenManagerAdapter>().SetOrientation(GetId(), orientation, options, true);
}

DMError Screen::GetScreenSupportedColorGamuts(std::vector<ScreenColorGamut>& colorGamuts) const
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetScreenSupportedColorGamuts(GetId(), colorGamuts);
}

DMError Screen::GetScreenColorGamut(ScreenColorGamut& colorGamut) const
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetScreenColorGamut(GetId(), colorGamut);
}

DMError Screen::SetScreenColorGamut(int32_t colorGamutIdx)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenColorGamut(GetId(), colorGamutIdx);
}

DMError Screen::GetScreenGamutMap(ScreenGamutMap& gamutMap) const
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetScreenGamutMap(GetId(), gamutMap);
}

DMError Screen::SetScreenGamutMap(ScreenGamutMap gamutMap)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenGamutMap(GetId(), gamutMap);
}

DMError Screen::SetScreenColorTransform()
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenColorTransform(GetId());
}

DMError Screen::GetPixelFormat(GraphicPixelFormat& pixelFormat) const
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetPixelFormat(GetId(), pixelFormat);
}

DMError Screen::SetPixelFormat(GraphicPixelFormat pixelFormat)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetPixelFormat(GetId(), pixelFormat);
}

DMError Screen::GetSupportedHDRFormats(std::vector<ScreenHDRFormat>& hdrFormats) const
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetSupportedHDRFormats(GetId(), hdrFormats);
}

DMError Screen::GetScreenHDRFormat(ScreenHDRFormat& hdrFormat) const
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetScreenHDRFormat(GetId(), hdrFormat);
}

DMError Screen::SetScreenHDRFormat(int32_t modeIdx)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenHDRFormat(GetId(), modeIdx);
}

DMError Screen::GetSupportedColorSpaces(std::vector<GraphicCM_ColorSpaceType>& colorSpaces) const
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetSupportedColorSpaces(GetId(), colorSpaces);
}

DMError Screen::GetScreenColorSpace(GraphicCM_ColorSpaceType& colorSpace) const
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetScreenColorSpace(GetId(), colorSpace);
}

DMError Screen::SetScreenColorSpace(GraphicCM_ColorSpaceType colorSpace)
{
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenColorSpace(GetId(), colorSpace);
}

ScreenId Screen::GetParentId() const
{
    UpdateScreenInfo();
    return pImpl_->GetScreenInfo()->GetParentId();
}

uint32_t Screen::GetModeId() const
{
    UpdateScreenInfo();
    return pImpl_->GetScreenInfo()->GetModeId();
}

std::string Screen::GetSerialNumber() const
{
    UpdateScreenInfo();
    return pImpl_->GetScreenInfo()->GetSerialNumber();
}

std::vector<sptr<SupportedScreenModes>> Screen::GetSupportedModes() const
{
    return pImpl_->GetScreenInfo()->GetModes();
}

DMError Screen::SetScreenActiveMode(uint32_t modeId)
{
    ScreenId screenId = GetId();
    if (modeId >= GetSupportedModes().size()) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return SingletonContainer::Get<ScreenManagerAdapter>().SetScreenActiveMode(screenId, modeId);
}

void Screen::UpdateScreenInfo(sptr<ScreenInfo> info) const
{
    if (info == nullptr) {
        TLOGE(WmsLogTag::DMS, "ScreenInfo is invalid");
        return;
    }
    pImpl_->SetScreenInfo(info);
}

void Screen::UpdateScreenInfo() const
{
    auto screenInfo = SingletonContainer::Get<ScreenManagerAdapter>().GetScreenInfo(GetId());
    UpdateScreenInfo(screenInfo);
}

DMError Screen::SetDensityDpi(uint32_t dpi) const
{
    if (dpi > DOT_PER_INCH_MAXIMUM_VALUE || dpi < DOT_PER_INCH_MINIMUM_VALUE) {
        TLOGE(WmsLogTag::DMS, "Invalid input dpi value, valid input range for DPI is %{public}u ~ %{public}u",
            DOT_PER_INCH_MINIMUM_VALUE, DOT_PER_INCH_MAXIMUM_VALUE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    // Calculate display density, Density = Dpi / 160.
    float density = static_cast<float>(dpi) / 160; // 160 is the coefficient between density and dpi.
    return SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualPixelRatio(GetId(), density);
}

DMError Screen::SetDensityDpiSystem(uint32_t dpi) const
{
    if (dpi > DOT_PER_INCH_MAXIMUM_VALUE || dpi < DOT_PER_INCH_MINIMUM_VALUE) {
        TLOGE(WmsLogTag::DMS, "Invalid input dpi value, valid input range for DPI is %{public}u ~ %{public}u",
            DOT_PER_INCH_MINIMUM_VALUE, DOT_PER_INCH_MAXIMUM_VALUE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    float density = static_cast<float>(dpi) / 160; // 160 is the coefficient between density and dpi.
    return SingletonContainer::Get<ScreenManagerAdapter>().SetVirtualPixelRatioSystem(GetId(), density);
}

DMError Screen::SetDefaultDensityDpi(uint32_t dpi) const
{
    if (dpi > DOT_PER_INCH_MAXIMUM_VALUE || dpi < DOT_PER_INCH_MINIMUM_VALUE) {
        TLOGE(WmsLogTag::DMS, "Invalid input dpi value, valid input range for DPI is %{public}u ~ %{public}u",
            DOT_PER_INCH_MINIMUM_VALUE, DOT_PER_INCH_MAXIMUM_VALUE);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    float density = static_cast<float>(dpi) / BASELINE_DENSITY; // 160 is the coefficient between density and dpi.
    return SingletonContainer::Get<ScreenManagerAdapter>().SetDefaultDensityDpi(GetId(), density);
}

DMError Screen::SetResolution(uint32_t width, uint32_t height, uint32_t dpi) const
{
    if (width <= 0 || height <= 0 || dpi > DOT_PER_INCH_MAXIMUM_VALUE || dpi < DOT_PER_INCH_MINIMUM_VALUE) {
        TLOGE(WmsLogTag::DMS, "Invalid param, w:%{public}u h:%{public}u dpi:%{public}u", width, height, dpi);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    // Calculate display density, Density = Dpi / 160.
    float density = static_cast<float>(dpi) / 160; // 160 is the coefficient between density and dpi.
    return SingletonContainer::Get<ScreenManagerAdapter>().SetResolution(GetId(), width, height, density);
}

DMError Screen::GetDensityInCurResolution(float& virtualPixelRatio) const
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetDensityInCurResolution(GetId(), virtualPixelRatio);
}

sptr<ScreenInfo> Screen::GetScreenInfo() const
{
    UpdateScreenInfo();
    return pImpl_->GetScreenInfo();
}

DMError Screen::GetScreenCapability(ScreenCapability& capability) const
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetScreenCapability(GetId(), capability);
}
} // namespace OHOS::Rosen