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
 * WITHOUT WARRANTIES OR CONDITIONS KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "window_limits_threshold.h"

#include <gtest/gtest.h>
#include <parameters.h>

#include <string>

using namespace testing;
using namespace testing::ext;

namespace OHOS::Rosen {
namespace {

class WindowLimitsThresholdTest : public testing::Test {};

/**
 * @tc.name: SaveAndLoadLimitsThresholdConfig
 * @tc.desc: Verify limits threshold configuration serialization, cache update, and lazy initialization.
 * @tc.type: FUNC
 */
HWTEST_F(WindowLimitsThresholdTest, SaveAndLoadLimitsThresholdConfig, TestSize.Level1)
{
    WindowLimitsThresholdConfig originalConfig = WindowLimitsThreshold::LoadLimitsThresholdConfig();

    WindowLimitsThresholdConfig config;
    config.enable = true;
    config.limitsThresholdPercentage = 81;
    WindowLimitsThreshold::SaveLimitsThresholdConfig(config);

    const auto loadedConfig = WindowLimitsThreshold::LoadLimitsThresholdConfig();
    EXPECT_TRUE(loadedConfig.enable);
    EXPECT_EQ(loadedConfig.limitsThresholdPercentage, 81);

    EXPECT_TRUE(WindowLimitsThreshold::limitsThresholdConfig_.has_value());
    EXPECT_TRUE(WindowLimitsThreshold::LimitsThresholdEnabled());

    // Cache reset triggers lazy initialization from system parameters
    WindowLimitsThreshold::limitsThresholdConfig_.reset();
    const auto cachedConfig = WindowLimitsThreshold::LoadLimitsThresholdConfig();
    EXPECT_EQ(cachedConfig.enable, config.enable);
    EXPECT_EQ(cachedConfig.limitsThresholdPercentage, config.limitsThresholdPercentage);

    WindowLimitsThreshold::SaveLimitsThresholdConfig(originalConfig);
}

/**
 * @tc.name: LimitsThresholdEnabled
 * @tc.desc: Verify enabled state follows the saved config.
 * @tc.type: FUNC
 */
HWTEST_F(WindowLimitsThresholdTest, LimitsThresholdEnabled, TestSize.Level1)
{
    WindowLimitsThresholdConfig originalConfig = WindowLimitsThreshold::LoadLimitsThresholdConfig();

    WindowLimitsThresholdConfig config;
    config.enable = true;
    config.limitsThresholdPercentage = 81;
    WindowLimitsThreshold::SaveLimitsThresholdConfig(config);
    EXPECT_TRUE(WindowLimitsThreshold::LimitsThresholdEnabled());

    config.enable = false;
    WindowLimitsThreshold::SaveLimitsThresholdConfig(config);
    EXPECT_FALSE(WindowLimitsThreshold::LimitsThresholdEnabled());

    WindowLimitsThreshold::SaveLimitsThresholdConfig(originalConfig);
}

/**
 * @tc.name: LoadLimitsThresholdConfigDefault
 * @tc.desc: Verify default values when no parameter has ever been set.
 * @tc.type: FUNC
 */
HWTEST_F(WindowLimitsThresholdTest, LoadLimitsThresholdConfigDefault, TestSize.Level1)
{
    WindowLimitsThresholdConfig originalConfig = WindowLimitsThreshold::LoadLimitsThresholdConfig();

    // Clear parameters to simulate never-configured state
    system::SetParameter("persist.windowlayout.windowLimitsThreshold.enable", "false");
    system::SetParameter("persist.windowlayout.windowLimitsThreshold.limitsthresholdpercentage", "0");
    WindowLimitsThreshold::limitsThresholdConfig_.reset();

    const auto loadedConfig = WindowLimitsThreshold::LoadLimitsThresholdConfig();
    EXPECT_FALSE(loadedConfig.enable);
    EXPECT_EQ(loadedConfig.limitsThresholdPercentage, 0);
    EXPECT_FALSE(WindowLimitsThreshold::LimitsThresholdEnabled());

    WindowLimitsThreshold::SaveLimitsThresholdConfig(originalConfig);
}

/**
 * @tc.name: LoadLimitsThresholdConfigInvalidPercentage
 * @tc.desc: Verify non-numeric percentage parameter falls back to default 0.
 * @tc.type: FUNC
 */
HWTEST_F(WindowLimitsThresholdTest, LoadLimitsThresholdConfigInvalidPercentage, TestSize.Level1)
{
    WindowLimitsThresholdConfig originalConfig = WindowLimitsThreshold::LoadLimitsThresholdConfig();

    // Non-numeric percentage fails from_chars parsing and falls back to default 0
    system::SetParameter("persist.windowlayout.windowLimitsThreshold.enable", "true");
    system::SetParameter("persist.windowlayout.windowLimitsThreshold.limitsthresholdpercentage", "abc");
    WindowLimitsThreshold::limitsThresholdConfig_.reset();

    const auto loadedConfig = WindowLimitsThreshold::LoadLimitsThresholdConfig();
    EXPECT_TRUE(loadedConfig.enable);
    EXPECT_EQ(loadedConfig.limitsThresholdPercentage, 0);

    WindowLimitsThreshold::SaveLimitsThresholdConfig(originalConfig);
}

/**
 * @tc.name: LoadLimitsThresholdConfigEmptyPercentage
 * @tc.desc: Verify empty percentage parameter falls back to default 0.
 * @tc.type: FUNC
 */
HWTEST_F(WindowLimitsThresholdTest, LoadLimitsThresholdConfigEmptyPercentage, TestSize.Level1)
{
    WindowLimitsThresholdConfig originalConfig = WindowLimitsThreshold::LoadLimitsThresholdConfig();

    // Empty percentage returns nullopt and falls back to default 0
    system::SetParameter("persist.windowlayout.windowLimitsThreshold.enable", "true");
    system::SetParameter("persist.windowlayout.windowLimitsThreshold.limitsthresholdpercentage", "");
    WindowLimitsThreshold::limitsThresholdConfig_.reset();

    const auto loadedConfig = WindowLimitsThreshold::LoadLimitsThresholdConfig();
    EXPECT_TRUE(loadedConfig.enable);
    EXPECT_EQ(loadedConfig.limitsThresholdPercentage, 0);

    WindowLimitsThreshold::SaveLimitsThresholdConfig(originalConfig);
}

} // namespace
} // namespace OHOS::Rosen
