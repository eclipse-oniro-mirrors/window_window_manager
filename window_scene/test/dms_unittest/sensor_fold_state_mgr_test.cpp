/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <hisysevent.h>
#include <parameters.h>
#include "gmock/gmock.h"
#include <functional>
#include "window_manager_hilog.h"
#include "screen_session_manager.h"
#include "sensor_agent.h"
#include "sensor_agent_type.h"

#define private public
#define protected public
#include "sensor_fold_state_mgr.h"
#undef private
#undef protected

namespace {
std::string g_logMsg;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
                   const char* msg)
{
    g_logMsg += msg;
}
}
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace DMS {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
bool g_callFlag = false;

class SensorFoldStateMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SensorFoldStateMgrTest::SetUpTestCase() {}

void SensorFoldStateMgrTest::TearDownTestCase() {}

void SensorFoldStateMgrTest::SetUp() {}

void SensorFoldStateMgrTest::TearDown()
{
    LOG_SetCallback(nullptr);
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: HandleSensorEventTest01
 * @tc.desc: test function : handle tent sensor event, repeat report tent mode
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateMgrTest, HandleSensorEventTest01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorStatus sensorStatus;
    sensorStatus.updateSensorType_ = DmsSensorType::SENSOR_TYPE_TENT;
    sensorStatus.tentSensorInfo_.tentType_ = 0;
    ScreenAxis axis;
    sensorStatus.axis_.emplace_back(axis);
    SensorFoldStateMgr::GetInstance().tentModeType_ = 0;
    SensorFoldStateMgr::GetInstance().HandleSensorEvent(sensorStatus);
    EXPECT_TRUE(g_logMsg.find("Repeat reporting tent mode") != std::string::npos);
}

/**
 * @tc.name: HandleSensorEventTest02
 * @tc.desc: test function : handle tent sensor event, tent mode from off to on
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateMgrTest, HandleSensorEventTest02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorStatus sensorStatus;
    sensorStatus.updateSensorType_ = DmsSensorType::SENSOR_TYPE_TENT;
    sensorStatus.tentSensorInfo_.tentType_ = 1;
    ScreenAxis axis;
    sensorStatus.axis_.emplace_back(axis);
    SensorFoldStateMgr::GetInstance().tentModeType_ = 0;
    SensorFoldStateMgr::GetInstance().HandleSensorEvent(sensorStatus);
    EXPECT_TRUE(g_logMsg.find("report tentStatus: 1") != std::string::npos);
}

/**
 * @tc.name: HandleSensorEventTest03
 * @tc.desc: test function : handle tent sensor event, tent mode from on to off
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateMgrTest, HandleSensorEventTest03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorStatus sensorStatus;
    sensorStatus.updateSensorType_ = DmsSensorType::SENSOR_TYPE_TENT;
    sensorStatus.tentSensorInfo_.tentType_ = 0;
    sensorStatus.tentSensorInfo_.hall_ = 0;
    ScreenAxis axis;
    sensorStatus.axis_.emplace_back(axis);
    SensorFoldStateMgr::GetInstance().tentModeType_ = 1;
    SensorFoldStateMgr::GetInstance().HandleSensorEvent(sensorStatus);
    EXPECT_TRUE(g_logMsg.find("report tentStatus: 0") != std::string::npos);
}

/**
 * @tc.name: HandleSensorEventTest04
 * @tc.desc: test function : handle tent sensor event, tent mode from on to off
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateMgrTest, HandleSensorEventTest04, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorStatus sensorStatus;
    sensorStatus.updateSensorType_ = DmsSensorType::SENSOR_TYPE_TENT;
    sensorStatus.tentSensorInfo_.tentType_ = 0;
    sensorStatus.tentSensorInfo_.hall_ = 0;
    ScreenAxis axis;
    sensorStatus.axis_.emplace_back(axis);
    SensorFoldStateMgr::GetInstance().tentModeType_ = 1;
    SensorFoldStateMgr::GetInstance().HandleSensorEvent(sensorStatus);
    EXPECT_TRUE(g_logMsg.find("report tentStatus: 0") != std::string::npos);
}

/**
 * @tc.name: HandleSensorEventTest05
 * @tc.desc: test function : handle posture sensor event, tent mode from on to off, hall is 0
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateMgrTest, HandleSensorEventTest05, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorStatus sensorStatus;
    sensorStatus.updateSensorType_ = DmsSensorType::SENSOR_TYPE_POSTURE;
    ScreenAxis axis = { 0, 0 };
    sensorStatus.axis_.emplace_back(axis);
    SensorFoldStateMgr::GetInstance().tentModeType_ = 1;
    SensorFoldStateMgr::GetInstance().HandleSensorEvent(sensorStatus);
    EXPECT_TRUE(g_logMsg.find("Exit tent mode due to hall sensor report folded") != std::string::npos);
}

/**
 * @tc.name: HandleSensorEventTest06
 * @tc.desc: test function : handle posture sensor event, tent mode from on to off, hall is 1 and angle is 180
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateMgrTest, HandleSensorEventTest06, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorStatus sensorStatus;
    sensorStatus.updateSensorType_ = DmsSensorType::SENSOR_TYPE_POSTURE;
    ScreenAxis axis = { 180, 1 };
    sensorStatus.axis_.emplace_back(axis);
    SensorFoldStateMgr::GetInstance().tentModeType_ = 1;
    SensorFoldStateMgr::GetInstance().HandleSensorEvent(sensorStatus);
    EXPECT_TRUE(g_logMsg.find("Exit tent mode due to angle sensor report angle:180") != std::string::npos);
}

/**
 * @tc.name: HandleSensorEventTest07
 * @tc.desc: test function : handle posture sensor event, invalid axis size
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateMgrTest, HandleSensorEventTest07, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorStatus sensorStatus;
    SensorFoldStateMgr::GetInstance().HandleSensorEvent(sensorStatus);
    EXPECT_TRUE(g_logMsg.find("invalid sensor status, axis size: 0") != std::string::npos);
}

/**
 * @tc.name: HandleSensorEventTest08
 * @tc.desc: test function : handle posture sensor event, fold state doesn't change, angle is 0
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateMgrTest, HandleSensorEventTest08, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorStatus sensorStatus;
    sensorStatus.updateSensorType_ = DmsSensorType::SENSOR_TYPE_POSTURE;
    ScreenAxis axis = { -1.0, 0 };
    sensorStatus.axis_.emplace_back(axis);
    SensorFoldStateMgr::GetInstance().tentModeType_ = 0;
    SensorFoldStateMgr::GetInstance().currentFoldStatus_[0] = FoldStatus::FOLDED;
    SensorFoldStateMgr::GetInstance().HandleSensorEvent(sensorStatus);
    EXPECT_EQ(SensorFoldStateMgr::GetInstance().globalFoldStatus_, FoldStatus::FOLDED);
}

/**
 * @tc.name: HandleSensorEventTest09
 * @tc.desc: test function : handle posture sensor event, fold status from expand to fold with SMALLER_BOUNDARY_FLAG
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateMgrTest, HandleSensorEventTest09, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorStatus sensorStatus;
    sensorStatus.updateSensorType_ = DmsSensorType::SENSOR_TYPE_POSTURE;
    ScreenAxis axis = { 10.0, 0 };
    sensorStatus.axis_.emplace_back(axis);
    SensorFoldStateMgr::GetInstance().tentModeType_ = 0;
    SensorFoldStateMgr::GetInstance().globalFoldStatus_ = FoldStatus::EXPAND;
    SensorFoldStateMgr::GetInstance().HandleSensorEvent(sensorStatus);
    EXPECT_EQ(SensorFoldStateMgr::GetInstance().foldAlgorithmStrategy_[0], 0);
    EXPECT_TRUE(g_logMsg.find("current state: 1, next state: 2.") != std::string::npos);
}

/**
 * @tc.name: HandleSensorEventTest10
 * @tc.desc: test function : handle posture sensor event, fold status from fold to expand with LARGER_BOUNDARY_FOR_THRESHOLD
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateMgrTest, HandleSensorEventTest10, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorStatus sensorStatus;
    sensorStatus.updateSensorType_ = DmsSensorType::SENSOR_TYPE_POSTURE;
    ScreenAxis axis = { 170.0, 1 };
    sensorStatus.axis_.emplace_back(axis);
    SensorFoldStateMgr::GetInstance().tentModeType_ = 0;
    SensorFoldStateMgr::GetInstance().currentFoldStatus_[0] = FoldStatus::FOLDED;
    SensorFoldStateMgr::GetInstance().globalFoldStatus_ = FoldStatus::FOLDED;
    SensorFoldStateMgr::GetInstance().HandleSensorEvent(sensorStatus);
    EXPECT_EQ(SensorFoldStateMgr::GetInstance().foldAlgorithmStrategy_[0], 1);
    EXPECT_TRUE(g_logMsg.find("current state: 2, next state: 1.") != std::string::npos);
}

/**
 * @tc.name: NotifyRunTaskSequence
 * @tc.desc: NotifyRunTaskSequence
 * @tc.type: FUNC
 */
HWTEST_F(SensorFoldStateMgrTest, FinishTaskSequence, TestSize.Level0)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    SensorFoldStateMgr::GetInstance().FinishTaskSequence();
    EXPECT_TRUE(g_logMsg.find("TaskSequenceProcess") != std::string::npos);
    LOG_SetCallback(nullptr);
    g_logMsg.clear();
}
}
}
} // namespace Rosen
} // namespace OHOS