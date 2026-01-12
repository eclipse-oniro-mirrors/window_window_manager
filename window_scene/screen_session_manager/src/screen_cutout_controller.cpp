/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "screen_cutout_controller.h"

#include "screen_scene_config.h"
#include "screen_session_manager.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr std::vector<int>::size_type LEFT = 0;
constexpr std::vector<int>::size_type TOP = 1;
constexpr std::vector<int>::size_type RIGHT = 2;
constexpr std::vector<int>::size_type BOTTOM = 3;
constexpr uint8_t HALF_SCREEN = 2;
}

sptr<CutoutInfo> ScreenCutoutController::GetScreenCutoutInfo(DisplayId displayId) const
{
    sptr<DisplayInfo> displayInfo = ScreenSessionManager::GetInstance().GetDisplayInfoById(displayId);
    if (!displayInfo) {
        TLOGE(WmsLogTag::DMS, "displayInfo invalid");
        return nullptr;
    }
    uint32_t width = static_cast<uint32_t>(displayInfo->GetWidth());
    uint32_t height = static_cast<uint32_t>(displayInfo->GetHeight());
    Rotation rotation = displayInfo->GetOriginRotation();

    return GetScreenCutoutInfo(displayId, width, height, rotation);
}

sptr<CutoutInfo> ScreenCutoutController::GetScreenCutoutInfo(DisplayId displayId, uint32_t width,
                                                             uint32_t height, Rotation rotation) const
{
    std::vector<DMRect> boundaryRects;
    GetCutoutArea(displayId, width, height, rotation, boundaryRects);

    WaterfallDisplayAreaRects waterfallArea = {};
    GetWaterfallArea(width, height, rotation, waterfallArea);

    return sptr<CutoutInfo>::MakeSptr(boundaryRects, waterfallArea);
}

void ScreenCutoutController::GetCutoutArea(DisplayId displayId, uint32_t width, uint32_t height,
                                           Rotation rotation, std::vector<DMRect>& cutoutArea) const
{
    FoldDisplayMode displayMode = ScreenSceneConfig::GetFoldDisplayMode(width, height);

    TLOGI_LIMITN_HOUR(WmsLogTag::DMS, THREE_TIMES,
        "display:[ID: %{public}" PRIu64 ", W: %{public}u, H: %{public}u, R: %{public}u,"
        "Mode %{public}u]", displayId, width, height, rotation, displayMode);

    std::vector<DMRect> boundaryRects;
    if (ScreenSessionManager::GetInstance().IsFoldable() &&
        (displayMode == FoldDisplayMode::MAIN || displayMode == FoldDisplayMode::GLOBAL_FULL)) {
        boundaryRects = ScreenSceneConfig::GetSubCutoutBoundaryRect();
    } else {
        boundaryRects = ScreenSceneConfig::GetCutoutBoundaryRect(displayId);
    }
    CheckBoundaryRectsWithRotation(boundaryRects, width, height, rotation);
    if (!boundaryRects.empty()) {
        CalcCutoutRects(boundaryRects, width, height, rotation, cutoutArea);
    }
}

void ScreenCutoutController::CalcCutoutRects(const std::vector<DMRect>& boundaryRects, uint32_t width,
    uint32_t height, Rotation rotation, std::vector<DMRect>& cutoutRects) const
{
    for (const DMRect& rect : boundaryRects) {
        switch (rotation) {
            case Rotation::ROTATION_0: {
                cutoutRects.emplace_back(rect);
                break;
            }
            case Rotation::ROTATION_90: {
                cutoutRects.emplace_back(
                    DMRect{ width - rect.posY_ - rect.height_, rect.posX_, rect.height_, rect.width_ });
                break;
            }
            case Rotation::ROTATION_180: {
                cutoutRects.emplace_back(DMRect{
                    width - rect.posX_ - rect.width_, height - rect.posY_ - rect.height_, rect.width_, rect.height_ });
                break;
            }
            case Rotation::ROTATION_270: {
                cutoutRects.emplace_back(
                    DMRect{ rect.posY_, height - rect.posX_ - rect.width_, rect.height_, rect.width_ });
                break;
            }
            default:
                break;
        }
    }
}

/**
 * @brief Check if the cutout area exceeds the screen boundaries
 *
 * @param boundaryRects List of cutout area rectangles in the initial physical screen orientation (rotation = 0).
 *                      Items in the list will be cleared if the cutout area exceeds the screen boundaries.
 * @param screenWidth Logical width of the current screen (in rotated coordinate system)
 * @param screenHeight Logical height of the current screen (in rotated coordinate system)
 * @param rotation Current screen rotation state
 */
void ScreenCutoutController::CheckBoundaryRectsWithRotation(std::vector<DMRect>& boundaryRects, uint32_t screenWidth,
    uint32_t screenHeight, Rotation rotation) const
{
    if (rotation == Rotation::ROTATION_90 || rotation == Rotation::ROTATION_270) {
        std::swap(screenWidth, screenHeight);
    }
    for (auto iter = boundaryRects.begin(); iter != boundaryRects.end();) {
        DMRect boundaryRect = *iter;
        if ((boundaryRect.posX_ < 0) || (boundaryRect.posY_ < 0) ||
            (static_cast<int32_t>(boundaryRect.width_) + boundaryRect.posX_ > static_cast<int32_t>(screenWidth)) ||
            (static_cast<int32_t>(boundaryRect.height_) + boundaryRect.posY_ > static_cast<int32_t>(screenHeight)) ||
            (boundaryRect.width_ > screenWidth) || (boundaryRect.height_ > screenHeight) ||
            boundaryRect.IsUninitializedRect()) {
                TLOGE(WmsLogTag::DMS, "boundary is invalid");
                iter = boundaryRects.erase(iter);
        } else {
            iter++;
        }
    }
}

void ScreenCutoutController::GetWaterfallArea(uint32_t width, uint32_t height, Rotation rotation,
                                              WaterfallDisplayAreaRects& waterfallArea) const
{
    TLOGI(WmsLogTag::DMS, "display:[W: %{public}u, H: %{public}u, R: %{public}u]", width, height, rotation);

    std::vector<int> numberVec = ScreenSceneConfig::GetCurvedScreenBoundaryConfig();
    if (ScreenSceneConfig::IsWaterfallDisplay() && !numberVec.empty()) {
        CalcWaterfallRects(numberVec, width, height, rotation, waterfallArea);
    }
}

void ScreenCutoutController::CalcWaterfallRects(const std::vector<int> &numberVec, uint32_t displayWidth,
    uint32_t displayHeight, Rotation rotation, WaterfallDisplayAreaRects& waterfallArea) const
{
    std::vector<uint32_t> realNumVec = { 0, 0, 0, 0 };
    for (auto i = LEFT; i <= BOTTOM; i++) {
        if (numberVec.size() > i) {
            realNumVec[i] = static_cast<uint32_t>(numberVec[i]);
        }
    }
    if (std::all_of(realNumVec.begin(), realNumVec.end(), [](uint32_t result) { return result == 0; })) {
        return;
    }

    if ((realNumVec[LEFT] > displayWidth / HALF_SCREEN) || (realNumVec[RIGHT] > displayWidth / HALF_SCREEN) ||
        (realNumVec[TOP] > displayHeight / HALF_SCREEN) || (realNumVec[BOTTOM] > displayHeight / HALF_SCREEN)) {
        TLOGE(WmsLogTag::DMS, "curved screen boundary data is not correct");
        return;
    }

    switch (rotation) {
        case Rotation::ROTATION_0: {
            InitRect(0, 0, realNumVec[LEFT], displayHeight, waterfallArea.left);
            InitRect(0, 0, displayWidth, realNumVec[TOP], waterfallArea.top);
            InitRect(displayWidth - realNumVec[RIGHT], 0, realNumVec[RIGHT], displayHeight, waterfallArea.right);
            InitRect(0, displayHeight - realNumVec[BOTTOM], displayWidth, realNumVec[BOTTOM], waterfallArea.bottom);
            return;
        }
        case Rotation::ROTATION_90: {
            InitRect(0, 0, realNumVec[BOTTOM], displayHeight, waterfallArea.left);
            InitRect(0, 0, displayWidth, realNumVec[LEFT], waterfallArea.top);
            InitRect(displayWidth - realNumVec[TOP], 0, realNumVec[TOP], displayHeight, waterfallArea.right);
            InitRect(0, displayHeight - realNumVec[RIGHT], displayWidth, realNumVec[RIGHT], waterfallArea.bottom);
            return;
        }
        case Rotation::ROTATION_180: {
            InitRect(0, 0, realNumVec[RIGHT], displayHeight, waterfallArea.left);
            InitRect(0, 0, displayWidth, realNumVec[BOTTOM], waterfallArea.top);
            InitRect(displayWidth - realNumVec[LEFT], 0, realNumVec[LEFT], displayHeight, waterfallArea.right);
            InitRect(0, displayHeight - realNumVec[TOP], displayWidth, realNumVec[TOP], waterfallArea.bottom);
            return;
        }
        case Rotation::ROTATION_270: {
            InitRect(0, 0, realNumVec[TOP], displayHeight, waterfallArea.left);
            InitRect(0, 0, displayWidth, realNumVec[RIGHT], waterfallArea.top);
            InitRect(displayWidth - realNumVec[BOTTOM], 0, realNumVec[BOTTOM], displayHeight, waterfallArea.right);
            InitRect(0, displayHeight - realNumVec[LEFT], displayWidth, realNumVec[LEFT], waterfallArea.bottom);
            return;
        }
        default: {
        }
    }
    return;
}

void ScreenCutoutController::InitRect(uint32_t left, uint32_t top, uint32_t width, uint32_t height, DMRect& rect) const
{
    if (width == 0 || height == 0) {
        rect = DMRect::NONE();
        return;
    }
    rect.posX_ = static_cast<int32_t>(left);
    rect.posY_ = static_cast<int32_t>(top);
    rect.width_ = width;
    rect.height_ = height;
}

} // namespace OHOS::Rosen