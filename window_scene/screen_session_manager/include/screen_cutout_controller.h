/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ROSEN_WINDOW_SCENE_SCREEN_CUTOUT_CONTROLLER_H
#define OHOS_ROSEN_WINDOW_SCENE_SCREEN_CUTOUT_CONTROLLER_H

#include <refbase.h>

#include "cutout_info.h"
#include "session/screen/include/screen_property.h"

namespace OHOS::Rosen {

class ScreenCutoutController : public RefBase {
public:
    ScreenCutoutController() = default;
    sptr<CutoutInfo> GetScreenCutoutInfo(DisplayId displayId) const;
    sptr<CutoutInfo> GetScreenCutoutInfo(DisplayId displayId, uint32_t width,
                                         uint32_t height, Rotation rotation) const;
    void GetCutoutArea(DisplayId displayId, uint32_t width, uint32_t height, Rotation rotation,
                       std::vector<DMRect>& cutoutArea) const;
    void GetWaterfallArea(uint32_t width, uint32_t height, Rotation rotation,
                          WaterfallDisplayAreaRects& waterfallArea) const;

private:
    void CalcWaterfallRects(const std::vector<int>& numberVec, uint32_t displayWidth, uint32_t displayHeight,
                            Rotation rotation, WaterfallDisplayAreaRects& waterfallArea) const;
    void CalcCutoutRects(const std::vector<DMRect>& boundaryRects, uint32_t width, uint32_t height,
                         Rotation rotation, std::vector<DMRect>& cutoutRects) const;
    void InitRect(uint32_t left, uint32_t top, uint32_t width, uint32_t height, DMRect& rect) const;
    void CheckBoundaryRectsWithRotation(std::vector<DMRect>& boundaryRects, uint32_t screenWidth, uint32_t screenHeight,
        Rotation rotation) const;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SCREEN_CUTOUT_CONTROLLER_H
