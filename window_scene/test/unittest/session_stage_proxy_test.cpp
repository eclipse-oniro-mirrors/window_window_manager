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

#include "session/container/include/zidl/session_stage_proxy.h"

#include <gtest/gtest.h>
#include <transaction/rs_transaction.h>

#include "iremote_object_mocker.h"
#include "mock_message_parcel.h"
#include "proto.h"
#include "string_wrapper.h"
#include "util.h"
#include "window_manager.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "mock_message_parcel.h"

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
class SessionStageProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionStageProxy> sessionStage_ = sptr<SessionStageProxy>::MakeSptr(iRemoteObjectMocker);
};

void SessionStageProxyTest::SetUpTestCase() {}

void SessionStageProxyTest::TearDownTestCase() {}

void SessionStageProxyTest::SetUp() {}

void SessionStageProxyTest::TearDown() {}

namespace {
/**
 * @tc.name: SetActive
 * @tc.desc: test function : SetActive
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetActive, TestSize.Level1)
{
    bool active = false;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->SetActive(active);
    ASSERT_EQ(WSError::WS_OK, res);
    active = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    res = sessionStage_->SetActive(active);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: UpdateDisplayId
 * @tc.desc: test function : UpdateDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateDisplayId, TestSize.Level1)
{
    uint64_t displayID = 0;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->UpdateDisplayId(displayID);
    ASSERT_EQ(WSError::WS_OK, res);
    displayID = 1;
    res = sessionStage_->UpdateDisplayId(displayID);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: UpdateRect
 * @tc.desc: test function : UpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateRect, TestSize.Level1)
{
    WSRect rect;
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    ASSERT_TRUE((sessionStage_ != nullptr));
    std::shared_ptr<RSTransaction> rsTransaction = std::make_shared<RSTransaction>();
    SceneAnimationConfig config{ rsTransaction, ROTATE_ANIMATION_DURATION,
        0, WindowAnimationCurve::LINEAR, {0.0f, 0.0f, 0.0f, 0.0f} };
    WSError res = sessionStage_->UpdateRect(rect, reason, config);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: UpdateRect01
 * @tc.desc: test function : UpdateRect01
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateRect01, TestSize.Level1)
{
    WSRect rect;
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    ASSERT_TRUE((sessionStage_ != nullptr));
    std::shared_ptr<RSTransaction> rsTransaction = std::make_shared<RSTransaction>();
    SceneAnimationConfig config{ rsTransaction, ROTATE_ANIMATION_DURATION,
    0, WindowAnimationCurve::LINEAR, {0.0f, 0.0f, 0.0f, 0.0f} };
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    WSError res = sessionStage_->UpdateRect(rect, reason, config);
    MockMessageParcel::ClearAllErrorFlag();
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: UpdateRect02
 * @tc.desc: test function : UpdateRect02
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateRect02, TestSize.Level1)
{
    WSRect rect;
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    ASSERT_TRUE((sessionStage_ != nullptr));
    std::shared_ptr<RSTransaction> rsTransaction = std::make_shared<RSTransaction>();
    SceneAnimationConfig config{ rsTransaction, ROTATE_ANIMATION_DURATION,
    0, WindowAnimationCurve::LINEAR, {0.0f, 0.0f, 0.0f, 0.0f} };

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    MockMessageParcel::SetWriteInt32ErrorCount(2);
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    MockMessageParcel::SetWriteUint32ErrorCount(3);
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    MockMessageParcel::SetWriteBoolErrorCount(1);
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    MockMessageParcel::SetWriteParcelableErrorCount(1);
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    WSError res = sessionStage_->UpdateRect(rect, reason, config);
    MockMessageParcel::ClearAllErrorFlag();
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: UpdateDensity
 * @tc.desc: test function : UpdateDensity
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateDensity, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->UpdateDensity();
}

/**
 * @tc.name: UpdateOrientation
 * @tc.desc: test function : UpdateOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateOrientation, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->UpdateOrientation();
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: UpdateSessionViewportConfig
 * @tc.desc: test function : UpdateSessionViewportConfig
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateSessionViewportConfig, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    SessionViewportConfig config;
    WSError res = sessionStage_->UpdateSessionViewportConfig(config);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: HandleBackEvent
 * @tc.desc: test function : HandleBackEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, HandleBackEvent, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->HandleBackEvent();
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: SwitchFreeMultiWindow
 * @tc.desc: test function : SwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SwitchFreeMultiWindow, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    bool enable = true;
    WSError res = sessionStage_->SwitchFreeMultiWindow(enable);
    ASSERT_EQ(WSError::WS_OK, res);
    enable = false;
    res = sessionStage_->SwitchFreeMultiWindow(enable);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: GetUIContentRemoteObj
 * @tc.desc: test function : GetUIContentRemoteObj
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, GetUIContentRemoteObj, TestSize.Level1)
{
    ASSERT_NE(sessionStage_, nullptr);
    sptr<IRemoteObject> remoteObj;
    WSError res = sessionStage_->GetUIContentRemoteObj(remoteObj);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: MarkProcessed
 * @tc.desc: test function : MarkProcessed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, MarkProcessed, TestSize.Level1)
{
    int32_t eventId = 1;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->MarkProcessed(eventId);
    ASSERT_EQ(WSError::WS_DO_NOTHING, res);
}

/**
 * @tc.name: UpdateFocus
 * @tc.desc: test function : UpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateFocus, TestSize.Level1)
{
    bool focus = false;
    ASSERT_TRUE((sessionStage_ != nullptr));
    auto info = sptr<FocusNotifyInfo>::MakeSptr(1, 1, 2, false);
    WSError res = sessionStage_->UpdateFocus(info, focus);
    ASSERT_EQ(WSError::WS_OK, res);
    focus = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    res = sessionStage_->UpdateFocus(info, focus);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: UpdateFocus01
 * @tc.desc: test function : UpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateFocus01, TestSize.Level1)
{
    bool focus = false;
    ASSERT_TRUE((sessionStage_ != nullptr));
    sptr<FocusNotifyInfo> info = nullptr;
    WSError res = sessionStage_->UpdateFocus(info, focus);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
    info = sptr<FocusNotifyInfo>::MakeSptr();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    res = sessionStage_->UpdateFocus(info, focus);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: NotifyHighlightChange
 * @tc.desc: test function : NotifyHighlightChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyHighlightChange, TestSize.Level1)
{
    bool isHighlight = false;
    ASSERT_TRUE((sessionStage_ != nullptr));
    auto info = sptr<HighlightNotifyInfo>::MakeSptr(0, std::vector<int32_t>(), 1, false);
    WSError res = sessionStage_->NotifyHighlightChange(info, isHighlight);
    ASSERT_EQ(WSError::WS_OK, res);
    isHighlight = true;
    res = sessionStage_->NotifyHighlightChange(info, isHighlight);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyHighlightChange01
 * @tc.desc: test function : NotifyHighlightChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyHighlightChange01, TestSize.Level1)
{
    bool isHighlight = false;
    ASSERT_TRUE((sessionStage_ != nullptr));
    sptr<HighlightNotifyInfo> info = nullptr;
    WSError res = sessionStage_->NotifyHighlightChange(info, isHighlight);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
    info = sptr<HighlightNotifyInfo>::MakeSptr();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    res = sessionStage_->NotifyHighlightChange(info, isHighlight);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: NotifyDestroy
 * @tc.desc: test function : NotifyDestroy
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyDestroy, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->NotifyDestroy();
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyCloseExistPipWindow
 * @tc.desc: test function : NotifyCloseExistPipWindow
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyCloseExistPipWindow, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->NotifyCloseExistPipWindow();
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyTransferComponentData
 * @tc.desc: test function : NotifyTransferComponentData
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyTransferComponentData, TestSize.Level1)
{
    AAFwk::WantParams wantParams;
    std::string testValue = "testValue";
    wantParams.SetParam("keyStr", OHOS::AAFwk::String::Box(testValue));
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->NotifyTransferComponentData(wantParams);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyTransferComponentDataSync
 * @tc.desc: test function : NotifyTransferComponentDataSync
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyTransferComponentDataSync, TestSize.Level1)
{
    AAFwk::WantParams wantParams;
    std::string testValue = "testValue";
    wantParams.SetParam("keyStr", OHOS::AAFwk::String::Box(testValue));
    AAFwk::WantParams reWantParams;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSErrorCode res = sessionStage_->NotifyTransferComponentDataSync(wantParams, reWantParams);
    ASSERT_EQ(WSErrorCode::WS_ERROR_TRANSFER_DATA_FAILED, res);
}

/**
 * @tc.name: NotifyOccupiedAreaChangeInfo
 * @tc.desc: test function : NotifyOccupiedAreaChangeInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyOccupiedAreaChangeInfo, TestSize.Level1)
{
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifyOccupiedAreaChangeInfo(info, nullptr, {}, {});
}

/**
 * @tc.name: NotifyKeyboardAnimationCompleted
 * @tc.desc: test function : NotifyKeyboardAnimationCompleted
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyKeyboardAnimationCompleted, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    KeyboardPanelInfo keyboardPanelInfo;
    sessionStage_->NotifyKeyboardAnimationCompleted(keyboardPanelInfo);
}

/**
 * @tc.name: NotifyKeyboardAnimationWillBegin
 * @tc.desc: test function : NotifyKeyboardAnimationWillBegin
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyKeyboardAnimationWillBegin, Function | SmallTest | Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_TRUE((sessionStage_ != nullptr));
    KeyboardAnimationInfo keyboardAnimationInfo;
    const std::shared_ptr<RSTransaction>& rsTransaction = std::make_shared<RSTransaction>();

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sessionStage_->NotifyKeyboardAnimationWillBegin(keyboardAnimationInfo, rsTransaction);
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    sessionStage_->NotifyKeyboardAnimationWillBegin(keyboardAnimationInfo, rsTransaction);
    EXPECT_TRUE(logMsg.find("KeyboardPanelInfo marshalling failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    sessionStage_->NotifyKeyboardAnimationWillBegin(keyboardAnimationInfo, rsTransaction);
    EXPECT_TRUE(logMsg.find("SendRequest failed") == std::string::npos);
}

/**
 * @tc.name: UpdateAvoidArea
 * @tc.desc: test function : UpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateAvoidArea, TestSize.Level1)
{
    sptr<AvoidArea> avoidArea = sptr<AvoidArea>::MakeSptr();
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->UpdateAvoidArea(avoidArea, type);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: DumpSessionElementInfo
 * @tc.desc: test function : DumpSessionElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, DumpSessionElementInfo, TestSize.Level1)
{
    std::vector<std::string> params;
    params.push_back("test1");
    params.push_back("test2");
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->DumpSessionElementInfo(params);
}

/**
 * @tc.name: NotifyScreenshot
 * @tc.desc: test function : NotifyScreenshot
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyScreenshot, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifyScreenshot();
}

/**
 * @tc.name: NotifyScreenshotAppEvent
 * @tc.desc: test function : NotifyScreenshotAppEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyScreenshotAppEvent, TestSize.Level1)
{
    ASSERT_NE(sessionStage_, nullptr);
    ScreenshotEventType type = ScreenshotEventType::SCROLL_SHOT_START;
    MockMessageParcel::ClearAllErrorFlag();
    auto ret = sessionStage_->NotifyScreenshotAppEvent(type);
    EXPECT_EQ(WSError::WS_OK, ret);

    sptr<SessionStageProxy> sessionStage = sptr<SessionStageProxy>::MakeSptr(nullptr);
    ret = sessionStage->NotifyScreenshotAppEvent(type);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = sessionStage_->NotifyScreenshotAppEvent(type);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = sessionStage_->NotifyScreenshotAppEvent(type);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: NotifyTouchOutside
 * @tc.desc: test function : NotifyTouchOutside
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyTouchOutside, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->NotifyTouchOutside();
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: UpdateWindowMode
 * @tc.desc: test function : UpdateWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateWindowMode, TestSize.Level1)
{
    WindowMode mode = WindowMode::WINDOW_MODE_UNDEFINED;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->UpdateWindowMode(mode);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifySessionFullScreen
 * @tc.desc: test function : NotifySessionFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifySessionFullScreen, TestSize.Level1)
{
    bool fullScreen = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifySessionFullScreen(fullScreen);
    fullScreen = false;
    sessionStage_->NotifySessionFullScreen(fullScreen);
}

/**
 * @tc.name: NotifyExtensionSecureLimitChange01
 * @tc.desc: test function : NotifyExtensionSecureLimitChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyExtensionSecureLimitChange01, TestSize.Level1)
{
    bool isLimit = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    auto res = sessionStage_->NotifyExtensionSecureLimitChange(isLimit);
    ASSERT_EQ(WSError::WS_OK, res);
    isLimit = false;
    res = sessionStage_->NotifyExtensionSecureLimitChange(isLimit);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyExtensionSecureLimitChange02
 * @tc.desc: test function : NotifyExtensionSecureLimitChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyExtensionSecureLimitChange02, TestSize.Level1)
{
    bool isLimit = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    auto res = sessionStage_->NotifyExtensionSecureLimitChange(isLimit);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: NotifyForegroundInteractiveStatus
 * @tc.desc: test function : NotifyForegroundInteractiveStatus
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyForegroundInteractiveStatus, TestSize.Level1)
{
    bool interactive = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifyForegroundInteractiveStatus(interactive);
    interactive = false;
    sessionStage_->NotifyForegroundInteractiveStatus(interactive);
}

/**
 * @tc.name: UpdateMaximizeMode
 * @tc.desc: test function : UpdateMaximizeMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateMaximizeMode, TestSize.Level1)
{
    MaximizeMode mode = MaximizeMode::MODE_FULL_FILL;
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->UpdateMaximizeMode(mode);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifySessionForeground
 * @tc.desc: test function : NotifySessionForeground
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifySessionForeground, TestSize.Level1)
{
    uint32_t reason = 1;
    bool withAnimation = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifySessionForeground(reason, withAnimation);
}

/**
 * @tc.name: NotifySessionBackground
 * @tc.desc: test function : NotifySessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifySessionBackground, TestSize.Level1)
{
    uint32_t reason = 1;
    bool withAnimation = true;
    bool isFromInnerkits = true;
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifySessionBackground(reason, withAnimation, isFromInnerkits);
}

/**
 * @tc.name: NotifyCompatibleModePropertyChange
 * @tc.desc: test function : NotifyCompatibleModePropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyCompatibleModePropertyChange, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    WSError res = sessionStage_->NotifyCompatibleModePropertyChange(compatibleModeProperty);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: UpdateTitleInTargetPos
 * @tc.desc: test function : UpdateTitleInTargetPos
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateTitleInTargetPos, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->UpdateTitleInTargetPos(true, 1);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyTransformChange
 * @tc.desc: test function : NotifyTransformChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyTransformChange, TestSize.Level1)
{
    Transform transform;
    ASSERT_TRUE((sessionStage_ != nullptr));
    sessionStage_->NotifyTransformChange(transform);
}

/**
 * @tc.name: NotifyWindowOcclusionState
 * @tc.desc: notify the occlusion state
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyWindowOcclusionState, TestSize.Level1)
{
    auto state = WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION;
    auto tempProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    auto ret = tempProxy->NotifyWindowOcclusionState(state);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SessionStageProxy> proxy = sptr<SessionStageProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->NotifyWindowOcclusionState(state);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    ret = proxy->NotifyWindowOcclusionState(state);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->NotifyWindowOcclusionState(state);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_NONE);
    ret = proxy->NotifyWindowOcclusionState(state);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: NotifyWindowVisibility
 * @tc.desc: test function : NotifyWindowVisibility
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyWindowVisibility, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->NotifyWindowVisibility(true);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyDensityFollowHost
 * @tc.desc: test function : NotifyDensityFollowHost
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyDensityFollowHost, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    ASSERT_EQ(WSError::WS_OK, sessionStage_->NotifyDensityFollowHost(true, 1.0f));
}

/**
 * @tc.name: NotifyDialogStateChange
 * @tc.desc: test function : NotifyDialogStateChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyDialogStateChange, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->NotifyDialogStateChange(true);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: SetPipActionEvent
 * @tc.desc: test function : SetPipActionEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetPipActionEvent, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->SetPipActionEvent("close", 0);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: SetPiPControlEvent
 * @tc.desc: test function : SetPiPControlEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetPiPControlEvent, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    auto controlType = WsPiPControlType::VIDEO_PLAY_PAUSE;
    auto status = WsPiPControlStatus::PLAY;
    WSError res = sessionStage_->SetPiPControlEvent(controlType, status);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyDisplayMove
 * @tc.desc: test function : NotifyDisplayMove
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyDisplayMove, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    DisplayId from = 0;
    DisplayId to = 1;
    sessionStage_->NotifyDisplayMove(from, to);
}

/**
 * @tc.name: NotifyKeyboardPanelInfoChange
 * @tc.desc: test function : NotifyKeyboardPanelInfoChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyKeyboardPanelInfoChange, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    KeyboardPanelInfo keyboardPanelInfo;
    sessionStage_->NotifyKeyboardPanelInfoChange(keyboardPanelInfo);
}

/**
 * @tc.name: PcAppInPadNormalClose
 * @tc.desc: test function : PcAppInPadNormalClose
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, PcAppInPadNormalClose, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    auto res = sessionStage_->PcAppInPadNormalClose();
    ASSERT_NE(WSError::WS_ERROR_INVALID_WINDOW, res);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    res = sessionStage_->PcAppInPadNormalClose();
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    sptr<SessionStageProxy> sessionStage = sptr<SessionStageProxy>::MakeSptr(nullptr);
    res = sessionStage->PcAppInPadNormalClose();
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: SetUniqueVirtualPixelRatio
 * @tc.desc: test function : SetUniqueVirtualPixelRatio
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetUniqueVirtualPixelRatio, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    sessionStage_->SetUniqueVirtualPixelRatio(true, 0.1f);
}

/**
 * @tc.name: UpdateAnimationSpeed
 * @tc.desc: test function : UpdateAnimationSpeed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateAnimationSpeed, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    float speed = 2.0f;
    ASSERT_TRUE(sessionStage_ != nullptr);

    sptr<SessionStageProxy> nullptrProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    nullptrProxy->UpdateAnimationSpeed(speed);
    EXPECT_TRUE(logMsg.find("remote is nullptr") != std::string::npos);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sessionStage_->UpdateAnimationSpeed(speed);
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteFloatErrorFlag(true);
    sessionStage_->UpdateAnimationSpeed(speed);
    EXPECT_TRUE(logMsg.find("Write speed failed") != std::string::npos);
    MockMessageParcel::SetWriteFloatErrorFlag(false);

    auto remoteMock = sptr<MockIRemoteObject>::MakeSptr();
    remoteMock->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionStageProxy> failSendProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);
    failSendProxy->UpdateAnimationSpeed(speed);
    EXPECT_TRUE(logMsg.find("SendRequest failed") != std::string::npos);

    logMsg.clear();
    sessionStage_->UpdateAnimationSpeed(speed);
    EXPECT_TRUE(logMsg.find("SendRequest failed") == std::string::npos);
}

/**
 * @tc.name: NotifyDumpInfo
 * @tc.desc: test function : NotifyDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyDumpInfo, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    auto res = sessionStage_->NotifyDumpInfo(params, info);
    ASSERT_NE(WSError::WS_OK, res);

    MockMessageParcel::SetReadStringVectorErrorFlag(true);
    res = sessionStage_->NotifyDumpInfo(params, info);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    res = sessionStage_->NotifyDumpInfo(params, info);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
    sptr<SessionStageProxy> sessionStage = sptr<SessionStageProxy>::MakeSptr(nullptr);
    res = sessionStage->NotifyDumpInfo(params, info);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, res);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: SetSplitButtonVisible
 * @tc.desc: test function : SetSplitButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetSplitButtonVisible, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    WSError res = sessionStage_->SetSplitButtonVisible(false);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: SetEnableDragBySystem
 * @tc.desc: test function : SetEnableDragBySystem
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetEnableDragBySystem, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    WSError res = sessionStage_->SetEnableDragBySystem(false);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: SetFullScreenWaterfallMode
 * @tc.desc: test function : SetFullScreenWaterfallMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetFullScreenWaterfallMode, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    WSError res = sessionStage_->SetFullScreenWaterfallMode(false);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: SetSupportEnterWaterfallMode
 * @tc.desc: test function : SetSupportEnterWaterfallMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetSupportEnterWaterfallMode, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    WSError res = sessionStage_->SetSupportEnterWaterfallMode(false);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: SendExtensionData
 * @tc.desc: test function : SendExtensionData
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SendExtensionData, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    data.WriteInterfaceToken(u"OpenHarmeny");
    WSError res = sessionStage_->SendExtensionData(data, reply, option);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: SendContainerModalEvent
 * @tc.desc: test function : SendContainerModalEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SendContainerModalEvent, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    WSError res = sessionStage_->SendContainerModalEvent("name", "value");
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyRotationChange
 * @tc.desc: test function : NotifyRotationChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyRotationChange, Function | SmallTest | Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    RotationChangeInfo info = { RotationChangeType::WINDOW_WILL_ROTATE, 0, 0, { 0, 0, 2720, 1270 } };
    RotationChangeResult res = sessionStage_->NotifyRotationChange(info);
    ASSERT_EQ(0, res.windowRect_.width_);
}

/**
 * @tc.name: NotifyTargetRotationInfo
 * @tc.desc: test function : NotifyTargetRotationInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyTargetRotationInfo, Function | SmallTest | Level1)
{
    OrientationInfo info;
    OrientationInfo currenInfo;
    ASSERT_TRUE(sessionStage_ != nullptr);
    WSError res = sessionStage_->NotifyTargetRotationInfo(info, currenInfo);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyPageRotationIsIgnored
 * @tc.desc: test function : NotifyPageRotationIsIgnored
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyPageRotationIsIgnored, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionStageProxyTest: NotifyPageRotationIsIgnored start";
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SessionStageProxy> sessionStageProxy = sptr<SessionStageProxy>::MakeSptr(remoteMocker);

    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    WSError errCode = sessionStageProxy->NotifyPageRotationIsIgnored();
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

   // Case 2: remote is nullptr
    sptr<SessionStageProxy> nullProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    errCode = nullProxy->NotifyPageRotationIsIgnored();
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);

    // Case 3: Failed to send request
    remoteMocker->SetRequestResult(ERR_TRANSACTION_FAILED);
    errCode = sessionStageProxy->NotifyPageRotationIsIgnored();
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    // Case 4: Success
    errCode = sessionStageProxy->NotifyPageRotationIsIgnored();
    MockMessageParcel::SetReadInt32ErrorFlag(false);
    EXPECT_EQ(errCode, WSError::WS_OK);

    MockMessageParcel::ClearAllErrorFlag();
    GTEST_LOG_(INFO) << "SessionStageProxyTest: NotifyPageRotationIsIgnored end";
}

/**
 * @tc.name: SetCurrentRotation
 * @tc.desc: test function : SetCurrentRotation
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetCurrentRotation, Function | SmallTest | Level1)
{
    int currentRotation = 1;
    ASSERT_TRUE(sessionStage_ != nullptr);
    WSError res = sessionStage_->SetCurrentRotation(currentRotation);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: ReadLittleStringVectorFromParcel
 * @tc.desc: test function : ReadLittleStringVectorFromParcel
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, ReadLittleStringVectorFromParcel, Function | SmallTest | Level1)
{
    bool isAttach = false;
    ASSERT_TRUE(sessionStage_ != nullptr);
    WSError res = sessionStage_->NotifyWindowAttachStateChange(isAttach);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: LinkKeyFrameNode
 * @tc.desc: test function : LinkKeyFrameNode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, LinkKeyFrameNode, Function | SmallTest | Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    WSError res = sessionStage_->LinkKeyFrameNode();
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: SetStageKeyFramePolicy
 * @tc.desc: test function : SetStageKeyFramePolicy
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetStageKeyFramePolicy, Function | SmallTest | Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    KeyFramePolicy keyFramePolicy;
    WSError res = sessionStage_->SetStageKeyFramePolicy(keyFramePolicy);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyWindowCrossAxisChange
 * @tc.desc: test function : NotifyWindowCrossAxisChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyWindowCrossAxisChange, Function | SmallTest | Level1)
{
    CrossAxisState state = CrossAxisState::STATE_CROSS;
    ASSERT_TRUE(sessionStage_ != nullptr);
    sessionStage_->NotifyWindowCrossAxisChange(state);
    ASSERT_NE(nullptr, sessionStage_);
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigUpdated
 * @tc.desc: test function : NotifyAppForceLandscapeConfigUpdated
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyAppForceLandscapeConfigUpdated, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->NotifyAppForceLandscapeConfigUpdated();
    EXPECT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigUpdated01
 * @tc.desc: NotifyAppForceLandscapeConfigUpdated_ShouldReturnIpcFailed_WhenWriteInterfaceTokenFails
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyAppForceLandscapeConfigUpdated01, TestSize.Level1)
{
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_TRUE((sessionStage_ != nullptr));
    WSError res = sessionStage_->NotifyAppForceLandscapeConfigUpdated();
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigUpdated02
 * @tc.desc: NotifyAppForceLandscapeConfigUpdated_ShouldReturnIpcFailed_WhenSessionStageIsNull
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyAppForceLandscapeConfigUpdated02, TestSize.Level1)
{
    sptr<SessionStageProxy> sessionStage = sptr<SessionStageProxy>::MakeSptr(nullptr);
    WSError res = sessionStage->NotifyAppForceLandscapeConfigUpdated();
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: CloseSpecificScene
 * @tc.desc: test function : CloseSpecificScene 1
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, CloseSpecificScene01, Function | SmallTest | Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SessionStageProxy> sProxy = sptr<SessionStageProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);

    auto res = sProxy->CloseSpecificScene();
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    remoteMocker->SetRequestResult(1);
    res = sProxy->CloseSpecificScene();
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    remoteMocker->SetRequestResult(0);
    res = sProxy->CloseSpecificScene();
    remoteMocker->SetRequestResult(0);
    EXPECT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: CloseSpecificScene
 * @tc.desc: test function : CloseSpecificScene 2
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, CloseSpecificScene02, Function | SmallTest | Level1)
{
    sptr<SessionStageProxy> sessionStage2_ = sptr<SessionStageProxy>::MakeSptr(nullptr);
    auto res = sessionStage2_->CloseSpecificScene();
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: SendFbActionEvent
 * @tc.desc: test function : SendFbActionEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SendFbActionEvent, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->SendFbActionEvent("SendFbActionEvent"));

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    ASSERT_EQ(WSError::WS_OK, sessionStage_->SendFbActionEvent("SendFbActionEvent"));

    sptr<SessionStageProxy> sProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sProxy->SendFbActionEvent("SendFbActionEvent"));

    auto remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    remoteMocker->sendRequestResult_ = 1;
    sptr<SessionStageProxy> sessionStage = sptr<SessionStageProxy>::MakeSptr(remoteMocker);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage->SendFbActionEvent("click"));

    MockMessageParcel::SetWriteStringErrorFlag(true);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->SendFbActionEvent("error"));
    MockMessageParcel::SetWriteStringErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: NotifyPiPActiveStatusChange
 * @tc.desc: test function : NotifyPiPActiveStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, NotifyPiPActiveStatusChange, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->NotifyPiPActiveStatusChange(true));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    ASSERT_EQ(WSError::WS_OK, sessionStage_->NotifyPiPActiveStatusChange(true));

    sptr<SessionStageProxy> sProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sProxy->NotifyPiPActiveStatusChange(true));

    auto remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    remoteMocker->sendRequestResult_ = 1;
    sptr<SessionStageProxy> sessionStage = sptr<SessionStageProxy>::MakeSptr(remoteMocker);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage->NotifyPiPActiveStatusChange(true));

    MockMessageParcel::SetWriteBoolErrorFlag(true);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->NotifyPiPActiveStatusChange(true));
    MockMessageParcel::SetWriteBoolErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: TestUpdateGlobalDisplayRectFromServer
 * @tc.desc: Test UpdateGlobalDisplayRectFromServer behavior in various IPC scenarios
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, TestUpdateGlobalDisplayRectFromServer, TestSize.Level1)
{
    WSRect rect = { 10, 20, 100, 200 };
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;

    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->UpdateGlobalDisplayRectFromServer(rect, reason));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Case 2: Failed to write rect
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->UpdateGlobalDisplayRectFromServer(rect, reason));
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    // Case 3: Failed to write reason
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->UpdateGlobalDisplayRectFromServer(rect, reason));
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    // Case 4: remote is nullptr
    sptr<SessionStageProxy> nullProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, nullProxy->UpdateGlobalDisplayRectFromServer(rect, reason));

    // Case 5: Failed to send request
    auto remoteMock = sptr<MockIRemoteObject>::MakeSptr();
    remoteMock->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionStageProxy> failSendProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, failSendProxy->UpdateGlobalDisplayRectFromServer(rect, reason));

    // Case 6: Success
    remoteMock->sendRequestResult_ = ERR_NONE;
    sptr<SessionStageProxy> successProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);
    EXPECT_EQ(WSError::WS_OK, successProxy->UpdateGlobalDisplayRectFromServer(rect, reason));
}

/**
 * @tc.name: UpdateIsShowDecorInFreeMultiWindow
 * @tc.desc: Test UpdateIsShowDecorInFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateIsShowDecorInFreeMultiWindow, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    bool isShow = true;
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->UpdateIsShowDecorInFreeMultiWindow(isShow));

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    ASSERT_EQ(WSError::WS_OK, sessionStage_->UpdateIsShowDecorInFreeMultiWindow(isShow));
    sptr<SessionStageProxy> sProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sProxy->UpdateIsShowDecorInFreeMultiWindow(isShow));

    auto remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    remoteMocker->sendRequestResult_ = 1;
    sptr<SessionStageProxy> sessionStage = sptr<SessionStageProxy>::MakeSptr(remoteMocker);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage->UpdateIsShowDecorInFreeMultiWindow(isShow));

    MockMessageParcel::SetWriteBoolErrorFlag(true);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->UpdateIsShowDecorInFreeMultiWindow(isShow));
    MockMessageParcel::SetWriteStringErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: UpdateBrightness
 * @tc.desc: Test UpdateBrightness
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, UpdateBrightness, TestSize.Level1)
{
    ASSERT_TRUE(sessionStage_ != nullptr);
    float brightness = 1.0f;
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->UpdateBrightness(brightness));

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    EXPECT_EQ(WSError::WS_OK, sessionStage_->UpdateBrightness(brightness));
    sptr<SessionStageProxy> sProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sProxy->UpdateBrightness(brightness));

    auto remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    remoteMocker->sendRequestResult_ = 1;
    sptr<SessionStageProxy> sessionStage = sptr<SessionStageProxy>::MakeSptr(remoteMocker);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage->UpdateBrightness(brightness));

    MockMessageParcel::SetWriteFloatErrorFlag(true);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->UpdateBrightness(brightness));
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: AddSidebarBlur
 * @tc.desc: Test AddSidebarBlur
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, AddSidebarBlur, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->AddSidebarBlur());
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Case 2: remote is nullptr
    sptr<SessionStageProxy> nullProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, nullProxy->AddSidebarBlur());

    // Case 3: Failed to send request
    auto remoteMock = sptr<MockIRemoteObject>::MakeSptr();
    remoteMock->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionStageProxy> failSendProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, failSendProxy->AddSidebarBlur());

    // Case 4: Success
    remoteMock->sendRequestResult_ = ERR_NONE;
    sptr<SessionStageProxy> successProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);
    EXPECT_EQ(WSError::WS_OK, successProxy->AddSidebarBlur());
}

/**
 * @tc.name: SetSidebarBlurStyleWithType
 * @tc.desc: Test SetSidebarBlurStyleWithType
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyTest, SetSidebarBlurStyleWithType, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    SidebarBlurType type = SidebarBlurType::NONE;
    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->SetSidebarBlurStyleWithType(type));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Case 2: Failed to write type
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->SetSidebarBlurStyleWithType(type));
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    // Case 3: remote is nullptr
    sptr<SessionStageProxy> nullProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, nullProxy->SetSidebarBlurStyleWithType(type));

    // Case 4: Failed to send request
    auto remoteMock = sptr<MockIRemoteObject>::MakeSptr();
    remoteMock->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionStageProxy> failSendProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, failSendProxy->SetSidebarBlurStyleWithType(type));

    // Case 5: Success
    remoteMock->sendRequestResult_ = ERR_NONE;
    sptr<SessionStageProxy> successProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);
    EXPECT_EQ(WSError::WS_OK, successProxy->SetSidebarBlurStyleWithType(type));
}
} // namespace
} // namespace Rosen
} // namespace OHOS