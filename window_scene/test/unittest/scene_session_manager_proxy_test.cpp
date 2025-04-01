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
#include "mock/mock_session.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
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

void sceneSessionManagerProxyTest::SetUpTestCase()
{
}

void sceneSessionManagerProxyTest::TearDownTestCase()
{
}

void sceneSessionManagerProxyTest::SetUp()
{
}

void sceneSessionManagerProxyTest::TearDown()
{
}

namespace {
/**
 * @tc.name: CreateAndConnectSpecificSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, CreateAndConnectSpecificSession, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    int32_t persistentId = 0;
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    sceneSessionManagerProxy->CreateAndConnectSpecificSession(sessionStage, eventChannel, node, property,
        persistentId, session, systemConfig, token);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: CreateAndConnectSpecificSession2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, CreateAndConnectSpecificSession2, Function | SmallTest | Level2)
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

    sceneSessionManagerProxy->CreateAndConnectSpecificSession(sessionStage, eventChannel, node, property,
        persistentId, session, systemConfig, token);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: CreateAndConnectSpecificSession3
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, CreateAndConnectSpecificSession3, Function | SmallTest | Level2)
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

    sceneSessionManagerProxy->CreateAndConnectSpecificSession(sessionStage, eventChannel, node, property,
        persistentId, session, systemConfig, token);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: CreateAndConnectSpecificSession4
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, CreateAndConnectSpecificSession4, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    int32_t persistentId = 0;
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token = nullptr;

    sceneSessionManagerProxy->CreateAndConnectSpecificSession(sessionStage, eventChannel, node, property,
        persistentId, session, systemConfig, token);
    EXPECT_NE(sceneSessionManagerProxy, nullptr);
}

/**
 * @tc.name: RecoverAndConnectSpecificSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverAndConnectSpecificSession, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    auto ret = sceneSessionManagerProxy->RecoverAndConnectSpecificSession(sessionStage, eventChannel, node, property,
        session, token);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: RecoverAndConnectSpecificSession2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverAndConnectSpecificSession2, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<WindowSessionProperty> property = nullptr;
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    auto ret = sceneSessionManagerProxy->RecoverAndConnectSpecificSession(sessionStage, eventChannel, node, property,
        session, token);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: RecoverAndConnectSpecificSession3
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverAndConnectSpecificSession3, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    sptr<IRemoteObject> token = nullptr;

    auto ret = sceneSessionManagerProxy->RecoverAndConnectSpecificSession(sessionStage, eventChannel, node, property,
        session, token);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverAndReconnectSceneSession, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    auto ret = sceneSessionManagerProxy->RecoverAndReconnectSceneSession(sessionStage, eventChannel, node, session,
        property, token);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverAndReconnectSceneSession2, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<WindowSessionProperty> property = nullptr;
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();

    auto ret = sceneSessionManagerProxy->RecoverAndReconnectSceneSession(sessionStage, eventChannel, node, session,
        property, token);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession3
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RecoverAndReconnectSceneSession3, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> node = RSSurfaceNode::Create(rsSurfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    SessionInfo info;
    sptr<ISession> session = sptr<SessionMocker>::MakeSptr(info);
    sptr<IRemoteObject> token = nullptr;

    auto ret = sceneSessionManagerProxy->RecoverAndReconnectSceneSession(sessionStage, eventChannel, node, session,
        property, token);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, DestroyAndDisconnectSpecificSession, Function | SmallTest | Level2)
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
HWTEST_F(
    sceneSessionManagerProxyTest,
    DestroyAndDisconnectSpecificSessionWithDetachCallback,
    Function | SmallTest | Level2
)
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
HWTEST_F(sceneSessionManagerProxyTest, UpdateSessionTouchOutsideListener, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, SkipSnapshotForAppProcess, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, SetGestureNavigationEnabled, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetFocusWindowInfo, Function | SmallTest | Level2)
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
 * @tc.name: SetSessionIcon
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetSessionIcon, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetAccessibilityWindowInfo, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetUnreliableWindowInfo, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetSessionInfos, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetSessionInfo, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, RegisterWindowManagerAgent01, Function | SmallTest | Level2)
{
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->RegisterWindowManagerAgent(type, windowManagerAgent));
    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->UnregisterWindowManagerAgent(type, windowManagerAgent));
}

/**
 * @tc.name: UpdateModalExtensionRect
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UpdateModalExtensionRect, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sceneSessionManagerProxy, nullptr);

    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    Rect rect { 1, 2, 3, 4 };
    sceneSessionManagerProxy->UpdateModalExtensionRect(token, rect);
    sceneSessionManagerProxy->UpdateModalExtensionRect(nullptr, rect);
}

/**
 * @tc.name: ProcessModalExtensionPointDown
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, ProcessModalExtensionPointDown, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, AddExtensionWindowStageToSCB, Function | SmallTest | Level2)
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
    sceneSessionManagerProxy->AddExtensionWindowStageToSCB(sessionStage, token, 12345);
}

/**
 * @tc.name: RemoveExtensionWindowStageFromSCB
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, RemoveExtensionWindowStageFromSCB, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, AddOrRemoveSecureSession001, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, UpdateExtWindowFlags, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->UpdateExtWindowFlags(token, 7, 7));
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sceneSessionManagerProxy->UpdateExtWindowFlags(nullptr, 7, 7));
}

/**
 * @tc.name: GetSessionInfoByContinueSessionId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetSessionInfoByContinueSessionId, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, RequestFocusStatus01, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, RequestFocusStatusBySA01, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, RaiseWindowToTop, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, BindDialogSessionTarget, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, BindDialogSessionTarget2, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, UpdateSessionAvoidAreaListener, Function | SmallTest | Level2)
{
    int32_t persistendId = 0;
    bool haveListener = true;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->UpdateSessionAvoidAreaListener(persistendId,
        haveListener));
}

/**
 * @tc.name: SetSessionLabel
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetSessionLabel, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> token = nullptr;
    std::string label = "SetSessionLabel";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sceneSessionManagerProxy->SetSessionLabel(token, label));
}

/**
 * @tc.name: IsValidSessionIds
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, IsValidSessionIds, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, RegisterSessionListener01, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, UnRegisterSessionListener, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetSnapshotByWindowId, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    int32_t windowId = -1;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    WMError ret = sceneSessionManagerProxy->GetSnapshotByWindowId(windowId, pixelMap);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetSessionSnapshotById
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetSessionSnapshotById, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    int32_t windowId = -1;
    SessionSnapshot snapshot;
    WMError ret = sceneSessionManagerProxy->GetSessionSnapshotById(windowId, snapshot);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: TerminateSessionNew
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, TerminateSessionNew, Function | SmallTest | Level2)
{
    sptr<ISessionListener> listener = nullptr;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, sceneSessionManagerProxy->TerminateSessionNew(nullptr, true, true));
}

/**
 * @tc.name: GetSessionDumpInfo
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetSessionDumpInfo, Function | SmallTest | Level2)
{
    std::vector<std::string> params;
    std::string info = "info";
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    ASSERT_EQ(WSError::WS_OK, sceneSessionManagerProxy->GetSessionDumpInfo(params, info));
}

/**
 * @tc.name: LockSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, LockSession, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetUIContentRemoteObj, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    sptr<IRemoteObject> remoteObj;
    ASSERT_EQ(WSError::WS_ERROR_IPC_FAILED, sceneSessionManagerProxy->GetUIContentRemoteObj(1, remoteObj));
}

/**
 * @tc.name: ClearSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, ClearSession, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, RegisterIAbilityManagerCollaborator, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, NotifyWindowExtensionVisibilityChange, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetTopWindowId, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetWindowStyleType, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, SetProcessWatermark, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t pid = 1000;
    const std::string watermarkName = "SetProcessWatermarkName";
    bool isEnabled = false;
    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->SetProcessWatermark(pid, watermarkName, isEnabled));
}

/**
 * @tc.name: GetWindowIdsByCoordinate
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetWindowIdsByCoordinate, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetProcessSurfaceNodeIdByPersistentId, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t pid = 123;
    std::vector<int32_t> persistentIds = {1, 2, 3};
    std::vector<uint64_t> surfaceNodeIds;
    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->GetProcessSurfaceNodeIdByPersistentId(
        pid, persistentIds, surfaceNodeIds));
}

/**
 * @tc.name: SkipSnapshotByUserIdAndBundleNames
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SkipSnapshotByUserIdAndBundleNames, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t userId = 1;
    std::vector<std::string> bundleNameList = {"a", "b", "c"};
    ASSERT_EQ(WMError::WM_OK, sceneSessionManagerProxy->SkipSnapshotByUserIdAndBundleNames(
        userId, bundleNameList));
}

/**
 * @tc.name: UpdateScreenLockStatusForApp
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, UpdateScreenLockStatusForApp, Function | SmallTest | Level2)
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
    std::vector<int32_t> persistentIds {0, 1, 2};
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
    std::vector<int32_t> persistentIds {0, 1, 2};
    ASSERT_EQ(sceneSessionManagerProxy->RemoveSkipSelfWhenShowOnVirtualScreenList(persistentIds), WMError::WM_OK);
}

/**
 * @tc.name: IsPcWindow
 * @tc.desc: IsPcWindow
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, IsPcWindow, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    bool isPcWindow = false;
    ASSERT_EQ(sceneSessionManagerProxy->IsPcWindow(isPcWindow),
        WMError::WM_ERROR_IPC_FAILED);
    ASSERT_EQ(isPcWindow, false);
}

/**
 * @tc.name: IsPcOrPadFreeMultiWindowMode
 * @tc.desc: IsPcOrPadFreeMultiWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, IsPcOrPadFreeMultiWindowMode, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    bool isPcOrPadFreeMultiWindowMode = false;
    ASSERT_EQ(sceneSessionManagerProxy->IsPcOrPadFreeMultiWindowMode(isPcOrPadFreeMultiWindowMode),
        WMError::WM_ERROR_IPC_FAILED);
    ASSERT_EQ(isPcOrPadFreeMultiWindowMode, false);
}

/**
 * @tc.name: IsWindowRectAutoSave
 * @tc.desc: IsWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, IsWindowRectAutoSave, Function | SmallTest | Level2)
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
 * @tc.name: GetDisplayIdByWindowId
 * @tc.desc: GetDisplayIdByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetDisplayIdByWindowId, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_TRUE(sceneSessionManagerProxy != nullptr);
    const std::vector<uint64_t> windowIds = {1, 2};
    std::unordered_map<uint64_t, DisplayId> windowDisplayIdMap;
    sceneSessionManagerProxy->GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
}

/**
 * @tc.name: SetGlobalDragResizeType
 * @tc.desc: SetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, SetGlobalDragResizeType, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetGlobalDragResizeType, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, SetAppDragResizeType, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetAppDragResizeType, Function | SmallTest | Level2)
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
 * @tc.name: GetFocusSessionToken
 * @tc.desc: GetFocusSessionToken
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetFocusSessionToken, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetFocusSessionElement, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, GetSessionSnapshot, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, SetSessionContinueState, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, UnlockSession, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, MoveSessionsToForeground, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, MoveSessionsToBackground, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, ClearAllSessions, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, UnregisterIAbilityManagerCollaborator, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, NotifyWatchGestureConsumeResult, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t keyCode = 0;
    bool isConsumed = true;
    WMError res = sceneSessionManagerProxy->NotifyWatchGestureConsumeResult(keyCode, isConsumed);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: NotifyWatchFocusActiveChange
 * @tc.desc: NotifyWatchFocusActiveChange
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, NotifyWatchFocusActiveChange, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    bool isActive = true;
    WMError res = sceneSessionManagerProxy->NotifyWatchFocusActiveChange(isActive);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: GetParentMainWindowId
 * @tc.desc: GetParentMainWindowId
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetParentMainWindowId, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    int32_t windowId = 0;
    int32_t mainWindowId = 0;
    WMError res = sceneSessionManagerProxy->GetParentMainWindowId(windowId, mainWindowId);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: ListWindowInfo
 * @tc.desc: ListWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, ListWindowInfo, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerProxy> sceneSessionManagerProxy =
        sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);

    WindowInfoOption windowInfoOption;
    std::vector<sptr<WindowInfo>> infos;
    WMError res = sceneSessionManagerProxy->ListWindowInfo(windowInfoOption, infos);
    ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: GetWindowModeType
 * @tc.desc: GetWindowModeType
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerProxyTest, GetWindowModeType, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, SetParentWindow, Function | SmallTest | Level2)
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
HWTEST_F(sceneSessionManagerProxyTest, MinimizeByWindowId, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto sceneSessionManagerProxy = sptr<SceneSessionManagerProxy>::MakeSptr(iRemoteObjectMocker);
    std::vector<int32_t> windowIds;
    WMError res = sceneSessionManagerProxy->MinimizeByWindowId(windowIds);
    ASSERT_EQ(WMError::WM_OK, res);
}
}  // namespace
}
}
