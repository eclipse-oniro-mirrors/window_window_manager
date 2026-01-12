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

#include "common/include/session_permission.h"
#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "mock/mock_scene_session.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/host/include/scene_session.h"
#include "session_info.h"
#include "session_manager.h"
#include "session_manager/include/scene_session_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class SceneSessionManagerTest9 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;
    ScreenSessionManagerClient* screenSessionManagerClient_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerTest9::ssm_ = nullptr;

void NotifyRecoverSceneSessionFuncTest(const sptr<SceneSession>& session, const SessionInfo& sessionInfo) {}

bool getStateFalse(const ManagerState key)
{
    return false;
}

bool getStateTrue(const ManagerState key)
{
    return true;
}

bool TraverseFuncTest(const sptr<SceneSession>& session)
{
    return true;
}

void SceneSessionManagerTest9::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerTest9::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest9::SetUp()
{
    screenSessionManagerClient_ = &ScreenSessionManagerClient::GetInstance();
}

void SceneSessionManagerTest9::TearDown()
{
    screenSessionManagerClient_ = nullptr;
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: RequestFocusStatusBySA
 * @tc.desc: SceneSessionManager request focus status by SA
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, RequestFocusStatusBySA, TestSize.Level1)
{
    int32_t persistentId = 3;
    bool isFocused = true;
    bool byForeground = true;
    FocusChangeReason reason = FocusChangeReason::CLICK;
    auto result = ssm_->SceneSessionManager::RequestFocusStatusBySA(persistentId, isFocused, byForeground, reason);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: SetFocusedSessionId
 * @tc.desc: SceneSessionManager set focused session id
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, SetFocusedSessionId, TestSize.Level1)
{
    int32_t focusedSession = ssm_->GetFocusedSessionId(DISPLAY_ID_INVALID);
    EXPECT_EQ(focusedSession, INVALID_SESSION_ID);
    int32_t persistentId = INVALID_SESSION_ID;
    ssm_->SetFocusedSessionId(persistentId, DEFAULT_DISPLAY_ID);
    WSError result01 = ssm_->SetFocusedSessionId(persistentId, DEFAULT_DISPLAY_ID);
    EXPECT_EQ(result01, WSError::WS_DO_NOTHING);
    persistentId = 10086;
    WSError result02 = ssm_->SetFocusedSessionId(persistentId, DEFAULT_DISPLAY_ID);
    EXPECT_EQ(result02, WSError::WS_OK);
    ASSERT_EQ(ssm_->GetFocusedSessionId(), 10086);
}

/**
 * @tc.name: NotifyRequestFocusStatusNotifyManager
 * @tc.desc: NotifyRequestFocusStatusNotifyManager test.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, NotifyRequestFocusStatusNotifyManager, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRequestFocusStatusNotifyManager";
    info.bundleName_ = "NotifyRequestFocusStatusNotifyManager";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ssm_->RegisterRequestFocusStatusNotifyManagerFunc(sceneSession);

    FocusChangeReason reasonInput = FocusChangeReason::DEFAULT;
    sceneSession->NotifyRequestFocusStatusNotifyManager(true, true, reasonInput);
    FocusChangeReason reasonResult = ssm_->GetFocusChangeReason();

    EXPECT_EQ(reasonInput, reasonResult);
}

/**
 * @tc.name: GetNextFocusableSession
 * @tc.desc: GetNextFocusableSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, GetNextFocusableSession, TestSize.Level0)
{
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);

    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> sceneSession03 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> sceneSession04 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> sceneSession05 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ASSERT_NE(sceneSession03, nullptr);
    ASSERT_NE(sceneSession04, nullptr);
    ASSERT_NE(sceneSession05, nullptr);
    ASSERT_NE(sceneSession->property_, nullptr);

    sceneSession->SetForceHideState(ForceHideState::NOT_HIDDEN);
    sceneSession->property_->SetFocusable(true);
    sceneSession->property_->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->SetZOrder(1);

    sceneSession02->SetFocusable(false);
    sceneSession02->SetZOrder(2);

    sceneSession03->SetZOrder(3);

    sceneSession04->SetForceHideState(ForceHideState::HIDDEN_WHEN_FOCUSED);
    sceneSession04->SetZOrder(4);

    sceneSession05->persistentId_ = 1;
    sceneSession05->SetZOrder(5);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession03));
    ssm_->sceneSessionMap_.insert(std::make_pair(4, sceneSession04));
    ssm_->sceneSessionMap_.insert(std::make_pair(5, sceneSession05));
    sptr<SceneSession> result = ssm_->GetNextFocusableSession(DEFAULT_DISPLAY_ID, 1);
    EXPECT_EQ(result, sceneSession);
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: RequestFocusSpecificCheck
 * @tc.desc: RequestFocusSpecificCheck
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, RequestFocusSpecificCheck, TestSize.Level0)
{
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);

    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    bool byForeground = true;
    FocusChangeReason reason = FocusChangeReason::CLIENT_REQUEST;
    sceneSession->SetForceHideState(ForceHideState::HIDDEN_WHEN_FOCUSED);
    WSError result = ssm_->RequestFocusSpecificCheck(DEFAULT_DISPLAY_ID, sceneSession, byForeground, reason);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_OPERATION);

    sceneSession->SetForceHideState(ForceHideState::NOT_HIDDEN);
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession01, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(0, sceneSession01));
    sceneSession01->parentSession_ = sceneSession;
    result = ssm_->RequestFocusSpecificCheck(DEFAULT_DISPLAY_ID, sceneSession, byForeground, reason);
    EXPECT_EQ(result, WSError::WS_OK);
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: RequestAllAppSessionUnfocus
 * @tc.desc: RequestAllAppSessionUnfocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, RequestAllAppSessionUnfocus, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->HandleHideNonSystemFloatingWindows(property, sceneSession);
    ssm_->RequestAllAppSessionUnfocus();
}

/**
 * @tc.name: RequestFocusClient01
 * @tc.desc: RequestFocusClient
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, RequestFocusClient01, TestSize.Level0)
{
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);

    SessionInfo info;
    info.abilityName_ = "RequestFocusTest1";
    info.bundleName_ = "RequestFocusTest1";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetFocusable(true);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_ = property;
    sceneSession->persistentId_ = 1;
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    sceneSession->SetZOrder(1);

    SessionInfo info2;
    info2.abilityName_ = "RequestFocusTest2";
    info2.bundleName_ = "RequestFocusTest2";
    sptr<SceneSession> sceneSession2 = nullptr;
    sptr<WindowSessionProperty> property2 = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property2, nullptr);
    property2->SetFocusable(true);
    property2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    sceneSession2->property_ = property2;
    sceneSession2->persistentId_ = 2;
    sceneSession2->isVisible_ = true;
    sceneSession2->state_ = SessionState::STATE_ACTIVE;
    sceneSession2->SetZOrder(2);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });
    FocusChangeReason reason = FocusChangeReason::CLIENT_REQUEST;

    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    ssm_->RequestSessionFocus(1, false, reason);
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 1);
    ssm_->RequestSessionFocus(2, false, reason);
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 2);
    ssm_->sceneSessionMap_.clear();
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: RequestFocusClient02
 * @tc.desc: RequestFocusClient
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, RequestFocusClient02, TestSize.Level0)
{
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);

    SessionInfo info;
    info.abilityName_ = "RequestFocusTest1";
    info.bundleName_ = "RequestFocusTest1";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetFocusable(true);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_ = property;
    sceneSession->persistentId_ = 1;
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    sceneSession->SetZOrder(1);

    SessionInfo info2;
    info2.abilityName_ = "RequestFocusTest2";
    info2.bundleName_ = "RequestFocusTest2";
    sptr<SceneSession> sceneSession2 = nullptr;
    sptr<WindowSessionProperty> property2 = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property2, nullptr);
    property2->SetFocusable(true);
    property2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    sceneSession2->property_ = property2;
    sceneSession2->persistentId_ = 2;
    sceneSession2->isVisible_ = true;
    sceneSession2->state_ = SessionState::STATE_ACTIVE;
    sceneSession2->SetZOrder(2);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });
    FocusChangeReason reason = FocusChangeReason::CLIENT_REQUEST;

    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    ssm_->RequestSessionUnfocus(2, reason);
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 1);
    ssm_->RequestSessionUnfocus(1, reason);
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 0);
    ssm_->sceneSessionMap_.clear();
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: TraverseSessionTreeFromTopToBottom
 * @tc.desc: TraverseSessionTreeFromTopToBottom
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TraverseSessionTreeFromTopToBottom, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->TraverseSessionTreeFromTopToBottom(TraverseFuncTest);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TraverseSessionTreeFromTopToBottom";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, nullptr));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession));
    ssm_->TraverseSessionTreeFromTopToBottom(TraverseFuncTest);
}

/**
 * @tc.name: TestRequestFocusStatus_01
 * @tc.desc: Test RequestFocusStatus with sceneSession is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestFocusStatus_01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto res = ssm_->RequestFocusStatus(1, false, false, FocusChangeReason::FLOATING_SCENE);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: TestRequestFocusStatus_02
 * @tc.desc: Test RequestFocusStatus with not call by the same process
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestFocusStatus_02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "RequestFocusStatus02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->SetCallingPid(0);
    sceneSession->isVisible_ = false;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto res = ssm_->RequestFocusStatus(1, false, false, FocusChangeReason::FLOATING_SCENE);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: TestRequestSessionFocusImmediately_01
 * @tc.desc: Test RequestSessionFocusImmediately with invalid persistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocusImmediately_01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    WSError ret = ssm_->RequestSessionFocusImmediately(0);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: TestRequestSessionFocusImmediately_02
 * @tc.desc: Test RequestSessionFocusImmediately with session is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocusImmediately_02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    WSError ret = ssm_->RequestSessionFocusImmediately(2);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: TestRequestSessionFocusImmediately_03
 * @tc.desc: Test RequestSessionFocusImmediately with session is not focusable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocusImmediately_03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "RequestSessionFocusImmediately02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->SetFocusable(false);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    auto ret = ssm_->RequestSessionFocusImmediately(1);
    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: TestRequestSessionFocusImmediately_04
 * @tc.desc: Test RequestSessionFocusImmediately with session is not focused on show
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocusImmediately_04, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "RequestSessionFocusImmediately02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->SetFocusable(true);
    sceneSession->SetFocusedOnShow(false);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto ret = ssm_->RequestSessionFocusImmediately(1);
    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: TestRequestSessionFocusImmediately_05
 * @tc.desc: Test RequestSessionFocusImmediately with ForceHideState HIDDEN_WHEN_FOCUSED
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocusImmediately_05, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "RequestSessionFocusImmediately02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->SetFocusable(true);
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetForceHideState(ForceHideState::HIDDEN_WHEN_FOCUSED);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto ret = ssm_->RequestSessionFocusImmediately(1);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: TestRequestSessionFocusImmediately_06
 * @tc.desc: Test RequestSessionFocusImmediately with ForceHideState NOT_HIDDEN
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocusImmediately_06, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "RequestSessionFocusImmediately02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->SetFocusable(true);
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetForceHideState(ForceHideState::NOT_HIDDEN);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    WSError ret = ssm_->RequestSessionFocusImmediately(1);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: TestRequestSessionFocusImmediately_07
 * @tc.desc: Test RequestSessionFocusImmediately with PCMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocusImmediately_07, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocusImmediately_07";
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->SetFocusable(true);
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetForceHideState(ForceHideState::NOT_HIDDEN);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;
    ssm_->systemConfig_.freeMultiWindowEnable_ = true;
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    WSError ret = ssm_->RequestSessionFocusImmediately(1, false);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: Test RequestSessionFocus_01
 * @tc.desc: Test RequestSessionFocus invalid persistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, RequestSessionFocus_01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    WSError ret = ssm_->RequestSessionFocus(0);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: RequestSessionFocus_02
 * @tc.desc: Test RequestSessionFocus with session is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, RequestSessionFocus_02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    WSError ret = ssm_->RequestSessionFocus(2);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: TestRequestSessionFocus_03
 * @tc.desc: Test RequestSessionFocus with session is not focusable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocus_03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocus_03";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetFocusable(false);
    sceneSession->UpdateVisibilityInner(true);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    WSError ret = ssm_->RequestSessionFocus(1, false, FocusChangeReason::DEFAULT);
    ASSERT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: TestRequestSessionFocus_04
 * @tc.desc: Test RequestSessionFocus with session is not visible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocus_04, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocus_04";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetFocusable(true);
    sceneSession->UpdateVisibilityInner(false);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    WSError ret = ssm_->RequestSessionFocus(1, false, FocusChangeReason::DEFAULT);
    ASSERT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: TestRequestSessionFocus_05
 * @tc.desc: Test RequestSessionFocus with session is not focused on show
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocus_05, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocus_05";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetFocusable(true);
    sceneSession->UpdateVisibilityInner(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetFocusedOnShow(false);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    WSError ret = ssm_->RequestSessionFocus(1, false, FocusChangeReason::DEFAULT);
    ASSERT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: TestRequestSessionFocus_06
 * @tc.desc: Test RequestSessionFocus with session is not focusable on show
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocus_06, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocus_06";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetFocusable(true);
    sceneSession->UpdateVisibilityInner(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetFocusableOnShow(false);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    WSError ret = ssm_->RequestSessionFocus(1, false, FocusChangeReason::FOREGROUND);
    ASSERT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: TestRequestSessionFocus_07
 * @tc.desc: Test RequestSessionFocus with windowType is APP_SUB_WINDOW_BASE
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocus_07, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocus_07";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetFocusable(true);
    sceneSession->UpdateVisibilityInner(true);
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetFocusableOnShow(true);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    WSError ret = ssm_->RequestSessionFocus(1, false, FocusChangeReason::DEFAULT);
    ASSERT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: TestRequestSessionFocus_08
 * @tc.desc: Test RequestSessionFocus with windowType is WINDOW_TYPE_DIALOG
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocus_08, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocus_08";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetFocusable(true);
    sceneSession->UpdateVisibilityInner(true);
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetFocusableOnShow(true);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    WSError ret = ssm_->RequestSessionFocus(1, false, FocusChangeReason::DEFAULT);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: TestRequestSessionFocus_09
 * @tc.desc: Test RequestSessionFocus with ForceHideState HIDDEN_WHEN_FOCUSED
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocus_09, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocus_08";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetFocusable(true);
    sceneSession->UpdateVisibilityInner(true);
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetFocusableOnShow(true);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetForceHideState(ForceHideState::HIDDEN_WHEN_FOCUSED);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    WSError ret = ssm_->RequestSessionFocus(1, false, FocusChangeReason::DEFAULT);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_OPERATION);
}

/**
 * @tc.name: TestRequestSessionFocus_10
 * @tc.desc: Test RequestSessionFocus with ForceHideState HIDDEN_WHEN_FOCUSED
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocus_10, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocus_10";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetFocusable(true);
    sceneSession->UpdateVisibilityInner(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetFocusableOnShow(false);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetForceHideState(ForceHideState::NOT_HIDDEN);
    sceneSession->SetTopmost(true);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    WSError ret = ssm_->RequestSessionFocus(1, false, FocusChangeReason::DEFAULT);
    ASSERT_EQ(ret, WSError::WS_OK);
    ASSERT_EQ(sceneSession->isFocused_, true);
}

/**
 * @tc.name: TestRequestSessionFocus_11
 * @tc.desc: Test RequestSessionFocus with focusGroup is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocus_11, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocus_11";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetDisplayId(-1);
    sceneSession->persistentId_ = 1;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    WSError ret = ssm_->RequestSessionFocus(1, false, FocusChangeReason::DEFAULT);
    EXPECT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TestRequestSessionFocus_12
 * @tc.desc: Test RequestSessionFocus with REQUEST_WITH_CHECK_SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocus_12, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocus_12";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->persistentId_ = 1;

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    WSError ret = ssm_->RequestSessionFocus(1, false, FocusChangeReason::REQUEST_WITH_CHECK_SUB_WINDOW);
    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: TestRequestSessionFocus_13
 * @tc.desc: Test RequestSessionFocus with REQUEST_WITH_CHECK_SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocus_13, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocus_13";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->persistentId_ = 1;

    SessionInfo subSessionInfo;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subSessionInfo, nullptr);
    subSession->persistentId_ = 2;
    subSession->SetFocusable(true);
    subSession->UpdateVisibilityInner(true);
    subSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->subSession_.push_back(subSession);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, subSession));
    ssm_->windowFocusController_->UpdateFocusedSessionId(DEFAULT_DISPLAY_ID, 2);
    WSError ret = ssm_->RequestSessionFocus(1, false, FocusChangeReason::REQUEST_WITH_CHECK_SUB_WINDOW);
    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: TestRequestSessionFocus_14
 * @tc.desc: Test RequestSessionFocus with windowType is APP_SUB_WINDOW_BASE and parent is not visible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, TestRequestSessionFocus_14, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "TestRequestSessionFocus_14";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->persistentId_ = 1;

    SessionInfo subSessionInfo;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subSessionInfo, nullptr);
    subSession->persistentId_ = 2;
    subSession->SetFocusable(true);
    subSession->UpdateVisibilityInner(true);
    subSession->SetSessionState(SessionState::STATE_FOREGROUND);
    subSession->SetFocusedOnShow(true);
    subSession->SetFocusableOnShow(true);
    subSession->property_->SetParentPersistentId(1);
    subSession->GetSessionProperty()->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, subSession));
    ssm_->windowFocusController_->UpdateFocusedSessionId(DEFAULT_DISPLAY_ID, 0);
    WSError ret = ssm_->RequestSessionFocus(2, false, FocusChangeReason::DEFAULT);
    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: RequestSessionUnfocus02
 * @tc.desc: RequestSessionUnfocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, RequestSessionUnfocus02, TestSize.Level0)
{
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);

    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    WSError ret = ssm_->RequestSessionUnfocus(0, FocusChangeReason::DEFAULT);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "RequestSessionUnfocus02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->windowFocusController_->UpdateFocusedSessionId(DEFAULT_DISPLAY_ID, 0);
    ret = ssm_->RequestSessionUnfocus(1, FocusChangeReason::DEFAULT);
    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
    ssm_->windowFocusController_->UpdateFocusedSessionId(DEFAULT_DISPLAY_ID, 2);
    sceneSession->persistentId_ = 1;
    sceneSession->SetZOrder(50);
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->isVisible_ = true;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    SessionInfo sessionInfo1;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sceneSession1->GetSessionProperty()->SetParentPersistentId(1);
    sceneSession1->persistentId_ = 2;
    sceneSession1->SetZOrder(100);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession1));
    ret = ssm_->RequestSessionUnfocus(2, FocusChangeReason::DEFAULT);
    EXPECT_EQ(ret, WSError::WS_OK);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    EXPECT_EQ(focusGroup->GetFocusedSessionId(), 1);

    focusGroup->SetLastFocusedSessionId(4);
    sceneSession1->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_FLOAT);
    SessionInfo sessionInfo2;
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    sceneSession2->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_DESKTOP);
    sceneSession2->SetZOrder(20);
    sceneSession2->state_ = SessionState::STATE_FOREGROUND;
    sceneSession2->isVisible_ = true;
    sceneSession2->persistentId_ = 4;
    ssm_->sceneSessionMap_.insert(std::make_pair(4, sceneSession2));

    ret = ssm_->RequestSessionUnfocus(1, FocusChangeReason::DEFAULT);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(focusGroup->GetFocusedSessionId(), 4);
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: RequestAllAppSessionUnfocusInner
 * @tc.desc: RequestAllAppSessionUnfocusInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, RequestAllAppSessionUnfocusInner, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->RequestAllAppSessionUnfocusInner();

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "RequestAllAppSessionUnfocusInner";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->RequestAllAppSessionUnfocusInner();

    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_FLOAT);
    ssm_->RequestAllAppSessionUnfocusInner();
}

/**
 * @tc.name: ProcessFocusWhenForeground
 * @tc.desc: ProcessFocusWhenForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ProcessFocusWhenForeground, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "ProcessFocusWhenForeground";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->persistentId_ = 1;
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = false;
    ssm_->ProcessFocusWhenForeground(sceneSession);

    sceneSession->isVisible_ = true;
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->ProcessFocusWhenForeground(sceneSession);
}

/**
 * @tc.name: ProcessFocusWhenForeground01
 * @tc.desc: ProcessFocusWhenForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ProcessFocusWhenForeground01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "ProcessFocusWhenForeground";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(2);
    sceneSession->SetFocusedOnShow(false);
    ssm_->ProcessFocusWhenForeground(sceneSession);
    EXPECT_EQ(sceneSession->IsFocusedOnShow(), false);

    sceneSession->isVisible_ = true;
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->ProcessFocusWhenForeground(sceneSession);
    EXPECT_EQ(sceneSession->IsFocusedOnShow(), true);
}

/**
 * @tc.name: ProcessFocusWhenForeground02
 * @tc.desc: ProcessFocusWhenForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ProcessFocusWhenForeground02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "ProcessFocusWhenForeground02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    focusGroup->SetNeedBlockNotifyFocusStatusUntilForeground(true);
    ssm_->ProcessFocusWhenForeground(sceneSession);
    EXPECT_EQ(focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground(), false);
}

/**
 * @tc.name: ProcessSubSessionForeground03
 * @tc.desc: ProcessSubSessionForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ProcessSubSessionForeground03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "ProcessSubSessionForeground03";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);

    SessionInfo subSessionInfo;
    sptr<SceneSession> subSceneSession = sptr<SceneSession>::MakeSptr(subSessionInfo, nullptr);
    ASSERT_NE(nullptr, subSceneSession);
    subSceneSession->SetTopmost(true);
    subSceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    subSceneSession->persistentId_ = 1;
    sceneSession->GetSubSession().push_back(subSceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, subSceneSession));

    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = true;

    SessionInfo subSessionInfo1;
    sptr<SceneSession> subSceneSession1 = sptr<SceneSession>::MakeSptr(subSessionInfo1, nullptr);
    ASSERT_NE(nullptr, subSceneSession1);
    subSceneSession1->SetTopmost(true);
    subSceneSession1->SetSessionState(SessionState::STATE_ACTIVE);
    subSceneSession1->persistentId_ = 0;
    sceneSession->GetSubSession().push_back(subSceneSession1);

    SessionInfo subSessionInfo2;
    sptr<SceneSession> subSceneSession2 = sptr<SceneSession>::MakeSptr(subSessionInfo2, nullptr);
    ASSERT_NE(nullptr, subSceneSession2);
    subSceneSession2->SetTopmost(true);
    subSceneSession2->SetSessionState(SessionState::STATE_CONNECT);
    sceneSession->GetSubSession().push_back(subSceneSession2);

    SessionInfo subSessionInfo3;
    sptr<SceneSession> subSceneSession3 = sptr<SceneSession>::MakeSptr(subSessionInfo3, nullptr);
    ASSERT_NE(nullptr, subSceneSession3);
    subSceneSession3->SetTopmost(false);
    subSceneSession3->SetSessionState(SessionState::STATE_ACTIVE);
    sceneSession->GetSubSession().push_back(subSceneSession3);

    sceneSession->GetSubSession().push_back(nullptr);

    ssm_->ProcessSubSessionForeground(sceneSession);

    ssm_->needBlockNotifyFocusStatusUntilForeground_ = false;
    ssm_->ProcessSubSessionForeground(sceneSession);

    focusGroup->SetFocusedSessionId(2);
    ssm_->ProcessSubSessionForeground(sceneSession);
}

/**
 * @tc.name: ProcessFocusWhenForegroundScbCore
 * @tc.desc: ProcessFocusWhenForegroundScbCore
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ProcessFocusWhenForegroundScbCore, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(0);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "ProcessFocusWhenForegroundScbCore";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->SetFocusableOnShow(false);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->ProcessFocusWhenForegroundScbCore(sceneSession);
    ASSERT_EQ(sceneSession->GetPostProcessFocusState().isFocused_, false);
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 0);

    sceneSession->SetFocusableOnShow(true);
    ssm_->ProcessFocusWhenForegroundScbCore(sceneSession); // SetPostProcessFocusState
    ASSERT_EQ(sceneSession->GetPostProcessFocusState().isFocused_, true);

    sceneSession->isVisible_ = true;
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->ProcessFocusWhenForegroundScbCore(sceneSession); // RequestSessionFocus
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 1);
}

/**
 * @tc.name: ProcessModalTopmostRequestFocusImmediately02
 * @tc.desc: ProcessModalTopmostRequestFocusImmediately
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ProcessModalTopmostRequestFocusImmediately02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "ProcessModalTopmostRequestFocusImmediately02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);

    SessionInfo subSessionInfo;
    sptr<SceneSession> subSceneSession = sptr<SceneSession>::MakeSptr(subSessionInfo, nullptr);
    ASSERT_NE(nullptr, subSceneSession);
    subSceneSession->SetTopmost(true);
    subSceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    subSceneSession->persistentId_ = 1;
    subSceneSession->SetFocusable(true);
    subSceneSession->SetFocusedOnShow(true);
    sceneSession->GetSubSession().push_back(subSceneSession);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, subSceneSession));
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = true;

    SessionInfo subSessionInfo1;
    sptr<SceneSession> subSceneSession1 = sptr<SceneSession>::MakeSptr(subSessionInfo1, nullptr);
    ASSERT_NE(nullptr, subSceneSession1);
    subSceneSession1->SetTopmost(true);
    subSceneSession1->SetSessionState(SessionState::STATE_ACTIVE);
    subSceneSession1->persistentId_ = 0;
    sceneSession->GetSubSession().push_back(subSceneSession1);

    SessionInfo subSessionInfo2;
    sptr<SceneSession> subSceneSession2 = sptr<SceneSession>::MakeSptr(subSessionInfo2, nullptr);
    ASSERT_NE(nullptr, subSceneSession2);
    subSceneSession2->SetTopmost(true);
    subSceneSession2->SetSessionState(SessionState::STATE_CONNECT);
    sceneSession->GetSubSession().push_back(subSceneSession2);

    SessionInfo subSessionInfo3;
    sptr<SceneSession> subSceneSession3 = sptr<SceneSession>::MakeSptr(subSessionInfo3, nullptr);
    ASSERT_NE(nullptr, subSceneSession3);
    subSceneSession3->SetTopmost(false);
    subSceneSession3->SetSessionState(SessionState::STATE_ACTIVE);
    sceneSession->GetSubSession().push_back(subSceneSession3);

    sceneSession->GetSubSession().push_back(nullptr);

    WSError ret = ssm_->ProcessModalTopmostRequestFocusImmediately(sceneSession);
    ASSERT_NE(ret, WSError::WS_OK);

    ssm_->needBlockNotifyFocusStatusUntilForeground_ = false;
    ssm_->ProcessSubSessionForeground(sceneSession);

    subSceneSession->persistentId_ = 0;
    ret = ssm_->ProcessModalTopmostRequestFocusImmediately(sceneSession);
    ASSERT_NE(ret, WSError::WS_OK);
}

/**
 * @tc.name: ProcessDialogRequestFocusImmediately02
 * @tc.desc: ProcessDialogRequestFocusImmediately
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ProcessDialogRequestFocusImmediately02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "ProcessDialogRequestFocusImmediately02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo dialogSessionInfo;
    sptr<SceneSession> dialogSceneSession = sptr<SceneSession>::MakeSptr(dialogSessionInfo, nullptr);
    ASSERT_NE(nullptr, dialogSceneSession);
    dialogSceneSession->SetTopmost(true);
    dialogSceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    dialogSceneSession->persistentId_ = 1;
    dialogSceneSession->SetFocusable(true);
    dialogSceneSession->SetFocusedOnShow(true);
    sceneSession->GetDialogVector().push_back(dialogSceneSession);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, dialogSceneSession));
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = true;

    SessionInfo dialogSessionInfo1;
    sptr<SceneSession> dialogSceneSession1 = sptr<SceneSession>::MakeSptr(dialogSessionInfo1, nullptr);
    ASSERT_NE(nullptr, dialogSceneSession1);
    dialogSceneSession1->SetTopmost(true);
    dialogSceneSession1->SetSessionState(SessionState::STATE_ACTIVE);
    dialogSceneSession1->persistentId_ = 0;
    sceneSession->GetDialogVector().push_back(dialogSceneSession1);

    SessionInfo dialogSessionInfo2;
    sptr<SceneSession> dialogSceneSession2 = sptr<SceneSession>::MakeSptr(dialogSessionInfo2, nullptr);
    ASSERT_NE(nullptr, dialogSceneSession2);
    dialogSceneSession2->SetTopmost(true);
    dialogSceneSession2->SetSessionState(SessionState::STATE_CONNECT);
    sceneSession->GetDialogVector().push_back(dialogSceneSession2);

    sceneSession->GetDialogVector().push_back(nullptr);

    WSError ret = ssm_->ProcessDialogRequestFocusImmediately(sceneSession);
    ASSERT_NE(ret, WSError::WS_OK);

    dialogSceneSession->persistentId_ = 0;
    ret = ssm_->ProcessDialogRequestFocusImmediately(sceneSession);
    ASSERT_NE(ret, WSError::WS_OK);
}

/**
 * @tc.name: ProcessSubWindowRequestFocusImmediately1
 * @tc.desc: Test sceneSession is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ProcessSubWindowRequestFocusImmediately1, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    sptr<SceneSession> sceneSession = nullptr;
    WSError ret = ssm_->ProcessSubWindowRequestFocusImmediately(sceneSession);
    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: ProcessSubWindowRequestFocusImmediately2
 * @tc.desc: Test subSession list is empty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ProcessSubWindowRequestFocusImmediately2, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "ProcessSubWindowRequestFocusImmediately2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    WSError ret = ssm_->ProcessSubWindowRequestFocusImmediately(sceneSession);
    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: ProcessSubWindowRequestFocusImmediately3
 * @tc.desc: Test sub session is focused
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ProcessSubWindowRequestFocusImmediately3, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "ProcessSubWindowRequestFocusImmediately3";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->persistentId_ = 1;

    SessionInfo subSessionInfo;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subSessionInfo, nullptr);
    subSession->persistentId_ = 2;
    subSession->SetFocusable(true);
    subSession->SetFocusedOnShow(true);
    sceneSession->subSession_.push_back(subSession);

    sceneSession->subSession_.push_back(nullptr);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, subSession));

    ssm_->windowFocusController_->UpdateFocusedSessionId(DEFAULT_DISPLAY_ID, 2);
    WSError ret = ssm_->ProcessSubWindowRequestFocusImmediately(sceneSession);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: ProcessSubWindowRequestFocusImmediately4
 * @tc.desc: Test sub session is not visible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ProcessSubWindowRequestFocusImmediately4, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "ProcessSubWindowRequestFocusImmediately4";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->persistentId_ = 1;

    SessionInfo subSessionInfo;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subSessionInfo, nullptr);
    subSession->persistentId_ = 2;
    subSession->SetFocusable(true);
    subSession->SetFocusedOnShow(true);
    sceneSession->subSession_.push_back(subSession);

    sceneSession->subSession_.push_back(nullptr);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, subSession));

    ssm_->windowFocusController_->UpdateFocusedSessionId(DEFAULT_DISPLAY_ID, 1);
    WSError ret = ssm_->ProcessSubWindowRequestFocusImmediately(sceneSession);
    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: ProcessSubWindowRequestFocusImmediately5
 * @tc.desc: Test sub session request focus success
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ProcessSubWindowRequestFocusImmediately5, TestSize.Level1)
{
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);

    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "ProcessSubWindowRequestFocusImmediately5";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->persistentId_ = 1;

    SessionInfo subSessionInfo;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subSessionInfo, nullptr);
    subSession->persistentId_ = 2;
    subSession->SetFocusable(true);
    subSession->SetFocusedOnShow(true);
    subSession->UpdateVisibilityInner(true);
    subSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->subSession_.push_back(subSession);

    sceneSession->subSession_.push_back(nullptr);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, subSession));

    ssm_->windowFocusController_->UpdateFocusedSessionId(DEFAULT_DISPLAY_ID, 1);
    WSError ret = ssm_->ProcessSubWindowRequestFocusImmediately(sceneSession);
    EXPECT_EQ(ret, WSError::WS_OK);
    ssm_->sceneSessionMap_.clear();
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: NotifyCompleteFirstFrameDrawing03
 * @tc.desc: NotifyCompleteFirstFrameDrawing
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, NotifyCompleteFirstFrameDrawing03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "NotifyCompleteFirstFrameDrawing03";
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->excludeFromMissions = true;
    sessionInfo.abilityInfo = abilityInfo;
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->NotifyCompleteFirstFrameDrawing(1);

    std::shared_ptr<AppExecFwk::EventHandler> eventHandler = std::make_shared<AppExecFwk::EventHandler>();
    ssm_->eventHandler_ = eventHandler;
    abilityInfo->excludeFromMissions = false;
    ssm_->NotifyCompleteFirstFrameDrawing(1);
}

/**
 * @tc.name: SetSessionLabel02
 * @tc.desc: SetSessionLabel
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, SetSessionLabel02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "SetSessionLabel02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<IRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(nullptr, token);
    sceneSession->SetAbilityToken(token);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->listenerController_ = nullptr;

    std::string label = "testLabel";
    ssm_->SetSessionLabel(token, label);

    std::shared_ptr<SessionListenerController> listenerController = std::make_shared<SessionListenerController>();
    ssm_->listenerController_ = listenerController;
    sessionInfo.isSystem_ = false;
    ssm_->SetSessionLabel(token, label);

    sessionInfo.isSystem_ = true;
    ssm_->SetSessionLabel(token, label);
}

/**
 * @tc.name: RecoverAndReconnectSceneSession02
 * @tc.desc: RecoverAndReconnectSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, RecoverAndReconnectSceneSession02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "RecoverAndReconnectSceneSession02";
    sessionInfo.moduleName_ = "moduleTest";
    sessionInfo.appIndex_ = 10;
    sessionInfo.persistentId_ = 1;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->excludeFromMissions = true;
    sessionInfo.abilityInfo = abilityInfo;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);

    sptr<WindowSessionProperty> property = sceneSession->GetSessionProperty();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetPersistentId(1);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->SetWindowState(WindowState::STATE_SHOWN);
    property->SetRequestedOrientation(Orientation::UNSPECIFIED);
    ssm_->alivePersistentIds_.push_back(1);
    ssm_->recoveringFinished_ = false;
    ssm_->recoverSceneSessionFunc_ = NotifyRecoverSceneSessionFuncTest;

    sptr<ISession> session;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto result = ssm_->RecoverAndReconnectSceneSession(nullptr, nullptr, nullptr, session, property, nullptr);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    property->SetPersistentId(2);
    result = ssm_->RecoverAndReconnectSceneSession(nullptr, nullptr, nullptr, session, property, nullptr);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: RefreshPcZorder
 * @tc.desc: RefreshPcZorder
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, RefreshPcZorder, TestSize.Level0)
{
    std::vector<int32_t> persistentIds;
    SessionInfo info1;
    info1.abilityName_ = "RefreshPcZorder1";
    info1.bundleName_ = "RefreshPcZorder1";
    sptr<SceneSession> session1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(session1, nullptr);
    persistentIds.push_back(session1->GetPersistentId());
    ssm_->sceneSessionMap_.insert({ session1->GetPersistentId(), session1 });
    SessionInfo info2;
    info2.abilityName_ = "RefreshPcZorder2";
    info2.bundleName_ = "RefreshPcZorder2";
    sptr<SceneSession> session2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(session2, nullptr);
    persistentIds.push_back(session2->GetPersistentId());
    ssm_->sceneSessionMap_.insert({ session2->GetPersistentId(), session2 });
    SessionInfo info3;
    info3.abilityName_ = "RefreshPcZorder3";
    info3.bundleName_ = "RefreshPcZorder3";
    sptr<SceneSession> session3 = sptr<SceneSession>::MakeSptr(info3, nullptr);
    ASSERT_NE(session3, nullptr);
    session3->SetZOrder(404);
    ssm_->sceneSessionMap_.insert({ session3->GetPersistentId(), session3 });
    persistentIds.push_back(999);
    uint32_t startZOrder = 100;
    std::vector<int32_t> newPersistentIds = persistentIds;
    ssm_->RefreshPcZOrderList(startZOrder, std::move(persistentIds));
    ssm_->RefreshPcZOrderList(UINT32_MAX, std::move(newPersistentIds));
    auto start = std::chrono::system_clock::now();
    // Due to RefreshPcZOrderList being asynchronous, spin lock is added.
    // The spin lock itself is set with a timeout escape time of 3 seconds
    while (true) {
        if ((session1->GetZOrder() != 0 && session2->GetZOrder() != 0 && session1->GetZOrder() != 100) ||
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() >= 3) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ASSERT_EQ(session2->GetZOrder(), 101);
    ASSERT_EQ(session3->GetZOrder(), 404);
    ASSERT_EQ(session1->GetZOrder(), UINT32_MAX);
}

/**
 * @tc.name: GetSessionRSVisible
 * @tc.desc: GetSessionRSVisible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, GetSessionRSVisible, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "GetSessionRSVisible";
    sessionInfo.moduleName_ = "moduleTest";
    uint64_t windowId = 10;
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession01->persistentId_ = windowId;
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
    currVisibleData.push_back(std::make_pair(0, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION));
    currVisibleData.push_back(std::make_pair(1, WindowVisibilityState::WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION));
    struct RSSurfaceNodeConfig config;
    sceneSession02->surfaceNode_ = RSSurfaceNode::Create(config);
    ASSERT_NE(nullptr, sceneSession02->surfaceNode_);
    sceneSession02->surfaceNode_->id_ = 0;
    sceneSession02->persistentId_ = windowId;
    ssm_->sceneSessionMap_.insert(std::make_pair(0, sceneSession02));

    WindowVisibilityState visibleState = WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION;
    bool actual = ssm_->GetSessionRSVisible(sceneSession01, currVisibleData, visibleState);
    EXPECT_EQ(actual, true);
    EXPECT_EQ(visibleState, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
}

/**
 * @tc.name: CheckClickFocusIsDownThroughFullScreen
 * @tc.desc: CheckClickFocusIsDownThroughFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, CheckClickFocusIsDownThroughFullScreen, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "CheckClickFocusIsDownThroughFullScreen";
    info.bundleName_ = "CheckClickFocusIsDownThroughFullScreen";

    sptr<SceneSessionMocker> sceneSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    sceneSession->zOrder_ = 1;
    sptr<SceneSessionMocker> focusedSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    focusedSession->zOrder_ = 2;
    FocusChangeReason reason = FocusChangeReason::CLICK;

    bool ret = ssm_->CheckClickFocusIsDownThroughFullScreen(focusedSession, sceneSession, FocusChangeReason::DEFAULT);
    EXPECT_EQ(ret, false);

    ret = ssm_->CheckClickFocusIsDownThroughFullScreen(focusedSession, nullptr, reason);
    EXPECT_EQ(ret, false);

    ret = ssm_->CheckClickFocusIsDownThroughFullScreen(nullptr, sceneSession, reason);
    EXPECT_EQ(ret, false);

    focusedSession->property_->SetDisplayId(DISPLAY_ID_FAKE);
    sceneSession->property_->SetDisplayId(DEFAULT_DISPLAY_ID);
    ret = ssm_->CheckClickFocusIsDownThroughFullScreen(focusedSession, sceneSession, reason);
    EXPECT_EQ(ret, false);

    focusedSession->property_->SetDisplayId(DEFAULT_DISPLAY_ID);
    focusedSession->blockingFocus_ = false;
    ret = ssm_->CheckClickFocusIsDownThroughFullScreen(focusedSession, sceneSession, reason);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckClickFocusIsDownThroughFullScreen_FullScreenCheck
 * @tc.desc: CheckClickFocusIsDownThroughFullScreen_FullScreenCheck
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, CheckClickFocusIsDownThroughFullScreen_FullScreenCheck, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "CheckClickFocusIsDownThroughFullScreen_FullScreenCheck";
    info.bundleName_ = "CheckClickFocusIsDownThroughFullScreen_FullScreenCheck";

    sptr<SceneSessionMocker> sceneSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    sceneSession->zOrder_ = 1;
    sptr<SceneSessionMocker> focusedSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    focusedSession->zOrder_ = 2;

    focusedSession->blockingFocus_ = true;
    focusedSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    focusedSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    focusedSession->GetSessionProperty()->SetDisplayId(DEFAULT_DISPLAY_ID);
    auto display = DisplayManager::GetInstance().GetDisplayById(DEFAULT_DISPLAY_ID);
    ASSERT_NE(nullptr, display);
    auto displayInfo = display->GetDisplayInfo();
    ASSERT_NE(nullptr, displayInfo);
    auto width = displayInfo->GetWidth();
    auto height = displayInfo->GetHeight();
    focusedSession->SetSessionRect({ 0, 0, width, height });
    std::vector<OHOS::Rosen::Rect> touchHotAreasInSceneSession(0);
    OHOS::Rosen::Rect area = { 0, 0, width, height };
    touchHotAreasInSceneSession.emplace_back(area);
    focusedSession->GetSessionProperty()->SetTouchHotAreas(touchHotAreasInSceneSession);
    bool ret = ssm_->CheckClickFocusIsDownThroughFullScreen(focusedSession, sceneSession, FocusChangeReason::CLICK);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: ShiftFocus
 * @tc.desc: ShiftFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ShiftFocus, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> focusedSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(focusedSession, nullptr);
    sptr<SceneSession> nextSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nextSession, nullptr);
    focusedSession->persistentId_ = 1;
    nextSession->persistentId_ = 4;
    ssm_->sceneSessionMap_.insert({ 1, focusedSession });
    ssm_->sceneSessionMap_.insert({ 4, nextSession });
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    WSError ret = ssm_->ShiftFocus(DEFAULT_DISPLAY_ID, nextSession, false, FocusChangeReason::DEFAULT);
    ASSERT_EQ(ret, WSError::WS_OK);
    ASSERT_EQ(focusedSession->isFocused_, false);
    ASSERT_EQ(nextSession->isFocused_, true);
    ret = ssm_->ShiftFocus(100, nextSession, false, FocusChangeReason::DEFAULT);
}

/**
 * @tc.name: CheckUIExtensionCreation
 * @tc.desc: CheckUIExtensionCreation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, CheckUIExtensionCreation, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    AppExecFwk::ElementName element;
    int32_t windowId = 5;
    uint32_t callingTokenId = 0;
    int32_t pid = 0;
    bool isSystemCalling = SessionPermission::IsSystemCalling();
    AppExecFwk::ExtensionAbilityType extensionAbilityType = AppExecFwk::ExtensionAbilityType::ACTION;
    auto ret = ssm_->CheckUIExtensionCreation(windowId, callingTokenId, element, extensionAbilityType, pid);
    ASSERT_EQ(ret, isSystemCalling ? WMError::WM_ERROR_INVALID_WINDOW : WMError::WM_ERROR_NOT_SYSTEM_APP);

    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, callback);
    ssm_->sceneSessionMap_.insert(std::pair<int32_t, sptr<SceneSession>>(0, sceneSession));
    windowId = 0;

    Session session(info);
    session.getStateFromManagerFunc_ = getStateFalse;
    ret = ssm_->CheckUIExtensionCreation(windowId, callingTokenId, element, extensionAbilityType, pid);
    ASSERT_EQ(ret, isSystemCalling ? WMError::WM_OK : WMError::WM_ERROR_NOT_SYSTEM_APP);

    session.getStateFromManagerFunc_ = getStateTrue;
    ssm_->CheckUIExtensionCreation(windowId, callingTokenId, element, extensionAbilityType, pid);
}

/**
 * @tc.name: CheckUIExtensionCreation01
 * @tc.desc: CheckUIExtensionCreation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, CheckUIExtensionCreation01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    AppExecFwk::ElementName element;
    int32_t windowId = 0;
    uint32_t callingTokenId = 0;
    int32_t pid = 0;
    bool isSystemCalling = SessionPermission::IsSystemCalling();
    AppExecFwk::ExtensionAbilityType extensionAbilityType = AppExecFwk::ExtensionAbilityType::ACTION;

    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, callback);
    ssm_->sceneSessionMap_.insert(std::pair<int32_t, sptr<SceneSession>>(0, sceneSession));
    Session session(info);
    session.getStateFromManagerFunc_ = getStateTrue;
    auto ret = ssm_->CheckUIExtensionCreation(windowId, callingTokenId, element, extensionAbilityType, pid);

    session.property_ = nullptr;
    ret = ssm_->CheckUIExtensionCreation(windowId, callingTokenId, element, extensionAbilityType, pid);
    ASSERT_EQ(ret, isSystemCalling ? WMError::WM_OK : WMError::WM_ERROR_NOT_SYSTEM_APP);

    sceneSession->IsShowOnLockScreen(0);
    session.zOrder_ = 1;
    ssm_->CheckUIExtensionCreation(windowId, callingTokenId, element, extensionAbilityType, pid);
    ASSERT_EQ(ret, isSystemCalling ? WMError::WM_OK : WMError::WM_ERROR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: GetLockScreenZOrder
 * @tc.desc: GetLockScreenZOrder
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, GetLockScreenZOrder, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, callback);
    constexpr uint32_t DEFAULT_LOCK_SCREEN_ZORDER = 2000;
    constexpr uint32_t LOCK_SCREEN_ZORDER = 2003;
    ssm_->sceneSessionMap_.insert(std::pair<int32_t, sptr<SceneSession>>(0, sceneSession));
    ASSERT_EQ(ssm_->GetLockScreenZOrder(), DEFAULT_LOCK_SCREEN_ZORDER);
    ASSERT_EQ(sceneSession->IsScreenLockWindow(), false);
    ASSERT_EQ(sceneSession->GetZOrder(), 0);

    info.bundleName_ = "SCBScreenLock2";
    sptr<SceneSession> lockScreenSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert(std::pair<int32_t, sptr<SceneSession>>(1, lockScreenSession));
    lockScreenSession->SetZOrder(0);
    ASSERT_EQ(ssm_->GetLockScreenZOrder(), DEFAULT_LOCK_SCREEN_ZORDER);
    lockScreenSession->SetZOrder(DEFAULT_LOCK_SCREEN_ZORDER);
    ASSERT_EQ(ssm_->GetLockScreenZOrder(), DEFAULT_LOCK_SCREEN_ZORDER);
    lockScreenSession->SetZOrder(DEFAULT_LOCK_SCREEN_ZORDER - 1);
    ASSERT_EQ(ssm_->GetLockScreenZOrder(), DEFAULT_LOCK_SCREEN_ZORDER);
    lockScreenSession->SetZOrder(LOCK_SCREEN_ZORDER);
    ASSERT_EQ(lockScreenSession->IsScreenLockWindow(), true);
    ASSERT_EQ(lockScreenSession->GetZOrder(), LOCK_SCREEN_ZORDER);
    ASSERT_EQ(ssm_->sceneSessionMap_.size(), 2);
    ASSERT_EQ(ssm_->GetLockScreenZOrder(), LOCK_SCREEN_ZORDER);
}

/**
 * @tc.name: IsLastPiPWindowVisible
 * @tc.desc: IsLastPiPWindowVisible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, IsLastPiPWindowVisible, TestSize.Level1)
{
    uint64_t surfaceId = 0;
    WindowVisibilityState lastVisibilityState = WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION;
    ssm_->sceneSessionMap_.insert(std::pair<int32_t, sptr<SceneSession>>(0, nullptr));
    auto ret = ssm_->IsLastPiPWindowVisible(surfaceId, lastVisibilityState);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: IsLastPiPWindowVisible01
 * @tc.desc: IsLastPiPWindowVisible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, IsLastPiPWindowVisible01, TestSize.Level1)
{
    uint64_t surfaceId = 0;
    WindowVisibilityState lastVisibilityState = WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION;

    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, callback);
    ssm_->sceneSessionMap_.insert(std::pair<int32_t, sptr<SceneSession>>(0, nullptr));

    struct RSSurfaceNodeConfig config;
    sceneSession->surfaceNode_ = RSSurfaceNode::Create(config);
    ASSERT_NE(nullptr, sceneSession->surfaceNode_);
    sceneSession->surfaceNode_->id_ = 0;
    ssm_->SelectSesssionFromMap(0);
    sptr<WindowSessionProperty> property = sceneSession->GetSessionProperty();
    property->SetWindowMode(WindowMode::WINDOW_MODE_PIP);

    auto ret = ssm_->IsLastPiPWindowVisible(surfaceId, lastVisibilityState);
    ASSERT_EQ(ret, false);
    ssm_->isScreenLocked_ = true;
    ret = ssm_->IsLastPiPWindowVisible(surfaceId, lastVisibilityState);
    ASSERT_EQ(ret, false);
    lastVisibilityState = WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION;
    ret = ssm_->IsLastPiPWindowVisible(surfaceId, lastVisibilityState);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: UpdateSpecificSessionClientDisplayId01
 * @tc.desc: UpdateSpecificSessionClientDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, UpdateSpecificSessionClientDisplayId01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "UpdateSpecificSessionClientDisplayId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->persistentId_ = 1;
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetParentPersistentId(1);
    auto displayId = ssm_->UpdateSpecificSessionClientDisplayId(property);
    EXPECT_EQ(0, displayId);
    EXPECT_EQ(0, property->GetDisplayId());

    property->SetDisplayId(999);
    displayId = ssm_->UpdateSpecificSessionClientDisplayId(property);
    EXPECT_EQ(999, displayId);
    EXPECT_EQ(0, property->GetDisplayId());

    property->SetDisplayId(999);
    property->SetIsFollowParentWindowDisplayId(true);
    displayId = ssm_->UpdateSpecificSessionClientDisplayId(property);
    EXPECT_EQ(0, displayId);
    EXPECT_EQ(0, property->GetDisplayId());
}

/**
 * @tc.name: UpdateSpecificSessionClientDisplayId02
 * @tc.desc: UpdateSpecificSessionClientDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, UpdateSpecificSessionClientDisplayId02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "UpdateSpecificSessionClientDisplayId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->persistentId_ = 1;
    sceneSession->SetClientDisplayId(999);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetParentPersistentId(1);
    auto displayId = ssm_->UpdateSpecificSessionClientDisplayId(property);
    EXPECT_EQ(999, displayId);
    EXPECT_EQ(0, property->GetDisplayId());

    property->SetDisplayId(999);
    displayId = ssm_->UpdateSpecificSessionClientDisplayId(property);
    EXPECT_EQ(999, displayId);
    EXPECT_EQ(0, property->GetDisplayId());
}

/**
 * @tc.name: UpdateSpecificSessionClientDisplayId03
 * @tc.desc: UpdateSpecificSessionClientDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, UpdateSpecificSessionClientDisplayId03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "UpdateSpecificSessionClientDisplayId";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    auto displayId = ssm_->UpdateSpecificSessionClientDisplayId(property);
    EXPECT_EQ(0, displayId);
    EXPECT_EQ(0, property->GetDisplayId());

    property->SetDisplayId(999);
    displayId = ssm_->UpdateSpecificSessionClientDisplayId(property);
    EXPECT_EQ(999, displayId);
    EXPECT_EQ(0, property->GetDisplayId());
}

/**
 * @tc.name: SetSkipEventOnCastPlusInner01
 * @tc.desc: SetSkipEventOnCastPlusInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, SetSkipEventOnCastPlusInner01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "SetSkipEventOnCastPlusInner";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->SetSkipEventOnCastPlusInner(sceneSession->GetPersistentId(), true);
    EXPECT_EQ(true, sceneSession->GetSessionProperty()->GetSkipEventOnCastPlus());
    ssm_->SetSkipEventOnCastPlusInner(sceneSession->GetPersistentId(), false);
    EXPECT_EQ(false, sceneSession->GetSessionProperty()->GetSkipEventOnCastPlus());
    ssm_->isUserBackground_ = false;
    ssm_->SetSkipEventOnCastPlusInner(sceneSession->GetPersistentId(), false);
    EXPECT_EQ(false, sceneSession->GetSessionProperty()->GetSkipEventOnCastPlus());
    ssm_->isUserBackground_ = true;
    ssm_->SetSkipEventOnCastPlusInner(sceneSession->GetPersistentId(), false);
    EXPECT_EQ(false, sceneSession->GetSessionProperty()->GetSkipEventOnCastPlus());
}

/**
 * @tc.name: SetParentWindowInner
 * @tc.desc: SetParentWindowInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, SetParentWindowInner, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest9";
    sessionInfo.abilityName_ = "SetParentWindowInner";
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> oldParentSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> newParentSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    oldParentSession->property_->SetSubWindowLevel(10);
    oldParentSession->persistentId_ = 10;
    newParentSession->property_->SetSubWindowLevel(1);
    newParentSession->persistentId_ = 1;
    subSession->persistentId_ = 100;
    subSession->isFocused_ = true;
    subSession->SetExclusivelyHighlighted(false);
    ssm_->SetParentWindowInner(subSession, oldParentSession, newParentSession);
}

/**
 * @tc.name: ResetSpecificWindowZIndex
 * @tc.desc: test function : ResetSpecificWindowZIndex
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest9, ResetSpecificWindowZIndex, TestSize.Level1)
{
    WSError ret = ssm_->ResetSpecificWindowZIndex(123);
    EXPECT_EQ(ret, WSError::WS_ERROR_NULLPTR);

    ssm_->specificZIndexByPidMap_[WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD] = 123;
    ret = ssm_->ResetSpecificWindowZIndex(123);
    EXPECT_EQ(ret, WSError::WS_OK);

    SetSpecificZIndexReason setReason = SetSpecificZIndexReason::SET;
    NotifySetSpecificWindowZIndexFunc func = [&setReason](WindowType windowType, int32_t zIndex,
        SetSpecificZIndexReason reason) {
        setReason = reason;
    };
    ssm_->SetSpecificWindowZIndexListener(func);
    ret = ssm_->ResetSpecificWindowZIndex(123);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(setReason, SetSpecificZIndexReason::RESET);
    ssm_->SetSpecificWindowZIndexListener(nullptr);
    ssm_->specificZIndexByPidMap_.clear();
}
} // namespace
} // namespace Rosen
} // namespace OHOS