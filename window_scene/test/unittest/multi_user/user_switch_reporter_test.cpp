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

#include <gtest/gtest.h>
#include "user_switch_reporter.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class UserSwitchReporterTest : public Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: ConstructorInitializesStartTime
 * @tc.desc: Verify that the constructor correctly records the switch start time.
 * @tc.type: FUNC
 */
HWTEST_F(UserSwitchReporterTest, ConstructorInitializesStartTime, Function | SmallTest | Level1)
{
    UserSwitchReporter reporter(UserSwitchEventType::SWITCHING, true);
    EXPECT_GT(reporter.startTime_, 0);
}

/**
 * @tc.name: ReportSwitchDurationCorrectly
 * @tc.desc: Verify that ReportSwitchDuration correctly reports the switch duration.
 * @tc.type: FUNC
 */
HWTEST_F(UserSwitchReporterTest, ReportSwitchDurationCorrectly, Function | SmallTest | Level1)
{
    UserSwitchReporter reporter(UserSwitchEventType::SWITCHING, true);
    EXPECT_TRUE(reporter.ReportSwitchDuration());
}
} // namespace Rosen
} // namespace OHOS
