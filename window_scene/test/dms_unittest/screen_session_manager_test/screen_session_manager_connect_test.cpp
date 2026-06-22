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
 
#include "dms_test_base.h"

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
class ScreenSessionManagerConnectTest : public DmsTestBase {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ScreenSessionManagerConnectTest::SetUpTestCase() {}

void ScreenSessionManagerConnectTest::TearDownTestCase() {}

void ScreenSessionManagerConnectTest::SetUp()
{
    ScreenSessionManager::GetInstance().OnStart();
}

void ScreenSessionManagerConnectTest::TearDown()
{
    LOG_SetCallback(nullptr);
    usleep(SLEEP_TIME_US);
}

namespace {
/**
 * @tc.name: ScreenConnectTest
 * @tc.desc: screen connect test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, ScreenConnectTest, TestSize.Level1)
{
    auto displayManagerAgentMock = sptr<DisplayManagerAgentMock>::MakeSptr();
    auto screenSessionManagerClientMock = sptr<ScreenSessionManagerClientMock>::MakeSptr();
    EXPECT_CALL(*screenSessionManagerClientMock, OnScreenConnectionChanged(_, _)).Times(1);
    EXPECT_CALL(*displayManagerAgentMock, NotifyScreenModeChange(_)).Times(1);
    ScreenSessionManager::GetInstance().SetClientProxy(screenSessionManagerClientMock);
    ScreenSessionManager::GetInstance().RegisterDisplayManagerAgent(
        displayManagerAgentMock, DisplayManagerAgentType::SCREEN_MODE_CHANGE_EVENT_LISTENER);

    TriggerScreenConnect(0);
    EXPECT_TRUE(WaitScreenConnectFinish(0));
    usleep(SLEEP_TIME_US);
    ScreenSessionManager::GetInstance().SetClientProxy(nullptr);
    ScreenSessionManager::GetInstance().UnregisterDisplayManagerAgent(
        displayManagerAgentMock, DisplayManagerAgentType::SCREEN_MODE_CHANGE_EVENT_LISTENER);
}

/**
 * @tc.name: ScreenDisconnectTest
 * @tc.desc: screen disconnect test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, ScreenDisconnectTest, TestSize.Level1)
{
    ASSERT_TRUE(CreatScreen(0));
    auto screenSessionManagerClientMock = sptr<ScreenSessionManagerClientMock>::MakeSptr();
    EXPECT_CALL(*screenSessionManagerClientMock, OnScreenConnectionChanged(_, _)).Times(1);
    ScreenSessionManager::GetInstance().SetClientProxy(screenSessionManagerClientMock);

    TriggerScreenDisconnect(0);
    EXPECT_TRUE(WaitScreenDisconnectFinish(0));
    usleep(SLEEP_TIME_US);
    ScreenSessionManager::GetInstance().SetClientProxy(nullptr);
}

}  // namespace
}  // namespace Rosen
}  // namespace OHOS
