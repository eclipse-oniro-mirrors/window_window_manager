/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "fold_crease_region_controller.h"
#include <parameters.h>
#include "session/screen/include/screen_session.h"
#include "screen_session_manager.h"
#include "fold_screen_state_internel.h"
#include "rs_adapter.h"
#include "sensor_agent.h"
#include "sensor_agent_type.h"

#include "window_manager_hilog.h"

namespace OHOS::Rosen::DMS {

WM_IMPLEMENT_SINGLE_INSTANCE(FoldCreaseRegionController)

FoldCreaseRegionController::FoldCreaseRegionController()
{
    InitModeCreaseRegion();
}

sptr<FoldCreaseRegion> FoldCreaseRegionController::GetCurrentFoldCreaseRegion()
{
    TLOGI(WmsLogTag::DMS, "GetCurrentFoldCreaseRegion");
    if (!isInitModeCreaseRegion_.load()) {
        InitModeCreaseRegion();
    }
    if (currentFoldCreaseRegion_ == nullptr) {
        FoldDisplayMode displayMode = FoldDisplayMode::FULL;
        sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_FULL);
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is null");
            return currentFoldCreaseRegion_;
        }
        Rotation targetRotation = Rotation::ROTATION_90;
        screenSession->AddRotationCorrection(targetRotation, displayMode);
        std::vector<DMRect> rects = GetCreaseRegionRects(SCREEN_ID_FULL, displayMode, targetRotation);
        currentFoldCreaseRegion_ = new FoldCreaseRegion(SCREEN_ID_FULL, rects);
    }
    return currentFoldCreaseRegion_;
}

FoldCreaseRegion FoldCreaseRegionController::GetLiveCreaseRegion()
{
    TLOGI(WmsLogTag::DMS, "enter");
    FoldDisplayMode displayMode = ScreenSessionManager::GetInstance().GetFoldDisplayMode();
    if (displayMode == FoldDisplayMode::UNKNOWN || displayMode == FoldDisplayMode::MAIN) {
        return FoldCreaseRegion(0, {});
    }
    if (!isInitModeCreaseRegion_.load()) {
        InitModeCreaseRegion();
    }
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_FULL);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return FoldCreaseRegion(0, {});
    }
    Rotation deviceRotation = screenSession->GetScreenProperty().GetDeviceRotation();
    std::vector<DMRect> curRects = GetCreaseRegionRects(SCREEN_ID_FULL, displayMode, deviceRotation);
    return FoldCreaseRegion(0, curRects);
}

std::vector<DMRect> FoldCreaseRegionController::GetCreaseRegionRects(ScreenId screenId,
    FoldDisplayMode displayMode, Rotation targetRotation)
{
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return {};
    }
    targetRotation = ScreenSessionManager::GetInstance().RemoveRotationCorrection(targetRotation);
    RRect bounds = screenSession->CalcBoundsInRotationZero(displayMode);
    std::shared_lock<std::shared_mutex> lock(creaseRegionMutex_);
    return RotateRectArray(displayModeRects_[displayMode], bounds.rect_.width_, bounds.rect_.height_, targetRotation);
}
 
void FoldCreaseRegionController::GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems)
{
    if (!isInitModeCreaseRegion_.load()) {
        InitModeCreaseRegion();
    }
    GetAllCreaseRegionByDisplayMode(FoldDisplayMode::MAIN, SCREEN_ID_FULL, foldCreaseRegionItems);
    GetAllCreaseRegionByDisplayMode(FoldDisplayMode::FULL, SCREEN_ID_FULL, foldCreaseRegionItems);
}
 
void FoldCreaseRegionController::GetAllCreaseRegionByDisplayMode(FoldDisplayMode displayMode, ScreenId screenId,
    std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems)
{
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    GetCreaseRegionByOrientation(screenSession, displayMode, DisplayOrientation::PORTRAIT, foldCreaseRegionItems);
    GetCreaseRegionByOrientation(screenSession, displayMode, DisplayOrientation::LANDSCAPE, foldCreaseRegionItems);
    GetCreaseRegionByOrientation(screenSession, displayMode,
        DisplayOrientation::PORTRAIT_INVERTED, foldCreaseRegionItems);
    GetCreaseRegionByOrientation(screenSession, displayMode,
        DisplayOrientation::LANDSCAPE_INVERTED, foldCreaseRegionItems);
}
 
void FoldCreaseRegionController::GetCreaseRegionByOrientation(const sptr<ScreenSession>& screenSession,
    FoldDisplayMode displayMode, DisplayOrientation orientation,
    std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    RRect boundsInRotationZero = screenSession->CalcBoundsInRotationZero(displayMode);
    Rotation rotation = screenSession->CalcRotationByDeviceOrientation(
        orientation, displayMode, boundsInRotationZero);
    std::vector<DMRect> curRects = GetCreaseRegionRects(screenSession->GetScreenId(), displayMode, rotation);
    FoldCreaseRegionItem creaseItem{orientation, displayMode,
        FoldCreaseRegion(screenSession->GetDisplayId(), curRects)};
    foldCreaseRegionItems.push_back(creaseItem);
}

void FoldCreaseRegionController::InitModeCreaseRegion()
{
    std::unique_lock<std::shared_mutex> lock(creaseRegionMutex_);
    auto curFoldScreenRect = system::GetParameter("const.display.foldscreen.crease_region", "");
    std::vector<int32_t> foldRect = FoldScreenStateInternel::StringFoldRectSplitToInt(curFoldScreenRect,
        FOLD_CREASE_DELIMITER);
    TLOGI(WmsLogTag::DMS, "FoldScreenRect:%{public}s, ", curFoldScreenRect.c_str());
    if (foldRect.size() != FOLD_CREASE_RECT_SIZE) {
        TLOGE(WmsLogTag::DMS, "foldRect is invalid");
        return;
    }
    std::vector<DMRect> allRect = ConvertToRectList(foldRect);
    GetDisplayModeRectMap(allRect);
    isInitModeCreaseRegion_.store(true);
}
 
void FoldCreaseRegionController::GetDisplayModeRectMap(const std::vector<DMRect>& allRect)
{
    if (allRect.empty()) {
        TLOGE(WmsLogTag::DMS, "allRect is empty");
        return;
    }
    std::vector<DMRect> fullRects;
    DMRect fullModeRect = allRect[0];
    fullRects.emplace_back(fullModeRect);
    displayModeRects_[FoldDisplayMode::FULL] = fullRects;
    displayModeRects_[FoldDisplayMode::COORDINATION] = fullRects;
    std::vector<DMRect> mainRects;
    displayModeRects_[FoldDisplayMode::MAIN] = mainRects;
}
 
std::vector<DMRect> FoldCreaseRegionController::ConvertToRectList(const std::vector<int32_t>& input)
{
    std::vector<DMRect> result;
    if (input.size() % FOLD_CREASE_RECT_SIZE != 0) {
        TLOGE(WmsLogTag::DMS, "error rect size");
        return result;
    }
    for (size_t i = 0; i < input.size(); i += FOLD_CREASE_RECT_SIZE) {
        DMRect rect;
        rect.posX_ = input[i + RECT_POS_X_INDEX];
        rect.posY_ = input[i + RECT_POS_Y_INDEX];
        if (input[i + RECT_POS_WIDTH_INDEX] < 0 || input[i + RECT_POS_HEIGHT_INDEX] < 0) {
            TLOGE(WmsLogTag::DMS, "error format num");
            return result;
        }
        rect.width_ = static_cast<uint32_t>(input[i + RECT_POS_WIDTH_INDEX]);
        rect.height_ = static_cast<uint32_t>(input[i + RECT_POS_HEIGHT_INDEX]);
        result.emplace_back(rect);
    }
 
    return result;
}

/**
 * @brief Rotates a single rectangle (based on the display orientation, the coordinate system
 * is reset according to the orientation)
 * @param originalRect The original rectangle (in PORTRAIT orientation, i.e., the parameters at 0 degree)
 * @param portraitWidth The width of the parent container when the original screen
 * is in portrait mode (the width of the parent container at 0 degree)
 * @param portraitHeight The height of the parent container when the original screen
 * is in portrait mode (the height of the parent container at 0 degree)
 * @param targetOrientation The target display orientation (the direction to rotate to)
 * @return The rectangle in the target orientation (based on the 0,0 coordinate system of the top-left
 * corner of the target orientation)
 */
DMRect FoldCreaseRegionController::RotateSingleRect(const DMRect& originalRect,
    uint32_t portraitWidth, uint32_t portraitHeight, Rotation targetRotation)
{
    if (portraitWidth == 0 || portraitHeight == 0) {
        TLOGW(WmsLogTag::DMS, "[RotateRect Error] Portrait width or height cannot be zero.");
        return {};
    }
    if (originalRect.width_ == 0 || originalRect.height_ == 0) {
        TLOGW(WmsLogTag::DMS, "[RotateRect Error] Original rect width or height cannot be zero.");
        return {};
    }
 
    const int32_t originalPosX = originalRect.posX_;
    const int32_t originalPosY = originalRect.posY_;
    const uint32_t originalWidth = originalRect.width_;
    const uint32_t originalHeight = originalRect.height_;
    const uint32_t portraitParentWidth = portraitWidth;
    const uint32_t portraitParentHeight = portraitHeight;
 
    int32_t newPosX = 0;
    int32_t newPosY = 0;
    uint32_t newWidth = 0;
    uint32_t newHeight = 0;
 
    switch (targetRotation) {
        case Rotation::ROTATION_0:
            newPosX = originalPosX;
            newPosY = originalPosY;
            newWidth = originalWidth;
            newHeight = originalHeight;
            break;
 
        case Rotation::ROTATION_90:
            newPosX = static_cast<int32_t>(portraitParentHeight - originalPosY - originalHeight);
            newPosY = originalPosX;
            newWidth = originalHeight;
            newHeight = originalWidth;
            break;
 
        case Rotation::ROTATION_180:
            newPosX = static_cast<int32_t>(portraitParentWidth - originalPosX - originalWidth);
            newPosY = static_cast<int32_t>(portraitParentHeight - originalPosY - originalHeight);
            newWidth = originalWidth;
            newHeight = originalHeight;
            break;
 
        case Rotation::ROTATION_270:
            newPosX = originalPosY;
            newPosY = static_cast<int32_t>(portraitParentWidth - originalPosX - originalWidth);
            newWidth = originalHeight;
            newHeight = originalWidth;
            break;
 
        default:
            TLOGW(WmsLogTag::DMS, "[RotateRect Error] Invalid target orientation.");
            return {};
    }
    return {newPosX, newPosY, newWidth, newHeight};
}
 
/**
 * @brief Batch rotation of rectangular arrays (based on display orientation)
 * @param originalRects Array of original rectangles (in Portrait orientation)
 * @param portraitWidth Width of the original portrait-shaped parent container
 * @param portraitHeight Height of the original portrait-shaped parent container
 * @param targetOrientation Target display orientation
 * @return Array of rectangles in the target orientation:
 * Rectangles that fail will be empty rectangles (with width_ = 0 and height_ = 0)
 */
std::vector<DMRect> FoldCreaseRegionController::RotateRectArray(const std::vector<DMRect>& originalRects,
    uint32_t portraitWidth, uint32_t portraitHeight, Rotation targetRotation)
{
    std::vector<DMRect> rotatedRects;
    rotatedRects.reserve(originalRects.size());
 
    for (const auto& currentRect : originalRects) {
        rotatedRects.emplace_back(RotateSingleRect(currentRect, portraitWidth, portraitHeight, targetRotation));
    }
 
    return rotatedRects;
}
} // namespace OHOS::Rosen