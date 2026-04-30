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

#include "window_adapter.h"
#include "window_agent.h"
#include "window_property.h"
#include "session_manager.h"
#include "singleton_mocker.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowRecoverManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowRecoverManagerTest::SetUpTestCase() {}

void WindowRecoverManagerTest::TearDownTestCase()
{
}

void WindowRecoverManagerTest::SetUp() {}

void WindowRecoverManagerTest::TearDown() {}

namespace {
/**
 * @tc.name: RegisterSessionRecoverCallbackFunc
 * @tc.desc: WindowAdapter/RegisterSessionRecoverCallbackFunc
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest, RegisterSessionRecoverCallbackFunc, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    int32_t persistentId = 1;
    auto testFunc = [] { return WMError::WM_OK; };
    windowAdapter.RegisterSessionRecoverCallbackFunc(persistentId, testFunc);
    ASSERT_NE(windowAdapter.sessionRecoverCallbackFuncMap_[persistentId], nullptr);
}

/**
 * @tc.name: RegisterSessionRecoverListener
 * @tc.desc: RegisterSessionRecoverListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest, RegisterSessionRecoverListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RegisterSessionRecoverListener");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    int32_t persistentId = windowSceneSessionImpl->property_->GetPersistentId();
    WindowAdapter& windowAdapter = SingletonContainer::Get<WindowAdapter>();
    windowSceneSessionImpl->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    windowSceneSessionImpl->RegisterSessionRecoverListener(true);
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 0);

    windowSceneSessionImpl->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    windowSceneSessionImpl->property_->SetCollaboratorType(CollaboratorType::RESERVE_TYPE);
    windowSceneSessionImpl->RegisterSessionRecoverListener(true);
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 0);

    windowSceneSessionImpl->property_->SetCollaboratorType(CollaboratorType::DEFAULT_TYPE);
    windowSceneSessionImpl->RegisterSessionRecoverListener(true);
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 1);
    windowAdapter.UnregisterSessionRecoverCallbackFunc(persistentId);
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 0);

    windowSceneSessionImpl->state_ = WindowState::STATE_DESTROYED;
    windowSceneSessionImpl->RegisterSessionRecoverListener(true);
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 1);
    windowAdapter.UnregisterSessionRecoverCallbackFunc(persistentId);
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 0);

    windowSceneSessionImpl->state_ = WindowState::STATE_SHOWN;
    windowSceneSessionImpl->RegisterSessionRecoverListener(true);
    EXPECT_EQ(CollaboratorType::DEFAULT_TYPE, windowSceneSessionImpl->property_->GetCollaboratorType());
    ASSERT_EQ(windowAdapter.sessionRecoverCallbackFuncMap_.size(), 1);
    windowAdapter.UnregisterSessionRecoverCallbackFunc(persistentId);
    EXPECT_EQ(CollaboratorType::DEFAULT_TYPE, windowSceneSessionImpl->property_->GetCollaboratorType());
}

/**
 * @tc.name: RecoverAndReconnectSceneSession
 * @tc.desc: WindowAdapter/RecoverAndReconnectSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest, RecoverAndReconnectSceneSession01, TestSize.Level1)
{
    WindowAdapter windowAdapter;
    sptr<ISessionStage> iSessionStage;
    sptr<IWindowEventChannel> eventChannel;
    sptr<ISession> session = nullptr;
    sptr<IRemoteObject> token = nullptr;
    auto ret1 =
        windowAdapter.RecoverAndReconnectSceneSession(iSessionStage, eventChannel, nullptr, session, nullptr, token);
    ASSERT_EQ(ret1, WMError::WM_OK);

    windowAdapter.isProxyValid_ = true;
    auto ret2 =
        windowAdapter.RecoverAndReconnectSceneSession(iSessionStage, eventChannel, nullptr, session, nullptr, token);
    ASSERT_EQ(ret2, WMError::WM_OK);

    sptr<IRemoteObject> remotObject = nullptr;
    windowAdapter.windowManagerServiceProxy_ = iface_cast<IWindowManager>(remotObject);
    auto ret3 =
        windowAdapter.RecoverAndReconnectSceneSession(iSessionStage, eventChannel, nullptr, session, nullptr, token);
    ASSERT_EQ(ret3, WMError::WM_DO_NOTHING);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession
 * @tc.desc: RecoverAndReconnectSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest, RecoverAndReconnectSceneSession02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RecoverAndReconnectSceneSession");
    sptr<WindowSceneSessionImpl> windowSceneSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);

    auto ret = windowSceneSessionImpl->RecoverAndReconnectSceneSession();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    windowSceneSessionImpl->isFocused_ = true;
    ret = windowSceneSessionImpl->RecoverAndReconnectSceneSession();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession
 * @tc.desc: RecoverAndReconnectSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest, RecoverAndReconnectSceneSession, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("RecoverAndReconnectSceneSession");
    sptr<WindowSceneSessionImpl> windowSceneSession = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    ASSERT_NE(nullptr, windowSceneSession);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowSceneSession->RecoverAndReconnectSceneSession());
}

/**
 * @tc.name: RegisterSessionRecoverListenerInputMethodFloat
 * @tc.desc: Register session recover listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest, RegisterSessionRecoverListenerInputMethodFloat, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    window->RegisterSessionRecoverListener(false);
    ASSERT_EQ(SingletonContainer::Get<WindowAdapter>().sessionRecoverCallbackFuncMap_.size(), 0);
}

/**
 * @tc.name: RegisterSessionRecoverListenerNonDefaultCollaboratorType01
 * @tc.desc: Register session recover listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest,
         RegisterSessionRecoverListenerNonDefaultCollaboratorType01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetCollaboratorType(CollaboratorType::OTHERS_TYPE);
    window->RegisterSessionRecoverListener(false); // false is main window
    ASSERT_EQ(SingletonContainer::Get<WindowAdapter>().sessionRecoverCallbackFuncMap_.size(), 0);
}

/**
 * @tc.name: RegisterSessionRecoverListenerNonDefaultCollaboratorType02
 * @tc.desc: Register session recover listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest,
         RegisterSessionRecoverListenerNonDefaultCollaboratorType02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetCollaboratorType(CollaboratorType::OTHERS_TYPE);
    window->RegisterSessionRecoverListener(true); // true is sub window
    ASSERT_EQ(SingletonContainer::Get<WindowAdapter>().sessionRecoverCallbackFuncMap_.size(), 0);
}

/**
 * @tc.name: RegisterSessionRecoverListenerSuccess01
 * @tc.desc: Register session recover listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest, RegisterSessionRecoverListenerSuccess01, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetCollaboratorType(CollaboratorType::DEFAULT_TYPE);
    window->RegisterSessionRecoverListener(false); // false is main window
    ASSERT_EQ(SingletonContainer::Get<WindowAdapter>().sessionRecoverCallbackFuncMap_.size(), 1);
    SingletonContainer::Get<WindowAdapter>().sessionRecoverCallbackFuncMap_.clear();
}

/**
 * @tc.name: RegisterSessionRecoverListenerSuccess02
 * @tc.desc: Register session recover listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest, RegisterSessionRecoverListenerSuccess02, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetCollaboratorType(CollaboratorType::DEFAULT_TYPE);
    window->RegisterSessionRecoverListener(true); // true is sub window
    ASSERT_EQ(SingletonContainer::Get<WindowAdapter>().sessionRecoverCallbackFuncMap_.size(), 1);
    SingletonContainer::Get<WindowAdapter>().sessionRecoverCallbackFuncMap_.clear();
}

/**
 * @tc.name: RegisterWindowRecoverStateChangeListener
 * @tc.desc: Register Window Recover State Change Listener
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest, RegisterWindowRecoverStateChangeListener, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetCollaboratorType(CollaboratorType::DEFAULT_TYPE);
    window->RegisterWindowRecoverStateChangeListener();
    EXPECT_NE(window->windowRecoverStateChangeFunc_, nullptr);
}

/**
 * @tc.name: OnWindowRecoverStateChange
 * @tc.desc: On Window Recover State Change
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest, OnWindowRecoverStateChange, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetCollaboratorType(CollaboratorType::DEFAULT_TYPE);
    window->OnWindowRecoverStateChange(false, WindowRecoverState::WINDOW_START_RECONNECT);
    EXPECT_EQ(window->property_->GetWindowState(), window->state_);

    window->UpdateFocusState(true);
    window->NotifyHighlightChange(true);
    window->OnWindowRecoverStateChange(false, WindowRecoverState::WINDOW_START_RECONNECT);
    EXPECT_EQ(window->property_->GetWindowState(), window->state_);

    window->OnWindowRecoverStateChange(false, WindowRecoverState::WINDOW_FINISH_RECONNECT);
    EXPECT_EQ(window->property_->GetWindowState(), window->state_);
}

/**
 * @tc.name: UpdateStartRecoverProperty
 * @tc.desc: Update Start Recover Property
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverManagerTest, UpdateStartRecoverProperty, TestSize.Level1)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    window->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window->property_->SetCollaboratorType(CollaboratorType::DEFAULT_TYPE);
    window->UpdateStartRecoverProperty(false);
    EXPECT_EQ(window->property_->GetWindowState(), window->state_);

    sptr<WindowOption> option2 = sptr<WindowOption>::MakeSptr();
    sptr<WindowSceneSessionImpl> window2 = sptr<WindowSceneSessionImpl>::MakeSptr(option2);
    window2->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    window2->property_->SetCollaboratorType(CollaboratorType::DEFAULT_TYPE);
    window2->UpdateStartRecoverProperty(true);
    EXPECT_EQ(window2->property_->GetWindowState(), window2->requestState_);
}

} // namespace
} // namespace Rosen
} // namespace OHOS