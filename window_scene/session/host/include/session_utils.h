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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SESSION_UTILS_H
#define OHOS_ROSEN_WINDOW_SCENE_SESSION_UTILS_H

#include "wm_common_inner.h"
#include "ws_common.h"

namespace OHOS::Rosen {
namespace SessionUtils {
constexpr const char* SESSION_NAME_MARK_HEAD = "#";
constexpr const char* SESSION_NAME_SEPARATOR = ":";

inline void CalcFloatWindowRectLimits(uint32_t maxFloatingWindowSize, float vpr, WindowLimits& limits)
{
    uint32_t maxWidth = (limits.maxWidth_ == 0 || limits.maxWidth_ >= INT32_MAX) ? INT32_MAX : limits.maxWidth_;
    uint32_t maxHeight = (limits.maxHeight_ == 0 || limits.maxHeight_ >= INT32_MAX) ? INT32_MAX : limits.maxHeight_;

    limits.minWidth_ = std::max(limits.minWidth_, static_cast<uint32_t>(MIN_FLOATING_WIDTH * vpr));
    limits.maxWidth_ = std::min(maxWidth, static_cast<uint32_t>(maxFloatingWindowSize * vpr));
    limits.minHeight_ = std::max(limits.minHeight_, static_cast<uint32_t>(MIN_FLOATING_HEIGHT * vpr));
    limits.maxHeight_ = std::min(maxHeight, static_cast<uint32_t>(maxFloatingWindowSize * vpr));
}

inline std::string ConvertSessionName(const std::string& bundleName, const std::string& name,
    const std::string& moduleName, const int32_t appIndex = 0)
{
    std::string strName;
    if (appIndex == 0) {
        strName = SESSION_NAME_MARK_HEAD + bundleName + SESSION_NAME_SEPARATOR + moduleName +
            SESSION_NAME_SEPARATOR + name;
    } else {
        strName = SESSION_NAME_MARK_HEAD + bundleName + SESSION_NAME_SEPARATOR + std::to_string(appIndex) +
            SESSION_NAME_SEPARATOR + moduleName + SESSION_NAME_SEPARATOR + name;
    }

    return strName;
}

inline std::string GetBundleNameBySessionName(const std::string& name)
{
    const size_t len = std::char_traits<char>::length(SESSION_NAME_MARK_HEAD);
    if (name.empty() || name.size() < len || name.compare(0, len, SESSION_NAME_MARK_HEAD) != 0) {
        return "";
    }
    size_t pos = name.find(SESSION_NAME_SEPARATOR);
    return pos != std::string::npos? name.substr(len, pos - 1) : name.substr(len);
}

inline std::string GetAppLockKey(const std::string& bundleName, const int32_t appIndex)
{
    return bundleName + "#" + std::to_string(appIndex);
}

/**
 * @brief Check if the aspect ratio is valid within the given window limits and decoration.
 *
 * @param ratio      The aspect ratio to check (width / height).
 * @param limits     The window size limits including decoration.
 * @param decoration The window decoration sizes (horizontal + vertical).
 * @return True if the aspect ratio is valid, false otherwise.
 */
bool IsAspectRatioValid(float ratio, const WindowLimits& limits, const WindowDecoration& decoration);

/**
 * @brief Adjust window size limits to satisfy the specified aspect ratio,
 *        taking window decoration into account.
 *
 * @param limits      Original window size limits including decoration.
 * @param decoration  Window decoration sizes (horizontal + vertical).
 * @param aspectRatio Desired content aspect ratio (width / height).
 * @return A new WindowLimits structure with aspect ratio constraints applied.
 */
WindowLimits AdjustLimitsByAspectRatio(const WindowLimits& limits,
                                       const WindowDecoration& decoration,
                                       float aspectRatio);

/**
 * @brief Adjust a window rectangle to satisfy both the specified aspect ratio
 *        and the given size limits, considering decoration sizes.
 *
 * @param rect        Original window rectangle including decoration.
 * @param limits      Window size limits including decoration.
 * @param decoration  Window decoration sizes (horizontal + vertical).
 * @param aspectRatio Desired content aspect ratio (width / height).
 * @param tolerancePx Pixel tolerance used to skip small adjustments.
 * @return A new WSRect adjusted to satisfy aspect ratio and size constraints.
 */
WSRect AdjustRectByAspectRatio(const WSRect& rect,
                               const WindowLimits& limits,
                               const WindowDecoration& decoration,
                               float aspectRatio,
                               int tolerancePx = 2);
} // namespace SessionUtils
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SESSION_UTILS_H
