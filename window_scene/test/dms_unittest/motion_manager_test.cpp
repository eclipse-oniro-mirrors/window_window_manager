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

#include <chrono>
#include <gtest/gtest.h>
#include "window_manager_hilog.h"
#include "motion_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
}

class MockMotionEventListener : public IMotionEventListener {
public:
    MockMotionEventListener() : lastRotation_(-1.0f), callCount_(0) {}
    ~MockMotionEventListener() override = default;
    
    void OnMotionRotationChanged(float sensorRotation) override {
        lastRotation_ = sensorRotation;
        callCount_++;
    }
    
    float GetLastRotation() const { return lastRotation_; }
    int GetCallCount() const { return callCount_; }
    void Reset() { lastRotation_ = -1.0f; callCount_ = 0; }
    
private:
    float lastRotation_;
    int callCount_;
};

class MotionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MotionManagerTest::SetUpTestCase()
{
}

void MotionManagerTest::TearDownTestCase()
{
}

void MotionManagerTest::SetUp()
{
    MotionManager::GetInstance().Reset();
}

void MotionManagerTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {

/**
 * @tc.name: GetInstance01
 * @tc.desc: test function : GetInstance
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, GetInstance01, TestSize.Level1)
{
    auto& instance1 = MotionManager::GetInstance();
    auto& instance2 = MotionManager::GetInstance();
    ASSERT_EQ(&instance1, &instance2);
}

/**
 * @tc.name: Init01
 * @tc.desc: test function : Init
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, Init01, TestSize.Level1)
{
    MotionManager::GetInstance().Init();
    ASSERT_TRUE(MotionManager::GetInstance().IsInitialized());
}

/**
 * @tc.name: Init02
 * @tc.desc: test function : Init twice
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, Init02, TestSize.Level1)
{
    MotionManager::GetInstance().Init();
    ASSERT_TRUE(MotionManager::GetInstance().IsInitialized());
    MotionManager::GetInstance().Init();
    ASSERT_TRUE(MotionManager::GetInstance().IsInitialized());
}

/**
 * @tc.name: SetMotionEventListener01
 * @tc.desc: test function : SetMotionEventListener
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, SetMotionEventListener01, TestSize.Level1)
{
    MockMotionEventListener listener;
    MotionManager::GetInstance().SetMotionEventListener(&listener);
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::MOTION_TYPE_ROTATION, 90.0f);
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: SetMotionEventListener02
 * @tc.desc: test function : SetMotionEventListener with null
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, SetMotionEventListener02, TestSize.Level1)
{
    MotionManager::GetInstance().SetMotionEventListener(nullptr);
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::MOTION_TYPE_ROTATION, 90.0f);
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: SubscribeMotionSensor01
 * @tc.desc: test function : SubscribeMotionSensor
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, SubscribeMotionSensor01, TestSize.Level1)
{
    MotionManager::GetInstance().SetScreenOnState(true);
    MotionManager::GetInstance().SubscribeMotionSensor(MotionType::MOTION_TYPE_ROTATION);
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: SubscribeMotionSensor02
 * @tc.desc: test function : SubscribeMotionSensor SMART_MOTION_TYPE_ROTATION
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, SubscribeMotionSensor02, TestSize.Level1)
{
    MotionManager::GetInstance().SetScreenOnState(true);
    MotionManager::GetInstance().SubscribeMotionSensor(MotionType::SMART_MOTION_TYPE_ROTATION);
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: SubscribeMotionSensor03
 * @tc.desc: test function : SubscribeMotionSensor screen off
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, SubscribeMotionSensor03, TestSize.Level1)
{
    MotionManager::GetInstance().SetScreenOnState(false);
    MotionManager::GetInstance().SubscribeMotionSensor(MotionType::MOTION_TYPE_ROTATION);
    ASSERT_FALSE(MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::MOTION_TYPE_ROTATION));
}

/**
 * @tc.name: UnsubscribeMotionSensor01
 * @tc.desc: test function : UnsubscribeMotionSensor
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, UnsubscribeMotionSensor01, TestSize.Level1)
{
    MotionManager::GetInstance().SetScreenOnState(true);
    MotionManager::GetInstance().SubscribeMotionSensor(MotionType::MOTION_TYPE_ROTATION);
    MotionManager::GetInstance().UnsubscribeMotionSensor(MotionType::MOTION_TYPE_ROTATION);
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: UnsubscribeMotionSensor02
 * @tc.desc: test function : UnsubscribeMotionSensor not subscribed
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, UnsubscribeMotionSensor02, TestSize.Level1)
{
    MotionManager::GetInstance().UnsubscribeMotionSensor(MotionType::MOTION_TYPE_ROTATION);
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: OnScreenOn01
 * @tc.desc: test function : OnScreenOn
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, OnScreenOn01, TestSize.Level1)
{
    MotionManager::GetInstance().OnScreenOn();
    ASSERT_TRUE(MotionManager::GetInstance().IsScreenOn());
}

/**
 * @tc.name: OnScreenOff01
 * @tc.desc: test function : OnScreenOff
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, OnScreenOff01, TestSize.Level1)
{
    MotionManager::GetInstance().OnScreenOff();
    ASSERT_FALSE(MotionManager::GetInstance().IsScreenOn());
}

/**
 * @tc.name: GetLastMotionRotation01
 * @tc.desc: test function : GetLastMotionRotation
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, GetLastMotionRotation01, TestSize.Level1)
{
    float rotation = MotionManager::GetInstance().GetLastMotionRotation();
    ASSERT_EQ(rotation, -1.0f);
}

/**
 * @tc.name: GetLastSmartMotionRotation01
 * @tc.desc: test function : GetLastSmartMotionRotation
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, GetLastSmartMotionRotation01, TestSize.Level1)
{
    float rotation = MotionManager::GetInstance().GetLastSmartMotionRotation();
    ASSERT_EQ(rotation, -1.0f);
}

/**
 * @tc.name: HandleMotionEvent01
 * @tc.desc: test function : HandleMotionEvent MOTION_TYPE_ROTATION
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, HandleMotionEvent01, TestSize.Level1)
{
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::MOTION_TYPE_ROTATION, 90.0f);
    float rotation = MotionManager::GetInstance().GetLastMotionRotation();
    ASSERT_EQ(rotation, 90.0f);
}

/**
 * @tc.name: HandleMotionEvent02
 * @tc.desc: test function : HandleMotionEvent SMART_MOTION_TYPE_ROTATION
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, HandleMotionEvent02, TestSize.Level1)
{
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::SMART_MOTION_TYPE_ROTATION, 180.0f);
    float rotation = MotionManager::GetInstance().GetLastSmartMotionRotation();
    ASSERT_EQ(rotation, 180.0f);
}

/**
 * @tc.name: CalculateAverageRotation01
 * @tc.desc: test function : CalculateAverageRotation valid values
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, CalculateAverageRotation01, TestSize.Level1)
{
    float avg = MotionManager::CalculateAverageRotation(90.0f, 180.0f);
    ASSERT_EQ(avg, 135.0f);
}

/**
 * @tc.name: CalculateAverageRotation02
 * @tc.desc: test function : CalculateAverageRotation invalid values
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, CalculateAverageRotation02, TestSize.Level1)
{
    float avg = MotionManager::CalculateAverageRotation(-1.0f, 90.0f);
    ASSERT_EQ(avg, -1.0f);
}

/**
 * @tc.name: CalculateAverageRotation03
 * @tc.desc: test function : CalculateAverageRotation both invalid
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, CalculateAverageRotation03, TestSize.Level1)
{
    float avg = MotionManager::CalculateAverageRotation(-1.0f, -1.0f);
    ASSERT_EQ(avg, -1.0f);
}

/**
 * @tc.name: CalculateAverageRotation04
 * @tc.desc: test function : CalculateAverageRotation same values
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, CalculateAverageRotation04, TestSize.Level1)
{
    float avg = MotionManager::CalculateAverageRotation(90.0f, 90.0f);
    ASSERT_EQ(avg, 90.0f);
}

/**
 * @tc.name: ConvertMotionActionToDeviceRotation01
 * @tc.desc: test function : ConvertMotionActionToDeviceRotation PORTRAIT
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertMotionActionToDeviceRotation01, TestSize.Level1)
{
    DeviceRotation rotation = MotionManager::ConvertMotionActionToDeviceRotation(
        MotionAction::MOTION_ACTION_PORTRAIT);
    ASSERT_EQ(rotation, DeviceRotation::ROTATION_PORTRAIT);
}

/**
 * @tc.name: ConvertMotionActionToDeviceRotation02
 * @tc.desc: test function : ConvertMotionActionToDeviceRotation LEFT_LANDSCAPE
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertMotionActionToDeviceRotation02, TestSize.Level1)
{
    DeviceRotation rotation = MotionManager::ConvertMotionActionToDeviceRotation(
        MotionAction::MOTION_ACTION_LEFT_LANDSCAPE);
    ASSERT_EQ(rotation, DeviceRotation::ROTATION_LANDSCAPE_INVERTED);
}

/**
 * @tc.name: ConvertMotionActionToDeviceRotation03
 * @tc.desc: test function : ConvertMotionActionToDeviceRotation PORTRAIT_INVERTED
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertMotionActionToDeviceRotation03, TestSize.Level1)
{
    DeviceRotation rotation = MotionManager::ConvertMotionActionToDeviceRotation(
        MotionAction::MOTION_ACTION_PORTRAIT_INVERTED);
    ASSERT_EQ(rotation, DeviceRotation::ROTATION_PORTRAIT_INVERTED);
}

/**
 * @tc.name: ConvertMotionActionToDeviceRotation04
 * @tc.desc: test function : ConvertMotionActionToDeviceRotation RIGHT_LANDSCAPE
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertMotionActionToDeviceRotation04, TestSize.Level1)
{
    DeviceRotation rotation = MotionManager::ConvertMotionActionToDeviceRotation(
        MotionAction::MOTION_ACTION_RIGHT_LANDSCAPE);
    ASSERT_EQ(rotation, DeviceRotation::ROTATION_LANDSCAPE);
}

/**
 * @tc.name: ConvertMotionActionToDeviceRotation05
 * @tc.desc: test function : ConvertMotionActionToDeviceRotation invalid
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertMotionActionToDeviceRotation05, TestSize.Level1)
{
    DeviceRotation rotation = MotionManager::ConvertMotionActionToDeviceRotation(999);
    ASSERT_EQ(rotation, DeviceRotation::INVALID);
}

/**
 * @tc.name: IsMotionSensorSubscribed01
 * @tc.desc: test function : IsMotionSensorSubscribed not subscribed
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, IsMotionSensorSubscribed01, TestSize.Level1)
{
    bool subscribed = MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::MOTION_TYPE_ROTATION);
    ASSERT_FALSE(subscribed);
}

/**
 * @tc.name: IsScreenOn01
 * @tc.desc: test function : IsScreenOn
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, IsScreenOn01, TestSize.Level1)
{
    MotionManager::GetInstance().SetScreenOnState(true);
    ASSERT_TRUE(MotionManager::GetInstance().IsScreenOn());
}

/**
 * @tc.name: IsScreenOn02
 * @tc.desc: test function : IsScreenOn false
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, IsScreenOn02, TestSize.Level1)
{
    MotionManager::GetInstance().SetScreenOnState(false);
    ASSERT_FALSE(MotionManager::GetInstance().IsScreenOn());
}

/**
 * @tc.name: IsInitialized01
 * @tc.desc: test function : IsInitialized
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, IsInitialized01, TestSize.Level1)
{
    MotionManager::GetInstance().Reset();
    ASSERT_FALSE(MotionManager::GetInstance().IsInitialized());
}

/**
 * @tc.name: IsDefaultSmartMotionEnabled01
 * @tc.desc: test function : IsDefaultSmartMotionEnabled
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, IsDefaultSmartMotionEnabled01, TestSize.Level1)
{
    MotionManager::GetInstance().SetDefaultSmartMotionEnabled(false);
    ASSERT_FALSE(MotionManager::GetInstance().IsDefaultSmartMotionEnabled());
}

/**
 * @tc.name: IsDefaultSmartMotionEnabled02
 * @tc.desc: test function : IsDefaultSmartMotionEnabled true
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, IsDefaultSmartMotionEnabled02, TestSize.Level1)
{
    MotionManager::GetInstance().SetDefaultSmartMotionEnabled(true);
    ASSERT_TRUE(MotionManager::GetInstance().IsDefaultSmartMotionEnabled());
}

/**
 * @tc.name: Reset01
 * @tc.desc: test function : Reset
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, Reset01, TestSize.Level1)
{
    MotionManager::GetInstance().Init();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::MOTION_TYPE_ROTATION, 90.0f);
    MotionManager::GetInstance().Reset();
    ASSERT_FALSE(MotionManager::GetInstance().IsInitialized());
    ASSERT_EQ(MotionManager::GetInstance().GetLastMotionRotation(), -1.0f);
}

/**
 * @tc.name: SetScreenOnState01
 * @tc.desc: test function : SetScreenOnState
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, SetScreenOnState01, TestSize.Level1)
{
    MotionManager::GetInstance().SetScreenOnState(true);
    ASSERT_TRUE(MotionManager::GetInstance().IsScreenOn());
    MotionManager::GetInstance().SetScreenOnState(false);
    ASSERT_FALSE(MotionManager::GetInstance().IsScreenOn());
}

/**
 * @tc.name: SetDefaultSmartMotionEnabled01
 * @tc.desc: test function : SetDefaultSmartMotionEnabled
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, SetDefaultSmartMotionEnabled01, TestSize.Level1)
{
    MotionManager::GetInstance().SetDefaultSmartMotionEnabled(true);
    ASSERT_TRUE(MotionManager::GetInstance().IsDefaultSmartMotionEnabled());
    MotionManager::GetInstance().SetDefaultSmartMotionEnabled(false);
    ASSERT_FALSE(MotionManager::GetInstance().IsDefaultSmartMotionEnabled());
}

/**
 * @tc.name: NotifyMotionRotationChangedInternal01
 * @tc.desc: test function : NotifyMotionRotationChangedInternal single motion
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, NotifyMotionRotationChangedInternal01, TestSize.Level1)
{
    MockMotionEventListener listener;
    MotionManager::GetInstance().SetMotionEventListener(&listener);
    MotionManager::GetInstance().SetScreenOnState(true);
    MotionManager::GetInstance().SubscribeMotionSensor(MotionType::MOTION_TYPE_ROTATION);
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::MOTION_TYPE_ROTATION, 90.0f);
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: NotifyMotionRotationChangedInternal02
 * @tc.desc: test function : NotifyMotionRotationChangedInternal both motion
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, NotifyMotionRotationChangedInternal02, TestSize.Level1)
{
    MockMotionEventListener listener;
    MotionManager::GetInstance().SetMotionEventListener(&listener);
    MotionManager::GetInstance().SetScreenOnState(true);
    MotionManager::GetInstance().SubscribeMotionSensor(MotionType::MOTION_TYPE_ROTATION);
    MotionManager::GetInstance().SubscribeMotionSensor(MotionType::SMART_MOTION_TYPE_ROTATION);
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::MOTION_TYPE_ROTATION, 90.0f);
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::SMART_MOTION_TYPE_ROTATION, 180.0f);
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: NotifyMotionRotationChangedInternal03
 * @tc.desc: test function : NotifyMotionRotationChangedInternal no listener
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, NotifyMotionRotationChangedInternal03, TestSize.Level1)
{
    MotionManager::GetInstance().SetMotionEventListener(nullptr);
    MotionManager::GetInstance().SetScreenOnState(true);
    MotionManager::GetInstance().SubscribeMotionSensor(MotionType::MOTION_TYPE_ROTATION);
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::MOTION_TYPE_ROTATION, 90.0f);
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: NotifyMotionRotationChangedInternal04
 * @tc.desc: test function : NotifyMotionRotationChangedInternal no subscription
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, NotifyMotionRotationChangedInternal04, TestSize.Level1)
{
    MockMotionEventListener listener;
    MotionManager::GetInstance().SetMotionEventListener(&listener);
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::MOTION_TYPE_ROTATION, 90.0f);
    usleep(SLEEP_TIME_US);
}

}
}
}