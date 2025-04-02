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

class WindowSessionTest : public testing::Test {
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
        void OnAppRemoveStartingWindow() override {}
    };
    std::shared_ptr<TLifecycleListener> lifecycleListener_ = std::make_shared<TLifecycleListener>();

    sptr<SessionStageMocker> mockSessionStage_ = nullptr;
    sptr<WindowEventChannelMocker> mockEventChannel_ = nullptr;
};

void WindowSessionTest::SetUpTestCase()
{
}

void WindowSessionTest::TearDownTestCase()
{
}

void WindowSessionTest::SetUp()
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

void WindowSessionTest::TearDown()
{
    session_ = nullptr;
    usleep(WAIT_SYNC_IN_NS);
}

RSSurfaceNode::SharedPtr WindowSessionTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    if (surfaceNode == nullptr) {
        GTEST_LOG_(INFO) << "WindowSessionTest::CreateRSSurfaceNode surfaceNode is nullptr";
    }
    return surfaceNode;
}

int32_t WindowSessionTest::GetTaskCount()
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
 * @tc.name: SetForceTouchable
 * @tc.desc: SetForceTouchable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetForceTouchable, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    bool touchable = false;
    session_->SetForceTouchable(touchable);
    ASSERT_EQ(session_->forceTouchable_, touchable);
}

/**
 * @tc.name: SetActive01
 * @tc.desc: set session active
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(WindowSessionTest, SetActive01, Function | SmallTest | Level2)
{
    sptr<ISession> sessionToken = nullptr;
    sptr<SessionStageMocker> mockSessionStage = new(std::nothrow) SessionStageMocker();
    EXPECT_NE(nullptr, mockSessionStage);
    EXPECT_CALL(*(mockSessionStage), SetActive(_)).WillOnce(Return(WSError::WS_OK));
    EXPECT_CALL(*(mockSessionStage), UpdateRect(_, _, _, _)).Times(0).WillOnce(Return(WSError::WS_OK));
    session_->sessionStage_ = mockSessionStage;
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, session_->SetActive(true));

    sptr<WindowEventChannelMocker> mockEventChannel = new(std::nothrow) WindowEventChannelMocker(mockSessionStage);
    EXPECT_NE(nullptr, mockEventChannel);
    auto surfaceNode = CreateRSSurfaceNode();
    SystemSessionConfig sessionConfig;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    ASSERT_EQ(WSError::WS_OK, session_->Connect(mockSessionStage,
            mockEventChannel, surfaceNode, sessionConfig, property));
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, session_->SetActive(true));
    ASSERT_EQ(false, session_->isActive_);

    session_->UpdateSessionState(SessionState::STATE_FOREGROUND);
    ASSERT_EQ(WSError::WS_OK, session_->SetActive(true));
    ASSERT_EQ(true, session_->isActive_);
}

/**
 * @tc.name: SetCompatibleModeEnableInPad
 * @tc.desc: SetCompatibleModeEnableInPad test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetCompatibleModeEnableInPad, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    bool enable = true;
    property->SetCompatibleModeEnableInPad(enable);
    ASSERT_EQ(property->GetCompatibleModeEnableInPad(), true);
}

/**
 * @tc.name: UpdateRect01
 * @tc.desc: update rect
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(WindowSessionTest, UpdateRect01, Function | SmallTest | Level2)
{
    sptr<ISession> sessionToken = nullptr;
    sptr<SessionStageMocker> mockSessionStage = new(std::nothrow) SessionStageMocker();
    EXPECT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;
    EXPECT_CALL(*(mockSessionStage), UpdateRect(_, _, _, _)).Times(AtLeast(1)).WillOnce(Return(WSError::WS_OK));

    WSRect rect = {0, 0, 0, 0};
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, session_->UpdateRect(rect,
        SizeChangeReason::UNDEFINED, "WindowSessionTest"));
    sptr<WindowEventChannelMocker> mockEventChannel = new(std::nothrow) WindowEventChannelMocker(mockSessionStage);
    EXPECT_NE(nullptr, mockEventChannel);
    SystemSessionConfig sessionConfig;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    ASSERT_EQ(WSError::WS_OK, session_->Connect(mockSessionStage,
            mockEventChannel, nullptr, sessionConfig, property));

    rect = {0, 0, 100, 100};
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, session_->UpdateRect(rect,
        SizeChangeReason::UNDEFINED, "WindowSessionTest"));
    ASSERT_EQ(rect, session_->winRect_);

    session_->UpdateSessionState(SessionState::STATE_ACTIVE);
    ASSERT_EQ(WSError::WS_OK, session_->UpdateRect(rect, SizeChangeReason::UNDEFINED, "WindowSessionTest"));

    session_->sessionStage_ = nullptr;
    ASSERT_EQ(WSError::WS_OK, session_->UpdateRect(rect, SizeChangeReason::UNDEFINED, "WindowSessionTest"));
}

/**
 * @tc.name: IsSessionValid01
 * @tc.desc: check func IsSessionValid
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, IsSessionValid01, Function | SmallTest | Level2)
{
    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_FALSE(session_->IsSessionValid());
    session_->state_ = SessionState::STATE_CONNECT;
    ASSERT_TRUE(session_->IsSessionValid());
}

/**
 * @tc.name: ConnectInner
 * @tc.desc: ConnectInner
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, ConnectInner, Function | SmallTest | Level2)
{
    SystemSessionConfig sessionConfig;
    session_->state_ = SessionState::STATE_CONNECT;
    session_->isTerminating_ = false;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetIsNeedUpdateWindowMode(true);
    session_->SetScreenId(233);
    session_->SetSessionProperty(property);
    auto res = session_->ConnectInner(mockSessionStage_, mockEventChannel_,
        nullptr, sessionConfig, property, nullptr, 1, 1, "");
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_SESSION);

    session_->isTerminating_ = true;
    auto res2 = session_->ConnectInner(mockSessionStage_, mockEventChannel_,
        nullptr, sessionConfig, property, nullptr, 1, 1, "");
    ASSERT_EQ(res2, WSError::WS_OK);

    property->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    property->SetIsNeedUpdateWindowMode(true);
    session_->SetScreenId(SCREEN_ID_INVALID);
    session_->SetSessionProperty(property);
    auto res3 = session_->ConnectInner(mockSessionStage_, mockEventChannel_,
        nullptr, sessionConfig, property, nullptr, 1, 1, "");
    ASSERT_EQ(res3, WSError::WS_OK);
}

/**
 * @tc.name: RemoveLifeCycleTask
 * @tc.desc: RemoveLifeCycleTask & PostLifeCycleTask
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, LifeCycleTask, Function | SmallTest | Level2)
{
    auto task = []() {};
    session_->PostLifeCycleTask(task, "task1", LifeCycleTaskType::START);
    ASSERT_EQ(session_->lifeCycleTaskQueue_.size(), 1);

    auto task2 = []() {};
    session_->PostLifeCycleTask(task2, "task2", LifeCycleTaskType::START);
    ASSERT_EQ(session_->lifeCycleTaskQueue_.size(), 2);

    LifeCycleTaskType taskType = LifeCycleTaskType{0};

    session_->RemoveLifeCycleTask(taskType);
    ASSERT_EQ(session_->lifeCycleTaskQueue_.size(), 1);

    session_->RemoveLifeCycleTask(taskType);
    ASSERT_EQ(session_->lifeCycleTaskQueue_.size(), 0);
}

/**
 * @tc.name: SetSessionProperty01
 * @tc.desc: SetSessionProperty
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetSessionProperty01, Function | SmallTest | Level2)
{
    ASSERT_EQ(session_->SetSessionProperty(nullptr), WSError::WS_OK);
}

/**
 * @tc.name: SetSessionRect
 * @tc.desc: check func SetSessionRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetSessionRect, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    WSRect rect = { 0, 0, 320, 240}; // width: 320, height: 240
    session_->SetSessionRect(rect);
    ASSERT_EQ(rect, session_->winRect_);
}

/**
 * @tc.name: GetSessionRect
 * @tc.desc: check func GetSessionRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetSessionRect, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    WSRect rect = { 0, 0, 320, 240}; // width: 320, height: 240
    session_->SetSessionRect(rect);
    ASSERT_EQ(rect, session_->GetSessionRect());
}

/**
 * @tc.name: GetLayoutRect
 * @tc.desc: check func GetLayoutRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetLayoutRect, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    WSRect rect = { 0, 0, 320, 240 }; // width: 320, height: 240
    session_->layoutRect_ = rect;
    session_->lastLayoutRect_ = session_->layoutRect_;
    ASSERT_EQ(rect, session_->GetLayoutRect());
    ASSERT_EQ(rect, session_->GetLastLayoutRect());
}

/**
 * @tc.name: CheckDialogOnForeground
 * @tc.desc: check func CheckDialogOnForeground
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, CheckDialogOnForeground, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->dialogVec_.clear();
    ASSERT_EQ(false, session_->CheckDialogOnForeground());
    SessionInfo info;
    info.abilityName_ = "dialogAbilityName";
    info.moduleName_ = "dialogModuleName";
    info.bundleName_ = "dialogBundleName";
    sptr<Session> dialogSession = new (std::nothrow) Session(info);
    ASSERT_NE(dialogSession, nullptr);
    dialogSession->state_ = SessionState::STATE_INACTIVE;
    session_->dialogVec_.push_back(dialogSession);
    ASSERT_EQ(false, session_->CheckDialogOnForeground());
    session_->dialogVec_.clear();
}

/**
 * @tc.name: IsTopDialog
 * @tc.desc: check func IsTopDialog
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, IsTopDialog, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->dialogVec_.clear();
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";

    sptr<Session> dialogSession1 = new (std::nothrow) Session(info);
    ASSERT_NE(dialogSession1, nullptr);
    dialogSession1->persistentId_ = 33;
    dialogSession1->SetParentSession(session_);
    dialogSession1->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(dialogSession1);

    sptr<Session> dialogSession2 = new (std::nothrow) Session(info);
    ASSERT_NE(dialogSession2, nullptr);
    dialogSession2->persistentId_ = 34;
    dialogSession2->SetParentSession(session_);
    dialogSession2->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(dialogSession2);

    sptr<Session> dialogSession3 = new (std::nothrow) Session(info);
    ASSERT_NE(dialogSession3, nullptr);
    dialogSession3->persistentId_ = 35;
    dialogSession3->SetParentSession(session_);
    dialogSession3->state_ = SessionState::STATE_INACTIVE;
    session_->dialogVec_.push_back(dialogSession3);

    ASSERT_EQ(false, dialogSession3->IsTopDialog());
    ASSERT_EQ(true, dialogSession2->IsTopDialog());
    ASSERT_EQ(false, dialogSession1->IsTopDialog());
    session_->dialogVec_.clear();
}

/**
 * @tc.name: GetGlobalScaledRect
 * @tc.desc: GetGlobalScaledRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetGlobalScaledRect, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    Rect globalScaledRect;
    sceneSession->globalRect_ = {100, 100, 50, 40};
    sceneSession->isScbCoreEnabled_ = true;
    sceneSession->scaleX_ = 0.5f;
    sceneSession->scaleY_ = 0.5f;
    WMError ret = sceneSession->GetGlobalScaledRect(globalScaledRect);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(100, globalScaledRect.posX_);
    ASSERT_EQ(100, globalScaledRect.posY_);
    ASSERT_EQ(25, globalScaledRect.width_);
    ASSERT_EQ(20, globalScaledRect.height_);
}

/**
 * @tc.name: RaiseToAppTop01
 * @tc.desc: RaiseToAppTop
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, RaiseToAppTop01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.bundleName_ = "testSession3";
    sptr<SceneSession> scensession = new (std::nothrow) SceneSession(info, nullptr);
    EXPECT_NE(scensession, nullptr);
    auto result = scensession->RaiseToAppTop();
    ASSERT_EQ(result, WSError::WS_OK);

    sptr<SceneSession::SessionChangeCallback> scensessionchangeCallBack =
        new (std::nothrow) SceneSession::SessionChangeCallback();
    EXPECT_NE(scensessionchangeCallBack, nullptr);
    scensession->RegisterSessionChangeCallback(scensessionchangeCallBack);
    result = scensession->RaiseToAppTop();
    ASSERT_EQ(result, WSError::WS_OK);

    NotifyRaiseToTopFunc onRaiseToTop_ = []() {};
    scensessionchangeCallBack->onRaiseToTop_ = onRaiseToTop_;
    result = scensession->RaiseToAppTop();
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdateSessionRect01
 * @tc.desc: UpdateSessionRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, UpdateSessionRect01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.bundleName_ = "testSession3";
    sptr<SceneSession> scensession = new (std::nothrow) SceneSession(info, nullptr);
    EXPECT_NE(scensession, nullptr);
    WSRect rect = {0, 0, 320, 240}; // width: 320, height: 240
    auto result = scensession->UpdateSessionRect(rect, SizeChangeReason::RESIZE);
    ASSERT_EQ(result, WSError::WS_OK);

    sptr<SceneSession::SessionChangeCallback> scensessionchangeCallBack =
        new (std::nothrow) SceneSession::SessionChangeCallback();
    EXPECT_NE(scensessionchangeCallBack, nullptr);
    scensession->RegisterSessionChangeCallback(scensessionchangeCallBack);
    result = scensession->UpdateSessionRect(rect, SizeChangeReason::RESIZE);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: OnSessionEvent01
 * @tc.desc: OnSessionEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, OnSessionEvent01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.bundleName_ = "testSession3";
    sptr<SceneSession> scensession = new (std::nothrow) SceneSession(info, nullptr);
    EXPECT_NE(scensession, nullptr);
    auto result = scensession->OnSessionEvent(SessionEvent::EVENT_MINIMIZE);
    ASSERT_EQ(result, WSError::WS_OK);

    sptr<SceneSession::SessionChangeCallback> scensessionchangeCallBack =
        new (std::nothrow) SceneSession::SessionChangeCallback();
    EXPECT_NE(scensessionchangeCallBack, nullptr);
    scensession->RegisterSessionChangeCallback(scensessionchangeCallBack);
    result = scensession->OnSessionEvent(SessionEvent::EVENT_MINIMIZE);
    ASSERT_EQ(result, WSError::WS_OK);

    int resultValue = 0;
    NotifySessionEventFunc onSessionEvent_ = [&resultValue](int32_t eventId, SessionEventParam param)
    { resultValue = 1; };
    scensessionchangeCallBack->OnSessionEvent_ = onSessionEvent_;
    result = scensession->OnSessionEvent(SessionEvent::EVENT_MINIMIZE);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: ConsumeMoveEvent01
 * @tc.desc: ConsumeMoveEvent, abnormal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, ConsumeMoveEvent01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.bundleName_ = "testSession3";
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    sceneSession->moveDragController_ = new MoveDragController(1);
    EXPECT_NE(sceneSession, nullptr);
    ASSERT_TRUE(sceneSession->moveDragController_);
    sceneSession->moveDragController_->InitMoveDragProperty();
    WSRect originalRect = { 100, 100, 1000, 1000 };

    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    auto result = sceneSession->moveDragController_->ConsumeMoveEvent(pointerEvent, originalRect);
    ASSERT_FALSE(result);

    pointerEvent = MMI::PointerEvent::Create();
    ASSERT_TRUE(pointerEvent);
    pointerEvent->SetPointerId(1);
    sceneSession->moveDragController_->moveDragProperty_.pointerId_ = 0;
    result = sceneSession->moveDragController_->ConsumeMoveEvent(pointerEvent, originalRect);
    ASSERT_FALSE(result);

    pointerEvent->SetPointerId(0);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    pointerEvent->SetButtonId(MMI::PointerEvent::MOUSE_BUTTON_RIGHT);
    result = sceneSession->moveDragController_->ConsumeMoveEvent(pointerEvent, originalRect);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: ConsumeMoveEvent02
 * @tc.desc: ConsumeMoveEvent, normal secne
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, ConsumeMoveEvent02, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.bundleName_ = "testSession3";
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->moveDragController_ = new MoveDragController(1);
    ASSERT_TRUE(sceneSession->moveDragController_);
    sceneSession->moveDragController_->InitMoveDragProperty();
    WSRect originalRect = { 100, 100, 1000, 1000 };
    sceneSession->moveDragController_->isStartMove_ = true;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_TRUE(pointerEvent);
    pointerEvent->SetAgentWindowId(1);
    pointerEvent->SetPointerId(0);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(0);
    pointerEvent->AddPointerItem(pointerItem);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    pointerItem.SetDisplayX(115);
    pointerItem.SetDisplayY(500);
    pointerItem.SetWindowX(15);
    pointerItem.SetWindowY(400);
    auto result = sceneSession->moveDragController_->ConsumeMoveEvent(pointerEvent, originalRect);
    ASSERT_EQ(result, true);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    pointerItem.SetDisplayX(145);
    pointerItem.SetDisplayY(550);
    result = sceneSession->moveDragController_->ConsumeMoveEvent(pointerEvent, originalRect);
    ASSERT_EQ(result, true);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    pointerItem.SetDisplayX(175);
    pointerItem.SetDisplayY(600);
    result = sceneSession->moveDragController_->ConsumeMoveEvent(pointerEvent, originalRect);
    ASSERT_EQ(result, true);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UP);
    pointerItem.SetDisplayX(205);
    pointerItem.SetDisplayY(650);
    result = sceneSession->moveDragController_->ConsumeMoveEvent(pointerEvent, originalRect);
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: ConsumeDragEvent01
 * @tc.desc: ConsumeDragEvent, abnormal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, ConsumeDragEvent01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.bundleName_ = "testSession3";
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->moveDragController_ = new MoveDragController(1);
    ASSERT_TRUE(sceneSession->moveDragController_);
    sceneSession->moveDragController_->InitMoveDragProperty();
    WSRect originalRect = { 100, 100, 1000, 1000 };
    SystemSessionConfig sessionConfig;

    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    sptr<WindowSessionProperty> property = nullptr;
    auto result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property,
        sessionConfig);
    ASSERT_EQ(result, false);

    pointerEvent = MMI::PointerEvent::Create();
    ASSERT_TRUE(pointerEvent);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UP);
    property = new WindowSessionProperty();
    sceneSession->moveDragController_->isStartDrag_ = false;
    result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property, sessionConfig);
    ASSERT_EQ(result, false);

    pointerEvent->SetPointerId(1);
    sceneSession->moveDragController_->moveDragProperty_.pointerId_ = 0;
    sceneSession->moveDragController_->isStartDrag_ = true;
    result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property, sessionConfig);
    ASSERT_EQ(result, false);

    pointerEvent->SetPointerId(0);
    result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property, sessionConfig);
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: ConsumeDragEvent02
 * @tc.desc: ConsumeDragEvent, normal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, ConsumeDragEvent02, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.bundleName_ = "testSession3";
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    sceneSession->moveDragController_ = new MoveDragController(1);
    ASSERT_TRUE(sceneSession->moveDragController_);
    sceneSession->moveDragController_->InitMoveDragProperty();
    WSRect originalRect = { 100, 100, 1000, 1000 };
    sptr<WindowSessionProperty> property = new WindowSessionProperty();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SystemSessionConfig sessionConfig;
    sessionConfig.isSystemDecorEnable_ = true;
    sessionConfig.backgroundswitch = true;
    sessionConfig.decorWindowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_TRUE(pointerEvent);
    pointerEvent->SetAgentWindowId(1);
    pointerEvent->SetPointerId(0);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    sceneSession->moveDragController_->moveDragProperty_.pointerId_ = pointerEvent->GetPointerId();
    sceneSession->moveDragController_->moveDragProperty_.pointerType_ = pointerEvent->GetSourceType();
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(0);
    pointerEvent->AddPointerItem(pointerItem);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    pointerItem.SetDisplayX(100);
    pointerItem.SetDisplayY(100);
    pointerItem.SetWindowX(0);
    pointerItem.SetWindowY(0);
    auto result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property,
        sessionConfig);
    ASSERT_EQ(result, true);

    sceneSession->moveDragController_->aspectRatio_ = 0.0f;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    pointerItem.SetDisplayX(150);
    pointerItem.SetDisplayY(150);
    result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property, sessionConfig);
    ASSERT_EQ(result, true);

    sceneSession->moveDragController_->aspectRatio_ = 1.0f;
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    pointerItem.SetDisplayX(200);
    pointerItem.SetDisplayY(200);
    result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property, sessionConfig);
    ASSERT_EQ(result, true);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UP);
    pointerItem.SetDisplayX(250);
    pointerItem.SetDisplayY(250);
    result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property, sessionConfig);
    ASSERT_EQ(result, true);
}

/**
 * @tc.name: ConsumeDragEvent03
 * @tc.desc: ConsumeDragEvent, normal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, ConsumeDragEvent03, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.bundleName_ = "testSession3";
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->moveDragController_ = new MoveDragController(1);
    ASSERT_TRUE(sceneSession->moveDragController_);
    sceneSession->moveDragController_->InitMoveDragProperty();
    WSRect originalRect = { 100, 100, 1000, 1000 };
    sptr<WindowSessionProperty> property = new WindowSessionProperty();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SystemSessionConfig sessionConfig;
    sessionConfig.isSystemDecorEnable_ = true;
    sessionConfig.backgroundswitch = true;
    sessionConfig.decorWindowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_TRUE(pointerEvent);
    pointerEvent->SetAgentWindowId(1);
    pointerEvent->SetPointerId(0);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    sceneSession->moveDragController_->moveDragProperty_.pointerId_ = pointerEvent->GetPointerId();
    sceneSession->moveDragController_->moveDragProperty_.pointerType_ = pointerEvent->GetSourceType();
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(0);
    pointerEvent->AddPointerItem(pointerItem);

    // LEFT_TOP
    pointerItem.SetWindowX(0);
    pointerItem.SetWindowY(0);
    auto result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property,
        sessionConfig);
    ASSERT_EQ(result, true);

    // RIGHT_TOP
    pointerItem.SetWindowX(1000);
    pointerItem.SetWindowY(0);
    result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property, sessionConfig);
    ASSERT_EQ(result, true);

    // RIGHT_BOTTOM
    pointerItem.SetWindowX(1000);
    pointerItem.SetWindowY(1000);
    result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property, sessionConfig);
    ASSERT_EQ(result, true);

    // LEFT_BOTTOM
    pointerItem.SetWindowX(0);
    pointerItem.SetWindowY(1000);
    result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property, sessionConfig);
    ASSERT_EQ(result, true);
}

/**
 * @tc.name: ConsumeDragEvent04
 * @tc.desc: ConsumeDragEvent, normal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, ConsumeDragEvent04, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.bundleName_ = "testSession3";
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->moveDragController_ = new MoveDragController(1);
    ASSERT_TRUE(sceneSession->moveDragController_);
    sceneSession->moveDragController_->InitMoveDragProperty();
    WSRect originalRect = { 100, 100, 1000, 1000 };
    sptr<WindowSessionProperty> property = new WindowSessionProperty();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SystemSessionConfig sessionConfig;
    sessionConfig.isSystemDecorEnable_ = true;
    sessionConfig.backgroundswitch = true;
    sessionConfig.decorWindowModeSupportType_ = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_TRUE(pointerEvent);
    pointerEvent->SetAgentWindowId(1);
    pointerEvent->SetPointerId(0);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    sceneSession->moveDragController_->moveDragProperty_.pointerId_ = pointerEvent->GetPointerId();
    sceneSession->moveDragController_->moveDragProperty_.pointerType_ = pointerEvent->GetSourceType();
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(0);
    pointerEvent->AddPointerItem(pointerItem);

    // LEFT
    pointerItem.SetWindowX(0);
    pointerItem.SetWindowY(500);
    auto result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property,
        sessionConfig);
    ASSERT_EQ(result, true);

    // TOP
    pointerItem.SetWindowX(500);
    pointerItem.SetWindowY(0);
    result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property, sessionConfig);
    ASSERT_EQ(result, true);

    // RIGHT
    pointerItem.SetWindowX(1000);
    pointerItem.SetWindowY(500);
    result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property, sessionConfig);
    ASSERT_EQ(result, true);

    // BOTTOM
    pointerItem.SetWindowX(500);
    pointerItem.SetWindowY(1000);
    result = sceneSession->moveDragController_->ConsumeDragEvent(pointerEvent, originalRect, property, sessionConfig);
    ASSERT_EQ(result, true);
}

/**
 * @tc.name: GetWindowId01
 * @tc.desc: GetWindowId, normal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetWindowId, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    ASSERT_EQ(0, session_->GetWindowId());
}

/**
 * @tc.name: GetRSVisible01
 * @tc.desc: GetRSVisible, normal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetRSVisible, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    ASSERT_EQ(WSError::WS_OK, session_->SetRSVisible(false));
    session_->state_ = SessionState::STATE_CONNECT;
    if (!session_->GetRSVisible()) {
        ASSERT_EQ(false, session_->GetRSVisible());
    }
}

/**
 * @tc.name: SetFocusable01
 * @tc.desc: SetFocusable, normal scene
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetFocusable, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: GetSnapshot
 * @tc.desc: GetSnapshot Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetSnapshot, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    std::shared_ptr<Media::PixelMap> snapshot = session_->Snapshot();
    ASSERT_EQ(snapshot, session_->GetSnapshot());
}

/**
 * @tc.name: NotifyExtensionDied
 * @tc.desc: NotifyExtensionDied Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, NotifyExtensionDied, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->NotifyExtensionDied();

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: NotifyExtensionTimeout
 * @tc.desc: NotifyExtensionTimeout Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, NotifyExtensionTimeout, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->NotifyExtensionTimeout(3);

    session_->RegisterLifecycleListener(lifecycleListener_);
    session_->NotifyExtensionTimeout(3);
    session_->UnregisterLifecycleListener(lifecycleListener_);

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: SetAspectRatio
 * @tc.desc: SetAspectRatio Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetAspectRatio, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_EQ(WSError::WS_OK, session_->SetAspectRatio(0.1f));
}

/**
 * @tc.name: UpdateSessionTouchable
 * @tc.desc: UpdateSessionTouchable Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, UpdateSessionTouchable, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->UpdateSessionTouchable(false);

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: SetFocusable02
 * @tc.desc: others
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetFocusable02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->state_ = SessionState::STATE_FOREGROUND;
    session_->sessionInfo_.isSystem_ = false;

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(true));
}

/**
 * @tc.name: GetFocusable01
 * @tc.desc: property_ is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetFocusable01, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->property_ = new WindowSessionProperty();
    ASSERT_EQ(true, session_->GetFocusable());
}

/**
 * @tc.name: GetFocusable02
 * @tc.desc: property_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetFocusable02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->property_ = nullptr;
    ASSERT_EQ(true, session_->GetFocusable());
}

/**
 * @tc.name: SetNeedNotify
 * @tc.desc: SetNeedNotify Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetNeedNotify, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->SetNeedNotify(false);

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: NeedNotify
 * @tc.desc: NeedNotify Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, NeedNotify, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->SetNeedNotify(true);
    ASSERT_EQ(true, session_->NeedNotify());
}

/**
 * @tc.name: SetFocusedOnShow
 * @tc.desc: SetFocusedOnShow Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetFocusedOnShow, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetFocusedOnShow(false);
    auto focusedOnShow = session_->IsFocusedOnShow();
    ASSERT_EQ(focusedOnShow, false);
    session_->SetFocusedOnShow(true);
    focusedOnShow = session_->IsFocusedOnShow();
    ASSERT_EQ(focusedOnShow, true);
}

/**
 * @tc.name: SetTouchable01
 * @tc.desc: IsSessionValid() return false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetTouchable01, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->sessionInfo_.isSystem_ = true;
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, session_->SetTouchable(false));
}

/**
 * @tc.name: SetTouchable02
 * @tc.desc: IsSessionValid() return true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetTouchable02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_FOREGROUND;
    session_->sessionInfo_.isSystem_ = false;
    ASSERT_EQ(WSError::WS_OK, session_->SetTouchable(false));
}

/**
 * @tc.name: SetSessionInfoLockedState01
 * @tc.desc: IsSessionValid() return false
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetSessionInfoLockedState01, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionInfoLockedState(false);
    ASSERT_EQ(false, session_->sessionInfo_.lockedState);
}

/**
 * @tc.name: SetSessionInfoLockedState02
 * @tc.desc: IsSessionValid() return true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetSessionInfoLockedState02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionInfoLockedState(true);
    ASSERT_EQ(true, session_->sessionInfo_.lockedState);
}

/**
 * @tc.name: GetCallingPid
 * @tc.desc: GetCallingPid Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetCallingPid, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetCallingPid(111);
    ASSERT_EQ(111, session_->GetCallingPid());
}

/**
 * @tc.name: GetCallingUid
 * @tc.desc: GetCallingUid Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetCallingUid, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetCallingUid(111);
    ASSERT_EQ(111, session_->GetCallingUid());
}

/**
 * @tc.name: GetAbilityToken
 * @tc.desc: GetAbilityToken Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetAbilityToken, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetAbilityToken(nullptr);
    ASSERT_EQ(nullptr, session_->GetAbilityToken());
}

/**
 * @tc.name: SetBrightness01
 * @tc.desc: property_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetBrightness01, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->property_ = nullptr;
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->SetBrightness(0.1f));
}

/**
 * @tc.name: SetBrightness02
 * @tc.desc: property_ is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetBrightness02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->property_ = new WindowSessionProperty();
    ASSERT_EQ(WSError::WS_OK, session_->SetBrightness(0.1f));
}

/**
 * @tc.name: UpdateHotRect
 * @tc.desc: UpdateHotRect Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, UpdateHotRect, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    WSRect rect;
    rect.posX_ = 0;
    rect.posY_ = 0;
    rect.width_ = 0;
    rect.height_ = 0;

    WSRectF newRect;
    const float outsideBorder = 4.0f * 1.5f;
    const size_t outsideBorderCount = 2;
    newRect.posX_ = rect.posX_ - outsideBorder;
    newRect.posY_ = rect.posY_ - outsideBorder;
    newRect.width_ = rect.width_ + outsideBorder * outsideBorderCount;
    newRect.height_ = rect.height_ + outsideBorder * outsideBorderCount;

    ASSERT_EQ(newRect, session_->UpdateHotRect(rect));
}

/**
 * @tc.name: SetTerminateSessionListener
 * @tc.desc: SetTerminateSessionListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetTerminateSessionListener, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->SetTerminateSessionListener(nullptr);

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: SetTerminateSessionListenerTotal
 * @tc.desc: SetTerminateSessionListenerTotal Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetTerminateSessionListenerTotal, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->SetTerminateSessionListenerTotal(nullptr);

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: SetSessionLabel
 * @tc.desc: SetSessionLabel Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetSessionLabel, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    NofitySessionLabelUpdatedFunc func = [](const std::string& label) {};
    session_->updateSessionLabelFunc_ = func;
    ASSERT_EQ(WSError::WS_OK, session_->SetSessionLabel("SetSessionLabel Test"));
}

/**
 * @tc.name: SetUpdateSessionLabelListener
 * @tc.desc: SetUpdateSessionLabelListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetUpdateSessionLabelListener, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    NofitySessionLabelUpdatedFunc func = nullptr;
    session_->SetUpdateSessionLabelListener(func);

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: SetPendingSessionToForegroundListener
 * @tc.desc: SetPendingSessionToForegroundListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetPendingSessionToForegroundListener, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->SetPendingSessionToForegroundListener(nullptr);

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: NotifyScreenshot
 * @tc.desc: NotifyScreenshot Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, NotifyScreenshot, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->sessionStage_ = nullptr;
    session_->NotifyScreenshot();

    session_->sessionStage_ = mockSessionStage_;
    session_->NotifyScreenshot();

    session_->property_ = new WindowSessionProperty();
    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: TransferBackPressedEventForConsumed02
 * @tc.desc: windowEventChannel_ is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, TransferBackPressedEventForConsumed02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->windowEventChannel_ = new TestWindowEventChannel();

    bool isConsumed = false;
    ASSERT_EQ(WSError::WS_OK, session_->TransferBackPressedEventForConsumed(isConsumed));
}

/**
 * @tc.name: GetUIContentRemoteObj
 * @tc.desc: GetUIContentRemoteObj Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetUIContentRemoteObj, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    sptr<SessionStageMocker> mockSessionStage = new(std::nothrow) SessionStageMocker();
    ASSERT_NE(mockSessionStage, nullptr);
    EXPECT_CALL(*(mockSessionStage), GetUIContentRemoteObj(_)).WillRepeatedly(Return(WSError::WS_OK));
    session_->sessionStage_ = mockSessionStage;
    session_->state_ = SessionState::STATE_FOREGROUND;
    sptr<IRemoteObject> remoteObj;
    ASSERT_EQ(WSError::WS_OK, session_->GetUIContentRemoteObj(remoteObj));

    session_->state_ = SessionState::STATE_BACKGROUND;
    ASSERT_EQ(WSError::WS_OK, session_->GetUIContentRemoteObj(remoteObj));
    Mock::VerifyAndClearExpectations(&mockSessionStage);

    session_->sessionInfo_.isSystem_ = true;
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, session_->GetUIContentRemoteObj(remoteObj));
}

/**
 * @tc.name: TransferKeyEventForConsumed02
 * @tc.desc: windowEventChannel_ is not nullptr, keyEvent is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, TransferKeyEventForConsumed02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->windowEventChannel_ = new TestWindowEventChannel();

    std::shared_ptr<MMI::KeyEvent> keyEvent = nullptr;
    bool isConsumed = false;
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->TransferKeyEventForConsumed(keyEvent, isConsumed));
}

/**
 * @tc.name: TransferKeyEventForConsumed03
 * @tc.desc: windowEventChannel_ is not nullptr, keyEvent is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, TransferKeyEventForConsumed03, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->windowEventChannel_ = new TestWindowEventChannel();

    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    bool isConsumed = false;
    ASSERT_EQ(WSError::WS_OK, session_->TransferKeyEventForConsumed(keyEvent, isConsumed));
}

/**
 * @tc.name: TransferFocusActiveEvent02
 * @tc.desc: windowEventChannel_ is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, TransferFocusActiveEvent02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->windowEventChannel_ = new TestWindowEventChannel();

    ASSERT_EQ(WSError::WS_OK, session_->TransferFocusActiveEvent(false));
}

/**
 * @tc.name: TransferFocusStateEvent02
 * @tc.desc: windowEventChannel_ is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, TransferFocusStateEvent02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->windowEventChannel_ = new TestWindowEventChannel();

    ASSERT_EQ(WSError::WS_OK, session_->TransferFocusStateEvent(false));
}

/**
 * @tc.name: SetCompatibleModeInPc
 * @tc.desc: SetCompatibleModeInPc test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetCompatibleModeInPc, Function | SmallTest | Level2)
{
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    bool enable = true;
    bool isSupportDragInPcCompatibleMode = true;
    property->SetCompatibleModeInPc(enable);
    ASSERT_EQ(property->GetCompatibleModeInPc(), true);
    property->SetIsSupportDragInPcCompatibleMode(isSupportDragInPcCompatibleMode);;
    ASSERT_EQ(property->GetIsSupportDragInPcCompatibleMode(), true);
}

/**
 * @tc.name: UpdateMaximizeMode
 * @tc.desc: UpdateMaximizeMode test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, UpdateMaximizeMode, Function | SmallTest | Level2)
{
    sptr<SessionStageMocker> mockSessionStage = new (std::nothrow) SessionStageMocker();
    EXPECT_NE(mockSessionStage, nullptr);
    session_->sessionStage_ = mockSessionStage;

    session_->sessionInfo_.isSystem_ = false;
    session_->state_ = SessionState::STATE_ACTIVE;
    auto ret = session_->UpdateMaximizeMode(true);
    ASSERT_EQ(ret, WSError::WS_OK);

    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    session_->SetSessionProperty(property);
    ret = session_->UpdateMaximizeMode(false);
    ASSERT_EQ(ret, WSError::WS_OK);

    session_->SetSessionProperty(nullptr);
    ret = session_->UpdateMaximizeMode(false);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateTitleInTargetPos
 * @tc.desc: UpdateTitleInTargetPos test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, UpdateTitleInTargetPos, Function | SmallTest | Level2)
{
    sptr<SessionStageMocker> mockSessionStage = new (std::nothrow) SessionStageMocker();
    EXPECT_NE(mockSessionStage, nullptr);
    session_->sessionStage_ = mockSessionStage;

    session_->sessionInfo_.isSystem_ = false;
    session_->state_ = SessionState::STATE_FOREGROUND;
    auto ret = session_->UpdateTitleInTargetPos(true, 20);
    ASSERT_NE(ret, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: SwitchFreeMultiWindow
 * @tc.desc: SwitchFreeMultiWindow test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SwitchFreeMultiWindow, Function | SmallTest | Level2)
{
    sptr<SessionStageMocker> mockSessionStage = new (std::nothrow) SessionStageMocker();
    EXPECT_NE(mockSessionStage, nullptr);
    session_->sessionStage_ = mockSessionStage;

    session_->sessionInfo_.isSystem_ = false;
    session_->state_ = SessionState::STATE_FOREGROUND;
    auto ret = session_->SwitchFreeMultiWindow(true);
    ASSERT_NE(ret, WSError::WS_ERROR_INVALID_SESSION);

    session_->sessionInfo_.isSystem_ = true;
    ret = session_->SwitchFreeMultiWindow(true);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: SetTouchHotAreas
 * @tc.desc: SetTouchHotAreas test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetTouchHotAreas, Function | SmallTest | Level2)
{
    session_->SetSessionProperty(nullptr);
    std::vector<Rect> touchHotAreas;
    session_->SetTouchHotAreas(touchHotAreas);
    ASSERT_EQ(session_->property_, nullptr);

    session_->property_ = new WindowSessionProperty();
    touchHotAreas = session_->property_->touchHotAreas_;
    session_->property_->SetTouchHotAreas(touchHotAreas);
    ASSERT_EQ(touchHotAreas, session_->property_->touchHotAreas_);
}

/**
 * @tc.name: NotifyOccupiedAreaChangeInfo
 * @tc.desc: NotifyOccupiedAreaChangeInfo test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, NotifyOccupiedAreaChangeInfo, Function | SmallTest | Level2)
{
    sptr<SessionStageMocker> mockSessionStage = new (std::nothrow) SessionStageMocker();
    EXPECT_NE(mockSessionStage, nullptr);
    session_->sessionStage_ = mockSessionStage;
    session_->NotifyOccupiedAreaChangeInfo(nullptr, nullptr);
    EXPECT_NE(session_->sessionStage_, nullptr);
}

/**
 * @tc.name: ProcessBackEvent
 * @tc.desc: ProcessBackEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, ProcessBackEvent, Function | SmallTest | Level2)
{
    sptr<SessionStageMocker> mockSessionStage = new (std::nothrow) SessionStageMocker();
    EXPECT_NE(mockSessionStage, nullptr);
    session_->sessionStage_ = mockSessionStage;

    session_->sessionInfo_.isSystem_ = false;
    session_->state_ = SessionState::STATE_FOREGROUND;
    auto ret = session_->ProcessBackEvent();
    ASSERT_NE(ret, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: ProcessBackGetAndSetSessionRequestRectEvent
 * @tc.desc: GetSessionRequestRectEvent, SetSessionRequestRectEvent test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, GetAndSetSessionRequestRect, Function | SmallTest | Level2)
{
    session_->SetSessionProperty(nullptr);
    session_->GetSessionRequestRect();
    ASSERT_EQ(session_->property_, nullptr);

    WSRect rect = {0, 0, 0, 0};
    session_->SetSessionRequestRect(rect);
    ASSERT_EQ(session_->property_, nullptr);
}

/**
 * @tc.name: SetSessionRect01
 * @tc.desc: SetSessionRect test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetSessionRect01, Function | SmallTest | Level2)
{
    WSRect rect = session_->GetSessionRect();
    session_->SetSessionRect(rect);
    ASSERT_EQ(rect, session_->winRect_);
}

/**
 * @tc.name: SetExclusivelyHighlighted
 * @tc.desc: SetExclusivelyHighlighted Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, SetExclusivelyHighlighted, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetExclusivelyHighlighted(false);
    bool isExclusivelyHighlighted = session_->GetSessionProperty()->GetExclusivelyHighlighted();
    ASSERT_EQ(isExclusivelyHighlighted, false);
    session_->SetExclusivelyHighlighted(true);
    isExclusivelyHighlighted = session_->GetSessionProperty()->GetExclusivelyHighlighted();
    ASSERT_EQ(isExclusivelyHighlighted, true);
}
 
/**
 * @tc.name: UpdateHighlightStatus
 * @tc.desc: UpdateHighlightStatus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, UpdateHighlightStatus, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    EXPECT_EQ(session_->UpdateHighlightStatus(false, false), WSError::WS_DO_NOTHING);
 
    EXPECT_EQ(session_->UpdateHighlightStatus(true, false), WSError::WS_OK);
    session_->isHighlight_ = false;
    EXPECT_EQ(session_->UpdateHighlightStatus(true, true), WSError::WS_OK);
}
 
/**
 * @tc.name: NotifyHighlightChange
 * @tc.desc: NotifyHighlightChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest, NotifyHighlightChange, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->sessionInfo_.isSystem_ = true;
    EXPECT_EQ(session_->NotifyHighlightChange(true), WSError::WS_ERROR_INVALID_SESSION);
    session_->sessionInfo_.isSystem_ = false;
    EXPECT_EQ(session_->NotifyHighlightChange(true), WSError::WS_ERROR_NULLPTR);
    session_->sessionStage_ = mockSessionStage_;
    session_->state_ = SessionState::STATE_CONNECT;
    EXPECT_EQ(session_->NotifyHighlightChange(true), WSError::WS_OK);
    session_->sessionStage_ = nullptr;
    EXPECT_EQ(session_->NotifyHighlightChange(true), WSError::WS_ERROR_NULLPTR);
}
}
} // namespace Rosen
} // namespace OHOS
