/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "screen_session_manager/include/screen_session_manager.h"
#include "display_manager_agent_default.h"
#include "iconsumer_surface.h"
#include "connection/screen_cast_connection.h"
#include "screen_scene_config.h"
#include <surface.h>
#include "scene_board_judgement.h"
#include "fold_screen_state_internel.h"
#include "common_test_utils.h"
#include "iremote_object_mocker.h"
#include "os_account_manager.h"
#include "screen_session_manager_client.h"
#include "../mock/mock_accesstoken_kit.h"
#include "test_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const int32_t CV_WAIT_SCREENOFF_MS = 1500;
const int32_t CV_WAIT_SCREENON_MS = 300;
const int32_t CV_WAIT_SCREENOFF_MS_MAX = 3500;
const uint32_t INVALID_DISPLAY_ORIENTATION = 99;
constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
constexpr int32_t CAST_WIRED_PROJECTION_START = 1005;
constexpr int32_t CAST_WIRED_PROJECTION_STOP = 1007;
bool g_isPcDevice = ScreenSceneConfig::GetExternalScreenDefaultMode() == "none";
const bool IS_SUPPORT_PC_MODE = system::GetBoolParameter("const.window.support_window_pcmode_switch", false);
std::string g_logMsg;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
    const char *msg)
{
    g_logMsg = msg;
}
const bool IS_SUPPORT_PC_MODE = system::GetBoolParameter("const.window.support_window_pcmode_switch", false);
}
class ScreenSessionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<ScreenSessionManager> ssm_;

    ScreenId DEFAULT_SCREEN_ID {0};
    ScreenId VIRTUAL_SCREEN_ID {2};
    ScreenId VIRTUAL_SCREEN_RS_ID {100};
    int32_t INVALID_USER_ID {1000};
    void SetAceessTokenPermission(const std::string processName);
    sptr<ScreenSession> InitTestScreenSession(std::string name, ScreenId &screenId);
    DMHookInfo CreateDefaultHookInfo();
};

sptr<ScreenSessionManager> ScreenSessionManagerTest::ssm_ = nullptr;

void ScreenSessionManagerTest::SetUpTestCase()
{
    ssm_ = new ScreenSessionManager();
    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("foundation", perms, 1);
}

void ScreenSessionManagerTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void ScreenSessionManagerTest::SetUp()
{
}

void ScreenSessionManagerTest::TearDown()
{
    usleep(SLEEP_TIME_IN_US);
}

sptr<ScreenSession> ScreenSessionManagerTest::InitTestScreenSession(std::string name, ScreenId &screenId)
{
    sptr displayManagerAgent = new (std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = name;
    screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(name, screenId, rsid, 0);
    return screenSession;
}

DMHookInfo ScreenSessionManagerTest::CreateDefaultHookInfo()
{
    uint32_t hookWidth = 500;
    uint32_t hookHeight = 700;
    float_t hookDensity = 3.0;
    uint32_t hookRotation = static_cast<uint32_t>(Rotation::ROTATION_0);
    uint32_t hookDisplayOrientation = static_cast<uint32_t>(DisplayOrientation::PORTRAIT);
    DMHookInfo dmHookInfo = { hookWidth, hookHeight, hookDensity, hookRotation, true, hookDisplayOrientation, true };
    return dmHookInfo;
}

namespace {
/**
 * @tc.name: RegisterDisplayManagerAgent
 * @tc.desc: RegisterDisplayManagerAgent test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, RegisterDisplayManagerAgent, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    DisplayManagerAgentType type = DisplayManagerAgentType::SCREEN_EVENT_LISTENER;
    EXPECT_NE(DMError::DM_ERROR_NOT_SYSTEM_APP, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    EXPECT_NE(DMError::DM_ERROR_NOT_SYSTEM_APP, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));

    type = DisplayManagerAgentType::PRIVATE_WINDOW_LISTENER;
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->RegisterDisplayManagerAgent(nullptr, type));
    EXPECT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->UnregisterDisplayManagerAgent(nullptr, type));

    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: CanEnterCoordinationRecording
 * @tc.desc: CanEnterCoordinationRecording test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CanEnterCoordinationRecording, TestSize.Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->virtualScreenCount_ = 1;
    ssm_->hdmiScreenCount_ = 0;

    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOptionRecord";
    virtualOption.virtualScreenType_ = VirtualScreenType::SCREEN_RECORDING;
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    auto ret = ssm_->CanEnterCoordination();
    EXPECT_EQ(ret, DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_RECORDING);
    ssm_->DestroyVirtualScreen(screenId)
}

/**
 * @tc.name: NotifyTentModeChange
 * @tc.desc: NotifyTentModeChange test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, NotifyTentModeChange, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    LOG_SetCallback(MyLogCallback);
    g_errLog.clear();
    ssm_->clientProxy_ = nullptr;
    ssm_->NotifyTentModeChange(TentMode::HOVER);
    EXPECT_TRUE(g_errLog.find("clientProxy null") != std::string::npos);
    g_errLog.clear();
    ssm_->clientProxy_ = sptr<ScreenSessionManagerClientTest>::MakeSptr();
    ssm_->NotifyTentModeChange(TentMode::HOVER);
    EXPECT_TRUE(g_errLog.find("clientProxy null") == std::string::npos);
    g_errLog.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: WakeupBegin
 * @tc.desc: WakeupBegin test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, WakeUpBegin, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    ScreenStateMachine::GetInstance().SetTransitionState(ScreenTransitionState::SCREEN_INIT);
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
    ASSERT_EQ(true, ssm_->WakeUpBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_SWITCH;
    ASSERT_EQ(true, ssm_->WakeUpBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT;
    ASSERT_EQ(true, ssm_->WakeUpBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS;
    ASSERT_EQ(true, ssm_->WakeUpBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON;
    ASSERT_EQ(true, ssm_->WakeUpBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_START_DREAM;
    ASSERT_EQ(false, ssm_->WakeUpBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_END_DREAM;
    ASSERT_EQ(true, ssm_->WakeUpBegin(reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: WakeUp for Multi
 * @tc.desc: WakeUp for Multi test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, WakeUpForMulti, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION;
    ASSERT_TRUE(ssm_->WakeUpBegin(reason));
    usleep(SLEEP_TIME_IN_US);
    ASSERT_TRUE(ssm_->WakeUpEnd());
    ASSERT_FALSE(ssm_->isMultiScreenCollaboration_);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SuspendBegin
 * @tc.desc: SuspendBegin test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SuspendBegin, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_HARD_KEY;
    ASSERT_EQ(true, ssm_->SuspendBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
    ASSERT_EQ(true, ssm_->SuspendBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_START_DREAM;
    ASSERT_EQ(true, ssm_->SuspendBegin(reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_END_DREAM;
    ASSERT_EQ(false, ssm_->SuspendBegin(reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: Suspend for Multi
 * @tc.desc: Suspend for Multi test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SuspendforMulti, Function | SmallTest | Level3)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION;
    ASSERT_TRUE(ssm_->SuspendBegin(reason));
    ASSERT_TRUE(ssm_->isMultiScreenCollaboration_);
    usleep(SLEEP_TIME_IN_US);
    ASSERT_TRUE(ssm_->SuspendEnd());
    ASSERT_FALSE(ssm_->isMultiScreenCollaboration_);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: GetInternalScreenId
 * @tc.desc: GetInternalScreenId test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetInternalScreenId, TestSize.Level1)
{
    ASSERT_EQ(SCREEN_ID_INVALID, ssm_->GetInternalScreenId());

    ScreenSessionConfig config;
    sptr<ScreenSession> session = new ScreenSession(config,
        ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    session->SetIsInternal(true);
    ScreenId id = 0;
    ssm_->screenSessionMap_.insert(std::make_pair(id, session));
    ASSERT_EQ(0, ssm_->GetInternalScreenId());

    ssm_->screenSessionMap_.erase(0);
}

/**
 * @tc.name: SetScreenPowerById01
 * @tc.desc: SetScreenPowerById test POWER_ON
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerById01, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerById(screenId, state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerById02
 * @tc.desc: SetScreenPowerById test POWER_OFF
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerById02, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
    ScreenPowerState state = ScreenPowerState::POWER_OFF;
    ASSERT_EQ(true, ssm_->SetScreenPowerById(screenId, state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerById03
 * @tc.desc: SetScreenPowerById test INVALID_STATE
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerById03, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
    ScreenPowerState state = ScreenPowerState::INVALID_STATE;
    ASSERT_EQ(false, ssm_->SetScreenPowerById(screenId, state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForAll
 * @tc.desc: SetScreenPowerForAll test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_SWITCH;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_HARD_KEY;
    state = ScreenPowerState::POWER_OFF;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    reason = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForAll01
 * @tc.desc: SetScreenPowerForAll01 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll01, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForAll02
 * @tc.desc: SetScreenPowerForAll02 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll02, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_RUNNING_LOCK;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForAll03
 * @tc.desc: SetScreenPowerForAll03 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll03, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_BATTERY;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForAll04
 * @tc.desc: SetScreenPowerForAll04 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll04, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_THERMAL;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForAll05
 * @tc.desc: SetScreenPowerForAll05 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll05, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_WORK;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForAll06
 * @tc.desc: SetScreenPowerForAll06 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll06, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_SYSTEM;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForAll07
 * @tc.desc: SetScreenPowerForAll07 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll07, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_APPLICATION;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForAll08
 * @tc.desc: SetScreenPowerForAll08 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll08, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_SETTINGS;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForAll09
 * @tc.desc: SetScreenPowerForAll09 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll09, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_HARD_KEY;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForAll10
 * @tc.desc: SetScreenPowerForAll10 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetScreenPowerForAll10, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    EXPECT_NE(displayManagerAgent, nullptr);

    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));

    VirtualScreenOption virtualOption;
    virtualOption.name_ = "createVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_TOUCH;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    ASSERT_EQ(true, ssm_->SetScreenPowerForAll(state, reason));

    EXPECT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: ScreenChange
 * @tc.desc: ScreenChange test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ScreenChange, TestSize.Level1)
{
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    ssm_->OnVirtualScreenChange(DEFAULT_SCREEN_ID, screenEvent);
    ssm_->OnVirtualScreenChange(VIRTUAL_SCREEN_ID, screenEvent);
    ssm_->OnScreenChange(DEFAULT_SCREEN_ID, screenEvent);
    ssm_->OnScreenChange(VIRTUAL_SCREEN_ID, screenEvent);
    EXPECT_TRUE(1);

    screenEvent = ScreenEvent::DISCONNECTED;
    ssm_->OnVirtualScreenChange(DEFAULT_SCREEN_ID, screenEvent);
    ssm_->OnVirtualScreenChange(VIRTUAL_SCREEN_ID, screenEvent);
    ssm_->OnScreenChange(DEFAULT_SCREEN_ID, screenEvent);
    ssm_->OnScreenChange(VIRTUAL_SCREEN_ID, screenEvent);
    EXPECT_TRUE(1);
}

/**
 * @tc.name: ScreenPower
 * @tc.desc: ScreenSesionManager screen power
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ScreenPower, TestSize.Level1)
{
    PowerStateChangeReason reason = PowerStateChangeReason::POWER_BUTTON;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    DisplayState displayState = DisplayState::ON;

    ASSERT_EQ(false, ssm_->WakeUpBegin(reason));
    ASSERT_EQ(false, ssm_->WakeUpEnd());

    ASSERT_EQ(false, ssm_->SuspendBegin(reason));
    ASSERT_EQ(false, ssm_->SuspendEnd());

    ASSERT_EQ(false, ssm_->SetScreenPowerForAll(state, reason));

    DisplayId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_EQ(true, ssm_->SetDisplayState(displayState));
    ASSERT_EQ(DisplayState::ON, ssm_->GetDisplayState(0));
}

/**
 * @tc.name: SyncScreenPowerState_StateON
 * @tc.desc: SyncScreenPowerState State is ON
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SyncScreenPowerState_StateON, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr) << "ScreenSessionManager instance is null";

    ssm_->SyncScreenPowerState(ScreenPowerState::POWER_ON);

    EXPECT_TRUE(g_logMsg.find("force sync power state") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SyncScreenPowerState_StateOFF
 * @tc.desc: SyncScreenPowerState State is OFF
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SyncScreenPowerState_StateOFF, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(ssm_, nullptr) << "ScreenSessionManager instance is null";

    ssm_->SyncScreenPowerState(ScreenPowerState::POWER_OFF);

    EXPECT_TRUE(g_logMsg.find("force sync power state") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetScreenPower
 * @tc.desc: GetScreenPower screen power
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenPower, TestSize.Level1)
{
    DisplayId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ssm_->GetScreenPower(0);
    EXPECT_TRUE(1);
}

/**
 * @tc.name: IsScreenRotationLocked
 * @tc.desc: IsScreenRotationLocked test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, IsScreenRotationLocked, TestSize.Level1)
{
    bool isLocked = false;
    DisplayId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_EQ(DMError::DM_OK, ssm_->IsScreenRotationLocked(isLocked));
}

/**
 * @tc.name: CreateScreenProperty
 * @tc.desc: CreateScreenProperty test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, CreateScreenProperty, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ScreenProperty property;
    ssm_->isDensityDpiLoad_ = false;
    ssm_->CreateScreenProperty(screenId, property);
    ssm_->isDensityDpiLoad_ = true;
    ASSERT_EQ(0, screenSession->GetScreenProperty().GetRefreshRate());
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetInternalWidth
 * @tc.desc: GetInternalWidth test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetInternalWidth, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    auto  screenId = ssm_->GetInternalScreenId();
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    ASSERT_EQ(screenSession, nullptr);
    ssm_->GetInternalWidth();
}

/**
 * @tc.name: InitExtendScreen01
 * @tc.desc: InitExtendScreen01 test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, InitExtendScreen01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    sptr<ScreenSession> screenSession1 = ssm_->GetScreenSession(ssm_->GetDefaultScreenId());
    ScreenProperty property = screenSession->GetScreenProperty();
    ssm_->InitExtendScreenProperty(screenId, screenSession, property);
    ssm_->InitExtendScreenDensity(screenSession, property);
    EXPECT_EQ(property.GetDensityInCurResolution(), screenSession1->GetScreenProperty().GetDensityInCurResolution());
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: InitExtendScreen
 * @tc.desc: InitExtendScreen test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, InitExtendScreen02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    sptr<ScreenSession> screenSession1 = ssm_->GetScreenSession(ssm_->GetDefaultScreenId());
    ScreenProperty property = screenSession->GetScreenProperty();
    screenSession->GetScreenProperty().SetScreenType(ScreenType::REAL);
    ssm_->InitExtendScreenDensity(screenSession, property);
    EXPECT_EQ(property.GetDensityInCurResolution(), screenSession1->GetScreenProperty().GetDensityInCurResolution());
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: SetExtendedScreenFallbackPlan
 * @tc.desc: SetExtendedScreenFallbackPlan test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetExtendedScreenFallbackPlan, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->SetExtendedScreenFallbackPlan(3030);
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    sptr<ScreenSession> screenSession = ssm_->GetScreenSession(screenId);
    screenSession->SetIsInternal(true);
    ssm_->SetExtendedScreenFallbackPlan(screenId);

    screenSession->SetIsInternal(false);
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    screenProperty.SetCurrentOffScreenRendering(false);
    ssm_->SetExtendedScreenFallbackPlan(screenId);

    screenProperty.SetCurrentOffScreenRendering(true);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    ASSERT_EQ(screenSession->displayNode_, nullptr);

    screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    ASSERT_EQ(screenSession->displayNode_, nullptr);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: BlockSetDisplayState
 * @tc.desc: BlockSetDisplayState test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, BlockSetDisplayState, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    EXPECT_TRUE(ssm_->BlockSetDisplayState());
}

/**
 * @tc.name: TryToCancelScreenOff
 * @tc.desc: TryToCancelScreenOff test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, TryToCancelScreenOff, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    bool ret = ssm_->sessionDisplayPowerController_->canCancelSuspendNotify_;
    bool ret1 = ssm_->gotScreenOffNotify_;
    bool ret2 = ssm_->needScreenOffNotify_;
    ssm_->sessionDisplayPowerController_->canCancelSuspendNotify_ = true;
    ASSERT_EQ(true, ssm_->TryToCancelScreenOff());

    ssm_->sessionDisplayPowerController_->canCancelSuspendNotify_ = false;
    ssm_->gotScreenOffNotify_ = true;
    ASSERT_EQ(false, ssm_->TryToCancelScreenOff());

    ssm_->gotScreenOffNotify_ = false;
    ssm_->needScreenOffNotify_ = false;
    ASSERT_EQ(false, ssm_->TryToCancelScreenOff());

    ssm_->needScreenOffNotify_ = true;
    ASSERT_EQ(true, ssm_->TryToCancelScreenOff());
    ssm_->sessionDisplayPowerController_->canCancelSuspendNotify_ = ret;
    ssm_->gotScreenOffNotify_ = ret1;
    ssm_->needScreenOffNotify_ = ret2;
}

/**
 * @tc.name: ForceSkipScreenOffAnimation
 * @tc.desc: ForceSkipScreenOffAnimation test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, ForceSkipScreenOffAnimation, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    bool ret = ssm_->sessionDisplayPowerController_->canCancelSuspendNotify_;
    bool ret1 = ssm_->gotScreenOffNotify_;
    bool ret2 = ssm_->needScreenOffNotify_;
    ssm_->sessionDisplayPowerController_->canCancelSuspendNotify_ = true;
    ssm_->ForceSkipScreenOffAnimation();
    ASSERT_TRUE(ssm_->sessionDisplayPowerController_->skipScreenOffBlock_);

    ssm_->sessionDisplayPowerController_->canCancelSuspendNotify_ = false;
    ssm_->gotScreenOffNotify_ = true;
    ssm_->ForceSkipScreenOffAnimation();
    ASSERT_TRUE(ssm_->sessionDisplayPowerController_->skipScreenOffBlock_);

    ssm_->gotScreenOffNotify_ = false;
    ssm_->needScreenOffNotify_ = false;
    ssm_->ForceSkipScreenOffAnimation();
    ASSERT_TRUE(ssm_->sessionDisplayPowerController_->skipScreenOffBlock_);

    ssm_->needScreenOffNotify_ = true;
    ssm_->ForceSkipScreenOffAnimation();
    ASSERT_TRUE(ssm_->sessionDisplayPowerController_->skipScreenOffBlock_);
    ssm_->sessionDisplayPowerController_->canCancelSuspendNotify_ = ret;
    ssm_->gotScreenOffNotify_ = ret1;
    ssm_->needScreenOffNotify_ = ret2;
}

/**
 * @tc.name: SetOrientation
 * @tc.desc: SetOrientation test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetOrientation, TestSize.Level1)
{
    Orientation orientation = Orientation::HORIZONTAL;
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_EQ(DMError::DM_OK, ssm_->SetOrientation(id, orientation, false));
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->SetOrientation(SCREEN_ID_INVALID, orientation, false));
    Orientation invalidOrientation = Orientation{20};
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, ssm_->SetOrientation(id, invalidOrientation, false));
}

/**
 * @tc.name: SetRotationFromWindow
 * @tc.desc: SetRotationFromWindow test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, SetRotationFromWindow, TestSize.Level1)
{
    Rotation targetRotation = Rotation::ROTATION_0;
    ScreenId id = 0;
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession(id, ScreenProperty(), 0);
    ssm_->screenSessionMap_[id] = screenSession;
    ASSERT_EQ(ssm_->SetRotationFromWindow(targetRotation), ssm_->SetRotation(id, targetRotation, true));
}

/**
 * @tc.name: GetDisplaySnapshot
 * @tc.desc: ScreenSesionManager screen shot
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDisplaySnapshot, TestSize.Level1)
{
    DisplayId displayId(0);
    DmErrorCode* errorCode = nullptr;
    ssm_->GetDisplaySnapshot(displayId, errorCode, false, false);
    EXPECT_TRUE(1);
}

/**
 * @tc.name: VirtualScreen
 * @tc.desc: ScreenSesionManager virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, VirtualScreen, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testVirtualOption";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    std::vector<ScreenId> mirrorScreenIds;
    ScreenId mainScreenId(DEFAULT_SCREEN_ID);
    ScreenId screenGroupId{1};
    ASSERT_EQ(DMError::DM_ERROR_NULLPTR, ssm_->MakeMirror(mainScreenId, mirrorScreenIds, screenGroupId));
    mirrorScreenIds.push_back(VIRTUAL_SCREEN_ID);
    ASSERT_NE(DMError::DM_OK, ssm_->MakeMirror(mainScreenId, mirrorScreenIds, screenGroupId));

    mirrorScreenIds.push_back(screenId);
    ASSERT_EQ(DMError::DM_OK, ssm_->MakeMirror(mainScreenId, mirrorScreenIds, screenGroupId));

    auto result1 = ssm_->SetVirtualScreenSurface(screenId, nullptr);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, result1);
    sptr<IConsumerSurface> surface = OHOS::IConsumerSurface::Create();
    auto result2 = ssm_->SetVirtualScreenSurface(screenId, surface->GetProducer());
    ASSERT_EQ(DMError::DM_OK, result2);
    ASSERT_EQ(DMError::DM_OK, ssm_->DestroyVirtualScreen(screenId));
}

/**
 * @tc.name: AutoRotate
 * @tc.desc: ScreenSesionManager SetVirtualMirrorScreenCanvasRotation test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, AutoRotate, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testAutoRotate";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }

    auto result1 = ssm_->SetVirtualMirrorScreenCanvasRotation(screenId, true);
    ASSERT_EQ(DMError::DM_OK, result1);
    auto result2 = ssm_->SetVirtualMirrorScreenCanvasRotation(screenId, false);
    ASSERT_EQ(DMError::DM_OK, result2);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetScreenSession
 * @tc.desc: GetScreenSession virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetScreenSession, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetScreenSession";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("GetScreenSession", screenId, rsid, 0);
    ASSERT_NE(ssm_->GetScreenSession(screenId), screenSession);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetDefaultScreenSession
 * @tc.desc: GetDefaultScreenSession virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDefaultScreenSession, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetDefaultScreenSession";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("GetDefaultScreenSession", screenId, rsid, 0);
    ASSERT_NE(ssm_->GetDefaultScreenSession(), screenSession);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: GetDefaultDisplayInfo
 * @tc.desc: GetDefaultDisplayInfo virtual screen
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, GetDefaultDisplayInfo, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetDefaultDisplayInfo";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    if (screenId != VIRTUAL_SCREEN_ID) {
        ASSERT_TRUE(screenId != VIRTUAL_SCREEN_ID);
    }
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("GetDefaultDisplayInfo", screenId, rsid, 0);
    sptr<DisplayInfo> displayInfo = new DisplayInfo();
    if (ssm_->GetScreenSession(screenId) == nullptr) {
        ASSERT_EQ(ssm_->GetDefaultDisplayInfo(), nullptr);
    }
    ASSERT_NE(ssm_->GetScreenSession(screenId), nullptr);
    ASSERT_NE(ssm_->GetDefaultDisplayInfo(), displayInfo);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: HookDisplayInfoByUid
 * @tc.desc: HookDisplayInfo by uid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HookDisplayInfoByUid, TestSize.Level1)
{
    sptr<IDisplayManagerAgent> displayManagerAgent = new DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "GetDefaultScreenSession";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());
    auto rsid = ssm_->screenIdManager_.ConvertToRsScreenId(screenId);
    sptr<ScreenSession> screenSession = new (std::nothrow) ScreenSession("GetDefaultDisplayInfo", screenId, rsid, 0);
    sptr<DisplayInfo> displayInfo = ssm_->GetDefaultDisplayInfo();
    ASSERT_NE(ssm_->GetScreenSession(screenId), nullptr);
    ASSERT_NE(displayInfo, nullptr);
    uint32_t uid = 20020001;
    DMHookInfo dmHookInfo;
    std::map<uint32_t, DMHookInfo> displayHookMap = {};
    displayHookMap[uid] = dmHookInfo;
    ASSERT_EQ(displayHookMap.find(uid) != displayHookMap.end(), true);
    displayInfo->SetWidth(100);
    ASSERT_EQ(displayInfo->GetWidth(), 100);
    displayInfo->SetHeight(100);
    ASSERT_EQ(displayInfo->GetHeight(), 100);
    displayInfo->SetVirtualPixelRatio(1.0);
    ASSERT_EQ(displayInfo->GetVirtualPixelRatio(), 1.0);
    displayInfo->SetRotation(Rotation::ROTATION_0);
    ASSERT_EQ(displayInfo->GetRotation(), Rotation::ROTATION_0);
    displayInfo->SetDisplayOrientation(DisplayOrientation::PORTRAIT);
    ASSERT_EQ(displayInfo->GetDisplayOrientation(), DisplayOrientation::PORTRAIT);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: HookDisplayInfoByUid02
 * @tc.desc: HookDisplayInfo by uid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HookDisplayInfoByUid02, TestSize.Level1)
{
    ScreenId screenId;
    sptr<ScreenSession> screenSession = InitTestScreenSession("HookDisplayInfoByUid02", screenId);
    ASSERT_NE(ssm_->GetScreenSession(screenId), nullptr);
    sptr<DisplayInfo> displayInfo = ssm_->GetDefaultDisplayInfo();
    ASSERT_NE(displayInfo, nullptr);
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    ssm_->displayHookMap_[uid] = dmHookInfo;
    EXPECT_NE(ssm_->displayHookMap_.find(uid), ssm_->displayHookMap_.end());
    displayInfo = ssm_->HookDisplayInfoByUid(displayInfo, screenSession);
    EXPECT_EQ(displayInfo->GetWidth(), dmHookInfo.width_);
    EXPECT_EQ(displayInfo->GetHeight(), dmHookInfo.height_);
    EXPECT_EQ(displayInfo->GetVirtualPixelRatio(), dmHookInfo.density_);
    EXPECT_EQ(static_cast<uint32_t>(displayInfo->GetRotation()), dmHookInfo.rotation_);
    EXPECT_EQ(static_cast<uint32_t>(displayInfo->GetDisplayOrientation()), dmHookInfo.displayOrientation_);
    ssm_->displayHookMap_.erase(uid);
    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: HookDisplayInfoByUid03
 * @tc.desc: HookDisplayInfo by uid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerTest, HookDisplayInfoByUid03, TestSize.Level1)
{
    ScreenId screenId;
    sptr<ScreenSession> screenSession = InitTestScreenSession("HookDisplayInfoByUid03", screenId);
    ASSERT_NE(ssm_->GetScreenSession(screenId), nullptr);
    sptr<DisplayInfo> displayInfo = ssm_->GetDefaultDisplayInfo();
    ASSERT_NE(displayInfo, nullptr);
    uint32_t uid = getuid();
    DMHookInfo dmHookInfo = CreateDefaultHookInfo();
    dmHookInfo.enableHookDisplayOrientation_ = false;
    dmHookInfo.displayOrientation_ = INVALID_DISPLAY_ORIENTATION;
    ssm_->displayHookMap_[uid] = dmHookInfo;
    EXPECT_NE(ssm_->displayHookMap_.find(uid), ssm_->displayHookMap_.end());
    displayInfo = ssm_->HookDisplayInfoByUid(displayInfo, screenSession);
    EXPECT_EQ(displayInfo->GetWidth(), dmHookInfo.width_);
    EXPECT_EQ(displayInfo->GetHeight(), dmHookInfo.height_);
    EXPECT_EQ(displayInfo->GetVirtualPixelRatio(), dmHookInfo.density_);
    EXPECT_EQ(static_cast<uint32_t>(displayInfo->GetRotation()), dmHookInfo.rotation_);
    EXPECT_NE(static_cast<uint32_t>(displayInfo->GetDisplayOrientation()), dmHookInfo.displayOrientation_);
    ssm_->displayHookMap_.erase(uid);
    ssm_->DestroyVirtualScreen(screenId);
}
}
} // namespace Rosen
} // namespace OHOS
