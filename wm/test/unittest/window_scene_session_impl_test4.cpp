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
#include "display_info.h"
#include "mock_ability_context_impl.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window_adapter.h"
#include "parameters.h"
#include "singleton_mocker.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
const std::string WATERFALL_WINDOW_EVENT = "scb_waterfall_window_event";
using Mocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
class WindowSceneSessionImplTest4 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
    std::unique_ptr<Mocker> mocker_ = std::make_unique<Mocker>();

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
    static constexpr uint32_t WAIT_SERVERAL_FRAMES = 36000;
};

void WindowSceneSessionImplTest4::SetUpTestCase() {}

void WindowSceneSessionImplTest4::TearDownTestCase() {}

void WindowSceneSessionImplTest4::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSceneSessionImplTest4::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

RSSurfaceNode::SharedPtr WindowSceneSessionImplTest4::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "startingWindowTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    return surfaceNode;
}

namespace {
/**
 * @tc.name: UpdateSurfaceNodeAfterCustomAnimation
 * @tc.desc: UpdateSurfaceNodeAfterCustomAnimation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, UpdateSurfaceNodeAfterCustomAnimation, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSurfaceNodeAfterCustomAnimation");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->hostSession_ = nullptr;
    auto ret = windowSceneSessionImpl->UpdateSurfaceNodeAfterCustomAnimation(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_END);
    ret = windowSceneSessionImpl->UpdateSurfaceNodeAfterCustomAnimation(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE);
    ret = windowSceneSessionImpl->UpdateSurfaceNodeAfterCustomAnimation(true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: ConsumePointerEvent
 * @tc.desc: ConsumePointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, ConsumePointerEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ConsumePointerEvent");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    windowSceneSessionImpl->ConsumePointerEvent(pointerEvent);
    windowSceneSessionImpl->hostSession_ = nullptr;
    windowSceneSessionImpl->ConsumePointerEvent(pointerEvent);
    pointerEvent = nullptr;
    windowSceneSessionImpl->ConsumePointerEvent(pointerEvent);
    windowSceneSessionImpl->property_->SetTurnScreenOn(true);
    auto ret = windowSceneSessionImpl->IsTurnScreenOn();
    EXPECT_EQ(true, ret);
    windowSceneSessionImpl->property_->SetTurnScreenOn(false);
    ret = windowSceneSessionImpl->IsTurnScreenOn();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: ConsumePointerEvent02
 * @tc.desc: ConsumePointerEvent02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, ConsumePointerEvent02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ConsumePointerEvent");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->uiContent_ = std::make_unique<Ace::UIContentMocker>();

    int32_t pointerId = 0;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetPointerId(pointerId);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    pointerEvent->SetSourceType(1);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(pointerId);
    pointerItem.SetOriginPointerId(pointerId);
    pointerEvent->AddPointerItem(pointerItem);

    windowSceneSessionImpl->property_->SetWindowDelayRaiseEnabled(false);
    EXPECT_EQ(false, windowSceneSessionImpl->property_->IsWindowDelayRaiseEnabled());
    windowSceneSessionImpl->ConsumePointerEvent(pointerEvent);

    windowSceneSessionImpl->SetUniqueVirtualPixelRatio(true, 1.0f);
    Rect rect = {1, 1, 1, 1};
    windowSceneSessionImpl->property_->SetWindowRect(rect);
    pointerItem.SetWindowX(1);
    pointerItem.SetWindowY(1);
    windowSceneSessionImpl->property_->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    windowSceneSessionImpl->property_->SetWindowDelayRaiseEnabled(true);
    EXPECT_EQ(true, windowSceneSessionImpl->property_->IsWindowDelayRaiseEnabled());
    windowSceneSessionImpl->ConsumePointerEvent(pointerEvent);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    windowSceneSessionImpl->ConsumePointerEvent(pointerEvent);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);
    windowSceneSessionImpl->ConsumePointerEvent(pointerEvent);
    AAFwk::Want want;
    windowSceneSessionImpl->uiContent_ = nullptr;
    windowSceneSessionImpl->OnNewWant(want);
    EXPECT_EQ(windowSceneSessionImpl->GetUIContentSharedPtr(), nullptr);
    windowSceneSessionImpl->ConsumePointerEvent(pointerEvent);
}

/**
 * @tc.name: GetSystemSizeLimits01
 * @tc.desc: GetSystemSizeLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetSystemSizeLimits01, TestSize.Level1)
{
    uint32_t minMainWidth = 10;
    uint32_t minMainHeight = 20;
    uint32_t minSubWidth = 30;
    uint32_t minSubHeight = 40;
    uint32_t minDialogWidth = 37;
    uint32_t minDialogHeight = 43;
    uint32_t displayWidth = 100;
    uint32_t displayHeight = 100;
    float displayVpr = 1.0f;
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSystemSizeLimits01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->windowSystemConfig_.miniWidthOfMainWindow_ = minMainWidth;
    windowSceneSessionImpl->windowSystemConfig_.miniHeightOfMainWindow_ = minMainHeight;
    windowSceneSessionImpl->windowSystemConfig_.miniWidthOfSubWindow_ = minSubWidth;
    windowSceneSessionImpl->windowSystemConfig_.miniHeightOfSubWindow_ = minSubHeight;
    windowSceneSessionImpl->windowSystemConfig_.miniWidthOfDialogWindow_ = minDialogWidth;
    windowSceneSessionImpl->windowSystemConfig_.miniHeightOfDialogWindow_ = minDialogHeight;

    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    const auto& [limits, _] = windowSceneSessionImpl->GetSystemSizeLimits(displayWidth, displayHeight, displayVpr);
    EXPECT_EQ(limits.minWidth_, minMainWidth);
    EXPECT_EQ(limits.minHeight_, minMainHeight);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    const auto& [limits2, _2] = windowSceneSessionImpl->GetSystemSizeLimits(displayWidth, displayHeight, displayVpr);
    EXPECT_EQ(limits2.minWidth_, minSubWidth);
    EXPECT_EQ(limits2.minHeight_, minSubHeight);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    const auto& [limits3, _3] = windowSceneSessionImpl->GetSystemSizeLimits(displayWidth, displayHeight, displayVpr);
    EXPECT_EQ(limits3.minWidth_, minDialogWidth);
    EXPECT_EQ(limits3.minHeight_, minDialogHeight);
}

/**
 * @tc.name: HandlePointDownEvent
 * @tc.desc: HandlePointDownEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, HandlePointDownEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HandlePointDownEvent");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    pointerEvent->SetSourceType(1);
    MMI::PointerEvent::PointerItem pointerItem;
    float vpr = 1.0f;

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->SetUniqueVirtualPixelRatio(true, vpr);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE);

    windowSceneSessionImpl->property_->SetDragEnabled(false);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    auto ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(true, ret);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetDragEnabled(false);
    ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(true, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);

    ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(true, ret);

    pointerItem.SetWindowX(100);
    pointerItem.SetWindowY(100);
    Rect rect = {1, 1, 1, 1};
    windowSceneSessionImpl->property_->SetWindowRect(rect);
    ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(true, ret);
    pointerItem.SetWindowX(1);
    ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(true, ret);

    windowSceneSessionImpl->property_->SetDragEnabled(true);
    ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(false, ret);

    pointerItem.SetWindowX(100);
    ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(true, ret);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    pointerItem.SetDisplayX(100);
    pointerItem.SetDisplayY(100);
    ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: UpdateWindowModeImmediately
 * @tc.desc: UpdateWindowModeImmediately
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, UpdateWindowModeImmediately, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateWindowModeImmediately");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->state_ = WindowState::STATE_CREATED;
    auto ret = windowSceneSessionImpl->UpdateWindowModeImmediately(WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, windowSceneSessionImpl->property_->GetWindowMode());

    windowSceneSessionImpl->state_ = WindowState::STATE_HIDDEN;
    ret = windowSceneSessionImpl->UpdateWindowModeImmediately(WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WindowMode::WINDOW_MODE_FLOATING, windowSceneSessionImpl->property_->GetWindowMode());

    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    ret = windowSceneSessionImpl->UpdateWindowModeImmediately(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WindowMode::WINDOW_MODE_SPLIT_PRIMARY, windowSceneSessionImpl->property_->GetWindowMode());

    ret = windowSceneSessionImpl->UpdateWindowModeImmediately(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WindowMode::WINDOW_MODE_SPLIT_SECONDARY, windowSceneSessionImpl->property_->GetWindowMode());

    windowSceneSessionImpl->state_ = WindowState::STATE_UNFROZEN;
    ret = windowSceneSessionImpl->UpdateWindowModeImmediately(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(WindowMode::WINDOW_MODE_SPLIT_SECONDARY, windowSceneSessionImpl->property_->GetWindowMode());
}

/**
 * @tc.name: AdjustWindowAnimationFlag
 * @tc.desc: AdjustWindowAnimationFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, AdjustWindowAnimationFlag, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AdjustWindowAnimationFlag");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;

    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    windowSceneSessionImpl->animationTransitionControllers_.push_back(sptr<IAnimationTransitionController>::MakeSptr());
    windowSceneSessionImpl->AdjustWindowAnimationFlag(true);
    auto ret = windowSceneSessionImpl->property_->GetAnimationFlag();
    EXPECT_EQ(3, ret);
    windowSceneSessionImpl->animationTransitionControllers_.clear();
    windowSceneSessionImpl->AdjustWindowAnimationFlag(true);
    ret = windowSceneSessionImpl->property_->GetAnimationFlag();
    EXPECT_EQ(1, ret);
    windowSceneSessionImpl->AdjustWindowAnimationFlag(false);
    ret = windowSceneSessionImpl->property_->GetAnimationFlag();
    EXPECT_EQ(0, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->AdjustWindowAnimationFlag(false);
    ret = windowSceneSessionImpl->property_->GetAnimationFlag();
    EXPECT_EQ(1, ret);
    windowSceneSessionImpl->enableDefaultAnimation_ = false;
    windowSceneSessionImpl->AdjustWindowAnimationFlag(false);
    ret = windowSceneSessionImpl->property_->GetAnimationFlag();
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name: UpdateConfigurationForAll
 * @tc.desc: UpdateConfigurationForAll
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, UpdateConfigurationForAll, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateConfigurationForAll");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();
    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle1", "CreateTestModule1", "CreateTestAbility1" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSession->hostSession_ = session;
    windowSession->property_->SetPersistentId(1);
    windowSession->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK, windowSession->Create(abilityContext_, session));
    windowSceneSessionImpl->UpdateConfigurationForAll(configuration);
    ASSERT_EQ(WMError::WM_OK, windowSession->Destroy(true));
}

/**
 * @tc.name: SetWindowFlags
 * @tc.desc: SetWindowFlags
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetWindowFlags, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowFlags");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->SetWindowFlags(1);
    auto ret = windowSceneSessionImpl->SetWindowFlags(1);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = windowSceneSessionImpl->SetWindowFlags(2);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    ret = windowSceneSessionImpl->NotifyPrepareClosePiPWindow();
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: ResetSuperFoldDisplayY
 * @tc.desc: ResetSuperFoldDisplayY
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, ResetSuperFoldDisplayY, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ResetSuperFoldDisplayY");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    windowSceneSessionImpl->superFoldOffsetY_ = 150;
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetDisplayY(100);
    pointerEvent->AddPointerItem(pointerItem);
    int32_t originalDisplayY = pointerItem.GetDisplayY();
    windowSceneSessionImpl->ResetSuperFoldDisplayY(pointerEvent);
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    int32_t updatedDisplayY = pointerItem.GetDisplayY();

    pointerItem.SetDisplayY(150);
    pointerEvent->AddPointerItem(pointerItem);
    originalDisplayY = pointerItem.GetDisplayY();
    windowSceneSessionImpl->ResetSuperFoldDisplayY(pointerEvent);
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    updatedDisplayY = pointerItem.GetDisplayY();
    ASSERT_EQ(updatedDisplayY, originalDisplayY - 150);
}

/**
 * @tc.name: ResetSuperFoldDisplayY01
 * @tc.desc: ResetSuperFoldDisplayY01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, ResetSuperFoldDisplayY01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ResetSuperFoldDisplayY01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    windowSceneSessionImpl->superFoldOffsetY_ = 150;
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetDisplayYPos(100);
    pointerEvent->AddPointerItem(pointerItem);
    auto originalDisplayY = pointerItem.GetDisplayYPos();
    windowSceneSessionImpl->ResetSuperFoldDisplayY(pointerEvent);
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    auto updatedDisplayY = pointerItem.GetDisplayYPos();

    pointerItem.SetDisplayYPos(150);
    pointerEvent->AddPointerItem(pointerItem);
    originalDisplayY = pointerItem.GetDisplayYPos();
    windowSceneSessionImpl->ResetSuperFoldDisplayY(pointerEvent);
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    updatedDisplayY = pointerItem.GetDisplayYPos();
    ASSERT_EQ(updatedDisplayY, originalDisplayY - 150);

    pointerItem.SetDisplayYPos(151.001);
    pointerEvent->AddPointerItem(pointerItem);
    originalDisplayY = pointerItem.GetDisplayYPos();
    windowSceneSessionImpl->ResetSuperFoldDisplayY(pointerEvent);
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    updatedDisplayY = pointerItem.GetDisplayYPos();
    ASSERT_EQ(updatedDisplayY, originalDisplayY - 150);
}

/**
 * @tc.name: ConsumePointerEventInner
 * @tc.desc: ConsumePointerEventInner
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, ConsumePointerEventInner, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ConsumePointerEventInner");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->SetPointerAction(10);
    windowSceneSessionImpl->ConsumePointerEventInner(pointerEvent, pointerItem);
    pointerEvent->SetPointerAction(4);
    windowSceneSessionImpl->ConsumePointerEventInner(pointerEvent, pointerItem);
    pointerEvent->SetPointerAction(2);
    windowSceneSessionImpl->ConsumePointerEventInner(pointerEvent, pointerItem);
}

/**
 * @tc.name: ConsumePointerEventInner02
 * @tc.desc: ConsumePointerEventInner02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, ConsumePointerEventInner02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ConsumePointerEventInner");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->property_->SetWindowDelayRaiseEnabled(true);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    windowSceneSessionImpl->ConsumePointerEventInner(pointerEvent, pointerItem, true);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);
    windowSceneSessionImpl->ConsumePointerEventInner(pointerEvent, pointerItem, true);
}

/**
 * @tc.name: HandleEventForCompatibleMode
 * @tc.desc: HandleEventForCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, HandleEventForCompatibleMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HandleEventForCompatibleMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->SetPointerAction(2);
    windowSceneSessionImpl->HandleEventForCompatibleMode(pointerEvent, pointerItem);
    pointerEvent->SetPointerAction(3);
    windowSceneSessionImpl->HandleEventForCompatibleMode(pointerEvent, pointerItem);
    pointerEvent->SetPointerAction(4);
    windowSceneSessionImpl->HandleEventForCompatibleMode(pointerEvent, pointerItem);
}

/**
 * @tc.name: HandleDownForCompatibleMode
 * @tc.desc: HandleDownForCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, HandleDownForCompatibleMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HandleDownForCompatibleMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->SetPointerAction(2);
    windowSceneSessionImpl->HandleDownForCompatibleMode(pointerEvent, pointerItem);
}

/**
 * @tc.name: HandleMoveForCompatibleMode
 * @tc.desc: HandleMoveForCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, HandleMoveForCompatibleMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HandleMoveForCompatibleMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->SetPointerAction(3);
    windowSceneSessionImpl->HandleMoveForCompatibleMode(pointerEvent, pointerItem);
}

/**
 * @tc.name: HandleUpForCompatibleMode
 * @tc.desc: HandleUpForCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, HandleUpForCompatibleMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HandleUpForCompatibleMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->SetPointerAction(4);
    windowSceneSessionImpl->HandleUpForCompatibleMode(pointerEvent, pointerItem);
}

/**
 * @tc.name: ConvertPointForCompatibleMode
 * @tc.desc: ConvertPointForCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, ConvertPointForCompatibleMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ConvertPointForCompatibleMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    int32_t transferX = 800;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->SetPointerAction(2);
    windowSceneSessionImpl->ConvertPointForCompatibleMode(pointerEvent, pointerItem, transferX);
    pointerEvent->SetPointerAction(3);
    windowSceneSessionImpl->ConvertPointForCompatibleMode(pointerEvent, pointerItem, transferX);
    pointerEvent->SetPointerAction(4);
    windowSceneSessionImpl->ConvertPointForCompatibleMode(pointerEvent, pointerItem, transferX);
}

/**
 * @tc.name: IsInMappingRegionForCompatibleMode
 * @tc.desc: IsInMappingRegionForCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, IsInMappingRegionForCompatibleMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsInMappingRegionForCompatibleMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->property_->SetWindowRect({ 880, 0, 800, 1600 });
    int32_t displayX = 400;
    int32_t displayY = 400;
    EXPECT_CALL(*session, GetGlobalScaledRect(_)).Times(1).WillOnce(Return(WMError::WM_ERROR_IPC_FAILED));
    bool ret = windowSceneSessionImpl->IsInMappingRegionForCompatibleMode(displayX, displayY);
    EXPECT_EQ(true, ret);
    displayX = 1000;
    displayY = 1000;
    EXPECT_CALL(*session, GetGlobalScaledRect(_)).Times(1).WillOnce(Return(WMError::WM_ERROR_IPC_FAILED));
    ret = windowSceneSessionImpl->IsInMappingRegionForCompatibleMode(displayX, displayY);
    EXPECT_EQ(false, ret);
    EXPECT_CALL(*session, GetGlobalScaledRect(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    ret = windowSceneSessionImpl->IsInMappingRegionForCompatibleMode(displayX, displayY);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: CheckTouchSlop
 * @tc.desc: CheckTouchSlop
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, CheckTouchSlop, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CheckTouchSlop");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    int32_t pointerId = 0;
    int32_t displayX = 400;
    int32_t displayY = 400;
    int32_t threshold = 50;
    bool ret = windowSceneSessionImpl->CheckTouchSlop(pointerId, displayX, displayY, threshold);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: IgnoreClickEvent
 * @tc.desc: IgnoreClickEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, IgnoreClickEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IgnoreClickEvent");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    pointerEvent->SetPointerAction(3);
    windowSceneSessionImpl->IgnoreClickEvent(pointerEvent);
    pointerEvent->SetPointerAction(4);
    windowSceneSessionImpl->IgnoreClickEvent(pointerEvent);
}

/**
 * @tc.name: RegisterKeyboardPanelInfoChangeListener
 * @tc.desc: RegisterKeyboardPanelInfoChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, RegisterKeyboardPanelInfoChangeListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterKeyboardPanelInfoChangeListener");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->keyboardPanelInfoChangeListeners_ = sptr<IKeyboardPanelInfoChangeListener>::MakeSptr();
    sptr<IKeyboardPanelInfoChangeListener> listener = nullptr;
    auto ret = windowSceneSessionImpl->RegisterKeyboardPanelInfoChangeListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
}

/**
 * @tc.name: RegisterWindowAttachStateChangeListener
 * @tc.desc: RegisterWindowAttachStateChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, RegisterWindowAttachStateChangeListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterKeyboardPanelInfoChangeListener");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->windowAttachStateChangeListener_ = sptr<IWindowAttachStateChangeListner>::MakeSptr();

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->hostSession_ = session;

    // case1: listener is nullptr
    sptr<IWindowAttachStateChangeListner> listener = nullptr;
    auto ret = windowSceneSessionImpl->RegisterWindowAttachStateChangeListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // case2: listener is not nullptr
    listener = sptr<IWindowAttachStateChangeListner>::MakeSptr();
    ret = windowSceneSessionImpl->RegisterWindowAttachStateChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // case3: host session is nullptr
    windowSceneSessionImpl->hostSession_ = nullptr;
    ret = windowSceneSessionImpl->RegisterWindowAttachStateChangeListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: UnregisterWindowAttachStateChangeListener
 * @tc.desc: UnregisterWindowAttachStateChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, UnregisterWindowAttachStateChangeListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UnregisterWindowAttachStateChangeListener");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->windowAttachStateChangeListener_ = sptr<IWindowAttachStateChangeListner>::MakeSptr();

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->hostSession_ = session;

    // case1: register listener
    sptr<IWindowAttachStateChangeListner> listener = sptr<IWindowAttachStateChangeListner>::MakeSptr();
    auto ret = windowSceneSessionImpl->RegisterWindowAttachStateChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // case2: unregister success
    ret = windowSceneSessionImpl->UnregisterWindowAttachStateChangeListener();
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(windowSceneSessionImpl->windowAttachStateChangeListener_, nullptr);

    // case3: host session is nullptr
    windowSceneSessionImpl->hostSession_ = nullptr;
    ret = windowSceneSessionImpl->UnregisterWindowAttachStateChangeListener();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: NotifyWindowAttachStateChange
 * @tc.desc: NotifyWindowAttachStateChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, NotifyWindowAttachStateChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyWindowAttachStateChange");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->windowAttachStateChangeListener_ = nullptr;

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->hostSession_ = session;

    // case1: listener is nullptr
    auto ret = windowSceneSessionImpl->NotifyWindowAttachStateChange(true);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);
    ret = windowSceneSessionImpl->NotifyWindowAttachStateChange(false);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);

    // case2: listener is not nullptr, then attach is true
    sptr<IWindowAttachStateChangeListner> listener = sptr<IWindowAttachStateChangeListner>::MakeSptr();
    auto ret2 = windowSceneSessionImpl->RegisterWindowAttachStateChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret2);
    ret = windowSceneSessionImpl->NotifyWindowAttachStateChange(true);
    EXPECT_EQ(WSError::WS_OK, ret);
    ret = windowSceneSessionImpl->NotifyWindowAttachStateChange(true);
    EXPECT_EQ(WSError::WS_OK, ret);

    // case3: listener is not nullptr, then attach is false
    ret = windowSceneSessionImpl->NotifyWindowAttachStateChange(false);
    EXPECT_EQ(WSError::WS_OK, ret);
    ret = windowSceneSessionImpl->NotifyWindowAttachStateChange(false);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: GetSystemBarPropertyByType
 * @tc.desc: GetSystemBarPropertyByType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetSystemBarPropertyByType, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSystemBarPropertyByType");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SystemBarProperty prop;
    prop.settingFlag_ = SystemBarSettingFlag::COLOR_SETTING;
    windowSceneSessionImpl->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, windowSceneSessionImpl->SetSpecificBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, prop));
    auto prop2 = windowSceneSessionImpl->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    ASSERT_EQ(prop2, prop);
}

/**
 * @tc.name: SetSpecificBarProperty
 * @tc.desc: SetSpecificBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetSpecificBarProperty, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSpecificBarProperty");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    SystemBarProperty property;
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->state_ = WindowState::STATE_INITIAL;
    auto type = WindowType::WINDOW_TYPE_STATUS_BAR;
    auto ret = windowSceneSessionImpl->SetSpecificBarProperty(type, property);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    windowSceneSessionImpl->state_ = WindowState::STATE_BOTTOM;
    ret = windowSceneSessionImpl->SetSpecificBarProperty(type, property);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    windowSceneSessionImpl->state_ = WindowState::STATE_CREATED;
    ret = windowSceneSessionImpl->SetSpecificBarProperty(type, property);
    EXPECT_EQ(WMError::WM_OK, ret);
    property = SystemBarProperty();
    property.settingFlag_ = SystemBarSettingFlag::DEFAULT_SETTING;
    ret = windowSceneSessionImpl->SetSpecificBarProperty(type, property);
    EXPECT_EQ(WMError::WM_OK, ret);
    property.settingFlag_ = SystemBarSettingFlag::COLOR_SETTING;
    ret = windowSceneSessionImpl->SetSpecificBarProperty(type, property);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetSystemBarPropertyForPage
 * @tc.desc: GetSystemBarPropertyForPage
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetSystemBarPropertyForPage, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSystemBarPropertyForPage");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPersistentId(1);
    windowSceneSessionImpl->property_ = property;
    std::map<WindowType, SystemBarProperty> properties;
    std::map<WindowType, SystemBarProperty> pageProperties;
    windowSceneSessionImpl->GetSystemBarPropertyForPage(properties, pageProperties);
    EXPECT_EQ(pageProperties[WindowType::WINDOW_TYPE_STATUS_BAR],
        windowSceneSessionImpl->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR));
    EXPECT_EQ(pageProperties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR],
        windowSceneSessionImpl->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR));
}

/**
 * @tc.name: NotifyDialogStateChange
 * @tc.desc: NotifyDialogStateChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, NotifyDialogStateChange, TestSize.Level1)
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
    ret = windowSceneSessionImpl->NotifyDialogStateChange(false);
    EXPECT_EQ(WSError::WS_OK, ret);
    ret = windowSceneSessionImpl->NotifyDialogStateChange(false);
    EXPECT_EQ(WSError::WS_OK, ret);
    ret = windowSceneSessionImpl->NotifyDialogStateChange(true);
    EXPECT_EQ(WSError::WS_OK, ret);
    windowSceneSessionImpl->property_->SetPersistentId(0);
    windowSceneSessionImpl->hostSession_ = nullptr;
    windowSceneSessionImpl->state_ = WindowState::STATE_DESTROYED;
    ret = windowSceneSessionImpl->NotifyDialogStateChange(true);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: GetWindowStatus01
 * @tc.desc: GetWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetWindowStatus01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowStatus01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    WindowStatus windowStatus;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->GetWindowStatus(windowStatus));
    ASSERT_EQ(WindowStatus::WINDOW_STATUS_UNDEFINED, windowStatus);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    ASSERT_EQ(WMError::WM_OK, window->GetWindowStatus(windowStatus));
    ASSERT_EQ(WindowStatus::WINDOW_STATUS_UNDEFINED, windowStatus);
}

/**
 * @tc.name: GetWindowStatus02
 * @tc.desc: GetWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetWindowStatus02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowStatus02");
    option->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowStatus windowStatus;
    ASSERT_EQ(WMError::WM_OK, window->GetWindowStatus(windowStatus));
    ASSERT_EQ(WindowStatus::WINDOW_STATUS_SPLITSCREEN, windowStatus);
}

/**
 * @tc.name: GetWindowStatus03
 * @tc.desc: GetWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetWindowStatus03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowStatus03");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetDisplayId(0);
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowStatus windowStatus;
    ASSERT_EQ(WMError::WM_OK, window->GetWindowStatus(windowStatus));
    ASSERT_EQ(WindowStatus::WINDOW_STATUS_FLOATING, windowStatus);
    window->property_->SetMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
    ASSERT_EQ(WMError::WM_OK, window->GetWindowStatus(windowStatus));
    ASSERT_EQ(WindowStatus::WINDOW_STATUS_MAXIMIZE, windowStatus);
    window->state_ = WindowState::STATE_HIDDEN;
    ASSERT_EQ(WMError::WM_OK, window->GetWindowStatus(windowStatus));
    ASSERT_EQ(WindowStatus::WINDOW_STATUS_MINIMIZE, windowStatus);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    window->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK, window->GetWindowStatus(windowStatus));
    ASSERT_EQ(WindowStatus::WINDOW_STATUS_FULLSCREEN, windowStatus);
}

/**
 * @tc.name: SetWindowTitle
 * @tc.desc: SetWindowTitle Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetWindowTitle, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowTitle");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetDisplayId(0);
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::string title = "SetWindowTitle";
    EXPECT_EQ(window->SetWindowTitle(title), WMError::WM_ERROR_INVALID_WINDOW);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->windowSystemConfig_.freeMultiWindowSupport_ = false;
    window->windowSystemConfig_.isSystemDecorEnable_ = false;
    EXPECT_EQ(window->SetWindowTitle(title), WMError::WM_ERROR_INVALID_WINDOW);
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    window->property_->SetDecorEnable(true);
    EXPECT_EQ(window->SetWindowTitle(title), WMError::WM_ERROR_NULLPTR);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_EQ(window->SetWindowTitle(title), WMError::WM_OK);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    EXPECT_EQ(window->SetWindowTitle(title), WMError::WM_ERROR_INVALID_WINDOW);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    EXPECT_EQ(window->SetWindowTitle(title), WMError::WM_ERROR_NULLPTR);
    EXPECT_EQ(window->Create(abilityContext_, session), WMError::WM_OK);
    EXPECT_EQ(window->SetWindowTitle(title), WMError::WM_OK);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    window->property_->SetDecorEnable(true);
    EXPECT_EQ(window->SetWindowTitle(title), WMError::WM_OK);
 
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetPcAppInpadCompatibleMode(true);
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.isSystemDecorEnable_ = false;
    EXPECT_EQ(WMError::WM_OK, window->SetWindowTitle(title));
    const std::string feature = "large_screen";
    std::string deviceType = OHOS::system::GetParameter("const.product.devicetype", "");
    auto context = std::make_shared<MockAbilityContextImpl>();
    window->context_ = context;
    context->hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();
    context->hapModuleInfo_->requiredDeviceFeatures = {{deviceType, {feature}}};
    EXPECT_EQ(WMError::WM_OK, window->SetWindowTitle(title));
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: AddSubWindowMapForExtensionWindow
 * @tc.desc: AddSubWindowMapForExtensionWindow Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, AddSubWindowMapForExtensionWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddSubWindowMapForExtensionWindow");
    option->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WindowSceneSessionImpl::subWindowSessionMap_.clear();
    window->AddSubWindowMapForExtensionWindow();
    EXPECT_EQ(WindowSceneSessionImpl::subWindowSessionMap_.size(), 0);
    window->context_ = abilityContext_;

    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(option);
    windowSession->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    windowSession->context_ = abilityContext_;
    windowSession->property_->SetPersistentId(2);
    WindowSessionImpl::GetWindowExtensionSessionSet().insert(windowSession);
    window->AddSubWindowMapForExtensionWindow();
    EXPECT_EQ(WindowSceneSessionImpl::subWindowSessionMap_.size(), 1);
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: IsPcOrFreeMultiWindowCapabilityEnabled
 * @tc.desc: IsPcOrFreeMultiWindowCapabilityEnabled Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, IsPcOrFreeMultiWindowCapabilityEnabled, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsPcOrFreeMultiWindowCapabilityEnabled");
    option->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(true, window->IsPcOrFreeMultiWindowCapabilityEnabled());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(false, window->IsPcOrFreeMultiWindowCapabilityEnabled());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    EXPECT_EQ(false, window->IsPcOrFreeMultiWindowCapabilityEnabled());
    window->property_->SetIsUIExtFirstSubWindow(true);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    EXPECT_EQ(true, window->IsPcOrFreeMultiWindowCapabilityEnabled());
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: GetParentSessionAndVerify
 * @tc.desc: GetParentSessionAndVerify Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetParentSessionAndVerify, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetParentSessionAndVerify");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sptr<WindowSessionImpl> parentSession = nullptr;
    auto res = window->GetParentSessionAndVerify(false, parentSession);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, res);
    res = window->GetParentSessionAndVerify(true, parentSession);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, res);

    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("GetParentSessionAndVerify2");
    subOption->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    SessionInfo subSessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentId(1);
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    std::vector<sptr<WindowSessionImpl>> vec;
    WindowSceneSessionImpl::subWindowSessionMap_.insert(
        std::pair<int32_t, std::vector<sptr<WindowSessionImpl>>>(1, vec));
    WindowSceneSessionImpl::subWindowSessionMap_[1].push_back(subWindow);
    res = subWindow->GetParentSessionAndVerify(false, parentSession);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, res);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(
        window->GetWindowName(), std::pair<uint64_t, sptr<WindowSessionImpl>>(window->GetWindowId(), window)));
    res = subWindow->GetParentSessionAndVerify(false, parentSession);
    EXPECT_EQ(WMError::WM_OK, res);
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: GetParentSessionAndVerify01
 * @tc.desc: GetParentSessionAndVerify Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetParentSessionAndVerify01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetParentSessionAndVerify01");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WindowSceneSessionImpl::windowSessionMap_.clear();
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(window->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(window->GetWindowId(), window)));

    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("GetParentSessionAndVerify01_subWindow");
    subOption->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    SessionInfo subSessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentId(1);
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    subWindow->property_->SetIsUIExtFirstSubWindow(true);
    sptr<WindowSessionImpl> parentSession = nullptr;
    auto res = subWindow->GetParentSessionAndVerify(false, parentSession);
    EXPECT_EQ(WMError::WM_OK, res);
    subWindow->property_->SetIsUIExtFirstSubWindow(false);
    res = subWindow->GetParentSessionAndVerify(false, parentSession);
    EXPECT_EQ(WMError::WM_OK, res);
    window->property_->SetSubWindowLevel(2);
    res = subWindow->GetParentSessionAndVerify(false, parentSession);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, res);
    EXPECT_NE(nullptr, parentSession);
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: MainWindowCloseInner
 * @tc.desc: MainWindowCloseInner Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MainWindowCloseInner, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MainWindowCloseInner");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(WMError::WM_OK, window->MainWindowCloseInner());
}

/**
 * @tc.name: SetWindowMode01
 * @tc.desc: SetWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetWindowMode01, TestSize.Level1)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("SetWindowMode01");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow->property_->SetPersistentId(1007);
    SessionInfo subSessionInfo = { "CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;
    subWindow->property_->SetWindowModeSupportType(0);
    auto ret = subWindow->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE, ret);
}

/**
 * @tc.name: SetWindowMode02
 * @tc.desc: SetWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetWindowMode02, TestSize.Level1)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("SetWindowMode02");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow->property_->SetPersistentId(1007);
    SessionInfo subSessionInfo = { "CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;
    subWindow->property_->SetWindowModeSupportType(1);
    auto ret = subWindow->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetWindowMode03
 * @tc.desc: SetWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetWindowMode03, TestSize.Level1)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("SetWindowMode03");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow->property_->SetPersistentId(0);
    SessionInfo subSessionInfo = {"CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility"};
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;
    subWindow->property_->SetWindowModeSupportType(1);
    auto ret = subWindow->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: UpdateNewSize01
 * @tc.desc: UpdateNewSize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, UpdateNewSize01, TestSize.Level1)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("UpdateNewSize01SubWindow");
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow->property_->SetPersistentId(1003);
    SessionInfo subSessionInfo = { "CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;
    subWindow->UpdateNewSize();
    Rect windowRect = { 0, 0, 0, 0 };
    WindowLimits windowLimits = { 0, 0, 0, 0, 0.0, 0, 0 };
    subWindow->property_->SetRequestRect(windowRect);
    subWindow->property_->SetWindowRect(windowRect);
    subWindow->property_->SetWindowLimits(windowLimits);
    subWindow->UpdateNewSize();
    windowRect.width_ = 10;
    windowRect.height_ = 10;
    subWindow->UpdateNewSize();
    windowRect.width_ = 0;
    windowRect.height_ = 0;
    ASSERT_NE(nullptr, subWindow->property_);
    subWindow->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    subWindow->UpdateNewSize();
    Rect windowRect1 = { 10, 10, 10, 10 };
    WindowLimits windowLimits1 = { 100, 100, 100, 100, 0.0, 0, 0 };
    subWindow->property_->SetRequestRect(windowRect1);
    subWindow->property_->SetWindowLimits(windowLimits1);
    subWindow->UpdateNewSize();
    Rect windowRect2 = { 200, 200, 200, 200 };
    subWindow->property_->SetRequestRect(windowRect2);
    subWindow->UpdateNewSize();
}

/**
 * @tc.name: UpdateNewSize02
 * @tc.desc: UpdateNewSize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, UpdateNewSize02, TestSize.Level1)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("UpdateNewSize01SubWindow");
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow->property_->SetPersistentId(1003);
    SessionInfo subSessionInfo = { "CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;
    subWindow->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    WindowLimits windowLimits = { 100, 100, 100, 100, 0.0, 0, 0 };
    subWindow->property_->SetWindowLimits(windowLimits);
    Rect windowRect = { 200, 200, 200, 200 };
    subWindow->property_->SetRequestRect(windowRect);
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    subWindow->windowSystemConfig_.freeMultiWindowEnable_ = false;
    subWindow->windowSystemConfig_.freeMultiWindowSupport_ = false;
    subWindow->UpdateNewSize();
    subWindow->property_->SetRequestRect(windowRect);
    subWindow->windowSystemConfig_.freeMultiWindowSupport_ = true;
    subWindow->UpdateNewSize();
    subWindow->property_->SetRequestRect(windowRect);
    subWindow->windowSystemConfig_.freeMultiWindowEnable_ = true;
    subWindow->windowSystemConfig_.freeMultiWindowSupport_ = false;
    subWindow->UpdateNewSize();
    subWindow->property_->SetRequestRect(windowRect);
    subWindow->windowSystemConfig_.freeMultiWindowSupport_ = true;
    subWindow->UpdateNewSize();
    subWindow->property_->SetRequestRect(windowRect);
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow->windowSystemConfig_.freeMultiWindowEnable_ = false;
    subWindow->windowSystemConfig_.freeMultiWindowSupport_ = false;
    subWindow->UpdateNewSize();
    subWindow->property_->SetRequestRect(windowRect);
    subWindow->windowSystemConfig_.freeMultiWindowSupport_ = true;
    subWindow->UpdateNewSize();
    subWindow->property_->SetRequestRect(windowRect);
    subWindow->windowSystemConfig_.freeMultiWindowEnable_ = true;
    subWindow->windowSystemConfig_.freeMultiWindowSupport_ = false;
    subWindow->UpdateNewSize();
    subWindow->property_->SetRequestRect(windowRect);
    subWindow->windowSystemConfig_.freeMultiWindowSupport_ = false;
    subWindow->UpdateNewSize();
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    subWindow->UpdateNewSize();
    ASSERT_NE(nullptr, subWindow->property_);
}

/**
 * @tc.name: UpdateSubWindowStateAndNotify01
 * @tc.desc: UpdateSubWindowStateAndNotify
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, UpdateSubWindowStateAndNotify01, TestSize.Level1)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("UpdateSubWindowStateAndNotify01");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow->property_->SetPersistentId(1005);
    SessionInfo subSessionInfo = { "CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSubWindowStateAndNotify02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1006);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    std::vector<sptr<WindowSessionImpl>> vec;
    WindowSceneSessionImpl::subWindowSessionMap_.insert(
        std::pair<int32_t, std::vector<sptr<WindowSessionImpl>>>(1006, vec));
    subWindow->UpdateSubWindowStateAndNotify(1006, WindowState::STATE_HIDDEN);
    WindowSceneSessionImpl::subWindowSessionMap_[1006].push_back(subWindow);
    subWindow->state_ = WindowState::STATE_SHOWN;
    subWindow->UpdateSubWindowStateAndNotify(1006, WindowState::STATE_HIDDEN);
    subWindow->state_ = WindowState::STATE_HIDDEN;
    subWindow->UpdateSubWindowStateAndNotify(1006, WindowState::STATE_HIDDEN);
    subWindow->state_ = WindowState::STATE_SHOWN;
    subWindow->UpdateSubWindowStateAndNotify(1006, WindowState::STATE_SHOWN);
    subWindow->state_ = WindowState::STATE_SHOWN;
    subWindow->UpdateSubWindowStateAndNotify(1006, WindowState::STATE_SHOWN);
    ASSERT_EQ(WMError::WM_OK, subWindow->Destroy(true));
    ASSERT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: PreLayoutOnShow01
 * @tc.desc: PreLayoutOnShow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, PreLayoutOnShow01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("PreLayoutOnShow01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(2345);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = nullptr;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window->uiContent_);
    window->PreLayoutOnShow(WindowType::WINDOW_TYPE_APP_SUB_WINDOW, displayInfo);
    window->hostSession_ = session;
    window->PreLayoutOnShow(WindowType::WINDOW_TYPE_APP_SUB_WINDOW, displayInfo);
    Rect testRect = {10, 20, 100, 200};
    window->GetProperty()->SetRequestRect(testRect);
    window->PreLayoutOnShow(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, displayInfo);
    ASSERT_EQ(window->GetRect(), testRect);
    window->PreLayoutOnShow(WindowType::WINDOW_TYPE_APP_SUB_WINDOW, displayInfo);
    ASSERT_EQ(window->GetRect(), testRect);
}

/**
 * @tc.name: PreLayoutOnShow02
 * @tc.desc: PreLayoutOnShow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, PreLayoutOnShow02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("PreLayoutOnShow02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(2345);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window->uiContent_);

    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();

    KeyboardLayoutParams tmpParams;
    const Rect landscapeKeyboardRect = {1, 1, 4, 3};
    tmpParams.LandscapeKeyboardRect_ = landscapeKeyboardRect;
    const Rect portraitKeyboardRect = {1, 1, 3, 4};
    tmpParams.PortraitKeyboardRect_ = portraitKeyboardRect;

    displayInfo->width_ = 10;
    displayInfo->height_ = 8;
    window->property_->SetRequestRect({0, 0, 0, 0});
    window->property_->SetKeyboardLayoutParams(tmpParams);
    window->PreLayoutOnShow(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, displayInfo);
    ASSERT_NE(window->property_->requestRect_, landscapeKeyboardRect);

    tmpParams.displayId_ = 0;
    displayInfo->screenId_ = 0;
    displayInfo->width_ = 8;
    displayInfo->height_ = 10;
    window->property_->AddKeyboardLayoutParams(0, tmpParams);
    window->PreLayoutOnShow(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, displayInfo);
    ASSERT_EQ(window->property_->requestRect_, portraitKeyboardRect);
}

/**
 * @tc.name: IsLandscape
 * @tc.desc: IsLandscape
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, IsLandscape, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsLandscape");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(2345);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, window->hostSession_);
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, window->uiContent_);
    window->IsLandscape(0);
    EXPECT_FALSE(window->IsLandscape(1234));
    EXPECT_FALSE(window->IsLandscape(DISPLAY_ID_INVALID));
}

/**
 * @tc.name: KeepKeyboardOnFocus01
 * @tc.desc: KeepKeyboardOnFocus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, KeepKeyboardOnFocus01, TestSize.Level1)
{
    sptr<WindowOption> keyboardOption = sptr<WindowOption>::MakeSptr();
    keyboardOption->SetWindowName("KeepKeyboardOnFocus01");
    keyboardOption->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sptr<WindowSceneSessionImpl> keyboardWindow = sptr<WindowSceneSessionImpl>::MakeSptr(keyboardOption);
    keyboardWindow->KeepKeyboardOnFocus(false);
    ASSERT_EQ(keyboardWindow->property_->keepKeyboardFlag_, false);

    keyboardWindow->KeepKeyboardOnFocus(true);
    ASSERT_EQ(keyboardWindow->property_->keepKeyboardFlag_, true);
}

static sptr<WindowSceneSessionImpl> CreateWindow(std::string windowName, WindowType type, int32_t id)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName(windowName);
    option->SetWindowType(type);
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(id);
    return window;
}

/**
 * @tc.name: GetParentMainWindowId
 * @tc.desc: GetParentMainWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetParentMainWindowId001, TestSize.Level1)
{
    using SessionPair = std::pair<uint64_t, sptr<WindowSessionImpl>>;
    // toastSubWindow is function caller
    sptr<WindowSceneSessionImpl> toastWindow = CreateWindow("toastWindow", WindowType::WINDOW_TYPE_APP_SUB_WINDOW, 99);
    toastWindow->property_->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_TOAST);
    toastWindow->windowSessionMap_["toastWindow"] = SessionPair(toastWindow->GetPersistentId(), toastWindow);
    int32_t res = 0;
    res = toastWindow->GetParentMainWindowId(0);
    ASSERT_EQ(res, 0);

    sptr<WindowSceneSessionImpl> mainWindow = CreateWindow("mainWindow", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, 100);
    ASSERT_NE(mainWindow, nullptr);
    toastWindow->windowSessionMap_["mainWindow"] = SessionPair(mainWindow->GetPersistentId(), mainWindow);
    toastWindow->property_->SetParentPersistentId(mainWindow->GetPersistentId());
    res = toastWindow->GetParentMainWindowId(toastWindow->GetPersistentId());
    ASSERT_EQ(res, mainWindow->GetPersistentId());
    res = 0;

    sptr<WindowSceneSessionImpl> subWindow = CreateWindow("subWindow", WindowType::WINDOW_TYPE_APP_SUB_WINDOW, 101);
    ASSERT_NE(subWindow, nullptr);
    subWindow->property_->SetParentPersistentId(mainWindow->GetPersistentId());
    toastWindow->property_->SetParentPersistentId(subWindow->GetPersistentId());
    toastWindow->windowSessionMap_["subWindow"] = SessionPair(subWindow->GetPersistentId(), subWindow);
    res = toastWindow->GetParentMainWindowId(toastWindow->GetPersistentId());
    ASSERT_EQ(res, mainWindow->GetPersistentId());
    res = 0;

    sptr<WindowSceneSessionImpl> dialogWindow = CreateWindow("dialogWindow", WindowType::WINDOW_TYPE_DIALOG, 102);
    ASSERT_NE(dialogWindow, nullptr);
    dialogWindow->property_->SetParentPersistentId(mainWindow->GetPersistentId());
    toastWindow->property_->SetParentPersistentId(dialogWindow->GetPersistentId());
    toastWindow->windowSessionMap_["dialogWindow"] = SessionPair(dialogWindow->GetPersistentId(), dialogWindow);
    res = toastWindow->GetParentMainWindowId(toastWindow->GetPersistentId());
    ASSERT_EQ(res, mainWindow->GetPersistentId());
    res = 0;

    sptr<WindowSceneSessionImpl> pipWindow = CreateWindow("dialogWindow", WindowType::WINDOW_TYPE_PIP, 103);
    ASSERT_NE(pipWindow, nullptr);
    pipWindow->property_->SetParentPersistentId(mainWindow->GetPersistentId());
    toastWindow->property_->SetParentPersistentId(pipWindow->GetPersistentId());
    toastWindow->windowSessionMap_.insert(std::make_pair(
        "dialogWindow", std::pair<uint64_t, sptr<WindowSessionImpl>>(pipWindow->GetPersistentId(), pipWindow)));
    res = toastWindow->GetParentMainWindowId(toastWindow->GetPersistentId());
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: FindParentMainSession001
 * @tc.desc: FindParentMainSession001
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, FindParentMainSession001, TestSize.Level1)
{
    using SessionPair = std::pair<uint64_t, sptr<WindowSessionImpl>>;
    // toastSubWindow is function caller
    sptr<WindowSceneSessionImpl> toastWindow = CreateWindow("toastWindow", WindowType::WINDOW_TYPE_APP_SUB_WINDOW, 99);
    toastWindow->property_->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_TOAST);
    ASSERT_NE(toastWindow, nullptr);
    toastWindow->windowSessionMap_["toastWindow"] = SessionPair(toastWindow->GetPersistentId(), toastWindow);
    sptr<WindowSessionImpl> result = nullptr;

    result = toastWindow->FindParentMainSession(0, toastWindow->windowSessionMap_);
    ASSERT_EQ(result, nullptr);

    // mainWindow need to be found
    sptr<WindowSessionImpl> mainWindow = CreateWindow("mainWindow", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, 100);
    ASSERT_NE(mainWindow, nullptr);
    toastWindow->windowSessionMap_["mainWindow"] = SessionPair(mainWindow->GetPersistentId(), mainWindow);
    toastWindow->property_->SetParentPersistentId(mainWindow->GetPersistentId());
    result = toastWindow->FindParentMainSession(toastWindow->GetParentId(), toastWindow->windowSessionMap_);
    ASSERT_EQ(result, mainWindow);
    result = nullptr;

    sptr<WindowSessionImpl> subWindow = CreateWindow("subWindow", WindowType::WINDOW_TYPE_APP_SUB_WINDOW, 101);
    ASSERT_NE(subWindow, nullptr);
    subWindow->property_->SetParentPersistentId(mainWindow->GetPersistentId());
    toastWindow->property_->SetParentPersistentId(subWindow->GetPersistentId());
    toastWindow->windowSessionMap_["subWindow"] = SessionPair(subWindow->GetPersistentId(), subWindow);
    result = toastWindow->FindParentMainSession(toastWindow->GetParentId(), toastWindow->windowSessionMap_);
    ASSERT_EQ(result, mainWindow);
    result = nullptr;

    sptr<WindowSessionImpl> floatWindow = CreateWindow("floatWindow", WindowType::WINDOW_TYPE_FLOAT, 105);
    ASSERT_NE(subWindow, nullptr);
    toastWindow->property_->SetParentPersistentId(floatWindow->GetPersistentId());
    floatWindow->property_->SetParentPersistentId(0);
    toastWindow->windowSessionMap_["floatWindow"] = SessionPair(floatWindow->GetPersistentId(), floatWindow);
    result = toastWindow->FindParentMainSession(toastWindow->GetParentId(), toastWindow->windowSessionMap_);
    ASSERT_EQ(result, floatWindow);
    result = nullptr;

    floatWindow->property_->SetParentPersistentId(mainWindow->GetPersistentId());
    result = toastWindow->FindParentMainSession(toastWindow->GetParentId(), toastWindow->windowSessionMap_);
    ASSERT_EQ(result, mainWindow);
}

/**
 * @tc.name: hasAncestorFloatSession
 * @tc.desc: hasAncestorFloatSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, hasAncestorFloatSession, TestSize.Level1)
{
    using SessionPair = std::pair<uint64_t, sptr<WindowSessionImpl>>;
    int32_t subWindowId = 1;
    auto subWindow = CreateWindow("subWindow", WindowType::WINDOW_TYPE_APP_SUB_WINDOW, subWindowId);
    subWindow->windowSessionMap_.clear();
    subWindow->windowSessionMap_["nullptr"] = SessionPair(0, nullptr);
    EXPECT_FALSE(subWindow->hasAncestorFloatSession(0, subWindow->windowSessionMap_));
    EXPECT_FALSE(subWindow->hasAncestorFloatSession(subWindowId, subWindow->windowSessionMap_));

    subWindow->windowSessionMap_["subWindow"] = SessionPair(subWindowId, subWindow);
    EXPECT_FALSE(subWindow->hasAncestorFloatSession(subWindowId, subWindow->windowSessionMap_));

    auto window = CreateWindow("window", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, 2);
    window->windowSessionMap_["window"] = SessionPair(window->GetPersistentId(), window);
    subWindow->property_->SetParentPersistentId(window->GetPersistentId());
    subWindow->property_->SetParentId(window->GetPersistentId());
    EXPECT_FALSE(subWindow->hasAncestorFloatSession(subWindowId, subWindow->windowSessionMap_));

    subWindowId = 3;
    auto subWindow2 = CreateWindow("subWindow2", WindowType::WINDOW_TYPE_APP_SUB_WINDOW, subWindowId);
    subWindow2->property_->SetParentPersistentId(subWindow->GetPersistentId());
    subWindow2->property_->SetParentId(subWindow->GetPersistentId());
    subWindow2->windowSessionMap_["subWindow2"] = SessionPair(subWindowId, subWindow2);
    EXPECT_FALSE(subWindow->hasAncestorFloatSession(subWindowId, subWindow->windowSessionMap_));

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    EXPECT_TRUE(subWindow->hasAncestorFloatSession(subWindowId, subWindow->windowSessionMap_));
    subWindow->windowSessionMap_.clear();
}

/**
 * @tc.name: GetParentSessionAndVerify_forFloatSubWindow
 * @tc.desc: hasAncestorFloatSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetParentSessionAndVerify_forFloatSubWindow, TestSize.Level1)
{
    using SessionPair = std::pair<uint64_t, sptr<WindowSessionImpl>>;
    auto toastSubWindow = CreateWindow("toastSubWindow", WindowType::WINDOW_TYPE_APP_SUB_WINDOW, 1);
    toastSubWindow->property_->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_TOAST);
    toastSubWindow->windowSessionMap_.clear();
    sptr<WindowSessionImpl> parentSession = nullptr;
    auto res = toastSubWindow->GetParentSessionAndVerify(true, parentSession);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, res);

    toastSubWindow->windowSessionMap_["toastSubWindow"] =
        SessionPair(toastSubWindow->GetPersistentId(), toastSubWindow);
    toastSubWindow->GetParentSessionAndVerify(true, parentSession);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, res);
    auto subWindow = CreateWindow("subWindow", WindowType::WINDOW_TYPE_APP_SUB_WINDOW, 2);
    subWindow->windowSessionMap_["subWindow"] = SessionPair(subWindow->GetPersistentId(), subWindow);
    auto floatWindow = CreateWindow("floatWindow", WindowType::WINDOW_TYPE_FLOAT, 3);
    floatWindow->windowSessionMap_["floatWindow"] = SessionPair(floatWindow->GetPersistentId(), floatWindow);
    toastSubWindow->property_->SetParentPersistentId(subWindow->GetPersistentId());
    toastSubWindow->property_->SetParentId(subWindow->GetPersistentId());
    subWindow->property_->SetParentPersistentId(floatWindow->GetPersistentId());
    subWindow->property_->SetParentId(floatWindow->GetPersistentId());
    res = toastSubWindow->GetParentSessionAndVerify(true, parentSession);
    toastSubWindow->windowSessionMap_.clear();
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: IsPcOrPadFreeMultiWindowMode
 * @tc.desc: IsPcOrPadFreeMultiWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, IsPcOrPadFreeMultiWindowMode, TestSize.Level1)
{
    sptr<WindowSessionImpl> mainWindow = CreateWindow("mainWindow", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, 100);
    ASSERT_NE(mainWindow, nullptr);
    mainWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(true, mainWindow->IsPcOrPadFreeMultiWindowMode());
    mainWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(false, mainWindow->IsPcOrPadFreeMultiWindowMode());
}

/**
 * @tc.name: IsPcOrPadFreeMultiWindowMode
 * @tc.desc: uitype = padwindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, IsPcOrPadFreeMultiWindowMode002, TestSize.Level1)
{
    sptr<WindowSessionImpl> mainWindow = CreateWindow("mainWindow", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, 100);
    ASSERT_NE(mainWindow, nullptr);
    mainWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    mainWindow->SetFreeMultiWindowMode(true);
    ASSERT_EQ(true, mainWindow->IsPcOrPadFreeMultiWindowMode());
    mainWindow->SetFreeMultiWindowMode(false);
    ASSERT_EQ(false, mainWindow->IsPcOrPadFreeMultiWindowMode());
}

static sptr<WindowSceneSessionImpl> CreateWindowWithDisplayId(std::string windowName,
                                                              WindowType type,
                                                              int64_t displayId = DISPLAY_ID_INVALID,
                                                              uint32_t parentId = INVALID_WINDOW_ID)
{
    using SessionPair = std::pair<uint64_t, sptr<WindowSessionImpl>>;
    static uint32_t windowPersistentId = 106;
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName(windowName);
    option->SetWindowType(type);
    option->SetParentId(parentId);
    option->SetDisplayId(displayId);
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(windowPersistentId++);
    WindowSessionImpl::windowSessionMap_[std::move(windowName)] = SessionPair(window->GetPersistentId(), window);
    return window;
}

/**
 * @tc.name: SetSpecificDisplayId01
 * @tc.desc: SetSpecificDisplayId01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetSpecificDisplayId01, TestSize.Level1)
{
    // create main window
    int64_t displayId = 12;
    auto mainWindowContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    sptr<WindowSceneSessionImpl> mainWindow =
        CreateWindowWithDisplayId("mainWindow", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, displayId);
    mainWindow->context_ = mainWindowContext;
    // create sub window
    sptr<WindowSceneSessionImpl> subWindow = CreateWindowWithDisplayId(
        "subWindow", WindowType::WINDOW_TYPE_APP_SUB_WINDOW, DISPLAY_ID_INVALID, mainWindow->GetPersistentId());

    // create float window
    sptr<WindowSceneSessionImpl> floatWindow = CreateWindowWithDisplayId("floatWindow", WindowType::WINDOW_TYPE_FLOAT);
    floatWindow->context_ = mainWindow->context_;
    // create other window
    uint64_t globalSearchDisplayId = 5678;
    sptr<WindowSceneSessionImpl> globalSearchWindow =
        CreateWindowWithDisplayId("globalWindow", WindowType::WINDOW_TYPE_GLOBAL_SEARCH, globalSearchDisplayId);
    // test display id
    subWindow->CreateAndConnectSpecificSession();
    ASSERT_EQ(subWindow->property_->GetDisplayId(), displayId);
    floatWindow->CreateSystemWindow(WindowType::WINDOW_TYPE_FLOAT);
    ASSERT_EQ(floatWindow->property_->GetDisplayId(), displayId);
    globalSearchWindow->CreateSystemWindow(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    ASSERT_EQ(globalSearchWindow->property_->GetDisplayId(), globalSearchDisplayId);
    floatWindow->CreateSystemWindow(WindowType::WINDOW_TYPE_FB);
    ASSERT_EQ(floatWindow->property_->GetDisplayId(), displayId);
}

/**
 * @tc.name: SetFullScreenWaterfallMode
 * @tc.desc: test SetFullScreenWaterfallMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetFullScreenWaterfallMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFullScreenWaterfallMode");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->windowMode_ = WindowMode::WINDOW_MODE_FULLSCREEN;
    EXPECT_EQ(WSError::WS_OK, window->SetFullScreenWaterfallMode(true));
    EXPECT_EQ(WSError::WS_DO_NOTHING, window->SetFullScreenWaterfallMode(true));
    EXPECT_TRUE(window->isFullScreenWaterfallMode_.load());
    EXPECT_EQ(WindowMode::WINDOW_MODE_FULLSCREEN, window->lastWindowModeBeforeWaterfall_.load());
    EXPECT_EQ(WSError::WS_OK, window->SetFullScreenWaterfallMode(false));
    EXPECT_FALSE(window->isFullScreenWaterfallMode_.load());
    EXPECT_EQ(WindowMode::WINDOW_MODE_UNDEFINED, window->lastWindowModeBeforeWaterfall_.load());
}

/**
 * @tc.name: SetSupportEnterWaterfallMode
 * @tc.desc: test SetSupportEnterWaterfallMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetSupportEnterWaterfallMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSupportEnterWaterfallMode");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WSError::WS_OK, window->SetSupportEnterWaterfallMode(true));
}

/**
 * @tc.name: OnContainerModalEvent
 * @tc.desc: test OnContainerModalEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, OnContainerModalEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnContainerModalEvent");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->OnContainerModalEvent("not_waterfall_window_event", ""));
}

/**
 * @tc.name: OnContainerModalEvent
 * @tc.desc: test OnContainerModalEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, OnContainerModalEvent02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OnContainerModalEvent02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_NE(nullptr, session);
    window->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, window->OnContainerModalEvent("win_change_to_2_3_landscape", ""));
    ASSERT_EQ(WMError::WM_OK, window->OnContainerModalEvent("win_change_to_1_1_landscape", ""));
    ASSERT_EQ(WMError::WM_OK, window->OnContainerModalEvent("win_change_to_18_9_landscape", ""));
    ASSERT_EQ(WMError::WM_OK, window->OnContainerModalEvent("win_change_to_default_landscape", ""));
    ASSERT_EQ(WMError::WM_OK, window->OnContainerModalEvent("win_change_to_split_landscape", ""));
    ASSERT_EQ(WMError::WM_DO_NOTHING, window->OnContainerModalEvent("win_change_to_split_landscape_error", ""));
    ASSERT_EQ(WMError::WM_OK, window->OnContainerModalEvent("win_hover_event", ""));
}

/**
 * @tc.name: HideTitleButton
 * @tc.desc: test HideTitleButton
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, HideTitleButton, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HideTitleButton");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_NE(nullptr, session);
    window->hostSession_ = session;
    bool hideSplitButton = true;
    bool hideMaximizeButton = true;
    bool hideMinimizeButton = true;
    bool hideCloseButton = true;
    window->HideTitleButton(hideSplitButton, hideMaximizeButton, hideMinimizeButton, hideCloseButton);
}

/**
 * @tc.name: UpdateConfigurationSyncForAll
 * @tc.desc: UpdateConfigurationSyncForAll
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, UpdateConfigurationSyncForAll, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateConfigurationSyncForAll");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();
    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle1", "CreateTestModule1", "CreateTestAbility1" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSession->hostSession_ = session;
    windowSession->property_->SetPersistentId(1);
    windowSession->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK, windowSession->Create(abilityContext_, session));
    windowSceneSessionImpl->UpdateConfigurationSyncForAll(configuration);
    ASSERT_EQ(WMError::WM_OK, windowSession->Destroy(true));
}

/**
 * @tc.name: NotifyDrawingCompleted
 * @tc.desc: NotifyDrawingCompleted
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, NotifyDrawingCompleted, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyDrawingCompleted");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    windowSessionImpl->hostSession_ = nullptr;
    auto ret = window->NotifyDrawingCompleted();
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: NotifyDrawingCompleted01
 * @tc.desc: NotifyDrawingCompleted
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, NotifyDrawingCompleted01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyDrawingCompleted01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSessionImpl->hostSession_ = session;
    WindowSessionProperty windowSessionProperty;
    windowSessionProperty.SetPersistentId(1);

    auto ret = window->NotifyDrawingCompleted();
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: MoveTo
 * @tc.desc: MoveTo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MoveTo, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    option->SetWindowName("MoveTo");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    MoveConfiguration moveConfiguration;

    windowSessionImpl->hostSession_ = nullptr;
    auto ret = window->MoveTo(0, 0, true, moveConfiguration);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    windowSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    ret = window->MoveTo(0, 0, true, moveConfiguration);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    windowSessionImpl->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    window->MoveTo(0, 0, true, moveConfiguration);
}

/**
 * @tc.name: MoveTo
 * @tc.desc: MoveTo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MoveTo002, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    option->SetWindowName("MoveTo002");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    MoveConfiguration moveConfiguration;
    auto ret = window->MoveTo(0, 0, true, moveConfiguration);

    windowSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    window->state_ = WindowState::STATE_INITIAL;
    window->property_->SetPersistentId(1);
    ret = window->MoveTo(0, 0, true, moveConfiguration);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: MoveTo
 * @tc.desc: MoveTo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MoveTo003, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    option->SetWindowName("MoveTo002");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    MoveConfiguration moveConfiguration;
    auto ret = window->MoveTo(0, 0, true, moveConfiguration);

    windowSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    window->state_ = WindowState::STATE_INITIAL;
    ret = window->MoveTo(0, 0, true, moveConfiguration);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    windowSessionImpl->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    int64_t displayId = 12;
    auto mainWindowContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    sptr<WindowSceneSessionImpl> mainWindow =
        CreateWindowWithDisplayId("mainWindow", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, displayId);
    mainWindow->context_ = mainWindowContext;
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ret = window->MoveTo(0, 0, true, moveConfiguration);
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: MoveWindowToGlobal
 * @tc.desc: MoveWindowToGlobal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MoveWindowToGlobal, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MoveWindowToGlobal");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    MoveConfiguration moveConfiguration;
    window->hostSession_ = nullptr;
    auto ret = window->MoveWindowToGlobal(0, 0, moveConfiguration);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_INITIAL;

    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ret = window->MoveWindowToGlobal(0, 0, moveConfiguration);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS);

    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    window->MoveWindowToGlobal(0, 0, moveConfiguration);
}

/**
 * @tc.name: MoveWindowToGlobal01
 * @tc.desc: MoveWindowToGlobal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MoveWindowToGlobal01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MoveWindowToGlobal01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    MoveConfiguration moveConfiguration;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_INITIAL;
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);

    auto ret = window->MoveWindowToGlobal(0, 0, moveConfiguration);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: OnContainerModalEvent01
 * @tc.desc: OnContainerModalEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, OnContainerModalEvent01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("VerifySubWindowLevel01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    std::string eventName = "window_scene_session_impl_test4";
    std::string value = "window_scene_session_impl_test4";
    auto ret = window->OnContainerModalEvent(eventName, value);
    ASSERT_EQ(WMError::WM_DO_NOTHING, ret);

    eventName = "scb_waterfall_window_event";
    window->hostSession_ = nullptr;
    ret = window->OnContainerModalEvent(eventName, value);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: IsSystemDensityChanged01
 * @tc.desc: IsSystemDensityChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, IsSystemDensityChanged01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsSystemDensityChanged01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto displayInfo = sptr<DisplayInfo>::MakeSptr();
    displayInfo->SetVirtualPixelRatio(1.5f);
    auto ret = window->IsSystemDensityChanged(displayInfo);
    ASSERT_EQ(true, ret);

    window->lastSystemDensity_ = 1.5f;
    ret = window->IsSystemDensityChanged(displayInfo);
    ASSERT_EQ(false, ret);
}

/**
 * @tc.name: GetWindowPropertyInfo
 * @tc.desc: GetWindowPropertyInfo01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetWindowPropertyInfo01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetFocusable(false);
    option->SetTouchable(true);
    option->SetDisplayId(999);
    option->SetWindowName("GetWindowPropertyInfo01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->hostSession_ = nullptr;
    WindowPropertyInfo windowPropertyInfo;
    auto ret = window->GetWindowPropertyInfo(windowPropertyInfo);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: GetWindowPropertyInfo
 * @tc.desc: GetWindowPropertyInfo02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetWindowPropertyInfo02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetFocusable(false);
    option->SetTouchable(true);
    option->SetDisplayId(999);
    option->SetWindowName("GetWindowPropertyInfo02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    WindowPropertyInfo windowPropertyInfo;
    auto ret = window->GetWindowPropertyInfo(windowPropertyInfo);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(false, windowPropertyInfo.isFocusable);
    EXPECT_EQ(true, windowPropertyInfo.isTouchable);
    EXPECT_EQ(999, windowPropertyInfo.displayId);
}

/**
 * @tc.name: SetWindowContainerColor01
 * @tc.desc: SetWindowContainerColor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetWindowContainerColor01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetBundleName("SetWindowContainerColor");
    option->SetWindowName("SetWindowContainerColor");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    std::string activeColor = "#00000000";
    std::string inactiveColor = "#FF000000";
    WMError res = window->SetWindowContainerColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->containerColorList_.insert("SetWindowContainerColor");
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    res = window->SetWindowContainerColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    res = window->SetWindowContainerColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    res = window->SetWindowContainerColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->SetDecorEnable(true);
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    res = window->SetWindowContainerColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    res = window->SetWindowContainerColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_OK);

    inactiveColor = "#F1000000";
    res = window->SetWindowContainerColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    inactiveColor = "rgb#FF000000";
    res = window->SetWindowContainerColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    activeColor = "rgb#00000000";
    res = window->SetWindowContainerColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SetWindowContainerColor02
 * @tc.desc: SetWindowContainerColor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetWindowContainerColor02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetBundleName("SetWindowContainerColor");
    option->SetWindowName("SetWindowContainerColor");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::string activeColor = "#00000000";
    std::string inactiveColor = "#FF000000";
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    auto res = window->SetWindowContainerColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->containerColorList_.insert("SetWindowContainerColor");
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    res = window->SetWindowContainerColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    res = window->SetWindowContainerColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: SetWindowContainerModalColor01
 * @tc.desc: SetWindowContainerModalColor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetWindowContainerModalColor01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetBundleName("SetWindowContainerModalColor");
    option->SetWindowName("SetWindowContainerModalColor");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    std::string activeColor = "#FF000000";
    std::string inactiveColor = "#00000000";
    WMError res = window->SetWindowContainerModalColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    res = window->SetWindowContainerModalColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    res = window->SetWindowContainerModalColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);

    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    res = window->SetWindowContainerModalColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);

    window->property_->SetDecorEnable(true);
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.isSystemDecorEnable_ = true;
    res = window->SetWindowContainerModalColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    res = window->SetWindowContainerModalColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_OK);

    inactiveColor = "rgb#FF000000";
    res = window->SetWindowContainerModalColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    activeColor = "rgb#00000000";
    res = window->SetWindowContainerModalColor(activeColor, inactiveColor);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SetRotationLocked
 * @tc.desc: SetRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetRotationLocked, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetRotationLocked");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(0);
    WMError ret = window->SetRotationLocked(false);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
 
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_NE(nullptr, session);
    window->hostSession_ = session;
    
    window->windowSystemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    ret = window->SetRotationLocked(false);
    EXPECT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
 
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = window->SetRotationLocked(false);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW_TYPE);
 
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_HANDWRITE);
    ret = window->SetRotationLocked(false);
    EXPECT_EQ(ret, WMError::WM_OK);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window->SetRotationLocked(false);
    EXPECT_EQ(ret, WMError::WM_OK);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ret = window->SetRotationLocked(false);
    EXPECT_EQ(ret, WMError::WM_OK);
}
 
/**
 * @tc.name: GetRotationLocked
 * @tc.desc: GetRotationLocked
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetRotationLocked, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetRotationLocked");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(0);
    bool locked = false;
    WMError ret = window->GetRotationLocked(locked);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
 
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    EXPECT_NE(nullptr, session);
    window->hostSession_ = session;
    
    window->windowSystemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    ret = window->GetRotationLocked(locked);
    EXPECT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
 
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = window->GetRotationLocked(locked);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW_TYPE);
 
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_HANDWRITE);
    ret = window->GetRotationLocked(locked);
    EXPECT_EQ(ret, WMError::WM_OK);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window->GetRotationLocked(locked);
    EXPECT_EQ(ret, WMError::WM_OK);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ret = window->GetRotationLocked(locked);
    EXPECT_EQ(ret, WMError::WM_OK);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
