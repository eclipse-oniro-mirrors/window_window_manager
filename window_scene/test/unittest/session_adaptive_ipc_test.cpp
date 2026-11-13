/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <ipc_types.h>

#include "iremote_object_mocker.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_session_stub.h"
#include "mock/mock_accesstoken_kit.h"
#include "session/container/include/zidl/session_stage_ipc_interface_code.h"
#include "session/container/include/zidl/session_stage_proxy.h"
#include "session/container/include/zidl/session_stage_stub.h"
#include "session/host/include/zidl/session_ipc_interface_code.h"
#include "session/host/include/zidl/session_proxy.h"
#include "session/host/include/zidl/session_stub.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/scene_session_manager_lite.h"
#include "session_manager/include/zidl/scene_session_manager_interface.h"
#include "session_manager/include/zidl/scene_session_manager_proxy.h"
#include "session_manager/include/zidl/scene_session_manager_lite_interface.h"
#include "session_manager/include/zidl/scene_session_manager_lite_proxy.h"
#include "session_manager/include/zidl/scene_session_manager_lite_stub.h"
#include "session_info.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr int32_t TEST_PERSISTENT_ID = 1;
}

class SessionAdaptiveIpcTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    GameAdaptiveResolutionConfig MakeConfig(int32_t windowId, bool enable);
};

void SessionAdaptiveIpcTest::SetUpTestCase() {}

void SessionAdaptiveIpcTest::TearDownTestCase() {}

void SessionAdaptiveIpcTest::SetUp() {}

void SessionAdaptiveIpcTest::TearDown()
{
    MockAccesstokenKit::ChangeMockStateToInit();
}

GameAdaptiveResolutionConfig SessionAdaptiveIpcTest::MakeConfig(int32_t windowId, bool enable)
{
    GameAdaptiveResolutionConfig config;
    config.enable_ = enable;
    config.scaleRatio_ = 0.5f;
    config.windowId_ = windowId;
    return config;
}

namespace {
/**
 * @tc.name: SessionStubHandleUpdateAdaptiveConfig
 * @tc.desc: valid config round-trips through the stub handler
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveIpcTest, SessionStubHandleUpdateAdaptiveConfig, TestSize.Level1)
{
    auto session = sptr<SessionStubMocker>::MakeSptr();
    ASSERT_NE(session, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto config = MakeConfig(TEST_PERSISTENT_ID, true);
    ASSERT_TRUE(data.WriteParcelable(&config));
    EXPECT_EQ(session->HandleUpdateAdaptiveResolutionConfig(data, reply), ERR_NONE);
    int32_t retCode = 0;
    ASSERT_TRUE(reply.ReadInt32(retCode));
    EXPECT_EQ(static_cast<WMError>(retCode), WMError::WM_OK);
}

/**
 * @tc.name: SessionStubHandleUpdateAdaptiveConfigInvalid
 * @tc.desc: an empty parcel is rejected
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveIpcTest, SessionStubHandleUpdateAdaptiveConfigInvalid, TestSize.Level1)
{
    auto session = sptr<SessionStubMocker>::MakeSptr();
    ASSERT_NE(session, nullptr);
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(session->HandleUpdateAdaptiveResolutionConfig(data, reply), ERR_INVALID_DATA);
}

/**
 * @tc.name: SessionProxyUpdateAdaptiveConfigErrorPaths
 * @tc.desc: null remote and a reply-less mocked remote both fail with an IPC error
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveIpcTest, SessionProxyUpdateAdaptiveConfigErrorPaths, TestSize.Level1)
{
    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    ASSERT_NE(nullProxy, nullptr);
    EXPECT_EQ(nullProxy->UpdateAdaptiveResolutionConfig(MakeConfig(TEST_PERSISTENT_ID, true)),
        WMError::WM_ERROR_IPC_FAILED);

    auto mockRemote = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionProxy> proxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    ASSERT_NE(proxy, nullptr);
    EXPECT_EQ(proxy->UpdateAdaptiveResolutionConfig(MakeConfig(TEST_PERSISTENT_ID, true)),
        WMError::WM_ERROR_IPC_FAILED);
}

/**
 * @tc.name: SessionStageStubHandleNotifyAdaptiveChanged
 * @tc.desc: status and config round-trip through the stage stub handler
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveIpcTest, SessionStageStubHandleNotifyAdaptiveChanged, TestSize.Level1)
{
    auto stage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(stage, nullptr);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(static_cast<int32_t>(GameAdaptiveResolutionStatus::SET));
    auto config = MakeConfig(TEST_PERSISTENT_ID, true);
    ASSERT_TRUE(data.WriteParcelable(&config));
    EXPECT_EQ(stage->HandleNotifyAdaptiveResolutionChanged(data, reply), ERR_NONE);
}

/**
 * @tc.name: SessionStageStubHandleNotifyAdaptiveChangedInvalid
 * @tc.desc: missing status or config is rejected
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveIpcTest, SessionStageStubHandleNotifyAdaptiveChangedInvalid, TestSize.Level1)
{
    auto stage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(stage, nullptr);
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(stage->HandleNotifyAdaptiveResolutionChanged(data, reply), ERR_INVALID_DATA);
    data.WriteInt32(static_cast<int32_t>(GameAdaptiveResolutionStatus::CANCELLED));
    EXPECT_EQ(stage->HandleNotifyAdaptiveResolutionChanged(data, reply), ERR_INVALID_DATA);
}

/**
 * @tc.name: SsmStubHandleSetGameAdaptiveInvalid
 * @tc.desc: an empty parcel is rejected before any state change
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveIpcTest, SsmStubHandleSetGameAdaptiveInvalid, TestSize.Level1)
{
    auto ssm = sptr<SceneSessionManager>::MakeSptr();
    ASSERT_NE(ssm, nullptr);
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(ssm->HandleSetGameAdaptiveResolutionForWindow(data, reply), ERR_INVALID_DATA);
}

/**
 * @tc.name: SsmStubHandleSetGameAdaptivePermissionDenied
 * @tc.desc: non-SA IPC caller gets NOT_SYSTEM_APP in the reply
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveIpcTest, SsmStubHandleSetGameAdaptivePermissionDenied, TestSize.Level1)
{
    auto ssm = sptr<SceneSessionManager>::MakeSptr();
    ASSERT_NE(ssm, nullptr);
    MockAccesstokenKit::MockIsSACalling(false);
    MessageParcel data;
    MessageParcel reply;
    auto config = MakeConfig(TEST_PERSISTENT_ID, true);
    ASSERT_TRUE(data.WriteParcelable(&config));
    EXPECT_EQ(ssm->HandleSetGameAdaptiveResolutionForWindow(data, reply), ERR_NONE);
    int32_t retCode = 0;
    ASSERT_TRUE(reply.ReadInt32(retCode));
    EXPECT_EQ(static_cast<WMError>(retCode), WMError::WM_ERROR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: SsmStubHandleSetGameAdaptiveSessionNotFound
 * @tc.desc: SA caller with a missing target window gets INVALID_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveIpcTest, SsmStubHandleSetGameAdaptiveSessionNotFound, TestSize.Level1)
{
    auto ssm = sptr<SceneSessionManager>::MakeSptr();
    ASSERT_NE(ssm, nullptr);
    MockAccesstokenKit::MockIsSACalling(true);
    MessageParcel data;
    MessageParcel reply;
    auto config = MakeConfig(TEST_PERSISTENT_ID, true);
    ASSERT_TRUE(data.WriteParcelable(&config));
    EXPECT_EQ(ssm->HandleSetGameAdaptiveResolutionForWindow(data, reply), ERR_NONE);
    int32_t retCode = 0;
    ASSERT_TRUE(reply.ReadInt32(retCode));
    EXPECT_EQ(static_cast<WMError>(retCode), WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: SsmProxySetGameAdaptiveErrorPaths
 * @tc.desc: null remote, send failure, empty reply and the server code passthrough
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveIpcTest, SsmProxySetGameAdaptiveErrorPaths, TestSize.Level1)
{
    // null remote
    sptr<SceneSessionManagerProxy> nullProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    ASSERT_NE(nullProxy, nullptr);
    EXPECT_EQ(nullProxy->SetGameAdaptiveResolutionForWindow(MakeConfig(TEST_PERSISTENT_ID, true)),
        WMError::WM_ERROR_IPC_FAILED);

    // writes pass; the send fails
    auto failingRemote = sptr<MockIRemoteObject>::MakeSptr();
    failingRemote->SetRequestResult(ERR_INVALID_DATA);
    sptr<SceneSessionManagerProxy> failingProxy = sptr<SceneSessionManagerProxy>::MakeSptr(failingRemote);
    ASSERT_NE(failingProxy, nullptr);
    EXPECT_EQ(failingProxy->SetGameAdaptiveResolutionForWindow(MakeConfig(TEST_PERSISTENT_ID, true)),
        WMError::WM_ERROR_IPC_FAILED);

    // writes and the send pass; the empty reply fails the read
    auto mockRemote = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> proxy = sptr<SceneSessionManagerProxy>::MakeSptr(mockRemote);
    ASSERT_NE(proxy, nullptr);
    EXPECT_EQ(proxy->SetGameAdaptiveResolutionForWindow(MakeConfig(TEST_PERSISTENT_ID, true)),
        WMError::WM_ERROR_IPC_FAILED);

    // a replying remote passes the server code through
    auto replyingRemote = sptr<RemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> replyingProxy = sptr<SceneSessionManagerProxy>::MakeSptr(replyingRemote);
    ASSERT_NE(replyingProxy, nullptr);
    EXPECT_CALL(*replyingRemote, SendRequest(_, _, _, _)).WillOnce(
        [](uint32_t, MessageParcel&, MessageParcel& reply, MessageOption&) {
            reply.WriteInt32(static_cast<int32_t>(WMError::WM_ERROR_NOT_SYSTEM_APP));
            return ERR_NONE;
        });
    EXPECT_EQ(replyingProxy->SetGameAdaptiveResolutionForWindow(MakeConfig(TEST_PERSISTENT_ID, true)),
        WMError::WM_ERROR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: SessionStageProxyNotifyAdaptiveChanged
 * @tc.desc: null remote fails; a mocked remote completes the async notify
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveIpcTest, SessionStageProxyNotifyAdaptiveChanged, TestSize.Level1)
{
    sptr<SessionStageProxy> nullProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    ASSERT_NE(nullProxy, nullptr);
    EXPECT_EQ(nullProxy->NotifyAdaptiveResolutionChanged(GameAdaptiveResolutionStatus::SET,
        MakeConfig(TEST_PERSISTENT_ID, true)), WSError::WS_ERROR_IPC_FAILED);

    auto mockRemote = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionStageProxy> proxy = sptr<SessionStageProxy>::MakeSptr(mockRemote);
    ASSERT_NE(proxy, nullptr);
    EXPECT_EQ(proxy->NotifyAdaptiveResolutionChanged(GameAdaptiveResolutionStatus::SET,
        MakeConfig(TEST_PERSISTENT_ID, true)), WSError::WS_OK);
}

/**
 * @tc.name: LiteStubAndProxySetGameAdaptive
 * @tc.desc: the lite stub keeps the SA gate in the reply; the lite proxy reports remote errors
 * @tc.type: FUNC
 */
HWTEST_F(SessionAdaptiveIpcTest, LiteStubAndProxySetGameAdaptive, TestSize.Level1)
{
    // stub: a non-SA parcel gets NOT_SYSTEM_APP in the reply
    MockAccesstokenKit::MockIsSACalling(false);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    ASSERT_TRUE(data.WriteInterfaceToken(SceneSessionManagerLiteStub::GetDescriptor()));
    auto config = MakeConfig(TEST_PERSISTENT_ID, true);
    ASSERT_TRUE(data.WriteParcelable(&config));
    uint32_t liteCode = static_cast<uint32_t>(
        ISceneSessionManagerLite::SceneSessionManagerLiteMessage::TRANS_ID_SET_GAME_ADAPTIVE_RESOLUTION_FOR_WINDOW);
    EXPECT_EQ(SceneSessionManagerLite::GetInstance().OnRemoteRequest(liteCode, data, reply, option), ERR_NONE);
    int32_t retCode = 0;
    ASSERT_TRUE(reply.ReadInt32(retCode));
    EXPECT_EQ(static_cast<WMError>(retCode), WMError::WM_ERROR_NOT_SYSTEM_APP);

    // proxy: null remote and a reply-less remote both fail
    sptr<SceneSessionManagerLiteProxy> nullProxy = sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    ASSERT_NE(nullProxy, nullptr);
    EXPECT_EQ(nullProxy->SetGameAdaptiveResolutionForWindow(MakeConfig(TEST_PERSISTENT_ID, true)),
        WMError::WM_ERROR_NULLPTR);
    auto mockRemote = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerLiteProxy> proxy = sptr<SceneSessionManagerLiteProxy>::MakeSptr(mockRemote);
    ASSERT_NE(proxy, nullptr);
    EXPECT_EQ(proxy->SetGameAdaptiveResolutionForWindow(MakeConfig(TEST_PERSISTENT_ID, true)),
        WMError::WM_ERROR_IPC_FAILED);
}
}
} // namespace Rosen
} // namespace OHOS
