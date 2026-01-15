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

#ifndef FOUNDATION_DM_SCREEN_H
#define FOUNDATION_DM_SCREEN_H

#include <cstdint>
#include <screen_manager/screen_types.h>
#include <string>
#include <surface.h>
#include <vector>

#include "dm_common.h"
#include "noncopyable.h"

namespace OHOS::Rosen {
class ScreenInfo;

struct VirtualScreenOption {
    std::string name_;
    uint32_t width_;
    uint32_t height_;
    float density_;
    sptr<Surface> surface_;
    int32_t flags_;
    bool isForShot_ {true};
    std::vector<uint64_t> missionIds_ {};
    VirtualScreenType virtualScreenType_ {VirtualScreenType::UNKNOWN};
    bool isSecurity_ {true};
    VirtualScreenFlag virtualScreenFlag_ {VirtualScreenFlag::DEFAULT};
    bool supportsFocus_ {true};
    bool supportsInput_ {true};
    std::string bundleName_;
    std::string serialNumber_;
};

class Screen : public RefBase {
friend class ScreenManager;
public:
    ~Screen();
    Screen(const Screen&) = delete;
    Screen(Screen&&) = delete;
    Screen& operator=(const Screen&) = delete;
    Screen& operator=(Screen&&) = delete;
    bool IsGroup() const;
    std::string GetName() const;

    /**
     * @brief Get screen id.
     *
     * @return Screen id.
     */
    ScreenId GetId() const;

    /**
     * @brief Get width of the screen.
     *
     * @return Width of the screen.
     */
    uint32_t GetWidth() const;

    /**
     * @brief Get height of the screen.
     *
     * @return Height of the screen.
     */
    uint32_t GetHeight() const;

    /**
     * @brief Get virtual width of the screen.
     *
     * @return Virtual width of the screen.
     */
    uint32_t GetVirtualWidth() const;

    /**
     * @brief Get virtual height of the screen.
     *
     * @return Virtual height of the screen.
     */
    uint32_t GetVirtualHeight() const;

    /**
     * @brief Get virtual pixel ratio of the screen.
     *
     * @return Virtual pixel ratio of the screen.
     */
    float GetVirtualPixelRatio() const;

    /**
     * @brief Get the Rotation of the screen.
     *
     * @return The Rotation of the screen.
     */
    Rotation GetRotation() const;

    /**
     * @brief Get the orientation of the screen.
     *
     * @return Orientation of the screen.
     */
    Orientation GetOrientation() const;

    /**
     * @brief Is a real screen.
     *
     * @return True means screen is real, false means the opposite.
     */
    bool IsReal() const;

    /**
     * @brief Get screen parent id.
     *
     * @return Screen parent id.
     */
    ScreenId GetParentId() const;

    /**
     * @brief Get screen serial number.
     *
     * @return Screen serial number.
     */
    std::string GetSerialNumber() const;

    /**
     * @brief Get screen mode id.
     *
     * @return Screen mode id.
     */
    uint32_t GetModeId() const;

    /**
     * @brief Get supported modes of the screen.
     *
     * @return Supported modes of the screen.
     */
    std::vector<sptr<SupportedScreenModes>> GetSupportedModes() const;

    /**
     * @brief Set screen active mode.
     *
     * @param modeId Mode id.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetScreenActiveMode(uint32_t modeId);

    /**
     * @brief Set orientation for the screen.
     * This interface is no longer recommended for use; please use SetScreenOrientation instead.
     *
     * @param orientation Orientation for the screen.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetOrientation(Orientation orientation) const;

    /**
     * @brief Set orientation for the screen.
     *
     * @param orientation Orientation for the screen.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetScreenOrientation(Orientation orientation) const;

    /**
     * @brief Set the density dpi of the screen.
     *
     * @param dpi Density dpi of the screen.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetDensityDpi(uint32_t dpi) const;

    /**
     * @brief Set the density dpi of the screen system window.
     *
     * @param dpi Density dpi of the screen.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetDensityDpiSystem(uint32_t dpi) const;

    /**
     * @brief Set the default density dpi of the screen system window.
     *
     * @param dpi Default density dpi of the screen.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetDefaultDensityDpi(uint32_t dpi) const;

    /**
     * @brief Get the screen info.
     *
     * @return Screen info.
     */
    sptr<ScreenInfo> GetScreenInfo() const;

    // colorspace, gamut
    /**
     * @brief Get the supported color gamuts of the screen.
     *
     * @param colorGamuts Supported color gamuts of the screen.
     * @return DM_OK means get success, others means get failed.
     */
    DMError GetScreenSupportedColorGamuts(std::vector<ScreenColorGamut>& colorGamuts) const;

    /**
     * @brief Get the color gamut of the screen.
     *
     * @param colorGamut Color gamut of the screen.
     * @return DM_OK means get success, others means get failed.
     */
    DMError GetScreenColorGamut(ScreenColorGamut& colorGamut) const;

    /**
     * @brief Set the color gamut of the screen.
     *
     * @param colorGamutIdx Color gamut of the screen.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetScreenColorGamut(int32_t colorGamutIdx);

    /**
     * @brief Get the gamut map of the screen.
     *
     * @param gamutMap Gamut map of the screen.
     * @return DM_OK means get success, others means get failed.
     */
    DMError GetScreenGamutMap(ScreenGamutMap& gamutMap) const;

    /**
     * @brief Set the gamut map of the screen.
     *
     * @param gamutMap Gamut map of the screen.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetScreenGamutMap(ScreenGamutMap gamutMap);

    /**
     * @brief Set color transform for the screen.
     *
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetScreenColorTransform();

    /**
     * @brief Set the resolution for the screen.
     *
     * @param width width of the screen
     * @param height height of the screen
     * @param dpi dpi of the screen
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetResolution(uint32_t width, uint32_t height, uint32_t dpi) const;

    /**
     * @brief get Density in current resolution
     * @param virtualPixelRatio virtualPixelRatio of the screen
     * @return DM_OK means set success, others means set failed.
     */
    DMError GetDensityInCurResolution(float& virtualPixelRatio) const;

    /**
     * @brief Get the pixel format of the screen.
     *
     * @return DM_OK means set success, others means set failed.
     */
    DMError GetPixelFormat(GraphicPixelFormat& pixelFormat) const;

    /**
     * @brief Set the pixel format of the screen.
     *
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetPixelFormat(GraphicPixelFormat pixelFormat);

    /**
     * @brief Get the supported HDR format of the screen.
     *
     * @param colorSpaces Supported HDR format of the screen.
     * @return DM_OK means get success, others means get failed.
     */
    DMError GetSupportedHDRFormats(std::vector<ScreenHDRFormat>& hdrFormats) const;

    /**
     * @brief Get the HDR format of the screen.
     *
     * @return DM_OK means set success, others means set failed.
     */
    DMError GetScreenHDRFormat(ScreenHDRFormat& hdrFormat) const;

    /**
     * @brief Set the HDR format of the screen.
     *
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetScreenHDRFormat(int32_t modeIdx);

    /**
     * @brief Get the supported color space of the screen.
     *
     * @param colorSpaces Supported color space of the screen.
     * @return DM_OK means get success, others means get failed.
     */
    DMError GetSupportedColorSpaces(std::vector<GraphicCM_ColorSpaceType>& colorSpaces) const;

    /**
     * @brief Get the color space of the screen.
     *
     * @param colorSpace Color space of the screen.
     * @return DM_OK means get success, others means get failed.
     */
    DMError GetScreenColorSpace(GraphicCM_ColorSpaceType& colorSpace) const;

    /**
     * @brief Set the color space of the screen.
     *
     * @param colorSpace Color space of the screen.
     * @return DM_OK means set success, others means set failed.
     */
    DMError SetScreenColorSpace(GraphicCM_ColorSpaceType colorSpace);

protected:
    // No more methods or variables can be defined here.
    explicit Screen(sptr<ScreenInfo> info);
    void UpdateScreenInfo() const;
    void UpdateScreenInfo(sptr<ScreenInfo> info) const;
private:
    // No more methods or variables can be defined here.
    class Impl;
    sptr<Impl> pImpl_;
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_DM_SCREEN_H