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
#include "gtx_input_event_sender.h"
#include "mock_window_adapter.h"
#include "singleton_mocker.h"
#include "window_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
using WindowMocker = SingletonMocker<WindowAdapter, MockWindowAdapter>;
class GtxInputEventSenderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
void GtxInputEventSenderTest::SetUpTestCase() {}

void GtxInputEventSenderTest::TearDownTestCase() {}

void GtxInputEventSenderTest::SetUp() {}

void GtxInputEventSenderTest::TearDown()
{
    GtxInputEventSender::GetInstance().UnRegisterInputEventScale();
}

namespace {
/**
 * @tc.name: GetTouchEvent
 * @tc.desc: GetTouchEvent.
 * @tc.type: FUNC
 */
HWTEST_F(GtxInputEventSenderTest, GetTouchEvent, TestSize.Level1)
{
    int32_t testPointerId = 1234;
    auto pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetPointerId(testPointerId);
    Rect testRect{};

    GtxTouchEventInfo touchEvent{};
    GtxInputEventSender::GetInstance().SetOpt(true);
    GtxInputEventSender::GetInstance().SetTouchEvent(testRect, pointerEvent);
    GtxInputEventSender::GetInstance().GetTouchEvent(touchEvent);
    GtxInputEventSender::GetInstance().SetOpt(false);

    EXPECT_EQ(testPointerId, touchEvent.pointerId);
}

/**
 * @tc.name: OnInputEventKeyEvent_Nullptr
 * @tc.desc: Test nullptr KeyEvent returns BEHAVIOR_NORMAL (nullptr branch).
 * @tc.type: FUNC
 */
HWTEST_F(GtxInputEventSenderTest, OnInputEventKeyEvent_Nullptr, TestSize.Level1)
{
    GtxInputEventSender::NapiAVSessionInputRedistributeCallback callback(0, 0, 1.0f, 1.0f);
    auto result = callback.OnInputEvent(std::shared_ptr<MMI::KeyEvent>(nullptr));
    EXPECT_EQ(result, Rosen::InputAfterRedistributeBehavior::BEHAVIOR_NORMAL);
}

/**
 * @tc.name: OnInputEventKeyEvent_Normal
 * @tc.desc: Test valid KeyEvent returns BEHAVIOR_NORMAL (normal branch).
 * @tc.type: FUNC
 */
HWTEST_F(GtxInputEventSenderTest, OnInputEventKeyEvent_Normal, TestSize.Level1)
{
    GtxInputEventSender::NapiAVSessionInputRedistributeCallback callback(0, 0, 1.0f, 1.0f);
    auto keyEvent = MMI::KeyEvent::Create();
    auto result = callback.OnInputEvent(keyEvent);
    EXPECT_EQ(result, Rosen::InputAfterRedistributeBehavior::BEHAVIOR_NORMAL);
}

/**
 * @tc.name: OnInputEventPointerEvent_Nullptr
 * @tc.desc: Test nullptr PointerEvent returns BEHAVIOR_NORMAL (nullptr branch).
 * @tc.type: FUNC
 */
HWTEST_F(GtxInputEventSenderTest, OnInputEventPointerEvent_Nullptr, TestSize.Level1)
{
    GtxInputEventSender::NapiAVSessionInputRedistributeCallback callback(0, 0, 1.0f, 1.0f);
    auto result = callback.OnInputEvent(std::shared_ptr<MMI::PointerEvent>(nullptr));
    EXPECT_EQ(result, Rosen::InputAfterRedistributeBehavior::BEHAVIOR_NORMAL);
}

/**
 * @tc.name: OnInputEventPointerEvent_Normal
 * @tc.desc: Test valid Pointer returns BEHAVIOR_NORMAL (normal branch).
 * @tc.type: FUNC
 */
HWTEST_F(GtxInputEventSenderTest, OnInputEventPointerEvent_Normal, TestSize.Level1)
{
    GtxInputEventSender::NapiAVSessionInputRedistributeCallback callback(0, 0, 1.0f, 1.0f);
    auto pointerEvent = MMI::PointerEvent::Create();
    auto result = callback.OnInputEvent(pointerEvent);
    EXPECT_EQ(result, Rosen::InputAfterRedistributeBehavior::BEHAVIOR_NORMAL);
}

/**
 * @tc.name: RegisterInputEventScale_FirstTime
 * @tc.desc: Test first time registration when mCallbackInstance is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(GtxInputEventSenderTest, RegisterInputEventScale_FirstTime, TestSize.Level1)
{
    auto& sender = GtxInputEventSender::GetInstance();

    EXPECT_EQ(sender.mCallbackInstance, nullptr);

    sender.RegisterInputEventScale(0, 0, 1.0f, 1.0f);

    EXPECT_NE(sender.mCallbackInstance, nullptr);

    sender.UnRegisterInputEventScale();
}

/**
 * @tc.name: RegisterInputEventScale_Repeat
 * @tc.desc: Test repeat registration when mCallbackInstance is not nullptr, should unregister first.
 * @tc.type: FUNC
 */
HWTEST_F(GtxInputEventSenderTest, RegisterInputEventScale_Repeat, TestSize.Level1)
{
    auto& sender = GtxInputEventSender::GetInstance();

    sender.RegisterInputEventScale(0, 0, 1.0f, 1.0f);
    auto callbackInstance1 = sender.mCallbackInstance;
    EXPECT_NE(sender.mCallbackInstance, nullptr);

    sender.RegisterInputEventScale(100, 100, 2.0f, 2.0f);
    auto callbackInstance2 = sender.mCallbackInstance;
    EXPECT_NE(callbackInstance2, nullptr);
    EXPECT_NE(callbackInstance1, callbackInstance2);

    sender.UnRegisterInputEventScale();
}
} // namespace
} // namespace Rosen
} // namespace OHOS