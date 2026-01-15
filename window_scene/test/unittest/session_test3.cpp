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
std::string g_errLog;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
    const char *msg)
{
    g_errLog = msg;
}
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

void WindowSessionTest3::SetUpTestCase() {}

void WindowSessionTest3::TearDownTestCase() {}

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
    auto isScreenLockedCallback = [this]() { return ssm_->IsScreenLocked(); };
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
HWTEST_F(WindowSessionTest3, NotifyContextTransparent, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, SetFocusable04, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, SetSystemFocusable, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, SetFocusableOnShow, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, CheckFocusable, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(session_->CheckFocusable(), true);
    session_->SetSystemFocusable(false);
    ASSERT_EQ(session_->CheckFocusable(), false);
}

/**
 * @tc.name: GetTouchable02
 * @tc.desc: GetTouchable Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, GetTouchable02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    EXPECT_EQ(true, session_->GetTouchable());
}

/**
 * @tc.name: UpdateDensity02
 * @tc.desc: UpdateDensity Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, UpdateDensity02, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, UpdateOrientation, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, SetActive, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, SetActive02, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, SetActive03, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, PresentFocusIfPointDown, TestSize.Level1)
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
 * @tc.name: PresentFocusIfPointDown01
 * @tc.desc: PresentFocusIfPointDown Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, PresentFocusIfPointDown01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->property_->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    session_->PresentFocusIfPointDown();

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session_->property_->SetRaiseEnabled(false);
    session_->PresentFocusIfPointDown();
}

/**
 * @tc.name: TransferPointerEvent06
 * @tc.desc: TransferPointerEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, TransferPointerEvent06, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, TransferPointerEvent07, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, TransferPointerEvent08, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, TransferPointerEvent09, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, TransferPointerEvent10, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, TransferPointerEvent11, TestSize.Level1)
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
 * @tc.name: HandlePointerEventForFocus_NullPtr
 * @tc.desc: HandlePointerEventForFocus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, HandlePointerEventForFocus_NullPtr, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    WSError ret = session_->HandlePointerEventForFocus(pointerEvent);
    EXPECT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: HandlePointerEventForFocus_Hover
 * @tc.desc: HandlePointerEventForFocus
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, HandlePointerEventForFocus_Hover, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->pointerAction_ = MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER;
    pointerEvent->sourceType_ = MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WSError ret = session_->HandlePointerEventForFocus(pointerEvent);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: TransferFocusStateEvent03
 * @tc.desc: TransferFocusStateEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, TransferFocusStateEvent03, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, Snapshot, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, SetBufferAvailableChangeListener, TestSize.Level1)
{
    session_->SetSessionState(SessionState::STATE_CONNECT);
    session_->SetSessionStateChangeNotifyManagerListener(nullptr);

    session_->bufferAvailable_ = true;
    session_->SetBufferAvailableChangeListener(nullptr);

    int resultValue = 0;
    NotifyBufferAvailableChangeFunc func =
        [&resultValue](const bool isAvailable, bool startWindowInvisible) { resultValue += 1; };
    session_->SetBufferAvailableChangeListener(func);
    ASSERT_EQ(resultValue, 1);
}

/**
 * @tc.name: NotifySessionFocusableChange
 * @tc.desc: NotifySessionFocusableChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifySessionFocusableChange, TestSize.Level1)
{
    int resultValue = 0;
    NotifySessionFocusableChangeFunc func = [&resultValue](const bool isFocusable) { resultValue += 1; };
    session_->SetSessionFocusableChangeListener(func);
    session_->NotifySessionFocusableChange(true);
    ASSERT_EQ(resultValue, 2);
}

/**
 * @tc.name: GetStateFromManager
 * @tc.desc: GetStateFromManager Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, GetStateFromManager, TestSize.Level1)
{
    ManagerState key = ManagerState{ 0 };
    GetStateFromManagerFunc func = [](const ManagerState key) { return true; };
    session_->getStateFromManagerFunc_ = func;
    session_->GetStateFromManager(key);

    session_->getStateFromManagerFunc_ = nullptr;
    ASSERT_EQ(false, session_->GetStateFromManager(key));

    // 覆盖default分支
    key = ManagerState{ -1 };
    ASSERT_EQ(false, session_->GetStateFromManager(key));
}

/**
 * @tc.name: NotifyUIRequestFocus
 * @tc.desc: NotifyUIRequestFocus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifyUIRequestFocus, TestSize.Level1)
{
    session_->requestFocusFunc_ = []() {};
    session_->NotifyUIRequestFocus();

    ASSERT_NE(session_, nullptr);
}

/**
 * @tc.name: NotifySessionTouchableChange
 * @tc.desc: NotifySessionTouchableChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifySessionTouchableChange, TestSize.Level1)
{
    int resultValue = 0;
    NotifySessionTouchableChangeFunc func = [&resultValue](const bool touchable) { resultValue += 1; };
    session_->SetSessionTouchableChangeListener(func);
    session_->NotifySessionTouchableChange(true);
    ASSERT_EQ(resultValue, 2);
}

/**
 * @tc.name: NotifyClick
 * @tc.desc: NotifyClick Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifyClick, TestSize.Level1)
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
    EXPECT_EQ(hasRequestFocus, false);
    EXPECT_EQ(hasIsClick, false);
}

/**
 * @tc.name: NotifyRequestFocusStatusNotifyManager
 * @tc.desc: NotifyRequestFocusStatusNotifyManager Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifyRequestFocusStatusNotifyManager, TestSize.Level1)
{
    int resultValue = 0;
    NotifyRequestFocusStatusNotifyManagerFunc func =
        [&resultValue](int32_t persistentId, const bool isFocused, const bool byForeground, FocusChangeReason reason) {
            resultValue += 1;
        };
    session_->SetRequestFocusStatusNotifyManagerListener(func);
    FocusChangeReason reason = FocusChangeReason::SCB_SESSION_REQUEST;
    session_->NotifyRequestFocusStatusNotifyManager(true, false, reason);
    EXPECT_EQ(resultValue, 1);
}

/**
 * @tc.name: PresentFocusIfNeed01
 * @tc.desc: PresentFocusIfNeed Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, PresentFocusIfNeed01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool hasNotifyManagerToRequestFocus = false;
    session_->isFocused_ = true;
    session_->property_->focusable_ = false;
    session_->SetRequestFocusStatusNotifyManagerListener(
        [&hasNotifyManagerToRequestFocus](int32_t persistentId, const bool isFocused, const bool byForeground,
            FocusChangeReason reason) {
            hasNotifyManagerToRequestFocus = true;
        });

    int32_t pointerAction = MMI::PointerEvent::POINTER_ACTION_MOVE;
    int32_t sourceType = MMI::PointerEvent::SOURCE_TYPE_UNKNOWN;
    session_->PresentFocusIfNeed(pointerAction, sourceType);
    EXPECT_EQ(hasNotifyManagerToRequestFocus, false);

    pointerAction = MMI::PointerEvent::POINTER_ACTION_DOWN;
    session_->PresentFocusIfNeed(pointerAction, sourceType);
    EXPECT_EQ(hasNotifyManagerToRequestFocus, false);

    session_->isFocused_ = false;
    session_->PresentFocusIfNeed(pointerAction, sourceType);
    EXPECT_EQ(hasNotifyManagerToRequestFocus, false);
}

/**
 * @tc.name: PresentFocusIfNeed02
 * @tc.desc: PresentFocusIfNeed Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, PresentFocusIfNeed02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool hasNotifyManagerToRequestFocus = false;
    session_->isFocused_ = false;
    session_->property_->focusable_ = false;
    session_->SetRequestFocusStatusNotifyManagerListener(
        [&hasNotifyManagerToRequestFocus](int32_t persistentId, const bool isFocused, const bool byForeground,
            FocusChangeReason reason) {
            hasNotifyManagerToRequestFocus = true;
        });

    hasNotifyManagerToRequestFocus = false;
    int32_t pointerAction = MMI::PointerEvent::POINTER_ACTION_DOWN;
    int32_t sourceType = MMI::PointerEvent::SOURCE_TYPE_UNKNOWN;
    session_->property_->focusable_ = true;
    session_->PresentFocusIfNeed(pointerAction, sourceType);
    EXPECT_EQ(hasNotifyManagerToRequestFocus, true);

    hasNotifyManagerToRequestFocus = false;
    pointerAction = MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN;
    session_->PresentFocusIfNeed(pointerAction, sourceType);
    EXPECT_EQ(hasNotifyManagerToRequestFocus, true);

    hasNotifyManagerToRequestFocus = false;
    pointerAction = MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER;
    sourceType = MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    session_->PresentFocusIfNeed(pointerAction, sourceType);
    EXPECT_EQ(hasNotifyManagerToRequestFocus, true);
}

/**
 * @tc.name: PresentFocusIfNeed03
 * @tc.desc: PresentFocusIfNeed Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, PresentFocusIfNeed03, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    int32_t pointerAction = MMI::PointerEvent::POINTER_ACTION_DOWN;
    int32_t sourceType = MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    session_->property_->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    session_->PresentFocusIfNeed(pointerAction, sourceType);

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session_->property_->SetRaiseEnabled(false);
    session_->PresentFocusIfNeed(pointerAction, sourceType);
}

/**
 * @tc.name: IsNeedRequestToTop
 * @tc.desc: IsNeedRequestToTop Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, IsNeedRequestToTop, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->property_->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    EXPECT_EQ(session_->IsNeedRequestToTop(), true);
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session_->property_->SetRaiseEnabled(false);
    EXPECT_EQ(session_->IsNeedRequestToTop(), false);
    session_->property_->SetRaiseEnabled(true);
    EXPECT_EQ(session_->IsNeedRequestToTop(), true);
}

/**
 * @tc.name: UpdateFocus03
 * @tc.desc: UpdateFocus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, UpdateFocus03, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, NotifyFocusStatus, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_CONNECT;
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    auto info = sptr<FocusNotifyInfo>::MakeSptr();
    session_->sessionStage_ = mockSessionStage;
    EXPECT_EQ(WSError::WS_OK, session_->NotifyFocusStatus(info, true));
    session_->sessionStage_ = nullptr;
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, session_->NotifyFocusStatus(info, true));
}

/**
 * @tc.name: RequestFocus
 * @tc.desc: RequestFocus Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, RequestFocus, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, UpdateWindowMode, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, RectSizeCheckProcess, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, RectCheckProcess, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, RectCheckProcess01, TestSize.Level1)
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

    WSRect rect = { 0, 0, 0, 0 };
    session_->GetLayoutController()->SetSessionRect(rect);
    session_->RectCheckProcess();

    WSRect curRect = session_->GetSessionRect();
    session_->GetLayoutController()->SetSessionRect({ curRect.posX_, curRect.posY_, curRect.width_, 200 });
    session_->RectCheckProcess();

    session_->SetAspectRatio(0.0f);
    session_->RectCheckProcess();

    session_->SetAspectRatio(0.5f);
    session_->RectCheckProcess();

    curRect = session_->GetSessionRect();
    session_->GetLayoutController()->SetSessionRect({ curRect.posX_, curRect.posY_, 200, curRect.height_ });
    session_->RectCheckProcess();

    session_->SetAspectRatio(1.0f);
    session_->RectCheckProcess();

    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
}

/**
 * @tc.name: SetAcquireRotateAnimationConfigFunc
 * @tc.desc: SetAcquireRotateAnimationConfigFunc Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetAcquireRotateAnimationConfigFunc, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetAcquireRotateAnimationConfigFunc(nullptr);
    ASSERT_EQ(session_->acquireRotateAnimationConfigFunc_, nullptr);
    int32_t duration = session_->GetRotateAnimationDuration();
    ASSERT_EQ(duration, ROTATE_ANIMATION_DURATION);

    AcquireRotateAnimationConfigFunc func = [](RotateAnimationConfig& config) { config.duration_ = 800; };
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
HWTEST_F(WindowSessionTest3, SetIsPcAppInPad, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool isPcAppInLargeScreenDevice = false;
    auto result = session_->SetIsPcAppInPad(isPcAppInLargeScreenDevice);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetPcAppInpadCompatibleMode
 * @tc.desc: SetPcAppInpadCompatibleMode Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetPcAppInpadCompatibleMode, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool enabled = false;
    auto result = session_->SetPcAppInpadCompatibleMode(enabled);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetPcAppInpadSpecificSystemBarInvisible
 * @tc.desc: SetPcAppInpadSpecificSystemBarInvisible Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetPcAppInpadSpecificSystemBarInvisible, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool isPcAppInpadSpecificSystemBarInvisible = false;
    auto result = session_->SetPcAppInpadSpecificSystemBarInvisible(isPcAppInpadSpecificSystemBarInvisible);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetPcAppInpadOrientationLandscape
 * @tc.desc: SetPcAppInpadOrientationLandscape Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetPcAppInpadOrientationLandscape, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool isPcAppInpadOrientationLandscape = false;
    auto result = session_->SetPcAppInpadOrientationLandscape(isPcAppInpadOrientationLandscape);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetMobileAppInPadLayoutFullScreen
 * @tc.desc: SetMobileAppInPadLayoutFullScreen Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetMobileAppInPadLayoutFullScreen, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool isMobileAppInPadLayoutFullScreen = false;
    auto result = session_->SetMobileAppInPadLayoutFullScreen(isMobileAppInPadLayoutFullScreen);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetBufferAvailable
 * @tc.desc: SetBufferAvailable Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, SetBufferAvailable, TestSize.Level1)
{
    int resultValue = 0;
    NotifyBufferAvailableChangeFunc func =
        [&resultValue](const bool isAvailable, bool startWindowInvisible) { resultValue = 1; };
    session_->SetBufferAvailableChangeListener(func);
    session_->SetBufferAvailable(true);
    ASSERT_EQ(session_->bufferAvailable_, true);
}

/**
 * @tc.name: NotifySessionInfoChange
 * @tc.desc: NotifySessionInfoChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifySessionInfoChange, TestSize.Level1)
{
    int resultValue = 0;
    NotifySessionInfoChangeNotifyManagerFunc func = [&resultValue](int32_t persistentid) { resultValue = 1; };
    session_->SetSessionInfoChangeNotifyManagerListener(func);
    session_->NotifySessionInfoChange();
    ASSERT_EQ(resultValue, 1);
}

/**
 * @tc.name: NotifySessionPropertyChange01
 * @tc.desc: NotifySessionPropertyChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, NotifySessionPropertyChange01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int resultValue = 0;
    NotifySessionPropertyChangeNotifyManagerFunc func =
        [&resultValue](int32_t persistentid, WindowInfoKey windowInfoKey) { resultValue = 1; };
    session_->sessionPropertyChangeNotifyManagerFunc_ = nullptr;
    session_->NotifySessionPropertyChange(WindowInfoKey::WINDOW_RECT);
    EXPECT_TRUE(g_errLog.find("Func is invalid") != std::string::npos);
    session_->SetSessionPropertyChangeNotifyManagerListener(func);
    session_->NotifySessionPropertyChange(WindowInfoKey::WINDOW_RECT);
    EXPECT_EQ(resultValue, 1);
}

/**
 * @tc.name: GetSurfaceNodeForMoveDrag
 * @tc.desc: GetSurfaceNodeForMoveDrag Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, GetSurfaceNodeForMoveDrag, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->leashWinSurfaceNode_ = nullptr;
    session_->surfaceNode_ = nullptr;
    std::shared_ptr<RSSurfaceNode> res = session_->GetSurfaceNodeForMoveDrag();
    ASSERT_EQ(res, nullptr);
}

/**
 * @tc.name: PcAppInPadNormalClose
 * @tc.desc: PcAppInPadNormalClose Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, PcAppInPadNormalClose, TestSize.Level1)
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
 * @tc.name: ResetDirtyFlags
 * @tc.desc: ResetDirtyFlags Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, ResetDirtyFlags, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, SetMainSessionUIStateDirty, TestSize.Level1)
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
 * @tc.name: GetScreenId
 * @tc.desc: test func: GetScreenId
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, GetScreenId, TestSize.Level1)
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
HWTEST_F(WindowSessionTest3, SetFreezeImmediately, TestSize.Level1)
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
    bool isHighlighted = true;
    ASSERT_EQ(session_->GetIsHighlighted(isHighlighted), WSError::WS_OK);
    ASSERT_EQ(isHighlighted, false);
}

/**
 * @tc.name: EncodeAndDecodeSnapShotRecoverValue
 * @tc.desc: EncodeAndDecodeSnapShotRecoverValue Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, EncodeAndDecodeSnapShotRecoverValue, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int32_t snapShotRecoverValue = 0;
    session_->UpdateWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    snapShotRecoverValue = session_->EncodeSnapShotRecoverValue(DisplayOrientation::PORTRAIT);
    EXPECT_EQ(session_->DecodeSnapShotRecoverValue(snapShotRecoverValue,
        SnapShotRecoverType::ROTATE), static_cast<int32_t>(DisplayOrientation::PORTRAIT));
    EXPECT_EQ(session_->DecodeSnapShotRecoverValue(snapShotRecoverValue,
        SnapShotRecoverType::EXIT_SPLIT_ON_BACKGROUND),
        static_cast<int32_t>(session_->IsExitSplitOnBackgroundRecover()));
}

/**
 * @tc.name: IsExitSplitOnBackgroundRecover
 * @tc.desc: IsExitSplitOnBackgroundRecover Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest3, IsExitSplitOnBackgroundRecover, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int32_t snapShotRecoverValue = 0;
    session_->UpdateWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(session_->IsExitSplitOnBackgroundRecover(), false);
    session_->UpdateWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(session_->IsExitSplitOnBackgroundRecover(), true);
    session_->UpdateWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(session_->IsExitSplitOnBackgroundRecover(), true);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
