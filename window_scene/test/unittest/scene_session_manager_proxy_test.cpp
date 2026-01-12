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
#include "iremote_object_mocker.h"
#include "mock/mock_message_parcel.h"
#include "mock/mock_session.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "mock/mock_ui_effect_controller_client_stub.h"
#include "mock/mock_ui_effect_controller_stub.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/zidl/scene_session_manager_interface.h"
#include "session_manager/include/zidl/scene_session_manager_proxy.h"
#include "window_manager_agent.h"
#include "zidl/window_manager_agent_interface.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class sceneSessionManagerProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<IRemoteObject> iRemoteObjectMocker;
};

void sceneSessionManagerProxyTest::SetUpTestCase() {}

void sceneSessionManagerProxyTest::TearDownTestCase() {}

void sceneSessionManagerProxyTest::SetUp() {}

void sceneSessionManagerProxyTest::TearDown() {}

namespace {
/**
 * @tc.name: CreateAndConnectSpecificSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, CreateAndConnectSpecificSession, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    node->SetRSUIContext(rsUIContext);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    int32_t persistentId = 0;
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    sceneSessionManagerProxy->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, persistentId, session, systemConfig, token);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: CreateAndConnectSpecificSession2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, CreateAndConnectSpecificSession2, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    int32_t persistentId = 0;
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    sceneSessionManagerProxy->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, persistentId, session, systemConfig, token);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: CreateAndConnectSpecificSession3
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, CreateAndConnectSpecificSession3, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<WindowSessionProperty> property = nullptr;
    int32_t persistentId = 0;
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    sceneSessionManagerProxy->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, persistentId, session, systemConfig, token);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: CreateAndConnectSpecificSession4
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, CreateAndConnectSpecificSession4, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    node->SetRSUIContext(rsUIContext);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    int32_t persistentId = 0;
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token = nullptr;

    sceneSessionManagerProxy->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, persistentId, session, systemConfig, token);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: RecoverAndConnectSpecificSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverAndConnectSpecificSession, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    node->SetRSUIContext(rsUIContext);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    auto ret = sceneSessionManagerProxy->RecoverAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, session, token);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: RecoverAndConnectSpecificSession2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverAndConnectSpecificSession2, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    node->SetRSUIContext(rsUIContext);
    sptr<WindowSessionProperty> property = nullptr;
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    auto ret = sceneSessionManagerProxy->RecoverAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, session, token);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: RecoverAndConnectSpecificSession3
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverAndConnectSpecificSession3, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    node->SetRSUIContext(rsUIContext);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    sptr<IRemoteObject> token = nullptr;

    auto ret = sceneSessionManagerProxy->RecoverAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, session, token);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverAndReconnectSceneSession, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    node->SetRSUIContext(rsUIContext);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    auto ret = sceneSessionManagerProxy->RecoverAndReconnectSceneSession(
        sessionStage, eventChannel, node, session, property, token);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverAndReconnectSceneSession2, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    node->SetRSUIContext(rsUIContext);
    sptr<WindowSessionProperty> property = nullptr;
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    auto ret = sceneSessionManagerProxy->RecoverAndReconnectSceneSession(
        sessionStage, eventChannel, node, session, property, token);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession3
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverAndReconnectSceneSession3, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    auto rsUIContext = ScreenSessionManagerClient::GetInstance().GetRSUIContext(0);
    node->SetRSUIContext(rsUIContext);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    sptr<IRemoteObject> token = nullptr;

    auto ret = sceneSessionManagerProxy->RecoverAndReconnectSceneSession(
        sessionStage, eventChannel, node, session, property, token);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, DestroyAndDisconnectSpecificSession, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    sceneSessionManagerProxy->DestroyAndDisconnectSpecificSession(0);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionWithDetachCallback
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, DestroyAndDisconnectSpecificSessionWithDetachCallback, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    sptr<IRemoteObject> callback = sptr<IRemoteObjectMocker>::MakeSptr();

    sceneSessionManagerProxy->DestroyAndDisconnectSpecificSessionWithDetachCallback(0, callback);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: UpdateSessionTouchOutsideListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UpdateSessionTouchOutsideListener, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    int32_t persistentId = 0;
    sceneSessionManagerProxy->UpdateSessionTouchOutsideListener(persistentId, true);
    sceneSessionManagerProxy->UpdateSessionTouchOutsideListener(persistentId, false);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: SkipSnapshotForAppProcess
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SkipSnapshotForAppProcess, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t pid = 1000;
    bool skip = false;
    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->SkipSnapshotForAppProcess(pid, skip));
}

/**
 * @tc.name: SetGestureNavigationEnabled
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetGestureNavigationEnabled, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    sceneSessionManagerProxy->SetGestureNavigationEnabled(true);
    sceneSessionManagerProxy->SetGestureNavigationEnabled(false);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: GetFocusWindowInfo
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetFocusWindowInfo, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    FocusChangeInfo focusInfo{};
    sceneSessionManagerProxy->GetFocusWindowInfo(focusInfo);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: GetFocusWindowInfoByAbilityToken
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetFocusWindowInfoByAbilityToken, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneSessionManagerProxy> proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(sceneSessionManagerProxy, nullptr);

    FocusChangeInfo focusInfo{};
    sptr<IRemoteObject> token = nullptr;
    proxy->GetFocusWindowInfoByAbilityToken(focusInfo, token);
    EXPECT_EQ(focusInfo.windowId_, INVALID_WINDOW_ID);
    
    token = sptr<IRemoteObjectMocker>::MakeSptr();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    proxy->GetFocusWindowInfoByAbilityToken(focusInfo, token);
    EXPECT_EQ(focusInfo.windowId_, INVALID_WINDOW_ID);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteRemoteObjectErrorFlag(true);
    proxy->GetFocusWindowInfoByAbilityToken(focusInfo, token);
    EXPECT_EQ(focusInfo.windowId_, INVALID_WINDOW_ID);
    MockMessageParcel::SetWriteRemoteObjectErrorFlag(false);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    proxy->GetFocusWindowInfoByAbilityToken(focusInfo, token);
    EXPECT_EQ(focusInfo.windowId_, INVALID_WINDOW_ID);
    remoteMocker->SetRequestResult(ERR_NONE);

    proxy->GetFocusWindowInfoByAbilityToken(focusInfo, token);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: GetFocusWindowInfoByAbilityToken_NullRemote
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetFocusWindowInfoByAbilityToken_NullRemote, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = nullptr;
    sptr<SceneSessionManagerProxy> proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(sceneSessionManagerProxy, nullptr);

    FocusChangeInfo focusInfo{};
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    proxy->GetFocusWindowInfoByAbilityToken(focusInfo, token);
    EXPECT_EQ(focusInfo.windowId_, INVALID_WINDOW_ID);
}

/**
 * @tc.name: SetSessionIcon
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetSessionIcon, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    std::shared_ptr<Media::PixelMap> icon = nullptr;
    sceneSessionManagerProxy->SetSessionIcon(token, icon);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: GetAccessibilityWindowInfo
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetAccessibilityWindowInfo, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    std::vector<sptr<AccessibilityWindowInfo>> infos{};
    sceneSessionManagerProxy->GetAccessibilityWindowInfo(infos);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: GetUnreliableWindowInfo
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetUnreliableWindowInfo, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    std::vector<sptr<UnreliableWindowInfo>> infos{};
    sceneSessionManagerProxy->GetUnreliableWindowInfo(0, infos);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: GetSessionInfos
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetSessionInfos, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    std::string deviceId;
    std::vector<SessionInfoBean> sessionInfos{};
    sceneSessionManagerProxy->GetSessionInfos(deviceId, 0, sessionInfos);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: GetSessionInfo
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetSessionInfo, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    std::string deviceId;
    SessionInfoBean sessionInfo;
    sceneSessionManagerProxy->GetSessionInfo(deviceId, 0, sessionInfo);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: RegisterWindowManagerAgent01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RegisterWindowManagerAgent01, TestSize.Level1)
{
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->RegisterWindowManagerAgent(type, windowManagerAgent));
    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->UnregisterWindowManagerAgent(type, windowManagerAgent));
}

/**
 * @tc.name: RegisterWindowPropertyChangeAgent01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RegisterWindowPropertyChangeAgent01, TestSize.Level1)
{
    WindowInfoKey windowInfoKey = WindowInfoKey::DISPLAY_ID;
    uint32_t interestInfo = 0;
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    EXPECT_EQ(WMError::WM_OK,
        sceneSessionManagerProxy->RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent));
    EXPECT_EQ(WMError::WM_OK,
        sceneSessionManagerProxy->UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent));
}

/**
 * @tc.name: UpdateModalExtensionRect
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UpdateModalExtensionRect, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sceneSessionManagerProxy, nullptr);

    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    Rect rect{ 1, 2, 3, 4 };
    sceneSessionManagerProxy->UpdateModalExtensionRect(token, rect);
    sceneSessionManagerProxy->UpdateModalExtensionRect(nullptr, rect);
}

/**
 * @tc.name: ProcessModalExtensionPointDown
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, ProcessModalExtensionPointDown, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sceneSessionManagerProxy, nullptr);

    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    sceneSessionManagerProxy->ProcessModalExtensionPointDown(token, 0, 0);
    sceneSessionManagerProxy->ProcessModalExtensionPointDown(nullptr, 0, 0);
}

/**
 * @tc.name: AddExtensionWindowStageToSCB
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, AddExtensionWindowStageToSCB, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sceneSessionManagerProxy, nullptr);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStage, nullptr);
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    sceneSessionManagerProxy->AddExtensionWindowStageToSCB(sessionStage, token, 12345, -1);
}

/**
 * @tc.name: SetStartWindowBackgroundColor
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetStartWindowBackgroundColor, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sceneSessionManagerProxy, nullptr);

    EXPECT_EQ(WMError::WM_OK,
        sceneSessionManagerProxy->SetStartWindowBackgroundColor("moduleName", "abilityName", 0xffffffff, 100));
}

/**
 * @tc.name: RemoveExtensionWindowStageFromSCB
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RemoveExtensionWindowStageFromSCB, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sceneSessionManagerProxy, nullptr);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStage, nullptr);
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    sceneSessionManagerProxy->RemoveExtensionWindowStageFromSCB(sessionStage, token);
}

/**
 * @tc.name: AddOrRemoveSecureSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, AddOrRemoveSecureSession001, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t persistentId = 12345;
    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->AddOrRemoveSecureSession(persistentId, true));
}

/**
 * @tc.name: UpdateExtWindowFlags
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UpdateExtWindowFlags, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->UpdateExtWindowFlags(token, 7, 7));
    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->UpdateExtWindowFlags(nullptr, 7, 7));
}

/**
 * @tc.name: GetSessionInfoByContinueSessionId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetSessionInfoByContinueSessionId, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    std::string continueSessionId = "test_01";
    SessionInfoBean missionInfo;
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED,
              sceneSessionManagerProxy->GetSessionInfoByContinueSessionId(continueSessionId, missionInfo));
}

/**
 * @tc.name: RequestFocusStatus01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RequestFocusStatus01, TestSize.Level1)
{
    int32_t persistendId = 0;
    bool isFocused = true;
    bool byForeground = true;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->RequestFocusStatus(persistendId, isFocused, byForeground));
}

/**
 * @tc.name: RequestFocusStatusBySA01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RequestFocusStatusBySA01, TestSize.Level1)
{
    FocusChangeReason reason = FocusChangeReason::CLICK;
    int32_t persistendId = 0;
    bool isFocused = true;
    bool byForeground = true;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    auto result = sceneSessionManagerProxy->SceneSessionManagerProxy::RequestFocusStatusBySA(
        persistendId, isFocused, byForeground, reason);
    ASSERT_EQ(WMError::WM_OK, result);
}

/**
 * @tc.name: RaiseWindowToTop
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RaiseWindowToTop, TestSize.Level1)
{
    int32_t persistendId = 0;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->RaiseWindowToTop(persistendId));
}

/**
 * @tc.name: BindDialogSessionTarget
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, BindDialogSessionTarget, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    sptr<IRemoteObject> targetToken = sptr<IRemoteObjectMocker>::MakeSptr();
    sceneSessionManagerProxy->BindDialogSessionTarget(0, targetToken);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: BindDialogSessionTarget2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, BindDialogSessionTarget2, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    sptr<IRemoteObject> targetToken = nullptr;
    sceneSessionManagerProxy->BindDialogSessionTarget(0, targetToken);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: UpdateSessionAvoidAreaListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UpdateSessionAvoidAreaListener, TestSize.Level1)
{
    int32_t persistendId = 0;
    bool haveListener = true;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->UpdateSessionAvoidAreaListener(persistendId, haveListener));
}

/**
 * @tc.name: NotifyScreenshotEvent
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, NotifyScreenshotEvent, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    ScreenshotEventType type = ScreenshotEventType::SCROLL_SHOT_START;
    auto ssmProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    auto ret = ssmProxy->NotifyScreenshotEvent(type);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);

    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ssmProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(nullptr, ssmProxy);
    ret = ssmProxy->NotifyScreenshotEvent(type);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);

    MockMessageParcel::SetReadInt32ErrorFlag(true);
    ret = ssmProxy->NotifyScreenshotEvent(type);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = ssmProxy->NotifyScreenshotEvent(type);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = ssmProxy->NotifyScreenshotEvent(type);
    EXPECT_EQ(WMError::WM_OK, ret);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: SetSessionLabel
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetSessionLabel, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    std::string label = "SetSessionLabel";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->SetSessionLabel(token, label));
}

/**
 * @tc.name: IsValidSessionIds
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, IsValidSessionIds, TestSize.Level1)
{
    std::vector<int32_t> sessionIds;
    std::vector<bool> results;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->IsValidSessionIds(sessionIds, results));
}

/**
 * @tc.name: RegisterSessionListener01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RegisterSessionListener01, TestSize.Level1)
{
    sptr<ISessionListener> listener = nullptr;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_ERROR_INVALID_PARAM, sceneSessionManagerProxy->RegisterSessionListener(listener));
}

/**
 * @tc.name: UnRegisterSessionListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UnRegisterSessionListener, TestSize.Level1)
{
    sptr<ISessionListener> listener = nullptr;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_ERROR_INVALID_PARAM, sceneSessionManagerProxy->UnRegisterSessionListener(listener));
}

/**
 * @tc.name: GetSnapshotByWindowId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetSnapshotByWindowId, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    int32_t windowId = -1;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    WMError ret = sceneSessionManagerProxy->GetSnapshotByWindowId(windowId, pixelMap);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
}

/**
 * @tc.name: GetSessionSnapshotById
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetSessionSnapshotById, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    int32_t windowId = -1;
    SessionSnapshot snapshot;
    WMError ret = sceneSessionManagerProxy->GetSessionSnapshotById(windowId, snapshot);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
}

/**
 * @tc.name: TerminateSessionNew
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, TerminateSessionNew, TestSize.Level1)
{
    sptr<ISessionListener> listener = nullptr;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, sceneSessionManagerProxy->TerminateSessionNew(nullptr, true, true));
}

/**
 * @tc.name: LockSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, LockSession, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->LockSession(0));
}

/**
 * @tc.name: GetUIContentRemoteObj
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetUIContentRemoteObj, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    sptr<IRemoteObject> remoteObj;
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sceneSessionManagerProxy->GetUIContentRemoteObj(1, remoteObj));
}

/**
 * @tc.name: GetRootUIContentRemoteObj
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetRootUIContentRemoteObj, TestSize.Level1)
{
    DisplayId displayId = 1000;
    sptr<IRemoteObject> uiContentRemoteObj = nullptr;
    auto tempProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    auto ret = tempProxy->GetRootUIContentRemoteObj(displayId, uiContentRemoteObj);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->GetRootUIContentRemoteObj(displayId, uiContentRemoteObj);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    ret = proxy->GetRootUIContentRemoteObj(displayId, uiContentRemoteObj);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint64ErrorFlag(false);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->GetRootUIContentRemoteObj(displayId, uiContentRemoteObj);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_NONE);
    ret = proxy->GetRootUIContentRemoteObj(displayId, uiContentRemoteObj);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: ClearSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, ClearSession, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->LockSession(0));
}

/**
 * @tc.name: RegisterIAbilityManagerCollaborator
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RegisterIAbilityManagerCollaborator, TestSize.Level1)
{
    sptr<AAFwk::IAbilityManagerCollaborator> impl = nullptr;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_ERROR_INVALID_PARAM, sceneSessionManagerProxy->RegisterIAbilityManagerCollaborator(0, impl));
}

/**
 * @tc.name: NotifyWindowExtensionVisibilityChange
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, NotifyWindowExtensionVisibilityChange, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->NotifyWindowExtensionVisibilityChange(0, 0, true));
}

/**
 * @tc.name: GetTopWindowId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetTopWindowId, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    uint32_t topWinId = 1;
    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->GetTopWindowId(0, topWinId));
}

/**
 * @tc.name: GetWindowStyleType
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetWindowStyleType, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    WindowStyleType styleType;
    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->GetWindowStyleType(styleType));
}

/**
 * @tc.name: SetProcessWatermark
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetProcessWatermark, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t pid = 1000;
    const std::string watermarkName = "SetProcessWatermarkName";
    bool isEnabled = false;
    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->SetProcessWatermark(pid, watermarkName, isEnabled));
}

/**
 * @tc.name: SetWatermarkImageForApp01
 * @tc.desc: set watermark image for app
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetWatermarkImageForApp01, TestSize.Level1)
{
    std::string watermarkName;
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    auto tempProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    auto ret = tempProxy->SetWatermarkImageForApp(pixelMap, watermarkName);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->SetWatermarkImageForApp(pixelMap, watermarkName);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->SetWatermarkImageForApp(pixelMap, watermarkName);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_NONE);
    ret = proxy->SetWatermarkImageForApp(pixelMap, watermarkName);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: RecoverWatermarkImageForApp01
 * @tc.desc: recover watermark image for app
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverWatermarkImageForApp01, TestSize.Level1)
{
    std::string watermarkName;
    auto tempProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    auto ret = tempProxy->RecoverWatermarkImageForApp(watermarkName);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->RecoverWatermarkImageForApp(watermarkName);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->RecoverWatermarkImageForApp(watermarkName);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_NONE);
    ret = proxy->RecoverWatermarkImageForApp(watermarkName);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: UpdateSessionOcclusionStateListener01
 * @tc.desc: recover watermark image for app
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UpdateSessionOcclusionStateListener01, TestSize.Level1)
{
    auto tempProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    auto ret = tempProxy->UpdateSessionOcclusionStateListener(1, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->UpdateSessionOcclusionStateListener(1, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = proxy->UpdateSessionOcclusionStateListener(1, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    MockMessageParcel::SetWriteBoolErrorFlag(true);
    ret = proxy->UpdateSessionOcclusionStateListener(1, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteBoolErrorFlag(false);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->UpdateSessionOcclusionStateListener(1, true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    ret = proxy->UpdateSessionOcclusionStateListener(1, true);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: GetWindowIdsByCoordinate
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetWindowIdsByCoordinate, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int64_t displayId = 1000;
    int32_t windowNumber = 2;
    int32_t x = 0;
    int32_t y = 0;
    std::vector<int32_t> windowIds;
    WMError res = sceneSessionManagerProxy->GetWindowIdsByCoordinate(displayId, windowNumber, x, y, windowIds);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: GetProcessSurfaceNodeIdByPersistentId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetProcessSurfaceNodeIdByPersistentId, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t pid = 123;
    std::vector<int32_t> persistentIds = { 1, 2, 3 };
    std::vector<uint64_t> surfaceNodeIds;
    ASSERT_EQ(WMError::WM_OK,
              sceneSessionManagerProxy->GetProcessSurfaceNodeIdByPersistentId(pid, persistentIds, surfaceNodeIds));
}

/**
 * @tc.name: SkipSnapshotByUserIdAndBundleNames
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SkipSnapshotByUserIdAndBundleNames, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t userId = 1;
    std::vector<std::string> bundleNameList = { "a", "b", "c" };
    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->SkipSnapshotByUserIdAndBundleNames(userId, bundleNameList));
}

/**
 * @tc.name: UpdateScreenLockStatusForApp
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UpdateScreenLockStatusForApp, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
    ASSERT_EQ(sceneSessionManagerProxy->UpdateScreenLockStatusForApp("", true), WMError::WM_OK);
}

/**
 * @tc.name: AddSkipSelfWhenShowOnVirtualScreenList
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, AddSkipSelfWhenShowOnVirtualScreenList, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
    std::vector<int32_t> persistentIds{ 0, 1, 2 };
    ASSERT_EQ(sceneSessionManagerProxy->AddSkipSelfWhenShowOnVirtualScreenList(persistentIds), WMError::WM_OK);
}

/**
 * @tc.name: RemoveSkipSelfWhenShowOnVirtualScreenList
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RemoveSkipSelfWhenShowOnVirtualScreenList, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
    std::vector<int32_t> persistentIds{ 0, 1, 2 };
    ASSERT_EQ(sceneSessionManagerProxy->RemoveSkipSelfWhenShowOnVirtualScreenList(persistentIds), WMError::WM_OK);
}

/**
 * @tc.name: IsPcWindow
 * @tc.desc: IsPcWindow
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, IsPcWindow, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    bool isPcWindow = false;
    ASSERT_EQ(sceneSessionManagerProxy->IsPcWindow(isPcWindow), WMError::WM_OK);
}

/**
 * @tc.name: IsFreeMultiWindow
 * @tc.desc: IsFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, IsFreeMultiWindow, TestSize.Level1)
{
    auto tempProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    bool isFreeMultiWindow = false;

    // remote == nullptr
    auto ret = tempProxy->IsFreeMultiWindow(isFreeMultiWindow);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    // WriteInterfaceToken failed
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    ret = proxy->IsFreeMultiWindow(isFreeMultiWindow);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // SendRequest failed
    ASSERT_NE(proxy, nullptr);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->IsFreeMultiWindow(isFreeMultiWindow);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    // ReadInt32 failed
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    ret = proxy->IsFreeMultiWindow(isFreeMultiWindow);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadInt32ErrorFlag(false);

    // ReadBool failed
    MockMessageParcel::SetReadBoolErrorFlag(true);
    ret = proxy->IsFreeMultiWindow(isFreeMultiWindow);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadBoolErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();

    // interface success
    ret = proxy->IsFreeMultiWindow(isFreeMultiWindow);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: IsPcOrPadFreeMultiWindowMode
 * @tc.desc: IsPcOrPadFreeMultiWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, IsPcOrPadFreeMultiWindowMode, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    bool isPcOrPadFreeMultiWindowMode = false;
    ASSERT_EQ(sceneSessionManagerProxy->IsPcOrPadFreeMultiWindowMode(isPcOrPadFreeMultiWindowMode),
              WMError::WM_OK);
}

/**
 * @tc.name: IsWindowRectAutoSave
 * @tc.desc: IsWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, IsWindowRectAutoSave, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    bool enabled = false;
    std::string key = "com.example.recposentryEntryAbility";
    int persistentId = 1;
    ASSERT_EQ(sceneSessionManagerProxy->IsWindowRectAutoSave(key, enabled, persistentId), WMError::WM_ERROR_IPC_FAILED);
}

/**
 * @tc.name: SetImageForRecent
 * @tc.desc: SetImageForRecent
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetImageForRecent, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_EQ(sceneSessionManagerProxy->SetImageForRecent(1, ImageFit::FILL, 1), WMError::WM_ERROR_IPC_FAILED);
}

/**
 * @tc.name: SetImageForRecentPixelMap
 * @tc.desc: SetImageForRecentPixelMap
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetImageForRecentPixelMap, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    auto pixelMap = std::make_shared<Media::PixelMap>();
    EXPECT_EQ(sceneSessionManagerProxy->SetImageForRecentPixelMap(pixelMap, ImageFit::FILL, 1),
        WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerProxy->SetImageForRecentPixelMap(pixelMap, ImageFit::FILL, 1),
        WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerProxy->SetImageForRecentPixelMap(pixelMap, ImageFit::FILL, 1),
        WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerProxy->SetImageForRecentPixelMap(pixelMap, ImageFit::FILL, 1),
        WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerProxy->SetImageForRecentPixelMap(pixelMap, ImageFit::FILL, 1),
        WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: RemoveImageForRecent
 * @tc.desc: RemoveImageForRecent
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RemoveImageForRecent, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    auto pixelMap = std::make_shared<Media::PixelMap>();
    EXPECT_EQ(sceneSessionManagerProxy->RemoveImageForRecent(1), WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerProxy->RemoveImageForRecent(1), WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerProxy->RemoveImageForRecent(1), WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: GetDisplayIdByWindowId
 * @tc.desc: GetDisplayIdByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetDisplayIdByWindowId, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_TRUE(sceneSessionManagerProxy != nullptr);
    const std::vector<uint64_t> windowIds = { 1, 2 };
    std::unordered_map<uint64_t, DisplayId> windowDisplayIdMap;
    sceneSessionManagerProxy->GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
}

/**
 * @tc.name: SetGlobalDragResizeType
 * @tc.desc: SetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetGlobalDragResizeType, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_TRUE(sceneSessionManagerProxy != nullptr);
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    sceneSessionManagerProxy->SetGlobalDragResizeType(dragResizeType);
}

/**
 * @tc.name: GetGlobalDragResizeType
 * @tc.desc: GetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetGlobalDragResizeType, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_TRUE(sceneSessionManagerProxy != nullptr);
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    sceneSessionManagerProxy->GetGlobalDragResizeType(dragResizeType);
}

/**
 * @tc.name: SetAppDragResizeType
 * @tc.desc: SetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetAppDragResizeType, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_TRUE(sceneSessionManagerProxy != nullptr);
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    const std::string bundleName = "test";
    sceneSessionManagerProxy->SetAppDragResizeType(bundleName, dragResizeType);
}

/**
 * @tc.name: GetAppDragResizeType
 * @tc.desc: GetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetAppDragResizeType, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_TRUE(sceneSessionManagerProxy != nullptr);
    DragResizeType toDragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    const std::string bundleName = "test";
    std::unordered_map<std::string, DragResizeType> appDragResizeTypeMap_;
    appDragResizeTypeMap_[bundleName] = toDragResizeType;
    sceneSessionManagerProxy->GetAppDragResizeType(bundleName, dragResizeType);
}

/**
 * @tc.name: SetAppKeyFramePolicy
 * @tc.desc: SetAppKeyFramePolicy
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetAppKeyFramePolicy, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_TRUE(sceneSessionManagerProxy != nullptr);
    const std::string bundleName = "test";
    KeyFramePolicy keyFramePolicy;
    keyFramePolicy.dragResizeType_ = DragResizeType::RESIZE_KEY_FRAME;
    WMError res = sceneSessionManagerProxy->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: GetFocusSessionToken
 * @tc.desc: GetFocusSessionToken
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetFocusSessionToken, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    DisplayId displayId = 1000;
    WSError res = sceneSessionManagerProxy->GetFocusSessionToken(token, displayId);
    ASSERT_EQ(WSError::WS_OK, res);
    sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    res = sceneSessionManagerProxy->GetFocusSessionToken(token, displayId);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: GetFocusSessionElement
 * @tc.desc: GetFocusSessionElement
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetFocusSessionElement, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    std::string device = "deviceTest";
    std::string bundle = "bundleTest";
    std::string ability = "abilityTest";
    OHOS::AppExecFwk::ElementName element(device, bundle, ability);
    DisplayId displayId = 1000;
    WSError res = sceneSessionManagerProxy->GetFocusSessionElement(element, displayId);
    ASSERT_EQ(WSError::WS_OK, res);
    sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    res = sceneSessionManagerProxy->GetFocusSessionElement(element, displayId);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: GetSessionSnapshot
 * @tc.desc: GetSessionSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetSessionSnapshot, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    std::string deviceId;
    int32_t persistentId = 0;
    SessionSnapshot snapshot;
    bool isLowResolution = true;
    WSError res = sceneSessionManagerProxy->GetSessionSnapshot(deviceId, persistentId, snapshot, isLowResolution);
    ASSERT_EQ(WSError::WS_OK, res);
    sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    res = sceneSessionManagerProxy->GetSessionSnapshot(deviceId, persistentId, snapshot, isLowResolution);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: SetSessionContinueState
 * @tc.desc: SetSessionContinueState
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetSessionContinueState, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    int32_t tempcontinueState = 0;
    ContinueState continueState = static_cast<ContinueState>(tempcontinueState);
    WSError res = sceneSessionManagerProxy->SetSessionContinueState(token, continueState);
    ASSERT_EQ(WSError::WS_OK, res);
    sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    res = sceneSessionManagerProxy->SetSessionContinueState(token, continueState);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: UnlockSession
 * @tc.desc: UnlockSession
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UnlockSession, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t sessionId = 0;
    WSError res = sceneSessionManagerProxy->UnlockSession(sessionId);
    ASSERT_EQ(WSError::WS_OK, res);
    sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    res = sceneSessionManagerProxy->UnlockSession(sessionId);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: MoveSessionsToForeground
 * @tc.desc: MoveSessionsToForeground
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, MoveSessionsToForeground, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    std::vector<int32_t> sessionIds;
    int32_t topSessionId = 0;
    WSError res = sceneSessionManagerProxy->MoveSessionsToForeground(sessionIds, topSessionId);
    ASSERT_EQ(WSError::WS_OK, res);
    sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    res = sceneSessionManagerProxy->MoveSessionsToForeground(sessionIds, topSessionId);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: MoveSessionsToBackground
 * @tc.desc: MoveSessionsToBackground
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, MoveSessionsToBackground, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    std::vector<int32_t> sessionIds;
    std::vector<int32_t> result;
    WSError res = sceneSessionManagerProxy->MoveSessionsToBackground(sessionIds, result);
    ASSERT_EQ(WSError::WS_OK, res);
    sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    res = sceneSessionManagerProxy->MoveSessionsToBackground(sessionIds, result);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: ClearAllSessions
 * @tc.desc: ClearAllSessions
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, ClearAllSessions, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    WSError res = sceneSessionManagerProxy->ClearAllSessions();
    ASSERT_EQ(WSError::WS_OK, res);
    sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    res = sceneSessionManagerProxy->ClearAllSessions();
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: UnregisterIAbilityManagerCollaborator
 * @tc.desc: UnregisterIAbilityManagerCollaborator
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UnregisterIAbilityManagerCollaborator, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t type = 0;
    WSError res = sceneSessionManagerProxy->UnregisterIAbilityManagerCollaborator(type);
    ASSERT_EQ(WSError::WS_OK, res);
    sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    res = sceneSessionManagerProxy->UnregisterIAbilityManagerCollaborator(type);
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: NotifyWatchGestureConsumeResult
 * @tc.desc: NotifyWatchGestureConsumeResult
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, NotifyWatchGestureConsumeResult, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t keyCode = 0;
    bool isConsumed = true;
    WMError res = sceneSessionManagerProxy->NotifyWatchGestureConsumeResult(keyCode, isConsumed);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: NotifyWatchFocusActiveChange
 * @tc.desc: NotifyWatchFocusActiveChange
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, NotifyWatchFocusActiveChange, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    bool isActive = true;
    WMError res = sceneSessionManagerProxy->NotifyWatchFocusActiveChange(isActive);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: GetParentMainWindowId
 * @tc.desc: GetParentMainWindowId
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetParentMainWindowId, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t windowId = 0;
    int32_t mainWindowId = 0;
    WMError res = sceneSessionManagerProxy->GetParentMainWindowId(windowId, mainWindowId);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: ListWindowInfo
 * @tc.desc: ListWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, ListWindowInfo, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    WindowInfoOption windowInfoOption;
    std::vector<sptr<WindowInfo>> infos;
    WMError res = sceneSessionManagerProxy->ListWindowInfo(windowInfoOption, infos);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: GetWindowModeType
 * @tc.desc: GetWindowModeType
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetWindowModeType, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    WindowModeType windowModeType;
    WMError res = sceneSessionManagerProxy->GetWindowModeType(windowModeType);
    ASSERT_EQ(WMError::WM_OK, res);
    sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    res = sceneSessionManagerProxy->GetWindowModeType(windowModeType);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: SetParentWindow
 * @tc.desc: SetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetParentWindow, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t subWindowId = 1;
    int32_t newParentWindowId = 2;
    WMError res = sceneSessionManagerProxy->SetParentWindow(subWindowId, newParentWindowId);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: MinimizeByWindowId
 * @tc.desc: MinimizeByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, MinimizeByWindowId, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    std::vector<int32_t> windowIds;
    WMError res = sceneSessionManagerProxy->MinimizeByWindowId(windowIds);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: SetForegroundWindowNum
 * @tc.desc: SetForegroundWindowNum
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetForegroundWindowNum, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    uint32_t windowNum = 1;
    WMError res = sceneSessionManagerProxy->SetForegroundWindowNum(windowNum);
    ASSERT_EQ(WMError::WM_OK, res);
    sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    res = sceneSessionManagerProxy->SetForegroundWindowNum(windowNum);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: AnimateTo
 * @tc.desc: AnimateTo
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, AnimateTo, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t windowId = 1;
    WindowAnimationProperty animationProperty;
    WindowAnimationOption animationOption;

    WMError res = sceneSessionManagerProxy->AnimateTo(windowId, animationProperty, animationOption);
    ASSERT_EQ(WMError::WM_OK, res);

    sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    res = sceneSessionManagerProxy->AnimateTo(windowId, animationProperty, animationOption);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: AnimateTo
 * @tc.desc: AnimateTo
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, CreateUIEffectController, Function | SmallTest | Level2)
{
    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneSessionManagerProxy> proxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    sptr<UIEffectControllerClientStubMocker> controllerClient = sptr<UIEffectControllerClientStubMocker>::MakeSptr();
    sptr<IUIEffectController> controller = sptr<UIEffectControllerStubMocker>::MakeSptr();
    int32_t id = -1;
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    EXPECT_EQ(proxy->CreateUIEffectController(controllerClient, controller, id), WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sptr<SceneSessionManagerProxy> proxyNull =
        sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    EXPECT_EQ(proxyNull->CreateUIEffectController(controllerClient, controller, id), WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker->SetRequestResult(10);
    EXPECT_EQ(proxy->CreateUIEffectController(controllerClient, controller, id), WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker->SetRequestResult(0);
    proxy->CreateUIEffectController(controllerClient, controller, id);
}

/**
 * @tc.name: GetPiPSettingSwitchStatus
 * @tc.desc: GetPiPSettingSwitchStatus
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetPiPSettingSwitchStatus, TestSize.Level1)
{
    auto tempProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    bool switchStatus = false;

    // remote == nullptr
    auto ret = tempProxy->GetPiPSettingSwitchStatus(switchStatus);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    // WriteInterfaceToken failed
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    ret = proxy->GetPiPSettingSwitchStatus(switchStatus);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // SendRequest failed
    ASSERT_NE(proxy, nullptr);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->GetPiPSettingSwitchStatus(switchStatus);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    // ReadBool failed
    MockMessageParcel::SetReadBoolErrorFlag(true);
    ret = proxy->GetPiPSettingSwitchStatus(switchStatus);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadBoolErrorFlag(false);

    // ReadInt32 failed
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    ret = proxy->GetPiPSettingSwitchStatus(switchStatus);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadInt32ErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();

    // interface success
    ret = proxy->GetPiPSettingSwitchStatus(switchStatus);
    EXPECT_EQ(ret, WMError::WM_OK);
}

HWTEST_F(sceneSessionManagerProxyTest, GetIsPipEnabled, TestSize.Level1)
{
    auto tempProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    bool isPipEnabled = false;
    
    // remote == nullptr
    auto ret = tempProxy->GetIsPipEnabled(isPipEnabled);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    
    // WriteInterfaceToken failed
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    ret = proxy->GetIsPipEnabled(isPipEnabled);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    
    // SendRequest failed
    ASSERT_NE(proxy, nullptr);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->GetIsPipEnabled(isPipEnabled);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);
    
    // ReadBool failed
    MockMessageParcel::SetReadBoolErrorFlag(true);
    ret = proxy->GetIsPipEnabled(isPipEnabled);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadBoolErrorFlag(false);
    
    // ReadInt32 failed
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    ret = proxy->GetIsPipEnabled(isPipEnabled);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadInt32ErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();
    
    // interface success
    ret = proxy->GetIsPipEnabled(isPipEnabled);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: SetScreenPrivacyWindowTagSwitch01
 * @tc.desc: SetScreenPrivacyWindowTagSwitch
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetScreenPrivacyWindowTagSwitch01, TestSize.Level1)
{
    auto tempProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    uint64_t screenId = 0;
    std::vector<std::string> privacyWindowTags;
    bool enable = false;

    // remote == nullptr
    auto ret = tempProxy->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    // WriteInterfaceToken failed
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    ret = proxy->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // SendRequest failed
    ASSERT_NE(proxy, nullptr);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);
}

/**
 * @tc.name: SetScreenPrivacyWindowTagSwitch02
 * @tc.desc: SetScreenPrivacyWindowTagSwitch
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetScreenPrivacyWindowTagSwitch02, TestSize.Level1)
{
    uint64_t screenId = 0;
    std::vector<std::string> privacyWindowTags;
    bool enable = false;

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);

    // ReadBool failed
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    auto ret = proxy->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteBoolErrorFlag(false);

    // ReadUint64 failed
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    ret = proxy->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint64ErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();

    // ReadString failed
    privacyWindowTags.push_back("test");
    MockMessageParcel::SetWriteStringErrorFlag(true);
    ret = proxy->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteStringErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();

    // interface success
    ret = proxy->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: AddSessionBlackList01
 * @tc.desc: AddSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, AddSessionBlackList01, TestSize.Level1)
{
    auto tempProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    std::unordered_set<std::string> bundleNames;
    std::unordered_set<std::string> privacyWindowTags;

    // remote == nullptr
    auto ret = tempProxy->AddSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    // WriteInterfaceToken failed
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    ret = proxy->AddSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // SendRequest failed
    ASSERT_NE(proxy, nullptr);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->AddSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);
}

/**
 * @tc.name: AddSessionBlackList02
 * @tc.desc: AddSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, AddSessionBlackList02, TestSize.Level1)
{
    std::unordered_set<std::string> bundleNames;
    std::unordered_set<std::string> privacyWindowTags;

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);

    // ReadUint64 failed
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    auto ret = proxy->AddSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint64ErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();

    // ReadString failed
    bundleNames.insert("test");
    MockMessageParcel::SetWriteStringErrorFlag(true);
    ret = proxy->AddSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteStringErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();

    // interface success
    ret = proxy->AddSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: RemoveSessionBlackList01
 * @tc.desc: RemoveSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RemoveSessionBlackList01, TestSize.Level1)
{
    auto tempProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    std::unordered_set<std::string> bundleNames;
    std::unordered_set<std::string> privacyWindowTags;

    // remote == nullptr
    auto ret = tempProxy->RemoveSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    // WriteInterfaceToken failed
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    ret = proxy->RemoveSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // SendRequest failed
    ASSERT_NE(proxy, nullptr);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->RemoveSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);
}

/**
 * @tc.name: RemoveSessionBlackList02
 * @tc.desc: RemoveSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RemoveSessionBlackList02, TestSize.Level1)
{
    std::unordered_set<std::string> bundleNames;
    std::unordered_set<std::string> privacyWindowTags;

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);

    // ReadUint64 failed
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    auto ret = proxy->RemoveSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint64ErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();

    // ReadString failed
    bundleNames.insert("test");
    MockMessageParcel::SetWriteStringErrorFlag(true);
    ret = proxy->RemoveSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteStringErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();

    // interface success
    ret = proxy->RemoveSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: UpdateOutline
 * @tc.desc: UpdateOutline
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UpdateOutline, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);

    sptr<IRemoteObject> remoteObject;
    OutlineParams params;
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    WMError ret = proxy->UpdateOutline(remoteObject, params);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    ret = proxy->UpdateOutline(remoteObject, params);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);

    remoteObject = sptr<MockIRemoteObject>::MakeSptr();
    MockMessageParcel::SetWriteRemoteObjectErrorFlag(true);
    ret = proxy->UpdateOutline(remoteObject, params);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);

    MockMessageParcel::SetWriteRemoteObjectErrorFlag(false);
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    ret = proxy->UpdateOutline(remoteObject, params);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);

    MockMessageParcel::SetWriteParcelableErrorFlag(false);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->UpdateOutline(remoteObject, params);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);

    remoteMocker->SetRequestResult(ERR_NONE);
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    ret = proxy->UpdateOutline(remoteObject, params);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);

    MockMessageParcel::SetReadInt32ErrorFlag(false);
    ret = proxy->UpdateOutline(remoteObject, params);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UpdateOutline01
 * @tc.desc: UpdateOutline
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UpdateOutline01, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);

    sptr<IRemoteObject> remoteObject;
    OutlineParams params;
    WMError ret = proxy->UpdateOutline(remoteObject, params);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
}

/**
 * @tc.name: ConvertToRelativeCoordinateExtended01
 * @tc.desc: ConvertToRelativeCoordinateExtended
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, ConvertToRelativeCoordinateExtended01, TestSize.Level1)
{
    Rect rect;
    Rect newRect;
    DisplayId newDisplayId = 0;
    rect = { 100, 2000, 400, 600 };
    newRect = { 0, 100, 200, 300 };

    sptr<MockIRemoteObject> remoteMocker = nullptr;
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    auto ret = proxy->ConvertToRelativeCoordinateExtended(rect, newRect, newDisplayId);
    EXPECT_EQ(ret, WMError::WS_ERROR_NULLPTR);
    ASSERT_NE(proxy, nullptr);

    remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);

    // WriteInterfaceToken failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->ConvertToRelativeCoordinateExtended(rect, newRect, newDisplayId);
    EXPECT_EQ(WMError::WS_ERROR_NULLPTR, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = proxy->ConvertToRelativeCoordinateExtended(rect, newRect, newDisplayId);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    ret = proxy->ConvertToRelativeCoordinateExtended(rect, newRect, newDisplayId);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    // SendRequest failed
    ASSERT_NE(proxy, nullptr);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->ConvertToRelativeCoordinateExtended(rect, newRect, newDisplayId);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);
}

/**
 * @tc.name: RecoverWindowPropertyChangeFlag01
 * @tc.desc: RecoverWindowPropertyChangeFlag
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverWindowPropertyChangeFlag01, TestSize.Level1)
{
    auto tempProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);
    uint32_t observedFlags = 0;
    uint32_t interestedFlags = 0;

    // remote == nullptr
    auto ret = tempProxy->RecoverWindowPropertyChangeFlag(observedFlags, interestedFlags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    // WriteInterfaceToken failed
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    ret = proxy->RecoverWindowPropertyChangeFlag(observedFlags, interestedFlags);
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // SendRequest failed
    ASSERT_NE(proxy, nullptr);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->RecoverWindowPropertyChangeFlag(observedFlags, interestedFlags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);
}

/**
 * @tc.name: RecoverWindowPropertyChangeFlag02
 * @tc.desc: RecoverWindowPropertyChangeFlag
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverWindowPropertyChangeFlag02, TestSize.Level1)
{
    uint32_t observedFlags = 0;
    uint32_t interestedFlags = 0;

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);

    // ReadUint32 failed
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    auto ret = proxy->RecoverWindowPropertyChangeFlag(observedFlags, interestedFlags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint32ErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();

    // interface success
    ret = proxy->RecoverWindowPropertyChangeFlag(observedFlags, interestedFlags);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: SetSpecificWindowZIndex
 * @tc.desc: SetSpecificWindowZIndex
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetSpecificWindowZIndex, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = nullptr;
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    auto ret = proxy->SetSpecificWindowZIndex(WindowType::WINDOW_TYPE_FLOAT, 20);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    ASSERT_NE(proxy, nullptr);

    remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->SetSpecificWindowZIndex(WindowType::WINDOW_TYPE_FLOAT, 20);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    ret = proxy->SetSpecificWindowZIndex(WindowType::WINDOW_TYPE_FLOAT, 20);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = proxy->SetSpecificWindowZIndex(WindowType::WINDOW_TYPE_FLOAT, 20);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    ret = proxy->SetSpecificWindowZIndex(WindowType::WINDOW_TYPE_FLOAT, 20);
    EXPECT_EQ(WSError::WS_OK, ret);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->SetSpecificWindowZIndex(WindowType::WINDOW_TYPE_FLOAT, 20);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_NONE);
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    ret = proxy->SetSpecificWindowZIndex(WindowType::WINDOW_TYPE_FLOAT, 20);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetReadInt32ErrorFlag(false);

    ret = proxy->SetSpecificWindowZIndex(WindowType::WINDOW_TYPE_FLOAT, 20);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ResetSpecificWindowZIndex
 * @tc.desc: ResetSpecificWindowZIndex
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, ResetSpecificWindowZIndex, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = nullptr;
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    auto ret = proxy->ResetSpecificWindowZIndex(10);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    ASSERT_NE(proxy, nullptr);

    remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->ResetSpecificWindowZIndex(10);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = proxy->ResetSpecificWindowZIndex(10);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    ret = proxy->ResetSpecificWindowZIndex(10);
    EXPECT_EQ(WSError::WS_OK, ret);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->ResetSpecificWindowZIndex(10);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_NONE);
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    ret = proxy->ResetSpecificWindowZIndex(10);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetReadInt32ErrorFlag(false);

    ret = proxy->ResetSpecificWindowZIndex(10);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: NotifySupportRotationRegistered
 * @tc.desc: NotifySupportRotationRegistered
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, NotifySupportRotationRegistered, TestSize.Level1)
{
    auto tempProxy = sptr<SceneSessionManagerProxy>::MakeSptr(nullptr);

    // remote == nullptr
    auto ret = tempProxy->NotifySupportRotationRegistered();
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    // WriteInterfaceToken failed
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_NE(proxy, nullptr);
    ret = proxy->NotifySupportRotationRegistered();
    EXPECT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    
    // SendRequest failed
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->NotifySupportRotationRegistered();
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);


    // interface success
    MockMessageParcel::ClearAllErrorFlag();
    ret = proxy->NotifySupportRotationRegistered();
    EXPECT_EQ(ret, WMError::WM_OK);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
