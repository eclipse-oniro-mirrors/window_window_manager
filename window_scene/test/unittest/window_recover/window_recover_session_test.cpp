/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <bundle_mgr_proxy.h>
#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>

#include "mock/mock_session_stage.h"
#include "iremote_object_mocker.h"
#include "session_manager.h"
#include "session_manager/include/scene_session_manager.h"
#include "unittest/window_event_channel_base.h"
#include "interfaces/include/ws_common.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowRecoverSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    SessionInfo mainSessionInfo;
    sptr<MainSession::SpecificSessionCallback> specificCallback = nullptr;
    sptr<MainSession> mainSession_;
    SessionInfo sceneSessionInfo;
    sptr<SceneSession> sceneSession_;
    sptr<SceneSessionManager> ssm_;
    std::shared_ptr<SessionManager> sm_;

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void WindowRecoverSessionTest::SetUpTestCase() {}

void WindowRecoverSessionTest::TearDownTestCase() {}

void WindowRecoverSessionTest::SetUp()
{
    SessionInfo mainSessionInfo;
    mainSessionInfo.abilityName_ = "testMainSession1";
    mainSessionInfo.moduleName_ = "testMainSession2";
    mainSessionInfo.bundleName_ = "testMainSession3";
    mainSession_ = sptr<MainSession>::MakeSptr(mainSessionInfo, specificCallback);
    EXPECT_NE(nullptr, mainSession_);

    SessionInfo sceneSessionInfo;
    sceneSessionInfo.abilityName_ = "testSceneSession1";
    sceneSessionInfo.moduleName_ = "testSceneSession2";
    sceneSessionInfo.bundleName_ = "testSceneSession3";
    sceneSession_ = sptr<MainSession>::MakeSptr(sceneSessionInfo, nullptr);
    EXPECT_NE(nullptr, sceneSession_);

    ssm_ = &SceneSessionManager::GetInstance();

    sm_ = std::make_shared<SessionManager>();
}

void WindowRecoverSessionTest::TearDown()
{
    mainSession_ = nullptr;
    sceneSession_ = nullptr;

    usleep(WAIT_SYNC_IN_NS);
    ssm_ = nullptr;

    sm_ = nullptr;
}

RSSurfaceNode::SharedPtr WindowRecoverSessionTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    return surfaceNode;
}

void NotifyRecoverSceneSessionFuncTest(const sptr<SceneSession>& session, const SessionInfo& sessionInfo) {}

namespace {

/**
 * @tc.name: Reconnect01
 * @tc.desc: check func Reconnect
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, Reconnect01, TestSize.Level1)
{
    auto surfaceNode = CreateRSSurfaceNode();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    sptr<TestWindowEventChannel> testWindowEventChannel = sptr<TestWindowEventChannel>::MakeSptr();
    EXPECT_NE(nullptr, testWindowEventChannel);

    auto result = mainSession_->Reconnect(nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = mainSession_->Reconnect(nullptr, nullptr, nullptr, property);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = mainSession_->Reconnect(nullptr, testWindowEventChannel, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = mainSession_->Reconnect(mockSessionStage, nullptr, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = mainSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_OK);

    property->SetWindowState(WindowState::STATE_HIDDEN);
    result = mainSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_OK);

    property->SetWindowState(WindowState::STATE_SHOWN);
    result = mainSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Reconnect02
 * @tc.desc: Reconnect02
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, Reconnect02, TestSize.Level1)
{
    auto surfaceNode = CreateRSSurfaceNode();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    sptr<TestWindowEventChannel> testWindowEventChannel = sptr<TestWindowEventChannel>::MakeSptr();
    EXPECT_NE(nullptr, testWindowEventChannel);

    auto result = sceneSession_->Reconnect(nullptr, nullptr, nullptr, property);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = sceneSession_->Reconnect(nullptr, testWindowEventChannel, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = sceneSession_->Reconnect(mockSessionStage, nullptr, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = sceneSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, nullptr);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    result = sceneSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_OK);

    property->SetWindowState(WindowState::STATE_INITIAL);
    result = sceneSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_OK);

    property->SetWindowState(WindowState::STATE_CREATED);
    result = sceneSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_OK);

    property->SetWindowState(WindowState::STATE_SHOWN);
    result = sceneSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_OK);

    property->SetWindowState(WindowState::STATE_HIDDEN);
    result = sceneSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_OK);

    property->SetWindowState(WindowState::STATE_FROZEN);
    result = sceneSession_->Reconnect(mockSessionStage, testWindowEventChannel, surfaceNode, property);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: ReconnectInner
 * @tc.desc: ReconnectInner
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, ReconnectInner, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = nullptr;
    WSError res = sceneSession_->ReconnectInner(property);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);

    property = sptr<WindowSessionProperty>::MakeSptr();
    property->windowState_ = WindowState::STATE_INITIAL;
    res = sceneSession_->ReconnectInner(property);
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_PARAM);

    property->windowState_ = WindowState::STATE_CREATED;
    res = sceneSession_->ReconnectInner(property);
    ASSERT_EQ(res, WSError::WS_OK);

    property->windowState_ = WindowState::STATE_SHOWN;
    res = sceneSession_->ReconnectInner(property);
    ASSERT_EQ(res, WSError::WS_OK);

    property->windowState_ = WindowState::STATE_HIDDEN;
    res = sceneSession_->ReconnectInner(property);
    ASSERT_EQ(res, WSError::WS_OK);

    property->windowState_ = WindowState::STATE_DESTROYED;
    res = sceneSession_->ReconnectInner(property);
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: CheckSessionPropertyOnRecovery
 * @tc.desc: CheckSessionPropertyOnRecovery
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, CheckSessionPropertyOnRecovery, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<WindowSessionProperty> property = nullptr;
    WSError result = ssm_->CheckSessionPropertyOnRecovery(property, false);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    property->SetWindowFlags(123);
    result = ssm_->CheckSessionPropertyOnRecovery(property, false);
    ASSERT_EQ(result, WSError::WS_ERROR_NOT_SYSTEM_APP);

    // 主窗，不需要恢复
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetPersistentId(100);
    result = ssm_->CheckSessionPropertyOnRecovery(property, false);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);

    // 主窗，需要恢复
    ssm_->SetAlivePersistentIds({ 100 });
    result = ssm_->CheckSessionPropertyOnRecovery(property, false);
    ASSERT_EQ(result, WSError::WS_OK);

    // 特殊窗，不需要恢复
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    property->SetParentPersistentId(111);
    result = ssm_->CheckSessionPropertyOnRecovery(property, true);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);

    // 特殊窗，需要恢复
    ssm_->SetAlivePersistentIds({ 111 });
    result = ssm_->CheckSessionPropertyOnRecovery(property, true);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession
 * @tc.desc: RecoverAndReconnectSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, RecoverAndReconnectSceneSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<WindowSessionProperty> property = nullptr;
    sptr<SessionStageMocker> sessionStage = nullptr;
    sptr<TestWindowEventChannel> eventChannel = nullptr;
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<ISession> session = nullptr;
    sptr<IRemoteObject> token = nullptr;
    // 0.property 为空
    auto result =
        ssm_->RecoverAndReconnectSceneSession(sessionStage, eventChannel, surfaceNode, session, property, token);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetPersistentId(100);
    ssm_->SetAlivePersistentIds({ 15, 16, 100 });
    // 1.已经恢复结束
    ssm_->recoveringFinished_ = true;
    result = ssm_->RecoverAndReconnectSceneSession(sessionStage, eventChannel, surfaceNode, session, property, token);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_OPERATION);

    ssm_->recoveringFinished_ = false;
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "testSceneSession1";
    sessionInfo.moduleName_ = "testSceneSession2";
    sessionInfo.bundleName_ = "testSceneSession3";
    property->SetSessionInfo(sessionInfo);

    // 2.recoverSceneSessionFunc_ 为空
    result = ssm_->RecoverAndReconnectSceneSession(sessionStage, eventChannel, surfaceNode, session, property, token);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    // 3. sessionStage为空
    property->SetPersistentId(15);
    ssm_->recoverSceneSessionFunc_ = NotifyRecoverSceneSessionFuncTest;
    result = ssm_->RecoverAndReconnectSceneSession(sessionStage, eventChannel, surfaceNode, session, property, token);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    // 4.重连
    sessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, sessionStage);
    eventChannel = sptr<TestWindowEventChannel>::MakeSptr();
    EXPECT_NE(nullptr, eventChannel);
    surfaceNode = CreateRSSurfaceNode();
    property->SetPersistentId(16);
    property->SetWindowState(WindowState::STATE_SHOWN);
    result = ssm_->RecoverAndReconnectSceneSession(sessionStage, eventChannel, surfaceNode, session, property, token);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: RecoverAndConnectSpecificSession
 * @tc.desc: RecoverAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, RecoverAndConnectSpecificSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<WindowSessionProperty> property = nullptr;
    sptr<SessionStageMocker> sessionStage = nullptr;
    sptr<TestWindowEventChannel> eventChannel = nullptr;
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<ISession> session = nullptr;
    sptr<IRemoteObject> token = nullptr;
    // 0. property 为空
    auto result =
        ssm_->RecoverAndConnectSpecificSession(sessionStage, eventChannel, surfaceNode, property, session, token);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    property->SetParentPersistentId(111);
    ssm_->SetAlivePersistentIds({ 111 });
    // 1.已经恢复结束
    ssm_->recoveringFinished_ = true;
    result = ssm_->RecoverAndConnectSpecificSession(sessionStage, eventChannel, surfaceNode, property, session, token);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_OPERATION);

    ssm_->recoveringFinished_ = false;
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "testSceneSession1";
    sessionInfo.moduleName_ = "testSceneSession2";
    sessionInfo.bundleName_ = "testSceneSession3";
    property->SetSessionInfo(sessionInfo);

    // 2.info persistentId 无效
    result = ssm_->RecoverAndConnectSpecificSession(sessionStage, eventChannel, surfaceNode, property, session, token);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    // 3. sessionStage 为空
    property->SetPersistentId(20);
    result = ssm_->RecoverAndConnectSpecificSession(sessionStage, eventChannel, surfaceNode, property, session, token);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    // 4.重连 窗口id：21，在前台
    sessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, sessionStage);
    eventChannel = sptr<TestWindowEventChannel>::MakeSptr();
    EXPECT_NE(nullptr, eventChannel);
    surfaceNode = CreateRSSurfaceNode();
    property->SetPersistentId(21);
    property->SetWindowState(WindowState::STATE_SHOWN);
    result = ssm_->RecoverAndConnectSpecificSession(sessionStage, eventChannel, surfaceNode, property, session, token);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: GetBatchAbilityInfos01
 * @tc.desc: GetBatchAbilityInfos01
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, GetBatchAbilityInfos01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    auto bundleMgr = ssm_->bundleMgr_;
    ssm_->bundleMgr_ = nullptr;
    int32_t userId = 100;
    std::vector<std::string> bundleNames = { "test1", "test2" };
    auto scbAbilityInfos = std::make_shared<std::vector<SCBAbilityInfo>>();
    WSError ret = ssm_->GetBatchAbilityInfos(bundleNames, userId, *scbAbilityInfos);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: IsWindowSupportCacheForRecovering
 * @tc.desc: test function : IsWindowSupportCacheForRecovering
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, IsWindowSupportCacheForRecovering, TestSize.Level1)
{
    std::vector<int32_t> recoveredPersistentIds = { 1 };
    ssm_->alivePersistentIds_.clear();
    ssm_->alivePersistentIds_.push_back(1);
    ssm_->alivePersistentIds_.push_back(2);
    ssm_->alivePersistentIds_.push_back(3);
    SessionInfo info;
    info.bundleName_ = "IsWindowSupportCacheForRecovering";
    info.abilityName_ = "IsWindowSupportCacheForRecovering";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->isRecovered_ = true;
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession));
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    sceneSession1->isRecovered_ = false;
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession1));
    sptr<SceneSession> sceneSession2 = nullptr;
    ssm_->sceneSessionMap_.insert(std::make_pair(4, sceneSession2));
    ssm_->ClearUnrecoveredSessions(recoveredPersistentIds);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    ssm_->recoveringFinished_ = true;
    auto ret = ssm_->IsWindowSupportCacheForRecovering(sceneSession, property);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: IsWindowSupportCacheForRecovering01
 * @tc.desc: test function : IsWindowSupportCacheForRecovering
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, IsWindowSupportCacheForRecovering01, TestSize.Level1)
{
    std::vector<int32_t> windowIds = { 0, 1 };
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(0, sceneSession));
    ssm_->OnNotifyAboveLockScreen(windowIds);

    SessionInfo info;
    info.bundleName_ = "IsWindowSupportCacheForRecovering01";
    info.abilityName_ = "IsWindowSupportCacheForRecovering01";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession1));
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    ssm_->recoveringFinished_ = false;
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    auto ret = ssm_->IsWindowSupportCacheForRecovering(sceneSession1, property);
    EXPECT_EQ(true, ret);
    property->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    ret = ssm_->IsWindowSupportCacheForRecovering(sceneSession1, property);
    EXPECT_EQ(false, ret);
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ret = ssm_->IsWindowSupportCacheForRecovering(sceneSession1, property);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: IsWindowSupportCacheForRecovering02
 * @tc.desc: test function : IsWindowSupportCacheForRecovering
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, IsWindowSupportCacheForRecovering02, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "IsWindowSupportCacheForRecovering02";
    info.abilityName_ = "IsWindowSupportCacheForRecovering02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    ssm_->recoveringFinished_ = false;
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    property->parentPersistentId_ = 1;
    NotifyBindDialogSessionFunc func = [](const sptr<SceneSession>& sceneSession) {};
    ssm_->bindDialogTargetFuncMap_.insert(std::make_pair(1, func));
    auto ret = ssm_->IsWindowSupportCacheForRecovering(sceneSession, property);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: CacheSpecificSessionForRecovering
 * @tc.desc: CacheSpecificSessionForRecovering
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, CacheSpecificSessionForRecovering, TestSize.Level1)
{
    sptr<WindowSessionProperty> property;
    ASSERT_NE(ssm_, nullptr);
    ssm_->recoveringFinished_ = false;
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->CacheSpecificSessionForRecovering(nullptr, property);
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);

    property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    ssm_->CacheSpecificSessionForRecovering(nullptr, property);
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    int32_t parentPersistentId = 1;
    property->SetParentPersistentId(parentPersistentId);
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    ASSERT_EQ(ssm_->recoverSubSessionCacheMap_[parentPersistentId].size(), 1);
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    ASSERT_EQ(ssm_->recoverSubSessionCacheMap_[parentPersistentId].size(), 2);
    ssm_->recoverSubSessionCacheMap_.clear();
}

HWTEST_F(WindowRecoverSessionTest, RecoverCachedSubSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    // 1. 没有缓存的子窗
    ssm_->RecoverCachedSubSession(123);
    ASSERT_EQ(ssm_->recoverSubSessionCacheMap_.size(), 0);

    // 2. 有缓存的子窗，进行恢复并清理CacheMap_
    SessionInfo info;
    info.bundleName_ = "SubSession";
    info.abilityName_ = "SubSession";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WindowAnchorInfo anchorInfo = { true, WindowAnchor::TOP_START, 0, 0 };
    property->SetWindowAnchorInfo(anchorInfo);
    sceneSession->SetSessionProperty(property);

    SessionInfo parentInfo;
    parentInfo.bundleName_ = "ParentSession";
    parentInfo.abilityName_ = "ParentSession";
    sptr<SceneSession> parentSession = sptr<SceneSession>::MakeSptr(parentInfo, nullptr);
    ssm_->sceneSessionMap_[123] = parentSession;
    NotifyCreateSubSessionFunc func = [](const sptr<SceneSession>& session) {};
    ssm_->createSubSessionFuncMap_[123] = func;

    ssm_->recoverSubSessionCacheMap_[123].emplace_back(sceneSession);
    ssm_->RecoverCachedSubSession(123);
    ASSERT_EQ(ssm_->recoverSubSessionCacheMap_.size(), 0);
    ssm_->sceneSessionMap_.clear();
    ssm_->createSubSessionFuncMap_.clear();
}

HWTEST_F(WindowRecoverSessionTest, RecoverCachedDialogSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    // 1. 没有缓存的模态窗
    ssm_->RecoverCachedDialogSession(123);
    ASSERT_EQ(ssm_->recoverDialogSessionCacheMap_.size(), 0);

    // 2. 有缓存的模态窗，进行恢复并清理CacheMap_
    SessionInfo info;
    info.bundleName_ = "SubSession";
    info.abilityName_ = "SubSession";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->recoverDialogSessionCacheMap_[123].emplace_back(sceneSession);
    ssm_->RecoverCachedDialogSession(123);
    ASSERT_EQ(ssm_->recoverDialogSessionCacheMap_.size(), 0);
}

/**
 * @tc.name: RemoveFailRecoveredSession
 * @tc.desc: RemoveFailRecoveredSession
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, RemoveFailRecoveredSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "RemoveFailRecoveredSession";
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, nullptr));
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession02));
    ssm_->failRecoveredPersistentIdSet_.insert(1);
    ssm_->failRecoveredPersistentIdSet_.insert(2);
    ssm_->failRecoveredPersistentIdSet_.insert(3);
    ssm_->RemoveFailRecoveredSession();
    EXPECT_EQ(ssm_->failRecoveredPersistentIdSet_.size(), 0);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, ssm_->HandleSecureSessionShouldHide(nullptr));
}

/**
 * @tc.name: ClearUnrecoveredSessions
 * @tc.desc: test func ClearUnrecoveredSessions
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, ClearUnrecoveredSessions, TestSize.Level1)
{
    ssm_->alivePersistentIds_.push_back(23);
    ssm_->alivePersistentIds_.push_back(24);
    ssm_->alivePersistentIds_.push_back(25);
    ssm_->alivePersistentIds_.push_back(26);
    ssm_->alivePersistentIds_.push_back(27);
    EXPECT_FALSE(ssm_->alivePersistentIds_.empty());
    std::vector<int32_t> recoveredPersistentIds;
    recoveredPersistentIds.push_back(23);
    recoveredPersistentIds.push_back(24);
    SessionInfo info;
    info.abilityName_ = "ClearUnrecoveredSessions";
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession01->SetRecovered(false);
    sceneSession02->SetRecovered(true);
    ASSERT_NE(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(26, sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(27, sceneSession02));
    ssm_->ClearUnrecoveredSessions(recoveredPersistentIds);
    EXPECT_EQ(ssm_->alivePersistentIds_.size(), 8);
}

/**
 * @tc.name: RecoverSessionInfo
 * @tc.desc: test func RecoverSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, RecoverSessionInfo, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "RecoverSessionInfo";
    property->SetSessionInfo(sessionInfo);
    ssm_->RecoverSessionInfo(property);
    SessionInfo info = property->GetSessionInfo();
    ASSERT_EQ(info.abilityName_, sessionInfo.abilityName_);
}

/**
 * @tc.name: UpdateRecoverPropertyForSuperFold
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, UpdateRecoverPropertyForSuperFold, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    Rect rect = {
        static_cast<int32_t>(10), static_cast<int32_t>(10), static_cast<uint32_t>(100), static_cast<uint32_t>(100)
    };
    property->SetWindowRect(rect);
    property->SetRequestRect(rect);
    property->SetDisplayId(0);
    ssm_->UpdateRecoverPropertyForSuperFold(property);
    EXPECT_EQ(property->GetWindowRect().posY_, 10);

    property->SetDisplayId(999);
    ssm_->UpdateRecoverPropertyForSuperFold(property);
    EXPECT_EQ(property->GetWindowRect().posX_, 10);
}

/**
 * @tc.name: RecoverSessionManagerService
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, RecoverSessionManagerService, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    bool funcInvoked = false;
    sm_->RegisterWindowManagerRecoverCallbackFunc(nullptr);
    sm_->RecoverSessionManagerService(nullptr);

    sm_->RegisterWindowManagerRecoverCallbackFunc([&]() { funcInvoked = true; });
    sm_->RecoverSessionManagerService(nullptr);
    ASSERT_EQ(funcInvoked, true);
}

/**
 * @tc.name: RegisterSMSRecoverListener1
 * @tc.desc: mockSessionManagerServiceProxy_ is null
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, RegisterSMSRecoverListener1, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    sm_->isRecoverListenerRegistered_ = false;
    sm_->mockSessionManagerServiceProxy_ = nullptr;
    sm_->RegisterSMSRecoverListener();
    ASSERT_EQ(sm_->isRecoverListenerRegistered_, false);
}

/**
 * @tc.name: RegisterSMSRecoverListener2
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, RegisterSMSRecoverListener2, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    sm_->isRecoverListenerRegistered_ = false;
    sm_->InitMockSMSProxy();
    sm_->RegisterSMSRecoverListener();
    ASSERT_EQ(sm_->isRecoverListenerRegistered_, true);
}

/**
 * @tc.name: RegisterSessionRecoverStateChangeListener
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, RegisterSessionRecoverStateChangeListener, TestSize.Level1)
{
    EXPECT_NE(nullptr, ssm_);
    ssm_->RegisterSessionRecoverStateChangeListener();
    EXPECT_NE(ssm_->sessionRecoverStateChangeFunc_, nullptr);
}

/**
 * @tc.name: OnSessionRecoverStateChange
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, OnSessionRecoverStateChange, TestSize.Level1)
{
    EXPECT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPersistentId(160);
    ssm_->OnSessionRecoverStateChange(SessionRecoverState::SESSION_START_RECONNECT, property);
    EXPECT_EQ(property->GetSessionInfo().persistentId_, 160);

    ssm_->sceneSessionMap_.clear();
    // main window
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    SessionInfo info;
    info.abilityName_ = "OnSessionRecoverStateChange";
    info.bundleName_ = "OnSessionRecoverStateChange";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetSessionProperty(property);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ 160, sceneSession });
    ssm_->OnSessionRecoverStateChange(SessionRecoverState::SESSION_FINISH_RECONNECT, property);
    EXPECT_EQ(ssm_->GetSceneSession(160)->IsRecovered(), true);

    // specifc window
    ssm_->GetSceneSession(160)->SetRecovered(false);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->SetSessionProperty(property);
    ssm_->OnSessionRecoverStateChange(SessionRecoverState::SESSION_FINISH_RECONNECT, property);
    EXPECT_EQ(ssm_->GetSceneSession(160)->IsRecovered(), false);

    ssm_->OnSessionRecoverStateChange(SessionRecoverState::SESSION_NOT_RECONNECT, property);
    EXPECT_EQ(ssm_->GetSceneSession(160)->IsRecovered(), false);
}

/**
 * @tc.name: RegisterRecoverStateChangeListener
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, RegisterRecoverStateChangeListener, TestSize.Level1)
{
    EXPECT_NE(nullptr, ssm_);
    ssm_->RegisterRecoverStateChangeListener();
    EXPECT_NE(ssm_->recoverStateChangeFunc_, nullptr);
}

/**
 * @tc.name: OnRecoverStateChange
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, OnRecoverStateChange, TestSize.Level1)
{
    EXPECT_NE(nullptr, ssm_);
    ssm_->displayBrightness_ = UNDEFINED_BRIGHTNESS;
    ssm_->OnRecoverStateChange(RecoverState::RECOVER_INITIAL);
    EXPECT_EQ(ssm_->displayBrightness_, UNDEFINED_BRIGHTNESS);
    ssm_->OnRecoverStateChange(RecoverState::RECOVER_END);
    EXPECT_EQ(ssm_->displayBrightness_, UNDEFINED_BRIGHTNESS);
    ssm_->OnRecoverStateChange(RecoverState::RECOVER_ENABLE_INPUT);
    EXPECT_EQ(ssm_->displayBrightness_, INVALID_BRIGHTNESS);
}

/**
 * @tc.name: SetEnableInputEvent
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(WindowRecoverSessionTest, SetEnableInputEvent, TestSize.Level1)
{
    EXPECT_NE(nullptr, ssm_);
    ssm_->displayBrightness_ = UNDEFINED_BRIGHTNESS;
    ssm_->SetEnableInputEvent(false);
    EXPECT_EQ(ssm_->displayBrightness_, UNDEFINED_BRIGHTNESS);
    ssm_->SetEnableInputEvent(true);
    EXPECT_EQ(ssm_->displayBrightness_, INVALID_BRIGHTNESS);
}

} // namespace
} // namespace Rosen
} // namespace OHOS