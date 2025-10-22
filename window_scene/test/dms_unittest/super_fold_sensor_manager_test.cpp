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

#include "fold_screen_controller/super_fold_sensor_manager.h"
#include "fold_screen_state_internel.h"
#include "window_manager_hilog.h"
#include "screen_session_manager.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
constexpr uint16_t VALID_HALL_STATUS = 0;
constexpr uint16_t HALL_HAVE_KEYBOARD_THRESHOLD = 0B0100;
}

class SuperFoldSensorManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SuperFoldSensorManagerTest::SetUpTestCase()
{
}

void SuperFoldSensorManagerTest::TearDownTestCase()
{
}

void SuperFoldSensorManagerTest::SetUp()
{
}

void SuperFoldSensorManagerTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {
/**
 * @tc.name: UnregisterPostureCallback01
 * @tc.desc: test function : UnregisterPostureCallback
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, UnregisterPostureCallback01, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.UnregisterPostureCallback();
    ASSERT_EQ(mgr.postureUser.callback, nullptr);
}

/**
 * @tc.name: UnregisterPostureCallback02
 * @tc.desc: test function : UnregisterPostureCallback
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, UnregisterPostureCallback02, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.UnregisterPostureCallback();
    void (*func)(SensorEvent *event) = nullptr;
    ASSERT_EQ(mgr.postureUser.callback, func);
}

/**
 * @tc.name: UnregisterHallCallback01
 * @tc.desc: test function : UnregisterHallCallback
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, UnregisterHallCallback01, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.UnregisterHallCallback();
    ASSERT_EQ(mgr.hallUser.callback, nullptr);
}

/**
 * @tc.name: UnregisterHallCallback02
 * @tc.desc: test function : UnregisterHallCallback
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, UnregisterHallCallback02, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.UnregisterHallCallback();
    void (*func)(SensorEvent *event) = nullptr;
    ASSERT_EQ(mgr.hallUser.callback, func);
}

/**
 * @tc.name: HandlePostureData01
 * @tc.desc: test function : HandlePostureData
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandlePostureData01, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandlePostureData(nullptr);
    ASSERT_EQ(mgr.curAngle_, 170.0F);
}

/**
 * @tc.name: HandlePostureData02
 * @tc.desc: test function : HandlePostureData
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandlePostureData02, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandlePostureData(nullptr);
    EXPECT_EQ(mgr.curAngle_, 170.0F);

    SensorEvent event;
    PostureData postureData;
    postureData.angle = 45.0F;
    mgr.HandlePostureData(&event);
    ASSERT_EQ(mgr.curAngle_, 170.0F);
}

/**
 * @tc.name: HandlePostureData03
 * @tc.desc: test function : HandlePostureData
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandlePostureData03, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandlePostureData(nullptr);
    ASSERT_EQ(mgr.curAngle_, 170.0F);

    SensorEvent event;
    PostureData postureData;
    postureData.angle = 45.0F;
    mgr.HandlePostureData(&event);
    EXPECT_EQ(mgr.curAngle_, 170.0F);

    event.data = reinterpret_cast<uint8_t*>(&postureData);
    event.dataLen = 0;
    mgr.HandlePostureData(&event);
    ASSERT_EQ(mgr.curAngle_, 170.0F);
}

/**
 * @tc.name: HandlePostureData04
 * @tc.desc: test function : HandlePostureData
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandlePostureData04, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandlePostureData(nullptr);
    ASSERT_EQ(mgr.curAngle_, 170.0F);

    SensorEvent event;
    PostureData postureData;
    postureData.angle = 45.0F;
    mgr.HandlePostureData(&event);
    EXPECT_EQ(mgr.curAngle_, 170.0F);

    event.data = reinterpret_cast<uint8_t*>(&postureData);
    event.dataLen = 0;
    mgr.HandlePostureData(&event);
    EXPECT_EQ(mgr.curAngle_, 170.0F);

    event.dataLen = sizeof(PostureData);
    mgr.HandlePostureData(&event);
    ASSERT_EQ(mgr.curAngle_, 45.0F);
}

/**
 * @tc.name: HandlePostureData05
 * @tc.desc: test function: HandlePostureData when taskScheduler is valid.
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandlePostureData05, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    auto scheduler = std::make_shared<TaskScheduler>("task_test");

    mgr.SetTaskScheduler(scheduler);
    SensorEvent event;
    PostureData postureData;
    postureData.angle = 45.0F;
    event.data = reinterpret_cast<uint8_t*>(&postureData);
    event.dataLen = sizeof(PostureData);
    mgr.HandlePostureData(&event);
    EXPECT_EQ(mgr.curAngle_, 45.0F);
}

/**
 * @tc.name: HandlePostureData06
 * @tc.desc: test function: HandlePostureData when taskScheduler is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandlePostureData06, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();

    mgr.SetTaskScheduler(nullptr);
    SensorEvent event;
    PostureData postureData;
    postureData.angle = 45.0F;
    event.data = reinterpret_cast<uint8_t*>(&postureData);
    event.dataLen = sizeof(PostureData);
    mgr.HandlePostureData(&event);
    EXPECT_EQ(mgr.curAngle_, 45.0F);
}


/**
 * @tc.name: NotifyFoldAngleChanged01
 * @tc.desc: test function : NotifyFoldAngleChanged
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, NotifyFoldAngleChanged01, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    float foldAngle = 0.0F;
    mgr.NotifyFoldAngleChanged(foldAngle);
    EXPECT_TRUE(ScreenSessionManager::GetInstance().lastFoldAngles_.empty());
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: NotifyFoldAngleChanged02
 * @tc.desc: test function : NotifyFoldAngleChanged
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, NotifyFoldAngleChanged02, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    float foldAngle = 30.0F;
    mgr.NotifyFoldAngleChanged(foldAngle);
    EXPECT_TRUE(ScreenSessionManager::GetInstance().lastFoldAngles_.empty());
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: NotifyFoldAngleChanged03
 * @tc.desc: test function : NotifyFoldAngleChanged
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, NotifyFoldAngleChanged03, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    float foldAngle = 165.0F;
    mgr.NotifyFoldAngleChanged(foldAngle);
    EXPECT_TRUE(ScreenSessionManager::GetInstance().lastFoldAngles_.empty());
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: NotifyFoldAngleChanged04
 * @tc.desc: test function : NotifyFoldAngleChanged
 * @tc.type: FUNC
 */
 HWTEST_F(SuperFoldSensorManagerTest, NotifyFoldAngleChanged04, Function | SmallTest | Level3)
 {
     SuperFoldSensorManager mgr = SuperFoldSensorManager();
     float foldAngle = 90.0F;
     mgr.NotifyFoldAngleChanged(foldAngle);
     EXPECT_TRUE(ScreenSessionManager::GetInstance().lastFoldAngles_.empty());
     usleep(SLEEP_TIME_US);
 }

 /**
 * @tc.name: NotifyFoldAngleChanged05
 * @tc.desc: test function : NotifyFoldAngleChanged
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, NotifyFoldAngleChanged05, Function | SmallTest | Level3)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    float foldAngle = 135.0F;
    mgr.NotifyFoldAngleChanged(foldAngle);
    EXPECT_TRUE(ScreenSessionManager::GetInstance().lastFoldAngles_.empty());
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: HandleHallData01
 * @tc.desc: test function : HandleHallData
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandleHallData01, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandleHallData(nullptr);
    ASSERT_EQ(mgr.curHall_, USHRT_MAX);
}

/**
 * @tc.name: HandleHallData02
 * @tc.desc: test function : HandleHallData
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandleHallData02, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandleHallData(nullptr);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    SensorEvent event;
    mgr.HandleHallData(&event);
    ASSERT_EQ(mgr.curHall_, USHRT_MAX);
}

/**
 * @tc.name: HandleHallData03
 * @tc.desc: test function : HandleHallData
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandleHallData03, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandleHallData(nullptr);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    SensorEvent event;
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    HallData hallData;
    event.data = reinterpret_cast<uint8_t*>(&hallData);
    mgr.HandleHallData(&event);
    ASSERT_EQ(mgr.curHall_, USHRT_MAX);
}

/**
 * @tc.name: HandleHallData04
 * @tc.desc: test function : HandleHallData
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandleHallData04, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandleHallData(nullptr);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    SensorEvent event;
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    HallData hallData;
    event.data = reinterpret_cast<uint8_t*>(&hallData);
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    event.dataLen = 0;
    mgr.HandleHallData(&event);
    ASSERT_EQ(mgr.curHall_, USHRT_MAX);
}

/**
 * @tc.name: HandleHallData05
 * @tc.desc: test function : HandleHallData
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandleHallData05, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandleHallData(nullptr);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    SensorEvent event;
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    HallData hallData;
    event.data = reinterpret_cast<uint8_t*>(&hallData);
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    event.dataLen = 0;
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    event.dataLen = sizeof(HallData);
    mgr.HandleHallData(&event);
    ASSERT_EQ(mgr.curHall_, VALID_HALL_STATUS);
}

/**
 * @tc.name: HandleHallData06
 * @tc.desc: test function : HandleHallData
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandleHallData06, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandleHallData(nullptr);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    SensorEvent event;
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    HallData hallData;
    event.data = reinterpret_cast<uint8_t*>(&hallData);
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    event.dataLen = 0;
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.curHall_, USHRT_MAX);

    event.dataLen = sizeof(HallData);
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.curHall_, VALID_HALL_STATUS);

    hallData.status = 132;
    mgr.HandleHallData(&event);
    ASSERT_EQ(mgr.curHall_, HALL_HAVE_KEYBOARD_THRESHOLD);
}

/**
 * @tc.name: HandleHallData07
 * @tc.desc: test function : HandleHallData when taskScheduler is valid.
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandleHallData07, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    SensorEvent event;
    HallData hallData;
    event.data = reinterpret_cast<uint8_t*>(&hallData);
    event.dataLen = sizeof(HallData);

    auto scheduler = std::make_shared<TaskScheduler>("task_test");
    mgr.SetTaskScheduler(scheduler);
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.curHall_, VALID_HALL_STATUS);
}

/**
 * @tc.name: HandleHallData08
 * @tc.desc: test function : HandleHallData when taskScheduler is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandleHallData08, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    SensorEvent event;
    HallData hallData;
    event.data = reinterpret_cast<uint8_t*>(&hallData);
    event.dataLen = sizeof(HallData);

    mgr.SetTaskScheduler(nullptr);
    mgr.HandleHallData(&event);
    EXPECT_EQ(mgr.curHall_, VALID_HALL_STATUS);
}


/**
 * @tc.name: NotifyHallChanged
 * @tc.desc: test function : NotifyHallChanged
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, NotifyHallChanged, Function | SmallTest | Level3)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.NotifyHallChanged(0B0000);
    ASSERT_EQ(mgr.curHall_, USHRT_MAX);
    mgr.NotifyHallChanged(0B0100);
    ASSERT_EQ(mgr.curHall_, USHRT_MAX);
    mgr.NotifyHallChanged(0B0101);
    ASSERT_EQ(mgr.curHall_, USHRT_MAX);
    mgr.curAngle_ = 165.0F;
    mgr.NotifyHallChanged(0B0000);
    ASSERT_EQ(mgr.curHall_, USHRT_MAX);
}

/**
 * @tc.name: HandleSuperSensorChange
 * @tc.desc: test function : HandleSuperSensorChange
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandleSuperSensorChange, Function | SmallTest | Level3)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    ScreenSessionManager::GetInstance().isExtendScreenConnected_ = true;
    mgr.HandleSuperSensorChange(SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED);
    ASSERT_EQ(mgr.curHall_, USHRT_MAX);
    ScreenSessionManager::GetInstance().isExtendScreenConnected_ = false;
    mgr.HandleSuperSensorChange(SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED);
    ASSERT_EQ(mgr.curHall_, USHRT_MAX);
}

/**
 * @tc.name: HandleScreenConnectChange
 * @tc.desc: test function : HandleScreenConnectChange
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandleScreenConnectChange, Function | SmallTest | Level3)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandleScreenConnectChange();
    ASSERT_EQ(mgr.curHall_, USHRT_MAX);
}

/**
 * @tc.name: HandleScreenDisconnectChange
 * @tc.desc: test function : HandleScreenDisconnectChange
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandleScreenDisconnectChange, Function | SmallTest | Level3)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandleScreenDisconnectChange();
    ASSERT_EQ(mgr.curHall_, USHRT_MAX);
}

/**
 * @tc.name: GetCurAngle
 * @tc.desc: test function : GetCurAngle
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, GetCurAngle, Function | SmallTest | Level3)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.curAngle_ = 110.0F;
    float angle = mgr.GetCurAngle();
    ASSERT_EQ(angle, 110.0F);
}

/**
 * @tc.name: HandleFoldStatusLockedToExpand
 * @tc.desc: HandleFoldStatusLockedToExpand
 * @tc.type: FUNC
 */
HWTEST_F(SuperFoldSensorManagerTest, HandleFoldStatusLockedToExpand, TestSize.Level1)
{
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        GTEST_SKIP();
    }
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    mgr.HandleFoldStatusLockedToExpand();
    EXPECT_NE(SuperFoldStateManager::GetInstance().curState_, SuperFoldStatus::UNKNOWN);

    mgr.HandleFoldStatusLockedToExpand();
    EXPECT_NE(SuperFoldStateManager::GetInstance().curState_, SuperFoldStatus::UNKNOWN);
}

/**
 * @tc.name: SetTaskScheduler01
 * @tc.number: SetTaskSchedulerTest_001
 * @tc.desc: taskScheduler_ could be properly set when the input is valid.
 */
HWTEST_F(SuperFoldSensorManagerTest, SetTaskScheduler01, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();
    auto scheduler = std::make_shared<TaskScheduler>("task_test");

    mgr.SetTaskScheduler(scheduler);
    EXPECT_EQ(mgr.taskScheduler_.get(), scheduler.get());
}

/**
 * @tc.name: SetTaskScheduler02
 * @tc.number: SetTaskSchedulerTest_002
 * @tc.desc: taskScheduler_ could be properly set when the input is nullptr.
 */
HWTEST_F(SuperFoldSensorManagerTest, SetTaskScheduler02, TestSize.Level1)
{
    SuperFoldSensorManager mgr = SuperFoldSensorManager();

    mgr.SetTaskScheduler(nullptr);
    EXPECT_EQ(mgr.taskScheduler_, nullptr);
}
}
} // namespace Rosen
} // namespace OHOS