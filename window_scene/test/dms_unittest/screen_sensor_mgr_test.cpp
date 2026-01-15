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
#include "product_config.h"
#include "screen_sensor_mgr.h"
#include "sensor_fold_state_mgr.h"
#include "fold_screen_controller/secondary_fold_sensor_manager.h"
#include "fold_screen_controller/fold_screen_controller_config.h"
#undef private
#undef protected

int32_t SubscribeSensor(int32_t sensorTypeId, const SensorUser* user)
{
    return 0;
}

int32_t UnsubscribeSensor(int32_t sensorTypeId, const SensorUser* user)
{
    return 0;
}

int32_t SetBatch(int32_t sensorTypeId, const SensorUser* user, int64_t samplingInterval, int64_t reportInterval)
{
    return 0;
}

int32_t ActivateSensor(int32_t sensorTypeId, const SensorUser* user)
{
    return 0;
}

int32_t DeactivateSensor(int32_t sensorTypeId, const SensorUser* user)
{
    return 0;
}

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

class MockProductConfig : public ProductConfig {
public:
    MOCK_METHOD(bool, IsSingleDisplaySuperFoldDevice, (), (override));
    MOCK_METHOD(bool, IsSecondaryDisplayFoldDevice, (), (override));
};
}

class ScreenSensorMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ScreenSensorMgrTest::SetUpTestCase() {}

void ScreenSensorMgrTest::TearDownTestCase() {}

void ScreenSensorMgrTest::SetUp() {}

void ScreenSensorMgrTest::TearDown()
{
    LOG_SetCallback(nullptr);
    usleep(SLEEP_TIME_US);
}

namespace {

/**
 * @tc.name: HandleSensorDataTest001
 * @tc.desc: test function : HandleSensorData
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, HandleSensorDataTest001, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorEvent event;
    event.data = nullptr;
    ScreenSensorMgr::GetInstance().HandleSensorData(&event);
    EXPECT_TRUE(g_logMsg.find("Received invalid sensor event or data is null") != std::string::npos);
}

/**
 * @tc.name: HandleSensorDataTest002
 * @tc.desc: test function : HandleSensorData
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, HandleSensorDataTest002, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorEvent event;
    PostureData postureData;
    postureData.angle = 45.0F;
    event.data = reinterpret_cast<uint8_t*>(&postureData);
    event.dataLen = 0;

    ScreenSensorMgr::GetInstance().HandleSensorData(&event);
    EXPECT_TRUE(g_logMsg.find("Sensor event data length is zero, skipping processing") != std::string::npos);
}

/**
 * @tc.name: HandleSensorDataTest003
 * @tc.desc: test function : HandleSensorData
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, HandleSensorDataTest003, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorEvent event;
    PostureData postureData;
    postureData.angle = 45.0F;
    event.data = reinterpret_cast<uint8_t*>(&postureData);
    event.dataLen = sizeof(PostureData);

    ScreenSensorMgr::GetInstance().HandleSensorData(&event);
    EXPECT_TRUE(g_logMsg.find("No callback registered for sensorTypeId") != std::string::npos);
}

/**
 * @tc.name: HandleSensorDataTest004
 * @tc.desc: test function : HandleSensorData
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, HandleSensorDataTest004, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    SensorEvent event;
    PostureData postureData;
    postureData.angle = 45.0F;
    event.data = reinterpret_cast<uint8_t*>(&postureData);
    event.dataLen = sizeof(PostureData);
    event.sensorTypeId = SENSOR_TYPE_ID_POSTURE;

    g_callFlag = false;
    int ret = ScreenSensorMgr::GetInstance().SubscribeSensorCallback(
        SENSOR_TYPE_ID_POSTURE, 10000000, [](SensorEvent* event) { g_callFlag = true; });
    EXPECT_EQ(ret, 0);
    ScreenSensorMgr::GetInstance().HandleSensorData(&event);
    EXPECT_TRUE(g_callFlag);
}

/**
 * @tc.name: RegisterPostureCallbackTest01
 * @tc.desc: test function : SingleDisplaySuperFoldDevice register posture success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, RegisterPostureCallbackTest01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSingleDisplaySuperFoldDevice()).WillRepeatedly(Return(true));
    ProductConfig::singleton_ = &mockObj;

    ScreenSensorMgr::GetInstance().RegisterPostureCallback();
    EXPECT_TRUE(g_logMsg.find("register posture callback success.") != std::string::npos);
    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: RegisterPostureCallbackTest02
 * @tc.desc: test function : SecondaryDevice register posture success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, RegisterPostureCallbackTest02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSingleDisplaySuperFoldDevice()).WillRepeatedly(Return(false));
    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(true));
    ProductConfig::singleton_ = &mockObj;

    ScreenSensorMgr::GetInstance().RegisterPostureCallback();
    EXPECT_TRUE(g_logMsg.find("RegisterPostureCallback: success.") != std::string::npos);
    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: RegisterPostureCallbackTest03
 * @tc.desc: test function : SingleDisplayFoldDevice register posture success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, RegisterPostureCallbackTest03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSingleDisplaySuperFoldDevice()).WillRepeatedly(Return(false));
    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(false));
    ProductConfig::singleton_ = &mockObj;

    ScreenSensorMgr::GetInstance().RegisterPostureCallback();
    EXPECT_TRUE(g_logMsg.find("FoldScreenSensorManager.RegisterPostureCallback success.") != std::string::npos);
    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: RegisterHallCallbackTest01
 * @tc.desc: test function : SingleDisplaySuperFoldDevice register hall success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, RegisterHallCallbackTest01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSingleDisplaySuperFoldDevice()).WillRepeatedly(Return(true));
    ProductConfig::singleton_ = &mockObj;

    ScreenSensorMgr::GetInstance().RegisterHallCallback();
    EXPECT_TRUE(g_logMsg.find("register hall callback success.") != std::string::npos);
    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: RegisterHallCallbackTest02
 * @tc.desc: test function : SecondaryDisplayFoldDevice register hall success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, RegisterHallCallbackTest02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSingleDisplaySuperFoldDevice()).WillRepeatedly(Return(false));
    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(true));
    ProductConfig::singleton_ = &mockObj;

    ScreenSensorMgr::GetInstance().RegisterHallCallback();
    EXPECT_TRUE(g_logMsg.find("success.") != std::string::npos);
    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: RegisterHallCallbackTest03
 * @tc.desc: test function : SingleDisplayFoldDevice register posture success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, RegisterHallCallbackTest03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSingleDisplaySuperFoldDevice()).WillRepeatedly(Return(false));
    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(false));
    ProductConfig::singleton_ = &mockObj;

    ScreenSensorMgr::GetInstance().RegisterHallCallback();
    EXPECT_TRUE(g_logMsg.find("success.") != std::string::npos);
    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: UnRegisterPostureCallbackTest01
 * @tc.desc: test function : UnRegisterPostureCallback success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, UnRegisterPostureCallbackTest01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    ScreenSensorMgr::GetInstance().RegisterPostureCallback();
    EXPECT_TRUE(g_logMsg.find("success.") != std::string::npos);
    g_logMsg.clear();

    ScreenSensorMgr::GetInstance().UnRegisterPostureCallback();
    EXPECT_TRUE(g_logMsg.find("success.") != std::string::npos);
}

/**
 * @tc.name: UnRegisterPostureCallbackTest02
 * @tc.desc: test function : UnRegisterPostureCallback failed
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, UnRegisterPostureCallbackTest02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSingleDisplaySuperFoldDevice()).WillRepeatedly(Return(true));
    ProductConfig::singleton_ = &mockObj;

    ScreenSensorMgr::GetInstance().UnRegisterPostureCallback();
    EXPECT_TRUE(g_logMsg.find("UnRegisterPostureCallback failed") != std::string::npos);
    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: UnRegisterPostureCallbackTest03
 * @tc.desc: test function : SecondaryDisplayFoldDevice unregister posture callback test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, UnRegisterPostureCallbackTest03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSingleDisplaySuperFoldDevice()).WillRepeatedly(Return(false));
    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(true));
    ProductConfig::singleton_ = &mockObj;
 
    ScreenSensorMgr::GetInstance().RegisterPostureCallback();
    EXPECT_TRUE(SecondaryFoldSensorManager::GetInstance().registerPosture_);
    
    ScreenSensorMgr::GetInstance().UnRegisterPostureCallback();
    EXPECT_TRUE(!SecondaryFoldSensorManager::GetInstance().registerPosture_);
 
    g_logMsg.clear();
    ScreenSensorMgr::GetInstance().UnRegisterPostureCallback();
    EXPECT_TRUE(g_logMsg.find("failed with ret: 1") != std::string::npos);
 
    ProductConfig::singleton_ = nullptr;
}
 
/**
 * @tc.name: UnRegisterPostureCallbackTest04
 * @tc.desc: test function : SingleDispalyFoldDevice unregister posture callback test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, UnRegisterPostureCallbackTest04, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSingleDisplaySuperFoldDevice()).WillRepeatedly(Return(false));
    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(false));
    ProductConfig::singleton_ = &mockObj;
    
    ScreenSensorMgr::GetInstance().RegisterPostureCallback();
    EXPECT_TRUE(FoldScreenSensorManager::GetInstance().registerPosture_);
    
    ScreenSensorMgr::GetInstance().UnRegisterPostureCallback();
    EXPECT_TRUE(!FoldScreenSensorManager::GetInstance().registerPosture_);
 
    g_logMsg.clear();
    ScreenSensorMgr::GetInstance().UnRegisterPostureCallback();
    EXPECT_TRUE(g_logMsg.find("failed with ret: 1") != std::string::npos);
 
    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: UnRegisterHallCallbackTest01
 * @tc.desc: test function : UnRegisterHallCallback success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, UnRegisterHallCallbackTest01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    ScreenSensorMgr::GetInstance().RegisterHallCallback();
    g_logMsg.clear();

    ScreenSensorMgr::GetInstance().UnRegisterHallCallback();
    EXPECT_TRUE(g_logMsg.find("success.") != std::string::npos);
}

/**
 * @tc.name: UnRegisterHallCallbackTest02
 * @tc.desc: test function : UnRegisterHallCallback failed
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, UnRegisterHallCallbackTest02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSingleDisplaySuperFoldDevice()).WillRepeatedly(Return(true));
    ProductConfig::singleton_ = &mockObj;

    ScreenSensorMgr::GetInstance().UnRegisterHallCallback();
    EXPECT_TRUE(g_logMsg.find("unregister hall sensor failed") != std::string::npos);
    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: UnRegisterHallCallbackTest03
 * @tc.desc: test function : SecondaryDisplayFoldDevice unregister hall callback test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, UnRegisterHallCallbackTest03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSingleDisplaySuperFoldDevice()).WillRepeatedly(Return(false));
    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(true));
    ProductConfig::singleton_ = &mockObj;
 
    g_logMsg.clear();
    ScreenSensorMgr::GetInstance().RegisterHallCallback();
    EXPECT_TRUE(g_logMsg.find("success") != std::string::npos);
    
    g_logMsg.clear();
    ScreenSensorMgr::GetInstance().UnRegisterHallCallback();
    EXPECT_TRUE(g_logMsg.find("success") != std::string::npos);
 
    g_logMsg.clear();
    ScreenSensorMgr::GetInstance().UnRegisterHallCallback();
    EXPECT_TRUE(g_logMsg.find("failed with ret: 1") != std::string::npos);
 
    ProductConfig::singleton_ = nullptr;
}
 
/**
 * @tc.name: UnRegisterHallCallbackTest04
 * @tc.desc: test function : SingleDispalyFoldDevice unregister hall callback test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, UnRegisterHallCallbackTest04, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockProductConfig mockObj;
    EXPECT_CALL(mockObj, IsSingleDisplaySuperFoldDevice()).WillRepeatedly(Return(false));
    EXPECT_CALL(mockObj, IsSecondaryDisplayFoldDevice()).WillRepeatedly(Return(false));
    ProductConfig::singleton_ = &mockObj;
 
    ScreenSensorMgr::GetInstance().RegisterHallCallback();
    EXPECT_TRUE(FoldScreenSensorManager::GetInstance().registerHall_);
    
    ScreenSensorMgr::GetInstance().UnRegisterHallCallback();
    EXPECT_TRUE(!FoldScreenSensorManager::GetInstance().registerHall_);
 
    g_logMsg.clear();
    ScreenSensorMgr::GetInstance().UnRegisterHallCallback();
    EXPECT_TRUE(g_logMsg.find("failed with ret: 1") != std::string::npos);
 
    ProductConfig::singleton_ = nullptr;
}

/**
 * @tc.name: HandlePostureDataTest01
 * @tc.desc: test function : HandlePostureData failed, invalid input
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, HandlePostureDataTest01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::unique_ptr<SensorEvent> event = nullptr;
    ScreenSensorMgr::GetInstance().HandlePostureData(event.get());
    EXPECT_TRUE(g_logMsg.find("SensorEvent is nullptr") != std::string::npos);

    g_logMsg.clear();
    event = std::make_unique<SensorEvent>();
    event->data = nullptr;
    ScreenSensorMgr::GetInstance().HandlePostureData(event.get());
    EXPECT_TRUE(g_logMsg.find("data is nullptr") != std::string::npos);

    g_logMsg.clear();
    PostureData data;
    event->data = reinterpret_cast<uint8_t*>(&data);
    event->dataLen = 0;
    ScreenSensorMgr::GetInstance().HandlePostureData(event.get());
    EXPECT_TRUE(g_logMsg.find("dataLen less than data size") != std::string::npos);
}

/**
 * @tc.name: HandlePostureDataTest02
 * @tc.desc: test function : HandlePostureData failed, invalid angle
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, HandlePostureDataTest02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::unique_ptr<SensorEvent> event = std::make_unique<SensorEvent>();
    PostureData data;
    data.angle = 190.0;
    event->data = reinterpret_cast<uint8_t*>(&data);
    event->dataLen = sizeof(PostureData);
    ScreenSensorMgr::GetInstance().HandlePostureData(event.get());
    EXPECT_TRUE(g_logMsg.find("invalid angle,") != std::string::npos);
}

/**
 * @tc.name: HandlePostureDataTest03
 * @tc.desc: test function : HandlePostureData success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, HandlePostureDataTest03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::unique_ptr<SensorEvent> event = std::make_unique<SensorEvent>();
    PostureData data;
    data.angle = 170.0;
    event->data = reinterpret_cast<uint8_t*>(&data);
    event->dataLen = sizeof(PostureData);

    ScreenSensorMgr::GetInstance().HandlePostureData(event.get());
    EXPECT_EQ(ScreenSensorMgr::GetInstance().angle_[0], 170.0);
}

/**
 * @tc.name: HandleHallDataTest01
 * @tc.desc: test function : HandleHallData failed, invalid input
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, HandleHallDataTest01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::unique_ptr<SensorEvent> event = std::make_unique<SensorEvent>();
    ExtHallData data;
    data.flag = 0;
    event->data = reinterpret_cast<uint8_t*>(&data);
    event->dataLen = sizeof(ExtHallData);
    ScreenSensorMgr::GetInstance().HandleHallData(event.get());
    EXPECT_TRUE(g_logMsg.find("NOT Support Extend Hall") != std::string::npos);

    g_logMsg.clear();
    ScreenSensorMgr::GetInstance().hall_[0] = 0;
    data.flag = 0xf;
    data.hall = 0;
    ScreenSensorMgr::GetInstance().HandleHallData(event.get());
    EXPECT_TRUE(g_logMsg.find("Hall don't change") != std::string::npos);
}

/**
 * @tc.name: HandleHallDataTest02
 * @tc.desc: test function : HandlePostureData failed, invalid hall
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, HandleHallDataTest02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::unique_ptr<SensorEvent> event = std::make_unique<SensorEvent>();
    ExtHallData data;
    data.flag = 0xf;
    ScreenSensorMgr::GetInstance().hall_[0] = 0;
    data.hall = 10;
    event->data = reinterpret_cast<uint8_t*>(&data);
    event->dataLen = sizeof(ExtHallData);
    ScreenSensorMgr::GetInstance().HandleHallData(event.get());
    EXPECT_TRUE(g_logMsg.find("invalid hall,") != std::string::npos);
}

/**
 * @tc.name: HandleHallDataTest03
 * @tc.desc: test function : HandlePostureData with abnormal angle
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, HandleHallDataTest03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::unique_ptr<SensorEvent> event = std::make_unique<SensorEvent>();
    ExtHallData data;
    data.flag = 0xf;
    ScreenSensorMgr::GetInstance().hall_[0] = 1;
    ScreenSensorMgr::GetInstance().angle_[0] = -1.0;
    ScreenSensorMgr::GetInstance().registerPosture_ = false;
    data.hall = 0;
    event->data = reinterpret_cast<uint8_t*>(&data);
    event->dataLen = sizeof(ExtHallData);
    ScreenSensorMgr::GetInstance().HandleHallData(event.get());
    EXPECT_TRUE(g_logMsg.find("hall value is: 0, let angle value is") != std::string::npos);
}

/**
 * @tc.name: UpdateSensorIntervalTest01
 * @tc.desc: test function : UpdateSensorInterval failed， invalid sensorTypeId
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, UpdateSensorIntervalTest01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    int32_t ret = ScreenSensorMgr::GetInstance().UpdateSensorInterval(100, 100000);
    EXPECT_TRUE(g_logMsg.find("User data not found for sensor type 100") != std::string::npos);
    EXPECT_EQ(1, ret);
}

/**
 * @tc.name: UpdateSensorIntervalTest02
 * @tc.desc: test function : UpdateSensorInterval success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, UpdateSensorIntervalTest02, TestSize.Level1)
{
    ScreenSensorMgr::GetInstance().RegisterPostureCallback();
    int32_t ret = ScreenSensorMgr::GetInstance().UpdateSensorInterval(SENSOR_TYPE_ID_POSTURE, 100000);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name: HasSubscribedSensorTest
 * @tc.desc: test function : HasSubscribedSensor test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorMgrTest, HasSubscribedSensorTest, TestSize.Level1)
{
    ScreenSensorMgr::GetInstance().RegisterPostureCallback();
    EXPECT_EQ(false, ScreenSensorMgr::GetInstance().HasSubscribedSensor(100000));
    EXPECT_EQ(true, ScreenSensorMgr::GetInstance().HasSubscribedSensor(SENSOR_TYPE_ID_POSTURE));
}

}
}
} // namespace Rosen
} // namespace OHOS