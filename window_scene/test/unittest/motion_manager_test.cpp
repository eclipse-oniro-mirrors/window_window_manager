/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 * @tc.name: SubscribeMotionSensor01
 * @tc.desc: test function : SubscribeMotionSensor DEVICE_MOTION_TYPE
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, SubscribeMotionSensor01, TestSize.Level1)
{
    MotionManager::GetInstance().SubscribeMotionSensor(MotionType::DEVICE_MOTION_TYPE);
    ASSERT_TRUE(MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::DEVICE_MOTION_TYPE));
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: SubscribeMotionSensor02
 * @tc.desc: test function : SubscribeMotionSensor SMART_MOTION_TYPE
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, SubscribeMotionSensor02, TestSize.Level1)
{
    MotionManager::GetInstance().SubscribeMotionSensor(MotionType::SMART_MOTION_TYPE);
    ASSERT_TRUE(MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::SMART_MOTION_TYPE));
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: SubscribeMotionSensor03
 * @tc.desc: test function : SubscribeMotionSensor screen off still allows TS control
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, SubscribeMotionSensor03, TestSize.Level1)
{
    MotionManager::GetInstance().SubscribeMotionSensor(MotionType::DEVICE_MOTION_TYPE);
    ASSERT_TRUE(MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::DEVICE_MOTION_TYPE));
}

/**
 * @tc.name: UnsubscribeMotionSensor01
 * @tc.desc: test function : UnsubscribeMotionSensor
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, UnsubscribeMotionSensor01, TestSize.Level1)
{
    MotionManager::GetInstance().SubscribeMotionSensor(MotionType::DEVICE_MOTION_TYPE);
    MotionManager::GetInstance().UnsubscribeMotionSensor(MotionType::DEVICE_MOTION_TYPE);
    ASSERT_FALSE(MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::DEVICE_MOTION_TYPE));
    usleep(SLEEP_TIME_US);
}

/**
 * @tc.name: UnsubscribeMotionSensor02
 * @tc.desc: test function : UnsubscribeMotionSensor not subscribed
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, UnsubscribeMotionSensor02, TestSize.Level1)
{
    MotionManager::GetInstance().UnsubscribeMotionSensor(MotionType::DEVICE_MOTION_TYPE);
    ASSERT_FALSE(MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::DEVICE_MOTION_TYPE));
    usleep(SLEEP_TIME_US);
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
 * @tc.desc: test function : HandleMotionEvent DEVICE_MOTION_TYPE
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, HandleMotionEvent01, TestSize.Level1)
{
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, 90.0f);
    float rotation = MotionManager::GetInstance().GetLastMotionRotation();
    ASSERT_EQ(rotation, 90);
}

/**
 * @tc.name: HandleMotionEvent02
 * @tc.desc: test function : HandleMotionEvent SMART_MOTION_TYPE
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, HandleMotionEvent02, TestSize.Level1)
{
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::SMART_MOTION_TYPE, 180.0f);
    float rotation = MotionManager::GetInstance().GetLastSmartMotionRotation();
    ASSERT_EQ(rotation, 180);
}

/**
 * @tc.name: ConvertMotionActionToDeviceRotation01
 * @tc.desc: test function : ConvertMotionActionToDeviceRotation PORTRAIT
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertMotionActionToDeviceRotation01, TestSize.Level1)
{
    DeviceRotation rotation = MotionManager::ConvertMotionActionToDeviceRotation(
        MotionAction::MOTION_PORTRAIT);
    ASSERT_EQ(rotation, DeviceRotation::ROTATION_PORTRAIT);
}

/**
 * @tc.name: ConvertMotionActionToDeviceRotation02
 * @tc.desc: test function : ConvertMotionActionToDeviceRotation LANDSCAPE
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertMotionActionToDeviceRotation02, TestSize.Level1)
{
    DeviceRotation rotation = MotionManager::ConvertMotionActionToDeviceRotation(
        MotionAction::MOTION_LANDSCAPE);
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
        MotionAction::MOTION_PORTRAIT_INVERTED);
    ASSERT_EQ(rotation, DeviceRotation::ROTATION_PORTRAIT_INVERTED);
}

/**
 * @tc.name: ConvertMotionActionToDeviceRotation04
 * @tc.desc: test function : ConvertMotionActionToDeviceRotation LANDSCAPE_INVERTED
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertMotionActionToDeviceRotation04, TestSize.Level1)
{
    DeviceRotation rotation = MotionManager::ConvertMotionActionToDeviceRotation(
        MotionAction::MOTION_LANDSCAPE_INVERTED);
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
 * @tc.name: ConvertDeviceMotionToFloat01
 * @tc.desc: test function : ConvertDeviceMotionToFloat PORTRAIT
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertDeviceMotionToFloat01, TestSize.Level1)
{
    float rotation = MotionManager::ConvertDeviceMotionToFloat(DeviceRotation::ROTATION_PORTRAIT);
    ASSERT_EQ(rotation, 0.0f);
}

/**
 * @tc.name: ConvertDeviceMotionToFloat02
 * @tc.desc: test function : ConvertDeviceMotionToFloat LANDSCAPE
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertDeviceMotionToFloat02, TestSize.Level1)
{
    float rotation = MotionManager::ConvertDeviceMotionToFloat(DeviceRotation::ROTATION_LANDSCAPE);
    ASSERT_EQ(rotation, 90.0f);
}

/**
 * @tc.name: ConvertDeviceMotionToFloat03
 * @tc.desc: test function : ConvertDeviceMotionToFloat PORTRAIT_INVERTED
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertDeviceMotionToFloat03, TestSize.Level1)
{
    float rotation = MotionManager::ConvertDeviceMotionToFloat(DeviceRotation::ROTATION_PORTRAIT_INVERTED);
    ASSERT_EQ(rotation, 180.0f);
}

/**
 * @tc.name: ConvertDeviceMotionToFloat04
 * @tc.desc: test function : ConvertDeviceMotionToFloat LANDSCAPE_INVERTED
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertDeviceMotionToFloat04, TestSize.Level1)
{
    float rotation = MotionManager::ConvertDeviceMotionToFloat(DeviceRotation::ROTATION_LANDSCAPE_INVERTED);
    ASSERT_EQ(rotation, 270.0f);
}

/**
 * @tc.name: ConvertDeviceMotionToFloat05
 * @tc.desc: test function : ConvertDeviceMotionToFloat INVALID
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, ConvertDeviceMotionToFloat05, TestSize.Level1)
{
    float rotation = MotionManager::ConvertDeviceMotionToFloat(DeviceRotation::INVALID);
    ASSERT_EQ(rotation, -1.0f);
}

/**
 * @tc.name: IsMotionSensorSubscribed01
 * @tc.desc: test function : IsMotionSensorSubscribed not subscribed
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, IsMotionSensorSubscribed01, TestSize.Level1)
{
    bool subscribed = MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::DEVICE_MOTION_TYPE);
    ASSERT_FALSE(subscribed);
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
 * @tc.name: Reset01
 * @tc.desc: test function : Reset
 * @tc.type: FUNC
 */
HWTEST_F(MotionManagerTest, Reset01, TestSize.Level1)
{
    MotionManager::GetInstance().Init();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, 90.0f);
    MotionManager::GetInstance().Reset();
    ASSERT_FALSE(MotionManager::GetInstance().IsInitialized());
    ASSERT_EQ(MotionManager::GetInstance().GetLastMotionRotation(), -1.0f);
}
}
}
}