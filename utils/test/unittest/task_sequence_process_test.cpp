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

namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_errLog += msg;
    }
}

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
    TaskSequenceProcess process = TaskSequenceProcess(10, 0, 1000, "test2");
    EXPECT_EQ(process.maxQueueSize_, 10);
    EXPECT_EQ(process.maxQueueNumber_, 1);
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
    process.AddTask(0, task);
    EXPECT_TRUE(taskCallback);
}

/**
* @tc.name: AddTaskTest02
* @tc.desc: AddTaskTest02
* @tc.type: FUNC
*/
HWTEST_F(TaskSequenceProcessTest, ATC_AddTask02, TestSize.Level0)
{
    TaskSequenceProcess process = TaskSequenceProcess(1, 1000, "test1");
    bool taskCallback =  false;
    std::function<void()> task = [&taskCallback]() {
        taskCallback =true;
    };
    process.AddTask(task);
    EXPECT_TRUE(taskCallback);
}

/**
* @tc.name: PopFromQueue01
* @tc.desc: PopFromQueue01
* @tc.type: FUNC
*/
HWTEST_F(TaskSequenceProcessTest, ATC_PopFromQueue01, TestSize.Level0)
{
    TaskSequenceProcess process = TaskSequenceProcess(3, 3, 1000, "test1");
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    process.PopFromQueue();
    EXPECT_TRUE(g_errLog.find("TaskSequenceProcess is empty") != std::string::npos);
    g_errLog.clear();
}

/**
* @tc.name: PopFromQueue02
* @tc.desc: PopFromQueue02
* @tc.type: FUNC
*/
HWTEST_F(TaskSequenceProcessTest, ATC_PopFromQueue02, TestSize.Level0)
{
    TaskSequenceProcess process = TaskSequenceProcess(3, 3, 1000, "test2");
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    process.taskRunningFlag_.store(true);
    process.PushToQueue(3, {1, nullptr});
    process.PopFromQueue();
    EXPECT_TRUE(g_errLog.find("TaskSequenceProcess do not pop") != std::string::npos);
    g_errLog.clear();
}

/**
* @tc.name: PopFromQueue03
* @tc.desc: PopFromQueue03
* @tc.type: FUNC
*/
HWTEST_F(TaskSequenceProcessTest, ATC_PopFromQueue03, TestSize.Level0)
{
    TaskSequenceProcess process = TaskSequenceProcess(3, 3, 1000, "test1");
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    process.taskRunningFlag_.store(false);
    process.PushToQueue(0, {0, nullptr});
    process.taskTimerId_ = 0;
    process.PopFromQueue();
    EXPECT_TRUE(g_errLog.find("TaskSequenceProcess do not StartSysTimer succ") != std::string::npos);
    g_errLog.clear();
}

/**
* @tc.name: FindMinSnTaskQueueId01
* @tc.desc: FindMinSnTaskQueueId01
* @tc.type: FUNC
*/
HWTEST_F(TaskSequenceProcessTest, ATC_FindMinSnTaskQueueId01, TestSize.Level0)
{
    TaskSequenceProcess process = TaskSequenceProcess(2, 2, 1000, "test1");
    uint64_t id = 0;
    auto res = !process.FindMinSnTaskQueueId(id);
    EXPECT_TRUE(res);

    process.taskQueueMap_[0];
    process.taskQueueMap_[1];
    res = !process.FindMinSnTaskQueueId(id);
    EXPECT_TRUE(res);

    process.PushToQueue(1, {1, nullptr});
    process.FindMinSnTaskQueueId(id);
    process.PushToQueue(0, {0, nullptr});
    res = process.FindMinSnTaskQueueId(id);
    EXPECT_TRUE(res);
}


/**
* @tc.name: PushToQueue01
* @tc.desc: PushToQueue01
* @tc.type: FUNC
*/
HWTEST_F(TaskSequenceProcessTest, ATC_PushToQueue01, TestSize.Level0)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    TaskSequenceProcess process = TaskSequenceProcess(1, 1, 1000, "test1");
    process.PushToQueue(0, {0, nullptr});
    process.PushToQueue(1, {0, nullptr});
    EXPECT_TRUE(g_errLog.find("Task Push fail, maxQueueNumber:") != std::string::npos);
    process.PushToQueue(0, {1, nullptr});

    g_errLog.clear();
}

/**
* @tc.name: ExecTask01
* @tc.desc: ExecTask01
* @tc.type: FUNC
*/
HWTEST_F(TaskSequenceProcessTest, ATC_ExecTask01, TestSize.Level0)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    TaskSequenceProcess process = TaskSequenceProcess(1, 1, 1000, "test1");
    process.PushToQueue(0, {0, nullptr});
    process.ExecTask();
    EXPECT_TRUE(g_errLog.find("TaskSequenceProcess do not execute") != std::string::npos);
    g_errLog.clear();
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
    process.PushToQueue(0, {0, task});
    process.FinishTask();
    EXPECT_TRUE(taskCallback);
}

/**
* @tc.name: CreateSysTimer01
* @tc.desc: CreateSysTimer01
* @tc.type: FUNC
*/
HWTEST_F(TaskSequenceProcessTest, ATC_CreateSysTimer01, TestSize.Level0)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    TaskSequenceProcess process = TaskSequenceProcess(1, 1, 1000, "test1");
    process.CreateSysTimer();
    EXPECT_TRUE(g_errLog.find("TaskTimerId is not zero") != std::string::npos);
    g_errLog.clear();
}

/**
* @tc.name: StartSysTimer01
* @tc.desc: StartSysTimer01
* @tc.type: FUNC
*/
HWTEST_F(TaskSequenceProcessTest, ATC_StartSysTimer01, TestSize.Level0)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    TaskSequenceProcess process = TaskSequenceProcess(1, 1, 1000, "test1");
    auto id = process.taskTimerId_;
    process.taskTimerId_ = 0;
    process.StartSysTimer();
    EXPECT_TRUE(g_errLog.find("TaskTimerId is zero") != std::string::npos);
    g_errLog.clear();

    process.taskTimerId_ = id;
    process.StartSysTimer();
}

/**
* @tc.name: StopSysTimer01
* @tc.desc: StopSysTimer01
* @tc.type: FUNC
*/
HWTEST_F(TaskSequenceProcessTest, ATC_StopSysTimer01, TestSize.Level0)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    TaskSequenceProcess process = TaskSequenceProcess(1, 1, 1000, "test1");
    auto id = process.taskTimerId_;
    process.taskTimerId_ = 0;
    process.StopSysTimer();
    EXPECT_TRUE(g_errLog.find("TaskSequenceProcess TaskTimerId is zero") != std::string::npos);
    g_errLog.clear();

    process.taskTimerId_ = id;
    process.StopSysTimer();
}
} // namespace
} // namespace Rosen
} // namespace OHOS