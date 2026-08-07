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

#include <gtest/gtest.h>
#include "session_sensor_plugin.h"
#include "motion_manager.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

bool IsSessionMotionSensorLoaded();
bool IsSessionMotionSensorSubscribed(int32_t motionType);

namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;

static void CallbackFunc1(const MotionSensorEvent&) {}
static void CallbackFunc2(const MotionSensorEvent&) { volatile int dummy = 1; }
static void CallbackFunc3(const MotionSensorEvent&) { volatile int dummy = 2; }

class MockMotionEventListener : public IMotionEventListener {
public:
    MockMotionEventListener() : rotationCalled_(false), smartRotationCalled_(false),
        lastRotation_(0.0f), lastSmartRotation_(0.0f) {}
    ~MockMotionEventListener() override = default;

    void OnMotionRotationChanged(float sensorRotation) override
    {
        rotationCalled_ = true;
        lastRotation_ = sensorRotation;
    }

    void OnMotionSmartRotationChanged(float sensorRotation) override
    {
        smartRotationCalled_ = true;
        lastSmartRotation_ = sensorRotation;
    }

    void Reset()
    {
        rotationCalled_ = false;
        smartRotationCalled_ = false;
        lastRotation_ = 0.0f;
        lastSmartRotation_ = 0.0f;
    }

    bool rotationCalled_;
    bool smartRotationCalled_;
    float lastRotation_;
    float lastSmartRotation_;
};
}

class SessionSensorPluginTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SessionSensorPluginTest::SetUpTestCase()
{
}

void SessionSensorPluginTest::TearDownTestCase()
{
}

void SessionSensorPluginTest::SetUp()
{
    SessionUnloadMotionSensor();
}

void SessionSensorPluginTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

HWTEST_F(SessionSensorPluginTest, LoadMotionSensor01, TestSize.Level1)
{
    bool ret = SessionLoadMotionSensor();
    EXPECT_TRUE(ret);
    EXPECT_TRUE(IsSessionMotionSensorLoaded());
}

HWTEST_F(SessionSensorPluginTest, LoadMotionSensor02, TestSize.Level1)
{
    SessionLoadMotionSensor();
    bool ret = SessionLoadMotionSensor();
    EXPECT_TRUE(ret);
}

HWTEST_F(SessionSensorPluginTest, UnloadMotionSensor01, TestSize.Level1)
{
    SessionLoadMotionSensor();
    SessionUnloadMotionSensor();
    EXPECT_FALSE(IsSessionMotionSensorLoaded());
}

HWTEST_F(SessionSensorPluginTest, UnloadMotionSensor02, TestSize.Level1)
{
    SessionUnloadMotionSensor();
    EXPECT_FALSE(IsSessionMotionSensorLoaded());
}

HWTEST_F(SessionSensorPluginTest, SubscribeCallback_NullCallback, TestSize.Level1)
{
    bool ret = SessionSubscribeCallback(700, nullptr);
    EXPECT_FALSE(ret);
}

HWTEST_F(SessionSensorPluginTest, SubscribeCallback_Success, TestSize.Level1)
{
    SessionLoadMotionSensor();
    auto callback = [](const MotionSensorEvent&) {};
    bool ret = SessionSubscribeCallback(700, callback);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(IsSessionMotionSensorSubscribed(700));
}

HWTEST_F(SessionSensorPluginTest, SubscribeCallback_SmartMotionType, TestSize.Level1)
{
    SessionLoadMotionSensor();
    auto callback = [](const MotionSensorEvent&) {};
    bool ret = SessionSubscribeCallback(701, callback);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(IsSessionMotionSensorSubscribed(701));
}

HWTEST_F(SessionSensorPluginTest, SubscribeCallback_SmartEnhanceType, TestSize.Level1)
{
    SessionLoadMotionSensor();
    auto callback = [](const MotionSensorEvent&) {};
    bool ret = SessionSubscribeCallback(703, callback);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(IsSessionMotionSensorSubscribed(703));
}

HWTEST_F(SessionSensorPluginTest, SubscribeCallback_Duplicate, TestSize.Level1)
{
    SessionLoadMotionSensor();
    auto callback = [](const MotionSensorEvent&) {};
    bool ret = SessionSubscribeCallback(700, callback);
    EXPECT_TRUE(ret);
    ret = SessionSubscribeCallback(700, callback);
    EXPECT_TRUE(ret);
}

HWTEST_F(SessionSensorPluginTest, UnsubscribeCallback_Success, TestSize.Level1)
{
    SessionLoadMotionSensor();
    auto callback = [](const MotionSensorEvent&) {};
    SessionSubscribeCallback(700, callback);
    bool ret = SessionUnsubscribeCallback(700, callback);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(IsSessionMotionSensorSubscribed(700));
}

HWTEST_F(SessionSensorPluginTest, UnsubscribeCallback_NullCallback, TestSize.Level1)
{
    SessionLoadMotionSensor();
    bool ret = SessionUnsubscribeCallback(700, nullptr);
    EXPECT_FALSE(ret);
}

HWTEST_F(SessionSensorPluginTest, UnsubscribeCallback_NotSubscribed, TestSize.Level1)
{
    SessionLoadMotionSensor();
    auto callback = [](const MotionSensorEvent&) {};
    bool ret = SessionUnsubscribeCallback(700, callback);
    EXPECT_FALSE(ret);
}

HWTEST_F(SessionSensorPluginTest, UnsubscribeCallback_DifferentCallback, TestSize.Level1)
{
    SessionLoadMotionSensor();
    SessionSubscribeCallback(700, CallbackFunc1);
    bool ret = SessionUnsubscribeCallback(700, CallbackFunc2);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(IsSessionMotionSensorSubscribed(700));
}

HWTEST_F(SessionSensorPluginTest, UnloadClearsSubscriptions, TestSize.Level1)
{
    SessionLoadMotionSensor();
    auto callback = [](const MotionSensorEvent&) {};
    SessionSubscribeCallback(700, callback);
    EXPECT_TRUE(IsSessionMotionSensorSubscribed(700));
    SessionUnloadMotionSensor();
    EXPECT_FALSE(IsSessionMotionSensorSubscribed(700));
}

class MotionManagerListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    MockMotionEventListener listener_;
};

void MotionManagerListenerTest::SetUpTestCase()
{
}

void MotionManagerListenerTest::TearDownTestCase()
{
}

void MotionManagerListenerTest::SetUp()
{
    MotionManager::GetInstance().Reset();
    SessionUnloadMotionSensor();
    listener_.Reset();
}

void MotionManagerListenerTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

HWTEST_F(MotionManagerListenerTest, SetMotionEventListener_DeviceRotation, TestSize.Level1)
{
    MotionManager::GetInstance().SetMotionEventListener(&listener_);
    MotionManager::GetInstance().Init();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, 90.0f);
    EXPECT_TRUE(listener_.rotationCalled_);
    EXPECT_EQ(listener_.lastRotation_, 90.0f);
}

HWTEST_F(MotionManagerListenerTest, SetMotionEventListener_SmartRotation, TestSize.Level1)
{
    MotionManager::GetInstance().SetMotionEventListener(&listener_);
    MotionManager::GetInstance().Init();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::SMART_MOTION_TYPE, 180.0f);
    EXPECT_TRUE(listener_.smartRotationCalled_);
    EXPECT_EQ(listener_.lastSmartRotation_, 180.0f);
}

HWTEST_F(MotionManagerListenerTest, SetMotionEventListener_NullListener, TestSize.Level1)
{
    MotionManager::GetInstance().SetMotionEventListener(nullptr);
    MotionManager::GetInstance().Init();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, 90.0f);
    EXPECT_FALSE(listener_.rotationCalled_);
}

HWTEST_F(MotionManagerListenerTest, SetMotionEventListener_PortraitRotation, TestSize.Level1)
{
    MotionManager::GetInstance().SetMotionEventListener(&listener_);
    MotionManager::GetInstance().Init();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, 0.0f);
    EXPECT_TRUE(listener_.rotationCalled_);
    EXPECT_EQ(listener_.lastRotation_, 0.0f);
}

HWTEST_F(MotionManagerListenerTest, SetMotionEventListener_SameRotationSkipped, TestSize.Level1)
{
    MotionManager::GetInstance().SetMotionEventListener(&listener_);
    MotionManager::GetInstance().Init();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, 90.0f);
    EXPECT_TRUE(listener_.rotationCalled_);
    EXPECT_EQ(listener_.lastRotation_, 90.0f);
    listener_.Reset();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, 90.0f);
    EXPECT_FALSE(listener_.rotationCalled_);
}

HWTEST_F(MotionManagerListenerTest, SetMotionEventListener_SameSmartRotationNotSkipped, TestSize.Level1)
{
    MotionManager::GetInstance().SetMotionEventListener(&listener_);
    MotionManager::GetInstance().Init();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::SMART_MOTION_TYPE, 90.0f);
    EXPECT_TRUE(listener_.smartRotationCalled_);
    EXPECT_EQ(listener_.lastSmartRotation_, 90.0f);
    listener_.Reset();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::SMART_MOTION_TYPE, 90.0f);
    EXPECT_TRUE(listener_.smartRotationCalled_);
}

HWTEST_F(MotionManagerListenerTest, HandleMotionEvent_InvalidType, TestSize.Level1)
{
    MotionManager::GetInstance().SetMotionEventListener(&listener_);
    MotionManager::GetInstance().Init();
    MotionManager::GetInstance().TestHandleMotionEvent(static_cast<MotionType>(999), 90.0f);
    EXPECT_FALSE(listener_.rotationCalled_);
    EXPECT_FALSE(listener_.smartRotationCalled_);
}

HWTEST_F(MotionManagerListenerTest, ResetClearsListener, TestSize.Level1)
{
    MotionManager::GetInstance().SetMotionEventListener(&listener_);
    MotionManager::GetInstance().Init();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, 90.0f);
    EXPECT_TRUE(listener_.rotationCalled_);
    listener_.Reset();
    MotionManager::GetInstance().Reset();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, 90.0f);
    EXPECT_FALSE(listener_.rotationCalled_);
}

HWTEST_F(MotionManagerListenerTest, ReplaceListener, TestSize.Level1)
{
    MotionManager::GetInstance().SetMotionEventListener(&listener_);
    MotionManager::GetInstance().Init();
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, 90.0f);
    EXPECT_TRUE(listener_.rotationCalled_);

    listener_.Reset();
    MockMotionEventListener newListener;
    MotionManager::GetInstance().SetMotionEventListener(&newListener);
    MotionManager::GetInstance().TestHandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, 180.0f);
    EXPECT_TRUE(newListener.rotationCalled_);
    EXPECT_EQ(newListener.lastRotation_, 180.0f);
    EXPECT_FALSE(listener_.rotationCalled_);
}

}
}
