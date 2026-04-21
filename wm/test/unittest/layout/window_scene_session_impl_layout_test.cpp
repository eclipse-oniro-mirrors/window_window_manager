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
#include <gmock/gmock.h>
#include <parameters.h>

#include "ability_context_impl.h"
#include "display_info.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window_adapter.h"
#include "mock_window_scene_session_impl.h"
#include "scene_board_judgement.h"
#include "session/host/include/scene_session.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class MockWindowChangeListener : public IWindowChangeListener {
public:
    MOCK_METHOD3(OnSizeChange,
                 void(Rect rect, WindowSizeChangeReason reason, const std::shared_ptr<RSTransaction>& rsTransaction));
};

class MockWindowLifeCycleListener : public IWindowLifeCycle {
public:
    MOCK_METHOD0(AfterForeground, void(void));
    MOCK_METHOD0(AfterBackground, void(void));
    MOCK_METHOD0(AfterFocused, void(void));
    MOCK_METHOD0(AfterUnfocused, void(void));
    MOCK_METHOD1(ForegroundFailed, void(int32_t));
    MOCK_METHOD0(AfterActive, void(void));
    MOCK_METHOD0(AfterInactive, void(void));
    MOCK_METHOD0(AfterResumed, void(void));
    MOCK_METHOD0(AfterPaused, void(void));
};

class WindowSceneSessionImplLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;

protected:
    // Helper function to create WindowSceneSessionImpl with option
    sptr<WindowSceneSessionImpl> CreateWindowSession(const std::string& name, WindowType windowType)
    {
        sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
        option->SetWindowName(name);
        sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
        window->property_->SetWindowType(windowType);
        return window;
    }

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
    static constexpr uint32_t WAIT_SERVERAL_FRAMES = 36000;
};

void WindowSceneSessionImplLayoutTest::SetUpTestCase() {}

void WindowSceneSessionImplLayoutTest::TearDownTestCase() {}

void WindowSceneSessionImplLayoutTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSceneSessionImplLayoutTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

RSSurfaceNode::SharedPtr WindowSceneSessionImplLayoutTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "startingWindowTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    return surfaceNode;
}

namespace {

/**
 * @tc.name: SetAspectRatio01
 * @tc.desc: SetAspectRatio test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SetAspectRatio01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAspectRatio01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetAspectRatio(0.1));
}

/**
 * @tc.name: SetAspectRatio02
 * @tc.desc: SetAspectRatio test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SetAspectRatio02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAspectRatio02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->SetAspectRatio(0.1));

    window->property_->SetPersistentId(1);
    window->property_->SetDisplayId(0);
    WindowLimits windowLimits = { 3000, 3000, 2000, 2000, 2.0, 2.0 };
    window->property_->SetWindowLimits(windowLimits);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    window->hostSession_ = session;
    session->GetSessionProperty()->SetWindowLimits(windowLimits);
    const float ratio = 1.2;
    ASSERT_EQ(WMError::WM_OK, window->SetAspectRatio(ratio));
    ASSERT_EQ(ratio, session->GetAspectRatio());
}

/**
 * @tc.name: SetAspectRatio03
 * @tc.desc: SetAspectRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SetAspectRatio03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAspectRatio03");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    auto ret = windowSceneSessionImpl->SetAspectRatio(MathHelper::INF);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    windowSceneSessionImpl->property_->SetPersistentId(1);
    ret = windowSceneSessionImpl->SetAspectRatio(MathHelper::INF);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = windowSceneSessionImpl->SetAspectRatio(MathHelper::NAG_INF);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = windowSceneSessionImpl->SetAspectRatio(std::sqrt(-1.0));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = windowSceneSessionImpl->SetAspectRatio(0.0f);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = windowSceneSessionImpl->SetAspectRatio(1.0f);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetAspectRatio04
 * @tc.desc: SetAspectRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SetAspectRatio04, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAspectRatio04");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    auto property = windowSceneSessionImpl->GetProperty();
    property->SetPersistentId(123);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_NE(nullptr, session);
    windowSceneSessionImpl->hostSession_ = session;

    //case 1: is compatibility mode => WM_OK
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    property->SetCompatibleModeProperty(compatibleModeProperty);
    const float ratio = 1.5f;
    auto ret = windowSceneSessionImpl->SetAspectRatio(ratio);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: ResetAspectRatioTest
 * @tc.desc: Verify ResetAspectRatio behavior in different scenarios.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, ResetAspectRatioTest, TestSize.Level1)
{
    auto option = sptr<WindowOption>::MakeSptr();
    auto window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetAspectRatio(1.5f);
    SessionInfo sessionInfo;
    auto mockHostSession = sptr<SessionMocker>::MakeSptr(sessionInfo);

    // Case 1: hostSession_ is nullptr
    {
        window->hostSession_ = nullptr;
        auto ret = window->ResetAspectRatio();
        EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
        EXPECT_FLOAT_EQ(1.5f, window->property_->GetAspectRatio());
    }

    // Case 2: hostSession_->SetAspectRatio fails
    {
        EXPECT_CALL(*mockHostSession, SetAspectRatio(_))
            .Times(1).WillOnce(Return(WSError::WS_ERROR_IPC_FAILED));
        window->hostSession_ = mockHostSession;
        auto ret = window->ResetAspectRatio();
        EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, ret);
        EXPECT_FLOAT_EQ(1.5f, window->property_->GetAspectRatio());
    }

    // Case 3: hostSession_->SetAspectRatio succeeds
    {
        EXPECT_CALL(*mockHostSession, SetAspectRatio(_))
            .Times(1).WillOnce(Return(WSError::WS_OK));
        auto ret = window->ResetAspectRatio();
        EXPECT_EQ(WMError::WM_OK, ret);
        EXPECT_FLOAT_EQ(0.0f, window->property_->GetAspectRatio());
    }
}

/**
 * @tc.name: SetWindowLimits01
 * @tc.desc: SetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SetWindowLimits01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowLimits01");
    option->SetDisplayId(0);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_FROZEN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->useUniqueDensity_ = true;
    window->virtualPixelRatio_ = 1.9;

    WindowLimits windowLimits = { 2000, 2000, 2000, 2000, 0.0f, 0.0f };
    EXPECT_EQ(WMError::WM_OK, window->SetWindowLimits(windowLimits, false));
    WindowLimits windowSizeLimits = window->property_->GetWindowLimits();
    EXPECT_EQ(windowSizeLimits.maxWidth_, 2000);
    EXPECT_EQ(windowSizeLimits.maxHeight_, 2000);
    EXPECT_EQ(windowSizeLimits.minWidth_, 2000);
    EXPECT_EQ(windowSizeLimits.minHeight_, 2000);

    windowLimits = { 2000, 2000, 100, 100, 0.0f, 0.0f };
    EXPECT_EQ(WMError::WM_OK, window->SetWindowLimits(windowLimits, false));
    windowSizeLimits = window->property_->GetWindowLimits();
    EXPECT_EQ(windowSizeLimits.maxWidth_, 2000);
    EXPECT_EQ(windowSizeLimits.maxHeight_, 2000);
    EXPECT_NE(windowSizeLimits.minWidth_, 100);
    EXPECT_NE(windowSizeLimits.minHeight_, 100);

    windowLimits = { 10000, 10000, 30, 30, 0.0f, 0.0f };
    EXPECT_EQ(WMError::WM_OK, window->SetWindowLimits(windowLimits, false));
    windowSizeLimits = window->property_->GetWindowLimits();
    EXPECT_NE(windowSizeLimits.maxWidth_, 10000);
    EXPECT_NE(windowSizeLimits.maxHeight_, 10000);
    EXPECT_NE(windowSizeLimits.minWidth_, 30);
    EXPECT_NE(windowSizeLimits.minHeight_, 30);
}

/**
 * @tc.name: SetWindowLimits06
 * @tc.desc: SetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SetWindowLimits06, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowLimits06");
    option->SetDisplayId(0);

    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_FROZEN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    window->useUniqueDensity_ = true;
    window->virtualPixelRatio_ = 1.9;

    WindowLimits windowLimits = { 2000, 2000, 2000, 2000, 0.0f, 0.0f };
    EXPECT_EQ(WMError::WM_OK, window->SetWindowLimits(windowLimits, true));
    WindowLimits windowSizeLimits = window->property_->GetWindowLimits();
    EXPECT_EQ(windowSizeLimits.maxWidth_, 2000);
    EXPECT_EQ(windowSizeLimits.maxHeight_, 2000);
    EXPECT_EQ(windowSizeLimits.minWidth_, 2000);
    EXPECT_EQ(windowSizeLimits.minHeight_, 2000);

    windowLimits = { 2000, 2000, 100, 100, 0.0f, 0.0f };
    EXPECT_EQ(WMError::WM_OK, window->SetWindowLimits(windowLimits, true));
    windowSizeLimits = window->property_->GetWindowLimits();
    EXPECT_EQ(windowSizeLimits.maxWidth_, 2000);
    EXPECT_EQ(windowSizeLimits.maxHeight_, 2000);
    EXPECT_NE(windowSizeLimits.minWidth_, 100);
    EXPECT_NE(windowSizeLimits.minHeight_, 100);

    windowLimits = { 2000, 2000, 100, 100, 0.0f, 0.0f };
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(WMError::WM_OK, window->SetWindowLimits(windowLimits, true));
    windowSizeLimits = window->property_->GetWindowLimits();
    EXPECT_EQ(windowSizeLimits.maxWidth_, 2000);
    EXPECT_EQ(windowSizeLimits.maxHeight_, 2000);
    EXPECT_EQ(windowSizeLimits.minWidth_, 100);
    EXPECT_EQ(windowSizeLimits.minHeight_, 100);

    windowLimits = { 10000, 10000, 30, 30, 0.0f, 0.0f };
    EXPECT_EQ(WMError::WM_OK, window->SetWindowLimits(windowLimits, true));
    windowSizeLimits = window->property_->GetWindowLimits();
    EXPECT_NE(windowSizeLimits.maxWidth_, 10000);
    EXPECT_NE(windowSizeLimits.maxHeight_, 10000);
    EXPECT_NE(windowSizeLimits.minWidth_, 30);
    EXPECT_NE(windowSizeLimits.minHeight_, 30);
}

/**
 * @tc.name: SetWindowLimits02
 * @tc.desc: SetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SetWindowLimits02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowLimits02");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    WindowLimits windowLimits = { 1000, 1000, 1000, 1000, 0.0f, 0.0f };

    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    auto ret = windowSceneSessionImpl->SetWindowLimits(windowLimits, false);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
    ret = windowSceneSessionImpl->SetWindowLimits(windowLimits, true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: SetWindowLimits03
 * @tc.desc: SetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SetWindowLimits03, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowLimits03");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    WindowLimits windowLimits = { 1000, 1000, 1000, 1000, 0.0f, 0.0f };
    windowSceneSessionImpl->property_->SetPersistentId(1);
    windowSceneSessionImpl->property_->SetDisplayId(0);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_CREATED;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    auto ret = windowSceneSessionImpl->SetWindowLimits(windowLimits, false);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = windowSceneSessionImpl->SetWindowLimits(windowLimits, true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetWindowLimits04
 * @tc.desc: SetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SetWindowLimits04, TestSize.Level1)
{
    sptr<WindowOption> subWindow = sptr<WindowOption>::MakeSptr();
    subWindow->SetWindowName("SetWindowLimits04");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(subWindow);
    WindowLimits windowLimits = { 1000, 1000, 1000, 1000, 0.0f, 0.0f };
    windowSceneSessionImpl->SetWindowLimits(windowLimits, false);
    windowSceneSessionImpl->property_->SetPersistentId(1004);
    windowSceneSessionImpl->property_->SetDisplayId(0);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = session;
    windowSceneSessionImpl->state_ = WindowState::STATE_CREATED;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_LAUNCHING);
    auto ret = windowSceneSessionImpl->SetWindowLimits(windowLimits, false);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = windowSceneSessionImpl->SetWindowLimits(windowLimits, true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetWindowLimits05
 * @tc.desc: SetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SetWindowLimits05, TestSize.Level1)
{
    sptr<WindowOption> subWindow = sptr<WindowOption>::MakeSptr();
    subWindow->SetWindowName("SetWindowLimits05");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(subWindow);
    WindowLimits windowLimits = { 1000, 1000, 1000, 1000, 0.0f, 0.0f };
    windowSceneSessionImpl->property_->SetPersistentId(1005);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = subSession;
    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, windowSceneSessionImpl->SetWindowLimits(windowLimits, false));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, windowSceneSessionImpl->SetWindowLimits(windowLimits, true));

    windowSceneSessionImpl->property_->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE);
    EXPECT_EQ(WMError::WM_OK, windowSceneSessionImpl->SetWindowLimits(windowLimits, false));
    EXPECT_EQ(WMError::WM_OK, windowSceneSessionImpl->SetWindowLimits(windowLimits, true));

    windowSceneSessionImpl->property_->SetDragEnabled(false);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, windowSceneSessionImpl->SetWindowLimits(windowLimits, false));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, windowSceneSessionImpl->SetWindowLimits(windowLimits, true));
}

/**
 * @tc.name: SetWindowLimits_Test_Diff_Unit
 * @tc.desc: SetWindowLimits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SetWindowLimits_Test_Diff_Unit, TestSize.Level1)
{
    sptr<WindowOption> windowOption = sptr<WindowOption>::MakeSptr();
    windowOption->SetWindowName("SetWindowLimits_Test_Diff_Unit");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(windowOption);
    windowSceneSessionImpl->GetProperty()->SetPersistentId(1005);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> sessionMocker = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSessionImpl->hostSession_ = sessionMocker;
    windowSceneSessionImpl->GetProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    // Case1: set window limits measured in px
    windowSceneSessionImpl->forceLimits_ = false;
    windowSceneSessionImpl->userLimitsSet_ = false;
    WindowLimits windowLimitsPx = { 2000, 2000, 2000, 2000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits limitsPxCopy = windowLimitsPx;
    EXPECT_EQ(WMError::WM_OK, windowSceneSessionImpl->SetWindowLimits(limitsPxCopy, true));
    EXPECT_EQ(windowSceneSessionImpl->forceLimits_, true);
    EXPECT_EQ(windowSceneSessionImpl->userLimitsSet_, true);
    WindowLimits userWindowLimits = windowSceneSessionImpl->GetProperty()->GetUserWindowLimits();
    EXPECT_EQ(userWindowLimits.maxWidth_, windowLimitsPx.maxWidth_);
    EXPECT_EQ(userWindowLimits.maxHeight_, windowLimitsPx.maxHeight_);
    EXPECT_EQ(userWindowLimits.minWidth_, windowLimitsPx.minWidth_);
    EXPECT_EQ(userWindowLimits.minHeight_, windowLimitsPx.minHeight_);
    EXPECT_EQ(userWindowLimits.pixelUnit_, windowLimitsPx.pixelUnit_);

    // Case1: set window limits measured in vp
    windowSceneSessionImpl->forceLimits_ = true;
    windowSceneSessionImpl->userLimitsSet_ = false;
    WindowLimits windowLimitsVp = { 1000, 1000, 1000, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    WindowLimits limitsVpCopy = windowLimitsVp;
    EXPECT_EQ(WMError::WM_OK, windowSceneSessionImpl->SetWindowLimits(limitsVpCopy, true));
    EXPECT_EQ(windowSceneSessionImpl->forceLimits_, false);
    EXPECT_EQ(windowSceneSessionImpl->userLimitsSet_, true);
    userWindowLimits = windowSceneSessionImpl->GetProperty()->GetUserWindowLimits();
    EXPECT_EQ(userWindowLimits.maxWidth_, windowLimitsVp.maxWidth_);
    EXPECT_EQ(userWindowLimits.maxHeight_, windowLimitsVp.maxHeight_);
    EXPECT_EQ(userWindowLimits.minWidth_, windowLimitsVp.minWidth_);
    EXPECT_EQ(userWindowLimits.minHeight_, windowLimitsVp.minHeight_);
    EXPECT_EQ(userWindowLimits.pixelUnit_, windowLimitsVp.pixelUnit_);
}

/**
 * @tc.name: SetMinimumDimensions_Test_Other_Window_Type
 * @tc.desc: SetMinimumDimensions test other window types besides the main window, sub window,
 *           dialog window and system window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SetMinimumDimensions_Test_Other_Window_Type, TestSize.Level1)
{
    sptr<WindowOption> windowOption = sptr<WindowOption>::MakeSptr();
    windowOption->SetWindowName("SetMinimumDimensions_Test_Other_Window_Type");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(windowOption);
    window->GetProperty()->SetPersistentId(1005);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> sessionMocker = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = sessionMocker;
    window->GetProperty()->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);

    WindowLimits systemLimits;
    WindowLimits systemLimitsVP = WindowLimits::DEFAULT_VP_LIMITS();
    const uint32_t displayWidth = 3120;
    const uint32_t displayHeight = 2080;
    const float virtualPixelRatio = 1.9f;
    window->SetMinimumDimensions(systemLimits, systemLimitsVP, displayWidth, displayHeight, virtualPixelRatio);
    EXPECT_EQ(systemLimits.minWidth_, static_cast<uint32_t>(MIN_FLOATING_WIDTH * virtualPixelRatio));
    EXPECT_EQ(systemLimits.minHeight_, static_cast<uint32_t>(MIN_FLOATING_HEIGHT * virtualPixelRatio));
    EXPECT_EQ(systemLimitsVP.minWidth_, MIN_FLOATING_WIDTH);
    EXPECT_EQ(systemLimitsVP.minHeight_, MIN_FLOATING_HEIGHT);
}

/**
 * @tc.name: UpdateWindowSizeLimits_Test_By_WinType
 * @tc.desc: UpdateWindowSizeLimits test result by window types
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, UpdateWindowSizeLimits_Test_By_WinType, TestSize.Level1)
{
    sptr<WindowOption> windowOption = sptr<WindowOption>::MakeSptr();
    windowOption->SetWindowName("UpdateWindowSizeLimits_Test_By_WinType");
    sptr<MockWindowSceneSessionImpl> window = sptr<MockWindowSceneSessionImpl>::MakeSptr(windowOption);
    window->GetProperty()->SetPersistentId(1005);
    window->GetProperty()->SetDisplayId(0);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> sessionMocker = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = sessionMocker;
    EXPECT_CALL(*window, GetVirtualPixelRatio(_))
        .Times(3)
        .WillRepeatedly(Return(3.25f));

    // Case1: Not system window
    window->GetProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->UpdateWindowSizeLimits();
    EXPECT_NE(window->GetProperty()->GetWindowLimits().minWidth_, 1);
    EXPECT_NE(window->GetProperty()->GetWindowLimits().minHeight_, 1);

    // Case2: System window and user limits has been set
    window->GetProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    window->userLimitsSet_ = true;
    window->UpdateWindowSizeLimits();
    EXPECT_NE(window->GetProperty()->GetWindowLimits().minWidth_, 1);
    EXPECT_NE(window->GetProperty()->GetWindowLimits().minHeight_, 1);

    // Case3: System window and user limits has not been set
    window->userLimitsSet_ = false;
    window->UpdateWindowSizeLimits();
    EXPECT_EQ(window->GetProperty()->GetWindowLimits().minWidth_, 1);
    EXPECT_EQ(window->GetProperty()->GetWindowLimits().minHeight_, 1);
}

/**
 * @tc.name: CalculateNewLimitsByRatio01
 * @tc.desc: CalculateNewLimitsByRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalculateNewLimitsByRatio01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateNewLimitsByRatio01");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    WindowLimits newLimits = { 0, 0, 0, 0, 0.0, 0.0 };
    WindowLimits newLimitsVP = WindowLimits::DEFAULT_VP_LIMITS();
    WindowLimits customizedLimits = { 3, 3, 3, 3, 0.0, 0.0 };
    windowSceneSessionImpl->CalculateNewLimitsByRatio(newLimits, newLimitsVP, customizedLimits);
    newLimits = { 500, 500, 500, 500, 0.0, 0.0 };
    customizedLimits = { 3, 3, 3, 3, 2.0, 2.0 };
    windowSceneSessionImpl->CalculateNewLimitsByRatio(newLimits, newLimitsVP, customizedLimits);
    customizedLimits = { 3, 3, 3, 3, 1.0, 1.0 };
    windowSceneSessionImpl->CalculateNewLimitsByRatio(newLimits, newLimitsVP, customizedLimits);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_END);
    auto ret = windowSceneSessionImpl->UpdateAnimationFlagProperty(true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: CalculateNewLimitsByRatio02
 * @tc.desc: CalculateNewLimitsByRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalculateNewLimitsByRatio02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateNewLimitsByRatio02");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    WindowLimits newLimits = { 0, 0, 0, 0, 0.0, 0.0 };
    WindowLimits newLimitsVP = WindowLimits::DEFAULT_VP_LIMITS();
    WindowLimits customizedLimits = { 3, 3, 3, 3, 0.0, 0.0 };
    windowSceneSessionImpl->CalculateNewLimitsByRatio(newLimits, newLimitsVP, customizedLimits);
    newLimits = { 500, 500, 500, 500, 0.0, 0.0 };
    customizedLimits = { 3, 3, 3, 3, 2.0, 2.0 };
    windowSceneSessionImpl->CalculateNewLimitsByRatio(newLimits, newLimitsVP, customizedLimits);
    customizedLimits = { 3, 3, 3, 3, 1.0, 1.0 };
    windowSceneSessionImpl->CalculateNewLimitsByRatio(newLimits, newLimitsVP, customizedLimits);
    windowSceneSessionImpl->property_->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE);
    auto ret = windowSceneSessionImpl->UpdateAnimationFlagProperty(true);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: MoveToAsync01
 * @tc.desc: MoveToAsync
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, MoveToAsync01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MoveToAsync01");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    option->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, window->MoveToAsync(10, 10));

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
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
HWTEST_F(WindowSceneSessionImplLayoutTest, MoveToAsync02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MoveToAsync02");
    option->SetWindowType(WindowType::WINDOW_TYPE_TOAST);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(10001);
    Rect rect;
    WMError ret;
    ret = window->Create(abilityContext_, nullptr);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
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
    } else {
        EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    }
}

/**
 * @tc.name: ResizeAsync01
 * @tc.desc: ResizeAsync
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, ResizeAsync01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ResizeAsync01");
    option->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    option->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
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
HWTEST_F(WindowSceneSessionImplLayoutTest, ResizeAsync02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ResizeAsync02");
    option->SetWindowType(WindowType::WINDOW_TYPE_TOAST);
    option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(10012);
    Rect rect;
    WMError ret;
    ret = window->Create(abilityContext_, nullptr);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(WMError::WM_OK, ret);
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
    } else {
        EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    }
}

/**
 * @tc.name: MoveTo01
 * @tc.desc: MoveTo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, MoveTo01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MoveTo01");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    windowSceneSession->property_->SetPersistentId(1);
    windowSceneSession->state_ = WindowState::STATE_HIDDEN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSceneSession->hostSession_ = session;
    ASSERT_EQ(WMError::WM_OK, windowSceneSession->MoveTo(2, 2));
}

/**
 * @tc.name: MoveTo02
 * @tc.desc: MoveTo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, MoveTo02, TestSize.Level1)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("MoveTo02SubWindow");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow->property_->SetPersistentId(1001);
    SessionInfo subSessionInfo = { "CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(2, 2));

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MoveTo02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle2", "CreateTestModule2", "CreateTestAbility2" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->state_ = WindowState::STATE_SHOWN;
    ASSERT_EQ(WMError::WM_OK, window->Create(abilityContext_, session));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(3, 3));
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(3, 3));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(3, 4));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(4, 4));
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(4, 4));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(4, 5));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(5, 5));
    ASSERT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: MoveTo03
 * @tc.desc: MoveTo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, MoveTo03, TestSize.Level1)
{
    sptr<WindowOption> subOption = sptr<WindowOption>::MakeSptr();
    subOption->SetWindowName("MoveTo01SubWindow");
    subOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<WindowSceneSessionImpl> subWindow = sptr<WindowSceneSessionImpl>::MakeSptr(subOption);
    subWindow->property_->SetPersistentId(1001);
    SessionInfo subSessionInfo = { "CreateSubTestBundle", "CreateSubTestModule", "CreateSubTestAbility" };
    sptr<SessionMocker> subSession = sptr<SessionMocker>::MakeSptr(subSessionInfo);
    subWindow->hostSession_ = subSession;
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(2, 2));

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("MoveTo02");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    WindowSceneSessionImpl::windowSessionMap_.insert(std::make_pair(
        window->GetWindowName(), std::pair<uint64_t, sptr<WindowSessionImpl>>(window->GetWindowId(), window)));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(3, 3));
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(4, 3));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(4, 4));
    window->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(5, 4));
    ASSERT_EQ(WMError::WM_OK, subWindow->MoveTo(5, 4));
    WindowSceneSessionImpl::windowSessionMap_.erase(window->GetWindowName());
}

/**
 * @tc.name: UpdateWindowModeForUITest01
 * @tc.desc: UpdateWindowModeForUITest
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, UpdateWindowModeForUITest01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateWindowModeForUITest01");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    const int32_t windowId = 1;
    window->property_->SetPersistentId(windowId);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WMError res = window->UpdateWindowModeForUITest(static_cast<int32_t>(WindowMode::WINDOW_MODE_UNDEFINED));
    EXPECT_EQ(WMError::WM_DO_NOTHING, res);
    res = window->UpdateWindowModeForUITest(static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN));
    EXPECT_EQ(WMError::WM_OK, res);
    res = window->UpdateWindowModeForUITest(static_cast<int32_t>(WindowMode::WINDOW_MODE_SPLIT_PRIMARY));
    EXPECT_EQ(WMError::WM_OK, res);
    res = window->UpdateWindowModeForUITest(static_cast<int32_t>(WindowMode::WINDOW_MODE_SPLIT_SECONDARY));
    EXPECT_EQ(WMError::WM_OK, res);
    res = window->UpdateWindowModeForUITest(static_cast<int32_t>(WindowMode::WINDOW_MODE_FLOATING));
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: GetWindowLimits_Test_Unit
 * @tc.desc: Test the return value of getWindowLimits under different unit input parameters.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, GetWindowLimits_Test_Unit, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetWindowLimits_Test_Unit");
    option->SetDisplayId(0);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->GetProperty()->SetPersistentId(2025);
    window->GetProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->state_ = WindowState::STATE_SHOWN;
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowLimits windowLimitsPx = { 2000, 2000, 2000, 2000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits windowLimitsVp = { 1000, 1000, 1000, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    window->GetProperty()->SetWindowLimits(windowLimitsPx);
    window->GetProperty()->SetWindowLimitsVP(windowLimitsVp);

    // Case1: getWindowLimits measured in px
    WindowLimits windowSizeLimitsPx;
    EXPECT_EQ(window->GetWindowLimits(windowSizeLimitsPx, false), WMError::WM_OK);
    EXPECT_EQ(windowSizeLimitsPx.maxWidth_, windowLimitsPx.maxWidth_);
    EXPECT_EQ(windowSizeLimitsPx.maxHeight_, windowLimitsPx.maxHeight_);
    EXPECT_EQ(windowSizeLimitsPx.minWidth_, windowLimitsPx.minWidth_);
    EXPECT_EQ(windowSizeLimitsPx.minHeight_, windowLimitsPx.minHeight_);
    EXPECT_EQ(windowSizeLimitsPx.pixelUnit_, windowLimitsPx.pixelUnit_);

    // Case2: getWindowLimits measured in vp
    WindowLimits windowSizeLimitsVp;
    EXPECT_EQ(window->GetWindowLimits(windowSizeLimitsVp, true), WMError::WM_OK);
    EXPECT_EQ(windowSizeLimitsVp.maxWidth_, windowLimitsVp.maxWidth_);
    EXPECT_EQ(windowSizeLimitsVp.maxHeight_, windowLimitsVp.maxHeight_);
    EXPECT_EQ(windowSizeLimitsVp.minWidth_, windowLimitsVp.minWidth_);
    EXPECT_EQ(windowSizeLimitsVp.minHeight_, windowLimitsVp.minHeight_);
    EXPECT_EQ(windowSizeLimitsVp.pixelUnit_, windowLimitsVp.pixelUnit_);
}

/**
 * @tc.name: FillWindowLimits_By_PixelUnit
 * @tc.desc: Test the return value of FillWindowLimits under different unit input parameters.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, FillWindowLimits_By_PixelUnit, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("FillWindowLimits_By_PixelUnit");
    option->SetDisplayId(0);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    window->GetProperty()->SetPersistentId(2025);
    window->GetProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    WindowLimits windowLimitsPx = { 2000, 2000, 2000, 2000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits windowLimitsVp = { 1000, 1000, 1000, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    window->GetProperty()->SetWindowLimits(windowLimitsPx);
    window->GetProperty()->SetWindowLimitsVP(windowLimitsVp);

    // Case1: px
    WindowLimits windowSizeLimitsPx;
    window->FillWindowLimits(windowSizeLimitsPx, PixelUnit::PX);
    EXPECT_EQ(windowSizeLimitsPx.maxWidth_, windowLimitsPx.maxWidth_);
    EXPECT_EQ(windowSizeLimitsPx.maxHeight_, windowLimitsPx.maxHeight_);
    EXPECT_EQ(windowSizeLimitsPx.minWidth_, windowLimitsPx.minWidth_);
    EXPECT_EQ(windowSizeLimitsPx.minHeight_, windowLimitsPx.minHeight_);
    EXPECT_EQ(windowSizeLimitsPx.pixelUnit_, windowLimitsPx.pixelUnit_);

    // Case2: vp
    WindowLimits windowSizeLimitsVp;
    window->FillWindowLimits(windowSizeLimitsVp, PixelUnit::VP);
    EXPECT_EQ(windowSizeLimitsVp.maxWidth_, windowLimitsVp.maxWidth_);
    EXPECT_EQ(windowSizeLimitsVp.maxHeight_, windowLimitsVp.maxHeight_);
    EXPECT_EQ(windowSizeLimitsVp.minWidth_, windowLimitsVp.minWidth_);
    EXPECT_EQ(windowSizeLimitsVp.minHeight_, windowLimitsVp.minHeight_);
    EXPECT_EQ(windowSizeLimitsVp.pixelUnit_, windowLimitsVp.pixelUnit_);
}

/**
 * @tc.name: UpdateAttachedWindowLimits01
 * @tc.desc: Test UpdateAttachedWindowLimits with null sessionStage
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, UpdateAttachedWindowLimits01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateAttachedWindowLimits01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(window, nullptr);

    WindowLimits limits = { 2000, 2000, 100, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WSError res = window->UpdateAttachedWindowLimits(999, limits, false, false);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: UpdateAttachedWindowLimits02
 * @tc.desc: Test UpdateAttachedWindowLimits with valid sessionStage
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, UpdateAttachedWindowLimits02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateAttachedWindowLimits02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(window, nullptr);

    // Create and attach sessionStage mock
    SessionInfo sessionInfo = { "UpdateAttachedWindowLimits02", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowLimits limits = { 1800, 1200, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WSError res = window->UpdateAttachedWindowLimits(888, limits, true, true);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: RemoveAttachedWindowLimits01
 * @tc.desc: Test RemoveAttachedWindowLimits functionality
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, RemoveAttachedWindowLimits01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RemoveAttachedWindowLimits01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "RemoveAttachedWindowLimits01", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    // First add limits
    WindowLimits limits = { 2000, 1000, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->UpdateAttachedWindowLimits(777, limits, true, true);

    // Then remove
    WSError res = window->RemoveAttachedWindowLimits(777);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: CalcSingleWinIntersect01
 * @tc.desc: Test PX limits intersection calculation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalcSingleWinIntersect01,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalcSingleWinIntersect01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    WindowLimits currentLimits = { 2000, 2000, 100, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits currentLimitsVP = { 1000, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    WindowLimits attachedLimits = { 800, 1500, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    AttachLimitOptions limitOptions{ true, true }; // Enable both height and width limits intersection
    float virtualPixelRatio = 2.0f;

    auto result = window->CalcSingleWinIntersect(
        currentLimits, currentLimitsVP, attachedLimits, limitOptions, virtualPixelRatio);

    EXPECT_TRUE(result.pxValid);
    EXPECT_TRUE(result.vpValid);
    EXPECT_EQ(result.pxLimits.minWidth_, 150); // max(100, 150)
    EXPECT_EQ(result.pxLimits.maxWidth_, 800);  // min(2000, 800)
    EXPECT_EQ(result.pxLimits.minHeight_, 1000); // max(1000, 250)
    EXPECT_EQ(result.pxLimits.maxHeight_, 1500); // min(2000, 1500)
}

/**
 * @tc.name: CalcSingleWinIntersect02
 * @tc.desc: Test VP limits intersection calculation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalcSingleWinIntersect02,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalcSingleWinIntersect02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    WindowLimits currentLimits = { 2000, 2000, 100, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits currentLimitsVP = { 1000, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    WindowLimits attachedLimits = { 1200, 600, 80, 120, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    AttachLimitOptions limitOptions{ true, true }; // Enable both height and width limits intersection
    float virtualPixelRatio = 2.0f;

    auto result = window->CalcSingleWinIntersect(
        currentLimits, currentLimitsVP, attachedLimits, limitOptions, virtualPixelRatio);

    EXPECT_TRUE(result.pxValid);
    EXPECT_TRUE(result.vpValid);
    EXPECT_EQ(result.vpLimits.minWidth_, 80);   // max(50, 80)
    EXPECT_EQ(result.vpLimits.maxWidth_, 1000);  // min(1000, 1200)
}

/**
 * @tc.name: CalcSingleWinIntersect03
 * @tc.desc: Test invalid intersection (min > max)
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalcSingleWinIntersect03,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalcSingleWinIntersect03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    WindowLimits currentLimits = { 2000, 2000, 100, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits currentLimitsVP = { 1000, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    // Attached limits have minWidth > current maxWidth (no intersection)
    WindowLimits attachedLimits = { 2200, 3000, 2500, 500, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    AttachLimitOptions limitOptions{ true, true }; // Enable both height and width limits intersection
    float virtualPixelRatio = 2.0f;

    auto result = window->CalcSingleWinIntersect(
        currentLimits, currentLimitsVP, attachedLimits, limitOptions, virtualPixelRatio);

    EXPECT_FALSE(result.pxValid);
}

/**
 * @tc.name: CalculateAttachedWindowLimitsIntersection01
 * @tc.desc: Test multiple attached windows with priority order
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalculateAttachedWindowLimitsIntersection01,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateAttachedWindowLimitsIntersection01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;

    // Simulate attached windows list
    WindowLimits limits1 = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits limits2 = { 2200, 1200, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetAttachedWindowLimits(1, limits1);
    window->property_->SetAttachedWindowLimits(2, limits2);

    // Set limit options for each window
    AttachLimitOptions limitOptions{ true, true };
    window->property_->SetAttachedLimitOptions(1, limitOptions);
    window->property_->SetAttachedLimitOptions(2, limitOptions);

    WindowLimits newLimits = { 2500, 1500, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits newLimitsVP = { 1250, 750, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    float virtualPixelRatio = 2.0f;

    window->CalculateAttachedWindowLimitsIntersection(newLimits, newLimitsVP, virtualPixelRatio);

    // First attached window: minWidth=max(100,200)=200, maxWidth=min(2500,2000)=2000
    // Second window applied: minWidth=max(200,150)=200, maxWidth=min(2000,2200)=2000
    EXPECT_EQ(newLimits.minWidth_, 200);
    EXPECT_EQ(newLimits.maxWidth_, 2000);
}

/**
 * @tc.name: CalculateAttachedWindowLimitsIntersection02
 * @tc.desc: Test with empty attached limits list
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalculateAttachedWindowLimitsIntersection02,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateAttachedWindowLimitsIntersection02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;

    WindowLimits newLimits = { 2500, 1500, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits newLimitsVP = { 1250, 750, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    float virtualPixelRatio = 2.0f;

    window->CalculateAttachedWindowLimitsIntersection(newLimits, newLimitsVP, virtualPixelRatio);

    // Limits should remain unchanged
    EXPECT_EQ(newLimits.minWidth_, 100);
    EXPECT_EQ(newLimits.maxWidth_, 2500);
}

/**
 * @tc.name: UpdateAttachedWindowLimits03
 * @tc.desc: Test UpdateAttachedWindowLimits updates existing sourceId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, UpdateAttachedWindowLimits03, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateAttachedWindowLimits03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo sessionInfo = { "UpdateAttachedWindowLimits03", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    // Add limits first time
    WindowLimits limits1 = { 1000, 800, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->UpdateAttachedWindowLimits(111, limits1, true, false);

    // Update with new limits for same sourceId
    WindowLimits limits2 = { 1200, 900, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->UpdateAttachedWindowLimits(111, limits2, true, true);

    // Verify the limits were updated, not duplicated
    auto attachedList = window->property_->GetAttachedWindowLimitsList();
    bool found = false;
    for (const auto& [id, limits] : attachedList) {
        if (id == 111) {
            EXPECT_EQ(limits.minWidth_, 150);
            EXPECT_EQ(limits.maxWidth_, 1200);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name: UpdateAttachedWindowLimits04
 * @tc.desc: Test UpdateAttachedWindowLimits with VP unit limits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, UpdateAttachedWindowLimits04, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateAttachedWindowLimits04");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo sessionInfo = { "UpdateAttachedWindowLimits04", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowLimits limits = { 800, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    WSError res = window->UpdateAttachedWindowLimits(222, limits, false, true);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: RemoveAttachedWindowLimits02
 * @tc.desc: Test RemoveAttachedWindowLimits with non-existent sourceId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, RemoveAttachedWindowLimits02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RemoveAttachedWindowLimits02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo sessionInfo = { "RemoveAttachedWindowLimits02", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    // Try to remove non-existent sourceId
    WSError res = window->RemoveAttachedWindowLimits(99999);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: RemoveAttachedWindowLimits03
 * @tc.desc: Test RemoveAttachedWindowLimits with multiple sources
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, RemoveAttachedWindowLimits03, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RemoveAttachedWindowLimits03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo sessionInfo = { "RemoveAttachedWindowLimits03", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    // Add multiple limits
    WindowLimits limits1 = { 1000, 800, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits limits2 = { 1200, 900, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits limits3 = { 1500, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->UpdateAttachedWindowLimits(1, limits1, true, true);
    window->UpdateAttachedWindowLimits(2, limits2, true, true);
    window->UpdateAttachedWindowLimits(3, limits3, true, true);

    // Remove middle one
    window->RemoveAttachedWindowLimits(2);

    // Verify only 2 remain and sourceId 2 is removed
    auto attachedList = window->property_->GetAttachedWindowLimitsList();
    EXPECT_EQ(attachedList.size(), 2u);
    for (const auto& [id, limits] : attachedList) {
        EXPECT_NE(id, 2);
    }
}

/**
 * @tc.name: RemoveAttachedWindowLimits04
 * @tc.desc: Test RemoveAttachedWindowLimits when sourceId matches current window (detaching)
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, RemoveAttachedWindowLimits04, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RemoveAttachedWindowLimits04");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo sessionInfo = { "RemoveAttachedWindowLimits04", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    // Add multiple limits
    WindowLimits limits1 = { 1000, 800, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits limits2 = { 1200, 900, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits limits3 = { 1500, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->UpdateAttachedWindowLimits(1, limits1, true, true);
    window->UpdateAttachedWindowLimits(2, limits2, true, true);
    window->UpdateAttachedWindowLimits(3, limits3, true, true);

    // Verify limits are added
    auto attachedList = window->property_->GetAttachedWindowLimitsList();
    EXPECT_EQ(attachedList.size(), 3u);

    // Call RemoveAttachedWindowLimits with current window's persistentId
    // This simulates the window detaching from all attached windows
    int32_t currentPersistentId = window->GetPersistentId();
    window->RemoveAttachedWindowLimits(currentPersistentId);

    // Verify all limits are cleared
    attachedList = window->property_->GetAttachedWindowLimitsList();
    EXPECT_EQ(attachedList.size(), 0u);

    // Verify limit options are also cleared
    auto limitOptionsList = window->property_->GetAttachedLimitOptionsList();
    EXPECT_EQ(limitOptionsList.size(), 0u);
}

/**
 * @tc.name: CalcSingleWinIntersect04
 * @tc.desc: Test intersection with height only
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalcSingleWinIntersect04,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalcSingleWinIntersect04");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    WindowLimits currentLimits = { 2000, 2000, 100, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits currentLimitsVP = { 1000, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    WindowLimits attachedLimits = { 1100, 1500, 80, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    // Only intersect height
    AttachLimitOptions limitOptions{ true, false }; // height=true, width=false
    float virtualPixelRatio = 2.0f;

    auto result = window->CalcSingleWinIntersect(
        currentLimits, currentLimitsVP, attachedLimits, limitOptions, virtualPixelRatio);

    EXPECT_TRUE(result.pxValid);
    EXPECT_TRUE(result.vpValid);
    // Width should remain unchanged
    EXPECT_EQ(result.pxLimits.minWidth_, 100);
    EXPECT_EQ(result.pxLimits.maxWidth_, 2000);
    // Height should be intersected
    EXPECT_EQ(result.pxLimits.minHeight_, 1000);  // max(1000, 250)
    EXPECT_EQ(result.pxLimits.maxHeight_, 1500);   // min(2000, 1500)
}

/**
 * @tc.name: CalcSingleWinIntersect05
 * @tc.desc: Test intersection with width only
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalcSingleWinIntersect05,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalcSingleWinIntersect05");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    WindowLimits currentLimits = { 2000, 2000, 100, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits currentLimitsVP = { 1000, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    WindowLimits attachedLimits = { 800, 2200, 150, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    // Only intersect width
    AttachLimitOptions limitOptions{ false, true }; // height=false, width=true
    float virtualPixelRatio = 2.0f;

    auto result = window->CalcSingleWinIntersect(
        currentLimits, currentLimitsVP, attachedLimits, limitOptions, virtualPixelRatio);

    EXPECT_TRUE(result.pxValid);
    EXPECT_TRUE(result.vpValid);
    // Width should be intersected
    EXPECT_EQ(result.pxLimits.minWidth_, 150);  // max(100, 150)
    EXPECT_EQ(result.pxLimits.maxWidth_, 800);   // min(2000, 800)
    // Height should remain unchanged
    EXPECT_EQ(result.pxLimits.minHeight_, 1000);
    EXPECT_EQ(result.pxLimits.maxHeight_, 2000);
}

/**
 * @tc.name: CalcSingleWinIntersect06
 * @tc.desc: Test intersection with both PX and VP invalid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalcSingleWinIntersect06,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalcSingleWinIntersect06");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    WindowLimits currentLimits = { 1000, 500, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits currentLimitsVP = { 500, 250, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    // Attached limits have no intersection with current
    WindowLimits attachedLimits = { 1500, 1500, 150, 800, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    AttachLimitOptions limitOptions{ true, true }; // Enable both height and width limits intersection
    float virtualPixelRatio = 2.0f;

    auto result = window->CalcSingleWinIntersect(
        currentLimits, currentLimitsVP, attachedLimits, limitOptions, virtualPixelRatio);

    EXPECT_FALSE(result.pxValid);
    EXPECT_FALSE(result.vpValid);
}

/**
 * @tc.name: CalcSingleWinIntersect07
 * @tc.desc: Test VP to PX conversion in intersection
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalcSingleWinIntersect07,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalcSingleWinIntersect07");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    WindowLimits currentLimits = { 2000, 2000, 100, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits currentLimitsVP = { 1000, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    // Attached limits in VP, should be converted to PX
    WindowLimits attachedLimits = { 900, 600, 60, 110, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    AttachLimitOptions limitOptions{ true, true }; // Enable both height and width limits intersection
    float virtualPixelRatio = 2.0f;

    auto result = window->CalcSingleWinIntersect(
        currentLimits, currentLimitsVP, attachedLimits, limitOptions, virtualPixelRatio);

    EXPECT_TRUE(result.pxValid);
    EXPECT_TRUE(result.vpValid);
    // VP->PX conversion: {maxWidth=1800, maxHeight=1200, minWidth=120, minHeight=220}
    // Intersection PX: max(100,120)=120, min(2000,1800)=1800, max(1000,220)=1000, min(2000,1200)=1200
    EXPECT_EQ(result.pxLimits.minWidth_, 120);
    EXPECT_EQ(result.pxLimits.maxWidth_, 1800);
    EXPECT_EQ(result.pxLimits.minHeight_, 1000);
    EXPECT_EQ(result.pxLimits.maxHeight_, 1200);
}

/**
 * @tc.name: CalcSingleWinIntersect08
 * @tc.desc: Test PX to VP conversion in intersection
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalcSingleWinIntersect08,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalcSingleWinIntersect08");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    WindowLimits currentLimits = { 2000, 2000, 100, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits currentLimitsVP = { 1000, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    // Attached limits in PX, should be converted to VP
    // maxHeight_=1200 so PX intersection is valid: min(2000,1200)=1200 >= max(1000,220)=1000
    WindowLimits attachedLimits = { 1800, 1200, 120, 220, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    AttachLimitOptions limitOptions{ true, true }; // Enable both height and width limits intersection
    float virtualPixelRatio = 2.0f;

    auto result = window->CalcSingleWinIntersect(
        currentLimits, currentLimitsVP, attachedLimits, limitOptions, virtualPixelRatio);

    EXPECT_TRUE(result.pxValid);
    EXPECT_TRUE(result.vpValid);
    // PX intersection: maxWidth_=min(2000,1800)=1800, maxHeight_=min(2000,1200)=1200
    //                 minWidth_=max(100,120)=120, minHeight_=max(1000,220)=1000
    // VP->PX conversion of attached: {900, 600, 60, 110}
    // VP intersection: maxWidth_=min(1000,900)=900, minWidth_=max(50,60)=60
    EXPECT_EQ(result.vpLimits.minWidth_, 60);
    EXPECT_EQ(result.vpLimits.maxWidth_, 900);
}

/**
 * @tc.name: CalcSingleWinIntersect09
 * @tc.desc: Test intersection with zero virtual pixel ratio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalcSingleWinIntersect09,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalcSingleWinIntersect09");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    WindowLimits currentLimits = { 2000, 2000, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits currentLimitsVP = { 1000, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    WindowLimits attachedLimits = { 1500, 800, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    AttachLimitOptions limitOptions{ true, true }; // Enable both height and width limits intersection
    float virtualPixelRatio = 0.0f; // Invalid ratio

    auto result = window->CalcSingleWinIntersect(
        currentLimits, currentLimitsVP, attachedLimits, limitOptions, virtualPixelRatio);

    // With zero ratio, conversion fails but direct PX intersection should still work
    EXPECT_TRUE(result.pxValid);
}

/**
 * @tc.name: CalculateAttachedWindowLimitsIntersection03
 * @tc.desc: Test with three attached windows (priority order)
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalculateAttachedWindowLimitsIntersection03,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateAttachedWindowLimitsIntersection03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;

    // Simulate three attached windows with different limits
    WindowLimits limits1 = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits limits2 = { 2200, 1200, 250, 350, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits limits3 = { 1800, 900, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetAttachedWindowLimits(1, limits1);
    window->property_->SetAttachedWindowLimits(2, limits2);
    window->property_->SetAttachedWindowLimits(3, limits3);

    // Set limit options for each window
    AttachLimitOptions limitOptions{ true, true };
    window->property_->SetAttachedLimitOptions(1, limitOptions);
    window->property_->SetAttachedLimitOptions(2, limitOptions);
    window->property_->SetAttachedLimitOptions(3, limitOptions);

    WindowLimits newLimits = { 2500, 1500, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits newLimitsVP = { 1250, 750, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    float virtualPixelRatio = 2.0f;

    window->CalculateAttachedWindowLimitsIntersection(newLimits, newLimitsVP, virtualPixelRatio);

    // First window: minWidth=max(100,200)=200, maxWidth=min(2500,2000)=2000
    // Second window: minWidth=max(200,250)=250, maxWidth=min(2000,2200)=2000
    // Third window: minWidth=max(250,150)=250, maxWidth=min(2000,1800)=1800
    EXPECT_EQ(newLimits.minWidth_, 250);
    EXPECT_EQ(newLimits.maxWidth_, 1800);
}

/**
 * @tc.name: CalculateAttachedWindowLimitsIntersection04
 * @tc.desc: Test with no intersect flags set
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalculateAttachedWindowLimitsIntersection04,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateAttachedWindowLimitsIntersection04");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;

    WindowLimits limits1 = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetAttachedWindowLimits(1, limits1);

    // No intersect flags set
    AttachLimitOptions limitOptions{ false, false };
    window->property_->SetAttachedLimitOptions(1, limitOptions);

    WindowLimits newLimits = { 2500, 1500, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits newLimitsVP = { 1250, 750, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    float virtualPixelRatio = 2.0f;

    window->CalculateAttachedWindowLimitsIntersection(newLimits, newLimitsVP, virtualPixelRatio);

    // Limits should remain unchanged since no intersect flags
    EXPECT_EQ(newLimits.minWidth_, 100);
    EXPECT_EQ(newLimits.maxWidth_, 2500); // maxWidth_ unchanged, was maxHeight_ value 1500
}

/**
 * @tc.name: CalculateAttachedWindowLimitsIntersection05
 * @tc.desc: Test with window that has no intersection (skipped)
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalculateAttachedWindowLimitsIntersection05,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateAttachedWindowLimitsIntersection05");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;

    // First window has valid intersection
    WindowLimits limits1 = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    // Second window has no intersection
    WindowLimits limits2 = { 4000, 3000, 2000, 3000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetAttachedWindowLimits(1, limits1);
    window->property_->SetAttachedWindowLimits(2, limits2);

    // Set limit options for both windows
    AttachLimitOptions limitOptions{ true, true };
    window->property_->SetAttachedLimitOptions(1, limitOptions);
    window->property_->SetAttachedLimitOptions(2, limitOptions);

    WindowLimits newLimits = { 2500, 1500, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits newLimitsVP = { 1250, 750, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    float virtualPixelRatio = 2.0f;

    window->CalculateAttachedWindowLimitsIntersection(newLimits, newLimitsVP, virtualPixelRatio);

    // Should only apply first window's limits (second skipped due to invalid intersection)
    EXPECT_EQ(newLimits.minWidth_, 200);
    EXPECT_EQ(newLimits.maxWidth_, 2000);
}

/**
 * @tc.name: CalculateAttachedWindowLimitsIntersection06
 * @tc.desc: Test with mixed PX and VP attached limits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalculateAttachedWindowLimitsIntersection06,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateAttachedWindowLimitsIntersection06");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;

    // First window in PX
    WindowLimits limits1 = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    // Second window in VP
    WindowLimits limits2 = { 800, 400, 80, 120, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    window->property_->SetAttachedWindowLimits(1, limits1);
    window->property_->SetAttachedWindowLimits(2, limits2);

    // Set limit options for both windows
    AttachLimitOptions limitOptions{ true, true };
    window->property_->SetAttachedLimitOptions(1, limitOptions);
    window->property_->SetAttachedLimitOptions(2, limitOptions);

    WindowLimits newLimits = { 2500, 1500, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits newLimitsVP = { 1250, 750, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    float virtualPixelRatio = 2.5f;

    window->CalculateAttachedWindowLimitsIntersection(newLimits, newLimitsVP, virtualPixelRatio);

    // First window PX: minWidth=max(100,200)=200, maxWidth=min(2500,2000)=2000
    // Second window VP->PX: {maxWidth=2000,maxHeight=1000,minWidth=200,minHeight=300}
    // PX: minWidth=max(200,200)=200, maxWidth=min(2000,2000)=2000
    EXPECT_EQ(newLimits.minWidth_, 200);
    EXPECT_EQ(newLimits.maxWidth_, 2000);
}

/**
 * @tc.name: CalculateAttachedWindowLimitsIntersection07
 * @tc.desc: Test with zero virtualPixelRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalculateAttachedWindowLimitsIntersection07,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateAttachedWindowLimitsIntersection07");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;

    // Add attached limits
    WindowLimits limits1 = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetAttachedWindowLimits(1, limits1);

    WindowLimits newLimits = { 2500, 1500, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits newLimitsVP = { 1250, 750, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    float virtualPixelRatio = 0.0f;  // Zero ratio - function should return early

    window->CalculateAttachedWindowLimitsIntersection(newLimits, newLimitsVP, virtualPixelRatio);

    // Limits should remain unchanged due to zero ratio
    EXPECT_EQ(newLimits.minWidth_, 100);
    EXPECT_EQ(newLimits.maxWidth_, 2500); // maxWidth_ unchanged, was maxHeight_ value 1500
    EXPECT_EQ(newLimitsVP.minWidth_, 50);
    EXPECT_EQ(newLimitsVP.maxWidth_, 1250); // maxWidthVP_ unchanged, was maxHeightVP_ value 750
}

/**
 * @tc.name: CalculateAttachedWindowLimitsIntersection08
 * @tc.desc: Test when not in PC or pad free multi-window mode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalculateAttachedWindowLimitsIntersection08,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateAttachedWindowLimitsIntersection08");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    // Set freeMultiWindowEnable to false - not in free multi-window mode
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;

    // Add attached limits
    WindowLimits limits1 = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetAttachedWindowLimits(1, limits1);

    WindowLimits newLimits = { 2500, 1500, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits newLimitsVP = { 1250, 750, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    float virtualPixelRatio = 2.0f;

    window->CalculateAttachedWindowLimitsIntersection(newLimits, newLimitsVP, virtualPixelRatio);

    // Limits should remain unchanged since not in free multi-window mode
    EXPECT_EQ(newLimits.minWidth_, 100);
    EXPECT_EQ(newLimits.maxWidth_, 2500); // maxWidth_ unchanged, was maxHeight_ value 1500
    EXPECT_EQ(newLimitsVP.minWidth_, 50);
    EXPECT_EQ(newLimitsVP.maxWidth_, 1250); // maxWidthVP_ unchanged, was maxHeightVP_ value 750
}

/**
 * @tc.name: CalculateAttachedWindowLimitsIntersection09
 * @tc.desc: Test with sub window (uses windowAnchorInfo instead of property limitOptions)
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, CalculateAttachedWindowLimitsIntersection09,
    Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("CalculateAttachedWindowLimitsIntersection09");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    // Sub window type
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;

    // Set up windowAnchorInfo for sub window
    WindowAnchorInfo anchorInfo;
    anchorInfo.isAnchoredByAttach_ = true;
    anchorInfo.isFromAttachOrDetach_ = true;
    anchorInfo.attachOptions.isIntersectedHeightLimit = true;
    anchorInfo.attachOptions.isIntersectedWidthLimit = false;  // Only limit height
    window->property_->SetWindowAnchorInfo(anchorInfo);

    // Add attached limits (sub window doesn't use per-source limitOptions)
    WindowLimits limits1 = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetAttachedWindowLimits(1, limits1);

    WindowLimits newLimits = { 2500, 1500, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits newLimitsVP = { 1250, 750, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    float virtualPixelRatio = 2.0f;

    window->CalculateAttachedWindowLimitsIntersection(newLimits, newLimitsVP, virtualPixelRatio);

    // Height should be intersected, width should remain unchanged
    EXPECT_EQ(newLimits.minWidth_, 100);   // Width not intersected
    EXPECT_EQ(newLimits.maxWidth_, 2500);  // Width not intersected
    EXPECT_EQ(newLimits.minHeight_, 300);  // Height intersected: max(200, 300)
    EXPECT_EQ(newLimits.maxHeight_, 1000); // Height intersected: min(1500, 1000)
}

/**
 * @tc.name: UpdateWindowSizeLimits01
 * @tc.desc: Test UpdateWindowSizeLimits with needNotifySession=false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, UpdateWindowSizeLimits01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateWindowSizeLimits01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(window, nullptr);

    WindowLimits limits = { 2000, 1200, 300, 400, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetWindowLimits(limits);

    // Call without notification
    window->UpdateWindowSizeLimits(false);

    // Verify limits were updated
    auto updatedLimits = window->property_->GetWindowLimits();
    EXPECT_EQ(updatedLimits.minWidth_, 300);
}

/**
 * @tc.name: UpdateWindowSizeLimits02
 * @tc.desc: Test UpdateWindowSizeLimits with needNotifySession=true and attached windows
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, UpdateWindowSizeLimits02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateWindowSizeLimits02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "UpdateWindowSizeLimits02", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;

    // Set up attached window scenario
    WindowLimits attachedLimits = { 1800, 1000, 250, 350, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetAttachedWindowLimits(1, attachedLimits);
    AttachLimitOptions limitOptions{ true, true }; // isIntersectedHeightLimit, isIntersectedWidthLimit
    window->property_->SetAttachedLimitOptions(1, limitOptions);

    // Test the intersection logic directly (UpdateWindowSizeLimits requires display mock)
    WindowLimits newLimits = { 2500, 1500, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits newLimitsVP = { 1250, 750, 100, 150, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    float virtualPixelRatio = 2.0f;

    window->CalculateAttachedWindowLimitsIntersection(newLimits, newLimitsVP, virtualPixelRatio);

    // Verify limits were intersected with attached window
    EXPECT_EQ(newLimits.minWidth_, 250); // max(200, 250)
}

/**
 * @tc.name: NotifySessionSideLimitsChanged01
 * @tc.desc: Test notification for sub window without attach relationship
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, NotifySessionSideLimitsChanged01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySessionSideLimitsChanged01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    SessionInfo sessionInfo = { "NotifySessionSideLimitsChanged01", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowLimits limitsToNotify = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    // No attach relationship set for sub window
    WindowAnchorInfo anchorInfo;
    anchorInfo.isAnchoredByAttach_ = false;
    window->property_->SetWindowAnchorInfo(anchorInfo);

    // Should not call NotifyAttachedWindowsLimitsChanged since no attach relationship
    EXPECT_CALL(*session, NotifyAttachedWindowsLimitsChanged(testing::_)).Times(0);
    window->NotifySessionSideLimitsChanged(limitsToNotify);
}

/**
 * @tc.name: NotifySessionSideLimitsChanged02
 * @tc.desc: Test notification for sub window with attach relationship
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, NotifySessionSideLimitsChanged02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySessionSideLimitsChanged02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    SessionInfo sessionInfo = { "NotifySessionSideLimitsChanged02", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowLimits limitsToNotify = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    // Set up attach relationship for sub window
    WindowAnchorInfo anchorInfo;
    anchorInfo.isAnchoredByAttach_ = true;
    anchorInfo.attachOptions.isIntersectedHeightLimit = true;
    anchorInfo.attachOptions.isIntersectedWidthLimit = true;
    window->property_->SetWindowAnchorInfo(anchorInfo);

    // User limits in PX
    WindowLimits userLimits = { 2200, 1200, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetUserWindowLimits(userLimits);

    // Should notify with PX limits
    EXPECT_CALL(*session, NotifyAttachedWindowsLimitsChanged(testing::_))
        .Times(1)
        .WillOnce(testing::Return(WSError::WS_OK));
    window->NotifySessionSideLimitsChanged(limitsToNotify);
}

/**
 * @tc.name: NotifySessionSideLimitsChanged03
 * @tc.desc: Test notification for sub window with VP unit user limits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, NotifySessionSideLimitsChanged03, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySessionSideLimitsChanged03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    SessionInfo sessionInfo = { "NotifySessionSideLimitsChanged03", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowLimits limitsToNotify = { 1000, 500, 100, 150, 0.0f, 0.0f, 0.0f, PixelUnit::VP };

    // Set up attach relationship for sub window
    WindowAnchorInfo anchorInfo;
    anchorInfo.isAnchoredByAttach_ = true;
    anchorInfo.attachOptions.isIntersectedHeightLimit = true;
    anchorInfo.attachOptions.isIntersectedWidthLimit = true;
    window->property_->SetWindowAnchorInfo(anchorInfo);

    // User limits in VP
    WindowLimits userLimits = { 1000, 500, 100, 150, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    window->property_->SetUserWindowLimits(userLimits);

    // Should notify with VP limits
    EXPECT_CALL(*session, NotifyAttachedWindowsLimitsChanged(testing::_))
        .Times(1)
        .WillOnce(testing::Return(WSError::WS_OK));
    window->NotifySessionSideLimitsChanged(limitsToNotify);
}

/**
 * @tc.name: NotifySessionSideLimitsChanged04
 * @tc.desc: Test notification for sub window without intersected limits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, NotifySessionSideLimitsChanged04, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySessionSideLimitsChanged04");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    SessionInfo sessionInfo = { "NotifySessionSideLimitsChanged04", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowLimits limitsToNotify = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    // Attach relationship but no intersected limits for sub window
    WindowAnchorInfo anchorInfo;
    anchorInfo.isAnchoredByAttach_ = true;
    anchorInfo.attachOptions.isIntersectedHeightLimit = false;
    anchorInfo.attachOptions.isIntersectedWidthLimit = false;
    window->property_->SetWindowAnchorInfo(anchorInfo);

    // Should return early without notification
    EXPECT_CALL(*session, NotifyAttachedWindowsLimitsChanged(testing::_)).Times(0);
    window->NotifySessionSideLimitsChanged(limitsToNotify);
}

/**
 * @tc.name: NotifySessionSideLimitsChanged05
 * @tc.desc: Test notification for main window with attached sub windows
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, NotifySessionSideLimitsChanged05, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySessionSideLimitsChanged05");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo sessionInfo = { "NotifySessionSideLimitsChanged05", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowLimits limitsToNotify = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    // Add sub window with limit options
    WindowLimits subLimits = { 1500, 800, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetAttachedWindowLimits(100, subLimits);
    AttachLimitOptions limitOptions{ true, true }; // sub window attaches with both height and width limits
    window->property_->SetAttachedLimitOptions(100, limitOptions);

    // Should notify session side
    EXPECT_CALL(*session, NotifyAttachedWindowsLimitsChanged(testing::_))
        .Times(1)
        .WillOnce(testing::Return(WSError::WS_OK));
    window->NotifySessionSideLimitsChanged(limitsToNotify);
}

/**
 * @tc.name: NotifySessionSideLimitsChanged06
 * @tc.desc: Test notification for main window without attached sub windows
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, NotifySessionSideLimitsChanged06, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySessionSideLimitsChanged06");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo sessionInfo = { "NotifySessionSideLimitsChanged06", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    WindowLimits limitsToNotify = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    // No attached sub windows (attachedLimitOptionsList_ is empty)
    // Should not notify session side
    EXPECT_CALL(*session, NotifyAttachedWindowsLimitsChanged(testing::_)).Times(0);
    window->NotifySessionSideLimitsChanged(limitsToNotify);
}

/**
 * @tc.name: NotifySessionSideLimitsChanged07
 * @tc.desc: Test notification when hostSession is null
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, NotifySessionSideLimitsChanged07, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifySessionSideLimitsChanged07");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    // Do NOT set hostSession_ - leave it as nullptr

    WindowLimits limitsToNotify = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    // Add sub window with limit options
    WindowLimits subLimits = { 1500, 800, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetAttachedWindowLimits(100, subLimits);
    AttachLimitOptions limitOptions{ true, true };
    window->property_->SetAttachedLimitOptions(100, limitOptions);

    // Should return early without crashing when hostSession_ is null
    window->NotifySessionSideLimitsChanged(limitsToNotify);

    // Verify early return when hostSession is null
    EXPECT_EQ(window->GetHostSession(), nullptr);
}

/**
 * @tc.name: AttachedWindowLimitsPriority01
 * @tc.desc: Test priority order with vector insertion order
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, AttachedWindowLimitsPriority01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AttachedWindowLimitsPriority01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    // Add limits in specific order
    WindowLimits limits1 = { 2400, 1200, 300, 400, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits limits2 = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits limits3 = { 2200, 1100, 250, 350, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    window->property_->SetAttachedWindowLimits(1, limits1);
    window->property_->SetAttachedWindowLimits(2, limits2);
    window->property_->SetAttachedWindowLimits(3, limits3);

    auto attachedList = window->property_->GetAttachedWindowLimitsList();

    // Verify insertion order is preserved
    EXPECT_EQ(attachedList.size(), 3u);
    EXPECT_EQ(attachedList[0].first, 1);
    EXPECT_EQ(attachedList[1].first, 2);
    EXPECT_EQ(attachedList[2].first, 3);
}

/**
 * @tc.name: AttachedWindowLimitsPriority02
 * @tc.desc: Test priority with existing sourceId update
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, AttachedWindowLimitsPriority02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("AttachedWindowLimitsPriority02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    // Add limits
    WindowLimits limits1 = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits limits2 = { 2200, 1100, 250, 350, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetAttachedWindowLimits(1, limits1);
    window->property_->SetAttachedWindowLimits(2, limits2);

    // Update existing sourceId 1
    WindowLimits limits1Updated = { 2100, 1050, 220, 330, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->property_->SetAttachedWindowLimits(1, limits1Updated);

    auto attachedList = window->property_->GetAttachedWindowLimitsList();

    // Should still be 2 entries, with sourceId 1 at the beginning
    EXPECT_EQ(attachedList.size(), 2u);
    EXPECT_EQ(attachedList[0].first, 1);
    EXPECT_EQ(attachedList[0].second.minWidth_, 220); // Updated value
}

/**
 * @tc.name: GetGlobalScaledRect
 * @tc.desc: GetGlobalScaledRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, GetGlobalScaledRect, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetGlobalScaledRect");
    option->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    const int32_t windowId = 2025;
    window->property_->SetPersistentId(windowId);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    HookWindowInfo hookWindowInfo;
    hookWindowInfo.enableHookWindow = true;
    hookWindowInfo.widthHookRatio = 0.5f;
    window->SetAppHookWindowInfo(hookWindowInfo);
    Rect globalScaledRect = { 0, 0, 800, 800 };
    EXPECT_CALL(*session, GetGlobalScaledRect(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    WMError res = window->GetGlobalScaledRect(globalScaledRect);
    EXPECT_EQ(res, WMError::WM_OK);
    EXPECT_NE(globalScaledRect.width_, 800);
}
/**
 * @tc.name: SyncAllAttachedLimitsToChild01
 * @tc.desc: Test SyncAllAttachedLimitsToChild stores all limits and options
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SyncAllAttachedLimitsToChild01, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SyncAllAttachedLimitsToChild01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "SyncAllAttachedLimitsToChild01", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    std::vector<std::pair<int32_t, WindowLimits>> limitsList;
    std::vector<std::pair<int32_t, AttachLimitOptions>> optionsList;

    // Parent's limits (sourceId=100)
    limitsList.emplace_back(100, WindowLimits{ 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    optionsList.emplace_back(100, AttachLimitOptions{ true, true });

    // Sub-window's limits (sourceId=200)
    limitsList.emplace_back(200, WindowLimits{ 1500, 800, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    optionsList.emplace_back(200, AttachLimitOptions{ true, false });

    WSError res = window->SyncAllAttachedLimitsToChild(limitsList, optionsList);
    EXPECT_EQ(res, WSError::WS_OK);

    // Verify limits were stored
    auto storedLimits = window->property_->GetAttachedWindowLimitsList();
    EXPECT_EQ(storedLimits.size(), 2u);

    // Verify options were stored
    auto storedOptions = window->property_->GetAttachedLimitOptionsList();
    EXPECT_EQ(storedOptions.size(), 2u);
}

/**
 * @tc.name: SyncAllAttachedLimitsToChild02
 * @tc.desc: Test SyncAllAttachedLimitsToChild clears existing limits first
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SyncAllAttachedLimitsToChild02, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SyncAllAttachedLimitsToChild02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "SyncAllAttachedLimitsToChild02", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    // Pre-populate with existing limits
    WindowLimits oldLimits = { 3000, 2000, 500, 600, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->UpdateAttachedWindowLimits(999, oldLimits, true, true);
    EXPECT_EQ(window->property_->GetAttachedWindowLimitsList().size(), 1u);

    // Sync new limits - should clear old and store new
    std::vector<std::pair<int32_t, WindowLimits>> limitsList;
    std::vector<std::pair<int32_t, AttachLimitOptions>> optionsList;
    limitsList.emplace_back(100, WindowLimits{ 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    optionsList.emplace_back(100, AttachLimitOptions{ true, true });

    WSError res = window->SyncAllAttachedLimitsToChild(limitsList, optionsList);
    EXPECT_EQ(res, WSError::WS_OK);

    // Verify old limits were replaced, not accumulated
    auto storedLimits = window->property_->GetAttachedWindowLimitsList();
    EXPECT_EQ(storedLimits.size(), 1u);

    // Verify old sourceId 999 is gone
    bool foundOld = false;
    for (const auto& [id, limits] : storedLimits) {
        if (id == 999) foundOld = true;
    }
    EXPECT_FALSE(foundOld);
}

/**
 * @tc.name: SyncAllAttachedLimitsToChild03
 * @tc.desc: Test SyncAllAttachedLimitsToChild with empty lists clears all
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SyncAllAttachedLimitsToChild03, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SyncAllAttachedLimitsToChild03");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "SyncAllAttachedLimitsToChild03", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    // Pre-populate with existing limits
    WindowLimits oldLimits = { 3000, 2000, 500, 600, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    window->UpdateAttachedWindowLimits(888, oldLimits, true, true);
    EXPECT_EQ(window->property_->GetAttachedWindowLimitsList().size(), 1u);

    // Sync with empty lists - should clear everything
    std::vector<std::pair<int32_t, WindowLimits>> limitsList;
    std::vector<std::pair<int32_t, AttachLimitOptions>> optionsList;

    WSError res = window->SyncAllAttachedLimitsToChild(limitsList, optionsList);
    EXPECT_EQ(res, WSError::WS_OK);

    EXPECT_EQ(window->property_->GetAttachedWindowLimitsList().size(), 0u);
    EXPECT_EQ(window->property_->GetAttachedLimitOptionsList().size(), 0u);
}

/**
 * @tc.name: SyncAllAttachedLimitsToChild04
 * @tc.desc: Test SyncAllAttachedLimitsToChild with VP unit limits
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneSessionImplLayoutTest, SyncAllAttachedLimitsToChild04, Function | SmallTest | Level2)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SyncAllAttachedLimitsToChild04");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(window, nullptr);

    SessionInfo sessionInfo = { "SyncAllAttachedLimitsToChild04", "Module", "Ability" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;

    std::vector<std::pair<int32_t, WindowLimits>> limitsList;
    std::vector<std::pair<int32_t, AttachLimitOptions>> optionsList;

    limitsList.emplace_back(300, WindowLimits{ 800, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP });
    optionsList.emplace_back(300, AttachLimitOptions{ false, true });

    WSError res = window->SyncAllAttachedLimitsToChild(limitsList, optionsList);
    EXPECT_EQ(res, WSError::WS_OK);

    auto storedLimits = window->property_->GetAttachedWindowLimitsList();
    EXPECT_EQ(storedLimits.size(), 1u);
    for (const auto& [id, limits] : storedLimits) {
        if (id == 300) {
            EXPECT_EQ(limits.pixelUnit_, PixelUnit::VP);
        }
    }
}

} // namespace
} // namespace Rosen
} // namespace OHOS
