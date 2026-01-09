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
#include "session/host/include/system_session.h"

#include "common/include/session_permission.h"
#include "key_event.h"
#include "mock/mock_accesstoken_kit.h"
#include "mock/mock_session.h"
#include "mock/mock_session_stage.h"
#include "session/host/include/session.h"
#include <ui/rs_surface_node.h>
#include "window_event_channel_base.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "pointer_event.h"
#include "string_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
constexpr int WAIT_ASYNC_US = 1000000;
class SystemSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    SessionInfo info;
    sptr<SystemSession::SpecificSessionCallback> specificCallback = nullptr;
    sptr<SystemSession> systemSession_;

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    sptr<SystemSession> GetSystemSession(const std::string& name);
    sptr<SceneSession> GetSceneSession(const std::string& name);
};

void SystemSessionTest::SetUpTestCase() {}

void SystemSessionTest::TearDownTestCase() {}

void SystemSessionTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testSystemSession1";
    info.moduleName_ = "testSystemSession2";
    info.bundleName_ = "testSystemSession3";
    systemSession_ = sptr<SystemSession>::MakeSptr(info, specificCallback);
    EXPECT_NE(nullptr, systemSession_);
}

void SystemSessionTest::TearDown()
{
    systemSession_ = nullptr;
}

RSSurfaceNode::SharedPtr SystemSessionTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    return surfaceNode;
}

sptr<SystemSession> SystemSessionTest::GetSystemSession(const std::string& name)
{
    SessionInfo info;
    info.abilityName_ = name;
    info.moduleName_ = name;
    info.bundleName_ = name;
    auto sysSession = sptr<SystemSession>::MakeSptr(info, nullptr);
    return sysSession;
}

sptr<SceneSession> SystemSessionTest::GetSceneSession(const std::string& name)
{
    SessionInfo info;
    info.abilityName_ = name;
    info.moduleName_ = name;
    info.bundleName_ = name;
    auto sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    return sceneSession;
}

namespace {

/**
 * @tc.name: TransferKeyEvent01
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, TransferKeyEvent01, TestSize.Level1)
{
    systemSession_->state_ = SessionState::STATE_END;

    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, systemSession_->TransferKeyEvent(nullptr));
}

/**
 * @tc.name: TransferKeyEvent02
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, TransferKeyEvent02, TestSize.Level1)
{
    systemSession_->state_ = SessionState::STATE_CONNECT;
    std::shared_ptr<MMI::KeyEvent> keyEvent = nullptr;

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, systemSession_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: ProcessBackEvent01
 * @tc.desc: check func ProcessBackEvent
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, ProcessBackEvent01, TestSize.Level1)
{
    systemSession_->state_ = SessionState::STATE_END;

    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, systemSession_->ProcessBackEvent());
}

/**
 * @tc.name: NotifyClientToUpdateRect01
 * @tc.desc: check func NotifyClientToUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, NotifyClientToUpdateRect01, TestSize.Level1)
{
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    systemSession_->sessionStage_ = mockSessionStage;
    auto ret = systemSession_->NotifyClientToUpdateRect("SystemSessionTest", nullptr);
    ASSERT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: CheckPointerEventDispatch
 * @tc.desc: check func CheckPointerEventDispatch
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, CheckPointerEventDispatch, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent_ = MMI::PointerEvent::Create();
    SessionInfo info;
    info.abilityName_ = "CheckPointerEventDispatch";
    info.bundleName_ = "CheckPointerEventDispatchBundleName";
    info.windowType_ = 2122;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SystemSession> sysSession = sptr<SystemSession>::MakeSptr(info, specificCallback_);
    sysSession->SetSessionState(SessionState::STATE_FOREGROUND);
    bool ret1 = sysSession->CheckPointerEventDispatch(pointerEvent_);
    ASSERT_EQ(true, ret1);
}

/**
 * @tc.name: UpdatePointerArea
 * @tc.desc: check func UpdatePointerArea
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, UpdatePointerArea, TestSize.Level1)
{
    WSRect rect = { 1, 1, 1, 1 };
    SessionInfo info;
    info.abilityName_ = "UpdatePointerArea";
    info.bundleName_ = "UpdatePointerAreaBundleName";
    info.windowType_ = 2122;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SystemSession> sysSession = sptr<SystemSession>::MakeSptr(info, specificCallback_);
    sysSession->UpdatePointerArea(rect);
    ASSERT_NE(sysSession->preRect_, rect);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->SetDecorEnable(true);
    sysSession->property_ = property;
    sysSession->UpdatePointerArea(rect);
    ASSERT_EQ(sysSession->preRect_, rect);
}

/**
 * @tc.name: ProcessPointDownSession
 * @tc.desc: test function : ProcessPointDownSession
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, ProcessPointDownSession, TestSize.Level1)
{
    ASSERT_TRUE(systemSession_ != nullptr);

    int32_t posX = 2;
    int32_t posY = 3;
    auto ret = systemSession_->ProcessPointDownSession(posX, posY);
    ASSERT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: GetMissionId
 * @tc.desc: test function : GetMissionId
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, GetMissionId, TestSize.Level1)
{
    ASSERT_TRUE(systemSession_ != nullptr);
    SessionInfo info;
    info.abilityName_ = "testSystemSession1";
    info.moduleName_ = "testSystemSession2";
    info.bundleName_ = "testSystemSession3";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    systemSession_->parentSession_ = session;
    auto ret = systemSession_->GetMissionId();
    ASSERT_EQ(0, ret);
}

/**
 * @tc.name: RectCheck
 * @tc.desc: test function : RectCheck
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, RectCheck, TestSize.Level1)
{
    ASSERT_TRUE(systemSession_ != nullptr);
    SessionInfo info;
    info.abilityName_ = "testRectCheck";
    info.moduleName_ = "testRectCheck";
    info.bundleName_ = "testRectCheck";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    EXPECT_NE(nullptr, session);
    systemSession_->parentSession_ = session;
    uint32_t curWidth = 100;
    uint32_t curHeight = 200;
    systemSession_->RectCheck(curWidth, curHeight);

    curWidth = 0;
    curHeight = 0;
    systemSession_->RectCheck(curWidth, curHeight);

    curWidth = 1930;
    curHeight = 0;
    systemSession_->RectCheck(curWidth, curHeight);

    curWidth = 330;
    curHeight = 0;
    systemSession_->RectCheck(curWidth, curHeight);

    curWidth = 330;
    curHeight = 1930;
    systemSession_->RectCheck(curWidth, curHeight);
}

/**
 * @tc.name: SetDialogSessionBackGestureEnabled01
 * @tc.desc: test function : SetDialogSessionBackGestureEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, SetDialogSessionBackGestureEnabled01, TestSize.Level1)
{
    ASSERT_TRUE(systemSession_ != nullptr);
    SessionInfo info;
    info.abilityName_ = "SetDialogSessionBackGestureEnabled";
    info.moduleName_ = "SetDialogSessionBackGestureEnabled";
    info.bundleName_ = "SetDialogSessionBackGestureEnabled";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    EXPECT_NE(nullptr, session);

    systemSession_->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    auto ret = systemSession_->SetDialogSessionBackGestureEnabled(true);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_CALLING);
}

/**
 * @tc.name: SetDialogSessionBackGestureEnabled02
 * @tc.desc: test function : SetDialogSessionBackGestureEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, SetDialogSessionBackGestureEnabled02, TestSize.Level1)
{
    ASSERT_TRUE(systemSession_ != nullptr);
    SessionInfo info;
    info.abilityName_ = "SetDialogSessionBackGestureEnabled02";
    info.moduleName_ = "SetDialogSessionBackGestureEnabled02";
    info.bundleName_ = "SetDialogSessionBackGestureEnabled02";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    EXPECT_NE(nullptr, session);

    systemSession_->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    auto ret = systemSession_->SetDialogSessionBackGestureEnabled(true);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: UpdateCameraWindowStatus01
 * @tc.desc: test function : UpdateCameraWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, UpdateCameraWindowStatus01, TestSize.Level1)
{
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(specificCallback, nullptr);
    bool result = false;

    systemSession_->UpdateCameraWindowStatus(true);

    systemSession_->specificCallback_ = specificCallback;
    systemSession_->UpdateCameraWindowStatus(true);

    systemSession_->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT_CAMERA);
    systemSession_->UpdateCameraWindowStatus(true);

    systemSession_->specificCallback_->onCameraFloatSessionChange_ = [&result](uint32_t accessTokenId, bool isShowing) {
        result = isShowing;
    };
    systemSession_->UpdateCameraWindowStatus(true);
    ASSERT_EQ(result, true);
}

/**
 * @tc.name: UpdateCameraWindowStatus02
 * @tc.desc: test function : UpdateCameraWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, UpdateCameraWindowStatus02, TestSize.Level1)
{
    auto sysSession = GetSystemSession("UpdateCameraWindowStatus02");
    ASSERT_NE(sysSession, nullptr);
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(specificCallback, nullptr);
    bool result = false;
    sysSession->specificCallback_ = specificCallback;

    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    sysSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    sysSession->UpdateCameraWindowStatus(true);

    sysSession->specificCallback_->onCameraSessionChange_ = [&result](uint32_t accessTokenId, bool isShowing) {
        result = isShowing;
    };

    result = false;
    sysSession->pipTemplateInfo_.pipTemplateType = static_cast<uint32_t>(PiPTemplateType::VIDEO_CALL);
    sysSession->UpdateCameraWindowStatus(true);
    ASSERT_EQ(result, true);

    result = false;
    sysSession->pipTemplateInfo_.pipTemplateType = static_cast<uint32_t>(PiPTemplateType::VIDEO_MEETING);
    sysSession->UpdateCameraWindowStatus(true);
    ASSERT_EQ(result, true);

    result = false;
    sysSession->pipTemplateInfo_.pipTemplateType = static_cast<uint32_t>(PiPTemplateType::VIDEO_LIVE);
    sysSession->UpdateCameraWindowStatus(true);
}

/**
 * @tc.name: Show01
 * @tc.desc: test function : Show
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, Show01, TestSize.Level1)
{
    // for CheckPermissionWithPropertyAnimation;
    auto windowProperty = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(windowProperty, nullptr);
    windowProperty->animationFlag_ = static_cast<uint32_t>(WindowAnimation::CUSTOM);

    // for TOAST or FLOAT permission
    systemSession_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    systemSession_->Show(windowProperty);
    usleep(WAIT_ASYNC_US);
    ASSERT_EQ(systemSession_->property_->animationFlag_, static_cast<uint32_t>(WindowAnimation::CUSTOM));
}

/**
 * @tc.name: Hide01
 * @tc.desc: test function : Hide
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, Hide01, TestSize.Level0)
{
    auto sysSession = GetSystemSession("Hide01");
    ASSERT_NE(sysSession, nullptr);

    // for CheckPermissionWithPropertyAnimation;
    auto windowProperty = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(windowProperty, nullptr);
    windowProperty->animationFlag_ = static_cast<uint32_t>(WindowAnimation::CUSTOM);

    sysSession->property_ = windowProperty;
    // for TOAST or FLOAT permission
    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_FLOAT);

    // for IsSessionValid
    sysSession->sessionInfo_.isSystem_ = false;
    sysSession->state_ = SessionState::STATE_CONNECT;
    sysSession->isActive_ = true;

    auto ret = sysSession->Hide();
    usleep(WAIT_ASYNC_US);
    ASSERT_EQ(ret, WSError::WS_OK);

    windowProperty->animationFlag_ = static_cast<uint32_t>(WindowAnimation::DEFAULT);
    ret = sysSession->Hide();
    usleep(WAIT_ASYNC_US);
    ASSERT_EQ(ret, WSError::WS_OK);

    ret = sysSession->Hide();
    sysSession->property_ = nullptr;
    usleep(WAIT_ASYNC_US);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: ProcessPointDownSession01
 * @tc.desc: test function : ProcessPointDownSession
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, ProcessPointDownSession01, TestSize.Level1)
{
    auto sysSession = GetSystemSession("ProcessPointDownSession01");
    ASSERT_NE(sysSession, nullptr);
    sysSession->persistentId_ = 1;
    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sysSession->state_ = SessionState::STATE_ACTIVE;

    auto parentSesssion = GetSceneSession("parentSession");
    ASSERT_NE(parentSesssion, nullptr);
    sysSession->parentSession_ = parentSesssion;

    parentSesssion->dialogVec_.push_back(sysSession);
    auto ret = sysSession->ProcessPointDownSession(0, 0);
    ASSERT_EQ(ret, WSError::WS_OK);

    // for IsTopDialog true
    auto topDialog = GetSystemSession("TopDialogSession");
    ASSERT_NE(topDialog, nullptr);
    topDialog->persistentId_ = 2;
    topDialog->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    topDialog->state_ = SessionState::STATE_ACTIVE;
    parentSesssion->dialogVec_.push_back(topDialog);

    ret = sysSession->ProcessPointDownSession(0, 0);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: ProcessPointDownSession02
 * @tc.desc: test function : ProcessPointDownSession
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, ProcessPointDownSession02, TestSize.Level1)
{
    auto sysSession = GetSystemSession("ProcessPointDownSession02");
    ASSERT_NE(sysSession, nullptr);
    sysSession->persistentId_ = 1;
    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sysSession->state_ = SessionState::STATE_ACTIVE;

    sysSession->property_->raiseEnabled_ = false;
    auto ret = sysSession->ProcessPointDownSession(0, 0);
    ASSERT_EQ(ret, WSError::WS_OK);

    sysSession->property_->raiseEnabled_ = true;
    ret = sysSession->ProcessPointDownSession(0, 0);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: TransferKeyEvent04
 * @tc.desc: test function : TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, TransferKeyEvent04, TestSize.Level1)
{
    auto sysSession = GetSystemSession("TransferKeyEvent04");
    ASSERT_NE(sysSession, nullptr);
    sysSession->persistentId_ = 1;
    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sysSession->state_ = SessionState::STATE_ACTIVE;

    std::shared_ptr<MMI::KeyEvent> keyEvent;

    auto ret = sysSession->TransferKeyEvent(nullptr);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);

    ret = sysSession->TransferKeyEvent(keyEvent);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: TransferKeyEvent05
 * @tc.desc: test function : TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, TransferKeyEvent05, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();

    auto sysSession = GetSystemSession("TransferKeyEvent05");
    ASSERT_NE(sysSession, nullptr);
    sysSession->persistentId_ = 1;
    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sysSession->state_ = SessionState::STATE_ACTIVE;

    auto ret = sysSession->TransferKeyEvent(keyEvent);
    ASSERT_EQ(ret, WSError::WS_DO_NOTHING);

    auto parentSesssion = GetSceneSession("parentSession");
    ASSERT_NE(parentSesssion, nullptr);
    sysSession->parentSession_ = parentSesssion;

    // for CheckKeyEventDispatch return true;
    parentSesssion->state_ = SessionState::STATE_FOREGROUND;
    ret = sysSession->TransferKeyEvent(keyEvent);
    ASSERT_EQ(ret, WSError::WS_DO_NOTHING);

    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_BACK);
    ret = sysSession->TransferKeyEvent(keyEvent);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_PERMISSION);

    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_HOME);
    parentSesssion->dialogVec_.push_back(sysSession);
    ret = sysSession->TransferKeyEvent(keyEvent);
    ASSERT_EQ(ret, WSError::WS_DO_NOTHING);

    auto topDialog = GetSystemSession("TopDialogSession");
    ASSERT_NE(topDialog, nullptr);
    topDialog->persistentId_ = 2;
    topDialog->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    topDialog->state_ = SessionState::STATE_ACTIVE;
    parentSesssion->dialogVec_.push_back(topDialog);
    ret = sysSession->TransferKeyEvent(keyEvent);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: ProcessBackEvent02
 * @tc.desc: test function : ProcessBackEvent
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, ProcessBackEvent02, TestSize.Level1)
{
    auto sysSession = GetSystemSession("ProcessBackEvent02");
    ASSERT_NE(sysSession, nullptr);
    sysSession->persistentId_ = 1;
    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sysSession->state_ = SessionState::STATE_ACTIVE;

    sysSession->dialogSessionBackGestureEnabled_ = false;
    auto ret = sysSession->ProcessBackEvent();
    ASSERT_EQ(ret, WSError::WS_OK);

    sysSession->dialogSessionBackGestureEnabled_ = true;
    ret = sysSession->ProcessBackEvent();
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);

    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = sysSession->ProcessBackEvent();
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: CheckKeyEventDispatch03
 * @tc.desc: test function : CheckKeyEventDispatch
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, CheckKeyEventDispatch03, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    auto sysSession = GetSystemSession("CheckKeyEventDispatch03");
    ASSERT_NE(sysSession, nullptr);
    sysSession->persistentId_ = 1;
    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sysSession->state_ = SessionState::STATE_ACTIVE;
    sysSession->isRSVisible_ = false;

    auto ret = sysSession->CheckKeyEventDispatch(keyEvent);
    ASSERT_EQ(ret, false);

    sysSession->isRSVisible_ = true;
    WSRect curRect = sysSession->GetSessionRect();
    sysSession->GetLayoutController()->SetSessionRect({ curRect.posX_, curRect.posY_, 0, curRect.height_ });
    ret = sysSession->CheckKeyEventDispatch(keyEvent);
    ASSERT_EQ(ret, false);

    sysSession->isRSVisible_ = true;
    curRect = sysSession->GetSessionRect();
    sysSession->GetLayoutController()->SetSessionRect({ curRect.posX_, curRect.posY_, 1, 0 });
    ret = sysSession->CheckKeyEventDispatch(keyEvent);
    ASSERT_EQ(ret, false);

    sysSession->isRSVisible_ = true;
    curRect = sysSession->GetSessionRect();
    sysSession->GetLayoutController()->SetSessionRect({ curRect.posX_, curRect.posY_, 1, 1 });
    ret = sysSession->CheckKeyEventDispatch(keyEvent);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: CheckKeyEventDispatch04
 * @tc.desc: test function : CheckKeyEventDispatch
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, CheckKeyEventDispatch04, TestSize.Level1)
{
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    auto sysSession = GetSystemSession("CheckKeyEventDispatch04");
    ASSERT_NE(sysSession, nullptr);
    sysSession->persistentId_ = 1;
    sysSession->isRSVisible_ = true;
    WSRect curRect = sysSession->GetSessionRect();
    sysSession->GetLayoutController()->SetSessionRect({ curRect.posX_, curRect.posY_, 1, 1 });
    sysSession->state_ = SessionState::STATE_DISCONNECT;

    auto parentSesssion = GetSceneSession("parentSession");
    ASSERT_NE(parentSesssion, nullptr);
    sysSession->parentSession_ = parentSesssion;

    parentSesssion->state_ = SessionState::STATE_FOREGROUND;
    auto ret = sysSession->CheckKeyEventDispatch(keyEvent);
    ASSERT_EQ(ret, false);

    parentSesssion->state_ = SessionState::STATE_ACTIVE;
    ret = sysSession->CheckKeyEventDispatch(keyEvent);
    ASSERT_EQ(ret, false);

    parentSesssion->state_ = SessionState::STATE_DISCONNECT;
    sysSession->state_ = SessionState::STATE_FOREGROUND;
    ret = sysSession->CheckKeyEventDispatch(keyEvent);
    ASSERT_EQ(ret, false);

    sysSession->state_ = SessionState::STATE_ACTIVE;
    ret = sysSession->CheckKeyEventDispatch(keyEvent);
    ASSERT_EQ(ret, false);

    parentSesssion->state_ = SessionState::STATE_DISCONNECT;
    sysSession->state_ = SessionState::STATE_DISCONNECT;
    ret = sysSession->CheckKeyEventDispatch(keyEvent);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: NotifyClientToUpdateRect02
 * @tc.desc: test function : NotifyClientToUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, NotifyClientToUpdateRect02, TestSize.Level1)
{
    auto sysSession = GetSystemSession("NotifyClientToUpdateRect02");
    ASSERT_NE(sysSession, nullptr);
    sysSession->persistentId_ = 1;
    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sysSession->state_ = SessionState::STATE_ACTIVE;

    // for NotifyClientToUpdateRectTask
    sysSession->isKeyboardPanelEnabled_ = true;

    sysSession->dirtyFlags_ = 0;
    sysSession->Session::UpdateSizeChangeReason(SizeChangeReason::MAXIMIZE);
    sysSession->NotifyClientToUpdateRect("SystemSessionTest", nullptr);
    usleep(WAIT_ASYNC_US);
    ASSERT_EQ(sysSession->GetSizeChangeReason(), SizeChangeReason::UNDEFINED);

    sysSession->Session::UpdateSizeChangeReason(SizeChangeReason::DRAG);
    sysSession->NotifyClientToUpdateRect("SystemSessionTest", nullptr);
    usleep(WAIT_ASYNC_US);
    ASSERT_EQ(sysSession->GetSizeChangeReason(), SizeChangeReason::DRAG);
}

/**
 * @tc.name: NotifyClientToUpdateRect03
 * @tc.desc: test function : NotifyClientToUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, NotifyClientToUpdateRect03, TestSize.Level1)
{
    auto sysSession = GetSystemSession("NotifyClientToUpdateRect03");
    ASSERT_NE(sysSession, nullptr);
    sysSession->persistentId_ = 1;
    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sysSession->state_ = SessionState::STATE_ACTIVE;

    sysSession->dirtyFlags_ = 0;
    sysSession->isKeyboardPanelEnabled_ = true;
    auto res = sysSession->Session::UpdateSizeChangeReason(SizeChangeReason::MAXIMIZE);
    EXPECT_EQ(res, WSError::WS_OK);

    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(specificCallback, nullptr);
    sysSession->specificCallback_ = specificCallback;
    sysSession->specificCallback_->onUpdateAvoidArea_ = nullptr;
    sysSession->GetLayoutController()->SetSessionRect({ 0, 0, 800, 800 });
    res = sysSession->NotifyClientToUpdateRect("SystemSessionTest", nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(res, WSError::WS_OK);

    sysSession->dirtyFlags_ = 0;
    sysSession->SetScbCoreEnabled(true);
    sysSession->Session::UpdateSizeChangeReason(SizeChangeReason::MAXIMIZE);
    sysSession->specificCallback_->onUpdateAvoidArea_ = [](const int32_t& persistentId) {};
    res = sysSession->NotifyClientToUpdateRect("SystemSessionTest", nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(sysSession->dirtyFlags_, static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA));
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: IsVisibleForeground01
 * @tc.desc: test function : IsVisibleForeground
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, IsVisibleForeground01, TestSize.Level1)
{
    auto sysSession = GetSystemSession("IsVisibleForeground01");
    ASSERT_NE(sysSession, nullptr);
    sysSession->persistentId_ = 1;
    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sysSession->state_ = SessionState::STATE_ACTIVE;
    sysSession->isVisible_ = false;

    auto ret = sysSession->IsVisibleForeground();
    ASSERT_EQ(ret, false);

    sysSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ret = sysSession->IsVisibleForeground();
    ASSERT_EQ(ret, false);

    auto parentSesssion = GetSceneSession("parentSession");
    ASSERT_NE(parentSesssion, nullptr);
    sysSession->parentSession_ = parentSesssion;
    parentSesssion->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ret = sysSession->IsVisibleForeground();
    ASSERT_EQ(ret, false);

    parentSesssion->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = sysSession->IsVisibleForeground();
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: UpdatePiPWindowStateChanged
 * @tc.desc: test function : UpdatePiPWindowStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, UpdatePiPWindowStateChanged, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "UpdatePiPWindowStateChanged";
    sessionInfo.moduleName_ = "UpdatePiPWindowStateChanged";
    sessionInfo.bundleName_ = "UpdatePiPWindowStateChanged";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, callback);
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(sessionInfo, callback);
    EXPECT_NE(nullptr, systemSession);
    PiPStateChangeCallback callbackFun = [](const std::string& bundleName, bool isForeground) { return; };
    callback->onPiPStateChange_ = callbackFun;
    EXPECT_EQ(WindowType::APP_MAIN_WINDOW_BASE, systemSession->GetWindowType());
    systemSession->UpdatePiPWindowStateChanged(true);

    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_PIP);
    sptr<SystemSession> system = sptr<SystemSession>::MakeSptr(sessionInfo, callback);
    EXPECT_NE(nullptr, system);
    EXPECT_EQ(WindowType::WINDOW_TYPE_PIP, system->GetWindowType());
    system->UpdatePiPWindowStateChanged(true);
}

/**
 * @tc.name: GetSubWindowZLevel
 * @tc.desc: GetSubWindowZLevel
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, GetSubWindowZLevel, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetSubWindowZLevel";
    info.bundleName_ = "GetSubWindowZLevel";
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, nullptr);
    systemSession->property_->zLevel_ = 1;
    EXPECT_EQ(1, systemSession->GetSubWindowZLevel());
}

/**
 * @tc.name: IsVisibleNotBackground
 * @tc.desc: IsVisibleNotBackground
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, IsVisibleNotBackground, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsVisibleNotBackground";
    info.bundleName_ = "IsVisibleNotBackground";
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, nullptr);
    EXPECT_EQ(false, systemSession->IsVisibleNotBackground());
    systemSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sptr<Session> parentSession = sptr<Session>::MakeSptr(info);
    parentSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    systemSession->SetParentSession(parentSession);
    EXPECT_EQ(false, systemSession->IsVisibleNotBackground());
    systemSession->SetSessionState(SessionState::STATE_FOREGROUND);
    systemSession->isVisible_ = true;
    parentSession->SetSessionState(SessionState::STATE_FOREGROUND);
    parentSession->isVisible_ = true;
    EXPECT_EQ(true, systemSession->IsVisibleNotBackground());
}

/**
 * @tc.name: SetAndGetFbTemplateInfo
 * @tc.desc: SetAndGetFbTemplateInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, SetAndGetFbTemplateInfo, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "Background01";

    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, nullptr);
    ASSERT_NE(systemSession, nullptr);
    systemSession->isActive_ = true;
    FloatingBallTemplateInfo fbTemplateInfo;
    fbTemplateInfo.template_ = 1;
    systemSession->SetFbTemplateInfo(fbTemplateInfo);
    EXPECT_EQ(systemSession->GetFbTemplateInfo().template_, fbTemplateInfo.template_);
}

/**
 * @tc.name: GetFbWindowId
 * @tc.desc: GetFbWindowId Test
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, GetFbWindowId, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAndGetFbWindowId";
    info.bundleName_ = "SetAndGetFbWindowId";

    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, nullptr);
    ASSERT_NE(systemSession, nullptr);

    EXPECT_EQ(systemSession->GetFbWindowId(), 0);

    uint32_t testWindowId = 100;
    auto func = [testWindowId](uint32_t& windowId) {
        windowId = testWindowId;
        return WMError::WM_OK;
    };
    systemSession->RegisterGetFbPanelWindowIdFunc(func);
    EXPECT_EQ(systemSession->GetFbWindowId(), testWindowId);
}

/**
 * @tc.name: UpdateFloatingBall
 * @tc.desc: UpdateFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, UpdateFloatingBall, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, specificCallback);

    FloatingBallTemplateInfo fbTemplateInfo;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    systemSession->SetSessionProperty(property);
    EXPECT_EQ(systemSession->UpdateFloatingBall(fbTemplateInfo), WMError::WM_DO_NOTHING);

    property->SetWindowType(WindowType::WINDOW_TYPE_FB);
    systemSession->SetSessionProperty(property);
    EXPECT_EQ(systemSession->UpdateFloatingBall(fbTemplateInfo), WMError::WM_OK);

    LOCK_GUARD_EXPR(SCENE_GUARD, systemSession->SetCallingPid(IPCSkeleton::GetCallingPid()));
    EXPECT_EQ(systemSession->UpdateFloatingBall(fbTemplateInfo), WMError::WM_OK);

    FloatingBallTemplateInfo fbTmpInfo {static_cast<uint32_t>(FloatingBallTemplate::STATIC), "", "", "", nullptr};
    systemSession->SetFbTemplateInfo(fbTmpInfo);
    EXPECT_EQ(systemSession->UpdateFloatingBall(fbTemplateInfo), WMError::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED);

    fbTmpInfo.template_ = static_cast<uint32_t>(FloatingBallTemplate::NORMAL);
    systemSession->SetFbTemplateInfo(fbTmpInfo);
    EXPECT_EQ(systemSession->UpdateFloatingBall(fbTemplateInfo), WMError::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED);

    fbTemplateInfo.template_ = static_cast<uint32_t>(FloatingBallTemplate::NORMAL);
    EXPECT_EQ(systemSession->UpdateFloatingBall(fbTemplateInfo), WMError::WM_OK);
}

/**
 * @tc.name: StopFloatingBall
 * @tc.desc: StopFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, StopFloatingBall, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, specificCallback);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    systemSession->SetSessionProperty(property);
    EXPECT_EQ(systemSession->StopFloatingBall(), WSError::WS_DO_NOTHING);

    property->SetWindowType(WindowType::WINDOW_TYPE_FB);
    systemSession->SetSessionProperty(property);
    EXPECT_EQ(systemSession->StopFloatingBall(), WSError::WS_OK);

    LOCK_GUARD_EXPR(SCENE_GUARD, systemSession->SetCallingPid(IPCSkeleton::GetCallingPid()));
    EXPECT_EQ(systemSession->StopFloatingBall(), WSError::WS_OK);
}

/**
 * @tc.name: RestoreFbMainWindow
 * @tc.desc: RestoreFbMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, RestoreFbMainWindow, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, specificCallback);

    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    systemSession->SetSessionProperty(property);
    EXPECT_EQ(systemSession->RestoreFbMainWindow(want), WMError::WM_DO_NOTHING);

    property->SetWindowType(WindowType::WINDOW_TYPE_FB);
    systemSession->SetSessionProperty(property);

    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    EXPECT_EQ(systemSession->RestoreFbMainWindow(want), WMError::WM_ERROR_INVALID_PERMISSION);

    MockAccesstokenKit::MockAccessTokenKitRet(0);
    EXPECT_EQ(systemSession->RestoreFbMainWindow(want), WMError::WM_ERROR_INVALID_CALLING);

    LOCK_GUARD_EXPR(SCENE_GUARD, systemSession->SetCallingPid(IPCSkeleton::GetCallingPid()));
    std::string bundle = "testBundle";
    want->SetBundle(bundle);
    EXPECT_EQ(systemSession->RestoreFbMainWindow(want), WMError::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED);

    systemSession->EditSessionInfo().bundleName_ = bundle;
    EXPECT_EQ(systemSession->RestoreFbMainWindow(want), WMError::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED);
    
    ++systemSession->fbClickCnt_;
    EXPECT_EQ(systemSession->RestoreFbMainWindow(want), WMError::WM_OK);
}

/**
 * @tc.name: GetFloatingBallWindowId
 * @tc.desc: GetFloatingBallWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, GetFloatingBallWindowId, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, specificCallback);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);

    uint32_t windowId = 0;
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    systemSession->SetSessionProperty(property);
    EXPECT_EQ(systemSession->GetFloatingBallWindowId(windowId), WMError::WM_DO_NOTHING);

    property->SetWindowType(WindowType::WINDOW_TYPE_FB);
    systemSession->SetSessionProperty(property);
    EXPECT_EQ(systemSession->GetFloatingBallWindowId(windowId), WMError::WM_ERROR_INVALID_CALLING);

    LOCK_GUARD_EXPR(SCENE_GUARD, systemSession->SetCallingPid(IPCSkeleton::GetCallingPid()));
    EXPECT_EQ(systemSession->GetFloatingBallWindowId(windowId), WMError::WM_OK);
}

/**
 * @tc.name: NotifyUpdateFloatingBall
 * @tc.desc: NotifyUpdateFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, NotifyUpdateFloatingBall, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, specificCallback);

    FloatingBallTemplateInfo fbTemplateInfo {1, "fb", "fb_content", "red", nullptr};
    FloatingBallTemplateInfo newFbTemplateInfo {2, "fb_new", "fb_content_new", "red", nullptr};
    systemSession->SetFloatingBallUpdateCallback(nullptr);
    systemSession->NotifyUpdateFloatingBall(newFbTemplateInfo);
    EXPECT_NE(fbTemplateInfo.template_, newFbTemplateInfo.template_);

    auto updateFbFuncCb = [&fbTemplateInfo] (const FloatingBallTemplateInfo& newFbTemplateInfo) {
        fbTemplateInfo.template_ = newFbTemplateInfo.template_;
    };
    systemSession->SetFloatingBallUpdateCallback(updateFbFuncCb);
    systemSession->NotifyUpdateFloatingBall(newFbTemplateInfo);
    EXPECT_EQ(fbTemplateInfo.template_, newFbTemplateInfo.template_);
}

/**
 * @tc.name: NotifyStopFloatingBall
 * @tc.desc: NotifyStopFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, NotifyStopFloatingBall, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, specificCallback);

    uint32_t stopFuncCallTimes = 0;
    auto fbStopFuncCb = [&stopFuncCallTimes] () {
        stopFuncCallTimes++;
    };

    systemSession->SetFloatingBallStopCallback(nullptr);
    systemSession->NotifyStopFloatingBall();
 
    systemSession->SetFloatingBallStopCallback(fbStopFuncCb);
    systemSession->NotifyStopFloatingBall();
    EXPECT_EQ(stopFuncCallTimes, 2);
}

/**
 * @tc.name: NotifyRestoreFloatingBallMainWindow
 * @tc.desc: NotifyRestoreFloatingBallMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, NotifyRestoreFloatingBallMainWindow, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, specificCallback);

    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetParam("NotifyRestoreFloatingBallMainWindow", 100);
    int res = -1;
    systemSession->SetFloatingBallRestoreMainWindowCallback(nullptr);
    systemSession->NotifyRestoreFloatingBallMainWindow(want);

    auto fbStopFuncCb = [&res] (const std::shared_ptr<AAFwk::Want>& want) {
        res = want->GetIntParam("NotifyRestoreFloatingBallMainWindow", -1);
    };
    systemSession->SetFloatingBallRestoreMainWindowCallback(fbStopFuncCb);
    systemSession->NotifyRestoreFloatingBallMainWindow(want);
    EXPECT_EQ(res, 100);
}

/**
 * @tc.name: SendFbActionEvent
 * @tc.desc: SendFbActionEvent
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, SendFbActionEvent, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, nullptr);

    systemSession_->sessionStage_ = nullptr;
    EXPECT_EQ(systemSession->SendFbActionEvent(""), WSError::WS_ERROR_NULLPTR);

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    systemSession_->sessionStage_ = mockSessionStage;
    auto ret = systemSession_->SendFbActionEvent("click");
    ASSERT_EQ(WSError::WS_OK, ret);

    ret = systemSession_->SendFbActionEvent("on");
    ASSERT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: NotifyRestoreFloatMainWindowCallback
 * @tc.desc: NotifyRestoreFloatMainWindowCallback
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, NotifyRestoreFloatMainWindowCallback, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, specificCallback);

    std::shared_ptr<AAFwk::WantParams> wantParams = std::make_shared<AAFwk::WantParams>();
    wantParams->SetParam("NotifyRestoreFloatMainWindowCallback", OHOS::AAFwk::String::Box("100"));
    
    systemSession->SetRestoreFloatMainWindowCallback(nullptr);
    EXPECT_EQ(systemSession->restoreFloatMainWindowFunc_, nullptr);
    systemSession->NotifyRestoreFloatMainWindow(wantParams);

    string res;
    auto func = [&res](const std::shared_ptr<AAFwk::WantParams>& wantParameters) {
        res = wantParameters->ToString();
    };
    systemSession->SetRestoreFloatMainWindowCallback(func);
    systemSession->NotifyRestoreFloatMainWindow(wantParams);
    EXPECT_EQ(res, "{\"NotifyRestoreFloatMainWindowCallback\":\"100\"}");
}

/**
 * @tc.name: RestoreFloatMainWindow
 * @tc.desc: RestoreFloatMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, RestoreFloatMainWindow, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, specificCallback);

    std::shared_ptr<AAFwk::WantParams> wantParams = std::make_shared<AAFwk::WantParams>();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_MEDIA);
    systemSession->SetSessionProperty(property);
    EXPECT_EQ(systemSession->RestoreFloatMainWindow(wantParams), WMError::WM_ERROR_INVALID_CALLING);
    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    systemSession->SetSessionProperty(property);

    LOCK_GUARD_EXPR(SCENE_GUARD, systemSession->SetCallingPid(IPCSkeleton::GetCallingPid() + 1));
    EXPECT_EQ(systemSession->RestoreFloatMainWindow(wantParams), WMError::WM_ERROR_INVALID_CALLING);
    LOCK_GUARD_EXPR(SCENE_GUARD, systemSession->SetCallingPid(IPCSkeleton::GetCallingPid()));

    EXPECT_EQ(systemSession->RestoreFloatMainWindow(wantParams), WMError::WM_ERROR_INVALID_CALLING);
    systemSession->state_.store(SessionState::STATE_FOREGROUND);
    EXPECT_EQ(systemSession->RestoreFloatMainWindow(wantParams), WMError::WM_ERROR_SYSTEM_ABNORMALLY);
    systemSession->state_.store(SessionState::STATE_ACTIVE);
    EXPECT_EQ(systemSession->RestoreFloatMainWindow(wantParams), WMError::WM_ERROR_SYSTEM_ABNORMALLY);
    systemSession->RegisterGetSCBEnterRecentFunc([]() {
        return true;
    });
    EXPECT_EQ(systemSession->RestoreFloatMainWindow(wantParams), WMError::WM_ERROR_START_ABILITY_FAILED);
    systemSession->RegisterGetSCBEnterRecentFunc([]() {
        return false;
    });

    systemSession->floatWindowDownEventCnt_ = 0;
    EXPECT_EQ(systemSession->RestoreFloatMainWindow(wantParams), WMError::WM_ERROR_INVALID_CALLING);
    systemSession->floatWindowDownEventCnt_ = 2;
    EXPECT_EQ(systemSession->RestoreFloatMainWindow(wantParams), WMError::WM_OK);
    EXPECT_EQ(systemSession->floatWindowDownEventCnt_, 0);
}

/**
 * @tc.name: RestoreFloatMainWindowGetIsAtRecent
 * @tc.desc: RestoreFloatMainWindowGetIsAtRecent Test
 * @tc.type: FUNC
 */
HWTEST_F(SystemSessionTest, RestoreFloatMainWindowGetIsAtRecent, TestSize.Level1)
{
    SessionInfo info;

    sptr<SystemSession> systemSession = sptr<SystemSession>::MakeSptr(info, nullptr);
    ASSERT_NE(systemSession, nullptr);

    EXPECT_EQ(systemSession->getSCBEnterRecentFunc_, nullptr);

    auto func = []() {
        return true;
    };
    systemSession->RegisterGetSCBEnterRecentFunc(func);
    EXPECT_EQ(systemSession->getSCBEnterRecentFunc_(), true);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
