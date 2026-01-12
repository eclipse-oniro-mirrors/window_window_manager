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

#include <gtest/gtest.h>

#include "task_sequence_process.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class TaskSequenceProcessTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void TaskSequenceProcessTest::SetUpTestCase() {}

void TaskSequenceProcessTest::TearDownTestCase() {}

void TaskSequenceProcessTest::SetUp() {}

void TaskSequenceProcessTest::TearDown() {}

namespace {

/**
 * @tc.name: TaskSequenceProcessTest01
 * @tc.desc: TaskSequenceProcessTest01
 * @tc.type: FUNC
 */
HWTEST_F(TaskSequenceProcessTest, ATC_TaskSequenceProcess01, TestSize.Level0)
{
    TaskSequenceProcess process = TaskSequenceProcess(0, 1000, "test1");
    EXPECT_EQ(process.maxQueueSize_, 1);
}

/**
 * @tc.name: TaskSequenceProcessTest02
 * @tc.desc: TaskSequenceProcessTest02
 * @tc.type: FUNC
 */
HWTEST_F(TaskSequenceProcessTest, ATC_TaskSequenceProcess02, TestSize.Level0)
{
    TaskSequenceProcess process = TaskSequenceProcess(10, 1000, "test2");
    EXPECT_EQ(process.maxQueueSize_, 10);
}

/**
 * @tc.name: AddTaskTest01
 * @tc.desc: AddTaskTest01
 * @tc.type: FUNC
 */
HWTEST_F(TaskSequenceProcessTest, ATC_AddTask01, TestSize.Level0)
{
    TaskSequenceProcess process = TaskSequenceProcess(1, 1000, "test3");
    bool taskCallback =  false;
    std::function<void()> task = [&taskCallback]() {
        taskCallback =true;
    };
    process.AddTask(task);
    EXPECT_TRUE(taskCallback);
}

/**
 * @tc.name: FinishTaskTest01
 * @tc.desc: FinishTaskTest01
 * @tc.type: FUNC
 */
HWTEST_F(TaskSequenceProcessTest, ATC_FinishTask01, TestSize.Level0)
{
    TaskSequenceProcess process = TaskSequenceProcess(1, 1000, "test4");
    bool taskCallback =  false;
    std::function<void()> task = [&taskCallback]() {
        taskCallback =true;
    };
    process.taskQueue_.push(task);
    process.FinishTask();
    EXPECT_TRUE(taskCallback);
}
} // namespace
} // namespace Rosen
} // namespace OHOS