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
#include "fold_screen_controller/fold_screen_sensor_manager.h"
#include "fold_screen_state_internel.h"
#include "window_manager_hilog.h"
#include "screen_session_manager.h"
#include "scene_board_judgement.h"
#include "fold_screen_controller/secondary_fold_sensor_manager.h"
#include "fold_screen_controller/fold_screen_controller_config.h"
#include "fold_screen_common.h"
#include "screen_sensor_mgr.h"
namespace {
std::string g_logMsg;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
    const char* msg)
{
    g_logMsg = msg;
}
}
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
}

class FoldScreenSensorManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

ScreenSessionManager *ssm_;
void FoldScreenSensorManagerTest::SetUpTestCase()
{
    ssm_ = &ScreenSessionManager::GetInstance();
}

void FoldScreenSensorManagerTest::TearDownTestCase()
{
}

void FoldScreenSensorManagerTest::SetUp()
{
}

void FoldScreenSensorManagerTest::TearDown()
{
    DMS::ScreenSensorMgr::GetInstance().UnSubscribeSensorCallback(
        SENSOR_TYPE_ID_POSTURE);
    DMS::ScreenSensorMgr::GetInstance().UnSubscribeSensorCallback(
        SENSOR_TYPE_ID_HALL_EXT);
    usleep(SLEEP_TIME_US);
}

namespace {

/**
* @tc.name  : UnSubscribeSensorCallback
* @tc.number: UnSubscribeSensorCallback_004
* @tc.desc  : FUNC
*/
HWTEST_F(FoldScreenSensorManagerTest, UnSubscribeSensorCallback_004, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    FoldScreenSensorManager manager;
    manager.RegisterPostureCallback();
    if (!(ssm_->IsFoldable())) {
        GTEST_SKIP();
    }
    EXPECT_TRUE(g_logMsg.find("Unsubscribe sensor type: sensorTypeId=%{public}d") == std::string::npos);
    EXPECT_TRUE(g_logMsg.find("failed.") == std::string::npos);

    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: HandlePostureData
 * @tc.desc: test function : HandlePostureData
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenSensorManagerTest, HandlePostureData, TestSize.Level1)
{
    FoldScreenSensorManager mgr = FoldScreenSensorManager();
    mgr.HandlePostureData(nullptr);
    EXPECT_EQ(mgr.globalAngle, -1.0F);

    SensorEvent event;
    PostureData postureData;
    postureData.angle = 45.0F;
    mgr.HandlePostureData(&event);
    EXPECT_EQ(mgr.globalAngle, -1.0F);

    event.data = reinterpret_cast<uint8_t*>(&postureData);
    event.dataLen = 0;
    mgr.HandlePostureData(&event);
    EXPECT_EQ(mgr.globalAngle, -1.0F);

    event.dataLen = sizeof(PostureData);
    mgr.HandlePostureData(&event);
    EXPECT_EQ(mgr.globalAngle, 45.0F);
}

/**
 * @tc.name: NotifyFoldAngleChanged
 * @tc.desc: test function : NotifyFoldAngleChanged
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenSensorManagerTest, NotifyFoldAngleChanged, TestSize.Level1)
{
    FoldScreenSensorManager mgr = FoldScreenSensorManager();
    float foldAngle = 0.0F;
    mgr.NotifyFoldAngleChanged(foldAngle);
    EXPECT_TRUE(ScreenSessionManager::GetInstance().lastFoldAngles_.empty());

    foldAngle = 30.0F;
    mgr.NotifyFoldAngleChanged(foldAngle);
    EXPECT_TRUE(ScreenSessionManager::GetInstance().lastFoldAngles_.empty());
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: HandleHallData
 * @tc.desc: test function : HandleHallData
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenSensorManagerTest, HandleHallData, TestSize.Level1)
{
    FoldScreenSensorManager mgr = FoldScreenSensorManager();
    mgr.HandleHallData(nullptr);
    EXPECT_EQ(mgr.globalHall, USHRT_MAX);

    SensorEvent event;
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.globalHall, USHRT_MAX);

    DMS::ExtHallData hallData;
    event.data = reinterpret_cast<uint8_t*>(&hallData);
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.globalHall, USHRT_MAX);
    
    event.dataLen = 0;
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.globalHall, USHRT_MAX);

    event.dataLen = sizeof(DMS::ExtHallData);
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.globalHall, USHRT_MAX);

    hallData.hall = 10;
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.globalHall, 65535);
}

/**
 * @tc.name: HandleHallDataWhenAngleMinusOne
 * @tc.desc: test function : HandleHallData
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenSensorManagerTest, HandleHallDataWhenAngleMinusOne, TestSize.Level1)
{
    FoldScreenSensorManager mgr = FoldScreenSensorManager();
    mgr.SetSensorFoldStateManager(new SensorFoldStateManager());
    mgr.registerPosture_ = false;
    mgr.SetGlobalAngle(-1.0F);
    mgr.SetGlobalHall(1);

    DMS::ExtHallData hallData = {
        .flag = (1 << 1),
        .hall = 0,
    };
    SensorEvent hallEvent = {
        .data = reinterpret_cast<uint8_t *>(&hallData),
        .dataLen = sizeof(DMS::ExtHallData),
    };
    mgr.HandleHallData(&hallEvent);
    EXPECT_EQ(mgr.globalAngle, ANGLE_MIN_VAL);
}

/**
 * @tc.name: HandleAbnormalAngleTest
 * @tc.desc: test function : HandleAbnormalAngle
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenSensorManagerTest, HandleAbnormalAngleTest, TestSize.Level1)
{
    FoldScreenSensorManager mgr = FoldScreenSensorManager();
    mgr.SetSensorFoldStateManager(new SensorFoldStateManager());
    mgr.SetGlobalAngle(-1.0F);
    mgr.SetGlobalHall(1);
    EXPECT_TRUE(mgr.HandleAbnormalAngle());

    mgr.SetGlobalAngle(-2.0F);
    EXPECT_FALSE(mgr.HandleAbnormalAngle());
}

/**
 * @tc.name: TriggerDisplaySwitch
 * @tc.desc: test function : TriggerDisplaySwitch
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenSensorManagerTest, TriggerDisplaySwitch, TestSize.Level1)
{
    FoldScreenSensorManager mgr = FoldScreenSensorManager();
    mgr.SetSensorFoldStateManager(new SensorFoldStateManager());
    mgr.registerPosture_ = false;
    mgr.TriggerDisplaySwitch();
    EXPECT_EQ(mgr.globalAngle, ANGLE_MIN_VAL);

    mgr.registerPosture_ = true;
    mgr.TriggerDisplaySwitch();
    if (FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
        EXPECT_EQ(mgr.globalAngle, 110);
    } else if (SceneBoardJudgement::IsSceneBoardEnabled() || FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        EXPECT_EQ(mgr.globalAngle, 25);
    } else if (FoldScreenStateInternel::IsSingleDisplayFoldDevice()) {
        EXPECT_EQ(mgr.globalAngle, 25);
    }
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: GetGlobalAngle
 * @tc.desc: test function : GetGlobalAngle
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenSensorManagerTest, GetGlobalAngle, TestSize.Level1)
{
    FoldScreenSensorManager mgr = FoldScreenSensorManager();
    mgr.SetSensorFoldStateManager(new SensorFoldStateManager());
    mgr.registerPosture_ = false;
    float angle = 150.0F;
    mgr.SetGlobalAngle(angle);
    ASSERT_NO_FATAL_FAILURE({mgr.GetGlobalAngle();});
    EXPECT_EQ(mgr.GetGlobalAngle(), angle);
}

/**
 * @tc.name: SetGlobalAngle
 * @tc.desc: test function : SetGlobalAngle
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenSensorManagerTest, SetGlobalAngle, TestSize.Level1)
{
    FoldScreenSensorManager mgr = FoldScreenSensorManager();
    mgr.SetSensorFoldStateManager(new SensorFoldStateManager());
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalAngle(-10.0f);});
    EXPECT_NE(mgr.GetGlobalAngle(), -10.0f);
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalAngle(190.0f);});
    EXPECT_NE(mgr.GetGlobalAngle(), 190.0f);
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalAngle(180.0f);});
    EXPECT_EQ(mgr.GetGlobalAngle(), 180.0f);
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalAngle(150.0f);});
    EXPECT_EQ(mgr.GetGlobalAngle(), 150.0f);
}

/**
 * @tc.name: SetGlobalAngleWhenParamInValid
 * @tc.desc: test function : SetGlobalAngle
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenSensorManagerTest, SetGlobalAngleWhenParamInValid, TestSize.Level1)
{
    FoldScreenSensorManager mgr = FoldScreenSensorManager();
    mgr.SetSensorFoldStateManager(new SensorFoldStateManager());
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalAngle(150.0f);});
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalAngle(-10.0f);});
    EXPECT_NE(mgr.GetGlobalAngle(), -10.0f);
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalAngle(190.0f);});
    EXPECT_NE(mgr.GetGlobalAngle(), 190.0f);
}

/**
 * @tc.name: GetGlobalHall
 * @tc.desc: test function : GetGlobalHall
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenSensorManagerTest, GetGlobalHall, TestSize.Level1)
{
    FoldScreenSensorManager mgr = FoldScreenSensorManager();
    mgr.SetSensorFoldStateManager(new SensorFoldStateManager());
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalHall(0);});
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalHall(1);});
    ASSERT_NO_FATAL_FAILURE({mgr.GetGlobalHall();});
    EXPECT_EQ(mgr.GetGlobalHall(), 1);
}

/**
 * @tc.name: SetGlobalHall
 * @tc.desc: test function : SetGlobalHall
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenSensorManagerTest, SetGlobalHall, TestSize.Level1)
{
    FoldScreenSensorManager mgr = FoldScreenSensorManager();
    mgr.SetSensorFoldStateManager(new SensorFoldStateManager());
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalHall(0);});
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalHall(1);});
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalHall(USHRT_MAX);});
    ASSERT_NO_FATAL_FAILURE({mgr.GetGlobalHall();});
    EXPECT_EQ(mgr.GetGlobalHall(), 1);
}

/**
 * @tc.name: SetGlobalHallWhenParamInValid
 * @tc.desc: test function : SetGlobalHall
 * @tc.type: FUNC
 */
HWTEST_F(FoldScreenSensorManagerTest, SetGlobalHallWhenParamInValid, TestSize.Level1)
{
    FoldScreenSensorManager mgr = FoldScreenSensorManager();
    mgr.SetSensorFoldStateManager(new SensorFoldStateManager());
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalHall(1);});
    ASSERT_NO_FATAL_FAILURE({mgr.SetGlobalHall(USHRT_MAX);});
    ASSERT_NO_FATAL_FAILURE({mgr.GetGlobalHall();});
    EXPECT_NE(mgr.GetGlobalHall(), USHRT_MAX);
}
}
} // namespace Rosen
} // namespace OHOS