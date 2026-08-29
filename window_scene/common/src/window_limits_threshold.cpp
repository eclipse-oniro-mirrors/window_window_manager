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

#include "window_limits_threshold.h"

#include <parameters.h>

#include "string_util.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr const char* LIMITS_THRESHOLD_ENABLED_PARAM_KEY =
    "persist.windowlayout.windowLimitsThreshold.enable";
constexpr const char* LIMITS_THRESHOLD_PERCENTAGE_PARAM_KEY =
    "persist.windowlayout.windowLimitsThreshold.limitsthresholdpercentage";
} // namespace

std::optional<WindowLimitsThresholdConfig> WindowLimitsThreshold::limitsThresholdConfig_;

template<typename T>
std::optional<T> GetOptionalNumericParameter(const char* key)
{
    std::string valueStr = system::GetParameter(key, "");
    if (valueStr.empty()) {
        return std::nullopt;
    }

    T value{};
    auto [_, ec] = std::from_chars(valueStr.data(), valueStr.data() + valueStr.size(), value);
    if (ec != std::errc{}) {
        return std::nullopt;
    }
    return value;
}

bool WindowLimitsThreshold::LimitsThresholdEnabled()
{
    const auto config = GetLimitsThresholdConfig();
    return config.enable;
}

void WindowLimitsThreshold::SaveLimitsThresholdConfig(const WindowLimitsThresholdConfig& config)
{
    system::SetParameter(LIMITS_THRESHOLD_ENABLED_PARAM_KEY, config.enable ? "true" : "false");
    system::SetParameter(LIMITS_THRESHOLD_PERCENTAGE_PARAM_KEY,
                         std::to_string(config.limitsThresholdPercentage));
    limitsThresholdConfig_ = config;
    TLOGD(WmsLogTag::WMS_LAYOUT, "%{public}s", config.ToString().c_str());
}

WindowLimitsThresholdConfig WindowLimitsThreshold::LoadLimitsThresholdConfig()
{
    WindowLimitsThresholdConfig config;
    config.enable = system::GetBoolParameter(LIMITS_THRESHOLD_ENABLED_PARAM_KEY, false);
    config.limitsThresholdPercentage =
        GetOptionalNumericParameter<int32_t>(LIMITS_THRESHOLD_PERCENTAGE_PARAM_KEY).value_or(0); // 2: default lead time

    TLOGD(WmsLogTag::WMS_LAYOUT, "%{public}s", config.ToString().c_str());
    return config;
}

WindowLimitsThresholdConfig WindowLimitsThreshold::GetLimitsThresholdConfig()
{
    if (!limitsThresholdConfig_.has_value()) {
        limitsThresholdConfig_ = LoadLimitsThresholdConfig();
    }
    return limitsThresholdConfig_.value();
}

} // namespace OHOS::Rosen
