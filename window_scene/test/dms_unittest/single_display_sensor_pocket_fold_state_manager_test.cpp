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

#include <parameter.h>
#include <parameters.h>
#include "screen_session_manager/include/fold_screen_controller/sensor_fold_state_manager/single_display_sensor_pocket_fold_state_manager.h"
#include "screen_session_manager/include/fold_screen_controller/single_display_pocket_fold_policy.h"
#include "fold_screen_state_internel.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
}
namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
std::recursive_mutex g_displayInfoMutex;
std::shared_ptr<TaskScheduler> screenPowerTaskScheduler_ = std::make_shared<TaskScheduler>("test");
sptr<SingleDisplayPocketFoldPolicy> g_policy;
}

class SingleDisplaySensorPocketFoldStateManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SingleDisplaySensorPocketFoldStateManagerTest::SetUpTestCase()
{
    g_policy = new SingleDisplayPocketFoldPolicy(g_displayInfoMutex, screenPowerTaskScheduler_);
}

void SingleDisplaySensorPocketFoldStateManagerTest::TearDownTestCase()
{
    usleep(SLEEP_TIME_IN_US);
}

void SingleDisplaySensorPocketFoldStateManagerTest::SetUp()
{
}

void SingleDisplaySensorPocketFoldStateManagerTest::TearDown()
{
}

namespace {
/**
 * @tc.name: HandleAngleChange
 * @tc.desc: test function : HandleAngleChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorPocketFoldStateManagerTest, HandleAngleChange, TestSize.Level1)
{
    float angel = 0;
    int hall = 0;
    sptr<FoldScreenPolicy> foldScreenPolicy = nullptr;
    SingleDisplaySensorPocketFoldStateManager manager;
    manager.HandleAngleChange(angel, hall, foldScreenPolicy);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: HandleHallChange
 * @tc.desc: test function : HandleHallChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorPocketFoldStateManagerTest, HandleHallChange, TestSize.Level1)
{
    float angel = 0;
    int hall = 0;
    sptr<FoldScreenPolicy> foldScreenPolicy = nullptr;
    SingleDisplaySensorPocketFoldStateManager manager;
    manager.HandleHallChange(angel, hall, foldScreenPolicy);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: UpdateSwitchScreenBoundaryForLargeFoldDevice
 * @tc.desc: test function : UpdateSwitchScreenBoundaryForLargeFoldDevice
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorPocketFoldStateManagerTest, UpdateSwitchScreenBoundaryForLargeFoldDevice,
        TestSize.Level1)
{
    float angel = 0;
    int hall = 0;
    SingleDisplaySensorPocketFoldStateManager manager;
    manager.UpdateSwitchScreenBoundaryForLargeFoldDevice(angel, hall);
    EXPECT_TRUE(true);

    angel = 91.0F;
    hall = 1;
    manager.UpdateSwitchScreenBoundaryForLargeFoldDevice(angel, hall);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: GetNextFoldState01
 * @tc.desc: test function : GetNextFoldState
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorPocketFoldStateManagerTest, GetNextFoldState01, TestSize.Level1)
{
    float angel = -0.1;
    int hall = 0;
    SingleDisplaySensorPocketFoldStateManager manager;
    auto result1 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result1), 0);

    manager.allowUserSensorForLargeFoldDevice = 0;
    angel = 90.0F;
    hall = 1;
    auto result2 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result2), 3);

    angel = 130.0F - 0.1;
    hall = 1;
    auto result3 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result3), 3);
    
    angel = 130.0F - 0.1;
    hall = 0;
    auto result4 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result4), 3);

    angel = 130.0F + 0.1;
    hall = 0;
    auto result5 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result5), 3);

    angel = 140.0F + 0.1;
    hall = 0;
    auto result6 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result6), 3);

    angel = 140.0F + 0.1;
    hall = 1;
    auto result7 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result7), 1);
}

/**
 * @tc.name: GetNextFoldState02
 * @tc.desc: test function : GetNextFoldState
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorPocketFoldStateManagerTest, GetNextFoldState02, TestSize.Level1)
{
    SingleDisplaySensorPocketFoldStateManager manager;
    manager.allowUserSensorForLargeFoldDevice = 1;
    float angel;
    int hall = 1;
    if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        angel = 25.0F;
        auto result1 = manager.GetNextFoldState(angel, hall);
        EXPECT_EQ(static_cast<int>(result1), 0);
    }

    angel = 70.0F - 0.1;
    auto result2 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result2), 2);

    angel = 70.0F + 0.1;
    auto result3 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result3), 3);
    
    angel = 130.0F - 0.1;
    auto result4 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result4), 3);

    angel = 130.0F + 0.1;
    auto result5 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result5), 3);

    angel = 80.0F - 0.1;
    auto result6 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result6), 3);

    angel = 70.0F + 0.1;
    hall = 0;
    auto result7 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result7), 3);

    angel = 130.0F + 0.1;
    auto result8 = manager.GetNextFoldState(angel, hall);
    EXPECT_EQ(static_cast<int>(result8), 3);
}

/**
 * @tc.name: TriggerTentExit
 * @tc.desc: test function : TriggerTentExit
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorPocketFoldStateManagerTest, TriggerTentExit, TestSize.Level1)
{
    float angle = 0.0F;
    int hall;
    bool result;
    SingleDisplaySensorPocketFoldStateManager mgr;

    hall = 0;
    result = mgr.TriggerTentExit(angle, hall);
    EXPECT_EQ(result, true);

    hall = 1;
    angle = 176.0F;
    result = mgr.TriggerTentExit(angle, hall);
    EXPECT_EQ(result, true);

    hall = 1;
    angle = 4.0F;
    result = mgr.TriggerTentExit(angle, hall);
    EXPECT_EQ(result, true);

    hall = 1;
    angle = 90.0F;
    result = mgr.TriggerTentExit(angle, hall);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: HandleTentChange
 * @tc.desc: test function : HandleTentChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorPocketFoldStateManagerTest, HandleTentChange, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        GTEST_SKIP();
    }
    SingleDisplaySensorPocketFoldStateManager mgr;
    
    ASSERT_EQ(mgr.IsTentMode(), false);

    mgr.HandleTentChange(true, nullptr);
    ASSERT_EQ(mgr.IsTentMode(), true);

    mgr.HandleTentChange(true, nullptr);
    ASSERT_EQ(mgr.IsTentMode(), true);

    mgr.HandleTentChange(false, nullptr);
    ASSERT_EQ(mgr.IsTentMode(), false);

    mgr.HandleTentChange(false, nullptr);
    ASSERT_EQ(mgr.IsTentMode(), false);

    mgr.HandleTentChange(1, g_policy);
    EXPECT_EQ(OHOS::system::GetParameter("persist.dms.device.status", "0"), "3");

    mgr.HandleTentChange(0, g_policy);
    EXPECT_EQ(OHOS::system::GetParameter("persist.dms.device.status", "0"), "1");

    mgr.HandleTentChange(2, g_policy);
    EXPECT_EQ(OHOS::system::GetParameter("persist.dms.device.status", "0"), "2");

    mgr.allowUserSensorForLargeFoldDevice = 0;
    mgr.currentAngle = 140.0F + 0.1;
    mgr.HandleTentChange(0, g_policy, 1);
    EXPECT_EQ(OHOS::system::GetParameter("persist.dms.device.status", "0"), "0");
}

/**
 * @tc.name: TentModeHandleSensorChange
 * @tc.desc: test function : TentModeHandleSensorChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorPocketFoldStateManagerTest, TentModeHandleSensorChange, TestSize.Level1)
{
    SingleDisplaySensorPocketFoldStateManager stateManager;
    
    stateManager.SetTentMode(true);
    stateManager.TentModeHandleSensorChange(4.0F, 0, nullptr);
    ASSERT_EQ(stateManager.IsTentMode(), false);
}

/**
 * @tc.name: ReportTentStatusChange
 * @tc.desc: test function : ReportTentStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorPocketFoldStateManagerTest, ReportTentStatusChange, TestSize.Level1)
{
    SingleDisplaySensorPocketFoldStateManager stateManager;
    ReportTentModeStatus tentStatus = ReportTentModeStatus::NORMAL_ENTER_TENT_MODE;

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    stateManager.ReportTentStatusChange(tentStatus);
    EXPECT_TRUE(g_logMsg.find("Write HiSysEvent error") == std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}
}
} // namespace Rosen
} // namespace OHOS