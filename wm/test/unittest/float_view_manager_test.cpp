/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include "float_view_controller.h"
#include "float_view_manager.h"
#include "float_view_option.h"
#include "window.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class MockFloatViewController : public FloatViewController {
public:
    MockFloatViewController(const FvOption& option, napi_env env) : FloatViewController(option, env) {}
    ~MockFloatViewController() override = default;

    MOCK_METHOD1(StopFloatView, WMError(const std::string& reason));
    MOCK_METHOD3(SyncWindowInfo, void(uint32_t windowId, const FloatViewWindowInfo& windowInfo,
        const std::string& reason));
    MOCK_METHOD2(SyncLimits, void(uint32_t windowId, const FloatViewLimits& limits));
    MOCK_METHOD1(ChangeState, void(const FvWindowState &newState));
};

class MockWindow : public Window {
public:
    WindowState state_ = WindowState::STATE_INITIAL;
    const uint32_t mockWindowId_ = 101;
    MockWindow() {};
    ~MockWindow() {};
    MOCK_METHOD3(Show, WMError(uint32_t reason, bool withAnimation, bool withFocus));
    MOCK_METHOD1(Destroy, WMError(uint32_t reason));
    uint32_t GetWindowId() const override
    {
        return mockWindowId_;
    }

    void SetWindowState(const WindowState& state)
    {
        state_ = state;
    }

    WindowState GetWindowState() const override
    {
        return state_;
    }
};

class FloatViewManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<FloatViewController> fvController_;
    sptr<FvOption> option_;
    sptr<MockWindow> mw_;
};

void FloatViewManagerTest::SetUpTestCase()
{
}

void FloatViewManagerTest::TearDownTestCase()
{
}

void FloatViewManagerTest::SetUp()
{
    option_ = sptr<FvOption>::MakeSptr();
    ASSERT_NE(nullptr, option_);
    fvController_ = sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    ASSERT_NE(nullptr, fvController_);
    mw_ = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw_);
}

void FloatViewManagerTest::TearDown()
{
    FloatViewManager::RemoveActiveController(fvController_);
    fvController_ = nullptr;
    option_ = nullptr;
}

namespace {
/**
 * @tc.name: HasActiveController
 * @tc.desc: Test HasActiveController with and without active controller
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, HasActiveController, TestSize.Level1)
{
    EXPECT_FALSE(FloatViewManager::HasActiveController());

    FloatViewManager::SetActiveController(fvController_);
    EXPECT_TRUE(FloatViewManager::HasActiveController());
    FloatViewManager::RemoveActiveController(fvController_);
}

/**
 * @tc.name: IsActiveController
 * @tc.desc: Test IsActiveController with various scenarios
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, IsActiveController, TestSize.Level1)
{
    EXPECT_FALSE(FloatViewManager::IsActiveController(fvController_));

    FloatViewManager::SetActiveController(fvController_);
    EXPECT_TRUE(FloatViewManager::IsActiveController(fvController_));

    auto otherController = sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    EXPECT_FALSE(FloatViewManager::IsActiveController(otherController));

    FloatViewManager::RemoveActiveController(fvController_);
}

/**
 * @tc.name: SetActiveController
 * @tc.desc: Test SetActiveController with null and valid controller
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, SetActiveController, TestSize.Level1)
{
    wptr<FloatViewController> nullController;
    FloatViewManager::SetActiveController(nullController);
    EXPECT_FALSE(FloatViewManager::HasActiveController());

    FloatViewManager::SetActiveController(fvController_);
    EXPECT_TRUE(FloatViewManager::HasActiveController());
    EXPECT_TRUE(FloatViewManager::IsActiveController(fvController_));
    FloatViewManager::RemoveActiveController(fvController_);
}

/**
 * @tc.name: RemoveActiveController
 * @tc.desc: Test RemoveActiveController with various scenarios
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, RemoveActiveController, TestSize.Level1)
{
    FloatViewManager::RemoveActiveController(fvController_);
    EXPECT_FALSE(FloatViewManager::HasActiveController());

    FloatViewManager::SetActiveController(fvController_);
    EXPECT_TRUE(FloatViewManager::HasActiveController());
    FloatViewManager::RemoveActiveController(fvController_);
    EXPECT_FALSE(FloatViewManager::HasActiveController());

    FloatViewManager::SetActiveController(fvController_);
    auto otherController = sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    FloatViewManager::RemoveActiveController(otherController);
    EXPECT_TRUE(FloatViewManager::HasActiveController());
    FloatViewManager::RemoveActiveController(fvController_);
}

/**
 * @tc.name: GetActiveController
 * @tc.desc: Test GetActiveController with and without active controller
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, GetActiveController, TestSize.Level1)
{
    auto controller = FloatViewManager::GetActiveController();
    EXPECT_EQ(nullptr, controller);

    FloatViewManager::SetActiveController(fvController_);
    controller = FloatViewManager::GetActiveController();
    EXPECT_NE(nullptr, controller);
    EXPECT_EQ(fvController_.GetRefPtr(), controller.GetRefPtr());
    FloatViewManager::RemoveActiveController(fvController_);
}

/**
 * @tc.name: DoActionHide
 * @tc.desc: Test DoActionHide with and without active controller
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, DoActionHide, TestSize.Level1)
{
    FloatViewManager::DoActionHide("test_reason");

    FloatViewManager::SetActiveController(fvController_);
    FloatViewManager::DoActionHide("test_reason");
    EXPECT_EQ(FvWindowState::FV_STATE_HIDDEN, fvController_->GetCurState());
    FloatViewManager::RemoveActiveController(fvController_);
}

/**
 * @tc.name: DoActionInSidebar
 * @tc.desc: Test DoActionInSidebar with and without active controller
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, DoActionInSidebar, TestSize.Level1)
{
    FloatViewManager::DoActionInSidebar("test_reason");

    FloatViewManager::SetActiveController(fvController_);
    FloatViewManager::DoActionInSidebar("test_reason");
    EXPECT_EQ(FvWindowState::FV_STATE_IN_SIDEBAR, fvController_->GetCurState());
    FloatViewManager::RemoveActiveController(fvController_);
}

/**
 * @tc.name: DoActionInFloatingBall
 * @tc.desc: Test DoActionInFloatingBall with and without active controller
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, DoActionInFloatingBall, TestSize.Level1)
{
    FloatViewManager::DoActionInFloatingBall("test_reason");

    FloatViewManager::SetActiveController(fvController_);
    FloatViewManager::DoActionInFloatingBall("test_reason");
    EXPECT_EQ(FvWindowState::FV_STATE_IN_FLOATING_BALL, fvController_->GetCurState());
    fvController_->SetBindState(true);
    FloatViewManager::DoActionInFloatingBall("test_reason");
    EXPECT_EQ(FvWindowState::FV_STATE_IN_FLOATING_BALL, fvController_->GetCurState());
    FloatViewManager::RemoveActiveController(fvController_);
}

/**
 * @tc.name: ControllerLifecycleTest
 * @tc.desc: Test controller lifecycle
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, ControllerLifecycleTest, TestSize.Level1)
{
    EXPECT_FALSE(FloatViewManager::HasActiveController());

    FloatViewManager::SetActiveController(fvController_);
    EXPECT_TRUE(FloatViewManager::HasActiveController());
    EXPECT_TRUE(FloatViewManager::IsActiveController(fvController_));

    FloatViewManager::RemoveActiveController(fvController_);
    EXPECT_FALSE(FloatViewManager::HasActiveController());
}

/**
 * @tc.name: RemoveActiveControllerBranchCoverage
 * @tc.desc: Test RemoveActiveController branch coverage
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, RemoveActiveControllerBranchCoverage, TestSize.Level1)
{
    auto otherController = sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    FloatViewManager::RemoveActiveController(otherController);
    EXPECT_EQ(nullptr, FloatViewManager::activeController_);

    FloatViewManager::RemoveActiveController(fvController_);
    EXPECT_EQ(nullptr, FloatViewManager::activeController_);
}

/**
 * @tc.name: DoActionEvent
 * @tc.desc: DoActionEvent (invalid action branch + valid action branch)
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, DoActionEvent, TestSize.Level1)
{
    // branch: action not in map => return
    FloatViewManager::DoActionEvent("not_exist_action", "ut");

    // branch: action in map => dispatch (use "hide" to avoid depending on StopFloatView)
    FloatViewManager::SetActiveController(fvController_);
    FloatViewManager::DoActionEvent("hide", "ut");
    EXPECT_EQ(FvWindowState::FV_STATE_HIDDEN, fvController_->GetCurState());
    FloatViewManager::RemoveActiveController(fvController_);
}

/**
 * @tc.name: DoActionStart
 * @tc.desc: DoActionStart with and without active controller
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, DoActionStart, TestSize.Level1)
{
    // branch: no active controller => do nothing
    FloatViewManager::DoActionStart();

    // branch: has active controller => call controller->StopFloatView
    auto mockController = sptr<MockFloatViewController>::MakeSptr(*option_, nullptr);
    ASSERT_NE(nullptr, mockController);

    FloatViewManager::SetActiveController(mockController);
    FloatViewManager::DoActionStart();
    EXPECT_EQ(FvWindowState::FV_STATE_STARTED, mockController->GetCurState());
    mockController->SetBindState(true);
    FloatViewManager::DoActionStart();
    EXPECT_EQ(FvWindowState::FV_STATE_STARTED, mockController->GetCurState());
    FloatViewManager::RemoveActiveController(mockController);
}

/**
 * @tc.name: DoActionClose
 * @tc.desc: DoActionClose with and without active controller
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, DoActionClose, TestSize.Level1)
{
    // branch: no active controller => do nothing
    FloatViewManager::DoActionClose("ut");

    // branch: has active controller => call controller->StopFloatView
    auto mockController = sptr<MockFloatViewController>::MakeSptr(*option_, nullptr);
    ASSERT_NE(nullptr, mockController);

    FloatViewManager::SetActiveController(mockController);
    FloatViewManager::DoActionClose("ut");
    EXPECT_EQ(FvWindowState::FV_STATE_UNDEFINED, mockController->GetCurState());
    FloatViewManager::RemoveActiveController(mockController);
}

/**
 * @tc.name: SyncFvWindowInfo
 * @tc.desc: SyncFvWindowInfo with and without active controller
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, SyncFvLimitsAndWindowInfo, TestSize.Level1)
{
    uint32_t windowId = 1;
    FloatViewWindowInfo windowInfo;
    windowInfo.scale_ = 1.1f;
    std::string reason = "ut";
    std::map<uint32_t, FloatViewLimits> limitsInfo;
    FloatViewLimits limit;
    limit.maxHeight_ = 1;
    limitsInfo.emplace(0, limit);
    // branch: no active controller => do nothing
    FloatViewManager::SyncFvLimits(windowId, limitsInfo);
    FloatViewManager::SyncFvWindowInfo(windowId, windowInfo, reason);

    // branch: has active controller => call controller->SyncWindowInfo
    auto mockController = sptr<MockFloatViewController>::MakeSptr(*option_, nullptr);
    ASSERT_NE(nullptr, mockController);
    mockController->window_ = mw_;
    FloatViewManager::SetActiveController(mockController);
    FloatViewManager::SyncFvLimits(mw_->GetWindowId(), limitsInfo);
    FloatViewManager::SyncFvWindowInfo(mw_->GetWindowId(), windowInfo, reason);
    FloatViewManager::RemoveActiveController(mockController);
    EXPECT_EQ(windowInfo.scale_, mockController->GetWindowInfo().scale_);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
