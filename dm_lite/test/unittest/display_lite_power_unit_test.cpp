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
#include "display_manager_lite.h"
#include "screen_manager_lite.h"
#include "mock_display_manager_adapter_lite.h"
#include "singleton_mocker.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<DisplayManagerAdapterLite, MockDisplayManagerAdapterLite>;

class DisplayLitePowerUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static inline DisplayId defaultId_ = 0;
    static inline uint32_t brightnessLevel_ = 80;
    static inline ScreenPowerState initialPowerState_;
    static inline DisplayState initialState_;
};

void DisplayLitePowerUnitTest::SetUpTestCase()
{
}

void DisplayLitePowerUnitTest::TearDownTestCase()
{
}

void DisplayLitePowerUnitTest::SetUp()
{
    initialPowerState_ = ScreenManagerLite::GetInstance().GetScreenPower(defaultId_);
    initialState_ = DisplayManagerLite::GetInstance().GetDisplayState(defaultId_);
}

void DisplayLitePowerUnitTest::TearDown()
{
    ScreenManagerLite::GetInstance().SetScreenPowerForAll(initialPowerState_, PowerStateChangeReason::POWER_BUTTON);
    DisplayStateCallback callback;
    DisplayManagerLite::GetInstance().SetDisplayState(initialState_, callback);
}

namespace {
/**
 * @tc.name: wake_up_begin_001
 * @tc.desc: call WakeUpBegin and check return value
 * @tc.type: FUNC
 */
HWTEST_F(DisplayLitePowerUnitTest, wake_up_begin_001, TestSize.Level1)
{
    Mocker m;
    EXPECT_CALL(m.Mock(), WakeUpBegin(PowerStateChangeReason::POWER_BUTTON)).Times(1).WillOnce(Return(true));
    bool ret  = DisplayManagerLite::GetInstance().WakeUpBegin(PowerStateChangeReason::POWER_BUTTON);
    ASSERT_EQ(true, ret);

    EXPECT_CALL(m.Mock(), WakeUpBegin(PowerStateChangeReason::POWER_BUTTON)).Times(1).WillOnce(Return(false));
    ret  = DisplayManagerLite::GetInstance().WakeUpBegin(PowerStateChangeReason::POWER_BUTTON);
    ASSERT_EQ(false, ret);
}

/**
 * @tc.name: wake_up_end_001
 * @tc.desc: call WakeUpEnd and check return value
 * @tc.type: FUNC
 */
HWTEST_F(DisplayLitePowerUnitTest, wake_up_end_001, TestSize.Level1)
{
    Mocker m;
    EXPECT_CALL(m.Mock(), WakeUpEnd()).Times(1).WillOnce(Return(true));
    bool ret  = DisplayManagerLite::GetInstance().WakeUpEnd();
    ASSERT_EQ(true, ret);

    EXPECT_CALL(m.Mock(), WakeUpEnd()).Times(1).WillOnce(Return(false));
    ret  = DisplayManagerLite::GetInstance().WakeUpEnd();
    ASSERT_EQ(false, ret);
}

/**
 * @tc.name: suspend_begin_001
 * @tc.desc: call SuspendBegin and check return value
 * @tc.type: FUNC
 */
HWTEST_F(DisplayLitePowerUnitTest, suspend_begin_001, TestSize.Level1)
{
    Mocker m;
    EXPECT_CALL(m.Mock(), SuspendBegin(PowerStateChangeReason::POWER_BUTTON)).Times(1).WillOnce(Return(true));
    bool ret  = DisplayManagerLite::GetInstance().SuspendBegin(PowerStateChangeReason::POWER_BUTTON);
    ASSERT_EQ(true, ret);

    EXPECT_CALL(m.Mock(), SuspendBegin(PowerStateChangeReason::POWER_BUTTON)).Times(1).WillOnce(Return(false));
    ret  = DisplayManagerLite::GetInstance().SuspendBegin(PowerStateChangeReason::POWER_BUTTON);
    ASSERT_EQ(false, ret);
}

/**
 * @tc.name: suspend_end_001
 * @tc.desc: call SuspendEnd and check return value
 * @tc.type: FUNC
 */
HWTEST_F(DisplayLitePowerUnitTest, suspend_end_001, TestSize.Level1)
{
    Mocker m;
    EXPECT_CALL(m.Mock(), SuspendEnd()).Times(1).WillOnce(Return(true));
    bool ret  = DisplayManagerLite::GetInstance().SuspendEnd();
    ASSERT_EQ(true, ret);

    EXPECT_CALL(m.Mock(), SuspendEnd()).Times(1).WillOnce(Return(false));
    ret  = DisplayManagerLite::GetInstance().SuspendEnd();
    ASSERT_EQ(false, ret);
}

/**
 * @tc.name: set_screen_brightness_001
 * @tc.desc: Call SetScreenBrightness with a valid value and check the GetScreenBrightness return value
 * @tc.type: FUNC
 */
HWTEST_F(DisplayLitePowerUnitTest, set_screen_brightness_001, TestSize.Level1)
{
    bool ret =
        DisplayManagerLite::GetInstance().SetScreenBrightness(DmsScreenBrightnessData(defaultId_, brightnessLevel_));
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: SetSpecifiedScreenPower
 * @tc.desc: test SetSpecifiedScreenPower
 * @tc.type: FUNC
 */
HWTEST_F(DisplayLitePowerUnitTest, set_specified_screen_power_001, TestSize.Level1)
{
    ScreenPowerState state = ScreenPowerState{0};
    PowerStateChangeReason reason = PowerStateChangeReason{0};
    bool ret = ScreenManagerLite::GetInstance().SetSpecifiedScreenPower(0, state, reason);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: set_screen_power_for_all_001
 * @tc.desc: Call SetScreenPowerForAll with valid value and check the GetScreenPower return value
 * @tc.type: FUNC
 */
HWTEST_F(DisplayLitePowerUnitTest, set_screen_power_for_all_001, TestSize.Level1)
{
    SingletonMocker<ScreenManagerAdapterLite, MockScreenManagerAdapterLite> m;
    EXPECT_CALL(m.Mock(), GetScreenPower(_)).Times(1).WillOnce(Return(ScreenPowerState::POWER_OFF));
    EXPECT_CALL(m.Mock(), SetScreenPowerForAll(_, PowerStateChangeReason::POWER_BUTTON))
        .Times(1).WillOnce(Return(true));

    bool ret = ScreenManagerLite::GetInstance().SetScreenPowerForAll(ScreenPowerState::POWER_OFF,
        PowerStateChangeReason::POWER_BUTTON);
    ASSERT_EQ(true, ret);

    ScreenPowerState state = ScreenManagerLite::GetInstance().GetScreenPower(defaultId_);
    ASSERT_EQ(state, ScreenPowerState::POWER_OFF);
}

/**
 * @tc.name: set_display_state_001
 * @tc.desc: Call SetDisplayState with valid value and check the GetDisplayState return value
 * @tc.type: FUNC
 */
HWTEST_F(DisplayLitePowerUnitTest, set_display_state_001, TestSize.Level1)
{
    DisplayState stateToSet = (initialState_ == DisplayState::OFF ? DisplayState::ON : DisplayState::OFF);
    Mocker m;
    EXPECT_CALL(m.Mock(), RegisterDisplayManagerAgent(_, DisplayManagerAgentType::DISPLAY_STATE_LISTENER))
        .Times(1).WillOnce(Return(DMError::DM_OK));
    EXPECT_CALL(m.Mock(), SetDisplayState(stateToSet)).Times(1).WillOnce(Return(true));
    DisplayStateCallback callback = [](DisplayState state) {};
    bool ret = DisplayManagerLite::GetInstance().SetDisplayState(stateToSet, callback);
    ASSERT_EQ(true, ret);

    EXPECT_CALL(m.Mock(), GetDisplayState(defaultId_)).Times(1).WillOnce(Return(stateToSet));
    DisplayState state = DisplayManagerLite::GetInstance().GetDisplayState(defaultId_);
    ASSERT_EQ(state, stateToSet);
}

/**
 * @tc.name: set_display_state_002
 * @tc.desc: Call SetDisplayState with invalid callback and check the GetDisplayState return value
 * @tc.type: FUNC
 */
HWTEST_F(DisplayLitePowerUnitTest, set_display_state_002, TestSize.Level1)
{
    Mocker m;
    EXPECT_CALL(m.Mock(), SetDisplayState(_)).Times(0);
    DisplayState stateToSet = (initialState_ == DisplayState::OFF ? DisplayState::ON : DisplayState::OFF);
    bool ret = DisplayManagerLite::GetInstance().SetDisplayState(stateToSet, nullptr);
    ASSERT_EQ(false, ret);
}
}
}
}