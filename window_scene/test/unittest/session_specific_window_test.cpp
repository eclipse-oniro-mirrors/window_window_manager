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

#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "session_info.h"
#include "session_manager.h"
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "window_manager_agent.h"
#include "zidl/window_manager_agent_interface.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class SessionSpecificWindowTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<SceneSessionManager> ssm_;

private:
    sptr<Session> session_ = nullptr;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;

    sptr<SessionStageMocker> mockSessionStage_ = nullptr;
    sptr<WindowEventChannelMocker> mockEventChannel_ = nullptr;
};

void SessionSpecificWindowTest::SetUpTestCase() {}

void SessionSpecificWindowTest::TearDownTestCase() {}

void SessionSpecificWindowTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    session_ = sptr<Session>::MakeSptr(info);
    ssm_ = sptr<SceneSessionManager>::MakeSptr();

    mockSessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage_, nullptr);

    mockEventChannel_ = sptr<WindowEventChannelMocker>::MakeSptr(mockSessionStage_);
    ASSERT_NE(mockEventChannel_, nullptr);
}

void SessionSpecificWindowTest::TearDown()
{
    session_ = nullptr;
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: BindDialogSessionTarget
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, BindDialogSessionTarget, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "BindDialogSessionTarget";
    info.bundleName_ = "BindDialogSessionTarget";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    property->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    sceneSession->SetSessionProperty(property);

    sptr<SceneSession> sceneSession1 = nullptr;
    WSError result = sceneSession->BindDialogSessionTarget(sceneSession1);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    sptr<SceneSession> sceneSession2 = sceneSession;
    result = sceneSession->BindDialogSessionTarget(sceneSession2);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: AddSubSession
 * @tc.desc: AddSubSession Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, AddSubSession, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySessionException";
    info.bundleName_ = "NotifySessionException";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> subSession = nullptr;
    bool res = session->AddSubSession(subSession);
    ASSERT_EQ(res, false);

    subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    res = session->AddSubSession(subSession);
    ASSERT_EQ(res, true);

    res = session->AddSubSession(subSession);
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: RemoveSubSession
 * @tc.desc: RemoveSubSession Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, RemoveSubSession, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySessionException";
    info.bundleName_ = "NotifySessionException";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    bool res = session->RemoveSubSession(0);
    ASSERT_EQ(res, false);

    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    res = session->AddSubSession(subSession);
    ASSERT_EQ(res, true);

    res = session->RemoveSubSession(subSession->GetPersistentId());
    ASSERT_EQ(res, true);
}

/**
 * @tc.name: ClearSpecificSessionCbMap
 * @tc.desc: ClearSpecificSessionCbMap
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, ClearSpecificSessionCbMap, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ClearSpecificSessionCbMap";
    info.bundleName_ = "ClearSpecificSessionCbMap";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto result = false;
    sceneSession->clearCallbackMapFunc_ = [&result](bool needRemove) { result = needRemove; };
    sceneSession->ClearSpecificSessionCbMap();
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(result, true);
}

/**
 * @tc.name: ClearSpecificSessionCbMap
 * @tc.desc: ClearSpecificSessionCbMap
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, ClearSpecificSessionCbMap01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ClearSpecificSessionCbMap01";
    info.bundleName_ = "ClearSpecificSessionCbMap01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    sceneSession->clearCallbackMapFunc_ = nullptr;
    sceneSession->ClearSpecificSessionCbMap();
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: ClearSpecificSessionCbMap
 * @tc.desc: ClearSpecificSessionCbMap
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, SpecificCallback01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ClearSpecificSessionCbMap01";
    info.bundleName_ = "ClearSpecificSessionCbMap01";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_FLOAT);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    bool isFromClient = true;
    sceneSession->needSnapshot_ = true;
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    bool result = false;
    auto specificCallback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCallback->onHandleSecureSessionShouldHide_ = [&result](const sptr<SceneSession>& sceneSession) {
        result = sceneSession->needSnapshot_;
        return WSError::WS_OK;
    };
    sceneSession->specificCallback_ = specificCallback;
    sceneSession->Disconnect(isFromClient);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(result, false);
    ASSERT_EQ(SessionState::STATE_DISCONNECT, sceneSession->state_);
}

/**
 * @tc.name: ClearSpecificSessionCbMap
 * @tc.desc: ClearSpecificSessionCbMap
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, SpecificCallback02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ClearSpecificSessionCbMap01";
    info.bundleName_ = "ClearSpecificSessionCbMap01";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_FLOAT);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    bool isFromClient = false;
    sceneSession->needSnapshot_ = true;
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    bool result = false;
    auto specificCallback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCallback->onHandleSecureSessionShouldHide_ = [&result](const sptr<SceneSession>& sceneSession) {
        result = sceneSession->needSnapshot_;
        return WSError::WS_OK;
    };
    sceneSession->specificCallback_ = specificCallback;
    sceneSession->Disconnect(isFromClient);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(result, true);
    ASSERT_EQ(SessionState::STATE_DISCONNECT, sceneSession->state_);
}

/**
 * @tc.name: GetKeyboardAvoidArea
 * @tc.desc: GetKeyboardAvoidArea01
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, GetKeyboardAvoidArea01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetKeyboardAvoidArea";
    info.bundleName_ = "GetKeyboardAvoidArea";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WSRect rect{ 100, 100, 100, 100 };
    AvoidArea avoidArea;
    GTEST_LOG_(INFO) << "1";
    sceneSession->GetKeyboardAvoidArea(rect, avoidArea);
    Rect result{ 0, 0, 0, 0 };
    ASSERT_EQ(avoidArea.topRect_, result);
}

/**
 * @tc.name: IsSystemSpecificSession
 * @tc.desc: IsSystemSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, IsSystemSpecificSession, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsSystemSpecificSession";
    info.bundleName_ = "IsSystemSpecificSession";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_FALSE(sceneSession->IsSystemSpecificSession());
    sceneSession->SetIsSystemSpecificSession(true);
    ASSERT_TRUE(sceneSession->IsSystemSpecificSession());
}

/**
 * @tc.name: SetAndIsSystemKeyboard
 * @tc.desc: test SetIsSystemKeyboard and IsSystemKeyboard func
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, SetAndIsSystemKeyboard, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAndIsSystemKeyboard";
    info.bundleName_ = "SetAndIsSystemKeyboard";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    ASSERT_EQ(false, session->IsSystemKeyboard());
    session->SetIsSystemKeyboard(true);
    ASSERT_EQ(true, session->IsSystemKeyboard());
}

/**
 * @tc.name: CheckDialogOnForeground
 * @tc.desc: check func CheckDialogOnForeground
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, CheckDialogOnForeground, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->dialogVec_.clear();
    ASSERT_EQ(false, session_->CheckDialogOnForeground());
    SessionInfo info;
    info.abilityName_ = "dialogAbilityName";
    info.moduleName_ = "dialogModuleName";
    info.bundleName_ = "dialogBundleName";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info);
    dialogSession->state_ = SessionState::STATE_INACTIVE;
    session_->dialogVec_.push_back(dialogSession);
    ASSERT_EQ(false, session_->CheckDialogOnForeground());
    session_->dialogVec_.clear();
}

/**
 * @tc.name: IsTopDialog
 * @tc.desc: check func IsTopDialog
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, IsTopDialog, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->dialogVec_.clear();
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";

    sptr<Session> dialogSession1 = sptr<Session>::MakeSptr(info);
    dialogSession1->persistentId_ = 33;
    dialogSession1->SetParentSession(session_);
    dialogSession1->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(dialogSession1);

    sptr<Session> dialogSession2 = sptr<Session>::MakeSptr(info);
    dialogSession2->persistentId_ = 34;
    dialogSession2->SetParentSession(session_);
    dialogSession2->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(dialogSession2);

    sptr<Session> dialogSession3 = sptr<Session>::MakeSptr(info);
    dialogSession3->persistentId_ = 35;
    dialogSession3->SetParentSession(session_);
    dialogSession3->state_ = SessionState::STATE_INACTIVE;
    session_->dialogVec_.push_back(dialogSession3);

    ASSERT_EQ(false, dialogSession3->IsTopDialog());
    ASSERT_EQ(true, dialogSession2->IsTopDialog());
    ASSERT_EQ(false, dialogSession1->IsTopDialog());
    session_->dialogVec_.clear();
}

/**
 * @tc.name: IsTopDialog02
 * @tc.desc: IsTopDialog Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, IsTopDialog02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetParentSession(nullptr);
    EXPECT_EQ(false, session_->IsTopDialog());

    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> parentSession = sptr<Session>::MakeSptr(info);
    parentSession->dialogVec_.clear();
    session_->SetParentSession(parentSession);
    auto result = session_->IsTopDialog();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsTopDialog03
 * @tc.desc: IsTopDialog Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, IsTopDialog03, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->dialogVec_.clear();
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> dialogSession1 = sptr<Session>::MakeSptr(info);
    sptr<Session> dialogSession2 = sptr<Session>::MakeSptr(info);
    dialogSession1->SetParentSession(session_);
    dialogSession2->SetParentSession(session_);
    session_->dialogVec_.push_back(dialogSession1);
    session_->dialogVec_.push_back(dialogSession2);
    dialogSession1->SetSessionState(SessionState::STATE_INACTIVE);
    dialogSession2->SetSessionState(SessionState::STATE_INACTIVE);
    EXPECT_EQ(false, dialogSession1->IsTopDialog());
}

/**
 * @tc.name: BindDialogToParentSession
 * @tc.desc: BindDialogToParentSession Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, BindDialogToParentSession, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session_->BindDialogToParentSession(session);

    sptr<Session> session1 = sptr<Session>::MakeSptr(info);
    session1->persistentId_ = 33;
    session1->SetParentSession(session_);
    session1->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(session1);

    sptr<Session> session2 = sptr<Session>::MakeSptr(info);
    session2->persistentId_ = 34;
    session2->SetParentSession(session_);
    session2->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(session2);
    session_->BindDialogToParentSession(session1);

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: HandlePointDownDialog
 * @tc.desc: HandlePointDownDialog Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, HandlePointDownDialog, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> dialogSession1 = sptr<Session>::MakeSptr(info);
    sptr<Session> dialogSession2 = sptr<Session>::MakeSptr(info);
    sptr<Session> dialogSession3 = sptr<Session>::MakeSptr(info);
    sptr<Session> dialogSession4 = nullptr;
    dialogSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    dialogSession2->SetSessionState(SessionState::STATE_ACTIVE);
    dialogSession2->SetSessionState(SessionState::STATE_INACTIVE);
    session_->dialogVec_.push_back(dialogSession1);
    session_->dialogVec_.push_back(dialogSession2);
    session_->dialogVec_.push_back(dialogSession3);
    session_->dialogVec_.push_back(dialogSession4);
    session_->HandlePointDownDialog();
    session_->SetPendingSessionToBackgroundForDelegatorListener(nullptr);
    EXPECT_EQ(WSError::WS_OK, session_->PendingSessionToBackgroundForDelegator(true));
}

/**
 * @tc.name: RemoveDialogToParentSession
 * @tc.desc: RemoveDialogToParentSession Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, RemoveDialogToParentSession, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session_->RemoveDialogToParentSession(session);

    sptr<Session> session1 = sptr<Session>::MakeSptr(info);
    session1->persistentId_ = 33;
    session1->SetParentSession(session_);
    session1->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(session1);

    sptr<Session> session2 = sptr<Session>::MakeSptr(info);
    session2->persistentId_ = 34;
    session2->SetParentSession(session_);
    session2->state_ = SessionState::STATE_ACTIVE;
    session_->dialogVec_.push_back(session2);
    session_->RemoveDialogToParentSession(session1);

    ASSERT_EQ(WSError::WS_OK, session_->SetFocusable(false));
}

/**
 * @tc.name: IsSystemSession
 * @tc.desc: IsSystemSession
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, IsSystemSession, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool res = session_->IsSystemSession();
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: HandleDialogBackground
 * @tc.desc: HandleDialogBackground Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, HandleDialogBackground, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    session_->HandleDialogBackground();

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<Session> session01 = nullptr;

    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> session02 = sptr<Session>::MakeSptr(info);
    sptr<Session> session03 = sptr<Session>::MakeSptr(info);

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    session02->sessionStage_ = mockSessionStage;
    session03->sessionStage_ = nullptr;

    session_->dialogVec_.push_back(session01);
    session_->dialogVec_.push_back(session02);
    session_->dialogVec_.push_back(session03);
    session_->HandleDialogBackground();
    session_->SetPendingSessionToBackgroundForDelegatorListener(nullptr);
    EXPECT_EQ(WSError::WS_OK, session_->PendingSessionToBackgroundForDelegator(true));
}

/**
 * @tc.name: HandleDialogForeground
 * @tc.desc: HandleDialogForeground Test
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, HandleDialogForeground, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    session_->HandleDialogForeground();

    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<Session> session01 = nullptr;

    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> session02 = sptr<Session>::MakeSptr(info);
    sptr<Session> session03 = sptr<Session>::MakeSptr(info);

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, mockSessionStage);
    session02->sessionStage_ = mockSessionStage;
    session03->sessionStage_ = nullptr;

    session_->dialogVec_.push_back(session01);
    session_->dialogVec_.push_back(session02);
    session_->dialogVec_.push_back(session03);
    session_->HandleDialogForeground();
    session_->SetPendingSessionToBackgroundForDelegatorListener(nullptr);
    EXPECT_EQ(WSError::WS_OK, session_->PendingSessionToBackgroundForDelegator(true));
}

/**
 * @tc.name: HandleSubWindowClick01
 * @tc.desc: parentSession and property is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, HandleSubWindowClick01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    auto result = session_->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_DOWN, 0);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: HandleSubWindowClick03
 * @tc.desc: parentSession->dialogVec_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, HandleSubWindowClick03, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info);
    session_->SetParentSession(dialogSession);

    auto result = session_->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_DOWN, 0);
    EXPECT_EQ(result, WSError::WS_OK);

    result = session_->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_MOVE, 0);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: HandleSubWindowClick03
 * @tc.desc: isExecuteDelayRaise is true
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, HandleSubWindowClick04, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info);
    session_->SetParentSession(dialogSession);

    session_->property_->SetRaiseEnabled(true);
    int32_t action = MMI::PointerEvent::POINTER_ACTION_BUTTON_UP;
    bool isExecuteDelayRaise = true;
    auto result = session_->HandleSubWindowClick(action, 0, isExecuteDelayRaise);
    EXPECT_EQ(result, WSError::WS_OK);

    session_->property_->SetRaiseEnabled(false);
    result = session_->HandleSubWindowClick(action, 0, isExecuteDelayRaise);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: HandleSubWindowClick05
 * @tc.desc: HandleSubWindowClick
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, HandleSubWindowClick05, Function | SmallTest | Level2)
{
    SessionInfo info1;
    info1.abilityName_ = "testSession1";
    info1.moduleName_ = "testSession1";
    info1.bundleName_ = "testSession1";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info1);

    SessionInfo info2;
    info2.abilityName_ = "testSession2";
    info2.moduleName_ = "testSession2";
    info2.bundleName_ = "testSession2";
    auto session = sptr<Session>::MakeSptr(info2);
    session->SetParentSession(dialogSession);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetRaiseEnabled(false);
    session->SetSessionProperty(property);

    auto result = session->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_DOWN, 0, false);
    EXPECT_EQ(result, WSError::WS_OK);

    result = session->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_DOWN, 0, true);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: HandleSubWindowClick06
 * @tc.desc: HandleSubWindowClick
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, HandleSubWindowClick06, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> subSession = sptr<Session>::MakeSptr(info);
    subSession->SetParentSession(session_);
    auto property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL));
    subSession->SetSessionProperty(property);
    bool isExecuteDelayRaise = false;
    auto result = session_->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN, 0, isExecuteDelayRaise);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: HandleSubWindowClick07
 * @tc.desc: HandleSubWindowClick
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, HandleSubWindowClick07, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    bool hasNotifyManagerToRequestFocus = false;
    session_->SetRaiseToAppTopForPointDownFunc([&hasNotifyManagerToRequestFocus]() {
        hasNotifyManagerToRequestFocus = true;
    });
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> subSession = sptr<Session>::MakeSptr(info);
    subSession->SetParentSession(session_);
    session_->property_->SetRaiseEnabled(true);
    hasNotifyManagerToRequestFocus = false;
    session_->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER,
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, false);
    EXPECT_EQ(hasNotifyManagerToRequestFocus, true);
}

/**
 * @tc.name: HandleSubWindowClick08
 * @tc.desc: HandleSubWindowClick
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, HandleSubWindowClick08, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    sptr<Session> parentSession = sptr<Session>::MakeSptr(info);

    bool hasNotifyManagerToRaise = false;
    parentSession->SetClickListener([&hasNotifyManagerToRaise](bool requestFocus, bool isClick) {
        hasNotifyManagerToRaise = true;
    });

    session_->property_->SetRaiseEnabled(false);
    hasNotifyManagerToRaise = false;
    session_->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER,
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, false);
    EXPECT_EQ(hasNotifyManagerToRaise, false);

    session_->SetParentSession(parentSession);
    hasNotifyManagerToRaise = false;
    session_->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_MOVE,
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, false);
    EXPECT_EQ(hasNotifyManagerToRaise, false);

    hasNotifyManagerToRaise = false;
    session_->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER,
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, false);
    EXPECT_EQ(hasNotifyManagerToRaise, true);

    hasNotifyManagerToRaise = false;
    session_->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_DOWN,
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, false);
    EXPECT_EQ(hasNotifyManagerToRaise, true);
}

/**
 * @tc.name: HandleSubWindowClick09
 * @tc.desc: test the modal sub window click
 * @tc.type: FUNC
 */
HWTEST_F(SessionSpecificWindowTest, HandleSubWindowClick09, Function | SmallTest | Level2)
{
    SessionInfo info1;
    info1.abilityName_ = "mainSession";
    info1.moduleName_ = "mainSession";
    info1.bundleName_ = "mainSession";
    sptr<Session> mainSession = sptr<Session>::MakeSptr(info1);
    SessionInfo info2;
    info2.abilityName_ = "subSession";
    info2.moduleName_ = "subSession";
    info2.bundleName_ = "subSession";
    sptr<Session> subSession = sptr<Session>::MakeSptr(info2);

    bool hasNotifyManagerToRaise = false;
    mainSession->SetClickListener([&hasNotifyManagerToRaise](bool requestFocus, bool isClick) {
        hasNotifyManagerToRaise = true;
    });
    subSession->SetParentSession(mainSession);

    auto property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL));
    subSession->SetSessionProperty(property);

    subSession->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_DOWN,
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(hasNotifyManagerToRaise, true);

    subSession->SetParentSession(nullptr);
    auto ret = subSession->HandleSubWindowClick(MMI::PointerEvent::POINTER_ACTION_DOWN,
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    EXPECT_EQ(ret, WSError::WS_OK);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
