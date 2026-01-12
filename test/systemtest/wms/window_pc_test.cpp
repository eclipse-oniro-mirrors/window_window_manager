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

class WindowPCTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

private:
    static constexpr uint32_t TEST_SLEEP_S = 1; // test sleep time
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
};

void WindowPCTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowPCTest::TearDown()
{
    abilityContext_ = nullptr;
}

namespace {
/**
 * @tc.name: setHandwritingFlag01
 * @tc.desc: setHandwritingFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, setHandwritingFlag01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window2_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10021);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_HANDWRITING)));

    window->Destroy(true, true);
}

/**
 * @tc.name: setHandwritingFlag02
 * @tc.desc: setHandwritingFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, setHandwritingFlag02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window2_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10022);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_HANDWRITING)));

    window->Destroy(true, true);
}

/**
 * @tc.name: setHandwritingFlag03
 * @tc.desc: setHandwritingFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, setHandwritingFlag03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window2_3");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->property_->SetPersistentId(10023);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_HANDWRITING)));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetResizeByDragEnabled01
 * @tc.desc: SetResizeByDragEnabled MAIN_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, SetResizeByDragEnabled01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window5_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10051);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    ASSERT_EQ(true, window->property_->GetDragEnabled());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetResizeByDragEnabled(false));
    ASSERT_EQ(false, window->property_->GetDragEnabled());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetResizeByDragEnabled(true));
    ASSERT_EQ(true, window->property_->GetDragEnabled());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetResizeByDragEnabled(false));
    ASSERT_EQ(false, window->property_->GetDragEnabled());

    window->Destroy(true, true);
}

/**
 * @tc.name: SetResizeByDragEnabled02
 * @tc.desc: SetResizeByDragEnabled SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, SetResizeByDragEnabled02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window5_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetSubWindowDecorEnable(true);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10052);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    ASSERT_EQ(true, window->property_->GetDragEnabled());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetResizeByDragEnabled(false));
    ASSERT_EQ(false, window->property_->GetDragEnabled());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetResizeByDragEnabled(true));
    ASSERT_EQ(true, window->property_->GetDragEnabled());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetResizeByDragEnabled(false));
    ASSERT_EQ(false, window->property_->GetDragEnabled());

    window->Destroy(true, true);
}

/**
 * @tc.name: SetResizeByDragEnabled03
 * @tc.desc: SetResizeByDragEnabled SYSTEM_SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, SetResizeByDragEnabled03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window5_3");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetSubWindowDecorEnable(true);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10053);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_TYPE, window->SetResizeByDragEnabled(true));

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_TYPE, window->SetResizeByDragEnabled(false));

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_TYPE, window->SetResizeByDragEnabled(true));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetWindowTitleMoveEnabled01
 * @tc.desc: SetWindowTitleMoveEnabled MAIN_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, SetWindowTitleMoveEnabled01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window6_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10061);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    ASSERT_EQ(WMError::WM_OK, window->SetWindowTitleMoveEnabled(false));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetWindowTitleMoveEnabled02
 * @tc.desc: SetWindowTitleMoveEnabled SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, SetWindowTitleMoveEnabled02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window6_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10062);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    ASSERT_EQ(WMError::WM_OK, window->SetWindowTitleMoveEnabled(false));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetWindowTitleMoveEnabled03
 * @tc.desc: SetWindowTitleMoveEnabled SYSTEM_SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, SetWindowTitleMoveEnabled03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window6_3");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10063);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetWindowTitleMoveEnabled(false));

    window->Destroy(true, true);
}

/**
 * @tc.name: StartMoveWindow01
 * @tc.desc: startMoving MAIN_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, StartMoveWindow01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window7_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10071);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    ASSERT_EQ(WmErrorCode::WM_ERROR_INVALID_CALLING, window->StartMoveWindow());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WmErrorCode::WM_ERROR_INVALID_CALLING, window->StartMoveWindow());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WmErrorCode::WM_OK, window->StartMoveWindow());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WmErrorCode::WM_ERROR_INVALID_CALLING, window->StartMoveWindow());

    window->Destroy(true, true);
}

/**
 * @tc.name: StartMoveWindow02
 * @tc.desc: startMoving SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, StartMoveWindow02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window7_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10072);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    ASSERT_EQ(WmErrorCode::WM_ERROR_INVALID_CALLING, window->StartMoveWindow());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT, window->StartMoveWindow());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WmErrorCode::WM_OK, window->StartMoveWindow());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT, window->StartMoveWindow());

    window->Destroy(true, true);
}

/**
 * @tc.name: StartMoveWindow03
 * @tc.desc: startMoving SYSTEM_SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, StartMoveWindow03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window7_3");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10073);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    ASSERT_EQ(WmErrorCode::WM_ERROR_INVALID_CALLING, window->StartMoveWindow());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT, window->StartMoveWindow());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WmErrorCode::WM_OK, window->StartMoveWindow());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT, window->StartMoveWindow());

    window->Destroy(true, true);
}

/**
 * @tc.name: SetMainWindowTopmost01
 * @tc.desc: SetMainWindowTopmost MAIN_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, SetMainWindowTopmost01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window8_1");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10081);
    window->state_ = WindowState::STATE_CREATED;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetMainWindowTopmost(true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_OK, window->SetMainWindowTopmost(false));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetMainWindowTopmost(true));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetMainWindowTopmost02
 * @tc.desc: SetMainWindowTopmost SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, SetMainWindowTopmost02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window8_2");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10082);
    window->state_ = WindowState::STATE_CREATED;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetMainWindowTopmost(true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetMainWindowTopmost(false));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetMainWindowTopmost(true));

    window->Destroy(true, true);
}

/**
 * @tc.name: SetMainWindowTopmost03
 * @tc.desc: SetMainWindowTopmost SYSTEM_SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowPCTest, SetMainWindowTopmost03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Window8_3");
    option->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(10083);
    window->state_ = WindowState::STATE_CREATED;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetMainWindowTopmost(true));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetMainWindowTopmost(false));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ASSERT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, window->SetMainWindowTopmost(true));

    window->Destroy(true, true);
}

} // namespace
} // namespace Rosen
} // namespace OHOS