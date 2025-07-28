/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "parameters.h"
#include "floating_ball_controller.h"
#include "floating_ball_manager.h"
#include "window.h"
#include "wm_common.h"
#include "ability_context_impl.h"

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
    MOCK_METHOD0(NotifyPrepareCloseFloatingBall, void());
    MOCK_METHOD2(UpdateFloatingBall, WMError(const FloatingBallTemplateBaseInfo& fbTemplateBaseInfo,
        const std::shared_ptr<Media::PixelMap>& icon));
    MOCK_METHOD1(RestoreFbMainWindow, WMError(const std::shared_ptr<AAFwk::Want>& want));
    MOCK_METHOD1(GetFloatingBallWindowId, WMError(uint32_t& windowId));
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

class MockLifeCycleListener : public IFbLifeCycle {
public:
    void OnFloatingBallStart() override
    {
        return;
    }
    void OnFloatingBallStop() override
    {
        return;
    }
};

class MockClickListener : public IFbClickObserver {
public:

    void OnClickEvent() override
    {
        return;
    }
};

class FloatingBallControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<FloatingBallController> fbController_;
    sptr<FbOption> option_;
    sptr<MockWindow> mw_;
};

void FloatingBallControllerTest::SetUpTestCase()
{
}

void FloatingBallControllerTest::TearDownTestCase()
{
}

void FloatingBallControllerTest::SetUp()
{
    int32_t mockWindowId = 100;
    mw_ = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw_);
    fbController_ = sptr<FloatingBallController>::MakeSptr(mw_, mockWindowId, nullptr);
    ASSERT_NE(nullptr, fbController_);
    option_ = sptr<FbOption>::MakeSptr();
    ASSERT_NE(nullptr, option_);
}

void FloatingBallControllerTest::TearDown()
{
    fbController_ = nullptr;
    option_ = nullptr;
    mw_ = nullptr;
}

namespace {
/**
 * @tc.name: CreateFloatingBallWindow
 * @tc.desc: CreateFloatingBallWindow
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallControllerTest, CreateFloatingBallWindow01, TestSize.Level1)
{
    std::shared_ptr<AbilityRuntime::AbilityContextImpl> contextPtr =
        std::make_shared<AbilityRuntime::AbilityContextImpl>();
    fbController_->contextPtr_ = &contextPtr;
    fbController_->fbOption_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, fbController_->CreateFloatingBallWindow());
    fbController_->fbOption_ = option_;
    mw_->SetWindowState(WindowState::STATE_INITIAL);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, fbController_->CreateFloatingBallWindow());
    mw_->SetWindowState(WindowState::STATE_SHOWN);
    EXPECT_EQ(WMError::WM_ERROR_FB_CREATE_FAILED, fbController_->CreateFloatingBallWindow());

    fbController_->CreateFloatingBallWindow();
    fbController_->UpdateMainWindow(nullptr);
    EXPECT_EQ(100, fbController_->mainWindowId_);
    fbController_->UpdateMainWindow(mw_);
    EXPECT_EQ(101, fbController_->mainWindowId_);
    fbController_->contextPtr_ = nullptr;
}

/**
 * @tc.name: StartFloatingBall
 * @tc.desc: StartFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallControllerTest, StartFloatingBall01, TestSize.Level1)
{
    sptr<FbOption> nullOption = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, fbController_->StartFloatingBall(nullOption));
    fbController_->curState_ = FbWindowState::STATE_STARTING;
    EXPECT_EQ(WMError::WM_ERROR_FB_REPEAT_OPERATION, fbController_->StartFloatingBall(option_));
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_FB_REPEAT_OPERATION, fbController_->StartFloatingBall(option_));

    fbController_->curState_ = FbWindowState::STATE_UNDEFINED;
    FloatingBallManager::SetActiveController(fbController_);
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, fbController_->StartFloatingBall(option_));
    FloatingBallManager::RemoveActiveController(fbController_);

    auto activeFbController = sptr<FloatingBallController>::MakeSptr(mw_, 100, nullptr);
    FloatingBallManager::SetActiveController(activeFbController);
    EXPECT_EQ(WMError::WM_ERROR_FB_REPEAT_CONTROLLER, fbController_->StartFloatingBall(option_));

    FloatingBallManager::RemoveActiveController(activeFbController);
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, fbController_->StartFloatingBall(option_));
    EXPECT_EQ(FbWindowState::STATE_UNDEFINED, fbController_->curState_);

    std::shared_ptr<AbilityRuntime::AbilityContextImpl> contextPtr =
        std::make_shared<AbilityRuntime::AbilityContextImpl>();
    fbController_->contextPtr_ = &contextPtr;
    EXPECT_NE(WMError::WM_OK, fbController_->StartFloatingBall(option_));
    EXPECT_EQ(FbWindowState::STATE_UNDEFINED, fbController_->GetControllerState());
    fbController_->contextPtr_ = nullptr;
}

/**
 * @tc.name: LifeCycleTest
 * @tc.desc: LifeCycleTest
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallControllerTest, LifeCycleTest01, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_ERROR_FB_INTERNAL_ERROR, fbController_->RegisterFbLifecycle(nullptr));
    auto lifeListener = sptr<MockLifeCycleListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, fbController_->RegisterFbLifecycle(lifeListener));
    // repeat register
    EXPECT_EQ(WMError::WM_OK, fbController_->RegisterFbLifecycle(lifeListener));
    fbController_->fbLifeCycleListeners_.emplace_back(nullptr);
    fbController_->OnFloatingBallStart();
    fbController_->OnFloatingBallStop();
    // unregister
    EXPECT_EQ(WMError::WM_ERROR_FB_INTERNAL_ERROR, fbController_->UnRegisterFbLifecycle(nullptr));
    EXPECT_EQ(WMError::WM_OK, fbController_->UnRegisterFbLifecycle(lifeListener));
    fbController_->fbLifeCycleListeners_.clear();
}

/**
 * @tc.name: ClickTest
 * @tc.desc: ClickTest
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallControllerTest, ClickTest01, TestSize.Level1)
{
    EXPECT_EQ(WMError::WM_ERROR_FB_INTERNAL_ERROR, fbController_->RegisterFbClickObserver(nullptr));
    auto clickListener = sptr<MockClickListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, fbController_->RegisterFbClickObserver(clickListener));
    // repeat register
    EXPECT_EQ(WMError::WM_OK, fbController_->RegisterFbClickObserver(clickListener));
    fbController_->fbClickObservers_.emplace_back(nullptr);
    fbController_->OnFloatingBallClick();
    // unregister
    EXPECT_EQ(WMError::WM_ERROR_FB_INTERNAL_ERROR, fbController_->UnRegisterFbClickObserver(nullptr));
    EXPECT_EQ(WMError::WM_OK, fbController_->UnRegisterFbClickObserver(clickListener));
    fbController_->fbClickObservers_.clear();
}

/**
 * @tc.name: UpdateFloatingBall
 * @tc.desc: UpdateFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallControllerTest, UpdateFloatingBall01, TestSize.Level1)
{
    fbController_->curState_ = FbWindowState::STATE_STOPPED;
    EXPECT_EQ(WMError::WM_ERROR_FB_INVALID_STATE, fbController_->UpdateFloatingBall(option_));

    fbController_->curState_ = FbWindowState::STATE_STARTED;
    sptr<FbOption> nullOption = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, fbController_->UpdateFloatingBall(nullOption));
    fbController_->window_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, fbController_->UpdateFloatingBall(option_));

    fbController_->window_ = mw_;
    fbController_->curState_ = FbWindowState::STATE_STOPPED;
    EXPECT_EQ(WMError::WM_ERROR_FB_INVALID_STATE, fbController_->UpdateFloatingBall(option_));
    EXPECT_CALL(*(mw_), UpdateFloatingBall(_, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    EXPECT_EQ(WMError::WM_OK, fbController_->UpdateFloatingBall(option_));
}

/**
 * @tc.name: StopFloatingBall
 * @tc.desc: StopFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallControllerTest, StopFloatingBall01, TestSize.Level1)
{
    fbController_->curState_ = FbWindowState::STATE_STOPPED;
    EXPECT_EQ(WMError::WM_ERROR_FB_REPEAT_OPERATION, fbController_->StopFloatingBallFromClient());
    fbController_->curState_ = FbWindowState::STATE_STOPPING;
    EXPECT_EQ(WMError::WM_ERROR_FB_REPEAT_OPERATION, fbController_->StopFloatingBallFromClient());
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    EXPECT_CALL(*(mw_), NotifyPrepareCloseFloatingBall()).Times(1);
    fbController_->window_ = mw_;
    EXPECT_EQ(WMError::WM_OK, fbController_->StopFloatingBallFromClient());
    fbController_->window_ = nullptr;
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, fbController_->StopFloatingBallFromClient());
}

/**
 * @tc.name: StopFloatingBall
 * @tc.desc: StopFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallControllerTest, StopFloatingBall02, TestSize.Level1)
{
    fbController_->stopFromClient_ = false;
    fbController_->curState_ = FbWindowState::STATE_STOPPING;
    EXPECT_EQ(WMError::WM_ERROR_FB_REPEAT_OPERATION, fbController_->StopFloatingBall());
    fbController_->curState_ = FbWindowState::STATE_STOPPED;
    EXPECT_EQ(WMError::WM_ERROR_FB_REPEAT_OPERATION, fbController_->StopFloatingBall());

    fbController_->curState_ = FbWindowState::STATE_STARTED;
    EXPECT_CALL(*(mw_), Destroy(_)).Times(1);
    fbController_->window_ = mw_;
    EXPECT_EQ(WMError::WM_OK, fbController_->StopFloatingBall());
    EXPECT_EQ(FbWindowState::STATE_STOPPED, fbController_->GetControllerState());

    fbController_->window_ = nullptr;
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, fbController_->StopFloatingBall());
    EXPECT_EQ(WMError::WM_ERROR_FB_INTERNAL_ERROR, fbController_->DestroyFloatingBallWindow());
    EXPECT_EQ(nullptr, fbController_->GetFbWindow());
}

/**
 * @tc.name: RestoreFloatingBallAbility
 * @tc.desc: RestoreFloatingBallAbility
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallControllerTest, RestoreFloatingBallAbility, TestSize.Level1)
{
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, fbController_->RestoreMainWindow(want));
    fbController_->window_ = mw_;
    fbController_->curState_ = FbWindowState::STATE_STOPPING;
    EXPECT_EQ(WMError::WM_ERROR_FB_INVALID_STATE, fbController_->RestoreMainWindow(want));
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    EXPECT_CALL(*(mw_), RestoreFbMainWindow(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    EXPECT_EQ(WMError::WM_OK, fbController_->RestoreMainWindow(want));
}

/**
 * @tc.name: GetFloatingBallInfo
 * @tc.desc: GetFloatingBallInfo
 * @tc.type: FUNC
 */
HWTEST_F(FloatingBallControllerTest, GetFloatingBallInfo, TestSize.Level1)
{
    EXPECT_CALL(*(mw_), GetFloatingBallWindowId(_)).Times(1).WillOnce(Return(WMError::WM_OK));
    fbController_->window_ = mw_;
    uint32_t mockId = 1;
    EXPECT_EQ(WMError::WM_ERROR_FB_INVALID_STATE, fbController_->GetFloatingBallWindowInfo(mockId));
    fbController_->curState_ = FbWindowState::STATE_STARTED;
    fbController_->window_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_FB_STATE_ABNORMALLY, fbController_->GetFloatingBallWindowInfo(mockId));
    fbController_->window_ = mw_;
    EXPECT_EQ(WMError::WM_OK, fbController_->GetFloatingBallWindowInfo(mockId));
}
}
}
}