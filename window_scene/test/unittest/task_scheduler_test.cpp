/*
 * Copyright (c)  Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#include "common/include/task_scheduler.h"
#include <gtest/gtest.h>

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class TaskSchedulerText : public testing::Test {
  public:
    TaskSchedulerText() {}
    ~TaskSchedulerText() {}
};

namespace {
/**
 * @tc.name: task_scheduler_test001
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(TaskSchedulerText, task_scheduler_test001, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "TaskSchedulerText: task_scheduler_test001 start";
    std::string threadName = "threadName";
    std::string name = "name";
    TaskScheduler* taskScheduler = new(std::nothrow) TaskScheduler(threadName);
    int res = 0;
    taskScheduler->RemoveTask(name);
    ASSERT_EQ(res, 0);
    delete taskScheduler;
    GTEST_LOG_(INFO) << "TaskSchedulerText: task_scheduler_test001 end";
}

/**
 * @tc.name: GetEventHandler
 * @tc.desc: GetEventHandler function
 * @tc.type: FUNC
 */
HWTEST_F(TaskSchedulerText, GetEventHandler, Function | SmallTest | Level2)
{
    std::string threadName = "threadName";
    std::shared_ptr<TaskScheduler> taskScheduler = std::make_shared<TaskScheduler>(threadName);
    ASSERT_NE(taskScheduler, nullptr);
    EXPECT_NE(taskScheduler->GetEventHandler(), nullptr);
}

/**
 * @tc.name: PostTask
 * @tc.desc: PostTask function
 * @tc.type: FUNC
 */
HWTEST_F(TaskSchedulerText, PostTask, Function | SmallTest | Level2)
{
    std::string threadName = "threadName";
    std::shared_ptr<TaskScheduler> taskScheduler = std::make_shared<TaskScheduler>(threadName);
    ASSERT_NE(taskScheduler, nullptr);
    int resultValue = 0;
    auto taskFunc = [&resultValue]() {
        GTEST_LOG_(INFO) << "START_TASK";
        resultValue = 1;
    };
    taskScheduler->PostAsyncTask(taskFunc);
    EXPECT_NE(taskScheduler->handler_, nullptr);
    EXPECT_EQ(resultValue, 0);

    std::string name = "ssmTask";
    int64_t delayTime = 1;
    taskScheduler->PostAsyncTask(taskFunc, name, delayTime);
    EXPECT_EQ(resultValue, 0);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
