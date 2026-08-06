/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ability_context_impl.h"
#include "mock_session.h"
#include "mock_uicontent.h"
#include "mock_window_adapter.h"
#include "scene_board_judgement.h"
#include "session/host/include/scene_session.h"
#include "singleton_mocker.h"
#include "window_scene.h"
#include "window_scene_session_impl.h"
#include "window_session_impl.h"

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

class MockWindowLifeCycleListener : public IWindowLifeCycle {
public:
    MOCK_METHOD0(AfterForeground, void(void));
    MOCK_METHOD0(AfterBackground, void(void));
    MOCK_METHOD0(AfterDidForeground, void(void));
    MOCK_METHOD0(AfterDidBackground, void(void));
    MOCK_METHOD0(AfterFocused, void(void));
    MOCK_METHOD0(AfterUnfocused, void(void));
    MOCK_METHOD1(ForegroundFailed, void(int32_t));
    MOCK_METHOD0(AfterActive, void(void));
    MOCK_METHOD0(AfterInactive, void(void));
    MOCK_METHOD0(AfterResumed, void(void));
    MOCK_METHOD0(AfterPaused, void(void));
    MOCK_METHOD0(AfterDestroyed, void(void));
};

class WindowSceneGamePrelaunchTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_;
    std::unique_ptr<Mocker> m = std::make_unique<Mocker>();

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void WindowSceneGamePrelaunchTest::SetUpTestCase() {}

void WindowSceneGamePrelaunchTest::TearDownTestCase() {}

void WindowSceneGamePrelaunchTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void WindowSceneGamePrelaunchTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

namespace {
/**
 * @tc.name: WindowSceneGoForegroundWithGamePrelaunch01
 * @tc.desc: Test WindowScene GoForeground with isGamePreLaunch=true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneGamePrelaunchTest, WindowSceneGoForegroundWithGamePrelaunch01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowSceneGoForegroundWithGamePrelaunch01");
    sptr<WindowSceneSessionImpl> mainWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    mainWindow->Create(abilityContext_, session);
    mainWindow->property_->SetPersistentId(1);
    
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    mockListener->SetIsWindowSceneListener(true);
    mainWindow->RegisterLifeCycleListener(mockListener);
    
    WindowScene windowScene;
    windowScene.mainWindow_ = mainWindow;
    windowScene.mainWindowId_ = mainWindow->GetWindowId();
    
    EXPECT_CALL(*mockListener, AfterForeground()).Times(1);
    auto ret = windowScene.GoForeground(0, true);
    
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: WindowSceneGoForegroundWithGamePrelaunch02
 * @tc.desc: Test WindowScene GoForeground with isGamePreLaunch=false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneGamePrelaunchTest, WindowSceneGoForegroundWithGamePrelaunch02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowSceneGoForegroundWithGamePrelaunch02");
    sptr<WindowSceneSessionImpl> mainWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    mainWindow->Create(abilityContext_, session);
    mainWindow->property_->SetPersistentId(1);
    
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    mainWindow->RegisterLifeCycleListener(mockListener);
    
    WindowScene windowScene;
    windowScene.mainWindow_ = mainWindow;
    windowScene.mainWindowId_ = mainWindow->GetWindowId();
    
    EXPECT_CALL(*mockListener, AfterForeground()).Times(1);
    auto ret = windowScene.GoForeground(0, false);
    
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: WindowSceneGoResumeWithGamePrelaunch01
 * @tc.desc: Test WindowScene GoResume with isGamePreLaunch=true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneGamePrelaunchTest, WindowSceneGoResumeWithGamePrelaunch01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowSceneGoResumeWithGamePrelaunch01");
    sptr<WindowSceneSessionImpl> mainWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    mainWindow->Create(abilityContext_, session);
    mainWindow->property_->SetPersistentId(1);
    
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    mockListener->SetIsWindowSceneListener(true);
    mainWindow->RegisterLifeCycleListener(mockListener);
    
    WindowScene windowScene;
    windowScene.mainWindow_ = mainWindow;
    windowScene.mainWindowId_ = mainWindow->GetWindowId();
    
    EXPECT_CALL(*mockListener, AfterResumed()).Times(1);
    auto ret = windowScene.GoResume(true);
    
    EXPECT_EQ(ret, WMError::WM_OK);
    EXPECT_TRUE(mainWindow->isDidForeground_);
}

/**
 * @tc.name: WindowSceneGoResumeWithGamePrelaunch02
 * @tc.desc: Test WindowScene GoResume with isGamePreLaunch=false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneGamePrelaunchTest, WindowSceneGoResumeWithGamePrelaunch02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowSceneGoResumeWithGamePrelaunch02");
    sptr<WindowSceneSessionImpl> mainWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    mainWindow->Create(abilityContext_, session);
    mainWindow->property_->SetPersistentId(1);
    
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    mainWindow->RegisterLifeCycleListener(mockListener);
    
    WindowScene windowScene;
    windowScene.mainWindow_ = mainWindow;
    windowScene.mainWindowId_ = mainWindow->GetWindowId();
    
    EXPECT_CALL(*mockListener, AfterResumed()).Times(1);
    auto ret = windowScene.GoResume(false);
    
    EXPECT_EQ(ret, WMError::WM_OK);
    EXPECT_TRUE(mainWindow->isDidForeground_);
}

/**
 * @tc.name: WindowSceneIntegration01
 * @tc.desc: Integration test for WindowScene with GamePrelaunch
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneGamePrelaunchTest, WindowSceneIntegration01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowSceneIntegration01");
    sptr<WindowSceneSessionImpl> mainWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    mainWindow->Create(abilityContext_, session);
    mainWindow->property_->SetPersistentId(1);
    
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    mockListener->SetIsWindowSceneListener(true);
    mainWindow->RegisterLifeCycleListener(mockListener);
    
    WindowScene windowScene;
    windowScene.mainWindow_ = mainWindow;
    windowScene.mainWindowId_ = mainWindow->GetWindowId();
    
    EXPECT_CALL(*mockListener, AfterForeground()).Times(1);
    auto ret1 = windowScene.GoForeground(0, true);
    EXPECT_EQ(ret1, WMError::WM_OK);
    
    EXPECT_CALL(*mockListener, AfterResumed()).Times(1);
    auto ret2 = windowScene.GoResume(true);
    EXPECT_EQ(ret2, WMError::WM_OK);
    
    EXPECT_TRUE(mainWindow->isDidForeground_);
}

/**
 * @tc.name: WindowSceneIntegration02
 * @tc.desc: Integration test for WindowScene without GamePrelaunch
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneGamePrelaunchTest, WindowSceneIntegration02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("WindowSceneIntegration02");
    sptr<WindowSceneSessionImpl> mainWindow = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    mainWindow->Create(abilityContext_, session);
    mainWindow->property_->SetPersistentId(1);
    
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    mainWindow->RegisterLifeCycleListener(mockListener);
    
    WindowScene windowScene;
    windowScene.mainWindow_ = mainWindow;
    windowScene.mainWindowId_ = mainWindow->GetWindowId();
    
    EXPECT_CALL(*mockListener, AfterForeground()).Times(1);
    auto ret1 = windowScene.GoForeground(0, false);
    EXPECT_EQ(ret1, WMError::WM_OK);
    
    EXPECT_CALL(*mockListener, AfterResumed()).Times(1);
    auto ret2 = windowScene.GoResume(false);
    EXPECT_EQ(ret2, WMError::WM_OK);
    
    EXPECT_TRUE(mainWindow->isDidForeground_);
}

/**
 * @tc.name: WindowSceneGoForegroundWithNullWindow
 * @tc.desc: Test WindowScene GoForeground with null main window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneGamePrelaunchTest, WindowSceneGoForegroundWithNullWindow, TestSize.Level0)
{
    WindowScene windowScene;
    windowScene.mainWindow_ = nullptr;
    
    auto ret = windowScene.GoForeground(0, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: WindowSceneGoResumeWithNullWindow
 * @tc.desc: Test WindowScene GoResume with null main window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSceneGamePrelaunchTest, WindowSceneGoResumeWithNullWindow, TestSize.Level0)
{
    WindowScene windowScene;
    windowScene.mainWindow_ = nullptr;
    
    auto ret = windowScene.GoResume(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
}
}
}
}
