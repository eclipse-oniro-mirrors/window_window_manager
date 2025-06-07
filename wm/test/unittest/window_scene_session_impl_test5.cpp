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
#include "mock_window.h"
#include "mock_window_adapter.h"
#include "pointer_event.h"
#include "scene_board_judgement.h"
#include "singleton_mocker.h"
#include "wm_common_inner.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using WindowAdapterMocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
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
 * @tc.name: IgnoreClickEvent
 * @tc.desc: IgnoreClickEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, IgnoreClickEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("IgnoreClickEvent");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    pointerEvent->SetPointerAction(1);
    window->IgnoreClickEvent(pointerEvent);

    pointerEvent->SetPointerAction(4);
    window->IgnoreClickEvent(pointerEvent);

    window->isOverTouchSlop_ = true;
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->pointers_.clear();
    pointerEvent->pointers_.push_back(pointerItem);
    window->IgnoreClickEvent(pointerEvent);

    window->isOverTouchSlop_ = false;
    ASSERT_EQ(pointerEvent->GetPointerAction(), MMI::PointerEvent::POINTER_ACTION_CANCEL);
    window->IgnoreClickEvent(pointerEvent);
}

/**
 * @tc.name: HandleUpForCompatibleMode
 * @tc.desc: HandleUpForCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, HandleUpForCompatibleMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("HandleUpForCompatibleMode");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    MMI::PointerEvent::PointerItem pointerItem;
    window->HandleUpForCompatibleMode(pointerEvent, pointerItem);

    window->isDown_ = true;
    window->eventMapTriggerByDisplay_.clear();
    window->eventMapTriggerByDisplay_.insert(std::pair<int32_t, std::vector<bool>>(-2, { true }));
    window->HandleUpForCompatibleMode(pointerEvent, pointerItem);
}

/**
 * @tc.name: HandleMoveForCompatibleMode
 * @tc.desc: HandleMoveForCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, HandleMoveForCompatibleMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("HandleMoveForCompatibleMode");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    MMI::PointerEvent::PointerItem pointerItem;
    window->HandleMoveForCompatibleMode(pointerEvent, pointerItem);

    window->isDown_ = true;
    window->eventMapTriggerByDisplay_.clear();
    window->eventMapTriggerByDisplay_.insert(std::pair<int32_t, std::vector<bool>>(-2, { true }));
    window->HandleMoveForCompatibleMode(pointerEvent, pointerItem);
}

/**
 * @tc.name: HandleDownForCompatibleMode
 * @tc.desc: HandleDownForCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, HandleDownForCompatibleMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("HandleDownForCompatibleMode");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    MMI::PointerEvent::PointerItem pointerItem;
    window->HandleDownForCompatibleMode(pointerEvent, pointerItem);

    pointerEvent->pointers_.clear();
    window->HandleDownForCompatibleMode(pointerEvent, pointerItem);

    pointerEvent->pointers_.push_back(pointerItem);
    sptr<DisplayInfo> displayInfo = nullptr;
    auto ret = window->GetVirtualPixelRatio(displayInfo);
    ASSERT_EQ(ret, 1.0f);
}

/**
 * @tc.name: Maximize
 * @tc.desc: Maximize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, Maximize, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("Maximize");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    window->hostSession_ = nullptr;
    MaximizePresentation presentation = MaximizePresentation::ENTER_IMMERSIVE;
    auto ret = window->Maximize(presentation);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: Maximize01
 * @tc.desc: Maximize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, Maximize01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("Maximize");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    MaximizePresentation presentation = MaximizePresentation::ENTER_IMMERSIVE;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->persistentId_ = ROTATE_ANIMATION_DURATION;
    window->state_ = WindowState::STATE_CREATED;

    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    auto ret = window->Maximize(presentation);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetWindowModeSupportType(0);
    ret = window->Maximize(presentation);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->SetWindowModeSupportType(1);
    SystemSessionConfig systemSessionConfig;
    systemSessionConfig.windowUIType_ = WindowUIType::INVALID_WINDOW;
    ret = window->Maximize(presentation);
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: Maximize02
 * @tc.desc: Maximize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, Maximize02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("Maximize");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    MaximizePresentation presentation = MaximizePresentation::ENTER_IMMERSIVE;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->persistentId_ = ROTATE_ANIMATION_DURATION;
    window->state_ = WindowState::STATE_CREATED;
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->SetWindowModeSupportType(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    presentation = MaximizePresentation::ENTER_IMMERSIVE;
    auto ret = window->Maximize(presentation);
    ASSERT_EQ(ret, WMError::WM_OK);

    ASSERT_EQ(window->enableImmersiveMode_, true);

    presentation = MaximizePresentation::EXIT_IMMERSIVE;
    window->Maximize(presentation);
    ASSERT_EQ(window->enableImmersiveMode_, false);

    presentation = MaximizePresentation::ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER;
    window->Maximize(presentation);
    ASSERT_EQ(window->enableImmersiveMode_, true);

    presentation = MaximizePresentation::FOLLOW_APP_IMMERSIVE_SETTING;
    window->Maximize(presentation);
}

/**
 * @tc.name: MoveWindowToGlobal
 * @tc.desc: MoveWindowToGlobal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, MoveWindowToGlobal, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("MoveWindowToGlobal");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    window->hostSession_ = nullptr;
    MoveConfiguration presentation;
    auto ret = window->MoveWindowToGlobal(1, 1, presentation);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: MoveWindowToGlobal01
 * @tc.desc: MoveWindowToGlobal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, MoveWindowToGlobal01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("Maximize");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);

    MoveConfiguration presentation;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->persistentId_ = ROTATE_ANIMATION_DURATION;
    window->state_ = WindowState::STATE_CREATED;

    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    auto ret = window->MoveWindowToGlobal(1, 1, presentation);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS);

    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    ret = window->MoveWindowToGlobal(1, 1, presentation);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_OPERATION);

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->MoveWindowToGlobal(1, 1, presentation);

    window->state_ = WindowState::STATE_SHOWN;
    ret = window->MoveWindowToGlobal(1, 1, presentation);
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: SetCustomDensity01
 * @tc.desc: SetCustomDensity
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetCustomDensity01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    float density = 0.4f;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetCustomDensity(density));

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->property_->SetWindowName("SetCustomDensity01");
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetCustomDensity(density));

    density = 1.5f;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetCustomDensity(density));

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(WMError::WM_OK, window->SetCustomDensity(density));
    ASSERT_EQ(density, window->customDensity_);
    ASSERT_EQ(WMError::WM_OK, window->SetCustomDensity(density));
}

/**
 * @tc.name: IsDefaultDensityEnabled01
 * @tc.desc: IsDefaultDensityEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, IsDefaultDensityEnabled01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->SetDefaultDensityEnabled(true);
    EXPECT_EQ(true, window->GetDefaultDensityEnabled());
    window->SetDefaultDensityEnabled(false);
    EXPECT_EQ(false, window->GetDefaultDensityEnabled());
}

/**
 * @tc.name: IsDefaultDensityEnabled02
 * @tc.desc: IsDefaultDensityEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, IsDefaultDensityEnabled02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(false, window->GetDefaultDensityEnabled());
}


/**
 * @tc.name: GetCustomDensity01
 * @tc.desc: GetCustomDensity
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetCustomDensity01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("SetCustomDensity01");
    window->state_ = WindowState::STATE_CREATED;
    float density = 1.5f;
    window->SetCustomDensity(density);
    EXPECT_EQ(density, window->GetCustomDensity());
}

/**
 * @tc.name: GetMainWindowCustomDensity02
 * @tc.desc: GetMainWindowCustomDensity
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetMainWindowCustomDensity01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("SetCustomDensity01");
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_CREATED;
    float density = 1.5f;
    window->SetCustomDensity(density);
    EXPECT_EQ(density, window->GetMainWindowCustomDensity());
}

/**
 * @tc.name: GetWindowDensityInfo01
 * @tc.desc: GetWindowDensityInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetWindowDensityInfo01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    WindowDensityInfo densityInfo;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->GetWindowDensityInfo(densityInfo));

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->property_->SetWindowName("GetWindowDensityInfo01");
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_CREATED;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, window->GetWindowDensityInfo(densityInfo));
}

/**
 * @tc.name: SwitchFreeMultiWindow01
 * @tc.desc: SwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SwitchFreeMultiWindow01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);
    auto ref = window->SwitchFreeMultiWindow(false);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_WINDOW, ref);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("SwitchFreeMultiWindow01");
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    ref = window->SwitchFreeMultiWindow(false);
    ASSERT_EQ(WSError::WS_ERROR_REPEAT_OPERATION, ref);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    ref = window->SwitchFreeMultiWindow(true);
    ASSERT_EQ(WSError::WS_ERROR_REPEAT_OPERATION, ref);

    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(window->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(window->GetWindowId(), window)));
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, window->SwitchFreeMultiWindow(true));
    EXPECT_EQ(true, window->windowSystemConfig_.freeMultiWindowEnable_);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, window->SwitchFreeMultiWindow(false));
    EXPECT_EQ(false, window->windowSystemConfig_.freeMultiWindowEnable_);
    WindowSceneSessionImpl::windowSessionMap_.erase(window->GetWindowName());
}

/**
 * @tc.name: SwitchFreeMultiWindow02
 * @tc.desc: SwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SwitchFreeMultiWindow02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowSceneSessionImpl> mainWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    mainWindow->property_->SetPersistentId(1);
    mainWindow->hostSession_ = session;
    mainWindow->property_->SetWindowName("SwitchFreeMultiWindow02_mainWindow");
    mainWindow->windowSystemConfig_.freeMultiWindowEnable_ = false;
    mainWindow->windowSystemConfig_.freeMultiWindowSupport_ = true;
    mainWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    mainWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(mainWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(mainWindow->GetWindowId(), mainWindow)));

    sptr<WindowSceneSessionImpl> floatWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    floatWindow->property_->SetPersistentId(2);
    floatWindow->hostSession_ = session;
    floatWindow->property_->SetWindowName("SwitchFreeMultiWindow02_floatWindow");
    floatWindow->windowSystemConfig_.freeMultiWindowEnable_ = false;
    floatWindow->windowSystemConfig_.freeMultiWindowSupport_ = true;
    floatWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    floatWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(floatWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(floatWindow->GetWindowId(), floatWindow)));

    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    subWindow->property_->SetPersistentId(3);
    subWindow->hostSession_ = session;
    subWindow->property_->SetWindowName("SwitchFreeMultiWindow03_subWindow");
    subWindow->windowSystemConfig_.freeMultiWindowEnable_ = false;
    subWindow->windowSystemConfig_.freeMultiWindowSupport_ = true;
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(subWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(subWindow->GetWindowId(), subWindow)));

    EXPECT_EQ(false, mainWindow->IsPcOrPadFreeMultiWindowMode());
    EXPECT_EQ(false, floatWindow->IsPcOrPadFreeMultiWindowMode());
    EXPECT_EQ(false, subWindow->IsPcOrPadFreeMultiWindowMode());
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, mainWindow->SwitchFreeMultiWindow(true));
    EXPECT_EQ(true, mainWindow->IsPcOrPadFreeMultiWindowMode());
    EXPECT_EQ(true, floatWindow->IsPcOrPadFreeMultiWindowMode());
    EXPECT_EQ(true, subWindow->IsPcOrPadFreeMultiWindowMode());

    EXPECT_EQ(WMError::WM_OK, mainWindow->Destroy(true));
    EXPECT_EQ(WMError::WM_OK, floatWindow->Destroy(true));
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
}

/**
 * @tc.name: ShowKeyboard01
 * @tc.desc: SwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, ShowKeyboard01, TestSize.Level1)
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
HWTEST_F(WindowSceneSessionImplTest5, ChangeKeyboardViewMode01, TestSize.Level1)
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

/**
 * @tc.name: StartMoveWindowWithCoordinate_01
 * @tc.desc: StartMoveWindowWithCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, StartMoveWindowWithCoordinate_01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(100, 50), WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_NE(window->StartMoveWindowWithCoordinate(100, 50), WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    ASSERT_NE(window->StartMoveWindowWithCoordinate(100, 50), WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    window->property_->SetIsPcAppInPad(true);
    ASSERT_NE(window->StartMoveWindowWithCoordinate(100, 50), WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->property_->SetIsPcAppInPad(true);
    ASSERT_NE(window->StartMoveWindowWithCoordinate(100, 50), WmErrorCode::WM_OK);
}

/**
 * @tc.name: StartMoveWindowWithCoordinate_02
 * @tc.desc: StartMoveWindowWithCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, StartMoveWindowWithCoordinate_02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    Rect windowRect = { 200, 200, 1000, 1000 };
    window->property_->SetWindowRect(windowRect);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetPersistentId(0);
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(-1, 50), WmErrorCode::WM_ERROR_STATE_ABNORMALLY);

    window->property_->SetPersistentId(1);
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(-1, 50), WmErrorCode::WM_ERROR_INVALID_PARAM);
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(100, -1), WmErrorCode::WM_ERROR_INVALID_PARAM);
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(1500, 50), WmErrorCode::WM_ERROR_INVALID_PARAM);
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(100, 1500), WmErrorCode::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: StartMoveWindowWithCoordinate_03
 * @tc.desc: StartMoveWindowWithCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, StartMoveWindowWithCoordinate_03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    Rect windowRect = { 200, 200, 1000, 1000 };
    window->property_->SetWindowRect(windowRect);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    EXPECT_CALL(*session, SyncSessionEvent(_)).Times(1).WillOnce(Return(WSError::WS_ERROR_REPEAT_OPERATION));
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(100, 50), WmErrorCode::WM_ERROR_REPEAT_OPERATION);

    EXPECT_CALL(*session, SyncSessionEvent(_)).Times(1).WillOnce(Return(WSError::WS_ERROR_NULLPTR));
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(100, 50), WmErrorCode::WM_ERROR_STATE_ABNORMALLY);

    EXPECT_CALL(*session, SyncSessionEvent(_)).Times(1).WillOnce(Return(WSError::WS_OK));
    ASSERT_EQ(window->StartMoveWindowWithCoordinate(100, 50), WmErrorCode::WM_OK);
}

/**
 * @tc.name: UpdateSystemBarProperties
 * @tc.desc: UpdateSystemBarProperties test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, UpdateSystemBarProperties, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSystemBarProperties");
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::unordered_map<WindowType, SystemBarProperty> systemBarProperties;
    std::unordered_map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    ASSERT_EQ(WMError::WM_OK, window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
}

/**
 * @tc.name: UpdateSystemBarProperties02
 * @tc.desc: UpdateSystemBarProperties02 test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, UpdateSystemBarProperties02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSystemBarProperties02");
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;

    WindowType windowType1 = WindowType::WINDOW_TYPE_STATUS_BAR;
    WindowType windowType2 = WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR;
    SystemBarProperty systemBarProperty1 = SystemBarProperty();
    SystemBarProperty systemBarProperty2 = SystemBarProperty(true, 100, 200);
    SystemBarPropertyFlag systemBarPropertyFlag1 = {true, true, true, true};
    SystemBarPropertyFlag systemBarPropertyFlag2 = {false, false, false, false};

    std::unordered_map<WindowType, SystemBarProperty> systemBarProperties;
    std::unordered_map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    systemBarProperties.insert({windowType1, systemBarProperty1});
    systemBarPropertyFlags.insert({windowType2, systemBarPropertyFlag2});
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));

    systemBarProperties.insert({windowType2, systemBarProperty2});
    systemBarPropertyFlags.insert({windowType1, systemBarPropertyFlag1});
    ASSERT_EQ(WMError::WM_OK, window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));

    window->state_ = WindowState::STATE_BOTTOM;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
            window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
}

/**
 * @tc.name: NotifyAfterDidForeground
 * @tc.desc: NotifyAfterDidForeground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, NotifyAfterDidForeground, TestSize.Level1)
{
    sptr<MockWindowLifeCycleListener> mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    sptr<IWindowLifeCycle> listener = static_cast<sptr<IWindowLifeCycle>>(mockListener);

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Test");
    option->SetDisplayId(0);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->RegisterLifeCycleListener(listener));

    ON_CALL(*mockListener, AfterDidForeground());
    ASSERT_EQ(WMError::WM_OK, window->Show(static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL), false));
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: NotifyAfterDidBackground
 * @tc.desc: NotifyAfterDidBackground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, NotifyAfterDidBackground, TestSize.Level1)
{
    sptr<MockWindowLifeCycleListener> mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    sptr<IWindowLifeCycle> listener = static_cast<sptr<IWindowLifeCycle>>(mockListener);

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Test");
    option->SetDisplayId(0);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->RegisterLifeCycleListener(listener));

    ON_CALL(*mockListener, AfterDidBackground());
    ASSERT_EQ(WMError::WM_OK, window->Hide(static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL), false, false));
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: Interactive
 * @tc.desc: Interactive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, Interactive, TestSize.Level1)
{
    sptr<MockWindowLifeCycleListener> mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    sptr<IWindowLifeCycle> listener = static_cast<sptr<IWindowLifeCycle>>(mockListener);

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Test");
    option->SetDisplayId(0);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->RegisterLifeCycleListener(listener));
    window->isDidForeground_ = false;
    window->isColdStart_ = true;
    window->state_ = WindowState::STATE_SHOWN;

    EXPECT_CALL(*mockListener, AfterInteractive()).Times(1);
    window->Interactive();
    EXPECT_EQ(window->isDidForeground_, true);
    EXPECT_EQ(window->isColdStart_, false);
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: GetParentWindow01
 * @tc.desc: GetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetParentWindow01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetParentWindow01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sptr<Window> parentWindow = nullptr;
    auto res = window->GetParentWindow(parentWindow);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->property_->SetParentPersistentId(2);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    res = window->GetParentWindow(parentWindow);
    EXPECT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    res = window->GetParentWindow(parentWindow);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);

    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    window->property_->SetIsUIExtFirstSubWindow(true);
    res = window->GetParentWindow(parentWindow);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);

    window->property_->SetIsUIExtFirstSubWindow(false);
    res = window->GetParentWindow(parentWindow);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARENT);
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: GetParentWindow02
 * @tc.desc: GetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetParentWindow02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetParentWindow01_parentWindow");
    sptr<WindowSceneSessionImpl> mainWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    mainWindow->hostSession_ = session;
    mainWindow->property_->SetPersistentId(1);
    mainWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    mainWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(mainWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(mainWindow->GetWindowId(), mainWindow)));

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("GetParentWindow01_subWindow");
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subWindowOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(1);
    subWindow->hostSession_ = session;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(subWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(subWindow->GetWindowId(), subWindow)));

    sptr<Window> parentWindow = nullptr;
    auto res = subWindow->GetParentWindow(parentWindow);
    EXPECT_EQ(res, WMError::WM_OK);
    ASSERT_NE(parentWindow, nullptr);
    EXPECT_EQ(parentWindow->GetWindowName(), mainWindow->GetWindowName());
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
    EXPECT_EQ(WMError::WM_OK, mainWindow->Destroy(true));
}

/**
 * @tc.name: SetParentWindow01
 * @tc.desc: SetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetParentWindow01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetParentWindow01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    int32_t newParentWindowId = 2;
    auto res = window->SetParentWindow(newParentWindowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->property_->SetParentPersistentId(2);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    res = window->SetParentWindow(newParentWindowId);
    EXPECT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    res = window->SetParentWindow(newParentWindowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);

    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    newParentWindowId = 1;
    res = window->SetParentWindow(newParentWindowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARENT);
    newParentWindowId = 2;
    res = window->SetParentWindow(newParentWindowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARENT);
    newParentWindowId = 3;
    res = window->SetParentWindow(newParentWindowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARENT);
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: SetParentWindow02
 * @tc.desc: SetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetParentWindow02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetParentWindow02_parentWindow");
    sptr<WindowSceneSessionImpl> mainWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    mainWindow->property_->SetPersistentId(1);
    mainWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    mainWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(mainWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(mainWindow->GetWindowId(), mainWindow)));

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("SetParentWindow02_subWindow");
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subWindowOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    subWindow->hostSession_ = session;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(subWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(subWindow->GetWindowId(), subWindow)));

    sptr<WindowOption> newParentWindowOption = sptr<WindowOption>::MakeSptr();
    newParentWindowOption->SetWindowName("SetParentWindow02_newParentWindow");
    sptr<WindowSceneSessionImpl> newParentWindow = sptr<WindowSceneSessionImpl>::MakeSptr(newParentWindowOption);
    newParentWindow->property_->SetPersistentId(3);
    newParentWindow->property_->SetParentPersistentId(1);
    newParentWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    newParentWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(newParentWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(newParentWindow->GetWindowId(), newParentWindow)));

    int32_t newParentWindowId = 3;
    auto res = subWindow->SetParentWindow(newParentWindowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARENT);
    newParentWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    WindowAdapterMocker mocker;
    EXPECT_CALL(mocker.Mock(), SetParentWindow(_, _)).WillOnce(Return(WMError::WM_OK));
    res = subWindow->SetParentWindow(newParentWindowId);
    EXPECT_EQ(res, WMError::WM_OK);
    sptr<Window> parentWindow = nullptr;
    EXPECT_EQ(subWindow->GetParentWindow(parentWindow), WMError::WM_OK);
    ASSERT_NE(parentWindow, nullptr);
    EXPECT_EQ(parentWindow->GetWindowName(), newParentWindow->GetWindowName());
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
}

/**
 * @tc.name: SetParentWindow03
 * @tc.desc: SetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetParentWindow03, TestSize.Level1)
{
    WindowSceneSessionImpl::windowSessionMap_.clear();
    sptr<WindowOption> parentOption1 = sptr<WindowOption>::MakeSptr();
    parentOption1->SetWindowName("SetParentWindow03_parentWindow1");
    parentOption1->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sptr<WindowSceneSessionImpl> parentWindow1 = sptr<WindowSceneSessionImpl>::MakeSptr(parentOption1);
    parentWindow1->property_->SetPersistentId(1);

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("SetParentWindow03_subWindow");
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subWindowOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    subWindow->hostSession_ = session;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    int32_t newParentWindowId = 3;
    EXPECT_EQ(subWindow->SetParentWindow(newParentWindowId), WMError::WM_ERROR_INVALID_PARENT);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(parentWindow1->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(parentWindow1->GetWindowId(), parentWindow1)));
    EXPECT_EQ(subWindow->SetParentWindow(newParentWindowId), WMError::WM_ERROR_INVALID_PARENT);
    parentWindow1->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    EXPECT_EQ(subWindow->SetParentWindow(newParentWindowId), WMError::WM_ERROR_INVALID_PARENT);

    sptr<WindowOption> parentOption2 = sptr<WindowOption>::MakeSptr();
    parentOption2->SetWindowName("SetParentWindow03_parentWindow2");
    parentOption2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> parentWindow2 = sptr<WindowSceneSessionImpl>::MakeSptr(parentOption2);
    parentWindow2->property_->SetPersistentId(3);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(parentWindow2->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(parentWindow2->GetWindowId(), parentWindow2)));
    WindowAdapterMocker mocker;
    WMError mockerResult = WMError::WM_ERROR_INVALID_WINDOW;
    EXPECT_CALL(mocker.Mock(), SetParentWindow(_, _)).WillOnce(Return(mockerResult));
    EXPECT_EQ(subWindow->SetParentWindow(newParentWindowId), mockerResult);
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
}

/**
 * @tc.name: SetParentWindow04
 * @tc.desc: SetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetParentWindow04, TestSize.Level1)
{
    WindowSceneSessionImpl::subWindowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.clear();
    sptr<WindowOption> parentOption1 = sptr<WindowOption>::MakeSptr();
    parentOption1->SetWindowName("SetParentWindow04_parentWindow1");
    parentOption1->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> parentWindow1 = sptr<WindowSceneSessionImpl>::MakeSptr(parentOption1);
    parentWindow1->property_->SetPersistentId(1);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(parentWindow1->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(parentWindow1->GetWindowId(), parentWindow1)));

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("SetParentWindow04_subWindow");
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subWindowOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    subWindow->hostSession_ = session;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WindowSceneSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, { subWindow }));

    sptr<WindowOption> parentOption2 = sptr<WindowOption>::MakeSptr();
    parentOption2->SetWindowName("SetParentWindow04_parentWindow2");
    parentOption2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> parentWindow2 = sptr<WindowSceneSessionImpl>::MakeSptr(parentOption2);
    parentWindow2->property_->SetPersistentId(3);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(parentWindow2->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(parentWindow2->GetWindowId(), parentWindow2)));

    std::vector<sptr<WindowSessionImpl>> subWindows;
    parentWindow1->GetSubWindows(1, subWindows);
    EXPECT_EQ(subWindows.size(), 1);
    WindowAdapterMocker mocker;
    EXPECT_CALL(mocker.Mock(), SetParentWindow(_, _)).WillOnce(Return(WMError::WM_OK));
    int32_t newParentWindowId = 3;
    EXPECT_EQ(subWindow->SetParentWindow(newParentWindowId), WMError::WM_OK);
    parentWindow1->GetSubWindows(1, subWindows);
    EXPECT_EQ(subWindows.size(), 0);
    subWindows.clear();
    parentWindow1->GetSubWindows(3, subWindows);
    EXPECT_EQ(subWindows.size(), 1);
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
}

/**
 * @tc.name: SetParentWindowInner
 * @tc.desc: SetParentWindowInner
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetParentWindowInner, TestSize.Level1)
{
    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("SetParentWindowInner_subWindow");
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subWindowOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    subWindow->hostSession_ = subSession;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow->state_ = WindowState::STATE_SHOWN;

    sptr<WindowOption> parentOption = sptr<WindowOption>::MakeSptr();
    parentOption->SetWindowName("SetParentWindowInner_parentWindow");
    parentOption->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> parentWindow = sptr<WindowSceneSessionImpl>::MakeSptr(parentOption);
    parentWindow->property_->SetPersistentId(3);
    sptr<SessionMocker> parentSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    parentWindow->hostSession_ = parentSession;
    parentWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    parentWindow->state_ = WindowState::STATE_HIDDEN;

    WindowAdapterMocker mocker;
    EXPECT_CALL(mocker.Mock(), SetParentWindow(_, _)).WillRepeatedly(Return(WMError::WM_OK));
    WindowSceneSessionImpl::subWindowSessionMap_.clear();
    EXPECT_EQ(WMError::WM_OK, subWindow->SetParentWindowInner(1, parentWindow));
    EXPECT_EQ(subWindow->state_, WindowState::STATE_HIDDEN);
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
    EXPECT_EQ(WMError::WM_OK, parentWindow->Destroy(true));
}

/**
 * @tc.name: SetSubWindowModal01
 * @tc.desc: SetSubWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetSubWindowModal01, TestSize.Level1)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("SetSubWindowModal01");
    subOption->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    subWindow->hostSession_ = session;
    EXPECT_CALL(*(session), UpdateSessionPropertyByAction(_, _))
        .WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ModalityType modalityType = ModalityType::APPLICATION_MODALITY;
    auto ret = subWindow->SetSubWindowModal(true, modalityType);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
}

/**
 * @tc.name: SetWindowModal01
 * @tc.desc: SetWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetWindowModal01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowModal01");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    EXPECT_CALL(*(session), UpdateSessionPropertyByAction(_, _))
        .WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    auto ret = window->SetWindowModal(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: IsFullScreenEnable
 * @tc.desc: IsFullScreenEnable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, IsFullScreenEnable, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowModeSupportType(0);
    ASSERT_EQ(window->IsFullScreenEnable(), false);
    window->property_->SetWindowModeSupportType(1);
    ASSERT_EQ(window->IsFullScreenEnable(), true);
    window->property_->SetDragEnabled(true);
    WindowLimits windowLimits = {5000, 5000, 500, 500, 0.0f, 0.0f};
    window->property_->SetWindowLimits(windowLimits);
    ASSERT_EQ(window->IsFullScreenEnable(), true);
    WindowLimits windowLimits1 = {800, 800, 500, 500, 0.0f, 0.0f};
    window->property_->SetWindowLimits(windowLimits1);
    ASSERT_EQ(window->IsFullScreenEnable(), false);
    window->property_->SetDragEnabled(false);
    ASSERT_EQ(window->IsFullScreenEnable(), true);
}

/**
 * @tc.name: IsFullScreenSizeWindow
 * @tc.desc: IsFullScreenSizeWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, IsFullScreenSizeWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetDisplayId(0);
    uint32_t width = 0;
    uint32_t height = 0;
    ASSERT_EQ(window->IsFullScreenSizeWindow(width, height), false);
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(0);
    ASSERT_NE(display, nullptr);
    auto displayInfo = display->GetDisplayInfo();
    ASSERT_NE(displayInfo, nullptr);
    width = static_cast<uint32_t>(displayInfo->GetWidth());
    height = static_cast<uint32_t>(displayInfo->GetHeight());
    ASSERT_EQ(window->IsFullScreenSizeWindow(width, height), true);
}

/**
 * @tc.name: SetWindowAnchorInfo
 * @tc.desc: SetWindowAnchorInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetWindowAnchorInfo01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowAnchorInfo01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto property = window->GetProperty();

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    WindowAnchorInfo windowAnchorInfo = { true, WindowAnchor::BOTTOM_END, 0, 0 };
    WMError ret = window->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    property->persistentId_ = 100;
    window->state_ = WindowState::STATE_CREATED;
    ret = window->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_OPERATION);

    property->subWindowLevel_ = 100;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = window->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_OPERATION);

    property->subWindowLevel_ = 1;
    window->windowSystemConfig_.supportFollowRelativePositionToParent_ = false;
    ret = window->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.supportFollowRelativePositionToParent_ = true;
    session->systemConfig_.supportFollowRelativePositionToParent_ = true;
    ret = window->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: SetFollowParentWindowLayoutEnabled
 * @tc.desc: SetFollowParentWindowLayoutEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetFollowParentWindowLayoutEnabled01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFollowParentWindowLayoutEnabled01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto property = window->GetProperty();
    window->windowSystemConfig_.supportFollowParentWindowLayout_ = true;

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    WMError ret = window->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    property->persistentId_ = 100;
    window->state_ = WindowState::STATE_CREATED;
    ret = window->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_OPERATION);

    property->subWindowLevel_ = 100;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = window->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_OPERATION);

    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ret = window->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_OPERATION);

    property->subWindowLevel_ = 1;
    ret = window->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: GetTargetOrientationConfigInfo
 * @tc.desc: GetTargetOrientationConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetTargetOrientationConfigInfo, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    Orientation targetOrientation = Orientation::USER_ROTATION_PORTRAIT;
    std::map<Rosen::WindowType, Rosen::SystemBarProperty> properties;
    Rosen::SystemBarProperty statusBarProperty;
    properties[Rosen::WindowType::WINDOW_TYPE_STATUS_BAR] = statusBarProperty;
    Ace::ViewportConfig config;
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    window->getTargetInfoCallback_ = sptr<FutureCallback>::MakeSptr();

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("GetTargetOrientationConfigInfo");
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_CREATED;
    EXPECT_EQ(WMError::WM_OK,
    window->GetTargetOrientationConfigInfo(targetOrientation, properties, config, avoidAreas));
}

/**
 * @tc.name: SetWindowTransitionAnimation
 * @tc.desc: SetWindowTransitionAnimation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetWindowTransitionAnimation01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowTransitionAnimation01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto property = window->GetProperty();

    WindowTransitionType type = WindowTransitionType::DESTROY;
    TransitionAnimation animation;

    property->SetWindowState(WindowState::STATE_DESTROYED);
    WMError ret = window->SetWindowTransitionAnimation(type, animation);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    property->persistentId_ = 100;

    property->SetWindowState(WindowState::STATE_SHOWN);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = window->SetWindowTransitionAnimation(type, animation);
    ASSERT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window->SetWindowTransitionAnimation(type, animation);
    ASSERT_EQ(ret, WMError::WM_OK);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = window->SetWindowTransitionAnimation(type, animation);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
}

/**
 * @tc.name: GetWindowTransitionAnimation
 * @tc.desc: GetWindowTransitionAnimation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetWindowTransitionAnimation01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowTransitionAnimation01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto property = window->GetProperty();

    WindowTransitionType type = WindowTransitionType::DESTROY;

    property->SetWindowState(WindowState::STATE_DESTROYED);
    std::shared_ptr<TransitionAnimation> ret = window->GetWindowTransitionAnimation(type);
    ASSERT_EQ(ret, nullptr);

    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    property->persistentId_ = 100;

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = window->GetWindowTransitionAnimation(type);
    ASSERT_EQ(ret, nullptr);

    property->SetWindowState(WindowState::STATE_SHOWN);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = window->GetWindowTransitionAnimation(type);
    ASSERT_EQ(ret, nullptr);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ret = window->GetWindowTransitionAnimation(type);
    ASSERT_EQ(ret, nullptr);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window->GetWindowTransitionAnimation(type);
    ASSERT_EQ(ret, nullptr);

    TransitionAnimation animation;
    property->SetTransitionAnimationConfig(type, animation);
    ret = window->GetWindowTransitionAnimation(type);
    ASSERT_NE(ret, nullptr);
}

/**
 * @tc.name: TransferLifeCycleEventToString
 * @tc.desc: TransferLifeCycleEventToString
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, TransferLifeCycleEventToString, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("TransferLifeCycleEventToString");
    option->SetDisplayId(0);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    window->RecordLifeCycleExceptionEvent(LifeCycleEvent::CREATE_EVENT, WMError::WM_ERROR_REPEAT_OPERATION);
    ASSERT_EQ(window->TransferLifeCycleEventToString(LifeCycleEvent::CREATE_EVENT), "CREATE");
    ASSERT_EQ(window->TransferLifeCycleEventToString(LifeCycleEvent::SHOW_EVENT), "SHOW");
    ASSERT_EQ(window->TransferLifeCycleEventToString(LifeCycleEvent::HIDE_EVENT), "HIDE");
    ASSERT_EQ(window->TransferLifeCycleEventToString(LifeCycleEvent::DESTROY_EVENT), "DESTROY");
}

/**
 * @tc.name: FillTargetOrientationConfig
 * @tc.desc: FillTargetOrientationConfig
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, FillTargetOrientationConfig, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    OrientationInfo info;
    info.rotation = 90;
    info.rect = {0, 0, 50, 50};
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    info.avoidAreas = avoidAreas;

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("FillTargetOrientationConfig");
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetDisplayId(3);
    window->state_ = WindowState::STATE_CREATED;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    Ace::ViewportConfig config = window->FillTargetOrientationConfig(info, displayInfo, 3);
    EXPECT_EQ(3, config.DisplayId());
}

/**
 * @tc.name: NotifyTargetRotationInfo
 * @tc.desc: NotifyTargetRotationInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, NotifyTargetRotationInfo, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    OrientationInfo info;
    info.rotation = 90;
    info.rect = {0, 0, 50, 50};
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    info.avoidAreas = avoidAreas;

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("NotifyTargetRotationInfo");
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_CREATED;
    EXPECT_EQ(WSError::WS_DO_NOTHING, window->NotifyTargetRotationInfo(info));
}

/**
 * @tc.name: StopMoveWindow
 * @tc.desc: StopMoveWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, StopMoveWindow, Function | SmallTest | TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT, window->StopMoveWindow());

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->state_ = WindowState::STATE_DESTROYED;
    ASSERT_EQ(WmErrorCode::WM_ERROR_STATE_ABNORMALLY, window->StopMoveWindow());

    window->state_ = WindowState::STATE_CREATED;
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    EXPECT_CALL(*session, SyncSessionEvent(_)).Times(1).WillOnce(Return(WSError::WS_ERROR_NULLPTR));
    ASSERT_EQ(WmErrorCode::WM_ERROR_STATE_ABNORMALLY, window->StopMoveWindow());

    EXPECT_CALL(*session, SyncSessionEvent(_)).Times(1).WillOnce(Return(WSError::WS_OK));
    ASSERT_EQ(WmErrorCode::WM_OK, window->StopMoveWindow());
}

/**
 * @tc.name: HandleWindowLimitsInCompatibleMode01
 * @tc.desc: HandleWindowLimitsInCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, HandleWindowLimitsInCompatibleMode01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetDisableWindowLimit(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    WindowSizeLimits windowSizeLimits = {0, 0, 0, 0};
    window->HandleWindowLimitsInCompatibleMode(windowSizeLimits);
    EXPECT_EQ(windowSizeLimits.maxWindowWidth, window->windowSystemConfig_.maxFloatingWindowSize_);
    EXPECT_EQ(windowSizeLimits.maxWindowHeight, window->windowSystemConfig_.maxFloatingWindowSize_);
    EXPECT_EQ(windowSizeLimits.minWindowWidth, window->windowSystemConfig_.miniWidthOfMainWindow_);
    EXPECT_EQ(windowSizeLimits.minWindowHeight, window->windowSystemConfig_.miniHeightOfMainWindow_);
}

/**
 * @tc.name: HandleWindowLimitsInCompatibleMode02
 * @tc.desc: HandleWindowLimitsInCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, HandleWindowLimitsInCompatibleMode02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetDisableWindowLimit(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    WindowSizeLimits windowSizeLimits = {0, 0, 0, 0};
    window->HandleWindowLimitsInCompatibleMode(windowSizeLimits);
    EXPECT_EQ(windowSizeLimits.maxWindowWidth, window->windowSystemConfig_.maxFloatingWindowSize_);
    EXPECT_EQ(windowSizeLimits.maxWindowHeight, window->windowSystemConfig_.maxFloatingWindowSize_);
    EXPECT_EQ(windowSizeLimits.minWindowWidth, window->windowSystemConfig_.miniWidthOfSubWindow_);
    EXPECT_EQ(windowSizeLimits.minWindowHeight, window->windowSystemConfig_.miniHeightOfSubWindow_);
}

/**
 * @tc.name: HandleWindowLimitsInCompatibleMode03
 * @tc.desc: HandleWindowLimitsInCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, HandleWindowLimitsInCompatibleMode03, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetDisableWindowLimit(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    WindowSizeLimits windowSizeLimits = {0, 0, 0, 0};
    window->HandleWindowLimitsInCompatibleMode(windowSizeLimits);
    EXPECT_EQ(windowSizeLimits.maxWindowWidth, window->windowSystemConfig_.maxFloatingWindowSize_);
    EXPECT_EQ(windowSizeLimits.maxWindowHeight, window->windowSystemConfig_.maxFloatingWindowSize_);
    EXPECT_EQ(windowSizeLimits.minWindowWidth, window->windowSystemConfig_.miniWidthOfDialogWindow_);
    EXPECT_EQ(windowSizeLimits.minWindowHeight, window->windowSystemConfig_.miniHeightOfDialogWindow_);
}

/**
 * @tc.name: HandleWindowLimitsInCompatibleMode04
 * @tc.desc: HandleWindowLimitsInCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, HandleWindowLimitsInCompatibleMode04, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetDisableWindowLimit(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->property_->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    WindowSizeLimits windowSizeLimits = {0, 0, 0, 0};
    window->HandleWindowLimitsInCompatibleMode(windowSizeLimits);
    EXPECT_EQ(windowSizeLimits.maxWindowWidth, window->windowSystemConfig_.maxFloatingWindowSize_);
    EXPECT_EQ(windowSizeLimits.maxWindowHeight, window->windowSystemConfig_.maxFloatingWindowSize_);
    EXPECT_EQ(windowSizeLimits.minWindowWidth, MIN_FLOATING_WIDTH);
    EXPECT_EQ(windowSizeLimits.minWindowHeight, MIN_FLOATING_HEIGHT);
}

/**
 * @tc.name: HandleWindowLimitsInCompatibleMode05
 * @tc.desc: HandleWindowLimitsInCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, HandleWindowLimitsInCompatibleMode05, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetDisableWindowLimit(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    WindowSizeLimits windowSizeLimits = {0, 0, 0, 0};
    window->HandleWindowLimitsInCompatibleMode(windowSizeLimits);
    EXPECT_EQ(windowSizeLimits.maxWindowWidth, 0);
    EXPECT_EQ(windowSizeLimits.maxWindowHeight, 0);
    EXPECT_EQ(windowSizeLimits.minWindowWidth, 0);
    EXPECT_EQ(windowSizeLimits.minWindowHeight, 0);
}

/**
 * @tc.name: IsDecorEnable1
 * @tc.desc: IsDecorEnable1
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, IsDecorEnable1, Function | SmallTest | Level2)
{
    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("IsDecorEnable1");
    subWindowOption->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(subWindowOption);
    window->property_->SetDecorEnable(true);
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    auto ret = window->IsDecorEnable();
    EXPECT_EQ(false, ret);
    subWindowOption->SetSubWindowMaximizeSupported(true);
    ret = window->IsDecorEnable();
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: GetAppForceLandscapeConfig01
 * @tc.desc: GetAppForceLandscapeConfig
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetAppForceLandscapeConfig01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
    AppForceLandscapeConfig config = {};
    auto res = window->GetAppForceLandscapeConfig(config);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(res, WMError::WM_OK);
        EXPECT_EQ(config.mode_, 0);
        EXPECT_EQ(config.homePage_, "");
        EXPECT_EQ(config.isSupportSplitMode_, false);
    }
}

/**
 * @tc.name: GetAppForceLandscapeConfig02
 * @tc.desc: GetAppForceLandscapeConfig
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetAppForceLandscapeConfig02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->hostSession_ = nullptr;

    AppForceLandscapeConfig config = {};
    auto res = window->GetAppForceLandscapeConfig(config);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
        EXPECT_EQ(config.mode_, 0);
        EXPECT_EQ(config.homePage_, "");
        EXPECT_EQ(config.isSupportSplitMode_, false);
    }
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigUpdated01
 * @tc.desc: NotifyAppForceLandscapeConfigUpdated
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, NotifyAppForceLandscapeConfigUpdated01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WSError res = window->NotifyAppForceLandscapeConfigUpdated();
    EXPECT_EQ(res, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigUpdated02
 * @tc.desc: NotifyAppForceLandscapeConfigUpdated
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, NotifyAppForceLandscapeConfigUpdated02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WSError res = window->NotifyAppForceLandscapeConfigUpdated();
    EXPECT_EQ(res, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: SetFrameRectForParticalZoomIn01
 * @tc.desc: Test SetFrameRectForParticalZoomIn when window type is invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetFrameRectForParticalZoomIn01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFrameRectForParticalZoomIn01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    Rect frameRect = { 10, 10, 10, 10 }; // 10 is valid frame rect param
    // window type is WINDOW_TYPE_APP_MAIN_WINDOW
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFrameRectForParticalZoomIn(frameRect));
}

/**
 * @tc.name: SetFrameRectForParticalZoomIn02
 * @tc.desc: Test SetFrameRectForParticalZoomIn when surfaceNode is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetFrameRectForParticalZoomIn02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFrameRectForParticalZoomIn02");
    option->SetWindowType(WindowType::WINDOW_TYPE_MAGNIFICATION);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->surfaceNode_ = nullptr;
    Rect frameRect = { 10, 10, 10, 10 }; // 10 is valid frame rect param
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFrameRectForParticalZoomIn(frameRect));
}

/**
 * @tc.name: SetFrameRectForParticalZoomIn03
 * @tc.desc: Test SetFrameRectForParticalZoomIn when display is invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetFrameRectForParticalZoomIn03, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFrameRectForParticalZoomIn03");
    option->SetWindowType(WindowType::WINDOW_TYPE_MAGNIFICATION);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_NE(nullptr, window->surfaceNode_);
    Rect frameRect = { 10, 10, 10, 10 }; // 10 is valid frame rect param
    // default displayId is -1
    EXPECT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, window->SetFrameRectForParticalZoomIn(frameRect));
}

/**
 * @tc.name: SetFrameRectForParticalZoomIn04
 * @tc.desc: SetFrameRectForParticalZoomIn when parameter frameRect is invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetFrameRectForParticalZoomIn04, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFrameRectForParticalZoomIn04");
    option->SetWindowType(WindowType::WINDOW_TYPE_MAGNIFICATION);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_NE(nullptr, window->surfaceNode_);
    Rect frameRect = { -1, 10, 10, 10 }; // -1 is invalid frame rect posX, 10 is valid frame rect param
    EXPECT_EQ(WSError::WS_OK, window->UpdateDisplayId(0)); // 0 is valid display id
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetFrameRectForParticalZoomIn(frameRect));
}

/**
 * @tc.name: SetFrameRectForParticalZoomIn05
 * @tc.desc: SetFrameRectForParticalZoomIn
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetFrameRectForParticalZoomIn05, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFrameRectForParticalZoomIn05");
    option->SetWindowType(WindowType::WINDOW_TYPE_MAGNIFICATION);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    EXPECT_NE(nullptr, window->surfaceNode_);
    Rect frameRect = { 10, 10, 10, 10 }; // 10 is valid frame rect param
    EXPECT_EQ(WSError::WS_OK, window->UpdateDisplayId(0)); // 0 is valid display id
    EXPECT_EQ(WMError::WM_OK, window->SetFrameRectForParticalZoomIn(frameRect));
}
}
} // namespace Rosen
} // namespace OHOS