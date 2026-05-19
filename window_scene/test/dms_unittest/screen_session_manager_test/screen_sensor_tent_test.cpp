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

#include "dms_test_base.h"
#include "screen_sensor_connector.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;

std::string g_logMsg;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
                   const char* msg)
{
    g_logMsg += msg;
}
}  // namespace
class ScreenSensorTentTest : public DmsTestBase {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ScreenSensorTentTest::SetUpTestCase()
{
    ScreenSessionManager::GetInstance().OnStart();
}

void ScreenSensorTentTest::TearDownTestCase() {}

void ScreenSensorTentTest::SetUp() {}

void ScreenSensorTentTest::TearDown()
{
    LOG_SetCallback(nullptr);
    usleep(SLEEP_TIME_US);
}

namespace {
/**
 * @tc.name: RotationEventTest
 * @tc.desc: eotation event test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSensorTentTest, RotationEventTest, TestSize.Level1)
{
    ScreenSensorConnector::SubscribeTentSensor();
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    int32_t data = 4;
    int32_t status = 0;
    TriggerTentSensorEvent(status, 4, &data);
    EXPECT_TRUE(g_logMsg.find("tent motion:0") != std::string::npos);
    status = 1;
    TriggerTentSensorEvent(status, 4, &data);
    EXPECT_TRUE(g_logMsg.find("tent motion:1") != std::string::npos);
    status = 2;
    TriggerTentSensorEvent(status, 4, &data);
    EXPECT_TRUE(g_logMsg.find("tent motion:2") != std::string::npos);
    status = 3;
    TriggerTentSensorEvent(status, 4, &data);
    EXPECT_TRUE(g_logMsg.find("tent motion:2") != std::string::npos);
    status = 4;
    TriggerTentSensorEvent(status, 4, &data);
    EXPECT_TRUE(g_logMsg.find("tent motion:1") != std::string::npos);
    status = 5;
    TriggerTentSensorEvent(status, 4, &data);
    EXPECT_TRUE(g_logMsg.find("tent motion:1") != std::string::npos);
    status = 99;
    TriggerTentSensorEvent(status, 4, &data);
    EXPECT_TRUE(g_logMsg.find("tent motion:3") != std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

}  // namespace
}  // namespace Rosen
}  // namespace OHOS