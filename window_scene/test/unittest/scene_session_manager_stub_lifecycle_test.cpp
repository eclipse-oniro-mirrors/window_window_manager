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
class SceneSessionManagerStubLifecycleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<SceneSessionManagerStub> stub_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void SceneSessionManagerStubLifecycleTest::SetUpTestCase() {}

void SceneSessionManagerStubLifecycleTest::TearDownTestCase() {}

void SceneSessionManagerStubLifecycleTest::SetUp()
{
    stub_ = sptr<SceneSessionManager>::MakeSptr();
}

void SceneSessionManagerStubLifecycleTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: HandleRecoverAndReconnectSceneSession
 * @tc.desc: test HandleRecoverAndReconnectSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleRecoverAndReconnectSceneSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionStage);
    int res = stub_->HandleRecoverAndReconnectSceneSession(data, reply);
    ASSERT_EQ(res, ERR_INVALID_DATA);

    data.WriteRemoteObject(sessionStage->AsObject());
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannel>::MakeSptr(sessionStage);
    ASSERT_NE(nullptr, eventChannel);
    data.WriteRemoteObject(eventChannel->AsObject());
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    surfaceNode->Marshalling(data);
    data.WriteBool(false);
    res = stub_->HandleRecoverAndReconnectSceneSession(data, reply);
    ASSERT_EQ(res, ERR_INVALID_STATE);

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
    res = stub_->HandleRecoverAndReconnectSceneSession(data, reply);
    EXPECT_EQ(res, ERR_INVALID_STATE);
}

/**
 * @tc.name: HandlePendingSessionToForeground_Fail
 * @tc.desc: test function : HandlePendingSessionToForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandlePendingSessionToForeground_Fail, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    data.WriteRemoteObject(token);
    int res = stub_->HandlePendingSessionToForeground(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandlePendingSessionToForeground_Success
 * @tc.desc: test function : HandlePendingSessionToForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandlePendingSessionToForeground_Success, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    int32_t windowMode = static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN);
    data.WriteRemoteObject(token);
    data.WriteInt32(windowMode);
    int res = stub_->HandlePendingSessionToForeground(data, reply);
    EXPECT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandlePendingSessionToBackgroundForDelegator
 * @tc.desc: test HandlePendingSessionToBackgroundForDelegator
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandlePendingSessionToBackgroundForDelegator, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    int res = stub_->HandlePendingSessionToBackgroundForDelegator(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleTerminateSessionNew
 * @tc.desc: test HandleTerminateSessionNew
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleTerminateSessionNew, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteBool(false);

    int res = stub_->HandleTerminateSessionNew(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleSetSessionContinueState
 * @tc.desc: test HandleSetSessionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleSetSessionContinueState, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    int32_t x = 1;
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());
    data.WriteInt32(x);

    int res = stub_->HandleSetSessionContinueState(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleClearSession
 * @tc.desc: test HandleClearSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleClearSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->HandleClearSession(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    int32_t persistentId = 65535;
    data.WriteInt32(persistentId);

    res = stub_->HandleClearSession(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleClearAllSessions
 * @tc.desc: test HandleClearAllSessions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleClearAllSessions, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleClearAllSessions(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleMinimizeAllAppWindows
 * @tc.desc: test HandleMinimizeAllAppWindows
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleMinimizeAllAppWindows, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    EXPECT_EQ(stub_->HandleMinimizeAllAppWindows(data, reply), ERR_INVALID_DATA);

    data.WriteUint64(0);
    EXPECT_EQ(stub_->HandleMinimizeAllAppWindows(data, reply), ERR_NONE);

    data.WriteUint64(0);
    data.WriteBool(true);
    EXPECT_EQ(stub_->HandleMinimizeAllAppWindows(data, reply), ERR_NONE);

    data.WriteUint64(0);
    data.WriteInt32(0);
    EXPECT_EQ(stub_->HandleMinimizeAllAppWindows(data, reply), ERR_NONE);
}

/**
 * @tc.name: HandleLockSession
 * @tc.desc: test HandleLockSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleLockSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->HandleLockSession(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
    int32_t sessionId = 65535;
    data.WriteInt32(sessionId);
    res = stub_->HandleLockSession(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUnlockSession
 * @tc.desc: test HandleUnlockSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleUnlockSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->HandleUnlockSession(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
    int32_t sessionId = 65535;
    data.WriteInt32(sessionId);
    res = stub_->HandleUnlockSession(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleMoveSessionsToForeground
 * @tc.desc: test HandleMoveSessionsToForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleMoveSessionsToForeground, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->HandleMoveSessionsToForeground(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    std::vector<int32_t> sessionIds = { 1, 2, 3, 15, 1423 };
    data.WriteInt32Vector(sessionIds);
    int32_t topSessionId = 1;
    data.WriteInt32(topSessionId);
    res = stub_->HandleMoveSessionsToForeground(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleMoveSessionsToBackground
 * @tc.desc: test HandleMoveSessionsToBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleMoveSessionsToBackground, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;

    std::vector<int32_t> sessionIds = { 1, 2, 3, 15, 1423 };
    data.WriteInt32Vector(sessionIds);
    std::vector<int32_t> result = { 1, 2, 3, 15, 1423 };
    data.WriteInt32Vector(result);

    int res = stub_->HandleMoveSessionsToBackground(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleUpdateSessionWindowVisibilityListener
 * @tc.desc: test HandleUpdateSessionWindowVisibilityListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleUpdateSessionWindowVisibilityListener, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteInt32(0);
    data.WriteBool(true);

    int res = stub_->HandleUpdateSessionWindowVisibilityListener(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetVisibilityWindowInfo
 * @tc.desc: test HandleGetVisibilityWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleGetVisibilityWindowInfo, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;

    int res = stub_->HandleGetVisibilityWindowInfo(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleGetAllMainWindowInfo
 * @tc.desc: test HandleGetAllMainWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, GetAllMainWindowInfo, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;
 
    int res = stub_->HandleGetAllMainWindowInfo(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}
 
/**
 * @tc.name: HandleGetMainWindowSnapshot
 * @tc.desc: test HandleGetMainWindowSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubLifecycleTest, HandleGetMainWindowSnapshot, TestSize.Level1)
{
    ASSERT_NE(nullptr, stub_);
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    EXPECT_EQ(stub_->HandleGetMainWindowSnapshot(data, reply), ERR_INVALID_DATA);
 
    std::vector<int32_t> windowIds;
    windowIds.emplace_back(1);
    windowIds.emplace_back(2);
    data.WriteInt32Vector(windowIds);
    EXPECT_EQ(stub_->HandleGetMainWindowSnapshot(data, reply), ERR_INVALID_DATA);
 
    data.WriteInt32Vector(windowIds);
    data.WriteBool(true);
    EXPECT_EQ(stub_->HandleGetMainWindowSnapshot(data, reply), ERR_INVALID_DATA);
 
    data.WriteInt32Vector(windowIds);
    data.WriteBool(true);
    data.WriteRemoteObject(stub_->AsObject());
    EXPECT_EQ(stub_->HandleGetMainWindowSnapshot(data, reply), ERR_NONE);
}
} // namespace
} // namespace Rosen
} // namespace OHOS