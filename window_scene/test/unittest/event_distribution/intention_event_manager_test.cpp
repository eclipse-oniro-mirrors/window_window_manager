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

#include "intention_event/include/intention_event_manager.h"
#include "mock_uicontent.h"
#include "session/host/include/scene_session.h"
#include "session_manager/include/scene_session_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class IntentionEventManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::unique_ptr<Ace::UIContent> uIContent_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    std::shared_ptr<IntentionEventManager::InputEventListener> inputEventListener_;
    std::shared_ptr<AppExecFwk::EventRunner> runner_;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
};

void IntentionEventManagerTest::SetUpTestCase() {}

void IntentionEventManagerTest::TearDownTestCase() {}

void IntentionEventManagerTest::SetUp()
{
    uIContent_ = std::make_unique<Ace::UIContentMocker>();
    EXPECT_NE(nullptr, uIContent_);
    runner_ = AppExecFwk::EventRunner::Create("TestRunner");
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner_);
    EXPECT_NE(nullptr, eventHandler_);
    inputEventListener_ =
        std::make_shared<IntentionEventManager::InputEventListener>(uIContent_.get(), eventHandler_, nullptr);
    EXPECT_NE(nullptr, inputEventListener_);
    SceneSessionManager::GetInstance().sceneSessionMap_.clear();
}

void IntentionEventManagerTest::TearDown()
{
    uIContent_ = nullptr;
    eventHandler_ = nullptr;
    inputEventListener_ = nullptr;
    runner_ = nullptr;
    SceneSessionManager::GetInstance().sceneSessionMap_.clear();
}

namespace {
/**
 * @tc.name: EnableInputEventListener
 * @tc.desc: EnableInputEventListener Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, EnableInputEventListener, TestSize.Level0)
{
    bool enable = DelayedSingleton<IntentionEventManager>::GetInstance()->
        EnableInputEventListener(nullptr, nullptr, nullptr);
    EXPECT_EQ(false, enable);
    enable = DelayedSingleton<IntentionEventManager>::GetInstance()->
        EnableInputEventListener(uIContent_.get(), nullptr, nullptr);
    EXPECT_EQ(false, enable);
    enable = DelayedSingleton<IntentionEventManager>::GetInstance()->
        EnableInputEventListener(uIContent_.get(), eventHandler_, nullptr);
    EXPECT_EQ(true, enable);
}

/**
 * @tc.name: DispatchKeyEventCallback
 * @tc.desc: DispatchKeyEventCallback Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, DispatchKeyEventCallback, TestSize.Level0)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = nullptr;
    inputEventListener_->DispatchKeyEventCallback(2024, keyEvent, true);
    keyEvent = MMI::KeyEvent::Create();
    EXPECT_NE(nullptr, keyEvent);
    inputEventListener_->DispatchKeyEventCallback(2024, keyEvent, true);
    inputEventListener_->DispatchKeyEventCallback(2024, keyEvent, false);
    SessionInfo info;
    info.bundleName_ = "IntentionEventManager";
    info.moduleName_ = "InputEventListener";
    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, callback);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, callback);
    EXPECT_NE(nullptr, sceneSession);
    SceneSessionManager::GetInstance().sceneSessionMap_.emplace(std::make_pair(2024, sceneSession));
    std::shared_ptr<IntentionEventManager::InputEventListener> inputEventListener =
        std::make_shared<IntentionEventManager::InputEventListener>(nullptr, nullptr, nullptr);
    inputEventListener->DispatchKeyEventCallback(2024, keyEvent, false);
    inputEventListener_->DispatchKeyEventCallback(2024, keyEvent, false);
}

/**
 * @tc.name: CheckPointerEvent
 * @tc.desc: CheckPointerEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, CheckPointerEvent, TestSize.Level0)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    std::shared_ptr<IntentionEventManager::InputEventListener> inputEventListener =
        std::make_shared<IntentionEventManager::InputEventListener>(nullptr, nullptr, nullptr);
    EXPECT_NE(nullptr, inputEventListener);
    EXPECT_EQ(false, inputEventListener->CheckPointerEvent(pointerEvent));
    pointerEvent = MMI::PointerEvent::Create();
    EXPECT_EQ(false, inputEventListener->CheckPointerEvent(pointerEvent));
    SceneSessionManager::GetInstance().SetEnableInputEvent(false);
    EXPECT_EQ(false, inputEventListener_->CheckPointerEvent(pointerEvent));
    SceneSessionManager::GetInstance().SetEnableInputEvent(true);
    pointerEvent->SetDispatchTimes(10);
    EXPECT_EQ(10, pointerEvent->GetDispatchTimes());
    EXPECT_EQ(true, inputEventListener_->CheckPointerEvent(pointerEvent));
}

/**
 * @tc.name: OnInputEventPointer1
 * @tc.desc: OnInputEventPointer1 Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, OnInputEventPointer1, TestSize.Level0)
{
    auto pointerEvent = MMI::PointerEvent::Create();
    EXPECT_NE(nullptr, pointerEvent);
    SessionInfo info;
    info.bundleName_ = "IntentionEventManager";
    info.moduleName_ = "InputEventListener";
    info.isSystem_ = true;
    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, callback);
    sptr<SceneSession> sceneSession0 = sptr<SceneSession>::MakeSptr(info, callback);
    EXPECT_NE(nullptr, sceneSession0);
    SceneSessionManager::GetInstance().sceneSessionMap_.emplace(std::make_pair(0, sceneSession0));
    pointerEvent->SetDispatchTimes(-1);
    EXPECT_EQ(-1, pointerEvent->GetDispatchTimes());
    pointerEvent->SetPointerId(10);
    EXPECT_EQ(10, pointerEvent->GetPointerId());
    MMI::PointerEvent::PointerItem item;
    item.SetPointerId(10);
    EXPECT_EQ(10, item.GetPointerId());
    pointerEvent->AddPointerItem(item);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    EXPECT_EQ(MMI::PointerEvent::POINTER_ACTION_MOVE, pointerEvent->GetPointerAction());
    inputEventListener_->OnInputEvent(pointerEvent);

    pointerEvent->pointers_.clear();
    pointerEvent->SetDispatchTimes(10);
    inputEventListener_->OnInputEvent(pointerEvent);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    inputEventListener_->OnInputEvent(pointerEvent);
    pointerEvent->AddPointerItem(item);
    inputEventListener_->OnInputEvent(pointerEvent);
}

/**
 * @tc.name: SetPointerEventStatus
 * @tc.desc: SetPointerEventStatus Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, SetPointerEventStatus, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetPointerEventStatus";
    info.bundleName_ = "SetPointerEventStatus";
    info.windowType_ = 1;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    EXPECT_NE(sceneSession, nullptr);

    int32_t action = MMI::PointerEvent::POINTER_ACTION_DOWN;
    inputEventListener_->SetPointerEventStatus(0, action, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, sceneSession);
    auto fingerPointerDownStatusList = sceneSession->GetFingerPointerDownStatusList();
    EXPECT_EQ(1, fingerPointerDownStatusList.size());

    action = MMI::PointerEvent::POINTER_ACTION_UP;
    inputEventListener_->SetPointerEventStatus(0, action, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, sceneSession);
    fingerPointerDownStatusList = sceneSession->GetFingerPointerDownStatusList();
    EXPECT_EQ(0, fingerPointerDownStatusList.size());

    action = MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN;
    inputEventListener_->SetPointerEventStatus(1, action, MMI::PointerEvent::SOURCE_TYPE_MOUSE, sceneSession);
    EXPECT_EQ(true, sceneSession->GetMousePointerDownEventStatus());

    action = MMI::PointerEvent::POINTER_ACTION_BUTTON_UP;
    inputEventListener_->SetPointerEventStatus(1, action, MMI::PointerEvent::SOURCE_TYPE_MOUSE, sceneSession);
    EXPECT_EQ(false, sceneSession->GetMousePointerDownEventStatus());

    action = MMI::PointerEvent::POINTER_ACTION_DOWN;
    inputEventListener_->SetPointerEventStatus(0, action, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, sceneSession);
    action = MMI::PointerEvent::POINTER_ACTION_CANCEL;
    inputEventListener_->SetPointerEventStatus(0, action, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, sceneSession);
    fingerPointerDownStatusList = sceneSession->GetFingerPointerDownStatusList();
    EXPECT_EQ(0, fingerPointerDownStatusList.size());

    action = MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN;
    inputEventListener_->SetPointerEventStatus(1, action, MMI::PointerEvent::SOURCE_TYPE_MOUSE, sceneSession);
    action = MMI::PointerEvent::POINTER_ACTION_CANCEL;
    inputEventListener_->SetPointerEventStatus(0, action, MMI::PointerEvent::SOURCE_TYPE_MOUSE, sceneSession);
    EXPECT_EQ(false, sceneSession->GetMousePointerDownEventStatus());
}

/**
 * @tc.name: OnInputEvent0
 * @tc.desc: OnInputEvent0 Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, OnInputEvent0, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    inputEventListener_->OnInputEvent(pointerEvent);
    pointerEvent = MMI::PointerEvent::Create();
    EXPECT_NE(nullptr, pointerEvent);

    pointerEvent->SetTargetWindowId(0);
    inputEventListener_->OnInputEvent(pointerEvent);

    SessionInfo info;
    info.bundleName_ = "IntentionEventManager";
    info.moduleName_ = "InputEventListener";
    info.isSystem_ = true;
    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, callback);
    sptr<SceneSession> sceneSession0 = sptr<SceneSession>::MakeSptr(info, callback);
    EXPECT_NE(nullptr, sceneSession0);
    SceneSessionManager::GetInstance().sceneSessionMap_.emplace(std::make_pair(0, sceneSession0));

    pointerEvent->SetDispatchTimes(10);
    EXPECT_EQ(10, pointerEvent->GetDispatchTimes());
    pointerEvent->SetPointerId(10);
    EXPECT_EQ(10, pointerEvent->GetPointerId());

    MMI::PointerEvent::PointerItem item;
    item.SetPointerId(10);
    EXPECT_EQ(10, item.GetPointerId());
    pointerEvent->AddPointerItem(item);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_UP);
    EXPECT_EQ(MMI::PointerEvent::POINTER_ACTION_UP, pointerEvent->GetPointerAction());
    inputEventListener_->OnInputEvent(pointerEvent);

    info.isSystem_ = false;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, callback);
    EXPECT_NE(nullptr, sceneSession1);
    SceneSessionManager::GetInstance().sceneSessionMap_.emplace(std::make_pair(1, sceneSession1));
    pointerEvent->SetTargetWindowId(1);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_FLOAT);
    sceneSession1->SetSessionProperty(property);
    inputEventListener_->OnInputEvent(pointerEvent);
    EXPECT_EQ(200010, pointerEvent->GetPointerId());
}

/**
 * @tc.name: OnInputEvent1
 * @tc.desc: OnInputEvent1 Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, OnInputEvent1, TestSize.Level0)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = nullptr;
    inputEventListener_->OnInputEvent(keyEvent);
    keyEvent = MMI::KeyEvent::Create();
    EXPECT_NE(nullptr, keyEvent);
    SceneSessionManager::GetInstance().SetEnableInputEvent(false);
    EXPECT_EQ(false, SceneSessionManager::GetInstance().IsInputEventEnabled());
    inputEventListener_->OnInputEvent(keyEvent);

    SceneSessionManager::GetInstance().SetEnableInputEvent(true);
    SceneSessionManager::GetInstance().SetFocusedSessionId(INVALID_SESSION_ID, DEFAULT_DISPLAY_ID);
    EXPECT_EQ(INVALID_SESSION_ID, SceneSessionManager::GetInstance().GetFocusedSessionId());
    inputEventListener_->OnInputEvent(keyEvent);

    SceneSessionManager::GetInstance().SetFocusedSessionId(1, DEFAULT_DISPLAY_ID);
    inputEventListener_->OnInputEvent(keyEvent);

    SessionInfo info;
    info.bundleName_ = "IntentionEventManager";
    info.moduleName_ = "InputEventListener";
    info.isSystem_ = true;
    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, callback);
    sptr<SceneSession> sceneSession0 = sptr<SceneSession>::MakeSptr(info, callback);
    EXPECT_NE(nullptr, sceneSession0);
    SceneSessionManager::GetInstance().sceneSessionMap_.emplace(std::make_pair(1, sceneSession0));
    EXPECT_EQ(1, SceneSessionManager::GetInstance().sceneSessionMap_.size());
    inputEventListener_->OnInputEvent(keyEvent);

    info.isSystem_ = false;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, callback);
    EXPECT_NE(nullptr, sceneSession1);
    SceneSessionManager::GetInstance().sceneSessionMap_.emplace(std::make_pair(2, sceneSession1));
    SceneSessionManager::GetInstance().SetFocusedSessionId(2, DEFAULT_DISPLAY_ID);
    EXPECT_EQ(2, SceneSessionManager::GetInstance().GetFocusedSessionId());
    auto focusedSceneSession = SceneSessionManager::GetInstance().GetSceneSession(2);
    inputEventListener_->OnInputEvent(keyEvent);
    EXPECT_NE(nullptr, focusedSceneSession);
}

/**
 * @tc.name: OnInputEvent2
 * @tc.desc: OnInputEvent2 Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, OnInputEvent2, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    EXPECT_NE(nullptr, keyEvent);
    SceneSessionManager::GetInstance().SetEnableInputEvent(true);
    SceneSessionManager::GetInstance().SetFocusedSessionId(1, DEFAULT_DISPLAY_ID);
    SessionInfo info;
    info.bundleName_ = "IntentionEventManager";
    info.moduleName_ = "InputEventListener";
    info.isSystem_ = true;
    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, callback);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, callback);
    EXPECT_NE(nullptr, sceneSession);
    auto func = [](std::shared_ptr<MMI::KeyEvent> keyEvent, bool isPreImeEvent) { return true; };
    sceneSession->SetNotifySystemSessionKeyEventFunc(func);
    SceneSessionManager::GetInstance().sceneSessionMap_.emplace(std::make_pair(1, sceneSession));
    EXPECT_EQ(1, SceneSessionManager::GetInstance().sceneSessionMap_.size());
    inputEventListener_->OnInputEvent(keyEvent);
    sceneSession->SetNotifySystemSessionKeyEventFunc(nullptr);
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_FN);
    EXPECT_EQ(MMI::KeyEvent::KEYCODE_FN, keyEvent->GetKeyCode());
    auto focusedSceneSession = SceneSessionManager::GetInstance().GetSceneSession(1);
    inputEventListener_->OnInputEvent(keyEvent);
    EXPECT_NE(nullptr, focusedSceneSession);
}

/**
 * @tc.name: OnInputEvent3
 * @tc.desc: OnInputEvent3 Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, OnInputEvent3, TestSize.Level1)
{
    std::shared_ptr<IntentionEventManager::InputEventListener> inputEventListener =
        std::make_shared<IntentionEventManager::InputEventListener>(nullptr, nullptr, nullptr);
    EXPECT_NE(nullptr, inputEventListener);
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    EXPECT_NE(nullptr, keyEvent);
    SceneSessionManager::GetInstance().SetEnableInputEvent(true);
    SceneSessionManager::GetInstance().SetFocusedSessionId(1, DEFAULT_DISPLAY_ID);
    SessionInfo info;
    info.bundleName_ = "IntentionEventManager";
    info.moduleName_ = "InputEventListener";
    info.isSystem_ = true;
    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, callback);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, callback);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->SetNotifySystemSessionKeyEventFunc(nullptr);
    SceneSessionManager::GetInstance().sceneSessionMap_.emplace(std::make_pair(1, sceneSession));
    EXPECT_EQ(1, SceneSessionManager::GetInstance().sceneSessionMap_.size());
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_UNKNOWN);
    EXPECT_EQ(MMI::KeyEvent::KEYCODE_UNKNOWN, keyEvent->GetKeyCode());
    inputEventListener->OnInputEvent(keyEvent);
    auto focusedSceneSession = SceneSessionManager::GetInstance().GetSceneSession(1);
    inputEventListener_->OnInputEvent(keyEvent);
    EXPECT_NE(nullptr, focusedSceneSession);
}

/**
 * @tc.name: OnInputEvent4
 * @tc.desc: OnInputEvent4 Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, OnInputEvent4, TestSize.Level1)
{
    std::shared_ptr<IntentionEventManager::InputEventListener> inputEventListener =
        std::make_shared<IntentionEventManager::InputEventListener>(nullptr, nullptr, nullptr);
    EXPECT_EQ(nullptr, inputEventListener->uiContent_);
    std::shared_ptr<MMI::AxisEvent> axisEvent = nullptr;
    inputEventListener->OnInputEvent(axisEvent);
    axisEvent = MMI::AxisEvent::Create();
    EXPECT_NE(nullptr, axisEvent);
    inputEventListener->OnInputEvent(axisEvent);
}

/**
 * @tc.name: IsKeyboardEvent
 * @tc.desc: IsKeyboardEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, IsKeyboardEvent, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    EXPECT_NE(nullptr, keyEvent);
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_BACK);
    EXPECT_EQ(true, inputEventListener_->IsKeyboardEvent(keyEvent));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: CreateAndEnableInputEventListener
 * @tc.desc: CreateAndEnableInputEventListener Test
 * @tc.type: FUNC
 */
HWTEST_F(IntentionEventManagerTest, CreateAndEnableInputEventListener, TestSize.Level1)
{
    bool enable = CreateAndEnableInputEventListener(nullptr, nullptr, nullptr);
    EXPECT_EQ(false, enable);
    enable = CreateAndEnableInputEventListener(uIContent_.get(), nullptr, nullptr);
    EXPECT_EQ(false, enable);
    enable = CreateAndEnableInputEventListener(uIContent_.get(), eventHandler_, nullptr);
    EXPECT_EQ(true, enable);
}
} // namespace
} // namespace Rosen
} // namespace OHOS