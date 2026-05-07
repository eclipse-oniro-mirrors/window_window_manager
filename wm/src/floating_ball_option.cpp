/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "floating_ball_option.h"

#include "window_manager_hilog.h"
#include "color_parser.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t TITLE_MIN_LEN = 1;
constexpr uint32_t TITLE_MAX_LEN = 64;
constexpr uint32_t CONTENT_MAX_LEN = 64;
constexpr int32_t PIXEL_MAP_MAX_SIZE = 192 * 1024;
}
FbOption::FbOption()
{
}

void FbOption::SetTemplate(const uint32_t& type)
{
    template_ = type;
}

void FbOption::SetTitle(const std::string& title)
{
    title_ = title;
}

void FbOption::SetContent(const std::string& content)
{
    content_ = content;
}

void FbOption::SetBackgroundColor(const std::string& color)
{
    backgroundColor_ = color;
}

void FbOption::SetTitleColor(const std::string& color)
{
    titleColor_ = color;
}

void FbOption::SetContentColor(const std::string& color)
{
    contentColor_ = color;
}

void FbOption::SetIcon(const std::shared_ptr<Media::PixelMap>& icon)
{
    icon_ = icon;
}

void FbOption::SetTextUpdateAnimationType(const uint32_t& type)
{
    textUpdateAnimationType_ = type;
}

void FbOption::SetShowWhenCreate(bool showWhenCreate)
{
    showWhenCreate_ = showWhenCreate;
}

uint32_t FbOption::GetTemplate() const
{
    return template_;
}

std::string FbOption::GetTitle() const
{
    return title_;
}

std::string FbOption::GetContent() const
{
    return content_;
}

std::string FbOption::GetBackgroundColor() const
{
    return backgroundColor_;
}

std::string FbOption::GetTitleColor() const
{
    return titleColor_;
}

std::string FbOption::GetContentColor() const
{
    return contentColor_;
}

std::shared_ptr<Media::PixelMap> FbOption::GetIcon() const
{
    return icon_;
}

uint32_t FbOption::GetTextUpdateAnimationType() const
{
    return textUpdateAnimationType_;
}

bool FbOption::GetShowWhenCreate() const
{
    return showWhenCreate_;
}

void FbOption::GetFbTemplateBaseInfo(FloatingBallTemplateBaseInfo& fbTemplateBaseInfo)
{
    fbTemplateBaseInfo.template_ = template_;
    fbTemplateBaseInfo.title_ = title_;
    fbTemplateBaseInfo.content_ = content_;
    fbTemplateBaseInfo.backgroundColor_ = backgroundColor_;
    fbTemplateBaseInfo.titleColor_ = titleColor_;
    fbTemplateBaseInfo.contentColor_ = contentColor_;
    fbTemplateBaseInfo.isVisibleInApp_ = visibleInApp_;
    fbTemplateBaseInfo.textUpdateAnimationType_ = textUpdateAnimationType_;
    fbTemplateBaseInfo.showWhenCreate_ = showWhenCreate_;
}

bool FbOption::IsValid(std::string& errMsg) const
{
    if (template_ < static_cast<uint32_t>(FloatingBallTemplate::STATIC) ||
        template_ >= static_cast<uint32_t>(FloatingBallTemplate::END)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Template %{public}d is invalid", template_);
        errMsg = "Template is invalid";
        return false;
    }
    if (title_.length() < TITLE_MIN_LEN || title_.length() > TITLE_MAX_LEN) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Title length Exceed the limit %{public}zu", title_.length());
        errMsg = "Title length Exceed the limit";
        return false;
    }
    if (content_.length() > CONTENT_MAX_LEN) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Content length Exceed the limit %{public}zu", content_.length());
        errMsg = "Content length Exceed the limit";
        return false;
    }
    if (icon_ != nullptr && icon_->GetByteCount() > PIXEL_MAP_MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Icon size Exceed the limit %{public}d", icon_->GetByteCount());
        errMsg = "Icon size Exceed the limit";
        return false;
    }
    if (template_ == static_cast<uint32_t>(FloatingBallTemplate::STATIC) && icon_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Template %{public}u need icon", template_);
        errMsg = "Template need icon";
        return false;
    }
    if (textUpdateAnimationType_ >= static_cast<uint32_t>(FloatingBallTextUpdateAnimationType::ANIMATION_END)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]animationType %{public}u is invalid", textUpdateAnimationType_);
        errMsg = "TextUpdateAnimationType is invalid";
        return false;
    }
    
    return VerifyColor(errMsg);
}
 
bool FbOption::VerifyColor(std::string& errMsg) const
{
    if (!backgroundColor_.empty() && !ColorParser::IsValidColorNoAlpha(backgroundColor_)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "BackgroundColor is invalid");
        errMsg = "BackgroundColor is invalid";
        return false;
    }
 
    bool hasTextColor = !titleColor_.empty() || !contentColor_.empty();
    if (hasTextColor && backgroundColor_.empty()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "When setting the text color, the background color must be set");
        errMsg = "When setting the text color, the background color must be set";
        return false;
    }
 
    if (!titleColor_.empty() && !ColorParser::IsValidColorNoAlpha(titleColor_)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "TitleColor is invalid");
        errMsg = "TitleColor is invalid";
        return false;
    }
    if (!contentColor_.empty() && !ColorParser::IsValidColorNoAlpha(contentColor_)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "ContentColor is invalid");
        errMsg = "ContentColor is invalid";
        return false;
    }
 
    return true;
}

} // namespace Rosen
} // namespace OHOS