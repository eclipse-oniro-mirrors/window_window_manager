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

#ifndef FOLD_CREASE_REGION_CONTROLLER_H
#define FOLD_CREASE_REGION_CONTROLLER_H

#include <refbase.h>
#include <shared_mutex>
#include "dm_common.h"
#include "fold_screen_info.h"
#include "wm_single_instance.h"
#include "session/screen/include/screen_session.h"

namespace OHOS::Rosen::DMS {
class FoldCreaseRegionController : public RefBase {
const ScreenId SCREEN_ID_FULL = 0;
const ScreenId SCREEN_ID_MAIN = 5;
const int32_t FOLD_CREASE_RECT_SIZE = 4;
const std::string FOLD_CREASE_DELIMITER = ",;";

const int32_t RECT_POS_X_INDEX = 0;
const int32_t RECT_POS_Y_INDEX = 1;
const int32_t RECT_POS_WIDTH_INDEX = 2;
const int32_t RECT_POS_HEIGHT_INDEX = 3;
WM_DECLARE_SINGLE_INSTANCE_BASE(FoldCreaseRegionController)
public:
    FoldCreaseRegionController();
    ~FoldCreaseRegionController() = default;
    sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion();
    FoldCreaseRegion GetLiveCreaseRegion();
    void GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems);
private:
    DMRect RotateSingleRect(const DMRect& originalRect,
        uint32_t portraitWidth, uint32_t portraitHeight, Rotation targetRotation);
    std::vector<DMRect> RotateRectArray(const std::vector<DMRect>& originalRects,
        uint32_t portraitWidth, uint32_t portraitHeight, Rotation targetRotation);
    void GetDisplayModeRectMap(const std::vector<DMRect>& allRect);
    std::vector<DMRect> ConvertToRectList(const std::vector<int32_t>& input);
    void InitModeCreaseRegion();
    std::vector<DMRect> GetCreaseRegionRects(ScreenId screenId, FoldDisplayMode displayMode, Rotation deviceRotation);
    void GetAllCreaseRegionByDisplayMode(FoldDisplayMode displayMode, ScreenId screenId,
        std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems);
    void GetCreaseRegionByOrientation(const sptr<ScreenSession>& screenSession, FoldDisplayMode displayMode,
        DisplayOrientation orientation, std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems);
    std::map<FoldDisplayMode, std::vector<DMRect>> displayModeRects_;
    std::atomic<bool> isInitModeCreaseRegion_ = false;
    std::shared_mutex creaseRegionMutex_;
    sptr<FoldCreaseRegion> currentFoldCreaseRegion_ = nullptr;
};
} // namespace OHOS::Rosen
#endif //FOLD_CREASE_REGION_CONTROLLER_H