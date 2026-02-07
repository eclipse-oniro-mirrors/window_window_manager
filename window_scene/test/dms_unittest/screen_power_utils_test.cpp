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

#include "window_manager_hilog.h"
#include "gtest/gtest.h"
#include "screen_power_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class ScreenPowerUtilsTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override;
    void TearDown() override;
};

void ScreenPowerUtilsTest::SetUp(void)
{}
void ScreenPowerUtilsTest::TearDown(void)
{}

/**
 * @tc.name: ForceTimingOut_001
 * @tc.desc: Test the EnablePowerForceTimingOut/DisablePowerForceTimingOut function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenPowerUtilsTest, ForceTimingOut_001, TestSize.Level1)
{
    TLOGI(WmsLogTag::DMS, "EnableForceTimingOut_001");
    EXPECT_FALSE(ScreenPowerUtils::GetEnablePowerForceTimingOut());

    ScreenPowerUtils::EnablePowerForceTimingOut();
    EXPECT_TRUE(ScreenPowerUtils::GetEnablePowerForceTimingOut());

    ScreenPowerUtils::DisablePowerForceTimingOut();
    EXPECT_FALSE(ScreenPowerUtils::GetEnablePowerForceTimingOut());
}
} // namespace Rosen
} // namespace OHOS