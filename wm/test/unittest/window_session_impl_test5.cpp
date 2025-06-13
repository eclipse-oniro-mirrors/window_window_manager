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

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "ability_context_impl.h"
#include "color_parser.h"
#include "mock_session.h"
#include "mock_session_stub.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "parameters.h"
#include "scene_board_judgement.h"
#include "window_helper.h"
#include "window_session_impl.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowSessionImplTest5 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowSessionImplTest5::SetUpTestCase() {}

void WindowSessionImplTest5::TearDownTestCase() {}

void WindowSessionImplTest5::SetUp()
{
}

void WindowSessionImplTest5::TearDown()
{
}

namespace {
/**
 * @tc.name: GetSubWindows
 * @tc.desc: GetSubWindows
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetSubWindows, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: GetSubWindows start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSubWindows");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    std::vector<sptr<WindowSessionImpl>> subWindows;
    window->GetSubWindows(1, subWindows);
    EXPECT_EQ(subWindows.size(), 0);

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("GetSubWindows_subWindow");
    sptr<WindowSessionImpl> subWindow = sptr<WindowSessionImpl>::MakeSptr(subWindowOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(1);
    subWindow->hostSession_ = session;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, { subWindow }));

    subWindows.clear();
    window->GetSubWindows(1, subWindows);
    ASSERT_EQ(subWindows.size(), 1);
    EXPECT_EQ(subWindows[0], subWindow);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: GetSubWindows end";
}

/**
 * @tc.name: RemoveSubWindow
 * @tc.desc: RemoveSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, RemoveSubWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RemoveSubWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("RemoveSubWindow_subWindow");
    sptr<WindowSessionImpl> subWindow = sptr<WindowSessionImpl>::MakeSptr(subWindowOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(1);
    subWindow->hostSession_ = session;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, { subWindow }));

    sptr<WindowOption> subWindowOption2 = sptr<WindowOption>::MakeSptr();
    subWindowOption2->SetWindowName("RemoveSubWindow_subWindow2");
    sptr<WindowSessionImpl> subWindow2 = sptr<WindowSessionImpl>::MakeSptr(subWindowOption2);
    subWindow2->property_->SetPersistentId(3);
    subWindow2->property_->SetParentPersistentId(1);
    subWindow2->hostSession_ = session;
    subWindow2->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow2->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WindowSessionImpl::subWindowSessionMap_[1].push_back(subWindow2);

    subWindow->RemoveSubWindow(2);
    std::vector<sptr<WindowSessionImpl>> subWindows;
    window->GetSubWindows(1, subWindows);
    ASSERT_EQ(subWindows.size(), 2);
    EXPECT_EQ(subWindows[0], subWindow);
    subWindow->RemoveSubWindow(1);
    subWindows.clear();
    window->GetSubWindows(1, subWindows);
    ASSERT_EQ(subWindows.size(), 1);
    EXPECT_NE(subWindows[0], subWindow);
}

/**
 * @tc.name: DestroySubWindow01
 * @tc.desc: DestroySubWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, DestroySubWindow01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("DestroySubWindow01");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);

    sptr<WindowOption> subOption01 = sptr<WindowOption>::MakeSptr();
    subOption01->SetWindowName("DestroySubWindow01_subwindow01");
    subOption01->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow01 = sptr<WindowSessionImpl>::MakeSptr(subOption01);
    subWindow01->property_->SetPersistentId(2);
    subWindow01->property_->SetParentPersistentId(1);
    SessionInfo sessionInfo1 = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session1 = sptr<SessionMocker>::MakeSptr(sessionInfo1);
    subWindow01->hostSession_ = session1;
    subWindow01->state_ = WindowState::STATE_CREATED;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, { subWindow01 }));

    sptr<WindowOption> subOption02 = sptr<WindowOption>::MakeSptr();
    subOption02->SetWindowName("DestroySubWindow01_subwindow02");
    subOption02->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow02 = sptr<WindowSessionImpl>::MakeSptr(subOption01);
    subWindow02->property_->SetPersistentId(3);
    subWindow02->property_->SetParentPersistentId(2);
    SessionInfo sessionInfo2 = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session2 = sptr<SessionMocker>::MakeSptr(sessionInfo2);
    subWindow02->hostSession_ = session2;
    subWindow02->state_ = WindowState::STATE_CREATED;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(2, { subWindow02 }));

    std::vector<sptr<WindowSessionImpl>> subWindows;
    window->GetSubWindows(1, subWindows);
    EXPECT_EQ(subWindows.size(), 1);
    subWindows.clear();
    window->GetSubWindows(2, subWindows);
    EXPECT_EQ(subWindows.size(), 1);
    window->DestroySubWindow();
    EXPECT_EQ(subWindow01->state_, WindowState::STATE_DESTROYED);
    EXPECT_EQ(subWindow02->state_, WindowState::STATE_DESTROYED);
    EXPECT_EQ(subWindow01->hostSession_, nullptr);
    EXPECT_EQ(subWindow02->hostSession_, nullptr);
    subWindows.clear();
    window->GetSubWindows(1, subWindows);
    EXPECT_EQ(subWindows.size(), 0);
    subWindows.clear();
    window->GetSubWindows(2, subWindows);
    EXPECT_EQ(subWindows.size(), 0);
}

/**
 * @tc.name: SetUniqueVirtualPixelRatioForSub
 * @tc.desc: SetUniqueVirtualPixelRatioForSub test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetUniqueVirtualPixelRatioForSub, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetUniqueVirtualPixelRatioForSub");
    option->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);

    sptr<WindowOption> subOption01 = sptr<WindowOption>::MakeSptr();
    subOption01->SetWindowName("SetUniqueVirtualPixelRatioForSub_subwindow01");
    subOption01->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow01 = sptr<WindowSessionImpl>::MakeSptr(subOption01);
    subWindow01->property_->SetPersistentId(2);
    subWindow01->property_->SetParentPersistentId(1);
    subWindow01->useUniqueDensity_ = false;
    subWindow01->virtualPixelRatio_ = 0.5f;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, { subWindow01 }));

    sptr<WindowOption> subOption02 = sptr<WindowOption>::MakeSptr();
    subOption02->SetWindowName("SetUniqueVirtualPixelRatioForSub_subwindow02");
    subOption02->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<WindowSessionImpl> subWindow02 = sptr<WindowSessionImpl>::MakeSptr(subOption01);
    subWindow02->property_->SetPersistentId(3);
    subWindow02->property_->SetParentPersistentId(2);
    subWindow01->useUniqueDensity_ = false;
    subWindow01->virtualPixelRatio_ = 0.3f;
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(2, { subWindow02 }));

    bool useUniqueDensity = true;
    float virtualPixelRatio = 1.0f;
    window->SetUniqueVirtualPixelRatioForSub(useUniqueDensity, virtualPixelRatio);
    EXPECT_EQ(subWindow01->useUniqueDensity_, useUniqueDensity);
    EXPECT_EQ(subWindow02->useUniqueDensity_, useUniqueDensity);
    EXPECT_NEAR(subWindow01->virtualPixelRatio_, virtualPixelRatio, 0.00001f);
    EXPECT_NEAR(subWindow02->virtualPixelRatio_, virtualPixelRatio, 0.00001f);
}

/**
 * @tc.name: RegisterWindowRotationChangeListener
 * @tc.desc: RegisterWindowRotationChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, RegisterWindowRotationChangeListener, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterWindowRotationChangeListener");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IWindowRotationChangeListener> listener = nullptr;
    WMError ret = window->RegisterWindowRotationChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IWindowRotationChangeListener>::MakeSptr();
    std::vector<sptr<IWindowRotationChangeListener>> holder;
    window->windowRotationChangeListeners_[window->property_->GetPersistentId()] = holder;
    ret = window->RegisterWindowRotationChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
    holder = window->windowRotationChangeListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    ASSERT_NE(existsListener, holder.end());

    ret = window->RegisterWindowRotationChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: UnregisterWindowRotationChangeListener
 * @tc.desc: UnregisterWindowRotationChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UnregisterWindowRotationChangeListener, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UnregisterWindowRotationChangeListener");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IWindowRotationChangeListener> listener = nullptr;
    WMError ret = window->UnregisterWindowRotationChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IWindowRotationChangeListener>::MakeSptr();
    std::vector<sptr<IWindowRotationChangeListener>> holder;
    window->windowRotationChangeListeners_[window->property_->GetPersistentId()] = holder;
    window->RegisterWindowRotationChangeListener(listener);
    ret = window->UnregisterWindowRotationChangeListener(listener);
    EXPECT_EQ(ret, WMError::WM_OK);

    holder = window->windowRotationChangeListeners_[window->property_->GetPersistentId()];
    auto existsListener = std::find(holder.begin(), holder.end(), listener);
    EXPECT_EQ(existsListener, holder.end());
}

/**
 * @tc.name: CheckMultiWindowRect
 * @tc.desc: CheckMultiWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, CheckMultiWindowRect, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CheckMultiWindowRect");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    uint32_t width = 100;
    uint32_t height = 100;
    windowSessionImpl->property_->SetPersistentId(1);
    windowSessionImpl->hostSession_ = session;
    windowSessionImpl->state_ = WindowState::STATE_SHOWN;
    auto ret = windowSessionImpl->CheckMultiWindowRect(width, height);
    EXPECT_EQ(WMError::WM_OK, ret);

    windowSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    windowSessionImpl->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ret = windowSessionImpl->CheckMultiWindowRect(width, height);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: NotifyRotationChange
 * @tc.desc: NotifyRotationChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyRotationChange, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyRotationChange");
    option->SetWindowType(WindowType::APP_WINDOW_BASE);
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSessionImpl->property_->SetPersistentId(1);
    windowSessionImpl->hostSession_ = session;
    windowSessionImpl->state_ = WindowState::STATE_SHOWN;
    RotationChangeInfo info = { RotationChangeType::WINDOW_WILL_ROTATE, 0, 1, { 0, 0, 2720, 1270 } };
    RotationChangeResult res = windowSessionImpl->NotifyRotationChange(info);
    EXPECT_EQ(RectType::RELATIVE_TO_SCREEN, res.rectType_);

    sptr<IWindowRotationChangeListener> listener = sptr<IWindowRotationChangeListener>::MakeSptr();
    std::vector<sptr<IWindowRotationChangeListener>> holder;
    windowSessionImpl->windowRotationChangeListeners_[windowSessionImpl->property_->GetPersistentId()] = holder;
    WMError ret = windowSessionImpl->RegisterWindowRotationChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
    res = windowSessionImpl->NotifyRotationChange(info);
    EXPECT_EQ(RectType::RELATIVE_TO_SCREEN, res.rectType_);

    info.type_ = RotationChangeType::WINDOW_DID_ROTATE;
    res = windowSessionImpl->NotifyRotationChange(info);
    EXPECT_EQ(RectType::RELATIVE_TO_SCREEN, res.rectType_);
}

/**
 * @tc.name: UpdateRectForPageRotation
 * @tc.desc: UpdateRectForPageRotation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UpdateRectForPageRotation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UpdateRectForPageRotation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("UpdateRectForPageRotation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    Rect wmRect = { 0, 0, 0, 0 };
    Rect preRect = { 0, 0, 0, 0 };
    WindowSizeChangeReason wmReason = WindowSizeChangeReason::PAGE_ROTATION;
    std::shared_ptr<RSTransaction> rsTransaction;
    SceneAnimationConfig config { .rsTransaction_ = rsTransaction };
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    std::shared_ptr<AvoidArea> avoidArea = std::make_shared<AvoidArea>();
    avoidArea->topRect_ = { 1, 0, 0, 0 };
    avoidArea->leftRect_ = { 0, 1, 0, 0 };
    avoidArea->rightRect_ = { 0, 0, 1, 0 };
    avoidArea->bottomRect_ = { 0, 0, 0, 1 };
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM;
    avoidAreas[type] = *avoidArea;

    window->property_->SetWindowRect(preRect);
    window->postTaskDone_ = false;
    window->UpdateRectForPageRotation(wmRect, preRect, wmReason, config, avoidAreas);
    EXPECT_EQ(window->postTaskDone_, false);

    WSRect rect = { 0, 0, 50, 50 };
    SizeChangeReason reason = SizeChangeReason::PAGE_ROTATION;
    auto res = window->UpdateRect(rect, reason, config, avoidAreas);
    EXPECT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UpdateRectForPageRotation end";
}

/**
 * @tc.name: RegisterPreferredOrientationChangeListener
 * @tc.desc: RegisterPreferredOrientationChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, RegisterPreferredOrientationChangeListener, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: RegisterPreferredOrientationChangeListener start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("PreferredOrientationChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IPreferredOrientationChangeListener> listener = nullptr;
    WMError res = window->RegisterPreferredOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IPreferredOrientationChangeListener>::MakeSptr();
    sptr<IPreferredOrientationChangeListener> holder;
    window->preferredOrientationChangeListener_[window->property_->GetPersistentId()] = holder;
    res = window->RegisterPreferredOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);
    holder = window->preferredOrientationChangeListener_[window->property_->GetPersistentId()];
    EXPECT_EQ(holder, listener);

    // already registered
    res = window->RegisterPreferredOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: RegisterPreferredOrientationChangeListener end";
}

/**
 * @tc.name: UnregisterPreferredOrientationChangeListener
 * @tc.desc: UnregisterPreferredOrientationChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UnregisterPreferredOrientationChangeListener, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UnregisterPreferredOrientationChangeListener start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("PreferredOrientationChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IPreferredOrientationChangeListener> listener = nullptr;
    WMError res = window->UnregisterPreferredOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IPreferredOrientationChangeListener>::MakeSptr();
    sptr<IPreferredOrientationChangeListener> holder;
    window->preferredOrientationChangeListener_[window->property_->GetPersistentId()] = holder;
    window->RegisterPreferredOrientationChangeListener(listener);

    res = window->UnregisterPreferredOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);

    holder = window->preferredOrientationChangeListener_[window->property_->GetPersistentId()];
    EXPECT_NE(holder, listener);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UnregisterPreferredOrientationChangeListener end";
}

/**
 * @tc.name: NotifyPreferredOrientationChange
 * @tc.desc: NotifyPreferredOrientationChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyPreferredOrientationChange, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPreferredOrientationChange");
    option->SetWindowType(WindowType::APP_WINDOW_BASE);
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSessionImpl->property_->SetPersistentId(1);
    windowSessionImpl->hostSession_ = session;
    windowSessionImpl->state_ = WindowState::STATE_SHOWN;

    Orientation orientation = Orientation::USER_ROTATION_PORTRAIT;
    windowSessionImpl->NotifyPreferredOrientationChange(orientation);

    sptr<IPreferredOrientationChangeListener> listener = sptr<IPreferredOrientationChangeListener>::MakeSptr();
    sptr<IPreferredOrientationChangeListener> holder;
    windowSessionImpl->preferredOrientationChangeListener_[windowSessionImpl->property_->GetPersistentId()] = holder;
    WMError res = windowSessionImpl->RegisterPreferredOrientationChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: RegisterOrientationChangeListener
 * @tc.desc: RegisterOrientationChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, RegisterOrientationChangeListener, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: RegisterOrientationChangeListener start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OrientationChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IWindowOrientationChangeListener> listener = nullptr;
    WMError res = window->RegisterOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IWindowOrientationChangeListener>::MakeSptr();
    sptr<IWindowOrientationChangeListener> holder;
    window->windowOrientationChangeListener_[window->property_->GetPersistentId()] = holder;
    res = window->RegisterOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);
    holder = window->windowOrientationChangeListener_[window->property_->GetPersistentId()];
    EXPECT_EQ(holder, listener);

    // already registered
    res = window->RegisterOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: RegisterOrientationChangeListener end";
}

/**
 * @tc.name: UnregisterOrientationChangeListener
 * @tc.desc: UnregisterOrientationChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, UnregisterOrientationChangeListener, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UnregisterOrientationChangeListener start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OrientationChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<IWindowOrientationChangeListener> listener = nullptr;
    WMError res = window->UnregisterOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);

    listener = sptr<IWindowOrientationChangeListener>::MakeSptr();
    sptr<IWindowOrientationChangeListener> holder;
    window->windowOrientationChangeListener_[window->property_->GetPersistentId()] = holder;
    window->RegisterOrientationChangeListener(listener);

    res = window->UnregisterOrientationChangeListener(listener);
    EXPECT_EQ(res, WMError::WM_OK);

    holder = window->windowOrientationChangeListener_[window->property_->GetPersistentId()];
    EXPECT_NE(holder, listener);
    GTEST_LOG_(INFO) << "WindowSessionImplTest4: UnregisterOrientationChangeListener end";
}

/**
 * @tc.name: NotifyClientOrientationChange
 * @tc.desc: NotifyClientOrientationChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, NotifyClientOrientationChange, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyClientOrientationChange");
    option->SetWindowType(WindowType::APP_WINDOW_BASE);
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = {"CreateTestBundle", "CreateTestModule", "CreateTestAbility"};
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSessionImpl->property_->SetPersistentId(1);
    windowSessionImpl->hostSession_ = session;
    windowSessionImpl->state_ = WindowState::STATE_SHOWN;

    windowSessionImpl->NotifyClientOrientationChange();

    sptr<IWindowOrientationChangeListener> listener = sptr<IWindowOrientationChangeListener>::MakeSptr();
    sptr<IWindowOrientationChangeListener> holder;
    windowSessionImpl->windowOrientationChangeListener_[windowSessionImpl->property_->GetPersistentId()] = holder;
    WMError res = windowSessionImpl->RegisterOrientationChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: GetDisplayOrientationForRotation
 * @tc.desc: GetDisplayOrientationForRotation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetDisplayOrientationForRotation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: GetDisplayOrientationForRotation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("GetDisplayOrientationForRotation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    window->SetDisplayOrientationForRotation(DisplayOrientation::PORTRAIT);
    EXPECT_EQ(window->GetDisplayOrientationForRotation(), DisplayOrientation::PORTRAIT);
    window->SetDisplayOrientationForRotation(DisplayOrientation::UNKNOWN);
    EXPECT_EQ(window->GetDisplayOrientationForRotation(), DisplayOrientation::UNKNOWN);
    window->SetDisplayOrientationForRotation(DisplayOrientation::LANDSCAPE);
    EXPECT_EQ(window->GetDisplayOrientationForRotation(), DisplayOrientation::LANDSCAPE);
    window->SetDisplayOrientationForRotation(DisplayOrientation::PORTRAIT_INVERTED);
    EXPECT_EQ(window->GetDisplayOrientationForRotation(), DisplayOrientation::PORTRAIT_INVERTED);
    window->SetDisplayOrientationForRotation(DisplayOrientation::LANDSCAPE_INVERTED);
    EXPECT_EQ(window->GetDisplayOrientationForRotation(), DisplayOrientation::LANDSCAPE_INVERTED);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: GetDisplayOrientationForRotation end";
}

/**
 * @tc.name: GetRequestedOrientation
 * @tc.desc: GetRequestedOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetRequestedOrientation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: GetRequestedOrientation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("GetRequestedOrientation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;

    window->SetPreferredRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    EXPECT_EQ(window->GetRequestedOrientation(), Orientation::USER_ROTATION_PORTRAIT);
    window->SetPreferredRequestedOrientation(Orientation::VERTICAL);
    EXPECT_EQ(window->GetRequestedOrientation(), Orientation::VERTICAL);
    window->SetPreferredRequestedOrientation(Orientation::HORIZONTAL);
    EXPECT_EQ(window->GetRequestedOrientation(), Orientation::HORIZONTAL);
    window->SetPreferredRequestedOrientation(Orientation::SENSOR);
    EXPECT_EQ(window->GetRequestedOrientation(), Orientation::SENSOR);
    window->SetPreferredRequestedOrientation(Orientation::FOLLOW_DESKTOP);
    EXPECT_EQ(window->GetRequestedOrientation(), Orientation::FOLLOW_DESKTOP);
    GTEST_LOG_(INFO) << "WindowSessionImplTest5: GetRequestedOrientation end";
}

/**
 * @tc.name: SetFollowScreenChange
 * @tc.desc: SetFollowScreenChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetFollowScreenChange, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetFollowScreenChange");

    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(0);
    WMError ret = window->SetFollowScreenChange(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    ret = window->SetFollowScreenChange(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE, ret);

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    ret = window->SetFollowScreenChange(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE, ret);

    window->property_->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    ret = window->SetFollowScreenChange(true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetScaleWindow
 * @tc.desc: GetScaleWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetScaleWindow, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetScaleWindow_window1");
    sptr<WindowSessionImpl> mainWindow = sptr<WindowSessionImpl>::MakeSptr(option);
    int32_t id = 1;
    mainWindow->property_->SetPersistentId(id);
    mainWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    mainWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WindowSessionImpl::windowSessionMap_.clear();
    WindowSessionImpl::windowSessionMap_.insert(std::make_pair(mainWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(mainWindow->GetWindowId(), mainWindow)));
    auto res = mainWindow->GetScaleWindow(id);
    EXPECT_NE(res, nullptr);

    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    option2->SetWindowName("GetScaleWindow_extensionWindow");
    sptr<WindowSessionImpl> extensionWindow = sptr<WindowSessionImpl>::MakeSptr(option2);
    WindowSessionImpl::windowExtensionSessionSet_.clear();
    WindowSessionImpl::windowExtensionSessionSet_.insert(extensionWindow);
    extensionWindow->property_->SetPersistentId(2);
    extensionWindow->isUIExtensionAbilityProcess_ = true;
    int32_t testId = 3;
    res = mainWindow->GetScaleWindow(testId);
    EXPECT_EQ(res, nullptr);
    extensionWindow->property_->SetParentPersistentId(testId);
    extensionWindow->property_->SetParentId(testId);
    res = mainWindow->GetScaleWindow(testId);
    EXPECT_NE(res, nullptr);
    mainWindow->isFocused_ = true;
    extensionWindow->isUIExtensionAbilityProcess_ = false;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    mainWindow->hostSession_ = session;
    res = mainWindow->GetScaleWindow(testId);
    EXPECT_NE(res, nullptr);
    mainWindow->isFocused_ = false;
    res = mainWindow->GetScaleWindow(testId);
    EXPECT_EQ(res, nullptr);
    WindowSessionImpl::windowSessionMap_.clear();
    WindowSessionImpl::windowExtensionSessionSet_.clear();
}

/**
 * @tc.name: GetWindowScaleCoordinate01
 * @tc.desc: GetWindowScaleCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetWindowScaleCoordinate01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowScaleCoordinate01_mainWindow");
    sptr<WindowSessionImpl> mainWindow = sptr<WindowSessionImpl>::MakeSptr(option);
    int32_t id = 1;
    mainWindow->property_->SetPersistentId(id);
    mainWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    int32_t x = 100;
    int32_t y = 100;
    auto res = mainWindow->GetWindowScaleCoordinate(x, y, id);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    WindowSessionImpl::windowSessionMap_.clear();
    WindowSessionImpl::windowSessionMap_.insert(std::make_pair(mainWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(mainWindow->GetWindowId(), mainWindow)));
    res = mainWindow->GetWindowScaleCoordinate(x, y, id);
    EXPECT_EQ(res, WMError::WM_OK);

    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    mainWindow->property_->SetCompatibleModeProperty(compatibleModeProperty);
    res = mainWindow->GetWindowScaleCoordinate(x, y, id);
    EXPECT_EQ(res, WMError::WM_OK);

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("GetWindowScaleCoordinate01_subWindow");
    sptr<WindowSessionImpl> subWindow = sptr<WindowSessionImpl>::MakeSptr(subWindowOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(id);
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WindowSessionImpl::windowSessionMap_.insert(std::make_pair(subWindow->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(subWindow->GetWindowId(), subWindow)));
    mainWindow->context_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    subWindow->context_ = mainWindow->context_;
    subWindow->property_->SetIsUIExtensionAbilityProcess(true);
    res = mainWindow->GetWindowScaleCoordinate(x, y, id);
    EXPECT_EQ(res, WMError::WM_OK);
    subWindow->property_->SetIsUIExtensionAbilityProcess(false);
    res = mainWindow->GetWindowScaleCoordinate(x, y, id);
    EXPECT_EQ(res, WMError::WM_OK);
    WindowSessionImpl::windowSessionMap_.clear();
}

/**
 * @tc.name: GetWindowScaleCoordinate02
 * @tc.desc: GetWindowScaleCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetWindowScaleCoordinate02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowScaleCoordinate02_extensionWindow");
    sptr<WindowSessionImpl> extensionWindow = sptr<WindowSessionImpl>::MakeSptr(option);
    WindowSessionImpl::windowExtensionSessionSet_.clear();
    WindowSessionImpl::windowExtensionSessionSet_.insert(extensionWindow);
    extensionWindow->property_->SetPersistentId(2);
    extensionWindow->isUIExtensionAbilityProcess_ = true;
    extensionWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    int32_t id = 1;
    int32_t x = 100;
    int32_t y = 100;
    auto res = extensionWindow->GetWindowScaleCoordinate(x, y, id);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    extensionWindow->property_->SetParentPersistentId(id);
    extensionWindow->property_->SetParentId(id);
    res = extensionWindow->GetWindowScaleCoordinate(x, y, id);
    EXPECT_EQ(res, WMError::WM_OK);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    extensionWindow->property_->SetCompatibleModeProperty(compatibleModeProperty);
    res = extensionWindow->GetWindowScaleCoordinate(x, y, id);
    EXPECT_EQ(res, WMError::WM_OK);
    WindowSessionImpl::windowExtensionSessionSet_.clear();
}

/**
 * @tc.name: GetPropertyByContext
 * @tc.desc: GetPropertyByContext
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, GetPropertyByContext, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetPropertyByContext");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->context_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    int32_t persistentId = 663;
    window->property_->SetPersistentId(persistentId);
    window->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    auto property = window->GetPropertyByContext();
    EXPECT_EQ(property->GetPersistentId(), persistentId);

    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("GetPropertyByContext_mainWindow");
    sptr<WindowSessionImpl> mainWindow = sptr<WindowSessionImpl>::MakeSptr(option1);
    int32_t mainPersistentId = 666;
    mainWindow->property_->SetPersistentId(mainPersistentId);
    window->windowSessionMap_.insert({mainWindow->GetWindowName(),
        std::pair<int32_t, sptr<WindowSessionImpl>>(mainPersistentId, mainWindow) });
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    mainWindow->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property = window->GetPropertyByContext();
    EXPECT_EQ(property->GetPersistentId(), persistentId);
    mainWindow->context_ = window->context_;
    property = window->GetPropertyByContext();
    EXPECT_EQ(property->GetPersistentId(), mainPersistentId);

    window->property_->SetIsUIExtensionAbilityProcess(true);
    mainWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    property = window->GetPropertyByContext();
    EXPECT_EQ(property->GetPersistentId(), persistentId);
    window->windowExtensionSessionSet_.insert(mainWindow);
    property = window->GetPropertyByContext();
    EXPECT_EQ(property->GetPersistentId(), mainPersistentId);
}

/**
 * @tc.name: IsAdaptToSimulationScale
 * @tc.desc: IsAdaptToSimulationScale
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsAdaptToSimulationScale, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsAdaptToSimulationScale");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->context_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    window->property_->SetPersistentId(704);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSimulationScale(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_EQ(window->IsAdaptToSimulationScale(), true);

    sptr<WindowOption> option1 = sptr<WindowOption>::MakeSptr();
    option1->SetWindowName("IsAdaptToSimulationScale_mainWindow");
    sptr<WindowSessionImpl> mainWindow = sptr<WindowSessionImpl>::MakeSptr(option1);
    int32_t mainPersistentId = 666;
    mainWindow->property_->SetPersistentId(mainPersistentId);
    window->windowSessionMap_.insert({mainWindow->GetWindowName(),
        std::pair<int32_t, sptr<WindowSessionImpl>>(mainPersistentId, mainWindow) });
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    mainWindow->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    mainWindow->context_ = window->context_;
    EXPECT_EQ(window->IsAdaptToSimulationScale(), false);
}

/**
 * @tc.name: IsAdaptToSubWindow
 * @tc.desc: IsAdaptToSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, IsAdaptToSubWindow, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsAdaptToSubWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->context_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    window->property_->SetPersistentId(772);
    EXPECT_EQ(window->IsAdaptToSubWindow(), false);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToSubWindow(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_EQ(window->IsAdaptToSubWindow(), true);
}

/**
 * @tc.name: SetIntentParam
 * @tc.desc: SetIntentParam
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest5, SetIntentParam, Function | SmallTest | Level2)
{
sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetIntentParam");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    auto testCallback = [](){};
    bool isColdStart = true;
    std::string intentParam = "testIntent";
    window->SetIntentParam(intentParam, testCallback, isColdStart);
    EXPECT_EQ(window->isColdStart_, true);
    EXPECT_EQ(window->intentParam_, intentParam);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
