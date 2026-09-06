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
#include "motion_manager.h"
#include "scene_session_manager.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;

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

class SceneSessionManagerMotionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionManagerMotionTest::SetUpTestCase()
{
}

void SceneSessionManagerMotionTest::TearDownTestCase()
{
}

void SceneSessionManagerMotionTest::SetUp()
{
    MotionManager::GetInstance().Reset();
}

void SceneSessionManagerMotionTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

HWTEST_F(SceneSessionManagerMotionTest, RegisterMotionSensor01, TestSize.Level1)
{
    bool ret = SceneSessionManager::GetInstance().RegisterMotionSensor(
        static_cast<int32_t>(MotionType::DEVICE_MOTION_TYPE));
    EXPECT_TRUE(ret);
    EXPECT_TRUE(MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::DEVICE_MOTION_TYPE));
}

HWTEST_F(SceneSessionManagerMotionTest, RegisterMotionSensor02, TestSize.Level1)
{
    bool ret = SceneSessionManager::GetInstance().RegisterMotionSensor(
        static_cast<int32_t>(MotionType::SMART_MOTION_TYPE));
    EXPECT_TRUE(ret);
    EXPECT_TRUE(MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::SMART_MOTION_TYPE));
}

HWTEST_F(SceneSessionManagerMotionTest, RegisterMotionSensor03, TestSize.Level1)
{
    bool ret = SceneSessionManager::GetInstance().RegisterMotionSensor(
        static_cast<int32_t>(MotionType::SMART_MOTION_ENHANCE_TYPE));
    EXPECT_TRUE(ret);
    EXPECT_TRUE(MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::SMART_MOTION_ENHANCE_TYPE));
}

HWTEST_F(SceneSessionManagerMotionTest, UnregisterMotionSensor01, TestSize.Level1)
{
    SceneSessionManager::GetInstance().RegisterMotionSensor(
        static_cast<int32_t>(MotionType::DEVICE_MOTION_TYPE));
    bool ret = SceneSessionManager::GetInstance().UnregisterMotionSensor(
        static_cast<int32_t>(MotionType::DEVICE_MOTION_TYPE));
    EXPECT_TRUE(ret);
    EXPECT_FALSE(MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::DEVICE_MOTION_TYPE));
}

HWTEST_F(SceneSessionManagerMotionTest, UnregisterMotionSensor02, TestSize.Level1)
{
    bool ret = SceneSessionManager::GetInstance().UnregisterMotionSensor(
        static_cast<int32_t>(MotionType::DEVICE_MOTION_TYPE));
    EXPECT_TRUE(ret);
}

HWTEST_F(SceneSessionManagerMotionTest, UnregisterMotionSensor03, TestSize.Level1)
{
    SceneSessionManager::GetInstance().RegisterMotionSensor(
        static_cast<int32_t>(MotionType::SMART_MOTION_TYPE));
    bool ret = SceneSessionManager::GetInstance().UnregisterMotionSensor(
        static_cast<int32_t>(MotionType::SMART_MOTION_TYPE));
    EXPECT_TRUE(ret);
    EXPECT_FALSE(MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::SMART_MOTION_TYPE));
}

HWTEST_F(SceneSessionManagerMotionTest, RegisterMotionSensor_InitMotionManager, TestSize.Level1)
{
    MotionManager::GetInstance().Reset();
    EXPECT_FALSE(MotionManager::GetInstance().IsInitialized());
    MotionManager::GetInstance().Init();
    EXPECT_TRUE(MotionManager::GetInstance().IsInitialized());
    bool ret = SceneSessionManager::GetInstance().RegisterMotionSensor(
        static_cast<int32_t>(MotionType::DEVICE_MOTION_TYPE));
    EXPECT_TRUE(ret);
    EXPECT_TRUE(MotionManager::GetInstance().IsMotionSensorSubscribed(MotionType::DEVICE_MOTION_TYPE));
}

HWTEST_F(SceneSessionManagerMotionTest, OnMotionRotationChanged01, TestSize.Level1)
{
    bool callbackCalled = false;
    float receivedRotation = 0.0f;
    SceneSessionManager::GetInstance().SetSensorRotationChangeListener(
        [&callbackCalled, &receivedRotation](float rotation) {
            callbackCalled = true;
            receivedRotation = rotation;
        });
    usleep(SLEEP_TIME_US);

    SceneSessionManager::GetInstance().OnMotionRotationChanged(90.0f);
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedRotation, 90.0f);
}

HWTEST_F(SceneSessionManagerMotionTest, OnMotionRotationChanged02, TestSize.Level1)
{
    SceneSessionManager::GetInstance().SetSensorRotationChangeListener(nullptr);
    usleep(SLEEP_TIME_US);

    bool callbackCalled = false;
    SceneSessionManager::GetInstance().OnMotionRotationChanged(90.0f);
    EXPECT_FALSE(callbackCalled);
}

HWTEST_F(SceneSessionManagerMotionTest, OnMotionSmartRotationChanged01, TestSize.Level1)
{
    bool callbackCalled = false;
    float receivedRotation = 0.0f;
    SceneSessionManager::GetInstance().SetSmartSensorRotationChangeListener(
        [&callbackCalled, &receivedRotation](float rotation) {
            callbackCalled = true;
            receivedRotation = rotation;
        });
    usleep(SLEEP_TIME_US);

    SceneSessionManager::GetInstance().OnMotionSmartRotationChanged(180.0f);
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedRotation, 180.0f);
}

HWTEST_F(SceneSessionManagerMotionTest, OnMotionSmartRotationChanged02, TestSize.Level1)
{
    SceneSessionManager::GetInstance().SetSmartSensorRotationChangeListener(nullptr);
    usleep(SLEEP_TIME_US);

    bool callbackCalled = false;
    SceneSessionManager::GetInstance().OnMotionSmartRotationChanged(180.0f);
    EXPECT_FALSE(callbackCalled);
}

HWTEST_F(SceneSessionManagerMotionTest, SetSensorRotationChangeListener01, TestSize.Level1)
{
    bool callbackCalled = false;
    SceneSessionManager::GetInstance().SetSensorRotationChangeListener(
        [&callbackCalled](float) {
            callbackCalled = true;
        });
    usleep(SLEEP_TIME_US);
    SceneSessionManager::GetInstance().OnMotionRotationChanged(0.0f);
    EXPECT_TRUE(callbackCalled);
}

HWTEST_F(SceneSessionManagerMotionTest, SetSmartSensorRotationChangeListener01, TestSize.Level1)
{
    bool callbackCalled = false;
    SceneSessionManager::GetInstance().SetSmartSensorRotationChangeListener(
        [&callbackCalled](float) {
            callbackCalled = true;
        });
    usleep(SLEEP_TIME_US);
    SceneSessionManager::GetInstance().OnMotionSmartRotationChanged(0.0f);
    EXPECT_TRUE(callbackCalled);
}

}
}
