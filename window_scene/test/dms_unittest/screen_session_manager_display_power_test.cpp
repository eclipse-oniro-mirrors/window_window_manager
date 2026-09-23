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

#include "screen_session_manager/include/screen_session_manager.h"
#include "screen_session_manager/include/screen_session_manager_adapter.h"
#include "display_manager_agent_default.h"
#include "screen_scene_config.h"
#include "common_test_utils.h"
#include "mock/mock_session_permission.h"
#include "../mock/mock_accesstoken_kit.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 100000;
std::string g_logMsg;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
    const char *msg)
{
    g_logMsg += msg;
}
}

class ScreenSessionManagerDisplayPowerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<ScreenSessionManager> ssm_;
    DisplayId DEFAULT_SCREEN_ID {0};
};

sptr<ScreenSessionManager> ScreenSessionManagerDisplayPowerTest::ssm_ = nullptr;

void ScreenSessionManagerDisplayPowerTest::SetUpTestCase()
{
    ssm_ = new ScreenSessionManager();
    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("foundation", perms, 1);
}

void ScreenSessionManagerDisplayPowerTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void ScreenSessionManagerDisplayPowerTest::SetUp()
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
}

void ScreenSessionManagerDisplayPowerTest::TearDown()
{
    LOG_SetCallback(nullptr);
    usleep(SLEEP_TIME_IN_US);
}

namespace {

/**
 * @tc.name: WakeUpBeginWithScreenId_PermissionDenied
 * @tc.desc: WakeUpBegin with displayId returns false when permission denied
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, WakeUpBeginWithScreenId_PermissionDenied, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    MockSessionPermission::MockIsStartByHdcd(false);
    DisplayId displayId = 0;
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    bool ret = ssm_->WakeUpBegin(displayId, reason);
    EXPECT_FALSE(ret);
    MockSessionPermission::MockIsStartByHdcd(true);
}

/**
 * @tc.name: WakeUpBeginWithScreenId_InvalidScreenId
 * @tc.desc: WakeUpBegin with displayId returns false when displayId is invalid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, WakeUpBeginWithScreenId_InvalidScreenId, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    DisplayId invalidId = 9999;
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    bool ret = ssm_->WakeUpBegin(invalidId, reason);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("Invalid displayId") != std::string::npos);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: WakeUpBeginWithScreenId_Success
 * @tc.desc: WakeUpBegin with displayId returns true on success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, WakeUpBeginWithScreenId_Success, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    ssm_->WakeUpBegin(DEFAULT_SCREEN_ID, reason);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: WakeUpEndWithScreenId_PermissionDenied
 * @tc.desc: WakeUpEnd with displayId returns false when permission denied
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, WakeUpEndWithScreenId_PermissionDenied, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    MockSessionPermission::MockIsStartByHdcd(false);
    DisplayId displayId = 0;
    bool ret = ssm_->WakeUpEnd(displayId);
    EXPECT_FALSE(ret);
    MockSessionPermission::MockIsStartByHdcd(true);
}

/**
 * @tc.name: WakeUpEndWithScreenId_InvalidScreenId
 * @tc.desc: WakeUpEnd with displayId returns false when displayId is invalid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, WakeUpEndWithScreenId_InvalidScreenId, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    DisplayId invalidId = 9999;
    bool ret = ssm_->WakeUpEnd(invalidId);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("Invalid displayId") != std::string::npos);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SuspendBeginWithScreenId_PermissionDenied
 * @tc.desc: SuspendBegin with displayId returns false when permission denied
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, SuspendBeginWithScreenId_PermissionDenied, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    MockSessionPermission::MockIsStartByHdcd(false);
    DisplayId displayId = 0;
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    bool ret = ssm_->SuspendBegin(displayId, reason);
    EXPECT_FALSE(ret);
    MockSessionPermission::MockIsStartByHdcd(true);
}

/**
 * @tc.name: SuspendBeginWithScreenId_InvalidScreenId
 * @tc.desc: SuspendBegin with displayId returns false when displayId is invalid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, SuspendBeginWithScreenId_InvalidScreenId, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    DisplayId invalidId = 9999;
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    bool ret = ssm_->SuspendBegin(invalidId, reason);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("invalid displayId") != std::string::npos);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SuspendBeginWithScreenId_Success
 * @tc.desc: SuspendBegin with displayId returns true on success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, SuspendBeginWithScreenId_Success, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    ssm_->SuspendBegin(DEFAULT_SCREEN_ID, reason);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SuspendEndWithScreenId_PermissionDenied
 * @tc.desc: SuspendEnd with displayId returns false when permission denied
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, SuspendEndWithScreenId_PermissionDenied, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    MockSessionPermission::MockIsStartByHdcd(false);
    DisplayId displayId = 0;
    bool ret = ssm_->SuspendEnd(displayId);
    EXPECT_FALSE(ret);
    MockSessionPermission::MockIsStartByHdcd(true);
}

/**
 * @tc.name: SuspendEndWithScreenId_InvalidScreenId
 * @tc.desc: SuspendEnd with displayId returns false when displayId is invalid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, SuspendEndWithScreenId_InvalidScreenId, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    DisplayId invalidId = 9999;
    bool ret = ssm_->SuspendEnd(invalidId);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("invalid displayId") != std::string::npos);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetDisplayStateWithScreenId_PermissionDenied
 * @tc.desc: SetDisplayState with displayId returns false when permission denied
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, SetDisplayStateWithScreenId_PermissionDenied, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    MockSessionPermission::MockIsStartByHdcd(false);
    DisplayId displayId = 0;
    DisplayState state = DisplayState::ON;
    bool ret = ssm_->SetDisplayState(displayId, state);
    EXPECT_FALSE(ret);
    MockSessionPermission::MockIsStartByHdcd(true);
}

/**
 * @tc.name: SetDisplayStateWithScreenId_InvalidId
 * @tc.desc: SetDisplayState with displayId returns false when displayId is invalid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, SetDisplayStateWithScreenId_InvalidId, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    DisplayId invalidId = 9999;
    DisplayState state = DisplayState::ON;
    bool ret = ssm_->SetDisplayState(invalidId, state);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("invalid displayId") != std::string::npos);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetDisplayStateWithScreenId_Success
 * @tc.desc: SetDisplayState with displayId returns true on success
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, SetDisplayStateWithScreenId_Success, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    DisplayState state = DisplayState::ON;
    ssm_->SetDisplayState(DEFAULT_SCREEN_ID, state);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForSpecifiedId_PermissionDenied
 * @tc.desc: SetScreenPowerForSpecifiedId returns false when permission denied
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, SetScreenPowerForSpecifiedId_PermissionDenied, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    MockSessionPermission::MockIsStartByHdcd(false);
    DisplayId displayId = 0;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    bool ret = ssm_->SetScreenPowerForSpecifiedId(displayId, state, reason);
    EXPECT_FALSE(ret);
    MockSessionPermission::MockIsStartByHdcd(true);
}

/**
 * @tc.name: SetScreenPowerForSpecifiedId_UnsupportedState
 * @tc.desc: SetScreenPowerForSpecifiedId returns false for unsupported state
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, SetScreenPowerForSpecifiedId_UnsupportedState, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    DisplayId displayId = 0;
    ScreenPowerState unsupportedState = static_cast<ScreenPowerState>(999);
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    bool ret = ssm_->SetScreenPowerForSpecifiedId(displayId, unsupportedState, reason);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("state not support") != std::string::npos);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForSpecifiedId_PowerOn_Success
 * @tc.desc: SetScreenPowerForSpecifiedId returns true for POWER_ON state
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, SetScreenPowerForSpecifiedId_PowerOn_Success, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    DisplayId displayId = 0;
    ScreenPowerState state = ScreenPowerState::POWER_ON;
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    ssm_->SetScreenPowerForSpecifiedId(displayId, state, reason);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: SetScreenPowerForSpecifiedId_PowerOff_Success
 * @tc.desc: SetScreenPowerForSpecifiedId returns true for POWER_OFF state
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, SetScreenPowerForSpecifiedId_PowerOff_Success, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    DisplayId displayId = 0;
    ScreenPowerState state = ScreenPowerState::POWER_OFF;
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    ssm_->SetScreenPowerForSpecifiedId(displayId, state, reason);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: DoWakeUpBegin_NotifyFailed
 * @tc.desc: DoWakeUpBegin returns false when NotifyDisplayPowerEvent fails
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, DoWakeUpBegin_NotifyFailed, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    ScreenSessionManagerAdapter::GetInstance().dmAgentContainer_.UnregisterAgent(displayManagerAgent, type);
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    bool ret = ssm_->DoWakeUpBegin(DEFAULT_SCREEN_ID, reason);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("Agents failed to notify") != std::string::npos);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: DoSuspendBegin_NotifyFailed
 * @tc.desc: DoSuspendBegin returns false when NotifyDisplayPowerEvent fails
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, DoSuspendBegin_NotifyFailed, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    ScreenSessionManagerAdapter::GetInstance().dmAgentContainer_.UnregisterAgent(displayManagerAgent, type);
    PowerStateChangeReason reason = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    bool ret = ssm_->DoSuspendBegin(DEFAULT_SCREEN_ID, reason);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("Agents failed to notify") != std::string::npos);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: NotifySpecifiedDisplayPowerEvent_NotifyFailed
 * @tc.desc: NotifySpecifiedDisplayPowerEvent returns false when NotifyDisplayPowerEvent fails
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, NotifySpecifiedDisplayPowerEvent_NotifyFailed, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    ScreenSessionManagerAdapter::GetInstance().dmAgentContainer_.UnregisterAgent(displayManagerAgent, type);
    bool ret = ssm_->NotifySpecifiedDisplayPowerEvent(DEFAULT_SCREEN_ID, DisplayPowerEvent::WAKE_UP,
        EventStatus::BEGIN, PowerStateChangeReason::STATE_CHANGE_REASON_INIT);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(g_logMsg.find("Agents failed to notify") != std::string::npos);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

/**
 * @tc.name: DoSetDisplayState_InvalidId
 * @tc.desc: DoSetDisplayState returns false for invalid displayId (no ScreenSession)
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDisplayPowerTest, DoSetDisplayState_InvalidId, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new(std::nothrow) DisplayManagerAgentDefault();
    ASSERT_NE(displayManagerAgent, nullptr);
    DisplayManagerAgentType type = DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER;
    EXPECT_EQ(DMError::DM_OK, ssm_->RegisterDisplayManagerAgent(displayManagerAgent, type));
    DisplayId invalidId = 9999;
    DisplayState state = DisplayState::ON;
    bool ret = ssm_->DoSetDisplayState(invalidId, state);
    EXPECT_FALSE(ret);
    EXPECT_EQ(DMError::DM_OK, ssm_->UnregisterDisplayManagerAgent(displayManagerAgent, type));
}

} // namespace
} // namespace Rosen
} // namespace OHOS
