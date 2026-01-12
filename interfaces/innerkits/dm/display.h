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

#ifndef FOUNDATION_DM_DISPLAY_H
#define FOUNDATION_DM_DISPLAY_H

#include <string>
#include "dm_common.h"
#include "noncopyable.h"

namespace OHOS::Rosen {
class DisplayInfo;
class CutoutInfo;
class FoldCreaseRegion;

class Display : public RefBase {
friend class DisplayManager;
public:
    ~Display();
    Display(const Display&) = delete;
    Display(Display&&) = delete;
    Display& operator=(const Display&) = delete;
    Display& operator=(Display&&) = delete;

    /**
     * @brief Get id of the display.
     *
     * @return Display id.
     */
    DisplayId GetId() const;

    /**
     * @brief Get name of the display.
     *
     * @return Display name.
     */
    std::string GetName() const;

    /**
     * @brief Get width of the display.
     *
     * @return Width of the display.
     */
    int32_t GetWidth() const;

    /**
     * @brief Get height of the display.
     *
     * @return Height of the display.
     */
    int32_t GetHeight() const;

    /**
     * @brief Get physical width of the display.
     *
     * @return Physical width of the display.
     */
    int32_t GetPhysicalWidth() const;

    /**
     * @brief Get physical height of the display.
     *
     * @return Physical height of the display.
     */
    int32_t GetPhysicalHeight() const;

    /**
     * @brief Get the refresh rate of the display.
     *
     * @return Refresh rate of the display.
     */
    uint32_t GetRefreshRate() const;

    /**
     * @brief Get screen id.
     *
     * @return The screen id.
     */
    ScreenId GetScreenId() const;

    /**
     * @brief Get the virtual pixel ratio of the display.
     *
     * @return Virtual pixel ratio of the display.
     */
    float GetVirtualPixelRatio() const;

    /**
     * @brief Get the Dpi of the display.
     *
     * @return Dpi of the display.
     */
    int GetDpi() const;

    /**
     * @brief Get the rotation of the display.
     *
     * @return Rotation of the display..
     */
    Rotation GetRotation() const;

    /**
     * @brief Get the rotation of the display before API 14.
     *
     * @return Rotation of the display before API 14.
     */
    Rotation GetOriginRotation() const;

    /**
     * @brief Get the Orientation of the display.
     *
     * @return Orientation indicates the direction of the display content.
     */
    Orientation GetOrientation() const;

    /**
     * @brief Get info of the display.
     *
     * @return Info of the display.
     */
    sptr<DisplayInfo> GetDisplayInfo() const;

    /**
     * @brief Get info of the display with cache.
     *
     * @return Info of the display.
     */
    sptr<DisplayInfo> GetDisplayInfoWithCache() const;

    /**
     * @brief Get cutout info of the display.
     *
     * @return Cutout info of the display.
     */
    sptr<CutoutInfo> GetCutoutInfo() const;

    /**
     * @brief Get rounded corner of the display.
     * @param roundedCorner roundedCorner of the display.
     * @return DM_OK means get success, others means get failed.
     */
    DMError GetRoundedCorner(std::vector<RoundedCorner>& roundedCorner) const;

    /**
     * @brief Judge if current display has immersive window.
     * @param immersive The flag to represent if current display has immersive window.
     *
     * @return DM error codes.
     */
    DMError HasImmersiveWindow(bool& immersive);
    
    /**
     * @brief get available area of the display.(the screen area without dock and statusbar)
     * @param area available area of the screen.
     * @return DMError
     */
    DMError GetAvailableArea(DMRect& area) const;

    /**
     * @brief Get the supported HDR format of the screen.
     *
     * @param colorSpaces Supported HDR format of the screen.
     * @return DM_OK means get success, others means get failed.
     */
    DMError GetSupportedHDRFormats(std::vector<uint32_t>& hdrFormats) const;

    /**
     * @brief Get the supported color space of the screen.
     *
     * @param colorSpaces Supported color space of the screen.
     * @return DM_OK means get success, others means get failed.
     */
    DMError GetSupportedColorSpaces(std::vector<uint32_t>& colorSpaces) const;

    /**
     * @brief Get current display information, including foldstatus, displaymode, rotation, and orientation information.
     * @param capabilitInfo capabilitInfo of the screen.
     * @return DMError
     */
    DMError GetDisplayCapability(std::string& capabilitInfo) const;

    /**
     * @brief Get the fold crease region in the current display mode.
     *
     * @return { FoldCreaseRegion } fold crease region in the current display mode.
     */
    DMError GetLiveCreaseRegion(FoldCreaseRegion& region) const;

protected:
    // No more methods or variables can be defined here.
    Display(const std::string& name, sptr<DisplayInfo> info);
private:
    // No more methods or variables can be defined here.
    void UpdateDisplayInfo(sptr<DisplayInfo>) const;
    void UpdateDisplayInfo() const;
    class Impl;
    sptr<Impl> pImpl_;
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_DM_DISPLAY_H