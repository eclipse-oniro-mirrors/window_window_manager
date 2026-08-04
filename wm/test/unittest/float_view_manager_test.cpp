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
    MOCK_METHOD5(Show, WMError(uint32_t reason, bool withAnimation, bool withFocus,
        int32_t requestId, int32_t scbRequestId));
    MOCK_METHOD2(Destroy, WMError(uint32_t reason, bool isFromInnerkits));
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
 * @tc.name: RemoveController
 * @tc.desc: Test RemoveController
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, RemoveController, TestSize.Level1)
{
    uint32_t windowId = 10;
    FloatViewManager::windowId2Controller_.clear();
    FloatViewManager::AddController(windowId, fvController_);
    EXPECT_EQ(1, FloatViewManager::windowId2Controller_.size());
    FloatViewManager::RemoveController(windowId + 1);
    EXPECT_EQ(1, FloatViewManager::windowId2Controller_.size());
    FloatViewManager::RemoveController(windowId);
    EXPECT_EQ(0, FloatViewManager::windowId2Controller_.size());
    FloatViewManager::windowId2Controller_.clear();
}

/**
 * @tc.name: GetController
 * @tc.desc: Test GetController
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, GetController, TestSize.Level1)
{
    uint32_t windowId = 10;
    FloatViewManager::windowId2Controller_.clear();
    FloatViewManager::AddController(windowId, fvController_);
    EXPECT_NE(nullptr, FloatViewManager::GetController(windowId).promote());
    EXPECT_EQ(nullptr, FloatViewManager::GetController(windowId + 1).promote());
    FloatViewManager::windowId2Controller_.clear();
}

/**
 * @tc.name: DoActionHide
 * @tc.desc: Test DoActionHide with and without controller in map
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, DoActionHide, TestSize.Level1)
{
    uint32_t windowId = 10;
    FloatViewManager::DoActionHide(windowId, "test_reason");
    FloatViewManager::windowId2Controller_.clear();
    FloatViewManager::AddController(windowId, fvController_);
    FloatViewManager::DoActionHide(windowId, "test_reason");
    EXPECT_EQ(FvWindowState::FV_STATE_HIDDEN, fvController_->GetCurState());
    FloatViewManager::windowId2Controller_.clear();
}

/**
 * @tc.name: DoActionInSidebar
 * @tc.desc: Test DoActionInSidebar with and without controller in map
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, DoActionInSidebar, TestSize.Level1)
{
    uint32_t windowId = 10;
    FloatViewManager::DoActionInSidebar(windowId, "test_reason");
    FloatViewManager::windowId2Controller_.clear();
    FloatViewManager::AddController(windowId, fvController_);
    FloatViewManager::DoActionInSidebar(windowId, "test_reason");
    EXPECT_EQ(FvWindowState::FV_STATE_IN_SIDEBAR, fvController_->GetCurState());
    FloatViewManager::windowId2Controller_.clear();
}

/**
 * @tc.name: DoActionInFloatingBall
 * @tc.desc: Test DoActionInFloatingBall with and without controller in map
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, DoActionInFloatingBall, TestSize.Level1)
{
    uint32_t windowId = 10;
    FloatViewManager::DoActionInFloatingBall(windowId, "test_reason");
    FloatViewManager::windowId2Controller_.clear();
    FloatViewManager::AddController(windowId, fvController_);
    FloatViewManager::DoActionInFloatingBall(windowId, "test_reason");
    EXPECT_EQ(FvWindowState::FV_STATE_IN_FLOATING_BALL, fvController_->GetCurState());
    fvController_->SetBindState(true);
    FloatViewManager::DoActionInFloatingBall(windowId, "test_reason");
    EXPECT_EQ(FvWindowState::FV_STATE_IN_FLOATING_BALL, fvController_->GetCurState());
    FloatViewManager::windowId2Controller_.clear();
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
    uint32_t windowId = 10;
    FloatViewManager::DoActionEvent(windowId, "not_exist_action", "ut");
    FloatViewManager::windowId2Controller_.clear();
    FloatViewManager::AddController(windowId, fvController_);
    FloatViewManager::DoActionEvent(windowId, "hide", "ut");
    EXPECT_EQ(FvWindowState::FV_STATE_HIDDEN, fvController_->GetCurState());
    FloatViewManager::windowId2Controller_.clear();
}

/**
 * @tc.name: DoActionStart
 * @tc.desc: DoActionStart with and without controller in map
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, DoActionStart, TestSize.Level1)
{
    uint32_t windowId = 10;
    FloatViewManager::DoActionStart(windowId + 1);

    auto mockController = sptr<MockFloatViewController>::MakeSptr(*option_, nullptr);
    ASSERT_NE(nullptr, mockController);

    FloatViewManager::windowId2Controller_.clear();
    FloatViewManager::AddController(windowId, mockController);
    FloatViewManager::DoActionStart(windowId);
    EXPECT_EQ(FvWindowState::FV_STATE_STARTED, mockController->GetCurState());
    mockController->SetBindState(true);
    FloatViewManager::DoActionStart(windowId);
    EXPECT_EQ(FvWindowState::FV_STATE_STARTED, mockController->GetCurState());
    FloatViewManager::windowId2Controller_.clear();
}

/**
 * @tc.name: DoActionCloseByMainWindow
 * @tc.desc: DoActionCloseByMainWindow with and without controller in map
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, DoActionCloseByMainWindow, TestSize.Level1)
{
    auto mockController1 = sptr<MockFloatViewController>::MakeSptr(*option_, nullptr);
    ASSERT_NE(nullptr, mockController1);
    mockController1->mainWindowId_ = 10;
    mockController1->window_ = mw_;
    auto mockController2 = sptr<MockFloatViewController>::MakeSptr(*option_, nullptr);
    ASSERT_NE(nullptr, mockController2);
    mockController2->mainWindowId_ = 10;
    mockController2->window_ = mw_;
    auto mockController3 = sptr<MockFloatViewController>::MakeSptr(*option_, nullptr);
    ASSERT_NE(nullptr, mockController3);
    mockController3->mainWindowId_ = 20;
    mockController3->window_ = mw_;
    FloatViewManager::windowId2Controller_.clear();
    FloatViewManager::AddController(101, mockController1);
    FloatViewManager::AddController(102, mockController2);
    FloatViewManager::AddController(103, nullptr);
    FloatViewManager::AddController(201, mockController3);

    FloatViewManager::DoActionCloseByMainWindow(10, "ut");
    EXPECT_EQ(FvWindowState::FV_STATE_STOPPED, mockController1->GetCurState());
    EXPECT_EQ(FvWindowState::FV_STATE_STOPPED, mockController2->GetCurState());
    EXPECT_EQ(FvWindowState::FV_STATE_UNDEFINED, mockController3->GetCurState());
    FloatViewManager::windowId2Controller_.clear();
}

/**
 * @tc.name: DoActionClose
 * @tc.desc: DoActionClose with and without controller in map
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewManagerTest, DoActionClose, TestSize.Level1)
{
    uint32_t windowId = 10;
    FloatViewManager::DoActionClose(windowId, "ut");

    auto mockController = sptr<MockFloatViewController>::MakeSptr(*option_, nullptr);
    ASSERT_NE(nullptr, mockController);

    FloatViewManager::windowId2Controller_.clear();
    FloatViewManager::AddController(windowId, mockController);
    FloatViewManager::DoActionClose(windowId, "ut");
    FloatViewManager::windowId2Controller_.clear();
}

/**
 * @tc.name: SyncFvWindowInfo
 * @tc.desc: SyncFvWindowInfo with and without controller in map
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
    FloatViewManager::SyncFvLimits(windowId, limitsInfo);
    FloatViewManager::SyncFvWindowInfo(windowId, windowInfo, reason);

    auto mockController = sptr<MockFloatViewController>::MakeSptr(*option_, nullptr);
    ASSERT_NE(nullptr, mockController);
    mockController->window_ = mw_;
    FloatViewManager::windowId2Controller_.clear();
    FloatViewManager::AddController(mw_->GetWindowId(), mockController);
    FloatViewManager::SyncFvLimits(mw_->GetWindowId(), limitsInfo);
    FloatViewManager::SyncFvWindowInfo(mw_->GetWindowId(), windowInfo, reason);
    FloatViewManager::windowId2Controller_.clear();
    EXPECT_EQ(windowInfo.scale_, mockController->GetWindowInfo().scale_);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
