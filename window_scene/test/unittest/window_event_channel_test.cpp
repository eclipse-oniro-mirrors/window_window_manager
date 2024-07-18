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
#include <axis_event.h>
#include <key_event.h>
#include <pointer_event.h>
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "session/container/include/window_event_channel.h"
#include "scene_board_judgement.h"
#include "window_manager_hilog.h"
#include "iremote_object_mocker.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Accessibility;
using namespace std;

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowEventChannelTest" };
}
class WindowEventChannelTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    WSError TransferAccessibilityChildTreeRegister(bool isChannelNull);
    WSError TransferAccessibilityChildTreeUnregister(bool isChannelNull);
    WSError TransferAccessibilityDumpChildInfo(bool isChannelNull);

    sptr<ISessionStage> sessionStage = new SessionStageMocker();
    sptr<WindowEventChannel> windowEventChannel_ = new WindowEventChannelMocker(sessionStage);
};

void WindowEventChannelTest::SetUpTestCase()
{
}

void WindowEventChannelTest::TearDownTestCase()
{
}

void WindowEventChannelTest::SetUp()
{
}

void WindowEventChannelTest::TearDown()
{
}

WSError WindowEventChannelTest::TransferAccessibilityChildTreeRegister(bool isChannelNull)
{
    uint32_t windowId = 0;
    int32_t treeId = 0;
    int64_t accessibilityId = 0;
    if (isChannelNull) {
        windowEventChannel_->sessionStage_ = nullptr;
    }
    return windowEventChannel_->TransferAccessibilityChildTreeRegister(windowId, treeId, accessibilityId);
}

WSError WindowEventChannelTest::TransferAccessibilityChildTreeUnregister(bool isChannelNull)
{
    if (isChannelNull) {
        windowEventChannel_->sessionStage_ = nullptr;
    }
    return windowEventChannel_->TransferAccessibilityChildTreeUnregister();
}

WSError WindowEventChannelTest::TransferAccessibilityDumpChildInfo(bool isChannelNull)
{
    std::vector<std::string> params;
    std::vector<std::string> info;
    if (isChannelNull) {
        windowEventChannel_->sessionStage_ = nullptr;
    }
    return windowEventChannel_->TransferAccessibilityDumpChildInfo(params, info);
}

namespace {
/**
 * @tc.name: OnDispatchEventProcessed
 * @tc.desc: normal function OnDispatchEventProcessed
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, OnDispatchEventProcessed, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "WindowEventChannelTest: OnDispatchEventProcessed";
    int32_t eventId = 12;
    int64_t actionTime = 8888;
    uint32_t res = 0;
    WindowEventChannel::OnDispatchEventProcessed(eventId, actionTime);
    ASSERT_EQ(0, res);
    GTEST_LOG_(INFO) << "WindowEventChannelTest: OnDispatchEventProcessed";
}

/**
 * @tc.name: TransferKeyEvent
 * @tc.desc: normal function TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferKeyEvent, Function | SmallTest | Level2)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_BACK);
    keyEvent->SetKeyAction(MMI::KeyEvent::KEY_ACTION_UP);
    auto res = windowEventChannel_->TransferKeyEvent(keyEvent);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: TransferPointerEvent
 * @tc.desc: normal function TransferPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferPointerEvent, Function | SmallTest | Level2)
{
    auto pointerEvent = MMI::PointerEvent::Create();
    sptr<WindowEventChannel> windowEventChannel = new (std::nothrow) WindowEventChannel(sessionStage);
    ASSERT_NE(nullptr, windowEventChannel);

    auto res = windowEventChannel->TransferPointerEvent(pointerEvent);
    EXPECT_EQ(res, WSError::WS_OK);

    windowEventChannel->SetIsUIExtension(true);
    windowEventChannel->SetUIExtensionUsage(UIExtensionUsage::MODAL);
    res = windowEventChannel->TransferPointerEvent(pointerEvent);
    EXPECT_EQ(res, WSError::WS_OK);

    windowEventChannel->SetUIExtensionUsage(UIExtensionUsage::EMBEDDED);
    res = windowEventChannel->TransferPointerEvent(pointerEvent);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: TransferBackpressedEventForConsumed
 * @tc.desc: normal function TransferBackpressedEventForConsumed
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferBackpressedEventForConsumed, Function | SmallTest | Level2)
{
    bool isConsumed = false;
    auto res = windowEventChannel_->TransferBackpressedEventForConsumed(isConsumed);
    ASSERT_EQ(res, WSError::WS_OK);
    isConsumed = true;
    res = windowEventChannel_->TransferBackpressedEventForConsumed(isConsumed);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: TransferKeyEventForConsumed
 * @tc.desc: normal function TransferKeyEventForConsumed
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferKeyEventForConsumed, Function | SmallTest | Level2)
{
    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);

    bool isConsumed = false;
    auto res = windowEventChannel_->TransferKeyEventForConsumed(keyEvent, isConsumed, false);
    EXPECT_EQ(res, WSError::WS_OK);
    isConsumed = true;
    res = windowEventChannel_->TransferKeyEventForConsumed(keyEvent, isConsumed, false);
    EXPECT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "Test uiExtension key event with modal usage";
    windowEventChannel_->SetIsUIExtension(true);
    windowEventChannel_->SetUIExtensionUsage(UIExtensionUsage::MODAL);
    res = windowEventChannel_->TransferKeyEventForConsumed(keyEvent, isConsumed, false);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "Test single uiExtension key event with constrained embedded usage";
    windowEventChannel_->SetUIExtensionUsage(UIExtensionUsage::CONSTRAINED_EMBEDDED);
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_BACK);
    res = windowEventChannel_->TransferKeyEventForConsumed(keyEvent, isConsumed, false);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(res, WSError::WS_ERROR_INVALID_PERMISSION);
    } else {
        ASSERT_EQ(res, WSError::WS_OK);
    }

    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_TAB);
    res = windowEventChannel_->TransferKeyEventForConsumed(keyEvent, isConsumed, false);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "Test combined uiExtension key event with constrained embedded usage";
    auto keyItemTab = MMI::KeyEvent::KeyItem();
    keyItemTab.SetKeyCode(MMI::KeyEvent::KEYCODE_TAB);
    keyItemTab.SetPressed(true);
    auto keyItemTest = MMI::KeyEvent::KeyItem();
    keyItemTest.SetKeyCode(MMI::KeyEvent::KEYCODE_SHIFT_RIGHT);
    keyItemTest.SetPressed(true);
    keyEvent->AddPressedKeyItems(keyItemTest);
    keyEvent->AddPressedKeyItems(keyItemTab);
    res = windowEventChannel_->TransferKeyEventForConsumed(keyEvent, isConsumed, false);
    ASSERT_EQ(res, WSError::WS_OK);

    keyEvent->Reset();
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_TAB);
    keyItemTest.SetKeyCode(MMI::KeyEvent::KEYCODE_ALT_LEFT);
    keyEvent->AddPressedKeyItems(keyItemTest);
    keyEvent->AddPressedKeyItems(keyItemTab);
    res = windowEventChannel_->TransferKeyEventForConsumed(keyEvent, isConsumed, false);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(res, WSError::WS_ERROR_INVALID_PERMISSION);
    } else {
        ASSERT_EQ(res, WSError::WS_OK);
    }
}

/**
 * @tc.name: TransferKeyEventForConsumedAsync01
 * @tc.desc: normal function TransferKeyEventForConsumedAsync01
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferKeyEventForConsumedAsync01, Function | SmallTest | Level2)
{
    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    bool isPreImeEvent = false;

    sptr<ISessionStage> sessionStage = new SessionStageMocker();
    sptr<WindowEventChannel> windowEventChannel = new WindowEventChannel(sessionStage);
    ASSERT_NE(windowEventChannel, nullptr);
    auto res = windowEventChannel->TransferKeyEventForConsumedAsync(keyEvent, isPreImeEvent, nullptr);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: TransferKeyEventForConsumedAsync02
 * @tc.desc: normal function TransferKeyEventForConsumedAsync02
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferKeyEventForConsumedAsync02, Function | SmallTest | Level2)
{
    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    bool isPreImeEvent = false;

    sptr<ISessionStage> sessionStage = new SessionStageMocker();
    sptr<WindowEventChannel> windowEventChannel = new WindowEventChannel(sessionStage);
    ASSERT_NE(windowEventChannel, nullptr);
    sptr<IRemoteObject> iRemoteObjectMocker = new (std::nothrow) IRemoteObjectMocker();
    auto res = windowEventChannel->TransferKeyEventForConsumedAsync(keyEvent, isPreImeEvent, iRemoteObjectMocker);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: WindowEventChannelListenerProxyOnTransferKeyEventForConsumed
 * @tc.desc: normal function WindowEventChannelListenerProxyOnTransferKeyEventForConsumed
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, WindowEventChannelListenerProxyOnTransferKeyEventForConsumed,
    Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = new (std::nothrow) IRemoteObjectMocker();
    WindowEventChannelListenerProxy listenerProxy(iRemoteObjectMocker);
    listenerProxy.OnTransferKeyEventForConsumed(100, true, true, WSError::WS_OK);
}

/**
 * @tc.name: TransferFocusActiveEvent
 * @tc.desc: normal function TransferFocusActiveEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferFocusActiveEvent, Function | SmallTest | Level2)
{
    bool isFocusActive = false;
    windowEventChannel_->sessionStage_ = nullptr;
    auto res = windowEventChannel_->TransferFocusActiveEvent(isFocusActive);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: PrintKeyEvent
 * @tc.desc: normal function PrintKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, PrintKeyEvent, Function | SmallTest | Level2)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_TRUE((windowEventChannel_ != nullptr));
    windowEventChannel_->PrintKeyEvent(keyEvent);
    windowEventChannel_->sessionStage_ = nullptr;
    windowEventChannel_->PrintKeyEvent(keyEvent);
    windowEventChannel_->PrintKeyEvent(nullptr);
}

/**
 * @tc.name: PrintPointerEvent
 * @tc.desc: normal function PrintPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, PrintPointerEvent, Function | SmallTest | Level2)
{
    auto pointerEvent = MMI::PointerEvent::Create();
    ASSERT_TRUE((windowEventChannel_ != nullptr));
    windowEventChannel_->PrintPointerEvent(pointerEvent);
    windowEventChannel_->PrintPointerEvent(nullptr);
}

/**
 * @tc.name: TransferFocusState
 * @tc.desc: normal function TransferFocusState
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferFocusState, Function | SmallTest | Level2)
{
    bool focusState = false;
    windowEventChannel_->sessionStage_ = nullptr;
    ASSERT_TRUE((windowEventChannel_ != nullptr));
    auto res = windowEventChannel_->TransferFocusState(focusState);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferAccessibilityChildTreeRegister01
 * @tc.desc: normal function TransferAccessibilityChildTreeRegister01
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferAccessibilityChildTreeRegister01, Function | SmallTest | Level2)
{
    WLOGFI("TransferAccessibilityChildTreeRegister01 begin");
    auto res = TransferAccessibilityChildTreeRegister(true);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
    WLOGFI("TransferAccessibilityChildTreeRegister01 end");
}

/**
 * @tc.name: TransferAccessibilityChildTreeRegister01
 * @tc.desc: normal function TransferAccessibilityChildTreeRegister01
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferAccessibilityChildTreeRegister02, Function | SmallTest | Level2)
{
    WLOGFI("TransferAccessibilityChildTreeRegister02 begin");
    auto res = TransferAccessibilityChildTreeRegister(false);
    ASSERT_EQ(res, WSError::WS_OK);
    WLOGFI("TransferAccessibilityChildTreeRegister02 end");
}

/**
 * @tc.name: TransferAccessibilityChildTreeUnregister01
 * @tc.desc: normal function TransferAccessibilityChildTreeUnregister01
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferAccessibilityChildTreeUnregister01, Function | SmallTest | Level2)
{
    WLOGFI("TransferAccessibilityChildTreeUnregister01 begin");
    auto res = TransferAccessibilityChildTreeUnregister(true);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
    WLOGFI("TransferAccessibilityChildTreeUnregister01 end");
}

/**
 * @tc.name: TransferAccessibilityChildTreeUnregister02
 * @tc.desc: normal function TransferAccessibilityChildTreeUnregister02
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferAccessibilityChildTreeUnregister02, Function | SmallTest | Level2)
{
    WLOGFI("TransferAccessibilityChildTreeUnregister02 begin");
    auto res = TransferAccessibilityChildTreeUnregister(false);
    ASSERT_EQ(res, WSError::WS_OK);
    WLOGFI("TransferAccessibilityChildTreeUnregister02 end");
}

/**
 * @tc.name: TransferAccessibilityDumpChildInfo01
 * @tc.desc: normal function TransferAccessibilityDumpChildInfo01
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferAccessibilityDumpChildInfo01, Function | SmallTest | Level2)
{
    WLOGFI("TransferAccessibilityDumpChildInfo01 begin");
    auto res = TransferAccessibilityDumpChildInfo(true);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
    WLOGFI("TransferAccessibilityDumpChildInfo01 end");
}

/**
 * @tc.name: TransferAccessibilityDumpChildInfo02
 * @tc.desc: normal function TransferAccessibilityDumpChildInfo02
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferAccessibilityDumpChildInfo02, Function | SmallTest | Level2)
{
    WLOGFI("TransferAccessibilityDumpChildInfo02 begin");
    auto res = TransferAccessibilityDumpChildInfo(false);
    ASSERT_EQ(res, WSError::WS_OK);
    WLOGFI("TransferAccessibilityDumpChildInfo02 end");
}

/**
 * @tc.name: TransferExecuteAction02
 * @tc.desc: normal function TransferExecuteAction02
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferExecuteAction03, Function | SmallTest | Level2)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = std::make_shared<MMI::KeyEvent>(0);
    auto res = windowEventChannel_->TransferKeyEvent(keyEvent);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: TransferPointerEvent
 * @tc.desc: normal function TransferPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferPointerEvent01, Function | SmallTest | Level2)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = std::make_shared<MMI::PointerEvent>(0);
    auto ret = windowEventChannel_->TransferPointerEvent(pointerEvent);
    ASSERT_NE(WSError::WS_ERROR_NULLPTR, ret);

    windowEventChannel_->sessionStage_ = new SessionStageMocker();
    pointerEvent = nullptr;
    ret = windowEventChannel_->TransferPointerEvent(pointerEvent);
    ASSERT_NE(WSError::WS_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: TransferPointerEvent
 * @tc.desc: normal function TransferPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferBackpressedEventForConsumed01, Function | SmallTest | Level2)
{
    bool isConsumed = true;
    windowEventChannel_->sessionStage_ = nullptr;
    auto ret = windowEventChannel_->TransferBackpressedEventForConsumed(isConsumed);

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, ret);
    windowEventChannel_->sessionStage_ = new SessionStageMocker();
    windowEventChannel_->TransferBackpressedEventForConsumed(isConsumed);
}

/**
 * @tc.name: TransferKeyEventForConsumed
 * @tc.desc: normal function TransferKeyEventForConsumed
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferKeyEventForConsumed01, Function | SmallTest | Level2)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = nullptr;
    bool isConsumed = true;
    bool isPreImeEvent = true;
    windowEventChannel_->sessionStage_ = nullptr;
    auto ret = windowEventChannel_->TransferKeyEventForConsumed(keyEvent, isConsumed, isPreImeEvent);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, ret);

    windowEventChannel_->sessionStage_ = new SessionStageMocker();
    windowEventChannel_->TransferKeyEventForConsumed(keyEvent, isConsumed, isPreImeEvent);

    keyEvent = std::make_shared<MMI::KeyEvent>(0);
    windowEventChannel_->TransferKeyEventForConsumed(keyEvent, isConsumed, isPreImeEvent);
    ASSERT_NE(WSError::WS_OK, ret);

    isPreImeEvent = false;
    windowEventChannel_->TransferKeyEventForConsumed(keyEvent, isConsumed, isPreImeEvent);

    std::shared_ptr<MMI::KeyEvent> event = nullptr;
    windowEventChannel_->PrintKeyEvent(event);

    event = std::make_shared<MMI::KeyEvent>(0);
    windowEventChannel_->PrintKeyEvent(event);
}

/**
 * @tc.name: TransferAccessibilityHoverEvent
 * @tc.desc: normal function TransferAccessibilityHoverEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowEventChannelTest, TransferAccessibilityHoverEvent, Function | SmallTest | Level2)
{
    std::shared_ptr<MMI::PointerEvent> event = std::make_shared<MMI::PointerEvent>(0);
    windowEventChannel_->PrintInfoPointerEvent(event);
    event = nullptr;
    windowEventChannel_->PrintInfoPointerEvent(event);

    float pointX = 0.0;
    float pointY = 0.0;
    int32_t sourceType = 0;
    int32_t eventType = 0;
    int64_t timeMs = 0;

    windowEventChannel_->sessionStage_ = nullptr;
    auto ret = windowEventChannel_->TransferAccessibilityHoverEvent(pointX, pointY, sourceType, eventType, timeMs);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, ret);

    windowEventChannel_->sessionStage_ = new SessionStageMocker();
    windowEventChannel_->TransferAccessibilityHoverEvent(pointX, pointY, sourceType, eventType, timeMs);

    std::shared_ptr<MMI::PointerEvent> pevent = nullptr;
    windowEventChannel_->PrintPointerEvent(pevent);
    pevent = std::make_shared<MMI::PointerEvent>(0);
    windowEventChannel_->PrintPointerEvent(pevent);
}
}
}
