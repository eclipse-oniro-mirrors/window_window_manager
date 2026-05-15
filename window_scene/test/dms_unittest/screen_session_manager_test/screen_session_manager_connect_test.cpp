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
#include "display_manager_agent_default.h"
#include "screen_scene_config.h"
#include "screen_session.h"
#include "virtual_screen_option.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
constexpr ScreenId SCREEN_ID_MAIN = 5;
constexpr ScreenId SCREEN_ID_DEFAULT = 0;

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

/**
 * @tc.name: HandleDisconnectEventDefault01
 * @tc.desc: test HandleDisconnectEventDefault with PROCESS_DISCONNECTED reason, concurrent user and virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, HandleDisconnectEventDefault01, TestSize.Level1)
{
    ScreenSceneConfig::isConcurrentUser_ = true;
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "virtualScreen01";
    ScreenId screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    ASSERT_NE(screenId, SCREEN_ID_INVALID);

    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
    ASSERT_NE(screenSession, nullptr);

    ScreenSessionManager::GetInstance().HandleDisconnectEventDefault(
        screenSession, screenId, ScreenEvent::DISCONNECTED, ScreenChangeReason::PROCESS_DISCONNECTED);

    auto sessionAfterDestroy = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
    EXPECT_EQ(sessionAfterDestroy, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = false;
}

/**
 * @tc.name: HandleDisconnectEventDefault02
 * @tc.desc: test HandleDisconnectEventDefault with PROCESS_DISCONNECTED reason, concurrent user and real screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, HandleDisconnectEventDefault02, TestSize.Level1)
{
    ScreenSceneConfig::isConcurrentUser_ = true;
    ASSERT_TRUE(CreatScreen(1));

    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(1);
    ASSERT_NE(screenSession, nullptr);
    ASSERT_EQ(screenSession->GetScreenProperty().GetScreenType(), ScreenType::REAL);

    auto displayManagerAgentMock = sptr<DisplayManagerAgentMock>::MakeSptr();
    EXPECT_CALL(*displayManagerAgentMock, OnScreenDisconnect(_)).Times(1);
    EXPECT_CALL(*displayManagerAgentMock, OnDisplayDestroy(_)).Times(1);
    ScreenSessionManager::GetInstance().RegisterDisplayManagerAgent(
        displayManagerAgentMock, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);

    ScreenSessionManager::GetInstance().HandleDisconnectEventDefault(
        screenSession, 1, ScreenEvent::DISCONNECTED, ScreenChangeReason::PROCESS_DISCONNECTED);

    usleep(SLEEP_TIME_US);
    ScreenSessionManager::GetInstance().UnregisterDisplayManagerAgent(
        displayManagerAgentMock, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    ScreenSceneConfig::isConcurrentUser_ = false;
}

/**
 * @tc.name: HandleDisconnectEventDefault03
 * @tc.desc: test HandleDisconnectEventDefault with DEFAULT reason
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, HandleDisconnectEventDefault03, TestSize.Level1)
{
    ASSERT_TRUE(CreatScreen(2));

    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(2);
    ASSERT_NE(screenSession, nullptr);

    auto screenSessionManagerClientMock = sptr<ScreenSessionManagerClientMock>::MakeSptr();
    EXPECT_CALL(*screenSessionManagerClientMock, OnScreenConnectionChanged(_, _)).Times(1);
    ScreenSessionManager::GetInstance().SetClientProxy(screenSessionManagerClientMock);

    ScreenSessionManager::GetInstance().HandleDisconnectEventDefault(
        screenSession, 2, ScreenEvent::DISCONNECTED, ScreenChangeReason::DEFAULT);

    usleep(SLEEP_TIME_US);
    ScreenSessionManager::GetInstance().SetClientProxy(nullptr);
}

/**
 * @tc.name: HandleDisconnectEventDefault04
 * @tc.desc: test HandleDisconnectEventDefault with PROCESS_DISCONNECTED reason but not concurrent user
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, HandleDisconnectEventDefault04, TestSize.Level1)
{
    ScreenSceneConfig::isConcurrentUser_ = false;
    ASSERT_TRUE(CreatScreen(3));

    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(3);
    ASSERT_NE(screenSession, nullptr);

    auto screenSessionManagerClientMock = sptr<ScreenSessionManagerClientMock>::MakeSptr();
    EXPECT_CALL(*screenSessionManagerClientMock, OnScreenConnectionChanged(_, _)).Times(1);
    ScreenSessionManager::GetInstance().SetClientProxy(screenSessionManagerClientMock);

    ScreenSessionManager::GetInstance().HandleDisconnectEventDefault(
        screenSession, 3, ScreenEvent::DISCONNECTED, ScreenChangeReason::PROCESS_DISCONNECTED);

    usleep(SLEEP_TIME_US);
    ScreenSessionManager::GetInstance().SetClientProxy(nullptr);
}

/**
 * @tc.name: HandleDisconnectEventInner01
 * @tc.desc: test HandleDisconnectEventInner with nullptr screenSession, should early return and log error
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, HandleDisconnectEventInner01, TestSize.Level1)
{
    LOG_SetCallback(MyLogCallback);
    g_logMsg.clear();

    ScreenSessionManager::GetInstance().connectScreenNumber_ = 5;
    ScreenId invalidScreenId = 100;
    ScreenEvent screenEvent = ScreenEvent::DISCONNECTED;
    bool phyMirrorEnable = false;

    ScreenSessionManager::GetInstance().HandleDisconnectEventInner(
        nullptr, invalidScreenId, screenEvent, phyMirrorEnable);

    EXPECT_TRUE(g_logMsg.find("screenSession is nullptr") != std::string::npos);
    EXPECT_EQ(ScreenSessionManager::GetInstance().connectScreenNumber_, 5);
    auto session = ScreenSessionManager::GetInstance().GetScreenSession(invalidScreenId);
    EXPECT_EQ(session, nullptr);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: HandleDisconnectEventInner02
 * @tc.desc: test HandleDisconnectEventInner with concurrent user
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, HandleDisconnectEventInner02, TestSize.Level1)
{
    ScreenSceneConfig::isConcurrentUser_ = true;
    ASSERT_TRUE(CreatScreen(4));

    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(4);
    ASSERT_NE(screenSession, nullptr);

    ScreenSessionManager::GetInstance().HandleDisconnectEventInner(
        screenSession, 4, ScreenEvent::DISCONNECTED, false);

    ScreenSceneConfig::isConcurrentUser_ = false;
}

/**
 * @tc.name: HandleDisconnectEventInner03
 * @tc.desc: test HandleDisconnectEventInner without concurrent user
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, HandleDisconnectEventInner03, TestSize.Level1)
{
    ScreenSceneConfig::isConcurrentUser_ = false;
    ASSERT_TRUE(CreatScreen(5));

    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(5);
    ASSERT_NE(screenSession, nullptr);

    ScreenSessionManager::GetInstance().HandleDisconnectEventInner(
        screenSession, 5, ScreenEvent::DISCONNECTED, false);
}

/**
 * @tc.name: HandleProcessDisconnectEvent01
 * @tc.desc: test HandleProcessDisconnectEvent with phyMirrorEnable true
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, HandleProcessDisconnectEvent01, TestSize.Level1)
{
    ASSERT_TRUE(CreatScreen(6));

    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(6);
    ASSERT_NE(screenSession, nullptr);

    auto displayManagerAgentMock = sptr<DisplayManagerAgentMock>::MakeSptr();
    EXPECT_CALL(*displayManagerAgentMock, OnScreenDisconnect(_)).Times(1);
    EXPECT_CALL(*displayManagerAgentMock, OnDisplayDestroy(_)).Times(1);
    ScreenSessionManager::GetInstance().RegisterDisplayManagerAgent(
        displayManagerAgentMock, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);

    ScreenSessionManager::GetInstance().HandleProcessDisconnectEvent(
        screenSession, 6, ScreenEvent::DISCONNECTED);

    usleep(SLEEP_TIME_US);
    ScreenSessionManager::GetInstance().UnregisterDisplayManagerAgent(
        displayManagerAgentMock, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
}

/**
 * @tc.name: HandleProcessDisconnectEvent02
 * @tc.desc: test HandleProcessDisconnectEvent with concurrent user
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, HandleProcessDisconnectEvent02, TestSize.Level1)
{
    ScreenSceneConfig::isConcurrentUser_ = true;
    ASSERT_TRUE(CreatScreen(7));

    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(7);
    ASSERT_NE(screenSession, nullptr);

    auto displayManagerAgentMock = sptr<DisplayManagerAgentMock>::MakeSptr();
    EXPECT_CALL(*displayManagerAgentMock, OnScreenDisconnect(_)).Times(1);
    EXPECT_CALL(*displayManagerAgentMock, OnDisplayDestroy(_)).Times(1);
    ScreenSessionManager::GetInstance().RegisterDisplayManagerAgent(
        displayManagerAgentMock, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);

    ScreenSessionManager::GetInstance().HandleProcessDisconnectEvent(
        screenSession, 7, ScreenEvent::DISCONNECTED);

    usleep(SLEEP_TIME_US);
    ScreenSessionManager::GetInstance().UnregisterDisplayManagerAgent(
        displayManagerAgentMock, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    ScreenSceneConfig::isConcurrentUser_ = false;
}

/**
 * @tc.name: HandleProcessDisconnectEvent03
 * @tc.desc: test HandleProcessDisconnectEvent without phyMirrorEnable (SCREEN_ID_MAIN) and not concurrent user
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, HandleProcessDisconnectEvent03, TestSize.Level1)
{
    ScreenSceneConfig::isConcurrentUser_ = false;
    ASSERT_TRUE(CreatScreen(SCREEN_ID_MAIN));

    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_MAIN);
    ASSERT_NE(screenSession, nullptr);

    auto displayManagerAgentMock = sptr<DisplayManagerAgentMock>::MakeSptr();
    EXPECT_CALL(*displayManagerAgentMock, OnScreenDisconnect(_)).Times(0);
    EXPECT_CALL(*displayManagerAgentMock, OnDisplayDestroy(_)).Times(0);
    ScreenSessionManager::GetInstance().RegisterDisplayManagerAgent(
        displayManagerAgentMock, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);

    ScreenSessionManager::GetInstance().HandleProcessDisconnectEvent(
        screenSession, SCREEN_ID_MAIN, ScreenEvent::DISCONNECTED);

    usleep(SLEEP_TIME_US);
    ScreenSessionManager::GetInstance().UnregisterDisplayManagerAgent(
        displayManagerAgentMock, DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
}

/**
 * @tc.name: OnScreenChangeDefaultWithProcessDisconnected01
 * @tc.desc: test OnScreenChangeDefault with PROCESS_DISCONNECTED reason
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerConnectTest, OnScreenChangeDefaultWithProcessDisconnected01, TestSize.Level1)
{
    ScreenSceneConfig::isConcurrentUser_ = true;
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "virtualScreenProcess";
    ScreenId screenId = ScreenSessionManager::GetInstance().CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    ASSERT_NE(screenId, SCREEN_ID_INVALID);

    TriggerScreenChange(screenId, ScreenEvent::CONNECTED, ScreenChangeReason::DEFAULT);
    usleep(SLEEP_TIME_US);

    TriggerScreenChange(screenId, ScreenEvent::DISCONNECTED, ScreenChangeReason::PROCESS_DISCONNECTED);
    usleep(SLEEP_TIME_US);

    auto sessionAfterDestroy = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
    EXPECT_EQ(sessionAfterDestroy, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = false;
}

}  // namespace
}  // namespace Rosen
}  // namespace OHOS
