/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "input_manager.h"
#include "input_transfer_station.h"
#include "mock_window_adapter.h"
#include "singleton_mocker.h"
#include "window_impl.h"
#include "window_input_redistribute_impl.h"
#include "window_input_redistribute_client.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
using WindowMocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
class InputTransferStationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    sptr<WindowImpl> window_;
    std::shared_ptr<InputEventListener> listener;
};
void InputTransferStationTest::SetUpTestCase() {}

void InputTransferStationTest::TearDownTestCase()
{
}

void InputTransferStationTest::SetUp()
{
    sptr<WindowOption> option = new WindowOption();
    option->SetWindowName("inputwindow");
    window_ = new WindowImpl(option);
    window_->Create(INVALID_WINDOW_ID);
    listener = std::make_shared<InputEventListener>(InputEventListener());
}

void InputTransferStationTest::TearDown()
{
    window_->Destroy();
    window_ = nullptr;
}

namespace {
/**
 * @tc.name: AddInputWindow
 * @tc.desc: add input window in station.
 * @tc.type: FUNC
 * @tc.require: issueI5I5L4
 */
HWTEST_F(InputTransferStationTest, AddInputWindow, TestSize.Level0)
{
    InputTransferStation::GetInstance().isRegisteredMMI_ = true;
    InputTransferStation::GetInstance().AddInputWindow(window_);
    InputTransferStation::GetInstance().isRegisteredMMI_ = false;
    window_->GetWindowProperty()->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    InputTransferStation::GetInstance().destroyed_ = true;
    InputTransferStation::GetInstance().AddInputWindow(window_);
    InputTransferStation::GetInstance().destroyed_ = false;
    InputTransferStation::GetInstance().AddInputWindow(window_);
    InputTransferStation::GetInstance().inputListener_ = listener;
    InputTransferStation::GetInstance().AddInputWindow(window_);
    InputTransferStation::GetInstance().isGameControllerLoaded_ = true;
    InputTransferStation::GetInstance().AddInputWindow(window_);
    InputTransferStation::GetInstance().isGameControllerLoaded_ = false;
    InputTransferStation::GetInstance().AddInputWindow(window_);
    InputTransferStation::GetInstance().LoadTouchPredictor();
    InputTransferStation::GetInstance().LoadTouchPredictor();
    ASSERT_EQ(true, InputTransferStation::GetInstance().isGameControllerLoaded_);
}

/**
 * @tc.name: RemoveInputWindow
 * @tc.desc: remove input window in station.
 * @tc.type: FUNC
 * @tc.require: issueI5I5L4
 */
HWTEST_F(InputTransferStationTest, RemoveInputWindow, TestSize.Level0)
{
    InputTransferStation::GetInstance().destroyed_ = true;
    InputTransferStation::GetInstance().RemoveInputWindow(window_->GetWindowId());

    InputTransferStation::GetInstance().destroyed_ = false;
    sptr<WindowInputChannel> inputChannel = sptr<WindowInputChannel>::MakeSptr(window_);
    InputTransferStation::GetInstance().windowInputChannels_.insert({ window_->GetWindowId(), inputChannel });
    InputTransferStation::GetInstance().RemoveInputWindow(window_->GetWindowId());
    auto iter = InputTransferStation::GetInstance().windowInputChannels_.find(window_->GetWindowId());
    ASSERT_EQ(iter, InputTransferStation::GetInstance().windowInputChannels_.end());
}

/**
 * @tc.name: OnInputEvent
 * @tc.desc: OnInputEvent keyEvent
 * @tc.type: FUNC
 */
HWTEST_F(InputTransferStationTest, OnInputEvent1, TestSize.Level1)
{
    auto keyEvent = MMI::KeyEvent::Create();
    auto tempKeyEvent = keyEvent;
    keyEvent = nullptr;
    listener->OnInputEvent(keyEvent);
    keyEvent = tempKeyEvent;
    InputTransferStation::GetInstance().destroyed_ = true;
    auto channel = InputTransferStation::GetInstance().GetInputChannel(0);
    listener->OnInputEvent(keyEvent);
    ASSERT_EQ(channel, nullptr);
}

/**
 * @tc.name: OnInputEvent
 * @tc.desc: OnInputEvent axisEvent
 * @tc.type: FUNC
 */
HWTEST_F(InputTransferStationTest, OnInputEvent2, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    auto axisEvent = MMI::AxisEvent::Create();
    auto tempAxisEvent = axisEvent;
    axisEvent = nullptr;
    listener->OnInputEvent(axisEvent);
    EXPECT_TRUE(g_errLog.find("AxisEvent is nullptr") != std::string::npos);
    axisEvent = tempAxisEvent;
    listener->OnInputEvent(axisEvent);
    EXPECT_FALSE(g_errLog.find("Receive axisEvent, windowId: %{public}d") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: OnInputEvent
 * @tc.desc: OnInputEvent pointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(InputTransferStationTest, OnInputEvent3, TestSize.Level1)
{
    auto pointerEvent = MMI::PointerEvent::Create();
    auto tempPointerEvent = pointerEvent;
    pointerEvent = nullptr;
    listener->OnInputEvent(pointerEvent);
    pointerEvent = tempPointerEvent;
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_DOWN));
    listener->OnInputEvent(pointerEvent);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_MOVE));
    pointerEvent->SetAgentWindowId(0);
    listener->OnInputEvent(pointerEvent);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_DOWN));
    pointerEvent->AddFlag(MMI::InputEvent::EVENT_FLAG_GESTURE_SUPPLEMENT);
    listener->OnInputEvent(pointerEvent);
    EXPECT_EQ(pointerEvent->GetSourceType(), MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    pointerEvent->SetAgentWindowId(static_cast<uint32_t>(-1));
    InputTransferStation::GetInstance().destroyed_ = true;
    auto channel = InputTransferStation::GetInstance().GetInputChannel(0);
    listener->OnInputEvent(pointerEvent);
    ASSERT_EQ(channel, nullptr);
}

/**
 * @tc.name: GetInputChannel
 * @tc.desc: GetInputChannel
 * @tc.type: FUNC
 */
HWTEST_F(InputTransferStationTest, GetInputChannel, TestSize.Level0)
{
    InputTransferStation::GetInstance().destroyed_ = true;
    auto channel = InputTransferStation::GetInstance().GetInputChannel(0);
    ASSERT_EQ(channel, nullptr);
    InputTransferStation::GetInstance().destroyed_ = false;
    ASSERT_EQ(channel, nullptr);
    InputTransferStation::GetInstance().AddInputWindow(window_);
    InputTransferStation::GetInstance().GetInputChannel(0);
}

/**
 * @tc.name: InjectTouchEvent
 * @tc.desc: InjectTouchEvent pointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(InputTransferStationTest, InjectTouchEvent, TestSize.Level1)
{
    auto pointerEvent = MMI::PointerEvent::Create();
    auto tempPointerEvent = pointerEvent;
    pointerEvent = nullptr;
    InputTransferStation::GetInstance().InjectTouchEvent(pointerEvent);
    pointerEvent = tempPointerEvent;
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_DOWN));
    InputTransferStation::GetInstance().InjectTouchEvent(pointerEvent);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_MOVE));
    pointerEvent->SetAgentWindowId(0);
    InputTransferStation::GetInstance().InjectTouchEvent(pointerEvent);
    InputTransferStation::GetInstance().destroyed_ = true;
    InputTransferStation::GetInstance().RemoveInputWindow(window_->GetWindowId());
    InputTransferStation::GetInstance().InjectTouchEvent(pointerEvent);
    auto channel = InputTransferStation::GetInstance().GetInputChannel(0);
    listener->OnInputEvent(pointerEvent);
    ASSERT_EQ(channel, nullptr);
}
 
class MockInputEventRecipientCallback : public IInputEventRecipientCallback {
public:
    explicit MockInputEventRecipientCallback(InputAfterRedistributeBehavior returnVal =
        InputAfterRedistributeBehavior::BEHAVIOR_NORMAL) : returnVal_(returnVal) {}
    InputAfterRedistributeBehavior OnInputEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
    {
        return returnVal_;
    }
    InputAfterRedistributeBehavior OnInputEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
    {
        return returnVal_;
    }
private:
    InputAfterRedistributeBehavior returnVal_;
};
 
/**
 * @tc.name: RegisterInputEventRedistribute
 * @tc.desc: Test Register and Unregister input event redistribute in WindowInputRedistributeImpl.
 * @tc.type: FUNC
 */
HWTEST_F(InputTransferStationTest, RegisterInputEventRedistribute, TestSize.Level1)
{
    IInputEventRecipientInfo validRecipientInfoNormal = {
        InputRedistributeIdentity::IDENTITY_MEDIA_CONTROLLER,
        InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT,
        InputEventType::KEY_EVENT,
        1,
        std::make_shared<MockInputEventRecipientCallback>(InputAfterRedistributeBehavior::BEHAVIOR_NORMAL)
    };
    IInputEventRecipientInfo validRecipientInfoIntercept = {
        InputRedistributeIdentity::IDENTITY_TOUCH_PREDICTOR,
        InputRedistributeTiming::REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT,
        InputEventType::POINTER_EVENT,
        2,
        std::make_shared<MockInputEventRecipientCallback>(InputAfterRedistributeBehavior::BEHAVIOR_INTERCEPT)
    };
    auto pointerEvent = MMI::PointerEvent::Create();
    auto keyEvent = MMI::KeyEvent::Create();

    listener->OnInputEvent(pointerEvent);
    listener->OnInputEvent(keyEvent);

    IInputEventRecipientInfo validRecipientInfoIntercept2 = validRecipientInfoIntercept;
    validRecipientInfoIntercept2.priority = 4;
 
    IInputEventRecipientInfo validRecipientInfoIntercept3 = validRecipientInfoIntercept;
    validRecipientInfoIntercept3.type = InputEventType::KEY_EVENT;

    WindowInputRedistributeClient::RegisterInputEventRedistribute(validRecipientInfoNormal);
    WindowInputRedistributeClient::RegisterInputEventRedistribute(validRecipientInfoIntercept);
    WindowInputRedistributeClient::RegisterInputEventRedistribute(validRecipientInfoIntercept2);
    WindowInputRedistributeClient::RegisterInputEventRedistribute(validRecipientInfoIntercept3);
    listener->OnInputEvent(pointerEvent);
    listener->OnInputEvent(keyEvent);
    auto channel = InputTransferStation::GetInstance().GetInputChannel(0);
    listener->OnInputEvent(pointerEvent);
    ASSERT_EQ(channel, nullptr);
    WindowInputRedistributeClient::UnRegisterInputEventRedistribute(validRecipientInfoNormal);
    WindowInputRedistributeClient::UnRegisterInputEventRedistribute(validRecipientInfoIntercept);
 
    IInputEventRecipientInfo invalidRecipientInfo = {
        InputRedistributeIdentity::IDENTITY_INVALID,
        InputRedistributeTiming::REDISTRIBUTE_INVALID_TIMING,
        InputEventType::INVALID,
        0,
        nullptr
    };
    WindowInputRedistributeClient::UnRegisterInputEventRedistribute(invalidRecipientInfo);
    WindowInputRedistributeClient::RegisterInputEventRedistribute(invalidRecipientInfo);
}
 
/**
 * @tc.name: SendEvent
 * @tc.desc: Test sending input events (KeyEvent and PointerEvent) in WindowInputRedistributeImpl.
 * @tc.type: FUNC
 */
HWTEST_F(InputTransferStationTest, SendEvent, TestSize.Level1)
{
    InputEventType eventType = InputEventType::INVALID;
    InputRedistributeTiming timing = InputRedistributeTiming::REDISTRIBUTE_INVALID_TIMING;
    WindowInputRedistributeImpl::GetInstance().GetRecipientResources(eventType, timing);
 
    auto keyEvent = MMI::KeyEvent::Create();
    auto tempKeyEvent = keyEvent;
    keyEvent = nullptr;
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT, keyEvent);
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT, keyEvent);
    WindowInputRedistributeClient::SendInputEvent(keyEvent);
    keyEvent = tempKeyEvent;
    WindowInputRedistributeImpl::GetInstance().SendEvent(timing, keyEvent);
    WindowInputRedistributeClient::SendInputEvent(keyEvent);
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT, keyEvent);
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT, keyEvent);
    
    auto pointerEvent = MMI::PointerEvent::Create();
    auto tempPointerEvent = pointerEvent;
    pointerEvent = nullptr;
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT, pointerEvent);
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT, pointerEvent);
    WindowInputRedistributeClient::SendInputEvent(pointerEvent);
    pointerEvent = tempPointerEvent;
    WindowInputRedistributeImpl::GetInstance().SendEvent(timing, pointerEvent);
    WindowInputRedistributeClient::SendInputEvent(pointerEvent);
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT, pointerEvent);
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_BEFORE_SEND_TO_COMPONENT, pointerEvent);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_DOWN));
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT, pointerEvent);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_MOVE));
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT, pointerEvent);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_PULL_MOVE));
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT, pointerEvent);
    pointerEvent->SetPointerAction(static_cast<int32_t>(MMI::PointerEvent::POINTER_ACTION_AXIS_UPDATE));
    WindowInputRedistributeImpl::GetInstance().SendEvent(
        InputRedistributeTiming::REDISTRIBUTE_AFTER_SEND_TO_COMPONENT, pointerEvent);
 
    auto channel = InputTransferStation::GetInstance().GetInputChannel(0);
    listener->OnInputEvent(pointerEvent);
    ASSERT_EQ(channel, nullptr);
}
} // namespace
}
} // namespace Rosen
} // namespace OHOS
