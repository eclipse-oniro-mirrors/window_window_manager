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

#include "key_event.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "mock/mock_pattern_detach_callback.h"
#include "session/host/include/extension_session.h"
#include "session/host/include/move_drag_controller.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/session.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/screen/include/screen_session.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string UNDEFINED = "undefined";
}

class WindowSessionTest3 : public testing::Test {
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
};

void WindowSessionTest3::SetUpTestCase()
{
}

void WindowSessionTest3::TearDownTestCase()
{
}

void WindowSessionTest3::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    session_ = sptr<Session>::MakeSptr(info);
    EXPECT_NE(nullptr, session_);
    session_->surfaceNode_ = CreateRSSurfaceNode();
    ssm_ = sptr<SceneSessionManager>::MakeSptr();
    session_->SetEventHandler(ssm_->taskScheduler_->GetEventHandler(), ssm_->eventHandler_);
    auto isScreenLockedCallback = [this]() {
        return ssm_->IsScreenLocked();
    };
    session_->RegisterIsScreenLockedCallback(isScreenLockedCallback);
}

void WindowSessionTest3::TearDown()
{
    session_ = nullptr;
    usleep(WAIT_SYNC_IN_NS);
}

RSSurfaceNode::SharedPtr WindowSessionTest3::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTest3SurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    if (surfaceNode == nullptr) {
        GTEST_LOG_(INFO) << "WindowSessionTest3::CreateRSSurfaceNode surfaceNode is nullptr";
    }
    return surfaceNode;
}

int32_t WindowSessionTest3::GetTaskCount()
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
 * @tc.name: NotifyContextTransparent
 * @tc.desc: NotifyContextTransparent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifyContextTransparent, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    NotifyContextTransparentFunc contextTransparentFunc = session_->contextTransparentFunc_;
    if (contextTransparentFunc == nullptr) {
        contextTransparentFunc = {};
    }
    session_->contextTransparentFunc_ = nullptr;
    session_->NotifyContextTransparent();

    session_->SetContextTransparentFunc(contextTransparentFunc);
    session_->NotifyContextTransparent();
    session_->SetPendingSessionToBackgroundForDelegatorListener(nullptr);
    EXPECT_EQ(WSError::WS_OK, session_->PendingSessionToBackgroundForDelegator(true));
}

/**
 * @tc.name: SetFocusable04
 * @tc.desc: SetFocusable Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetFocusable04, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    auto result = session_->SetFocusable(false);
    ASSERT_EQ(result, WSError::WS_OK);

    session_->isFocused_ = true;
    session_->property_->SetFocusable(false);

    result = session_->SetFocusable(false);
    EXPECT_EQ(result, WSError::WS_OK);
    EXPECT_EQ(session_->GetFocusable(), false);
}

/**
 * @tc.name: SetSystemFocusable
 * @tc.desc: SetSystemFocusable Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetSystemFocusable, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    ASSERT_EQ(session_->GetSystemFocusable(), true);
    bool systemFocusable = false;
    session_->SetSystemFocusable(systemFocusable);
    ASSERT_EQ(session_->GetSystemFocusable(), systemFocusable);
}

/**
 * @tc.name: SetFocusableOnShow
 * @tc.desc: SetFocusableOnShow Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetFocusableOnShow, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    ASSERT_EQ(session_->IsFocusableOnShow(), true);
    bool focusableOnShow = false;
    session_->SetFocusableOnShow(focusableOnShow);
    usleep(10000); // sleep 10ms
    ASSERT_EQ(session_->IsFocusableOnShow(), focusableOnShow);
}

/**
 * @tc.name: CheckFocusable
 * @tc.desc: CheckFocusable Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, CheckFocusable, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(session_->CheckFocusable(), true);
    session_->SetSystemFocusable(false);
    ASSERT_EQ(session_->CheckFocusable(), false);
}

/**
 * @tc.name: SetTouchable03
 * @tc.desc: IsSessionValid() and touchable return true
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetTouchable03, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    session_->sessionInfo_.isSystem_ = false;
    EXPECT_EQ(WSError::WS_OK, session_->SetTouchable(true));
}

/**
 * @tc.name: GetTouchable02
 * @tc.desc: GetTouchable Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, GetTouchable02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    EXPECT_EQ(true, session_->GetTouchable());
}

/**
 * @tc.name: UpdateDensity02
 * @tc.desc: UpdateDensity Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, UpdateDensity02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;
    auto result = session_->UpdateDensity();
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdateOrientation
 * @tc.desc: UpdateOrientation Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, UpdateOrientation, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->sessionInfo_.isSystem_ = true;
    auto result = session_->UpdateOrientation();
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    session_->sessionInfo_.isSystem_ = false;
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    result = session_->UpdateOrientation();
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;
    result = session_->UpdateOrientation();
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetActive
 * @tc.desc: SetActive Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetActive, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_CONNECT);
    auto result = session_->SetActive(false);
    EXPECT_EQ(result, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: SetActive02
 * @tc.desc: SetActive Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetActive02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;
    auto result = session_->SetActive(true);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetActive03
 * @tc.desc: SetActive Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetActive03, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;

    session_->SetSessionState(SessionState::STATE_CONNECT);
    auto result = session_->SetActive(true);
    EXPECT_EQ(result, WSError::WS_OK);

    session_->isActive_ = true;
    result = session_->SetActive(false);
    EXPECT_EQ(result, WSError::WS_OK);

    session_->SetSessionState(SessionState::STATE_ACTIVE);
    session_->isActive_ = true;
    result = session_->SetActive(false);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: PresentFocusIfPointDown
 * @tc.desc: PresentFocusIfPointDown Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, PresentFocusIfPointDown, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->isFocused_ = true;
    session_->PresentFocusIfPointDown();

    session_->isFocused_ = false;
    session_->property_->SetFocusable(false);
    session_->PresentFocusIfPointDown();
    session_->SetPendingSessionToBackgroundForDelegatorListener(nullptr);
    EXPECT_EQ(WSError::WS_OK, session_->PendingSessionToBackgroundForDelegator(true));
}

/**
 * @tc.name: TransferPointerEvent06
 * @tc.desc: TransferPointerEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, TransferPointerEvent06, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_CONNECT);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);

    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info);
    ASSERT_NE(dialogSession, nullptr);
    dialogSession->SetSessionState(SessionState::STATE_ACTIVE);
    session_->dialogVec_.push_back(dialogSession);
    pointerEvent->pointerAction_ = MMI::PointerEvent::POINTER_ACTION_DOWN;

    auto result = session_->TransferPointerEvent(pointerEvent);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: TransferPointerEvent07
 * @tc.desc: TransferPointerEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, TransferPointerEvent07, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_CONNECT);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    auto result = session_->TransferPointerEvent(pointerEvent);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferPointerEvent08
 * @tc.desc: TransferPointerEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, TransferPointerEvent08, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_CONNECT);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info);
    ASSERT_NE(dialogSession, nullptr);

    session_->SetParentSession(dialogSession);
    auto result = session_->TransferPointerEvent(pointerEvent);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferPointerEvent09
 * @tc.desc: TransferPointerEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, TransferPointerEvent09, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info);
    ASSERT_NE(dialogSession, nullptr);

    session_->SetParentSession(dialogSession);
    dialogSession->dialogVec_.push_back(session_);
    pointerEvent->pointerAction_ = MMI::PointerEvent::POINTER_ACTION_MOVE;
    auto result = session_->TransferPointerEvent(pointerEvent);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferPointerEvent10
 * @tc.desc: TransferPointerEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, TransferPointerEvent10, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    pointerEvent->pointerAction_ = MMI::PointerEvent::POINTER_ACTION_DOWN;

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);

    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info);
    sptr<Session> dialogSession2 = sptr<Session>::MakeSptr(info);
    sptr<Session> dialogSession3 = sptr<Session>::MakeSptr(info);
    ASSERT_NE(dialogSession, nullptr);
    ASSERT_NE(dialogSession2, nullptr);
    ASSERT_NE(dialogSession3, nullptr);
    dialogSession2->SetSessionState(SessionState::STATE_FOREGROUND);
    dialogSession3->SetSessionState(SessionState::STATE_ACTIVE);
    dialogSession2->persistentId_ = 9;
    session_->SetParentSession(dialogSession);
    dialogSession->dialogVec_.push_back(dialogSession2);
    dialogSession->dialogVec_.push_back(dialogSession3);
    auto result = session_->TransferPointerEvent(pointerEvent);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferPointerEvent11
 * @tc.desc: TransferPointerEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, TransferPointerEvent11, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);

    session_->property_->SetWindowType(WindowType::APP_WINDOW_BASE);

    session_->windowEventChannel_ = nullptr;
    auto result = session_->TransferPointerEvent(pointerEvent);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferFocusStateEvent03
 * @tc.desc: TransferFocusStateEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, TransferFocusStateEvent03, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->windowEventChannel_ = nullptr;
    session_->sessionInfo_.isSystem_ = true;
    EXPECT_EQ(session_->TransferFocusStateEvent(true), WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: Snapshot
 * @tc.desc: Snapshot Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, Snapshot, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int32_t persistentId = 1424;
    std::string bundleName = "testBundleName";
    session_->scenePersistence_ = sptr<ScenePersistence>::MakeSptr(bundleName, persistentId);
    ASSERT_NE(session_->scenePersistence_, nullptr);
    struct RSSurfaceNodeConfig config;
    session_->surfaceNode_ = RSSurfaceNode::Create(config);
    ASSERT_NE(session_->surfaceNode_, nullptr);
    EXPECT_EQ(nullptr, session_->Snapshot(false, 0.0f));

    session_->bufferAvailable_ = true;
    EXPECT_EQ(nullptr, session_->Snapshot(false, 0.0f));

    session_->surfaceNode_->bufferAvailable_ = true;
    EXPECT_EQ(nullptr, session_->Snapshot(false, 0.0f));

    session_->surfaceNode_ = nullptr;
    EXPECT_EQ(nullptr, session_->Snapshot(false, 0.0f));
}

/**
 * @tc.name: SetBufferAvailableChangeListener
 * @tc.desc: SetBufferAvailableChangeListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetBufferAvailableChangeListener, Function | SmallTest | Level2)
{
    session_->SetSessionState(SessionState::STATE_CONNECT);
    session_->SetSessionStateChangeNotifyManagerListener(nullptr);

    session_->bufferAvailable_ = true;
    session_->SetBufferAvailableChangeListener(nullptr);

    int resultValue = 0;
    NotifyBufferAvailableChangeFunc func = [&resultValue](const bool isAvailable) {
        resultValue += 1;
    };
    session_->SetBufferAvailableChangeListener(func);
    ASSERT_EQ(resultValue, 1);
}

/**
 * @tc.name: NotifySessionFocusableChange
 * @tc.desc: NotifySessionFocusableChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifySessionFocusableChange, Function | SmallTest | Level2)
{
    int resultValue = 0;
    NotifySessionFocusableChangeFunc func = [&resultValue](const bool isFocusable) {
        resultValue += 1;
    };
    session_->SetSessionFocusableChangeListener(func);
    session_->NotifySessionFocusableChange(true);
    ASSERT_EQ(resultValue, 2);
}

/**
 * @tc.name: GetStateFromManager
 * @tc.desc: GetStateFromManager Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, GetStateFromManager, Function | SmallTest | Level2)
{
    ManagerState key = ManagerState{0};
    GetStateFromManagerFunc func = [](const ManagerState key) {
        return true;
    };
    session_->getStateFromManagerFunc_ = func;
    session_->GetStateFromManager(key);

    session_->getStateFromManagerFunc_ = nullptr;
    ASSERT_EQ(false, session_->GetStateFromManager(key));

    // 覆盖default分支
    key = ManagerState{-1};
    ASSERT_EQ(false, session_->GetStateFromManager(key));
}

/**
 * @tc.name: NotifyUIRequestFocus
 * @tc.desc: NotifyUIRequestFocus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifyUIRequestFocus, Function | SmallTest | Level2)
{
    session_->requestFocusFunc_ = []() {};
    session_->NotifyUIRequestFocus();

    ASSERT_NE(session_, nullptr);
}

/**
 * @tc.name: SetCompatibleModeInPc
 * @tc.desc: SetCompatibleModeInPc Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetCompatibleModeInPc, Function | SmallTest | Level2)
{
    auto enable = true;
    auto isSupportDragInPcCompatibleMode = true;
    ASSERT_EQ(WSError::WS_OK, session_->SetCompatibleModeInPc(enable, isSupportDragInPcCompatibleMode));

    ASSERT_EQ(WSError::WS_OK, session_->SetCompatibleModeInPc(enable, isSupportDragInPcCompatibleMode));

    enable = false;
    ASSERT_EQ(WSError::WS_OK, session_->SetCompatibleModeInPc(enable, isSupportDragInPcCompatibleMode));
}

/**
 * @tc.name: NotifySessionTouchableChange
 * @tc.desc: NotifySessionTouchableChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifySessionTouchableChange, Function | SmallTest | Level2)
{
    int resultValue = 0;
    NotifySessionTouchableChangeFunc func = [&resultValue](const bool touchable) {
        resultValue += 1;
    };
    session_->SetSessionTouchableChangeListener(func);
    session_->NotifySessionTouchableChange(true);
    ASSERT_EQ(resultValue, 2);
}

/**
 * @tc.name: NotifyClick
 * @tc.desc: NotifyClick Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifyClick, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int resultValue = 0;
    bool hasRequestFocus = true;
    bool hasIsClick = true;
    NotifyClickFunc func = [&resultValue, &hasRequestFocus, &hasIsClick](bool requestFocus, bool isClick) {
        resultValue = 1;
        hasRequestFocus = requestFocus;
        hasIsClick = isClick;
    };
    session_->SetClickListener(func);
    session_->NotifyClick(false, false);
    EXPECT_EQ(resultValue, 1);
    EXPECT_EQ(hasRequestFocus, false);
    EXPECT_EQ(hasIsClick, false);
}

/**
 * @tc.name: NotifyRequestFocusStatusNotifyManager
 * @tc.desc: NotifyRequestFocusStatusNotifyManager Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifyRequestFocusStatusNotifyManager, Function | SmallTest | Level2)
{
    int resultValue = 0;
    NotifyRequestFocusStatusNotifyManagerFunc func = [&resultValue](int32_t persistentId,
        const bool isFocused, const bool byForeground, FocusChangeReason reason) {
        resultValue += 1;
    };
    session_->SetRequestFocusStatusNotifyManagerListener(func);
    FocusChangeReason reason = FocusChangeReason::SCB_SESSION_REQUEST;
    session_->NotifyRequestFocusStatusNotifyManager(true, false, reason);
    EXPECT_EQ(resultValue, 1);
}

/**
 * @tc.name: PresentFoucusIfNeed
 * @tc.desc: PresentFoucusIfNeed Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, PresentFoucusIfNeed, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int32_t pointerAction = MMI::PointerEvent::POINTER_ACTION_DOWN;
    session_->PresentFoucusIfNeed(pointerAction);
    pointerAction = MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN;
    session_->PresentFoucusIfNeed(pointerAction);
    session_->property_->focusable_ = false;
    session_->PresentFoucusIfNeed(pointerAction);
    session_->isFocused_ = true;
    session_->PresentFoucusIfNeed(pointerAction);
    EXPECT_EQ(true, session_->CheckPointerEventDispatch(nullptr));
}

/**
 * @tc.name: UpdateFocus03
 * @tc.desc: UpdateFocus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, UpdateFocus03, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->isFocused_ = true;
    EXPECT_EQ(WSError::WS_OK, session_->UpdateFocus(false));
}

/**
 * @tc.name: NotifyFocusStatus
 * @tc.desc: NotifyFocusStatus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifyFocusStatus, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_CONNECT;
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;
    EXPECT_EQ(WSError::WS_OK, session_->NotifyFocusStatus(true));
    session_->sessionStage_ = nullptr;
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, session_->NotifyFocusStatus(true));
}

/**
 * @tc.name: RequestFocus
 * @tc.desc: RequestFocus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, RequestFocus, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_FOREGROUND;
    session_->sessionInfo_.isSystem_ = false;
    EXPECT_EQ(WSError::WS_OK, session_->RequestFocus(true));
    EXPECT_EQ(WSError::WS_OK, session_->RequestFocus(false));
}

/**
 * @tc.name: UpdateWindowMode
 * @tc.desc: UpdateWindowMode Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, UpdateWindowMode, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;

    session_->state_ = SessionState::STATE_END;
    auto result = session_->UpdateWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    session_->state_ = SessionState::STATE_DISCONNECT;
    result = session_->UpdateWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    EXPECT_EQ(session_->property_->windowMode_, WindowMode::WINDOW_MODE_UNDEFINED);
    EXPECT_EQ(session_->property_->isNeedUpdateWindowMode_, true);
    EXPECT_EQ(result, WSError::WS_OK);

    session_->state_ = SessionState::STATE_CONNECT;
    result = session_->UpdateWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(session_->property_->windowMode_, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(session_->property_->maximizeMode_, MaximizeMode::MODE_RECOVER);
    EXPECT_EQ(result, WSError::WS_OK);

    session_->state_ = SessionState::STATE_CONNECT;
    result = session_->UpdateWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(result, WSError::WS_OK);

    session_->state_ = SessionState::STATE_CONNECT;
    result = session_->UpdateWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    EXPECT_EQ(result, WSError::WS_OK);

    session_->sessionStage_ = nullptr;
    result = session_->UpdateWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: RectSizeCheckProcess
 * @tc.desc: RectSizeCheckProcess Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, RectSizeCheckProcess, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->RectSizeCheckProcess(1, 0, 2, 0, 0);
    session_->RectSizeCheckProcess(1, 0, 1, 0, 0);
    session_->RectSizeCheckProcess(0, 1, 0, 2, 0);
    session_->RectSizeCheckProcess(0, 1, 0, 0, 0);
    EXPECT_EQ(true, session_->CheckPointerEventDispatch(nullptr));
}

/**
 * @tc.name: RectCheckProcess
 * @tc.desc: RectCheckProcess Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, RectCheckProcess, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->isVisible_ = true;
    session_->RectCheckProcess();

    session_->state_ = SessionState::STATE_FOREGROUND;
    session_->RectCheckProcess();
    EXPECT_EQ(true, session_->CheckPointerEventDispatch(nullptr));
}

/**
 * @tc.name: RectCheckProcess01
 * @tc.desc: RectCheckProcess01 Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, RectCheckProcess01, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_INACTIVE;
    session_->isVisible_ = false;
    session_->RectCheckProcess();

    session_->state_ = SessionState::STATE_ACTIVE;
    session_->isVisible_ = true;
    session_->RectCheckProcess();

    session_->property_->displayId_ = 0;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(0, ScreenProperty(), 0);
    ASSERT_NE(screenSession, nullptr);
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    ASSERT_NE(&screenProperty, nullptr);
    screenSession->screenId_ = 0;
    screenSession->SetVirtualPixelRatio(0.0f);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(0, screenSession));
    session_->RectCheckProcess();

    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    screenSession->SetVirtualPixelRatio(1.0f);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(0, screenSession));
    session_->RectCheckProcess();

    WSRect rect = {0, 0, 0, 0};
    session_->winRect_ = rect;
    session_->RectCheckProcess();

    session_->winRect_.height_ = 200;
    session_->RectCheckProcess();

    session_->aspectRatio_ = 0.0f;
    session_->RectCheckProcess();

    session_->aspectRatio_ = 0.5f;
    session_->RectCheckProcess();

    session_->winRect_.width_ = 200;
    session_->RectCheckProcess();

    session_->aspectRatio_ = 1.0f;
    session_->RectCheckProcess();

    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
}

/**
 * @tc.name: SetAcquireRotateAnimationConfigFunc
 * @tc.desc: SetAcquireRotateAnimationConfigFunc Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetAcquireRotateAnimationConfigFunc, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetAcquireRotateAnimationConfigFunc(nullptr);
    ASSERT_EQ(session_->acquireRotateAnimationConfigFunc_, nullptr);
    int32_t duration = session_->GetRotateAnimationDuration();
    ASSERT_EQ(duration, ROTATE_ANIMATION_DURATION);

    AcquireRotateAnimationConfigFunc func = [](RotateAnimationConfig& config) {
        config.duration_ = 800;
    };
    session_->SetAcquireRotateAnimationConfigFunc(func);
    ASSERT_NE(session_->acquireRotateAnimationConfigFunc_, nullptr);
    duration = session_->GetRotateAnimationDuration();
    ASSERT_EQ(duration, 800);
}

/**
 * @tc.name: SetIsPcAppInPad
 * @tc.desc: SetIsPcAppInPad Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetIsPcAppInPad, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    bool isPcAppInPad = false;
    auto result = session_->SetIsPcAppInPad(isPcAppInPad);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetBufferAvailable
 * @tc.desc: SetBufferAvailable Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetBufferAvailable, Function | SmallTest | Level2)
{
    int resultValue = 0;
    NotifyBufferAvailableChangeFunc func = [&resultValue](const bool isAvailable) {
        resultValue = 1;
    };
    session_->SetBufferAvailableChangeListener(func);
    session_->SetBufferAvailable(true);
    ASSERT_EQ(session_->bufferAvailable_, true);
}

/**
 * @tc.name: NotifySessionInfoChange
 * @tc.desc: NotifySessionInfoChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifySessionInfoChange, Function | SmallTest | Level2)
{
    int resultValue = 0;
    NotifyBufferAvailableChangeFunc func = [&resultValue](const bool isAvailable) {
        resultValue = 1;
    };
    session_->SetSessionInfoChangeNotifyManagerListener(func);
    session_->NotifySessionInfoChange();
    ASSERT_EQ(resultValue, 1);
}

/**
 * @tc.name: SetCompatibleModeEnableInPad
 * @tc.desc: SetCompatibleModeEnableInPad Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetCompatibleModeEnableInPad, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_FOREGROUND;
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    bool enable = true;
    session_->sessionStage_ = nullptr;
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->SetCompatibleModeEnableInPad(enable));

    ASSERT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;
    ASSERT_EQ(WSError::WS_OK, session_->SetCompatibleModeEnableInPad(enable));

    enable = false;
    ASSERT_EQ(WSError::WS_OK, session_->SetCompatibleModeEnableInPad(enable));
}

/**
 * @tc.name: GetSurfaceNodeForMoveDrag
 * @tc.desc: GetSurfaceNodeForMoveDrag Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, GetSurfaceNodeForMoveDrag, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->leashWinSurfaceNode_ = nullptr;
    session_->surfaceNode_ = nullptr;
    std::shared_ptr<RSSurfaceNode> res = session_->GetSurfaceNodeForMoveDrag();
    ASSERT_EQ(res, nullptr);
}

/**
 * @tc.name: CompatibleFullScreenRecover
 * @tc.desc: CompatibleFullScreenRecover Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, CompatibleFullScreenRecover, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;
    session_->sessionInfo_.isSystem_ = true;
    auto result = session_->CompatibleFullScreenRecover();
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    session_->sessionInfo_.isSystem_ = false;
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    result = session_->CompatibleFullScreenRecover();
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: CompatibleFullScreenMinimize
 * @tc.desc: CompatibleFullScreenMinimize Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, CompatibleFullScreenMinimize, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;
    session_->sessionInfo_.isSystem_ = true;
    auto result = session_->CompatibleFullScreenMinimize();
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    session_->sessionInfo_.isSystem_ = false;
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    result = session_->CompatibleFullScreenMinimize();
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: CompatibleFullScreenClose
 * @tc.desc: CompatibleFullScreenClose Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, CompatibleFullScreenClose, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;
    session_->sessionInfo_.isSystem_ = true;
    auto result = session_->CompatibleFullScreenClose();
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    session_->sessionInfo_.isSystem_ = false;
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    result = session_->CompatibleFullScreenClose();
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: PcAppInPadNormalClose
 * @tc.desc: PcAppInPadNormalClose Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, PcAppInPadNormalClose, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->sessionInfo_.isSystem_ = true;
    auto result = session_->PcAppInPadNormalClose();
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    session_->sessionInfo_.isSystem_ = false;
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    result = session_->PcAppInPadNormalClose();
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;
    result = session_->PcAppInPadNormalClose();
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: GetSnapshotPixelMap
 * @tc.desc: GetSnapshotPixelMap Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, GetSnapshotPixelMap, Function | SmallTest | Level2)
{
    session_->scenePersistence_ = nullptr;
    EXPECT_EQ(nullptr, session_->GetSnapshotPixelMap(6.6f, 8.8f));
    session_->scenePersistence_ = sptr<ScenePersistence>::MakeSptr("GetSnapshotPixelMap", 2024);
    EXPECT_NE(nullptr, session_->scenePersistence_);
    session_->scenePersistence_->isSavingSnapshot_.store(true);
    session_->snapshot_ = nullptr;
    EXPECT_EQ(nullptr, session_->GetSnapshotPixelMap(6.6f, 8.8f));
}

/**
 * @tc.name: ResetDirtyFlags
 * @tc.desc: ResetDirtyFlags Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, ResetDirtyFlags, Function | SmallTest | Level2)
{
    session_->isVisible_ = false;
    session_->dirtyFlags_ = 96;
    session_->ResetDirtyFlags();
    EXPECT_EQ(64, session_->dirtyFlags_);

    session_->isVisible_ = true;
    session_->dirtyFlags_ = 16;
    session_->ResetDirtyFlags();
    EXPECT_EQ(0, session_->dirtyFlags_);
}

/**
 * @tc.name: SetMainSessionUIStateDirty
 * @tc.desc: SetMainSessionUIStateDirty Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetMainSessionUIStateDirty, Function | SmallTest | Level2)
{
    SessionInfo infoDirty;
    infoDirty.abilityName_ = "SetMainSessionUIStateDirty";
    infoDirty.moduleName_ = "SetMainSessionUIStateDirty";
    infoDirty.bundleName_ = "SetMainSessionUIStateDirty";
    infoDirty.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_END);
    sptr<Session> sessionDirty = sptr<Session>::MakeSptr(infoDirty);
    EXPECT_NE(nullptr, sessionDirty);

    session_->parentSession_ = nullptr;
    EXPECT_EQ(nullptr, session_->GetParentSession());
    sessionDirty->SetUIStateDirty(false);
    session_->SetMainSessionUIStateDirty(false);
    EXPECT_EQ(false, sessionDirty->GetUIStateDirty());

    session_->SetParentSession(sessionDirty);
    EXPECT_EQ(sessionDirty, session_->GetParentSession());
    session_->SetMainSessionUIStateDirty(false);
    EXPECT_EQ(false, sessionDirty->GetUIStateDirty());

    infoDirty.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<Session> sessionUIState = sptr<Session>::MakeSptr(infoDirty);
    EXPECT_NE(nullptr, sessionUIState);
    session_->SetParentSession(sessionUIState);
    session_->SetMainSessionUIStateDirty(true);
    EXPECT_EQ(true, sessionUIState->GetUIStateDirty());
}

/**
 * @tc.name: SetStartingBeforeVisible
 * @tc.desc: test SetStartingBeforeVisible
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetStartingBeforeVisible, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetStartingBeforeVisible(true);
    ASSERT_EQ(true, session_->isStartingBeforeVisible_);
    ASSERT_EQ(true, session_->GetStartingBeforeVisible());

    session_->SetStartingBeforeVisible(false);
    ASSERT_EQ(false, session_->isStartingBeforeVisible_);
    ASSERT_EQ(false, session_->GetStartingBeforeVisible());
}

/**
 * @tc.name: GetScreenId
 * @tc.desc: test func: GetScreenId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, GetScreenId, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->sessionInfo_.screenId_ = 100;
    ASSERT_EQ(session_->GetScreenId(), 100);
}

/**
 * @tc.name: SetFreezeImmediately
 * @tc.desc: SetFreezeImmediately Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetFreezeImmediately, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    struct RSSurfaceNodeConfig config;
    session_->surfaceNode_ = RSSurfaceNode::Create(config);
    ASSERT_NE(session_->surfaceNode_, nullptr);
    ASSERT_EQ(nullptr, session_->SetFreezeImmediately(1.0f, false, 1.0f));
    session_->surfaceNode_->bufferAvailable_ = true;
    ASSERT_EQ(nullptr, session_->SetFreezeImmediately(1.0f, false, 1.0f));
    ASSERT_EQ(nullptr, session_->SetFreezeImmediately(1.0f, true, 1.0f));
    session_->surfaceNode_ = nullptr;
    ASSERT_EQ(nullptr, session_->SetFreezeImmediately(1.0f, false, 1.0f));
}

/**
 * @tc.name: GetIsHighlighted
 * @tc.desc: GetIsHighlighted Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, GetIsHighlighted, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    bool isHighlighted = false;
    ASSERT_EQ(session_->GetIsHighlighted(isHighlighted), WSError::WS_OK);
    ASSERT_EQ(isHighlighted, false);
}
}
} // namespace Rosen
} // namespace OHOS
