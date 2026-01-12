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
#include "ability_context_impl.h"
#include "display_manager_proxy.h"
#include "mock_uicontent.h"
#include "mock_window_adapter.h"
#include "scene_board_judgement.h"
#include "singleton_mocker.h"
#include "window_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;

class WindowImplEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();
};
void WindowImplEventTest::SetUpTestCase() {}

void WindowImplEventTest::TearDownTestCase() {}

void WindowImplEventTest::SetUp() {}

void WindowImplEventTest::TearDown()
{
}

namespace {
/**
 * @tc.name: SetTouchable01
 * @tc.desc: create window but not show, test SetTouchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplEventTest, SetTouchable01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTouchable01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    ASSERT_TRUE(window->GetTouchable());
    ASSERT_EQ(WMError::WM_OK, window->SetTouchable(false));
    ASSERT_FALSE(window->GetTouchable());
    ASSERT_EQ(WMError::WM_OK, window->SetTouchable(true));
    ASSERT_TRUE(window->GetTouchable());
    WindowState state = window->GetWindowState();
    window->SetWindowState(WindowState::STATE_DESTROYED);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetTouchable(false));
    ASSERT_TRUE(window->GetTouchable());
    window->SetWindowState(state);

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}

/**
 * @tc.name: SetTouchable02
 * @tc.desc: create window with show, test SetTouchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowImplEventTest, SetTouchable02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTouchable02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<WindowImpl> window = sptr<WindowImpl>::MakeSptr(option);
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

    EXPECT_CALL(m->Mock(), GetSystemConfig(_)).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(m->Mock(), CreateWindow(_, _, _, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Create(INVALID_WINDOW_ID));
    EXPECT_CALL(m->Mock(), AddWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Show());
    ASSERT_TRUE(window->GetTouchable());
    EXPECT_CALL(m->Mock(), UpdateProperty(_, _))
        .Times(2)
        .WillOnce(Return(WMError::WM_OK))
        .WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->SetTouchable(false));
    ASSERT_FALSE(window->GetTouchable());
    ASSERT_EQ(WMError::WM_OK, window->SetTouchable(true));
    ASSERT_TRUE(window->GetTouchable());

    EXPECT_CALL(m->Mock(), DestroyWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
}
}
}
}