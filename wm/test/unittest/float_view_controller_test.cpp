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
#include "ability_context_impl.h"
#include "float_view_controller.h"
#include "float_window_manager.h"
#include "native_engine/native_engine.h"
#include "window.h"
#include "wm_common.h"
#include "float_view_manager.h"
#include "window_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class MockWindow : public Window {
public:
    WindowState state_ = WindowState::STATE_INITIAL;
    const uint32_t mockWindowId_ = 101;
    MockWindow() {};
    ~MockWindow() {};
    MOCK_METHOD3(Show, WMError(uint32_t reason, bool withAnimation, bool withFocus));
    MOCK_METHOD1(Destroy, WMError(uint32_t reason));
    MOCK_METHOD0(NotifyPrepareCloseFloatView, void());
    MOCK_METHOD1(UpdateFloatView, WMError(const FloatViewTemplateInfo& fvTemplateInfo));
    MOCK_METHOD1(RestoreFloatViewMainWindow, WMError(const std::shared_ptr<AAFwk::WantParams>& wantParams));
    MOCK_METHOD6(NapiSetUIContent, WMError(const std::string& contentUrl, napi_env env, napi_value storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability));
    MOCK_METHOD6(AniSetUIContent, WMError(const std::string& contentUrl, ani_env* env, ani_object storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability));
    MOCK_METHOD1(RegisterLifeCycleListener, WMError(const sptr<IWindowLifeCycle>& listener));
    MOCK_METHOD1(UnregisterLifeCycleListener, WMError(const sptr<IWindowLifeCycle>& listener));
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

class MockStateChangeListener : public IFvStateChangeObserver {
public:
    void OnStateChange(const FloatViewState& state, const std::string& stopReason) override
    {
        return;
    }
};

class MockRectChangeListener : public IFvRectChangeObserver {
public:
    void OnRectangleChange(const Rect& rect, double scale, const std::string& reason) override
    {
        return;
    }
};

class MockLimitsChangeListener : public IFvLimitsChangeObserver {
public:
    uint32_t maxHeight_  = 0;
    void OnLimitsChange(const FloatViewLimits& limits) override
    {
        maxHeight_ = limits.maxHeight_;
        return;
    }
};

class FloatViewControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<FloatViewController> fvController_;
    sptr<FvOption> option_;
    sptr<MockWindow> mw_;
};

void FloatViewControllerTest::SetUpTestCase()
{
}

void FloatViewControllerTest::TearDownTestCase()
{
}

void FloatViewControllerTest::SetUp()
{
    mw_ = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw_);
    option_ = sptr<FvOption>::MakeSptr();
    ASSERT_NE(nullptr, option_);
    fvController_ = sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    ASSERT_NE(nullptr, fvController_);
}

void FloatViewControllerTest::TearDown()
{
    fvController_ = nullptr;
    option_ = nullptr;
    mw_ = nullptr;
}

namespace {
/**
 * @tc.name: ChangeState
 * @tc.desc: ChangeState with all valid states
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, ChangeState, TestSize.Level1)
{
    FvWindowState state = fvController_->GetCurState();
    EXPECT_EQ(FvWindowState::FV_STATE_UNDEFINED, state);
    auto listener = sptr<MockStateChangeListener>::MakeSptr();
    fvController_->stateChangeObservers_.push_back(nullptr);
    fvController_->stateChangeObservers_.push_back(listener);
    fvController_->ChangeState(FvWindowState::FV_STATE_STARTED);
    EXPECT_EQ(FvWindowState::FV_STATE_STARTED, fvController_->GetCurState());
    fvController_->ChangeState(FvWindowState::FV_STATE_STOPPED);
    EXPECT_EQ(FvWindowState::FV_STATE_STOPPED, fvController_->GetCurState());
    fvController_->ChangeState(FvWindowState::FV_STATE_STARTING);
    EXPECT_EQ(FvWindowState::FV_STATE_STARTING, fvController_->GetCurState());
    fvController_->ChangeState(FvWindowState::FV_STATE_IN_SIDEBAR);
    EXPECT_EQ(FvWindowState::FV_STATE_IN_SIDEBAR, fvController_->GetCurState());
    fvController_->ChangeState(FvWindowState::FV_STATE_IN_FLOATING_BALL);
    EXPECT_EQ(FvWindowState::FV_STATE_IN_FLOATING_BALL, fvController_->GetCurState());
}

/**
 * @tc.name: IsStateWithWindow
 * @tc.desc: IsStateWithWindow
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, IsStateWithWindow, TestSize.Level1)
{
    EXPECT_TRUE(fvController_->IsStateWithWindow(FvWindowState::FV_STATE_STARTED));
    EXPECT_TRUE(fvController_->IsStateWithWindow(FvWindowState::FV_STATE_HIDDEN));
    EXPECT_TRUE(fvController_->IsStateWithWindow(FvWindowState::FV_STATE_IN_SIDEBAR));
    EXPECT_TRUE(fvController_->IsStateWithWindow(FvWindowState::FV_STATE_IN_FLOATING_BALL));
    EXPECT_FALSE(fvController_->IsStateWithWindow(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_FALSE(fvController_->IsStateWithWindow(FvWindowState::FV_STATE_STOPPED));
}

/**
 * @tc.name: SetBindState
 * @tc.desc: SetBindState
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, SetBindState, TestSize.Level1)
{
    fvController_->SetBindState(true);
    EXPECT_TRUE(fvController_->IsBind());
    fvController_->SetBindState(false);
    EXPECT_FALSE(fvController_->IsBind());
}

/**
 * @tc.name: StartFloatView
 * @tc.desc: StartFloatView with various states
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, StartFloatView, TestSize.Level1)
{
    fvController_->SetBindState(true);
    EXPECT_NE(WMError::WM_OK, fvController_->StartFloatView());

    fvController_->SetBindState(false);
    EXPECT_NE(WMError::WM_OK, fvController_->StartFloatView());
}

/**
 * @tc.name: StartFloatViewSingle
 * @tc.desc: StartFloatViewSingle with various scenarios
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, StartFloatViewSingle, TestSize.Level1)
{
    fvController_->ChangeState(FvWindowState::FV_STATE_STARTING);
    EXPECT_EQ(WMError::WM_ERROR_FV_REPEAT_OPERATION, fvController_->StartFloatViewSingle());

    fvController_->ChangeState(FvWindowState::FV_STATE_STARTED);
    EXPECT_EQ(WMError::WM_ERROR_FV_REPEAT_OPERATION, fvController_->StartFloatViewSingle());

    fvController_->ChangeState(FvWindowState::FV_STATE_STOPPING);
    EXPECT_EQ(WMError::WM_ERROR_FV_INVALID_STATE, fvController_->StartFloatViewSingle());

    auto otherController = sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    FloatViewManager::SetActiveController(otherController);
    EXPECT_EQ(WMError::WM_ERROR_FV_START_FAILED, fvController_->StartFloatViewSingle());
    FloatViewManager::SetActiveController(fvController_);
    EXPECT_EQ(WMError::WM_ERROR_FV_INVALID_STATE, fvController_->StartFloatViewSingle());

    EXPECT_NE(WMError::WM_OK, fvController_->StartFloatViewSingle(false));
    EXPECT_NE(WMError::WM_OK, fvController_->StartFloatViewSingle(true));
}

/**
 * @tc.name: CreateFloatViewWindow
 * @tc.desc: Test CreateFloatViewWindow with various scenarios
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, CreateFloatViewWindow, TestSize.Level1)
{
    void* invalidContext = nullptr;
    option_->SetContext(invalidContext);
    fvController_ = sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, fvController_->CreateFloatViewWindow());

    std::shared_ptr<AbilityRuntime::Context> context = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    fvController_->option_.contextPtr_ = &context;
    EXPECT_EQ(WMError::WM_ERROR_FV_START_FAILED, fvController_->CreateFloatViewWindow());

    WindowSessionImpl::windowSessionMap_.clear();
    sptr<WindowOption> windowOption = sptr<WindowOption>::MakeSptr();
    windowOption->SetWindowName("IsAnyWindowMatchState");
    sptr<WindowSessionImpl> windowSession = sptr<WindowSessionImpl>::MakeSptr(windowOption);
    WindowSessionImpl::windowSessionMap_.insert(std::make_pair("test",
        std::pair<uint64_t, sptr<WindowSessionImpl>>(1, windowSession)));
    windowSession->state_ = WindowState::STATE_SHOWN;

    FloatWindowManager::pipCnt_ = 1;
    EXPECT_NE(WMError::WM_OK, fvController_->CreateFloatViewWindow());

    Rect rect {0, 0, 100, 100};
    fvController_->option_.SetRect(rect);
    EXPECT_NE(WMError::WM_OK, fvController_->CreateFloatViewWindow());
    FloatWindowManager::pipCnt_ = 0;
}

/**
 * @tc.name: StopFloatViewFromClient
 * @tc.desc: StopFloatViewFromClient with various states
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, StopFloatViewFromClient, TestSize.Level1)
{
    fvController_->SetBindState(true);
    EXPECT_NE(WMError::WM_OK, fvController_->StopFloatViewFromClient());

    fvController_->SetBindState(false);
    fvController_->ChangeState(FvWindowState::FV_STATE_STARTED);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, fvController_->StopFloatViewFromClient());
}

/**
 * @tc.name: StopFloatViewFromClientSingle
 * @tc.desc: StopFloatViewFromClientSingle with various states
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, StopFloatViewFromClientSingle, TestSize.Level1)
{
    fvController_->ChangeState(FvWindowState::FV_STATE_STOPPED);
    EXPECT_EQ(WMError::WM_ERROR_FV_REPEAT_OPERATION, fvController_->StopFloatViewFromClientSingle());

    fvController_->ChangeState(FvWindowState::FV_STATE_STOPPING);
    EXPECT_EQ(WMError::WM_ERROR_FV_REPEAT_OPERATION, fvController_->StopFloatViewFromClientSingle());

    fvController_->ChangeState(FvWindowState::FV_STATE_UNDEFINED);
    EXPECT_EQ(WMError::WM_ERROR_FV_INVALID_STATE, fvController_->StopFloatViewFromClientSingle());

    fvController_->ChangeState(FvWindowState::FV_STATE_STARTED);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, fvController_->StopFloatViewFromClientSingle());

    fvController_->window_ = mw_;
    EXPECT_EQ(WMError::WM_OK, fvController_->StopFloatViewFromClientSingle());
}

/**
 * @tc.name: StopFloatView
 * @tc.desc: StopFloatView with various states
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, StopFloatView, TestSize.Level1)
{
    fvController_->ChangeState(FvWindowState::FV_STATE_STOPPED);
    EXPECT_EQ(WMError::WM_ERROR_FV_REPEAT_OPERATION, fvController_->StopFloatView("test"));

    fvController_->ChangeState(FvWindowState::FV_STATE_STOPPING);
    EXPECT_EQ(WMError::WM_ERROR_FV_REPEAT_OPERATION, fvController_->StopFloatView("test"));

    fvController_->ChangeState(FvWindowState::FV_STATE_STARTED);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, fvController_->StopFloatView("test"));
    fvController_->window_ = mw_;
    fvController_->ChangeState(FvWindowState::FV_STATE_STARTED);
    EXPECT_EQ(WMError::WM_OK, fvController_->StopFloatView("test"));
}

/**
 * @tc.name: StopFloatView
 * @tc.desc: StopFloatView with various states
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, DestroyFloatViewWindow, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, fvController_->DestroyFloatViewWindow("testReason"));
    fvController_->window_ = mw_;
    EXPECT_CALL(*mw_, Destroy(_)).Times(AtLeast(1)).WillRepeatedly(Return(WMError::WM_ERROR_INVALID_WINDOW));
    EXPECT_EQ(WMError::WM_ERROR_SYSTEM_ABNORMALLY, fvController_->DestroyFloatViewWindow("testReason"));
    EXPECT_CALL(*mw_, Destroy(_)).Times(AtLeast(1)).WillRepeatedly(Return(WMError::WM_OK));
    EXPECT_EQ(WMError::WM_OK, fvController_->DestroyFloatViewWindow("testReason"));
    fvController_->window_ = mw_;
    EXPECT_EQ(WMError::WM_OK, fvController_->DestroyFloatViewWindow("testReason"));
}

/**
 * @tc.name: RestoreMainWindow
 * @tc.desc: RestoreMainWindow with various states
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, RestoreMainWindow, TestSize.Level1)
{
    std::shared_ptr<AAFwk::WantParams> wantParams = std::make_shared<AAFwk::WantParams>();
    fvController_->ChangeState(FvWindowState::FV_STATE_STARTED);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, fvController_->RestoreMainWindow(wantParams));

    fvController_->ChangeState(FvWindowState::FV_STATE_STOPPED);
    EXPECT_EQ(WMError::WM_ERROR_FV_INVALID_STATE, fvController_->RestoreMainWindow(wantParams));

    fvController_->ChangeState(FvWindowState::FV_STATE_STARTED);
    fvController_->window_ = mw_;
    EXPECT_EQ(WMError::WM_OK, fvController_->RestoreMainWindow(wantParams));
}

/**
 * @tc.name: SetFloatViewContext
 * @tc.desc: SetFloatViewContext
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, SetFloatViewContext, TestSize.Level1)
{
    fvController_->window_ = nullptr;
    EXPECT_EQ(WMError::WM_OK, fvController_->SetFloatViewContext());
    fvController_->window_ = mw_;
    option_->SetUIPath("");
    EXPECT_EQ(WMError::WM_OK, fvController_->SetFloatViewContext());
    option_->SetUIPath("testPath");
    EXPECT_EQ(WMError::WM_OK, fvController_->SetFloatViewContext());
}

/**
 * @tc.name: NoneTypeSetUIContext
 * @tc.desc: SetUIContext is not napi and ani
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, NoneTypeSetUIContext, TestSize.Level1)
{
    sptr<FloatViewController> controller =
        sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    controller->type_ = FloatViewController::APIType::NONE;
    EXPECT_EQ(WMError::WM_OK, controller->SetUIContextInner());
}

/**
 * @tc.name: NAPISetUIContext
 * @tc.desc: SetUIContext with various states
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, NAPISetUIContext, TestSize.Level1)
{
    sptr<FloatViewController> napiController =
        sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    std::string contextUrl = "test_url";
    std::shared_ptr<NativeReference> contentStorage = nullptr;

    ani_object fakeStorage = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, napiController->SetUIContext(contextUrl, fakeStorage));

    napiController->window_ = nullptr;
    EXPECT_EQ(WMError::WM_OK, napiController->SetUIContext(contextUrl, contentStorage));

    napiController->ChangeState(FvWindowState::FV_STATE_STARTED);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, napiController->SetUIContext(contextUrl, contentStorage));
    napiController->ChangeState(FvWindowState::FV_STATE_HIDDEN);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, napiController->SetUIContext(contextUrl, contentStorage));
    napiController->ChangeState(FvWindowState::FV_STATE_IN_SIDEBAR);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, napiController->SetUIContext(contextUrl, contentStorage));
    napiController->ChangeState(FvWindowState::FV_STATE_IN_FLOATING_BALL);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, napiController->SetUIContext(contextUrl, contentStorage));

    napiController->window_ = mw_;
    EXPECT_CALL(*mw_, NapiSetUIContent(_, _, _, _, _, _)).WillOnce(Return(WMError::WM_OK));
    EXPECT_EQ(WMError::WM_OK, napiController->SetUIContext(contextUrl, contentStorage));
    EXPECT_CALL(*mw_, NapiSetUIContent(_, _, _, _, _, _)).WillOnce(Return(WMError::WM_DO_NOTHING));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, napiController->SetUIContext(contextUrl, contentStorage));
}

/**
 * @tc.name: ANISetUIContext
 * @tc.desc: SetUIContext with various states
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, ANISetUIContext, TestSize.Level1)
{
    sptr<FloatViewController> aniController =
        sptr<FloatViewController>::MakeSptr(*option_, static_cast<ani_env*>(nullptr));
    std::string contextUrl = "test_url";
    ani_object contentStorage = nullptr;

    std::shared_ptr<NativeReference> fakeStorage = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, aniController->SetUIContext(contextUrl, fakeStorage));

    aniController->window_ = nullptr;
    EXPECT_EQ(WMError::WM_OK, aniController->SetUIContext(contextUrl, contentStorage));

    aniController->ChangeState(FvWindowState::FV_STATE_STARTED);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, aniController->SetUIContext(contextUrl, contentStorage));
    aniController->ChangeState(FvWindowState::FV_STATE_HIDDEN);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, aniController->SetUIContext(contextUrl, contentStorage));
    aniController->ChangeState(FvWindowState::FV_STATE_IN_SIDEBAR);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, aniController->SetUIContext(contextUrl, contentStorage));
    aniController->ChangeState(FvWindowState::FV_STATE_IN_FLOATING_BALL);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, aniController->SetUIContext(contextUrl, contentStorage));

    aniController->window_ = mw_;
    EXPECT_CALL(*mw_, AniSetUIContent(_, _, _, _, _, _)).WillOnce(Return(WMError::WM_OK));
    EXPECT_EQ(WMError::WM_OK, aniController->SetUIContext(contextUrl, contentStorage));
    EXPECT_CALL(*mw_, AniSetUIContent(_, _, _, _, _, _)).WillOnce(Return(WMError::WM_DO_NOTHING));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, aniController->SetUIContext(contextUrl, contentStorage));
}

/**
 * @tc.name: SetVisibilityInApp
 * @tc.desc: SetVisibilityInApp with null window
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, SetVisibilityInApp, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_OK, fvController_->SetVisibilityInApp(true));
    EXPECT_EQ(WMError::WM_OK, fvController_->SetVisibilityInApp(false));

    fvController_->ChangeState(FvWindowState::FV_STATE_STARTED);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, fvController_->SetVisibilityInApp(true));

    EXPECT_CALL(*mw_, UpdateFloatView(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    fvController_->window_ = mw_;
    EXPECT_EQ(WMError::WM_OK, fvController_->SetVisibilityInApp(true));
    EXPECT_CALL(*mw_, UpdateFloatView(_)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    EXPECT_EQ(WMError::WM_ERROR_SYSTEM_ABNORMALLY, fvController_->SetVisibilityInApp(true));
}

/**
 * @tc.name: SetWindowSize
 * @tc.desc: SetWindowSize with null window
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, SetWindowSize, TestSize.Level1)
{
    Rect rect {0, 0, 100, 100};
    EXPECT_EQ(WMError::WM_OK, fvController_->SetWindowSize(rect));

    fvController_->ChangeState(FvWindowState::FV_STATE_STARTED);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, fvController_->SetWindowSize(rect));

    fvController_->window_ = mw_;
    EXPECT_CALL(*mw_, UpdateFloatView(_)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    EXPECT_EQ(WMError::WM_ERROR_SYSTEM_ABNORMALLY, fvController_->SetWindowSize(rect));

    EXPECT_CALL(*mw_, UpdateFloatView(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    EXPECT_EQ(WMError::WM_OK, fvController_->SetWindowSize(rect));
    fvController_->window_ = nullptr;
}

/**
 * @tc.name: GetWindow
 * @tc.desc: GetWindow
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, GetWindow, TestSize.Level1)
{
    fvController_->window_ = mw_;
    sptr<Window> window = fvController_->GetWindow();
    EXPECT_TRUE(fvController_->GetWindow() != nullptr);
    fvController_->window_ = nullptr;
    window = fvController_->GetWindow();
    EXPECT_TRUE(fvController_->GetWindow() == nullptr);
}

/**
 * @tc.name: GetWindow
 * @tc.desc: GetWindow
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, SyncWindowInfoTest, TestSize.Level1)
{
    FloatViewWindowInfo windowInfo;
    windowInfo.scale_ = 1.1;
    fvController_->SyncWindowInfo(1, windowInfo, "testReason");
    fvController_->window_ = mw_;
    fvController_->SyncWindowInfo(1, windowInfo, "testReason");
    fvController_->SyncWindowInfo(fvController_->window_->GetWindowId(), windowInfo, "testReason");
    EXPECT_EQ(fvController_->windowInfo_.scale_, windowInfo.scale_);
    EXPECT_EQ(fvController_->GetWindowInfo().scale_, windowInfo.scale_);
    auto listener = sptr<MockRectChangeListener>::MakeSptr();
    fvController_->rectChangeObservers_.push_back(nullptr);
    fvController_->rectChangeObservers_.push_back(listener);
    fvController_->SyncWindowInfo(fvController_->window_->GetWindowId(), windowInfo, "testReason");
    EXPECT_EQ(fvController_->windowInfo_.scale_, windowInfo.scale_);
}

/**
 * @tc.name: GetWindow
 * @tc.desc: GetWindow
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, SyncLimitsTest, TestSize.Level1)
{
    FloatViewLimits limitsInfo;
    limitsInfo.maxHeight_ = 1;
    fvController_->SyncLimits(1, limitsInfo);
    fvController_->window_ = mw_;
    fvController_->SyncLimits(1, limitsInfo);

    auto listener = sptr<MockLimitsChangeListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, fvController_->RegisterLimitsChangeListener(listener));
    fvController_->limitsChangeObservers_.push_back(nullptr);
    fvController_->SyncLimits(fvController_->window_->GetWindowId(), limitsInfo);
    EXPECT_EQ(listener->maxHeight_, limitsInfo.maxHeight_);
}

/**
 * @tc.name: RegisterStateChangeListener
 * @tc.desc: RegisterStateChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, RegisterStateChangeListener, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, fvController_->RegisterStateChangeListener(nullptr));
    auto listener = sptr<MockStateChangeListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, fvController_->RegisterStateChangeListener(listener));
    EXPECT_EQ(WMError::WM_ERROR_FV_REPEAT_OPERATION, fvController_->RegisterStateChangeListener(listener));
}

/**
 * @tc.name: UnregisterStateChangeListener
 * @tc.desc: UnregisterStateChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, UnregisterStateChangeListener, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, fvController_->UnregisterStateChangeListener(nullptr));
    auto listener = sptr<MockStateChangeListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, fvController_->UnregisterStateChangeListener(listener));
}

/**
 * @tc.name: RegisterRectChangeListener
 * @tc.desc: RegisterRectChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, RegisterRectChangeListener, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, fvController_->RegisterRectChangeListener(nullptr));
    auto listener = sptr<MockRectChangeListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, fvController_->RegisterRectChangeListener(listener));
    EXPECT_EQ(WMError::WM_ERROR_FV_REPEAT_OPERATION, fvController_->RegisterRectChangeListener(listener));
}

/**
 * @tc.name: UnregisterRectChangeListener
 * @tc.desc: UnregisterRectChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, UnregisterRectChangeListener, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, fvController_->UnregisterRectChangeListener(nullptr));
    auto listener = sptr<MockRectChangeListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, fvController_->UnregisterRectChangeListener(listener));
}

/**
 * @tc.name: RegisterLimitsChangeListener
 * @tc.desc: RegisterLimitsChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, RegisterLimitsChangeListener, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, fvController_->RegisterLimitsChangeListener(nullptr));
    auto listener = sptr<MockLimitsChangeListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, fvController_->RegisterLimitsChangeListener(listener));
    EXPECT_EQ(WMError::WM_ERROR_FV_REPEAT_OPERATION, fvController_->RegisterLimitsChangeListener(listener));
}

/**
 * @tc.name: UnregisterLimitsChangeListener
 * @tc.desc: UnregisterLimitsChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, UnregisterLimitsChangeListener, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, fvController_->UnregisterLimitsChangeListener(nullptr));
    auto listener = sptr<MockLimitsChangeListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, fvController_->UnregisterLimitsChangeListener(listener));
}

/**
 * @tc.name: ListenerNullTest
 * @tc.desc: Test all listener operations with null
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, ListenerNullTest, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, fvController_->RegisterStateChangeListener(nullptr));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, fvController_->UnregisterStateChangeListener(nullptr));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, fvController_->RegisterRectChangeListener(nullptr));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, fvController_->UnregisterRectChangeListener(nullptr));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, fvController_->RegisterLimitsChangeListener(nullptr));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, fvController_->UnregisterLimitsChangeListener(nullptr));
}

/**
 * @tc.name: StartFloatViewInner
 * @tc.desc: Test StartFloatViewInner with various scenarios
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, StartFloatViewInner, TestSize.Level1)
{
    void* invalidContext = nullptr;
    option_->SetContext(invalidContext);
    fvController_ = sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    EXPECT_NE(WMError::WM_OK, fvController_->StartFloatViewInner());
}

/**
 * @tc.name: AfterDestroy_Test
 * @tc.desc: AfterDestroy_Test
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, AfterDestroy_Test, TestSize.Level1)
{
    fvController_ = sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    FloatViewManager::SetActiveController(fvController_);
    fvController_->mainWindowLifeCycleListener_ = sptr<FloatViewController::WindowLifeCycleListener>::MakeSptr();
    fvController_->mainWindowLifeCycleListener_->AfterDestroyed();
    EXPECT_EQ(FvWindowState::FV_STATE_UNDEFINED, fvController_->curState_);
}

/**
 * @tc.name: SetShowWhenCreate_Test
 * @tc.desc: SetShowWhenCreate_Test
 * @tc.type: FUNC
 */
HWTEST_F(FloatViewControllerTest, SetShowWhenCreate_Test, TestSize.Level1)
{
    fvController_ = sptr<FloatViewController>::MakeSptr(*option_, static_cast<napi_env>(nullptr));
    FloatViewManager::SetActiveController(fvController_);
    fvController_->SetShowWhenCreate(true);
    fvController_->window_ = mw_;
    fvController_->SetShowWhenCreate(true);
    EXPECT_EQ(true, fvController_->option_.showWhenCreate_);
}
}
}
}
