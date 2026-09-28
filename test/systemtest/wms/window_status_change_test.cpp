/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

// gtest
#include <gtest/gtest.h>

#include "ability_context_impl.h"

#include "mock_session.h"
#include "window_scene_session_impl.h"
#include "window_test_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Utils = WindowTestUtils;

class WindowStatusChangeTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

private:
    static constexpr uint32_t TEST_SLEEP_S = 1; // test sleep time
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
};

void WindowStatusChangeTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowStatusChangeTest::TearDown()
{
    abilityContext_ = nullptr;
}

namespace {
/**
 * @tc.name: ChangeWindowStatus01
 * @tc.desc: one FLOATING MainWindow, maximize ImmersiveMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowStatusChangeTest, ChangeWindowStatus01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10011);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->windowSystemConfig_.supportMultiWindowScreenSet_.insert(window->property_->GetDisplayId());
    ASSERT_EQ(WindowState::STATE_CREATED, window->GetWindowState());

    ASSERT_EQ(WMError::WM_OK, window->Show());
    sleep(TEST_SLEEP_S);
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());

    ASSERT_EQ(WMError::WM_OK, window->Maximize());
    ASSERT_EQ(false, window->GetImmersiveModeEnabledState());

    ASSERT_EQ(WMError::WM_OK, window->Maximize(MaximizePresentation::ENTER_IMMERSIVE));
    ASSERT_EQ(true, window->GetImmersiveModeEnabledState());

    ASSERT_EQ(WMError::WM_OK, window->Maximize(MaximizePresentation::EXIT_IMMERSIVE));
    ASSERT_EQ(false, window->GetImmersiveModeEnabledState());

    window->Destroy(true, true);
}

/**
 * @tc.name: ChangeWindowStatus02
 * @tc.desc: one FLOATING MainWindow, maximize and recover
 * @tc.type: FUNC
 */
HWTEST_F(WindowStatusChangeTest, ChangeWindowStatus02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10012);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WindowState::STATE_CREATED, window->GetWindowState());

    ASSERT_EQ(WMError::WM_OK, window->Show());
    sleep(TEST_SLEEP_S);
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->Maximize());
    WindowStatus windowStatus1;
    ASSERT_EQ(WMError::WM_OK, window->GetWindowStatus(windowStatus1));
    ASSERT_NE(WindowStatus::WINDOW_STATUS_FULLSCREEN, windowStatus1);
    ASSERT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, window->Recover());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, window->GetWindowMode());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->Maximize());
    WindowStatus windowStatus2;
    ASSERT_EQ(WMError::WM_OK, window->GetWindowStatus(windowStatus2));
    ASSERT_EQ(WindowStatus::WINDOW_STATUS_FULLSCREEN, windowStatus2);
    ASSERT_EQ(WMError::WM_OK, window->Recover());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, window->GetWindowMode());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->Maximize());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, window->GetWindowMode());
    ASSERT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, window->Recover());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, window->GetWindowMode());

    window->Destroy(true, true);
}

/**
 * @tc.name: ChangeWindowStatus03
 * @tc.desc: one FLOATING SubWindow, maximize and recover
 * @tc.type: FUNC
 */
HWTEST_F(WindowStatusChangeTest, ChangeWindowStatus03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_3");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10013);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WindowState::STATE_CREATED, window->GetWindowState());

    ASSERT_EQ(WMError::WM_OK, window->Show());
    sleep(TEST_SLEEP_S);
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->Maximize());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, window->GetWindowMode());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->Recover());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, window->GetWindowMode());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->Maximize());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, window->GetWindowMode());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->Recover());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, window->GetWindowMode());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->Maximize());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, window->GetWindowMode());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->Recover());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, window->GetWindowMode());

    window->Destroy(true, true);
}

/**
 * @tc.name: ChangeWindowStatus04
 * @tc.desc: one FULLSCREEN SubWindow, recover
 * @tc.type: FUNC
 */
HWTEST_F(WindowStatusChangeTest, ChangeWindowStatus04, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_4");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10014);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WindowState::STATE_CREATED, window->GetWindowState());

    ASSERT_EQ(WMError::WM_OK, window->Show());
    sleep(TEST_SLEEP_S);
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->Recover());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FULLSCREEN, window->GetWindowMode());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->Recover());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FULLSCREEN, window->GetWindowMode());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->Recover());
    ASSERT_EQ(WindowMode::WINDOW_MODE_FULLSCREEN, window->GetWindowMode());

    window->Destroy(true, true);
}

/**
 * @tc.name: ChangeWindowStatus05
 * @tc.desc: one FLOATING MainWindow, minimize and restore
 * @tc.type: FUNC
 */
HWTEST_F(WindowStatusChangeTest, ChangeWindowStatus05, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window2_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10021);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WindowState::STATE_CREATED, window->GetWindowState());

    ASSERT_EQ(WMError::WM_OK, window->Show());
    sleep(TEST_SLEEP_S);
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->Minimize());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->Restore());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->Minimize());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());
    ASSERT_EQ(WMError::WM_OK, window->Restore());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->Minimize());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->Restore());

    window->Destroy(true, true);
}

/**
 * @tc.name: ChangeWindowStatus06
 * @tc.desc: one FLOATING SubWindow, minimize and restore
 * @tc.type: FUNC
 */
HWTEST_F(WindowStatusChangeTest, ChangeWindowStatus06, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window2_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10022);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WindowState::STATE_CREATED, window->GetWindowState());

    ASSERT_EQ(WMError::WM_OK, window->Show());
    sleep(TEST_SLEEP_S);
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->Minimize());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->Restore());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->Minimize());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->Restore());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->Minimize());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->GetWindowState());
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->Restore());

    window->Destroy(true, true);
}
} // namespace

} // namespace Rosen
} // namespace OHOS