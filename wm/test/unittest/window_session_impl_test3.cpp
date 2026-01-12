/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ability_context_impl.h"
#include "display_info.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window.h"
#include "parameters.h"
#include "scene_board_judgement.h"
#include "window_manager_hilog.h"
#include "window_session_impl.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        logMsg = msg;
    }
}

namespace OHOS {
namespace Rosen {
class WindowSessionImplTest3 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<WindowSessionImpl> window_;
private:
    static constexpr int32_t PERSISTENT_ID_ONE = 1;
    static constexpr int32_t PERSISTENT_ID_TWO = 2;
};

void WindowSessionImplTest3::SetUpTestCase() {}

void WindowSessionImplTest3::TearDownTestCase() {}

void WindowSessionImplTest3::SetUp() {}

void WindowSessionImplTest3::TearDown()
{
    if (window_ != nullptr) {
        window_->Destroy();
    }
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
 * @tc.name: SetInputEventConsumer
 * @tc.desc: SetInputEventConsumer01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetInputEventConsumer01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest3: SetInputEventConsumer01 start";
    window_ = GetTestWindowImpl("SetInputEventConsumer01");
    ASSERT_NE(window_, nullptr);
    window_->inputEventConsumer_ = nullptr;
    std::shared_ptr<IInputEventConsumer> inputEventConsumer = std::make_shared<MockInputEventConsumer>();
    window_->SetInputEventConsumer(inputEventConsumer);
    ASSERT_NE(window_->inputEventConsumer_, nullptr);
    GTEST_LOG_(INFO) << "WindowSessionImplTest3: SetInputEventConsumer01 end";
}

/**
 * @tc.name: SetContinueState
 * @tc.desc: SetContinueState test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetContinueState, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest3: SetContinueState start";
    window_ = GetTestWindowImpl("SetContinueState");
    ASSERT_NE(window_, nullptr);
    WMError ret = window_->SetContinueState(static_cast<int32_t>(ContinueState::CONTINUESTATE_INACTIVE));
    ASSERT_EQ(ret, WMError::WM_OK);
    ret = window_->SetContinueState(-100);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
        ret = window_->SetContinueState(3);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
    GTEST_LOG_(INFO) << "WindowSessionImplTest3: SetContinueState end";
}

/**
 * @tc.name: GetListeners
 * @tc.desc: GetListeners01 IDisplayMoveListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, GetListeners01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest3: GetListeners01 start";
    window_ = GetTestWindowImpl("GetListeners01");
    ASSERT_NE(window_, nullptr);
    window_->displayMoveListeners_.clear();
    window_->NotifyDisplayMove(0, 100);
    ASSERT_TRUE(window_->displayMoveListeners_[window_->GetPersistentId()].empty());

    sptr<IDisplayMoveListener> displayMoveListener = new (std::nothrow) MockIDisplayMoveListener();
    ASSERT_EQ(window_->RegisterDisplayMoveListener(displayMoveListener), WMError::WM_OK);
    window_->NotifyDisplayMove(0, 100);
    ASSERT_FALSE(window_->displayMoveListeners_[window_->GetPersistentId()].empty());
    GTEST_LOG_(INFO) << "WindowSessionImplTest3: GetListeners01 end";
}

/**
 * @tc.name: RegisterWindowNoInteractionListener
 * @tc.desc: RegisterWindowNoInteractionListener01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, RegisterWindowNoInteractionListener01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest3: RegisterWindowNoInteractionListener01 start";
    window_ = GetTestWindowImpl("RegisterWindowNoInteractionListener01");
    ASSERT_NE(window_, nullptr);
    ASSERT_EQ(window_->RegisterWindowNoInteractionListener(nullptr), WMError::WM_ERROR_NULLPTR);
    ASSERT_EQ(window_->UnregisterWindowNoInteractionListener(nullptr), WMError::WM_ERROR_NULLPTR);

    sptr<IWindowNoInteractionListener> windowNoInteractionListenerSptr =
        new (std::nothrow) MockIWindowNoInteractionListener();
    ASSERT_EQ(window_->RegisterWindowNoInteractionListener(windowNoInteractionListenerSptr), WMError::WM_OK);
    ASSERT_EQ(window_->UnregisterWindowNoInteractionListener(windowNoInteractionListenerSptr), WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest3: RegisterWindowNoInteractionListener01 end";
}

/**
 * @tc.name: SetForceSplitConfig
 * @tc.desc: SetForceSplitConfig
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetForceSplitConfig, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest3: SetForceSplitConfig start";
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    window_ = GetTestWindowImpl("SetForceSplitConfig");
    ASSERT_NE(window_, nullptr);

    int32_t FORCE_SPLIT_MODE = 5;
    int32_t NAV_FORCE_SPLIT_MODE = 6;
    AppForceLandscapeConfig config = { FORCE_SPLIT_MODE, true, false, {}, {}, {}, false, false, false, false };
    window_->SetForceSplitConfig(config);

    config = { FORCE_SPLIT_MODE, false, false, {}, {}, {}, false, false, false, false };
    window_->SetForceSplitConfig(config);

    config = { NAV_FORCE_SPLIT_MODE, true, false, {}, {}, {}, false, false, false, false };
    window_->SetForceSplitConfig(config);

    config = { NAV_FORCE_SPLIT_MODE, false, false, {}, {}, {}, false, false, false, false };
    window_->SetForceSplitConfig(config);
    EXPECT_TRUE(logMsg.find("uiContent is null!") != std::string::npos);
    LOG_SetCallback(nullptr);
    GTEST_LOG_(INFO) << "WindowSessionImplTest3: SetForceSplitConfig end";
}

/**
 * @tc.name: GetAppForceLandscapeConfig
 * @tc.desc: GetAppForceLandscapeConfig
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, GetAppForceLandscapeConfig, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetAppForceLandscapeConfig start";
    window_ = GetTestWindowImpl("GetAppForceLandscapeConfig");
    ASSERT_NE(window_, nullptr);

    AppForceLandscapeConfig config = {};
    window_->hostSession_ = nullptr;
    auto res = window_->GetAppForceLandscapeConfig(config);
    ASSERT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetAppForceLandscapeConfig end";
}

/**
 * @tc.name: IsSceneBoardEnabled
 * @tc.desc: IsSceneBoardEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, IsSceneBoardEnabled, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsSceneBoardEnabled start";
    window_ = GetTestWindowImpl("IsSceneBoardEnabled");
    ASSERT_NE(window_, nullptr);

    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window_->hostSession_ = session;
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    bool result = SceneBoardJudgement::IsSceneBoardEnabled();
    ASSERT_EQ(result, window_->IsSceneBoardEnabled());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsSceneBoardEnabled end";
}

/**
 * @tc.name: IsFocused
 * @tc.desc: IsFocused
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, IsFocused, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsFocused start";
    window_ = GetTestWindowImpl("IsFocused");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(INVALID_SESSION_ID);
    auto ret = window_->IsFocused();
    ASSERT_EQ(ret, false);

    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    auto info = sptr<FocusNotifyInfo>::MakeSptr();
    info->isSyncNotify_ = false;
    window_->UpdateFocus(info, true);
    ret = window_->IsFocused();
    ASSERT_EQ(ret, true);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsFocused end";
}

/**
 * @tc.name: IsNotifyInteractiveDuplicative
 * @tc.desc: IsNotifyInteractiveDuplicative
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, IsNotifyInteractiveDuplicative, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsNotifyInteractiveDuplicative start";
    window_ = GetTestWindowImpl("IsNotifyInteractiveDuplicative");
    ASSERT_NE(window_, nullptr);
    window_->hasFirstNotifyInteractive_ = true;
    window_->interactive_ = true;
    window_->NotifyForegroundInteractiveStatus(true);
    auto ret = window_->IsNotifyInteractiveDuplicative(true);
    ASSERT_EQ(ret, true);
    GTEST_LOG_(INFO) << "IsNotifyInteractiveDuplicative: IsNotifyInteractiveDuplicative end";
}

/**
 * @tc.name: SetMainWindowTopmost
 * @tc.desc: SetMainWindowTopmost
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetMainWindowTopmost, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetMainWindowTopmost start";
    window_ = GetTestWindowImpl("SetMainWindowTopmost");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(INVALID_SESSION_ID);
    auto ret = window_->SetMainWindowTopmost(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetMainWindowTopmost end";
}

/**
 * @tc.name: SetDecorVisible
 * @tc.desc: SetDecorVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetDecorVisible, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetDecorVisible start";
    window_ = GetTestWindowImpl("SetDecorVisible");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(INVALID_SESSION_ID);
    auto ret = window_->SetDecorVisible(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetDecorVisible end";
}

/**
 * @tc.name: SetWindowModal
 * @tc.desc: SetWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetWindowModal, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetWindowModal start";
    window_ = GetTestWindowImpl("SetWindowModal");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(INVALID_SESSION_ID);
    auto ret = window_->SetWindowModal(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    ret = window_->SetWindowModal(false);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    window_->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = window_->SetWindowModal(true);
    ASSERT_EQ(WMError::WM_OK, ret);
    ret = window_->SetWindowModal(false);
    ASSERT_EQ(WMError::WM_OK, ret);

    window_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = window_->SetWindowModal(true);
    ASSERT_EQ(WMError::WM_OK, ret);
    ret = window_->SetWindowModal(false);
    ASSERT_EQ(WMError::WM_OK, ret);
    
    window_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = window_->SetWindowModal(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
    ret = window_->SetWindowModal(false);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
    
    window_->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = window_->SetWindowModal(false);
    ASSERT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    ret = window_->SetWindowModal(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetWindowModal end";
}

/**
 * @tc.name: SetDecorButtonStyle
 * @tc.desc: SetDecorButtonStyle
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetDecorButtonStyle, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetDecorButtonStyle start";
    window_ = GetTestWindowImpl("SetDecorButtonStyle");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(INVALID_SESSION_ID);
    DecorButtonStyle decorButtonStyle;
    auto ret = window_->SetDecorButtonStyle(decorButtonStyle);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetDecorButtonStyle end";
}

/**
 * @tc.name: GetDecorButtonStyle
 * @tc.desc: GetDecorButtonStyle
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, GetDecorButtonStyle, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetDecorButtonStyle start";
    window_ = GetTestWindowImpl("GetDecorButtonStyle");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(INVALID_SESSION_ID);
    DecorButtonStyle decorButtonStyle;
    auto ret = window_->GetDecorButtonStyle(decorButtonStyle);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetDecorButtonStyle end";
}

/**
 * @tc.name: RegisterMainWindowCloseListeners
 * @tc.desc: RegisterMainWindowCloseListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, RegisterMainWindowCloseListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterMainWindowCloseListeners start";
    window_ = GetTestWindowImpl("RegisterMainWindowCloseListeners");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(INVALID_SESSION_ID);
    sptr<IMainWindowCloseListener> listener = sptr<IMainWindowCloseListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);
    auto ret = window_->RegisterMainWindowCloseListeners(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    ret = window_->RegisterMainWindowCloseListeners(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);

    window_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ret = window_->RegisterMainWindowCloseListeners(listener);
    ASSERT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterMainWindowCloseListeners end";
}

/**
 * @tc.name: UnregisterMainWindowCloseListeners
 * @tc.desc: UnregisterMainWindowCloseListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, UnregisterMainWindowCloseListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UnregisterMainWindowCloseListeners start";
    window_ = GetTestWindowImpl("UnregisterMainWindowCloseListeners");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(INVALID_SESSION_ID);
    sptr<IMainWindowCloseListener> listener = sptr<IMainWindowCloseListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);
    auto ret = window_->UnregisterMainWindowCloseListeners(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    ret = window_->UnregisterMainWindowCloseListeners(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);

    window_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ret = window_->UnregisterMainWindowCloseListeners(listener);
    ASSERT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UnregisterMainWindowCloseListeners end";
}

/**
 * @tc.name: RegisterWindowWillCloseListeners
 * @tc.desc: RegisterWindowWillCloseListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, RegisterWindowWillCloseListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterWindowWillCloseListeners start";
    window_ = GetTestWindowImpl("RegisterWindowWillCloseListeners");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(INVALID_SESSION_ID);
    sptr<IWindowWillCloseListener> listener = sptr<IWindowWillCloseListener>::MakeSptr();
    auto ret = window_->RegisterWindowWillCloseListeners(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    window_->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ret = window_->RegisterWindowWillCloseListeners(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);

    window_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ret = window_->RegisterWindowWillCloseListeners(listener);
    ASSERT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterWindowWillCloseListeners end";
}

/**
 * @tc.name: UnRegisterWindowWillCloseListeners
 * @tc.desc: UnRegisterWindowWillCloseListeners
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, UnRegisterWindowWillCloseListeners, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UnRegisterWindowWillCloseListeners start";
    window_ = GetTestWindowImpl("UnRegisterWindowWillCloseListeners");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(INVALID_SESSION_ID);
    sptr<IWindowWillCloseListener> listener = sptr<IWindowWillCloseListener>::MakeSptr();
    auto ret = window_->UnRegisterWindowWillCloseListeners(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    window_->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ret = window_->UnRegisterWindowWillCloseListeners(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);

    window_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ret = window_->UnRegisterWindowWillCloseListeners(listener);
    ASSERT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UnRegisterWindowWillCloseListeners end";
}

/**
 * @tc.name: RegisterSwitchFreeMultiWindowListener
 * @tc.desc: RegisterSwitchFreeMultiWindowListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, RegisterSwitchFreeMultiWindowListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterSwitchFreeMultiWindowListener start";
    window_ = GetTestWindowImpl("RegisterSwitchFreeMultiWindowListener");
    ASSERT_NE(window_, nullptr);
    sptr<ISwitchFreeMultiWindowListener> listener = sptr<ISwitchFreeMultiWindowListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);
    window_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    auto ret = window_->RegisterSwitchFreeMultiWindowListener(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterSwitchFreeMultiWindowListener end";
}

/**
 * @tc.name: UnregisterSwitchFreeMultiWindowListener
 * @tc.desc: UnregisterSwitchFreeMultiWindowListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, UnregisterSwitchFreeMultiWindowListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UnregisterSwitchFreeMultiWindowListener start";
    window_ = GetTestWindowImpl("UnregisterSwitchFreeMultiWindowListener");
    ASSERT_NE(window_, nullptr);
    sptr<ISwitchFreeMultiWindowListener> listener = sptr<ISwitchFreeMultiWindowListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);
    window_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    auto ret = window_->UnregisterSwitchFreeMultiWindowListener(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
    window_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ret = window_->UnregisterSwitchFreeMultiWindowListener(listener);
    ASSERT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UnregisterSwitchFreeMultiWindowListener end";
}

/**
 * @tc.name: SetSplitButtonVisible
 * @tc.desc: SetSplitButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetSplitButtonVisible, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetSplitButtonVisible start";
    window_ = GetTestWindowImpl("SetSplitButtonVisible");
    ASSERT_NE(window_, nullptr);
    auto ret = window_->SetSplitButtonVisible(true);
    ASSERT_EQ(ret, WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetSplitButtonVisible end";
}

/**
 * @tc.name: NotifyNoInteractionTimeout
 * @tc.desc: NotifyNoInteractionTimeout
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, NotifyNoInteractionTimeout, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyNoInteractionTimeout start";
    window_ = GetTestWindowImpl("NotifyNoInteractionTimeout");
    ASSERT_NE(window_, nullptr);
    IWindowNoInteractionListenerSptr listener = nullptr;
    auto ret = window_->NotifyNoInteractionTimeout(listener);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: NotifyNoInteractionTimeout end";
}

/**
 * @tc.name: IsVerticalOrientation
 * @tc.desc: IsVerticalOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, IsVerticalOrientation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsVerticalOrientation start";
    window_ = GetTestWindowImpl("IsVerticalOrientation");
    ASSERT_NE(window_, nullptr);
    Orientation orientation = Orientation::VERTICAL;
    auto ret = window_->IsVerticalOrientation(orientation);
    ASSERT_EQ(ret, true);
    orientation = Orientation::REVERSE_VERTICAL;
    ret = window_->IsVerticalOrientation(orientation);
    ASSERT_EQ(ret, true);
    orientation = Orientation::SENSOR_VERTICAL;
    ret = window_->IsVerticalOrientation(orientation);
    ASSERT_EQ(ret, true);
    orientation = Orientation::AUTO_ROTATION_PORTRAIT_RESTRICTED;
    ret = window_->IsVerticalOrientation(orientation);
    ASSERT_EQ(ret, true);
    orientation = Orientation::USER_ROTATION_PORTRAIT;
    ret = window_->IsVerticalOrientation(orientation);
    ASSERT_EQ(ret, true);
    orientation = Orientation::USER_ROTATION_PORTRAIT_INVERTED;
    ret = window_->IsVerticalOrientation(orientation);
    ASSERT_EQ(ret, true);
    orientation = Orientation::UNSPECIFIED;
    ret = window_->IsVerticalOrientation(orientation);
    ASSERT_EQ(ret, false);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsVerticalOrientation end";
}

/**
 * @tc.name: IsHorizontalOrientation
 * @tc.desc: IsHorizontalOrientation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, IsHorizontalOrientation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsHorizontalOrientation start";
    window_ = GetTestWindowImpl("IsHorizontalOrientation");
    ASSERT_NE(window_, nullptr);
    Orientation orientation = Orientation::HORIZONTAL;
    auto ret = window_->IsHorizontalOrientation(orientation);
    ASSERT_EQ(ret, true);
    orientation = Orientation::REVERSE_HORIZONTAL;
    ret = window_->IsHorizontalOrientation(orientation);
    ASSERT_EQ(ret, true);
    orientation = Orientation::SENSOR_HORIZONTAL;
    ret = window_->IsHorizontalOrientation(orientation);
    ASSERT_EQ(ret, true);
    orientation = Orientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED;
    ret = window_->IsHorizontalOrientation(orientation);
    ASSERT_EQ(ret, true);
    orientation = Orientation::USER_ROTATION_LANDSCAPE;
    ret = window_->IsHorizontalOrientation(orientation);
    ASSERT_EQ(ret, true);
    orientation = Orientation::USER_ROTATION_LANDSCAPE_INVERTED;
    ret = window_->IsHorizontalOrientation(orientation);
    ASSERT_EQ(ret, true);
    orientation = Orientation::UNSPECIFIED;
    ret = window_->IsHorizontalOrientation(orientation);
    ASSERT_EQ(ret, false);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsHorizontalOrientation end";
}

/**
 * @tc.name: MarkProcessed
 * @tc.desc: MarkProcessed
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, MarkProcessed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MarkProcessed start";
    window_ = GetTestWindowImpl("MarkProcessed");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    auto ret = window_->MarkProcessed(1);
    ASSERT_EQ(ret, WSError::WS_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: MarkProcessed end";
}

/**
 * @tc.name: UpdateRectForOtherReasonTask
 * @tc.desc: UpdateRectForOtherReasonTask
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, UpdateRectForOtherReasonTask, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateRectForOtherReasonTask start";
    window_ = GetTestWindowImpl("UpdateRectForOtherReasonTask");
    ASSERT_NE(window_, nullptr);
    Rect wmRect = { 0, 0, 0, 0 };
    Rect preRect = { 0, 0, 0, 0 };
    WindowSizeChangeReason wmReason = WindowSizeChangeReason::UNDEFINED;
    std::shared_ptr<RSTransaction> rsTransaction = nullptr;
    window_->lastSizeChangeReason_ = WindowSizeChangeReason::UNDEFINED;
    window_->postTaskDone_ = false;
    window_->UpdateRectForOtherReasonTask(wmRect, preRect, wmReason, rsTransaction);
    ASSERT_EQ(window_->postTaskDone_, true);
    window_->UpdateRectForOtherReasonTask(wmRect, preRect, wmReason, rsTransaction);
    window_->postTaskDone_ = false;
    wmRect.posX_ = 1;
    window_->UpdateRectForOtherReasonTask(wmRect, preRect, wmReason, rsTransaction);
    ASSERT_EQ(window_->postTaskDone_, true);
    WindowType windowType = window_->GetType();
    Rect requestRect = { 0, 0, 50, 50 };
    Rect windowRect = { 0, 0, 0, 0 };
    window_->postTaskDone_ = true;
    window_->property_->SetWindowRect(windowRect);
    window_->property_->SetRequestRect(requestRect);
    window_->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT_NAVIGATION);
    window_->SetNotifySizeChangeFlag(true);
    ASSERT_EQ(window_->notifySizeChangeFlag_, true);
    window_->UpdateRectForOtherReasonTask(wmRect, preRect, wmReason, rsTransaction);
    ASSERT_EQ(window_->notifySizeChangeFlag_, false);
    window_->postTaskDone_ = true;
    window_->property_->SetWindowRect(windowRect);
    window_->property_->SetRequestRect(requestRect);
    window_->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    window_->SetNotifySizeChangeFlag(true);
    ASSERT_EQ(window_->notifySizeChangeFlag_, true);
    window_->UpdateRectForOtherReasonTask(wmRect, preRect, wmReason, rsTransaction);
    ASSERT_EQ(window_->notifySizeChangeFlag_, false);
    window_->property_->SetWindowType(windowType);
    window_->handler_ = nullptr;
    window_->UpdateRectForOtherReason(wmRect, preRect, wmReason, rsTransaction);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateRectForOtherReasonTask end";
}

/**
 * @tc.name: CopyUniqueDensityParameter
 * @tc.desc: CopyUniqueDensityParameter
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, CopyUniqueDensityParameter, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: CopyUniqueDensityParameter start";
    window_ = GetTestWindowImpl("CopyUniqueDensityParameter");
    ASSERT_NE(window_, nullptr);
    sptr<WindowSessionImpl> parentWindow = GetTestWindowImpl("CopyUniqueDensityParameter01");
    ASSERT_NE(parentWindow, nullptr);
    window_->useUniqueDensity_ = false;
    window_->virtualPixelRatio_ = 1.0f;
    parentWindow->useUniqueDensity_ = true;
    parentWindow->virtualPixelRatio_ = 1.0f;
    window_->CopyUniqueDensityParameter(parentWindow);
    ASSERT_EQ(window_->useUniqueDensity_, true);
    parentWindow = nullptr;
    window_->CopyUniqueDensityParameter(parentWindow);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: CopyUniqueDensityParameter end";
}

/**
 * @tc.name: RaiseToAppTopOnDrag
 * @tc.desc: RaiseToAppTopOnDrag
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, RaiseToAppTopOnDrag, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RaiseToAppTopOnDrag start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RaiseToAppTop");
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSessionImpl>::MakeSptr(option);
    windowSessionImpl->property_->SetPersistentId(1);
    windowSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    auto ret = windowSessionImpl->RaiseToAppTopOnDrag();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);

    SessionInfo sessionInfo = { "CreateTestBundle0", "CreateTestModule0", "CreateTestAbility0" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    windowSessionImpl->hostSession_ = session;
    ret = windowSessionImpl->RaiseToAppTopOnDrag();
    EXPECT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    windowSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = windowSessionImpl->RaiseToAppTopOnDrag();
    EXPECT_EQ(WMError::WM_OK, ret);
    windowSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = windowSessionImpl->RaiseToAppTopOnDrag();
    EXPECT_EQ(WMError::WM_OK, ret);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RaiseToAppTopOnDrag end";
}

/**
 * @tc.name: SetRaiseByClickEnabled
 * @tc.desc: SetRaiseByClickEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetRaiseByClickEnabled, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetRaiseByClickEnabled start";
    window_ = GetTestWindowImpl("SetRaiseByClickEnabled");
    ASSERT_NE(window_, nullptr);
    window_->property_->parentPersistentId_ = 2;
    window_->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    window_->state_ = WindowState::STATE_SHOWN;
    window_->property_->SetPersistentId(1);
    auto ret = window_->SetRaiseByClickEnabled(true);
    ASSERT_EQ(ret, WMError::WM_OK);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetRaiseByClickEnabled end";
}

/**
 * @tc.name: SetSubWindowModal
 * @tc.desc: SetSubWindowModal
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetSubWindowModal, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetSubWindowModal start";
    window_ = GetTestWindowImpl("SetSubWindowModal");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    window_->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ModalityType modalityType = ModalityType::APPLICATION_MODALITY;
    window_->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window_->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window_->windowSystemConfig_.freeMultiWindowSupport_ = false;
    auto ret = window_->SetSubWindowModal(true, modalityType);
    ASSERT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window_->vsyncStation_ = nullptr;
    window_->ClearVsyncStation();
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetSubWindowModal end";
}

/**
 * @tc.name: UpdateFrameLayoutCallbackIfNeeded
 * @tc.desc: UpdateFrameLayoutCallbackIfNeeded
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, UpdateFrameLayoutCallbackIfNeeded, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateFrameLayoutCallbackIfNeeded start";
    window_ = GetTestWindowImpl("UpdateFrameLayoutCallbackIfNeeded");
    ASSERT_NE(window_, nullptr);
    window_->enableFrameLayoutFinishCb_ = false;
    WindowSizeChangeReason wmReason = WindowSizeChangeReason::FULL_TO_SPLIT;
    window_->UpdateFrameLayoutCallbackIfNeeded(wmReason);
    ASSERT_EQ(window_->enableFrameLayoutFinishCb_, true);

    window_->enableFrameLayoutFinishCb_ = false;
    wmReason = WindowSizeChangeReason::SPLIT_TO_FULL;
    window_->UpdateFrameLayoutCallbackIfNeeded(wmReason);
    ASSERT_EQ(window_->enableFrameLayoutFinishCb_, true);

    window_->enableFrameLayoutFinishCb_ = false;
    wmReason = WindowSizeChangeReason::FULL_TO_FLOATING;
    window_->UpdateFrameLayoutCallbackIfNeeded(wmReason);
    ASSERT_EQ(window_->enableFrameLayoutFinishCb_, true);

    window_->enableFrameLayoutFinishCb_ = false;
    wmReason = WindowSizeChangeReason::FLOATING_TO_FULL;
    window_->UpdateFrameLayoutCallbackIfNeeded(wmReason);
    ASSERT_EQ(window_->enableFrameLayoutFinishCb_, true);

    window_->enableFrameLayoutFinishCb_ = false;
    wmReason = WindowSizeChangeReason::DRAG_END;
    window_->windowSystemConfig_.freeMultiWindowEnable_ = true;
    window_->windowSystemConfig_.freeMultiWindowSupport_ = true;
    window_->UpdateFrameLayoutCallbackIfNeeded(wmReason);
    ASSERT_EQ(window_->enableFrameLayoutFinishCb_, true);

    window_->windowSystemConfig_.freeMultiWindowSupport_ = false;
    window_->UpdateFrameLayoutCallbackIfNeeded(wmReason);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UpdateFrameLayoutCallbackIfNeeded end";
}

/**
 * @tc.name: SetTargetAPIVersion
 * @tc.desc: SetTargetAPIVersion
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetTargetAPIVersion, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetTargetAPIVersion start";
    window_ = GetTestWindowImpl("SetAPPWindowIcon");
    ASSERT_NE(window_, nullptr);
    uint32_t version = 14;
    window_->SetTargetAPIVersion(version);
    EXPECT_EQ(version, window_->GetTargetAPIVersion());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetTargetAPIVersion end";
}

/**
 * @tc.name: SetAPPWindowIcon
 * @tc.desc: SetAPPWindowIcon
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, GetTargetAPIVersion, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetTargetAPIVersion start";
    window_ = GetTestWindowImpl("GetTargetAPIVersion");
    ASSERT_NE(window_, nullptr);
    EXPECT_EQ(0, window_->GetTargetAPIVersion());
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetTargetAPIVersion end";
}

/**
 * @tc.name: SetAPPWindowIcon
 * @tc.desc: SetAPPWindowIcon
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetAPPWindowIcon, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAPPWindowIcon start";
    window_ = GetTestWindowImpl("SetAPPWindowIcon");
    ASSERT_NE(window_, nullptr);
    window_->uiContent_ = nullptr;
    std::shared_ptr<Media::PixelMap> icon = std::make_shared<Media::PixelMap>();
    ASSERT_NE(icon, nullptr);
    auto ret = window_->SetAPPWindowIcon(icon);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetAPPWindowIcon end";
}

/**
 * @tc.name: SetBackgroundColor
 * @tc.desc: SetBackgroundColor
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetBackgroundColor, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBackgroundColor start";
    window_ = GetTestWindowImpl("SetBackgroundColor");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_CREATED;
    std::string color = "";
    auto ret = window_->SetBackgroundColor(color);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);

    color = "#FF0000";
    ret = window_->SetBackgroundColor(color);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_OPERATION);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: SetBackgroundColor end";
}

/**
 * @tc.name: Find
 * @tc.desc: Find
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, Find, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: Find start";
    window_ = GetTestWindowImpl("Find");
    ASSERT_NE(window_, nullptr);
    window_->windowSessionMap_.clear();
    std::string name = "Find";
    sptr<WindowSessionImpl> window1 = GetTestWindowImpl("Find1");
    ASSERT_NE(window1, nullptr);
    window_->windowSessionMap_.insert(std::make_pair(name, std::make_pair(1, window1)));
    auto ret = window_->Find(name);
    ASSERT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: Find end";
}

/**
 * @tc.name: RegisterWindowTitleButtonRectChangeListener
 * @tc.desc: RegisterWindowTitleButtonRectChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, RegisterWindowTitleButtonRectChangeListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterWindowTitleButtonRectChangeListener start";
    window_ = GetTestWindowImpl("RegisterWindowTitleButtonRectChangeListener");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_SHOWN;
    window_->interactive_ = true;
    window_->hasFirstNotifyInteractive_ = true;
    window_->NotifyForegroundInteractiveStatus(true);

    sptr<IWindowTitleButtonRectChangedListener> listener = nullptr;
    auto ret = window_->RegisterWindowTitleButtonRectChangeListener(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: RegisterWindowTitleButtonRectChangeListener end";
}

/**
 * @tc.name: GetUIContentWithId
 * @tc.desc: GetUIContentWithId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, GetUIContentWithId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetUIContentWithId start";
    window_ = GetTestWindowImpl("GetUIContentWithId");
    ASSERT_NE(window_, nullptr);
    window_->windowSessionMap_.clear();
    std::string name = "GetUIContentWithId";
    sptr<WindowSessionImpl> window1 = GetTestWindowImpl("GetUIContentWithId1");
    ASSERT_NE(window1, nullptr);
    window_->windowSessionMap_.insert(std::make_pair(name, std::make_pair(1, window1)));
    auto ret = window_->GetUIContentWithId(1);
    ASSERT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: GetUIContentWithId end";
}

/**
 * @tc.name: UnregisterWindowRectChangeListener
 * @tc.desc: UnregisterWindowRectChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, UnregisterWindowRectChangeListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UnregisterWindowRectChangeListener start";
    window_ = GetTestWindowImpl("UnregisterWindowRectChangeListener");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->state_ = WindowState::STATE_SHOWN;
    window_->windowRectChangeListeners_.clear();
    sptr<IWindowRectChangeListener> listener = nullptr;
    auto ret = window_->UnregisterWindowRectChangeListener(listener);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: UnregisterWindowRectChangeListener end";
}

/**
 * @tc.name: IsFloatingWindowAppType
 * @tc.desc: IsFloatingWindowAppType
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, IsFloatingWindowAppType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsFloatingWindowAppType start";
    window_ = GetTestWindowImpl("IsFloatingWindowAppType");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(1);
    window_->property_->isFloatingWindowAppType_ = true;
    window_->state_ = WindowState::STATE_CREATED;
    auto ret = window_->IsFloatingWindowAppType();
    ASSERT_EQ(ret, true);
    GTEST_LOG_(INFO) << "WindowSessionImplTest: IsFloatingWindowAppType end";
}

/**
 * @tc.name: IsAdaptToCompatibleImmersive
 * @tc.desc: IsAdaptToCompatibleImmersive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, IsAdaptToCompatibleImmersive, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WindowSessionImplTest3: IsAdaptToCompatibleImmersive start";
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsAdaptToCompatibleImmersive");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToImmersive(true);
    window->property_->SetCompatibleModeProperty(compatibleModeProperty);
    EXPECT_EQ(true, window->IsAdaptToCompatibleImmersive());
    GTEST_LOG_(INFO) << "WindowSessionImplTest3: IsAdaptToCompatibleImmersive end";
}

/**
 * @tc.name: SetAvoidAreaOption
 * @tc.desc: SetAvoidAreaOption
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetAvoidAreaOption, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAvoidAreaOption");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    WMError res = window->SetAvoidAreaOption(3);
    ASSERT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: GetAvoidAreaOption
 * @tc.desc: GetAvoidAreaOption
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, GetAvoidAreaOption, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetAvoidAreaOption");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    uint32_t avoidAreaOption = 0;
    WMError res = window->GetAvoidAreaOption(avoidAreaOption);
    ASSERT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: SetWatchGestureConsumed
 * @tc.desc: SetWatchGestureConsumed test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetWatchGestureConsumed, TestSize.Level1)
{
    window_ = GetTestWindowImpl("SetWatchGestureConsumed");
    ASSERT_NE(window_, nullptr);
    bool isWatchGestureConsumed = false;
    window_->SetWatchGestureConsumed(isWatchGestureConsumed);
    ASSERT_EQ(window_->GetWatchGestureConsumed(), false);
}

/**
 * @tc.name: NotifyConsumeResultToFloatWindow
 * @tc.desc: NotifyConsumeResultToFloatWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, NotifyConsumeResultToFloatWindow, TestSize.Level1)
{
    window_ = GetTestWindowImpl("NotifyConsumeResultToFloatWindow");
    ASSERT_NE(window_, nullptr);
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_TAB);
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_DOWN);
    window_->SetWatchGestureConsumed(false);
    bool isConsumed = false;
    window_->NotifyConsumeResultToFloatWindow(keyEvent, isConsumed);
    ASSERT_EQ(window_->GetWatchGestureConsumed(), false);
}

/**
 * @tc.name: IsSystemWindow
 * @tc.desc: IsSystemWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, IsSystemWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsSystemWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_FLOAT);
    bool res = window->IsSystemWindow();
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: IsAppWindow
 * @tc.desc: IsAppWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, IsAppWindow, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("IsAppWindow");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);

    window->hostSession_ = session;
    window->state_ = WindowState::STATE_CREATED;
    window->property_->SetWindowType(WindowType::APP_WINDOW_BASE);
    bool res = window->IsAppWindow();
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: SetMouseEventFilter
 * @tc.desc: SetMouseEventFilter
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetMouseEventFilter, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetMouseEventFilter");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    WMError res = window->SetMouseEventFilter([](const OHOS::MMI::PointerEvent& event) {
        return true;
    });
    ASSERT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: ClearMouseEventFilter
 * @tc.desc: ClearMouseEventFilter
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, ClearMouseEventFilter, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ClearMouseEventFilter");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    WMError res = window->ClearMouseEventFilter();
    ASSERT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: SetTouchEventFilter
 * @tc.desc: SetTouchEventFilter
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetTouchEventFilter, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetTouchEventFilter");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    WMError res = window->SetTouchEventFilter([](const OHOS::MMI::PointerEvent& event) {
        return true;
    });
    ASSERT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: ClearTouchEventFilter
 * @tc.desc: ClearTouchEventFilter
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, ClearTouchEventFilter, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ClearTouchEventFilter");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    ASSERT_EQ(WMError::WM_OK, window->Create(nullptr, session));
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    WMError res = window->ClearTouchEventFilter();
    ASSERT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: FilterPointerEvent
 * @tc.desc: FilterPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, FilterPointerEvent, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("FilterPointerEvent");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetSourceType(OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    pointerEvent->SetPointerAction(OHOS::MMI::PointerEvent::POINTER_ACTION_MOVE);
    window->touchEventFilter_ = nullptr;
    auto ret = window->FilterPointerEvent(pointerEvent);
    ASSERT_EQ(false, ret);

    window->SetTouchEventFilter([](const OHOS::MMI::PointerEvent& event) {
        return true;
    });
    ret = window->FilterPointerEvent(pointerEvent);
    ASSERT_EQ(true, ret);

    pointerEvent->SetPointerAction(OHOS::MMI::PointerEvent::POINTER_ACTION_MOVE);
    EXPECT_TRUE(logMsg.find("id") != std::string::npos);
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: FilterPointerEvent01
 * @tc.desc: FilterPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, FilterPointerEvent01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("FilterPointerEvent01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetSourceType(OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    pointerEvent->SetPointerAction(OHOS::MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);
    window->mouseEventFilter_ = nullptr;
    auto ret = window->FilterPointerEvent(pointerEvent);
    ASSERT_EQ(false, ret);

    window->SetMouseEventFilter([](const OHOS::MMI::PointerEvent& event) {
        return true;
    });
    ret = window->FilterPointerEvent(pointerEvent);
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: NotifyPointerEvent
 * @tc.desc: NotifyPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, NotifyPointerEvent, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyPointerEvent");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    window->NotifyPointerEvent(pointerEvent);

    pointerEvent = MMI::PointerEvent::Create();

    window->inputEventConsumer_ = nullptr;
    window->NotifyPointerEvent(pointerEvent);

    window->state_ = WindowState::STATE_CREATED;
    window->property_->persistentId_ = ROTATE_ANIMATION_DURATION;
    SessionInfo info;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(info);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    window->SetWindowDelayRaiseEnabled(true);
    ASSERT_EQ(true, window->IsWindowDelayRaiseEnabled());
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    window->NotifyPointerEvent(pointerEvent);
    EXPECT_TRUE(logMsg.find("Delay,id") == std::string::npos);
    logMsg.clear();

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);
    window->NotifyPointerEvent(pointerEvent);
    EXPECT_TRUE(logMsg.find("Delay,id") != std::string::npos);
    logMsg.clear();

    window->SetWindowDelayRaiseEnabled(false);
    window->NotifyPointerEvent(pointerEvent);
    EXPECT_TRUE(logMsg.find("UI content does not consume") != std::string::npos);
    logMsg.clear();

    window->inputEventConsumer_ = std::make_shared<MockInputEventConsumer>();
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UNKNOWN);
    ASSERT_EQ(pointerEvent->GetPointerAction(), MMI::PointerEvent::POINTER_ACTION_UNKNOWN);
    window->NotifyPointerEvent(pointerEvent);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    window->NotifyPointerEvent(pointerEvent);
}

/**
 * @tc.name: InjectTouchEvent
 * @tc.desc: InjectTouchEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, InjectTouchEvent, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("InjectTouchEvent");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    auto ret = window->InjectTouchEvent(pointerEvent);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
 
    pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    window->uiContent_ = nullptr;
    ret = window->InjectTouchEvent(pointerEvent);
    ASSERT_EQ(ret, WMError::WM_ERROR_SYSTEM_ABNORMALLY);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UNKNOWN);
    ret = window->InjectTouchEvent(pointerEvent);
    ASSERT_EQ(ret, WMError::WM_ERROR_SYSTEM_ABNORMALLY);

    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ret = window->InjectTouchEvent(pointerEvent);
    ASSERT_EQ(ret, WMError::WM_OK);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    ret = window->InjectTouchEvent(pointerEvent);
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: SetAvoidAreaOption01
 * @tc.desc: SetAvoidAreaOption
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetAvoidAreaOption01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetAvoidAreaOption01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    window->state_ = WindowState::STATE_DESTROYED;
    auto ret = window->SetAvoidAreaOption(0);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: GetAvoidAreaOption01
 * @tc.desc: GetAvoidAreaOption
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, GetAvoidAreaOption01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GetAvoidAreaOption01");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    window->state_ = WindowState::STATE_DESTROYED;
    uint32_t avoidAreaOption = 1;
    auto ret = window->SetAvoidAreaOption(avoidAreaOption);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: SetWindowDelayRaiseEnabled
 * @tc.desc: SetWindowDelayRaiseEnabled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SetWindowDelayRaiseEnabled, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetWindowDelayRaiseEnabled");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    window->state_ = WindowState::STATE_DESTROYED;
    auto ret = window->SetWindowDelayRaiseEnabled(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->state_ = WindowState::STATE_CREATED;
    window->property_->persistentId_ = ROTATE_ANIMATION_DURATION;
    SessionInfo info;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(info);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    window->windowSystemConfig_.freeMultiWindowSupport_ = true;
    ret = window->SetWindowDelayRaiseEnabled(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ret = window->SetWindowDelayRaiseEnabled(true);
    ASSERT_EQ(ret, WMError::WM_OK);

    window->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    window->property_->SetPcAppInpadCompatibleMode(true);
    window->windowSystemConfig_.freeMultiWindowEnable_ = false;
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    ret = window->SetWindowDelayRaiseEnabled(true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: NotifyWatchFocusActiveChange
 * @tc.desc: NotifyWatchFocusActiveChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, NotifyWatchFocusActiveChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("NotifyWatchFocusActiveChange");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);

    window->state_ = WindowState::STATE_DESTROYED;
    auto ret = window->NotifyWatchFocusActiveChange(true);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_WINDOW);

    window->state_ = WindowState::STATE_CREATED;
    SessionInfo info;
    window->hostSession_ = sptr<SessionMocker>::MakeSptr(info);
    window->property_->persistentId_ = ROTATE_ANIMATION_DURATION;
    ret = window->SetWindowDelayRaiseEnabled(true);
}

/**
 * @tc.name: UpdateSubWindowInfo
 * @tc.desc: UpdateSubWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, UpdateSubWindowInfo, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("UpdateSubWindowInfo");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    window->hostSession_ = session;
    window->property_->SetPersistentId(1);
    window->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    sptr<WindowOption> subWindowOption = sptr<WindowOption>::MakeSptr();
    subWindowOption->SetWindowName("UpdateSubWindowInfo_subWindow");
    sptr<WindowSessionImpl> subWindow = sptr<WindowSessionImpl>::MakeSptr(subWindowOption);
    subWindow->property_->SetPersistentId(2);
    subWindow->property_->SetParentPersistentId(1);
    subWindow->hostSession_ = session;
    subWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subWindow->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subWindow->context_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    WindowSessionImpl::subWindowSessionMap_.insert(std::pair<int32_t,
        std::vector<sptr<WindowSessionImpl>>>(1, { subWindow }));

    EXPECT_NE(subWindow->context_, nullptr);
    EXPECT_EQ(subWindow->property_->GetSubWindowLevel(), 0);
    int subWindowLevel = 5;
    window->UpdateSubWindowInfo(subWindowLevel, nullptr);
    EXPECT_EQ(subWindow->property_->GetSubWindowLevel(), 6);
    EXPECT_EQ(subWindow->context_, nullptr);
    EXPECT_EQ(WMError::WM_OK, subWindow->Destroy(true));
    EXPECT_EQ(WMError::WM_OK, window->Destroy(true));
}

/**
 * @tc.name: RegisterFreeWindowModeChangeListener
 * @tc.desc: RegisterFreeWindowModeChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, RegisterFreeWindowModeChangeListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("freeWindow1");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(1);
    sptr<IFreeWindowModeChangeListener> listener = sptr<IFreeWindowModeChangeListener>::MakeSptr();
    auto ret = window->RegisterFreeWindowModeChangeListener(listener);
    window->NotifyFreeWindowModeChange(true);
    EXPECT_EQ(WMError::WM_OK, ret);
 
    listener = nullptr;
    window->NotifyFreeWindowModeChange(true);
    ret = window->RegisterFreeWindowModeChangeListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    window->Destroy();
}
 
/**
 * @tc.name: UnregisterFreeWindowModeChangeListener
 * @tc.desc: UnregisterFreeWindowModeChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, UnregisterFreeWindowModeChangeListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("freeWindow2");
    sptr<WindowSessionImpl> window = sptr<WindowSessionImpl>::MakeSptr(option);
    window->property_->SetPersistentId(2);
    sptr<IFreeWindowModeChangeListener> listener = sptr<IFreeWindowModeChangeListener>::MakeSptr();
    auto ret = window->RegisterFreeWindowModeChangeListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
 
    auto ret1 = window->UnregisterFreeWindowModeChangeListener((listener));
    EXPECT_EQ(WMError::WM_OK, ret1);
    window->Destroy();
}

/**
 * @tc.name: SwitchSystemWindow
 * @tc.desc: SwitchSystemWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, SwitchSystemWindow, Function | SmallTest | Level1)
{
    window_ = GetTestWindowImpl("GetUIContentWithId");
    ASSERT_NE(window_, nullptr);
    window_->windowSessionMap_.clear();
    sptr<WindowOption> sysOption = sptr<WindowOption>::MakeSptr();
    sysOption->SetWindowName("SwitchSystemWindow");
    sysOption->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sptr<WindowSessionImpl> sysWindow = sptr<WindowSessionImpl>::MakeSptr(sysOption);
    ASSERT_NE(sysWindow, nullptr);
    ASSERT_NE(sysWindow->property_, nullptr);
    sysWindow->property_->SetPersistentId(PERSISTENT_ID_TWO);
    sysWindow->windowSystemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    sysWindow->windowSystemConfig_.freeMultiWindowSupport_ = true;
    sysWindow->windowSystemConfig_.isSystemDecorEnable_ = true;
    // cover empty map
    sysWindow->SwitchSystemWindow(false, PERSISTENT_ID_ONE);
 
    std::vector<sptr<WindowSessionImpl>> vec;
    window_->windowSessionMap_.insert(std::make_pair("SwitchSystemWindow",
        std::make_pair(PERSISTENT_ID_ONE, sysWindow)));
    window_->SwitchSystemWindow(true, PERSISTENT_ID_ONE);
    EXPECT_EQ(sysWindow->windowSystemConfig_.freeMultiWindowEnable_, true);
}

/**
 * @tc.name: NotifyUIExtHostRectChangeInGlobalDisplayListeners
 * @tc.desc: NotifyUIExtHostRectChangeInGlobalDisplayListeners test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, NotifyUIExtHostRectChangeInGlobalDisplayListeners, TestSize.Level1)
{
    sptr<WindowSessionImpl> window = GetTestWindowImpl("NotifyUIExtHostRectChangeInGlobalDisplayListeners");
    window->uiContent_ = std::make_unique<Ace::UIContentMocker>();
    Rect rect;
    WindowSizeChangeReason reason = WindowSizeChangeReason::MOVE;
    window->NotifyUIExtHostRectChangeInGlobalDisplayListeners(rect, reason);
    window->rectChangeInGlobalDisplayUIExtListenerIds_.emplace(111);
    ASSERT_FALSE(window->rectChangeInGlobalDisplayUIExtListenerIds_.empty());
    window->NotifyUIExtHostRectChangeInGlobalDisplayListeners(rect, reason);

    window->property_->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    window->NotifyUIExtHostRectChangeInGlobalDisplayListeners(rect, reason);
    window->rectChangeInGlobalDisplayUIExtListenerIds_.clear();
    ASSERT_TRUE(window->rectChangeInGlobalDisplayUIExtListenerIds_.empty());
    window->NotifyUIExtHostRectChangeInGlobalDisplayListeners(rect, reason);
}

/**
 * @tc.name: UpdateSubWindowStateWithOptions
 * @tc.desc: UpdateSubWindowStateWithOptions
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionImplTest3, UpdateSubWindowStateWithOptions, Function | SmallTest | Level1)
{
    window_ = GetTestWindowImpl("MainWindow");
    ASSERT_NE(window_, nullptr);
    window_->property_->SetPersistentId(100);
    StateChangeOption option(window_->GetPersistentId(), WindowState::STATE_HIDDEN, 0, false, false, false, false,
        false);
    window_->UpdateSubWindowStateWithOptions(option);

    sptr<WindowSessionImpl> subwindow1 = nullptr;
    sptr<WindowSessionImpl> subwindow2 = GetTestWindowImpl("SubWindow2");
    ASSERT_NE(subwindow2, nullptr);
    subwindow2->state_ = WindowState::STATE_SHOWN;
    subwindow2->property_->SetPersistentId(2);
    sptr<WindowSessionImpl> subwindow3 = GetTestWindowImpl("SubWindow3");
    ASSERT_NE(subwindow3, nullptr);
    subwindow3->state_ = WindowState::STATE_HIDDEN;
    subwindow2->property_->SetPersistentId(3);
    sptr<WindowSessionImpl> subwindow4 = GetTestWindowImpl("SubWindow4");
    ASSERT_NE(subwindow4, nullptr);
    subwindow3->state_ = WindowState::STATE_INITIAL;
    window_->subWindowSessionMap_[window_->GetPersistentId()].push_back(subwindow1);
    window_->subWindowSessionMap_[window_->GetPersistentId()].push_back(subwindow2);
    window_->subWindowSessionMap_[window_->GetPersistentId()].push_back(subwindow3);
    window_->subWindowSessionMap_[window_->GetPersistentId()].push_back(subwindow4);
    // Hide branch
    window_->UpdateSubWindowStateWithOptions(option);
    subwindow2->followCreatorLifecycle_ = true;
    subwindow2->state_ = WindowState::STATE_SHOWN;
    window_->UpdateSubWindowStateWithOptions(option);
    ASSERT_EQ(subwindow2->state_, WindowState::STATE_HIDDEN);

    // Show branch
    option.newState_ = WindowState::STATE_SHOWN;
    window_->UpdateSubWindowStateWithOptions(option);
    subwindow3->requestState_ = WindowState::STATE_SHOWN;
    subwindow3->state_ = WindowState::STATE_HIDDEN;
    window_->UpdateSubWindowStateWithOptions(option);
    subwindow3->followCreatorLifecycle_ = true;
    subwindow3->state_ = WindowState::STATE_HIDDEN;
    window_->UpdateSubWindowStateWithOptions(option);
    ASSERT_EQ(subwindow3->state_, WindowState::STATE_SHOWN);
}
} // namespace
} // namespace Rosen
} // namespace OHOS