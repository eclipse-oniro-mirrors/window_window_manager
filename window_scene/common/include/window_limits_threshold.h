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

#ifndef OHOS_ROSEN_WINDOW_LIMITS_THRESHOLD_H
#define OHOS_ROSEN_WINDOW_LIMITS_THRESHOLD_H

#include <optional>
#include <set>
#include <string>

#include "dm_common.h"

namespace OHOS::Rosen {

/**
 * @brief Configuration of display isolation for window operations.
 */
struct WindowLimitsThresholdConfig {
    bool enable = false;

    int32_t limitsThresholdPercentage = 0;

    std::string ToString() const
    {
        std::ostringstream oss;

        oss << "enable: " << enable
            << ", limitsThresholdPercentage: " << limitsThresholdPercentage;

        return oss.str();
    }
};

/**
 * @brief Provides display isolation policy checks for window operations.
 */
class WindowLimitsThreshold final {
public:
    WindowLimitsThreshold() = delete;
    ~WindowLimitsThreshold() = delete;

    static bool LimitsThresholdEnabled();

    static void SaveLimitsThresholdConfig(const WindowLimitsThresholdConfig& config);

    static WindowLimitsThresholdConfig LoadLimitsThresholdConfig();

private:
    static WindowLimitsThresholdConfig GetLimitsThresholdConfig();

    static std::optional<WindowLimitsThresholdConfig> limitsThresholdConfig_;
};

} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_LIMITS_THRESHOLD_H
