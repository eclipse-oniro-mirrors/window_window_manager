/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_DMSERVER_FOLD_SCREEN_INFO_H
#define FOUNDATION_DMSERVER_FOLD_SCREEN_INFO_H

#include <parcel.h>

#include "class_var_definition.h"
#include "cutout_info.h"
#include "dm_common.h"

namespace OHOS::Rosen {
/**
 * @brief Fold Crease Region
 */
class FoldCreaseRegion : public Parcelable {
public:
    FoldCreaseRegion() = default;

    FoldCreaseRegion(DisplayId displayId, const std::vector<DMRect>& creaseRects)
        : displayId_(displayId), creaseRects_(creaseRects) {}

    ~FoldCreaseRegion() override = default;

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteUint64(displayId_) &&
        WriteCreaseRectVector(creaseRects_, parcel);
    }

    static FoldCreaseRegion *Unmarshalling(Parcel& parcel)
    {
        DisplayId displayId = DISPLAY_ID_INVALID;
        std::vector<DMRect> creaseRects;
        parcel.ReadUint64(displayId);
        ReadCreaseRectVector(creaseRects, parcel);
        auto* region = new FoldCreaseRegion(displayId, creaseRects);
        return region;
    }

DEFINE_VAR_DEFAULT_FUNC_GET_SET(DisplayId, DisplayId, displayId, DISPLAY_ID_INVALID)

DEFINE_VAR_FUNC_GET(std::vector<DMRect>, CreaseRects, creaseRects)

    void SetCreaseRects(std::vector<DMRect> value)
    { creaseRects_ = std::move(value); }
private:
    const static uint32_t MAX_CREASE_REGION_SIZE = 20;

    static bool WriteCreaseRectVector(const std::vector<DMRect>& creaseRects, Parcel& parcel)
    {
        auto size = static_cast<uint32_t>(creaseRects.size());
        if (!parcel.WriteUint32(size)) {
            return false;
        }
        if (size > MAX_CREASE_REGION_SIZE) {
            return false;
        }
        for (auto rect : creaseRects) {
            if (!(parcel.WriteInt32(rect.posX_) && parcel.WriteInt32(rect.posY_) &&
                parcel.WriteUint32(rect.width_) && parcel.WriteUint32(rect.height_))) {
                return false;
            }
        }
        return true;
    }

    static bool ReadCreaseRectVector(std::vector<DMRect>& creaseRects, Parcel& parcel)
    {
        uint32_t size;
        if (!parcel.ReadUint32(size)) {
            return false;
        }
        if (size > MAX_CREASE_REGION_SIZE) {
            return false;
        }
        for (uint32_t index = 0; index < size; index++) {
            int32_t posX;
            int32_t posY;
            uint32_t width;
            uint32_t height;
            if (!(parcel.ReadInt32(posX) && parcel.ReadInt32(posY) &&
                parcel.ReadUint32(width) && parcel.ReadUint32(height))) {
                return false;
            }
            DMRect rect = {posX, posY, width, height};
            creaseRects.push_back(rect);
        }
        return true;
    }
};

struct FoldCreaseRegionItem {
    DisplayOrientation orientation_;
    FoldDisplayMode foldDisplayMode_;
    FoldCreaseRegion region_;
    FoldCreaseRegionItem(DisplayOrientation orientation, FoldDisplayMode foldDisplayMode, FoldCreaseRegion region)
        : orientation_(orientation), foldDisplayMode_(foldDisplayMode), region_(region) {}
};
} // namespace OHOS::Rosen
#endif // FOUNDATION_DMSERVER_FOLD_SCREEN_INFO_H