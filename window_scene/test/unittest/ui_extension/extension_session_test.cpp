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

#include "extension_session.h"
#include "accessibility_event_info.h"
#include "session_info.h"
#include "interfaces/include/ws_common.h"
#include "key_event.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "extension_data_handler_mock.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class ExtensionSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<ExtensionSession> extensionSession_ = nullptr;
    sptr<SessionStageMocker> mockSessionStage_ = nullptr;
    sptr<WindowEventChannelMocker> mockEventChannel_ = nullptr;
    sptr<ExtensionSession::ExtensionSessionEventCallback> extSessionEventCallback_ = nullptr;
};

void ExtensionSessionTest::SetUpTestCase() {}

void ExtensionSessionTest::TearDownTestCase() {}

void ExtensionSessionTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "ExtensionSessionTest";
    info.bundleName_ = "ExtensionSessionTest";
    extensionSession_ = sptr<ExtensionSession>::MakeSptr(info);
    ASSERT_NE(extensionSession_, nullptr);

    mockSessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage_, nullptr);

    mockEventChannel_ = sptr<WindowEventChannelMocker>::MakeSptr(mockSessionStage_);
    ASSERT_NE(mockEventChannel_, nullptr);

    extSessionEventCallback_ = sptr<ExtensionSession::ExtensionSessionEventCallback>::MakeSptr();
    ASSERT_NE(extSessionEventCallback_, nullptr);
}

void ExtensionSessionTest::TearDown()
{
    extensionSession_ = nullptr;
    mockSessionStage_ = nullptr;
    mockEventChannel_ = nullptr;
    extSessionEventCallback_ = nullptr;
}

namespace {
/**
 * @tc.name: Connect
 * @tc.desc: test function : Connect
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, Connect, TestSize.Level0)
{
    SystemSessionConfig sessionConfig;
    extensionSession_->state_ = SessionState::STATE_DISCONNECT;
    auto res =
        extensionSession_->Connect(mockSessionStage_, mockEventChannel_, nullptr, sessionConfig, nullptr, nullptr, "");
    ASSERT_EQ(res, WSError::WS_OK);

    extensionSession_->state_ = SessionState::STATE_DISCONNECT;
    res = extensionSession_->Connect(mockSessionStage_, nullptr, nullptr, sessionConfig, nullptr, nullptr, "");
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: Connect_TestTransparentUIExtension
 * @tc.desc: test function : Connect test transparent ability
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, Connect_TestTransparentUIExtension, TestSize.Level1)
{
    SystemSessionConfig sessionConfig;
    extensionSession_->state_ = SessionState::STATE_DISCONNECT;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    extensionSession_->dataHandler_ = std::make_shared<Extension::MockDataHandler>();
    extensionSession_->SetIsTransparentUIExtension(true);
    auto res = extensionSession_->Connect(mockSessionStage_, mockEventChannel_, nullptr, sessionConfig, property,
        nullptr, "");
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: RegisterExtensionSessionEventCallback
 * @tc.desc: test function : RegisterExtensionSessionEventCallback
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, RegisterExtensionSessionEventCallback, TestSize.Level1)
{
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    ASSERT_NE(nullptr, extensionSession_->GetExtensionSessionEventCallback());
}

/**
 * @tc.name: GetExtensionSessionEventCallback
 * @tc.desc: test function : GetExtensionSessionEventCallback
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, GetExtensionSessionEventCallback, TestSize.Level1)
{
    ASSERT_NE(nullptr, extensionSession_->GetExtensionSessionEventCallback());
}

/**
 * @tc.name: TransferAbilityResult
 * @tc.desc: test function : TransferAbilityResult
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferAbilityResult, TestSize.Level0)
{
    MockFunction<void(uint32_t, const AAFwk::Want&)> mockTransferAbilityResultFunc;
    extSessionEventCallback_->transferAbilityResultFunc_ = mockTransferAbilityResultFunc.AsStdFunction();
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    uint32_t test = 0;
    AAFwk::Want want;
    EXPECT_CALL(mockTransferAbilityResultFunc, Call(_, _)).Times(1);
    WSError result = extensionSession_->TransferAbilityResult(test, want);
    ASSERT_EQ(result, WSError::WS_OK);

    extSessionEventCallback_->transferAbilityResultFunc_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockTransferAbilityResultFunc, Call(_, _)).Times(0);
    result = extensionSession_->TransferAbilityResult(test, want);
    ASSERT_EQ(result, WSError::WS_OK);

    extSessionEventCallback_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockTransferAbilityResultFunc, Call(_, _)).Times(0);
    result = extensionSession_->TransferAbilityResult(test, want);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: TransferExtensionData
 * @tc.desc: test function : TransferExtensionData
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferExtensionData, TestSize.Level0)
{
    MockFunction<void(const AAFwk::WantParams&)> mockTransferExtensionDataFunc;
    extSessionEventCallback_->transferExtensionDataFunc_ = mockTransferExtensionDataFunc.AsStdFunction();
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    AAFwk::WantParams wantParams;
    EXPECT_CALL(mockTransferExtensionDataFunc, Call(_)).Times(1);
    int32_t result = extensionSession_->TransferExtensionData(wantParams);
    ASSERT_EQ(result, ERR_NONE);

    extSessionEventCallback_->transferExtensionDataFunc_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockTransferExtensionDataFunc, Call(_)).Times(0);
    result = extensionSession_->TransferExtensionData(wantParams);
    ASSERT_EQ(result, ERR_NONE);

    extSessionEventCallback_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockTransferExtensionDataFunc, Call(_)).Times(0);
    result = extensionSession_->TransferExtensionData(wantParams);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: TransferComponentData
 * @tc.desc: test function : TransferComponentData
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferComponentData, TestSize.Level0)
{
    extensionSession_->sessionStage_ = mockSessionStage_;

    extensionSession_->state_ = SessionState::STATE_DISCONNECT;
    AAFwk::WantParams wantParams;
    EXPECT_CALL(*mockSessionStage_, NotifyTransferComponentData).Times(0);
    WSError result = extensionSession_->TransferComponentData(wantParams);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    extensionSession_->state_ = SessionState::STATE_CONNECT;
    EXPECT_CALL(*mockSessionStage_, NotifyTransferComponentData).Times(1);
    result = extensionSession_->TransferComponentData(wantParams);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: TransferComponentDataSync
 * @tc.desc: test function : TransferComponentDataSync
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferComponentDataSync, TestSize.Level1)
{
    extensionSession_->sessionStage_ = mockSessionStage_;

    extensionSession_->state_ = SessionState::STATE_DISCONNECT;
    AAFwk::WantParams wantParams;
    AAFwk::WantParams reWantParams;
    EXPECT_CALL(*mockSessionStage_, NotifyTransferComponentDataSync).Times(0);
    auto res = extensionSession_->TransferComponentDataSync(wantParams, reWantParams);
    ASSERT_EQ(res, WSErrorCode::WS_ERROR_TRANSFER_DATA_FAILED);

    extensionSession_->state_ = SessionState::STATE_CONNECT;
    EXPECT_CALL(*mockSessionStage_, NotifyTransferComponentDataSync).Times(1).WillOnce(Return(WSErrorCode::WS_OK));
    res = extensionSession_->TransferComponentDataSync(wantParams, reWantParams);
    ASSERT_EQ(res, WSErrorCode::WS_OK);
}

/**
 * @tc.name: NotifySyncOn
 * @tc.desc: test function : NotifySyncOn
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, NotifySyncOn, TestSize.Level1)
{
    MockFunction<void()> mockNotifySyncOnFunc;
    extSessionEventCallback_->notifySyncOnFunc_ = mockNotifySyncOnFunc.AsStdFunction();
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifySyncOnFunc, Call()).Times(1);
    extensionSession_->NotifySyncOn();

    extSessionEventCallback_->notifySyncOnFunc_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifySyncOnFunc, Call()).Times(0);
    extensionSession_->NotifySyncOn();

    extSessionEventCallback_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifySyncOnFunc, Call()).Times(0);
    extensionSession_->NotifySyncOn();
}

/**
 * @tc.name: NotifyAsyncOn
 * @tc.desc: test function : NotifyAsyncOn
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, NotifyAsyncOn, TestSize.Level1)
{
    MockFunction<void()> mockNotifyAsyncOnFunc;
    extSessionEventCallback_->notifyAsyncOnFunc_ = mockNotifyAsyncOnFunc.AsStdFunction();
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifyAsyncOnFunc, Call()).Times(1);
    extensionSession_->NotifyAsyncOn();

    extSessionEventCallback_->notifyAsyncOnFunc_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifyAsyncOnFunc, Call()).Times(0);
    extensionSession_->NotifyAsyncOn();

    extSessionEventCallback_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifyAsyncOnFunc, Call()).Times(0);
    extensionSession_->NotifyAsyncOn();
}

/**
 * @tc.name: NotifyDensityFollowHost01
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, NotifyDensityFollowHost01, TestSize.Level1)
{
    extensionSession_->state_ = SessionState::STATE_CONNECT;
    extensionSession_->sessionStage_ = mockSessionStage_;

    bool isFollowHost = true;
    float densityValue = 1.0f;
    EXPECT_CALL(*mockSessionStage_, NotifyDensityFollowHost(isFollowHost, densityValue));
    WSError res = extensionSession_->NotifyDensityFollowHost(isFollowHost, densityValue);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: NotifyDensityFollowHost02
 * @tc.desc: session is invalid
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, NotifyDensityFollowHost02, TestSize.Level1)
{
    bool isFollowHost = true;
    float densityValue = 1.0f;
    WSError res = extensionSession_->NotifyDensityFollowHost(isFollowHost, densityValue);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, res);
}

/**
 * @tc.name: NotifyDensityFollowHost03
 * @tc.desc: sessionStage_ is invalid
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, NotifyDensityFollowHost03, TestSize.Level1)
{
    extensionSession_->state_ = SessionState::STATE_CONNECT;
    extensionSession_->sessionStage_ = nullptr;

    bool isFollowHost = true;
    float densityValue = 1.0f;
    WSError res = extensionSession_->NotifyDensityFollowHost(isFollowHost, densityValue);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, res);
}

/**
 * @tc.name: UpdateSessionViewportConfig1
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, UpdateSessionViewportConfig1, TestSize.Level0)
{
    ASSERT_NE(extensionSession_, nullptr);
    extensionSession_->sessionStage_ = mockSessionStage_;
    extensionSession_->state_ = SessionState::STATE_CONNECT;
    SessionViewportConfig config;
    EXPECT_CALL(*mockSessionStage_, UpdateSessionViewportConfig).Times(1).WillOnce(Return(WSError::WS_OK));
    WSError res = extensionSession_->UpdateSessionViewportConfig(config);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: UpdateSessionViewportConfig2
 * @tc.desc: session is invalid
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, UpdateSessionViewportConfig2, TestSize.Level1)
{
    ASSERT_NE(extensionSession_, nullptr);
    SessionViewportConfig config;
    WSError res = extensionSession_->UpdateSessionViewportConfig(config);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, res);
}

/**
 * @tc.name: UpdateSessionViewportConfig3
 * @tc.desc: sessionStage_ is null
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, UpdateSessionViewportConfig3, TestSize.Level1)
{
    ASSERT_NE(extensionSession_, nullptr);
    extensionSession_->sessionStage_ = nullptr;
    extensionSession_->state_ = SessionState::STATE_CONNECT;
    SessionViewportConfig config;
    WSError res = extensionSession_->UpdateSessionViewportConfig(config);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, res);
}

/**
 * @tc.name: TriggerBindModalUIExtension
 * @tc.desc: test function : TriggerBindModalUIExtension
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TriggerBindModalUIExtension, TestSize.Level1)
{
    extensionSession_->isFirstTriggerBindModal_ = false;
    MockFunction<void()> mockNotifyBindModalFunc;
    extSessionEventCallback_->notifyBindModalFunc_ = mockNotifyBindModalFunc.AsStdFunction();
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifyBindModalFunc, Call()).Times(0);
    extensionSession_->TriggerBindModalUIExtension();

    extensionSession_->isFirstTriggerBindModal_ = true;
    EXPECT_CALL(mockNotifyBindModalFunc, Call()).Times(1);
    extensionSession_->TriggerBindModalUIExtension();

    extSessionEventCallback_->notifyBindModalFunc_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifyBindModalFunc, Call()).Times(0);
    extensionSession_->TriggerBindModalUIExtension();

    extSessionEventCallback_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifyBindModalFunc, Call()).Times(0);
    extensionSession_->TriggerBindModalUIExtension();
}

/**
 * @tc.name: TransferAccessibilityEvent
 * @tc.desc: test function : TransferAccessibilityEvent
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferAccessibilityEvent, TestSize.Level1)
{
    OHOS::Accessibility::AccessibilityEventInfo info1;
    int64_t uiExtensionIdLevel = 6;
    WSError result = extensionSession_->TransferAccessibilityEvent(info1, uiExtensionIdLevel);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: TransferAccessibilityHoverEvent01
 * @tc.desc: TransferAccessibilityHoverEvent01
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferAccessibilityHoverEvent01, TestSize.Level1)
{
    extensionSession_->windowEventChannel_ = mockEventChannel_;
    EXPECT_CALL(*mockEventChannel_, TransferAccessibilityHoverEvent);
    float pointX = 0.0f;
    float pointY = 0.0f;
    int32_t sourceType = 0;
    int32_t eventType = 0;
    int64_t timeMs = 0;
    WSError result = extensionSession_->TransferAccessibilityHoverEvent(pointX, pointY, sourceType, eventType, timeMs);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: TransferAccessibilityHoverEvent02
 * @tc.desc: TransferAccessibilityHoverEvent02
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferAccessibilityHoverEvent02, TestSize.Level1)
{
    extensionSession_->windowEventChannel_ = nullptr;
    float pointX = 0.0f;
    float pointY = 0.0f;
    int32_t sourceType = 0;
    int32_t eventType = 0;
    int64_t timeMs = 0;
    WSError result = extensionSession_->TransferAccessibilityHoverEvent(pointX, pointY, sourceType, eventType, timeMs);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferAccessibilityChildTreeRegister01
 * @tc.desc: TransferAccessibilityChildTreeRegister01
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferAccessibilityChildTreeRegister01, TestSize.Level1)
{
    extensionSession_->windowEventChannel_ = mockEventChannel_;
    EXPECT_CALL(*mockEventChannel_, TransferAccessibilityChildTreeRegister);
    uint32_t windowId = 0;
    int32_t treeId = 0;
    int64_t accessibilityId = 0;
    WSError result = extensionSession_->TransferAccessibilityChildTreeRegister(windowId, treeId, accessibilityId);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: TransferAccessibilityChildTreeRegister02
 * @tc.desc: TransferAccessibilityChildTreeRegister02
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferAccessibilityChildTreeRegister02, TestSize.Level1)
{
    extensionSession_->windowEventChannel_ = nullptr;
    uint32_t windowId = 0;
    int32_t treeId = 0;
    int64_t accessibilityId = 0;
    WSError result = extensionSession_->TransferAccessibilityChildTreeRegister(windowId, treeId, accessibilityId);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferAccessibilityChildTreeUnregister01
 * @tc.desc: TransferAccessibilityChildTreeUnregister01
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferAccessibilityChildTreeUnregister01, TestSize.Level1)
{
    extensionSession_->windowEventChannel_ = mockEventChannel_;
    EXPECT_CALL(*mockEventChannel_, TransferAccessibilityChildTreeUnregister);
    WSError result = extensionSession_->TransferAccessibilityChildTreeUnregister();
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: TransferAccessibilityChildTreeUnregister02
 * @tc.desc: TransferAccessibilityChildTreeUnregister02
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferAccessibilityChildTreeUnregister02, TestSize.Level1)
{
    extensionSession_->windowEventChannel_ = nullptr;
    WSError result = extensionSession_->TransferAccessibilityChildTreeUnregister();
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferAccessibilityDumpChildInfo01
 * @tc.desc: TransferAccessibilityDumpChildInfo01
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferAccessibilityDumpChildInfo01, TestSize.Level1)
{
    extensionSession_->windowEventChannel_ = mockEventChannel_;
    EXPECT_CALL(*mockEventChannel_, TransferAccessibilityDumpChildInfo);
    std::vector<std::string> params;
    std::vector<std::string> info;
    WSError result = extensionSession_->TransferAccessibilityDumpChildInfo(params, info);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: TransferAccessibilityDumpChildInfo02
 * @tc.desc: TransferAccessibilityDumpChildInfo02
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferAccessibilityDumpChildInfo02, TestSize.Level1)
{
    extensionSession_->windowEventChannel_ = nullptr;
    std::vector<std::string> params;
    std::vector<std::string> info;
    WSError result = extensionSession_->TransferAccessibilityDumpChildInfo(params, info);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferKeyEventForConsumed01
 * @tc.desc: TransferKeyEventForConsumed not timeout
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferKeyEventForConsumed01, TestSize.Level0)
{
    extensionSession_->windowEventChannel_ = mockEventChannel_;
    extensionSession_->channelListener_ = sptr<WindowEventChannelListener>::MakeSptr();
    ASSERT_NE(extensionSession_->channelListener_, nullptr);
    EXPECT_CALL(*mockEventChannel_, TransferKeyEventForConsumedAsync)
        .WillOnce([](const std::shared_ptr<MMI::KeyEvent>& keyEvent,
                     bool isPreImeEvent,
                     const sptr<IRemoteObject>& listener) {
            auto channelListener = iface_cast<IWindowEventChannelListener>(listener);
            channelListener->OnTransferKeyEventForConsumed(keyEvent->GetId(), isPreImeEvent, true, WSError::WS_OK);
            return WSError::WS_OK;
        });

    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    bool isConsumed = false;
    bool isTimeout = true;
    bool isPreImeEvent = false;
    WSError result = extensionSession_->TransferKeyEventForConsumed(keyEvent, isConsumed, isTimeout, isPreImeEvent);
    ASSERT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(isTimeout, false);
}

/**
 * @tc.name: TransferKeyEventForConsumed02
 * @tc.desc: TransferKeyEventForConsumed timeout
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferKeyEventForConsumed02, TestSize.Level1)
{
    extensionSession_->windowEventChannel_ = mockEventChannel_;
    EXPECT_CALL(*mockEventChannel_, TransferKeyEventForConsumedAsync);
    extensionSession_->channelListener_ = sptr<WindowEventChannelListener>::MakeSptr();
    ASSERT_NE(extensionSession_->channelListener_, nullptr);

    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    bool isConsumed = false;
    bool isTimeout = false;
    bool isPreImeEvent = false;
    WSError result = extensionSession_->TransferKeyEventForConsumed(keyEvent, isConsumed, isTimeout, isPreImeEvent);
    ASSERT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(isTimeout, true);
}

/**
 * @tc.name: TransferKeyEventForConsumed03
 * @tc.desc: TransferKeyEventForConsumed windowEventChannel_ nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferKeyEventForConsumed03, TestSize.Level1)
{
    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    bool isConsumed = false;
    bool isTimeout = false;
    bool isPreImeEvent = false;
    WSError result = extensionSession_->TransferKeyEventForConsumed(keyEvent, isConsumed, isTimeout, isPreImeEvent);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferKeyEventForConsumed04
 * @tc.desc: TransferKeyEventForConsumed keyEvent nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferKeyEventForConsumed04, TestSize.Level1)
{
    extensionSession_->windowEventChannel_ = mockEventChannel_;

    auto keyEvent = nullptr;
    bool isConsumed = false;
    bool isTimeout = false;
    bool isPreImeEvent = false;
    WSError result = extensionSession_->TransferKeyEventForConsumed(keyEvent, isConsumed, isTimeout, isPreImeEvent);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferKeyEventForConsumed05
 * @tc.desc: TransferKeyEventForConsumed channelListener_ nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferKeyEventForConsumed05, TestSize.Level1)
{
    extensionSession_->windowEventChannel_ = mockEventChannel_;
    extensionSession_->channelListener_ = nullptr;

    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    bool isConsumed = false;
    bool isTimeout = false;
    bool isPreImeEvent = false;
    WSError result = extensionSession_->TransferKeyEventForConsumed(keyEvent, isConsumed, isTimeout, isPreImeEvent);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferKeyEventForConsumed06
 * @tc.desc: TransferKeyEventForConsumed not return OK
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferKeyEventForConsumed06, TestSize.Level1)
{
    extensionSession_->windowEventChannel_ = mockEventChannel_;
    extensionSession_->channelListener_ = sptr<WindowEventChannelListener>::MakeSptr();
    ASSERT_NE(extensionSession_->channelListener_, nullptr);
    EXPECT_CALL(*mockEventChannel_, TransferKeyEventForConsumedAsync).WillOnce(Return(WSError::WS_DO_NOTHING));

    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    bool isConsumed = false;
    bool isTimeout = false;
    bool isPreImeEvent = false;
    WSError result = extensionSession_->TransferKeyEventForConsumed(keyEvent, isConsumed, isTimeout, isPreImeEvent);
    ASSERT_EQ(result, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: WindowEventChannelListenerOnRemoteRequest01
 * @tc.desc: WindowEventChannelListenerOnRemoteRequest01 test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, WindowEventChannelListenerOnRemoteRequest01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(WindowEventChannelListener::GetDescriptor());
    data.WriteInt32(0);
    data.WriteBool(true);
    data.WriteBool(true);
    data.WriteInt32(0);
    uint32_t code = static_cast<uint32_t>(IWindowEventChannelListener::WindowEventChannelListenerMessage::
                                              TRANS_ID_ON_TRANSFER_KEY_EVENT_FOR_CONSUMED_ASYNC);
    WindowEventChannelListener listener;
    ASSERT_EQ(listener.OnRemoteRequest(code, data, reply, option), 0);
}

/**
 * @tc.name: WindowEventChannelListenerOnRemoteRequest02
 * @tc.desc: WindowEventChannelListenerOnRemoteRequest02 test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, WindowEventChannelListenerOnRemoteRequest02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(WindowEventChannelListener::GetDescriptor());
    data.WriteBool(true);
    data.WriteInt32(0);
    uint32_t code = static_cast<uint32_t>(10001);
    WindowEventChannelListener listener;
    ASSERT_EQ(listener.OnRemoteRequest(code, data, reply, option), IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: ChannelDeathRecipientOnRemoteDied01
 * @tc.desc: ChannelDeathRecipientOnRemoteDied01 test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, ChannelDeathRecipientOnRemoteDied01, TestSize.Level1)
{
    sptr<WindowEventChannelListener> listener = sptr<WindowEventChannelListener>::MakeSptr();
    EXPECT_NE(nullptr, listener);
    sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
    deathRecipient = sptr<ChannelDeathRecipient>::MakeSptr(listener);
    EXPECT_NE(nullptr, deathRecipient);
    sptr<IRemoteObject> wptrDeath = nullptr;
    wptrDeath = sptr<WindowEventChannel>::MakeSptr(nullptr);
    ASSERT_NE(nullptr, wptrDeath);
    deathRecipient->OnRemoteDied(wptrDeath);
    EXPECT_NE(nullptr, deathRecipient);
}

/**
 * @tc.name: ChannelDeathRecipientOnRemoteDied02
 * @tc.desc: ChannelDeathRecipientOnRemoteDied02 test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, ChannelDeathRecipientOnRemoteDied02, TestSize.Level1)
{
    sptr<WindowEventChannelListener> listener = sptr<WindowEventChannelListener>::MakeSptr();
    EXPECT_NE(nullptr, listener);
    sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
    deathRecipient = sptr<ChannelDeathRecipient>::MakeSptr(listener);
    EXPECT_NE(nullptr, deathRecipient);
    deathRecipient->OnRemoteDied(nullptr);
    EXPECT_NE(nullptr, deathRecipient);
}

/**
 * @tc.name: TransferKeyEventAsync
 * @tc.desc: TransferKeyEventAsync
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TransferKeyEventAsync, TestSize.Level0)
{
    extensionSession_->windowEventChannel_ = mockEventChannel_;
    extensionSession_->channelListener_ = sptr<WindowEventChannelListener>::MakeSptr();
    ASSERT_NE(extensionSession_->channelListener_, nullptr);

    auto keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    bool isPreImeEvent = false;
    EXPECT_CALL(*mockEventChannel_, TransferKeyEventForConsumedAsync).Times(1).WillOnce(Return(WSError::WS_OK));
    WSError result = extensionSession_->TransferKeyEventAsync(keyEvent, isPreImeEvent);
    ASSERT_EQ(result, WSError::WS_OK);

    keyEvent = nullptr;
    EXPECT_CALL(*mockEventChannel_, TransferKeyEventForConsumedAsync).Times(0);
    result = extensionSession_->TransferKeyEventAsync(keyEvent, isPreImeEvent);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    extensionSession_->windowEventChannel_ = nullptr;
    EXPECT_CALL(*mockEventChannel_, TransferKeyEventForConsumedAsync).Times(0);
    result = extensionSession_->TransferKeyEventAsync(keyEvent, isPreImeEvent);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateAvoidArea
 * @tc.desc: test function : UpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, UpdateAvoidArea, TestSize.Level0)
{
    extensionSession_->sessionStage_ = mockSessionStage_;

    extensionSession_->state_ = SessionState::STATE_DISCONNECT;
    sptr<AvoidArea> avoidArea = sptr<AvoidArea>::MakeSptr();
    ASSERT_NE(avoidArea, nullptr);
    AvoidAreaType type = AvoidAreaType::TYPE_SYSTEM;
    EXPECT_CALL(*mockSessionStage_, UpdateAvoidArea).Times(0);
    WSError res = extensionSession_->UpdateAvoidArea(avoidArea, type);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, res);

    extensionSession_->state_ = SessionState::STATE_CONNECT;
    EXPECT_CALL(*mockSessionStage_, UpdateAvoidArea).Times(1).WillOnce(Return(WSError::WS_OK));
    res = extensionSession_->UpdateAvoidArea(avoidArea, type);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: test function : GetAvoidAreaByType
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, GetAvoidAreaByType, TestSize.Level0)
{
    MockFunction<AvoidArea(AvoidAreaType type, int32_t apiVersion)> mockNotifyGetAvoidAreaByTypeFunc;
    extSessionEventCallback_->notifyGetAvoidAreaByTypeFunc_ = mockNotifyGetAvoidAreaByTypeFunc.AsStdFunction();
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    AvoidAreaType typeSystem = AvoidAreaType::TYPE_SYSTEM;
    AvoidAreaType typeCutout = AvoidAreaType::TYPE_CUTOUT;
    AvoidAreaType typeSystemGesture = AvoidAreaType::TYPE_SYSTEM_GESTURE;
    AvoidAreaType typeKeyboard = AvoidAreaType::TYPE_KEYBOARD;
    AvoidAreaType typeNavigationIndicator = AvoidAreaType::TYPE_NAVIGATION_INDICATOR;
    AvoidArea expectedAvoidArea;
    expectedAvoidArea.topRect_ = { 10, 20, 30, 40 };
    EXPECT_CALL(mockNotifyGetAvoidAreaByTypeFunc, Call(_, _)).Times(5).WillRepeatedly(Return(expectedAvoidArea));
    auto res = extensionSession_->GetAvoidAreaByType(typeSystem);
    ASSERT_EQ(res, expectedAvoidArea);
    res = extensionSession_->GetAvoidAreaByType(typeCutout);
    ASSERT_EQ(res, expectedAvoidArea);
    res = extensionSession_->GetAvoidAreaByType(typeSystemGesture);
    ASSERT_EQ(res, expectedAvoidArea);
    res = extensionSession_->GetAvoidAreaByType(typeKeyboard);
    ASSERT_EQ(res, expectedAvoidArea);
    res = extensionSession_->GetAvoidAreaByType(typeNavigationIndicator);
    ASSERT_EQ(res, expectedAvoidArea);

    extSessionEventCallback_->notifyGetAvoidAreaByTypeFunc_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifyGetAvoidAreaByTypeFunc, Call(_, _)).Times(0);
    res = extensionSession_->GetAvoidAreaByType(typeSystem);
    ASSERT_EQ(res, AvoidArea());
    res = extensionSession_->GetAvoidAreaByType(typeCutout);
    ASSERT_EQ(res, AvoidArea());
    res = extensionSession_->GetAvoidAreaByType(typeSystemGesture);
    ASSERT_EQ(res, AvoidArea());
    res = extensionSession_->GetAvoidAreaByType(typeKeyboard);
    ASSERT_EQ(res, AvoidArea());
    res = extensionSession_->GetAvoidAreaByType(typeNavigationIndicator);
    ASSERT_EQ(res, AvoidArea());

    extSessionEventCallback_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifyGetAvoidAreaByTypeFunc, Call(_, _)).Times(0);
    res = extensionSession_->GetAvoidAreaByType(typeSystem);
    ASSERT_EQ(res, AvoidArea());
    res = extensionSession_->GetAvoidAreaByType(typeCutout);
    ASSERT_EQ(res, AvoidArea());
    res = extensionSession_->GetAvoidAreaByType(typeSystemGesture);
    ASSERT_EQ(res, AvoidArea());
    res = extensionSession_->GetAvoidAreaByType(typeKeyboard);
    ASSERT_EQ(res, AvoidArea());
    res = extensionSession_->GetAvoidAreaByType(typeNavigationIndicator);
    ASSERT_EQ(res, AvoidArea());
}

/**
 * @tc.name: Background
 * @tc.desc: test function : Background
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, Background, TestSize.Level0)
{
    ASSERT_NE(extensionSession_->property_, nullptr);
    extensionSession_->state_ = SessionState::STATE_DISCONNECT;
    extensionSession_->property_->type_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    auto res = extensionSession_->Background();
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_SESSION);

    extensionSession_->state_ = SessionState::STATE_ACTIVE;
    res = extensionSession_->Background();
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_SESSION);

    extensionSession_->property_->type_ = WindowType::WINDOW_TYPE_UI_EXTENSION;
    res = extensionSession_->Background();
    ASSERT_EQ(res, WSError::WS_OK);
    ASSERT_FALSE(extensionSession_->isActive_);
    ASSERT_EQ(extensionSession_->state_, SessionState::STATE_BACKGROUND);

    extensionSession_->state_ = SessionState::STATE_DISCONNECT;
    res = extensionSession_->Background();
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_SESSION);

    extensionSession_->state_ = SessionState::STATE_BACKGROUND;
    res = extensionSession_->Background();
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: NotifyExtensionEventAsync
 * @tc.desc: test function : NotifyExtensionEventAsync
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, NotifyExtensionEventAsync, TestSize.Level1)
{
    ASSERT_NE(nullptr, extSessionEventCallback_);
    MockFunction<void(uint32_t)> mockNotifyExtensionEventFunc;
    extSessionEventCallback_->notifyExtensionEventFunc_ = mockNotifyExtensionEventFunc.AsStdFunction();
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifyExtensionEventFunc, Call(_)).Times(1);
    extensionSession_->NotifyExtensionEventAsync(0);

    extSessionEventCallback_->notifyExtensionEventFunc_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifyExtensionEventFunc, Call(_)).Times(0);
    extensionSession_->NotifyExtensionEventAsync(0);

    extSessionEventCallback_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockNotifyExtensionEventFunc, Call(_)).Times(0);
    extensionSession_->NotifyExtensionEventAsync(0);
}

/**
 * @tc.name: NotifyDumpInfo
 * @tc.desc: test function : NotifyDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, NotifyDumpInfo, TestSize.Level1)
{
    extensionSession_->sessionStage_ = mockSessionStage_;
    extensionSession_->state_ = SessionState::STATE_DISCONNECT;
    std::vector<std::string> params;
    std::vector<std::string> info;
    WSError res = extensionSession_->NotifyDumpInfo(params, info);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, res);

    extensionSession_->state_ = SessionState::STATE_CONNECT;
    EXPECT_CALL(*mockSessionStage_, NotifyDumpInfo).Times(1).WillOnce(Return(WSError::WS_OK));
    res = extensionSession_->NotifyDumpInfo(params, info);
    ASSERT_EQ(WSError::WS_OK, res);

    extensionSession_->sessionStage_ = nullptr;
    res = extensionSession_->NotifyDumpInfo(params, info);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, res);
}

/**
 * @tc.name: GetStatusBarHeight
 * @tc.desc: test function : GetStatusBarHeight
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, GetStatusBarHeight, TestSize.Level1)
{
    ASSERT_NE(nullptr, extSessionEventCallback_);
    MockFunction<uint32_t()> mockGetStatusBarHeightFunc;
    extSessionEventCallback_->getStatusBarHeightFunc_ = mockGetStatusBarHeightFunc.AsStdFunction();
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockGetStatusBarHeightFunc, Call()).Times(1);
    extensionSession_->GetStatusBarHeight();

    extSessionEventCallback_->getStatusBarHeightFunc_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockGetStatusBarHeightFunc, Call()).Times(0);
    extensionSession_->GetStatusBarHeight();

    extSessionEventCallback_ = nullptr;
    extensionSession_->RegisterExtensionSessionEventCallback(extSessionEventCallback_);
    EXPECT_CALL(mockGetStatusBarHeightFunc, Call()).Times(0);
    extensionSession_->GetStatusBarHeight();
}

/**
 * @tc.name: TryUpdateExtensionPersistentId
 * @tc.desc: test function : TryUpdateExtensionPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionTest, TryUpdateExtensionPersistentId, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ExtensionSessionTest";
    info.bundleName_ = "ExtensionSessionTest";
    info.persistentId_ = INVALID_SESSION_ID;
    sptr<ExtensionSession> extensionSessionA = sptr<ExtensionSession>::MakeSptr(info);
    info.persistentId_ = extensionSessionA->GetPersistentId();
    sptr<ExtensionSession> extensionSessionB = sptr<ExtensionSession>::MakeSptr(info);
    ASSERT_EQ(info.persistentId_ + 1, extensionSessionB->GetPersistentId());
    extensionSessionA.clear();
    info.persistentId_ += 4096;
    sptr<ExtensionSession> extensionSessionC = sptr<ExtensionSession>::MakeSptr(info);
    ASSERT_EQ(info.persistentId_, extensionSessionC->GetPersistentId());
}
} // namespace
} // namespace Rosen
} // namespace OHOS