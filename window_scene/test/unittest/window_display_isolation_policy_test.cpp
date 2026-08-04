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

#include "window_display_isolation_policy.h"

#include <gtest/gtest.h>

#include <set>
#include <string>

#include "dm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Rosen {
namespace {

class WindowDisplayIsolationPolicyTest : public testing::Test {};

/**
 * @tc.name: SaveAndLoadDisplayIsolationSystemConfig
 * @tc.desc: Verify display isolation configuration serialization, cache update, and lazy initialization.
 * @tc.type: FUNC
 */
HWTEST_F(WindowDisplayIsolationPolicyTest, SaveAndLoadDisplayIsolationSystemConfig, TestSize.Level1)
{
    DisplayIsolationConfig originalConfig = WindowDisplayIsolationPolicy::LoadDisplayIsolationSystemConfig();

    DisplayIsolationConfig config;
    config.moveIsolatedDisplayIds = { 7, 9 };
    config.dragIsolatedDisplayIds = { 8 };
    WindowDisplayIsolationPolicy::SaveDisplayIsolationSystemConfig(config);

    const auto loadedConfig = WindowDisplayIsolationPolicy::LoadDisplayIsolationSystemConfig();
    EXPECT_EQ(loadedConfig.moveIsolatedDisplayIds, (std::set<DisplayId> { 7, 9 }));
    EXPECT_EQ(loadedConfig.dragIsolatedDisplayIds, (std::set<DisplayId> { 8 }));

    EXPECT_TRUE(WindowDisplayIsolationPolicy::displayIsolationConfig_.has_value());
    auto cachedConfig = WindowDisplayIsolationPolicy::GetDisplayIsolationConfig();
    EXPECT_EQ(cachedConfig.moveIsolatedDisplayIds, config.moveIsolatedDisplayIds);
    EXPECT_EQ(cachedConfig.dragIsolatedDisplayIds, config.dragIsolatedDisplayIds);

    WindowDisplayIsolationPolicy::displayIsolationConfig_.reset();
    cachedConfig = WindowDisplayIsolationPolicy::GetDisplayIsolationConfig();
    EXPECT_TRUE(WindowDisplayIsolationPolicy::displayIsolationConfig_.has_value());
    EXPECT_EQ(cachedConfig.moveIsolatedDisplayIds, config.moveIsolatedDisplayIds);
    EXPECT_EQ(cachedConfig.dragIsolatedDisplayIds, config.dragIsolatedDisplayIds);

    WindowDisplayIsolationPolicy::SaveDisplayIsolationSystemConfig(originalConfig);
}

/**
 * @tc.name: IsOperationEnable
 * @tc.desc: Verify isolated display movement rules without depending on the current device type.
 * @tc.type: FUNC
 */
HWTEST_F(WindowDisplayIsolationPolicyTest, IsOperationEnable, TestSize.Level1)
{
    const std::set<DisplayId> isolatedDisplayIds = { 5 };
    const std::set<DisplayId> emptyIsolatedDisplayIds;
    constexpr DisplayId innerDisplayId = 0;
    constexpr DisplayId outerDisplayId = 5;
    constexpr DisplayId extendedDisplayId = 1001;
    constexpr DisplayId anotherExtendedDisplayId = 1002;

    EXPECT_FALSE(WindowDisplayIsolationPolicy::IsOperationEnable(
        isolatedDisplayIds, innerDisplayId, outerDisplayId));
    EXPECT_FALSE(WindowDisplayIsolationPolicy::IsOperationEnable(
        isolatedDisplayIds, outerDisplayId, innerDisplayId));
    EXPECT_FALSE(WindowDisplayIsolationPolicy::IsOperationEnable(
        isolatedDisplayIds, outerDisplayId, extendedDisplayId));
    EXPECT_FALSE(WindowDisplayIsolationPolicy::IsOperationEnable(
        isolatedDisplayIds, extendedDisplayId, outerDisplayId));
    EXPECT_TRUE(WindowDisplayIsolationPolicy::IsOperationEnable(
        isolatedDisplayIds, innerDisplayId, extendedDisplayId));
    EXPECT_TRUE(WindowDisplayIsolationPolicy::IsOperationEnable(
        isolatedDisplayIds, extendedDisplayId, innerDisplayId));
    EXPECT_TRUE(WindowDisplayIsolationPolicy::IsOperationEnable(
        isolatedDisplayIds, extendedDisplayId, anotherExtendedDisplayId));
    EXPECT_TRUE(WindowDisplayIsolationPolicy::IsOperationEnable(
        isolatedDisplayIds, outerDisplayId, outerDisplayId));
    EXPECT_TRUE(WindowDisplayIsolationPolicy::IsOperationEnable(
        isolatedDisplayIds, DISPLAY_ID_INVALID, outerDisplayId));
    EXPECT_TRUE(WindowDisplayIsolationPolicy::IsOperationEnable(
        emptyIsolatedDisplayIds, innerDisplayId, outerDisplayId));
}

/**
 * @tc.name: IsMoveAndDragEnable
 * @tc.desc: Verify move and drag operations use their independent isolation sets.
 * @tc.type: FUNC
 */
HWTEST_F(WindowDisplayIsolationPolicyTest, IsMoveAndDragEnable, TestSize.Level1)
{
    constexpr DisplayId innerDisplayId = 0;
    constexpr DisplayId moveIsolatedDisplayId = 5;
    constexpr DisplayId dragIsolatedDisplayId = 7;
    constexpr DisplayId extendedDisplayId = 1001;
    const auto originalConfig = WindowDisplayIsolationPolicy::LoadDisplayIsolationSystemConfig();
    DisplayIsolationConfig testConfig;
    testConfig.moveIsolatedDisplayIds = { moveIsolatedDisplayId };
    testConfig.dragIsolatedDisplayIds = { dragIsolatedDisplayId };
    WindowDisplayIsolationPolicy::SaveDisplayIsolationSystemConfig(testConfig);

    EXPECT_FALSE(WindowDisplayIsolationPolicy::IsMoveEnable(innerDisplayId, moveIsolatedDisplayId));
    EXPECT_FALSE(WindowDisplayIsolationPolicy::IsMoveEnable(moveIsolatedDisplayId, extendedDisplayId));
    EXPECT_TRUE(WindowDisplayIsolationPolicy::IsMoveEnable(innerDisplayId, dragIsolatedDisplayId));
    EXPECT_TRUE(WindowDisplayIsolationPolicy::IsMoveEnable(innerDisplayId, extendedDisplayId));

    EXPECT_FALSE(WindowDisplayIsolationPolicy::IsDragEnable(innerDisplayId, dragIsolatedDisplayId));
    EXPECT_FALSE(WindowDisplayIsolationPolicy::IsDragEnable(dragIsolatedDisplayId, extendedDisplayId));
    EXPECT_TRUE(WindowDisplayIsolationPolicy::IsDragEnable(innerDisplayId, moveIsolatedDisplayId));
    EXPECT_TRUE(WindowDisplayIsolationPolicy::IsDragEnable(innerDisplayId, extendedDisplayId));

    DisplayIsolationConfig updatedConfig;
    updatedConfig.moveIsolatedDisplayIds = { dragIsolatedDisplayId };
    updatedConfig.dragIsolatedDisplayIds = { moveIsolatedDisplayId };
    WindowDisplayIsolationPolicy::SaveDisplayIsolationSystemConfig(updatedConfig);
    EXPECT_TRUE(WindowDisplayIsolationPolicy::IsMoveEnable(innerDisplayId, moveIsolatedDisplayId));
    EXPECT_FALSE(WindowDisplayIsolationPolicy::IsMoveEnable(innerDisplayId, dragIsolatedDisplayId));
    EXPECT_FALSE(WindowDisplayIsolationPolicy::IsDragEnable(innerDisplayId, moveIsolatedDisplayId));
    EXPECT_TRUE(WindowDisplayIsolationPolicy::IsDragEnable(innerDisplayId, dragIsolatedDisplayId));

    WindowDisplayIsolationPolicy::SaveDisplayIsolationSystemConfig(originalConfig);
}

} // namespace
} // namespace OHOS::Rosen
