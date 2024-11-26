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
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowSessionTest2"};
}

class WindowSessionTest2 : public testing::Test {
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
            int64_t uiExtensionIdLevel) override {}
        void OnDrawingCompleted() override {}
    };
    std::shared_ptr<TLifecycleListener> lifecycleListener_ = std::make_shared<TLifecycleListener>();

    sptr<SessionStageMocker> mockSessionStage_ = nullptr;
    sptr<WindowEventChannelMocker> mockEventChannel_ = nullptr;
};

void WindowSessionTest2::SetUpTestCase()
{
}

void WindowSessionTest2::TearDownTestCase()
{
}

void WindowSessionTest2::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    session_ = new (std::nothrow) Session(info);
    session_->surfaceNode_ = CreateRSSurfaceNode();
    EXPECT_NE(nullptr, session_);
    ssm_ = new SceneSessionManager();
    session_->SetEventHandler(ssm_->taskScheduler_->GetEventHandler(), ssm_->eventHandler_);
    auto isScreenLockedCallback = [this]() {
        return ssm_->IsScreenLocked();
    };
    session_->RegisterIsScreenLockedCallback(isScreenLockedCallback);

    mockSessionStage_ = new (std::nothrow) SessionStageMocker();
    ASSERT_NE(mockSessionStage_, nullptr);

    mockEventChannel_ = new (std::nothrow) WindowEventChannelMocker(mockSessionStage_);
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

int32_t WindowSessionTest2::GetTaskCount()
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
 * @tc.name: SetParentSession
 * @tc.desc: SetParentSession Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetParentSession, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> session = new (std::nothrow) Session(info);
    session_->SetParentSession(session);

    session_->property_ = new WindowSessionProperty();
    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: BindDialogToParentSession
 * @tc.desc: BindDialogToParentSession Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, BindDialogToParentSession, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> session = new (std::nothrow) Session(info);
    session_->BindDialogToParentSession(session);

    sptr<Session> session1 = new (std::nothrow) Session(info);
    ASSERT_NE(session1, nullptr);
    session1->persistentId_ = 33;
    session1->SetParentSession(session_);
    session1->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(session1);

    sptr<Session> session2 = new (std::nothrow) Session(info);
    ASSERT_NE(session2, nullptr);
    session2->persistentId_ = 34;
    session2->SetParentSession(session_);
    session2->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(session2);
    session_->BindDialogToParentSession(session1);

    session_->property_ = new WindowSessionProperty();
    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: RemoveDialogToParentSession
 * @tc.desc: RemoveDialogToParentSession Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, RemoveDialogToParentSession, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> session = new (std::nothrow) Session(info);
    session_->RemoveDialogToParentSession(session);

    sptr<Session> session1 = new (std::nothrow) Session(info);
    ASSERT_NE(session1, nullptr);
    session1->persistentId_ = 33;
    session1->SetParentSession(session_);
    session1->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(session1);

    sptr<Session> session2 = new (std::nothrow) Session(info);
    ASSERT_NE(session2, nullptr);
    session2->persistentId_ = 34;
    session2->SetParentSession(session_);
    session2->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(session2);
    session_->RemoveDialogToParentSession(session1);

    session_->property_ = new WindowSessionProperty();
    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: TransferPointerEvent01
 * @tc.desc: !IsSystemSession() && !IsSessionValid() is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferPointerEvent01, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, TransferPointerEvent02, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, TransferPointerEvent03, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);

    session_->property_ = new WindowSessionProperty();
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo info;
    info.abilityName_ = "dialogAbilityName";
    info.moduleName_ = "dialogModuleName";
    info.bundleName_ = "dialogBundleName";
    sptr<Session> dialogSession = new (std::nothrow) Session(info);
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
HWTEST_F(WindowSessionTest2, TransferPointerEvent04, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);

    session_->property_ = new WindowSessionProperty();
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    SessionInfo info;
    info.abilityName_ = "dialogAbilityName";
    info.moduleName_ = "dialogModuleName";
    info.bundleName_ = "dialogBundleName";
    sptr<Session> dialogSession = new (std::nothrow) Session(info);
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
HWTEST_F(WindowSessionTest2, TransferPointerEvent05, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();

    session_->property_ = new WindowSessionProperty();
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_SCENE_BOARD);

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferPointerEvent(pointerEvent));
}

/**
 * @tc.name: TransferPointerEvent06
 * @tc.desc: windowEventChannel_ is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferPointerEvent06, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);

    session_->property_ = new WindowSessionProperty();
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
 * @tc.name: TransferKeyEvent01
 * @tc.desc: !IsSystemSession() && !IsSessionValid() is true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferKeyEvent01, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, TransferKeyEvent02, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, TransferKeyEvent03, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);

    session_->property_ = new WindowSessionProperty();
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo info;
    info.abilityName_ = "dialogAbilityName";
    info.moduleName_ = "dialogModuleName";
    info.bundleName_ = "dialogBundleName";
    sptr<Session> dialogSession = new (std::nothrow) Session(info);
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
HWTEST_F(WindowSessionTest2, TransferKeyEvent04, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);

    session_->property_ = new WindowSessionProperty();
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    SessionInfo info;
    info.abilityName_ = "dialogAbilityName";
    info.moduleName_ = "dialogModuleName";
    info.bundleName_ = "dialogBundleName";
    sptr<Session> dialogSession = new (std::nothrow) Session(info);
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
HWTEST_F(WindowSessionTest2, TransferKeyEvent05, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;

    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);

    session_->property_ = new WindowSessionProperty();
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_SCENE_BOARD);

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: TransferBackPressedEventForConsumed01
 * @tc.desc: windowEventChannel_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, TransferBackPressedEventForConsumed01, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, TransferKeyEventForConsumed01, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, TransferFocusActiveEvent01, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, TransferFocusStateEvent01, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, Snapshot01, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->surfaceNode_ = nullptr;

    ASSERT_EQ(nullptr, session_->Snapshot());
}

/**
 * @tc.name: SaveSnapshot
 * @tc.desc: SaveSnapshot Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SaveSnapshot, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->scenePersistence_ = nullptr;
    session_->snapshot_ = nullptr;
    session_->SaveSnapshot(true);
    EXPECT_EQ(session_->snapshot_, nullptr);

    session_->scenePersistence_ = new ScenePersistence(session_->sessionInfo_.bundleName_, session_->persistentId_);

    session_->SaveSnapshot(false);
    ASSERT_EQ(session_->snapshot_, nullptr);

    session_->SaveSnapshot(true);
    ASSERT_EQ(session_->snapshot_, nullptr);
}

/**
 * @tc.name: SetSessionStateChangeListenser
 * @tc.desc: SetSessionStateChangeListenser Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionStateChangeListenser, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    NotifySessionStateChangeFunc func = nullptr;
    session_->SetSessionStateChangeListenser(func);

    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: SetSessionFocusableChangeListener
 * @tc.desc: SetSessionFocusableChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionFocusableChangeListener, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    NotifySessionFocusableChangeFunc func = [](const bool isFocusable)
    {
    };
    session_->SetSessionFocusableChangeListener(func);

    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: SetSessionTouchableChangeListener
 * @tc.desc: SetSessionTouchableChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionTouchableChangeListener, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    NotifySessionTouchableChangeFunc func = [](const bool touchable)
    {
    };
    session_->SetSessionTouchableChangeListener(func);

    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: SetSessionInfoLockedStateChangeListener
 * @tc.desc: SetSessionInfoLockedStateChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoLockedStateChangeListener, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    NotifySessionTouchableChangeFunc func = [](const bool lockedState)
    {
    };
    session_->SetSessionInfoLockedStateChangeListener(func);

    session_->SetSessionInfoLockedState(true);
    ASSERT_EQ(true, session_->sessionInfo_.lockedState);
}

/**
 * @tc.name: SetClickListener
 * @tc.desc: SetClickListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetClickListener, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, UpdateFocus01, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, UpdateFocus02, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, UpdateWindowMode01, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->property_ = nullptr;

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->UpdateWindowMode(WindowMode::WINDOW_MODE_UNDEFINED));
}

/**
 * @tc.name: NotifyForegroundInteractiveStatus
 * @tc.desc: NotifyForegroundInteractiveStatus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifyForegroundInteractiveStatus, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int res = 0;
    session_->sessionStage_ = nullptr;
    bool interactive = true;
    session_->NotifyForegroundInteractiveStatus(interactive);

    sptr<SessionStageMocker> mockSessionStage = new(std::nothrow) SessionStageMocker();
    ASSERT_NE(mockSessionStage, nullptr);
    session_->sessionStage_ = mockSessionStage;
    session_->state_ = SessionState::STATE_FOREGROUND;
    interactive = false;
    session_->NotifyForegroundInteractiveStatus(interactive);
    ASSERT_EQ(0, res);
}

/**
 * @tc.name: SetEventHandler001
 * @tc.desc: SetEventHandler Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetEventHandler001, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int res = 0;
    std::shared_ptr<AppExecFwk::EventHandler> handler = nullptr;
    session_->SetEventHandler(handler);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: PostTask002
 * @tc.desc: PostTask Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, PostTask002, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 0;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    if (property == nullptr) {
        return;
    }
    property->SetPersistentId(persistentId);
    int32_t res = session_->GetPersistentId();
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: GetSurfaceNode
 * @tc.desc: GetSurfaceNode Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetSurfaceNode, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->surfaceNode_ = nullptr;
    std::shared_ptr<RSSurfaceNode> res = session_->GetSurfaceNode();
    ASSERT_EQ(res, nullptr);
}

/**
 * @tc.name: GetLeashWinSurfaceNode
 * @tc.desc: GetLeashWinSurfaceNode Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetLeashWinSurfaceNode, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->leashWinSurfaceNode_ = nullptr;
    std::shared_ptr<RSSurfaceNode> res = session_->GetLeashWinSurfaceNode();
    ASSERT_EQ(res, nullptr);
}

/**
 * @tc.name: SetSessionInfoAncoSceneState
 * @tc.desc: SetSessionInfoAncoSceneState Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoAncoSceneState, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int res = 0;
    int32_t ancoSceneState = 0;
    session_->SetSessionInfoAncoSceneState(ancoSceneState);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: SetSessionInfoTime
 * @tc.desc: SetSessionInfoTime Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoTime, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int res = 0;
    std::string time = "";
    session_->SetSessionInfoTime(time);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: SetSessionInfoAbilityInfo
 * @tc.desc: SetSessionInfoAbilityInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoAbilityInfo, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int res = 0;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    session_->SetSessionInfoAbilityInfo(abilityInfo);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: SetSessionInfoWant
 * @tc.desc: SetSessionInfoWant Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoWant, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int res = 0;
    std::shared_ptr<AAFwk::Want> want = nullptr;
    session_->SetSessionInfoWant(want);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: SetSessionInfoProcessOptions
 * @tc.desc: SetSessionInfoProcessOptions Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoProcessOptions, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    std::shared_ptr<AAFwk::ProcessOptions> processOptions = std::make_shared<AAFwk::ProcessOptions>();
    session_->SetSessionInfoProcessOptions(processOptions);
    ASSERT_EQ(processOptions, session_->sessionInfo_.processOptions);
}

/**
 * @tc.name: SetSessionInfoPersistentId
 * @tc.desc: SetSessionInfoPersistentId Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoPersistentId, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int res = 0;
    int32_t persistentId = 0;
    session_->SetSessionInfoPersistentId(persistentId);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: SetSessionInfoCallerPersistentId
 * @tc.desc: SetSessionInfoCallerPersistentId Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionInfoCallerPersistentId, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int res = 0;
    int32_t callerPersistentId = 0;
    session_->SetSessionInfoCallerPersistentId(callerPersistentId);
    ASSERT_EQ(res, 0);
}

/**
 * @tc.name: PostExportTask
 * @tc.desc: PostExportTask Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, PostExportTask, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 0;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    if (property == nullptr) {
        return;
    }
    property->SetPersistentId(persistentId);
    int32_t ret = session_->GetPersistentId();
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: GetPersistentId
 * @tc.desc: GetPersistentId Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetPersistentId, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 0;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    if (property == nullptr) {
        return;
    }
    property->SetPersistentId(persistentId);
    int32_t ret = session_->GetPersistentId();
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: SetLeashWinSurfaceNode
 * @tc.desc: SetLeashWinSurfaceNode Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetLeashWinSurfaceNode, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetSessionInfoContinueState, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetSessionInfoIsClearSession01, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetSessionInfoIsClearSession02, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetSessionInfoAffinity, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetSessionInfo, Function | SmallTest | Level2)
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
    session_->SetSessionInfo(info);
    ASSERT_EQ(nullptr, session_->sessionInfo_.want);
    ASSERT_EQ(nullptr, session_->sessionInfo_.callerToken_);
    ASSERT_EQ(1, session_->sessionInfo_.requestCode);
    ASSERT_EQ(1, session_->sessionInfo_.callerPersistentId_);
    ASSERT_EQ(1, session_->sessionInfo_.callingTokenId_);
    ASSERT_EQ(1, session_->sessionInfo_.uiAbilityId_);
    ASSERT_EQ("", session_->sessionInfo_.continueSessionId_);
    ASSERT_EQ(nullptr, session_->sessionInfo_.startSetting);
}

/**
 * @tc.name: SetScreenId
 * @tc.desc: SetScreenId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetScreenId, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, RegisterLifecycleListener, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, UnregisterLifecycleListener, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, NotifyActivation02, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, NotifyConnect, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, NotifyForeground02, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, NotifyBackground02, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, NotifyDisconnect, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, NotifyExtensionDied02, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, NotifyTransferAccessibilityEvent, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, GetAspectRatio, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    float ret = session_->aspectRatio_;
    float res = 0.0f;
    ASSERT_EQ(ret, res);
}

/**
 * @tc.name: SetAspectRatio02
 * @tc.desc: SetAspectRatio
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetAspectRatio02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    float radio = 2.0f;
    WSError ERR = session_->SetAspectRatio(radio);
    float ret = session_->aspectRatio_;
    ASSERT_EQ(ret, radio);
    ASSERT_EQ(ERR, WSError::WS_OK);
}

/**
 * @tc.name: GetSessionState
 * @tc.desc: GetSessionState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetSessionState, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetSessionState02, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetChangeSessionVisibilityWithStatusBarEventListener, Function | SmallTest | Level2)
{
    int resultValue = 0;
    NotifyChangeSessionVisibilityWithStatusBarFunc func1 = [&resultValue](SessionInfo& info, const bool visible) {
        resultValue = 1;
    };
    NotifyChangeSessionVisibilityWithStatusBarFunc func2 = [&resultValue](SessionInfo& info, const bool visible) {
        resultValue = 2;
    };

    session_->SetChangeSessionVisibilityWithStatusBarEventListener(func1);
    ASSERT_NE(session_->changeSessionVisibilityWithStatusBarFunc_, nullptr);

    SessionInfo info;
    session_->changeSessionVisibilityWithStatusBarFunc_(info, true);
    ASSERT_EQ(resultValue, 1);

    session_->SetChangeSessionVisibilityWithStatusBarEventListener(func2);
    ASSERT_NE(session_->changeSessionVisibilityWithStatusBarFunc_, nullptr);
    session_->changeSessionVisibilityWithStatusBarFunc_(info, true);
    ASSERT_EQ(resultValue, 2);
}

/**
 * @tc.name: UpdateSessionState
 * @tc.desc: UpdateSessionState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, UpdateSessionState, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, GetTouchable, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetSystemTouchable, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, GetSystemTouchable, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetVisible, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, GetVisible02, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetVisibilityState, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    WindowVisibilityState state { WINDOW_VISIBILITY_STATE_NO_OCCLUSION};
    ASSERT_EQ(WSError::WS_OK, session_->SetVisibilityState(state));
    ASSERT_EQ(state, session_->visibilityState_);
}

/**
 * @tc.name: GetVisibilityState
 * @tc.desc: GetVisibilityState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetVisibilityState, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    WindowVisibilityState state { WINDOW_LAYER_STATE_MAX};
    ASSERT_EQ(state, session_->GetVisibilityState());
}

/**
 * @tc.name: SetDrawingContentState
 * @tc.desc: SetDrawingContentState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetDrawingContentState, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, GetDrawingContentState, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, GetBrightness, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->property_ = nullptr;
    ASSERT_EQ(UNDEFINED_BRIGHTNESS, session_->GetBrightness());
}

/**
 * @tc.name: IsActive02
 * @tc.desc: IsActive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, IsActive02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    bool res = session_->IsActive();
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: IsSystemSession
 * @tc.desc: IsSystemSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, IsSystemSession, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    bool res = session_->IsSystemSession();
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: Hide
 * @tc.desc: Hide
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, Hide, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    auto result = session_->Hide();
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Show
 * @tc.desc: Show
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, Show, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    auto result = session_->Show(property);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: DrawingCompleted
 * @tc.desc: DrawingCompleled
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, DrawingCompleted, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    auto result = session_->DrawingCompleted();
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: IsSystemActive
 * @tc.desc: IsSystemActive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, IsSystemActive, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetSystemActive, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, IsTerminated, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetAttachState01, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, SetAttachState02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 123;
    sptr<PatternDetachCallbackMocker> detachCallback = new PatternDetachCallbackMocker();
    EXPECT_CALL(*detachCallback, OnPatternDetach(persistentId)).Times(1);
    session_->persistentId_ = persistentId;
    session_->SetAttachState(true);
    session_->RegisterDetachCallback(detachCallback);
    session_->SetAttachState(false);
    usleep(WAIT_SYNC_IN_NS);
    Mock::VerifyAndClearExpectations(&detachCallback);
}

/**
 * @tc.name: RegisterDetachCallback01
 * @tc.desc: RegisterDetachCallback Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, RegisterDetachCallback01, Function | SmallTest | Level2)
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
HWTEST_F(WindowSessionTest2, RegisterDetachCallback02, Function | SmallTest | Level2)
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
 * @tc.name: RegisterDetachCallback03
 * @tc.desc: RegisterDetachCallback Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, RegisterDetachCallback03, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 123;
    sptr<PatternDetachCallbackMocker> detachCallback = new PatternDetachCallbackMocker();
    EXPECT_CALL(*detachCallback, OnPatternDetach(persistentId)).Times(::testing::AtLeast(1));
    session_->persistentId_ = persistentId;
    session_->SetAttachState(true);
    session_->SetAttachState(false);
    session_->RegisterDetachCallback(detachCallback);
    Mock::VerifyAndClearExpectations(&detachCallback);
}

/**
 * @tc.name: SetContextTransparentFunc
 * @tc.desc: SetContextTransparentFunc Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetContextTransparentFunc, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetContextTransparentFunc(nullptr);
    ASSERT_EQ(session_->contextTransparentFunc_, nullptr);
    NotifyContextTransparentFunc func = [](){};
    session_->SetContextTransparentFunc(func);
    ASSERT_NE(session_->contextTransparentFunc_, nullptr);
}

/**
 * @tc.name: NeedCheckContextTransparent
 * @tc.desc: NeedCheckContextTransparent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NeedCheckContextTransparent, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetContextTransparentFunc(nullptr);
    ASSERT_EQ(session_->NeedCheckContextTransparent(), false);
    NotifyContextTransparentFunc func = [](){};
    session_->SetContextTransparentFunc(func);
    ASSERT_EQ(session_->NeedCheckContextTransparent(), true);
}

/**
 * @tc.name: SetShowRecent001
 * @tc.desc: Exist detect task when in recent.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetShowRecent001, Function | SmallTest | Level2)
{
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    auto task = [](){};
    int64_t delayTime = 3000;
    session_->handler_->PostTask(task, taskName, delayTime);
    int32_t beforeTaskNum = GetTaskCount();

    session_->SetShowRecent(true);
    ASSERT_EQ(beforeTaskNum, GetTaskCount());
    session_->handler_->RemoveTask(taskName);
}

/**
 * @tc.name: SetShowRecent002
 * @tc.desc: SetShowRecent:showRecent is false, showRecent_ is false.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetShowRecent002, Function | SmallTest | Level2)
{
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    auto task = [](){};
    int64_t delayTime = 3000;
    session_->handler_->PostTask(task, taskName, delayTime);
    session_->showRecent_ = false;
    int32_t beforeTaskNum = GetTaskCount();

    session_->SetShowRecent(false);
    ASSERT_EQ(beforeTaskNum, GetTaskCount());
    session_->handler_->RemoveTask(taskName);
}

/**
 * @tc.name: SetShowRecent003
 * @tc.desc: SetShowRecent:showRecent is false, showRecent_ is true, detach task.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetShowRecent003, Function | SmallTest | Level2)
{
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    auto task = [](){};
    int64_t delayTime = 3000;
    session_->handler_->PostTask(task, taskName, delayTime);
    session_->showRecent_ = true;
    session_->isAttach_ = false;
    int32_t beforeTaskNum = GetTaskCount();

    session_->SetShowRecent(false);
    ASSERT_EQ(beforeTaskNum, GetTaskCount());
    session_->handler_->RemoveTask(taskName);
}

/**
 * @tc.name: SetShowRecent004
 * @tc.desc: SetShowRecent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetShowRecent004, Function | SmallTest | Level2)
{
    session_->systemConfig_.uiType_ = "phone";
    ssm_->SetScreenLocked(false);

    session_->property_ = new WindowSessionProperty();
    session_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);

    bool showRecent = false;
    session_->showRecent_ = true;
    session_->SetAttachState(true);
    session_->SetShowRecent(showRecent);
    ASSERT_EQ(session_->GetShowRecent(), showRecent);
}

/**
 * @tc.name: CreateDetectStateTask001
 * @tc.desc: Create detection task when there are no pre_existing tasks.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, CreateDetectStateTask001, Function | SmallTest | Level2)
{
    session_->systemConfig_.uiType_ = "phone";
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    DetectTaskInfo detectTaskInfo;
    detectTaskInfo.taskState = DetectTaskState::NO_TASK;
    int32_t beforeTaskNum = GetTaskCount();
    session_->SetDetectTaskInfo(detectTaskInfo);
    session_->CreateDetectStateTask(false, WindowMode::WINDOW_MODE_FULLSCREEN);

    ASSERT_EQ(beforeTaskNum + 1, GetTaskCount());
    ASSERT_EQ(DetectTaskState::DETACH_TASK, session_->GetDetectTaskInfo().taskState);
    session_->handler_->RemoveTask(taskName);

    session_->showRecent_ = true;
    session_->CreateDetectStateTask(false, WindowMode::WINDOW_MODE_FULLSCREEN);
}

/**
 * @tc.name: CreateDetectStateTask002
 * @tc.desc: Detect state when window mode changed.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, CreateDetectStateTask002, Function | SmallTest | Level2)
{
    session_->systemConfig_.uiType_ = "phone";
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    auto task = [](){};
    int64_t delayTime = 3000;
    session_->handler_->PostTask(task, taskName, delayTime);
    int32_t beforeTaskNum = GetTaskCount();

    DetectTaskInfo detectTaskInfo;
    detectTaskInfo.taskState = DetectTaskState::DETACH_TASK;
    detectTaskInfo.taskWindowMode = WindowMode::WINDOW_MODE_FULLSCREEN;
    session_->SetDetectTaskInfo(detectTaskInfo);
    session_->CreateDetectStateTask(true, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);

    ASSERT_EQ(beforeTaskNum - 1, GetTaskCount());
    ASSERT_EQ(DetectTaskState::NO_TASK, session_->GetDetectTaskInfo().taskState);
    ASSERT_EQ(WindowMode::WINDOW_MODE_UNDEFINED, session_->GetDetectTaskInfo().taskWindowMode);
    session_->handler_->RemoveTask(taskName);

    session_->showRecent_ = true;
    session_->CreateDetectStateTask(false, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
}

/**
 * @tc.name: CreateDetectStateTask003
 * @tc.desc: Detect sup and down tree tasks for the same type.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, CreateDetectStateTask003, Function | SmallTest | Level2)
{
    session_->systemConfig_.uiType_ = "phone";
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    DetectTaskInfo detectTaskInfo;
    detectTaskInfo.taskState = DetectTaskState::DETACH_TASK;
    detectTaskInfo.taskWindowMode = WindowMode::WINDOW_MODE_FULLSCREEN;
    int32_t beforeTaskNum = GetTaskCount();
    session_->SetDetectTaskInfo(detectTaskInfo);
    session_->CreateDetectStateTask(false, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);

    ASSERT_EQ(beforeTaskNum + 1, GetTaskCount());
    ASSERT_EQ(DetectTaskState::DETACH_TASK, session_->GetDetectTaskInfo().taskState);
    session_->handler_->RemoveTask(taskName);

    session_->showRecent_ = true;
    session_->CreateDetectStateTask(false, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
}

/**
 * @tc.name: CreateDetectStateTask004
 * @tc.desc: Detection tasks under the same window mode.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, CreateDetectStateTask004, Function | SmallTest | Level2)
{
    session_->systemConfig_.uiType_ = "phone";
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    DetectTaskInfo detectTaskInfo;
    int32_t beforeTaskNum = GetTaskCount();
    detectTaskInfo.taskState = DetectTaskState::DETACH_TASK;
    detectTaskInfo.taskWindowMode = WindowMode::WINDOW_MODE_FULLSCREEN;
    session_->SetDetectTaskInfo(detectTaskInfo);
    session_->CreateDetectStateTask(true, WindowMode::WINDOW_MODE_FULLSCREEN);

    ASSERT_EQ(beforeTaskNum + 1, GetTaskCount());
    ASSERT_EQ(DetectTaskState::ATTACH_TASK, session_->GetDetectTaskInfo().taskState);
    session_->handler_->RemoveTask(taskName);

    session_->showRecent_ = true;
    session_->CreateDetectStateTask(false, WindowMode::WINDOW_MODE_FULLSCREEN);
}

/**
 * @tc.name: GetAttachState001
 * @tc.desc: GetAttachState001
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetAttachState001, Function | SmallTest | Level2)
{
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    session_->SetAttachState(false);
    bool isAttach = session_->GetAttachState();
    ASSERT_EQ(false, isAttach);
    session_->handler_->RemoveTask(taskName);
}

/**
 * @tc.name: ResetSessionConnectState
 * @tc.desc: ResetSessionConnectState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, ResetSessionConnectState, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->ResetSessionConnectState();
    ASSERT_EQ(session_->state_, SessionState::STATE_DISCONNECT);
    ASSERT_EQ(session_->GetCallingPid(), -1);
}

/**
 * @tc.name: PostExportTask02
 * @tc.desc: PostExportTask
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, PostExportTask02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    std::string name = "sessionExportTask";
    auto task = [](){};
    int64_t delayTime = 0;

    session_->PostExportTask(task, name, delayTime);
    auto result = session_->GetBufferAvailable();
    ASSERT_EQ(result, false);

    sptr<SceneSessionManager> sceneSessionManager = new SceneSessionManager();
    session_->SetEventHandler(sceneSessionManager->taskScheduler_->GetEventHandler(),
        sceneSessionManager->eventHandler_);
    session_->PostExportTask(task, name, delayTime);
    auto result2 = session_->GetBufferAvailable();
    ASSERT_EQ(result2, false);
}

/**
 * @tc.name: SetLeashWinSurfaceNode02
 * @tc.desc: SetLeashWinSurfaceNode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetLeashWinSurfaceNode02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->leashWinSurfaceNode_ = WindowSessionTest2::CreateRSSurfaceNode();
    session_->SetLeashWinSurfaceNode(nullptr);

    session_->leashWinSurfaceNode_ = nullptr;
    session_->SetLeashWinSurfaceNode(nullptr);
    auto result = session_->GetBufferAvailable();
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: GetCloseAbilityWantAndClean
 * @tc.desc: GetCloseAbilityWantAndClean
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetCloseAbilityWantAndClean, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    AAFwk::Want outWant;
    session_->sessionInfo_.closeAbilityWant = std::make_shared<AAFwk::Want>();
    session_->GetCloseAbilityWantAndClean(outWant);

    session_->sessionInfo_.closeAbilityWant = nullptr;
    session_->GetCloseAbilityWantAndClean(outWant);
    auto result = session_->GetBufferAvailable();
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: SetScreenId02
 * @tc.desc: SetScreenId Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetScreenId02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    uint64_t screenId = 0;
    session_->sessionStage_ = new (std::nothrow) SessionStageMocker();
    session_->SetScreenId(screenId);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: SetSessionState
 * @tc.desc: SetSessionState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionState, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    SessionState state03 = SessionState::STATE_CONNECT;
    session_->SetSessionState(state03);
    ASSERT_EQ(state03, session_->state_);
}

/**
 * @tc.name: SetFocusable03
 * @tc.desc: SetFocusable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetFocusable03, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->isFocused_ = true;
    session_->property_ = new (std::nothrow) WindowSessionProperty();
    session_->property_->focusable_ = false;
    bool isFocusable = true;

    auto result = session_->SetFocusable(isFocusable);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: GetFocused
 * @tc.desc: GetFocused Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetFocused, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    bool result = session_->GetFocused();
    ASSERT_EQ(result, false);

    session_->isFocused_ = true;
    bool result2 = session_->GetFocused();
    ASSERT_EQ(result2, true);
}

/**
 * @tc.name: UpdatePointerArea
 * @tc.desc: UpdatePointerArea Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, UpdatePointerArea, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    WSRect rect = { 0, 0, 0, 0 };
    session_->preRect_ = rect;
    session_->UpdatePointerArea(rect);
    ASSERT_EQ(session_->GetFocused(), false);
}

/**
 * @tc.name: UpdateSizeChangeReason02
 * @tc.desc: UpdateSizeChangeReason Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, UpdateSizeChangeReason02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    WSError result = session_->UpdateSizeChangeReason(reason);
    ASSERT_EQ(result, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: UpdateDensity
 * @tc.desc: UpdateDensity Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, UpdateDensity, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_FALSE(session_->IsSessionValid());
    WSError result = session_->UpdateDensity();
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    session_->state_ = SessionState::STATE_CONNECT;
    ASSERT_TRUE(session_->IsSessionValid());
    session_->sessionStage_ = nullptr;
    WSError result02 = session_->UpdateDensity();
    ASSERT_EQ(result02, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateSizeChangeReason
 * @tc.desc: UpdateSizeChangeReason UpdateDensity
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, UpdateSizeChangeReason, Function | SmallTest | Level2)
{
    SizeChangeReason reason = SizeChangeReason{1};
    ASSERT_EQ(session_->UpdateSizeChangeReason(reason), WSError::WS_OK);
}

/**
 * @tc.name: SetPendingSessionActivationEventListener
 * @tc.desc: SetPendingSessionActivationEventListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetPendingSessionActivationEventListener, Function | SmallTest | Level2)
{
    int resultValue = 0;
    session_->SetPendingSessionActivationEventListener([&resultValue](const SessionInfo& info) {
        resultValue = 1;
    });
    usleep(WAIT_SYNC_IN_NS);
    session_->SetTerminateSessionListener([&resultValue](const SessionInfo& info) {
        resultValue = 2;
    });
    usleep(WAIT_SYNC_IN_NS);
    LifeCycleTaskType taskType = LifeCycleTaskType{0};
    session_->RemoveLifeCycleTask(taskType);
    ASSERT_EQ(resultValue, 0);
}

/**
 * @tc.name: SetSessionIcon
 * @tc.desc: SetSessionIcon UpdateDensity
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionIcon, Function | SmallTest | Level2)
{
    std::shared_ptr<Media::PixelMap> icon;
    session_->SetSessionIcon(icon);
    ASSERT_EQ(session_->Clear(), WSError::WS_OK);
    session_->SetSessionSnapshotListener(nullptr);
    NotifyPendingSessionActivationFunc func = [](const SessionInfo& info) {};
    session_->pendingSessionActivationFunc_ = func;
    ASSERT_EQ(session_->PendingSessionToForeground(), WSError::WS_OK);

    session_->scenePersistence_ = sptr<ScenePersistence>::MakeSptr("SetSessionIcon", 1);
    session_->updateSessionIconFunc_ = nullptr;
    ASSERT_EQ(WSError::WS_OK, session_->SetSessionIcon(icon));

    NofitySessionIconUpdatedFunc func2 = [](const std::string& iconPath) {};
    session_->updateSessionIconFunc_ = func2;
    ASSERT_EQ(WSError::WS_OK, session_->SetSessionIcon(icon));

    NotifyTerminateSessionFuncNew func3 = [](const SessionInfo& info, bool needStartCaller, bool isFromBroker) {};
    session_->terminateSessionFuncNew_ = func3;
    ASSERT_EQ(WSError::WS_OK, session_->Clear());
}

/**
 * @tc.name: SetSessionExceptionListener
 * @tc.desc: SetSessionExceptionListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSessionExceptionListener, Function | SmallTest | Level2)
{
    session_->SetSessionExceptionListener(nullptr, true);
    session_->SetSessionExceptionListener([](const SessionInfo& info, bool removeSession, bool startFail) {}, true);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_NE(nullptr, session_->jsSceneSessionExceptionFunc_);
}
/**
 * @tc.name: SetRaiseToAppTopForPointDownFunc
 * @tc.desc: SetRaiseToAppTopForPointDownFunc Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetRaiseToAppTopForPointDownFunc, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetRaiseToAppTopForPointDownFunc(nullptr);

    NotifyRaiseToTopForPointDownFunc func = []() {};
    session_->raiseToTopForPointDownFunc_ = func;
    session_->RaiseToAppTopForPointDown();
    session_->HandlePointDownDialog();
    session_->ClearDialogVector();

    session_->SetBufferAvailableChangeListener(nullptr);
    session_->UnregisterSessionChangeListeners();
    session_->SetSessionStateChangeNotifyManagerListener(nullptr);
    session_->SetSessionInfoChangeNotifyManagerListener(nullptr);
    session_->NotifyFocusStatus(true);

    session_->SetRequestFocusStatusNotifyManagerListener(nullptr);
    session_->SetNotifyUIRequestFocusFunc(nullptr);
    session_->SetNotifyUILostFocusFunc(nullptr);
    session_->UnregisterSessionChangeListeners();

    NotifyPendingSessionToBackgroundForDelegatorFunc func2 = [](const SessionInfo& info, bool shouldBackToCaller) {};
    session_->pendingSessionToBackgroundForDelegatorFunc_ = func2;
    ASSERT_EQ(WSError::WS_OK, session_->PendingSessionToBackgroundForDelegator(true));
}

/**
 * @tc.name: NotifyCloseExistPipWindow
 * @tc.desc: check func NotifyCloseExistPipWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifyCloseExistPipWindow, Function | SmallTest | Level2)
{
    sptr<SessionStageMocker> mockSessionStage = new(std::nothrow) SessionStageMocker();
    ASSERT_NE(mockSessionStage, nullptr);
    ManagerState key = ManagerState{0};
    session_->GetStateFromManager(key);
    session_->NotifyUILostFocus();

    session_->lostFocusFunc_ = []() {};
    session_->NotifyUILostFocus();

    session_->SetSystemSceneBlockingFocus(true);
    session_->GetBlockingFocus();
    session_->sessionStage_ = mockSessionStage;
    EXPECT_CALL(*(mockSessionStage), NotifyCloseExistPipWindow()).Times(1).WillOnce(Return(WSError::WS_OK));
    ASSERT_EQ(WSError::WS_OK, session_->NotifyCloseExistPipWindow());
    session_->sessionStage_ = nullptr;
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->NotifyCloseExistPipWindow());
}

/**
 * @tc.name: SetSystemConfig
 * @tc.desc: SetSystemConfig Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetSystemConfig, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    SystemSessionConfig systemConfig;
    session_->SetSystemConfig(systemConfig);
    float snapshotScale = 0.5;
    session_->SetSnapshotScale(snapshotScale);
    session_->ProcessBackEvent();
    session_->NotifyOccupiedAreaChangeInfo(nullptr);
    session_->UpdateMaximizeMode(true);
    ASSERT_EQ(session_->GetZOrder(), 0);

    session_->SetUINodeId(0);
    session_->GetUINodeId();
    session_->SetShowRecent(true);
    session_->GetShowRecent();
    session_->SetBufferAvailable(true);

    session_->SetNeedSnapshot(true);
    session_->SetFloatingScale(0.5);
    ASSERT_EQ(session_->GetFloatingScale(), 0.5f);
    session_->SetScale(50, 100, 50, 100);
    session_->GetScaleX();
    session_->GetScaleY();
    session_->GetPivotX();
    session_->GetPivotY();
    session_->SetSCBKeepKeyboard(true);
    session_->GetSCBKeepKeyboardFlag();
    ASSERT_EQ(WSError::WS_OK, session_->MarkProcessed(11));
}

/**
 * @tc.name: SetOffset
 * @tc.desc: SetOffset Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetOffset, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetOffset(50, 100);
    session_->GetOffsetX();
    session_->GetOffsetY();
    WSRectF bounds;
    session_->SetBounds(bounds);
    session_->GetBounds();
    session_->UpdateTitleInTargetPos(true, 100);
    session_->SetNotifySystemSessionPointerEventFunc(nullptr);
    session_->SetNotifySystemSessionKeyEventFunc(nullptr);
    ASSERT_EQ(session_->GetBufferAvailable(), false);
}

/**
 * @tc.name: SetBackPressedListenser
 * @tc.desc: SetBackPressedListenser Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetBackPressedListenser, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int32_t result = 0;
    session_->SetBackPressedListenser([&result](const bool needMoveToBackground) {
        result = 1;
    });
    usleep(WAIT_SYNC_IN_NS);
    session_->backPressedFunc_(true);
    ASSERT_EQ(result, 1);
}

/**
 * @tc.name: SetUpdateSessionIconListener
 * @tc.desc: SetUpdateSessionIconListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetUpdateSessionIconListener, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    WLOGFI("SetUpdateSessionIconListener begin!");

    session_->SetUpdateSessionIconListener(session_->updateSessionIconFunc_);

    WLOGFI("SetUpdateSessionIconListener end!");
}

/**
 * @tc.name: NotifyContextTransparent
 * @tc.desc: NotifyContextTransparent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifyContextTransparent, Function | SmallTest | Level2)
{
    WLOGFI("NotifyContextTransparent begin!");
    ASSERT_NE(session_, nullptr);

    NotifyContextTransparentFunc contextTransparentFunc = session_->contextTransparentFunc_;
    if (contextTransparentFunc == nullptr) {
        contextTransparentFunc = []() {};
    }
    session_->contextTransparentFunc_ = nullptr;
    session_->NotifyContextTransparent();

    session_->SetContextTransparentFunc(contextTransparentFunc);
    session_->NotifyContextTransparent();

    WLOGFI("NotifyContextTransparent end!");
}

/**
 * @tc.name: NotifySessionInfoLockedStateChange
 * @tc.desc: NotifySessionInfoLockedStateChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, NotifySessionInfoLockedStateChange, Function | SmallTest | Level2)
{
    WLOGFI("NotifySessionInfoLockedStateChange begin!");
    ASSERT_NE(session_, nullptr);

    NotifySessionInfoLockedStateChangeFunc sessionInfoLockedStateChangeFunc =
        session_->sessionInfoLockedStateChangeFunc_;
    if (sessionInfoLockedStateChangeFunc == nullptr) {
        sessionInfoLockedStateChangeFunc = [](const bool lockedState) {};
    }
    session_->sessionInfoLockedStateChangeFunc_ = nullptr;
    session_->NotifySessionInfoLockedStateChange(true);

    session_->SetSessionInfoLockedStateChangeListener(sessionInfoLockedStateChangeFunc);
    session_->NotifySessionInfoLockedStateChange(true);

    WLOGFI("NotifySessionInfoLockedStateChange end!");
}

/**
 * @tc.name: GetMainSession
 * @tc.desc: GetMainSession Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, GetMainSession, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "getMainSession";
    info.moduleName_ = "getMainSession";
    info.bundleName_ = "getMainSession";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    ASSERT_NE(session, nullptr);
    session->property_ = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(session->property_, nullptr);
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    EXPECT_EQ(session, session->GetMainSession());

    sptr<Session> subSession = sptr<Session>::MakeSptr(info);
    ASSERT_NE(subSession, nullptr);
    subSession->SetParentSession(session);
    subSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(subSession->property_, nullptr);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(session, subSession->GetMainSession());

    sptr<Session> subSubSession = sptr<Session>::MakeSptr(info);
    ASSERT_NE(subSubSession, nullptr);
    subSubSession->SetParentSession(subSession);
    subSubSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(subSubSession->property_, nullptr);
    subSubSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(session, subSubSession->GetMainSession());
}

/**
 * @tc.name: IsSupportDetectWindow
 * @tc.desc: IsSupportDetectWindow Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, IsSupportDetectWindow, Function | SmallTest | Level2)
{
    session_->systemConfig_.uiType_ = "phone";
    ssm_->SetScreenLocked(true);
    bool ret = session_->IsSupportDetectWindow(true);
    ASSERT_EQ(ret, false);

    ssm_->SetScreenLocked(false);
    session_->property_ = new WindowSessionProperty();
    session_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = session_->IsSupportDetectWindow(true);
    ASSERT_EQ(ret, false);

    ssm_->SetScreenLocked(false);
    session_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    session_->systemConfig_.uiType_ = "pc";
    ret = session_->IsSupportDetectWindow(false);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: ShouldCreateDetectTask
 * @tc.desc: ShouldCreateDetectTask Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, ShouldCreateDetectTask, Function | SmallTest | Level2)
{
    DetectTaskInfo detectTaskInfo;
    detectTaskInfo.taskState = DetectTaskState::ATTACH_TASK;
    detectTaskInfo.taskWindowMode = WindowMode::WINDOW_MODE_FULLSCREEN;
    session_->SetDetectTaskInfo(detectTaskInfo);
    bool ret = session_->ShouldCreateDetectTask(true, WindowMode::WINDOW_MODE_UNDEFINED);
    ASSERT_EQ(ret, true);
    detectTaskInfo.taskState = DetectTaskState::DETACH_TASK;
    session_->SetDetectTaskInfo(detectTaskInfo);
    ret = session_->ShouldCreateDetectTask(false, WindowMode::WINDOW_MODE_UNDEFINED);
    ASSERT_EQ(ret, true);
    ret = session_->ShouldCreateDetectTask(true, WindowMode::WINDOW_MODE_UNDEFINED);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: ShouldCreateDetectTaskInRecent
 * @tc.desc: ShouldCreateDetectTaskInRecent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, ShouldCreateDetectTaskInRecent, Function | SmallTest | Level2)
{
    bool ret = session_->ShouldCreateDetectTaskInRecent(true, true, true);
    ASSERT_EQ(ret, false);
    ret = session_->ShouldCreateDetectTaskInRecent(false, true, true);
    ASSERT_EQ(ret, true);
    ret = session_->ShouldCreateDetectTaskInRecent(false, true, false);
    ASSERT_EQ(ret, false);
    ret = session_->ShouldCreateDetectTaskInRecent(false, false, false);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: CreateWindowStateDetectTask
 * @tc.desc: CreateWindowStateDetectTask Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, CreateWindowStateDetectTask, Function | SmallTest | Level2)
{
    auto isScreenLockedCallback = [this]() { return ssm_->IsScreenLocked(); };
    session_->RegisterIsScreenLockedCallback(isScreenLockedCallback);
    session_->SetSessionState(SessionState::STATE_CONNECT);
    bool isAttach = true;
    session_->CreateWindowStateDetectTask(isAttach, WindowMode::WINDOW_MODE_UNDEFINED);
    ASSERT_EQ(isAttach, true);

    session_->handler_ = nullptr;
    session_->CreateWindowStateDetectTask(false, WindowMode::WINDOW_MODE_UNDEFINED);
    ASSERT_EQ(session_->handler_, nullptr);
}

/**
 * @tc.name: SetOffset01
 * @tc.desc: SetOffset Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest2, SetOffset01, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetOffset(0, 0);
    ASSERT_EQ(session_->GetOffsetX(), 0);
}
}
} // namespace Rosen
} // namespace OHOS
