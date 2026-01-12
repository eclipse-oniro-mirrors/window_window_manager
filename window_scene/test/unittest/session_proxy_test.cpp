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

#include "session_proxy.h"
#include "iremote_object_mocker.h"
#include <gtest/gtest.h>
#include "accessibility_event_info.h"
#include "ws_common.h"
#include "mock_message_parcel.h"
#include "pointer_event.h"
#include "feature/window_keyframe/rs_window_keyframe_node.h"
#include "transaction/rs_transaction.h"
#include "window_manager_hilog.h"

// using namespace FRAME_TRACE;
using namespace testing;
using namespace testing::ext;

namespace {
    std::string logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        logMsg = msg;
    }
}

namespace OHOS {
namespace Rosen {
class SessionProxyTest : public testing::Test {
public:
    SessionProxyTest() {}
    ~SessionProxyTest() {}
};
namespace {
/**
 * @tc.name: TestOnSessionEvent
 * @tc.desc: Verify OnSessionEvent handles various IPC conditions correctly
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, TestOnSessionEvent, TestSize.Level1)
{
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    SessionEvent event = SessionEvent::EVENT_MAXIMIZE;
    SessionEventParam param { .waterfallResidentState = 0 };

    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->OnSessionEvent(event, param));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Case 2: Failed to write eventId or waterfallResidentState
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->OnSessionEvent(event, param));
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    // Case 3: remote is nullptr
    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, nullProxy->OnSessionEvent(event, param));

    // Case 4: Failed to send request
    mockRemote->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionProxy> failProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, failProxy->OnSessionEvent(event, param));

    // Case 5: Success when event is EVENT_MAXIMIZE
    mockRemote->sendRequestResult_ = ERR_NONE;
    sptr<SessionProxy> okProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_OK, okProxy->OnSessionEvent(event, param));

    // Case 6: Success when event is not EVENT_MAXIMIZE
    event = SessionEvent::EVENT_MINIMIZE;
    EXPECT_EQ(WSError::WS_OK, okProxy->OnSessionEvent(event, param));
}

/**
 * @tc.name: TestOnSessionEvent
 * @tc.desc: Verify OnSessionEvent handles various IPC conditions correctly
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, TestOnSessionEvent02, TestSize.Level1)
{
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    SessionEvent event = SessionEvent::EVENT_SWITCH_COMPATIBLE_MODE;
    SessionEventParam param { .compatibleStyleMode = 0 };
 
    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->OnSessionEvent(event, param));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
 
    // Case 2: Failed to write eventId or compatibleStyleMode
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->OnSessionEvent(event, param));
    MockMessageParcel::SetWriteUint32ErrorFlag(false);
 
    // Case 3: remote is nullptr
    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, nullProxy->OnSessionEvent(event, param));
 
    // Case 4: Failed to send request
    mockRemote->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionProxy> failProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, failProxy->OnSessionEvent(event, param));
 
    // Case 5: Success when event is EVENT_MAXIMIZE
    mockRemote->sendRequestResult_ = ERR_NONE;
    sptr<SessionProxy> okProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_OK, okProxy->OnSessionEvent(event, param));
 
    // Case 6: Success when event is not EVENT_MAXIMIZE
    event = SessionEvent::EVENT_MINIMIZE;
    EXPECT_EQ(WSError::WS_OK, okProxy->OnSessionEvent(event, param));
}

/**
 * @tc.name: UpdateSessionRect
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateSessionRect, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateSessionRect start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    WSRect rect{ .posX_ = 1, .posY_ = 1, .width_ = 100, .height_ = 100 };
    SizeChangeReason reason = SizeChangeReason::RECOVER;
    WSError res = sProxy->UpdateSessionRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateSessionRect end";
}

/**
 * @tc.name: Restore
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, OnRestoreMainWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: OnRestoreMainWindow start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    WSError res = sProxy->OnRestoreMainWindow();
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: OnRestoreMainWindow end";
}

/**
 * @tc.name: RestoreFloatMainWindow
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, RestoreFloatMainWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: RestoreFloatMainWindow start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    std::shared_ptr<AAFwk::WantParams> wantParams = std::shared_ptr<AAFwk::WantParams>();

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_EQ(sProxy->RestoreFloatMainWindow(wantParams), WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    ASSERT_EQ(sProxy->RestoreFloatMainWindow(wantParams), WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteParcelableErrorFlag(false);

    auto remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    remoteMocker->sendRequestResult_ = 1;
    sptr<SessionProxy> failProxy_ = sptr<SessionProxy>::MakeSptr(remoteMocker);
    ASSERT_EQ(failProxy_->RestoreFloatMainWindow(wantParams), WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::SetReadUint32ErrorFlag(true);
    ASSERT_EQ(sProxy->RestoreFloatMainWindow(wantParams), WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadUint32ErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();
    ASSERT_EQ(sProxy->RestoreFloatMainWindow(wantParams), WMError::WM_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: RestoreFloatMainWindow end";
}

/**
 * @tc.name: RaiseToAppTop
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, RaiseToAppTop, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: RaiseToAppTop start";
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    WSError res = sProxy->RaiseToAppTop();
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    sptr<SessionProxy> tempProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = tempProxy->RaiseToAppTop();
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    res = sProxy->RaiseToAppTop();
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    res = sProxy->RaiseToAppTop();
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: RaiseToAppTop end";
}

/**
 * @tc.name: RaiseAboveTarget
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, RaiseAboveTarget, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: RaiseAboveTarget start";
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SessionProxy> proxy = sptr<SessionProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    int32_t subWindowId = 0;
    WSError res = proxy->RaiseAboveTarget(subWindowId);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    sptr<SessionProxy> tempProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = tempProxy->RaiseAboveTarget(subWindowId);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    res = proxy->RaiseAboveTarget(subWindowId);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    res = proxy->RaiseAboveTarget(subWindowId);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: RaiseAboveTarget end";
}

/**
 * @tc.name: OnNeedAvoid
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, OnNeedAvoid, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: OnNeedAvoid start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    bool status = false;
    WSError res = sProxy->OnNeedAvoid(status);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: OnNeedAvoid end";
}

/**
 * @tc.name: GetAvoidAreaByTypeIgnoringVisibility
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetAvoidAreaByTypeIgnoringVisibility, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: GetAvoidAreaByTypeIgnoringVisibility start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM;
    AvoidArea res = sProxy->GetAvoidAreaByTypeIgnoringVisibility(type);
    AvoidArea empty;
    EXPECT_EQ(res, empty);
    GTEST_LOG_(INFO) << "SessionProxyTest: GetAvoidAreaByTypeIgnoringVisibility end";
}

/**
 * @tc.name: RequestSessionBack
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, RequestSessionBack, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: RequestSessionBack start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    bool needMoveToBackground = true;
    WSError res = sProxy->RequestSessionBack(needMoveToBackground);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: RequestSessionBack end";
}

/**
 * @tc.name: MarkProcessed
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, MarkProcessed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: MarkProcessed start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    int32_t eventId = 0;
    WSError res = sProxy->MarkProcessed(eventId);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: MarkProcessed end";
}

/**
 * @tc.name: SetGlobalMaximizeMode
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SetGlobalMaximizeMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SetGlobalMaximizeMode start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    MaximizeMode mode = MaximizeMode::MODE_AVOID_SYSTEM_BAR;
    WSError res = sProxy->SetGlobalMaximizeMode(mode);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: SetGlobalMaximizeMode end";
}

/**
 * @tc.name: GetGlobalMaximizeMode
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetGlobalMaximizeMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: GetGlobalMaximizeMode start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    MaximizeMode mode = MaximizeMode::MODE_AVOID_SYSTEM_BAR;
    WSError res = sProxy->GetGlobalMaximizeMode(mode);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: GetGlobalMaximizeMode end";
}

/**
 * @tc.name: SetAspectRatio
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SetAspectRatio, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SetAspectRatio start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    float ratio = 10;
    WSError res = sProxy->SetAspectRatio(ratio);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: SetAspectRatio end";
}

/**
 * @tc.name: UpdateWindowSceneAfterCustomAnimation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateWindowSceneAfterCustomAnimation, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateWindowSceneAfterCustomAnimation start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    bool isAdd = false;
    WSError res = sProxy->UpdateWindowSceneAfterCustomAnimation(isAdd);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateWindowSceneAfterCustomAnimation end";
}

/**
 * @tc.name: SetSystemWindowEnableDrag
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SetSystemWindowEnableDrag, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SetSystemWindowEnableDrag start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    bool enableDrag = false;
    WMError res = sProxy->SetSystemWindowEnableDrag(enableDrag);
    ASSERT_EQ(res, WMError::WM_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: SetSystemWindowEnableDrag end";
}

/**
 * @tc.name: TransferAbilityResult
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, TransferAbilityResult, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: TransferAbilityResult start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    uint32_t resultCode = 0;
    AAFwk::Want want;
    WSError res = sProxy->TransferAbilityResult(resultCode, want);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: TransferAbilityResult end";
}

/**
 * @tc.name: NotifyExtensionDied
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyExtensionDied, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyExtensionDied start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    sProxy->NotifyExtensionDied();

    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyExtensionDied end";
}

/**
 * @tc.name: NotifyExtensionTimeout
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyExtensionTimeout, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyExtensionTimeout start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    sProxy->NotifyExtensionTimeout(2);

    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyExtensionTimeout end";
}

/**
 * @tc.name: UpdateWindowAnimationFlag
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateWindowAnimationFlag, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateWindowAnimationFlag start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    bool needDefaultAnimationFlag = false;
    WSError res = sProxy->UpdateWindowAnimationFlag(needDefaultAnimationFlag);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateWindowAnimationFlag end";
}

/**
 * @tc.name: TransferAccessibilityEvent
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, TransferAccessibilityEvent, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: TransferAccessibilityEvent start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    Accessibility::AccessibilityEventInfo info;
    int64_t uiExtensionIdLevel = 0;
    WSError res = sProxy->TransferAccessibilityEvent(info, uiExtensionIdLevel);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: TransferAccessibilityEvent end";
}

/**
 * @tc.name: OnTitleAndDockHoverShowChange
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, OnTitleAndDockHoverShowChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: OnTitleAndDockHoverShowChange start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    bool isTitleHoverShown = true;
    bool isDockHoverShown = true;
    WSError res = sProxy->OnTitleAndDockHoverShowChange(isTitleHoverShown, isDockHoverShown);
    EXPECT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: OnTitleAndDockHoverShowChange end";
}

/**
 * @tc.name: UpdatePiPControlStatus
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdatePiPControlStatus, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdatePiPControlStatus start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    auto controlType = WsPiPControlType::VIDEO_PLAY_PAUSE;
    auto status = WsPiPControlStatus::PLAY;
    WSError res = sProxy->UpdatePiPControlStatus(controlType, status);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdatePiPControlStatus end";
}

/**
 * @tc.name: SetAutoStartPiP
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionProxyTest, SetAutoStartPiP, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAutoStartPiP: SetAutoStartPiP start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    bool isAutoStartValid = true;
    uint32_t priority = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    ASSERT_EQ(WSError::WS_OK, sProxy->SetAutoStartPiP(isAutoStartValid, priority, width, height));
    GTEST_LOG_(INFO) << "SetAutoStartPiP: SetAutoStartPiP end";
}

/**
 * @tc.name: UpdatePiPTemplateInfo
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionProxyTest, UpdatePiPTemplateInfo, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "UpdatePiPTemplateInfo: UpdatePiPTemplateInfo start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    PiPTemplateInfo templateInfo;
    ASSERT_EQ(WSError::WS_OK, sProxy->UpdatePiPTemplateInfo(templateInfo));
    GTEST_LOG_(INFO) << "UpdatePiPTemplateInfo: UpdatePiPTemplateInfo end";
}

/**
 * @tc.name: SetPipParentWindowId
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionProxyTest, SetPipParentWindowId, Function | SmallTest | Level2)
{
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    uint32_t windowId = 100;
 
    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->SetPipParentWindowId(windowId));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
 
    // Case 2: Failed to write windowId
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->SetPipParentWindowId(windowId));
    MockMessageParcel::SetWriteUint32ErrorFlag(false);
 
    // Case 3: remote is nullptr
    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, nullProxy->SetPipParentWindowId(windowId));
 
    // Case 4: Failed to send request
    mockRemote->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionProxy> failProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, failProxy->SetPipParentWindowId(windowId));
 
    // Case 5: Success
    mockRemote->sendRequestResult_ = ERR_NONE;
    sptr<SessionProxy> okProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_OK, okProxy->SetPipParentWindowId(windowId));
}

/**
 * @tc.name: IsPiPActive
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionProxyTest, IsPiPActive, Function | SmallTest | Level2)
{
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);

    bool status = false;
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(sessionProxy->IsPiPActive(status), WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    EXPECT_EQ(nullProxy->IsPiPActive(status), WMError::WM_ERROR_IPC_FAILED);

    mockRemote->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionProxy> failProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(failProxy->IsPiPActive(status), WMError::WM_ERROR_IPC_FAILED);

    mockRemote->sendRequestResult_ = ERR_NONE;
    sptr<SessionProxy> okProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    EXPECT_EQ(okProxy->IsPiPActive(status), WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadInt32ErrorFlag(false);

    MockMessageParcel::SetReadBoolErrorFlag(true);
    EXPECT_EQ(okProxy->IsPiPActive(status), WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadBoolErrorFlag(false);
    EXPECT_EQ(okProxy->IsPiPActive(status), WMError::WM_OK);
}

/**
 * @tc.name: SetWindowAnchorInfo
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionProxyTest, SetWindowAnchorInfo, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SetWindowAnchorInfo: SetWindowAnchorInfo start";
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto sProxy = sptr<SessionProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(sProxy, nullptr);
    WindowAnchorInfo anchorInfo;
    WSError res = sProxy->SetWindowAnchorInfo(anchorInfo);
    ASSERT_EQ(res, WSError::WS_OK);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    res = sProxy->SetWindowAnchorInfo(anchorInfo);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    res = sProxy->SetWindowAnchorInfo(anchorInfo);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteParcelableErrorFlag(false);

    sptr<SessionProxy> tempProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = tempProxy->SetWindowAnchorInfo(anchorInfo);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    res = sProxy->SetWindowAnchorInfo(anchorInfo);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::SetReadInt32ErrorFlag(true);
    res = sProxy->SetWindowAnchorInfo(anchorInfo);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadInt32ErrorFlag(false);

    MockMessageParcel::ClearAllErrorFlag();
    GTEST_LOG_(INFO) << "SetWindowAnchorInfo: SetWindowAnchorInfo end";
}

/**
 * @tc.name: SetWindowTransitionAnimation
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionProxyTest, SetWindowTransitionAnimation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SetWindowTransitionAnimation: SetWindowTransitionAnimation start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    TransitionAnimation animation;
    WSError res = sProxy->SetWindowTransitionAnimation(WindowTransitionType::DESTROY, animation);
    ASSERT_EQ(res, WSError::WS_OK);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    res = sProxy->SetWindowTransitionAnimation(WindowTransitionType::DESTROY, animation);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    res = sProxy->SetWindowTransitionAnimation(WindowTransitionType::DESTROY, animation);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    res = sProxy->SetWindowTransitionAnimation(WindowTransitionType::DESTROY, animation);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteParcelableErrorFlag(false);

    MockMessageParcel::SetReadInt32ErrorFlag(true);
    res = sProxy->SetWindowTransitionAnimation(WindowTransitionType::DESTROY, animation);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadInt32ErrorFlag(false);

    MockMessageParcel::ClearAllErrorFlag();
    GTEST_LOG_(INFO) << "SetWindowTransitionAnimation: SetWindowTransitionAnimation end";
}

/**
 * @tc.name: GetGlobalScaledRect
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetGlobalScaledRect, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: GetGlobalScaledRect start";
    Rect rect;
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    WMError res = sProxy->GetGlobalScaledRect(rect);
    ASSERT_EQ(res, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    sptr<SessionProxy> tempProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = tempProxy->GetGlobalScaledRect(rect);
    ASSERT_EQ(res, WMError::WM_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    res = sProxy->GetGlobalScaledRect(rect);
    ASSERT_EQ(res, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::SetReadInt32ErrorFlag(true);
    MockMessageParcel::SetReadUint32ErrorFlag(true);
    res = sProxy->GetGlobalScaledRect(rect);
    ASSERT_EQ(res, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    res = sProxy->GetGlobalScaledRect(rect);
    ASSERT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: GetGlobalScaledRect end";
}

/**
 * @tc.name: GetStatusBarHeight
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetStatusBarHeight, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: GetStatusBarHeight start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    int32_t res = sProxy->GetStatusBarHeight();
    ASSERT_EQ(res, 0);
    GTEST_LOG_(INFO) << "SessionProxyTest: GetStatusBarHeight end";
}

/**
 * @tc.name: SetDialogSessionBackGestureEnabled
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SetDialogSessionBackGestureEnabled, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SetDialogSessionBackGestureEnabled start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    WSError res = sProxy->SetDialogSessionBackGestureEnabled(true);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: SetDialogSessionBackGestureEnabled end";
}

/**
 * @tc.name: GetAppForceLandscapeConfig
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetAppForceLandscapeConfig, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: GetAppForceLandscapeConfig start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    AppForceLandscapeConfig config = {};
    auto res = sProxy->GetAppForceLandscapeConfig(config);
    ASSERT_EQ(res, WMError::WM_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: GetAppForceLandscapeConfig end";
}

/**
 * @tc.name: NotifyExtensionEventAsync
 * @tc.desc: NotifyExtensionEventAsync test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyExtensionEventAsync, TestSize.Level1)
{
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);
    sProxy->NotifyExtensionEventAsync(0);

    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    sProxy->NotifyExtensionEventAsync(0);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    sProxy->NotifyExtensionEventAsync(0);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sProxy->NotifyExtensionEventAsync(0);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: RequestFocus
 * @tc.desc: RequestFocus Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, RequestFocus, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: RequestFocus start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    WSError res = sProxy->RequestFocus(true);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: RequestFocus end";
}

/**
 * @tc.name: UpdateClientRect01
 * @tc.desc: UpdateClientRect test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateClientRect01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateClientRect01 start";
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    WSRect rect = { 200, 200, 200, 200 };
    ASSERT_EQ(sProxy->UpdateClientRect(rect), WSError::WS_ERROR_IPC_FAILED);

    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_EQ(sProxy->UpdateClientRect(rect), WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateClientRect01 start";
}

/**
 * @tc.name: TransferExtensionData
 * @tc.desc: TransferExtensionData test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, TransferExtensionData, TestSize.Level1)
{
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);
    AAFwk::WantParams wantParams;
    auto res = sProxy->TransferExtensionData(wantParams);
    ASSERT_EQ(res, IPC_PROXY_ERR);

    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);

    res = sProxy->TransferExtensionData(wantParams);
    ASSERT_EQ(res, ERR_NONE);
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    res = sProxy->TransferExtensionData(wantParams);
    ASSERT_EQ(res, IPC_PROXY_ERR);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    res = sProxy->TransferExtensionData(wantParams);
    ASSERT_EQ(res, IPC_PROXY_ERR);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: NotifyAsyncOn
 * @tc.desc: NotifyAsyncOn test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyAsyncOn, TestSize.Level1)
{
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);
    sProxy->NotifyAsyncOn();

    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    sProxy->NotifyAsyncOn();

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sProxy->NotifyAsyncOn();
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: OnSetWindowRectAutoSave
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, OnSetWindowRectAutoSave, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: OnSetWindowRectAutoSave start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    WSError res = sProxy->OnSetWindowRectAutoSave(true, false);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: OnSetWindowRectAutoSave end";
}

/**
 * @tc.name: NotifyMainModalTypeChange
 * @tc.desc: NotifyMainModalTypeChange test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyMainModalTypeChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyMainModalTypeChange start";
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_EQ(sProxy->NotifyMainModalTypeChange(true), WSError::WS_ERROR_IPC_FAILED);
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_EQ(sProxy->NotifyMainModalTypeChange(true), WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyMainModalTypeChange end";
}

/**
 * @tc.name: Foreground
 * @tc.desc: Foreground test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, Foreground, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: Foreground start";
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    bool isFromClient = true;
    std::string identityToken = "foregroundTest";
    WSError ret = sProxy->Foreground(property, isFromClient, identityToken);
    ASSERT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: Foreground end";
}

/**
 * @tc.name: Foreground02
 * @tc.desc: Foreground test property is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, Foreground02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: Foreground02 start";
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);

    bool isFromClient = true;
    std::string identityToken = "foregroundTest";
    WSError ret = sProxy->Foreground(nullptr, isFromClient, identityToken);
    ASSERT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: Foreground02 end";
}

/**
 * @tc.name: Foreground03
 * @tc.desc: Foreground test isFromClient is false
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, Foreground03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: Foreground03 start";
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    bool isFromClient = false;
    std::string identityToken = "foregroundTest";
    WSError ret = sProxy->Foreground(property, isFromClient, identityToken);
    ASSERT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: Foreground03 end";
}

/**
 * @tc.name: Background
 * @tc.desc: Background test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, Background, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: Background start";
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);

    bool isFromClient = true;
    std::string identityToken = "backgroundTest";
    WSError ret = sProxy->Background(isFromClient, identityToken);
    ASSERT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: Background end";
}

/**
 * @tc.name: Background02
 * @tc.desc: Background test isFromClient is false
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, Background02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: Background02 start";
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);

    bool isFromClient = false;
    std::string identityToken = "backgroundTest";
    WSError ret = sProxy->Background(isFromClient, identityToken);
    ASSERT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: Background02 end";
}

/**
 * @tc.name: Show
 * @tc.desc: Show Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, Show, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: Show start";
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSError ret = sProxy->Show(property);
    ASSERT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);

    // property is nullptr
    ret = sProxy->Show(nullptr);
    ASSERT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: Show end";
}

/**
 * @tc.name: Hide
 * @tc.desc: Hide Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, Hide, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: Hide start";
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);

    WSError ret = sProxy->Hide();
    ASSERT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: Hide end";
}

/**
 * @tc.name: Disconnect
 * @tc.desc: Disconnect Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, Disconnect, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: Disconnect start";
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);

    bool isFromClient = true;
    std::string identityToken = "disconnectTest";
    WSError ret = sProxy->Disconnect(isFromClient, identityToken);
    ASSERT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);

    // isFromClient is false
    isFromClient = false;
    ret = sProxy->Disconnect(isFromClient, identityToken);
    ASSERT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: Disconnect end";
}

/**
 * @tc.name: DrawingCompleted
 * @tc.desc: DrawingCompleted Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, DrawingCompleted, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: DrawingCompleted start";
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);

    WSError ret = sProxy->DrawingCompleted();
    ASSERT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: DrawingCompleted end";
}

/**
 * @tc.name: NotifySupportWindowModesChange
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifySupportWindowModesChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifySupportWindowModesChange start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes = { AppExecFwk::SupportWindowMode::FULLSCREEN,
                                                                        AppExecFwk::SupportWindowMode::SPLIT,
                                                                        AppExecFwk::SupportWindowMode::FLOATING };
    WSError res = sProxy->NotifySupportWindowModesChange(supportedWindowModes);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifySupportWindowModesChange end";
}

/**
 * @tc.name: GetIsMidScene
 * @tc.desc: GetIsMidScene
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetIsMidScene, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: GetIsMidScene start";
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(nullptr);

    bool isMidScene = false;
    WSError res = sProxy->GetIsMidScene(isMidScene);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: GetIsMidScene end";
}

/**
 * @tc.name: ChangeSessionVisibilityWithStatusBar
 * @tc.desc: ChangeSessionVisibilityWithStatusBar test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, ChangeSessionVisibilityWithStatusBar, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: ChangeSessionVisibilityWithStatusBar start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    bool visible = true;
    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    WSError res = sProxy->ChangeSessionVisibilityWithStatusBar(abilitySessionInfo, visible);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->ChangeSessionVisibilityWithStatusBar(abilitySessionInfo, visible);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    abilitySessionInfo = nullptr;
    res = sProxy->ChangeSessionVisibilityWithStatusBar(abilitySessionInfo, visible);
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_SESSION);
    GTEST_LOG_(INFO) << "SessionProxyTest: ChangeSessionVisibilityWithStatusBar end";
}

/**
 * @tc.name: SyncSessionEvent
 * @tc.desc: SyncSessionEvent test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SyncSessionEvent, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SyncSessionEvent start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    SessionEvent event = SessionEvent::EVENT_MAXIMIZE;
    WSError res = sProxy->SyncSessionEvent(event);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: SyncSessionEvent end";
}

/**
 * @tc.name: OnLayoutFullScreenChange
 * @tc.desc: OnLayoutFullScreenChange test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, OnLayoutFullScreenChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: OnLayoutFullScreenChange start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    bool isLayoutFullScreen = true;
    WSError res = sProxy->OnLayoutFullScreenChange(isLayoutFullScreen);
    ASSERT_EQ(res, WSError::WS_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->OnLayoutFullScreenChange(isLayoutFullScreen);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: OnLayoutFullScreenChange end";
}

/**
 * @tc.name: OnDefaultDensityEnabled
 * @tc.desc: OnDefaultDensityEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, OnDefaultDensityEnabled, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: OnDefaultDensityEnabled start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    bool isDefaultDensityEnabled = true;
    WSError res = sProxy->OnDefaultDensityEnabled(isDefaultDensityEnabled);
    ASSERT_EQ(res, WSError::WS_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->OnDefaultDensityEnabled(isDefaultDensityEnabled);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: OnDefaultDensityEnabled end";
}

/**
 * @tc.name: NotifyRemovePrelaunchStartingWindow
 * @tc.desc: NotifyRemovePrelaunchStartingWindow test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyRemovePrelaunchStartingWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyRemovePrelaunchStartingWindow start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    WMError res = sProxy->NotifyRemovePrelaunchStartingWindow();
    ASSERT_EQ(res, WMError::WM_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->NotifyRemovePrelaunchStartingWindow();
    ASSERT_EQ(res, WMError::WM_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyRemovePrelaunchStartingWindow end";
}

/**
 * @tc.name: NotifyFrameLayoutFinishFromApp
 * @tc.desc: NotifyFrameLayoutFinishFromApp test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyFrameLayoutFinishFromApp, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyFrameLayoutFinishFromApp start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    bool notifyListener = true;
    WSRect rect = { 200, 200, 200, 200 };
    WSError res = sProxy->NotifyFrameLayoutFinishFromApp(notifyListener, rect);
    ASSERT_EQ(res, WSError::WS_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->NotifyFrameLayoutFinishFromApp(notifyListener, rect);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyFrameLayoutFinishFromApp end";
}

/**
 * @tc.name: NotifySnapshotUpdate
 * @tc.desc: NotifySnapshotUpdate test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifySnapshotUpdate, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifySnapshotUpdate start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    WMError res = sProxy->NotifySnapshotUpdate();
    EXPECT_EQ(res, WMError::WM_OK);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    res = sProxy->NotifySnapshotUpdate();
    EXPECT_EQ(res, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();

    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->NotifySnapshotUpdate();
    EXPECT_EQ(res, WMError::WM_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifySnapshotUpdate end";
}

/**
 * @tc.name: RaiseAppMainWindowToTop
 * @tc.desc: RaiseAppMainWindowToTop test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, RaiseAppMainWindowToTop, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: RaiseAppMainWindowToTop start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    WSError res = sProxy->RaiseAppMainWindowToTop();
    ASSERT_EQ(res, WSError::WS_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->RaiseAppMainWindowToTop();
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: RaiseAppMainWindowToTop end";
}

/**
 * @tc.name: GetAllAvoidAreas
 * @tc.desc: GetAllAvoidAreas test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetAllAvoidAreas, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: GetAllAvoidAreas start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    WSError res = sProxy->GetAllAvoidAreas(avoidAreas);
    ASSERT_EQ(res, WSError::WS_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->GetAllAvoidAreas(avoidAreas);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: GetAllAvoidAreas end";
}

/**
 * @tc.name: SetLandscapeMultiWindow
 * @tc.desc: SetLandscapeMultiWindow test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SetLandscapeMultiWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SetLandscapeMultiWindow start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    bool isLandscapeMultiWindow = true;
    WSError res = sProxy->SetLandscapeMultiWindow(isLandscapeMultiWindow);
    ASSERT_EQ(res, WSError::WS_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->SetLandscapeMultiWindow(isLandscapeMultiWindow);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: SetLandscapeMultiWindow end";
}

/**
 * @tc.name: NotifySyncOn
 * @tc.desc: NotifySyncOn test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifySyncOn, TestSize.Level1)
{
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);
    sProxy->NotifySyncOn();

    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    sProxy->NotifySyncOn();

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sProxy->NotifySyncOn();
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: TriggerBindModalUIExtension
 * @tc.desc: TriggerBindModalUIExtension test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, TriggerBindModalUIExtension, TestSize.Level1)
{
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);
    sProxy->TriggerBindModalUIExtension();

    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    sProxy->TriggerBindModalUIExtension();

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sProxy->TriggerBindModalUIExtension();
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: NotifyPiPWindowPrepareClose
 * @tc.desc: NotifyPiPWindowPrepareClose test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyPiPWindowPrepareClose, TestSize.Level1)
{
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);
    sProxy->NotifyPiPWindowPrepareClose();

    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    sProxy->NotifyPiPWindowPrepareClose();

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sProxy->NotifyPiPWindowPrepareClose();
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: UpdatePiPRect
 * @tc.desc: UpdatePiPRect test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdatePiPRect, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdatePiPRect start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    Rect rect = { 200, 200, 200, 200 };
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    WSError res = sProxy->UpdatePiPRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->UpdatePiPRect(rect, reason);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdatePiPRect end";
}

/**
 * @tc.name: SetSessionLabelAndIcon
 * @tc.desc: SetSessionLabelAndIcon
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SetSessionLabelAndIcon, TestSize.Level1)
{
    std::string label = "SetSessionLabelAndIcon";
    std::shared_ptr<Media::PixelMap> icon = std::make_shared<Media::PixelMap>();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(nullptr);

    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sProxy->SetSessionLabelAndIcon(label, icon));
}

/**
 * @tc.name: UpdateFlag
 * @tc.desc: UpdateFlag
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateFlag, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateFlag start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    std::string flag = "test";
    WSError res = sProxy->UpdateFlag(flag);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateFlag end";
}

/**
 * @tc.name: ProcessPointDownSession
 * @tc.desc: ProcessPointDownSession test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, ProcessPointDownSession, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: ProcessPointDownSession start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t posX = 50;
    int32_t posY = 50;
    WSError res = sProxy->ProcessPointDownSession(posX, posY);
    ASSERT_EQ(res, WSError::WS_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->ProcessPointDownSession(posX, posY);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: ProcessPointDownSession end";
}

/**
 * @tc.name: SendPointEventForMoveDrag
 * @tc.desc: SendPointEventForMoveDrag test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SendPointEventForMoveDrag, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SendPointEventForMoveDrag start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    bool isExecuteDelayRaise = true;
    WSError res = sProxy->SendPointEventForMoveDrag(pointerEvent, isExecuteDelayRaise);
    ASSERT_EQ(res, WSError::WS_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->SendPointEventForMoveDrag(pointerEvent, isExecuteDelayRaise);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: SendPointEventForMoveDrag end";
}

/**
 * @tc.name: IsStartMoving
 * @tc.desc: IsStartMoving test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, IsStartMoving, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: IsStartMoving start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    bool res = sProxy->IsStartMoving();
    ASSERT_EQ(res, false);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->IsStartMoving();
    ASSERT_EQ(res, false);
    GTEST_LOG_(INFO) << "SessionProxyTest: IsStartMoving end";
}

/**
 * @tc.name: UpdateRectChangeListenerRegistered
 * @tc.desc: UpdateRectChangeListenerRegistered test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateRectChangeListenerRegistered, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateRectChangeListenerRegistered start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    bool isRegister = true;
    WSError res = sProxy->UpdateRectChangeListenerRegistered(isRegister);
    ASSERT_EQ(res, WSError::WS_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->UpdateRectChangeListenerRegistered(isRegister);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateRectChangeListenerRegistered end";
}

/**
 * @tc.name: SendExtensionData
 * @tc.desc: SendExtensionData test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SendExtensionData, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SendExtensionData start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    WSError res = sProxy->SendExtensionData(data, reply, option);
    ASSERT_EQ(res, WSError::WS_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->SendExtensionData(data, reply, option);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: SendExtensionData end";
}

/**
 * @tc.name: SetGestureBackEnabled
 * @tc.desc: SetGestureBackEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SetGestureBackEnabled, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SetGestureBackEnabled start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    bool isEnabled = true;
    WMError res = sProxy->SetGestureBackEnabled(isEnabled);
    ASSERT_EQ(res, WMError::WM_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->SetGestureBackEnabled(isEnabled);
    ASSERT_EQ(res, WMError::WM_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: SetGestureBackEnabled end";
}

/**
 * @tc.name: NotifySubModalTypeChange
 * @tc.desc: NotifySubModalTypeChange test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifySubModalTypeChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifySubModalTypeChange start";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);

    SubWindowModalType subWindowModalType = SubWindowModalType::TYPE_APPLICATION_MODALITY;
    WSError res = sProxy->NotifySubModalTypeChange(subWindowModalType);
    ASSERT_EQ(res, WSError::WS_OK);
    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = sProxy->NotifySubModalTypeChange(subWindowModalType);
    ASSERT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifySubModalTypeChange end";
}

/**
 * @tc.name: GetCrossAxisState
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetCrossAxisState, TestSize.Level1)
{
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    CrossAxisState state = CrossAxisState::STATE_CROSS;
    WSError res = sProxy->GetCrossAxisState(state);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: OnContainerModalEvent
 * @tc.desc: OnContainerModalEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, OnContainerModalEvent, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: OnContainerModalEvent start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    WSError res = sProxy->OnContainerModalEvent("name", "value");
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: OnContainerModalEvent end";
}

/**
 * @tc.name: GetWaterfallMode
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetWaterfallMode, TestSize.Level1)
{
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    bool isWaterfallMode = false;
    WSError res = sProxy->GetWaterfallMode(isWaterfallMode);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: NotifyFollowParentMultiScreenPolicy
 * @tc.desc: NotifyFollowParentMultiScreenPolicy test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyFollowParentMultiScreenPolicy, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyFollowParentMultiScreenPolicy start";
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_EQ(sProxy->NotifyFollowParentMultiScreenPolicy(true), WSError::WS_ERROR_IPC_FAILED);
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_EQ(sProxy->NotifyFollowParentMultiScreenPolicy(true), WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyFollowParentMultiScreenPolicy end";
}


/**
 * @tc.name: UpdateRotationChangeRegistered
 * @tc.desc: UpdateRotationChangeRegistered test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateRotationChangeRegistered, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateRotationChangeRegistered start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    WSError res = sProxy->UpdateRotationChangeRegistered(0, false);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateRotationChangeRegistered start";
}

/**
 * @tc.name: UpdateScreenshotAppEventRegistered
 * @tc.desc: UpdateScreenshotAppEventRegistered test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateScreenshotAppEventRegistered, Function | SmallTest | Level2)
{
    MockMessageParcel::ClearAllErrorFlag();
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);
    auto ret = sProxy->UpdateScreenshotAppEventRegistered(0, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    ret = sProxy->UpdateScreenshotAppEventRegistered(0, true);
    EXPECT_EQ(ret, WMError::WM_OK);

    MockMessageParcel::SetReadInt32ErrorFlag(true);
    ret = sProxy->UpdateScreenshotAppEventRegistered(0, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteBoolErrorFlag(true);
    ret = sProxy->UpdateScreenshotAppEventRegistered(0, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = sProxy->UpdateScreenshotAppEventRegistered(0, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = sProxy->UpdateScreenshotAppEventRegistered(0, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: UpdateAcrossDisplaysChangeRegistered
 * @tc.desc: UpdateAcrossDisplaysChangeRegistered test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateAcrossDisplaysChangeRegistered, Function | SmallTest | Level2)
{
    MockMessageParcel::ClearAllErrorFlag();
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);
    auto ret = sProxy->UpdateAcrossDisplaysChangeRegistered(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sProxy = sptr<SessionProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(sProxy, nullptr);
    ret = sProxy->UpdateAcrossDisplaysChangeRegistered(true);
    EXPECT_EQ(ret, WMError::WM_OK);

    MockMessageParcel::SetReadInt32ErrorFlag(true);
    ret = sProxy->UpdateAcrossDisplaysChangeRegistered(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = sProxy->UpdateAcrossDisplaysChangeRegistered(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::SetWriteBoolErrorFlag(true);
    ret = sProxy->UpdateAcrossDisplaysChangeRegistered(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = sProxy->UpdateAcrossDisplaysChangeRegistered(true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: UpdateAcrossDisplaysChangeRegistered
 * @tc.desc: UpdateAcrossDisplaysChangeRegistered test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, IsMainWindowFullScreenAcrossDisplays, Function | SmallTest | Level2)
{
    bool isAcrossDisplays = false;
    MockMessageParcel::ClearAllErrorFlag();
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);
    auto ret = sProxy->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sProxy = sptr<SessionProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(sProxy, nullptr);
    ret = sProxy->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
    EXPECT_EQ(ret, WMError::WM_OK);

    MockMessageParcel::SetReadInt32ErrorFlag(true);
    ret = sProxy->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = sProxy->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = sProxy->IsMainWindowFullScreenAcrossDisplays(isAcrossDisplays);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: OnUpdateColorMode
 * @tc.desc: OnUpdateColorMode test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, OnUpdateColorMode, Function | SmallTest | Level2)
{
    std::string colorMode = "DARK";
    bool hasDarkRes = true;
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);
    auto ret = sProxy->OnUpdateColorMode(colorMode, hasDarkRes);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sProxy = sptr<SessionProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(sProxy, nullptr);
    ret = sProxy->OnUpdateColorMode(colorMode, hasDarkRes);
    EXPECT_EQ(ret, WMError::WM_OK);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = sProxy->OnUpdateColorMode(colorMode, hasDarkRes);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::SetWriteBoolErrorFlag(true);
    ret = sProxy->OnUpdateColorMode(colorMode, hasDarkRes);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteStringErrorFlag(true);
    ret = sProxy->OnUpdateColorMode(colorMode, hasDarkRes);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = sProxy->OnUpdateColorMode(colorMode, hasDarkRes);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteBoolErrorFlag(false);
    MockMessageParcel::SetWriteStringErrorFlag(false);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
}

/**
 * @tc.name: GetTargetOrientationConfigInfo
 * @tc.desc: GetTargetOrientationConfigInfo test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetTargetOrientationConfigInfo, Function | SmallTest | Level2)
{
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);

    std::map<Rosen::WindowType, Rosen::SystemBarProperty> targetProperties;
    Rosen::SystemBarProperty targetStatusBarProperty;
    targetProperties[Rosen::WindowType::WINDOW_TYPE_STATUS_BAR] = targetStatusBarProperty;

    std::map<Rosen::WindowType, Rosen::SystemBarProperty> currentProperties;
    Rosen::SystemBarProperty currentStatusBarProperty;
    currentProperties[Rosen::WindowType::WINDOW_TYPE_STATUS_BAR] = currentStatusBarProperty;
    WSError res = sProxy->GetTargetOrientationConfigInfo(Orientation::USER_ROTATION_PORTRAIT,
        targetProperties, currentProperties);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: ConvertOrientationAndRotation
 * @tc.desc: Test ConvertOrientationAndRotation behavior in various IPC scenarios
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, ConvertOrientationAndRotation, Function | SmallTest | Level1)
{
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    RotationInfoType from = RotationInfoType::DISPLAY_ORIENTATION;
    RotationInfoType to = RotationInfoType::DISPLAY_ORIENTATION;
    int32_t value = 0;
    int32_t convertedValue = 0;

    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED,
        sessionProxy->ConvertOrientationAndRotation(from, to, value, convertedValue));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Case 2: Failed to write RotationInfoType
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED,
        sessionProxy->ConvertOrientationAndRotation(from, to, value, convertedValue));
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    // Case 3: Failed to write convert value
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED,
        sessionProxy->ConvertOrientationAndRotation(from, to, value, convertedValue));
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    // Case 4: remote is nullptr
    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED,
        nullProxy->ConvertOrientationAndRotation(from, to, value, convertedValue));

    // Case 5: Failed to send request
    mockRemote->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionProxy> failProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED,
        failProxy->ConvertOrientationAndRotation(from, to, value, convertedValue));

    // Case 6: Success
    mockRemote->sendRequestResult_ = ERR_NONE;
    sptr<SessionProxy> okProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_OK,
        okProxy->ConvertOrientationAndRotation(from, to, value, convertedValue));
}

/**
 * @tc.name: KeyFrameAnimateEnd
 * @tc.desc: KeyFrameAnimateEnd test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, KeyFrameAnimateEnd, Function | SmallTest | Level2)
{
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    ASSERT_EQ(sProxy->KeyFrameAnimateEnd(), WSError::WS_OK);
}

/**
 * @tc.name: UpdateKeyFrameCloneNode
 * @tc.desc: UpdateKeyFrameCloneNode test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateKeyFrameCloneNode, Function | SmallTest | Level2)
{
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    auto rsKeyFrameNode = RSWindowKeyFrameNode::Create();
    ASSERT_NE(rsKeyFrameNode, nullptr);
    auto rsTransaction = std::make_shared<RSTransaction>();
    ASSERT_NE(rsTransaction, nullptr);
    ASSERT_EQ(sProxy->UpdateKeyFrameCloneNode(rsKeyFrameNode, rsTransaction), WSError::WS_OK);

    rsKeyFrameNode.reset();
    rsTransaction.reset();
    ASSERT_EQ(sProxy->UpdateKeyFrameCloneNode(rsKeyFrameNode, rsTransaction), WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: GetIsHighlighted
 * @tc.desc: GetIsHighlighted test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetIsHighlighted, Function | SmallTest | Level2)
{
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    bool isHighlighted = false;
    ASSERT_EQ(sProxy->GetIsHighlighted(isHighlighted), WSError::WS_OK);
}

/**
 * @tc.name: SetSubWindowSource
 * @tc.desc: SetSubWindowSource test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SetSubWindowSource, Function | SmallTest | Level2)
{
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    auto res = sProxy->SetSubWindowSource(SubWindowSource::SUB_WINDOW_SOURCE_UNKNOWN);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    sptr<SessionProxy> tempProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = tempProxy->SetSubWindowSource(SubWindowSource::SUB_WINDOW_SOURCE_UNKNOWN);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(1);
    res = sProxy->SetSubWindowSource(SubWindowSource::SUB_WINDOW_SOURCE_UNKNOWN);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(0);
    SubWindowSource source = SubWindowSource::SUB_WINDOW_SOURCE_UNKNOWN;
    EXPECT_EQ(sProxy->SetSubWindowSource(source), WSError::WS_OK);
}

/**
 * @tc.name: StartMovingWithCoordinate
 * @tc.desc: StartMovingWithCoordinate test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, StartMovingWithCoordinate, TestSize.Level2)
{
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    WSError res = sProxy->StartMovingWithCoordinate(0, 0, 0, 0, 0);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
 
    sptr<SessionProxy> tempProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    res = tempProxy->StartMovingWithCoordinate(0, 0, 0, 0, 0);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
 
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    res = sProxy->StartMovingWithCoordinate(0, 0, 0, 0, 0);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);
 
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    res = sProxy->StartMovingWithCoordinate(0, 0, 0, 0, 0);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadInt32ErrorFlag(false);

    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    res = sProxy->StartMovingWithCoordinate(0, 0, 0, 0, 0);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint64ErrorFlag(false);
 
    MockMessageParcel::ClearAllErrorFlag();
    res = sProxy->StartMovingWithCoordinate(-1, 0, 0, 0, 0);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    res = sProxy->StartMovingWithCoordinate(0, -1, 0, 0, 0);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    res = sProxy->StartMovingWithCoordinate(0, 0, -1, 0, 0);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    res = sProxy->StartMovingWithCoordinate(0, 0, 0, -1, 0);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);

    res = sProxy->StartMovingWithCoordinate(0, 0, 0, 0, 0);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: SetFrameRectForPartialZoomIn
 * @tc.desc: SetFrameRectForPartialZoomIn test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SetFrameRectForPartialZoomIn, Function | SmallTest | Level2)
{
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SessionProxy> sProxy = sptr<SessionProxy>::MakeSptr(remoteMocker);

    Rect frameRect;
    WSError res = sProxy->SetFrameRectForPartialZoomIn(frameRect);
    EXPECT_EQ(res, WSError::WS_OK);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    res = sProxy->SetFrameRectForPartialZoomIn(frameRect);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    res = sProxy->SetFrameRectForPartialZoomIn(frameRect);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    res = sProxy->SetFrameRectForPartialZoomIn(frameRect);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    MockMessageParcel::SetReadInt32ErrorFlag(true);
    res = sProxy->SetFrameRectForPartialZoomIn(frameRect);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadInt32ErrorFlag(false);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    res = sProxy->SetFrameRectForPartialZoomIn(frameRect);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: SetFrameRectForPartialZoomIn01
 * @tc.desc: SetFrameRectForPartialZoomIn test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SetFrameRectForPartialZoomIn01, Function | SmallTest | Level2)
{
    Rect frameRect;
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    WSError res = sProxy->SetFrameRectForPartialZoomIn(frameRect);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: ChangeKeyboardEffectOption01
 * @tc.desc: ChangeKeyboardEffectOption test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, ChangeKeyboardEffectOption01, Function | SmallTest | Level2)
{
    KeyboardEffectOption effectOption;
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    WSError res = sProxy->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: UpdateFloatingBall
 * @tc.desc: UpdateFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateFloatingBall, Function | SmallTest | Level2)
{
    auto iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    FloatingBallTemplateInfo fbTemplateInfo;

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    WMError res = sProxy->UpdateFloatingBall(fbTemplateInfo);
    ASSERT_EQ(res, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    res = sProxy->UpdateFloatingBall(fbTemplateInfo);
    ASSERT_EQ(res, WMError::WM_OK);

    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, sProxy->UpdateFloatingBall(fbTemplateInfo));
    MockMessageParcel::SetWriteParcelableErrorFlag(false);

    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, sProxy->UpdateFloatingBall(fbTemplateInfo));

    iRemoteObjectMocker->sendRequestResult_ = 1;
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, sProxy->UpdateFloatingBall(fbTemplateInfo));
}

/**
 * @tc.name: RestoreFbMainWindow
 * @tc.desc: RestoreFbMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, RestoreFbMainWindow, Function | SmallTest | Level2)
{
    auto iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto want = std::make_shared<AAFwk::Want>();
    ASSERT_NE(want, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, sProxy->RestoreFbMainWindow(want));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, sProxy->RestoreFbMainWindow(want));
    MockMessageParcel::SetWriteParcelableErrorFlag(false);

    WMError res = sProxy->RestoreFbMainWindow(want);
    ASSERT_EQ(res, WMError::WM_OK);

    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, sProxy->RestoreFbMainWindow(want));

    iRemoteObjectMocker->sendRequestResult_ = 1;
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, sProxy->RestoreFbMainWindow(want));
}

/**
 * @tc.name: GetFloatingBallWindowId
 * @tc.desc: GetFloatingBallWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetFloatingBallWindowId, Function | SmallTest | Level2)
{
    auto iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    uint32_t windowId = 0;

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, sProxy->GetFloatingBallWindowId(windowId));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    WMError res = sProxy->GetFloatingBallWindowId(windowId);
    ASSERT_EQ(res, WMError::WM_OK);

    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, sProxy->GetFloatingBallWindowId(windowId));

    iRemoteObjectMocker->sendRequestResult_ = 1;
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, sProxy->GetFloatingBallWindowId(windowId));
}

/**
 * @tc.name: NotifyFloatingBallPrepareClose
 * @tc.desc: NotifyFloatingBallPrepareClose
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyFloatingBallPrepareClose, Function | SmallTest | Level2)
{
    auto iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sProxy->NotifyFloatingBallPrepareClose();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    sProxy->NotifyFloatingBallPrepareClose();

    sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    sProxy->NotifyFloatingBallPrepareClose();

    iRemoteObjectMocker->sendRequestResult_ = 1;
    sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    sProxy->NotifyFloatingBallPrepareClose();
}

/**
 * @tc.name: TestUpdateGlobalDisplayRectFromClient
 * @tc.desc: Test UpdateGlobalDisplayRectFromClient behavior in various IPC scenarios
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, TestUpdateGlobalDisplayRectFromClient, Function | SmallTest | Level1)
{
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    WSRect rect { 10, 20, 200, 100 };
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;

    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->UpdateGlobalDisplayRectFromClient(rect, reason));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Case 2: Failed to write rect
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->UpdateGlobalDisplayRectFromClient(rect, reason));
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    // Case 3: Failed to write reason
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->UpdateGlobalDisplayRectFromClient(rect, reason));
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    // Case 4: remote is nullptr
    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, nullProxy->UpdateGlobalDisplayRectFromClient(rect, reason));

    // Case 5: Failed to send request
    mockRemote->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionProxy> failProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, failProxy->UpdateGlobalDisplayRectFromClient(rect, reason));

    // Case 6: Success
    mockRemote->sendRequestResult_ = ERR_NONE;
    sptr<SessionProxy> okProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_OK, okProxy->UpdateGlobalDisplayRectFromClient(rect, reason));
}

/**
 * @tc.name: GetAppHookWindowInfoFromServer
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetAppHookWindowInfoFromServer, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: GetAppHookWindowInfoFromServer start";
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    MockMessageParcel::ClearAllErrorFlag();
    HookWindowInfo hookWindowInfo;

    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, sProxy->GetAppHookWindowInfoFromServer(hookWindowInfo));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Case 2: remote is nullptr
    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, nullProxy->GetAppHookWindowInfoFromServer(hookWindowInfo));

    // Case 3: Failed to send request
    mockRemote->SetRequestResult(ERR_TRANSACTION_FAILED);
    sptr<SessionProxy> failSendRequestProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, failSendRequestProxy->GetAppHookWindowInfoFromServer(hookWindowInfo));
    mockRemote->SetRequestResult(ERR_NONE);

    // Case 4: Failed to read replyInfo and ret
    MockMessageParcel::SetReadBoolErrorFlag(true);
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, sProxy->GetAppHookWindowInfoFromServer(hookWindowInfo));
    MockMessageParcel::SetReadBoolErrorFlag(false);
    MockMessageParcel::SetReadInt32ErrorFlag(false);

    // Case 5: Success
    sptr<SessionProxy> okProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WMError::WM_OK, okProxy->GetAppHookWindowInfoFromServer(hookWindowInfo));
    MockMessageParcel::ClearAllErrorFlag();
    GTEST_LOG_(INFO) << "SessionProxyTest: GetAppHookWindowInfoFromServer end";
}

/**
 * @tc.name: GetAppHookWindowInfoFromServer
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyWindowStatusDidChangeAfterShowWindow, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyWindowStatusDidChangeAfterShowWindow start";
    logMsg.clear();
    LOG_SetCallBack(MyLogCallback);

    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();

    auto sProxy = sptr<MockIRemoteObject>::MakeSptr(nullptr);
    sProxy->NotifyWindowStatusDidChangeAfterShowWindow();
    EXPECT_TRUE(logMsg.find("remote is null") != std::string::npos);

    sProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sProxy->NotifyWindowStatusDidChangeAfterShowWindow();
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    mockRemote->SetRequestResult(ERR_INVALID_DATA);
    sProxy->NotifyWindowStatusDidChangeAfterShowWindow();
    EXPECT_TRUE(logMsg.find("SendRequest failed") != std::string::npos);

    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyWindowStatusDidChangeAfterShowWindow end";
}

/**
 * @tc.name: TestSetContentAspectRatio
 * @tc.desc: Test SetContentAspectRatio behavior in various IPC scenarios
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, TestSetContentAspectRatio, TestSize.Level1)
{
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    float ratio = 1.5f;
    bool isPersistent = true;
    bool needUpdateRect = false;

    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->SetContentAspectRatio(ratio, isPersistent, needUpdateRect));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Case 2: Failed to write ratio
    MockMessageParcel::SetWriteFloatErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->SetContentAspectRatio(ratio, isPersistent, needUpdateRect));
    MockMessageParcel::SetWriteFloatErrorFlag(false);

    // Case 3: Failed to write isPersistent and needUpdateRect
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->SetContentAspectRatio(ratio, isPersistent, needUpdateRect));
    MockMessageParcel::SetWriteBoolErrorFlag(false);

    // Case 4: remote is nullptr
    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, nullProxy->SetContentAspectRatio(ratio, isPersistent, needUpdateRect));

    // Case 5: Failed to send request
    mockRemote->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionProxy> failProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, failProxy->SetContentAspectRatio(ratio, isPersistent, needUpdateRect));

    // Case 6: Success
    mockRemote->sendRequestResult_ = ERR_NONE;
    sptr<SessionProxy> okProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_OK, okProxy->SetContentAspectRatio(ratio, isPersistent, needUpdateRect));
}

/**
 * @tc.name: TestSetDecorVisible
 * @tc.desc: Test SetDecorVisible behavior in various IPC scenarios
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, TestSetDecorVisible, TestSize.Level1)
{
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    bool isVisible = true;

    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->SetDecorVisible(isVisible));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Case 2: Failed to write isVisible
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->SetDecorVisible(isVisible));
    MockMessageParcel::SetWriteBoolErrorFlag(false);

    // Case 3: remote is nullptr
    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, nullProxy->SetDecorVisible(isVisible));

    // Case 4: Failed to send request
    mockRemote->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionProxy> failProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, failProxy->SetDecorVisible(isVisible));

    // Case 5: Success
    mockRemote->sendRequestResult_ = ERR_NONE;
    sptr<SessionProxy> okProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_OK, okProxy->SetDecorVisible(isVisible));
}

/**
 * @tc.name: NotifyIsFullScreenInForceSplitMode
 * @tc.desc: NotifyIsFullScreenInForceSplitMode test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyIsFullScreenInForceSplitMode, TestSize.Level3)
{
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    bool isFullScreen = true;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    sessionProxy->NotifyIsFullScreenInForceSplitMode(isFullScreen);
    MockMessageParcel::SetReadInt32ErrorFlag(false);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->NotifyIsFullScreenInForceSplitMode(isFullScreen));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteBoolErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->NotifyIsFullScreenInForceSplitMode(isFullScreen));
    MockMessageParcel::SetWriteBoolErrorFlag(false);

    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, nullProxy->NotifyIsFullScreenInForceSplitMode(isFullScreen));

    mockRemote->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionProxy> failProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, failProxy->NotifyIsFullScreenInForceSplitMode(isFullScreen));

    mockRemote->sendRequestResult_ = ERR_NONE;
    sptr<SessionProxy> okProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_OK, okProxy->NotifyIsFullScreenInForceSplitMode(isFullScreen));
}

/**
 * @tc.name: RestartApp
 * @tc.desc: RestartApp test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, RestartApp, TestSize.Level3)
{
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    std::shared_ptr<AAFwk::Want> want = nullptr;

    MockMessageParcel::ClearAllErrorFlag();
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, sessionProxy->RestartApp(want));

    want = std::make_shared<AAFwk::Want>();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->RestartApp(want));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionProxy->RestartApp(want));
    MockMessageParcel::SetWriteParcelableErrorFlag(false);

    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, nullProxy->RestartApp(want));

    mockRemote->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionProxy> failProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, failProxy->RestartApp(want));

    mockRemote->sendRequestResult_ = ERR_NONE;
    sptr<SessionProxy> okProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    EXPECT_EQ(WSError::WS_OK, okProxy->RestartApp(want));
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigEnableUpdated
 * @tc.desc: Test NotifyAppForceLandscapeConfigEnableUpdated normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyAppForceLandscapeConfigEnableUpdated, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyAppForceLandscapeConfigEnableUpdated start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    auto res = sProxy->NotifyAppForceLandscapeConfigEnableUpdated();
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyAppForceLandscapeConfigEnableUpdated end";
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigEnableUpdated01
 * @tc.desc: ShouldReturnIpcFailed_WhenWriteInterfaceTokenFails
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyAppForceLandscapeConfigEnableUpdated01, TestSize.Level1)
{
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    auto res = sProxy->NotifyAppForceLandscapeConfigEnableUpdated();
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigEnableUpdated02
 * @tc.desc: NotifyAppForceLandscapeConfigEnableUpdated_ShouldReturnIpcFailed_WhenRemoteIsNull
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyAppForceLandscapeConfigEnableUpdated02, TestSize.Level1)
{
    auto sProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(sProxy, nullptr);
    auto res = sProxy->NotifyAppForceLandscapeConfigEnableUpdated();
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigEnableUpdated03
 * @tc.desc: ShouldReturnIpcFailed_WhenSendRequestFails
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyAppForceLandscapeConfigEnableUpdated03, TestSize.Level1)
{
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    mockRemote->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    auto sProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    ASSERT_NE(sProxy, nullptr);
    auto res = sProxy->NotifyAppForceLandscapeConfigEnableUpdated();
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
}
} // namespace
} // namespace Rosen
} // namespace OHOS