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
#include <regex>
#include <pointer_event.h>
#include <ui/rs_surface_node.h>

#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "mock/mock_pattern_detach_callback.h"
#include "session/host/include/extension_session.h"
#include "session/host/include/move_drag_controller.h"
#include "session/host/include/scene_session.h"
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/session.h"
#include "session_info.h"
#include "key_event.h"
#include "wm_common.h"
#include "window_event_channel_base.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string UNDEFINED = "undefined";
}

class WindowSessionLifecycleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    int32_t GetTaskCount();
    sptr<SceneSessionManager> ssm_;

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();

    sptr<Session> session_ = nullptr;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;

    class TLifecycleListener : public ILifecycleListener {
    public:
        virtual ~TLifecycleListener() {}
        void OnActivation() override {}
        void OnConnect() override {}
        void OnForeground() override {}
        void OnBackground() override {}
        void OnDisconnect() override {}
        void OnExtensionDied() override {}
        void OnExtensionTimeout(int32_t errorCode) override {}
        void OnAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
                                  int64_t uiExtensionIdLevel) override
        {
        }
        void OnDrawingCompleted() override {}
        void OnAppRemoveStartingWindow() override {}
    };
    std::shared_ptr<TLifecycleListener> lifecycleListener_ = std::make_shared<TLifecycleListener>();
};

void WindowSessionLifecycleTest::SetUpTestCase() {}

void WindowSessionLifecycleTest::TearDownTestCase() {}

void WindowSessionLifecycleTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    session_ = sptr<Session>::MakeSptr(info);
    session_->surfaceNode_ = CreateRSSurfaceNode();
    EXPECT_NE(nullptr, session_);
    ssm_ = sptr<SceneSessionManager>::MakeSptr();
    session_->SetEventHandler(ssm_->taskScheduler_->GetEventHandler(), ssm_->eventHandler_);
    auto isScreenLockedCallback = [this]() { return ssm_->IsScreenLocked(); };
    session_->RegisterIsScreenLockedCallback(isScreenLockedCallback);
}

void WindowSessionLifecycleTest::TearDown()
{
    session_ = nullptr;
    usleep(WAIT_SYNC_IN_NS);
}

RSSurfaceNode::SharedPtr WindowSessionLifecycleTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    if (surfaceNode == nullptr) {
        GTEST_LOG_(INFO) << "WindowSessionTest::CreateRSSurfaceNode surfaceNode is nullptr";
    }
    return surfaceNode;
}

int32_t WindowSessionLifecycleTest::GetTaskCount()
{
    std::string dumpInfo = session_->handler_->GetEventRunner()->GetEventQueue()->DumpCurrentQueueSize();
    std::regex pattern("\\d+");
    std::smatch matches;
    int32_t taskNum = 0;
    while (std::regex_search(dumpInfo, matches, pattern)) {
        taskNum += std::stoi(matches.str());
        dumpInfo = matches.suffix();
    }
    return taskNum;
}

namespace {

/**
 * @tc.name: Connect01
 * @tc.desc: check func Connect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, Connect01, TestSize.Level1)
{
    auto surfaceNode = CreateRSSurfaceNode();
    session_->state_ = SessionState::STATE_CONNECT;
    SystemSessionConfig systemConfig;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    auto result = session_->Connect(nullptr, nullptr, nullptr, systemConfig, property);
    ASSERT_EQ(result, WSError::WS_OK);

    session_->state_ = SessionState::STATE_DISCONNECT;
    result = session_->Connect(nullptr, nullptr, nullptr, systemConfig, property);
    ASSERT_EQ(result, WSError::WS_OK);

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    result = session_->Connect(mockSessionStage, nullptr, surfaceNode, systemConfig, property);
    ASSERT_EQ(result, WSError::WS_OK);

    sptr<TestWindowEventChannel> testWindowEventChannel = sptr<TestWindowEventChannel>::MakeSptr();
    EXPECT_NE(nullptr, testWindowEventChannel);
    result = session_->Connect(mockSessionStage, testWindowEventChannel, surfaceNode, systemConfig, property);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Reconnect01
 * @tc.desc: check func Reconnect01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, Reconnect01, TestSize.Level1)
{
    auto surfaceNode = CreateRSSurfaceNode();

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    auto result = session_->Reconnect(nullptr, nullptr, nullptr, property);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    result = session_->Reconnect(mockSessionStage, nullptr, surfaceNode, property);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    sptr<TestWindowEventChannel> testWindowEventChannel = sptr<TestWindowEventChannel>::MakeSptr();
    EXPECT_NE(nullptr, testWindowEventChannel);
    result = session_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, nullptr);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = session_->Reconnect(nullptr, testWindowEventChannel, surfaceNode, property);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = session_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Foreground01
 * @tc.desc: check func Foreground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, Foreground01, TestSize.Level1)
{
    session_->state_ = SessionState::STATE_DISCONNECT;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    auto result = session_->Foreground(property);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    session_->state_ = SessionState::STATE_CONNECT;
    session_->isActive_ = true;
    result = session_->Foreground(property);
    ASSERT_EQ(result, WSError::WS_OK);

    session_->isActive_ = false;
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Foreground02
 * @tc.desc: Foreground Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, Foreground02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    session_->SetSessionState(SessionState::STATE_BACKGROUND);
    session_->isActive_ = true;
    auto result = session_->Foreground(property);
    ASSERT_EQ(result, WSError::WS_OK);

    session_->SetSessionState(SessionState::STATE_INACTIVE);
    session_->isActive_ = false;
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    session_->sessionStage_ = mockSessionStage;
    result = session_->Foreground(property);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Foreground03
 * @tc.desc: Foreground Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, Foreground03, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    session_->SetSessionState(SessionState::STATE_BACKGROUND);
    session_->isActive_ = true;

    property->type_ = WindowType::WINDOW_TYPE_DIALOG;
    auto result = session_->Foreground(property);
    ASSERT_EQ(result, WSError::WS_OK);

    SessionInfo parentInfo;
    parentInfo.abilityName_ = "testSession1";
    parentInfo.moduleName_ = "testSession2";
    parentInfo.bundleName_ = "testSession3";
    sptr<Session> parentSession = sptr<Session>::MakeSptr(parentInfo);
    ASSERT_NE(parentSession, nullptr);
    session_->SetParentSession(parentSession);
    session_->SetSessionState(SessionState::STATE_INACTIVE);
    result = session_->Foreground(property);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Background01
 * @tc.desc: check func Background
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, Background01, TestSize.Level1)
{
    session_->state_ = SessionState::STATE_CONNECT;
    auto result = session_->Background();
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    session_->state_ = SessionState::STATE_INACTIVE;
    result = session_->Background();
    ASSERT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(session_->state_, SessionState::STATE_BACKGROUND);
}

/**
 * @tc.name: Background2
 * @tc.desc: Background2 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, Background2, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_ACTIVE);
    auto result = session_->Background();
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Background03
 * @tc.desc: Background03 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, Background03, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_ACTIVE);
    session_->property_ = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(session_->property_, nullptr);
    session_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    auto result = session_->Background();
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: Disconnect01
 * @tc.desc: check func Disconnect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, Disconnect01, TestSize.Level1)
{
    session_->state_ = SessionState::STATE_CONNECT;
    auto result = session_->Disconnect();
    ASSERT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(session_->state_, SessionState::STATE_DISCONNECT);

    session_->state_ = SessionState::STATE_BACKGROUND;
    result = session_->Disconnect();
    ASSERT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(session_->state_, SessionState::STATE_DISCONNECT);
}

/**
 * @tc.name: TerminateSessionNew01
 * @tc.desc: check func TerminateSessionNew
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, TerminateSessionNew01, TestSize.Level1)
{
    NotifyTerminateSessionFuncNew callback =
        [](const SessionInfo& info, bool needStartCaller, bool isFromBroker, bool isForceClean) {};
    bool needStartCaller = false;
    bool isFromBroker = false;
    sptr<AAFwk::SessionInfo> info = sptr<AAFwk::SessionInfo>::MakeSptr();
    session_->terminateSessionFuncNew_ = nullptr;
    session_->TerminateSessionNew(info, needStartCaller, isFromBroker);

    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, session_->TerminateSessionNew(nullptr, needStartCaller, isFromBroker));
}

/**
 * @tc.name: TerminateSessionNew02
 * @tc.desc: terminateSessionFuncNew_ is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, TerminateSessionNew02, TestSize.Level1)
{
    bool needStartCaller = true;
    bool isFromBroker = true;
    sptr<AAFwk::SessionInfo> info = sptr<AAFwk::SessionInfo>::MakeSptr();
    session_->SetTerminateSessionListenerNew(
        [](const SessionInfo& info, bool needStartCaller, bool isFromBroker, bool isForceClean) {});
    usleep(WAIT_SYNC_IN_NS);
    auto result = session_->TerminateSessionNew(info, needStartCaller, isFromBroker);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: NotifyDestroy
 * @tc.desc: check func NotifyDestroy
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, NotifyDestroy, TestSize.Level1)
{
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    session_->sessionStage_ = mockSessionStage;
    EXPECT_CALL(*(mockSessionStage), NotifyDestroy()).Times(1).WillOnce(Return(WSError::WS_OK));
    ASSERT_EQ(WSError::WS_OK, session_->NotifyDestroy());
    session_->sessionStage_ = nullptr;
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->NotifyDestroy());
}

/**
 * @tc.name: IsActive01
 * @tc.desc: IsActive, normal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, IsActive, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->isActive_ = false;
    if (!session_->IsActive()) {
        ASSERT_EQ(false, session_->IsActive());
    }
}

/**
 * @tc.name: IsActive43
 * @tc.desc: IsActive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, IsActive43, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool res = session_->IsActive();
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: IsLifecycleForeground01
 * @tc.desc: IsLifecycleForeground, normal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, IsLifecycleForeground, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_FOREGROUND;
    ASSERT_EQ(true, session_->IsLifecycleForeground());
    session_->state_ = SessionState::STATE_ACTIVE;
    ASSERT_EQ(true, session_->IsLifecycleForeground());
    session_->state_ = SessionState::STATE_INACTIVE;
    ASSERT_EQ(false, session_->IsLifecycleForeground());
    session_->state_ = SessionState::STATE_BACKGROUND;
    ASSERT_EQ(false, session_->IsLifecycleForeground());
    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_EQ(false, session_->IsLifecycleForeground());
    session_->state_ = SessionState::STATE_CONNECT;
    ASSERT_EQ(false, session_->IsLifecycleForeground());
}

/**
 * @tc.name: IsSessionNotBackground
 * @tc.desc: IsSessionNotBackground, normal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, IsSessionNotBackground, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    EXPECT_EQ(true, session_->IsSessionNotBackground());
    session_->state_ = SessionState::STATE_CONNECT;
    EXPECT_EQ(true, session_->IsSessionNotBackground());
    session_->state_ = SessionState::STATE_FOREGROUND;
    EXPECT_EQ(true, session_->IsSessionNotBackground());
    session_->state_ = SessionState::STATE_ACTIVE;
    EXPECT_EQ(true, session_->IsSessionNotBackground());
    session_->state_ = SessionState::STATE_INACTIVE;
    EXPECT_EQ(false, session_->IsSessionNotBackground());
    session_->state_ = SessionState::STATE_BACKGROUND;
    EXPECT_EQ(false, session_->IsSessionNotBackground());
}

/**
 * @tc.name: IsVisibleNotBackground
 * @tc.desc: IsVisibleNotBackground, normal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, IsVisibleNotBackground, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->isVisible_ = false;
    session_->state_ = SessionState::STATE_ACTIVE;
    EXPECT_EQ(false, session_->IsVisibleNotBackground());
    session_->isVisible_ = true;
    EXPECT_EQ(true, session_->IsVisibleNotBackground());
    session_->state_ = SessionState::STATE_BACKGROUND;
    EXPECT_EQ(false, session_->IsVisibleNotBackground());
}

/**
 * @tc.name: IsVisibleNotBackground01
 * @tc.desc: IsVisibleNotBackground, normal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, IsVisibleNotBackground01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsVisibleNotBackground";
    info.moduleName_ = "IsVisibleNotBackground";
    info.bundleName_ = "IsVisibleNotBackground";
    sptr<Session> subSession = sptr<Session>::MakeSptr(info);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    EXPECT_EQ(subSession->IsVisibleNotBackground(), false);
    subSession->SetSessionState(SessionState::STATE_FOREGROUND);
    EXPECT_EQ(subSession->IsVisibleNotBackground(), false);
    subSession->isVisible_ = true;
    EXPECT_EQ(subSession->IsVisibleNotBackground(), true);

    sptr<Session> parentSession = sptr<Session>::MakeSptr(info);
    parentSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    subSession->SetParentSession(parentSession);
    EXPECT_EQ(subSession->IsVisibleNotBackground(), false);
    parentSession->SetSessionState(SessionState::STATE_FOREGROUND);
    EXPECT_EQ(subSession->IsVisibleNotBackground(), false);
    parentSession->isVisible_ = true;
    EXPECT_EQ(subSession->IsVisibleNotBackground(), true);

    subSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subSession->property_->SetZLevelAboveParentLoosened(true);
    subSession->isVisible_ = true;
    EXPECT_EQ(subSession->IsVisibleNotBackground(), true);
}

/**
 * @tc.name: NotifyActivation
 * @tc.desc: NotifyActivation Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, NotifyActivation, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->NotifyActivation();

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: NotifyActivation022
 * @tc.desc: NotifyActivation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, NotifyActivation022, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->NotifyActivation();

    session_->RegisterLifecycleListener(lifecycleListener_);
    session_->NotifyActivation();
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    session_->UnregisterLifecycleListener(lifecycleListener_);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: NotifyForeground
 * @tc.desc: NotifyForeground Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, NotifyForeground, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->NotifyForeground();

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: NotifyForeground024
 * @tc.desc: NotifyForeground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, NotifyForeground024, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->NotifyForeground();

    session_->RegisterLifecycleListener(lifecycleListener_);
    session_->NotifyForeground();
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    session_->UnregisterLifecycleListener(lifecycleListener_);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: NotifyBackground
 * @tc.desc: NotifyBackground Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, NotifyBackground, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->NotifyBackground();

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: NotifyBackground025
 * @tc.desc: NotifyBackground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, NotifyBackground025, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->NotifyBackground();

    session_->RegisterLifecycleListener(lifecycleListener_);
    session_->NotifyBackground();
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    session_->UnregisterLifecycleListener(lifecycleListener_);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: TerminateSessionTotal01
 * @tc.desc: abilitySessionInfo is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, TerminateSessionTotal01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION,
              session_->TerminateSessionTotal(nullptr, TerminateType::CLOSE_AND_KEEP_MULTITASK));
}

/**
 * @tc.name: TerminateSessionTotal02
 * @tc.desc: abilitySessionInfo is not nullptr, isTerminating is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, TerminateSessionTotal02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    session_->isTerminating_ = true;
    ASSERT_EQ(WSError::WS_ERROR_INVALID_OPERATION,
              session_->TerminateSessionTotal(abilitySessionInfo, TerminateType::CLOSE_AND_KEEP_MULTITASK));
}

/**
 * @tc.name: TerminateSessionTotal03
 * @tc.desc: abilitySessionInfo is not nullptr, isTerminating is false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, TerminateSessionTotal03, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    session_->isTerminating_ = false;
    session_->SetTerminateSessionListenerTotal(nullptr);
    ASSERT_EQ(WSError::WS_OK,
              session_->TerminateSessionTotal(abilitySessionInfo, TerminateType::CLOSE_AND_KEEP_MULTITASK));
}

/**
 * @tc.name: PendingSessionToBackgroundForDelegator
 * @tc.desc: PendingSessionToBackgroundForDelegator Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, PendingSessionToBackgroundForDelegator, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetPendingSessionToBackgroundForDelegatorListener(nullptr);
    ASSERT_EQ(WSError::WS_OK, session_->PendingSessionToBackgroundForDelegator(true));
}

/**
 * @tc.name: NotifyConnect023
 * @tc.desc: NotifyConnect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, NotifyConnect023, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->NotifyConnect();

    session_->RegisterLifecycleListener(lifecycleListener_);
    session_->NotifyConnect();
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    session_->UnregisterLifecycleListener(lifecycleListener_);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: NotifyDisconnect026
 * @tc.desc: NotifyDisconnect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, NotifyDisconnect026, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->NotifyDisconnect();

    session_->RegisterLifecycleListener(lifecycleListener_);
    session_->NotifyDisconnect();
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    session_->UnregisterLifecycleListener(lifecycleListener_);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: UpdateSessionState32
 * @tc.desc: UpdateSessionState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, UpdateSessionState32, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    SessionState state = SessionState::STATE_CONNECT;
    session_->UpdateSessionState(state);
    ASSERT_EQ(session_->state_, SessionState::STATE_CONNECT);
}

/**
 * @tc.name: IsSystemSession44
 * @tc.desc: IsSystemSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, IsSystemSession44, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool res = session_->IsSystemSession();
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: Hide45
 * @tc.desc: Hide
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, Hide45, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    auto result = session_->Hide();
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Show46
 * @tc.desc: Show
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, Show46, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    auto result = session_->Show(property);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: IsSystemActive47
 * @tc.desc: IsSystemActive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, IsSystemActive47, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool res = session_->IsSystemActive();
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: IsTerminated49
 * @tc.desc: IsTerminated
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionLifecycleTest, IsTerminated49, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    bool res = session_->IsTerminated();
    ASSERT_EQ(true, res);
    session_->state_ = SessionState::STATE_FOREGROUND;
    res = session_->IsTerminated();
    ASSERT_EQ(false, res);
    session_->state_ = SessionState::STATE_ACTIVE;
    res = session_->IsTerminated();
    ASSERT_EQ(false, res);
    session_->state_ = SessionState::STATE_INACTIVE;
    res = session_->IsTerminated();
    ASSERT_EQ(false, res);
    session_->state_ = SessionState::STATE_BACKGROUND;
    res = session_->IsTerminated();
    ASSERT_EQ(false, res);
    session_->state_ = SessionState::STATE_CONNECT;
    res = session_->IsTerminated();
    ASSERT_EQ(false, res);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
