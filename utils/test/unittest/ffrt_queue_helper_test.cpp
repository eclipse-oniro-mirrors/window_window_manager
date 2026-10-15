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
#include <string>

#include "ffrt_queue_helper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
std::string StripGid(std::string label);
} // namespace Rosen
} // namespace OHOS

namespace OHOS {
namespace Rosen {
namespace {
class FfrtQueueHelperTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};
} // namespace

/**
 * @tc.name: StripGid001
 * @tc.desc: strip trailing _<gid> from sq_ queue-task label
 * @tc.type: FUNC
 */
HWTEST_F(FfrtQueueHelperTest, StripGid001, TestSize.Level1)
{
    EXPECT_EQ(StripGid("sq_FfrtQueueHelper_5_SubmitTask_42"), "sq_FfrtQueueHelper_5_SubmitTask");
    EXPECT_EQ(StripGid("sq_dfx_freeze_task_queue_16_NotifyAppFaultTask_24611"),
        "sq_dfx_freeze_task_queue_16_NotifyAppFaultTask");
    EXPECT_EQ(StripGid("sq_DualDisplayFoldPolicy_3_Submit_99"), "sq_DualDisplayFoldPolicy_3_Submit");
}

/**
 * @tc.name: StripGid002
 * @tc.desc: name containing _<digits> is kept; only trailing gid stripped
 * @tc.type: FUNC
 */
HWTEST_F(FfrtQueueHelperTest, StripGid002, TestSize.Level1)
{
    EXPECT_EQ(StripGid("sq_Q_5_Task_42_100"), "sq_Q_5_Task_42");
    EXPECT_EQ(StripGid("sq_Q_5_On_42_Ev_100"), "sq_Q_5_On_42_Ev");
}

/**
 * @tc.name: StripGid003
 * @tc.desc: non-sq_ (normal task) labels returned unchanged
 * @tc.type: FUNC
 */
HWTEST_F(FfrtQueueHelperTest, StripGid003, TestSize.Level1)
{
    EXPECT_EQ(StripGid("timeStartTask"), "timeStartTask");
    EXPECT_EQ(StripGid("root"), "root");
    EXPECT_EQ(StripGid(""), "");
}

/**
 * @tc.name: StripGid004
 * @tc.desc: sq_ label without trailing all-digit suffix is not modified
 * @tc.type: FUNC
 */
HWTEST_F(FfrtQueueHelperTest, StripGid004, TestSize.Level1)
{
    EXPECT_EQ(StripGid("sq_Q_5_Name"), "sq_Q_5_Name");
    EXPECT_EQ(StripGid("sq_"), "sq_");
    EXPECT_EQ(StripGid("sq_Q_5_"), "sq_Q_5_");
}

/**
 * @tc.name: StripGid005
 * @tc.desc: empty-name queue task: only trailing _<gid> stripped
 * @tc.type: FUNC
 */
HWTEST_F(FfrtQueueHelperTest, StripGid005, TestSize.Level1)
{
    EXPECT_EQ(StripGid("sq_Q_5__42"), "sq_Q_5_");
}

/**
 * @tc.name: StopStatCollectionWithoutStart
 * @tc.desc: StopStatCollection without prior Start is a safe no-op
 * @tc.type: FUNC
 */
HWTEST_F(FfrtQueueHelperTest, StopStatCollectionWithoutStart, TestSize.Level1)
{
    FfrtQueueHelper helper;
    helper.StopStatCollection();
}

/**
 * @tc.name: StartAndStopStatCollection
 * @tc.desc: Start then Stop must not crash
 * @tc.type: FUNC
 */
HWTEST_F(FfrtQueueHelperTest, StartAndStopStatCollection, TestSize.Level1)
{
    FfrtQueueHelper helper;
    helper.StartStatCollection();
    helper.StopStatCollection();
}

/**
 * @tc.name: DoubleStartStatCollection
 * @tc.desc: second Start is idempotent (no-op)
 * @tc.type: FUNC
 */
HWTEST_F(FfrtQueueHelperTest, DoubleStartStatCollection, TestSize.Level1)
{
    FfrtQueueHelper helper;
    helper.StartStatCollection();
    helper.StartStatCollection();
    helper.StopStatCollection();
}

/**
 * @tc.name: StartStopStartStatCollection
 * @tc.desc: Start-Stop-Start-Stop re-entrancy must not crash
 * @tc.type: FUNC
 */
HWTEST_F(FfrtQueueHelperTest, StartStopStartStatCollection, TestSize.Level1)
{
    FfrtQueueHelper helper;
    helper.StartStatCollection();
    helper.StopStatCollection();
    helper.StartStatCollection();
    helper.StopStatCollection();
}
} // namespace
} // namespace Rosen
} // namespace OHOS
