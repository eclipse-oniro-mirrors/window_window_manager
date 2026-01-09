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

#include "session_utils.h"

#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace SessionUtils {
bool IsAspectRatioValid(float aspectRatio, const WindowLimits& limits, const WindowDecoration& decoration)
{
    if (MathHelper::NearZero(aspectRatio)) {
        return true;
    }

    WindowLimits contentLimits = limits;
    contentLimits.Clip(decoration.Horizontal(), decoration.Vertical());
    if (!contentLimits.IsValid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Invalid contentLimits: %{public}s", contentLimits.ToString().c_str());
        return false;
    }

    float minRatio = contentLimits.maxHeight_ > 0 ?
        static_cast<float>(contentLimits.minWidth_) / static_cast<float>(contentLimits.maxHeight_) : 0.0f;
    float maxRatio = contentLimits.minHeight_ > 0 ?
        static_cast<float>(contentLimits.maxWidth_) / static_cast<float>(contentLimits.minHeight_) : FLT_MAX;

    if (MathHelper::LessNotEqual(aspectRatio, minRatio) || MathHelper::GreatNotEqual(aspectRatio, maxRatio)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Invalid aspectRatio: %{public}f, valid range: [%{public}f, %{public}f]",
            aspectRatio, minRatio, maxRatio);
        return false;
    }
    return true;
}

WindowLimits AdjustLimitsByAspectRatio(const WindowLimits& limits,
                                       const WindowDecoration& decoration,
                                       float aspectRatio)
{
    if (MathHelper::NearZero(aspectRatio)) {
        return limits;
    }

    const uint32_t decorW = decoration.Horizontal();
    const uint32_t decorH = decoration.Vertical();

    // Clip limits by decoration
    WindowLimits adjustedLimits = limits;
    adjustedLimits.Clip(decorW, decorH);
    if (!adjustedLimits.IsValid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Invalid adjustedLimits: %{public}s", adjustedLimits.ToString().c_str());
        return limits;
    }

    // Pre-calc aspect ratio constraints
    const uint32_t minWByH = static_cast<uint32_t>(std::ceil(adjustedLimits.minHeight_ * aspectRatio));
    const uint32_t minHByW = static_cast<uint32_t>(std::ceil(adjustedLimits.minWidth_ / aspectRatio));
    const uint32_t maxHByW = static_cast<uint32_t>(std::floor(adjustedLimits.maxWidth_ / aspectRatio));
    const uint32_t maxWByH = static_cast<uint32_t>(std::floor(adjustedLimits.maxHeight_ * aspectRatio));

    // Adjust constraints to satisfy aspect ratio.
    if (adjustedLimits.minWidth_ < minWByH) {
        adjustedLimits.minWidth_ = minWByH;
    } else {
        adjustedLimits.minHeight_ = minHByW;
    }

    if (adjustedLimits.maxWidth_ > maxWByH) {
        adjustedLimits.maxWidth_ = maxWByH;
    } else {
        adjustedLimits.maxHeight_ = maxHByW;
    }

    // Add back decorations
    adjustedLimits.Expand(decorW, decorH);
    return adjustedLimits.IsValid() ? adjustedLimits : limits;
}

WSRect AdjustRectByAspectRatio(const WSRect& rect,
                               const WindowLimits& limits,
                               const WindowDecoration& decoration,
                               float aspectRatio,
                               int tolerancePx)
{
    if (MathHelper::NearZero(aspectRatio)) {
        return rect;
    }

    const uint32_t decorW = decoration.Horizontal();
    const uint32_t decorH = decoration.Vertical();

    // Clip limits by decoration
    WindowLimits adjustedLimits = limits;
    adjustedLimits.Clip(decorW, decorH);
    if (!adjustedLimits.IsValid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Invalid adjustedLimits: %{public}s", adjustedLimits.ToString().c_str());
        return rect;
    }

    // Width-prioritized aspect ratio adjustment
    uint32_t minWByH = static_cast<uint32_t>(std::ceil(adjustedLimits.minHeight_ * aspectRatio));
    uint32_t maxWByH = static_cast<uint32_t>(std::floor(adjustedLimits.maxHeight_ * aspectRatio));
    uint32_t minW = std::max(adjustedLimits.minWidth_, minWByH);
    uint32_t maxW = std::min(adjustedLimits.maxWidth_, maxWByH);
    if (minW > maxW) {
        TLOGE(WmsLogTag::WMS_LAYOUT,
            "Aspect ratio conflicts with limits, aspectRatio: %{public}f, adjustedLimits: %{public}s",
            aspectRatio, adjustedLimits.ToString().c_str());
        return rect;
    }

    // Available rect (excluding decoration)
    uint32_t availW = static_cast<uint32_t>(rect.width_) - decorW;
    uint32_t targetW = std::clamp(availW, minW, maxW);
    uint32_t targetH = static_cast<uint32_t>(std::round(static_cast<float>(targetW) / aspectRatio));
    targetH = std::clamp(targetH, adjustedLimits.minHeight_, adjustedLimits.maxHeight_);

    WSRect adjustedRect = rect;
    // Add back decorations
    adjustedRect.width_ = static_cast<int32_t>(targetW + decorW);
    adjustedRect.height_ = static_cast<int32_t>(targetH + decorH);

    // Tolerance: <= tolerancePx pixels is considered no adjustment
    if (std::abs(rect.width_ - adjustedRect.width_) <= tolerancePx &&
        std::abs(rect.height_ - adjustedRect.height_) <= tolerancePx) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Adjustment within tolerance: %{public}dpx, no adjustment", tolerancePx);
        return rect;
    }
    return adjustedRect;
}
} // namespace SessionUtils
} // namespace OHOS::Rosen
