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

#include "fold_screen_controller/fold_screen_sensor_manager.h"
#include "fold_screen_controller/fold_screen_controller.h"
#include "parameters.h"
#include "screen_session_manager/include/fold_screen_controller/sensor_fold_state_manager/dual_display_sensor_fold_state_manager.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "fold_screen_state_internel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 100000;
    const float INWARD_FOLDED_THRESHOLD = static_cast<float>(system::GetIntParameter<int32_t>
        ("const.fold.folded_threshold", 85));
    const float INWARD_EXPAND_THRESHOLD = static_cast<float>(system::GetIntParameter<int32_t>
        ("const.fold.expand_threshold", 145));
    const float INWARD_HALF_FOLDED_MAX_THRESHOLD = static_cast<float>(system::GetIntParameter<int32_t>
        ("const.half_folded_max_threshold", 135));
    const float INWARD_HALF_FOLDED_MIN_THRESHOLD = static_cast<float>(system::GetIntParameter<int32_t>
        ("const.fold.half_folded_min_threshold", 85));
    constexpr int32_t HALL_THRESHOLD = 1;
    constexpr int32_t HALL_FOLDED_THRESHOLD = 0;
    constexpr float INWARD_FOLDED_LOWER_THRESHOLD = 10.0F;
    constexpr float INWARD_FOLDED_UPPER_THRESHOLD = 20.0F;
    constexpr float ANGLE_BUFFER = 0.001F;
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_logMsg += msg;
    }
}

class DualDisplaySensorFoldStateManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void DualDisplaySensorFoldStateManagerTest::SetUpTestCase()
{
}

void DualDisplaySensorFoldStateManagerTest::TearDownTestCase()
{
}

void DualDisplaySensorFoldStateManagerTest::SetUp()
{
}

void DualDisplaySensorFoldStateManagerTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {

    /**
     * @tc.name: DualDisplaySensorFoldStateManager
     * @tc.desc: DualDisplaySensorFoldStateManager
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest, DualDisplaySensorFoldStateManager, TestSize.Level1)
    {
        if (!FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
            GTEST_SKIP();
        }
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        ASSERT_EQ(mgr.packageNames_.size(), 1);
    }

    /**
     * @tc.name: HandleAngleChange
     * @tc.desc: HandleAngleChange
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest, HandleAngleChange, TestSize.Level1)
    {
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        float angle = 0.0f;
        int hall = 0;
        sptr<FoldScreenPolicy> foldScreenPolicy = new FoldScreenPolicy();
        mgr.HandleAngleChange(angle, hall, foldScreenPolicy);
        ASSERT_TRUE(std::islessequal(angle, INWARD_FOLDED_THRESHOLD + ANGLE_BUFFER));
        ASSERT_FALSE(hall == HALL_THRESHOLD);

        hall = 1;
        mgr.HandleAngleChange(angle, hall, foldScreenPolicy);
        ASSERT_TRUE(std::islessequal(angle, INWARD_FOLDED_THRESHOLD + ANGLE_BUFFER));
        ASSERT_TRUE(hall == HALL_THRESHOLD);

        angle = 100.0f;
        mgr.HandleAngleChange(angle, hall, foldScreenPolicy);
        ASSERT_FALSE(std::islessequal(angle, INWARD_FOLDED_THRESHOLD + ANGLE_BUFFER));
        ASSERT_TRUE(hall == HALL_THRESHOLD);

        hall = 0;
        mgr.HandleAngleChange(angle, hall, foldScreenPolicy);
        ASSERT_FALSE(std::islessequal(angle, INWARD_FOLDED_THRESHOLD + ANGLE_BUFFER));
        ASSERT_FALSE(hall == HALL_THRESHOLD);
    }

    /**
     * @tc.name: HandleHallChangeInner
     * @tc.desc: HandleHallChangeInner
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest, HandleHallChangeInner, TestSize.Level1)
    {
        std::recursive_mutex mutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        float angle = 0.0f;
        int hall = 1;
        sptr<FoldScreenPolicy> foldScreenPolicy = new FoldScreenPolicy();
        mgr.applicationStateObserver_ = nullptr;
        mgr.HandleHallChangeInner(angle, hall, foldScreenPolicy);
        ASSERT_FALSE(mgr.applicationStateObserver_ != nullptr);
        ASSERT_TRUE(hall == HALL_THRESHOLD);

        mgr.applicationStateObserver_ = new ApplicationStateObserver();
        mgr.HandleHallChangeInner(angle, hall, foldScreenPolicy);
        ASSERT_TRUE(mgr.applicationStateObserver_ != nullptr);
        ASSERT_TRUE(hall == HALL_THRESHOLD);

        mgr.applicationStateObserver_ = nullptr;
        hall = 0;
        mgr.HandleHallChangeInner(angle, hall, foldScreenPolicy);
        ASSERT_FALSE(mgr.applicationStateObserver_ != nullptr);
        ASSERT_FALSE(hall == HALL_THRESHOLD);
    }

    /**
     * @tc.name: HandleHallChangeWhenHallIs1NormalHallChange
     * @tc.desc: DualDisplaySensorFoldStateManager.HandleHallChange
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest,
        HandleHallChangeWhenHallIs1NormalHallChange,
        TestSize.Level1)
    {
        // test hall = 1
        std::recursive_mutex mutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler =
            std::make_shared<TaskScheduler>("HandleHallChange");
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        int hall = 1;
        float angle = 150.0f;
        sptr<FoldScreenPolicy> foldScreenPolicy = new FoldScreenPolicy();
        ASSERT_NO_FATAL_FAILURE({mgr.HandleHallChange(angle, hall, foldScreenPolicy);});
    }

    /**
     * @tc.name: HandleHallChangeWhenAngleLess170
     * @tc.desc: DualDisplaySensorFoldStateManager.HandleHallChange
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest,
        HandleHallChangeWhenAngleLess170,
        TestSize.Level1)
    {
        // test angle < 170
        std::recursive_mutex mutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler =
            std::make_shared<TaskScheduler>("HandleHallChange");
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        int hall = 1;
        float angle = 150.0f;
        sptr<FoldScreenPolicy> foldScreenPolicy = new FoldScreenPolicy();
        ASSERT_NO_FATAL_FAILURE({mgr.HandleHallChange(angle, hall, foldScreenPolicy);});
    }

    /**
     * @tc.name: HandleHallChangeWhenHallIs0AndAngleMore170HallChanged
     * @tc.desc: DualDisplaySensorFoldStateManager.HandleHallChange
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest,
        HandleHallChangeWhenHallIs0AndAngleMore170HallChanged,
        TestSize.Level1)
    {
        // test hall = 0 angle > 170 and hall change to 1
        std::recursive_mutex mutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler =
            std::make_shared<TaskScheduler>("HandleHallChange");
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        float angle = 178.0f;
        int hall = 0;
        FoldScreenSensorManager::GetInstance().SetGlobalHall(1);
        sptr<FoldScreenPolicy> foldScreenPolicy = new FoldScreenPolicy();
        ASSERT_NO_FATAL_FAILURE({mgr.HandleHallChange(angle, hall, foldScreenPolicy);});
        usleep(500000);
    }

    /**
     * @tc.name: HandleHallChangeWhenHallIs0AndAngleMore170HallNotChanged
     * @tc.desc: DualDisplaySensorFoldStateManager.HandleHallChange
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest,
        HandleHallChangeWhenHallIs0AndAngleMore170HallNotChanged,
        TestSize.Level1)
    {
        // test hall = 0 angle > 170 and angle not change
        std::recursive_mutex mutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler =
            std::make_shared<TaskScheduler>("HandleHallChange");
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        float angle = 178.0f;
        int hall = 0;
        FoldScreenSensorManager::GetInstance().SetGlobalHall(0);
        FoldScreenSensorManager::GetInstance().SetGlobalAngle(angle);
        sptr<FoldScreenPolicy> foldScreenPolicy = new FoldScreenPolicy();
        ASSERT_NO_FATAL_FAILURE({mgr.HandleHallChange(angle, hall, foldScreenPolicy);});
        usleep(500000);
    }

    /**
     * @tc.name: HandleHallChangeWhenHallIs0AndAngleMore170AngleChangedToLess170
     * @tc.desc: DualDisplaySensorFoldStateManager.HandleHallChange
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest,
        HandleHallChangeWhenHallIs0AndAngleMore170AngleChangedToLess170,
        TestSize.Level1)
    {
        // test hall = 0 angle < 170 and angle change less 170
        std::recursive_mutex mutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler =
            std::make_shared<TaskScheduler>("HandleHallChange");
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        float angle = 178.0f;
        int hall = 0;
        FoldScreenSensorManager::GetInstance().SetGlobalHall(0);
        FoldScreenSensorManager::GetInstance().SetGlobalAngle(150.0f);
        sptr<FoldScreenPolicy> foldScreenPolicy = new FoldScreenPolicy();
        ASSERT_NO_FATAL_FAILURE({mgr.HandleHallChange(angle, hall, foldScreenPolicy);});
        usleep(500000);
    }

    /**
     * @tc.name: HandleHallChangeWhenHallIs0AndAngleMore170AngleChangedToMore170
     * @tc.desc: DualDisplaySensorFoldStateManager.HandleHallChange
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest,
        HandleHallChangeWhenHallIs0AndAngleMore170AngleChangedToMore170,
        TestSize.Level1)
    {
        // test hall = 0 angle > 170 and angle change more than 170
        std::recursive_mutex mutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler =
            std::make_shared<TaskScheduler>("HandleHallChange");
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        float angle = 178.0f;
        int hall = 0;
        FoldScreenSensorManager::GetInstance().SetGlobalHall(0);
        FoldScreenSensorManager::GetInstance().SetGlobalAngle(175.0f);
        sptr<FoldScreenPolicy> foldScreenPolicy = new FoldScreenPolicy();
        ASSERT_NO_FATAL_FAILURE({mgr.HandleHallChange(angle, hall, foldScreenPolicy);});
        usleep(500000);
    }

    /**
     * @tc.name: HandleHallChangeWhenHallIs0AndAngleMore170ChangedToLess170InWaitting
     * @tc.desc: DualDisplaySensorFoldStateManager.HandleHallChange
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest,
        HandleHallChangeWhenHallIs0AndAngleMore170ChangedToLess170InWaitting,
        TestSize.Level1)
    {
        // test hall = 0 angle > 170 and angle change more than 170, report angle when in task
        std::recursive_mutex mutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler =
            std::make_shared<TaskScheduler>("HandleHallChange");
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        float angle = 178.0f;
        int hall = 0;
        sptr<FoldScreenPolicy> foldScreenPolicy = new FoldScreenPolicy();
        mgr.HandleHallChange(angle, hall, foldScreenPolicy);
        ASSERT_NO_FATAL_FAILURE({mgr.HandleHallChange(150.0f, hall, foldScreenPolicy);});
    }

    /**
     * @tc.name: CheckUpdateAngleShouldBeFalseWhenHallIsFold1AndAngleGreaterThan170
     * @tc.desc: DualDisplaySensorFoldStateManager.CheckUpdateAngle
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest,
        CheckUpdateAngleShouldBeFalseWhenHallIsFold1AndAngleGreaterThan170,
        TestSize.Level1)
    {
        // test hall = 0 angle > 170 and angle change more than 170, report angle when in task
        std::recursive_mutex mutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler =
            std::make_shared<TaskScheduler>("HandleHallChange");
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        float angle = 188.0f;
        int hall = HALL_FOLDED_THRESHOLD;
        ASSERT_NO_FATAL_FAILURE({mgr.CheckUpdateAngle(angle, hall);});
        ASSERT_FALSE(mgr.CheckUpdateAngle(angle, hall));
    }

    /**
     * @tc.name: HandleAngleChangeInTaskDoNothingWhenAngleLessThan0AndHallIs1
     * @tc.desc: DualDisplaySensorFoldStateManager.HandleAngleChangeInTask
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest,
        HandleAngleChangeInTaskDoNothingWhenAngleLessThan0AndHallIs1,
        TestSize.Level1)
    {
        // test hall = 0 angle > 170 and angle change more than 170, report angle when in task
        std::recursive_mutex mutex;
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler =
            std::make_shared<TaskScheduler>("HandleHallChange");
        sptr<FoldScreenPolicy> foldScreenPolicy = new FoldScreenPolicy();
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        float angle = 10.0f;
        int hall = HALL_THRESHOLD;
        ASSERT_NO_FATAL_FAILURE({mgr.HandleAngleChangeInTask(angle, hall, foldScreenPolicy);});
    }

    /**
     * @tc.name: GetNextFoldState
     * @tc.desc: GetNextFoldState
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest, GetNextFoldState, TestSize.Level1)
    {
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        float angle = 200.0f;
        int hall = 1;
        FoldStatus ret = mgr.GetNextFoldState(angle, hall);
        ASSERT_TRUE(std::isgreaterequal(angle, INWARD_EXPAND_THRESHOLD + ANGLE_BUFFER));
        ASSERT_EQ(ret, FoldStatus::EXPAND);

        angle = 0.0f;
        ret = mgr.GetNextFoldState(angle, hall);
        ASSERT_TRUE(std::islessequal
        (angle, INWARD_FOLDED_LOWER_THRESHOLD + ANGLE_BUFFER));
        ASSERT_EQ(ret, FoldStatus::FOLDED);

        mgr.isHallSwitchApp_ = true;
        angle = 30.0f;
        ret = mgr.GetNextFoldState(angle, hall);
        ASSERT_TRUE(std::isgreaterequal(angle, INWARD_FOLDED_UPPER_THRESHOLD + ANGLE_BUFFER));
        ASSERT_TRUE(std::islessequal(angle, INWARD_HALF_FOLDED_MAX_THRESHOLD + ANGLE_BUFFER));
        ASSERT_EQ(ret, FoldStatus::HALF_FOLD);

        mgr.isHallSwitchApp_ = false;
        angle = 120.0f;
        ret = mgr.GetNextFoldState(angle, hall);
        ASSERT_TRUE(std::isgreaterequal(angle, INWARD_HALF_FOLDED_MIN_THRESHOLD + ANGLE_BUFFER));
        ASSERT_TRUE(std::islessequal(angle, INWARD_HALF_FOLDED_MAX_THRESHOLD + ANGLE_BUFFER));
        ASSERT_EQ(ret, FoldStatus::HALF_FOLD);
    }

    /**
     * @tc.name: RegisterApplicationStateObserver
     * @tc.desc: RegisterApplicationStateObserver
     * @tc.type: FUNC
     */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest, RegisterApplicationStateObserver, TestSize.Level1)
    {
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler = nullptr;
        DualDisplaySensorFoldStateManager mgr = DualDisplaySensorFoldStateManager(screenPowerTaskScheduler);
        mgr.RegisterApplicationStateObserver();
        ASSERT_NE(mgr.applicationStateObserver_, nullptr);
    }
    
    /**
    * @tc.name: ReportDualTentModeStatus
    * @tc.desc: test function : ReportDualTentModeStatus
    * @tc.type: FUNC
    */
    HWTEST_F(DualDisplaySensorFoldStateManagerTest, ReportDualTentModeStatus, TestSize.Level1)
    {
        g_logMsg.clear();
        LOG_SetCallback(MyLogCallback);
        std::shared_ptr<TaskScheduler> scheduler = nullptr;
        DualDisplaySensorFoldStateManager stateManager(scheduler);
        struct TestCase {
            ReportDualTentModeStatus status;
            int expectedValue;
        };
        std::vector<TestCase> cases = { { ReportDualTentModeStatus::NORMAL_EXIT_TENT_MODE, 0 },
                                        { ReportDualTentModeStatus::NORMAL_ENTER_TENT_MODE, 1 },
                                        { ReportDualTentModeStatus::ABNORMAL_EXIT_TENT_MODE_DUE_TO_ANGLE, 2 },
                                        { ReportDualTentModeStatus::ABNORMAL_EXIT_TENT_MODE_DUE_TO_HALL, 3 } };
        for (const auto& tc : cases) {
            g_logMsg.clear();
            stateManager.ReportTentStatusChange(tc.status);
            std::string expected = "report tentStatus: " + std::to_string(tc.expectedValue);
            EXPECT_TRUE(g_logMsg.find(expected) != std::string::npos)
                << "Failed for status " << static_cast<int>(tc.status) << "; expected: " << expected
                << ", got: " << g_logMsg;
        }
        LOG_SetCallback(nullptr);
    }
}
} // namespace Rosen
} // namespace OHOS