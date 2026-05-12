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

#include "float_view_option.h"

namespace OHOS {
namespace Rosen {
FvOption::FvOption()
{
}

void FvOption::SetTemplate(uint32_t type)
{
    template_ = type;
}

void FvOption::SetUIPath(const std::string& uiPath)
{
    uiPath_ = uiPath;
}

void FvOption::SetStorage(const std::shared_ptr<NativeReference>& storageRef)
{
    storage_ = storageRef;
}

void FvOption::SetStorage(const ani_object& storageRef)
{
    aniStorage_ = storageRef;
}

void FvOption::SetRect(const Rect& rect)
{
    rect_ = rect;
}

void FvOption::SetVisibilityInApp(bool visible)
{
    visibleInApp_ = visible;
}

void FvOption::SetContext(void* contextPtr)
{
    contextPtr_ = contextPtr;
}

void FvOption::SetShowWhenCreate(bool showWhenCreate)
{
    showWhenCreate_ = showWhenCreate;
}

// LCOV_EXCL_START
uint32_t FvOption::GetTemplate() const
{
    return template_;
}

const std::string& FvOption::GetUIPath() const
{
    return uiPath_;
}

std::shared_ptr<NativeReference> FvOption::GetStorage() const
{
    return storage_;
}

ani_object FvOption::GetAniStorage() const
{
    return aniStorage_;
}

Rect FvOption::GetRect() const
{
    return rect_;
}

bool FvOption::GetVisibilityInApp() const
{
    return visibleInApp_;
}

void* FvOption::GetContext() const
{
    return contextPtr_;
}

bool FvOption::GetShowWhenCreate() const
{
    return showWhenCreate_;
}

bool FvOption::IsRectValid() const
{
    if (rect_.width_ == 0 || rect_.height_ == 0) {
        return false;
    }
    return true;
}

bool FvOption::IsUIPathValid() const
{
    return !uiPath_.empty();
}

void FvOption::GetFvTemplateInfo(FloatViewTemplateInfo& fvTemplateInfo) const
{
    fvTemplateInfo.template_ = template_;
    fvTemplateInfo.visibleInApp_ = visibleInApp_;
    fvTemplateInfo.rect_ = rect_;
    fvTemplateInfo.showWhenCreate_ = showWhenCreate_;
}

// LCOV_EXCL_STOP

} // namespace Rosen
} // namespace OHOS