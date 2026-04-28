/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_WM_LAYOUT_COMMON_H
#define OHOS_ROSEN_WM_LAYOUT_COMMON_H

#include <cstdint>

namespace OHOS {
namespace Rosen {

/**
 * @struct SnapshotAnimationConfig
 * @brief Configuration for snapshot animation duration and delay.
 */
struct SnapshotAnimationConfig {
    int64_t duration = -1;  // Animation duration in ms, -1 means use system default
    int64_t delay = -1;     // Animation delay in ms, -1 means use system default
};

/**
 * Enum for across-display policy used when maximizing in the half-folded state of a foldable 2-in-1 device.
 */
enum class AcrossDisplayPresentation : uint32_t {
    /**
     * Indicates following the current acrossDisplayPresentation.
     * If the acrossDisplayPresentation has not been set, the default system policy applies:
     * In the half-folded state of the device, the window enters single-screen maximization
     * (i.e., when maximized, the window is displayed only on the upper or lower half of the screen).
     * In the expanded state, the window is maximized and remains across-display mode
     * (i.e., spanning across both the upper and lower displays) when folded back to the half-folded state.
     */
    FOLLOW_ACROSS_DISPLAY_SETTING = 0,

    /**
     * In the half-folded state of the device, the window could directly enter the across-display mode.
     * In the expanded state, the window is maximized and remains across-display mode
     * when folded back to the half-folded state.
     */
    ENTER_ACROSS_DISPLAY_MODE = 1,

    /**
     * In the half-folded state of the device, the window exits across-display mode and enters
     * single-screen maximization.
     * In the expanded state, the window is maximized and will exit across-display mode upon
     * re-entering half-folded.
     */
    EXIT_ACROSS_DISPLAY_MODE = 2,
};

} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_WM_LAYOUT_COMMON_H
