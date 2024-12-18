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

// using namespace FRAME_TRACE;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionProxyTest : public testing::Test {
  public:
    SessionProxyTest() {}
    ~SessionProxyTest() {}
};
namespace {

/**
 * @tc.name: WriteAbilitySessionInfoBasic
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, WriteAbilitySessionInfoBasic, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: WriteAbilitySessionInfoBasic start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
    sptr<AAFwk::SessionInfo> abilitySessionInfo = nullptr;
    MessageParcel data;
    bool res = sProxy->WriteAbilitySessionInfoBasic(data, abilitySessionInfo);
    ASSERT_EQ(res, false);

    sptr<AAFwk::SessionInfo> abilitySessionInfo1 = new(std::nothrow) AAFwk::SessionInfo();
    ASSERT_NE(abilitySessionInfo1, nullptr);
    res = sProxy->WriteAbilitySessionInfoBasic(data, abilitySessionInfo1);
    ASSERT_EQ(res, false);
    GTEST_LOG_(INFO) << "SessionProxyTest: WriteAbilitySessionInfoBasic end";
}

/**
 * @tc.name: OnSessionEvent
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, OnSessionEvent, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: OnSessionEvent start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
    SessionEvent event = SessionEvent::EVENT_MAXIMIZE;
    WSError res = sProxy->OnSessionEvent(event);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: OnSessionEvent end";
}

/**
 * @tc.name: UpdateSessionRect
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateSessionRect, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateSessionRect start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
    WSRect rect{.posX_ = 1, .posY_ = 1, .width_ = 100, .height_ = 100};
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
HWTEST_F(SessionProxyTest, OnRestoreMainWindow, Function | SmallTest | Level2)
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
 * @tc.name: RaiseToAppTop
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, RaiseToAppTop, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: RaiseToAppTop start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
    WSError res = sProxy->RaiseToAppTop();
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: RaiseToAppTop end";
}

/**
 * @tc.name: OnNeedAvoid
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, OnNeedAvoid, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: OnNeedAvoid start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
    bool status = false;
    WSError res = sProxy->OnNeedAvoid(status);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: OnNeedAvoid end";
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetAvoidAreaByType, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: GetAvoidAreaByType start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
    AvoidAreaType status = AvoidAreaType::TYPE_SYSTEM;
    AvoidArea res = sProxy->GetAvoidAreaByType(status);
    AvoidArea area;
    ASSERT_EQ(res, area);

    GTEST_LOG_(INFO) << "SessionProxyTest: GetAvoidAreaByType end";
}

/**
 * @tc.name: RequestSessionBack
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, RequestSessionBack, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: RequestSessionBack start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
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
HWTEST_F(SessionProxyTest, MarkProcessed, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: MarkProcessed start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
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
HWTEST_F(SessionProxyTest, SetGlobalMaximizeMode, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SetGlobalMaximizeMode start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
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
HWTEST_F(SessionProxyTest, GetGlobalMaximizeMode, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: GetGlobalMaximizeMode start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
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
HWTEST_F(SessionProxyTest, SetAspectRatio, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SetAspectRatio start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
    float ratio = 10;
    WSError res = sProxy->SetAspectRatio(ratio);
    ASSERT_EQ(res, WSError::WS_OK);

    GTEST_LOG_(INFO) << "SessionProxyTest: SetAspectRatio end";
}

/**
 * @tc.name: UpdateSessionPropertyByAction01
 * @tc.desc: UpdateSessionPropertyByAction
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateSessionPropertyByAction01, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateSessionPropertyByAction01 start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    WMError res = sProxy->UpdateSessionPropertyByAction(nullptr,
        WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON);
    ASSERT_EQ(res, WMError::WM_OK);
    delete sProxy;
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateSessionPropertyByAction01 end";
}

/**
 * @tc.name: UpdateSessionPropertyByAction02
 * @tc.desc: UpdateSessionPropertyByAction
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateSessionPropertyByAction02, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateSessionPropertyByAction02 start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    WMError res = sProxy->UpdateSessionPropertyByAction(nullptr,
        WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON);
    ASSERT_EQ(res, WMError::WM_OK);
    delete sProxy;
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateSessionPropertyByAction02 end";
}

/**
 * @tc.name: UpdateWindowSceneAfterCustomAnimation
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateWindowSceneAfterCustomAnimation, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateWindowSceneAfterCustomAnimation start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
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
HWTEST_F(SessionProxyTest, SetSystemWindowEnableDrag, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SetSystemWindowEnableDrag start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
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
HWTEST_F(SessionProxyTest, TransferAbilityResult, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: TransferAbilityResult start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
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
HWTEST_F(SessionProxyTest, NotifyExtensionDied, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyExtensionDied start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
    sProxy->NotifyExtensionDied();

    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyExtensionDied end";
}

/**
 * @tc.name: NotifyExtensionTimeout
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, NotifyExtensionTimeout, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyExtensionTimeout start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
    sProxy->NotifyExtensionTimeout(2);

    GTEST_LOG_(INFO) << "SessionProxyTest: NotifyExtensionTimeout end";
}

/**
 * @tc.name: UpdateWindowAnimationFlag
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateWindowAnimationFlag, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateWindowAnimationFlag start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
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
HWTEST_F(SessionProxyTest, TransferAccessibilityEvent, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: TransferAccessibilityEvent start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
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
HWTEST_F(SessionProxyTest, OnTitleAndDockHoverShowChange, Function | SmallTest | Level2)
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
HWTEST_F(SessionProxyTest, UpdatePiPControlStatus, Function | SmallTest | Level2)
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
HWTEST_F(SessionProxyTest, SetAutoStartPiP, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SetAutoStartPiP: SetAutoStartPiP start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    bool isAutoStartValid = true;
    uint32_t priority = 0;
    ASSERT_EQ(WSError::WS_OK, sProxy->SetAutoStartPiP(isAutoStartValid, priority));
    GTEST_LOG_(INFO) << "SetAutoStartPiP: SetAutoStartPiP end";
}

/**
 * @tc.name: GetGlobalScaledRect
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetGlobalScaledRect, Function | SmallTest | Level2)
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
 * @tc.name: RequestFocus
 * @tc.desc: RequestFocus Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, RequestFocus, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: RequestFocus start";
    sptr<IRemoteObject> iRemoteObjectMocker = new IRemoteObjectMocker();
    SessionProxy* sProxy = new(std::nothrow) SessionProxy(iRemoteObjectMocker);
    WSError res = sProxy->RequestFocus(true);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: RequestFocus end";
}

/**
 * @tc.name: UpdateClientRect01
 * @tc.desc: UpdateClientRect test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateClientRect01, Function | SmallTest | Level2)
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
 * @tc.name: GetAppForceLandscapeConfig
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, GetAppForceLandscapeConfig, Function | SmallTest | Level2)
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
 * @tc.name: OnSetWindowRectAutoSave
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, OnSetWindowRectAutoSave, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: OnSetWindowRectAutoSave start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    WSError res = sProxy->OnSetWindowRectAutoSave(true);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: OnSetWindowRectAutoSave end";
}
} // namespace
} // namespace Rosen
} // namespace OHOS
