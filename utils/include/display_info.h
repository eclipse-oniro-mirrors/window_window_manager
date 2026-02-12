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

#ifndef FOUNDATION_DMSERVER_DISPLAY_INFO_H
#define FOUNDATION_DMSERVER_DISPLAY_INFO_H

#include <cstdint>
#include <parcel.h>

#include "class_var_definition.h"
#include "display.h"
#include "dm_common.h"
#include "noncopyable.h"

namespace OHOS::Rosen {
class DisplayInfo : public Parcelable {
friend class AbstractDisplay;
friend class ScreenSession;
public:
    DisplayInfo() = default;
    ~DisplayInfo() = default;
    WM_DISALLOW_COPY_AND_MOVE(DisplayInfo);

    bool Marshalling(Parcel& parcel) const override;
    static DisplayInfo *Unmarshalling(Parcel& parcel);

    DEFINE_VAR_DEFAULT_FUNC_GET_SET(std::string, Name, name, "");
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(DisplayId, DisplayId, id, DISPLAY_ID_INVALID);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(DisplayType, DisplayType, type, DisplayType::DEFAULT);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(int32_t, Width, width, 0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(int32_t, Height, height, 0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(int32_t, PhysicalWidth, physicalWidth, 0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(int32_t, PhysicalHeight, physicalHeight, 0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(uint32_t, RefreshRate, refreshRate, 0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(ScreenId, ScreenId, screenId, SCREEN_ID_INVALID);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(ScreenId, ScreenGroupId, screenGroupId, SCREEN_ID_INVALID);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(float, VirtualPixelRatio, virtualPixelRatio, 1.0f);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(float, DensityInCurResolution, densityInCurResolution, 1.0f);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(float, DefaultVirtualPixelRatio, defaultVirtualPixelRatio, 1.0f);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(float, XDpi, xDpi, 0.0f);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(float, YDpi, yDpi, 0.0f);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(int32_t, Dpi, dpi, 320);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(Rotation, Rotation, rotation, Rotation::ROTATION_0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(Orientation, Orientation, orientation, Orientation::UNSPECIFIED);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(int32_t, OffsetX, offsetX, 0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(int32_t, OffsetY, offsetY, 0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(DisplayState, DisplayState, displayState, DisplayState::UNKNOWN);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(bool, WaterfallDisplayCompressionStatus, waterfallDisplayCompressionStatus, false);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(DisplayOrientation, DisplayOrientation, displayOrientation,
        DisplayOrientation::UNKNOWN);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(DisplayStateChangeType, DisplayStateChangeType, displayStateChangeType,
        DisplayStateChangeType::UNKNOWN);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(bool, IsDefaultVertical, isDefaultVertical, true);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(bool, AliveStatus, alive, true);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(std::vector<uint32_t>, ColorSpaces, colorSpaces, std::vector<uint32_t>());
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(std::vector<uint32_t>, HdrFormats, hdrFormats, std::vector<uint32_t>());
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(uint32_t, DefaultDeviceRotationOffset, defaultDeviceRotationOffset, 0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(uint32_t, AvailableWidth, availableWidth, 0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(uint32_t, AvailableHeight, availableHeight, 0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(float, ScaleX, scaleX, 1.0f);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(float, ScaleY, scaleY, 1.0f);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(float, PivotX, pivotX, 0.5f);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(float, PivotY, pivotY, 0.5f);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(float, TranslateX, translateX, 0.0f);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(float, TranslateY, translateY, 0.0f);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(ScreenShape, ScreenShape, screenShape, ScreenShape::RECTANGLE);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(Rotation, OriginRotation, originRotation, Rotation::ROTATION_0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(int32_t, X, x, 0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(int32_t, Y, y, 0);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(DisplaySourceMode, DisplaySourceMode, displaySourceMode, DisplaySourceMode::NONE);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(std::vector<uint32_t>, SupportedRefreshRate, supportedRefreshRate,
        std::vector<uint32_t>());
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(bool, SupportsFocus, supportsFocus, true);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(bool, SupportsInput, supportsInput, true);
    DEFINE_VAR_DEFAULT_FUNC_GET_SET(std::string, BundleName, bundleName, "");
};
} // namespace OHOS::Rosen
#endif // FOUNDATION_DMSERVER_DISPLAY_INFO_H