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
#include "interfaces/include/ws_common.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_manager_agent.h"
#include "session_manager.h"
#include "zidl/window_manager_agent_interface.h"
#include "screen_session_manager/include/screen_session_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

const uint32_t SCREEN_STATE_FSM_DELAY_US = 100000;

class ScreenStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<ScreenStateMachine> fsm_;
};

sptr<ScreenStateMachine> ScreenStateMachineTest::fsm_ = nullptr;

void ScreenStateMachineTest::SetUpTestCase()
{
    fsm_ = new ScreenStateMachine();
}

void ScreenStateMachineTest::TearDownTestCase()
{
    fsm_ = nullptr;
}

void ScreenStateMachineTest::SetUp()
{
}

void ScreenStateMachineTest::TearDown()
{
    usleep(SCREEN_STATE_FSM_DELAY_US);
}

namespace {

/**
 * @tc.name: ConstructorSetsInitialState
 * @tc.desc: ConstructorSetsInitialState func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenStateMachineTest, ConstructorSetsInitialState, TestSize.Level1)
{
    EXPECT_EQ(fsm_->GetTransitionState(), ScreenTransitionState::SCREEN_INIT);
}

/**
 * @tc.name: InitStateMachineTest01
 * @tc.desc: InitStateMachine func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenStateMachineTest, InitStateMachineTest01, TestSize.Level1)
{
    uint32_t refCnt = 0;
    fsm_->InitStateMachine(refCnt);
    EXPECT_EQ(fsm_->initRefCnt_, refCnt);
    EXPECT_EQ(fsm_->GetTransitionState(), ScreenTransitionState::SCREEN_ON);

    refCnt = 1;
    fsm_->InitStateMachine(refCnt);
    EXPECT_EQ(fsm_->initRefCnt_, refCnt);
    EXPECT_FALSE(fsm_->stateMachine_.empty());
}

/**
 * @tc.name: SetTransitionStateTest01
 * @tc.desc: SetTransitionState func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenStateMachineTest, SetTransitionStateTest01, TestSize.Level1)
{
    fsm_->SetTransitionState(ScreenTransitionState::SCREEN_OFF);
    EXPECT_EQ(fsm_->GetTransitionState(), ScreenTransitionState::SCREEN_OFF);

    fsm_->SetTransitionState(ScreenTransitionState::SCREEN_ON);
    EXPECT_EQ(fsm_->GetTransitionState(), ScreenTransitionState::SCREEN_ON);
}

/**
 * @tc.name: TransferStateTest01
 * @tc.desc: TransferState func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenStateMachineTest, TransferStateTest01, TestSize.Level1)
{
    fsm_->TransferState(ScreenTransitionState::SCREEN_OFF);
    EXPECT_EQ(fsm_->GetTransitionState(), ScreenTransitionState::SCREEN_OFF);

    fsm_->TransferState(ScreenTransitionState::SCREEN_ON);
    EXPECT_EQ(fsm_->GetTransitionState(), ScreenTransitionState::SCREEN_ON);
}

/**
 * @tc.name: SetCurrentPowerStatusTest01
 * @tc.desc: SetCurrentPowerStatus func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenStateMachineTest, SetCurrentPowerStatusTest01, TestSize.Level1)
{
    fsm_->SetCurrentPowerStatus(ScreenPowerStatus::POWER_STATUS_ON);
    EXPECT_EQ(fsm_->GetCurrentPowerStatus(), ScreenPowerStatus::POWER_STATUS_ON);

    fsm_->SetCurrentPowerStatus(ScreenPowerStatus::POWER_STATUS_OFF);
    EXPECT_EQ(fsm_->GetCurrentPowerStatus(), ScreenPowerStatus::POWER_STATUS_OFF);
}

/**
 * @tc.name: HandlePowerStateChangeTestInvalidTransition
 * @tc.desc: HandlePowerStateChange func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenStateMachineTest, HandlePowerStateChangeTestInvalidTransition, TestSize.Level1)
{
    fsm_->TransferState(ScreenTransitionState::SCREEN_OFF);
    ScreenPowerInfoType type;
    EXPECT_FALSE(fsm_->HandlePowerStateChange(ScreenPowerEvent::SCREEN_POWER_EVENT_MAX, type));
}

}
} // namespace Rosen
} // namespace OHOS