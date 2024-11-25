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
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window_adapter.h"
#include "singleton_mocker.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
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
HWTEST_F(WindowSceneSessionImplTest4, UpdateSurfaceNodeAfterCustomAnimation, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("UpdateSurfaceNodeAfterCustomAnimation");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    windowSceneSessionImpl->hostSession_ = nullptr;
    auto ret = windowSceneSessionImpl->UpdateSurfaceNodeAfterCustomAnimation(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_END);
    ret = windowSceneSessionImpl->UpdateSurfaceNodeAfterCustomAnimation(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE);
    ret = windowSceneSessionImpl->UpdateSurfaceNodeAfterCustomAnimation(true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RegisterSessionRecoverListener
 * @tc.desc: RegisterSessionRecoverListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, RegisterSessionRecoverListener, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("RegisterSessionRecoverListener");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    int32_t persistentId = windowSceneSessionImpl->property_->GetPersistentId();
    WindowAdapter& windowAdapter = SingletonContainer::Get<WindowAdapter>();
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    windowSceneSessionImpl->RegisterSessionRecoverListener(true);
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 0);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetCollaboratorType(CollaboratorType::RESERVE_TYPE);
    windowSceneSessionImpl->RegisterSessionRecoverListener(true);
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 0);

    windowSceneSessionImpl->property_->SetCollaboratorType(CollaboratorType::DEFAULT_TYPE);
    windowSceneSessionImpl->RegisterSessionRecoverListener(true);
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 1);
    windowAdapter.UnregisterSessionRecoverCallbackFunc(persistentId);
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 0);

    windowSceneSessionImpl->state_ = WindowState::STATE_DESTROYED;
    windowSceneSessionImpl->RegisterSessionRecoverListener(true);
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 1);
    windowAdapter.UnregisterSessionRecoverCallbackFunc(persistentId);
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 0);

    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->RegisterSessionRecoverListener(true);
    EXPECT_EQ(CollaboratorType::DEFAULT_TYPE, windowSceneSessionImpl->property_->GetCollaboratorType());
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 1);
    windowAdapter.UnregisterSessionRecoverCallbackFunc(persistentId);
    EXPECT_EQ(CollaboratorType::DEFAULT_TYPE, windowSceneSessionImpl->property_->GetCollaboratorType());
}

/**
 * @tc.name: ConsumePointerEvent
 * @tc.desc: ConsumePointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, ConsumePointerEvent, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("ConsumePointerEvent");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    windowSceneSessionImpl->hostSession_ = session;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    windowSceneSessionImpl->ConsumePointerEvent(pointerEvent);
    windowSceneSessionImpl->hostSession_ = nullptr;
    windowSceneSessionImpl->ConsumePointerEvent(pointerEvent);
    pointerEvent = nullptr;
    windowSceneSessionImpl->ConsumePointerEvent(pointerEvent);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetTurnScreenOn(true);
    auto ret = windowSceneSessionImpl->IsTurnScreenOn();
    EXPECT_EQ(true, ret);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetTurnScreenOn(false);
    ret = windowSceneSessionImpl->IsTurnScreenOn();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: GetSystemSizeLimits01
 * @tc.desc: GetSystemSizeLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetSystemSizeLimits01, Function | SmallTest | Level2)
{
    uint32_t minMainWidth = 10;
    uint32_t minMainHeight = 20;
    uint32_t minSubWidth = 30;
    uint32_t minSubHeight = 40;
    uint32_t displayWidth = 100;
    uint32_t displayHeight = 100;
    float displayVpr = 1.0f;
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("GetSystemSizeLimits01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);
    windowSceneSessionImpl->windowSystemConfig_.miniWidthOfMainWindow_ = minMainWidth;
    windowSceneSessionImpl->windowSystemConfig_.miniHeightOfMainWindow_ = minMainHeight;
    windowSceneSessionImpl->windowSystemConfig_.miniWidthOfSubWindow_ = minSubWidth;
    windowSceneSessionImpl->windowSystemConfig_.miniHeightOfSubWindow_ = minSubHeight;

    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    WindowLimits limits = windowSceneSessionImpl->GetSystemSizeLimits(displayWidth, displayHeight, displayVpr);
    EXPECT_EQ(limits.minWidth_, minMainWidth);
    EXPECT_EQ(limits.minHeight_, minMainHeight);

    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    limits = windowSceneSessionImpl->GetSystemSizeLimits(displayWidth, displayHeight, displayVpr);
    EXPECT_EQ(limits.minWidth_, minSubWidth);
    EXPECT_EQ(limits.minHeight_, minSubHeight);

    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    limits = windowSceneSessionImpl->GetSystemSizeLimits(displayWidth, displayHeight, displayVpr);
    EXPECT_EQ(limits.minWidth_, static_cast<uint32_t>(MIN_FLOATING_WIDTH * displayVpr));
    EXPECT_EQ(limits.minHeight_, static_cast<uint32_t>(MIN_FLOATING_HEIGHT * displayVpr));
}

/**
 * @tc.name: CalculateNewLimitsByRatio01
 * @tc.desc: CalculateNewLimitsByRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, CalculateNewLimitsByRatio01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateNewLimitsByRatio01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    WindowLimits newLimits = {0, 0, 0, 0, 0.0, 0.0};
    WindowLimits customizedLimits = {3, 3, 3, 3, 0.0, 0.0};
    windowSceneSessionImpl->CalculateNewLimitsByRatio(newLimits, customizedLimits);
    newLimits = {500, 500, 500, 500, 0.0, 0.0};
    customizedLimits = {3, 3, 3, 3, 2.0, 2.0};
    windowSceneSessionImpl->CalculateNewLimitsByRatio(newLimits, customizedLimits);
    customizedLimits = {3, 3, 3, 3, 1.0, 1.0};
    windowSceneSessionImpl->CalculateNewLimitsByRatio(newLimits, customizedLimits);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_END);
    auto ret = windowSceneSessionImpl->UpdateAnimationFlagProperty(true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: CalculateNewLimitsByRatio02
 * @tc.desc: CalculateNewLimitsByRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, CalculateNewLimitsByRatio02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateNewLimitsByRatio02");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    WindowLimits newLimits = {0, 0, 0, 0, 0.0, 0.0};
    WindowLimits customizedLimits = {3, 3, 3, 3, 0.0, 0.0};
    windowSceneSessionImpl->CalculateNewLimitsByRatio(newLimits, customizedLimits);
    newLimits = {500, 500, 500, 500, 0.0, 0.0};
    customizedLimits = {3, 3, 3, 3, 2.0, 2.0};
    windowSceneSessionImpl->CalculateNewLimitsByRatio(newLimits, customizedLimits);
    customizedLimits = {3, 3, 3, 3, 1.0, 1.0};
    windowSceneSessionImpl->CalculateNewLimitsByRatio(newLimits, customizedLimits);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE);
    auto ret = windowSceneSessionImpl->UpdateAnimationFlagProperty(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: HandlePointDownEvent
 * @tc.desc: HandlePointDownEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, HandlePointDownEvent, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("HandlePointDownEvent");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    MMI::PointerEvent::PointerItem pointerItem;
    int32_t sourceType = 1;
    float vpr = 1.0f;
    WSRect rect = {1, 1, 1, 1};

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    windowSceneSessionImpl->hostSession_ = session;
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE);

    windowSceneSessionImpl->property_->SetDragEnabled(false);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    auto ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem, sourceType, vpr, rect);
    EXPECT_EQ(true, ret);

    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetDragEnabled(false);
    ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem, sourceType, vpr, rect);
    EXPECT_EQ(true, ret);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);

    ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem, sourceType, vpr, rect);
    EXPECT_EQ(true, ret);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);

    pointerItem.SetWindowX(100);
    pointerItem.SetWindowY(100);
    ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem, sourceType, vpr, rect);
    EXPECT_EQ(true, ret);
    pointerItem.SetWindowX(1);
    ret = windowSceneSessionImpl->HandlePointDownEvent(pointerEvent, pointerItem, sourceType, vpr, rect);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: UpdateWindowModeImmediately
 * @tc.desc: UpdateWindowModeImmediately
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, UpdateWindowModeImmediately, Function | SmallTest | Level2)
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
    windowSceneSessionImpl->hostSession_ = nullptr;
    ret = windowSceneSessionImpl->UpdateWindowModeImmediately(WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
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
HWTEST_F(WindowSceneSessionImplTest4, AdjustWindowAnimationFlag, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("AdjustWindowAnimationFlag");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;

    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    windowSceneSessionImpl->animationTransitionController_ = sptr<IAnimationTransitionController>::MakeSptr();
    ASSERT_NE(nullptr, windowSceneSessionImpl->animationTransitionController_);
    windowSceneSessionImpl->AdjustWindowAnimationFlag(true);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    auto ret = windowSceneSessionImpl->property_->GetAnimationFlag();
    EXPECT_EQ(3, ret);
    windowSceneSessionImpl->animationTransitionController_ = nullptr;
    windowSceneSessionImpl->AdjustWindowAnimationFlag(true);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    ret = windowSceneSessionImpl->property_->GetAnimationFlag();
    EXPECT_EQ(1, ret);
    windowSceneSessionImpl->AdjustWindowAnimationFlag(false);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    ret = windowSceneSessionImpl->property_->GetAnimationFlag();
    EXPECT_EQ(0, ret);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->AdjustWindowAnimationFlag(false);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    ret = windowSceneSessionImpl->property_->GetAnimationFlag();
    EXPECT_EQ(1, ret);
    windowSceneSessionImpl->enableDefaultAnimation_ = false;
    windowSceneSessionImpl->AdjustWindowAnimationFlag(false);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    ret = windowSceneSessionImpl->property_->GetAnimationFlag();
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name: UpdateConfigurationForAll
 * @tc.desc: UpdateConfigurationForAll
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, UpdateConfigurationForAll, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("UpdateConfigurationForAll");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();
    ASSERT_NE(nullptr, configuration);
    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSession);
    SessionInfo sessionInfo = {"CreateTestBundle1", "CreateTestModule1", "CreateTestAbility1"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    windowSession->hostSession_ = session;
    ASSERT_NE(nullptr, windowSession->property_);
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
HWTEST_F(WindowSceneSessionImplTest4, SetWindowFlags, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("SetWindowFlags");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->SetWindowFlags(1);
    auto ret = windowSceneSessionImpl->SetWindowFlags(1);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = windowSceneSessionImpl->SetWindowFlags(2);
    EXPECT_EQ(WMError::WM_OK, ret);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    ret = windowSceneSessionImpl->NotifyPrepareClosePiPWindow();
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: ConsumePointerEventInner
 * @tc.desc: ConsumePointerEventInner
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, ConsumePointerEventInner, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("ConsumePointerEventInner");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
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
 * @tc.name: HandleEventForCompatibleMode
 * @tc.desc: HandleEventForCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, HandleEventForCompatibleMode, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("HandleEventForCompatibleMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
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
HWTEST_F(WindowSceneSessionImplTest4, HandleDownForCompatibleMode, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("HandleDownForCompatibleMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
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
HWTEST_F(WindowSceneSessionImplTest4, HandleMoveForCompatibleMode, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("HandleMoveForCompatibleMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
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
HWTEST_F(WindowSceneSessionImplTest4, HandleUpForCompatibleMode, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("HandleUpForCompatibleMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
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
HWTEST_F(WindowSceneSessionImplTest4, ConvertPointForCompatibleMode, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("ConvertPointForCompatibleMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
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
HWTEST_F(WindowSceneSessionImplTest4, IsInMappingRegionForCompatibleMode, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("IsInMappingRegionForCompatibleMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->property_->SetWindowRect({ 880, 0, 800, 1600 });
    int32_t displayX = 400;
    int32_t displayY = 400;
    bool ret = windowSceneSessionImpl->IsInMappingRegionForCompatibleMode(displayX, displayY);
    EXPECT_EQ(true, ret);
    displayX = 1000;
    displayY = 1000;
    ret = windowSceneSessionImpl->IsInMappingRegionForCompatibleMode(displayX, displayY);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: CheckTouchSlop
 * @tc.desc: CheckTouchSlop
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, CheckTouchSlop, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("CheckTouchSlop");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
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
HWTEST_F(WindowSceneSessionImplTest4, IgnoreClickEvent, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("IgnoreClickEvent");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
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
HWTEST_F(WindowSceneSessionImplTest4, RegisterKeyboardPanelInfoChangeListener, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("RegisterKeyboardPanelInfoChangeListener");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    windowSceneSessionImpl->keyboardPanelInfoChangeListeners_ = sptr<IKeyboardPanelInfoChangeListener>::MakeSptr();
    ASSERT_NE(nullptr, windowSceneSessionImpl->keyboardPanelInfoChangeListeners_);
    sptr<IKeyboardPanelInfoChangeListener> listener = nullptr;
    auto ret = windowSceneSessionImpl->RegisterKeyboardPanelInfoChangeListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
}

/**
 * @tc.name: GetSystemBarPropertyByType
 * @tc.desc: GetSystemBarPropertyByType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetSystemBarPropertyByType, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("GetSystemBarPropertyByType");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    windowSceneSessionImpl->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
}

/**
 * @tc.name: SetSpecificBarProperty
 * @tc.desc: SetSpecificBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetSpecificBarProperty, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("SetSpecificBarProperty");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
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
 * @tc.name: MoveToAsync01
 * @tc.desc: MoveToAsync
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MoveToAsync01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MoveToAsync01");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    option->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->MoveToAsync(10, 10));

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS, window->MoveToAsync(10, 10));
}

/**
 * @tc.name: MoveToAsync02
 * @tc.desc: MoveToAsync
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MoveToAsync02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MoveToAsync02");
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
    window->property_->SetPersistentId(10001);

    Rect rect;
    WMError ret;
    ret = window->Create(abilityContext_, nullptr);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    window->state_ = WindowState::STATE_SHOWN;
    ret = window->MoveToAsync(500, 500);
    EXPECT_EQ(WMError::WM_OK, ret);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(500, rect.posX_);
    EXPECT_EQ(500, rect.posY_);

    window->state_ = WindowState::STATE_HIDDEN;
    ret = window->MoveToAsync(20000, 20000);
    usleep(WAIT_SERVERAL_FRAMES);
    EXPECT_EQ(WMError::WM_OK, ret);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(20000, rect.posX_);
    EXPECT_EQ(20000, rect.posY_);
    ASSERT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: ResizeAsync01
 * @tc.desc: ResizeAsync
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, ResizeAsync01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ResizeAsync01");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    option->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OP_IN_CUR_STATUS, window->ResizeAsync(500, 500));
}

/**
 * @tc.name: ResizeAsync02
 * @tc.desc: ResizeAsync
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, ResizeAsync02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ResizeAsync02");
    option->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    Rect rect;
    WMError ret;
    ret = window->Create(abilityContext_, nullptr);
    EXPECT_EQ(WMError::WM_OK, ret);
    window->property_->SetPersistentId(10012);
    ret = window->Show();
    EXPECT_EQ(WMError::WM_OK, ret);

    WindowLimits windowLimits;
    ret = window->GetWindowLimits(windowLimits);
    EXPECT_EQ(WMError::WM_OK, ret);

    window->state_ = WindowState::STATE_SHOWN;
    ret = window->ResizeAsync(windowLimits.maxWidth_ - 100, windowLimits.maxHeight_ - 100);
    EXPECT_EQ(WMError::WM_OK, ret);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(windowLimits.maxWidth_ - 100, rect.width_);
    EXPECT_EQ(windowLimits.maxHeight_ - 100, rect.height_);

    window->state_ = WindowState::STATE_HIDDEN;
    ret = window->ResizeAsync(windowLimits.maxWidth_ + 100, windowLimits.maxHeight_ + 100);
    EXPECT_EQ(WMError::WM_OK, ret);
    usleep(WAIT_SERVERAL_FRAMES);
    rect = window->property_->GetWindowRect();
    EXPECT_EQ(windowLimits.maxWidth_, rect.width_);
    EXPECT_EQ(windowLimits.maxHeight_, rect.height_);
    ASSERT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: NotifyDialogStateChange
 * @tc.desc: NotifyDialogStateChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, NotifyDialogStateChange, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("NotifyDialogStateChange");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
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
 * @tc.name: MoveTo02
 * @tc.desc: MoveTo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MoveTo02, Function | SmallTest | Level2)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, subOption);
    subOption->SetWindowName("MoveTo02SubWindow");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    ASSERT_NE(nullptr, subWindow);
    ASSERT_NE(nullptr, subWindow->property_);
    subWindow->property_->SetPersistentId(1001);
    SessionInfo subSessionInfo = {"CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility"};
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    ASSERT_NE(nullptr, subSession);
    subWindow->hostSession_ = subSession;
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(2, 2));

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("MoveTo02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle2", "CreateTestModule2", "CreateTestAbility2"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(3, 3));
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(3, 3));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(3, 4));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(4, 4));
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(4, 4));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(4, 5));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(5, 5));
    ASSERT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: GetWindowStatus01
 * @tc.desc: GetWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetWindowStatus01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("GetWindowStatus01");
    sptr<WindowSceneSessionImpl> window = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;

    WindowStatus windowStatus;
    ASSERT_EQ(WMError::WM_OK, window->GetWindowStatus(windowStatus));
    ASSERT_EQ(WindowStatus::WINDOW_STATUS_UNDEFINED, windowStatus);
}

/**
 * @tc.name: GetWindowStatus02
 * @tc.desc: GetWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetWindowStatus02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("GetWindowStatus02");
    option->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    sptr<WindowSceneSessionImpl> window = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
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
HWTEST_F(WindowSceneSessionImplTest4, GetWindowStatus03, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("GetWindowStatus03");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetDisplayId(0);
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, window);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
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
 * @tc.name: VerifySubWindowLevel
 * @tc.desc: VerifySubWindowLevel Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, VerifySubWindowLevel, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    EXPECT_NE(nullptr, option);
    option->SetWindowName("VerifySubWindowLevel");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    option->SetDisplayId(0);
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_NE(nullptr, window);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    EXPECT_NE(nullptr, session);
    window->hostSession_ = session;
    ASSERT_EQ(false, window->VerifySubWindowLevel(window->GetParentId()));
}

/**
 * @tc.name: AddSubWindowMapForExtensionWindow
 * @tc.desc: AddSubWindowMapForExtensionWindow Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, AddSubWindowMapForExtensionWindow, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("AddSubWindowMapForExtensionWindow");
    option->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->AddSubWindowMapForExtensionWindow();
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: IsPcOrPadCapabilityEnabled
 * @tc.desc: IsPcOrPadCapabilityEnabled Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, IsPcOrPadCapabilityEnabled, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("IsPcOrPadCapabilityEnabled");
    option->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(true, window->IsPcOrPadCapabilityEnabled());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(false, window->IsPcOrPadCapabilityEnabled());
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    EXPECT_EQ(false, window->IsPcOrPadCapabilityEnabled());
    window->property_->SetIsUIExtFirstSubWindow(true);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    EXPECT_EQ(true, window->IsPcOrPadCapabilityEnabled());
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: GetParentSessionAndVerify
 * @tc.desc: GetParentSessionAndVerify Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, GetParentSessionAndVerify, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("GetParentSessionAndVerify");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sptr<WindowSessionImpl> parentSession = nullptr;
    auto res = window->GetParentSessionAndVerify(false, parentSession);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, res);
    res = window->GetParentSessionAndVerify(true, parentSession);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, res);

    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, subOption);
    subOption->SetWindowName("GetParentSessionAndVerify2");
    subOption->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    ASSERT_NE(nullptr, subWindow);
    SessionInfo subSessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    ASSERT_NE(nullptr, subSession);
    subWindow->hostSession_ = subSession;
    ASSERT_NE(nullptr, subWindow->property_);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentId(1);
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    std::vector<sptr<WindowSessionImpl>> vec;
    WindowSceneSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, vec));
    WindowSceneSessionImpl::subWindowSessionMap_[1].push_back(subWindow);
    res = subWindow->GetParentSessionAndVerify(false, parentSession);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, res);
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(window->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(window->GetWindowId(), window)));
    res = subWindow->GetParentSessionAndVerify(false, parentSession);
    EXPECT_EQ(WMError::WM_OK, res);
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: MainWindowCloseInner
 * @tc.desc: MainWindowCloseInner Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MainWindowCloseInner, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("MainWindowCloseInner");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, window);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(WMError::WM_OK, window->MainWindowCloseInner());
}

/**
 * @tc.name: MoveTo03
 * @tc.desc: MoveTo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MoveTo03, Function | SmallTest | Level2)
{
    sptr<WindowOption> subOption = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, subOption);
    subOption->SetWindowName("MoveTo01SubWindow");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = new (std::nothrow) WindowSceneSessionImpl(subOption);
    ASSERT_NE(nullptr, subWindow);
    ASSERT_NE(nullptr, subWindow->property_);
    subWindow->property_->SetPersistentId(1001);
    SessionInfo subSessionInfo = {"CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility"};
    sptr<SessionMocker> subSession = new (std::nothrow) SessionMocker(subSessionInfo);
    ASSERT_NE(nullptr, subSession);
    subWindow->hostSession_ = subSession;
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(2, 2));

    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("MoveTo02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, window);
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(window->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(window->GetWindowId(), window)));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(3, 3));
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(4, 3));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(4, 4));
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(5, 4));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(5, 4));
    WindowSceneSessionImpl::windowSessionMap_.erase(window->GetWindowName());
}

/**
 * @tc.name: SetWindowMode01
 * @tc.desc: SetWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, SetWindowMode01, Function | SmallTest | Level2)
{
    sptr<WindowOption> subOption = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, subOption);
    subOption->SetWindowName("SetWindowMode01");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = new (std::nothrow) WindowSceneSessionImpl(subOption);
    ASSERT_NE(nullptr, subWindow);
    ASSERT_NE(nullptr, subWindow->property_);
    subWindow->property_->SetPersistentId(1007);
    SessionInfo subSessionInfo = {"CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility"};
    sptr<SessionMocker> subSession = new (std::nothrow) SessionMocker(subSessionInfo);
    ASSERT_NE(nullptr, subSession);
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
HWTEST_F(WindowSceneSessionImplTest4, SetWindowMode02, Function | SmallTest | Level2)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("SetWindowMode02");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    ASSERT_NE(nullptr, subWindow->property_);
    subWindow->property_->SetPersistentId(1007);
    SessionInfo subSessionInfo = {"CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility"};
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;
    subWindow->property_->SetWindowModeSupportType(1);
    auto ret = subWindow->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UpdateNewSize01
 * @tc.desc: UpdateNewSize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, UpdateNewSize01, Function | SmallTest | Level2)
{
    sptr<WindowOption> subOption = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, subOption);
    subOption->SetWindowName("UpdateNewSize01SubWindow");
    sptr<WindowSceneSessionImpl> subWindow = new (std::nothrow) WindowSceneSessionImpl(subOption);
    ASSERT_NE(nullptr, subWindow);
    ASSERT_NE(nullptr, subWindow->property_);
    subWindow->property_->SetPersistentId(1003);
    SessionInfo subSessionInfo = {"CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility"};
    sptr<SessionMocker> subSession = new (std::nothrow) SessionMocker(subSessionInfo);
    ASSERT_NE(nullptr, subSession);
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
 * @tc.name: UpdateSubWindowStateAndNotify01
 * @tc.desc: UpdateSubWindowStateAndNotify
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, UpdateSubWindowStateAndNotify01, Function | SmallTest | Level2)
{
    sptr<WindowOption> subOption = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, subOption);
    subOption->SetWindowName("UpdateSubWindowStateAndNotify01");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = new (std::nothrow) WindowSceneSessionImpl(subOption);
    ASSERT_NE(nullptr, subWindow);
    ASSERT_NE(nullptr, subWindow->property_);
    subWindow->property_->SetPersistentId(1005);
    SessionInfo subSessionInfo = {"CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility"};
    sptr<SessionMocker> subSession = new (std::nothrow) SessionMocker(subSessionInfo);
    ASSERT_NE(nullptr, subSession);
    subWindow->hostSession_ = subSession;

    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    option->SetWindowName("UpdateSubWindowStateAndNotify02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = new (std::nothrow) WindowSceneSessionImpl(option);
    ASSERT_NE(nullptr, window);
    ASSERT_NE(nullptr, window->property_);
    window->property_->SetPersistentId(1006);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    window->hostSession_ = session;
    std::vector<sptr<WindowSessionImpl>> vec;
    WindowSceneSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
            std::vector<sptr<WindowSessionImpl>>>(1006, vec));
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
HWTEST_F(WindowSceneSessionImplTest4, PreLayoutOnShow01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("PreLayoutOnShow01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(window->property_, nullptr);
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
}

/**
 * @tc.name: KeepKeyboardOnFocus01
 * @tc.desc: KeepKeyboardOnFocus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, KeepKeyboardOnFocus01, Function | SmallTest | Level2)
{
    sptr<WindowOption> keyboardOption = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, keyboardOption);
    keyboardOption->SetWindowName("KeepKeyboardOnFocus01");
    keyboardOption->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sptr<WindowSceneSessionImpl> keyboardWindow = new (std::nothrow) WindowSceneSessionImpl(keyboardOption);
    ASSERT_NE(nullptr, keyboardWindow);

    keyboardWindow->KeepKeyboardOnFocus(false);
    ASSERT_EQ(keyboardWindow->property_->keepKeyboardFlag_, false);

    keyboardWindow->KeepKeyboardOnFocus(true);
    ASSERT_EQ(keyboardWindow->property_->keepKeyboardFlag_, true);
}

/**
 * @tc.name: MoveAndResizeKeyboard01
 * @tc.desc: MoveAndResizeKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MoveAndResizeKeyboard01, Function | SmallTest | Level2)
{
    sptr<WindowOption> keyboardOption = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, keyboardOption);
    keyboardOption->SetWindowName("MoveAndResizeKeyboard01");
    keyboardOption->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sptr<WindowSceneSessionImpl> keyboardWindow = new (std::nothrow) WindowSceneSessionImpl(keyboardOption);
    ASSERT_NE(nullptr, keyboardWindow);

    bool isLandscape = false;
    keyboardWindow->property_->displayId_ = 0;
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(0);
    if (display != nullptr) {
        isLandscape = display->GetWidth() > display->GetHeight();
    }
    KeyboardLayoutParams param;
    param.LandscapeKeyboardRect_ = { 100, 100, 100, 200 };
    param.PortraitKeyboardRect_ = { 200, 200, 200, 100 };
    auto result = keyboardWindow->MoveAndResizeKeyboard(param);
    auto expectRect = isLandscape ? param.LandscapeKeyboardRect_ : param.PortraitKeyboardRect_;
    ASSERT_EQ(keyboardWindow->property_->requestRect_, expectRect);
    ASSERT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: MoveAndResizeKeyboard02
 * @tc.desc: MoveAndResizeKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, MoveAndResizeKeyboard02, Function | SmallTest | Level2)
{
    sptr<WindowOption> keyboardOption = new (std::nothrow) WindowOption();
    ASSERT_NE(nullptr, keyboardOption);
    keyboardOption->SetWindowName("MoveAndResizeKeyboard02");
    keyboardOption->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sptr<WindowSceneSessionImpl> keyboardWindow = new (std::nothrow) WindowSceneSessionImpl(keyboardOption);
    ASSERT_NE(nullptr, keyboardWindow);

    bool isLandscape = false;
    keyboardWindow->property_->displayId_ = DISPLAY_ID_INVALID;
    auto defaultDisplayInfo = DisplayManager::GetInstance().GetDefaultDisplay();
    if (defaultDisplayInfo != nullptr) {
        isLandscape = defaultDisplayInfo->GetWidth() > defaultDisplayInfo->GetHeight();
    }
    KeyboardLayoutParams param;
    param.LandscapeKeyboardRect_ = { 100, 100, 100, 200 };
    param.PortraitKeyboardRect_ = { 200, 200, 200, 100 };
    auto result = keyboardWindow->MoveAndResizeKeyboard(param);
    auto expectRect = isLandscape ? param.LandscapeKeyboardRect_ : param.PortraitKeyboardRect_;
    ASSERT_EQ(keyboardWindow->property_->requestRect_, expectRect);
    ASSERT_EQ(result, WMError::WM_OK);
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
HWTEST_F(WindowSceneSessionImplTest4, GetParentMainWindowId001, Function | SmallTest | Level2)
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
    toastWindow->windowSessionMap_.insert(std::make_pair("dialogWindow",
        std::pair<uint64_t, sptr<WindowSessionImpl>>(pipWindow->GetPersistentId(), pipWindow)));
    res = toastWindow->GetParentMainWindowId(toastWindow->GetPersistentId());
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: FindParentMainSession001
 * @tc.desc: FindParentMainSession001
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, FindParentMainSession001, Function | SmallTest | Level2)
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
 * @tc.name: IsPcOrPadFreeMultiWindowMode
 * @tc.desc: IsPcOrPadFreeMultiWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest4, IsPcOrPadFreeMultiWindowMode, Function | SmallTest | Level2)
{
    sptr<WindowSessionImpl> mainWindow = CreateWindow("mainWindow", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, 100);
    ASSERT_NE(mainWindow, nullptr);
    mainWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(true, mainWindow->IsPcOrPadFreeMultiWindowMode());
    mainWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(false, mainWindow->IsPcOrPadFreeMultiWindowMode());
}

static sptr<WindowSceneSessionImpl> CreateWindowWithDisplayId(std::string windowName, WindowType type,
    int64_t displayId = DISPLAY_ID_INVALID, uint32_t parentId = INVALID_WINDOW_ID)
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
HWTEST_F(WindowSceneSessionImplTest4, SetSpecificDisplayId01, Function | SmallTest | Level2)
{
    // create main window
    int64_t displayId = 12;
    auto mainWindowContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    sptr<WindowSceneSessionImpl> mainWindow =
        CreateWindowWithDisplayId("mainWindow", WindowType::WINDOW_TYPE_APP_MAIN_WINDOW, displayId);
    mainWindow->context_ = mainWindowContext;
    // create sub window
    sptr<WindowSceneSessionImpl> subWindow = CreateWindowWithDisplayId("subWindow",
        WindowType::WINDOW_TYPE_APP_SUB_WINDOW, DISPLAY_ID_INVALID, mainWindow->GetPersistentId());
    
    // create float window
    sptr<WindowSceneSessionImpl> floatWindow = CreateWindowWithDisplayId("floatWindow", WindowType::WINDOW_TYPE_FLOAT);
    floatWindow->context_ = mainWindow->context_;
    // create other window
    uint64_t globalSearchDisplayId = 5678;
    sptr<WindowSceneSessionImpl> globalSearchWindow = CreateWindowWithDisplayId("globalWindow",
        WindowType::WINDOW_TYPE_GLOBAL_SEARCH, globalSearchDisplayId);
    // test display id
    subWindow->CreateAndConnectSpecificSession();
    ASSERT_EQ(subWindow->property_->GetDisplayId(), displayId);
    floatWindow->CreateSystemWindow(WindowType::WINDOW_TYPE_FLOAT);
    ASSERT_EQ(floatWindow->property_->GetDisplayId(), displayId);
    globalSearchWindow->CreateSystemWindow(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    ASSERT_EQ(globalSearchWindow->property_->GetDisplayId(), globalSearchDisplayId);
}
}
} // namespace Rosen
} // namespace OHOS