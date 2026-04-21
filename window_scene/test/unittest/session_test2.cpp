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
#include "process_options.h"
#include "key_event.h"
#include "wm_common.h"
#include "window_manager_hilog.h"
#include "accessibility_event_info.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string UNDEFINED = "undefined";
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg += msg;
    }
}

class WindowSessionTest2 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

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

    sptr<SessionStageMocker> mockSessionStage_ = nullptr;
    sptr<WindowEventChannelMocker> mockEventChannel_ = nullptr;
};

class SurfaceNodeChangedSession : public Session {
public:
    explicit SurfaceNodeChangedSession(const SessionInfo& info) : Session(info) {}
    bool isOnSurfaceNodeChangedCalled_ = false;

protected:
    void OnSurfaceNodeChanged() override
    {
        isOnSurfaceNodeChangedCalled_ = true;
    }
};

void WindowSessionTest2::SetUpTestCase() {}

void WindowSessionTest2::TearDownTestCase() {}

void WindowSessionTest2::SetUp()
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

    mockSessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage_, nullptr);

    mockEventChannel_ = sptr<WindowEventChannelMocker>::MakeSptr(mockSessionStage_);
    ASSERT_NE(mockEventChannel_, nullptr);
}

void WindowSessionTest2::TearDown()
{
    session_ = nullptr;
    usleep(WAIT_SYNC_IN_NS);
}

RSSurfaceNode::SharedPtr WindowSessionTest2::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTest2SurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    if (surfaceNode == nullptr) {
        GTEST_LOG_(INFO) << "WindowSessionTest2::CreateRSSurfaceNode surfaceNode is nullptr";
    }
    return surfaceNode;
}

namespace {
/**
 * @tc.name: SetParentSession
 * @tc.desc: SetParentSession Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetParentSession, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session_->SetParentSession(session);

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: TransferPointerEvent01
 * @tc.desc: !IsSystemSession() && !IsSessionValid() is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferPointerEvent01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = false;
    session_->state_ = SessionState::STATE_DISCONNECT;

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, session_->TransferPointerEvent(pointerEvent));
}

/**
 * @tc.name: TransferPointerEvent02
 * @tc.desc: pointerEvent is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferPointerEvent02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->sessionInfo_.isSystem_ = true;

    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferPointerEvent(pointerEvent));
}

/**
 * @tc.name: TransferPointerEvent03
 * @tc.desc: WindowType is WINDOW_TYPE_APP_MAIN_WINDOW, CheckDialogOnForeground() is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferPointerEvent03, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo info;
    info.abilityName_ = "dialogAbilityName";
    info.moduleName_ = "dialogModuleName";
    info.bundleName_ = "dialogBundleName";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info);
    ASSERT_NE(dialogSession, nullptr);
    dialogSession->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(dialogSession);

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferPointerEvent(pointerEvent));
}

/**
 * @tc.name: TransferPointerEvent04
 * @tc.desc: parentSession_ && parentSession_->CheckDialogOnForeground() is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferPointerEvent04, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    SessionInfo info;
    info.abilityName_ = "dialogAbilityName";
    info.moduleName_ = "dialogModuleName";
    info.bundleName_ = "dialogBundleName";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info);
    ASSERT_NE(dialogSession, nullptr);
    dialogSession->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(dialogSession);
    session_->parentSession_ = session_;

    ASSERT_EQ(WSError::WS_ERROR_INVALID_PERMISSION, session_->TransferPointerEvent(pointerEvent));
}

/**
 * @tc.name: TransferPointerEvent05
 * @tc.desc: windowEventChannel_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferPointerEvent05, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_SCENE_BOARD);

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferPointerEvent(pointerEvent));
}

/**
 * @tc.name: TransferPointerEvent06
 * @tc.desc: windowEventChannel_ is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferPointerEvent06, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_SCENE_BOARD);
    session_->windowEventChannel_ = mockEventChannel_;

    auto needNotifyClient = true;
    session_->TransferPointerEvent(pointerEvent, needNotifyClient);

    needNotifyClient = false;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    ASSERT_EQ(WSError::WS_OK, session_->TransferPointerEvent(pointerEvent, needNotifyClient));

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_PULL_MOVE);
    ASSERT_EQ(WSError::WS_OK, session_->TransferPointerEvent(pointerEvent, needNotifyClient));

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW);
    ASSERT_EQ(WSError::WS_OK, session_->TransferPointerEvent(pointerEvent, needNotifyClient));

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_LEAVE_WINDOW);
    ASSERT_EQ(WSError::WS_OK, session_->TransferPointerEvent(pointerEvent, needNotifyClient));

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_PULL_IN_WINDOW);
    ASSERT_EQ(WSError::WS_OK, session_->TransferPointerEvent(pointerEvent, needNotifyClient));

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_PULL_OUT_WINDOW);
    ASSERT_EQ(WSError::WS_OK, session_->TransferPointerEvent(pointerEvent, needNotifyClient));
}

/**
 * @tc.name: TransferPointerEvent07
 * @tc.desc: isExecuteDelayRaise is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferPointerEvent07, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->sessionInfo_.isSystem_ = true;
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session_->windowEventChannel_ = mockEventChannel_;

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);
    session_->UpdateFocus(false);
    session_->property_->SetFocusable(true);
    bool needNotifyClient = true;
    bool isExecuteDelayRaise = true;
    ASSERT_EQ(WSError::WS_OK, session_->TransferPointerEvent(pointerEvent, needNotifyClient, isExecuteDelayRaise));
}

/**
 * @tc.name: TransferKeyEvent01
 * @tc.desc: !IsSystemSession() && !IsSessionValid() is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferKeyEvent01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = false;
    session_->state_ = SessionState::STATE_DISCONNECT;

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferKeyEvent(keyEvent));

    session_->windowEventChannel_ = mockEventChannel_;
    ASSERT_EQ(WSError::WS_OK, session_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: TransferKeyEvent02
 * @tc.desc: keyEvent is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferKeyEvent02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: TransferKeyEvent03
 * @tc.desc: WindowType is WINDOW_TYPE_APP_MAIN_WINDOW, CheckDialogOnForeground() is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferKeyEvent03, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo info;
    info.abilityName_ = "dialogAbilityName";
    info.moduleName_ = "dialogModuleName";
    info.bundleName_ = "dialogBundleName";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info);
    ASSERT_NE(dialogSession, nullptr);
    dialogSession->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(dialogSession);

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: TransferKeyEvent04
 * @tc.desc: parentSession_ && parentSession_->CheckDialogOnForeground() is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferKeyEvent04, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    SessionInfo info;
    info.abilityName_ = "dialogAbilityName";
    info.moduleName_ = "dialogModuleName";
    info.bundleName_ = "dialogBundleName";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info);
    ASSERT_NE(dialogSession, nullptr);
    dialogSession->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(dialogSession);
    session_->parentSession_ = session_;

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: TransferKeyEvent05
 * @tc.desc: windowEventChannel_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferKeyEvent05, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_SCENE_BOARD);

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: TransferBackPressedEventForConsumed01
 * @tc.desc: windowEventChannel_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferBackPressedEventForConsumed01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->windowEventChannel_ = nullptr;

    bool isConsumed = false;
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferBackPressedEventForConsumed(isConsumed));
}

/**
 * @tc.name: TransferKeyEventForConsumed01
 * @tc.desc: windowEventChannel_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferKeyEventForConsumed01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->windowEventChannel_ = nullptr;

    auto keyEvent = MMI::KeyEvent::Create();
    bool isConsumed = false;
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferKeyEventForConsumed(keyEvent, isConsumed));
}

/**
 * @tc.name: TransferFocusActiveEvent01
 * @tc.desc: windowEventChannel_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferFocusActiveEvent01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->windowEventChannel_ = nullptr;

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferFocusActiveEvent(false));
}

/**
 * @tc.name: TransferFocusStateEvent01
 * @tc.desc: windowEventChannel_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferFocusStateEvent01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->windowEventChannel_ = nullptr;

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferFocusStateEvent(false));
}

/**
 * @tc.name: Snapshot01
 * @tc.desc: ret is false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, Snapshot01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->surfaceNode_ = nullptr;

    ASSERT_EQ(nullptr, session_->Snapshot());
}

/**
 * @tc.name: SetSessionStateChangeListenser
 * @tc.desc: SetSessionStateChangeListenser Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionStateChangeListenser, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    NotifySessionStateChangeFunc func = nullptr;
    session_->SetSessionStateChangeListenser(func);

    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: SetClearSubSessionCallback
 * @tc.desc: SetClearSubSessionCallback Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetClearSubSessionCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetClearSubSessionCallback";
    info.moduleName_ = "SetClearSubSessionCallback";
    info.bundleName_ = "SetClearSubSessionCallback";
    sptr<Session> session = sptr<Session>::MakeSptr(info);

    NotifyClearSubSessionFunc func = nullptr;
    session->SetClearSubSessionCallback(func);
    EXPECT_TRUE(session->clearSubSessionFunc_ == nullptr);

    func = [](const int32_t subPersistentId) {
    };
    session->SetClearSubSessionCallback(func);
    EXPECT_FALSE(session->clearSubSessionFunc_ == nullptr);
}

/**
 * @tc.name: NotifySessionStateChange
 * @tc.desc: NotifySessionStateChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifySessionStateChange, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo info;
    info.abilityName_ = "NotifySessionStateChange";
    info.moduleName_ = "NotifySessionStateChange";
    info.bundleName_ = "NotifySessionStateChange";
    sptr<Session> session = sptr<Session>::MakeSptr(info);

    session->NotifySessionStateChange(SessionState::STATE_ACTIVE);
    EXPECT_TRUE(g_logMsg.find("notify clear subSession") == std::string::npos);

    info.abilityName_ = "parentSession";
    info.moduleName_ = "parentSession";
    info.bundleName_ = "parentSession";
    sptr<Session> parentSession = sptr<Session>::MakeSptr(info);
    EXPECT_NE(parentSession, nullptr);

    session->SetParentSession(parentSession);
    session->NotifySessionStateChange(SessionState::STATE_ACTIVE);
    EXPECT_TRUE(g_logMsg.find("notify clear subSession") == std::string::npos);

    parentSession->clearSubSessionFunc_ = [](const int32_t subPersistentId) {
    };
    session->SetParentSession(parentSession);
    session->NotifySessionStateChange(SessionState::STATE_ACTIVE);
    EXPECT_TRUE(g_logMsg.find("notify clear subSession") == std::string::npos);

    session->NotifySessionStateChange(SessionState::STATE_DISCONNECT);
    EXPECT_TRUE(g_logMsg.find("notify clear subSession") != std::string::npos);
}

/**
 * @tc.name: SetSessionFocusableChangeListener
 * @tc.desc: SetSessionFocusableChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionFocusableChangeListener, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    NotifySessionFocusableChangeFunc func = [](const bool isFocusable) {};
    session_->SetSessionFocusableChangeListener(func);

    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: SetSessionTouchableChangeListener
 * @tc.desc: SetSessionTouchableChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionTouchableChangeListener, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    NotifySessionTouchableChangeFunc func = [](const bool touchable) {};
    session_->SetSessionTouchableChangeListener(func);

    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: SetSessionInfoLockedStateChangeListener
 * @tc.desc: SetSessionInfoLockedStateChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoLockedStateChangeListener, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    NotifySessionTouchableChangeFunc func = [](const bool lockedState) {};
    session_->SetSessionInfoLockedStateChangeListener(func);

    session_->SetSessionInfoLockedState(true);
    ASSERT_EQ(true, session_->sessionInfo_.lockedState);
}

/**
 * @tc.name: SetClickListener
 * @tc.desc: SetClickListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetClickListener, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    NotifyClickFunc func = nullptr;
    session_->SetClickListener(func);

    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: UpdateFocus01
 * @tc.desc: isFocused_ equal isFocused
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, UpdateFocus01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    bool isFocused = session_->isFocused_;
    ASSERT_EQ(WSError::WS_DO_NOTHING, session_->UpdateFocus(isFocused));
}

/**
 * @tc.name: UpdateFocus02
 * @tc.desc: isFocused_ not equal isFocused, IsSessionValid() return false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, UpdateFocus02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    bool isFocused = session_->isFocused_;
    ASSERT_EQ(WSError::WS_OK, session_->UpdateFocus(!isFocused));
}

/**
 * @tc.name: UpdateWindowMode01
 * @tc.desc: IsSessionValid() return false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, UpdateWindowMode01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    ASSERT_EQ(WSError::WS_OK, session_->UpdateWindowMode(WindowMode::WINDOW_MODE_UNDEFINED));
}

/**
 * @tc.name: NotifyForegroundInteractiveStatus
 * @tc.desc: NotifyForegroundInteractiveStatus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifyForegroundInteractiveStatus, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 123;
    session_->sessionStage_ = nullptr;
    bool interactive = true;
    session_->NotifyForegroundInteractiveStatus(interactive);

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    session_->sessionStage_ = mockSessionStage;
    session_->state_ = SessionState::STATE_FOREGROUND;
    session_->persistentId_ = persistentId;
    interactive = false;
    session_->NotifyForegroundInteractiveStatus(interactive);
    EXPECT_TRUE(g_logMsg.find("id:123") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetEventHandler001
 * @tc.desc: SetEventHandler Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetEventHandler001, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    std::shared_ptr<AppExecFwk::EventHandler> handler = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> exportHandler = nullptr;
    session_->SetEventHandler(handler, exportHandler);
    EXPECT_EQ(nullptr, session_->handler_);
    EXPECT_EQ(nullptr, session_->exportHandler_);
}

/**
 * @tc.name: PostTask002
 * @tc.desc: PostTask Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, PostTask002, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 0;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetPersistentId(persistentId);
    int32_t res = session_->GetPersistentId();
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: GetSurfaceNode
 * @tc.desc: GetSurfaceNode Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetSurfaceNode, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->surfaceNode_ = nullptr;
    std::shared_ptr<RSSurfaceNode> res = session_->GetSurfaceNode();
    ASSERT_EQ(res, nullptr);
}

/**
 * @tc.name: GetSurfaceNode
 * @tc.desc: GetSurfaceNode Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetSurfaceNode02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->surfaceNode_ = nullptr;
    std::shared_ptr<RSSurfaceNode> res = session_->GetSurfaceNode(false);
    ASSERT_EQ(res, nullptr);
    res = session_->GetSurfaceNode(true);
    ASSERT_EQ(res, nullptr);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session_->surfaceNode_ = surfaceNode;
    res = session_->GetSurfaceNode(false);
    ASSERT_EQ(res, surfaceNode);
    res = session_->GetSurfaceNode(true);
    ASSERT_EQ(res, surfaceNode);
    session_->surfaceNode_ = nullptr;
}

/**
 * @tc.name: GetLeashWinSurfaceNode
 * @tc.desc: GetLeashWinSurfaceNode Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetLeashWinSurfaceNode, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->leashWinSurfaceNode_ = nullptr;
    std::shared_ptr<RSSurfaceNode> res = session_->GetLeashWinSurfaceNode();
    ASSERT_EQ(res, nullptr);
}

/**
 * @tc.name: SetAndGetShadowSurfaceNode
 * @tc.desc: SetAndGetShadowSurfaceNode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetAndGetShadowSurfaceNode, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSurfaceNode(nullptr);
    EXPECT_EQ(session_->GetSurfaceNode(), nullptr);
    EXPECT_EQ(session_->GetShadowSurfaceNode(), nullptr);

    std::shared_ptr<RSSurfaceNode> surfaceNode = WindowSessionTest2::CreateRSSurfaceNode();
    EXPECT_NE(nullptr, surfaceNode);
    session_->SetSurfaceNode(surfaceNode);
    EXPECT_NE(session_->GetSurfaceNode(), nullptr);
    EXPECT_NE(session_->GetShadowSurfaceNode(), nullptr);
}

/**
 * @tc.name: SetSurfaceNodeTriggerOnSurfaceNodeChanged
 * @tc.desc: SetSurfaceNode should trigger OnSurfaceNodeChanged callback
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSurfaceNodeTriggerOnSurfaceNodeChanged, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSurfaceNodeTriggerOnSurfaceNodeChanged";
    info.bundleName_ = "SetSurfaceNodeTriggerOnSurfaceNodeChanged";
    sptr<SurfaceNodeChangedSession> session = sptr<SurfaceNodeChangedSession>::MakeSptr(info);
    ASSERT_NE(session, nullptr);

    std::shared_ptr<RSSurfaceNode> surfaceNode = WindowSessionTest2::CreateRSSurfaceNode();
    ASSERT_NE(surfaceNode, nullptr);
    session->SetSurfaceNode(surfaceNode);
    EXPECT_TRUE(session->isOnSurfaceNodeChangedCalled_);
}

/**
 * @tc.name: SetAndGetLeashWinShadowSurfaceNode
 * @tc.desc: SetAndGetLeashWinShadowSurfaceNode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetAndGetLeashWinShadowSurfaceNode, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetLeashWinSurfaceNode(nullptr);
    EXPECT_EQ(session_->GetLeashWinSurfaceNode(), nullptr);
    EXPECT_EQ(session_->GetLeashWinShadowSurfaceNode(), nullptr);

    std::shared_ptr<RSSurfaceNode> leashWinSurfaceNode = WindowSessionTest2::CreateRSSurfaceNode();
    EXPECT_NE(nullptr, leashWinSurfaceNode);
    session_->SetLeashWinSurfaceNode(leashWinSurfaceNode);
    EXPECT_NE(session_->GetLeashWinSurfaceNode(), nullptr);
    EXPECT_NE(session_->GetLeashWinShadowSurfaceNode(), nullptr);
}

/**
 * @tc.name: SetSessionInfoAncoSceneState
 * @tc.desc: SetSessionInfoAncoSceneState Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoAncoSceneState, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    int32_t ancoSceneState = 0;
    session_->SetSessionInfoAncoSceneState(ancoSceneState);
    EXPECT_EQ(0, session_->sessionInfo_.ancoSceneState);
}

/**
 * @tc.name: SetSessionInfoTime
 * @tc.desc: SetSessionInfoTime Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoTime, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    std::string time = "";
    session_->SetSessionInfoTime(time);
    EXPECT_EQ("", session_->sessionInfo_.time);
}

/**
 * @tc.name: SetSessionInfoAbilityInfo
 * @tc.desc: SetSessionInfoAbilityInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoAbilityInfo, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    session_->SetSessionInfoAbilityInfo(abilityInfo);
    EXPECT_EQ(nullptr, session_->sessionInfo_.abilityInfo);
}

/**
 * @tc.name: GetSessionInfoAbilityInfo
 * @tc.desc: GetSessionInfoAbilityInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetSessionInfoAbilityInfo, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->sessionInfo_.abilityInfo = nullptr;
    EXPECT_EQ(nullptr, session_->GetSessionInfoAbilityInfo());
}

/**
 * @tc.name: SetSessionInfoWant
 * @tc.desc: SetSessionInfoWant Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoWant, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    std::shared_ptr<AAFwk::Want> want = nullptr;
    session_->SetSessionInfoWant(want);
    EXPECT_EQ(nullptr, session_->sessionInfo_.want);
}

/**
 * @tc.name: SetSessionInfoPersistentId
 * @tc.desc: SetSessionInfoPersistentId Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoPersistentId, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 0;
    session_->SetSessionInfoPersistentId(persistentId);
    EXPECT_EQ(0, session_->sessionInfo_.persistentId_);
}

/**
 * @tc.name: SetSessionInfoCallerPersistentId
 * @tc.desc: SetSessionInfoCallerPersistentId Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoCallerPersistentId, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    int32_t callerPersistentId = 0;
    session_->SetSessionInfoCallerPersistentId(callerPersistentId);
    EXPECT_EQ(0, session_->sessionInfo_.callerPersistentId_);
}

/**
 * @tc.name: PostExportTask
 * @tc.desc: PostExportTask Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, PostExportTask, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 0;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetPersistentId(persistentId);
    int32_t ret = session_->GetPersistentId();
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: GetPersistentId
 * @tc.desc: GetPersistentId Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetPersistentId, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 0;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(nullptr, property);
    property->SetPersistentId(persistentId);
    int32_t ret = session_->GetPersistentId();
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: SetLeashWinSurfaceNode
 * @tc.desc: SetLeashWinSurfaceNode Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetLeashWinSurfaceNode, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    auto leashWinSurfaceNode = WindowSessionTest2::CreateRSSurfaceNode();
    session_->SetLeashWinSurfaceNode(leashWinSurfaceNode);
    ASSERT_EQ(session_->leashWinSurfaceNode_, leashWinSurfaceNode);
}

/**
 * @tc.name: SetSessionInfoContinueState
 * @tc.desc: SetSessionInfoContinueState Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoContinueState, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    enum ContinueState state = CONTINUESTATE_UNKNOWN;
    session_->SetSessionInfoContinueState(state);
    ASSERT_EQ(session_->sessionInfo_.continueState, state);
}

/**
 * @tc.name: SetSessionInfoIsClearSession01
 * @tc.desc: SetSessionInfoIsClearSession return false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoIsClearSession01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionInfoIsClearSession(false);
    ASSERT_EQ(false, session_->sessionInfo_.isClearSession);
}

/**
 * @tc.name: SetSessionInfoIsClearSession02
 * @tc.desc: SetSessionInfoIsClearSession return true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoIsClearSession02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionInfoIsClearSession(true);
    ASSERT_EQ(true, session_->sessionInfo_.isClearSession);
}

/**
 * @tc.name: SetSessionInfoAffinity
 * @tc.desc: SetSessionInfoAffinity
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoAffinity, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    std::string affinity = "setSessionIofoAffinity";
    session_->SetSessionInfoAffinity(affinity);
    ASSERT_EQ(affinity, session_->sessionInfo_.sessionAffinity);
}

/**
 * @tc.name: SetSessionInfo
 * @tc.desc: SetSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfo, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.want = nullptr;
    info.callerToken_ = nullptr;
    info.requestCode = 1;
    info.callerPersistentId_ = 1;
    info.callingTokenId_ = 1;
    info.uiAbilityId_ = 1;
    info.startSetting = nullptr;
    info.continueSessionId_ = "";
    std::shared_ptr<AAFwk::ProcessOptions> processOptions = std::make_shared<AAFwk::ProcessOptions>();
    info.processOptions = processOptions;
    session_->SetSessionInfo(info);
    ASSERT_EQ(nullptr, session_->sessionInfo_.want);
    ASSERT_EQ(nullptr, session_->sessionInfo_.callerToken_);
    ASSERT_EQ(1, session_->sessionInfo_.requestCode);
    ASSERT_EQ(1, session_->sessionInfo_.callerPersistentId_);
    ASSERT_EQ(1, session_->sessionInfo_.callingTokenId_);
    ASSERT_EQ(1, session_->sessionInfo_.uiAbilityId_);
    ASSERT_EQ("", session_->sessionInfo_.continueSessionId_);
    ASSERT_EQ(nullptr, session_->sessionInfo_.startSetting);
    ASSERT_EQ(processOptions, session_->sessionInfo_.processOptions);
    info.continueSessionId_ = "continueSessionId";
    session_->SetSessionInfo(info);
    ASSERT_EQ(session_->sessionInfo_.continueSessionId_, "continueSessionId");
}

/**
 * @tc.name: SetScreenId
 * @tc.desc: SetScreenId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetScreenId, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: RegisterLifecycleListener
 * @tc.desc: RegisterLifecycleListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, RegisterLifecycleListener, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    const std::shared_ptr<ILifecycleListener>& listener = nullptr;
    bool ret = session_->RegisterLifecycleListener(listener);
    ASSERT_EQ(false, ret);
}

/**
 * @tc.name: UnregisterLifecycleListener
 * @tc.desc: UnregisterLifecycleListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, UnregisterLifecycleListener, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    const std::shared_ptr<ILifecycleListener>& listener = nullptr;
    bool ret = session_->UnregisterLifecycleListener(listener);
    ASSERT_EQ(false, ret);
}

/**
 * @tc.name: NotifyActivation02
 * @tc.desc: NotifyActivation
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifyActivation02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->NotifyActivation();
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: NotifyConnect
 * @tc.desc: NotifyConnect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifyConnect, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->NotifyConnect();
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: NotifyForeground02
 * @tc.desc: NotifyForeground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifyForeground02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->NotifyForeground();
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: NotifyBackground02
 * @tc.desc: NotifyBackground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifyBackground02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->NotifyBackground();
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: NotifyDisconnect
 * @tc.desc: NotifyDisconnect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifyDisconnect, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->NotifyDisconnect();
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: NotifyExtensionDied02
 * @tc.desc: NotifyExtensionDied
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifyExtensionDied02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->NotifyExtensionDied();

    session_->RegisterLifecycleListener(lifecycleListener_);
    session_->NotifyExtensionDied();
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    session_->UnregisterLifecycleListener(lifecycleListener_);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: NotifyTransferAccessibilityEvent
 * @tc.desc: NotifyTransferAccessibilityEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifyTransferAccessibilityEvent, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    OHOS::Accessibility::AccessibilityEventInfo info1;
    int64_t uiExtensionIdLevel = 6;
    session_->NotifyTransferAccessibilityEvent(info1, uiExtensionIdLevel);

    session_->RegisterLifecycleListener(lifecycleListener_);
    session_->NotifyTransferAccessibilityEvent(info1, uiExtensionIdLevel);
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    session_->UnregisterLifecycleListener(lifecycleListener_);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: GetAspectRatio
 * @tc.desc: GetAspectRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetAspectRatio, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    float ret = session_->GetAspectRatio();
    float res = 0.0f;
    ASSERT_EQ(ret, res);
}

/**
 * @tc.name: SetAspectRatio02
 * @tc.desc: SetAspectRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetAspectRatio02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    float radio = 2.0f;
    WSError ERR = session_->SetAspectRatio(radio);
    float ret = session_->GetAspectRatio();
    ASSERT_EQ(ret, radio);
    ASSERT_EQ(ERR, WSError::WS_OK);
}

/**
 * @tc.name: GetSessionState
 * @tc.desc: GetSessionState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetSessionState, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    SessionState state = session_->GetSessionState();
    ASSERT_EQ(state, session_->state_);
}

/**
 * @tc.name: SetSessionState02
 * @tc.desc: SetSessionState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionState02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    SessionState state = SessionState::STATE_CONNECT;
    session_->SetSessionState(state);
    ASSERT_EQ(state, session_->state_);
}

/**
 * @tc.name: SetChangeSessionVisibilityWithStatusBarEventListener
 * @tc.desc: SetChangeSessionVisibilityWithStatusBarEventListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetChangeSessionVisibilityWithStatusBarEventListener, TestSize.Level1)
{
    int resultValue = 0;
    session_->SetChangeSessionVisibilityWithStatusBarEventListener(
        [&resultValue](const SessionInfo& info, const bool visible) { resultValue = 1; });
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_NE(session_->changeSessionVisibilityWithStatusBarFunc_, nullptr);

    SessionInfo info;
    session_->changeSessionVisibilityWithStatusBarFunc_(info, true);
    ASSERT_EQ(resultValue, 1);

    session_->SetChangeSessionVisibilityWithStatusBarEventListener(
        [&resultValue](const SessionInfo& info, const bool visible) { resultValue = 2; });
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_NE(session_->changeSessionVisibilityWithStatusBarFunc_, nullptr);
    session_->changeSessionVisibilityWithStatusBarFunc_(info, true);
    ASSERT_EQ(resultValue, 2);
}

/**
 * @tc.name: UpdateSesionState
 * @tc.desc: UpdateSesionState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, UpdateSesionState, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    SessionState state = SessionState::STATE_CONNECT;
    session_->UpdateSessionState(state);
    ASSERT_EQ(session_->state_, SessionState::STATE_CONNECT);
}

/**
 * @tc.name: GetTouchable
 * @tc.desc: GetTouchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetTouchable, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool res = session_->GetTouchable();
    ASSERT_EQ(true, res);
}

/**
 * @tc.name: SetSystemTouchable
 * @tc.desc: SetSystemTouchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSystemTouchable, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool touchable = false;
    session_->SetSystemTouchable(touchable);
    ASSERT_EQ(session_->systemTouchable_, touchable);
}

/**
 * @tc.name: GetSystemTouchable
 * @tc.desc: GetSystemTouchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetSystemTouchable, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool res = session_->GetSystemTouchable();
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: SetRSVisible
 * @tc.desc: SetRSVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetVisible, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool isVisible = false;
    ASSERT_EQ(WSError::WS_OK, session_->SetRSVisible(isVisible));
}

/**
 * @tc.name: GetRSVisible02
 * @tc.desc: GetRSVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetRSVisible02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    if (!session_->GetRSVisible()) {
        ASSERT_EQ(false, session_->GetRSVisible());
    }
}

/**
 * @tc.name: SetVisibilityState
 * @tc.desc: SetVisibilityState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetVisibilityState, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    WindowVisibilityState state{ WINDOW_VISIBILITY_STATE_NO_OCCLUSION };
    ASSERT_EQ(WSError::WS_OK, session_->SetVisibilityState(state));
    ASSERT_EQ(state, session_->visibilityState_);
}

/**
 * @tc.name: GetVisibilityState
 * @tc.desc: GetVisibilityState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetVisibilityState, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    WindowVisibilityState state{ WINDOW_LAYER_STATE_MAX };
    ASSERT_EQ(state, session_->GetVisibilityState());
}

/**
 * @tc.name: SetDrawingContentState
 * @tc.desc: SetDrawingContentState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetDrawingContentState, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool isRSDrawing = false;
    ASSERT_EQ(WSError::WS_OK, session_->SetDrawingContentState(isRSDrawing));
    ASSERT_EQ(false, session_->isRSDrawing_);
}

/**
 * @tc.name: GetDrawingContentState
 * @tc.desc: GetDrawingContentState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetDrawingContentState, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool res = session_->GetDrawingContentState();
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: GetBrightness
 * @tc.desc: GetBrightness
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetBrightness, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_EQ(UNDEFINED_BRIGHTNESS, session_->GetBrightness());
}

/**
 * @tc.name: IsActive02
 * @tc.desc: IsActive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, IsActive02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool res = session_->IsActive();
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: Hide
 * @tc.desc: Hide
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, Hide, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    auto res = session_->Hide();
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: Show
 * @tc.desc: Show
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, Show, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    auto result = session_->Show(property);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: DrawingCompleted
 * @tc.desc: DrawingCompleled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, DrawingCompleted, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    auto result = session_->DrawingCompleted();
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: RemoveStartingWindow
 * @tc.desc: RemoveStartingWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, RemoveStartingWindow, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->RegisterLifecycleListener(lifecycleListener_);
    session_->RemoveStartingWindow();
    uint64_t screenId = 0;
    session_->SetScreenId(screenId);
    session_->UnregisterLifecycleListener(lifecycleListener_);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: IsSystemActive
 * @tc.desc: IsSystemActive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, IsSystemActive, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool res = session_->IsSystemActive();
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: SetSystemActive
 * @tc.desc: SetSystemActive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSystemActive48, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool systemActive = false;
    session_->SetSystemActive(systemActive);
    ASSERT_EQ(systemActive, session_->isSystemActive_);
}

/**
 * @tc.name: IsTerminated
 * @tc.desc: IsTerminated
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, IsTerminated, TestSize.Level1)
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

/**
 * @tc.name: SetAttachState01
 * @tc.desc: SetAttachState Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetAttachState01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetAttachState(false);
    ASSERT_EQ(session_->isAttach_, false);
}

/**
 * @tc.name: SetAttachState02
 * @tc.desc: SetAttachState Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetAttachState02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 123;
    sptr<PatternDetachCallbackMocker> detachCallback = sptr<PatternDetachCallbackMocker>::MakeSptr();
    EXPECT_CALL(*detachCallback, OnPatternDetach(persistentId)).Times(1);
    session_->persistentId_ = persistentId;
    session_->SetAttachState(true);
    session_->RegisterDetachCallback(detachCallback);
    session_->SetAttachState(false);
    usleep(WAIT_SYNC_IN_NS);
    Mock::VerifyAndClearExpectations(&detachCallback);
}

/**
 * @tc.name: SetAttachState03
 * @tc.desc: SetAttachState Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetAttachState03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 123;
    session_->persistentId_ = persistentId;

    session_->SetAttachState(true);
    EXPECT_TRUE(g_logMsg.find("NotifyWindowAttachStateChange, persistentId") == std::string::npos);
}

/**
 * @tc.name: RegisterDetachCallback01
 * @tc.desc: RegisterDetachCallback Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, RegisterDetachCallback01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    sptr<IPatternDetachCallback> detachCallback;
    session_->RegisterDetachCallback(detachCallback);
    ASSERT_EQ(session_->detachCallback_, detachCallback);
}

/**
 * @tc.name: RegisterDetachCallback02
 * @tc.desc: RegisterDetachCallback Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, RegisterDetachCallback02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    sptr<IPatternDetachCallback> detachCallback;
    session_->RegisterDetachCallback(detachCallback);
    ASSERT_EQ(session_->detachCallback_, detachCallback);
    sptr<IPatternDetachCallback> detachCallback2;
    session_->RegisterDetachCallback(detachCallback2);
    ASSERT_EQ(session_->detachCallback_, detachCallback2);
}

/**
 * @tc.name: SetContextTransparentFunc
 * @tc.desc: SetContextTransparentFunc Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetContextTransparentFunc, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetContextTransparentFunc(nullptr);
    ASSERT_EQ(session_->contextTransparentFunc_, nullptr);
    NotifyContextTransparentFunc func = []() {};
    session_->SetContextTransparentFunc(func);
    ASSERT_NE(session_->contextTransparentFunc_, nullptr);
}

/**
 * @tc.name: NeedCheckContextTransparent
 * @tc.desc: NeedCheckContextTransparent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NeedCheckContextTransparent, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetContextTransparentFunc(nullptr);
    ASSERT_EQ(session_->NeedCheckContextTransparent(), false);
    NotifyContextTransparentFunc func = []() {};
    session_->SetContextTransparentFunc(func);
    ASSERT_EQ(session_->NeedCheckContextTransparent(), true);
}

/**
 * @tc.name: GetBorderUnoccupied
 * @tc.desc: GetBorderUnoccupied Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetBorderUnoccupied, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool res = session_->GetBorderUnoccupied();
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: SetBorderUnoccupied
 * @tc.desc: SetBorderUnoccupied Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetBorderUnoccupied, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetBorderUnoccupied(true);
    bool res = session_->GetBorderUnoccupied();
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: TestEnsureMoveDragShadowSurfaceNodeSurface
 * @tc.desc: Verify EnsureMoveDragShadowSurfaceNode in different scenarios
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TestEnsureMoveDragShadowSurfaceNodeSurface, TestSize.Level1)
{
    // Case 1: surfaceNode_ and moveDragShadowSurfaceNode_ are not null
    {
        RSSurfaceNodeConfig config;
        session_->surfaceNode_ = RSSurfaceNode::Create(config);
        session_->moveDragShadowSurfaceNode_ = RSSurfaceNode::Create(config);

        auto result = session_->EnsureMoveDragShadowSurfaceNode();
        EXPECT_NE(result, nullptr);
    }

    // Case 2: surfaceNode_ is null, return nullptr
    {
        session_->surfaceNode_ = nullptr;
        session_->moveDragShadowSurfaceNode_ = nullptr;

        auto result = session_->EnsureMoveDragShadowSurfaceNode();
        EXPECT_EQ(result, nullptr);
    }
}

/**
 * @tc.name: TestEnsureMoveDragLeashWinShadowSurfaceNode
 * @tc.desc: Verify EnsureMoveDragLeashWinShadowSurfaceNode in different scenarios
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TestEnsureMoveDragLeashWinShadowSurfaceNode, TestSize.Level1)
{
    // Case 1: leashWinSurfaceNode_ and moveDragLeashWinShadowSurfaceNode_ are not null
    {
        RSSurfaceNodeConfig config;
        session_->leashWinSurfaceNode_ = RSSurfaceNode::Create(config);
        session_->moveDragLeashWinShadowSurfaceNode_ = RSSurfaceNode::Create(config);

        auto result = session_->EnsureMoveDragLeashWinShadowSurfaceNode();
        EXPECT_NE(result, nullptr);
    }

    // Case 2: leashWinSurfaceNode_ is null, return nullptr
    {
        session_->leashWinSurfaceNode_ = nullptr;
        session_->moveDragLeashWinShadowSurfaceNode_ = nullptr;

        auto result = session_->EnsureMoveDragLeashWinShadowSurfaceNode();
        EXPECT_EQ(result, nullptr);
    }
}

/**
 * @tc.name: TestGetMoveDragTargetSurfaceNode
 * @tc.desc: Verify GetMoveDragTargetSurfaceNode in different scenarios
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TestGetMoveDragTargetSurfaceNode, TestSize.Level1)
{
    // Case 1: leashWinSurfaceNode_ is not null, return leashWinSurfaceNode_
    {
        RSSurfaceNodeConfig config;
        session_->leashWinSurfaceNode_ = RSSurfaceNode::Create(config);

        auto result = session_->GetMoveDragTargetSurfaceNode();
        EXPECT_EQ(result, session_->leashWinSurfaceNode_);
    }

    // Case 2: leashWinSurfaceNode_ is null, return surfaceNode_
    {
        session_->leashWinSurfaceNode_ = nullptr;

        auto result = session_->GetMoveDragTargetSurfaceNode();
        EXPECT_EQ(result, session_->surfaceNode_);
    }
}

/**
 * @tc.name: TestGetMoveDragTargetShadowSurfaceNode
 * @tc.desc: Verify GetMoveDragTargetShadowSurfaceNode returns correct shadow node
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TestGetMoveDragTargetShadowSurfaceNode, TestSize.Level1)
{
    RSSurfaceNodeConfig config;

    // Case 1: leashWinShadowSurfaceNode exists, return it
    {
        session_->leashWinSurfaceNode_ = RSSurfaceNode::Create(config);
        session_->moveDragLeashWinShadowSurfaceNode_ = session_->leashWinSurfaceNode_;

        auto result = session_->GetMoveDragTargetShadowSurfaceNode();
        EXPECT_EQ(result, session_->moveDragLeashWinShadowSurfaceNode_);
    }

    // Case 2: leashWinShadowSurfaceNode is null, fallback to moveDragShadowSurfaceNode
    {
        session_->leashWinSurfaceNode_ = nullptr;
        session_->moveDragLeashWinShadowSurfaceNode_ = nullptr;

        session_->surfaceNode_ = RSSurfaceNode::Create(config);
        session_->moveDragShadowSurfaceNode_ = session_->surfaceNode_;

        auto result = session_->GetMoveDragTargetShadowSurfaceNode();
        EXPECT_EQ(result, session_->moveDragShadowSurfaceNode_);
    }
}
} // namespace
} // namespace Rosen
} // namespace OHOS
