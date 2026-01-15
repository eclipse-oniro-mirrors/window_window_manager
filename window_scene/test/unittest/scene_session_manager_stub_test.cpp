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
#include <message_option.h>
#include <message_parcel.h>

#include "iremote_object_mocker.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/zidl/scene_session_manager_interface.h"
#include "session/container/include/window_event_channel.h"
#include "window_manager_agent.h"
#include "zidl/scene_session_manager_stub.h"
#include "zidl/window_manager_agent_interface.h"
#include "pattern_detach_callback.h"
#include "test/mock/mock_session_stage.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class SceneSessionManagerStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<SceneSessionManagerStub> stub_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void SceneSessionManagerStubTest::SetUpTestCase() {}

void SceneSessionManagerStubTest::TearDownTestCase() {}

void SceneSessionManagerStubTest::SetUp()
{
    stub_ = sptr<SceneSessionManager>::MakeSptr();
}

void SceneSessionManagerStubTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: TransIdCreateAndConnectSpecificSession
 * @tc.desc: test TransIdCreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdCreateAndConnectSpecificSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannel>::MakeSptr(sessionStage);
    ASSERT_NE(nullptr, eventChannel);
    data.WriteRemoteObject(eventChannel->AsObject());
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    surfaceNode->SetRSUIContext(rsUIContext);
    ASSERT_NE(nullptr, surfaceNode);
    surfaceNode->Marshalling(data);
    data.WriteBool(false);
    stub_->HandleCreateAndConnectSpecificSession(data, reply);

    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    surfaceNode->Marshalling(data);
    data.WriteBool(true);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetTokenState(true);
    data.WriteStrongParcelable(property);
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_CREATE_AND_CONNECT_SPECIFIC_SESSION);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_TRANSACTION_FAILED);
}

/**
 * @tc.name: TransIdRecoverAndConnectSpecificSession
 * @tc.desc: test TransIdRecoverAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdRecoverAndConnectSpecificSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannel>::MakeSptr(sessionStage);
    ASSERT_NE(nullptr, eventChannel);
    data.WriteRemoteObject(eventChannel->AsObject());
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    surfaceNode->SetRSUIContext(rsUIContext);
    ASSERT_NE(nullptr, surfaceNode);
    surfaceNode->Marshalling(data);
    data.WriteBool(false);
    stub_->HandleRecoverAndConnectSpecificSession(data, reply);

    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    surfaceNode->Marshalling(data);
    data.WriteBool(true);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetTokenState(true);
    data.WriteStrongParcelable(property);
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_RECOVER_AND_CONNECT_SPECIFIC_SESSION);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_TRANSACTION_FAILED);
}

/**
 * @tc.name: TransIdRecoverAndReconnectSceneSession
 * @tc.desc: test TransIdRecoverAndReconnectSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdRecoverAndReconnectSceneSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannel>::MakeSptr(sessionStage);
    ASSERT_NE(nullptr, eventChannel);
    data.WriteRemoteObject(eventChannel->AsObject());
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    surfaceNode->SetRSUIContext(rsUIContext);
    ASSERT_NE(nullptr, surfaceNode);
    surfaceNode->Marshalling(data);
    data.WriteBool(false);
    stub_->HandleRecoverAndConnectSpecificSession(data, reply);

    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    surfaceNode->Marshalling(data);
    data.WriteBool(true);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetTokenState(true);
    data.WriteStrongParcelable(property);
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_RECOVER_AND_RECONNECT_SCENE_SESSION);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_TRANSACTION_FAILED);
}

/**
 * @tc.name: TransIdDestroyAndDisconnectSpecificSession
 * @tc.desc: test TransIdDestroyAndDisconnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdDestroyAndDisconnectSpecificSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_DESTROY_AND_DISCONNECT_SPECIFIC_SESSION);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdDestroyAndDisconnectSpecificSessionWithDetachCallback
 * @tc.desc: test TransIdDestroyAndDisconnectSpecificSessionWithDetachCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdDestroyAndDisconnectSpecificSessionWithDetachCallback, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<PatternDetachCallback> callback = sptr<PatternDetachCallback>::MakeSptr();
    ASSERT_NE(nullptr, callback);
    data.WriteRemoteObject(callback->AsObject());

    uint32_t code = static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::
                                              TRANS_ID_DESTROY_AND_DISCONNECT_SPECIFIC_SESSION_WITH_DETACH_CALLBACK);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdRequestFocus
 * @tc.desc: test TransIdRequestFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdRequestFocus, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);
    bool isFocused = true;
    data.WriteBool(isFocused);

    uint32_t code = static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_REQUEST_FOCUS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetFocusSessionInfo
 * @tc.desc: test TransIdGetFocusSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetFocusSessionInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_INFO);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetFocusSessionInfo1
 * @tc.desc: test TransIdGetFocusSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetFocusSessionInfo1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_INFO);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: TransIdSetSessionLabel
 * @tc.desc: test TransIdSetSessionLabel
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdSetSessionLabel, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteString(static_cast<string>("123"));
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_SET_SESSION_LABEL);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdSetSessionIcon
 * @tc.desc: test TransIdSetSessionIcon
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdSetSessionIcon, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_SET_SESSION_ICON);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: TransIdIsValidSessionIds
 * @tc.desc: test TransIdIsValidSessionIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdIsValidSessionIds, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    std::vector<int32_t> points{ 0, 0 };
    data.WriteInt32Vector(points);

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_IS_VALID_SESSION_IDS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdPendingSessionToForeground
 * @tc.desc: test TransIdPendingSessionToForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdPendingSessionToForeground, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    int32_t windowMode = static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());
    data.WriteInt32(windowMode);
    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_PENDING_SESSION_TO_FOREGROUND);

    data.WriteInt32(1);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdPendingSessionToBackgroundForDelegator
 * @tc.desc: test TransIdPendingSessionToBackgroundForDelegator
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdPendingSessionToBackgroundForDelegator, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    data.WriteString("TransIdPendingSessionToBackgroundForDelegator UT Testing");
    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetFocusSessionToken
 * @tc.desc: test TransIdGetFocusSessionToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetFocusSessionToken, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_TOKEN);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetFocusSessionToken1
 * @tc.desc: test TransIdGetFocusSessionToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetFocusSessionToken1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_TOKEN);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: TransIdGetFocusSessionElement
 * @tc.desc: test TransIdGetFocusSessionElement
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetFocusSessionElement, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_ELEMENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetFocusSessionElement1
 * @tc.desc: test TransIdGetFocusSessionElement
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetFocusSessionElement1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_ELEMENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: TransIdCheckWindowId
 * @tc.desc: test TransIdCheckWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdCheckWindowId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    int32_t windowId = 1000;
    SessionInfo info;
    info.abilityName_ = "HandleCheckWindowId";
    info.bundleName_ = "HandleCheckWindowId1";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    SceneSessionManager::GetInstance().sceneSessionMap_[windowId] = sceneSession;
    data.WriteInt32(windowId);

    uint32_t code = static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_CHECK_WINDOW_ID);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: TransIdSetGestureNavigationEnabled
 * @tc.desc: test TransIdSetGestureNavigationEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdSetGestureNavigationEnabled, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteBool(false);

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_SET_GESTURE_NAVIGATION_ENABLED);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetWindowInfo
 * @tc.desc: test TransIdGetWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetWindowInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));

    uint32_t code = static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_INFO);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetUnreliableWindowInfo
 * @tc.desc: test TransIdGetUnreliableWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetUnreliableWindowInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    int32_t windowId = 0;
    data.WriteInt32(windowId);

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_UNRELIABLE_WINDOW_INFO);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdRegisterSessionListener
 * @tc.desc: test TransIdRegisterSessionListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdRegisterSessionListener, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<PatternDetachCallback> callback = sptr<PatternDetachCallback>::MakeSptr();
    ASSERT_NE(nullptr, callback);
    data.WriteRemoteObject(callback->AsObject());

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_REGISTER_SESSION_LISTENER);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdUnRegisterSessionListener
 * @tc.desc: test TransIdUnRegisterSessionListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdUnRegisterSessionListener, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<PatternDetachCallback> callback = sptr<PatternDetachCallback>::MakeSptr();
    ASSERT_NE(nullptr, callback);
    data.WriteRemoteObject(callback->AsObject());

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_UNREGISTER_SESSION_LISTENER);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetMissionInfos
 * @tc.desc: test TransIdGetMissionInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetMissionInfos, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteString16(static_cast<std::u16string>(u"123"));
    int32_t numMax = 100;
    data.WriteInt32(numMax);

    uint32_t code = static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_MISSION_INFOS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetSessionInfo
 * @tc.desc: test TransIdGetSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetSessionInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteString16(static_cast<std::u16string>(u"123"));
    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_MISSION_INFO_BY_ID);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetSessionInfoByContinueSessionId
 * @tc.desc: test TransIdGetSessionInfoByContinueSessionId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetSessionInfoByContinueSessionId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteString("test_01");

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_SESSION_INFO_BY_CONTINUE_SESSION_ID);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdDumpSessionAll
 * @tc.desc: test TransIdDumpSessionAll
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdDumpSessionAll, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_DUMP_SESSION_ALL);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdDumpSessionWithId
 * @tc.desc: test TransIdDumpSessionWithId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdDumpSessionWithId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    int32_t x = 1;
    data.WriteInt32(x);

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_DUMP_SESSION_WITH_ID);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdTerminateSessionNew
 * @tc.desc: test TransIdTerminateSessionNew
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdTerminateSessionNew, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<AAFwk::SessionInfo> abilitySessionInfo = nullptr;
    data.WriteParcelable(abilitySessionInfo);
    data.WriteBool(true);
    data.WriteBool(true);

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_TERMINATE_SESSION_NEW);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: TransIdUpdateSessionAvoidAreaListener
 * @tc.desc: test TransIdUpdateSessionAvoidAreaListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdUpdateSessionAvoidAreaListener, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);
    bool haveAvoidAreaListener = false;
    data.WriteBool(haveAvoidAreaListener);

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_UPDATE_AVOIDAREA_LISTENER);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetSessionDump
 * @tc.desc: test TransIdGetSessionDump
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetSessionDump, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    std::vector<std::string> params = { "-a" };
    data.WriteStringVector(params);
    stub_->HandleGetSessionDump(data, reply);

    params.clear();
    params.push_back("-w");
    params.push_back("23456");
    data.WriteStringVector(params);
    stub_->HandleGetSessionDump(data, reply);

    params.clear();
    data.WriteStringVector(params);

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_SESSION_DUMP_INFO);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_TRANSACTION_FAILED);
}

/**
 * @tc.name: TransIdGetSessionSnapshot
 * @tc.desc: test TransIdGetSessionSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetSessionSnapshot, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteString16(static_cast<std::u16string>(u"123"));
    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);
    bool isLowResolution = false;
    data.WriteBool(isLowResolution);

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_SESSION_SNAPSHOT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetSessionSnapshotById
 * @tc.desc: test TransIdGetSessionSnapshotById
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetSessionSnapshotById, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    int32_t persistentId = -1;
    data.WriteInt32(persistentId);

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_SESSION_SNAPSHOT_BY_ID);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetUIContentRemoteObj
 * @tc.desc: test TransIdGetUIContentRemoteObj
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetUIContentRemoteObj, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteInt32(1);

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_UI_CONTENT_REMOTE_OBJ);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdBindDialogTarget
 * @tc.desc: test TransIdBindDialogTarget
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdBindDialogTarget, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    uint64_t persistentId = 65535;
    data.WriteUint64(persistentId);
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_BIND_DIALOG_TARGET);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdNotifyDumpInfoResult
 * @tc.desc: test TransIdNotifyDumpInfoResult
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdNotifyDumpInfoResult, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    auto res = stub_->HandleNotifyDumpInfoResult(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    uint32_t vectorSize = 90;
    data.WriteUint32(vectorSize);
    res = stub_->HandleNotifyDumpInfoResult(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdSetSessionContinueState
 * @tc.desc: test TransIdSetSessionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdSetSessionContinueState, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());
    int32_t x = 1;
    data.WriteInt32(x);

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_SET_SESSION_CONTINUE_STATE);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdClearSession
 * @tc.desc: test TransIdClearSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdClearSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    int32_t x = 1;
    data.WriteInt32(x);

    uint32_t code = static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_CLEAR_SESSION);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdClearAllSessions
 * @tc.desc: test TransIdClearAllSessions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdClearAllSessions, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_CLEAR_ALL_SESSIONS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdLockSession
 * @tc.desc: test TransIdLockSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdLockSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    int32_t x = 1;
    data.WriteInt32(x);

    uint32_t code = static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_LOCK_SESSION);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdUnlockSession
 * @tc.desc: test TransIdUnlockSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdUnlockSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    int32_t x = 1;
    data.WriteInt32(x);

    uint32_t code = static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_UNLOCK_SESSION);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: OnRemoteRequest01
 * @tc.desc: test TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, OnRemoteRequest01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest02
 * @tc.desc: test TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, OnRemoteRequest02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest03
 * @tc.desc: test TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, OnRemoteRequest03, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    WindowInfoKey windowInfoKey = WindowInfoKey::DISPLAY_ID;
    data.WriteInt32(static_cast<int32_t>(windowInfoKey));
    uint32_t interestInfo = 0;
    data.WriteUint32(interestInfo);
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_REGISTER_WINDOW_PROPERTY_CHANGE_AGENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest04
 * @tc.desc: test TRANS_ID_UNREGISTER_WINDOW_PROPERTY_CHANGE_AGENT
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, OnRemoteRequest04, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    WindowInfoKey windowInfoKey = WindowInfoKey::DISPLAY_ID;
    data.WriteInt32(static_cast<int32_t>(windowInfoKey));
    uint32_t interestInfo = 0;
    data.WriteUint32(interestInfo);
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_UNREGISTER_WINDOW_PROPERTY_CHANGE_AGENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: HandleCreateAndConnectSpecificSession
 * @tc.desc: test HandleCreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleCreateAndConnectSpecificSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannel>::MakeSptr(sessionStage);
    ASSERT_NE(nullptr, eventChannel);
    data.WriteRemoteObject(eventChannel->AsObject());
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    surfaceNode->SetRSUIContext(rsUIContext);
    surfaceNode->Marshalling(data);
    data.WriteBool(false);
    stub_->HandleCreateAndConnectSpecificSession(data, reply);

    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    surfaceNode->Marshalling(data);
    data.WriteBool(true);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetTokenState(true);
    data.WriteStrongParcelable(property);
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleCreateAndConnectSpecificSession(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleRecoverAndConnectSpecificSession
 * @tc.desc: test HandleRecoverAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRecoverAndConnectSpecificSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannel>::MakeSptr(sessionStage);
    ASSERT_NE(nullptr, eventChannel);
    data.WriteRemoteObject(eventChannel->AsObject());
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    surfaceNode->SetRSUIContext(rsUIContext);
    surfaceNode->Marshalling(data);
    data.WriteBool(false);
    int res = stub_->HandleRecoverAndConnectSpecificSession(data, reply);
    EXPECT_EQ(res, ERR_INVALID_STATE);

    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    surfaceNode->Marshalling(data);
    data.WriteBool(true);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetTokenState(true);
    data.WriteStrongParcelable(property);
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    res = stub_->HandleRecoverAndConnectSpecificSession(data, reply);
    EXPECT_EQ(res, ERR_INVALID_STATE);
}

/**
 * @tc.name: HandleDestroyAndDisconnectSpcificSession
 * @tc.desc: test HandleDestroyAndDisconnectSpcificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleDestroyAndDisconnectSpcificSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));

    int res = stub_->HandleDestroyAndDisconnectSpcificSession(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleDestroyAndDisconnectSpcificSessionWithDetachCallback
 * @tc.desc: test HandleDestroyAndDisconnectSpcificSessionWithDetachCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleDestroyAndDisconnectSpcificSessionWithDetachCallback, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<PatternDetachCallback> callback = sptr<PatternDetachCallback>::MakeSptr();
    data.WriteRemoteObject(callback->AsObject());

    int res = stub_->HandleDestroyAndDisconnectSpcificSession(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleRequestFocusStatus
 * @tc.desc: test HandleRequestFocusStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRequestFocusStatus, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);
    bool isFocused = true;
    data.WriteBool(isFocused);

    int res = stub_->HandleRequestFocusStatus(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleRequestFocusStatusBySA
 * @tc.desc: test HandleRequestFocusStatusBySA
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRequestFocusStatusBySA, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);
    bool isFocused = true;
    data.WriteBool(isFocused);

    int res = stub_->HandleRequestFocusStatusBySA(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleRegisterWindowManagerAgent
 * @tc.desc: test HandleRegisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRegisterWindowManagerAgent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleRegisterWindowManagerAgent(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUnregisterWindowManagerAgent
 * @tc.desc: test HandleUnregisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleUnregisterWindowManagerAgent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleUnregisterWindowManagerAgent(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleRegisterWindowManagerAgent01
 * @tc.desc: test HandleRegisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRegisterWindowPropertyChangeAgent01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WindowInfoKey windowInfoKey = WindowInfoKey::DISPLAY_ID;
    uint32_t interestInfo = 0;
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteInt32(static_cast<int32_t>(windowInfoKey));
    data.WriteUint32(interestInfo);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleRegisterWindowPropertyChangeAgent(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUnregisterWindowPropertyChangeAgent01
 * @tc.desc: test HandleUnregisterWindowPropertyChangeAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleUnregisterWindowPropertyChangeAgent01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WindowInfoKey windowInfoKey = WindowInfoKey::DISPLAY_ID;
    uint32_t interestInfo = 0;
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteInt32(static_cast<int32_t>(windowInfoKey));
    data.WriteUint32(interestInfo);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleUnregisterWindowPropertyChangeAgent(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSkipSnapshotForAppProcess
 * @tc.desc: test HandleSkipSnapshotForAppProcess
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSkipSnapshotForAppProcess, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInt32(123);
    data.WriteBool(true);

    int res = stub_->HandleSkipSnapshotForAppProcess(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetFocusSessionInfo
 * @tc.desc: test HandleGetFocusSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetFocusSessionInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint64(0);
    int res = stub_->HandleGetFocusSessionInfo(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetFocusSessionInfo1
 * @tc.desc: test HandleGetFocusSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetFocusSessionInfo1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int res = stub_->HandleGetFocusSessionInfo(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleGetFocusWindowInfoByAbilityToken
 * @tc.desc: test HandleGetFocusWindowInfoByAbilityToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetFocusWindowInfoByAbilityToken, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);
    int res = stub_->HandleGetFocusWindowInfoByAbilityToken(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    token = sptr<IRemoteObjectMocker>::MakeSptr();
    data.WriteRemoteObject(token);
    res = stub_->HandleGetFocusWindowInfoByAbilityToken(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetFocusSessionElement
 * @tc.desc: test HandleGetFocusSessionElement
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetFocusSessionElement, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint64(0);
    int res = stub_->HandleGetFocusSessionElement(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetFocusSessionElement1
 * @tc.desc: test HandleGetFocusSessionElement
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetFocusSessionElement1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int res = stub_->HandleGetFocusSessionElement(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleSetSessionLabel
 * @tc.desc: test HandleSetSessionLabel
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetSessionLabel, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);
    std::string label = "TestLabel";
    data.WriteString(label);
    int result = stub_->HandleSetSessionLabel(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleSetSessionIcon
 * @tc.desc: test HandleSetSessionIcon
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetSessionIcon, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleSetSessionIcon(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleIsValidSessionIds
 * @tc.desc: test HandleIsValidSessionIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleIsValidSessionIds, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    std::vector<int32_t> points{ 0, 0 };
    data.WriteInt32Vector(points);
    int res = stub_->HandleIsValidSessionIds(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetSessionInfos
 * @tc.desc: test HandleGetSessionInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionInfos, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    data.WriteString16(static_cast<std::u16string>(u"123"));
    int res = stub_->HandleGetSessionInfos(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleGetSessionInfo
 * @tc.desc: test HandleGetSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionInfo, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    data.WriteString16(static_cast<std::u16string>(u"123"));
    int res = stub_->HandleGetSessionInfo(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteString16(static_cast<std::u16string>(u"123"));
    data.WriteInt32(123);
    res = stub_->HandleGetSessionInfo(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetSessionInfo2
 * @tc.desc: test HandleGetSessionInfo2
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionInfo2, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteString16(static_cast<std::u16string>(u"TestDeviceId"));
    data.WriteInt32(123456789);
    int result = stub_->HandleGetSessionInfo(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleDumpSessionAll
 * @tc.desc: test HandleDumpSessionAll
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleDumpSessionAll, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleDumpSessionAll(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleDumpSessionWithId
 * @tc.desc: test HandleDumpSessionWithId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleDumpSessionWithId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleDumpSessionWithId(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    int32_t x = 1;
    data.WriteInt32(x);
    res = stub_->HandleDumpSessionWithId(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetFocusSessionToken
 * @tc.desc: test HandleGetFocusSessionToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetFocusSessionToken, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint64(0);
    int res = stub_->HandleGetFocusSessionToken(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetFocusSessionToken1
 * @tc.desc: test HandleGetFocusSessionToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetFocusSessionToken1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int res = stub_->HandleGetFocusSessionToken(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleCheckWindowId
 * @tc.desc: test HandleCheckWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleCheckWindowId, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    int32_t windowId = 1000;
    SessionInfo info;
    info.abilityName_ = "HandleCheckWindowId";
    info.bundleName_ = "HandleCheckWindowId1";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    SceneSessionManager::GetInstance().sceneSessionMap_[windowId] = sceneSession;
    data.WriteInt32(windowId);

    int res = stub_->HandleCheckWindowId(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleSetGestureNavigationEnabled
 * @tc.desc: test HandleSetGestureNavigationEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetGestureNavigationEnabled, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteBool(false);

    int res = stub_->HandleSetGestureNavigationEnabled(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleConvertToRelativeCoordinateExtended
 * @tc.desc: test HandleConvertToRelativeCoordinateExtended
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleConvertToRelativeCoordinateExtended, TestSize.Level1)
{
    MessageParcel data0;
    MessageParcel reply;

    Rect rect = {200, 100, 400, 600};
    DisplayId newDisplayId = 0;
    data0.WriteInt32(rect.posX_);
    data0.WriteInt32(rect.posY_);
    data0.WriteInt32(rect.width_);
    data0.WriteInt32(rect.height_);
    data0.WriteInt64(newDisplayId);
    int res0 = stub_->HandleConvertToRelativeCoordinateExtended(data0, reply);

    MessageParcel data1;
    data1.WriteInt32(rect.posX_);
    data1.WriteInt32(rect.posY_);
    data1.WriteInt32(rect.width_);
    data1.WriteInt32(rect.height_);
    int res1 = stub_->HandleConvertToRelativeCoordinateExtended(data1, reply);
    EXPECT_EQ(res1, ERR_TRANSACTION_FAILED);

    MessageParcel data2;
    data2.WriteInt32(rect.posX_);
    data2.WriteInt32(rect.posY_);
    data2.WriteInt32(rect.width_);
    int res2 = stub_->HandleConvertToRelativeCoordinateExtended(data2, reply);
    EXPECT_EQ(res2, ERR_TRANSACTION_FAILED);

    MessageParcel data3;
    data3.WriteInt32(rect.posX_);
    data3.WriteInt32(rect.posY_);
    int res3 = stub_->HandleConvertToRelativeCoordinateExtended(data3, reply);
    EXPECT_EQ(res3, ERR_TRANSACTION_FAILED);

    MessageParcel data4;
    data4.WriteInt32(rect.posX_);
    int res4 = stub_->HandleConvertToRelativeCoordinateExtended(data4, reply);
    EXPECT_EQ(res4, ERR_TRANSACTION_FAILED);

    MessageParcel data5;
    int res5 = stub_->HandleConvertToRelativeCoordinateExtended(data5, reply);
    EXPECT_EQ(res5, ERR_TRANSACTION_FAILED);
}

/**
 * @tc.name: HandleGetAccessibilityWindowInfo
 * @tc.desc: test HandleGetAccessibilityWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetAccessibilityWindowInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));

    int res = stub_->HandleGetAccessibilityWindowInfo(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetUnreliableWindowInfo
 * @tc.desc: test HandleGetUnreliableWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetUnreliableWindowInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    int32_t windowId = 0;
    data.WriteInt32(windowId);

    int res = stub_->HandleGetUnreliableWindowInfo(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetSessionContinueState1
 * @tc.desc: test HandleSetSessionContinueState1
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetSessionContinueState1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    int32_t x = 0;
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());
    data.WriteInt32(x);

    int res = stub_->HandleSetSessionContinueState(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetSessionDump
 * @tc.desc: test HandleGetSessionDump
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionDump, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    std::vector<std::string> params = { "-a" };
    data.WriteStringVector(params);
    stub_->HandleGetSessionDump(data, reply);

    params.clear();
    params.push_back("-w");
    params.push_back("23456");
    data.WriteStringVector(params);
    stub_->HandleGetSessionDump(data, reply);

    params.clear();
    data.WriteStringVector(params);

    int res = stub_->HandleGetSessionDump(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUpdateSessionAvoidAreaListener
 * @tc.desc: test HandleUpdateSessionAvoidAreaListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleUpdateSessionAvoidAreaListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);
    bool haveAvoidAreaListener = false;
    data.WriteBool(haveAvoidAreaListener);

    int res = stub_->HandleUpdateSessionAvoidAreaListener(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetSessionSnapshot
 * @tc.desc: test HandleGetSessionSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionSnapshot, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    data.WriteString16(static_cast<std::u16string>(u"123"));
    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);
    bool isLowResolution = false;
    data.WriteBool(isLowResolution);

    int res = stub_->HandleGetSessionSnapshot(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetSessionSnapshotById
 * @tc.desc: test HandleGetSessionSnapshotById
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionSnapshotById, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;
    int32_t persistentId = -1;
    data.WriteInt32(persistentId);
    int res = stub_->HandleGetSessionSnapshotById(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleBindDialogTarget
 * @tc.desc: test HandleBindDialogTarget
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleBindDialogTarget, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    uint64_t persistentId = 65535;
    data.WriteUint64(persistentId);
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleBindDialogTarget(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleNotifyDumpInfoResult
 * @tc.desc: test HandleNotifyDumpInfoResult
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleNotifyDumpInfoResult, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    uint32_t vectorSize = 128;
    data.WriteUint32(vectorSize);
    stub_->HandleNotifyDumpInfoResult(data, reply);

    std::vector<std::string> info = { "-a", "-b123", "-c3456789", "" };
    vectorSize = static_cast<uint32_t>(info.size());
    data.WriteUint32(vectorSize);
    uint32_t curSize;
    for (const auto& elem : info) {
        const char* curInfo = elem.c_str();
        curSize = static_cast<uint32_t>(strlen(curInfo));
        data.WriteUint32(curSize);
        if (curSize != 0) {
            data.WriteRawData(curInfo, curSize);
        }
    }

    int res = stub_->HandleNotifyDumpInfoResult(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUnregisterCollaborator
 * @tc.desc: test HandleUnregisterCollaborator
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleUnregisterCollaborator, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleUnregisterCollaborator(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    int32_t type = CollaboratorType::RESERVE_TYPE;
    data.WriteInt32(type);
    res = stub_->HandleUnregisterCollaborator(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUpdateSessionTouchOutsideListener
 * @tc.desc: test HandleUpdateSessionTouchOutsideListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleUpdateSessionTouchOutsideListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);
    bool haveAvoidAreaListener = true;
    data.WriteBool(haveAvoidAreaListener);

    int res = stub_->HandleUpdateSessionTouchOutsideListener(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleRaiseWindowToTop
 * @tc.desc: test HandleRaiseWindowToTop
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRaiseWindowToTop, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);

    int res = stub_->HandleRaiseWindowToTop(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleNotifyWindowExtensionVisibilityChange
 * @tc.desc: test HandleNotifyWindowExtensionVisibilityChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleNotifyWindowExtensionVisibilityChange, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleNotifyWindowExtensionVisibilityChange(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    int32_t pid = 123;
    data.WriteInt32(pid);
    res = stub_->HandleNotifyWindowExtensionVisibilityChange(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    int32_t uid = 1231;
    data.WriteInt32(pid);
    data.WriteInt32(uid);
    res = stub_->HandleNotifyWindowExtensionVisibilityChange(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    bool visible = true;
    data.WriteInt32(pid);
    data.WriteInt32(uid);
    data.WriteBool(visible);
    res = stub_->HandleNotifyWindowExtensionVisibilityChange(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetTopWindowId
 * @tc.desc: test HandleGetTopWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetTopWindowId, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    uint32_t mainWinId = 65535;
    data.WriteUint32(mainWinId);

    int res = stub_->HandleGetTopWindowId(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleShiftAppWindowFocus
 * @tc.desc: test HandleShiftAppWindowFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleShiftAppWindowFocus, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    int32_t sourcePersistentId = 12345;
    data.WriteInt32(sourcePersistentId);
    int32_t targetPersistentId = 65535;
    data.WriteInt32(targetPersistentId);

    int res = stub_->HandleShiftAppWindowFocus(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetStartWindowBackgroundColor
 * @tc.desc: test HandleSetStartWindowBackgroundColor
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetStartWindowBackgroundColor, TestSize.Level1)
{
    ASSERT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleSetStartWindowBackgroundColor(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteString("moduleName");
    res = stub_->HandleSetStartWindowBackgroundColor(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteString("moduleName");
    data.WriteString("abilityName");
    res = stub_->HandleSetStartWindowBackgroundColor(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteString("moduleName");
    data.WriteString("abilityName");
    data.WriteUint32(0xffffffff);
    res = stub_->HandleSetStartWindowBackgroundColor(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteString("moduleName");
    data.WriteString("abilityName");
    data.WriteUint32(0xffffffff);
    data.WriteInt32(100);
    res = stub_->HandleSetStartWindowBackgroundColor(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleNotifyScreenshotEvent
 * @tc.desc: test HandleNotifyScreenshotEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleNotifyScreenshotEvent, TestSize.Level1)
{
    ASSERT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int res = stub_->HandleNotifyScreenshotEvent(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteInt32(2);
    res = stub_->HandleNotifyScreenshotEvent(data, reply);
    EXPECT_EQ(res, ERR_NONE);

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_NOTIFY_SCREEN_SHOT_EVENT);
    res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleAddExtensionWindowStageToSCB
 * @tc.desc: test HandleAddExtensionWindowStageToSCB
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleAddExtensionWindowStageToSCB, TestSize.Level1)
{
    ASSERT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleAddExtensionWindowStageToSCB(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    res = stub_->HandleAddExtensionWindowStageToSCB(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    data.WriteRemoteObject(token);
    res = stub_->HandleAddExtensionWindowStageToSCB(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    ASSERT_NE(token, nullptr);
    data.WriteRemoteObject(token);
    data.WriteUint64(12345);
    res = stub_->HandleAddExtensionWindowStageToSCB(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    ASSERT_NE(token, nullptr);
    data.WriteRemoteObject(token);
    data.WriteInt64(-1);
    res = stub_->HandleAddExtensionWindowStageToSCB(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteRemoteObject(sessionStage->AsObject());
    ASSERT_NE(token, nullptr);
    data.WriteRemoteObject(token);
    data.WriteUint64(12345);
    data.WriteBool(false);
    data.WriteInt64(0);
    res = stub_->HandleAddExtensionWindowStageToSCB(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleRemoveExtensionWindowStageFromSCB
 * @tc.desc: test HandleRemoveExtensionWindowStageFromSCB
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRemoveExtensionWindowStageFromSCB, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStage, nullptr);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    data.WriteRemoteObject(token);

    sptr<SceneSessionManager> stub = sptr<SceneSessionManager>::MakeSptr();
    stub->remoteExtSessionMap_.clear();
    stub->remoteExtSessionMap_.insert(std::make_pair(sessionStage->AsObject(), token));
    int res = stub->HandleRemoveExtensionWindowStageFromSCB(data, reply);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    ASSERT_NE(sessionStage, nullptr);
    data.WriteRemoteObject(sessionStage->AsObject());
    ASSERT_NE(token, nullptr);
    data.WriteRemoteObject(token);
    data.WriteBool(false);
    res = stub->HandleRemoveExtensionWindowStageFromSCB(data, reply);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUpdateModalExtensionRect
 * @tc.desc: test HandleUpdateModalExtensionRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleUpdateModalExtensionRect, TestSize.Level1)
{
    ASSERT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);
    Rect rect{ 1, 2, 3, 4 };
    data.WriteInt32(rect.posX_);
    data.WriteInt32(rect.posY_);
    data.WriteInt32(rect.width_);
    data.WriteInt32(rect.height_);

    int res = stub_->HandleUpdateModalExtensionRect(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleProcessModalExtensionPointDown
 * @tc.desc: test HandleProcessModalExtensionPointDown
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleProcessModalExtensionPointDown, TestSize.Level1)
{
    ASSERT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);
    int32_t posX = 114;
    data.WriteInt32(posX);
    int32_t posY = 514;
    data.WriteInt32(posY);

    int res = stub_->HandleProcessModalExtensionPointDown(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleAddOrRemoveSecureSession
 * @tc.desc: test HandleAddOrRemoveSecureSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleAddOrRemoveSecureSession, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleAddOrRemoveSecureSession(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteInt32(15);
    res = stub_->HandleAddOrRemoveSecureSession(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteInt32(15);
    data.WriteBool(true);
    res = stub_->HandleAddOrRemoveSecureSession(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetUIContentRemoteObj
 * @tc.desc: test HandleGetUIContentRemoteObj
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetUIContentRemoteObj, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleGetUIContentRemoteObj(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
    data.WriteInt32(1);
    res = stub_->HandleGetUIContentRemoteObj(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetHostWindowRect
 * @tc.desc: test HandleGetHostWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetHostWindowRect, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    int32_t hostWindowId = 65535;
    data.WriteInt32(hostWindowId);

    int res = stub_->HandleGetHostWindowRect(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetHostGlobalScaledRect
 * @tc.desc: test HandleGetHostGlobalScaledRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetHostGlobalScaledRect, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;
    int32_t hostWindowId = 65535;
    data.WriteInt32(hostWindowId);
    int res = stub_->HandleGetHostGlobalScaledRect(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetCallingWindowWindowStatus
 * @tc.desc: test HandleGetCallingWindowWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetCallingWindowWindowStatus, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);

    int res = stub_->HandleGetCallingWindowWindowStatus(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleGetCallingWindowRect
 * @tc.desc: test HandleGetCallingWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetCallingWindowRect, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);

    MessageParcel data;
    MessageParcel reply;

    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);

    int res = stub_->HandleGetCallingWindowRect(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleGetSessionInfoByContinueSessionId
 * @tc.desc: test HandleGetSessionInfoByContinueSessionId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionInfoByContinueSessionId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleGetSessionInfoByContinueSessionId(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
    data.WriteString("test_01");
    res = stub_->HandleGetSessionInfoByContinueSessionId(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUpdateExtWindowFlags
 * @tc.desc: test HandleUpdateExtWindowFlags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleUpdateExtWindowFlags, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);
    data.WriteInt32(7);
    data.WriteInt32(7);

    int res = stub_->HandleUpdateExtWindowFlags(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleGetWindowStyleType
 * @tc.desc: test HandleGetWindowStyleType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetWindowStyleType, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    int res = stub_->HandleGetWindowStyleType(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetProcessWatermark
 * @tc.desc: test HandleSetProcessWatermark
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetProcessWatermark, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInt32(123);
    data.WriteString("SetProcessWatermarkName");
    data.WriteBool(true);

    int res = stub_->HandleSetProcessWatermark(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetProcessSurfaceNodeIdByPersistentId
 * @tc.desc: test HandleGetProcessSurfaceNodeIdByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetProcessSurfaceNodeIdByPersistentId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t pid = 123;
    std::vector<int32_t> persistentIds = { 1, 2, 3 };
    std::vector<uint64_t> surfaceNodeIds;
    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteInt32(pid);
    data.WriteInt32Vector(persistentIds);
    data.WriteUInt64Vector(surfaceNodeIds);
    int res = stub_->HandleGetProcessSurfaceNodeIdByPersistentId(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSkipSnapshotByUserIdAndBundleNames
 * @tc.desc: test HandleSkipSnapshotByUserIdAndBundleNames
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSkipSnapshotByUserIdAndBundleNames, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t userId = 100;
    std::vector<std::string> bundleNameList = { "a", "b", "c" };
    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteInt32(userId);
    data.WriteStringVector(bundleNameList);
    int res = stub_->HandleSkipSnapshotByUserIdAndBundleNames(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUpdateSessionScreenLock
 * @tc.desc: test HandleUpdateSessionScreenLock
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleUpdateSessionScreenLock, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("");
    data.WriteBool(true);
    int res = stub_->HandleUpdateSessionScreenLock(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleAddSkipSelfWhenShowOnVirtualScreenList
 * @tc.desc: test HandleAddSkipSelfWhenShowOnVirtualScreenList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleAddSkipSelfWhenShowOnVirtualScreenList, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint64(1u);
    data.WriteInt32(1);
    int res = stub_->HandleAddSkipSelfWhenShowOnVirtualScreenList(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleRemoveSkipSelfWhenShowOnVirtualScreenList
 * @tc.desc: test HandleRemoveSkipSelfWhenShowOnVirtualScreenList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRemoveSkipSelfWhenShowOnVirtualScreenList, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint64(1u);
    data.WriteInt32(1);
    int res = stub_->HandleRemoveSkipSelfWhenShowOnVirtualScreenList(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleIsPcWindow
 * @tc.desc: test HandleIsPcWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleIsPcWindow, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int res = stub_->HandleIsPcWindow(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleIsFreeMultiWindow
 * @tc.desc: test HandleIsFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleIsFreeMultiWindow, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_IS_FREE_MULTI_WINDOW);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleIsPcOrPadFreeMultiWindowMode
 * @tc.desc: test HandleIsPcOrPadFreeMultiWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleIsPcOrPadFreeMultiWindowMode, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int res = stub_->HandleIsPcOrPadFreeMultiWindowMode(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleIsWindowRectAutoSave
 * @tc.desc: test HandleIsWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleIsWindowRectAutoSave, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::string key = "com.example.recposentryEntryAbility";
    int persistentId = 1;
    data.WriteString(key);
    data.WriteInt32(persistentId);
    int res = stub_->HandleIsWindowRectAutoSave(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetImageForRecent
 * @tc.desc: test HandleSetImageForRecent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetImageForRecent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t imgResourceId = 1;
    ImageFit imageFit = ImageFit::FILL;
    int32_t persistentId = 1;
    data.WriteUint32(imgResourceId);
    data.WriteUint32(static_cast<uint32_t>(imageFit));
    data.WriteInt32(persistentId);
    int res = stub_->HandleSetImageForRecent(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetImageForRecentPixelMap
 * @tc.desc: test HandleSetImageForRecentPixelMap
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetImageForRecentPixelMap, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ImageFit imageFit = ImageFit::FILL;
    int32_t persistentId = 1;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    data.WriteParcelable(pixelMap.get());
    int res = stub_->HandleSetImageForRecentPixelMap(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    // create pixelMap
    const uint32_t colors[1] = { 0x6f0000ff };
    uint32_t colorsLength = sizeof(colors) / sizeof(colors[0]);
    const int32_t offset = 0;
    Media::InitializationOptions opts;
    opts.size.width = 1;
    opts.size.height = 1;
    opts.pixelFormat = Media::PixelFormat::RGBA_8888;
    opts.alphaType = Media::AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    int32_t stride = opts.size.width;
    pixelMap = Media::PixelMap::Create(colors, colorsLength, offset, stride, opts);

    data.WriteParcelable(pixelMap.get());
    res = stub_->HandleSetImageForRecentPixelMap(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteParcelable(pixelMap.get());
    data.WriteUint32(static_cast<uint32_t>(imageFit));
    res = stub_->HandleSetImageForRecentPixelMap(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteParcelable(pixelMap.get());
    data.WriteUint32(static_cast<uint32_t>(imageFit));
    data.WriteInt32(persistentId);
    res = stub_->HandleSetImageForRecentPixelMap(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleRemoveImageForRecent
 * @tc.desc: test HandleRemoveImageForRecent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRemoveImageForRecent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t persistentId = 1;
    data.WriteInt32(persistentId);
    int res = stub_->HandleRemoveImageForRecent(data, reply);
    EXPECT_EQ(res, ERR_NONE);

    res = stub_->HandleRemoveImageForRecent(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleGetDisplayIdByWindowId
 * @tc.desc: test HandleGetDisplayIdByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetDisplayIdByWindowId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    const std::vector<uint64_t> windowIds = { 1, 2 };
    data.WriteUInt64Vector(windowIds);

    int res = stub_->HandleGetDisplayIdByWindowId(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleRegisterCollaborator
 * @tc.desc: test HandleRegisterCollaborator
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRegisterCollaborator, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->HandleRegisterCollaborator(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    int32_t type = CollaboratorType::RESERVE_TYPE;
    data.WriteInt32(type);
    res = stub_->HandleRegisterCollaborator(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleSetGlobalDragResizeType
 * @tc.desc: test HandleSetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetGlobalDragResizeType, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    data.WriteUint32(static_cast<uint32_t>(dragResizeType));
    int res = stub_->HandleSetGlobalDragResizeType(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetGlobalDragResizeType
 * @tc.desc: test HandleGetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetGlobalDragResizeType, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int res = stub_->HandleGetGlobalDragResizeType(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetAppDragResizeType
 * @tc.desc: test HandleSetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetAppDragResizeType, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    const std::string bundleName = "test";
    data.WriteString(bundleName);
    data.WriteUint32(static_cast<uint32_t>(dragResizeType));
    int res = stub_->HandleSetAppDragResizeType(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetParentWindow
 * @tc.desc: test HandleSetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetParentWindow, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t subWindowId = 1;
    int32_t newParentWindowId = 2;
    data.WriteInt32(subWindowId);
    data.WriteInt32(newParentWindowId);
    int res = stub_->HandleSetParentWindow(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetAppDragResizeType
 * @tc.desc: test HandleGetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetAppDragResizeType, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    const std::string bundleName = "test";
    data.WriteString(bundleName);
    int res = stub_->HandleGetAppDragResizeType(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetAppKeyFramePolicy
 * @tc.desc: test HandleSetAppKeyFramePolicy
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetAppKeyFramePolicy, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    const std::string bundleName = "test";
    KeyFramePolicy keyFramePolicy;
    data.WriteString(bundleName);
    data.WriteParcelable(&keyFramePolicy);
    int res = stub_->HandleSetAppKeyFramePolicy(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleminiMizeByWindowId
 * @tc.desc: test HandleMinimizeByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleMinimizeByWindowId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::vector<int32_t> windowIds;
    data.WriteInt32Vector(windowIds);
    int res = stub_->HandleMinimizeByWindowId(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetForegroundWindowNum
 * @tc.desc: test HandleSetForegroundWindowNum
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetForegroundWindowNum, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t windowNum = 1;
    data.WriteInt32(windowNum);
    int res = stub_->HandleSetForegroundWindowNum(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleAnimateTo
 * @tc.desc: test HandleAnimateTo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleAnimateTo, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t windowId = 1;
    data.WriteInt32(windowId);

    WindowAnimationProperty animationProperty;
    data.WriteParcelable(&animationProperty);

    WindowAnimationOption animationOption;
    data.WriteParcelable(&animationOption);

    int res = stub_->HandleAnimateTo(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetPiPSettingSwitchStatus
 * @tc.desc: test HandleGetPiPSettingSwitchStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetPiPSettingSwitchStatus, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    bool switchStatus = false;
    data.WriteBool(switchStatus);

    int res = stub_->HandleGetPiPSettingSwitchStatus(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 *@tc.name: HandleGetIsPipEnabled
 *@tc.desc: test HandleGetIsPipEnabled
 *@tc.type: FUNC
*/
HWTEST_F(SceneSessionManagerStubTest, HandleGetIsPipEnabled, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    bool isPipEnabled = false;
    data.WriteBool(isPipEnabled);
    
    int res = stub_->HandleGetIsPipEnabled(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetScreenPrivacyWindowTagSwitch01
 * @tc.desc: test HandleSetScreenPrivacyWindowTagSwitch
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetScreenPrivacyWindowTagSwitch01, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    std::vector<std::string> privacyWindowTags = { "test" };
    uint64_t screenId = INVALID_SCREEN_ID;
    data.WriteUint64(screenId);

    uint64_t size = privacyWindowTags.size();
    data.WriteUint64(size);
    for (auto privacyWindowTag : privacyWindowTags) {
        data.WriteString(privacyWindowTag);
    }

    bool enable = false;
    data.WriteBool(enable);

    int res = stub_->HandleSetScreenPrivacyWindowTagSwitch(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetScreenPrivacyWindowTagSwitch02
 * @tc.desc: Size is too big
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetScreenPrivacyWindowTagSwitch02, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    std::vector<std::string> privacyWindowTags;
    for (uint64_t i = 0; i < 200; ++i) {
        privacyWindowTags.emplace_back("test");
    }
    uint64_t screenId = INVALID_SCREEN_ID;
    data.WriteUint64(screenId);

    uint64_t size = privacyWindowTags.size();
    data.WriteUint64(size);
    for (auto privacyWindowTag : privacyWindowTags) {
        data.WriteString(privacyWindowTag);
    }

    bool enable = false;
    data.WriteBool(enable);

    int res = stub_->HandleSetScreenPrivacyWindowTagSwitch(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleAddSessionBlackList01
 * @tc.desc: test HandleAddSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleAddSessionBlackList01, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    std::unordered_set<std::string> bundleNames = { "test" };
    std::vector<std::string> privacyWindowTags = { "test" };

    uint64_t size = bundleNames.size();
    data.WriteUint64(size);
    for (auto bundleName : bundleNames) {
        data.WriteString(bundleName);
    }

    size = privacyWindowTags.size();
    data.WriteUint64(size);
    for (auto privacyWindowTag : privacyWindowTags) {
        data.WriteString(privacyWindowTag);
    }

    int res = stub_->HandleAddSessionBlackList(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleAddSessionBlackList02
 * @tc.desc: Size is too big
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleAddSessionBlackList02, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    std::unordered_set<std::string> bundleNames;
    for (int32_t i = 0; i < 200; ++i) {
        bundleNames.insert(std::to_string(i));
    }
    std::vector<std::string> privacyWindowTags;
    for (uint64_t j = 0; j < 200; ++j) {
        privacyWindowTags.emplace_back("test");
    }

    uint64_t size = bundleNames.size();
    data.WriteUint64(size);
    for (auto bundleName : bundleNames) {
        data.WriteString(bundleName);
    }

    size = privacyWindowTags.size();
    data.WriteUint64(size);
    for (auto privacyWindowTag : privacyWindowTags) {
        data.WriteString(privacyWindowTag);
    }

    int res = stub_->HandleAddSessionBlackList(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleRemoveSessionBlackList01
 * @tc.desc: test HandleRemoveSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRemoveSessionBlackList01, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    std::unordered_set<std::string> bundleNames = { "test" };
    std::vector<std::string> privacyWindowTags = { "test" };

    uint64_t size = bundleNames.size();
    data.WriteUint64(size);
    for (auto bundleName : bundleNames) {
        data.WriteString(bundleName);
    }

    size = privacyWindowTags.size();
    data.WriteUint64(size);
    for (auto privacyWindowTag : privacyWindowTags) {
        data.WriteString(privacyWindowTag);
    }

    int res = stub_->HandleRemoveSessionBlackList(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleRemoveSessionBlackList02
 * @tc.desc: Size is too big
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRemoveSessionBlackList02, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    std::unordered_set<std::string> bundleNames;
    for (int32_t i = 0; i < 200; ++i) {
        bundleNames.insert(std::to_string(i));
    }
    std::vector<std::string> privacyWindowTags;
    for (uint64_t j = 0; j < 200; ++j) {
        privacyWindowTags.emplace_back("test");
    }

    uint64_t size = bundleNames.size();
    data.WriteUint64(size);
    for (auto bundleName : bundleNames) {
        data.WriteString(bundleName);
    }

    size = privacyWindowTags.size();
    data.WriteUint64(size);
    for (auto privacyWindowTag : privacyWindowTags) {
        data.WriteString(privacyWindowTag);
    }

    int res = stub_->HandleRemoveSessionBlackList(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleSetSpecificWindowZIndex
 * @tc.desc: HandleSetSpecificWindowZIndex
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetSpecificWindowZIndex, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    int ret = stub_->HandleSetSpecificWindowZIndex(data, reply);
    EXPECT_EQ(ret, ERR_INVALID_DATA);

    data.WriteUint64(2106);
    ret = stub_->HandleSetSpecificWindowZIndex(data, reply);
    EXPECT_EQ(ret, ERR_NONE);

    data.WriteInt32(20);
    ret = stub_->HandleSetSpecificWindowZIndex(data, reply);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
}

/**
 * @tc.name: ResetSpecificWindowZIndex
 * @tc.desc: ResetSpecificWindowZIndex
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, ResetSpecificWindowZIndex, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_RESET_SPECIFIC_WINDOW_ZINDEX);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    int ret = stub_->HandleResetSpecificWindowZIndex(data, reply);
    EXPECT_EQ(ret, ERR_INVALID_DATA);

    data.WriteInt32(20);
    ret = stub_->HandleSetSpecificWindowZIndex(data, reply);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleNotifySupportRotationRegistered
 * @tc.desc: HandleNotifySupportRotationRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleNotifySupportRotationRegistered, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleNotifySupportRotationRegistered(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}
} // namespace
} // namespace Rosen
} // namespace OHOS