/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_FLOAT_VIEW_TEMPLATE_INFO_H
#define OHOS_FLOAT_VIEW_TEMPLATE_INFO_H

#include "wm_common.h"

namespace OHOS::Rosen {
class FloatViewTemplateInfo : public Parcelable {
public:
    FloatViewTemplateInfo() = default;
    ~FloatViewTemplateInfo() override = default;

    uint32_t template_ {};
    bool visibleInApp_ {true};
    bool isBind_ {false};
    uint32_t bindWindowId_ {INVALID_WINDOW_ID};
    Rect rect_ {};
    bool showWhenCreate_ {true};
    std::string id_ {};

    // LCOV_EXCL_START
    bool Marshalling(Parcel& parcel) const override
    {
        if (!parcel.WriteUint32(template_) || !parcel.WriteBool(visibleInApp_) ||
            !parcel.WriteUint32(bindWindowId_) || !parcel.WriteBool(showWhenCreate_) ||
            !parcel.WriteBool(isBind_) || !parcel.WriteString(id_)) {
            return false;
        }
        if (!parcel.WriteInt32(rect_.posX_) || !parcel.WriteInt32(rect_.posY_) ||
            !parcel.WriteUint32(rect_.width_) || !parcel.WriteUint32(rect_.height_)) {
            return false;
        }
        return true;
    }
    // LCOV_EXCL_STOP

    static FloatViewTemplateInfo* Unmarshalling(Parcel& parcel)
    {
        std::unique_ptr<FloatViewTemplateInfo> fvTemplateInfo = std::make_unique<FloatViewTemplateInfo>();
        if (!parcel.ReadUint32(fvTemplateInfo->template_) || !parcel.ReadBool(fvTemplateInfo->visibleInApp_) ||
            !parcel.ReadUint32(fvTemplateInfo->bindWindowId_) || !parcel.ReadBool(fvTemplateInfo->showWhenCreate_) ||
            !parcel.ReadBool(fvTemplateInfo->isBind_) || !parcel.ReadString(fvTemplateInfo->id_)) {
            return nullptr;
        }
        if (!parcel.ReadInt32(fvTemplateInfo->rect_.posX_) || !parcel.ReadInt32(fvTemplateInfo->rect_.posY_) ||
            !parcel.ReadUint32(fvTemplateInfo->rect_.width_) || !parcel.ReadUint32(fvTemplateInfo->rect_.height_)) {
            return nullptr;
        }
        return fvTemplateInfo.release();
    }
};
} // namespace OHOS::Rosen
#endif // OHOS_FLOAT_VIEW_TEMPLATE_INFO_H