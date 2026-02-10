/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <algorithm>

#include "ability_context_impl.h"
#include "common_test_utils.h"
#include "extension_data_handler.h"
#include "iremote_object_mocker.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window_adapter.h"
#include "singleton_mocker.h"
#define private public
#define protected public
#include "window_session_impl.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using Mocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
uint32_t MaxWith = 32;

class WindowSessionImplRotationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<WindowSessionImpl> window_;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

protected:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
};

void WindowSessionImplRotationTest::SetUpTestCase() {}

void WindowSessionImplRotationTest::TearDownTestCase() {}

void WindowSessionImplRotationTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSessionImplRotationTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
    if (window_ != nullptr) {
        window_->Destroy();
    }
}

RSSurfaceNode::SharedPtr WindowSessionImplRotationTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "startingWindowTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    return surfaceNode;
}

namespace {
sptr<WindowSessionImpl> GetTestWindowImpl(const std::string& name)
{
    sptr<WindowOption> option = new (std::nothrow) WindowOption();
    if (option == nullptr) {
        return nullptr;
    }
    option->SetWindowName(name);
    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    if (window == nullptr) {
        return nullptr;
    }
    SessionInfo sessionInfo = { name, name, name };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    if (session == nullptr) {
        return nullptr;
    }
    window->hostSession_ = session;
    return window;
}

/**
 * @tc.name: UpdateRectForRotation
 * @tc.desc: UpdateRectForRotation Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, UpdateRectForRotation, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowSessionCreateCheck");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    Rect wmRect;
    wmRect.posX_ = 0;
    wmRect.posY_ = 0;
    wmRect.height_ = 50;
    wmRect.width_ = 50;

    WSRect rect;
    wmRect.posX_ = 0;
    wmRect.posY_ = 0;
    wmRect.height_ = 50;
    wmRect.width_ = 50;

    Rect preRect;
    preRect.posX_ = 0;
    preRect.posY_ = 0;
    preRect.height_ = 200;
    preRect.width_ = 200;

    window->property_->SetWindowRect(preRect);
    WindowSizeChangeReason wmReason = WindowSizeChangeReason{ 0 };
    std::shared_ptr<RSTransaction> rsTransaction;
    SceneAnimationConfig config = { rsTransaction, ROTATE_ANIMATION_DURATION,
        0, WindowAnimationCurve::LINEAR, {0.0f, 0.0f, 0.0f, 0.0f} };
    window->UpdateRectForRotation(wmRect, preRect, wmReason, config);

    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    auto res = window->UpdateRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: SetCurrentRotation
 * @tc.desc: SetCurrentRotation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, SetCurrentRotation, Function | SmallTest | Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetCurrentRotation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    auto ret = window->SetCurrentRotation(FULL_CIRCLE_DEGREE + 1);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);
    ret = window->SetCurrentRotation(ZERO_CIRCLE_DEGREE - 1);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);
    ret = window->SetCurrentRotation(ONE_FOURTH_FULL_CIRCLE_DEGREE);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: SetRequestedOrientation01
 * @tc.desc: SetRequestedOrientation01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, SetRequestedOrientation01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetRequestedOrientation01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetRequestedOrientation01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;

    Orientation ori = Orientation::VERTICAL;
    window->SetRequestedOrientation(ori);
    Orientation ret = window->property_->GetRequestedOrientation();
    ASSERT_EQ(ret, ori);

    window->SetRequestedOrientation(Orientation::AUTO_ROTATION_UNSPECIFIED);
    Orientation ret1 = window->property_->GetRequestedOrientation();
    ASSERT_EQ(ret1, Orientation::AUTO_ROTATION_UNSPECIFIED);

    window->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    Orientation ret2 = window->property_->GetRequestedOrientation();
    ASSERT_EQ(ret2, Orientation::USER_ROTATION_PORTRAIT);

    window->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE);
    Orientation ret3 = window->property_->GetRequestedOrientation();
    ASSERT_EQ(ret3, Orientation::USER_ROTATION_LANDSCAPE);

    window->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT_INVERTED);
    Orientation ret4 = window->property_->GetRequestedOrientation();
    ASSERT_EQ(ret4, Orientation::USER_ROTATION_PORTRAIT_INVERTED);

    window->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE_INVERTED);
    Orientation ret5 = window->property_->GetRequestedOrientation();
    ASSERT_EQ(ret5, Orientation::USER_ROTATION_LANDSCAPE_INVERTED);

    window->SetRequestedOrientation(Orientation::FOLLOW_DESKTOP);
    Orientation ret6 = window->property_->GetRequestedOrientation();
    ASSERT_EQ(ret6, Orientation::FOLLOW_DESKTOP);
    ASSERT_EQ(WMError::WM_OK, window->Destroy());
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetRequestedOrientation01 end";
}

/**
 * @tc.name: SetRequestedOrientation02
 * @tc.desc: SetRequestedOrientation02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, SetRequestedOrientation02, TestSize.Level1)
{
    auto window = GetTestWindowImpl("SetRequestedOrientation02");
    ASSERT_NE(window, nullptr);
    window->property_->SetRequestedOrientation(Orientation::BEGIN);
    window->SetRequestedOrientation(Orientation::END);

    window->property_->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    window->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);

    window->property_->SetRequestedOrientation(Orientation::BEGIN);
    window->SetRequestedOrientation(Orientation::BEGIN);
    window->Destroy();
}

/**
 * @tc.name: SetRequestedOrientation03
 * @tc.desc: SetRequestedOrientation03
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, SetRequestedOrientation03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetRequestedOrientation03 start";
    window_ = GetTestWindowImpl("SetRequestedOrientation03");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    Orientation orientation = Orientation::VERTICAL;
    window_->property_->requestedOrientation_ = Orientation::VERTICAL;
    window_->SetRequestedOrientation(orientation);
    orientation = Orientation::USER_ROTATION_PORTRAIT;
    window_->SetRequestedOrientation(orientation);
    auto ret = window_->property_->GetRequestedOrientation();
    ASSERT_EQ(ret, orientation);
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetRequestedOrientation03 end";
}

/**
 * @tc.name: GetRequestedOrientationtest01
 * @tc.desc: GetRequestedOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, GetRequestedOrientation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: GetRequestedOrientationtest01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();

    option->SetWindowName("GetRequestedOrientation");

    sptr<WindowSessionImpl> window = new (std::nothrow) WindowSessionImpl(option);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = new (std::nothrow) SessionMocker(sessionInfo);
    ASSERT_NE(nullptr, session);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));

    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;

    Orientation ori = Orientation::HORIZONTAL;
    window->SetUserRequestedOrientation(ori);
    Orientation ret = window->GetRequestedOrientation();
    ASSERT_EQ(ret, ori);

    window->SetUserRequestedOrientation(Orientation::AUTO_ROTATION_UNSPECIFIED);
    Orientation ret1 = window->GetRequestedOrientation();
    ASSERT_EQ(ret1, Orientation::AUTO_ROTATION_UNSPECIFIED);

    window->SetUserRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    Orientation ret2 = window->GetRequestedOrientation();
    ASSERT_EQ(ret2, Orientation::USER_ROTATION_PORTRAIT);

    window->SetUserRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE);
    Orientation ret3 = window->GetRequestedOrientation();
    ASSERT_EQ(ret3, Orientation::USER_ROTATION_LANDSCAPE);

    window->SetUserRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT_INVERTED);
    Orientation ret4 = window->GetRequestedOrientation();
    ASSERT_EQ(ret4, Orientation::USER_ROTATION_PORTRAIT_INVERTED);

    window->SetUserRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE_INVERTED);
    Orientation ret5 = window->GetRequestedOrientation();
    ASSERT_EQ(ret5, Orientation::USER_ROTATION_LANDSCAPE_INVERTED);

    window->SetUserRequestedOrientation(Orientation::FOLLOW_DESKTOP);
    Orientation ret6 = window->GetRequestedOrientation();
    ASSERT_EQ(ret6, Orientation::FOLLOW_DESKTOP);

    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: GetRequestedOrientationtest01 end";
}

/**
 * @tc.name: GetRequestedOrientation02
 * @tc.desc: GetRequestedOrientation02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, GetRequestedOrientation02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: GetRequestedOrientation02 start";
    window_ = GetTestWindowImpl("GetRequestedOrientation02");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(INVALID_SESSION_ID);
    auto ret = window_->GetRequestedOrientation();
    ASSERT_EQ(ret, Orientation::UNSPECIFIED);
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: GetRequestedOrientation02 end";
}

/**
 * @tc.name: UpdateRectForRotation02
 * @tc.desc: UpdateRectForRotation02 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, UpdateRectForRotation02, TestSize.Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateRectForRotation02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    auto runner = AppExecFwk::EventRunner::Create("UpdateRectForRotation02");
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    runner->Run();
    window->handler_ = handler;
 
    Rect wmRect;
    wmRect.posX_ = 0;
    wmRect.posY_ = 0;
    wmRect.height_ = 50;
    wmRect.width_ = 50;
 
    Rect preRect;
    preRect.posX_ = 0;
    preRect.posY_ = 0;
    preRect.height_ = 200;
    preRect.width_ = 200;
 
    window->property_->SetWindowRect(preRect);
    WindowSizeChangeReason wmReason = WindowSizeChangeReason::SNAPSHOT_ROTATION;
    std::shared_ptr<RSTransaction> rsTransaction;
    SceneAnimationConfig config = { rsTransaction, ROTATE_ANIMATION_DURATION,
        0, WindowAnimationCurve::LINEAR, {0.0f, 0.0f, 0.0f, 0.0f} };
    window->UpdateRectForRotation(wmRect, preRect, wmReason, config);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(window->lastSizeChangeReason_, WindowSizeChangeReason::ROTATION);
 
    preRect.height_ = 200;
    preRect.width_ = 200;
    window->property_->SetWindowRect(preRect);
    wmReason = WindowSizeChangeReason::ROTATION;
    window->UpdateRectForRotation(wmRect, preRect, wmReason, config);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(window->lastSizeChangeReason_, WindowSizeChangeReason::ROTATION);
}

/**
 * @tc.name: RegisterWindowRotationChangeListener
 * @tc.desc: RegisterWindowRotationChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, RegisterWindowRotationChangeListener, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionImplRotationTest, UnregisterWindowRotationChangeListener, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionImplRotationTest, CheckMultiWindowRect, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionImplRotationTest, NotifyRotationChange, Function | SmallTest | Level2)
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
} // namespace
} // namespace Rosen
} // namespace OHOS