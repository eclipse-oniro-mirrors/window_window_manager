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
 * See the License for the specific language governing permissions permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <gmock/gmock.h>
#include "float_view_controller.h"
#include "float_window_manager.h"
#include "floating_ball_controller.h"
#include "floating_ball_manager.h"
#include "picture_in_picture_manager.h"
#include "float_view_option.h"
#include "float_view_manager.h"
#include "floating_ball_option.h"
#include "window.h"
#include "wm_common.h"
#include "window_scene_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class MockFloatViewController : public FloatViewController {
public:
    MockFloatViewController(const FvOption& option, napi_env env) : FloatViewController(option, env) {}
    virtual ~MockFloatViewController() = default;

    MOCK_METHOD1(StartFloatViewSingle, WMError(bool showWhenCreate));
    MOCK_METHOD0(StopFloatViewFromClientSingle, WMError());
    MOCK_CONST_METHOD0(GetWindow, sptr<Window>());
    MOCK_METHOD0(GetCurState, FvWindowState());
    MOCK_METHOD0(IsBind, bool());
    MOCK_METHOD1(SetBindState, void(bool isBind));
    MOCK_METHOD1(SetBindWindowId, void(uint32_t windowId));
};

class MockFloatingBallController : public FloatingBallController {
public:
    MockFloatingBallController(const sptr<Window>& mainWindow, const uint32_t& mainWindowId, void* context)
        : FloatingBallController(mainWindow, mainWindowId, context) {}
    virtual ~MockFloatingBallController() = default;

    MOCK_METHOD2(StartFloatingBallSingle, WMError(const sptr<FbOption>& option, bool showWhenCreate));
    MOCK_METHOD0(StopFloatingBallFromClientSingle, WMError());
    MOCK_METHOD1(SetOption, void(const sptr<FbOption>& option));
    MOCK_METHOD1(SetBindState, void(bool isBind));
    MOCK_METHOD1(SetBindWindowId, void(uint32_t windowId));
    MOCK_CONST_METHOD0(GetFbWindow, sptr<Window>());
    MOCK_METHOD0(GetCurState, FbWindowState());
    MOCK_METHOD0(IsBind, bool());
};

class MockWindow : public Window {
public:
    WindowState state_ = WindowState::STATE_INITIAL;
    const uint32_t mockWindowId_ = 101;
    MockWindow() {};
    ~MockWindow() {};
    MOCK_METHOD3(Show, WMError(uint32_t reason, bool withAnimation, bool withFocus));
    MOCK_METHOD1(Destroy, WMError(uint32_t reason));
    MOCK_METHOD1(UpdateFloatView, WMError(const FloatViewTemplateInfo& fvTemplateInfo));
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

class FloatWindowManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockFloatViewController> fvController_;
    sptr<MockFloatingBallController> fbController_;
    sptr<FvOption> fvOption_;
    sptr<FbOption> fbOption_;
    sptr<MockWindow> mw_;
};

void FloatWindowManagerTest::SetUpTestCase()
{
}

void FloatWindowManagerTest::TearDownTestCase()
{
}

void FloatWindowManagerTest::SetUp()
{
    fvOption_ = sptr<FvOption>::MakeSptr();
    fbOption_ = sptr<FbOption>::MakeSptr();
    ASSERT_NE(nullptr, fvOption_);
    ASSERT_NE(nullptr, fbOption_);
    
    fvController_ = sptr<MockFloatViewController>::MakeSptr(*fvOption_, nullptr);
    fbController_ = sptr<MockFloatingBallController>::MakeSptr(nullptr, 0, nullptr);
    
    ASSERT_NE(nullptr, fvController_);
    ASSERT_NE(nullptr, fbController_);

    mw_ = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw_);
}

void FloatWindowManagerTest::TearDown()
{
    if (fvController_ != nullptr) {
        FloatViewManager::RemoveActiveController(fvController_);
    }
    fvController_ = nullptr;
    fbController_ = nullptr;
    fvOption_ = nullptr;
    fbOption_ = nullptr;
}

namespace {
HWTEST_F(FloatWindowManagerTest, GetControllerId, TestSize.Level1)
{
    std::string id1 = FloatWindowManager::GetControllerId();
    std::string id2 = FloatWindowManager::GetControllerId();
    EXPECT_NE(id1, id2);
    EXPECT_FALSE(id1.empty());
    EXPECT_FALSE(id2.empty());

    std::set<std::string> ids;
    for (int i = 0; i < 10; i++) {
        std::string id = FloatWindowManager::GetControllerId();
        EXPECT_TRUE(ids.insert(id).second);
    }
    EXPECT_EQ(10, ids.size());
}

HWTEST_F(FloatWindowManagerTest, Bind, TestSize.Level1)
{
    fbController_->bindState_ = false;
    fbController_->curState_ = FbWindowState::STATE_UNDEFINED;
    fvController_->bindState_ = false;
    fvController_->curState_ = FvWindowState::FV_STATE_UNDEFINED;

    EXPECT_EQ(WMError::WM_OK, FloatWindowManager::Bind(fvController_, fbController_, *fbOption_));
    EXPECT_TRUE(fvController_->bindState_);
    fbController_->bindState_ = false;
    fvController_->bindState_ = false;
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_FB_INVALID_STATE, FloatWindowManager::Bind(fvController_, fbController_, *fbOption_));

    fbController_->bindState_ = true;
    fvController_->bindState_ = false;
    fbController_->curState_ = FbWindowState::STATE_UNDEFINED;
    EXPECT_EQ(WMError::WM_ERROR_FB_INVALID_STATE, FloatWindowManager::Bind(fvController_, fbController_, *fbOption_));

    fbController_->bindState_ = true;
    fvController_->bindState_ = false;
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_FB_INVALID_STATE, FloatWindowManager::Bind(fvController_, fbController_, *fbOption_));
    fbController_->curState_ = FbWindowState::STATE_UNDEFINED;

    fbController_->bindState_ = false;
    fvController_->bindState_ = false;
    fvController_->curState_ = FvWindowState::FV_STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_FV_INVALID_STATE, FloatWindowManager::Bind(fvController_, fbController_, *fbOption_));

    fbController_->bindState_ = false;
    fvController_->bindState_ = true;
    fvController_->curState_ = FvWindowState::FV_STATE_UNDEFINED;
    EXPECT_EQ(WMError::WM_ERROR_FV_INVALID_STATE, FloatWindowManager::Bind(fvController_, fbController_, *fbOption_));

    fbController_->bindState_ = false;
    fvController_->bindState_ = true;
    fvController_->curState_ = FvWindowState::FV_STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_FV_INVALID_STATE, FloatWindowManager::Bind(fvController_, fbController_, *fbOption_));
}

HWTEST_F(FloatWindowManagerTest, UnBind, TestSize.Level1)
{
    fbController_->bindState_ = true;
    fbController_->curState_ = FbWindowState::STATE_UNDEFINED;
    fvController_->bindState_ = true;
    fvController_->curState_ = FvWindowState::FV_STATE_UNDEFINED;

    EXPECT_EQ(WMError::WM_ERROR_FV_INVALID_STATE, FloatWindowManager::UnBind(fvController_, fbController_));
    EXPECT_EQ(WMError::WM_ERROR_FV_INVALID_STATE, FloatWindowManager::UnBind(nullptr, fbController_));
    EXPECT_EQ(WMError::WM_ERROR_FB_INVALID_STATE, FloatWindowManager::UnBind(fvController_, nullptr));
    FloatWindowManager::floatViewToFloatingBallMap_.insert(std::make_pair(fvController_, fbController_));
    EXPECT_EQ(WMError::WM_OK, FloatWindowManager::UnBind(fvController_, fbController_));

    fbController_->bindState_ = true;
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    fvController_->bindState_ = true;
    EXPECT_EQ(WMError::WM_ERROR_FB_INVALID_STATE, FloatWindowManager::UnBind(fvController_, fbController_));

    fbController_->bindState_ = false;
    fbController_->curState_ = FbWindowState::STATE_UNDEFINED;
    fvController_->bindState_ = true;
    EXPECT_EQ(WMError::WM_ERROR_FB_INVALID_STATE, FloatWindowManager::UnBind(fvController_, fbController_));

    fbController_->bindState_ = false;
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    fvController_->bindState_ = true;
    EXPECT_EQ(WMError::WM_ERROR_FB_INVALID_STATE, FloatWindowManager::UnBind(fvController_, fbController_));
    fbController_->curState_ = FbWindowState::STATE_UNDEFINED;

    fbController_->bindState_ = true;
    fvController_->bindState_ = true;
    fvController_->curState_ = FvWindowState::FV_STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_FV_INVALID_STATE, FloatWindowManager::UnBind(fvController_, fbController_));

    fbController_->bindState_ = true;
    fvController_->bindState_ = false;
    fvController_->curState_ = FvWindowState::FV_STATE_UNDEFINED;
    EXPECT_EQ(WMError::WM_ERROR_FV_INVALID_STATE, FloatWindowManager::UnBind(fvController_, fbController_));

    fbController_->bindState_ = true;
    fvController_->bindState_ = false;
    fvController_->curState_ = FvWindowState::FV_STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_FV_INVALID_STATE, FloatWindowManager::UnBind(fvController_, fbController_));
}

HWTEST_F(FloatWindowManagerTest, GetBoundFloatingBall, TestSize.Level1)
{
    auto boundFb = FloatWindowManager::GetBoundFloatingBall(fvController_);
    EXPECT_EQ(nullptr, boundFb);

    sptr<MockFloatViewController> nullController;
    boundFb = FloatWindowManager::GetBoundFloatingBall(nullController);
    EXPECT_EQ(nullptr, boundFb);

    EXPECT_CALL(*fbController_, GetCurState()).WillRepeatedly(Return(FbWindowState::STATE_UNDEFINED));
    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(false));
    EXPECT_CALL(*fvController_, GetCurState()).WillRepeatedly(Return(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_CALL(*fvController_, IsBind()).WillRepeatedly(Return(false));
    
    FloatWindowManager::Bind(fvController_, fbController_, *fbOption_);
    
    boundFb = FloatWindowManager::GetBoundFloatingBall(fvController_);
    EXPECT_NE(nullptr, boundFb);
    EXPECT_EQ(fbController_.GetRefPtr(), boundFb.GetRefPtr());
}

HWTEST_F(FloatWindowManagerTest, GetBoundFloatView, TestSize.Level1)
{
    auto boundFv = FloatWindowManager::GetBoundFloatView(fbController_);
    EXPECT_EQ(nullptr, boundFv);

    EXPECT_CALL(*fbController_, GetCurState()).WillRepeatedly(Return(FbWindowState::STATE_UNDEFINED));
    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(false));
    EXPECT_CALL(*fvController_, GetCurState()).WillRepeatedly(Return(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_CALL(*fvController_, IsBind()).WillRepeatedly(Return(false));
    
    FloatWindowManager::Bind(fvController_, fbController_, *fbOption_);
    
    boundFv = FloatWindowManager::GetBoundFloatView(fbController_);
    EXPECT_NE(nullptr, boundFv);
    EXPECT_EQ(fvController_.GetRefPtr(), boundFv.GetRefPtr());

    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(true));
    EXPECT_CALL(*fvController_, IsBind()).WillRepeatedly(Return(true));
    FloatWindowManager::UnBind(fvController_, fbController_);
    boundFv = FloatWindowManager::GetBoundFloatView(fbController_);
    EXPECT_EQ(nullptr, boundFv);

    sptr<FloatingBallController> nullController;
    boundFv = FloatWindowManager::GetBoundFloatView(nullController);
    EXPECT_EQ(nullptr, boundFv);
}

HWTEST_F(FloatWindowManagerTest, StartBindFloatView, TestSize.Level1)
{
    auto mockFvController = sptr<MockFloatViewController>::MakeSptr(*fvOption_, nullptr);
    auto mockWindow = sptr<Window>::MakeSptr();
    auto mockFbWindow = sptr<Window>::MakeSptr();

    EXPECT_CALL(*fbController_, GetCurState()).WillRepeatedly(Return(FbWindowState::STATE_UNDEFINED));
    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(false));
    EXPECT_CALL(*fbController_, SetOption(_)).WillRepeatedly(Return());
    EXPECT_CALL(*fbController_, SetBindState(true)).WillRepeatedly(Return());
    EXPECT_CALL(*mockFvController, GetCurState()).WillRepeatedly(Return(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_CALL(*mockFvController, IsBind()).WillRepeatedly(Return(false));
    EXPECT_CALL(*mockFvController, SetBindState(true)).WillRepeatedly(Return());
    FloatWindowManager::Bind(mockFvController, fbController_, *fbOption_);

    EXPECT_CALL(*mockFvController, StartFloatViewSingle(true)).
        WillRepeatedly(Return(WMError::WM_ERROR_INVALID_OPERATION));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, FloatWindowManager::StartBindFloatView(mockFvController));
    wptr<FloatViewController> nullController;
    EXPECT_EQ(WMError::WM_DO_NOTHING, FloatWindowManager::StartBindFloatView(nullController));
}

HWTEST_F(FloatWindowManagerTest, StartBindFloatingBall, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_DO_NOTHING, FloatWindowManager::StartBindFloatingBall(fbController_, fbOption_));
    wptr<FloatingBallController> nullController;
    EXPECT_EQ(WMError::WM_DO_NOTHING, FloatWindowManager::StartBindFloatingBall(nullController, fbOption_));

    auto mockFvController = sptr<MockFloatViewController>::MakeSptr(*fvOption_, nullptr);
    auto mockWindow = sptr<Window>::MakeSptr();
    auto mockFbWindow = sptr<Window>::MakeSptr();
    
    EXPECT_CALL(*fbController_, GetCurState()).WillRepeatedly(Return(FbWindowState::STATE_UNDEFINED));
    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(false));
    
    EXPECT_CALL(*mockFvController, GetCurState()).WillRepeatedly(Return(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_CALL(*mockFvController, IsBind()).WillRepeatedly(Return(false));
    
    FloatWindowManager::Bind(mockFvController, fbController_, *fbOption_);
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, FloatWindowManager::StartBindFloatingBall(fbController_,
        fbOption_));
    FloatWindowManager::UnBind(mockFvController, fbController_);
}

HWTEST_F(FloatWindowManagerTest, StopBindFloatView, TestSize.Level1)
{
    wptr<MockFloatingBallController> nullFbController;
    EXPECT_EQ(WMError::WM_DO_NOTHING, FloatWindowManager::StopBindFloatView(nullFbController));
    wptr<MockFloatViewController> nullFvController;
    EXPECT_EQ(WMError::WM_DO_NOTHING, FloatWindowManager::StopBindFloatView(nullFvController));

    auto mockFvController = sptr<MockFloatViewController>::MakeSptr(*fvOption_, nullptr);
    
    EXPECT_CALL(*fbController_, GetCurState()).WillRepeatedly(Return(FbWindowState::STATE_UNDEFINED));
    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(false));
    
    EXPECT_CALL(*mockFvController, GetCurState()).WillRepeatedly(Return(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_CALL(*mockFvController, IsBind()).WillRepeatedly(Return(false));
    
    FloatWindowManager::Bind(mockFvController, fbController_, *fbOption_);
    mockFvController->curState_ = FvWindowState::FV_STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, FloatWindowManager::StopBindFloatView(mockFvController));

    EXPECT_CALL(*mw_, Destroy(_)).WillRepeatedly(Return(WMError::WM_OK));
    mockFvController->window_ = mw_;

    EXPECT_EQ(WMError::WM_ERROR_INVALID_OPERATION, FloatWindowManager::StopBindFloatView(mockFvController));
    fbController_->window_ = mw_;
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    mockFvController->curState_ = FvWindowState::FV_STATE_STARTED;
    EXPECT_EQ(WMError::WM_OK, FloatWindowManager::StopBindFloatView(mockFvController));
    FloatWindowManager::UnBind(mockFvController, fbController_);
}

HWTEST_F(FloatWindowManagerTest, StopBindFloatingBall, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_DO_NOTHING, FloatWindowManager::StopBindFloatingBall(fbController_));
    wptr<FloatingBallController> nullController;
    EXPECT_EQ(WMError::WM_DO_NOTHING, FloatWindowManager::StopBindFloatingBall(nullController));

    auto mockFvController = sptr<MockFloatViewController>::MakeSptr(*fvOption_, nullptr);
    
    EXPECT_CALL(*fbController_, GetCurState()).WillRepeatedly(Return(FbWindowState::STATE_UNDEFINED));
    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(false));
    
    EXPECT_CALL(*mockFvController, GetCurState()).WillRepeatedly(Return(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_CALL(*mockFvController, IsBind()).WillRepeatedly(Return(false));
    
    FloatWindowManager::Bind(mockFvController, fbController_, *fbOption_);
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, FloatWindowManager::StopBindFloatingBall(fbController_));

    EXPECT_CALL(*mw_, Destroy(_)).WillRepeatedly(Return(WMError::WM_OK));
    fbController_->window_ = mw_;
    EXPECT_EQ(WMError::WM_ERROR_FB_INTERNAL_ERROR, FloatWindowManager::StopBindFloatingBall(fbController_));
    mockFvController->window_ = mw_;
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    mockFvController->curState_ = FvWindowState::FV_STATE_STARTED;
    EXPECT_EQ(WMError::WM_OK, FloatWindowManager::StopBindFloatingBall(fbController_));
    FloatWindowManager::UnBind(mockFvController, fbController_);
}

HWTEST_F(FloatWindowManagerTest, BindUnBindCycle, TestSize.Level1)
{
    EXPECT_CALL(*fbController_, GetCurState()).WillRepeatedly(Return(FbWindowState::STATE_UNDEFINED));
    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(false));
    EXPECT_CALL(*fvController_, GetCurState()).WillRepeatedly(Return(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_CALL(*fvController_, IsBind()).WillRepeatedly(Return(false));
    
    EXPECT_EQ(WMError::WM_OK, FloatWindowManager::Bind(fvController_, fbController_, *fbOption_));
    EXPECT_TRUE(fvController_->bindState_);
    
    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(true));
    EXPECT_CALL(*fvController_, IsBind()).WillRepeatedly(Return(true));
    
    EXPECT_EQ(WMError::WM_OK, FloatWindowManager::UnBind(fvController_, fbController_));
    EXPECT_FALSE(fvController_->bindState_);
}

HWTEST_F(FloatWindowManagerTest, MultiplePairsTest, TestSize.Level1)
{
    auto fvController2 = sptr<MockFloatViewController>::MakeSptr(*fvOption_, nullptr);
    auto fbController2 = sptr<MockFloatingBallController>::MakeSptr(nullptr, 0, nullptr);
    
    EXPECT_CALL(*fbController_, GetCurState()).WillRepeatedly(Return(FbWindowState::STATE_UNDEFINED));
    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(false));
    EXPECT_CALL(*fvController_, GetCurState()).WillRepeatedly(Return(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_CALL(*fvController_, IsBind()).WillRepeatedly(Return(false));
    
    EXPECT_CALL(*fbController2, GetCurState()).WillRepeatedly(Return(FbWindowState::STATE_UNDEFINED));
    EXPECT_CALL(*fbController2, IsBind()).WillRepeatedly(Return(false));
    EXPECT_CALL(*fvController2, GetCurState()).WillRepeatedly(Return(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_CALL(*fvController2, IsBind()).WillRepeatedly(Return(false));
    
    EXPECT_EQ(WMError::WM_OK, FloatWindowManager::Bind(fvController_, fbController_, *fbOption_));
    EXPECT_EQ(WMError::WM_OK, FloatWindowManager::Bind(fvController2, fbController2, *fbOption_));
    
    auto boundFb1 = FloatWindowManager::GetBoundFloatingBall(fvController_);
    auto boundFb2 = FloatWindowManager::GetBoundFloatingBall(fvController2);
    
    EXPECT_NE(nullptr, boundFb1);
    EXPECT_NE(nullptr, boundFb2);
    EXPECT_EQ(fbController_.GetRefPtr(), boundFb1.GetRefPtr());
    EXPECT_EQ(fbController2.GetRefPtr(), boundFb2.GetRefPtr());
}

HWTEST_F(FloatWindowManagerTest, DestroyFloatWindow_Test, TestSize.Level1)
{
    sptr<Window> window = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, FloatWindowManager::DestroyFloatWindow(window));

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("floatView");
    option->SetWindowType(WindowType::WINDOW_TYPE_FV);
    sptr<WindowSceneSessionImpl> mockFvWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    FloatWindowManager::floatViewCnt_ = 1;
    FloatWindowManager::DestroyFloatWindow(mockFvWindow);
    EXPECT_EQ(0, FloatWindowManager::floatViewCnt_);
    FloatWindowManager::floatViewCnt_ = 0;
    FloatWindowManager::DestroyFloatWindow(mockFvWindow);
    EXPECT_EQ(0, FloatWindowManager::floatViewCnt_);

    option->SetWindowName("floatingball");
    option->SetWindowType(WindowType::WINDOW_TYPE_FB);
    sptr<WindowSceneSessionImpl> mockFbWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    FloatWindowManager::floatingBallCnt_ = 1;
    FloatWindowManager::DestroyFloatWindow(mockFbWindow);
    EXPECT_EQ(0, FloatWindowManager::floatingBallCnt_);
    FloatWindowManager::floatingBallCnt_ = 0;
    FloatWindowManager::DestroyFloatWindow(mockFbWindow);
    EXPECT_EQ(0, FloatWindowManager::floatingBallCnt_);

    option->SetWindowName("pip");
    option->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    sptr<WindowSceneSessionImpl> mockPipWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    FloatWindowManager::pipCnt_ = 1;
    FloatWindowManager::DestroyFloatWindow(mockPipWindow);
    EXPECT_EQ(0, FloatWindowManager::pipCnt_);
    FloatWindowManager::pipCnt_ = 0;
    FloatWindowManager::DestroyFloatWindow(mockPipWindow);
    EXPECT_EQ(0, FloatWindowManager::pipCnt_);
}

HWTEST_F(FloatWindowManagerTest, CreatePipWindow_Test, TestSize.Level1)
{
    WMError error = WMError::WM_OK;
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    PiPTemplateInfo templateInfo {};
    std::shared_ptr<OHOS::AbilityRuntime::Context> context = nullptr;
    FloatWindowManager::floatViewCnt_ = 1;

    sptr<Window> pipWindow = FloatWindowManager::CreatePipWindow(option, templateInfo, context, error);
    EXPECT_EQ(nullptr, pipWindow);
    FloatWindowManager::floatViewCnt_ = 0;

    pipWindow = FloatWindowManager::CreatePipWindow(option, templateInfo, context, error);
    EXPECT_EQ(nullptr, pipWindow);
}

HWTEST_F(FloatWindowManagerTest, CreateFbWindow_Test, TestSize.Level1)
{
    WMError error = WMError::WM_OK;
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    FloatingBallTemplateBaseInfo templateInfo(0, "title", "content", "#000000", "", "", true, false,
        INVALID_WINDOW_ID, true, "test");
    std::shared_ptr<Media::PixelMap> icon = nullptr;
    std::shared_ptr<OHOS::AbilityRuntime::Context> context = nullptr;

    // branch: floatViewCnt_ > 0 and no bound/active fv => conflict
    FloatWindowManager::floatViewCnt_ = 1;
    wptr<FloatingBallController> nullFbController;
    auto fbWindow = FloatWindowManager::CreateFbWindow(option, templateInfo, icon, context, error, nullFbController);
    EXPECT_EQ(nullptr, fbWindow);
    EXPECT_EQ(WMError::WM_ERROR_FLOAT_CONFLICT_WITH_OTHERS, error);

    // branch: floatViewCnt_ > 0 but bound fv is active => allow create (Window::CreateFb may still return nullptr)
    FloatWindowManager::floatViewCnt_ = 1;
    FloatWindowManager::floatingBallCnt_ = 0;
    EXPECT_CALL(*fbController_, GetCurState()).WillRepeatedly(Return(FbWindowState::STATE_UNDEFINED));
    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(false));
    EXPECT_CALL(*fvController_, GetCurState()).WillRepeatedly(Return(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_CALL(*fvController_, IsBind()).WillRepeatedly(Return(false));
    FloatWindowManager::Bind(fvController_, fbController_, *fbOption_);

    FloatViewManager::SetActiveController(fvController_);
    error = WMError::WM_OK;
    fbWindow = FloatWindowManager::CreateFbWindow(option, templateInfo, icon, context, error, fbController_);
    EXPECT_EQ(nullptr, fbWindow);
    EXPECT_EQ(0, FloatWindowManager::floatingBallCnt_);
    FloatViewManager::RemoveActiveController(fvController_);
    FloatWindowManager::UnBind(fvController_, fbController_);

    // branch: floatViewCnt_ == 0 => directly create
    FloatWindowManager::floatViewCnt_ = 0;
    error = WMError::WM_OK;
    fbWindow = FloatWindowManager::CreateFbWindow(option, templateInfo, icon, context, error, fbController_);
    EXPECT_EQ(nullptr, fbWindow);
    EXPECT_EQ(0, FloatWindowManager::floatingBallCnt_);
}

HWTEST_F(FloatWindowManagerTest, CreateFvWindow_Test, TestSize.Level1)
{
    WMError error = WMError::WM_OK;
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    FloatViewTemplateInfo templateInfo {};
    std::shared_ptr<OHOS::AbilityRuntime::Context> context = nullptr;

    // branch: pipCnt_ > 0 => conflict
    FloatWindowManager::pipCnt_ = 1;
    auto fvWindow = FloatWindowManager::CreateFvWindow(option, templateInfo, context, error, fvController_);
    EXPECT_EQ(nullptr, fvWindow);

    // branch: floatingBallCnt_ > 0 and no bound/active fb => conflict
    FloatWindowManager::pipCnt_ = 0;
    FloatWindowManager::floatingBallCnt_ = 1;
    error = WMError::WM_OK;
    wptr<FloatViewController> nullFvController;
    fvWindow = FloatWindowManager::CreateFvWindow(option, templateInfo, context, error, nullFvController);
    EXPECT_EQ(nullptr, fvWindow);

    // branch: floatingBallCnt_ > 0 but bound fb is active => allow create
    FloatWindowManager::pipCnt_ = 0;
    FloatWindowManager::floatingBallCnt_ = 1;
    FloatWindowManager::floatViewCnt_ = 0;
    EXPECT_CALL(*fbController_, GetCurState()).WillRepeatedly(Return(FbWindowState::STATE_UNDEFINED));
    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(false));
    EXPECT_CALL(*fvController_, GetCurState()).WillRepeatedly(Return(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_CALL(*fvController_, IsBind()).WillRepeatedly(Return(false));
    FloatWindowManager::Bind(fvController_, fbController_, *fbOption_);

    FloatingBallManager::SetActiveController(fbController_);
    error = WMError::WM_OK;
    fvWindow = FloatWindowManager::CreateFvWindow(option, templateInfo, context, error, fvController_);
    EXPECT_EQ(nullptr, fvWindow);
    EXPECT_EQ(0, FloatWindowManager::floatViewCnt_);
    FloatingBallManager::RemoveActiveController(fbController_);
    FloatWindowManager::UnBind(fvController_, fbController_);

    // branch: pipCnt_ == 0 and floatingBallCnt_ == 0 => directly create
    FloatWindowManager::pipCnt_ = 0;
    FloatWindowManager::floatingBallCnt_ = 0;
    error = WMError::WM_OK;
    fvWindow = FloatWindowManager::CreateFvWindow(option, templateInfo, context, error, fvController_);
    EXPECT_EQ(nullptr, fvWindow);
    EXPECT_EQ(0, FloatWindowManager::floatViewCnt_);
}

/**
 * @tc.name: RemoveRelation_Test
 * @tc.desc: Cover branches of FloatWindowManager::RemoveRelation
 * @tc.type: FUNC
 */
HWTEST_F(FloatWindowManagerTest, RemoveRelation_Test, TestSize.Level1)
{
    // Prepare a relation via Bind (Bind internally calls AddRelation).
    EXPECT_CALL(*fbController_, GetCurState()).WillRepeatedly(Return(FbWindowState::STATE_UNDEFINED));
    EXPECT_CALL(*fbController_, IsBind()).WillRepeatedly(Return(false));

    EXPECT_CALL(*fvController_, GetCurState()).WillRepeatedly(Return(FvWindowState::FV_STATE_UNDEFINED));
    EXPECT_CALL(*fvController_, IsBind()).WillRepeatedly(Return(false));

    EXPECT_EQ(WMError::WM_OK, FloatWindowManager::Bind(fvController_, fbController_, *fbOption_));
    EXPECT_NE(nullptr, FloatWindowManager::GetBoundFloatingBall(fvController_));

    // branch: it->second != nullptr && fbController == nullptr => do nothing
    sptr<FloatingBallController> nullFb;
    FloatWindowManager::RemoveRelation(fvController_, nullFb);
    EXPECT_NE(nullptr, FloatWindowManager::GetBoundFloatingBall(fvController_));

    // branch: fbController not null but not equal to mapped controller => do nothing
    auto otherFbController = sptr<MockFloatingBallController>::MakeSptr(nullptr, 0, nullptr);
    FloatWindowManager::RemoveRelation(fvController_, otherFbController);
    EXPECT_NE(nullptr, FloatWindowManager::GetBoundFloatingBall(fvController_));

    // branch: fvController not found in map => do nothing
    auto otherFvController = sptr<MockFloatViewController>::MakeSptr(*fvOption_, nullptr);
    FloatWindowManager::RemoveRelation(otherFvController, fbController_);

    // branch: all conditions satisfied => erase relation
    FloatWindowManager::RemoveRelation(fvController_, fbController_);
    EXPECT_EQ(nullptr, FloatWindowManager::GetBoundFloatingBall(fvController_));
}

/**
 * @tc.name: FindFloatViewByFloatingBall_Test
 * @tc.desc: Cover branches of FloatWindowManager::FindFloatViewByFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(FloatWindowManagerTest, FindFloatViewByFloatingBall_Test, TestSize.Level1)
{
    FloatWindowManager::floatViewToFloatingBallMap_.clear();
    EXPECT_EQ(nullptr, FloatWindowManager::FindFloatViewByFloatingBall(nullptr));
    EXPECT_EQ(nullptr, FloatWindowManager::FindFloatViewByFloatingBall(fbController_));

    FloatWindowManager::floatViewToFloatingBallMap_.insert(std::make_pair(fvController_, nullptr));
    FloatWindowManager::floatViewToFloatingBallMap_.insert(std::make_pair(nullptr, fbController_));
    EXPECT_EQ(nullptr, FloatWindowManager::FindFloatViewByFloatingBall(fbController_));
}

/**
 * @tc.name: ProcessBindFloatViewStateChange_AllBranches_Test
 * @tc.desc: Merge & cover all branches of FloatWindowManager::ProcessBindFloatViewStateChange in one case
 * @tc.type: FUNC
 */
HWTEST_F(FloatWindowManagerTest, ProcessBindFloatViewStateChange_AllBranches_Test, TestSize.Level1)
{
    // branch 1: fvController is null => return directly
    FloatWindowManager::floatViewToFloatingBallMap_.clear();
    wptr<FloatViewController> nullFv;
    FloatWindowManager::ProcessBindFloatViewStateChange(nullFv, FvWindowState::FV_STATE_STARTED);

    // branch 2: no relation in map => return directly
    FloatWindowManager::floatViewToFloatingBallMap_.clear();
    FloatWindowManager::ProcessBindFloatViewStateChange(fvController_, FvWindowState::FV_STATE_STARTED);

    // branch 3: relation exists but mapped fb is nullptr => return directly
    FloatWindowManager::floatViewToFloatingBallMap_.clear();
    FloatWindowManager::floatViewToFloatingBallMap_.insert(std::make_pair(fvController_, nullptr));
    FloatWindowManager::ProcessBindFloatViewStateChange(fvController_, FvWindowState::FV_STATE_STARTED);
    
    // branch 4: state == FV_STATE_STARTED => fv true, fb false + early return
    FloatWindowManager::floatViewToFloatingBallMap_.clear();
    FloatWindowManager::floatViewToFloatingBallMap_.insert(std::make_pair(fvController_, fbController_));
    FloatWindowManager::ProcessBindFloatViewStateChange(fvController_, FvWindowState::FV_STATE_STARTED);
    EXPECT_EQ(true, fvController_->option_.showWhenCreate_);

    // branch 5: state == FV_STATE_IN_FLOATING_BALL => fv false, fb true
    FloatWindowManager::floatViewToFloatingBallMap_.clear();
    FloatWindowManager::floatViewToFloatingBallMap_.insert(std::make_pair(fvController_, fbController_));
    FloatWindowManager::ProcessBindFloatViewStateChange(fvController_, FvWindowState::FV_STATE_IN_FLOATING_BALL);
    EXPECT_EQ(false, fvController_->option_.showWhenCreate_);

    // branch 6: other state => no SetShowWhenCreate called
    FloatWindowManager::floatViewToFloatingBallMap_.clear();
    FloatWindowManager::floatViewToFloatingBallMap_.insert(std::make_pair(fvController_, fbController_));
    FloatWindowManager::ProcessBindFloatViewStateChange(fvController_, FvWindowState::FV_STATE_STOPPED);
    EXPECT_EQ(false, fvController_->option_.showWhenCreate_);
}
}
}
}