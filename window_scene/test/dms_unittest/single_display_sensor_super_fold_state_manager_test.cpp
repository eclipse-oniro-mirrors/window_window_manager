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
#include <parameters.h>

#include "fold_screen_controller/fold_screen_controller_config.h"
#include "screen_session_manager/include/fold_screen_controller/sensor_fold_state_manager/single_display_sensor_super_fold_state_manager.h"
#include "fold_screen_controller/single_display_super_fold_policy.h"
#include "scene_board_judgement.h"
#include "fold_screen_state_internel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
}
class SingleDisplaySensorSuperFoldStateManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SingleDisplaySensorSuperFoldStateManagerTest::SetUpTestCase()
{
}

void SingleDisplaySensorSuperFoldStateManagerTest::TearDownTestCase()
{
    usleep(SLEEP_TIME_IN_US);
}

void SingleDisplaySensorSuperFoldStateManagerTest::SetUp()
{
}

void SingleDisplaySensorSuperFoldStateManagerTest::TearDown()
{
}

namespace {
/**
 * @tc.name: HandleAngleChange
 * @tc.desc: test function : HandleAngleChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorSuperFoldStateManagerTest, HandleAngleChange, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    float angel = 0;
    int hall = 0;
    sptr<FoldScreenPolicy> foldScreenPolicy = nullptr;
    SingleDisplaySensorSuperFoldStateManager manager;
    manager.HandleAngleChange(angel, hall, foldScreenPolicy);
    EXPECT_EQ(manager.mState_, FoldStatus::UNKNOWN);
}

/**
 * @tc.name: HandleHallChange
 * @tc.desc: test function : HandleHallChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorSuperFoldStateManagerTest, HandleHallChange, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    float angel = 0;
    int hall = 0;
    sptr<FoldScreenPolicy> foldScreenPolicy = nullptr;
    SingleDisplaySensorSuperFoldStateManager manager;
    manager.HandleHallChange(angel, hall, foldScreenPolicy);
    EXPECT_EQ(manager.mState_, FoldStatus::UNKNOWN);
}

/**
 * @tc.name: UpdateSwitchScreenBoundaryForLargeFoldDevice
 * @tc.desc: test function : UpdateSwitchScreenBoundaryForLargeFoldDevice
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorSuperFoldStateManagerTest, UpdateSwitchScreenBoundaryForLargeFoldDevice,
        TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    float angel = 0;
    int hall = 0;
    SingleDisplaySensorSuperFoldStateManager manager;
    manager.UpdateSwitchScreenBoundaryForLargeFoldDevice(angel, hall);
    EXPECT_EQ(manager.allowUserSensorForLargeFoldDevice, 0);

    angel = 91.0F;
    hall = 1;
    manager.UpdateSwitchScreenBoundaryForLargeFoldDevice(angel, hall);
    EXPECT_EQ(manager.allowUserSensorForLargeFoldDevice, 1);
}

/**
 * @tc.name: GetNextFoldState01
 * @tc.desc: test function : GetNextFoldState
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorSuperFoldStateManagerTest, GetNextFoldState01, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    float angel = -0.1;
    int hall = 0;
    SingleDisplaySensorSuperFoldStateManager manager;
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
HWTEST_F(SingleDisplaySensorSuperFoldStateManagerTest, GetNextFoldState02, TestSize.Level1)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        GTEST_SKIP() << "SceneBoard is not enabled, skipping test.";
    }
    SingleDisplaySensorSuperFoldStateManager manager;
    manager.allowUserSensorForLargeFoldDevice = 1;
    int hall = 1;
    float angel;
    if (FoldScreenStateInternel::IsSingleDisplayFoldDevice()) {
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
 * @tc.name: HandleAngleChange
 * @tc.desc: test function : HandleAngleChange02
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorSuperFoldStateManagerTest, HandleAngleChange02, TestSize.Level1)
{
    SingleDisplaySensorSuperFoldStateManager manager;
    float angle = 90.0f;
    int hall = 1;
    sptr<FoldScreenPolicy> foldScreenPolicy = nullptr;
    manager.SetTentMode(1);
    manager.HandleAngleChange(angle, hall, foldScreenPolicy);
    EXPECT_FLOAT_EQ(manager.currentAngle_, angle);
}
 
/**
 * @tc.name: HandleHallChange
 * @tc.desc: test function : HandleHallChange02
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorSuperFoldStateManagerTest, HandleHallChange02, TestSize.Level1)
{
    SingleDisplaySensorSuperFoldStateManager manager;
    float angle = 90.0f;
    int hall = 1;
    sptr<FoldScreenPolicy> foldScreenPolicy = nullptr;
    manager.SetTentMode(1);
    manager.HandleHallChange(angle, hall, foldScreenPolicy);
    EXPECT_EQ(manager.currentHall_, hall);
}
 
/**
 * @tc.name: HandleTentChange
 * @tc.desc: test function : HandleTentChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorSuperFoldStateManagerTest, HandleTentChange, TestSize.Level1)
{
    SingleDisplaySensorSuperFoldStateManager mgr;
    mgr.SetTentMode(0);
    int32_t hall = 0;
    mgr.HandleTentChange(0, nullptr, hall);
    EXPECT_EQ(mgr.tentModeType_, 0);
    mgr.HandleTentChange(1, nullptr, hall);
    EXPECT_EQ(mgr.tentModeType_, 1);
 
    std::recursive_mutex displayInfoMutex;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
    auto policy = sptr<SingleDisplaySuperFoldPolicy>::MakeSptr(displayInfoMutex, screenPowerTaskScheduler);
    mgr.SetTentMode(0);
    mgr.HandleTentChange(1, policy, hall);
    EXPECT_EQ(mgr.tentModeType_, 1);
    mgr.SetTentMode(1);
    mgr.HandleTentChange(0, policy, hall);
    EXPECT_FLOAT_EQ(mgr.currentAngle_, ANGLE_MIN_VAL);
    hall = 1;
    mgr.SetTentMode(1);
    mgr.HandleAngleChange(90.0f, hall, nullptr);
    mgr.HandleTentChange(0, policy, hall);
    EXPECT_FLOAT_EQ(mgr.currentAngle_, 90.0f);
    hall = -1;
    mgr.SetTentMode(1);
    mgr.HandleTentChange(0, policy, hall);
    EXPECT_FLOAT_EQ(mgr.currentAngle_, 90.0f);
 
    mgr.SetTentMode(1);
    mgr.HandleAngleChange(160.0f, hall, nullptr);
    mgr.HandleTentChange(0, policy, hall);
    EXPECT_FLOAT_EQ(mgr.currentAngle_, 160.0f);
}
 
/**
 * @tc.name: TriggerTentExit
 * @tc.desc: test function : TriggerTentExit
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorSuperFoldStateManagerTest, TriggerTentExit, TestSize.Level1)
{
    float angle = 0.0F;
    int hall;
    bool result;
    SingleDisplaySensorSuperFoldStateManager mgr;
 
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
 * @tc.name: TentModeHandleSensorChange
 * @tc.desc: test function : TentModeHandleSensorChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorSuperFoldStateManagerTest, TentModeHandleSensorChange, TestSize.Level1)
{
    SingleDisplaySensorSuperFoldStateManager stateManager;
    
    stateManager.SetTentMode(1);
    stateManager.TentModeHandleSensorChange(4.0F, 0, nullptr);
    ASSERT_EQ(stateManager.IsTentMode(), false);
    stateManager.SetTentMode(1);
    stateManager.TentModeHandleSensorChange(160.0F, 0, nullptr);
    ASSERT_EQ(stateManager.IsTentMode(), false);
}
 
/**
 * @tc.name: ReportTentStatusChange
 * @tc.desc: test function : ReportTentStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(SingleDisplaySensorSuperFoldStateManagerTest, ReportTentStatusChange, TestSize.Level1)
{
    SingleDisplaySensorSuperFoldStateManager stateManager;
    ReportTentModeStatus tentStatus = ReportTentModeStatus::NORMAL_ENTER_TENT_MODE;
    stateManager.ReportTentStatusChange(tentStatus);
    ASSERT_EQ(tentStatus, ReportTentModeStatus::NORMAL_ENTER_TENT_MODE);
}
}
}
}