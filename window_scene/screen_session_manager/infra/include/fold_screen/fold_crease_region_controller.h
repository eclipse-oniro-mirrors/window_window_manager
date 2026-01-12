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
#include "dm_common.h"
#include "fold_screen_info.h"
#include "wm_single_instance.h"

namespace OHOS::Rosen::DMS {
class FoldCreaseRegionController : public RefBase {
WM_DECLARE_SINGLE_INSTANCE_BASE(FoldCreaseRegionController)
public:
    FoldCreaseRegionController();
    ~FoldCreaseRegionController() = default;
    sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion();
    FoldCreaseRegion GetLiveCreaseRegion();
    void GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems) const;

private:
    FoldCreaseRegion GetFoldCreaseRegion(bool isVertical) const;
    void GetFoldCreaseRect(bool isVertical, const std::vector<int32_t>& foldRect,
        std::vector<DMRect>& foldCreaseRect) const;
    std::vector<DMRect> GetFoldCreaseRegionRect(bool isVertical) const;
    sptr<FoldCreaseRegion> currentFoldCreaseRegion_ = nullptr;
    FoldCreaseRegion liveCreaseRegion_ = FoldCreaseRegion(0, {});
    mutable std::mutex liveCreaseRegionMutex_;
};
} // namespace OHOS::Rosen
#endif //FOLD_CREASE_REGION_CONTROLLER_H