/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#include "mock_session.h"
#include "parameters.h"
#include "picture_in_picture_controller.h"
#include "picture_in_picture_manager.h"
#include "window.h"
#include "window_session_impl.h"
#include "wm_common.h"
#include "xcomponent_controller.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class MockWindow : public Window {
public:
    MockWindow() {};
    ~MockWindow() {};
    MOCK_METHOD3(Show, WMError(uint32_t reason, bool withAnimation, bool withFocus));
    MOCK_METHOD1(Destroy, WMError(uint32_t reason));
    MOCK_METHOD0(NotifyPrepareClosePiPWindow, WMError());
    MOCK_METHOD4(SetAutoStartPiP, void(bool isAutoStart, uint32_t priority, uint32_t width, uint32_t height));
    MOCK_CONST_METHOD0(GetWindowState, WindowState());
};

class MockXComponentController : public XComponentController {
public:
    MockXComponentController() {}
    ~MockXComponentController() {}
    MOCK_METHOD2(GetGlobalPosition, XComponentControllerErrorCode(float& offsetX, float& offsetY));
    MOCK_METHOD2(GetSize, XComponentControllerErrorCode(float& width, float& height));
    MOCK_METHOD1(SetExtController,
                 XComponentControllerErrorCode(std::shared_ptr<XComponentController> xComponentController));
    MOCK_METHOD1(ResetExtController,
                 XComponentControllerErrorCode(std::shared_ptr<XComponentController> xComponentController));
};

class MockPictureInPictureController : public PictureInPictureController {
public:
    MockPictureInPictureController(sptr<PipOption> pipOption, sptr<Window> mainWindow, uint32_t mainWindowId,
        napi_env env) : PictureInPictureController(pipOption, mainWindow, mainWindowId, env) {}
    ~MockPictureInPictureController() {};
    MOCK_METHOD1(GetNavigationController, NavigationController*(const std::string& navId));
};

class MockNavigationController : public NavigationController {
public:
    void SetInPIPMode(int32_t handle)
    {
        handle_ = handle;
    }

    void DeletePIPMode(int32_t handle)
    {
        handle_ = 0;
    }

    MOCK_METHOD0(IsNavDestinationInTopStack, bool());
    MOCK_METHOD0(GetTopHandle, int32_t());
public:
    int handle_ {0};
};

class PictureInPictureControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;

    sptr<MockPictureInPictureController> pipControl_ {nullptr};
    std::shared_ptr<MockNavigationController> naviControl_ {nullptr};
};

void PictureInPictureControllerTest::SetUpTestCase() {}

void PictureInPictureControllerTest::TearDownTestCase()
{
}

void PictureInPictureControllerTest::SetUp()
{
    sptr<MockWindow> mw = sptr<MockWindow>::MakeSptr();
    sptr<PipOption> option = sptr<PipOption>::MakeSptr();
    uint32_t mainWindowId = 100;
    pipControl_ = sptr<MockPictureInPictureController>::MakeSptr(option, mw, mainWindowId, nullptr);
    ASSERT_NE(nullptr, pipControl_);
    naviControl_ = std::make_shared<MockNavigationController>();
    ASSERT_NE(nullptr, naviControl_);
}

void PictureInPictureControllerTest::TearDown()
{
    pipControl_ = nullptr;
    naviControl_ = nullptr;
}

class MockPiPActiveStatus : public IPiPActiveStatusObserver {
public:
    void OnActiveStatusChange(bool status) override
    {
        return;
    }
};

namespace {
/**
 * @tc.name: ShowPictureInPictureWindow01
 * @tc.desc: ShowPictureInPictureWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, ShowPictureInPictureWindow01, TestSize.Level1)
{
    StartPipType startType = StartPipType::NULL_START;
    sptr<MockWindow> mw = new (std::nothrow) MockWindow();
    ASSERT_NE(nullptr, mw);
    sptr<MockWindow> mw1 = new (std::nothrow) MockWindow();
    ASSERT_NE(nullptr, mw1);
    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipControl =
        new (std::nothrow) PictureInPictureController(option, mw, 100, nullptr);

    pipControl->pipOption_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->ShowPictureInPictureWindow(startType));
    pipControl->pipOption_ = option;

    pipControl->window_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_PIP_STATE_ABNORMALLY, pipControl->ShowPictureInPictureWindow(startType));
    pipControl->window_ = mw;

    auto listener = sptr<IPiPLifeCycle>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    pipControl->RegisterPiPLifecycle(listener);
    EXPECT_CALL(*(mw), Show(_, _, _)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR, pipControl->ShowPictureInPictureWindow(startType));
    EXPECT_CALL(*(mw), Show(_, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, pipControl->ShowPictureInPictureWindow(startType));
    EXPECT_CALL(*(mw), Show(_, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    startType = StartPipType::AUTO_START;
    ASSERT_EQ(WMError::WM_OK, pipControl->ShowPictureInPictureWindow(startType));
    startType = StartPipType::NULL_START;
    pipControl->pipOption_->SetContentSize(10, 10);
    EXPECT_CALL(*(mw), Show(_, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, pipControl->ShowPictureInPictureWindow(startType));
    pipControl->pipOption_->SetContentSize(0, 10);
    EXPECT_CALL(*(mw), Show(_, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, pipControl->ShowPictureInPictureWindow(startType));
    pipControl->pipOption_->SetContentSize(10, 0);
    EXPECT_CALL(*(mw), Show(_, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, pipControl->ShowPictureInPictureWindow(startType));
    pipControl->pipOption_->SetContentSize(0, 0);
    EXPECT_CALL(*(mw), Show(_, _, _)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, pipControl->ShowPictureInPictureWindow(startType));
}

/**
 * @tc.name: StopPictureInPicture01
 * @tc.desc: StopPictureInPicture
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, StopPictureInPicture01, TestSize.Level1)
{
    sptr<MockWindow> mw = new (std::nothrow) MockWindow();
    ASSERT_NE(nullptr, mw);
    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipControl =
        new (std::nothrow) PictureInPictureController(option, mw, 100, nullptr);

    pipControl->curState_ = PiPWindowState::STATE_STOPPING;
    pipControl->isStoppedFromClient_ = true;
    ASSERT_EQ(WMError::WM_ERROR_PIP_STATE_ABNORMALLY, pipControl->StopPictureInPicture(true, StopPipType::NULL_STOP));

    pipControl->isStoppedFromClient_ = false;
    ASSERT_EQ(WMError::WM_ERROR_PIP_REPEAT_OPERATION, pipControl->StopPictureInPicture(true, StopPipType::NULL_STOP));

    pipControl->curState_ = PiPWindowState::STATE_STOPPED;
    ASSERT_EQ(WMError::WM_ERROR_PIP_REPEAT_OPERATION, pipControl->StopPictureInPicture(true, StopPipType::NULL_STOP));
    pipControl->curState_ = PiPWindowState::STATE_UNDEFINED;
    ASSERT_EQ(WMError::WM_ERROR_PIP_STATE_ABNORMALLY, pipControl->StopPictureInPicture(false, StopPipType::NULL_STOP));

    pipControl->window_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_PIP_STATE_ABNORMALLY, pipControl->StopPictureInPicture(true, StopPipType::NULL_STOP));

    pipControl->window_ = mw;
    pipControl->isStoppedFromClient_ = true;
    pipControl->curState_ = PiPWindowState::STATE_STOPPING;
    ASSERT_EQ(WMError::WM_OK, pipControl->StopPictureInPicture(false, StopPipType::NULL_STOP));

    pipControl->window_ = mw;
    pipControl->curState_ = PiPWindowState::STATE_STARTED;
    ASSERT_EQ(PiPWindowState::STATE_STARTED, pipControl->GetControllerState());
    ASSERT_EQ(WMError::WM_OK, pipControl->StopPictureInPicture(true, StopPipType::NULL_STOP));
    ASSERT_NE(PiPWindowState::STATE_STARTED, pipControl->GetControllerState());
}

/**
 * @tc.name: CreatePictureInPictureWindow
 * @tc.desc: CreatePictureInPictureWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, CreatePictureInPictureWindow01, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    pipControl->pipOption_ = nullptr;
    StartPipType startType = StartPipType::AUTO_START;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->CreatePictureInPictureWindow(startType));
    pipControl->pipOption_ = option;
    option->SetContext(nullptr);
    ASSERT_EQ(nullptr, option->GetContext());
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->CreatePictureInPictureWindow(startType));
    AbilityRuntime::AbilityContextImpl* contextPtr = new AbilityRuntime::AbilityContextImpl();
    option->SetContext(contextPtr);
    pipControl->mainWindow_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->CreatePictureInPictureWindow(startType));
    delete contextPtr;
}

/**
 * @tc.name: CreatePictureInPictureWindow
 * @tc.desc: CreatePictureInPictureWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, CreatePictureInPictureWindow02, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    pipControl->pipOption_ = option;
    StartPipType startType = StartPipType::AUTO_START;
    AbilityRuntime::AbilityContextImpl* contextPtr = new AbilityRuntime::AbilityContextImpl();
    option->SetContext(contextPtr);

    std::shared_ptr<MockXComponentController> xComponentController = std::make_shared<MockXComponentController>();
    ASSERT_NE(nullptr, xComponentController);
    pipControl->pipOption_->SetXComponentController(nullptr);
    pipControl->pipOption_->SetTypeNodeEnabled(false);
    pipControl->mainWindow_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->CreatePictureInPictureWindow(startType));
    pipControl->pipOption_->SetXComponentController(xComponentController);
    ASSERT_EQ(pipControl->pipOption_->GetXComponentController(), xComponentController);
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->CreatePictureInPictureWindow(startType));
    pipControl->pipOption_->SetXComponentController(nullptr);
    pipControl->mainWindow_ = mw;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->CreatePictureInPictureWindow(startType));
    delete contextPtr;
}

/**
 * @tc.name: CreatePictureInPictureWindow
 * @tc.desc: CreatePictureInPictureWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, CreatePictureInPictureWindow03, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    pipControl->pipOption_ = option;
    option->SetDefaultWindowSizeType(1);
    AbilityRuntime::AbilityContextImpl* contextPtr = new AbilityRuntime::AbilityContextImpl();
    option->SetContext(contextPtr);
    std::shared_ptr<MockXComponentController> xComponentController = std::make_shared<MockXComponentController>();
    pipControl->pipOption_->SetXComponentController(xComponentController);
    ASSERT_EQ(pipControl->pipOption_->GetXComponentController(), xComponentController);
    pipControl->pipOption_->SetTypeNodeEnabled(true);
    pipControl->mainWindow_ = mw;
    StartPipType startType = StartPipType::NULL_START;
    EXPECT_CALL(*(mw), GetWindowState()).Times(2).WillOnce(Return(WindowState::STATE_CREATED));
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->CreatePictureInPictureWindow(startType));
    EXPECT_EQ(1, option->GetDefaultWindowSizeType());
    startType = StartPipType::AUTO_START;
    delete contextPtr;
}

/**
 * @tc.name: StartPictureInPicture
 * @tc.desc: StartPictureInPicture
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, StartPictureInPicture01, TestSize.Level1)
{
    StartPipType startType = StartPipType::AUTO_START;
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    pipControl->pipOption_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->StartPictureInPicture(startType));
    pipControl->pipOption_ = option;
    option->SetContext(nullptr);
    ASSERT_EQ(nullptr, option->GetContext());
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->StartPictureInPicture(startType));
    AbilityRuntime::AbilityContextImpl* contextPtr = new AbilityRuntime::AbilityContextImpl();
    option->SetContext(contextPtr);
    pipControl->curState_ = PiPWindowState::STATE_STARTING;
    EXPECT_EQ(WMError::WM_ERROR_PIP_REPEAT_OPERATION, pipControl->StartPictureInPicture(startType));
    pipControl->curState_ = PiPWindowState::STATE_STARTED;
    EXPECT_EQ(WMError::WM_ERROR_PIP_REPEAT_OPERATION, pipControl->StartPictureInPicture(startType));
    delete contextPtr;
}

/**
 * @tc.name: StartPictureInPicture
 * @tc.desc: StartPictureInPicture
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, StartPictureInPicture02, TestSize.Level1)
{
    StartPipType startType = StartPipType::AUTO_START;
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    pipControl->pipOption_ = option;
    AbilityRuntime::AbilityContextImpl* contextPtr = new AbilityRuntime::AbilityContextImpl();
    option->SetContext(contextPtr);
    pipControl->curState_ = PiPWindowState::STATE_UNDEFINED;

    pipControl->pipOption_->SetNavigationId("navId");
    pipControl->mainWindow_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->StartPictureInPicture(startType));
    pipControl->pipOption_->SetNavigationId("");
    auto pipControl1 = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    auto pipControl2 = sptr<PictureInPictureController>::MakeSptr(option, mw, 200, nullptr);
    PictureInPictureManager::SetActiveController(pipControl1);
    PictureInPictureManager::IsActiveController(pipControl2);
    pipControl->mainWindowId_ = 100;
    PictureInPictureManager::IsAttachedToSameWindow(100);
    pipControl->window_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->StartPictureInPicture(startType));
    pipControl->window_ = mw;
    pipControl->pipOption_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_PIP_CREATE_FAILED, pipControl->StartPictureInPicture(startType));
    pipControl->pipOption_ = option;
    PictureInPictureManager::RemoveActiveController(pipControl1);
    PictureInPictureManager::IsActiveController(pipControl);
    pipControl->StartPictureInPicture(startType);
    delete contextPtr;
}

/**
 * @tc.name: StartPictureInPictureInner
 * @tc.desc: StartPictureInPictureInner
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, StartPictureInPictureInner, TestSize.Level1)
{
    StartPipType startType = StartPipType::USER_START;
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    pipControl->pipOption_->SetTypeNodeEnabled(true);
    ASSERT_NE(WMError::WM_OK, pipControl->StartPictureInPictureInner(startType));
}

/**
 * @tc.name: StopPictureInPictureFromClient
 * @tc.desc: StopPictureInPictureFromClient
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, StopPictureInPictureFromClient, TestSize.Level1)
{
    sptr<MockWindow> mw = new (std::nothrow) MockWindow();
    ASSERT_NE(nullptr, mw);
    sptr<MockWindow> mw1 = new (std::nothrow) MockWindow();
    ASSERT_NE(nullptr, mw1);
    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipControl =
        new (std::nothrow) PictureInPictureController(option, mw, 100, nullptr);

    pipControl->window_ = mw1;
    ASSERT_NE(WMError::WM_ERROR_PIP_STATE_ABNORMALLY, pipControl->StopPictureInPictureFromClient());
    pipControl->window_ = nullptr;
    ASSERT_NE(WMError::WM_OK, pipControl->StopPictureInPictureFromClient());
    pipControl->window_ = mw1;
    pipControl->curState_ = PiPWindowState::STATE_STOPPING;
    EXPECT_EQ(WMError::WM_ERROR_PIP_REPEAT_OPERATION, pipControl->StopPictureInPictureFromClient());
    pipControl->curState_ = PiPWindowState::STATE_STOPPED;
    EXPECT_EQ(WMError::WM_ERROR_PIP_REPEAT_OPERATION, pipControl->StopPictureInPictureFromClient());
    pipControl->curState_ = PiPWindowState::STATE_RESTORING;
    EXPECT_EQ(WMError::WM_ERROR_PIP_REPEAT_OPERATION, pipControl->StopPictureInPictureFromClient());
    pipControl->curState_ = PiPWindowState::STATE_UNDEFINED;
    EXPECT_CALL(*(mw1), NotifyPrepareClosePiPWindow()).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    EXPECT_EQ(WMError::WM_ERROR_PIP_DESTROY_FAILED, pipControl->StopPictureInPictureFromClient());
    EXPECT_CALL(*(mw1), NotifyPrepareClosePiPWindow()).Times(1).WillOnce(Return(WMError::WM_OK));
    EXPECT_EQ(WMError::WM_OK, pipControl->StopPictureInPictureFromClient());
}

/**
 * @tc.name: GetPipWindow
 * @tc.desc: GetPipWindow/SetPipWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, GetPipWindow, TestSize.Level1)
{
    sptr<MockWindow> mw = new MockWindow();
    sptr<PipOption> option = new PipOption();
    sptr<Window> window_;
    sptr<Window> window;
    sptr<PictureInPictureController> pipControl = new PictureInPictureController(option, mw, 100, nullptr);

    pipControl->SetPipWindow(window);
    auto ret = pipControl->GetPipWindow();
    ASSERT_EQ(pipControl->window_, ret);
}

/**
 * @tc.name: SetAutoStartEnabled
 * @tc.desc: SetAutoStartEnabled
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, SetAutoStartEnabled, TestSize.Level1)
{
    bool enable = true;
    sptr<MockWindow> mw = new (std::nothrow) MockWindow();
    ASSERT_NE(nullptr, mw);
    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipControl =
        new (std::nothrow) PictureInPictureController(option, mw, 100, nullptr);

    pipControl->mainWindow_ = nullptr;
    pipControl->SetAutoStartEnabled(enable);
    pipControl->mainWindow_ = mw;
    pipControl->pipOption_ = nullptr;
    pipControl->SetAutoStartEnabled(enable);
    pipControl->pipOption_ = option;

    pipControl->isAutoStartEnabled_ = enable;
    ASSERT_EQ(true, pipControl->isAutoStartEnabled_);
    pipControl->pipOption_->SetTypeNodeEnabled(true);
    ASSERT_EQ(true, pipControl->IsTypeNodeEnabled());
    EXPECT_CALL(*(mw), SetAutoStartPiP(_, _, _, _)).WillRepeatedly(Return());
    pipControl->SetAutoStartEnabled(enable);
    enable = false;
    pipControl->isAutoStartEnabled_ = enable;
    ASSERT_EQ(false, pipControl->isAutoStartEnabled_);
    pipControl->pipOption_->SetTypeNodeEnabled(true);
    ASSERT_EQ(true, pipControl->IsTypeNodeEnabled());
    EXPECT_CALL(*(mw), SetAutoStartPiP(_, _, _, _)).WillRepeatedly(Return());
    pipControl->SetAutoStartEnabled(enable);
    pipControl->pipOption_->SetTypeNodeEnabled(false);
    ASSERT_EQ(false, pipControl->IsTypeNodeEnabled());
    EXPECT_CALL(*(mw), SetAutoStartPiP(_, _, _, _)).WillRepeatedly(Return());
    pipControl->SetAutoStartEnabled(enable);
    pipControl->pipOption_->SetNavigationId("");
    ASSERT_EQ("", pipControl->pipOption_->GetNavigationId());
    pipControl->SetAutoStartEnabled(enable);
}

/**
 * @tc.name: IsAutoStartEnabled
 * @tc.desc: IsAutoStartEnabled
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, IsAutoStartEnabled, TestSize.Level1)
{
    bool enable = true;
    sptr<MockWindow> mw = new MockWindow();
    sptr<PipOption> option = new PipOption();
    sptr<PictureInPictureController> pipControl = new PictureInPictureController(option, mw, 100, nullptr);

    pipControl->IsAutoStartEnabled(enable);
    auto ret = pipControl->GetControllerState();
    ASSERT_EQ(PiPWindowState::STATE_UNDEFINED, ret);
}

/**
 * @tc.name: UpdateContentSize01
 * @tc.desc: UpdateContentSize
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, UpdateContentSize01, TestSize.Level1)
{
    sptr<MockWindow> mw = new (std::nothrow) MockWindow();
    ASSERT_NE(nullptr, mw);
    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipControl =
        new (std::nothrow) PictureInPictureController(option, mw, 100, nullptr);

    int32_t width = 0;
    int32_t height = 0;
    pipControl->UpdateContentSize(width, height);
    height = 150;
    pipControl->UpdateContentSize(width, height);
    height = 0;
    width = 100;
    pipControl->UpdateContentSize(width, height);
    height = 150;
    pipControl->UpdateContentSize(width, height);

    pipControl->curState_ = PiPWindowState::STATE_UNDEFINED;
    pipControl->UpdateContentSize(width, height);
    pipControl->curState_ = PiPWindowState::STATE_STARTED;
    pipControl->UpdateContentSize(width, height);
}

/**
 * @tc.name: UpdateContentSize02
 * @tc.desc: UpdateContentSize
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, UpdateContentSize02, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    std::shared_ptr<MockXComponentController> xComponentController = std::make_shared<MockXComponentController>();
    ASSERT_NE(nullptr, xComponentController);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    pipControl->curState_ = PiPWindowState::STATE_STARTED;
    pipControl->window_ = nullptr;
    int32_t width = 10;
    int32_t height = 20;
    pipControl->UpdateContentSize(width, height);
    pipControl->window_ = mw;

    pipControl->pipOption_->SetTypeNodeEnabled(true);
    ASSERT_EQ(true, pipControl->IsTypeNodeEnabled());
    pipControl->mainWindowXComponentController_ = xComponentController;
    pipControl->UpdateContentSize(width, height);
    pipControl->pipOption_->SetTypeNodeEnabled(false);
    pipControl->windowRect_ = { 0, 0, 0, 0 };
    pipControl->IsContentSizeChanged(0, 0, 0, 0);
    pipControl->UpdateContentSize(width, height);
    pipControl->IsContentSizeChanged(10, 10, 10, 10);
    pipControl->UpdateContentSize(width, height);
    pipControl->mainWindowXComponentController_ = nullptr;
    pipControl->UpdateContentSize(width, height);
}

/**
 * @tc.name: UpdatePiPControlStatus
 * @tc.desc: UpdatePiPControlStatus
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, UpdatePiPControlStatus, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    auto status = PiPControlStatus::ENABLED;
    pipControl->UpdatePiPControlStatus(controlType, status);
    ASSERT_EQ(1, pipControl->pipOption_->GetControlEnable().size());
    status = PiPControlStatus::PLAY;
    pipControl->UpdatePiPControlStatus(controlType, status);
    ASSERT_EQ(1, pipControl->pipOption_->GetControlStatus().size());
    pipControl->window_ = nullptr;
    pipControl->UpdatePiPControlStatus(controlType, status);
    pipControl->window_ = mw;
    pipControl->UpdatePiPControlStatus(controlType, status);
}

/**
 * @tc.name: IsContentSizeChanged
 * @tc.desc: IsContentSizeChanged
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, IsContentSizeChanged, TestSize.Level1)
{
    sptr<MockWindow> mw = new (std::nothrow) MockWindow();
    ASSERT_NE(nullptr, mw);
    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipControl =
        new (std::nothrow) PictureInPictureController(option, mw, 100, nullptr);
    pipControl->windowRect_ = { 0, 0, 0, 0 };
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(10.5, 0, 0, 0));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(0, 10.5, 0, 0));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(0, 0, 10.5, 0));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(0, 0, 0, 10.5));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(10.5, 10.5, 0, 0));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(10.5, 0, 10.5, 0));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(10.5, 0, 0, 10.5));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(0, 10.5, 10.5, 0));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(0, 10.5, 0, 10.5));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(0, 0, 10.5, 10.5));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(10.5, 10.5, 10.5, 0));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(10.5, 10.5, 0, 10.5));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(10.5, 0, 10.5, 10.5));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(0, 10.5, 10.5, 10.5));
    ASSERT_EQ(true, pipControl->IsContentSizeChanged(10.5, 10.5, 10.5, 10.5));
    ASSERT_EQ(false, pipControl->IsContentSizeChanged(0, 0, 0, 0));
}

/**
 * @tc.name: DoActionEvent
 * @tc.desc: DoActionEvent
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, DoActionEvent, TestSize.Level1)
{
    std::string actionName = "";
    int32_t status = 0;
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    sptr<IPiPActionObserver> listener = nullptr;

    pipControl->DoActionEvent(actionName, status);
    ASSERT_EQ(0, pipControl->pipOption_->GetControlStatus().size());
    actionName = "nextVideo";
    pipControl->DoActionEvent(actionName, status);
    ASSERT_EQ(1, pipControl->pipOption_->GetControlStatus().size());
}

/**
 * @tc.name: DoControlEvent
 * @tc.desc: DoControlEvent
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, DoControlEvent, TestSize.Level1)
{
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    auto status = PiPControlStatus::PLAY;
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    sptr<IPiPControlObserver> listener = nullptr;

    pipControl->pipOption_ = nullptr;
    pipControl->DoControlEvent(controlType, status);
    pipControl->pipOption_ = option;
    pipControl->DoControlEvent(controlType, status);
    pipControl->RegisterPiPControlObserver(listener);
    pipControl->DoControlEvent(controlType, status);
    ASSERT_EQ(1, pipControl->pipOption_->GetControlStatus().size());
}

/**
 * @tc.name: ActiveStatusChange
 * @tc.desc: ActiveStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, ActiveStatusChange, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    auto listener = sptr<MockPiPActiveStatus>::MakeSptr();

    pipControl->RegisterPiPActiveStatusChange(listener);
    pipControl->ActiveStatusChange(true);
    ASSERT_EQ(true, pipControl->curActiveStatus_);
}

/**
 * @tc.name: RegisterPiPActiveStatusChange
 * @tc.desc: RegisterPiPActiveStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, RegisterPiPActiveStatusChange, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    auto listener = sptr<MockPiPActiveStatus>::MakeSptr();

    pipControl->RegisterPiPActiveStatusChange(listener);
    ASSERT_EQ(pipControl->PiPActiveStatusObserver_.size(), 1);
    pipControl->UnregisterPiPActiveStatusChange(listener);
    ASSERT_EQ(pipControl->PiPActiveStatusObserver_.size(), 0);
}

/**
 * @tc.name: PreRestorePictureInPicture
 * @tc.desc: PreRestorePictureInPicture
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, PreRestorePictureInPicture, TestSize.Level1)
{
    sptr<IPiPLifeCycle> listener = nullptr;
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    pipControl->curState_ = PiPWindowState::STATE_STARTED;
    pipControl->RegisterPiPLifecycle(listener);
    pipControl->PreRestorePictureInPicture();
    ASSERT_EQ(PiPWindowState::STATE_RESTORING, pipControl->curState_);
}

/**
 * @tc.name: RestorePictureInPictureWindow
 * @tc.desc: RestorePictureInPictureWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, RestorePictureInPictureWindow, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    pipControl->curState_ = PiPWindowState::STATE_STARTED;
    pipControl->window_ = mw;
    pipControl->RestorePictureInPictureWindow();
    ASSERT_EQ(PiPWindowState::STATE_STOPPING, pipControl->curState_);
}

/**
 * @tc.name: UpdateWinRectByComponent
 * @tc.desc: UpdateWinRectByComponent
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, UpdateWinRectByComponent, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    std::shared_ptr<MockXComponentController> xComponentController = std::make_shared<MockXComponentController>();
    ASSERT_NE(nullptr, xComponentController);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    pipControl->pipOption_->SetTypeNodeEnabled(true);
    pipControl->UpdateWinRectByComponent();
    ASSERT_EQ(pipControl->windowRect_.width_, 16);
    ASSERT_EQ(pipControl->windowRect_.height_, 9);

    pipControl->pipOption_->SetTypeNodeEnabled(false);
    pipControl->mainWindowXComponentController_ = nullptr;
    pipControl->UpdateWinRectByComponent();
    pipControl->mainWindowXComponentController_ = xComponentController;

    pipControl->windowRect_.width_ = 0;
    pipControl->windowRect_.height_ = 10;
    pipControl->UpdateWinRectByComponent();
    ASSERT_EQ(pipControl->windowRect_.height_, 0);
    pipControl->windowRect_.width_ = 10;
    pipControl->windowRect_.height_ = 0;
    pipControl->UpdateWinRectByComponent();
    pipControl->windowRect_.width_ = 0;
    pipControl->UpdateWinRectByComponent();
    pipControl->windowRect_.width_ = 10;
    pipControl->windowRect_.height_ = 10;
    pipControl->UpdateWinRectByComponent();
    ASSERT_EQ(pipControl->windowRect_.posX_, 0);
    ASSERT_EQ(pipControl->windowRect_.posY_, 0);
}

/**
 * @tc.name: RegisterPiPLifecycle
 * @tc.desc: RegisterPiPLifecycle/UnregisterPiPLifecycle
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, RegisterListener, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    auto listener = sptr<IPiPLifeCycle>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    auto listener1 = sptr<IPiPLifeCycle>::MakeSptr();
    ASSERT_NE(nullptr, listener1);
    pipControl->pipLifeCycleListeners_.push_back(listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, pipControl->RegisterPiPLifecycle(nullptr));
    ASSERT_EQ(WMError::WM_OK, pipControl->RegisterPiPLifecycle(listener));
    ASSERT_EQ(WMError::WM_OK, pipControl->RegisterPiPLifecycle(listener1));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, pipControl->UnregisterPiPLifecycle(nullptr));
    ASSERT_EQ(WMError::WM_OK, pipControl->UnregisterPiPLifecycle(listener));
}

/**
 * @tc.name: RegisterPiPActionObserver
 * @tc.desc: RegisterPiPActionObserver/UnregisterPiPActionObserver
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, RegisterPiPActionObserver, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    auto listener = sptr<IPiPActionObserver>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    auto listener1 = sptr<IPiPActionObserver>::MakeSptr();
    ASSERT_NE(nullptr, listener1);
    pipControl->pipActionObservers_.push_back(listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, pipControl->RegisterPiPActionObserver(nullptr));
    ASSERT_EQ(WMError::WM_OK, pipControl->RegisterPiPActionObserver(listener));
    ASSERT_EQ(WMError::WM_OK, pipControl->RegisterPiPActionObserver(listener1));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, pipControl->UnregisterPiPActionObserver(nullptr));
    ASSERT_EQ(WMError::WM_OK, pipControl->UnregisterPiPActionObserver(listener));
}

/**
 * @tc.name: RegisterPiPControlObserver
 * @tc.desc: RegisterPiPControlObserver/UnregisterPiPControlObserver
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, RegisterPiPControlObserver, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    auto listener = sptr<IPiPControlObserver>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    auto listener1 = sptr<IPiPControlObserver>::MakeSptr();
    ASSERT_NE(nullptr, listener1);
    pipControl->pipControlObservers_.push_back(listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, pipControl->RegisterPiPControlObserver(nullptr));
    ASSERT_EQ(WMError::WM_OK, pipControl->RegisterPiPControlObserver(listener));
    ASSERT_EQ(WMError::WM_OK, pipControl->RegisterPiPControlObserver(listener1));
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, pipControl->UnregisterPiPControlObserver(nullptr));
    ASSERT_EQ(WMError::WM_OK, pipControl->UnregisterPiPControlObserver(listener));
}

/**
 * @tc.name: IsPullPiPAndHandleNavigation
 * @tc.desc: IsPullPiPAndHandleNavigation
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, IsPullPiPAndHandleNavigation, TestSize.Level1)
{
    sptr<MockWindow> mw = new MockWindow();
    sptr<PipOption> option = new PipOption();
    sptr<PictureInPictureController> pipControl = new PictureInPictureController(option, mw, 100, nullptr);
    pipControl->pipOption_->SetTypeNodeEnabled(true);
    ASSERT_EQ(true, pipControl->IsPullPiPAndHandleNavigation());
    pipControl->pipOption_->SetTypeNodeEnabled(false);
    pipControl->pipOption_->SetNavigationId("");
    ASSERT_EQ(true, pipControl->IsPullPiPAndHandleNavigation());
    pipControl->pipOption_->SetNavigationId("navId");
    pipControl->mainWindow_ = nullptr;
    ASSERT_EQ(false, pipControl->IsPullPiPAndHandleNavigation());
    pipControl->mainWindow_ = mw;
}

/**
 * @tc.name: IsPullPiPAndHandleNavigation_01
 * @tc.desc: IsPullPiPAndHandleNavigation should return false when GetNavigationController returns nullptr
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, IsPullPiPAndHandleNavigation_01, TestSize.Level1)
{
    pipControl_->pipOption_->SetTypeNodeEnabled(false);
    pipControl_->pipOption_->SetNavigationId("navId");
    EXPECT_CALL(*(pipControl_), GetNavigationController(_)).Times(1).WillOnce(Return(nullptr));
    EXPECT_EQ(false, pipControl_->IsPullPiPAndHandleNavigation());
}

/**
 * @tc.name: IsPullPiPAndHandleNavigation_02
 * @tc.desc: IsPullPiPAndHandleNavigation should return false when Top is not navDestination
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, IsPullPiPAndHandleNavigation_02, TestSize.Level1)
{
    pipControl_->pipOption_->SetTypeNodeEnabled(false);
    pipControl_->pipOption_->SetNavigationId("navId");

    EXPECT_CALL(*(pipControl_), GetNavigationController(_)).Times(1).WillOnce(Return(naviControl_.get()));
    EXPECT_CALL(*(naviControl_), IsNavDestinationInTopStack()).Times(1).WillOnce(Return(false));
    EXPECT_EQ(false, pipControl_->IsPullPiPAndHandleNavigation());
}

/**
 * @tc.name: IsPullPiPAndHandleNavigation_03
 * @tc.desc: IsPullPiPAndHandleNavigation should return true when pipOption's handle_ is valid
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, IsPullPiPAndHandleNavigation_03, TestSize.Level1)
{
    pipControl_->pipOption_->SetTypeNodeEnabled(false);
    pipControl_->pipOption_->SetNavigationId("navId");

    pipControl_->pipOption_->SetHandleId(2);
    EXPECT_CALL(*(pipControl_), GetNavigationController(_)).Times(1).WillOnce(Return(naviControl_.get()));
    EXPECT_CALL(*(naviControl_), GetTopHandle()).Times(1).WillOnce(Return(1));
    EXPECT_EQ(true, pipControl_->IsPullPiPAndHandleNavigation());
}

/**
 * @tc.name: IsPullPiPAndHandleNavigation_04
 * @tc.desc: IsPullPiPAndHandleNavigation should return false when GetTopHandle return -1
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, IsPullPiPAndHandleNavigation_04, TestSize.Level1)
{
    pipControl_->pipOption_->SetTypeNodeEnabled(false);
    pipControl_->pipOption_->SetNavigationId("navId");

    EXPECT_CALL(*(pipControl_), GetNavigationController(_)).Times(1).WillOnce(Return(naviControl_.get()));
    EXPECT_CALL(*(naviControl_), IsNavDestinationInTopStack()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*(naviControl_), GetTopHandle()).Times(1).WillOnce(Return(-1));
    EXPECT_EQ(false, pipControl_->IsPullPiPAndHandleNavigation());
}

/**
 * @tc.name: IsPullPiPAndHandleNavigation_05
 * @tc.desc: IsPullPiPAndHandleNavigation should return true, and firstHandleId_ should update by handleId_,
             when firstHandleId_ == -1
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, IsPullPiPAndHandleNavigation_05, TestSize.Level1)
{
    pipControl_->pipOption_->SetTypeNodeEnabled(false);
    pipControl_->pipOption_->SetNavigationId("navId");

    EXPECT_CALL(*(pipControl_), GetNavigationController(_)).Times(1).WillOnce(Return(naviControl_.get()));
    EXPECT_CALL(*(naviControl_), IsNavDestinationInTopStack()).Times(1).WillOnce(Return(true));

    int32_t topHandle = 1;
    EXPECT_CALL(*(naviControl_), GetTopHandle()).Times(1).WillOnce(Return(topHandle));
    EXPECT_EQ(true, pipControl_->IsPullPiPAndHandleNavigation());
    EXPECT_EQ(topHandle, pipControl_->firstHandleId_);
    EXPECT_EQ(pipControl_->firstHandleId_, topHandle);
}

/**
 * @tc.name: IsPullPiPAndHandleNavigation_06
 * @tc.desc: IsPullPiPAndHandleNavigation should return true, and handleId_ should update by
             pipControl_->firstHandleId_, when firstHandleId_ != -1
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, IsPullPiPAndHandleNavigation_06, TestSize.Level1)
{
    pipControl_->pipOption_->SetTypeNodeEnabled(false);
    pipControl_->pipOption_->SetNavigationId("navId");

    EXPECT_CALL(*(pipControl_), GetNavigationController(_)).Times(1).WillOnce(Return(naviControl_.get()));
    EXPECT_CALL(*(naviControl_), IsNavDestinationInTopStack()).Times(1).WillOnce(Return(true));

    int32_t topHandle = 1;
    EXPECT_CALL(*(naviControl_), GetTopHandle()).Times(1).WillOnce(Return(topHandle));
    pipControl_->firstHandleId_ = 2;
    EXPECT_EQ(true, pipControl_->IsPullPiPAndHandleNavigation());
    EXPECT_EQ(pipControl_->firstHandleId_, pipControl_->handleId_);
    EXPECT_EQ(pipControl_->handleId_, 2);
}

/**
 * @tc.name: ResetExtController
 * @tc.desc: ResetExtController
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, ResetExtController, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    std::shared_ptr<MockXComponentController> xComponentController = std::make_shared<MockXComponentController>();
    ASSERT_NE(nullptr, xComponentController);
    std::shared_ptr<MockXComponentController> xComponentController1 = std::make_shared<MockXComponentController>();
    ASSERT_NE(nullptr, xComponentController1);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    pipControl->pipOption_->SetTypeNodeEnabled(true);
    pipControl->ResetExtController();
    pipControl->pipOption_->SetTypeNodeEnabled(false);
    pipControl->ResetExtController();

    pipControl->pipXComponentController_ = nullptr;
    pipControl->mainWindowXComponentController_ = nullptr;
    pipControl->ResetExtController();
    pipControl->mainWindowXComponentController_ = nullptr;
    pipControl->pipXComponentController_ = xComponentController;
    pipControl->ResetExtController();
    pipControl->pipXComponentController_ = nullptr;
    pipControl->mainWindowXComponentController_ = xComponentController1;
    pipControl->ResetExtController();
    pipControl->pipXComponentController_ = xComponentController;
    EXPECT_CALL(*(xComponentController1), ResetExtController(_))
        .Times(1)
        .WillOnce(Return(XComponentControllerErrorCode::XCOMPONENT_CONTROLLER_TYPE_ERROR));
    pipControl->ResetExtController();
    EXPECT_CALL(*(xComponentController1), ResetExtController(_))
        .Times(1)
        .WillOnce(Return(XComponentControllerErrorCode::XCOMPONENT_CONTROLLER_NO_ERROR));
    pipControl->ResetExtController();
}

/**
 * @tc.name: OnPictureInPictureStart
 * @tc.desc: OnPictureInPictureStart
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, OnPictureInPictureStart, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    pipControl->OnPictureInPictureStart();
}

/**
 * @tc.name: IsTypeNodeEnabled
 * @tc.desc: IsTypeNodeEnabled
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, IsTypeNodeEnabled, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    pipControl->pipOption_->SetTypeNodeEnabled(true);
    ASSERT_TRUE(pipControl->IsTypeNodeEnabled());
    pipControl->pipOption_->SetTypeNodeEnabled(false);
    ASSERT_TRUE(!pipControl->IsTypeNodeEnabled());
    pipControl->pipOption_ = nullptr;
    ASSERT_TRUE(!pipControl->IsTypeNodeEnabled());
}

/**
 * @tc.name: GetTypeNode
 * @tc.desc: GetTypeNode
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, GetTypeNode, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    pipControl->pipOption_->SetTypeNodeRef(nullptr);
    ASSERT_EQ(nullptr, pipControl->GetTypeNode());
    pipControl->pipOption_ = nullptr;
    ASSERT_EQ(nullptr, pipControl->GetTypeNode());
}

/**
 * @tc.name: SetXComponentController
 * @tc.desc: SetXComponentController
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, SetXComponentController, TestSize.Level1)
{
    sptr<IPiPLifeCycle> listener = nullptr;
    std::shared_ptr<MockXComponentController> xComponentController = std::make_shared<MockXComponentController>();
    ASSERT_NE(nullptr, xComponentController);
    std::shared_ptr<MockXComponentController> xComponentController1 = std::make_shared<MockXComponentController>();
    ASSERT_NE(nullptr, xComponentController1);
    sptr<MockWindow> mw = new MockWindow();
    sptr<PipOption> option = new PipOption();
    sptr<PictureInPictureController> pipControl = new PictureInPictureController(option, mw, 100, nullptr);
    pipControl->pipOption_->SetTypeNodeEnabled(true);
    ASSERT_EQ(WMError::WM_OK, pipControl->SetXComponentController(xComponentController));
    pipControl->pipOption_->SetTypeNodeEnabled(false);
    pipControl->window_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_PIP_STATE_ABNORMALLY, pipControl->SetXComponentController(xComponentController));
    pipControl->window_ = mw;

    pipControl->pipXComponentController_ = nullptr;
    pipControl->mainWindowXComponentController_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_PIP_STATE_ABNORMALLY, pipControl->SetXComponentController(xComponentController));
    pipControl->mainWindowXComponentController_ = nullptr;
    pipControl->pipXComponentController_ = xComponentController;
    ASSERT_EQ(WMError::WM_ERROR_PIP_STATE_ABNORMALLY, pipControl->SetXComponentController(xComponentController));
    pipControl->mainWindowXComponentController_ = xComponentController1;
    EXPECT_CALL(*(xComponentController1), SetExtController(_))
        .Times(1)
        .WillOnce(Return(XComponentControllerErrorCode::XCOMPONENT_CONTROLLER_TYPE_ERROR));
    ASSERT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR, pipControl->SetXComponentController(xComponentController));
    EXPECT_CALL(*(xComponentController1), SetExtController(_))
        .Times(1)
        .WillOnce(Return(XComponentControllerErrorCode::XCOMPONENT_CONTROLLER_NO_ERROR));
    ASSERT_EQ(WMError::WM_OK, pipControl->SetXComponentController(xComponentController));
}

/**
 * @tc.name: RegisterPiPTypeNodeChange
 * @tc.desc: RegisterPiPTypeNodeChange
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, RegisterPiPTypeNodeChange, Function | SmallTest | Level2)
{
    sptr<MockWindow> mw = new MockWindow();
    sptr<PipOption> option = new PipOption();
    sptr<PictureInPictureController> pipControl = new PictureInPictureController(option, mw, 100, nullptr);
    auto listener = sptr<IPiPTypeNodeObserver>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, pipControl->RegisterPiPTypeNodeChange(nullptr));
    ASSERT_EQ(WMError::WM_OK, pipControl->RegisterPiPTypeNodeChange(listener));
}

/**
 * @tc.name: UnRegisterPiPTypeNodeChange
 * @tc.desc: UnRegisterPiPTypeNodeChange
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, UnRegisterPiPTypeNodeChange, Function | SmallTest | Level2)
{
    sptr<MockWindow> mw = new MockWindow();
    sptr<PipOption> option = new PipOption();
    sptr<PictureInPictureController> pipControl = new PictureInPictureController(option, mw, 100, nullptr);
    auto listener = sptr<IPiPTypeNodeObserver>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, pipControl->UnRegisterPiPTypeNodeChange(nullptr));
    ASSERT_EQ(WMError::WM_OK, pipControl->UnRegisterPiPTypeNodeChange(listener));
}

/**
 * @tc.name: UpdateContentNodeRef
 * @tc.desc: UpdateContentNodeRef
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, UpdateContentNodeRef, TestSize.Level1)
{
    sptr<MockWindow> mw = new MockWindow();
    sptr<PipOption> option = new PipOption();
    sptr<PictureInPictureController> pipControl = new PictureInPictureController(option, mw, 100, nullptr);
    pipControl->pipOption_ = nullptr;
    napi_ref nodeRef = nullptr;
    pipControl->UpdateContentNodeRef(nodeRef);
    pipControl->pipOption_ = option;
    pipControl->pipOption_->SetTypeNodeEnabled(true);
    pipControl->UpdateContentNodeRef(nodeRef);
    pipControl->pipOption_->SetTypeNodeEnabled(false);
    pipControl->UpdateContentNodeRef(nodeRef);
    pipControl->isAutoStartEnabled_ = true;
    pipControl->UpdateContentNodeRef(nodeRef);
    pipControl->isAutoStartEnabled_ = false;
    ASSERT_EQ(true, pipControl->IsTypeNodeEnabled());
}

/**
 * @tc.name: UpdatePiPSourceRect
 * @tc.desc: UpdatePiPSourceRect
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, UpdatePiPSourceRect, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    std::shared_ptr<MockXComponentController> xComponentController = std::make_shared<MockXComponentController>();
    ASSERT_NE(nullptr, xComponentController);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    pipControl->pipOption_->SetTypeNodeEnabled(true);
    pipControl->window_ = mw;
    pipControl->UpdatePiPSourceRect();
    ASSERT_EQ(0, pipControl->windowRect_.posX_);
    pipControl->pipOption_->SetTypeNodeEnabled(false);
    pipControl->UpdatePiPSourceRect();

    pipControl->mainWindowXComponentController_ = nullptr;
    pipControl->window_ = mw;
    pipControl->UpdatePiPSourceRect();
    pipControl->mainWindowXComponentController_ = xComponentController;
    pipControl->window_ = nullptr;
    pipControl->UpdatePiPSourceRect();
    pipControl->mainWindowXComponentController_ = nullptr;
    pipControl->UpdatePiPSourceRect();
    pipControl->mainWindowXComponentController_ = xComponentController;
    pipControl->window_ = mw;
    pipControl->UpdatePiPSourceRect();
    ASSERT_EQ(0, pipControl->windowRect_.posX_);
    ASSERT_EQ(0, pipControl->windowRect_.posY_);
    ASSERT_EQ(0, pipControl->windowRect_.width_);
    ASSERT_EQ(0, pipControl->windowRect_.height_);
}

/**
 * @tc.name: DestroyPictureInPictureWindow
 * @tc.desc: DestroyPictureInPictureWindow
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, DestroyPictureInPictureWindow, TestSize.Level1)
{
    sptr<MockWindow> mw = new (std::nothrow) MockWindow();
    ASSERT_NE(nullptr, mw);
    sptr<PipOption> option = new (std::nothrow) PipOption();
    ASSERT_NE(nullptr, option);
    sptr<PictureInPictureController> pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    pipControl->window_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR, pipControl->DestroyPictureInPictureWindow());

    sptr<MockWindow> window = sptr<MockWindow>::MakeSptr();
    pipControl->window_ = window;
    EXPECT_CALL(*(window), Destroy(0)).Times(1).WillOnce(Return(WMError::WM_DO_NOTHING));
    ASSERT_EQ(WMError::WM_ERROR_PIP_DESTROY_FAILED, pipControl->DestroyPictureInPictureWindow());

    EXPECT_CALL(*(window), Destroy(0)).Times(1).WillOnce(Return(WMError::WM_OK));
    pipControl->pipOption_ = nullptr;
    pipControl->mainWindow_ = nullptr;
    pipControl->window_ = window;
    ASSERT_EQ(WMError::WM_OK, pipControl->DestroyPictureInPictureWindow());
    pipControl->mainWindow_ = mw;
    pipControl->window_ = window;
    EXPECT_CALL(*(window), Destroy(0)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, pipControl->DestroyPictureInPictureWindow());
    pipControl->pipOption_ = option;
    pipControl->pipOption_->SetNavigationId("navId");
    pipControl->pipOption_->SetTypeNodeEnabled(false);
    pipControl->mainWindow_ = nullptr;
    pipControl->window_ = window;
    EXPECT_CALL(*(window), Destroy(0)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, pipControl->DestroyPictureInPictureWindow());
    pipControl->pipOption_->SetNavigationId("");
    pipControl->mainWindow_ = mw;
    pipControl->window_ = window;
    EXPECT_CALL(*(window), Destroy(0)).Times(1).WillOnce(Return(WMError::WM_OK));
    ASSERT_EQ(WMError::WM_OK, pipControl->DestroyPictureInPictureWindow());
}

/**
 * @tc.name: PrepareSource
 * @tc.desc: PrepareSource
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, PrepareSource, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    ASSERT_NE(nullptr, pipControl);
    pipControl->pipOption_->SetTypeNodeEnabled(true);
    pipControl->PrepareSource();
    pipControl->pipOption_->SetTypeNodeEnabled(false);
    pipControl->mainWindow_ = nullptr;
    pipControl->PrepareSource();
    pipControl->mainWindow_ = mw;
    pipControl->pipOption_->SetNavigationId("");
    pipControl->PrepareSource();
}

/**
 * @tc.name: LocateSource
 * @tc.desc: LocateSource
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, LocateSource, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    ASSERT_NE(nullptr, pipControl);
    pipControl->window_ = nullptr;
    pipControl->LocateSource();

    pipControl->window_ = mw;
    pipControl->LocateSource();
}

/**
 * @tc.name: StopPictureInPictureInner
 * @tc.desc: StopPictureInPictureInner
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, StopPictureInPictureInner, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    pipControl->pipOption_ = nullptr;
    pipControl->StopPictureInPictureInner(StopPipType::NULL_STOP, true);
    pipControl->pipOption_ = option;
    pipControl->window_ = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR,
              pipControl->StopPictureInPictureInner(StopPipType::NULL_STOP, true));
    pipControl->mainWindow_ = mw;
    ASSERT_EQ(WMError::WM_ERROR_PIP_INTERNAL_ERROR,
              pipControl->StopPictureInPictureInner(StopPipType::NULL_STOP, true));
    auto window = sptr<MockWindow>::MakeSptr();
    pipControl->window_ = window;
    ASSERT_EQ(WMError::WM_OK, pipControl->StopPictureInPictureInner(StopPipType::NULL_STOP, true));
    ASSERT_EQ(WMError::WM_OK, pipControl->StopPictureInPictureInner(StopPipType::NULL_STOP, false));
}

/**
 * @tc.name: GetPipPossible
 * @tc.desc: GetPipPossible
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, GetPipPossible, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);

    const std::string multiWindowUIType = system::GetParameter("const.window.multiWindowUIType", "");
    bool isDeviceSupported = multiWindowUIType == "HandsetSmartWindow" || multiWindowUIType == "FreeFormMultiWindow" ||
        multiWindowUIType == "TabletSmartWindow";

    bool pipSupported = false;
    pipControl->pipOption_ = option;
    pipControl->GetPipPossible(pipSupported);
    ASSERT_EQ(isDeviceSupported, pipSupported);
}

/**
 * @tc.name: GetPipEnabled
 * @tc.desc: GetPipEnabled
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, GetPipEnabled, TestSize.Level1)
{
    const std::string multiWindowUIType = system::GetParameter("const.window.multiWindowUIType", "");
    bool isDeviceSupported = multiWindowUIType == "HandsetSmartWindow" || multiWindowUIType == "FreeFormMultiWindow" ||
        multiWindowUIType == "TabletSmartWindow";
    bool pipSupported = PictureInPictureControllerBase::GetPipEnabled();
    ASSERT_EQ(isDeviceSupported, pipSupported);
}

/**
 * @tc.name: GetPipSettingSwitchStatusEnabled
 * @tc.desc: GetPipSettingSwitchStatusEnabled
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, GetPipSettingSwitchStatusEnabled, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto option = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, option);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    const std::string multiWindowUIType = system::GetParameter("const.window.multiWindowUIType", "");
    bool isDeviceSupported = multiWindowUIType == "HandsetSmartWindow" || multiWindowUIType == "TabletSmartWindow";
    ASSERT_EQ(isDeviceSupported, pipControl->GetPipSettingSwitchStatusEnabled());
}

/**
 * @tc.name: GetPiPSettingSwitchStatus
 * @tc.desc: GetPiPSettingSwitchStatus
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, GetPiPSettingSwitchStatus, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto pipOption = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, pipOption);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(pipOption, mw, 1, nullptr);
    EXPECT_EQ(false, pipControl->GetPiPSettingSwitchStatus());

    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetPiPSettingSwitchStatus");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    WindowSessionImpl::windowSessionMap_.clear();
    WindowSessionImpl::windowSessionMap_.insert(std::make_pair(window->GetWindowName(),
        std::pair<uint64_t, sptr<WindowSessionImpl>>(window->GetWindowId(), window)));
    EXPECT_EQ(false, pipControl->GetPiPSettingSwitchStatus());
    WindowSessionImpl::windowSessionMap_.clear();
}

/**
 * @tc.name: IsPiPActive
 * @tc.desc: IsPiPActive
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, IsPiPActive, TestSize.Level1)
{
    auto mw = sptr<MockWindow>::MakeSptr();
    ASSERT_NE(nullptr, mw);
    auto pipOption = sptr<PipOption>::MakeSptr();
    ASSERT_NE(nullptr, pipOption);
    auto pipControl = sptr<PictureInPictureController>::MakeSptr(pipOption, mw, 1, nullptr);
    pipControl->curState_ = PiPWindowState::STATE_STARTING;
    bool status = false;
    EXPECT_EQ(pipControl->IsPiPActive(status), WMError::WM_OK);
    pipControl->curState_ = PiPWindowState::STATE_STARTED;
    EXPECT_EQ(pipControl->IsPiPActive(status), WMError::WM_ERROR_PIP_INTERNAL_ERROR);

    auto window = sptr<MockWindow>::MakeSptr();
    pipControl->window_ = window;
    EXPECT_EQ(pipControl->IsPiPActive(status), WMError::WM_OK);
}

/**
 * @tc.name: DeletePIPMode
 * @tc.desc: DeletePIPMode
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, DeletePIPMode_WhenMainWindowIsNull, TestSize.Level1)
{
    naviControl_->SetInPIPMode(10);
    EXPECT_EQ(10, naviControl_->handle_);

    pipControl_->mainWindow_ = nullptr;
    pipControl_->DeletePIPMode();
    EXPECT_EQ(10, naviControl_->handle_);
}

HWTEST_F(PictureInPictureControllerTest, DeletePIPMode_WhenNavigationIdIsEmpty, TestSize.Level1)
{
    naviControl_->SetInPIPMode(100);
    EXPECT_EQ(100, naviControl_->handle_);

    pipControl_->pipOption_->SetNavigationId("");
    pipControl_->DeletePIPMode();
    EXPECT_EQ(100, naviControl_->handle_);
}

HWTEST_F(PictureInPictureControllerTest, DeletePIPMode_WhenGetNavigationControllerReturnNull, TestSize.Level1)
{
    naviControl_->SetInPIPMode(20);
    EXPECT_EQ(20, naviControl_->handle_);

    EXPECT_CALL(*(pipControl_), GetNavigationController(_)).Times(1).WillOnce(Return(nullptr));
    pipControl_->pipOption_->SetNavigationId("navId");
    pipControl_->DeletePIPMode();
    EXPECT_EQ(20, naviControl_->handle_);
}

HWTEST_F(PictureInPictureControllerTest, DeletePIPMode_WhenGetNavigationControllerReturnNotNull, TestSize.Level1)
{
    naviControl_->SetInPIPMode(30);
    EXPECT_EQ(30, naviControl_->handle_);

    EXPECT_CALL(*(pipControl_), GetNavigationController(_)).Times(1).WillOnce(Return(naviControl_.get()));
    pipControl_->pipOption_->SetNavigationId("navId");
    pipControl_->DeletePIPMode();
    EXPECT_EQ(0, naviControl_->handle_);
}

HWTEST_F(PictureInPictureControllerTest, GetNavigationController, TestSize.Level1)
{
    sptr<MockWindow> mw = sptr<MockWindow>::MakeSptr();
    sptr<PipOption> option = sptr<PipOption>::MakeSptr();
    sptr<MockPictureInPictureController> pipControl =
        sptr<MockPictureInPictureController>::MakeSptr(option, mw, 100, nullptr);
    EXPECT_EQ(nullptr, pipControl->GetNavigationController(""));
}

/**
 * @tc.name: GetStateChangeReason
 * @tc.desc: GetStateChangeReason/SetStateChangeReason
 * @tc.type: FUNC
 */
HWTEST_F(PictureInPictureControllerTest, GetStateChangeReason, TestSize.Level1)
{
    sptr<MockWindow> mw = new MockWindow();
    sptr<PipOption> option = new PipOption();
    sptr<Window> window_;
    sptr<Window> window;
    sptr<PictureInPictureController> pipControl = new PictureInPictureController(option, mw, 100, nullptr);
 
    pipControl->SetStateChangeReason(PiPStateChangeReason::REQUEST_DELETE);
    ASSERT_EQ(PiPStateChangeReason::REQUEST_DELETE, pipControl->GetStateChangeReason());
}
} // namespace
} // namespace Rosen
} // namespace OHOS