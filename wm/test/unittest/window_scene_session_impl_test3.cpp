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
class WindowSceneSessionImplTest3 : public testing::Test {
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
};

void WindowSceneSessionImplTest3::SetUpTestCase() {}

void WindowSceneSessionImplTest3::TearDownTestCase() {}

void WindowSceneSessionImplTest3::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSceneSessionImplTest3::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

RSSurfaceNode::SharedPtr WindowSceneSessionImplTest3::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "startingWindowTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    return surfaceNode;
}

namespace {
/**
 * @tc.name: UpdateOrientation
 * @tc.desc: UpdateOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, UpdateOrientation, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateOrientation");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto ret = windowSceneSessionImpl->UpdateOrientation();
    EXPECT_EQ(WSError::WS_OK, ret);

    windowSceneSessionImpl->hostSession_ = nullptr;
    windowSceneSessionImpl->UpdateDensity();

    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->UpdateDensity();

    windowSceneSessionImpl->userLimitsSet_ = true;
    windowSceneSessionImpl->UpdateDensity();
}

/**
 * @tc.name: UpdateBrightness
 * @tc.desc: UpdateBrightness
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, UpdateBrightness, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(option, nullptr);
    option->SetWindowName("UpdateBrightness");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(windowSceneSessionImpl, nullptr);
    EXPECT_EQ(WSError::WS_OK, windowSceneSessionImpl->UpdateBrightness(1.0f));
}

/**
 * @tc.name: SetWindowMask
 * @tc.desc: SetWindowMask
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, SetWindowMask, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowMask");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::vector<std::vector<uint32_t>> windowMask;

    windowSceneSessionImpl->hostSession_ = nullptr;
    auto ret = windowSceneSessionImpl->SetWindowMask(windowMask);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    windowSceneSessionImpl->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle0", "CreateTestModule0", "CreateTestAbility0" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    ret = windowSceneSessionImpl->SetWindowMask(windowMask);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    windowMask = { { 1, 1, 1 }, { 2, 2, 2 }, { 3, 3, 3 } };
    ret = windowSceneSessionImpl->SetWindowMask(windowMask);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_FALSE(nullptr == windowSceneSessionImpl->HandleWindowMask(windowMask));
    Rect rect = { 9, 9, 9, 9 };
    windowSceneSessionImpl->property_->SetRequestRect(rect);
    EXPECT_TRUE(nullptr == windowSceneSessionImpl->HandleWindowMask(windowMask));
    rect = { 3, 3, 3, 3 };
    windowSceneSessionImpl->property_->SetRequestRect(rect);
    EXPECT_FALSE(nullptr == windowSceneSessionImpl->HandleWindowMask(windowMask));
    rect = { 2, 2, 2, 2 };
    windowSceneSessionImpl->property_->SetRequestRect(rect);
    EXPECT_TRUE(nullptr == windowSceneSessionImpl->HandleWindowMask(windowMask));
    rect = { 0, 0, 0, 0 };
    windowSceneSessionImpl->property_->SetRequestRect(rect);
    EXPECT_FALSE(nullptr == windowSceneSessionImpl->HandleWindowMask(windowMask));
}

/**
 * @tc.name: UpdateTitleInTargetPos
 * @tc.desc: UpdateTitleInTargetPos
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, UpdateTitleInTargetPos, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateTitleInTargetPos");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto ret = windowSceneSessionImpl->UpdateTitleInTargetPos(true, 0);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_WINDOW, ret);
    ret = windowSceneSessionImpl->UpdateTitleInTargetPos(false, 0);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_WINDOW, ret);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle0", "CreateTestModule0", "CreateTestAbility0" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->windowSessionMap_.insert(
        std::make_pair(windowSceneSessionImpl->GetWindowName(),
                       std::make_pair(windowSceneSessionImpl->GetWindowId(), windowSceneSessionImpl)));
    ret = windowSceneSessionImpl->SwitchFreeMultiWindow(true);
    EXPECT_EQ(WSError::WS_OK, ret);
    ret = windowSceneSessionImpl->SwitchFreeMultiWindow(false);
    EXPECT_EQ(WSError::WS_OK, ret);

    windowSceneSessionImpl->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ASSERT_NE(nullptr, windowSceneSessionImpl->uiContent_);
    ret = windowSceneSessionImpl->UpdateTitleInTargetPos(true, 0);
    EXPECT_EQ(WSError::WS_OK, ret);
    ret = windowSceneSessionImpl->UpdateTitleInTargetPos(false, 0);
    EXPECT_EQ(WSError::WS_OK, ret);

    windowSceneSessionImpl->uiContent_ = nullptr;
    ret = windowSceneSessionImpl->UpdateTitleInTargetPos(true, 0);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
    ret = windowSceneSessionImpl->UpdateTitleInTargetPos(false, 0);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
    windowSceneSessionImpl->windowSessionMap_.erase(windowSceneSessionImpl->GetWindowName());
}

/**
 * @tc.name: CheckParmAndPermission01
 * @tc.desc: CheckParmAndPermission
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, CheckParmAndPermission01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CheckParmAndPermission01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->surfaceNode_ = nullptr;

    auto ret = windowSceneSessionImpl->CheckParmAndPermission();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    ret = windowSceneSessionImpl->SetCornerRadius(1.0f);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    auto surfaceNode_mocker = CreateRSSurfaceNode();
    ASSERT_NE(nullptr, surfaceNode_mocker);
    windowSceneSessionImpl->surfaceNode_ = surfaceNode_mocker;
    ret = windowSceneSessionImpl->CheckParmAndPermission();
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: PerformBack
 * @tc.desc: PerformBack
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, PerformBack, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("PerformBack");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    windowSceneSessionImpl->PerformBack();

    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    ASSERT_NE(nullptr, abilityContext_);
    windowSceneSessionImpl->context_ = abilityContext_;
    windowSceneSessionImpl->PerformBack();
    windowSceneSessionImpl->context_ = nullptr;
    windowSceneSessionImpl->PerformBack();

    windowSceneSessionImpl->hostSession_ = nullptr;
    windowSceneSessionImpl->PerformBack();
}

/**
 * @tc.name: AdjustKeyboardLayout
 * @tc.desc: AdjustKeyboardLayout
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, AdjustKeyboardLayout, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AdjustKeyboardLayout");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    KeyboardLayoutParams params;

    auto ret = windowSceneSessionImpl->AdjustKeyboardLayout(params);
    EXPECT_EQ(WMError::WM_OK, ret);

    windowSceneSessionImpl->hostSession_ = nullptr;
    ret = windowSceneSessionImpl->AdjustKeyboardLayout(params);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: TestWindowScreenListenerOnDisconnectBehavior
 * @tc.desc: TestWindowScreenListenerOnDisconnectBehavior
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, TestWindowScreenListenerOnDisconnectBehavior, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowScreenListenerTest");

    sptr<WindowSceneSessionImpl> windowSession1 = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSession1);
    windowSession1->property_->SetPersistentId(1);
    windowSession1->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSession1->property_->keyboardLayoutParamsMap_[0] = {};

    sptr<WindowSceneSessionImpl> windowSession2 = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSession2);
    windowSession2->property_->SetPersistentId(2);
    windowSession2->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    windowSession2->property_->keyboardLayoutParamsMap_[0] = {};

    WindowSceneSessionImpl::windowSessionMap_["WindowName1"] = {1, windowSession1};
    WindowSceneSessionImpl::windowSessionMap_["WindowName2"] = {2, windowSession2};

    WindowSceneSessionImpl::WindowScreenListener listener;
    listener.OnDisconnect(0);
    EXPECT_EQ(windowSession1->property_->keyboardLayoutParamsMap_.size(), 1); // should not clear the layout
    EXPECT_EQ(windowSession2->property_->keyboardLayoutParamsMap_.size(), 0); // should clear the layout
    WindowSceneSessionImpl::windowSessionMap_.erase("WindowName1");
    WindowSceneSessionImpl::windowSessionMap_.erase("WindowName2");
}

/**
 * @tc.name: UpdateSubWindowState
 * @tc.desc: UpdateSubWindowState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, UpdateSubWindowState, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSubWindowState");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    WindowType type = WindowType::APP_SUB_WINDOW_BASE;
    ASSERT_NE(nullptr, windowSceneSessionImpl);

    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->UpdateSubWindowState(type);
    windowSceneSessionImpl->state_ = WindowState::STATE_CREATED;
    windowSceneSessionImpl->UpdateSubWindowState(type);
    type = WindowType::SYSTEM_WINDOW_BASE;
    windowSceneSessionImpl->UpdateSubWindowState(type);
    type = WindowType::APP_MAIN_WINDOW_BASE;
    windowSceneSessionImpl->UpdateSubWindowState(type);
    ASSERT_EQ(false, windowSceneSessionImpl->GetDefaultDensityEnabled());
}

/**
 * @tc.name: GetWindowFlags
 * @tc.desc: GetWindowFlags
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, GetWindowFlags, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowFlags");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->property_->SetWindowFlags(0);
    auto ret = windowSceneSessionImpl->GetWindowFlags();
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name: IsApplicationModalSubWindowShowing
 * @tc.desc: Test whether application modal sub window could be detected correctly
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, IsApplicationModalSubWindowShowing, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsApplicationModalSubWindowShowing_ParentWindow");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(101);

    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("IsApplicationModalSubWindowShowing_SubWindow1");
    subOption->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSceneSessionImpl> subWindow1 = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow1->property_->SetPersistentId(102);
    subWindow1->property_->SetParentPersistentId(101);
    window->subWindowSessionMap_[window->GetPersistentId()].push_back(subWindow1);
    EXPECT_EQ(false, window->IsApplicationModalSubWindowShowing(101));

    subOption->SetWindowName("IsApplicationModalSubWindowShowing_SubWindow2");
    sptr<WindowSceneSessionImpl> subWindow2 = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow2->property_->SetPersistentId(103);
    subWindow2->property_->SetParentPersistentId(101);
    subWindow2->property_->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    subWindow2->property_->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL);
    subWindow2->state_ = WindowState::STATE_SHOWN;
    window->subWindowSessionMap_[window->GetPersistentId()].push_back(subWindow2);
    EXPECT_EQ(false, window->IsApplicationModalSubWindowShowing(101));
}

/**
 * @tc.name: NotifyPrepareClosePiPWindow
 * @tc.desc: NotifyPrepareClosePiPWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, NotifyPrepareClosePiPWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPrepareClosePiPWindow");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    auto ret = windowSceneSessionImpl->NotifyPrepareClosePiPWindow();
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);

    auto ret1 = windowSceneSessionImpl->KeepKeyboardOnFocus(true);
    EXPECT_EQ(WmErrorCode::WM_OK, ret1);
}

/**
 * @tc.name: NotifyWindowSessionProperty
 * @tc.desc: NotifyWindowSessionProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, NotifyWindowSessionProperty, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyWindowSessionProperty");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto ret = windowSceneSessionImpl->NotifyWindowSessionProperty();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    ret = windowSceneSessionImpl->NotifyWindowSessionProperty();
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: AddWindowFlag01
 * @tc.desc: AddWindowFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, AddWindowFlag01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddWindowFlag");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto ret = windowSceneSessionImpl->AddWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: AddWindowFlag02
 * @tc.desc: AddWindowFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, AddWindowFlag02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddWindowFlag02");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->property_->SetPersistentId(10001);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;

    auto ret = windowSceneSessionImpl->AddWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_TRUE(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID) & windowSceneSessionImpl->GetWindowFlags());
}

/**
 * @tc.name: AddWindowFlag03
 * @tc.desc: AddWindowFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, AddWindowFlag03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddWindowFlag03");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->property_->SetPersistentId(10001);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;

    auto ret = windowSceneSessionImpl->AddWindowFlag(WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_TRUE(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE) &
                windowSceneSessionImpl->GetWindowFlags());
}

/**
 * @tc.name: AddWindowFlag04
 * @tc.desc: AddWindowFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, AddWindowFlag04, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddWindowFlag04");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->property_->SetPersistentId(10001);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;

    auto ret = windowSceneSessionImpl->AddWindowFlag(WindowFlag::WINDOW_FLAG_HANDWRITING);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_TRUE(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_HANDWRITING) & windowSceneSessionImpl->GetWindowFlags());
}

/**
 * @tc.name: AddWindowFlag05
 * @tc.desc: AddWindowFlag
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, AddWindowFlag05, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AddWindowFlag05");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->property_->SetPersistentId(10001);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;

    auto ret = windowSceneSessionImpl->AddWindowFlag(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_TRUE(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED) &
                windowSceneSessionImpl->GetWindowFlags());
}

/**
 * @tc.name: SetDefaultProperty
 * @tc.desc: SetDefaultProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, SetDefaultProperty, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetDefaultProperty");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_TOAST);
    windowSceneSessionImpl->SetDefaultProperty();
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_POINTER);
    windowSceneSessionImpl->SetDefaultProperty();

    auto ret = windowSceneSessionImpl->UpdateDisplayId(0);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ChangeCallingWindowId
 * @tc.desc: ChangeCallingWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, ChangeCallingWindowId, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ChangeCallingWindowId");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->hostSession_ = nullptr;
    auto ret = windowSceneSessionImpl->ChangeCallingWindowId(0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    SessionInfo sessionInfo = { "CreateTestBundle0", "CreateTestModule0", "CreateTestAbility0" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    ret = windowSceneSessionImpl->ChangeCallingWindowId(10);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    EXPECT_EQ(0, windowSceneSessionImpl->property_->callingSessionId_);
}

/**
 * @tc.name: ChangeCallingWindowId01
 * @tc.desc: ChangeCallingWindowId01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, ChangeCallingWindowId01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ChangeCallingWindowId01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->hostSession_ = nullptr;
    auto ret = windowSceneSessionImpl->ChangeCallingWindowId(0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    SessionInfo sessionInfo = { "CreateTestBundle0", "CreateTestModule0", "CreateTestAbility0" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->property_->SetParentPersistentId(2);
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    ret = windowSceneSessionImpl->ChangeCallingWindowId(10);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(10, windowSceneSessionImpl->property_->callingSessionId_);
}

/**
 * @tc.name: RaiseToAppTop
 * @tc.desc: RaiseToAppTop
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, RaiseToAppTop, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RaiseToAppTop");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->property_->SetPersistentId(6);
    windowSceneSessionImpl->property_->SetParentPersistentId(0);
    auto ret = windowSceneSessionImpl->RaiseToAppTop();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    SessionInfo sessionInfo = { "CreateTestBundle0", "CreateTestModule0", "CreateTestAbility0" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    ret = windowSceneSessionImpl->RaiseToAppTop();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARENT, ret);
}

/**
 * @tc.name: SetTouchHotAreas
 * @tc.desc: SetTouchHotAreas
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, SetTouchHotAreas, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTouchHotAreas");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    std::vector<Rect> rects;
    Rect rect = { 800, 800, 1200, 1200 };
    rects.push_back(rect);
    auto ret = windowSceneSessionImpl->SetTouchHotAreas(rects);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: SetKeyboardTouchHotAreas
 * @tc.desc: SetKeyboardTouchHotAreas
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, SetKeyboardTouchHotAreas, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetKeyboardTouchHotAreas");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    KeyboardTouchHotAreas hotAreas;
    Rect rect = { 800, 800, 1200, 1200 };
    hotAreas.landscapeKeyboardHotAreas_.push_back(rect);
    hotAreas.landscapePanelHotAreas_.push_back(rect);
    hotAreas.portraitKeyboardHotAreas_.push_back(rect);
    hotAreas.portraitPanelHotAreas_.push_back(rect);
    auto ret = windowSceneSessionImpl->SetKeyboardTouchHotAreas(hotAreas);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_TYPE, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ret = windowSceneSessionImpl->SetKeyboardTouchHotAreas(hotAreas);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_CREATED;
    windowSceneSessionImpl->property_->SetPersistentId(1);
    ret = windowSceneSessionImpl->SetKeyboardTouchHotAreas(hotAreas);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetWindowLimits01
 * @tc.desc: GetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, GetWindowLimits01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowLimits01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    WindowLimits oldWindowLimits = { 4000, 4000, 2000, 2000, 0.0f, 0.0f };

    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->property_->SetDisplayId(0);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_CREATED;
    auto ret = windowSceneSessionImpl->SetWindowLimits(oldWindowLimits, false);
    EXPECT_EQ(WMError::WM_OK, ret);

    WindowLimits newWindowLimits;
    ret = windowSceneSessionImpl->GetWindowLimits(newWindowLimits);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(oldWindowLimits.maxWidth_, newWindowLimits.maxWidth_);
    EXPECT_EQ(oldWindowLimits.maxHeight_, newWindowLimits.maxHeight_);
    EXPECT_EQ(oldWindowLimits.minWidth_, newWindowLimits.minWidth_);
    EXPECT_EQ(oldWindowLimits.minHeight_, newWindowLimits.minHeight_);
}

/**
 * @tc.name: GetWindowLimits02
 * @tc.desc: GetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, GetWindowLimits02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowLimits02");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    WindowLimits windowLimits = { 1000, 1000, 1000, 1000, 0.0f, 0.0f };

    windowSceneSessionImpl->hostSession_ = nullptr;
    auto ret = windowSceneSessionImpl->GetWindowLimits(windowLimits);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: IsValidSystemWindowType
 * @tc.desc: IsValidSystemWindowType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, IsValidSystemWindowType, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsValidSystemWindowType");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    WindowType type = WindowType::WINDOW_TYPE_DRAGGING_EFFECT;
    EXPECT_FALSE(!windowSceneSessionImpl->IsValidSystemWindowType(type));
    type = WindowType::WINDOW_TYPE_SEARCHING_BAR;
    EXPECT_FALSE(!windowSceneSessionImpl->IsValidSystemWindowType(type));
    type = WindowType::WINDOW_TYPE_PANEL;
    EXPECT_FALSE(!windowSceneSessionImpl->IsValidSystemWindowType(type));
    type = WindowType::WINDOW_TYPE_VOLUME_OVERLAY;
    EXPECT_FALSE(!windowSceneSessionImpl->IsValidSystemWindowType(type));
    type = WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR;
    EXPECT_FALSE(!windowSceneSessionImpl->IsValidSystemWindowType(type));
    type = WindowType::WINDOW_TYPE_SYSTEM_TOAST;
    EXPECT_FALSE(!windowSceneSessionImpl->IsValidSystemWindowType(type));
    type = WindowType::WINDOW_TYPE_SYSTEM_FLOAT;
    EXPECT_FALSE(!windowSceneSessionImpl->IsValidSystemWindowType(type));
    type = WindowType::WINDOW_TYPE_HANDWRITE;
    EXPECT_FALSE(!windowSceneSessionImpl->IsValidSystemWindowType(type));
    type = WindowType::WINDOW_TYPE_NEGATIVE_SCREEN;
    EXPECT_TRUE(!windowSceneSessionImpl->IsValidSystemWindowType(type));
    type = WindowType::WINDOW_TYPE_THEME_EDITOR;
    EXPECT_TRUE(!windowSceneSessionImpl->IsValidSystemWindowType(type));
}

/**
 * @tc.name: UpdateFloatingWindowSizeBySizeLimits
 * @tc.desc: UpdateFloatingWindowSizeBySizeLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, UpdateFloatingWindowSizeBySizeLimits, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    option->SetWindowName("UpdateFloatingWindowSizeBySizeLimits");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_END);
    uint32_t maxWidth = 32;
    windowSceneSessionImpl->UpdateFloatingWindowSizeBySizeLimits(maxWidth, maxWidth);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    WindowLimits windowLimits = { 0, 0, 0, 0, 0.0f, 0.0f };
    windowSceneSessionImpl->property_->SetWindowLimits(windowLimits);
    windowSceneSessionImpl->UpdateFloatingWindowSizeBySizeLimits(maxWidth, maxWidth);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT_CAMERA);
    windowSceneSessionImpl->UpdateFloatingWindowSizeBySizeLimits(maxWidth, maxWidth);
}

/**
 * @tc.name: UpdateFloatingWindowSizeBySizeLimits01
 * @tc.desc: UpdateFloatingWindowSizeBySizeLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, UpdateFloatingWindowSizeBySizeLimits01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateFloatingWindowSizeBySizeLimits01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    uint32_t maxWidth = 32;
    WindowLimits windowLimits = { 1, 1, 1, 1, 0.0f, 2.0f };
    windowSceneSessionImpl->property_->SetWindowLimits(windowLimits);
    windowSceneSessionImpl->UpdateFloatingWindowSizeBySizeLimits(maxWidth, maxWidth);
    WindowLimits windowLimits1 = { 1, 2, 2, 2, 0.0f, 0.0f };
    windowSceneSessionImpl->property_->SetWindowLimits(windowLimits1);
    windowSceneSessionImpl->UpdateFloatingWindowSizeBySizeLimits(maxWidth, maxWidth);
    WindowLimits windowLimits2 = { 1, 2, 2, 2, 0.0f, 2.0f };
    windowSceneSessionImpl->property_->SetWindowLimits(windowLimits2);
    windowSceneSessionImpl->UpdateFloatingWindowSizeBySizeLimits(maxWidth, maxWidth);
}

/**
 * @tc.name: UpdateFloatingWindowSizeBySizeLimits02
 * @tc.desc: UpdateFloatingWindowSizeBySizeLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, UpdateFloatingWindowSizeBySizeLimits02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateFloatingWindowSizeBySizeLimits02");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT_CAMERA);
    WindowLimits windowLimits = { 1, 1, 1, 1, 0.0f, 2.0f };
    windowSceneSessionImpl->property_->SetWindowLimits(windowLimits);
    uint32_t widthLimits = 100;
    uint32_t heightLimits = 200;
    windowSceneSessionImpl->UpdateFloatingWindowSizeBySizeLimits(widthLimits, heightLimits);
    EXPECT_EQ(widthLimits, 100);
    EXPECT_EQ(heightLimits, 200);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    windowSceneSessionImpl->UpdateFloatingWindowSizeBySizeLimits(widthLimits, heightLimits);
    EXPECT_EQ(widthLimits, 1);
    EXPECT_EQ(heightLimits, 1);
}

/**
 * @tc.name: IsDecorEnable
 * @tc.desc: IsDecorEnable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, IsDecorEnable, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsDecorEnable");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->windowSystemConfig_.freeMultiWindowSupport_ = true;
    windowSceneSessionImpl->DisableAppWindowDecor();
    auto ret = windowSceneSessionImpl->IsDecorEnable();
    EXPECT_EQ(false, ret);
    windowSceneSessionImpl->windowSystemConfig_.freeMultiWindowSupport_ = false;
    ret = windowSceneSessionImpl->IsDecorEnable();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: UpdateWindowState
 * @tc.desc: UpdateWindowState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, UpdateWindowState, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateWindowState");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->windowSystemConfig_.maxFloatingWindowSize_ = UINT32_MAX;
    windowSceneSessionImpl->UpdateWindowState();

    windowSceneSessionImpl->windowSystemConfig_.maxFloatingWindowSize_ = 1920;
    windowSceneSessionImpl->property_->SetIsNeedUpdateWindowMode(true);
    windowSceneSessionImpl->UpdateWindowState();
    windowSceneSessionImpl->property_->SetIsNeedUpdateWindowMode(false);
    windowSceneSessionImpl->UpdateWindowState();
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetDragEnabled(true);
    windowSceneSessionImpl->UpdateWindowState();
    windowSceneSessionImpl->property_->SetDragEnabled(false);
    windowSceneSessionImpl->UpdateWindowState();
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    windowSceneSessionImpl->property_->SetDragEnabled(true);
    windowSceneSessionImpl->UpdateWindowState();
    windowSceneSessionImpl->property_->SetDragEnabled(false);
    windowSceneSessionImpl->UpdateWindowState();
    auto ret = windowSceneSessionImpl->property_->GetIsNeedUpdateWindowMode();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: Resize
 * @tc.desc: Resize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, Resize, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Resize");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    auto ret = windowSceneSessionImpl->Resize(0, 0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    ret = windowSceneSessionImpl->Resize(100, 100);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->property_->SetDisplayId(0);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    ret = windowSceneSessionImpl->Resize(100, 100);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ret = windowSceneSessionImpl->Resize(100, 100);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ret = windowSceneSessionImpl->Resize(100, 100);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(true);
    windowSceneSessionImpl->property_->SetWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN);
    windowSceneSessionImpl->property_->SetDragEnabled(false);
    ret = windowSceneSessionImpl->Resize(100, 100);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    windowSceneSessionImpl->property_->SetDragEnabled(true);
    WindowLimits windowLimits = {5000, 5000, 50, 50, 0.0f, 0.0f};
    windowSceneSessionImpl->property_->SetWindowLimits(windowLimits);
    ret = windowSceneSessionImpl->Resize(100, 100);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    WindowLimits windowLimits1 = {800, 800, 50, 50, 0.0f, 0.0f};
    windowSceneSessionImpl->property_->SetWindowLimits(windowLimits1);
    ret = windowSceneSessionImpl->Resize(100, 100);
    if (!windowSceneSessionImpl->IsFreeMultiWindowMode()) {
        EXPECT_EQ(WMError::WM_OK, ret);
    } else {
        EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    }
}

/**
 * @tc.name: Resize01
 * @tc.desc: Resize
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, Resize01, TestSize.Level1)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("Resize01SubWindow");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow->property_->SetPersistentId(1002);
    SessionInfo subSessionInfo = { "CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_OPERATION, subWindow->Resize(100, 100));
    subWindow->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    subWindow->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Resize01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1003);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    Rect request = { 100, 100, 100, 100 };
    subWindow->property_->SetRequestRect(request);
    subWindow->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ASSERT_EQ(WMError::WM_OK, subWindow->Resize(100, 100));
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(
        window->GetWindowName(), std::pair<uint64_t, sptr<WindowSessionImpl>>(window->GetWindowId(), window)));
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    ASSERT_EQ(WMError::WM_OK, subWindow->Resize(100, 100));
    ASSERT_EQ(WMError::WM_OK, subWindow->Resize(200, 200));
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ASSERT_EQ(WMError::WM_OK, subWindow->Resize(100, 200));
    ASSERT_EQ(WMError::WM_OK, subWindow->Resize(200, 200));
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: GetAvoidAreaByType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, GetAvoidAreaByType, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetAvoidAreaByType");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    AvoidArea avoidArea;
    auto ret = windowSceneSessionImpl->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidArea);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->hostSession_ = nullptr;
    ret = windowSceneSessionImpl->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT, avoidArea);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: IsLayoutFullScreen
 * @tc.desc: IsLayoutFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, IsLayoutFullScreen, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsLayoutFullScreen");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    auto ret = windowSceneSessionImpl->IsLayoutFullScreen();
    EXPECT_EQ(false, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = windowSceneSessionImpl->IsLayoutFullScreen();
    EXPECT_EQ(false, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ret = windowSceneSessionImpl->IsLayoutFullScreen();
    EXPECT_EQ(false, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    ret = windowSceneSessionImpl->IsLayoutFullScreen();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: IsLayoutFullScreen
 * @tc.desc: IsLayoutFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, IsLayoutFullScreen002, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsLayoutFullScreen002");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->property_->SetWindowState(WindowState::STATE_SHOWN);
    auto ret = windowSceneSessionImpl->IsLayoutFullScreen();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: MaximizeFloating
 * @tc.desc: MaximizeFloating
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, MaximizeFloating, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MaximizeFloating");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    auto ret = windowSceneSessionImpl->MaximizeFloating();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->SetGlobalMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
    ret = windowSceneSessionImpl->MaximizeFloating();
    EXPECT_EQ(WMError::WM_OK, ret);
    auto ret1 = windowSceneSessionImpl->GetGlobalMaximizeMode();
    EXPECT_EQ(MaximizeMode::MODE_RECOVER, ret1);
    windowSceneSessionImpl->SetGlobalMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    ret = windowSceneSessionImpl->MaximizeFloating();
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetWindowModeSupportType(WINDOW_MODE_SUPPORT_FLOATING);
    ret = windowSceneSessionImpl->MaximizeFloating();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    ret1 = windowSceneSessionImpl->GetGlobalMaximizeMode();
    EXPECT_EQ(MaximizeMode::MODE_RECOVER, ret1);
    windowSceneSessionImpl->hostSession_ = nullptr;
    ret1 = windowSceneSessionImpl->GetGlobalMaximizeMode();
    EXPECT_EQ(MaximizeMode::MODE_RECOVER, ret1);
}

/**
 * @tc.name: MaximizeFloating01
 * @tc.desc: MaximizeFloating
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, MaximizeFloating01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MaximizeFloating01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto ret = window->MaximizeFloating();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->property_->SetPersistentId(1);
    window->hostSession_ = session;
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetDisableFullScreen(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    ret = window->MaximizeFloating();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    compatibleModeProperty->SetDisableFullScreen(false);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_CALL(*(session), GetGlobalMaximizeMode(_)).WillRepeatedly(DoAll(
        SetArgReferee<0>(MaximizeMode::MODE_AVOID_SYSTEM_BAR),
        Return(WSError::WS_OK)
    ));
    EXPECT_EQ(MaximizeMode::MODE_AVOID_SYSTEM_BAR, window->GetGlobalMaximizeMode());
    ret = window->MaximizeFloating();
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: MaximizeFloating03
 * @tc.desc: MaximizeFloating03
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, MaximizeFloating03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MaximizeFloating");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    auto ret = windowSceneSessionImpl->MaximizeFloating();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->SetGlobalMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
    ret = windowSceneSessionImpl->MaximizeFloating();
    EXPECT_EQ(WMError::WM_OK, ret);
    auto ret1 = windowSceneSessionImpl->GetGlobalMaximizeMode();
    EXPECT_EQ(MaximizeMode::MODE_RECOVER, ret1);
    windowSceneSessionImpl->SetGlobalMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    ret = windowSceneSessionImpl->MaximizeFloating();
    EXPECT_EQ(WMError::WM_OK, ret);
    SystemBarProperty statusProperty =
        windowSceneSessionImpl->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    SystemBarProperty navigationIndicatorPorperty =
        windowSceneSessionImpl->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR);
    statusProperty.enable_ = false;
    navigationIndicatorPorperty.enable_ = false;
    ret = windowSceneSessionImpl->MaximizeFloating();
    EXPECT_EQ(WMError::WM_OK, ret);
    statusProperty.enable_ = true;
    navigationIndicatorPorperty.enable_ = true;
    ret = windowSceneSessionImpl->MaximizeFloating();
    EXPECT_EQ(WMError::WM_OK, ret);
    statusProperty.enable_ = true;
    navigationIndicatorPorperty.enable_ = false;
    ret = windowSceneSessionImpl->MaximizeFloating();
    EXPECT_EQ(WMError::WM_OK, ret);
    statusProperty.enable_ = false;
    navigationIndicatorPorperty.enable_ = true;
    ret = windowSceneSessionImpl->MaximizeFloating();
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: Recover
 * @tc.desc: Recover
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, Recover, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Recover");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSessionImpl->Recover());

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetMaximizeMode(MaximizeMode::MODE_RECOVER);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    auto ret = windowSceneSessionImpl->Recover();
    EXPECT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, ret);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    ret = windowSceneSessionImpl->Recover();
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    ret = windowSceneSessionImpl->Recover();
    windowSceneSessionImpl->enableImmersiveMode_ = true;
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ret = windowSceneSessionImpl->Recover();
    EXPECT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = windowSceneSessionImpl->Recover();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
}

/**
 * @tc.name: RestorePcMainWindow
 * @tc.desc: RestorePcMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, RestorePcMainWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Restore");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, windowSceneSessionImpl->Restore());
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(false);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(false);
    auto ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_OK, ret);

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(false);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(true);
    ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RestorePcSubWindow
 * @tc.desc: RestorePcSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, RestorePcSubWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Restore");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(false);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(false);
    auto ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(false);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(true);
    ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);
}

/**
 * @tc.name: RestorePadMainWindow
 * @tc.desc: RestorePadMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, RestorePadMainWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Restore");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(false);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(false);
    auto ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(false);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(true);
    ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(true);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(false);
    ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RestorePadSubWindow
 * @tc.desc: RestorePadSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, RestorePadSubWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Restore");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(false);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(false);
    auto ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(false);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(true);
    ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(true);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(false);
    ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);
}

/**
 * @tc.name: RestorePadFloatingMainWindow
 * @tc.desc: RestorePadFloatingMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, RestorePadFloatingMainWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Restore");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(false);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(false);
    auto ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(false);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(true);
    ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(true);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(false);
    ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RestorePadFloatingSubWindow
 * @tc.desc: RestorePadFloatingSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, RestorePadFloatingSubWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Restore");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(false);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(false);
    auto ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(false);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(true);
    ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(true);
    windowSceneSessionImpl->property_->SetIsAppSupportPhoneInPc(false);
    ret = windowSceneSessionImpl->Restore();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);
}

/**
 * @tc.name: StartMove
 * @tc.desc: StartMove
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, StartMove, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("StartMove");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->hostSession_ = nullptr;
    windowSceneSessionImpl->StartMove();
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->windowSystemConfig_.freeMultiWindowSupport_ = true;
    ASSERT_NE(nullptr, windowSceneSessionImpl->property_);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    windowSceneSessionImpl->StartMove();
}

/**
 * @tc.name: IsStartMoving01
 * @tc.desc: get main window move flag, test IsStartMoving
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, IsStartMoving01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsStartMoving");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    bool isMoving = windowSceneSessionImpl->IsStartMoving();
    EXPECT_EQ(false, isMoving);
}

/**
 * @tc.name: DisableAppWindowDecor
 * @tc.desc: DisableAppWindowDecor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, DisableAppWindowDecor, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DisableAppWindowDecor");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;

    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    auto ret = windowSceneSessionImpl->DisableAppWindowDecor();
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = windowSceneSessionImpl->DisableAppWindowDecor();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
}

/**
 * @tc.name: UpdateMaximizeMode
 * @tc.desc: UpdateMaximizeMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, UpdateMaximizeMode, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateMaximizeMode");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->enableImmersiveMode_ = true;
    windowSceneSessionImpl->uiContent_ = nullptr;
    auto ret = windowSceneSessionImpl->UpdateMaximizeMode(MaximizeMode::MODE_RECOVER);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);

    windowSceneSessionImpl->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ret = windowSceneSessionImpl->UpdateMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_TRUE(windowSceneSessionImpl->enableImmersiveMode_);

    ret = windowSceneSessionImpl->UpdateMaximizeMode(MaximizeMode::MODE_RECOVER);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_FALSE(windowSceneSessionImpl->enableImmersiveMode_);

    ret = windowSceneSessionImpl->UpdateMaximizeMode(MaximizeMode::MODE_RECOVER);
    EXPECT_EQ(WSError::WS_OK, ret);

    windowSceneSessionImpl->enableImmersiveMode_ = true;
    windowSceneSessionImpl->hostSession_ = nullptr;
    ret = windowSceneSessionImpl->UpdateMaximizeMode(MaximizeMode::MODE_RECOVER);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: GetSystemBarProperties
 * @tc.desc: GetSystemBarProperties
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, GetSystemBarProperties, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSystemBarProperties");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::map<WindowType, SystemBarProperty> properties;
    auto ret = windowSceneSessionImpl->GetSystemBarProperties(properties);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: Hide
 * @tc.desc: Hide
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, Hide, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Hide");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->hostSession_ = nullptr;
    auto ret = windowSceneSessionImpl->Hide(2, false, false);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->property_->SetPersistentId(0);
    ret = windowSceneSessionImpl->Hide(2, false, false);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = windowSceneSessionImpl->Hide(2, false, false);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: Hide
 * @tc.desc: reason = WindowStateChangeReason::USER_SWITCH
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, Hide002, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Hide002");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->hostSession_ = nullptr;
    auto reason = static_cast<uint32_t>(WindowStateChangeReason::USER_SWITCH);
    auto ret = windowSceneSessionImpl->Hide(reason, false, false);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: IsSessionMainWindow
 * @tc.desc: IsSessionMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, IsSessionMainWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsSessionMainWindow");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(option);
    windowSession->property_->SetPersistentId(1);
    windowSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->windowSessionMap_.insert(std::make_pair("window1", std::make_pair(1, windowSession)));
    auto ret = windowSceneSessionImpl->IsSessionMainWindow(1);
    EXPECT_EQ(true, ret);
    windowSceneSessionImpl->windowSessionMap_.insert(std::make_pair("window1", std::make_pair(1, windowSession)));
    windowSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = windowSceneSessionImpl->IsSessionMainWindow(1);
    EXPECT_EQ(false, ret);
    ret = windowSceneSessionImpl->IsSessionMainWindow(0);
    EXPECT_EQ(false, ret);
    windowSession = nullptr;
    ret = windowSceneSessionImpl->IsSessionMainWindow(0);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: Show
 * @tc.desc: Show
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, Show, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Show");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    auto ret = windowSceneSessionImpl->Show(2, true);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = windowSceneSessionImpl->Maximize();
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = windowSceneSessionImpl->Minimize();
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = windowSceneSessionImpl->Minimize();
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);
    ret = windowSceneSessionImpl->Maximize();
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = windowSceneSessionImpl->Minimize();
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: Show
 * @tc.desc: Show01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, Show01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Show01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    auto ret = windowSceneSessionImpl->Show(2, true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: Show02
 * @tc.desc: Show  withFocus params
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, Show02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Show02");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    auto ret = windowSceneSessionImpl->Show(0, false, true);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = windowSceneSessionImpl->Show(0, false, false);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: Show02
 * @tc.desc: Show  withFocus params
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, Show03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Show03");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->property_->SetWindowState(WindowState::STATE_SHOWN);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    auto reason = static_cast<uint32_t>(WindowStateChangeReason::USER_SWITCH);
    auto ret = windowSceneSessionImpl->Show(reason, false, true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: NotifySpecificWindowSessionProperty
 * @tc.desc: NotifySpecificWindowSessionProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, NotifySpecificWindowSessionProperty, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySpecificWindowSessionProperty");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    WindowType type = WindowType::WINDOW_TYPE_STATUS_BAR;
    SystemBarProperty property;
    windowSceneSessionImpl->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    auto ret = windowSceneSessionImpl->NotifySpecificWindowSessionProperty(type, property);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = windowSceneSessionImpl->SetWindowFlags(0);
    EXPECT_EQ(WMError::WM_OK, ret);
    type = WindowType::WINDOW_TYPE_NAVIGATION_BAR;
    ret = windowSceneSessionImpl->NotifySpecificWindowSessionProperty(type, property);
    EXPECT_EQ(WMError::WM_OK, ret);
    type = WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR;
    ret = windowSceneSessionImpl->NotifySpecificWindowSessionProperty(type, property);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = windowSceneSessionImpl->SetSpecificBarProperty(type, property);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->hostSession_ = nullptr;
    ret = windowSceneSessionImpl->SetWindowFlags(0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: Close
 * @tc.desc: Close
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, Close, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Close");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSessionImpl->hostSession_ = nullptr;
    auto ret = windowSceneSessionImpl->Close();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->context_ = abilityContext_;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ret = windowSceneSessionImpl->Close();
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ret = windowSceneSessionImpl->Close();
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    ret = windowSceneSessionImpl->Close();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ret = windowSceneSessionImpl->Close();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: GetWindowWithId
 * @tc.desc: GetWindowWithId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, GetWindowWithId, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowWithId");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(option);
    windowSession->property_->SetPersistentId(1);
    windowSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->windowSessionMap_.insert(std::make_pair("window1", std::make_pair(1, windowSession)));
    auto ret = windowSceneSessionImpl->GetWindowWithId(1);
    EXPECT_FALSE(ret == nullptr);
    ret = windowSceneSessionImpl->GetWindowWithId(0);
    EXPECT_FALSE(ret == nullptr);
    windowSession = nullptr;
    ret = windowSceneSessionImpl->GetWindowWithId(0);
    EXPECT_FALSE(ret == nullptr);
}

/**
 * @tc.name: GetMainWindowWithId
 * @tc.desc: GetMainWindowWithId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, GetMainWindowWithId, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetMainWindowWithId");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(option);
    windowSession->property_->SetPersistentId(1);
    windowSceneSessionImpl->windowSessionMap_.insert(std::make_pair("window1", std::make_pair(1, windowSession)));
    windowSession->property_->SetWindowType(WindowType::WINDOW_TYPE_MEDIA);
    auto ret = windowSceneSessionImpl->GetMainWindowWithId(1);
    EXPECT_TRUE(ret == nullptr);
    windowSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ret = windowSceneSessionImpl->GetMainWindowWithId(1);
    EXPECT_TRUE(ret == nullptr);
    ret = windowSceneSessionImpl->GetMainWindowWithId(0);
    EXPECT_TRUE(ret == nullptr);
}

/**
 * @tc.name: PreNotifyKeyEvent
 * @tc.desc: PreNotifyKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, PreNotifyKeyEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("PreNotifyKeyEvent");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    windowSceneSessionImpl->uiContent_ = nullptr;
    auto ret = windowSceneSessionImpl->PreNotifyKeyEvent(keyEvent);
    EXPECT_EQ(false, ret);
    windowSceneSessionImpl->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: Recover01
 * @tc.desc: Recover
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, Recover01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("Recover");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    windowSceneSessionImpl->hostSession_ = nullptr;
    auto ret = windowSceneSessionImpl->Recover(1);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = windowSceneSessionImpl->Recover(1);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    windowSceneSessionImpl->windowSystemConfig_.freeMultiWindowEnable_ = true;
    ret = windowSceneSessionImpl->Recover(1);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    windowSceneSessionImpl->windowSystemConfig_.freeMultiWindowSupport_ = true;
    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetMaximizeMode(MaximizeMode::MODE_RECOVER);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ret = windowSceneSessionImpl->Recover(1);
    EXPECT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, ret);
    windowSceneSessionImpl->property_->SetMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
    ret = windowSceneSessionImpl->Recover(1);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    ret = windowSceneSessionImpl->Recover(1);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = windowSceneSessionImpl->Recover(0);
    EXPECT_EQ(WMError::WM_ERROR_REPEAT_OPERATION, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = windowSceneSessionImpl->Recover(0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    windowSceneSessionImpl->property_->SetWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN);
    ret = windowSceneSessionImpl->Recover(0);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetPcAppInpadCompatibleMode(true);
    windowSceneSessionImpl->windowSystemConfig_.freeMultiWindowEnable_ = false;
    ret = windowSceneSessionImpl->Recover(0);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: FindParentSessionByParentId
 * @tc.desc: FindParentSessionByParentId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, FindParentSessionByParentId, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("FindParentSessionByParentId");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(option);
    windowSession->property_->SetPersistentId(1);

    windowSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->windowSessionMap_.insert(std::make_pair("window1", std::make_pair(1, windowSession)));
    EXPECT_FALSE(nullptr != windowSceneSessionImpl->FindParentSessionByParentId(1));
    windowSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSession->property_->SetParentPersistentId(1);
    EXPECT_FALSE(nullptr != windowSceneSessionImpl->FindParentSessionByParentId(1));
    windowSession->property_->SetParentPersistentId(0);
    windowSession->property_->SetIsUIExtFirstSubWindow(true);
    EXPECT_FALSE(nullptr != windowSceneSessionImpl->FindParentSessionByParentId(1));
}

/**
 * @tc.name: PreLayoutOnShow
 * @tc.desc: PreLayoutOnShow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, PreLayoutOnShow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("PreLayoutOnShow");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    window->PreLayoutOnShow(window->property_->GetWindowType()); // uicontent is nullptr
    ASSERT_EQ(WindowState::STATE_INITIAL, window->state_);
    ASSERT_EQ(WindowState::STATE_INITIAL, window->requestState_);

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Rect request = { 100, 100, 100, 100 };
    window->property_->SetRequestRect(request);
    // uicontent is not nullptr and session is nullptr
    window->PreLayoutOnShow(window->property_->GetWindowType());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->state_);
    ASSERT_EQ(WindowState::STATE_SHOWN, window->requestState_);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    // uicontent is not nullptr and session is not nullptr
    window->PreLayoutOnShow(window->property_->GetWindowType());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->state_);
    ASSERT_EQ(WindowState::STATE_SHOWN, window->requestState_);

    request = { 100, 100, 0, 100 };
    window->property_->SetRequestRect(request);
    window->PreLayoutOnShow(window->property_->GetWindowType());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->state_);
    ASSERT_EQ(WindowState::STATE_SHOWN, window->requestState_);

    request = { 100, 100, 100, 0 };
    window->property_->SetRequestRect(request);
    window->PreLayoutOnShow(window->property_->GetWindowType());
    ASSERT_EQ(WindowState::STATE_SHOWN, window->state_);
    ASSERT_EQ(WindowState::STATE_SHOWN, window->requestState_);
}

/**
 * @tc.name: InitSystemSessionDragEnable_IsSubOrNot
 * @tc.desc: InitSystemSessionDragEnable Test, is Sub window or not
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, InitSystemSessionDragEnable_IsSubOrNot, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->property_->SetDragEnabled(true);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->InitSystemSessionDragEnable();
    ASSERT_EQ(window->property_->GetDragEnabled(), false);

    window->property_->SetDragEnabled(true);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    window->InitSystemSessionDragEnable();
    ASSERT_EQ(window->property_->GetDragEnabled(), true);
}

/**
 * @tc.name: InitSystemSessionDragEnable_IsDialogOrNot
 * @tc.desc: InitSystemSessionDragEnable Test, is dialog window or not
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, InitSystemSessionDragEnable_IsDialogOrNot, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("InitSystemSessionDragEnable_IsDialogOrNot");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->property_->SetDragEnabled(true);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    window->InitSystemSessionDragEnable();
    ASSERT_EQ(window->property_->GetDragEnabled(), false);

    window->property_->SetDragEnabled(true);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    window->InitSystemSessionDragEnable();
    ASSERT_EQ(window->property_->GetDragEnabled(), true);
}

/**
 * @tc.name: SetWindowRectAutoSave
 * @tc.desc: SetWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, SetWindowRectAutoSave, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSceneSessionImplTest3: SetWindowRectAutoSave start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowRectAutoSave");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto ret = windowSceneSessionImpl->SetWindowRectAutoSave(true, false);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    ret = windowSceneSessionImpl->SetWindowRectAutoSave(true, false);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = windowSceneSessionImpl->SetWindowRectAutoSave(false, false);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ret = windowSceneSessionImpl->SetWindowRectAutoSave(true, false);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);
    GTEST_LOG_(INFO) << "WindowSceneSessionImplTest3: SetWindowRectAutoSave end";
}

/**
 * @tc.name: SetSupportedWindowModes
 * @tc.desc: SetSupportedWindowModes
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, SetSupportedWindowModes, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSupportedWindowModes");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes = { AppExecFwk::SupportWindowMode::FULLSCREEN,
                                                                      AppExecFwk::SupportWindowMode::SPLIT,
                                                                      AppExecFwk::SupportWindowMode::FLOATING };

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    auto ret = windowSceneSessionImpl->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ret = windowSceneSessionImpl->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);
    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = windowSceneSessionImpl->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, ret);

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    supportedWindowModes.clear();
    ret = windowSceneSessionImpl->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::FULLSCREEN);
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::SPLIT);
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::FLOATING);
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::FLOATING);
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::FLOATING);
    ret = windowSceneSessionImpl->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    supportedWindowModes.clear();
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::FULLSCREEN);
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::SPLIT);
    ret = windowSceneSessionImpl->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_OK, ret);

    supportedWindowModes.clear();
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::FLOATING);
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::SPLIT);
    ret = windowSceneSessionImpl->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_OK, ret);

    windowSceneSessionImpl->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    windowSceneSessionImpl->property_->SetPcAppInpadCompatibleMode(true);
    windowSceneSessionImpl->windowSystemConfig_.freeMultiWindowEnable_ = false;
    ret = windowSceneSessionImpl->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetSupportedWindowModes01
 * @tc.desc: SetSupportedWindowModes
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, SetSupportedWindowModes01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSupportedWindowModes01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes;
    auto ret = window->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = window->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    supportedWindowModes.push_back(static_cast<AppExecFwk::SupportWindowMode>(10));
    ret = window->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    supportedWindowModes.clear();
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::SPLIT);
    ret = window->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: SetSupportedWindowModes02
 * @tc.desc: SetSupportedWindowModes
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, SetSupportedWindowModes02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSupportedWindowModes02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes;
    auto ret = window->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = window->SetSupportedWindowModes(supportedWindowModes, true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    supportedWindowModes.push_back(static_cast<AppExecFwk::SupportWindowMode>(10));
    ret = window->SetSupportedWindowModes(supportedWindowModes, true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    supportedWindowModes.clear();
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::SPLIT);
    ret = window->SetSupportedWindowModes(supportedWindowModes, true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);

    supportedWindowModes.clear();
    ret = window->SetSupportedWindowModes(supportedWindowModes, true);
    EXPECT_EQ(WMError::WM_ERROR_ILLEGAL_PARAM, ret);

    supportedWindowModes.clear();
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::FULLSCREEN);
    ret = window->SetSupportedWindowModes(supportedWindowModes, true);
    EXPECT_EQ(WMError::WM_ERROR_ILLEGAL_PARAM, ret);
    
    supportedWindowModes.clear();
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::SPLIT);
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::FLOATING);
    ret = window->SetSupportedWindowModes(supportedWindowModes, true);
    EXPECT_EQ(WMError::WM_OK, ret);

    supportedWindowModes.clear();
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::SPLIT);
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::FLOATING);
    window->grayOutMaximizeButton_ = true;
    ret = window->SetSupportedWindowModes(supportedWindowModes, false);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetSupportedWindowModes04
 * @tc.desc: SetSupportedWindowModes
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, SetSupportedWindowModes04, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetSupportedWindowModes04");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes;
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::FULLSCREEN);
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::SPLIT);
    supportedWindowModes.push_back(AppExecFwk::SupportWindowMode::FLOATING);

    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    auto ret = window->SetSupportedWindowModes(supportedWindowModes);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GrayOutMaximizeButton
 * @tc.desc: GrayOutMaximizeButton
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, GrayOutMaximizeButton, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GrayOutMaximizeButton");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto ret = window->GrayOutMaximizeButton(true);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    std::unique_ptr<Ace::UIContent> uiContent = Ace::UIContent::Create(nullptr, nullptr);
    window->uiContent_ = std::move(uiContent);
    ret = window->GrayOutMaximizeButton(true);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = window->GrayOutMaximizeButton(true);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: CheckAndModifyWindowRect
 * @tc.desc: CheckAndModifyWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplTest3, CheckAndModifyWindowRect, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CheckAndModifyWindowRect");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    uint32_t width = 0;
    uint32_t height = 0;
    auto ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    width = 100;
    height = 100;
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    windowSceneSessionImpl->property_->SetIsPcAppInPad(true);
    windowSceneSessionImpl->property_->SetWindowModeSupportType(WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN);
    windowSceneSessionImpl->property_->SetDragEnabled(false);
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSceneSessionImpl->property_->SetDragEnabled(true);
    WindowLimits windowLimits = {5000, 5000, 50, 50, 0.0f, 0.0f};
    windowSceneSessionImpl->property_->SetWindowLimits(windowLimits);
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    EXPECT_EQ(WMError::WM_OK, ret);
    WindowLimits windowLimits1 = {800, 800, 50, 50, 0.0f, 0.0f};
    windowSceneSessionImpl->property_->SetWindowLimits(windowLimits1);
    ret = windowSceneSessionImpl->CheckAndModifyWindowRect(width, height);
    if (!windowSceneSessionImpl->IsFreeMultiWindowMode()) {
        EXPECT_EQ(WMError::WM_OK, ret);
    } else {
        EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
    }
}
} // namespace
} // namespace Rosen
} // namespace OHOS
