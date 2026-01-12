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
#include <regex>
#include <chrono>
#include <thread>
#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>
#include "interfaces/include/ws_common.h"
#include "screen_fold_data.h"
#include "ffrt_queue_helper.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "window_manager_agent.h"
#include "session_manager.h"
#include "zidl/window_manager_agent_interface.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "application_info.h"
#include "context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string EMPTY_DEVICE_ID = "";
}
class SceneSessionManagerLifecycleTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp() override;

    void TearDown() override;

    static void SetVisibleForAccessibility(sptr<SceneSession>& sceneSession);
    int32_t GetTaskCount(sptr<SceneSession>& session);
    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerLifecycleTest::ssm_ = nullptr;

void WindowChangedFuncTest(int32_t persistentId, WindowUpdateType type) {}

void ProcessStatusBarEnabledChangeFuncTest(bool enable) {}

void SceneSessionManagerLifecycleTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerLifecycleTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerLifecycleTest::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerLifecycleTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerLifecycleTest::SetVisibleForAccessibility(sptr<SceneSession>& sceneSession)
{
    sceneSession->SetTouchable(true);
    sceneSession->forceTouchable_ = true;
    sceneSession->systemTouchable_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->foregroundInteractiveStatus_.store(true);
}

int32_t SceneSessionManagerLifecycleTest::GetTaskCount(sptr<SceneSession>& session)
{
    std::string dumpInfo = session->handler_->GetEventRunner()->GetEventQueue()->DumpCurrentQueueSize();
    std::regex pattern("\\d+");
    std::smatch matches;
    int32_t taskNum = 0;
    while (std::regex_search(dumpInfo, matches, pattern)) {
        taskNum += std::stoi(matches.str());
        dumpInfo = matches.suffix();
    }
    return taskNum;
}

namespace {
/**
 * @tc.name: NotifySessionForeground
 * @tc.desc: SceneSesionManager NotifySessionForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, NotifySessionForeground, TestSize.Level1)
{
    sptr<SceneSession> sceneSession = nullptr;
    SessionInfo info;
    info.bundleName_ = "bundleName";
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    uint32_t reason = 1;
    bool withAnimation = true;
    sceneSession->NotifySessionForeground(reason, withAnimation);
}

/**
 * @tc.name: NotifySessionForeground
 * @tc.desc: SceneSesionManager NotifySessionForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, NotifySessionBackground, TestSize.Level1)
{
    sptr<SceneSession> sceneSession = nullptr;
    SessionInfo info;
    info.bundleName_ = "bundleName";
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    uint32_t reason = 1;
    bool withAnimation = true;
    bool isFromInnerkits = true;
    sceneSession->NotifySessionBackground(reason, withAnimation, isFromInnerkits);
}

/**
 * @tc.name: NotifySessionMovedToFront
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, NotifySessionMovedToFront, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->NotifySessionMovedToFront(0);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ssm_->NotifySessionMovedToFront(100);
}

/**
 * @tc.name: PendingSessionToBackgroundForDelegator
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, PendingSessionToBackgroundForDelegator, TestSize.Level1)
{
    WSError ret;
    ret = ssm_->PendingSessionToBackgroundForDelegator(nullptr, true);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ret = ssm_->PendingSessionToBackgroundForDelegator(nullptr, true);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: OnSessionStateChange
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, OnSessionStateChange, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    ssm_->OnSessionStateChange(100, SessionState::STATE_END);
    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ssm_->OnSessionStateChange(100, SessionState::STATE_END);
    ssm_->OnSessionStateChange(100, SessionState::STATE_FOREGROUND);
    ssm_->OnSessionStateChange(0, SessionState::STATE_FOREGROUND);
    sceneSession->focusedOnShow_ = false;
    ssm_->OnSessionStateChange(0, SessionState::STATE_FOREGROUND);
    ssm_->OnSessionStateChange(100, SessionState::STATE_BACKGROUND);
    ssm_->OnSessionStateChange(0, SessionState::STATE_BACKGROUND);

    sptr<WindowSessionProperty> property_ = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property_);
    property_->type_ = WindowType::APP_MAIN_WINDOW_END;
    sceneSession->property_ = property_;
    ssm_->OnSessionStateChange(100, SessionState::STATE_END);
    ssm_->OnSessionStateChange(100, SessionState::STATE_FOREGROUND);
    ssm_->OnSessionStateChange(0, SessionState::STATE_FOREGROUND);
    ssm_->OnSessionStateChange(100, SessionState::STATE_BACKGROUND);
    ssm_->OnSessionStateChange(0, SessionState::STATE_BACKGROUND);
    ASSERT_NE(nullptr, ssm_);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession
 * @tc.desc: check func RecoverAndReconnectSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RecoverAndReconnectSceneSession, TestSize.Level1)
{
    sptr<ISession> session;
    auto result = ssm_->RecoverAndReconnectSceneSession(nullptr, nullptr, nullptr, session, nullptr, nullptr);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    std::vector<int32_t> recoveredPersistentIds = { 0, 1, 2 };
    ssm_->SetAlivePersistentIds(recoveredPersistentIds);
    property->SetPersistentId(1);
    result = ssm_->RecoverAndReconnectSceneSession(nullptr, nullptr, nullptr, session, property, nullptr);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession
 * @tc.desc: check func RecoverAndReconnectSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RecoverAndReconnectSceneSession02, TestSize.Level1)
{
    sptr<ISession> session;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    std::vector<int32_t> recoveredPersistentIds = { 0, 1, 2 };
    ssm_->SetAlivePersistentIds(recoveredPersistentIds);
    property->SetPersistentId(1);
    ProcessShiftFocusFunc shiftFocusFunc_;
    auto result = ssm_->RecoverAndReconnectSceneSession(nullptr, nullptr, nullptr, session, property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    result = ssm_->RecoverAndReconnectSceneSession(nullptr, nullptr, nullptr, session, property, nullptr);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: CreateSceneSession
 * @tc.desc: CreateSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, CreateSceneSession, TestSize.Level1)
{
    SessionInfo info;
    ssm_->CreateSceneSession(info, nullptr);
    info.isSystem_ = true;
    info.windowType_ = 3000;
    ssm_->CreateSceneSession(info, nullptr);
    info.windowType_ = 3;
    ssm_->CreateSceneSession(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sptr<SceneSession> getSceneSession1 = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(nullptr, getSceneSession1);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    info.windowType_ = 1001;
    sptr<SceneSession> getSceneSession2 = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(nullptr, getSceneSession2);
    ASSERT_EQ(WindowType::WINDOW_TYPE_APP_SUB_WINDOW, getSceneSession2->GetWindowType());

    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    info.windowType_ = 2105;
    sptr<SceneSession> getSceneSession3 = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(nullptr, getSceneSession3);
    ASSERT_EQ(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, getSceneSession3->GetWindowType());

    property->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    info.windowType_ = 2500;
    sptr<SceneSession> getSceneSession4 = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(nullptr, getSceneSession4);
    ASSERT_EQ(WindowType::SYSTEM_SUB_WINDOW_BASE, getSceneSession4->GetWindowType());

    ssm_->NotifySessionUpdate(info, ActionType::SINGLE_START, 0);
    info.persistentId_ = 0;
    ssm_->UpdateSceneSessionWant(info);
    info.persistentId_ = 1;
    ssm_->UpdateSceneSessionWant(info);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->UpdateSceneSessionWant(info);
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    ASSERT_NE(nullptr, want);
    info.want = want;
    ssm_->UpdateSceneSessionWant(info);
    sceneSession->SetCollaboratorType(0);
    ssm_->UpdateSceneSessionWant(info);
    sceneSession->SetCollaboratorType(1);
    ssm_->UpdateSceneSessionWant(info);
    ssm_->sceneSessionMap_.erase(1);
    sptr<SceneSession> sceneSession1;
    ssm_->UpdateCollaboratorSessionWant(sceneSession1, 1);
    ssm_->UpdateCollaboratorSessionWant(sceneSession, 1);
    SessionInfo info1;
    info1.ancoSceneState = 0;
    sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    ssm_->UpdateCollaboratorSessionWant(sceneSession1, 1);
    ASSERT_EQ(CollaboratorType::DEFAULT_TYPE, sceneSession1->GetCollaboratorType());
}

/**
 * @tc.name: RequestSceneSessionBackground
 * @tc.desc: RequestSceneSessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionBackground, TestSize.Level1)
{
    sptr<SceneSession> sceneSession;
    ssm_->RequestSceneSessionActivation(sceneSession, true);
    SessionInfo info;
    sptr<AAFwk::SessionInfo> abilitySessionInfo;
    ssm_->NotifyCollaboratorAfterStart(sceneSession, abilitySessionInfo);
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, true, false));
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, true, true));
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, false, true));
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, false, false));
    ssm_->NotifyForegroundInteractiveStatus(sceneSession, true);
    ssm_->NotifyForegroundInteractiveStatus(sceneSession, false);
    ssm_->DestroyDialogWithMainWindow(sceneSession);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, true, false));
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, true, true));
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, false, true));
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, false, false));
    ssm_->NotifyForegroundInteractiveStatus(sceneSession, true);
    ssm_->NotifyForegroundInteractiveStatus(sceneSession, false);
    ssm_->NotifyCollaboratorAfterStart(sceneSession, abilitySessionInfo);
    ssm_->DestroyDialogWithMainWindow(sceneSession);
    ssm_->DestroySubSession(sceneSession);
    ssm_->SetAbilitySessionInfo(sceneSession);
    ssm_->RequestSceneSessionActivation(sceneSession, true);
    ssm_->RequestInputMethodCloseKeyboard(1);
    ssm_->sceneSessionMap_.insert({ 1, nullptr });
    ssm_->RequestInputMethodCloseKeyboard(1);
    ssm_->RequestSceneSessionActivation(sceneSession, true);
    ssm_->RequestSceneSessionActivation(sceneSession, false);
    ssm_->StartUIAbilityBySCB(sceneSession);
    ssm_->RequestSceneSessionActivation(sceneSession, true);
    abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    ASSERT_NE(nullptr, abilitySessionInfo);
    ssm_->NotifyCollaboratorAfterStart(sceneSession, abilitySessionInfo);
    ssm_->DestroySubSession(sceneSession);
    ssm_->EraseSceneSessionMapById(1);
    ASSERT_EQ(nullptr, ssm_->GetSceneSession(1));
}

/**
 * @tc.name: RequestSceneSessionDestruction
 * @tc.desc: RequestSceneSessionDestruction
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionDestruction, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "111";
    info.bundleName_ = "222";
    sptr<SceneSession> sceneSession;
    sptr<ISessionStage> sessionStage;
    ssm_->AddClientDeathRecipient(sessionStage, sceneSession);
    ssm_->RequestSceneSessionDestruction(sceneSession, true);
    ssm_->RequestSceneSessionDestruction(sceneSession, false);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->AddClientDeathRecipient(sessionStage, sceneSession);
    ssm_->RequestSceneSessionDestruction(sceneSession, true);
    ssm_->RequestSceneSessionDestruction(sceneSession, false);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->RequestSceneSessionDestruction(sceneSession, true);
    ssm_->RequestSceneSessionDestruction(sceneSession, false);
    ssm_->sceneSessionMap_.erase(sceneSession->GetPersistentId());
    sptr<AAFwk::SessionInfo> sceneSessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    ASSERT_NE(nullptr, sceneSessionInfo);
    ssm_->RequestSceneSessionDestructionInner(sceneSession, sceneSessionInfo, true);
    ssm_->RequestSceneSessionDestructionInner(sceneSession, sceneSessionInfo, false);
    std::shared_ptr<SessionListenerController> listenerController = std::make_shared<SessionListenerController>();
    ASSERT_NE(nullptr, listenerController);
    ssm_->listenerController_ = listenerController;
    ssm_->RequestSceneSessionDestructionInner(sceneSession, sceneSessionInfo, true);
    ssm_->RequestSceneSessionDestructionInner(sceneSession, sceneSessionInfo, false);
    ssm_->AddClientDeathRecipient(sessionStage, sceneSession);
    ASSERT_EQ(AncoSceneState::DEFAULT_STATE, sceneSession->GetSessionInfo().ancoSceneState);
}

/**
 * @tc.name: RequestSceneSessionBackground002
 * @tc.desc: SceneSesionManager request scene session background
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionBackground002, TestSize.Level1)
{
    bool isDelegator = false;
    SessionInfo info;
    info.abilityName_ = "RequestSceneSessionBackground002";
    info.bundleName_ = "RequestSceneSessionBackground002";
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, isDelegator));
}

/**
 * @tc.name: RequestSceneSessionDestruction002
 * @tc.desc: SceneSesionManager request scene session destruction
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionDestruction002, TestSize.Level1)
{
    bool needRemoveSession = false;
    SessionInfo info;
    info.abilityName_ = "RequestSceneSessionDestruction002";
    info.bundleName_ = "RequestSceneSessionDestruction002";
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionDestruction(sceneSession, needRemoveSession));
}

/**
 * @tc.name: NotifyForegroundInteractiveStatus
 * @tc.desc: SceneSesionManager notify scene session interactive status
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, NotifyForegroundInteractiveStatus, TestSize.Level1)
{
    bool interactive = false;
    SessionInfo info;
    info.abilityName_ = "NotifyForegroundInteractiveStatus";
    info.bundleName_ = "NotifyForegroundInteractiveStatus";
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->NotifyForegroundInteractiveStatus(sceneSession, interactive);
}

/**
 * @tc.name: NotifySessionCreate
 * @tc.desc: SceneSesionManager notify session create
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, NotifySessionCreate, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySessionCreate";
    info.bundleName_ = "NotifySessionCreate";
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->NotifySessionCreate(nullptr, info);
    EXPECT_EQ(info.want, nullptr);
    ssm_->NotifySessionCreate(sceneSession, info);
    info.want = std::make_shared<AAFwk::Want>();
    sceneSession = nullptr;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->NotifySessionCreate(sceneSession, info);
    EXPECT_NE(info.want, nullptr);
    AppExecFwk::AbilityInfo aInfo;
    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>(aInfo);
    ASSERT_NE(nullptr, abilityInfo);
    int32_t collaboratorType = CollaboratorType::RESERVE_TYPE;
    ssm_->NotifyLoadAbility(collaboratorType, abilitySessionInfo, abilityInfo);
}

/**
 * @tc.name: UpdateRecoveredSessionInfo
 * @tc.desc: SceneSessionManager load window scene xml
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, UpdateRecoveredSessionInfo02, TestSize.Level1)
{
    std::vector<int32_t> recoveredPersistentIds;
    ASSERT_NE(ssm_, nullptr);
    ssm_->UpdateRecoveredSessionInfo(recoveredPersistentIds);
    recoveredPersistentIds.push_back(0);
    ssm_->UpdateRecoveredSessionInfo(recoveredPersistentIds);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    sptr<KeyboardSession::SpecificSessionCallback> specificCallback;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 0, sceneSession });
    ssm_->UpdateRecoveredSessionInfo(recoveredPersistentIds);
    constexpr uint32_t WAIT_SYNC_IN_NS = 50000;
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: RequestSceneSession01
 * @tc.desc: SceneSesionManager test RequestSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSession01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestSceneSession01";
    info.bundleName_ = "RequestSceneSession01";
    info.persistentId_ = 0;
    auto windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();

    sptr<SceneSession> sceneSession = ssm_->RequestSceneSession(info, windowSessionProperty);
    ASSERT_EQ(sceneSession, nullptr);
}

/**
 * @tc.name: RequestSceneSession02
 * @tc.desc: SceneSesionManager test RequestSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSession02, TestSize.Level1)
{
    SessionInfo info1;
    info1.persistentId_ = 1;
    info1.bundleName_ = "RequestSceneSession";
    info1.isPersistentRecover_ = false;

    auto windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    SessionInfo info2;
    info2.abilityName_ = "RequestSceneSession02";
    info2.bundleName_ = "RequestSceneSession02";
    info2.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    sptr<SceneSession> getSceneSession = ssm_->RequestSceneSession(info2, windowSessionProperty);
    ASSERT_NE(getSceneSession->GetSessionInfo().bundleName_, info2.bundleName_);
}

/**
 * @tc.name: RequestSceneSession03
 * @tc.desc: SceneSesionManager test RequestSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSession03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestSceneSession";
    info.bundleName_ = "RequestSceneSession";
    info.persistentId_ = 1;
    info.isPersistentRecover_ = true;
    auto windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    auto sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });

    sptr<SceneSession> getSceneSession = ssm_->RequestSceneSession(info, windowSessionProperty);
    ASSERT_EQ(getSceneSession, nullptr);
}

/**
 * @tc.name: RequestSceneSession04
 * @tc.desc: SceneSesionManager test RequestSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSession04, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestSceneSession";
    info.bundleName_ = "RequestSceneSession";
    info.persistentId_ = 1;
    info.isPersistentRecover_ = true;
    auto windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();

    sptr<SceneSession> sceneSession = ssm_->RequestSceneSession(info, windowSessionProperty);
    ASSERT_EQ(sceneSession, nullptr);
}

/**
 * @tc.name: RequestSceneSessionBackground01
 * @tc.desc: SceneSesionManager test RequestSceneSessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionBackground01, TestSize.Level1)
{
    bool isDelegator = false;
    bool isToDesktop = false;
    bool isSaveSnapshot = true;
    SessionInfo info;
    info.abilityName_ = "RequestSceneSessionBackground";
    info.bundleName_ = "RequestSceneSessionBackground";
    info.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ASSERT_EQ(ssm_->RequestSceneSessionBackground(sceneSession, isDelegator, isToDesktop, isSaveSnapshot),
              WSError::WS_OK);
}

/**
 * @tc.name: RequestSceneSessionBackground02
 * @tc.desc: SceneSesionManager test RequestSceneSessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionBackground02, TestSize.Level1)
{
    bool isDelegator = false;
    bool isToDesktop = true;
    bool isSaveSnapshot = true;
    SessionInfo info;
    info.abilityName_ = "RequestSceneSessionBackground";
    info.bundleName_ = "RequestSceneSessionBackground";
    info.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ASSERT_EQ(ssm_->RequestSceneSessionBackground(sceneSession, isDelegator, isToDesktop, isSaveSnapshot),
              WSError::WS_OK);
}

/**
 * @tc.name: RequestSceneSessionBackground03
 * @tc.desc: SceneSesionManager test RequestSceneSessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionBackground03, TestSize.Level1)
{
    bool isDelegator = false;
    bool isToDesktop = true;
    bool isSaveSnapshot = true;
    SessionInfo info;
    info.abilityName_ = "RequestSceneSessionBackground";
    info.bundleName_ = "RequestSceneSessionBackground";
    info.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->SetBrightness(sceneSession, 0.5);
    ASSERT_EQ(ssm_->RequestSceneSessionBackground(sceneSession, isDelegator, isToDesktop, isSaveSnapshot),
              WSError::WS_OK);
}

/**
 * @tc.name: RequestSceneSessionBackground04
 * @tc.desc: SceneSesionManager test RequestSceneSessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionBackground04, TestSize.Level1)
{
    bool isDelegator = true;
    bool isToDesktop = true;
    bool isSaveSnapshot = true;
    SessionInfo info;
    info.abilityName_ = "RequestSceneSessionBackground";
    info.bundleName_ = "RequestSceneSessionBackground";
    info.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ASSERT_EQ(ssm_->RequestSceneSessionBackground(sceneSession, isDelegator, isToDesktop, isSaveSnapshot),
              WSError::WS_OK);
}

/**
 * @tc.name: RequestSceneSessionDestruction01
 * @tc.desc: SceneSesionManager test RequestSceneSessionDestruction
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionDestruction01, TestSize.Level1)
{
    bool needRemoveSession = false;
    SessionInfo info;
    info.abilityName_ = "RequestSceneSessionDestruction";
    info.bundleName_ = "RequestSceneSessionDestruction";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ASSERT_EQ(ssm_->RequestSceneSessionDestruction(sceneSession, needRemoveSession), WSError::WS_OK);
}

/**
 * @tc.name: RequestSceneSessionDestruction02
 * @tc.desc: SceneSesionManager test RequestSceneSessionDestruction
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionDestruction02, TestSize.Level1)
{
    bool needRemoveSession = false;
    SessionInfo info;
    info.abilityName_ = "RequestSceneSessionDestruction";
    info.bundleName_ = "RequestSceneSessionDestruction";
    info.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ASSERT_EQ(ssm_->RequestSceneSessionDestruction(sceneSession, needRemoveSession), WSError::WS_OK);
}

/**
 * @tc.name: StartOrMinimizeUIAbilityBySCB1
 * @tc.desc: Normal test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, StartOrMinimizeUIAbilityBySCB1, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "StartOrMinimizeUIAbilityBySCB1";
    info.bundleName_ = "StartOrMinimizeUIAbilityBySCB1";
    ssm_->systemConfig_.backgroundswitch = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sceneSession->SetSessionProperty(property);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetIsAppSupportPhoneInPc(false);

    sceneSession->SetSessionState(SessionState::STATE_BACKGROUND);
    sceneSession->SetMinimizedFlagByUserSwitch(true);

    WSError ret = ssm_->StartOrMinimizeUIAbilityBySCB(sceneSession, true);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: StartOrMinimizeUIAbilityBySCB2
 * @tc.desc: Normal test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, StartOrMinimizeUIAbilityBySCB2, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "StartOrMinimizeUIAbilityBySCB2";
    info.bundleName_ = "StartOrMinimizeUIAbilityBySCB2";
    ssm_->systemConfig_.backgroundswitch = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sceneSession->SetSessionProperty(property);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetIsAppSupportPhoneInPc(false);

    sceneSession->SetSessionState(SessionState::STATE_BACKGROUND);
    WSError ret = ssm_->StartOrMinimizeUIAbilityBySCB(sceneSession, false);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: RequestSceneSessionByCall01
 * @tc.desc: SceneSesionManager test RequestSceneSessionByCall
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionByCall01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestSceneSessionByCall";
    info.bundleName_ = "RequestSceneSessionByCall";
    info.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ASSERT_EQ(ssm_->RequestSceneSessionByCall(sceneSession), WSError::WS_OK);
}

/**
 * @tc.name: RequestSceneSessionByCall02
 * @tc.desc: SceneSesionManager test RequestSceneSessionByCall
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionByCall02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestSceneSessionByCall";
    info.bundleName_ = "RequestSceneSessionByCall";
    info.persistentId_ = 1;
    info.callState_ = static_cast<uint32_t>(AAFwk::CallToState::BACKGROUND);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ASSERT_EQ(ssm_->RequestSceneSessionByCall(sceneSession), WSError::WS_OK);
}

/**
 * @tc.name: RequestSceneSessionByCall03
 * @tc.desc: SceneSesionManager test RequestSceneSessionByCall
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RequestSceneSessionByCall03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestSceneSessionByCall";
    info.bundleName_ = "RequestSceneSessionByCall";
    info.persistentId_ = 1;
    info.callState_ = static_cast<uint32_t>(AAFwk::CallToState::FOREGROUND);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ASSERT_EQ(ssm_->RequestSceneSessionByCall(sceneSession), WSError::WS_OK);
}

/**
 * @tc.name: ClearSession
 * @tc.desc: ClearSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, ClearSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, ssm_->ClearSession(nullptr));
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, ssm_->ClearSession(sceneSession));
}

/**
 * @tc.name: RegisterVisibilityChangedDetectFunc
 * @tc.desc: RegisterVisibilityChangedDetectFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RegisterVisibilityChangedDetectFunc, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "VisibilityChanged";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    ssm_->RegisterVisibilityChangedDetectFunc(nullptr);
    EXPECT_EQ(nullptr, sceneSession->visibilityChangedDetectFunc_);

    ssm_->RegisterVisibilityChangedDetectFunc(sceneSession);
    EXPECT_NE(nullptr, sceneSession->visibilityChangedDetectFunc_);
}

/**
 * @tc.name: RecoveryVisibilityPidCount
 * @tc.desc: RecoveryVisibilityPidCount
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, RecoveryVisibilityPidCount, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    int32_t pid = 10;
    SessionInfo info;
    info.abilityName_ = "VisibilityChanged";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    sceneSession->SetCallingPid(pid);

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    ssm_->sceneSessionMap_.insert({ 2, sceneSession2 });

    sceneSession2->SetCallingPid(pid);
    sceneSession2->isVisible_ = true;

    ssm_->RecoveryVisibilityPidCount(pid);
    EXPECT_EQ(1, ssm_->visibleWindowCountMap_[pid]);
}

/**
 * @tc.name: VisibilityChangedDetectFunc
 * @tc.desc: VisibilityChangedDetectFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, VisibilityChangedDetectFunc, TestSize.Level1)
{
    int32_t pid = 20;
    SessionInfo info;
    info.abilityName_ = "VisibilityChanged";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    ASSERT_NE(nullptr, ssm_);
    ssm_->RegisterVisibilityChangedDetectFunc(sceneSession1);
    ssm_->RegisterVisibilityChangedDetectFunc(sceneSession2);
    EXPECT_EQ(0, ssm_->visibleWindowCountMap_[pid]);

    sceneSession1->visibilityChangedDetectFunc_(pid, false, true);
    EXPECT_EQ(1, ssm_->visibleWindowCountMap_[pid]);

    sceneSession2->visibilityChangedDetectFunc_(pid, false, true);
    EXPECT_EQ(2, ssm_->visibleWindowCountMap_[pid]);

    sceneSession1->visibilityChangedDetectFunc_(pid, true, false);
    EXPECT_EQ(1, ssm_->visibleWindowCountMap_[pid]);

    sceneSession2->visibilityChangedDetectFunc_(pid, true, false);
    EXPECT_EQ(0, ssm_->visibleWindowCountMap_[pid]);
}

/**
 * @tc.name: StartUIAbilityBySCBTimeoutCheck
 * @tc.desc: StartUIAbilityBySCBTimeoutCheck
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, StartUIAbilityBySCBTimeoutCheck, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";
    sessionInfo.ancoSceneState = AncoSceneState::NOTIFY_FOREGROUND;
    ASSERT_NE(ssm_, nullptr);
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    sceneSession->SetScbCoreEnabled(true);
    sptr<AAFwk::SessionInfo> abilitySessionInfo = ssm_->SetAbilitySessionInfo(sceneSession);
    ASSERT_NE(abilitySessionInfo, nullptr);
    bool isColdStart = true;
    ASSERT_EQ(ssm_->StartUIAbilityBySCBTimeoutCheck(sceneSession,
                  abilitySessionInfo, static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL), isColdStart),
              2097202);
    ASSERT_EQ(ssm_->StartUIAbilityBySCBTimeoutCheck(sceneSession,
                  abilitySessionInfo, static_cast<uint32_t>(WindowStateChangeReason::USER_SWITCH), isColdStart),
              2097202);
}

/**
 * @tc.name: SubmitTaskAndWait
 * @tc.desc: SubmitTaskAndWait
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest, SubmitTaskAndWait, TestSize.Level1)
{
    uint64_t timeout = 3000;
    auto ffrtQueueHelper = std::make_shared<FfrtQueueHelper>();
    ASSERT_NE(ffrtQueueHelper, nullptr);
    auto timeoutTask = []() {
        std::cout << "===enter timeoutTask begin sleep 4 sec===" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(4));
    };
    ASSERT_EQ(ffrtQueueHelper->SubmitTaskAndWait(std::move(timeoutTask), timeout), true);
    auto task = []() {
        std::cout << "===enter task begin sleep 1 sec===" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };
    ASSERT_EQ(ffrtQueueHelper->SubmitTaskAndWait(std::move(task), timeout), false);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
