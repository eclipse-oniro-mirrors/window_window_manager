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

#include "ability_context_impl.h"

#include "mock_session.h"
#include "mock_uicontent.h"
#include "window_scene_session_impl.h"
#include "window_test_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Utils = WindowTestUtils;

class WindowDecorTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

private:
    static constexpr uint32_t TEST_SLEEP_S = 1; // test sleep time
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
};

void WindowDecorTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowDecorTest::TearDown()
{
    abilityContext_ = nullptr;
}

namespace {
/**
 * @tc.name: SetDecorVisible01
 * @tc.desc: SetDecorVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowDecorTest, SetDecorVisible01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetDecorVisible(true));

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetDecorVisible(true));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetDecorVisible(false));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetDecorVisible02
 * @tc.desc: SetDecorVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowDecorTest, SetDecorVisible02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10012);
    window->hostSession_ = session;

    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetDecorVisible(true));

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetDecorVisible(true));
    ASSERT_EQ(WMError::WM_OK, window->SetDecorVisible(false));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetDecorVisible03
 * @tc.desc: SetDecorVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowDecorTest, SetDecorVisible03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window1_3");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10013);
    window->hostSession_ = session;

    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetDecorVisible(true));

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetDecorVisible(true));
    ASSERT_EQ(WMError::WM_OK, window->SetDecorVisible(false));

    window->Destroy(true, true);
}

/**
 * @tc.name: DecorHeight01
 * @tc.desc: DecorHeight
 * @tc.type: FUNC
 */
HWTEST_F(WindowDecorTest, DecorHeight01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window2_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10021);
    window->property_->SetDisplayId(0);

    int32_t height = 40;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetDecorHeight(height));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->GetDecorHeight(height));

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetDecorHeight(height));
    ASSERT_EQ(WMError::WM_OK, window->GetDecorHeight(height));

    window->Destroy(true, true);
}

/**
 * @tc.name: DecorHeight02
 * @tc.desc: DecorHeight
 * @tc.type: FUNC
 */
HWTEST_F(WindowDecorTest, DecorHeight02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window2_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10022);
    window->property_->SetDisplayId(0);

    int32_t height = 50;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetDecorHeight(height));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->GetDecorHeight(height));

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetDecorHeight(height));
    ASSERT_EQ(WMError::WM_OK, window->GetDecorHeight(height));

    window->Destroy(true, true);
}

/**
 * @tc.name: DecorHeight03
 * @tc.desc: DecorHeight
 * @tc.type: FUNC
 */
HWTEST_F(WindowDecorTest, DecorHeight03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window2_3");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10023);
    window->property_->SetDisplayId(0);

    int32_t height = 60;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->SetDecorHeight(height));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->GetDecorHeight(height));

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetDecorHeight(height));
    ASSERT_EQ(WMError::WM_OK, window->GetDecorHeight(height));

    window->Destroy(true, true);
}

/**
 * @tc.name: TitleButtonVisible01
 * @tc.desc: TitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowDecorTest, TitleButtonVisible01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window3_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10031);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    window->windowSystemConfig_.isSystemDecorEnable_ = false;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetTitleButtonVisible(false, false, false, true));
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    ASSERT_EQ(WMError::WM_OK, window->SetTitleButtonVisible(false, false, false, true));

    ASSERT_EQ(WMError::WM_OK, window->SetTitleButtonVisible(true, true, true, true));
    ASSERT_EQ(WMError::WM_OK, window->SetTitleButtonVisible(false, true, true, true));
    ASSERT_EQ(WMError::WM_OK, window->SetTitleButtonVisible(true, false, true, true));
    ASSERT_EQ(WMError::WM_OK, window->SetTitleButtonVisible(true, true, false, true));
    ASSERT_EQ(WMError::WM_OK, window->SetTitleButtonVisible(true, false, false, true));
    ASSERT_EQ(WMError::WM_OK, window->SetTitleButtonVisible(false, true, false, true));
    ASSERT_EQ(WMError::WM_OK, window->SetTitleButtonVisible(false, false, true, true));
    ASSERT_EQ(WMError::WM_OK, window->SetTitleButtonVisible(false, false, false, true));

    window->Destroy(true, true);
}

/**
 * @tc.name: TitleButtonVisible02
 * @tc.desc: TitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowDecorTest, TitleButtonVisible02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window3_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10032);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    window->windowSystemConfig_.isSystemDecorEnable_ = false;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetTitleButtonVisible(false, false, false, true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetTitleButtonVisible(false, false, false, true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetTitleButtonVisible(false, false, false, true));
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetTitleButtonVisible(false, false, false, true));

    window->Destroy(true, true);
}

/**
 * @tc.name: TitleButtonVisible03
 * @tc.desc: TitleButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowDecorTest, TitleButtonVisible03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window3_3");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10033);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    window->windowSystemConfig_.isSystemDecorEnable_ = false;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetTitleButtonVisible(false, false, false, true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetTitleButtonVisible(false, false, false, true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetTitleButtonVisible(false, false, false, true));
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetTitleButtonVisible(false, false, false, true));

    window->Destroy(true, true);
}

/**
 * @tc.name: DisableAppWindowDecor01
 * @tc.desc: DisableAppWindowDecor
 * @tc.type: FUNC
 */
HWTEST_F(WindowDecorTest, DisableAppWindowDecor01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window4_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10041);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->DisableAppWindowDecor());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->DisableAppWindowDecor());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->DisableAppWindowDecor());

    window->Destroy(true, true);
}

/**
 * @tc.name: DisableAppWindowDecor02
 * @tc.desc: DisableAppWindowDecor
 * @tc.type: FUNC
 */
HWTEST_F(WindowDecorTest, DisableAppWindowDecor02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window4_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10042);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->DisableAppWindowDecor());

    window->Destroy(true, true);
}

/**
 * @tc.name: DisableAppWindowDecor03
 * @tc.desc: DisableAppWindowDecor
 * @tc.type: FUNC
 */
HWTEST_F(WindowDecorTest, DisableAppWindowDecor03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window4_3");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10043);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, window->DisableAppWindowDecor());

    window->Destroy(true, true);
}
} // namespace
} // namespace Rosen
} // namespace OHOS