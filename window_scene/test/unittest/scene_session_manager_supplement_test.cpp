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
#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>

#include "interfaces/include/ws_common.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "scene_board_judgement.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "window_manager_agent.h"
#include "session_manager.h"
#include "zidl/window_manager_agent_interface.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SceneSessionManagerSupplementTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerSupplementTest::ssm_ = nullptr;

void SceneSessionManagerSupplementTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerSupplementTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerSupplementTest::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerSupplementTest::TearDown()
{
    ssm_->sceneSessionMap_.clear();
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: LoadFreeMultiWindowConfigWithTrue
 * @tc.desc: Test LoadFreeMultiWindowConfig for input true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, LoadFreeMultiWindowConfigWithTrue, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowEnable_ = false;
    ssm_->LoadFreeMultiWindowConfig(true);
    auto config = ssm_->GetSystemSessionConfig();
    ASSERT_EQ(config.freeMultiWindowEnable_, true);
}

/**
 * @tc.name: LoadFreeMultiWindowConfigWithFalse
 * @tc.desc: Test LoadFreeMultiWindowConfig for input false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, LoadFreeMultiWindowConfigWithFalse, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowEnable_ = true;
    ssm_->LoadFreeMultiWindowConfig(false);
    auto config = ssm_->GetSystemSessionConfig();
    ASSERT_EQ(config.freeMultiWindowEnable_, false);
}

/**
 * @tc.name: SwitchFreeMultiWindowWithTrue
 * @tc.desc: Test LoadFreeMultiWindowConfig for input false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, SwitchFreeMultiWindowWithTrue, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowSupport_ = false;
    auto res = ssm_->SwitchFreeMultiWindow(true);
    ASSERT_EQ(res, WSError::WS_ERROR_DEVICE_NOT_SUPPORT);
}

/**
 * @tc.name: SwitchFreeMultiWindowWithFalse
 * @tc.desc: Test LoadFreeMultiWindowConfig for input false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, SwitchFreeMultiWindowWithFalse, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;

    auto res = ssm_->SwitchFreeMultiWindow(true);
    ASSERT_EQ(res, WSError::WS_OK);
    auto config = ssm_->GetSystemSessionConfig();
    ASSERT_EQ(config.freeMultiWindowEnable_, true);
}

/**
 * @tc.name: TestSwitchWindowWithNullSceneSession
 * @tc.desc: Test LoadFreeMultiWindowConfig with sceneSession is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestSwitchWindowWithNullSceneSession, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;
    sptr<SceneSession> sceneSession;
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });

    auto res = ssm_->SwitchFreeMultiWindow(false);
    ASSERT_EQ(res, WSError::WS_OK);
    auto config = ssm_->GetSystemSessionConfig();
    ASSERT_EQ(config.freeMultiWindowEnable_, false);
}

/**
 * @tc.name: TestSwitchWindowWithSceneSession
 * @tc.desc: Test LoadFreeMultiWindowConfig with sceneSession is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestSwitchWindowWithSceneSession, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });

    auto res = ssm_->SwitchFreeMultiWindow(false);
    ASSERT_EQ(res, WSError::WS_OK);
    auto config = ssm_->GetSystemSessionConfig();
    ASSERT_EQ(config.freeMultiWindowEnable_, false);
}

/**
 * @tc.name: TestSwitchWindowWithProperty
 * @tc.desc: Test LoadFreeMultiWindowConfig with property is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestSwitchWindowWithProperty, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);
    sceneSession->SetSessionProperty(property);

    auto res = ssm_->SwitchFreeMultiWindow(false);
    ASSERT_EQ(res, WSError::WS_OK);
    auto config = ssm_->GetSystemSessionConfig();
    ASSERT_EQ(config.freeMultiWindowEnable_, false);
}

/**
 * @tc.name: TestSwitchWindowWithPropertyInputTrue
 * @tc.desc: Test LoadFreeMultiWindowConfig with property is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestSwitchWindowWithPropertyInputTrue, TestSize.Level1)
{
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);
    sceneSession->SetSessionProperty(property);

    auto res = ssm_->SwitchFreeMultiWindow(true);
    ASSERT_EQ(res, WSError::WS_OK);
    auto config = ssm_->GetSystemSessionConfig();
    ASSERT_EQ(config.freeMultiWindowEnable_, true);
}

/**
 * @tc.name: TestSetEnableInputEventWithInputTrue
 * @tc.desc: Test LoadFreeMultiWindowConfig for input true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestSetEnableInputEventWithInputTrue, TestSize.Level1)
{
    ssm_->enableInputEvent_ = false;
    ssm_->SetEnableInputEvent(true);
    ASSERT_EQ(ssm_->enableInputEvent_, true);
    auto res = ssm_->IsInputEventEnabled();
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: RequestSceneSessionActivationInner
 * @tc.desc: SceneSessionManagerSupplementTest RequestSceneSessionActivationInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, RequestSceneSessionActivationInner, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    sceneSession->SetCollaboratorType(0);
    auto ret = ssm_->RequestSceneSessionActivationInner(sceneSession, true);
    ASSERT_EQ(ret, WSError::WS_OK);
    ret = ssm_->RequestSceneSessionActivationInner(sceneSession, false);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: RequestSceneSessionActivationInnerTest01
 * @tc.desc: SceneSessionManagerSupplementTest RequestSceneSessionActivationInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, RequestSceneSessionActivationInnerTest_01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";
    sessionInfo.ancoSceneState = AncoSceneState::NOTIFY_FOREGROUND;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    sceneSession->SetScbCoreEnabled(true);
    auto ret = ssm_->RequestSceneSessionActivationInner(sceneSession, true);
    ASSERT_EQ(ret, WSError::WS_OK);
    ret = ssm_->RequestSceneSessionActivationInner(sceneSession, false);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: RequestSceneSessionActivationInnerTest_02
 * @tc.desc: SceneSessionManagerSupplementTest RequestSceneSessionActivationInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, RequestSceneSessionActivationInnerTest_02,
        TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "testBundleName";
    sessionInfo.abilityName_ = "testAbilityName";
    sessionInfo.ancoSceneState = AncoSceneState::NOTIFY_START_FAILED;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    sceneSession->SetScbCoreEnabled(true);
    auto ret = ssm_->RequestSceneSessionActivationInner(sceneSession, true);
    ASSERT_EQ(ret, WSError::WS_ERROR_PRE_HANDLE_COLLABORATOR_FAILED);
}

/**
 * @tc.name: RequestSceneSessionActivationInnerTest_03
 * @tc.desc: SceneSessionManagerSupplementTest RequestSceneSessionActivationInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, RequestSceneSessionActivationInnerTest_03, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "testBundleName";
    sessionInfo.abilityName_ = "testAbilityName";
    sessionInfo.ancoSceneState = AncoSceneState::NOTIFY_START_FAILED;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetScbCoreEnabled(false);
    auto ret = ssm_->RequestSceneSessionActivationInner(sceneSession, true);
    ASSERT_EQ(ret, WSError::WS_ERROR_PRE_HANDLE_COLLABORATOR_FAILED);

    sceneSession->SetScbCoreEnabled(true);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->SetRestartApp(false);
    ret = ssm_->RequestSceneSessionActivationInner(sceneSession, true);
    ASSERT_EQ(ret, WSError::WS_ERROR_PRE_HANDLE_COLLABORATOR_FAILED);

    sceneSession->SetRestartApp(true);
    ret = ssm_->RequestSceneSessionActivationInner(sceneSession, true);
    ASSERT_EQ(ret, WSError::WS_ERROR_PRE_HANDLE_COLLABORATOR_FAILED);

    sceneSession->isVisible_ = false;
    ret = ssm_->RequestSceneSessionActivationInner(sceneSession, true);
    ASSERT_EQ(ret, WSError::WS_ERROR_PRE_HANDLE_COLLABORATOR_FAILED);
}

/**
 * @tc.name: NotifyCollaboratorAfterStart
 * @tc.desc: SceneSessionManagerSupplementTest NotifyCollaboratorAfterStart
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, NotifyCollaboratorAfterStart, TestSize.Level1)
{
    sptr<SceneSession> sceneSession;
    sptr<AAFwk::SessionInfo> sceneSessionInfo;
    ssm_->NotifyCollaboratorAfterStart(sceneSession, sceneSessionInfo);
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    auto ret = ssm_->RequestSceneSessionBackground(sceneSession, true, true);
    ASSERT_EQ(ret, WSError::WS_OK);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->NotifyCollaboratorAfterStart(sceneSession, sceneSessionInfo);
    sceneSessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    ssm_->NotifyCollaboratorAfterStart(sceneSession, sceneSessionInfo);
    ssm_->NotifyCollaboratorAfterStart(sceneSession, sceneSessionInfo);
    ret = ssm_->RequestSceneSessionBackground(sceneSession, true, true);
    ASSERT_EQ(ret, WSError::WS_OK);
    ret = ssm_->RequestSceneSessionBackground(sceneSession, true, false);
    ASSERT_EQ(ret, WSError::WS_OK);
    ssm_->brightnessSessionId_ = sceneSession->GetPersistentId();
    ret = ssm_->RequestSceneSessionBackground(sceneSession, false, true);
    ASSERT_EQ(ret, WSError::WS_OK);
    ret = ssm_->RequestSceneSessionBackground(sceneSession, false, false);
    ASSERT_EQ(ret, WSError::WS_OK);
    ssm_->brightnessSessionId_ = 0;
    ssm_->systemConfig_.backgroundswitch = true;
    ret = ssm_->RequestSceneSessionBackground(sceneSession, true, true);
    ASSERT_EQ(ret, WSError::WS_OK);
    ret = ssm_->RequestSceneSessionBackground(sceneSession, true, false);
    ASSERT_EQ(ret, WSError::WS_OK);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    ret = ssm_->RequestSceneSessionBackground(sceneSession, false, true);
    ASSERT_EQ(ret, WSError::WS_OK);
    ret = ssm_->RequestSceneSessionBackground(sceneSession, false, false);
    ASSERT_EQ(ret, WSError::WS_OK);
    ssm_->NotifyForegroundInteractiveStatus(sceneSession, true);
    ssm_->NotifyForegroundInteractiveStatus(sceneSession, false);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->NotifyForegroundInteractiveStatus(sceneSession, true);
}

/**
 * @tc.name: TestDestroyDialogWithMainWindow_01
 * @tc.desc: Test DestroyDialogWithMainWindow with sceneSession is null
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestDestroyDialogWithMainWindow_01, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession;
    auto res = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TestDestroyDialogWithMainWindow_02
 * @tc.desc: Test DestroyDialogWithMainWindow with window type is WINDOW_TYPE_APP_COMPONENT
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestDestroyDialogWithMainWindow_02, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession;
    ASSERT_NE(ssm_, nullptr);
    ssm_->DestroySubSession(sceneSession);
    ssm_->RequestSceneSessionDestruction(sceneSession, true);
    ssm_->RequestSceneSessionDestruction(sceneSession, false);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_COMPONENT);
    sceneSession->SetSessionProperty(property);
    auto res = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: TestDestroyDialogWithMainWindow_03
 * @tc.desc: Test DestroyDialogWithMainWindow with subSession is null
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestDestroyDialogWithMainWindow_03, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    sptr<SceneSession> sceneSession2;
    sceneSession->dialogVec_.push_back(sceneSession2);
    sceneSession->subSession_.push_back(sceneSession2);
    auto res = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: TestDestroyDialogWithMainWindow_04
 * @tc.desc: Test DestroyDialogWithMainWindow with subSession is not null
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestDestroyDialogWithMainWindow_04, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    sptr<SceneSession> sceneSession2;
    sceneSession->dialogVec_.push_back(sceneSession2);
    sceneSession->subSession_.push_back(sceneSession2);
    sceneSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    ssm_->DestroySubSession(sceneSession);
    auto res = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: TestDestroyDialogWithMainWindow_05
 * @tc.desc: Test DestroyDialogWithMainWindow with insert sceneSession2 in sceneSessionMap_
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestDestroyDialogWithMainWindow_05, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    sptr<SceneSession> sceneSession2;
    sceneSession->dialogVec_.push_back(sceneSession2);
    sceneSession->subSession_.push_back(sceneSession2);
    sceneSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    ssm_->DestroySubSession(sceneSession);
    ssm_->RequestSceneSessionDestruction(sceneSession, true);
    ssm_->RequestSceneSessionDestruction(sceneSession, false);
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });
    auto res = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: DestroyDialogWithSessionIsNull
 * @tc.desc: DestroyDialogWithMainWindow with with session is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, DestroyDialogWithSessionIsNull, TestSize.Level1)
{
    sptr<SceneSession> sceneSession;
    auto res = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: DestroyDialogWithFalseType
 * @tc.desc: DestroyDialogWithMainWindow with type is WINDOW_TYPE_APP_COMPONENT
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, DestroyDialogWithFalseType, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_COMPONENT);
    sceneSession->SetSessionProperty(property);
    auto res = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: DestroyDialogWithTrueType
 * @tc.desc: DestroyDialogWithMainWindow with type is WINDOW_TYPE_APP_MAIN_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, DestroyDialogWithTrueType, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    sptr<SceneSession> sceneSession2;
    sceneSession->dialogVec_.push_back(sceneSession2);
    sceneSession->subSession_.push_back(sceneSession2);
    auto res = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: DestroyDialogWithTrueType_01
 * @tc.desc: DestroyDialogWithMainWindow with type is WINDOW_TYPE_APP_MAIN_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, DestroyDialogWithTrueType_01, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    sptr<SceneSession> sceneSession2;
    sceneSession->dialogVec_.push_back(sceneSession2);
    sceneSession->subSession_.push_back(sceneSession2);
    sceneSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });
    auto res = ssm_->DestroyDialogWithMainWindow(sceneSession);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: TestCreateAndConnectSession_01
 * @tc.desc: Test for CreateAndConnectSpecificSession with property is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCreateAndConnectSession_01, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 0;

    auto res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TestCreateAndConnectSession_02
 * @tc.desc: Test for CreateAndConnectSpecificSession with WindowType is WINDOW_TYPE_UI_EXTENSION
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCreateAndConnectSession_02, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 0;
    property->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);

    auto res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(res, WSError::WS_ERROR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: TestCreateAndConnectSession_03
 * @tc.desc: Test for CreateAndConnectSpecificSession with WindowType is WINDOW_TYPE_INPUT_METHOD_FLOAT
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCreateAndConnectSession_03, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 0;
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);

    auto res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TestCreateAndConnectSession_04
 * @tc.desc: Test for CreateAndConnectSpecificSession with WindowType is WINDOW_TYPE_FLOAT
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCreateAndConnectSession_04, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 0;
    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    property->SetFloatingWindowAppType(true);

    auto res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(res, WSError::WS_ERROR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: TestCreateAndConnectSession_05
 * @tc.desc: Test for CreateAndConnectSpecificSession with WindowType is WINDOW_TYPE_APP_SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCreateAndConnectSession_05, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 0;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->SetFloatingWindowAppType(true);
    property->SetIsUIExtFirstSubWindow(true);

    auto res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TestCreateAndConnectSession_06
 * @tc.desc: Test for CreateAndConnectSpecificSession with WindowType is WINDOW_TYPE_SYSTEM_ALARM_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCreateAndConnectSession_06, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 0;
    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW);
    property->SetFloatingWindowAppType(true);
    property->SetIsUIExtFirstSubWindow(true);

    auto res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: TestCreateAndConnectSession_07
 * @tc.desc: Test for CreateAndConnectSpecificSession with WindowType is WINDOW_TYPE_PIP
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCreateAndConnectSession_07, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 0;
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    property->SetFloatingWindowAppType(true);
    property->SetIsUIExtFirstSubWindow(true);

    auto res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(res, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: TestCreateAndConnectSession_08
 * @tc.desc: Test for CreateAndConnectSpecificSession with WindowType is WINDOW_TYPE_MUTISCREEN_COLLABORATION
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCreateAndConnectSession_08, Function | SmallTest | Level3)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 0;
    property->SetWindowType(WindowType::WINDOW_TYPE_MUTISCREEN_COLLABORATION);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    auto res = ssm_->CreateAndConnectSpecificSession(sessionStage, eventChannel, node, property, id, session,
        systemConfig, token);
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: ClosePipWindowIfExist
 * @tc.desc: ClosePipWindowIfExist
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, ClosePipWindowIfExist, TestSize.Level1)
{
    ssm_->ClosePipWindowIfExist(WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW);
    ssm_->ClosePipWindowIfExist(WindowType::WINDOW_TYPE_PIP);
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_COMPONENT);
    sceneSession->SetSessionProperty(property);
    PiPTemplateInfo pipInfo;
    pipInfo.priority = 0;
    auto res = ssm_->CheckPiPPriority(pipInfo);
    ASSERT_EQ(res, true);
    ssm_->sceneSessionMap_.insert({ 0, sceneSession });
    ssm_->ClosePipWindowIfExist(WindowType::WINDOW_TYPE_PIP);
    res = ssm_->CheckPiPPriority(pipInfo);
    ASSERT_EQ(res, true);
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    PiPTemplateInfo pipInfo1;
    pipInfo1.priority = 0;
    sceneSession->SetPiPTemplateInfo(pipInfo1);
    res = ssm_->CheckPiPPriority(pipInfo);
    ASSERT_EQ(res, true);
    sceneSession->isVisible_ = true;
    res = ssm_->CheckPiPPriority(pipInfo);
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: CheckSysWinPermissionWithUIExtTypeThenFalse
 * @tc.desc: Check system window permission while type is WINDOW_TYPE_UI_EXTENSION return false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, CheckSysWinPermissionWithUIExtTypeThenFalse, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);

    ASSERT_NE(ssm_, nullptr);
    bool res = ssm_->CheckSystemWindowPermission(property);
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: CheckSysWinPermissionWithInputTypeThenFalse
 * @tc.desc: Check system window permission while type is WINDOW_TYPE_INPUT_METHOD_FLOAT return false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, CheckSysWinPermissionWithInputTypeThenFalse, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);

    ASSERT_NE(ssm_, nullptr);
    bool res = ssm_->CheckSystemWindowPermission(property);
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: CheckSysWinPermWithInputStatusTypeThenFalse
 * @tc.desc: Check system window permission while type is WINDOW_TYPE_INPUT_METHOD_STATUS_BAR return false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, CheckSysWinPermWithInputStatusTypeThenFalse, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR);

    ASSERT_NE(ssm_, nullptr);
    bool res = ssm_->CheckSystemWindowPermission(property);
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: CheckSysWinPermWithDraggingTypeThenTrue
 * @tc.desc: Check system window permission while type is WINDOW_TYPE_DRAGGING_EFFECT return false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, CheckSysWinPermWithDraggingTypeThenTrue, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_DRAGGING_EFFECT);

    ASSERT_NE(ssm_, nullptr);
    bool res = ssm_->CheckSystemWindowPermission(property);
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: CheckSysWinPermWithToastTypeThenTrue
 * @tc.desc: Check system window permission while type is WINDOW_TYPE_TOAST return false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, CheckSysWinPermWithToastTypeThenTrue, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_TOAST);

    ASSERT_NE(ssm_, nullptr);
    bool res = ssm_->CheckSystemWindowPermission(property);
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: CheckSysWinPermWithDialgTypeThenTrue
 * @tc.desc: Check system window permission while type is WINDOW_TYPE_DIALOG return true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, CheckSysWinPermWithDialgTypeThenTrue, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);

    ASSERT_NE(ssm_, nullptr);
    bool res = ssm_->CheckSystemWindowPermission(property);
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: CheckSysWinPermWithPipTypeThenTrue
 * @tc.desc: Check system window permission while type is WINDOW_TYPE_DIALOG return true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, CheckSysWinPermWithPipTypeThenTrue, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);

    ASSERT_NE(ssm_, nullptr);
    bool res = ssm_->CheckSystemWindowPermission(property);
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: CheckSysWinPermWithFloatTypeThenFalse
 * @tc.desc: Check system window permission while type is WINDOW_TYPE_FLOAT return false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, CheckSysWinPermWithFloatTypeThenFalse, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);

    ASSERT_NE(ssm_, nullptr);
    bool res = ssm_->CheckSystemWindowPermission(property);
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: SetAlivePersistentIdsWithNoId
 * @tc.desc: Set Alive Persistent Ids With no persistentId then IsNeedRecover check return false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, SetAlivePersistentIdsWithNoId, TestSize.Level1)
{
    std::vector<int32_t> alivePersistentIds;
    ASSERT_NE(ssm_, nullptr);
    ssm_->SetAlivePersistentIds(alivePersistentIds);

    int persistentId = 1;
    bool res = ssm_->IsNeedRecover(persistentId);
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: SetAlivePersistentIdsWithIds
 * @tc.desc: Set Alive Persistent Ids With persistentIds then IsNeedRecover check return true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, SetAlivePersistentIdsWithIds, TestSize.Level1)
{
    std::vector<int32_t> alivePersistentIds = { 0, 1, 2 };
    ASSERT_NE(ssm_, nullptr);
    ssm_->SetAlivePersistentIds(alivePersistentIds);
    ASSERT_EQ(ssm_->alivePersistentIds_, alivePersistentIds);

    bool res = ssm_->IsNeedRecover(0);
    ASSERT_EQ(res, true);
    res = ssm_->IsNeedRecover(1);
    ASSERT_EQ(res, true);
    res = ssm_->IsNeedRecover(2);
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: RecoverAndConnectSpecificSession
 * @tc.desc: RecoverAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, RecoverAndConnectSpecificSession, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property;
    sptr<ISession> session;
    sptr<IRemoteObject> token;
    auto ret = ssm_->RecoverAndConnectSpecificSession(sessionStage, eventChannel, node, property, session, token);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
    property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    ret = ssm_->RecoverAndConnectSpecificSession(sessionStage, eventChannel, node, property, session, token);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
    ssm_->NotifyRecoveringFinished();
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(ssm_->recoveringFinished_, true);
}

/**
 * @tc.name: TestCacheSpecificSessionForRecovering_01
 * @tc.desc: Test CacheSpecificSessionForRecovering with recoveringFinished_ is false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCacheSpecificSessionForRecovering_01, TestSize.Level1)
{
    sptr<SceneSession> sceneSession;
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<WindowSessionProperty> property;
    ssm_->recoveringFinished_ = true;
    ssm_->recoverSubSessionCacheMap_.clear();
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    ASSERT_EQ(ssm_->recoverSubSessionCacheMap_.size(), 0);
}

/**
 * @tc.name: TestCacheSpecificSessionForRecovering_02
 * @tc.desc: Test CacheSpecificSessionForRecovering with property is null
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCacheSpecificSessionForRecovering_02, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property;
    ssm_->recoveringFinished_ = false;
    ssm_->recoverSubSessionCacheMap_.clear();
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    ASSERT_EQ(ssm_->recoverSubSessionCacheMap_.size(), 0);
}

/**
 * @tc.name: TestCacheSpecificSessionForRecovering_03
 * @tc.desc: Test CacheSpecificSessionForRecovering with sceneSession is null
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCacheSpecificSessionForRecovering_03, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ssm_->recoveringFinished_ = false;
    ssm_->recoverSubSessionCacheMap_.clear();
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    ASSERT_EQ(ssm_->recoverSubSessionCacheMap_.size(), 0);
}

/**
 * @tc.name: TestCacheSpecificSessionForRecovering_04
 * @tc.desc: Test CacheSpecificSessionForRecovering with windowType is WINDOW_TYPE_APP_MAIN_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCacheSpecificSessionForRecovering_04, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ssm_->recoveringFinished_ = false;
    ssm_->recoverSubSessionCacheMap_.clear();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    ASSERT_EQ(ssm_->recoverSubSessionCacheMap_.size(), 0);
}

/**
 * @tc.name: TestCacheSpecificSessionForRecovering_05
 * @tc.desc: Test CacheSpecificSessionForRecovering with windowType is APP_SUB_WINDOW_END and no parentId in map
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCacheSpecificSessionForRecovering_05, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ssm_->recoveringFinished_ = false;
    ssm_->recoverSubSessionCacheMap_.clear();
    property->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    ASSERT_EQ(ssm_->recoverSubSessionCacheMap_.size(), 0);
}

/**
 * @tc.name: TestCacheSpecificSessionForRecovering_06
 * @tc.desc: Test CacheSpecificSessionForRecovering with windowType is WINDOW_TYPE_APP_SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCacheSpecificSessionForRecovering_06, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ssm_->recoveringFinished_ = false;
    ssm_->recoverSubSessionCacheMap_.clear();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->SetParentPersistentId(1);
    NotifyCreateSubSessionFunc func;
    ssm_->createSubSessionFuncMap_.clear();
    ssm_->createSubSessionFuncMap_.insert({ 1, func });
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    ASSERT_EQ(ssm_->recoverSubSessionCacheMap_.size(), 0);
    ssm_->createSubSessionFuncMap_.clear();
}

/**
 * @tc.name: TestCacheSpecificSessionForRecovering_07
 * @tc.desc: Test CacheSpecificSessionForRecovering with windowType is WINDOW_TYPE_DIALOG
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCacheSpecificSessionForRecovering_07, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ssm_->recoveringFinished_ = false;
    ssm_->recoverDialogSessionCacheMap_.clear();
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    property->SetParentPersistentId(1);
    NotifyCreateSubSessionFunc func;
    ssm_->bindDialogTargetFuncMap_.clear();
    ssm_->bindDialogTargetFuncMap_.insert({ 1, func });
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    ASSERT_EQ(ssm_->recoverDialogSessionCacheMap_.size(), 0);
}

/**
 * @tc.name: TestCacheSpecificSessionForRecovering_08
 * @tc.desc: Test CacheSpecificSessionForRecovering with windowType is WINDOW_TYPE_DIALOG and no parentId in map
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestCacheSpecificSessionForRecovering_08, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ssm_->recoveringFinished_ = false;
    ssm_->recoverDialogSessionCacheMap_.clear();
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    NotifyCreateSubSessionFunc func;
    ssm_->CacheSpecificSessionForRecovering(sceneSession, property);
    ASSERT_EQ(ssm_->recoverDialogSessionCacheMap_.size(), 1);
}

/**
 * @tc.name: TestRecoverAndReconnectSceneSession_01
 * @tc.desc: Test RecoverAndReconnectSceneSession with property is null
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestRecoverAndReconnectSceneSession_01, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property;
    sptr<ISession> session;
    sptr<IRemoteObject> token;

    auto ret = ssm_->RecoverAndReconnectSceneSession(sessionStage, eventChannel, node, session, property, token);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TestRecoverAndReconnectSceneSession_02
 * @tc.desc: Test RecoverAndReconnectSceneSession with recoveringFinished_ is true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestRecoverAndReconnectSceneSession_02, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sptr<ISession> session;
    sptr<IRemoteObject> token;

    property->SetPersistentId(1);
    ssm_->alivePersistentIds_.push_back(1);
    ssm_->recoveringFinished_ = true;
    auto ret = ssm_->RecoverAndReconnectSceneSession(sessionStage, eventChannel, node, session, property, token);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_OPERATION);
    ssm_->alivePersistentIds_.clear();
}

/**
 * @tc.name: TestRecoverAndReconnectSceneSession_03
 * @tc.desc: Test RecoverAndReconnectSceneSession with recoveringFinished_ is false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestRecoverAndReconnectSceneSession_03, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sptr<ISession> session;
    sptr<IRemoteObject> token;

    property->SetPersistentId(1);
    ssm_->alivePersistentIds_.push_back(1);
    ssm_->recoveringFinished_ = false;
    auto ret = ssm_->RecoverAndReconnectSceneSession(sessionStage, eventChannel, node, session, property, token);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
    ssm_->alivePersistentIds_.clear();
}

/**
 * @tc.name: TestRecoverAndReconnectSceneSession_04
 * @tc.desc: Test RecoverAndReconnectSceneSession with windowType is APP_SUB_WINDOW_END
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestRecoverAndReconnectSceneSession_04, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sptr<ISession> session;
    sptr<IRemoteObject> token;

    property->SetPersistentId(1);
    ssm_->alivePersistentIds_.push_back(1);
    ssm_->recoveringFinished_ = false;
    property->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    auto ret = ssm_->RecoverAndReconnectSceneSession(sessionStage, eventChannel, node, session, property, token);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
    ssm_->alivePersistentIds_.clear();
}

/**
 * @tc.name: NotifyCreateSpecificSession
 * @tc.desc: NotifyCreateSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, NotifyCreateSpecificSession, TestSize.Level1)
{
    sptr<SceneSession> sceneSession;
    sptr<WindowSessionProperty> property;
    ASSERT_NE(ssm_, nullptr);
    ssm_->NotifyCreateSpecificSession(sceneSession, property, WindowType::APP_MAIN_WINDOW_BASE);
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->NotifyCreateSpecificSession(sceneSession, property, WindowType::APP_MAIN_WINDOW_BASE);
    property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    ssm_->NotifyCreateSpecificSession(sceneSession, property, WindowType::APP_MAIN_WINDOW_BASE);
    ssm_->NotifyCreateSpecificSession(sceneSession, property, WindowType::SYSTEM_SUB_WINDOW_BASE);
    ssm_->NotifyCreateSpecificSession(sceneSession, property, WindowType::APP_SUB_WINDOW_BASE);
    ssm_->NotifyCreateSpecificSession(sceneSession, property, WindowType::WINDOW_TYPE_SCENE_BOARD);
    ssm_->NotifyCreateSpecificSession(sceneSession, property, WindowType::WINDOW_TYPE_TOAST);
    ssm_->NotifyCreateSpecificSession(sceneSession, property, WindowType::WINDOW_TYPE_FLOAT);
    property->SetParentPersistentId(1);
    ASSERT_EQ(property->GetParentPersistentId(), 1);
    SessionInfo info1;
    info1.bundleName_ = "test3";
    info1.abilityName_ = "test3";
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession2 });
    ASSERT_TRUE(ssm_->sceneSessionMap_.find(1) != ssm_->sceneSessionMap_.end());
    ssm_->NotifyCreateSpecificSession(sceneSession, property, WindowType::WINDOW_TYPE_FLOAT);
    ssm_->NotifyCreateSpecificSession(sceneSession, property, WindowType::WINDOW_TYPE_DIALOG);
    ssm_->NotifyCreateSpecificSession(sceneSession, property, WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ssm_->NotifyCreateSpecificSession(sceneSession, property, WindowType::BELOW_APP_SYSTEM_WINDOW_BASE);
}

/**
 * @tc.name: NotifyCreateSubSession
 * @tc.desc: NotifyCreateSubSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, NotifyCreateSubSession, TestSize.Level1)
{
    sptr<SceneSession> sceneSession;
    ssm_->NotifyCreateSubSession(1, sceneSession);
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->NotifyCreateSubSession(1, sceneSession);
    ssm_->UnregisterSpecificSessionCreateListener(1);
    ASSERT_TRUE(ssm_->createSubSessionFuncMap_.find(1) == ssm_->createSubSessionFuncMap_.end());
    SessionInfo info1;
    info1.bundleName_ = "test1";
    info1.abilityName_ = "test2";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    NotifyCreateSubSessionFunc func;
    ssm_->createSubSessionFuncMap_.insert({ 1, func });
    ssm_->NotifyCreateSubSession(1, sceneSession);
    ssm_->UnregisterSpecificSessionCreateListener(1);
    ssm_->createSubSessionFuncMap_.clear();
    ASSERT_TRUE(ssm_->createSubSessionFuncMap_.find(1) == ssm_->createSubSessionFuncMap_.end());
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sceneSession->SetSessionProperty(property);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ASSERT_TRUE(ssm_->sceneSessionMap_.find(1) != ssm_->sceneSessionMap_.end());
    ssm_->NotifySessionTouchOutside(1, 0);
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ssm_->NotifySessionTouchOutside(1, 0);
    sceneSession->persistentId_ = property->callingSessionId_;
    ssm_->NotifySessionTouchOutside(1, 0);
    sceneSession->persistentId_ = property->callingSessionId_ + 1;
    ssm_->NotifySessionTouchOutside(1, 0);
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: TestDestroyAndDisconSpecSessionInner_01
 * @tc.desc: Test DestroyAndDisconnectSpecificSessionInner with invaild persistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestDestroyAndDisconSpecSessionInner_01, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    int invaildPersistentId = -1;
    auto ret = ssm_->DestroyAndDisconnectSpecificSessionInner(invaildPersistentId);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TestDestroyAndDisconSpecSessionInner_02
 * @tc.desc: Test DestroyAndDisconnectSpecificSessionInner with true persistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestDestroyAndDisconSpecSessionInner_02, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    uint32_t uid = 0;
    ssm_->GetTopWindowId(1, uid);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->GetTopWindowId(1, uid);
    auto ret = ssm_->DestroyAndDisconnectSpecificSessionInner(1);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: TestDestroyAndDisconSpecSessionInner_03
 * @tc.desc: Test DestroyAndDisconnectSpecificSessionInner with WindowType APP_SUB_WINDOW_BASE
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestDestroyAndDisconSpecSessionInner_03, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    uint32_t uid = 0;
    ssm_->GetTopWindowId(1, uid);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->GetTopWindowId(1, uid);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    property->SetParentPersistentId(100);
    ASSERT_EQ(property->GetParentPersistentId(), 100);
    sceneSession->SetSessionProperty(property);
    auto ret = ssm_->DestroyAndDisconnectSpecificSessionInner(1);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: TestDestroyAndDisconSpecSessionInner_04
 * @tc.desc: Test DestroyAndDisconnectSpecificSessionInner with WindowType WINDOW_TYPE_DIALOG
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestDestroyAndDisconSpecSessionInner_04, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    uint32_t uid = 0;
    ssm_->GetTopWindowId(1, uid);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->GetTopWindowId(1, uid);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    property->SetParentPersistentId(100);
    ASSERT_EQ(property->GetParentPersistentId(), 100);
    sceneSession->SetSessionProperty(property);
    auto ret = ssm_->DestroyAndDisconnectSpecificSessionInner(1);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: GetFocusWindowInfo
 * @tc.desc: GetFocusWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, GetFocusWindowInfo, TestSize.Level1)
{
    sptr<WindowSessionProperty> property;
    sptr<SceneSession> sceneSession;
    ssm_->NotifySessionForCallback(sceneSession, true);
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    info.isSystem_ = true;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->NotifySessionForCallback(sceneSession, true);
    info.isSystem_ = false;
    ssm_->NotifySessionForCallback(sceneSession, true);
    property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetBrightness(1.f);
    auto ret = ssm_->SetBrightness(sceneSession, 1.f);
    ASSERT_EQ(ret, WSError::WS_OK);
    ret = ssm_->SetBrightness(sceneSession, 2.f);
    ASSERT_EQ(ret, WSError::WS_OK);
    ssm_->displayBrightness_ = 2.f;
    ret = ssm_->SetBrightness(sceneSession, 2.f);
    ASSERT_EQ(ret, WSError::WS_OK);
    ssm_->displayBrightness_ = 3.f;
    ret = ssm_->SetBrightness(sceneSession, 2.f);
    ASSERT_EQ(ret, WSError::WS_OK);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ret = ssm_->UpdateBrightness(1);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    property->SetBrightness(-1.f);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    sceneSession->SetSessionProperty(property);
    ret = ssm_->UpdateBrightness(1);
    ASSERT_EQ(ret, WSError::WS_OK);
    property->SetBrightness(3.f);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    ret = ssm_->UpdateBrightness(1);
    ASSERT_EQ(ret, WSError::WS_OK);
    FocusChangeInfo changeInfo;
    ssm_->GetFocusWindowInfo(changeInfo);
}

/**
 * @tc.name: TestIsSessionVisible_01
 * @tc.desc: Test IsSessionVisible with session is null then false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestIsSessionVisible_01, TestSize.Level1)
{
    sptr<SceneSession> sceneSession;
    bool ret = ssm_->IsSessionVisible(sceneSession);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: TestIsSessionVisible_02
 * @tc.desc: Test IsSessionVisible with isScbCoreEnabled_ is true then false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestIsSessionVisible_02, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetScbCoreEnabled(true);
    sceneSession->isVisible_ = false;

    bool ret = ssm_->IsSessionVisible(sceneSession);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: TestIsSessionVisible_03
 * @tc.desc: Test IsSessionVisible with WindowType is APP_SUB_WINDOW_BASE then false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestIsSessionVisible_03, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetScbCoreEnabled(false);
    sceneSession->isVisible_ = true;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession->SetSessionProperty(property);

    bool ret = ssm_->IsSessionVisible(sceneSession);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: TestIsSessionVisible_04
 * @tc.desc: Test IsSessionVisible with mainSession is not null then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestIsSessionVisible_04, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetScbCoreEnabled(false);
    sceneSession->isVisible_ = true;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    SessionInfo info1;
    info1.bundleName_ = "test3";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession1 });
    property->SetParentPersistentId(100);
    sceneSession->SetSessionProperty(property);
    sceneSession->SetParentSession(sceneSession1);

    bool ret = ssm_->IsSessionVisible(sceneSession);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: TestIsSessionVisible_05
 * @tc.desc: Test IsSessionVisible with session and mainSession state is STATE_ACTIVE then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestIsSessionVisible_05, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetScbCoreEnabled(false);
    sceneSession->isVisible_ = true;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    SessionInfo info1;
    info1.bundleName_ = "test3";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession1 });
    property->SetParentPersistentId(1);
    sceneSession->SetSessionProperty(property);
    sceneSession->SetParentSession(sceneSession1);
    sceneSession1->state_ = SessionState::STATE_ACTIVE;
    sceneSession->state_ = SessionState::STATE_ACTIVE;

    bool ret = ssm_->IsSessionVisible(sceneSession);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: TestIsSessionVisible_06
 * @tc.desc: Test IsSessionVisible with session state is STATE_ACTIVE and mainSession state is STATE_INACTIVE then false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestIsSessionVisible_06, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetScbCoreEnabled(false);
    sceneSession->isVisible_ = true;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    SessionInfo info1;
    info1.bundleName_ = "test3";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession1 });
    property->SetParentPersistentId(1);
    sceneSession->SetSessionProperty(property);
    sceneSession->SetParentSession(sceneSession1);
    sceneSession1->state_ = SessionState::STATE_INACTIVE;
    sceneSession->state_ = SessionState::STATE_ACTIVE;

    bool ret = ssm_->IsSessionVisible(sceneSession);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: TestIsSessionVisible_07
 * @tc.desc: Test IsSessionVisible with session state is STATE_INACTIVE and isVisible_ is false then false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestIsSessionVisible_07, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetScbCoreEnabled(false);
    sceneSession->isVisible_ = false;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    SessionInfo info1;
    info1.bundleName_ = "test3";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession1 });
    property->SetParentPersistentId(1);
    sceneSession->SetSessionProperty(property);
    sceneSession->SetParentSession(sceneSession1);
    sceneSession->state_ = SessionState::STATE_INACTIVE;

    bool ret = ssm_->IsSessionVisible(sceneSession);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: TestIsSessionVisible_08
 * @tc.desc: Test IsSessionVisible with window type is WINDOW_TYPE_APP_MAIN_WINDOW and isVisible_ is true then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestIsSessionVisible_08, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetScbCoreEnabled(false);
    sceneSession->isVisible_ = true;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    sceneSession->state_ = SessionState::STATE_INACTIVE;

    bool ret = ssm_->IsSessionVisible(sceneSession);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: TestIsSessionVisible_09
 * @tc.desc: Test IsSessionVisible with window type is WINDOW_TYPE_APP_MAIN_WINDOW and isVisible_ is false then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, TestIsSessionVisible_09, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetScbCoreEnabled(false);
    sceneSession->isVisible_ = false;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionProperty(property);
    sceneSession->state_ = SessionState::STATE_ACTIVE;

    bool ret = ssm_->IsSessionVisible(sceneSession);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: RegisterBindDialogTargetListener
 * @tc.desc: RegisterBindDialogTargetListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerSupplementTest, RegisterBindDialogTargetListener, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "test1";
    info.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    int32_t persistentId = 1;
    ssm_->UnregisterSpecificSessionCreateListener(persistentId);
    NotifyBindDialogSessionFunc func1;
    ssm_->RegisterBindDialogTargetListener(sceneSession, std::move(func1));
    ssm_->UnregisterSpecificSessionCreateListener(persistentId);

    NotifyBindDialogSessionFunc func2;
    ssm_->bindDialogTargetFuncMap_.insert({ persistentId, func2 });
    ASSERT_NE(ssm_->bindDialogTargetFuncMap_.find(persistentId), ssm_->bindDialogTargetFuncMap_.end());
    ssm_->bindDialogTargetFuncMap_.erase(persistentId);
    ASSERT_EQ(ssm_->bindDialogTargetFuncMap_.find(persistentId), ssm_->bindDialogTargetFuncMap_.end());
    ssm_->bindDialogTargetFuncMap_.insert({ persistentId, func2 });
    ASSERT_NE(ssm_->bindDialogTargetFuncMap_.find(persistentId), ssm_->bindDialogTargetFuncMap_.end());
    ssm_->bindDialogTargetFuncMap_.clear();
    ASSERT_EQ(ssm_->bindDialogTargetFuncMap_.find(persistentId), ssm_->bindDialogTargetFuncMap_.end());
}
} // namespace
} // namespace Rosen
} // namespace OHOS