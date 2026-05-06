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
#include "future_callback.h"
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
namespace {
    std::string g_logMsg;
    void LogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_logMsg += msg;
    }
}
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
 * @tc.name: SetCurrentRotation02
 * @tc.desc: Test SetCurrentRotation with boundary values and property verification
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, SetCurrentRotation02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetCurrentRotation02 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetCurrentRotation02");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    // Case 1: Boundary value - ZERO_CIRCLE_DEGREE (0)
    auto ret = window->SetCurrentRotation(ZERO_CIRCLE_DEGREE);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(window->property_->GetSessionInfo().currentRotation_, ZERO_CIRCLE_DEGREE);

    // Case 2: Boundary value - FULL_CIRCLE_DEGREE (360)
    ret = window->SetCurrentRotation(FULL_CIRCLE_DEGREE);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(window->property_->GetSessionInfo().currentRotation_, FULL_CIRCLE_DEGREE);

    // Case 3: Typical value - 180 degrees
    ret = window->SetCurrentRotation(180);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(window->property_->GetSessionInfo().currentRotation_, 180);

    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetCurrentRotation02 end";
}

/**
 * @tc.name: GetSceneNodeCount
 * @tc.desc: GetSceneNodeCount
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, GetSceneNodeCount, Function | SmallTest | Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: GetSceneNodeCount start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSceneNodeCount");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    
    // Case 1: rsUIDirector_ is valid by default - should return success
    uint32_t nodeCount = 0;
    auto ret = window->GetSceneNodeCount(nodeCount);
    EXPECT_EQ(ret, WSError::WS_OK);
    
    // Case 2: Explicitly set rsUIDirector_ to nullptr - should return error
    window->rsUIDirector_ = nullptr;
    nodeCount = 0;
    ret = window->GetSceneNodeCount(nodeCount);
    EXPECT_EQ(ret, WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: GetSceneNodeCount end";
}

/**
 * @tc.name: GetSceneNodeCount_Callback
 * @tc.desc: GetSceneNodeCount with callback parameter
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, GetSceneNodeCount_Callback, Function | SmallTest | Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: GetSceneNodeCount_Callback start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetSceneNodeCount_Callback");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    // Create a mock callback object
    sptr<MockIRemoteObject> callbackMocker = sptr<MockIRemoteObject>::MakeSptr();

    // Case 1: Call with valid callback - should return success
    auto ret = window->GetSceneNodeCount(callbackMocker);
    EXPECT_EQ(ret, WSError::WS_OK);

    // Case 2: Call with null callback - should return success but log error
    window->GetSceneNodeCount(nullptr);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_FALSE(g_logMsg.find("iface_cast failed") != std::string::npos);

    // Case 3: rsUIDirector_ is nullptr - should handle gracefully
    window->rsUIDirector_ = nullptr;
    window->GetSceneNodeCount(callbackMocker);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_FALSE(g_logMsg.find("rsUIDirector is nullptr") != std::string::npos);

    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: GetSceneNodeCount_Callback end";
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

/**
 * @tc.name: HandleSetOrientationCommon01
 * @tc.desc: 测试窗口无效场景
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, HandleSetOrientationCommon01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("HandleSetOrientationCommon01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    
    window->hostSession_ = nullptr;
    Orientation orientation = Orientation::VERTICAL;
    bool needAnimation = true;
    
    WMError result = window->HandleSetOrientationCommon(orientation, needAnimation);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_WINDOW);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon01 end";
}

/**
 * @tc.name: HandleSetOrientationCommon02
 * @tc.desc: 测试无需强制设置方向且需要动画场景
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, HandleSetOrientationCommon02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon02 start";
    window_ = GetTestWindowImpl("HandleSetOrientationCommon02");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    
    Orientation orientation = Orientation::VERTICAL;
    bool needAnimation = true;
    
    WMError result = window_->HandleSetOrientationCommon(orientation, needAnimation);
    EXPECT_EQ(result, WMError::WM_OK);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon02 end";
}

/**
 * @tc.name: HandleSetOrientationCommon03
 * @tc.desc: 测试支持旋转全屏场景
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, HandleSetOrientationCommon03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon03 start";
    window_ = GetTestWindowImpl("HandleSetOrientationCommon03");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    
    Orientation orientation = Orientation::USER_ROTATION_PORTRAIT;
    bool needAnimation = true;
    
    WMError result = window_->HandleSetOrientationCommon(orientation, needAnimation);
    EXPECT_EQ(result, WMError::WM_OK);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon03 end";
}

/**
 * @tc.name: HandleSetOrientationCommon04
 * @tc.desc: 测试需要动画场景
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, HandleSetOrientationCommon04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon04 start";
    window_ = GetTestWindowImpl("HandleSetOrientationCommon04");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    
    Orientation orientation = Orientation::USER_ROTATION_PORTRAIT;
    bool needAnimation = true;
    
    WMError result = window_->HandleSetOrientationCommon(orientation, needAnimation);
    EXPECT_EQ(result, WMError::WM_OK);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon04 end";
}

/**
 * @tc.name: HandleSetOrientationCommon05
 * @tc.desc: 测试无效方向场景
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, HandleSetOrientationCommon05, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon05 start";
    window_ = GetTestWindowImpl("HandleSetOrientationCommon05");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    
    Orientation orientation = Orientation::INVALID;
    bool needAnimation = false;
    
    WMError result = window_->HandleSetOrientationCommon(orientation, needAnimation);
    EXPECT_EQ(result, WMError::WM_OK);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon05 end";
}

/**
 * @tc.name: HandleSetOrientationCommon06
 * @tc.desc: 测试有效方向，需要动画场景
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, HandleSetOrientationCommon06, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon06 start";
    window_ = GetTestWindowImpl("HandleSetOrientationCommon06");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    
    Orientation orientation = Orientation::USER_ROTATION_PORTRAIT;
    bool needAnimation = true;
    
    WMError result = window_->HandleSetOrientationCommon(orientation, needAnimation);
    EXPECT_EQ(result, WMError::WM_OK);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon06 end";
}

/**
 * @tc.name: HandleSetOrientationCommon07
 * @tc.desc: 测试有效方向，无需动画场景
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, HandleSetOrientationCommon07, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon07 start";
    window_ = GetTestWindowImpl("HandleSetOrientationCommon07");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    
    Orientation orientation = Orientation::VERTICAL;
    bool needAnimation = false;
    
    WMError result = window_->HandleSetOrientationCommon(orientation, needAnimation);
    EXPECT_EQ(result, WMError::WM_OK);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: HandleSetOrientationCommon07 end";
}

/**
 * @tc.name: SetPreferredOrientationWithResult01
 * @tc.desc: 测试 HandleSetOrientationCommon 返回 WM_ERROR_INVALID_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, SetPreferredOrientationWithResult01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetPreferredOrientationWithResult01 start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetPreferredOrientationWithResult01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    
    window->property_->SetPersistentId(INVALID_SESSION_ID);
    window->state_ = WindowState::STATE_DESTROYED;
    
    Orientation orientation = Orientation::VERTICAL;
    uint32_t promiseId = 123;
    bool needAnimation = true;
    
    WMError result = window->SetPreferredOrientationWithResult(orientation, promiseId, needAnimation);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_WINDOW);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetPreferredOrientationWithResult01 end";
}

/**
 * @tc.name: SetPreferredOrientationWithResult02
 * @tc.desc: 测试 HandleSetOrientationCommon 返回 WM_DO_NOTHING
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, SetPreferredOrientationWithResult02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetPreferredOrientationWithResult02 start";
    window_ = GetTestWindowImpl("SetPreferredOrientationWithResult02");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    
    window_->property_->SetRequestedOrientation(Orientation::VERTICAL, false);
    
    Orientation orientation = Orientation::VERTICAL;
    uint32_t promiseId = 123;
    bool needAnimation = true;
    
    WMError result = window_->SetPreferredOrientationWithResult(orientation, promiseId, needAnimation);
    EXPECT_EQ(result, WMError::WM_OK);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetPreferredOrientationWithResult02 end";
}

/**
 * @tc.name: SetPreferredOrientationWithResult03
 * @tc.desc: 测试 hostSession 为空
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, SetPreferredOrientationWithResult03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetPreferredOrientationWithResult03 start";
    window_ = GetTestWindowImpl("SetPreferredOrientationWithResult03");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    window_->hostSession_ = nullptr;
    
    Orientation orientation = Orientation::USER_ROTATION_PORTRAIT;
    uint32_t promiseId = 123;
    bool needAnimation = true;
    
    WMError result = window_->SetPreferredOrientationWithResult(orientation, promiseId, needAnimation);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_WINDOW);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetPreferredOrientationWithResult03 end";
}

/**
 * @tc.name: SetPreferredOrientationWithResult04
 * @tc.desc: 测试正常流程，HandleSetOrientationCommon 返回 WM_OK
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, SetPreferredOrientationWithResult04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetPreferredOrientationWithResult04 start";
    window_ = GetTestWindowImpl("SetPreferredOrientationWithResult04");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    
    SessionInfo sessionInfo = { "SetPreferredOrientationWithResult04", "bundle", "ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window_->hostSession_ = session;
    
    Orientation orientation = Orientation::USER_ROTATION_PORTRAIT;
    uint32_t promiseId = 123;
    bool needAnimation = true;
    
    WMError result = window_->SetPreferredOrientationWithResult(orientation, promiseId, needAnimation);
    EXPECT_EQ(result, WMError::WM_OK);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetPreferredOrientationWithResult04 end";
}

/**
 * @tc.name: RegisterNotifyOrientationExecutionResultFunc
 * @tc.desc: 测试注册回调
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, RegisterNotifyOrientationExecutionResultFunc, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: RegisterNotifyOrientationExecutionResultFunc start";
    window_ = GetTestWindowImpl("RegisterNotifyOrientationExecutionResultFunc");
    ASSERT_NE(window_, nullptr);
    
    bool callbackCalled = false;
    window_->RegisterNotifyOrientationExecutionResultFunc([&callbackCalled](uint32_t, OrientationExecutionResult) {
        callbackCalled = true;
    });
    
    EXPECT_FALSE(callbackCalled);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: RegisterNotifyOrientationExecutionResultFunc end";
}

/**
 * @tc.name: NotifyOrientationExecutionResult01
 * @tc.desc: 测试正常回调执行
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, NotifyOrientationExecutionResult01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: NotifyOrientationExecutionResult01 start";
    window_ = GetTestWindowImpl("NotifyOrientationExecutionResult01");
    ASSERT_NE(window_, nullptr);
    
    bool callbackCalled = false;
    uint32_t receivedPromiseId = 0;
    OrientationExecutionResult receivedResult = OrientationExecutionResult::ORIENTATION_IGNORED;
    
    window_->RegisterNotifyOrientationExecutionResultFunc([&callbackCalled, &receivedPromiseId, &receivedResult](
        uint32_t promiseId, OrientationExecutionResult result) {
        callbackCalled = true;
        receivedPromiseId = promiseId;
        receivedResult = result;
    });
    
    uint32_t promiseId = 123;
    OrientationExecutionResult result = OrientationExecutionResult::ORIENTATION_APPLIED;
    
    WSError ret = window_->NotifyOrientationExecutionResult(promiseId, result);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_FALSE(callbackCalled);
    EXPECT_EQ(receivedPromiseId, 0);
    EXPECT_EQ(receivedResult, OrientationExecutionResult::ORIENTATION_IGNORED);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: NotifyOrientationExecutionResult01 end";
}

/**
 * @tc.name: NotifyOrientationExecutionResult02
 * @tc.desc: 测试回调为空场景
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, NotifyOrientationExecutionResult02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: NotifyOrientationExecutionResult02 start";
    window_ = GetTestWindowImpl("NotifyOrientationExecutionResult02");
    ASSERT_NE(window_, nullptr);
    
    WSError ret = window_->NotifyOrientationExecutionResult(123, OrientationExecutionResult::ORIENTATION_APPLIED);
    EXPECT_EQ(ret, WSError::WS_OK);
    
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: NotifyOrientationExecutionResult02 end";
}

/**
 * @tc.name: ConvertUserOrientationToUserPageOrientation
 * @tc.desc: Test ConvertUserOrientationToUserPageOrientation with all branch scenarios
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, ConvertUserOrientationToUserPageOrientation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: ConvertUserOrientationToUserPageOrientation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ConvertUserOrientationToUserPageOrientation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    // Case 1: USER_ROTATION_LANDSCAPE → USER_PAGE_ROTATION_LANDSCAPE
    EXPECT_EQ(window->ConvertUserOrientationToUserPageOrientation(Orientation::USER_ROTATION_LANDSCAPE),
              Orientation::USER_PAGE_ROTATION_LANDSCAPE);

    // Case 2: USER_ROTATION_LANDSCAPE_INVERTED → USER_PAGE_ROTATION_LANDSCAPE_INVERTED
    EXPECT_EQ(window->ConvertUserOrientationToUserPageOrientation(Orientation::USER_ROTATION_LANDSCAPE_INVERTED),
              Orientation::USER_PAGE_ROTATION_LANDSCAPE_INVERTED);

    // Case 3: USER_ROTATION_PORTRAIT → USER_PAGE_ROTATION_PORTRAIT
    EXPECT_EQ(window->ConvertUserOrientationToUserPageOrientation(Orientation::USER_ROTATION_PORTRAIT),
              Orientation::USER_PAGE_ROTATION_PORTRAIT);

    // Case 4: USER_ROTATION_PORTRAIT_INVERTED → USER_PAGE_ROTATION_PORTRAIT_INVERTED
    EXPECT_EQ(window->ConvertUserOrientationToUserPageOrientation(Orientation::USER_ROTATION_PORTRAIT_INVERTED),
              Orientation::USER_PAGE_ROTATION_PORTRAIT_INVERTED);

    // Case 5: UNSPECIFIED (default) → UNSPECIFIED
    EXPECT_EQ(window->ConvertUserOrientationToUserPageOrientation(Orientation::UNSPECIFIED),
              Orientation::UNSPECIFIED);

    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: ConvertUserOrientationToUserPageOrientation end";
}

/**
 * @tc.name: OrientationCheckMethods
 * @tc.desc: Test IsUserOrientation, IsUserPageOrientation, IsVerticalOrientation, IsHorizontalOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, OrientationCheckMethods, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: OrientationCheckMethods start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("OrientationCheckMethods");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    // IsUserOrientation: true case + false case
    EXPECT_TRUE(window->IsUserOrientation(Orientation::USER_ROTATION_PORTRAIT));
    EXPECT_FALSE(window->IsUserOrientation(Orientation::VERTICAL));

    // IsUserPageOrientation: true case + false case
    EXPECT_TRUE(window->IsUserPageOrientation(Orientation::USER_PAGE_ROTATION_PORTRAIT));
    EXPECT_FALSE(window->IsUserPageOrientation(Orientation::VERTICAL));

    // IsVerticalOrientation: true case + false case
    EXPECT_TRUE(window->IsVerticalOrientation(Orientation::VERTICAL));
    EXPECT_FALSE(window->IsVerticalOrientation(Orientation::HORIZONTAL));

    // IsHorizontalOrientation: true case + false case
    EXPECT_TRUE(window->IsHorizontalOrientation(Orientation::HORIZONTAL));
    EXPECT_FALSE(window->IsHorizontalOrientation(Orientation::VERTICAL));

    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: OrientationCheckMethods end";
}

/**
 * @tc.name: SetUserRequestedOrientation
 * @tc.desc: Test SetUserRequestedOrientation with valid and invalid window scenarios
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, SetUserRequestedOrientation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetUserRequestedOrientation start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetUserRequestedOrientation");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;

    // Case 1: Normal scenario - window is valid
    window->SetUserRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    EXPECT_EQ(window->property_->GetUserRequestedOrientation(), Orientation::USER_ROTATION_PORTRAIT);

    // Case 2: Window is invalid - property should not be updated
    window->state_ = WindowState::STATE_DESTROYED;
    window->SetUserRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE);
    EXPECT_EQ(window->property_->GetUserRequestedOrientation(), Orientation::USER_ROTATION_PORTRAIT);

    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetUserRequestedOrientation end";
}

/**
 * @tc.name: SimpleOrientationMethods
 * @tc.desc: Test UpdateOrientation, UpdateCurrentWindowOrientation and GetCurrentWindowOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, SimpleOrientationMethods, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SimpleOrientationMethods start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SimpleOrientationMethods");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    // Case 1: UpdateOrientation - always returns WS_OK
    EXPECT_EQ(window->UpdateOrientation(), WSError::WS_OK);

    // Case 2: UpdateCurrentWindowOrientation + GetCurrentWindowOrientation
    window->UpdateCurrentWindowOrientation(DisplayOrientation::PORTRAIT);
    EXPECT_EQ(window->GetCurrentWindowOrientation(), DisplayOrientation::PORTRAIT);

    // Case 3: Update with different orientation
    window->UpdateCurrentWindowOrientation(DisplayOrientation::LANDSCAPE);
    EXPECT_EQ(window->GetCurrentWindowOrientation(), DisplayOrientation::LANDSCAPE);

    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SimpleOrientationMethods end";
}

/**
 * @tc.name: isNeededForciblySetOrientation_Complete
 * @tc.desc: Test isNeededForciblySetOrientation with IsUserOrientation branch
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, isNeededForciblySetOrientation_Complete, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: isNeededForciblySetOrientation_Complete start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("isNeededForciblySetOrientation_Complete");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;
    window->SetRequestedOrientation(Orientation::VERTICAL);

    // Case 1: USER_ROTATION_PORTRAIT - direct return true (IsUserOrientation branch)
    EXPECT_TRUE(window->isNeededForciblySetOrientation(Orientation::USER_ROTATION_PORTRAIT));

    // Case 2: USER_ROTATION_LANDSCAPE - direct return true (IsUserOrientation branch)
    EXPECT_TRUE(window->isNeededForciblySetOrientation(Orientation::USER_ROTATION_LANDSCAPE));

    // Case 3: USER_ROTATION_PORTRAIT_INVERTED - direct return true (IsUserOrientation branch)
    EXPECT_TRUE(window->isNeededForciblySetOrientation(Orientation::USER_ROTATION_PORTRAIT_INVERTED));

    // Case 4: USER_ROTATION_LANDSCAPE_INVERTED - direct return true (IsUserOrientation branch)
    EXPECT_TRUE(window->isNeededForciblySetOrientation(Orientation::USER_ROTATION_LANDSCAPE_INVERTED));

    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: isNeededForciblySetOrientation_Complete end";
}

/**
 * @tc.name: ConvertInvalidOrientation_Complete
 * @tc.desc: Test ConvertInvalidOrientation with all USER_ROTATION_* and non-User orientations
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, ConvertInvalidOrientation_Complete, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: ConvertInvalidOrientation_Complete start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetDisplayId(0);
    option->SetWindowName("ConvertInvalidOrientation_Complete");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->state_ = WindowState::STATE_CREATED;

    // Case 1: USER_ROTATION_LANDSCAPE → USER_PAGE_ROTATION_LANDSCAPE (IsUserOrientation branch)
    window->SetRequestedOrientation(Orientation::UNSPECIFIED, false);
    EXPECT_EQ(window->ConvertInvalidOrientation(), Orientation::USER_PAGE_ROTATION_LANDSCAPE);

    // Case 2: USER_ROTATION_PORTRAIT_INVERTED → USER_PAGE_ROTATION_PORTRAIT_INVERTED
    window->SetRequestedOrientation(Orientation::UNSPECIFIED, false);
    EXPECT_EQ(window->ConvertInvalidOrientation(), Orientation::USER_PAGE_ROTATION_PORTRAIT_INVERTED);

    // Case 3: VERTICAL (non-User orientation) → VERTICAL (not converted)
    window->SetRequestedOrientation(Orientation::VERTICAL);
    EXPECT_EQ(window->ConvertInvalidOrientation(), Orientation::VERTICAL);

    // Case 4: UNSPECIFIED (non-User orientation) → UNSPECIFIED (not converted)
    window->SetRequestedOrientation(Orientation::UNSPECIFIED);
    EXPECT_EQ(window->ConvertInvalidOrientation(), Orientation::UNSPECIFIED);

    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: ConvertInvalidOrientation_Complete end";
}

/**
 * @tc.name: NotifyRotationChangeResult_NullFuture
 * @tc.desc: Test NotifyRotationChangeResult when getRotationResultFuture_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, NotifyRotationChangeResult_NullFuture, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: NotifyRotationChangeResult_NullFuture start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyRotationChangeResult_NullFuture");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->getRotationResultFuture_ = nullptr;
    RotationChangeResult result = { RectType::RELATIVE_TO_SCREEN, { 0, 0, 100, 100 } };
    window->NotifyRotationChangeResult(result);
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: NotifyRotationChangeResult_NullFuture end";
}

/**
 * @tc.name: NotifyRotationChangeResult_ValidFuture
 * @tc.desc: Test NotifyRotationChangeResult when getRotationResultFuture_ is valid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, NotifyRotationChangeResult_ValidFuture, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: NotifyRotationChangeResult_ValidFuture start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyRotationChangeResult_ValidFuture");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->getRotationResultFuture_ = sptr<FutureCallback>::MakeSptr();
    RotationChangeResult result = { RectType::RELATIVE_TO_SCREEN, { 0, 0, 100, 100 } };
    window->NotifyRotationChangeResult(result);
    EXPECT_NE(window->getRotationResultFuture_, nullptr);
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: NotifyRotationChangeResult_ValidFuture end";
}

/**
 * @tc.name: SetRotationLocked_NotSupported
 * @tc.desc: Test SetRotationLocked default implementation returns WM_ERROR_DEVICE_NOT_SUPPORT
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, SetRotationLocked_NotSupported, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetRotationLocked_NotSupported start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetRotationLocked_NotSupported");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    auto ret = window->SetRotationLocked(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: SetRotationLocked_NotSupported end";
}

/**
 * @tc.name: GetRotationLocked_NotSupported
 * @tc.desc: Test GetRotationLocked default implementation returns WM_ERROR_DEVICE_NOT_SUPPORT
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplRotationTest, GetRotationLocked_NotSupported, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: GetRotationLocked_NotSupported start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetRotationLocked_NotSupported");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    bool locked = false;
    auto ret = window->GetRotationLocked(locked);
    EXPECT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "WindowSessionImplRotationTest: GetRotationLocked_NotSupported end";
}

} // namespace
} // namespace Rosen
} // namespace OHOS