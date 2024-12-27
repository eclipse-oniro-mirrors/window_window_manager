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

void SceneSessionManagerStubTest::SetUpTestCase()
{
}

void SceneSessionManagerStubTest::TearDownTestCase()
{
}

void SceneSessionManagerStubTest::SetUp()
{
    stub_ = new SceneSessionManager();
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
HWTEST_F(SceneSessionManagerStubTest, TransIdCreateAndConnectSpecificSession, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<ISessionStage> sessionStage = new SessionStageMocker();
    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IWindowEventChannel> eventChannel = new WindowEventChannel(sessionStage);
    ASSERT_NE(nullptr, eventChannel);
    data.WriteRemoteObject(eventChannel->AsObject());
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_NE(nullptr, surfaceNode);
    surfaceNode->Marshalling(data);
    data.WriteBool(false);
    stub_->HandleCreateAndConnectSpecificSession(data, reply);

    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    surfaceNode->Marshalling(data);
    data.WriteBool(true);
    sptr<WindowSessionProperty> property = new WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    property->SetTokenState(true);
    data.WriteStrongParcelable(property);
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
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
HWTEST_F(SceneSessionManagerStubTest, TransIdRecoverAndConnectSpecificSession, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<ISessionStage> sessionStage = new SessionStageMocker();
    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IWindowEventChannel> eventChannel = new WindowEventChannel(sessionStage);
    ASSERT_NE(nullptr, eventChannel);
    data.WriteRemoteObject(eventChannel->AsObject());
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_NE(nullptr, surfaceNode);
    surfaceNode->Marshalling(data);
    data.WriteBool(false);
    stub_->HandleRecoverAndConnectSpecificSession(data, reply);

    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    surfaceNode->Marshalling(data);
    data.WriteBool(true);
    sptr<WindowSessionProperty> property = new WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    property->SetTokenState(true);
    data.WriteStrongParcelable(property);
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
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
HWTEST_F(SceneSessionManagerStubTest, TransIdRecoverAndReconnectSceneSession, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<ISessionStage> sessionStage = new SessionStageMocker();
    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IWindowEventChannel> eventChannel = new WindowEventChannel(sessionStage);
    ASSERT_NE(nullptr, eventChannel);
    data.WriteRemoteObject(eventChannel->AsObject());
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_NE(nullptr, surfaceNode);
    surfaceNode->Marshalling(data);
    data.WriteBool(false);
    stub_->HandleRecoverAndConnectSpecificSession(data, reply);

    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    surfaceNode->Marshalling(data);
    data.WriteBool(true);
    sptr<WindowSessionProperty> property = new WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    property->SetTokenState(true);
    data.WriteStrongParcelable(property);
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
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
HWTEST_F(SceneSessionManagerStubTest, TransIdDestroyAndDisconnectSpecificSession, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdDestroyAndDisconnectSpecificSessionWithDetachCallback,
         Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<PatternDetachCallback> callback = new PatternDetachCallback();
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
HWTEST_F(SceneSessionManagerStubTest, TransIdRequestFocus, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdGetFocusSessionInfo, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_INFO);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdSetSessionLabel
 * @tc.desc: test TransIdSetSessionLabel
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdSetSessionLabel, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteString(static_cast<string>("123"));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
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
HWTEST_F(SceneSessionManagerStubTest, TransIdSetSessionIcon, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
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
HWTEST_F(SceneSessionManagerStubTest, TransIdIsValidSessionIds, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    std::vector<int32_t> points{0, 0};
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
HWTEST_F(SceneSessionManagerStubTest, TransIdPendingSessionToForeground, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_PENDING_SESSION_TO_FOREGROUND);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdPendingSessionToBackgroundForDelegator
 * @tc.desc: test TransIdPendingSessionToBackgroundForDelegator
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdPendingSessionToBackgroundForDelegator, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

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
HWTEST_F(SceneSessionManagerStubTest, TransIdGetFocusSessionToken, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_TOKEN);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdGetFocusSessionElement
 * @tc.desc: test TransIdGetFocusSessionElement
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdGetFocusSessionElement, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_ELEMENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: TransIdCheckWindowId
 * @tc.desc: test TransIdCheckWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdCheckWindowId, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    int32_t windowId = 1000;
    SessionInfo info;
    info.abilityName_ = "HandleCheckWindowId";
    info.bundleName_ = "HandleCheckWindowId1";
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
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
HWTEST_F(SceneSessionManagerStubTest, TransIdSetGestureNavigationEnabled, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdGetWindowInfo, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdGetUnreliableWindowInfo, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdRegisterSessionListener, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<PatternDetachCallback> callback = new PatternDetachCallback();
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
HWTEST_F(SceneSessionManagerStubTest, TransIdUnRegisterSessionListener, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<PatternDetachCallback> callback = new PatternDetachCallback();
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
HWTEST_F(SceneSessionManagerStubTest, TransIdGetMissionInfos, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdGetSessionInfo, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdGetSessionInfoByContinueSessionId, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdDumpSessionAll, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdDumpSessionWithId, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdTerminateSessionNew, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<AAFwk::SessionInfo> abilitySessionInfo = 0;
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
HWTEST_F(SceneSessionManagerStubTest, TransIdUpdateSessionAvoidAreaListener, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdGetSessionDump, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    std::vector<std::string> params = {"-a"};
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
HWTEST_F(SceneSessionManagerStubTest, TransIdGetSessionSnapshot, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdGetSessionSnapshotById, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdGetUIContentRemoteObj, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdBindDialogTarget, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    uint64_t persistentId = 65535;
    data.WriteUint64(persistentId);
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
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
HWTEST_F(SceneSessionManagerStubTest, TransIdNotifyDumpInfoResult, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    uint32_t vectorSize = 128;
    data.WriteUint32(vectorSize);
    stub_->HandleNotifyDumpInfoResult(data, reply);

    std::vector<std::string> info = {"-a", "-b123", "-c3456789", ""};
    vectorSize = static_cast<uint32_t>(info.size());
    data.WriteUint32(vectorSize);
    uint32_t curSize;
    for (const auto &elem : info) {
        const char *curInfo = elem.c_str();
        curSize = static_cast<uint32_t>(strlen(curInfo));
        data.WriteUint32(curSize);
        if (curSize != 0) {
            data.WriteRawData(curInfo, curSize);
        }
    }

    uint32_t code =
        static_cast<uint32_t>(ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_NOTIFY_DUMP_INFO_RESULT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_TRANSACTION_FAILED);
}

/**
 * @tc.name: TransIdSetSessionContinueState
 * @tc.desc: test TransIdSetSessionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, TransIdSetSessionContinueState, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
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
HWTEST_F(SceneSessionManagerStubTest, TransIdClearSession, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdClearAllSessions, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdLockSession, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, TransIdUnlockSession, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, OnRemoteRequest01, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest02
 * @tc.desc: test TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, OnRemoteRequest02, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: HandleCreateAndConnectSpecificSession
 * @tc.desc: test HandleCreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleCreateAndConnectSpecificSession, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;

    sptr<ISessionStage> sessionStage = new SessionStageMocker();
    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IWindowEventChannel> eventChannel = new WindowEventChannel(sessionStage);
    ASSERT_NE(nullptr, eventChannel);
    data.WriteRemoteObject(eventChannel->AsObject());
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    surfaceNode->Marshalling(data);
    data.WriteBool(false);
    stub_->HandleCreateAndConnectSpecificSession(data, reply);

    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    surfaceNode->Marshalling(data);
    data.WriteBool(true);
    sptr<WindowSessionProperty> property = new WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    property->SetTokenState(true);
    data.WriteStrongParcelable(property);
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleCreateAndConnectSpecificSession(data, reply);
    EXPECT_EQ(res, ERR_INVALID_STATE);
}

/**
 * @tc.name: HandleRecoverAndConnectSpecificSession
 * @tc.desc: test HandleRecoverAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRecoverAndConnectSpecificSession, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;

    sptr<ISessionStage> sessionStage = new SessionStageMocker();
    ASSERT_NE(nullptr, sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IWindowEventChannel> eventChannel = new WindowEventChannel(sessionStage);
    ASSERT_NE(nullptr, eventChannel);
    data.WriteRemoteObject(eventChannel->AsObject());
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    surfaceNode->Marshalling(data);
    data.WriteBool(false);
    stub_->HandleRecoverAndConnectSpecificSession(data, reply);

    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    surfaceNode->Marshalling(data);
    data.WriteBool(true);
    sptr<WindowSessionProperty> property = new WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    property->SetTokenState(true);
    data.WriteStrongParcelable(property);
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    ASSERT_NE(nullptr, windowManagerAgent);
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleRecoverAndConnectSpecificSession(data, reply);
    EXPECT_EQ(res, ERR_INVALID_STATE);
}

/**
 * @tc.name: HandleDestroyAndDisconnectSpcificSession
 * @tc.desc: test HandleDestroyAndDisconnectSpcificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleDestroyAndDisconnectSpcificSession, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, HandleDestroyAndDisconnectSpcificSessionWithDetachCallback,
    Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<PatternDetachCallback> callback = new PatternDetachCallback();
    data.WriteRemoteObject(callback->AsObject());

    int res = stub_->HandleDestroyAndDisconnectSpcificSession(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleRequestFocusStatus
 * @tc.desc: test HandleRequestFocusStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRequestFocusStatus, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

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
 * @tc.name: HandleRegisterWindowManagerAgent
 * @tc.desc: test HandleRegisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRegisterWindowManagerAgent, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleRegisterWindowManagerAgent(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUnregisterWindowManagerAgent
 * @tc.desc: test HandleUnregisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleUnregisterWindowManagerAgent, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleUnregisterWindowManagerAgent(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetFocusSessionInfo
 * @tc.desc: test HandleGetFocusSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetFocusSessionInfo, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleGetFocusSessionInfo(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetFocusSessionElement
 * @tc.desc: test HandleGetFocusSessionElement
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetFocusSessionElement, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    int res = stub_->HandleGetFocusSessionElement(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetSessionLabel
 * @tc.desc: test HandleSetSessionLabel
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetSessionLabel, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteString(static_cast<string>("123"));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleSetSessionLabel(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetSessionIcon
 * @tc.desc: test HandleSetSessionIcon
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetSessionIcon, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;

    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleSetSessionIcon(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleIsValidSessionIds
 * @tc.desc: test HandleIsValidSessionIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleIsValidSessionIds, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    std::vector<int32_t> points {0, 0};
    data.WriteInt32Vector(points);
    int res = stub_->HandleIsValidSessionIds(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetSessionInfos
 * @tc.desc: test HandleGetSessionInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionInfos, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;

    data.WriteString16(static_cast<std::u16string>(u"123"));
    int32_t numMax = 100;
    data.WriteInt32(numMax);

    int res = stub_->HandleGetSessionInfos(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetSessionInfo
 * @tc.desc: test HandleGetSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionInfo, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;

    data.WriteString16(static_cast<std::u16string>(u"123"));
    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);

    int res = stub_->HandleGetSessionInfo(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleDumpSessionAll
 * @tc.desc: test HandleDumpSessionAll
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleDumpSessionAll, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, HandleDumpSessionWithId, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    int32_t x = 1;
    data.WriteInt32(x);

    int res = stub_->HandleDumpSessionWithId(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetFocusSessionToken
 * @tc.desc: test HandleGetFocusSessionToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetFocusSessionToken, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleGetFocusSessionToken(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleCheckWindowId
 * @tc.desc: test HandleCheckWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleCheckWindowId, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;

    int32_t windowId = 1000;
    SessionInfo info;
    info.abilityName_ = "HandleCheckWindowId";
    info.bundleName_ = "HandleCheckWindowId1";
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
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
HWTEST_F(SceneSessionManagerStubTest, HandleSetGestureNavigationEnabled, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteBool(false);

    int res = stub_->HandleSetGestureNavigationEnabled(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetAccessibilityWindowInfo
 * @tc.desc: test HandleGetAccessibilityWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetAccessibilityWindowInfo, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, HandleGetUnreliableWindowInfo, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    int32_t windowId = 0;
    data.WriteInt32(windowId);

    int res = stub_->HandleGetUnreliableWindowInfo(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetSessionContinueState
 * @tc.desc: test HandleSetSessionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetSessionContinueState, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    int32_t x = 1;
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());
    data.WriteInt32(x);

    int res = stub_->HandleSetSessionContinueState(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetSessionContinueState1
 * @tc.desc: test HandleSetSessionContinueState1
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetSessionContinueState1, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    int32_t x = 0;
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
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
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionDump, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;

    std::vector<std::string> params = {"-a"};
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
HWTEST_F(SceneSessionManagerStubTest, HandleUpdateSessionAvoidAreaListener, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

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
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionSnapshot, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

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
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionSnapshotById, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }
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
HWTEST_F(SceneSessionManagerStubTest, HandleBindDialogTarget, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;

    uint64_t persistentId = 65535;
    data.WriteUint64(persistentId);
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandleBindDialogTarget(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleNotifyDumpInfoResult
 * @tc.desc: test HandleNotifyDumpInfoResult
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleNotifyDumpInfoResult, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;

    uint32_t vectorSize = 128;
    data.WriteUint32(vectorSize);
    stub_->HandleNotifyDumpInfoResult(data, reply);

    std::vector<std::string> info = {"-a", "-b123", "-c3456789", ""};
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
HWTEST_F(SceneSessionManagerStubTest, HandleUnregisterCollaborator, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;

    int32_t type = CollaboratorType::RESERVE_TYPE;
    data.WriteInt32(type);

    int res = stub_->HandleUnregisterCollaborator(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUpdateSessionTouchOutsideListener
 * @tc.desc: test HandleUpdateSessionTouchOutsideListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleUpdateSessionTouchOutsideListener, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

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
HWTEST_F(SceneSessionManagerStubTest, HandleRaiseWindowToTop, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

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
HWTEST_F(SceneSessionManagerStubTest, HandleNotifyWindowExtensionVisibilityChange, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;

    int32_t pid = 65535;
    data.WriteInt32(pid);
    int32_t uid = 12345;
    data.WriteInt32(uid);
    bool visible = true;
    data.WriteBool(visible);

    int res = stub_->HandleNotifyWindowExtensionVisibilityChange(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetTopWindowId
 * @tc.desc: test HandleGetTopWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetTopWindowId, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

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
HWTEST_F(SceneSessionManagerStubTest, HandleShiftAppWindowFocus, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

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
 * @tc.name: HandleAddExtensionWindowStageToSCB
 * @tc.desc: test HandleAddExtensionWindowStageToSCB
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleAddExtensionWindowStageToSCB, Function | SmallTest | Level2)
{
    ASSERT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStage, nullptr);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    data.WriteRemoteObject(token);
    data.WriteUint64(12345);

    int res = stub_->HandleAddExtensionWindowStageToSCB(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleRemoveExtensionWindowStageFromSCB
 * @tc.desc: test HandleRemoveExtensionWindowStageFromSCB
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleRemoveExtensionWindowStageFromSCB, Function | SmallTest | Level2)
{
    ASSERT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStage, nullptr);
    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    data.WriteRemoteObject(token);

    int res = stub_->HandleRemoveExtensionWindowStageFromSCB(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUpdateModalExtensionRect
 * @tc.desc: test HandleUpdateModalExtensionRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleUpdateModalExtensionRect, Function | SmallTest | Level2)
{
    ASSERT_NE(stub_, nullptr);

    MessageParcel data;
    MessageParcel reply;

    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);
    Rect rect { 1, 2, 3, 4 };
    data.WriteInt32(rect.posX_);
    data.WriteInt32(rect.posY_);
    data.WriteInt32(rect.width_);
    data.WriteInt32(rect.height_);

    int res = stub_->HandleUpdateModalExtensionRect(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleProcessModalExtensionPointDown
 * @tc.desc: test HandleProcessModalExtensionPointDown
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleProcessModalExtensionPointDown, Function | SmallTest | Level2)
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
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleAddOrRemoveSecureSession
 * @tc.desc: test HandleAddOrRemoveSecureSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleAddOrRemoveSecureSession, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInt32(12345);
    data.WriteBool(true);

    int res = stub_->HandleAddOrRemoveSecureSession(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetUIContentRemoteObj
 * @tc.desc: test HandleGetUIContentRemoteObj
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetUIContentRemoteObj, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(1);
    int res = stub_->HandleGetUIContentRemoteObj(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetHostWindowRect
 * @tc.desc: test HandleGetHostWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetHostWindowRect, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;

    int32_t hostWindowId = 65535;
    data.WriteInt32(hostWindowId);

    int res = stub_->HandleGetHostWindowRect(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetCallingWindowWindowStatus
 * @tc.desc: test HandleGetCallingWindowWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetCallingWindowWindowStatus, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

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
HWTEST_F(SceneSessionManagerStubTest, HandleGetCallingWindowRect, Function | SmallTest | Level2)
{
    if (stub_ == nullptr) {
        return;
    }

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
HWTEST_F(SceneSessionManagerStubTest, HandleGetSessionInfoByContinueSessionId, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteString("test_01");

    int res = stub_->HandleGetSessionInfoByContinueSessionId(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUpdateExtWindowFlags
 * @tc.desc: test HandleUpdateExtWindowFlags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleUpdateExtWindowFlags, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(token);
    data.WriteInt32(7);
    data.WriteInt32(7);

    int res = stub_->HandleUpdateExtWindowFlags(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetWindowStyleType
 * @tc.desc: test HandleGetWindowStyleType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetWindowStyleType, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    int res = stub_->HandleGetWindowStyleType(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetProcessSurfaceNodeIdByPersistentId
 * @tc.desc: test HandleGetProcessSurfaceNodeIdByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetProcessSurfaceNodeIdByPersistentId, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    int32_t pid = 123;
    std::vector<int32_t> persistentIds = {1, 2, 3};
    std::vector<uint64_t> surfaceNodeIds;
    data.WriteInterfaceToken(SceneSessionManagerStub::GetDescriptor());
    data.WriteInt32(pid);
    data.WriteInt32Vector(persistentIds);
    data.WriteUInt64Vector(surfaceNodeIds);
    int res = stub_->HandleGetProcessSurfaceNodeIdByPersistentId(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleReleaseForegroundSessionScreenLock
 * @tc.desc: test HandleReleaseForegroundSessionScreenLock
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleReleaseForegroundSessionScreenLock, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    int res = stub_->HandleReleaseForegroundSessionScreenLock(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleIsWindowRectAutoSave
 * @tc.desc: test HandleIsWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleIsWindowRectAutoSave, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    std::string key = "com.example.recposentryEntryAbility";
    data.WriteString(key);
    int res = stub_->HandleIsWindowRectAutoSave(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetDisplayIdByWindowId
 * @tc.desc: test HandleGetDisplayIdByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetDisplayIdByWindowId, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    const std::vector<uint64_t> windowIds = {1, 2};
    data.WriteUInt64Vector(windowIds);

    int res = stub_->HandleGetDisplayIdByWindowId(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleIsPcOrPadFreeMultiWindowMode
 * @tc.desc: test HandleIsPcOrPadFreeMultiWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleIsPcOrPadFreeMultiWindowMode, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    int res = stub_->HandleIsPcOrPadFreeMultiWindowMode(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleSetGlobalDragResizeType
 * @tc.desc: test HandleSetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleSetGlobalDragResizeType, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, HandleGetGlobalDragResizeType, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerStubTest, HandleSetAppDragResizeType, Function | SmallTest | Level2)
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
 * @tc.name: HandleGetAppDragResizeType
 * @tc.desc: test HandleGetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest, HandleGetAppDragResizeType, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    const std::string bundleName = "test";
    data.WriteString(bundleName);
    int res = stub_->HandleGetAppDragResizeType(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}
}
}
}