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
#include "application_context.h"
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
namespace {
using WindowAdapterMocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
}
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
 * @tc.name: TestCheckWaterfallResidentState
 * @tc.desc: Verify CheckWaterfallResidentState validates states correctly based on window type
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, TestCheckWaterfallResidentState, TestSize.Level1)
{
    auto option = sptr<WindowOption>::MakeSptr();
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    // Case 1: Main window, any state should return true
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    EXPECT_TRUE(window->CheckWaterfallResidentState(WaterfallResidentState::UNCHANGED));
    EXPECT_TRUE(window->CheckWaterfallResidentState(WaterfallResidentState::OPEN));
    EXPECT_TRUE(window->CheckWaterfallResidentState(WaterfallResidentState::CLOSE));

    // Case 2: Sub window, only UNCHANGED should return true
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    EXPECT_TRUE(window->CheckWaterfallResidentState(WaterfallResidentState::UNCHANGED));
    EXPECT_FALSE(window->CheckWaterfallResidentState(WaterfallResidentState::OPEN));
    EXPECT_FALSE(window->CheckWaterfallResidentState(WaterfallResidentState::CLOSE));
}

/**
 * @tc.name: TestApplyMaximizePresentation
 * @tc.desc: Verify ApplyMaximizePresentation sets immersive mode and hover flags correctly for all presentations
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, TestApplyMaximizePresentation, TestSize.Level1)
{
    auto option = sptr<WindowOption>::MakeSptr();
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    // Case 1: ENTER_IMMERSIVE
    window->ApplyMaximizePresentation(MaximizePresentation::ENTER_IMMERSIVE);
    EXPECT_TRUE(window->enableImmersiveMode_);
    EXPECT_TRUE(window->titleHoverShowEnabled_);
    EXPECT_TRUE(window->dockHoverShowEnabled_);

    // Case 2: EXIT_IMMERSIVE
    window->ApplyMaximizePresentation(MaximizePresentation::EXIT_IMMERSIVE);
    EXPECT_FALSE(window->enableImmersiveMode_);
    EXPECT_TRUE(window->titleHoverShowEnabled_);
    EXPECT_TRUE(window->dockHoverShowEnabled_);

    // Case 3: ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER
    window->ApplyMaximizePresentation(MaximizePresentation::ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER);
    EXPECT_TRUE(window->enableImmersiveMode_);
    EXPECT_FALSE(window->titleHoverShowEnabled_);
    EXPECT_FALSE(window->dockHoverShowEnabled_);

    // Case 4: FOLLOW_APP_IMMERSIVE_SETTING
    window->enableImmersiveMode_ = false;
    window->ApplyMaximizePresentation(MaximizePresentation::FOLLOW_APP_IMMERSIVE_SETTING);
    EXPECT_FALSE(window->enableImmersiveMode_);
    EXPECT_TRUE(window->titleHoverShowEnabled_);
    EXPECT_TRUE(window->dockHoverShowEnabled_);

    // Case 5: Invalid enum value (default branch)
    auto invalidPresentation = static_cast<MaximizePresentation>(999);
    window->ApplyMaximizePresentation(invalidPresentation);
    EXPECT_TRUE(window->titleHoverShowEnabled_);
    EXPECT_TRUE(window->dockHoverShowEnabled_);
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

    // waterfallResidentState is invalid and windowType is invalid
    ret = window->Maximize(presentation, WaterfallResidentState::OPEN);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);

    // waterfallResidentState is valid but windowType is invalid
    ret = window->Maximize(presentation, WaterfallResidentState::UNCHANGED);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);

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
 * @tc.name: TestDefaultDisplayId
 * @tc.desc: test function: SetDefaultDisplayIdIfNee
 * @tc.desc: test function: SetIsFollowParentWindowDisplayId
 * @tc.desc: test function: IsFollowParentWindowDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, TestDefaultDisplayId, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetDisplayId(DISPLAY_ID_INVALID);
    window->SetDefaultDisplayIdIfNeed();
    auto ret = window->property_->IsFollowParentWindowDisplayId();
    EXPECT_EQ(true, ret);

    window->property_->SetIsFollowParentWindowDisplayId(false);
    ret = window->property_->IsFollowParentWindowDisplayId();
    EXPECT_EQ(false, ret);
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
    bool applyToSubWindow = false;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetCustomDensity(density, applyToSubWindow));

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->property_->SetWindowName("SetCustomDensity01");
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_CREATED;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, window->SetCustomDensity(density, applyToSubWindow));

    density = 1.5f;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, window->SetCustomDensity(density, applyToSubWindow));

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    EXPECT_EQ(WMError::WM_OK, window->SetCustomDensity(density, applyToSubWindow));
    EXPECT_EQ(density, window->customDensity_);
    applyToSubWindow = true;
    EXPECT_EQ(WMError::WM_OK, window->SetCustomDensity(density, applyToSubWindow));

    sptr<WindowSceneSessionImpl> window1;
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(window->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(1, window)));
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair("winTest",
        std::pair<uint64_t, sptr<WindowSessionImpl>>(2, window1)));
    EXPECT_EQ(WMError::WM_OK, window->SetCustomDensity(density, applyToSubWindow));
    EXPECT_EQ(WMError::WM_OK, window->SetCustomDensity(1.6f, false));
    WindowSceneSessionImpl::windowSessionMap_.erase(window->GetWindowName());
    WindowSceneSessionImpl::windowSessionMap_.erase("winTest");
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
    window->isEnableDefaultDensityWhenCreate_ = true;
    EXPECT_EQ(window->IsDefaultDensityEnabled(), true);
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
    window->SetCustomDensity(density, false);
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
    window->SetCustomDensity(density, false);
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
 * @tc.name: IsMainWindowFullScreenAcrossDisplays01
 * @tc.desc: IsMainWindowFullScreenAcrossDisplays
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, IsMainWindowFullScreenAcrossDisplays01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    bool isAcrossDisplays = false;
    window->hostSession_ = nullptr;
    auto ret = window->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_CREATED;
    ret = window->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RecoverSessionProperty
 * @tc.desc: RecoverSessionProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, RecoverSessionProperty, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->RecoverSessionProperty();
}

/**
 * @tc.name: UpdateColorMode
 * @tc.desc: UpdateColorMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, UpdateColorMode, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;
    auto ret = window->UpdateColorMode(configuration);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    AbilityRuntime::ApplicationContext::applicationContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    ret = window->UpdateColorMode(configuration);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    AbilityRuntime::ApplicationContext::applicationContext_->contextImpl_ =
        std::make_shared<AbilityRuntime::ContextImpl>();
    ret = window->UpdateColorMode(configuration);
    configuration = std::make_shared<AppExecFwk::Configuration>();
    configuration->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    ret = window->UpdateColorMode(configuration);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = window->UpdateColorMode(configuration);
    EXPECT_EQ(WMError::WM_OK, ret);
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
    floatWindow->windowSystemConfig_.freeMultiWindowEnable_ = false;
    EXPECT_EQ(WSError::WS_OK, floatWindow->SwitchFreeMultiWindow(true));
    EXPECT_EQ(true, floatWindow->IsPcOrPadFreeMultiWindowMode());
    EXPECT_EQ(WSError::WS_OK, subWindow->SwitchFreeMultiWindow(true));
    EXPECT_EQ(true, subWindow->IsPcOrPadFreeMultiWindowMode());

    EXPECT_EQ(WMError::WM_OK, mainWindow->Destroy(true));
    EXPECT_EQ(WMError::WM_OK, floatWindow->Destroy(true));
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
}

/**
 * @tc.name: SwitchFreeMultiWindow03
 * @tc.desc: SwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SwitchFreeMultiWindow03, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
 
    sptr<WindowSceneSessionImpl> mainWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    mainWindow->property_->SetPersistentId(1);
    mainWindow->hostSession_ = session;
    mainWindow->property_->SetWindowName("SwitchFreeMultiWindow03_mainWindow");
    mainWindow->windowSystemConfig_.freeMultiWindowEnable_ = true;
    mainWindow->windowSystemConfig_.freeMultiWindowSupport_ = true;
    mainWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    mainWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(mainWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(mainWindow->GetWindowId(), mainWindow)));
    mainWindow->haveSetSupportedWindowModes_ = true;
    mainWindow->property_->SetWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING);
    EXPECT_EQ(mainWindow->property_->GetWindowModeSupportType(), WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING);
    EXPECT_EQ(mainWindow->SwitchFreeMultiWindow(false), WSError::WS_OK);
    EXPECT_EQ(WindowHelper::IsWindowModeSupported(mainWindow->property_->GetWindowModeSupportType(),
        WindowMode::WINDOW_MODE_FULLSCREEN), false);
    EXPECT_EQ(mainWindow->windowSystemConfig_.freeMultiWindowEnable_, false);
    WindowSceneSessionImpl::windowSessionMap_.erase(mainWindow->property_->GetWindowName());
}

/**
 * @tc.name: SwitchFreeMultiWindow04
 * @tc.desc: SwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SwitchFreeMultiWindow04, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
 
    sptr<WindowSceneSessionImpl> mainWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    mainWindow->property_->SetPersistentId(1);
    mainWindow->hostSession_ = session;
    mainWindow->property_->SetWindowName("SwitchFreeMultiWindow04_mainWindow");
    mainWindow->windowSystemConfig_.freeMultiWindowSupport_ = true;
    mainWindow->windowSystemConfig_.freeMultiWindowEnable_ = true;
    mainWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    mainWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(mainWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(mainWindow->GetWindowId(), mainWindow)));
    mainWindow->haveSetSupportedWindowModes_ = true;
    mainWindow->property_->SetWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN);
    EXPECT_EQ(mainWindow->property_->GetWindowModeSupportType(), WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN);
    EXPECT_EQ(mainWindow->SwitchFreeMultiWindow(false), WSError::WS_OK);
    EXPECT_EQ(mainWindow->windowSystemConfig_.freeMultiWindowEnable_, false);

    mainWindow->pendingWindowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN;
    EXPECT_EQ(mainWindow->SwitchFreeMultiWindow(true), WSError::WS_OK);
    EXPECT_EQ(mainWindow->property_->GetWindowModeSupportType(), WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN);
    WindowSceneSessionImpl::windowSessionMap_.erase(mainWindow->property_->GetWindowName());
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
    keyboardWindow->property_->SetWindowName("ShowKeyboard01");
    keyboardWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardWindow->state_ = WindowState::STATE_DESTROYED;

    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::DARK_IMMERSIVE_MODE;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::BACKGROUND_FLOW_LIGHT;
    effectOption.gradientMode_ = KeyboardGradientMode::LINEAR_GRADIENT;
    // normal value
    ASSERT_EQ(keyboardWindow->ShowKeyboard(1000, 0, effectOption), WMError::WM_ERROR_INVALID_WINDOW);

    effectOption.viewMode_ = KeyboardViewMode::VIEW_MODE_END;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::END;
    effectOption.gradientMode_ = KeyboardGradientMode::END;
    // exception value
    ASSERT_EQ(keyboardWindow->ShowKeyboard(1000, 0, effectOption), WMError::WM_ERROR_INVALID_WINDOW);
    auto lastOption = keyboardWindow->property_->GetKeyboardEffectOption();
    ASSERT_EQ(lastOption.viewMode_, KeyboardViewMode::NON_IMMERSIVE_MODE);
}

/**
 * @tc.name: ChangeKeyboardEffectOption01
 * @tc.desc: SwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, ChangeKeyboardEffectOption01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowSceneSessionImpl> keyboardWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    keyboardWindow->property_->SetPersistentId(1000);
    keyboardWindow->hostSession_ = session;
    keyboardWindow->property_->SetWindowName("ChangeKeyboardEffectOption01");
    keyboardWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);

    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::NON_IMMERSIVE_MODE;
    keyboardWindow->property_->SetKeyboardEffectOption(effectOption);
    auto result = WMError::WM_OK;
    // exception mode value
    effectOption.viewMode_ = KeyboardViewMode::VIEW_MODE_END;
    result = keyboardWindow->ChangeKeyboardEffectOption(effectOption);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);

    effectOption.viewMode_ = KeyboardViewMode::NON_IMMERSIVE_MODE;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::END;
    result = keyboardWindow->ChangeKeyboardEffectOption(effectOption);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);

    // same mode
    effectOption.viewMode_ = KeyboardViewMode::NON_IMMERSIVE_MODE;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::NONE;
    result = keyboardWindow->ChangeKeyboardEffectOption(effectOption);
    ASSERT_EQ(result, WMError::WM_DO_NOTHING);

    // invalid window state
    keyboardWindow->state_ = WindowState::STATE_DESTROYED;
    effectOption.viewMode_ = KeyboardViewMode::LIGHT_IMMERSIVE_MODE;
    result = keyboardWindow->ChangeKeyboardEffectOption(effectOption);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_WINDOW);

    // window state not shown
    keyboardWindow->state_ = WindowState::STATE_HIDDEN;
    effectOption.viewMode_ = KeyboardViewMode::LIGHT_IMMERSIVE_MODE;
    result = keyboardWindow->ChangeKeyboardEffectOption(effectOption);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_WINDOW);

    keyboardWindow->state_ = WindowState::STATE_SHOWN;
    effectOption.viewMode_ = KeyboardViewMode::DARK_IMMERSIVE_MODE;
    result = keyboardWindow->ChangeKeyboardEffectOption(effectOption);
    ASSERT_EQ(result, WMError::WM_OK);
    auto lastOption = keyboardWindow->property_->GetKeyboardEffectOption();
    ASSERT_EQ(lastOption.viewMode_, KeyboardViewMode::DARK_IMMERSIVE_MODE);
}

/**
 * @tc.name: ChangeKeyboardEffectOption02
 * @tc.desc: SwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, ChangeKeyboardEffectOption02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowSceneSessionImpl> keyboardWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    keyboardWindow->property_->SetPersistentId(1000);
    keyboardWindow->hostSession_ = session;
    keyboardWindow->property_->SetWindowName("ChangeKeyboardEffectOption02");
    keyboardWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);

    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::NON_IMMERSIVE_MODE;
    keyboardWindow->property_->SetKeyboardEffectOption(effectOption);
    auto result = WMError::WM_OK;
    // exception mode value
    effectOption.viewMode_ = KeyboardViewMode::VIEW_MODE_END;
    result = keyboardWindow->ChangeKeyboardEffectOption(effectOption);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);

    effectOption.viewMode_ = KeyboardViewMode::NON_IMMERSIVE_MODE;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::END;
    result = keyboardWindow->ChangeKeyboardEffectOption(effectOption);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);

    effectOption.viewMode_ = KeyboardViewMode::NON_IMMERSIVE_MODE;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::NONE;
    effectOption.gradientMode_ = KeyboardGradientMode::END;
    result = keyboardWindow->ChangeKeyboardEffectOption(effectOption);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);
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
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_SHOWN;
    std::unordered_map<WindowType, SystemBarProperty> systemBarProperties;
    systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarProperty();
    std::unordered_map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR] = { false, false, false, false };
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR] = { true, false, true, false };
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
    window->state_ = WindowState::STATE_DESTROYED;
    systemBarPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR] = { true, true, false, false };
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
        window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
    window->state_ = WindowState::STATE_SHOWN;
    systemBarPropertyFlags[WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE] = { false, false, false, false };
    EXPECT_EQ(WMError::WM_DO_NOTHING, window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
}

/**
 * @tc.name: UpdateSystemBarPropertyForPage
 * @tc.desc: UpdateSystemBarPropertyForPage test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, UpdateSystemBarPropertyForPage, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSystemBarProperty");
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_DESTROYED;
    auto type = WindowType::WINDOW_TYPE_STATUS_BAR;
    SystemBarProperty systemBarProperty;
    SystemBarPropertyFlag systemBarPropertyFlag;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW,
        window->UpdateSystemBarPropertyForPage(type, systemBarProperty, systemBarPropertyFlag));
    window->state_ = WindowState::STATE_SHOWN;
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarPropertyForPage(type, systemBarProperty, systemBarPropertyFlag));
    window->nowsystemBarPropertyMap_[type] = systemBarProperty;
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarPropertyForPage(type, systemBarProperty, systemBarPropertyFlag));
    systemBarPropertyFlag = { true, false, false, false };
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarPropertyForPage(type, systemBarProperty, systemBarPropertyFlag));
    systemBarPropertyFlag = { true, true, false, false };
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarPropertyForPage(type, systemBarProperty, systemBarPropertyFlag));
    systemBarPropertyFlag = { true, true, true, false };
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarPropertyForPage(type, systemBarProperty, systemBarPropertyFlag));
    systemBarPropertyFlag = { true, true, true, true };
    EXPECT_EQ(WMError::WM_OK, window->UpdateSystemBarPropertyForPage(type, systemBarProperty, systemBarPropertyFlag));
}

/**
 * @tc.name: UpdateStatusBarColorHistory
 * @tc.desc: UpdateStatusBarColorHistory test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, UpdateStatusBarColorHistory, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateStatusBarColorHistory");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto reason = StatusBarColorChangeReason::WINDOW_CONFIGURATION;
    EXPECT_EQ(window->UpdateStatusBarColorHistory(reason, std::optional<uint32_t>(1)), 1);
    reason = StatusBarColorChangeReason::NAVIGATION_CONFIGURATION;
    std::optional<uint32_t> op;
    EXPECT_EQ(window->UpdateStatusBarColorHistory(reason, op), 1);
    EXPECT_EQ(window->UpdateStatusBarColorHistory(reason, std::optional<uint32_t>(1)), 1);
    EXPECT_EQ(window->UpdateStatusBarColorHistory(reason, std::optional<uint32_t>(1)), 1);
}

/**
 * @tc.name: SetSystemBarPropertyForPage
 * @tc.desc: SetSystemBarPropertyForPage
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetSystemBarPropertyForPage, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSystemBarPropertyForPage");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPersistentId(1);
    windowSceneSessionImpl->property_ = property;
    std::optional<SystemBarProperty> prop;
    windowSceneSessionImpl->state_ = WindowState::STATE_DESTROYED;
    auto ret = windowSceneSessionImpl->SetSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, prop);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    ret = windowSceneSessionImpl->SetSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, prop);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);
    windowSceneSessionImpl->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = windowSceneSessionImpl->SetSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, prop);
    EXPECT_EQ(WMError::WM_OK, ret);
    std::optional<SystemBarProperty> prop1 = SystemBarProperty();
    ret = windowSceneSessionImpl->SetSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, prop1);
    EXPECT_EQ(WMError::WM_OK, ret);
}


/**
 * @tc.name: SetStatusBarColorForPage
 * @tc.desc: SetStatusBarColorForPage
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetStatusBarColorForPage, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetStatusBarColorForPage");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPersistentId(1);
    window->property_ = property;
    window->state_ = WindowState::STATE_SHOWN;
    std::optional<uint32_t> color;
    EXPECT_EQ(WMError::WM_DO_NOTHING, window->SetStatusBarColorForPage(color));
    window->isAtomicServiceUseColor_ = true;
    EXPECT_EQ(WMError::WM_OK, window->SetStatusBarColorForPage(color));
    window->isAtomicServiceUseColor_ = true;
    window->isNavigationUseColor_ = true;
    window->statusBarColorHistory_.push(
        std::pair<StatusBarColorChangeReason,
        uint32_t>(StatusBarColorChangeReason::NAVIGATION_CONFIGURATION, 1));
    EXPECT_EQ(WMError::WM_OK, window->SetStatusBarColorForPage(color));
    window->isAtomicServiceUseColor_ = true;
    window->statusBarColorHistory_.push(
        std::pair<StatusBarColorChangeReason,
        uint32_t>(StatusBarColorChangeReason::ATOMICSERVICE_CONFIGURATION, 1));
    EXPECT_EQ(WMError::WM_OK, window->SetStatusBarColorForPage(color));
    EXPECT_EQ(WMError::WM_OK, window->SetStatusBarColorForPage(std::optional<uint32_t>(0x00FFFFFF)));
}

/**
 * @tc.name: SetSystemBarProperties
 * @tc.desc: SetSystemBarProperties test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetSystemBarProperties, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::map<WindowType, SystemBarProperty> properties;
    std::map<WindowType, SystemBarPropertyFlag> propertyFlags;
    EXPECT_EQ(window->SetSystemBarProperties(properties, propertyFlags), WMError::WM_OK);
    auto type = WindowType::WINDOW_TYPE_STATUS_BAR;
    propertyFlags[type] = SystemBarPropertyFlag();
    EXPECT_EQ(window->SetSystemBarProperties(properties, propertyFlags), WMError::WM_OK);
    propertyFlags[type].contentColorFlag = true;
    EXPECT_EQ(window->SetSystemBarProperties(properties, propertyFlags), WMError::WM_OK);
    properties[type] = SystemBarProperty();
    window->nowsystemBarPropertyMap_[type] = SystemBarProperty();
    EXPECT_EQ(window->SetSystemBarProperties(properties, propertyFlags), WMError::WM_OK);
    window->nowsystemBarPropertyMap_[type].contentColor_ = 0;
    EXPECT_EQ(window->SetSystemBarProperties(properties, propertyFlags), WMError::WM_OK);
}

/**
 * @tc.name: GetSystemBarProperties
 * @tc.desc: GetSystemBarProperties test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetSystemBarProperties, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::map<WindowType, SystemBarProperty> properties;
    EXPECT_EQ(WMError::WM_OK, window->GetSystemBarProperties(properties));
    window->nowsystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarProperty();
    EXPECT_EQ(WMError::WM_OK, window->GetSystemBarProperties(properties));
}

/**
 * @tc.name: updateSystemBarproperty
 * @tc.desc: updateSystemBarproperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, updateSystemBarproperty, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("updateSystemBarproperty");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    SystemBarProperty prop = SystemBarProperty();
    window->updateSystemBarproperty(WindowType::WINDOW_TYPE_STATUS_BAR, prop);
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPersistentId(1);
    window->property_ = property;
    window->state_ = WindowState::STATE_SHOWN;
    EXPECT_EQ(WMError::WM_OK, window->updateSystemBarproperty(WindowType::WINDOW_TYPE_STATUS_BAR, prop));
}

/**
 * @tc.name: MobileAppInPadLayoutFullScreenChange
 * @tc.desc: MobileAppInPadLayoutFullScreenChange test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, MobileAppInPadLayoutFullScreenChange, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MobileAppInPadLayoutFullScreenChange");
    option->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;

    window->property_->SetMobileAppInPadLayoutFullScreen(true);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    bool statusBarEnable = true;
    bool navigationEnable = true;
    window->enableImmersiveMode_ = true;
    window->MobileAppInPadLayoutFullScreenChange(statusBarEnable, navigationEnable);
    EXPECT_EQ(false, window->enableImmersiveMode_);
    statusBarEnable = false;
    navigationEnable = false;
    window->enableImmersiveMode_ = false;
    window->MobileAppInPadLayoutFullScreenChange(statusBarEnable, navigationEnable);
    EXPECT_EQ(true, window->enableImmersiveMode_);
    statusBarEnable = false;
    navigationEnable = true;
    window->MobileAppInPadLayoutFullScreenChange(statusBarEnable, navigationEnable);
    statusBarEnable = true;
    navigationEnable = false;
    window->MobileAppInPadLayoutFullScreenChange(statusBarEnable, navigationEnable);
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
 * @tc.name: Resume
 * @tc.desc: Resume
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, Resume, TestSize.Level1)
{
    sptr<MockWindowStageLifeCycleListener> mockListener = sptr<MockWindowStageLifeCycleListener>::MakeSptr();
    sptr<IWindowStageLifeCycle> listener = static_cast<sptr<IWindowStageLifeCycle>>(mockListener);

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Test");
    option->SetDisplayId(0);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    EXPECT_EQ(WMError::WM_OK, window->RegisterWindowStageLifeCycleListener(listener));
    window->isDidForeground_ = false;
    window->isColdStart_ = true;
    window->state_ = WindowState::STATE_SHOWN;
    EXPECT_CALL(*mockListener, AfterLifecycleResumed()).Times(1);
    window->Resume();
    EXPECT_EQ(window->isDidForeground_, true);
    EXPECT_EQ(window->isColdStart_, false);

    EXPECT_CALL(*mockListener, AfterLifecyclePaused()).Times(1);
    window->Pause();
    EXPECT_EQ(window->isDidForeground_, true);
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: NotifyDialogStateChange
 * @tc.desc: NotifyDialogStateChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, NotifyDialogStateChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyDialogStateChange");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    auto ret = windowSceneSessionImpl->NotifyDialogStateChange(true);
    EXPECT_EQ(WSError::WS_OK, ret);
    windowSceneSessionImpl->state_ = WindowState::STATE_HIDDEN;
    windowSceneSessionImpl->requestState_ = WindowState::STATE_SHOWN;
    ret = windowSceneSessionImpl->NotifyDialogStateChange(true);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: NotifyFreeMultiWindowModeResume
 * @tc.desc: NotifyFreeMultiWindowModeResume
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, NotifyFreeMultiWindowModeResume, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyFreeMultiWindowModeResume");
    option->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> multiWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    multiWindow->hostSession_ = session;
    multiWindow->property_->SetPersistentId(1);
    multiWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(true, multiWindow->IsPcOrFreeMultiWindowCapabilityEnabled());
    multiWindow->Resume();
    EXPECT_EQ(false, multiWindow->isColdStart_);
    multiWindow->NotifyFreeMultiWindowModeResume();
    EXPECT_EQ(true, multiWindow->isDidForeground_);
    EXPECT_EQ(WMError::WM_OK, multiWindow->Destroy(true));
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
    sptr<Window> parentWindow = nullptr;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetPcAppInpadCompatibleMode(true);
    auto res = window->GetParentWindow(parentWindow);
    EXPECT_EQ(res, WMError::WM_OK);
    window->property_->SetPcAppInpadCompatibleMode(false);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    res = window->GetParentWindow(parentWindow);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->property_->SetParentPersistentId(2);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

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
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
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
    int32_t newParentWindowId = 3;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

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
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    subWindow->property_->SetPcAppInpadCompatibleMode(true);
    subWindow->windowSystemConfig_.freeMultiWindowEnable_ = true;
    subWindow->windowSystemConfig_.freeMultiWindowSupport_ = true;
    EXPECT_EQ(subWindow->SetParentWindow(newParentWindowId), WMError::WM_OK);
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
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ret = window->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);

    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->subWindowLevel_ = 100;
    ret = window->SetWindowAnchorInfo(windowAnchorInfo);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);

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
    window->windowSystemConfig_.supportFollowParentWindowLayout_ = false;
    ret = window->SetFollowParentWindowLayoutEnabled(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.supportFollowParentWindowLayout_ = true;
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

    std::map<Rosen::WindowType, Rosen::SystemBarProperty> targetProperties;
    Rosen::SystemBarProperty targetStatusBarProperty;
    targetProperties[Rosen::WindowType::WINDOW_TYPE_STATUS_BAR] = targetStatusBarProperty;
    std::make_shared<Ace::ViewportConfig> targetConfig;
    std::map<AvoidAreaType, AvoidArea> targetAvoidAreas;
    Rosen::ViewportConfigAndAvoidArea targetViewportConfigAndAvoidArea = { targetConfig, targetAvoidAreas };

    std::map<Rosen::WindowType, Rosen::SystemBarProperty> currentProperties;
    Rosen::SystemBarProperty currentStatusBarProperty;
    currentProperties[Rosen::WindowType::WINDOW_TYPE_STATUS_BAR] = currentStatusBarProperty;
    std::make_shared<Ace::ViewportConfig> currentConfig;
    std::map<AvoidAreaType, AvoidArea> currentAvoidAreas;
    Rosen::ViewportConfigAndAvoidArea currentViewportConfigAndAvoidArea = { currentConfig, currentAvoidAreas };

    window->getTargetInfoCallback_ = sptr<FutureCallback>::MakeSptr();

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("GetTargetOrientationConfigInfo");
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_CREATED;
    EXPECT_EQ(WMError::WM_OK,
    window->GetTargetOrientationConfigInfo(targetOrientation, targetProperties,
        currentProperties, targetViewportConfigAndAvoidArea, currentViewportConfigAndAvoidArea));
    targetOrientation = Orientation::INVALID;
    EXPECT_EQ(WMError::WM_OK,
    window->GetTargetOrientationConfigInfo(targetOrientation, targetProperties,
        currentProperties, targetViewportConfigAndAvoidArea, currentViewportConfigAndAvoidArea));
}

/**
 * @tc.name: IsTransitionAnimationSupported
 * @tc.desc: IsTransitionAnimationSupported
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, IsTransitionAnimationSupported, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsTransitionAnimationSupported");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto property = window->GetProperty();
    SessionInfo sessionInfo;
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    property->persistentId_ = 100;

    window->state_ = WindowState::STATE_DESTROYED;
    WMError ret = window->IsTransitionAnimationSupported();
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->state_ = WindowState::STATE_SHOWN;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = window->IsTransitionAnimationSupported();
    ASSERT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    property->SetIsPcAppInPad(true);
    ret = window->IsTransitionAnimationSupported();
    ASSERT_EQ(ret, WMError::WM_OK);

    property->SetIsPcAppInPad(false);
    ret = window->IsTransitionAnimationSupported();
    ASSERT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window->IsTransitionAnimationSupported();
    ASSERT_EQ(ret, WMError::WM_OK);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    ret = window->IsTransitionAnimationSupported();
    ASSERT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    ret = window->IsTransitionAnimationSupported();
    ASSERT_EQ(ret, WMError::WM_OK);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = window->IsTransitionAnimationSupported();
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
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

    property->SetIsPcAppInPad(true);
    ret = window->SetWindowTransitionAnimation(type, animation);
    ASSERT_EQ(ret, WMError::WM_OK);
    property->SetIsPcAppInPad(false);
    ret = window->SetWindowTransitionAnimation(type, animation);
    ASSERT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window->SetWindowTransitionAnimation(type, animation);
    ASSERT_EQ(ret, WMError::WM_OK);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    ret = window->SetWindowTransitionAnimation(type, animation);
    ASSERT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
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

    property->SetWindowState(WindowState::STATE_SHOWN);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = window->GetWindowTransitionAnimation(type);
    ASSERT_EQ(ret, nullptr);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    TransitionAnimation animation;
    property->SetTransitionAnimationConfig(type, animation);
    property->SetIsPcAppInPad(true);
    ret = window->GetWindowTransitionAnimation(type);
    ASSERT_NE(ret, nullptr);
    property->SetIsPcAppInPad(false);
    ret = window->GetWindowTransitionAnimation(type);
    ASSERT_EQ(ret, nullptr);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window->GetWindowTransitionAnimation(type);
    ASSERT_NE(ret, nullptr);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    ret = window->GetWindowTransitionAnimation(type);
    ASSERT_NE(ret, nullptr);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = window->GetWindowTransitionAnimation(type);
    ASSERT_EQ(ret, nullptr);
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

    OrientationInfo currentInfo;
    currentInfo.rotation = 90;
    currentInfo.rect = {0, 0, 50, 50};
    std::map<AvoidAreaType, AvoidArea> currentAvoidAreas;
    currentInfo.avoidAreas = currentAvoidAreas;

    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetWindowName("NotifyTargetRotationInfo");
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_CREATED;
    EXPECT_EQ(WSError::WS_DO_NOTHING, window->NotifyTargetRotationInfo(info, currentInfo));
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
        EXPECT_EQ(config.supportSplit_, -1);
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
        EXPECT_EQ(config.supportSplit_, -1);
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
 * @tc.name: SetFrameRectForPartialZoomIn01
 * @tc.desc: Test SetFrameRectForPartialZoomIn when window type is invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetFrameRectForPartialZoomIn01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFrameRectForPartialZoomIn01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    Rect frameRect = { 10, 10, 10, 10 }; // 10 is valid frame rect param
    // window type is WINDOW_TYPE_APP_MAIN_WINDOW
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFrameRectForPartialZoomIn(frameRect));
}

/**
 * @tc.name: SetFrameRectForPartialZoomIn02
 * @tc.desc: Test SetFrameRectForPartialZoomIn when window session is invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetFrameRectForPartialZoomIn02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFrameRectForPartialZoomIn02");
    option->SetWindowType(WindowType::WINDOW_TYPE_MAGNIFICATION);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    Rect frameRect = { 10, 10, 10, 10 }; // 10 is valid frame rect param
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetFrameRectForPartialZoomIn(frameRect));

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SetFrameRectForPartialZoomIn02";
    sessionInfo.abilityName_ = "SetFrameRectForPartialZoomIn02";
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->persistentId_ = 100; // 100 is persistent id
    window->state_ = WindowState::STATE_CREATED;
    EXPECT_EQ(WMError::WM_OK, window->SetFrameRectForPartialZoomIn(frameRect));
}

/**
 * @tc.name: UpdateEnableDragWhenSwitchMultiWindow
 * @tc.desc: UpdateEnableDragWhenSwitchMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, UpdateEnableDragWhenSwitchMultiWindow, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateEnableDragWhenSwitchMultiWindow");
    option->SetWindowType(WindowType::WINDOW_TYPE_MAGNIFICATION);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->hasSetEnableDrag_.store(true);
    window->UpdateEnableDragWhenSwitchMultiWindow(false);
    EXPECT_EQ(true, window->property_->dragEnabled_);

    window->hasSetEnableDrag_.store(false);
    window->UpdateEnableDragWhenSwitchMultiWindow(false);
    EXPECT_EQ(false, window->property_->dragEnabled_);

    window->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->disableDragResize_ = true;
    window->property_->compatibleModeProperty_ = property;
    window->UpdateEnableDragWhenSwitchMultiWindow(true);
    EXPECT_EQ(false, window->property_->dragEnabled_);

    property->disableDragResize_ = false;
    window->UpdateEnableDragWhenSwitchMultiWindow(true);
    EXPECT_EQ(true, window->property_->dragEnabled_);
}

/**
 * @tc.name: GetConfigurationFromAbilityInfo
 * @tc.desc: GetConfigurationFromAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetConfigurationFromAbilityInfo, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetConfigurationFromAbilityInfo");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::shared_ptr<AbilityRuntime::AbilityContextImpl> context =
        std::make_shared<AbilityRuntime::AbilityContextImpl>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    context->SetAbilityInfo(info);
    window->context_ = context;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsSupportRotateFullScreen(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    window->GetConfigurationFromAbilityInfo();
    auto supportType = window->property_->GetWindowModeSupportType();
    auto expceted = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;
    EXPECT_EQ(supportType & expceted, expceted);
}

/**
 * @tc.name: GetConfigurationFromAbilityInfo02
 * @tc.desc: GetConfigurationFromAbilityInfo02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetConfigurationFromAbilityInfo02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetConfigurationFromAbilityInfo02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::shared_ptr<AbilityRuntime::AbilityContextImpl> context =
        std::make_shared<AbilityRuntime::AbilityContextImpl>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    context->SetAbilityInfo(info);
    window->context_ = context;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToDragScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->enableImmersiveMode_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->property_->SetCollaboratorType(static_cast<int32_t>(CollaboratorType::RESERVE_TYPE));
    window->GetConfigurationFromAbilityInfo();
    EXPECT_EQ(window->enableImmersiveMode_, false);
}

/**
 * @tc.name: UpdateDecorEnable
 * @tc.desc: UpdateDecorEnable test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, UpdateDecorEnable, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSceneSessionImpl: UpdateDecorEnable start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateDecorEnable");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    WindowMode mode = WindowMode::WINDOW_MODE_FULLSCREEN;
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window->property_->windowMode_ = WindowMode::WINDOW_MODE_FULLSCREEN;
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->property_->SetCollaboratorType(static_cast<int32_t>(CollaboratorType::RESERVE_TYPE));
    window->UpdateDecorEnable(true, mode);
    ASSERT_TRUE(window->IsDecorEnable());

    window->UpdateDecorEnable(true, mode);
    window->property_->windowMode_ = WindowMode::WINDOW_MODE_FLOATING;
    ASSERT_TRUE(window->IsDecorEnable());
}

/**
 * @tc.name: TestMoveWindowToGlobalDisplay
 * @tc.desc: Test MoveWindowToGlobalDisplay under multiple conditions
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, TestMoveWindowToGlobalDisplay, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    sptr<SessionMocker> mockHostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    auto property = window->GetProperty();

    property->SetPersistentId(123);
    property->SetGlobalDisplayRect({ 100, 100, 300, 300 });
    property->SetRequestRect({ 0, 0, 300, 300 });

    // Case 1: session is null
    window->hostSession_ = nullptr;
    auto ret = window->MoveWindowToGlobalDisplay(100, 100);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    // Case 2: windowMode is not floating
    window->hostSession_ = mockHostSession;
    property->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ret = window->MoveWindowToGlobalDisplay(100, 100);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS);

    // Case 3: Same position
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ret = window->MoveWindowToGlobalDisplay(100, 100);
    EXPECT_EQ(ret, WMError::WM_OK);

    // Case 4: Illegal position
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ret = window->MoveWindowToGlobalDisplay(INT32_MAX, INT32_MAX);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    EXPECT_CALL(*mockHostSession, UpdateGlobalDisplayRectFromClient(_, _)).Times(2).WillOnce(Return(WSError::WS_OK));

    // Case 5: Move to new position
    window->state_ = WindowState::STATE_SHOWN;
    ret = window->MoveWindowToGlobalDisplay(200, 300);
    EXPECT_EQ(ret, WMError::WM_OK);

    // Case 6: Window is not shown
    window->state_ = WindowState::STATE_HIDDEN;
    ret = window->MoveWindowToGlobalDisplay(200, 300);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: GetDragAreaByDownEvent01
 * @tc.desc: GetDragAreaByDownEvent01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetDragAreaByDownEvent01, TestSize.Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetDragAreaByDownEvent01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;

    windowSceneSessionImpl->SetUniqueVirtualPixelRatio(true, 0.0f);
    AreaType dragType = windowSceneSessionImpl->GetDragAreaByDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(dragType, AreaType::UNDEFINED);
}

/**
 * @tc.name: GetDragAreaByDownEvent02
 * @tc.desc: GetDragAreaByDownEvent02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetDragAreaByDownEvent02, TestSize.Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetDragAreaByDownEvent02");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    pointerEvent->SetSourceType(1);
    MMI::PointerEvent::PointerItem pointerItem;

    windowSceneSessionImpl->SetUniqueVirtualPixelRatio(true, 1.0f);
    Rect rect = {1, 1, 1, 1};
    windowSceneSessionImpl->property_->SetWindowRect(rect);
    pointerItem.SetWindowX(1);
    pointerItem.SetWindowY(1);
    windowSceneSessionImpl->property_->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    windowSceneSessionImpl->property_->SetDragEnabled(true);
    windowSceneSessionImpl->SetDragActivated(true);
    AreaType dragType = windowSceneSessionImpl->GetDragAreaByDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(dragType, AreaType::LEFT_TOP);
    windowSceneSessionImpl->property_->SetDragEnabled(false);
    windowSceneSessionImpl->SetDragActivated(false);
    dragType = windowSceneSessionImpl->GetDragAreaByDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(dragType, AreaType::UNDEFINED);
}

/**
 * @tc.name: GetDragAreaByDownEvent03
 * @tc.desc: GetDragAreaByDownEvent03
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetDragAreaByDownEvent03, TestSize.Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetDragAreaByDownEvent03");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    pointerEvent->SetSourceType(1);
    MMI::PointerEvent::PointerItem pointerItem;

    windowSceneSessionImpl->SetUniqueVirtualPixelRatio(true, 1.0f);
    Rect rect = {1, 1, 1, 1};
    windowSceneSessionImpl->property_->SetWindowRect(rect);
    pointerItem.SetWindowX(1);
    pointerItem.SetWindowY(1);
    windowSceneSessionImpl->property_->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FULLSCREEN);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    windowSceneSessionImpl->property_->SetDragEnabled(true);
    AreaType dragType = windowSceneSessionImpl->GetDragAreaByDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(dragType, AreaType::LEFT_TOP);
}

/**
 * @tc.name: GetDragAreaByDownEvent04
 * @tc.desc: GetDragAreaByDownEvent04
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetDragAreaByDownEvent04, TestSize.Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetDragAreaByDownEvent04");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    pointerEvent->SetSourceType(1);
    MMI::PointerEvent::PointerItem pointerItem;
    windowSceneSessionImpl->SetUniqueVirtualPixelRatio(true, 1.0f);
    windowSceneSessionImpl->property_->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FULLSCREEN);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    windowSceneSessionImpl->property_->SetDragEnabled(false);
    AreaType dragType = windowSceneSessionImpl->GetDragAreaByDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(dragType, AreaType::UNDEFINED);
}

/**
 * @tc.name: GetDragAreaByDownEvent05
 * @tc.desc: Test corner finger touch drag hot zone
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, GetDragAreaByDownEvent05, TestSize.Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetDragAreaByDownEvent05");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    pointerEvent->SetSourceType(2);
    MMI::PointerEvent::PointerItem pointerItem;

    windowSceneSessionImpl->SetUniqueVirtualPixelRatio(true, 1.0f);
    Rect rect = {100, 100, 1000, 1000};
    windowSceneSessionImpl->property_->SetWindowRect(rect);
    
    pointerItem.SetWindowX(10);
    pointerItem.SetWindowY(30);
    windowSceneSessionImpl->property_->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    AreaType dragType = windowSceneSessionImpl->GetDragAreaByDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(dragType, AreaType::LEFT_TOP);

    pointerItem.SetWindowX(990);
    pointerItem.SetWindowY(30);
    dragType = windowSceneSessionImpl->GetDragAreaByDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(dragType, AreaType::RIGHT_TOP);

    pointerItem.SetWindowX(990);
    pointerItem.SetWindowY(970);
    dragType = windowSceneSessionImpl->GetDragAreaByDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(dragType, AreaType::RIGHT_BOTTOM);

    pointerItem.SetWindowX(10);
    pointerItem.SetWindowY(970);
    dragType = windowSceneSessionImpl->GetDragAreaByDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(dragType, AreaType::LEFT_BOTTOM);

    pointerEvent->SetSourceType(1);
    dragType = windowSceneSessionImpl->GetDragAreaByDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(dragType, AreaType::UNDEFINED);
}

/**
 * @tc.name: UpdateImmersiveBySwitchMode
 * @tc.desc: Test UpdateImmersiveBySwitchMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, UpdateImmersiveBySwitchMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    sptr<SessionMocker> mockHostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    auto property = window->GetProperty();

    property->SetPersistentId(123);

    window->hostSession_ = nullptr;
    window->enableImmersiveMode_ = true;
    window->UpdateImmersiveBySwitchMode(true);
    EXPECT_EQ(window->enableImmersiveMode_, false);

    window->enableImmersiveMode_ = false;
    window->hostSession_ = nullptr;
    window->UpdateImmersiveBySwitchMode(true);
    EXPECT_EQ(window->enableImmersiveMode_, false);

    window->enableImmersiveMode_ = false;
    window->hostSession_ = mockHostSession;
    window->UpdateImmersiveBySwitchMode(true);
    EXPECT_EQ(window->enableImmersiveMode_, false);

    window->cacheEnableImmersiveMode_ = false;
    window->UpdateImmersiveBySwitchMode(false);
    EXPECT_EQ(window->enableImmersiveMode_, false);

    window->cacheEnableImmersiveMode_ = true;
    window->hostSession_ = nullptr;
    window->UpdateImmersiveBySwitchMode(false);
    EXPECT_EQ(window->enableImmersiveMode_, true);

    window->cacheEnableImmersiveMode_ = true;
    window->hostSession_ = mockHostSession;
    window->UpdateImmersiveBySwitchMode(false);
    EXPECT_EQ(window->enableImmersiveMode_, true);
}

/**
 * @tc.name: UpdateImmersiveBySwitchMode2
 * @tc.desc: Test UpdateImmersiveBySwitchMode2
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, UpdateImmersiveBySwitchMode2, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo;
    sptr<SessionMocker> mockHostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    auto property = window->GetProperty();

    property->SetPersistentId(123);

    window->cacheEnableImmersiveMode_ = false;
    window->maximizeLayoutFullScreen_ = true;
    window->isIgnoreSafeArea_ = true;
    window->hostSession_ = mockHostSession;
    window->UpdateImmersiveBySwitchMode(false);
    EXPECT_EQ(window->isIgnoreSafeArea_, false);

    window->cacheEnableImmersiveMode_ = true;
    window->maximizeLayoutFullScreen_ = true;
    window->isIgnoreSafeArea_ = true;
    window->hostSession_ = mockHostSession;
    window->UpdateImmersiveBySwitchMode(false);
    EXPECT_EQ(window->isIgnoreSafeArea_, true);

    window->cacheEnableImmersiveMode_ = true;
    window->maximizeLayoutFullScreen_ = false;
    window->isIgnoreSafeArea_ = false;
    window->hostSession_ = mockHostSession;
    window->UpdateImmersiveBySwitchMode(false);
    EXPECT_EQ(window->isIgnoreSafeArea_, true);
}

/**
 * @tc.name: TestSetContentAspectRatio
 * @tc.desc: Test SetContentAspectRatio under multiple conditions
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, TestSetContentAspectRatio, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto property = window->GetProperty();
    property->SetPersistentId(123);
    SessionInfo sessionInfo;
    sptr<SessionMocker> mockHostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);

    // Case 1: Not main window
    float ratio = 1.5f;
    bool isPersistent = true;
    bool needUpdateRect = true;
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    auto ret = window->SetContentAspectRatio(ratio, isPersistent, needUpdateRect);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);

    // Case 2: Invalid session
    window->hostSession_ = nullptr;
    ret = window->SetContentAspectRatio(ratio, isPersistent, needUpdateRect);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    window->hostSession_ = mockHostSession;

    // Case 3: Invalid ratio => WM_ERROR_ILLEGAL_PARAM
    ratio = MathHelper::INF;
    ret = window->SetContentAspectRatio(ratio, isPersistent, needUpdateRect);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);
    ratio = MathHelper::NAG_INF;
    ret = window->SetContentAspectRatio(ratio, isPersistent, needUpdateRect);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);
    ratio = std::numeric_limits<float>::quiet_NaN();
    ret = window->SetContentAspectRatio(ratio, isPersistent, needUpdateRect);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);
    ratio = 0.0f;
    ret = window->SetContentAspectRatio(ratio, isPersistent, needUpdateRect);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    // Case 4: host returns failure => WM_ERROR_ILLEGAL_PARAM
    ratio = 1.5f;
    EXPECT_CALL(*mockHostSession, SetContentAspectRatio(_, _, _))
        .Times(1).WillOnce(Return(WSError::WS_ERROR_IPC_FAILED));
    ret = window->SetContentAspectRatio(ratio, isPersistent, needUpdateRect);
    EXPECT_EQ(ret, WMError::WM_ERROR_ILLEGAL_PARAM);

    // Case 5: host returns success => WM_OK
    EXPECT_CALL(*mockHostSession, SetContentAspectRatio(_, _, _))
        .Times(1).WillOnce(Return(WSError::WS_OK));
    ret = window->SetContentAspectRatio(ratio, isPersistent, needUpdateRect);
    EXPECT_EQ(ret, WMError::WM_OK);

    // Case 6: is compatibility mode => WM_OK
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    property->SetCompatibleModeProperty(compatibleModeProperty);
    ret = window->SetContentAspectRatio(ratio, isPersistent, needUpdateRect);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: CalculateNewLimitsByLimits
 * @tc.desc: test function: CalculateNewLimitsByLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, CalculateNewLimitsByLimits, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> testImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    WindowLimits customizedLimits = {200, 200, 10, 10, 0.0f, 0.0f, 1.0f, PixelUnit::VP};
    WindowLimits expectLimits;
    WindowLimits newLimits;
    WindowLimits newLimitsVP = WindowLimits::DEFAULT_VP_LIMITS();
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(0);
    ASSERT_NE(nullptr, display);
    auto displayInfo = display->GetDisplayInfo();
    ASSERT_NE(nullptr, displayInfo);
    float virtualPixelRatio = testImpl->GetVirtualPixelRatio(displayInfo);
    testImpl->property_->SetDisplayId(0);
    testImpl->property_->SetConfigWindowLimitsVP(customizedLimits);

    // set system limits
    testImpl->windowSystemConfig_.maxFloatingWindowSize_ = 6240;
    testImpl->windowSystemConfig_.miniWidthOfMainWindow_ = 1;
    testImpl->windowSystemConfig_.miniHeightOfMainWindow_ = 1;
    testImpl->windowSystemConfig_.miniWidthOfSubWindow_ = 1;
    testImpl->windowSystemConfig_.miniHeightOfSubWindow_ = 1;
    testImpl->windowSystemConfig_.miniWidthOfDialogWindow_ = 1;
    testImpl->windowSystemConfig_.miniHeightOfDialogWindow_ = 1;

    // user set flag is false, window type is mian window
    testImpl->userLimitsSet_ = false;
    customizedLimits = {200, 200, 10, 10, 0.0f, 0.0f, 1.0f};
    expectLimits ={200, 200, 10, 10, 0.0f, 0.0f, 1.0f};
    testImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    testImpl->CalculateNewLimitsByLimits(newLimits, newLimitsVP, customizedLimits, virtualPixelRatio);
    EXPECT_EQ(customizedLimits.maxWidth_, expectLimits.maxWidth_);
    EXPECT_EQ(customizedLimits.maxHeight_, expectLimits.maxHeight_);
    EXPECT_EQ(customizedLimits.minWidth_, expectLimits.minWidth_);
    EXPECT_EQ(customizedLimits.minHeight_, expectLimits.minHeight_);

    // user set flag is false, window type is sys window
    testImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    customizedLimits = {200, 200, 10, 10, 0.0f, 0.0f, 1.0f};
    expectLimits ={200, 200, 10, 10, 0.0f, 0.0f, 1.0f};
    testImpl->CalculateNewLimitsByLimits(newLimits, newLimitsVP, customizedLimits, virtualPixelRatio);
    testImpl->CalculateNewLimitsByLimits(newLimits, newLimitsVP, customizedLimits, virtualPixelRatio);
    EXPECT_EQ(customizedLimits.maxWidth_, expectLimits.maxWidth_);
    EXPECT_EQ(customizedLimits.maxHeight_, expectLimits.maxHeight_);
    EXPECT_EQ(customizedLimits.minWidth_, expectLimits.minWidth_);
    EXPECT_EQ(customizedLimits.minHeight_, expectLimits.minHeight_);

    // user set flag is true
    testImpl->userLimitsSet_ = true;
    customizedLimits = {200, 200, 10, 10, 0.0f, 0.0f, 1.0f};
    WindowLimits userLimits = {900, 900, 100, 100, 0.0f, 0.0f, 1.0f};
    testImpl->property_->SetUserWindowLimits(userLimits);
    testImpl->CalculateNewLimitsByLimits(newLimits, newLimitsVP, customizedLimits, virtualPixelRatio);
    EXPECT_EQ(customizedLimits.maxWidth_, userLimits.maxWidth_);
    EXPECT_EQ(customizedLimits.maxHeight_, userLimits.maxHeight_);
    EXPECT_EQ(customizedLimits.minWidth_, userLimits.minWidth_);
    EXPECT_EQ(customizedLimits.minHeight_, userLimits.minHeight_);
}

/**
 * @tc.name: SetDefaultDensityEnabled_forCompatMode
 * @tc.desc: SetDefaultDensityEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetDefaultDensityEnabled_forCompatMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDefaultDensityEnabled_forCompatMode");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->defaultDensityEnabledStageConfig_.store(false);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    EXPECT_EQ(WMError::WM_OK, window->SetDefaultDensityEnabled(true));
    EXPECT_TRUE(window->IsStageDefaultDensityEnabled());
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_EQ(window->IsAdaptToSimulationScale(), true);
    window->defaultDensityEnabledStageConfig_.store(false);
    EXPECT_EQ(WMError::WM_OK, window->SetDefaultDensityEnabled(true));
    EXPECT_FALSE(window->IsStageDefaultDensityEnabled());
}

/**
 * @tc.name: ShouldSkipSupportWindowModeCheck01
 * @tc.desc: ShouldSkipSupportWindowModeCheck01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, ShouldSkipSupportWindowModeCheck01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ShouldSkipSupportWindowModeCheck");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    uint32_t windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY;
    bool result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: ShouldSkipSupportWindowModeCheck02
 * @tc.desc: ShouldSkipSupportWindowModeCheck02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, ShouldSkipSupportWindowModeCheck02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ShouldSkipSupportWindowModeCheck");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    uint32_t windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY;
    bool result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(result, true);

    windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY;
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(result, true);

    windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING;
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: ShouldSkipSupportWindowModeCheck03
 * @tc.desc: ShouldSkipSupportWindowModeCheck03
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, ShouldSkipSupportWindowModeCheck03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ShouldSkipSupportWindowModeCheck");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    uint32_t windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY;
    bool result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(result, true);

    windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY;
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(result, true);

    windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING;
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: ShouldSkipSupportWindowModeCheck04
 * @tc.desc: ShouldSkipSupportWindowModeCheck04
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, ShouldSkipSupportWindowModeCheck04, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ShouldSkipSupportWindowModeCheck");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->isAdaptToDragScale_ = false;
    window->property_->compatibleModeProperty_ = property;
    uint32_t windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN;
    bool result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(result, false);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(result, false);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(result, false);

    windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING;
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(result, false);

    property->isAdaptToDragScale_ = true;
    window->property_->compatibleModeProperty_ = property;
    windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN;
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(result, true);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(result, false);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(result, false);

    windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING;
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: ShouldSkipSupportWindowModeCheck05
 * @tc.desc: ShouldSkipSupportWindowModeCheck05
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, ShouldSkipSupportWindowModeCheck05, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ShouldSkipSupportWindowModeCheck");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->isAdaptToDragScale_ = false;
    window->property_->compatibleModeProperty_ = property;
    uint32_t windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN;

    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    bool result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(result, true);

    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(result, false);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(result, false);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(result, false);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: ShouldSkipSupportWindowModeCheck06
 * @tc.desc: ShouldSkipSupportWindowModeCheck06
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, ShouldSkipSupportWindowModeCheck06, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ShouldSkipSupportWindowModeCheck");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sptr<CompatibleModeProperty> property = sptr<CompatibleModeProperty>::MakeSptr();
    property->isAdaptToDragScale_ = false;
    window->property_->compatibleModeProperty_ = property;
    uint32_t windowModeSupportType = WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN;

    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    bool result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(result, false);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(result, false);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(result, false);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(result, false);

    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(result, false);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(result, false);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(result, false);
    result = window->ShouldSkipSupportWindowModeCheck(windowModeSupportType, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SetForceSplitConfigEnable01
 * @tc.desc: Test SetForceSplitConfigEnable when window type is not main window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetForceSplitConfigEnable01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetForceSplitConfigEnable01");
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    
    // Should return early when window type is not main window
    window->SetForceSplitConfigEnable(true);
    // No crash means the function returned early as expected
}

/**
 * @tc.name: SetForceSplitConfigEnable02
 * @tc.desc: Test SetForceSplitConfigEnable when uiContent is null
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, SetForceSplitConfigEnable02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetForceSplitConfigEnable02");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    
    // uiContent should be null by default
    window->SetForceSplitConfigEnable(true);
    // No crash means the function handled null uiContent correctly
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigEnableUpdated01
 * @tc.desc: Test NotifyAppForceLandscapeConfigEnableUpdated when window type is not main window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, NotifyAppForceLandscapeConfigEnableUpdated01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyAppForceLandscapeConfigEnableUpdated01");
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    
    WSError res = window->NotifyAppForceLandscapeConfigEnableUpdated();
    EXPECT_EQ(res, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigEnableUpdated02
 * @tc.desc: Test NotifyAppForceLandscapeConfigEnableUpdated when GetAppForceLandscapeConfigEnable fails
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest5, NotifyAppForceLandscapeConfigEnableUpdated02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyAppForceLandscapeConfigEnableUpdated02");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(window, nullptr);
    
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    
    // GetAppForceLandscapeConfigEnable will fail if listener is not registered
    WSError res = window->NotifyAppForceLandscapeConfigEnableUpdated();
    EXPECT_EQ(res, WSError::WS_DO_NOTHING);
}
}
} // namespace Rosen
} // namespace OHOS