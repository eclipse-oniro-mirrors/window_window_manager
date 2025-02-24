/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <parameters.h>
#include "ability_context_impl.h"
#include "common_test_utils.h"
#include "display_info.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window_adapter.h"
#include "pointer_event.h"
#include "singleton_mocker.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowSceneSessionImplTest5 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowSceneSessionImplTest5::SetUpTestCase() {}

void WindowSceneSessionImplTest5::TearDownTestCase() {}

void WindowSceneSessionImplTest5::SetUp() {}

void WindowSceneSessionImplTest5::TearDown() {}

namespace {
/**
 * @tc.name: ShowKeyboard01
 * @tc.desc: SwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, ShowKeyboard01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowSceneSessionImpl> keyboardWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    keyboardWindow->property_->SetPersistentId(1000);
    keyboardWindow->hostSession_ = session;
    keyboardWindow->property_->SetWindowName("SwitchFreeMultiWindow02_mainWindow");
    keyboardWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardWindow->state_ = WindowState::STATE_DESTROYED;

    // normal value
    ASSERT_EQ(keyboardWindow->ShowKeyboard(KeyboardViewMode::DARK_IMMERSIVE_MODE), WMError::WM_ERROR_INVALID_WINDOW);

    // exception value
    ASSERT_EQ(keyboardWindow->ShowKeyboard(KeyboardViewMode::VIEW_MODE_END), WMError::WM_ERROR_INVALID_WINDOW);
    ASSERT_EQ(keyboardWindow->property_->GetKeyboardViewMode(), KeyboardViewMode::NON_IMMERSIVE_MODE);
}

/**
 * @tc.name: ChangeKeyboardViewMode01
 * @tc.desc: SwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, ChangeKeyboardViewMode01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowSceneSessionImpl> keyboardWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    keyboardWindow->property_->SetPersistentId(1000);
    keyboardWindow->hostSession_ = session;
    keyboardWindow->property_->SetWindowName("SwitchFreeMultiWindow02_mainWindow");
    keyboardWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardWindow->property_->SetKeyboardViewMode(KeyboardViewMode::NON_IMMERSIVE_MODE);

    auto result = WMError::WM_OK;
    // exception mode value
    result = keyboardWindow->ChangeKeyboardViewMode(KeyboardViewMode::VIEW_MODE_END);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);

    // same mode
    result = keyboardWindow->ChangeKeyboardViewMode(KeyboardViewMode::NON_IMMERSIVE_MODE);
    ASSERT_EQ(result, WMError::WM_DO_NOTHING);

    // invalid window state
    keyboardWindow->state_ = WindowState::STATE_DESTROYED;
    result = keyboardWindow->ChangeKeyboardViewMode(KeyboardViewMode::LIGHT_IMMERSIVE_MODE);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_WINDOW);

    // window state not shown
    keyboardWindow->state_ = WindowState::STATE_HIDDEN;
    result = keyboardWindow->ChangeKeyboardViewMode(KeyboardViewMode::LIGHT_IMMERSIVE_MODE);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_WINDOW);

    keyboardWindow->state_ = WindowState::STATE_SHOWN;
    result = keyboardWindow->ChangeKeyboardViewMode(KeyboardViewMode::DARK_IMMERSIVE_MODE);
    ASSERT_EQ(result, WMError::WM_OK);
    auto currentMode = keyboardWindow->property_->GetKeyboardViewMode();
    ASSERT_EQ(currentMode, KeyboardViewMode::DARK_IMMERSIVE_MODE);
}
} // namespace
} // namespace Rosen
} // namespace OHOS