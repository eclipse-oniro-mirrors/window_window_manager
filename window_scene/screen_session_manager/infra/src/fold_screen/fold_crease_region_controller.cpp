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
namespace {
const ScreenId SCREEN_ID_FULL = 0;
const ScreenId SCREEN_ID_MAIN = 5;
constexpr int32_t FOLD_CREASE_RECT_SIZE = 4; //numbers of parameter on the current device is 4
const std::string g_FoldScreenRect = system::GetParameter("const.display.foldscreen.crease_region", "");
const std::string FOLD_CREASE_DELIMITER = ",;";

const int32_t RECT_POS_X_INDEX = 0;
const int32_t RECT_POS_Y_INDEX = 1;
const int32_t RECT_POS_WIDTH_INDEX = 2;
const int32_t RECT_POS_HEIGHT_INDEX = 3;
} // namespace

WM_IMPLEMENT_SINGLE_INSTANCE(FoldCreaseRegionController)

FoldCreaseRegionController::FoldCreaseRegionController()
{
    TLOGI(WmsLogTag::DMS, "FoldCreaseRegionController created");
    currentFoldCreaseRegion_ = sptr<FoldCreaseRegion>::MakeSptr(SCREEN_ID_FULL, GetFoldCreaseRegionRect(true));
}

FoldCreaseRegion FoldCreaseRegionController::GetFoldCreaseRegion(bool isVertical) const
{
    std::vector<DMRect> foldCreaseRect = GetFoldCreaseRegionRect(isVertical);
    return FoldCreaseRegion(SCREEN_ID_FULL, foldCreaseRect);
}

std::vector<DMRect> FoldCreaseRegionController::GetFoldCreaseRegionRect(bool isVertical) const
{
    std::vector<DMRect> foldCreaseRect = {};
    std::vector<int32_t> foldRect = FoldScreenStateInternel::StringFoldRectSplitToInt(g_FoldScreenRect,
        FOLD_CREASE_DELIMITER);
    if (foldRect.size() != FOLD_CREASE_RECT_SIZE) {
        TLOGE(WmsLogTag::DMS, "foldRect is invalid");
        return foldCreaseRect;
    }
    GetFoldCreaseRect(isVertical, foldRect, foldCreaseRect);
    return foldCreaseRect;
}

void FoldCreaseRegionController::GetFoldCreaseRect(bool isVertical,
    const std::vector<int32_t>& foldRect, std::vector<DMRect>& foldCreaseRect) const
{
    int32_t liveCreaseRegionPosX;
    int32_t liveCreaseRegionPosY;
    uint32_t liveCreaseRegionPosWidth;
    uint32_t liveCreaseRegionPosHeight;
    if (isVertical) {
        TLOGI(WmsLogTag::DMS, "the current FoldCreaseRect is vertical");
        liveCreaseRegionPosX = foldRect[RECT_POS_X_INDEX];
        liveCreaseRegionPosY = foldRect[RECT_POS_Y_INDEX];
        liveCreaseRegionPosWidth = static_cast<uint32_t>(foldRect[RECT_POS_WIDTH_INDEX]);
        liveCreaseRegionPosHeight = static_cast<uint32_t>(foldRect[RECT_POS_HEIGHT_INDEX]);
    } else {
        TLOGI(WmsLogTag::DMS, "the current FoldCreaseRect is horizontal");
        liveCreaseRegionPosX = foldRect[RECT_POS_Y_INDEX];
        liveCreaseRegionPosY = foldRect[RECT_POS_X_INDEX];
        liveCreaseRegionPosWidth = static_cast<uint32_t>(foldRect[RECT_POS_HEIGHT_INDEX]);
        liveCreaseRegionPosHeight = static_cast<uint32_t>(foldRect[RECT_POS_WIDTH_INDEX]);
    }
    foldCreaseRect = {
        {
            liveCreaseRegionPosX, liveCreaseRegionPosY,
            liveCreaseRegionPosWidth, liveCreaseRegionPosHeight
        }
    };
    return;
}

sptr<FoldCreaseRegion> FoldCreaseRegionController::GetCurrentFoldCreaseRegion()
{
    TLOGI(WmsLogTag::DMS, "GetCurrentFoldCreaseRegion");
    return currentFoldCreaseRegion_;
}

FoldCreaseRegion FoldCreaseRegionController::GetLiveCreaseRegion()
{
    TLOGI(WmsLogTag::DMS, "enter");
    std::lock_guard<std::mutex> lock_mode(liveCreaseRegionMutex_);
    FoldDisplayMode displayMode = ScreenSessionManager::GetInstance().GetFoldDisplayMode();
    if (displayMode == FoldDisplayMode::UNKNOWN || displayMode == FoldDisplayMode::MAIN) {
        return FoldCreaseRegion(0, {});
    }
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_FULL);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "default screenSession is null");
        return FoldCreaseRegion(0, {});
    }
    DisplayOrientation displayOrientation = screenSession->GetScreenProperty().GetDisplayOrientation();
    if (displayMode == FoldDisplayMode::FULL) {
        switch (displayOrientation) {
            case DisplayOrientation::PORTRAIT:
            case DisplayOrientation::PORTRAIT_INVERTED: {
                liveCreaseRegion_ = GetFoldCreaseRegion(true);
                break;
            }
            case DisplayOrientation::LANDSCAPE:
            case DisplayOrientation::LANDSCAPE_INVERTED: {
                liveCreaseRegion_ = GetFoldCreaseRegion(false);
                break;
            }
            default: {
                TLOGE(WmsLogTag::DMS, "displayOrientation is invalid");
                return FoldCreaseRegion(0, {});
            }
        }
    }
    return liveCreaseRegion_;
}

void FoldCreaseRegionController::GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems) const
{
    FoldCreaseRegionItem MCreaseItem{DisplayOrientation::LANDSCAPE, FoldDisplayMode::MAIN,
        FoldCreaseRegion(0, {})};
    FoldCreaseRegionItem FPorCreaseItem{DisplayOrientation::PORTRAIT, FoldDisplayMode::FULL,
        GetFoldCreaseRegion(true)};
    FoldCreaseRegionItem FLandCreaseItem{DisplayOrientation::LANDSCAPE, FoldDisplayMode::FULL,
        GetFoldCreaseRegion(false)};
    foldCreaseRegionItems.push_back(MCreaseItem);
    foldCreaseRegionItems.push_back(FPorCreaseItem);
    foldCreaseRegionItems.push_back(FLandCreaseItem);
}
} // namespace OHOS::Rosen