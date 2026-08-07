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

class GamePrelaunchTest : public testing::Test {
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

void GamePrelaunchTest::SetUpTestCase() {}

void GamePrelaunchTest::TearDownTestCase() {}

void GamePrelaunchTest::SetUp()
{
    abilityContext_ = std::make_shared<AbilityRuntime::AbilityContextImpl>();
}

void GamePrelaunchTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    abilityContext_ = nullptr;
}

namespace {
/**
 * @tc.name: SetIsGamePreLaunch01
 * @tc.desc: Test SetIsGamePreLaunch with true value
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, SetIsGamePreLaunch01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetIsGamePreLaunch01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    window->Create(abilityContext_, session);
    window->property_->SetPersistentId(1);
    
    auto ret = window->SetIsGamePreLaunch(true);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: SetIsGamePreLaunch02
 * @tc.desc: Test SetIsGamePreLaunch with false value
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, SetIsGamePreLaunch02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("SetIsGamePreLaunch02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    window->Create(abilityContext_, session);
    window->property_->SetPersistentId(1);
    
    auto ret = window->SetIsGamePreLaunch(false);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: ClearIsGamePreLaunch01
 * @tc.desc: Test ClearIsGamePreLaunch
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, ClearIsGamePreLaunch01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ClearIsGamePreLaunch01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    window->Create(abilityContext_, session);
    window->property_->SetPersistentId(1);
    
    window->SetIsGamePreLaunch(true);
    auto ret = window->ClearIsGamePreLaunch();
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: ResumeWithGamePrelaunch01
 * @tc.desc: Test Resume with isGamePreLaunch=true
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, ResumeWithGamePrelaunch01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ResumeWithGamePrelaunch01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    window->Create(abilityContext_, session);
    window->property_->SetPersistentId(1);
    
    window->Resume(true);
    EXPECT_TRUE(window->isDidForeground_);
    EXPECT_FALSE(window->isColdStart_);
}

/**
 * @tc.name: ResumeWithGamePrelaunch02
 * @tc.desc: Test Resume with isGamePreLaunch=false
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, ResumeWithGamePrelaunch02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("ResumeWithGamePrelaunch02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    window->Create(abilityContext_, session);
    window->property_->SetPersistentId(1);
    
    window->Resume(false);
    EXPECT_TRUE(window->isDidForeground_);
    EXPECT_FALSE(window->isColdStart_);
}

/**
 * @tc.name: SetIsWindowSceneListener01
 * @tc.desc: Test SetIsWindowSceneListener with true value
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, SetIsWindowSceneListener01, TestSize.Level0)
{
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    
    mockListener->SetIsWindowSceneListener(true);
    EXPECT_TRUE(mockListener->IsWindowSceneListener());
}

/**
 * @tc.name: SetIsWindowSceneListener02
 * @tc.desc: Test SetIsWindowSceneListener with false value
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, SetIsWindowSceneListener02, TestSize.Level0)
{
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    
    mockListener->SetIsWindowSceneListener(false);
    EXPECT_FALSE(mockListener->IsWindowSceneListener());
}

/**
 * @tc.name: SessionInfoIsGamePrelaunch01
 * @tc.desc: Test SessionInfo isGamePrelaunch_ field
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, SessionInfoIsGamePrelaunch01, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "TestBundle";
    sessionInfo.abilityName_ = "TestAbility";
    sessionInfo.isGamePrelaunch_ = true;
    
    EXPECT_TRUE(sessionInfo.isGamePrelaunch_);
}

/**
 * @tc.name: SessionInfoIsGamePrelaunch02
 * @tc.desc: Test SessionInfo isGamePrelaunch_ default value
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, SessionInfoIsGamePrelaunch02, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "TestBundle";
    sessionInfo.abilityName_ = "TestAbility";
    
    EXPECT_FALSE(sessionInfo.isGamePrelaunch_);
}

/**
 * @tc.name: LifeCycleChangeReasonGamePrelaunchBackground
 * @tc.desc: Test LifeCycleChangeReason::GAME_PRELAUNCH_BACKGROUND enum value
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, LifeCycleChangeReasonGamePrelaunchBackground, TestSize.Level0)
{
    LifeCycleChangeReason reason = LifeCycleChangeReason::GAME_PRELAUNCH_BACKGROUND;
    EXPECT_EQ(static_cast<int32_t>(reason), static_cast<int32_t>(LifeCycleChangeReason::QUICK_BATCH_BACKGROUND) + 1);
}

/**
 * @tc.name: GamePrelaunchLifecycleListener01
 * @tc.desc: Test lifecycle listener behavior with GamePrelaunch
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, GamePrelaunchLifecycleListener01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GamePrelaunchLifecycleListener01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    window->Create(abilityContext_, session);
    window->property_->SetPersistentId(1);
    
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    window->RegisterLifeCycleListener(mockListener);
    
    window->SetIsGamePreLaunch(true);
    
    EXPECT_CALL(*mockListener, AfterForeground()).Times(1);
    window->NotifyAfterForeground(true, true);
}

/**
 * @tc.name: GamePrelaunchLifecycleListener02
 * @tc.desc: Test lifecycle listener behavior without GamePrelaunch
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, GamePrelaunchLifecycleListener02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GamePrelaunchLifecycleListener02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    window->Create(abilityContext_, session);
    window->property_->SetPersistentId(1);
    
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    window->RegisterLifeCycleListener(mockListener);
    
    window->SetIsGamePreLaunch(false);
    
    EXPECT_CALL(*mockListener, AfterForeground()).Times(1);
    window->NotifyAfterForeground(true, true);
}

/**
 * @tc.name: GamePrelaunchWindowSceneListener01
 * @tc.desc: Test lifecycle listener with WindowSceneListener and GamePrelaunch
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, GamePrelaunchWindowSceneListener01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GamePrelaunchWindowSceneListener01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    window->Create(abilityContext_, session);
    window->property_->SetPersistentId(1);
    
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    mockListener->SetIsWindowSceneListener(true);
    window->RegisterLifeCycleListener(mockListener);
    
    window->SetIsGamePreLaunch(true);
    
    EXPECT_CALL(*mockListener, AfterResumed()).Times(1);
    window->NotifyAfterResumed();
}

/**
 * @tc.name: GamePrelaunchWindowSceneListener02
 * @tc.desc: Test non-WindowSceneListener with GamePrelaunch should not be called
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, GamePrelaunchWindowSceneListener02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GamePrelaunchWindowSceneListener02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    window->Create(abilityContext_, session);
    window->property_->SetPersistentId(1);
    
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    mockListener->SetIsWindowSceneListener(false);
    window->RegisterLifeCycleListener(mockListener);
    
    window->SetIsGamePreLaunch(true);
    
    EXPECT_CALL(*mockListener, AfterResumed()).Times(0);
    window->NotifyAfterResumed();
}

/**
 * @tc.name: GamePrelaunchIntegration01
 * @tc.desc: Integration test for GamePrelaunch flow
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, GamePrelaunchIntegration01, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GamePrelaunchIntegration01");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    window->Create(abilityContext_, session);
    window->property_->SetPersistentId(1);
    
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    mockListener->SetIsWindowSceneListener(true);
    window->RegisterLifeCycleListener(mockListener);
    
    window->SetIsGamePreLaunch(true);
    window->Resume(true);
    
    EXPECT_TRUE(window->isDidForeground_);
    EXPECT_FALSE(window->isColdStart_);
    
    window->ClearIsGamePreLaunch();
}

/**
 * @tc.name: GamePrelaunchIntegration02
 * @tc.desc: Integration test for normal flow without GamePrelaunch
 * @tc.type: FUNC
 */
HWTEST_F(GamePrelaunchTest, GamePrelaunchIntegration02, TestSize.Level0)
{
    sptr<WindowOption> option = sptr<WindowOption>::MakeSptr();
    option->SetWindowName("GamePrelaunchIntegration02");
    sptr<WindowSceneSessionImpl> window = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    SessionInfo sessionInfo = { "TestBundle", "TestModule", "TestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    
    window->Create(abilityContext_, session);
    window->property_->SetPersistentId(1);
    
    auto mockListener = sptr<MockWindowLifeCycleListener>::MakeSptr();
    window->RegisterLifeCycleListener(mockListener);
    
    window->SetIsGamePreLaunch(false);
    window->Resume(false);
    
    EXPECT_TRUE(window->isDidForeground_);
    EXPECT_FALSE(window->isColdStart_);
}
}
}
}
