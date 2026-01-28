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

#include <gtest/gtest.h>

#include "multi_screen_change_utils.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "display_manager_agent_default.h"
#include "zidl/screen_session_manager_client_interface.h"
#include "common_test_utils.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
}
namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
}
class MultiScreenChangeUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void SetAceessTokenPermission(const std::string processName);
    static MultiScreenChangeUtils& multiSCU_;
    static ScreenSessionManager& ssm_;
};

ScreenSessionManager& MultiScreenChangeUtilsTest::ssm_ =
    ScreenSessionManager::GetInstance();

void MultiScreenChangeUtilsTest::SetUpTestCase()
{
    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("foundation", perms, 1);
}

void MultiScreenChangeUtilsTest::TearDownTestCase()
{
    usleep(SLEEP_TIME_IN_US);
}

void MultiScreenChangeUtilsTest::SetUp()
{
}

void MultiScreenChangeUtilsTest::TearDown()
{
}

namespace {

/**
 * @tc.name: ScreenPropertyChangeNotify
 * @tc.desc: ScreenPropertyChangeNotify func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ScreenPropertyChangeNotify, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.ScreenPropertyChangeNotify(innerScreen, externalScreen);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenPropertyChangeNotify(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ScreenPropertyChangeNotify(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenPropertyChangeNotify(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: ScreenDensityChangeNotify
 * @tc.desc: ScreenDensityChangeNotify func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ScreenDensityChangeNotify, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.ScreenDensityChangeNotify(innerScreen, externalScreen);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenDensityChangeNotify(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ScreenDensityChangeNotify(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenDensityChangeNotify(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: ScreenExtendPositionChange
 * @tc.desc: ScreenExtendPositionChange func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ScreenExtendPositionChange, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.ScreenExtendPositionChange(innerScreen, externalScreen);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenExtendPositionChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ScreenExtendPositionChange(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenExtendPositionChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: SetScreenAvailableStatus
 * @tc.desc: SetScreenAvailableStatus func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, SetScreenAvailableStatus, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = nullptr;
    bool isScreenAvailable = true;

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    multiSCU_.SetScreenAvailableStatus(screenSession, isScreenAvailable);
    screenSession = new ScreenSession();
    multiSCU_.SetScreenAvailableStatus(screenSession, isScreenAvailable);
    EXPECT_TRUE(g_logMsg.find("screenSession is null") == std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: ScreenMainPositionChange
 * @tc.desc: ScreenMainPositionChange func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ScreenMainPositionChange, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.ScreenMainPositionChange(innerScreen, externalScreen);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenMainPositionChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ScreenMainPositionChange(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenMainPositionChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: SetExternalScreenOffScreenRendering
 * @tc.desc: SetExternalScreenOffScreenRendering func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, SetExternalScreenOffScreenRendering, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.SetExternalScreenOffScreenRendering(innerScreen, externalScreen);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.SetExternalScreenOffScreenRendering(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.SetExternalScreenOffScreenRendering(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.SetExternalScreenOffScreenRendering(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: ScreenCombinationChange
 * @tc.desc: ScreenCombinationChange func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ScreenCombinationChange, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    ScreenCombination externalCombination = ScreenCombination::SCREEN_MIRROR;
    multiSCU_.ScreenCombinationChange(innerScreen, externalScreen, externalCombination);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenCombinationChange(innerScreen, externalScreen, externalCombination);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ScreenCombinationChange(innerScreen, externalScreen, externalCombination);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenCombinationChange(innerScreen, externalScreen, externalCombination);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: ScreenSerialNumberChange
 * @tc.desc: ScreenSerialNumberChange func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ScreenSerialNumberChange, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.ScreenSerialNumberChange(innerScreen, externalScreen);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenSerialNumberChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ScreenSerialNumberChange(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenSerialNumberChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: ScreenActiveModesChange
 * @tc.desc: ScreenActiveModesChange func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ScreenActiveModesChange, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.ScreenActiveModesChange(innerScreen, externalScreen);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenActiveModesChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ScreenActiveModesChange(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenActiveModesChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: ScreenRSIdChange
 * @tc.desc: ScreenRSIdChange func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ScreenRSIdChange, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.ScreenRSIdChange(innerScreen, externalScreen);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenRSIdChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ScreenRSIdChange(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenRSIdChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: ScreenNameChange
 * @tc.desc: ScreenNameChange func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ScreenNameChange, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.ScreenNameChange(innerScreen, externalScreen);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenNameChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ScreenNameChange(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenNameChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: ScreenPropertyChange
 * @tc.desc: ScreenPropertyChange func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ScreenPropertyChange, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.ScreenPropertyChange(innerScreen, externalScreen);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenPropertyChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ScreenPropertyChange(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenPropertyChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: ExchangeScreenSupportedRefreshRate
 * @tc.desc: ExchangeScreenSupportedRefreshRate func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ExchangeScreenSupportedRefreshRate, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.ExchangeScreenSupportedRefreshRate(innerScreen, externalScreen);
 
    sptr<IDisplayManagerAgent> displayManagerAgent = sptr<DisplayManagerAgentDefault>::MakeSptr();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ExchangeScreenSupportedRefreshRate(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
 
    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ExchangeScreenSupportedRefreshRate(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);
 
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ExchangeScreenSupportedRefreshRate(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: ExchangeScreenSupportedRefreshRate01
 * @tc.desc: ExchangeScreenSupportedRefreshRate func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ExchangeScreenSupportedRefreshRate01, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.ExchangeScreenSupportedRefreshRate(innerScreen, externalScreen);
 
    sptr<IDisplayManagerAgent> displayManagerAgent = sptr<DisplayManagerAgentDefault>::MakeSptr();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    ssm_.GetOrCreatePhysicalScreenSession(screenId);
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ExchangeScreenSupportedRefreshRate(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
 
    ssm_.GetOrCreatePhysicalScreenSession(0);
    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ExchangeScreenSupportedRefreshRate(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);
 
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ExchangeScreenSupportedRefreshRate(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: ScreenPhysicalInfoChange
 * @tc.desc: ScreenPhysicalInfoChange func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, ScreenPhysicalInfoChange, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.ScreenPhysicalInfoChange(innerScreen, externalScreen);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenPhysicalInfoChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.ScreenPhysicalInfoChange(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.ScreenPhysicalInfoChange(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: CreateMirrorSession
 * @tc.desc: CreateMirrorSession func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, CreateMirrorSession, TestSize.Level1)
{
    sptr<ScreenSession> innerScreen = nullptr;
    sptr<ScreenSession> externalScreen = nullptr;
    multiSCU_.CreateMirrorSession(innerScreen, externalScreen);

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    ScreenId screenId = ssm_.CreateVirtualScreen(
        virtualOption, displayManagerAgent->AsObject());
    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.CreateMirrorSession(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);

    externalScreen = ssm_.GetOrCreateScreenSession(0);
    innerScreen = nullptr;
    multiSCU_.CreateMirrorSession(innerScreen, externalScreen);
    ASSERT_NE(externalScreen, nullptr);

    innerScreen = ssm_.GetScreenSession(screenId);
    multiSCU_.CreateMirrorSession(innerScreen, externalScreen);
    ASSERT_NE(innerScreen, nullptr);
}

/**
 * @tc.name: CreateExtendSession
 * @tc.desc: CreateExtendSession func
 * @tc.type: FUNC
 */
HWTEST_F(MultiScreenChangeUtilsTest, CreateExtendSession, TestSize.Level1)
{
    sptr<ScreenSession> screenSession = nullptr;
    multiSCU_.CreateExtendSession(screenSession);

    screenSession = new ScreenSession();
    ASSERT_NE(screenSession, nullptr);
    multiSCU_.CreateExtendSession(screenSession);
}
}
} // namespace Rosen
} // namespace OHOS
