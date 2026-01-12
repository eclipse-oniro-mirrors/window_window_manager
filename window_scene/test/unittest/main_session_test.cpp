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
#include <pointer_event.h>
#include "session/host/include/main_session.h"

#include "common/include/session_permission.h"
#include "key_event.h"
#include "mock/mock_session_stage.h"
#include "session/host/include/session.h"
#include "session/host/include/system_session.h"
#include <ui/rs_surface_node.h>
#include "window_event_channel_base.h"
#include "window_helper.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class MainSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    SessionInfo info;
    sptr<MainSession::SpecificSessionCallback> specificCallback = nullptr;
    sptr<MainSession> mainSession_;

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    SystemSessionConfig systemConfig_;
};

void MainSessionTest::SetUpTestCase() {}

void MainSessionTest::TearDownTestCase() {}

void MainSessionTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testMainSession1";
    info.moduleName_ = "testMainSession2";
    info.bundleName_ = "testMainSession3";
    mainSession_ = sptr<MainSession>::MakeSptr(info, specificCallback);
    ASSERT_NE(nullptr, mainSession_);
}

void MainSessionTest::TearDown()
{
    mainSession_ = nullptr;
}

RSSurfaceNode::SharedPtr MainSessionTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    return surfaceNode;
}

namespace {

/**
 * @tc.name: MainSession01
 * @tc.desc: check func MainSession
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, MainSession01, TestSize.Level1)
{
    sptr<MainSession> pMainSession = nullptr;
    sptr<MainSession::SpecificSessionCallback> pSpecificCallback = nullptr;

    SessionInfo info;
    info.persistentId_ = -1;
    info.abilityName_ = "";
    info.moduleName_ = "";
    info.bundleName_ = "";
    pMainSession = sptr<MainSession>::MakeSptr(info, pSpecificCallback);
    ASSERT_NE(nullptr, pMainSession);

    info.persistentId_ = 0;
    pMainSession = sptr<MainSession>::MakeSptr(info, pSpecificCallback);
    ASSERT_NE(nullptr, pMainSession);

    info.persistentId_ = -1;
    info.abilityName_ = "MainSession01";
    info.moduleName_ = "MainSession02";
    info.bundleName_ = "MainSession03";
    pSpecificCallback = new (std::nothrow) MainSession::SpecificSessionCallback;
    pMainSession = sptr<MainSession>::MakeSptr(info, pSpecificCallback);
    ASSERT_NE(nullptr, pMainSession);

    info.persistentId_ = 0;
    pMainSession = sptr<MainSession>::MakeSptr(info, pSpecificCallback);
    ASSERT_NE(nullptr, pMainSession);
}

/**
 * @tc.name: MainSession02
 * @tc.desc: check func MainSession
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, MainSession02, TestSize.Level1)
{
    sptr<MainSession> pMainSession = nullptr;
    sptr<MainSession::SpecificSessionCallback> pSpecificCallback = nullptr;

    SessionInfo info;
    info.persistentId_ = 1999;
    info.abilityName_ = "test";
    info.moduleName_ = "test";
    info.bundleName_ = "test";
    pMainSession = sptr<MainSession>::MakeSptr(info, pSpecificCallback);
    ASSERT_NE(nullptr, pMainSession);

    ASSERT_NE(nullptr, pMainSession->scenePersistence_);
    auto hasSnapshot = pMainSession->scenePersistence_->HasSnapshot();
    EXPECT_EQ(hasSnapshot, false);
}

/**
 * @tc.name: TransferKeyEvent01
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, TransferKeyEvent01, TestSize.Level1)
{
    mainSession_->state_ = SessionState::STATE_END;

    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, mainSession_->TransferKeyEvent(nullptr));
}

/**
 * @tc.name: TransferKeyEvent02
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, TransferKeyEvent02, TestSize.Level1)
{
    mainSession_->state_ = SessionState::STATE_CONNECT;
    std::shared_ptr<MMI::KeyEvent> keyEvent = nullptr;
    mainSession_->ClearDialogVector();
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, mainSession_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: TransferKeyEvent03
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, TransferKeyEvent03, TestSize.Level1)
{
    mainSession_->state_ = SessionState::STATE_CONNECT;
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    SessionInfo info;
    info.abilityName_ = "testDialogSession1";
    info.moduleName_ = "testDialogSession2";
    info.bundleName_ = "testDialogSession3";
    sptr<Session> dialogSession = sptr<SystemSession>::MakeSptr(info, nullptr);
    dialogSession->SetSessionState(SessionState::STATE_ACTIVE);
    mainSession_->BindDialogToParentSession(dialogSession);

    ASSERT_EQ(WSError::WS_ERROR_INVALID_PERMISSION, mainSession_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: TransferKeyEvent04
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, TransferKeyEvent04, TestSize.Level1)
{
    mainSession_->state_ = SessionState::STATE_CONNECT;
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    mainSession_->windowEventChannel_ = nullptr;
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, mainSession_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: ProcessPointDownSession01
 * @tc.desc: check func ProcessPointDownSession
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, ProcessPointDownSession01, TestSize.Level1)
{
    EXPECT_EQ(WSError::WS_OK, mainSession_->ProcessPointDownSession(100, 200));
    mainSession_->ClearDialogVector();
    EXPECT_EQ(WSError::WS_OK, mainSession_->ProcessPointDownSession(10, 20));
}

/**
 * @tc.name: ProcessPointDownSession02
 * @tc.desc: check func ProcessPointDownSession
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, ProcessPointDownSession02, TestSize.Level1)
{
    mainSession_->BindDialogToParentSession(mainSession_);
    EXPECT_EQ(WSError::WS_OK, mainSession_->ProcessPointDownSession(10, 20));
}

/**
 * @tc.name: ProcessPointDownSession03
 * @tc.desc: check func ProcessPointDownSession
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, ProcessPointDownSession03, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    SessionInfo info;
    info.abilityName_ = "dialogAbilityName";
    info.moduleName_ = "dialogModuleName";
    info.bundleName_ = "dialogBundleName";
    sptr<Session> dialogSession = sptr<Session>::MakeSptr(info);
    dialogSession->state_ = SessionState::STATE_INACTIVE;
    mainSession_->dialogVec_.push_back(dialogSession);
    EXPECT_EQ(WSError::WS_OK, mainSession_->ProcessPointDownSession(0, 0));

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    property->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    mainSession_->SetSessionProperty(property);
    EXPECT_EQ(WSError::WS_OK, mainSession_->ProcessPointDownSession(0, 0));
}

/**
 * @tc.name: SetTopmost01
 * @tc.desc: check func SetTopmost
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, SetTopmost01, TestSize.Level1)
{
    EXPECT_EQ(WSError::WS_OK, mainSession_->SetTopmost(true));
    EXPECT_EQ(WSError::WS_OK, mainSession_->SetTopmost(false));
}

/**
 * @tc.name: SetTopmost02
 * @tc.desc: check func SetTopmost
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, SetTopmost02, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    mainSession_->SetSessionProperty(property);
    ASSERT_TRUE(mainSession_->GetSessionProperty() != nullptr);
    mainSession_->RegisterSessionTopmostChangeCallback([](bool topmost) {});
    EXPECT_EQ(WSError::WS_OK, mainSession_->SetTopmost(true));
}

/**
 * @tc.name: SetMainWindowTopmost
 * @tc.desc: check func SetMainWindowTopmost
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, SetMainWindowTopmost, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    mainSession_->SetSessionProperty(property);
    ASSERT_TRUE(mainSession_->GetSessionProperty() != nullptr);
    mainSession_->SetMainWindowTopmostChangeCallback([](bool topmost) {});
    EXPECT_EQ(WSError::WS_OK, mainSession_->SetMainWindowTopmost(true));
}

/**
 * @tc.name: UpdatePointerArea
 * @tc.desc: check func UpdatePointerArea
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, UpdatePointerArea, TestSize.Level1)
{
    WSRect Rect = { 0, 0, 50, 50 };
    mainSession_->UpdateWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    mainSession_->UpdatePointerArea(Rect);
    mainSession_->UpdateWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    mainSession_->UpdatePointerArea(Rect);
    ASSERT_EQ(Rect, mainSession_->preRect_);
}

/**
 * @tc.name: CheckPointerEventDispatch
 * @tc.desc: check func CheckPointerEventDispatch
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, CheckPointerEventDispatch, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();

    mainSession_->SetSessionState(SessionState::STATE_FOREGROUND);
    bool res = mainSession_->CheckPointerEventDispatch(pointerEvent);
    EXPECT_EQ(res, true);

    mainSession_->SetSessionState(SessionState::STATE_ACTIVE);
    res = mainSession_->CheckPointerEventDispatch(pointerEvent);
    EXPECT_EQ(res, true);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_LEAVE_WINDOW);
    mainSession_->SetSessionState(SessionState::STATE_DISCONNECT);
    res = mainSession_->CheckPointerEventDispatch(pointerEvent);
    EXPECT_EQ(res, true);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_PULL_DOWN);
    mainSession_->SetSessionState(SessionState::STATE_DISCONNECT);
    res = mainSession_->CheckPointerEventDispatch(pointerEvent);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: RectCheck03
 * @tc.desc: check func RectCheck
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, RectCheck03, TestSize.Level1)
{
    ASSERT_NE(mainSession_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testMainSessionRectCheck";
    info.moduleName_ = "testMainSessionRectCheck";
    info.bundleName_ = "testMainSessionRectCheck";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    EXPECT_NE(nullptr, session);
    mainSession_->parentSession_ = session;
    uint32_t curWidth = 100;
    uint32_t curHeight = 200;
    mainSession_->RectCheck(curWidth, curHeight);

    curWidth = 300;
    curHeight = 200;
    mainSession_->RectCheck(curWidth, curHeight);

    curWidth = 1930;
    curHeight = 200;
    mainSession_->RectCheck(curWidth, curHeight);

    curWidth = 330;
    curHeight = 200;
    mainSession_->RectCheck(curWidth, curHeight);

    curWidth = 330;
    curHeight = 1930;
    mainSession_->RectCheck(curWidth, curHeight);
}

/**
 * @tc.name: NotifyClientToUpdateInteractive
 * @tc.desc: check func NotifyClientToUpdateInteractive
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, NotifyClientToUpdateInteractive, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyClientToUpdateInteractive";
    info.bundleName_ = "NotifyClientToUpdateInteractive";
    sptr<MainSession> testSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(testSession, nullptr);
    testSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(testSession->sessionStage_, nullptr);

    bool interactive = !(testSession->isClientInteractive_);
    testSession->SetSessionState(SessionState::STATE_ACTIVE);
    testSession->NotifyClientToUpdateInteractive(interactive);
    ASSERT_EQ(testSession->isClientInteractive_, interactive);
    interactive = !interactive;
    testSession->SetSessionState(SessionState::STATE_FOREGROUND);
    testSession->NotifyClientToUpdateInteractive(interactive);
    ASSERT_EQ(testSession->isClientInteractive_, interactive);
    interactive = !interactive;
    testSession->SetSessionState(SessionState::STATE_BACKGROUND);
    testSession->NotifyClientToUpdateInteractive(interactive);
    ASSERT_NE(testSession->isClientInteractive_, interactive);

    testSession->sessionStage_ = nullptr;
    testSession->NotifyClientToUpdateInteractive(interactive);
    ASSERT_NE(testSession->isClientInteractive_, interactive);
}

/**
 * @tc.name: IsExitSplitOnBackground01
 * @tc.desc: check func IsExitSplitOnBackground
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, IsExitSplitOnBackground01, TestSize.Level1)
{
    bool isExitSplitOnBackground = true;
    mainSession_->SetExitSplitOnBackground(isExitSplitOnBackground);
    bool ret = mainSession_->IsExitSplitOnBackground();
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: IsExitSplitOnBackground02
 * @tc.desc: check func IsExitSplitOnBackground
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, IsExitSplitOnBackground02, TestSize.Level1)
{
    bool isExitSplitOnBackground = false;
    mainSession_->SetExitSplitOnBackground(isExitSplitOnBackground);
    bool ret = mainSession_->IsExitSplitOnBackground();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: OnTitleAndDockHoverShowChange
 * @tc.desc: OnTitleAndDockHoverShowChange
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, OnTitleAndDockHoverShowChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnTitleAndDockHoverShowChange";
    info.bundleName_ = "OnTitleAndDockHoverShowChange";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetTitleAndDockHoverShowChangeCallback([](bool isTitleHoverShown, bool isDockHoverShown) { return; });
    EXPECT_NE(sceneSession->onTitleAndDockHoverShowChangeFunc_, nullptr);
    EXPECT_EQ(sceneSession->OnTitleAndDockHoverShowChange(true, true), WSError::WS_OK);
}

/**
 * @tc.name: OnTitleAndDockHoverShowChange02
 * @tc.desc: check func OnTitleAndDockHoverShowChange
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, OnTitleAndDockHoverShowChange02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnTitleAndDockHoverShowChange02";
    info.bundleName_ = "OnTitleAndDockHoverShowChange02";
    sptr<MainSession> testSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(testSession, nullptr);

    auto callbackFlag = 1;
    testSession->onTitleAndDockHoverShowChangeFunc_ = [&callbackFlag](bool isTitleHoverShown, bool isDockHoverShown) {
        callbackFlag += 1;
    };
    testSession->OnTitleAndDockHoverShowChange(true, true);
    EXPECT_EQ(callbackFlag, 2);
}

/**
 * @tc.name: OnRestoreMainWindow
 * @tc.desc: OnRestoreMainWindow function01
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, OnRestoreMainWindow, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnRestoreMainWindow";
    info.bundleName_ = "OnRestoreMainWindow";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    EXPECT_EQ(WSError::WS_OK, session->OnRestoreMainWindow());

    session->onRestoreMainWindowFunc_ = nullptr;
    EXPECT_EQ(WSError::WS_OK, session->OnRestoreMainWindow());

    NotifyRestoreMainWindowFunc func = [](bool isAppSupportPhoneInPc, int32_t callingPid, uint32_t callingToken) {
        return;
    };
    session->onRestoreMainWindowFunc_ = func;
    EXPECT_EQ(WSError::WS_OK, session->OnRestoreMainWindow());
}

/**
 * @tc.name: OnRestoreMainWindow02
 * @tc.desc: check func OnRestoreMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, OnRestoreMainWindow02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnRestoreMainWindow02";
    info.bundleName_ = "OnRestoreMainWindow02";
    sptr<MainSession> testSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(testSession, nullptr);

    testSession->onRestoreMainWindowFunc_ = nullptr;
    EXPECT_EQ(testSession->OnRestoreMainWindow(), WSError::WS_OK);

    auto callbackFlag = 1;
    testSession->onRestoreMainWindowFunc_ = [&callbackFlag](
        bool isAppSupportPhoneInPc, int32_t callingPid, uint32_t callingToken) {
        callbackFlag += 1;
    };
    testSession->OnRestoreMainWindow();
    EXPECT_EQ(callbackFlag, 2);
}

/**
 * @tc.name: OnRestoreMainWindow03
 * @tc.desc: check func OnRestoreMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, OnRestoreMainWindow03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnRestoreMainWindow03";
    info.bundleName_ = "OnRestoreMainWindow03";
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<MainSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    sceneSession->property_->SetIsAppSupportPhoneInPc(true);
    subSession->RegisterIsAppBoundSystemTrayCallback(
        [](int32_t callingPid, uint32_t callingToken, const std::string &instanceKey) { return false; });
    WSError result = sceneSession->OnRestoreMainWindow();
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_CALLING);
}

/**
 * @tc.name: OnSetWindowRectAutoSave
 * @tc.desc: OnSetWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, OnSetWindowRectAutoSave, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnSetWindowRectAutoSave";
    info.bundleName_ = "OnSetWindowRectAutoSave";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    session->onSetWindowRectAutoSaveFunc_ = nullptr;
    EXPECT_EQ(nullptr, session->onSetWindowRectAutoSaveFunc_);

    NotifySetWindowRectAutoSaveFunc func = [](bool enabled, bool isSaveBySpecifiedFlag) { return; };
    session->onSetWindowRectAutoSaveFunc_ = func;
    EXPECT_NE(nullptr, session->onSetWindowRectAutoSaveFunc_);
}

/**
 * @tc.name: OnSetWindowRectAutoSave02
 * @tc.desc: check func OnSetWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, OnSetWindowRectAutoSave02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnSetWindowRectAutoSave02";
    info.bundleName_ = "OnSetWindowRectAutoSave02";
    sptr<MainSession> testSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(testSession, nullptr);

    testSession->onSetWindowRectAutoSaveFunc_ = nullptr;
    EXPECT_EQ(testSession->OnSetWindowRectAutoSave(true, true), WSError::WS_OK);

    auto callbackFlag = 1;
    testSession->onSetWindowRectAutoSaveFunc_ = [&callbackFlag](bool enabled, bool isSaveBySpecifiedFlag) {
        callbackFlag += 1;
    };
    testSession->OnSetWindowRectAutoSave(true, true);
    EXPECT_EQ(callbackFlag, 2);
}

/**
 * @tc.name: NotifyMainModalTypeChange
 * @tc.desc: NotifyMainModalTypeChange function01
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, NotifyMainModalTypeChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyMainModalTypeChange";
    info.bundleName_ = "NotifyMainModalTypeChange";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->RegisterMainModalTypeChangeCallback([](bool isModal) { return; });
    ASSERT_NE(sceneSession->onMainModalTypeChange_, nullptr);
    EXPECT_EQ(WSError::WS_OK, sceneSession->NotifyMainModalTypeChange(true));
}

/**
 * @tc.name: NotifyMainModalTypeChange02
 * @tc.desc: check func NotifyMainModalTypeChange
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, NotifyMainModalTypeChange02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyMainModalTypeChange02";
    info.bundleName_ = "NotifyMainModalTypeChange02";
    sptr<MainSession> testSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(testSession, nullptr);

    testSession->onMainModalTypeChange_ = nullptr;
    EXPECT_EQ(testSession->NotifyMainModalTypeChange(true), WSError::WS_OK);

    auto callbackFlag = 1;
    testSession->onMainModalTypeChange_ = [&callbackFlag](bool isModal) { callbackFlag += 1; };
    testSession->NotifyMainModalTypeChange(true);
    EXPECT_EQ(callbackFlag, 2);
}

/**
 * @tc.name: IsModal
 * @tc.desc: IsModal function01
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, IsModal, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsModal";
    info.bundleName_ = "IsModal";
    sptr<MainSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ASSERT_EQ(sceneSession->IsModal(), false);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    property->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    sceneSession->SetSessionProperty(property);
    EXPECT_EQ(sceneSession->IsModal(), true);
}

/**
 * @tc.name: IsApplicationModal
 * @tc.desc: IsApplicationModal function01
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, IsApplicationModal, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsApplicationModal";
    info.bundleName_ = "IsApplicationModal";
    sptr<MainSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    EXPECT_EQ(sceneSession->IsApplicationModal(), false);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    property->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    sceneSession->SetSessionProperty(property);
    EXPECT_EQ(sceneSession->IsApplicationModal(), true);
}

/**
 * @tc.name: SetSessionLabelAndIcon
 * @tc.desc: SetSessionLabelAndIcon
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, SetSessionLabelAndIcon, TestSize.Level1)
{
    std::string label = "";
    std::shared_ptr<Media::PixelMap> icon;
    ASSERT_EQ(WSError::WS_ERROR_INVALID_PERMISSION, mainSession_->SetSessionLabelAndIcon(label, icon));
}

/**
 * @tc.name: SetSessionLabelAndIconInner
 * @tc.desc: SetSessionLabelAndIconInner
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, SetSessionLabelAndIconInner, TestSize.Level1)
{
    std::string label = "test";
    std::shared_ptr<Media::PixelMap> icon;

    ASSERT_EQ(WSError::WS_OK, mainSession_->SetSessionLabelAndIconInner(label, icon));

    mainSession_->updateSessionLabelAndIconFunc_=nullptr;
    ASSERT_EQ(WSError::WS_OK, mainSession_->SetSessionLabelAndIconInner(label, icon));

    mainSession_->SetUpdateSessionLabelAndIconListener(
        [](const std::string& label, const std::shared_ptr<Media::PixelMap>& icon) {
    });
    ASSERT_EQ(WSError::WS_OK, mainSession_->SetSessionLabelAndIconInner(label, icon));
}

/**
 * @tc.name: SetUpdateSessionLabelAndIconListener
 * @tc.desc: SetUpdateSessionLabelAndIconListener
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, SetUpdateSessionLabelAndIconListener, TestSize.Level1)
{
    std::string label = "test";
    std::shared_ptr<Media::PixelMap> icon;
    mainSession_->SetUpdateSessionLabelAndIconListener(
        [](const std::string& label, const std::shared_ptr<Media::PixelMap>& icon) {
    });
    ASSERT_NE(nullptr, mainSession_->updateSessionLabelAndIconFunc_);
}

/**
 * @tc.name: NotifySupportWindowModesChange
 * @tc.desc: NotifySupportWindowModesChange
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, NotifySupportWindowModesChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySupportWindowModesChange";
    info.bundleName_ = "NotifySupportWindowModesChange";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes = { AppExecFwk::SupportWindowMode::FULLSCREEN,
                                                                        AppExecFwk::SupportWindowMode::SPLIT,
                                                                        AppExecFwk::SupportWindowMode::FLOATING };

    EXPECT_EQ(WSError::WS_OK, session->NotifySupportWindowModesChange(supportedWindowModes));

    session->onSetSupportedWindowModesFunc_ = nullptr;
    EXPECT_EQ(WSError::WS_OK, session->NotifySupportWindowModesChange(supportedWindowModes));

    session->onSetSupportedWindowModesFunc_ = [](std::vector<AppExecFwk::SupportWindowMode>&& supportedWindowModes) {
        return;
    };

    EXPECT_EQ(WSError::WS_OK, session->NotifySupportWindowModesChange(supportedWindowModes));
}

/**
 * @tc.name: NotifySupportWindowModesChange02
 * @tc.desc: check func NotifySupportWindowModesChange
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, NotifySupportWindowModesChange02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySupportWindowModesChange02";
    info.bundleName_ = "NotifySupportWindowModesChange02";
    sptr<MainSession> testSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(testSession, nullptr);
    std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes = { AppExecFwk::SupportWindowMode::FULLSCREEN,
                                                                        AppExecFwk::SupportWindowMode::SPLIT,
                                                                        AppExecFwk::SupportWindowMode::FLOATING };

    testSession->onSetSupportedWindowModesFunc_ = nullptr;
    EXPECT_EQ(testSession->NotifySupportWindowModesChange(supportedWindowModes), WSError::WS_OK);

    auto callbackFlag = 1;
    testSession->onSetSupportedWindowModesFunc_ =
        [&callbackFlag](std::vector<AppExecFwk::SupportWindowMode>&& supportedWindowModes) { callbackFlag += 1; };
    testSession->NotifySupportWindowModesChange(supportedWindowModes);
    EXPECT_EQ(callbackFlag, 2);
}

/**
 * @tc.name: NotifySessionLockStateChange
 * @tc.desc: NotifySessionLockStateChange
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, NotifySessionLockStateChange, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "NotifySessionLockStateChangeBundle";
    info.moduleName_ = "NotifySessionLockStateChangeModule";
    info.abilityName_ = "NotifySessionLockStateChangeAbility";
    info.appIndex_ = 0;

    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    bool isLockedState = session->GetSessionLockState();
    session->NotifySessionLockStateChange(isLockedState);
    EXPECT_EQ(session->GetSessionLockState(), isLockedState);

    isLockedState = !session->GetSessionLockState();
    session->NotifySessionLockStateChange(isLockedState);
    EXPECT_EQ(session->GetSessionLockState(), isLockedState);

    isLockedState = true;
    session->SetSessionLockState(isLockedState);
    session->RegisterSessionLockStateChangeCallback([](bool isLockedState) {});
    session->NotifySessionLockStateChange(isLockedState);
    EXPECT_EQ(session->GetSessionLockState(), isLockedState);
}

/**
 * @tc.name: SessionLockState
 * @tc.desc: SetSessionLockState setter and getter test
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, SetSessionLockState, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "SetSessionLockStateBundle";
    info.moduleName_ = "SetSessionLockStateModule";
    info.abilityName_ = "SetSessionLockStateAbility";
    info.appIndex_ = 0;

    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    EXPECT_EQ(session->GetSessionLockState(), false);

    bool isLockedState = true;
    session->SetSessionLockState(isLockedState);
    EXPECT_EQ(session->GetSessionLockState(), isLockedState);

    isLockedState = false;
    session->SetSessionLockState(isLockedState);
    EXPECT_EQ(session->GetSessionLockState(), isLockedState);
}

/**
 * @tc.name: RegisterSessionLockStateChangeCallback
 * @tc.desc: RegisterSessionLockStateChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, RegisterSessionLockStateChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "RegisterSessionLockStateChangeCallbackBundle";
    info.moduleName_ = "RegisterSessionLockStateChangeCallbackModule";
    info.abilityName_ = "RegisterSessionLockStateChangeCallbackAbility";
    info.appIndex_ = 0;

    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    
    bool isLockedState = true;
    session->SetSessionLockState(isLockedState);
    session->RegisterSessionLockStateChangeCallback([](bool isLockedState) {});
    ASSERT_NE(session->onSessionLockStateChangeCallback_, nullptr);
    EXPECT_EQ(session->GetSessionLockState(), isLockedState);
}

/**
 * @tc.name: SetRecentSessionState
 * @tc.desc: check func SetRecentSessionState
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, SetRecentSessionState, TestSize.Level1)
{
    RecentSessionInfo info;
    SessionState state = SessionState::STATE_DISCONNECT;
    mainSession_->SetRecentSessionState(info, state);
    EXPECT_EQ(info.sessionState, RecentSessionState::DISCONNECT);

    state = SessionState::STATE_CONNECT;
    mainSession_->SetRecentSessionState(info, state);
    EXPECT_EQ(info.sessionState, RecentSessionState::CONNECT);

    state = SessionState::STATE_FOREGROUND;
    mainSession_->SetRecentSessionState(info, state);
    EXPECT_EQ(info.sessionState, RecentSessionState::FOREGROUND);

    state = SessionState::STATE_BACKGROUND;
    mainSession_->SetRecentSessionState(info, state);
    EXPECT_EQ(info.sessionState, RecentSessionState::BACKGROUND);

    state = SessionState::STATE_ACTIVE;
    mainSession_->SetRecentSessionState(info, state);
    EXPECT_EQ(info.sessionState, RecentSessionState::ACTIVE);

    state = SessionState::STATE_INACTIVE;
    mainSession_->SetRecentSessionState(info, state);
    EXPECT_EQ(info.sessionState, RecentSessionState::INACTIVE);

    state = SessionState::STATE_END;
    mainSession_->SetRecentSessionState(info, state);
    EXPECT_EQ(info.sessionState, RecentSessionState::END);
}

/**
 * @tc.name: UpdateFlag
 * @tc.desc: UpdateFlag
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, UpdateFlag, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateFlag";
    info.bundleName_ = "UpdateFlag";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);

    session->onUpdateFlagFunc_ = nullptr;
    ASSERT_EQ(nullptr, session->onUpdateFlagFunc_);

    std::string flag = "test";
    EXPECT_EQ(WSError::WS_OK, session->UpdateFlag(flag));

    NotifyUpdateFlagFunc func = [](const std::string& flag) { return; };
    session->onUpdateFlagFunc_ = func;
    EXPECT_NE(nullptr, session->onUpdateFlagFunc_);
    EXPECT_EQ(WSError::WS_OK, session->UpdateFlag(flag));
}

/**
 * @tc.name: NotifySubAndDialogFollowRectChange
 * @tc.desc: NotifySubAndDialogFollowRectChange
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, NotifySubAndDialogFollowRectChange01, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);

    bool isCall = false;
    auto task = [&isCall](const WSRect& rect, bool isGlobal, bool needFlush) { isCall = true; };
    mainSession->RegisterNotifySurfaceBoundsChangeFunc(subSession->GetPersistentId(), std::move(task));
    ASSERT_NE(nullptr, mainSession->notifySurfaceBoundsChangeFuncMap_[subSession->GetPersistentId()]);

    WSRect rect;
    subSession->isFollowParentLayout_ = false;
    mainSession->NotifySubAndDialogFollowRectChange(rect, false, false);
    EXPECT_NE(false, isCall);

    subSession->isFollowParentLayout_ = true;
    sptr<SceneSession::SpecificSessionCallback> callBack = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    mainSession->specificCallback_ = callBack;
    auto getSessionCallBack = [&subSession](int32_t persistentId) { return subSession; };
    callBack->onGetSceneSessionByIdCallback_ = getSessionCallBack;
    mainSession->NotifySubAndDialogFollowRectChange(rect, false, false);
    EXPECT_EQ(true, isCall);
}

/**
 * @tc.name: GetRouterStackInfo
 * @tc.desc: GetRouterStackInfo
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, GetRouterStackInfo, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetRouterStackInfo";
    info.bundleName_ = "GetRouterStackInfo";
    sptr<MainSession> testSession = sptr<MainSession>::MakeSptr(info, nullptr);
    testSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();

    std::string routerInfo;
    auto res = testSession->GetRouterStackInfo(routerInfo);
    EXPECT_EQ(res, WMError::WM_OK);

    testSession->sessionStage_ = nullptr;
    res = testSession->GetRouterStackInfo(routerInfo);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifySubAndDialogFollowRectChange_scaleMode
 * @tc.desc: NotifySubAndDialogFollowRectChange
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, NotifySubAndDialogFollowRectChange_scaleMode, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySubAndDialogFollowRectChange_scaleMode";
    info.bundleName_ = "NotifySubAndDialogFollowRectChange_scaleMode";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> subSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> subSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> subSession3 = nullptr;

    WSRect updateRect1;
    auto task1 = [&updateRect1](const WSRect& rect, bool isGlobal, bool needFlush) { updateRect1 = rect; };
    mainSession->RegisterNotifySurfaceBoundsChangeFunc(subSession1->GetPersistentId(), std::move(task1));
    WSRect updateRect2;
    auto task2 = [&updateRect2](const WSRect& rect, bool isGlobal, bool needFlush) { updateRect2 = rect; };
    mainSession->RegisterNotifySurfaceBoundsChangeFunc(subSession2->GetPersistentId(), std::move(task2));
    ASSERT_NE(nullptr, mainSession->notifySurfaceBoundsChangeFuncMap_[subSession1->GetPersistentId()]);
    WSRect rect = { 100, 100, 400, 400 };
    WSRect resultRect = { 100, 100, 200, 200 };
    float scaleX = 0.5f;
    float scaleY = 0.5f;
    mainSession->SetScale(scaleX, scaleY, 0.5f, 0.5f);
    mainSession->SetSessionRect(rect);
    ASSERT_EQ(rect, mainSession->GetSessionRect());
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    mainSession->property_->SetCompatibleModeProperty(compatibleModeProperty);
    subSession1->isFollowParentLayout_ = true;
    subSession2->isFollowParentLayout_ = true;
    sptr<SceneSession::SpecificSessionCallback> callBack = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    mainSession->specificCallback_ = callBack;
    auto getSessionCallBack = [&subSession1, &subSession2, &subSession3](int32_t persistentId) {
        if (subSession1->GetPersistentId() == persistentId) {
            return subSession1;
        } else if (subSession2->GetPersistentId() == persistentId) {
            return subSession2;
        }
        return subSession3;
    };
    callBack->onGetSceneSessionByIdCallback_ = getSessionCallBack;
    mainSession->NotifySubAndDialogFollowRectChange(rect, false, false);
    mainSession->callingPid_ = 1;
    subSession1->callingPid_ = 2;
    subSession2->callingPid_ = 3;
    EXPECT_EQ(resultRect, updateRect1);
    EXPECT_EQ(resultRect, updateRect2);
}

/**
 * @tc.name: NotifySubAndDialogFollowRectChange_compatMode
 * @tc.desc: NotifySubAndDialogFollowRectChange
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, NotifySubAndDialogFollowRectChange_compatMode, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySubAndDialogFollowRectChange_compatMode";
    info.bundleName_ = "NotifySubAndDialogFollowRectChange_compatMode";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> subSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> subSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);

    WSRect updateRect1;
    auto task1 = [&updateRect1](const WSRect& rect, bool isGlobal, bool needFlush) { updateRect1 = rect; };
    mainSession->RegisterNotifySurfaceBoundsChangeFunc(subSession1->GetPersistentId(), std::move(task1));
    WSRect updateRect2;
    auto task2 = [&updateRect2](const WSRect& rect, bool isGlobal, bool needFlush) { updateRect2 = rect; };
    mainSession->RegisterNotifySurfaceBoundsChangeFunc(subSession2->GetPersistentId(), std::move(task2));
    ASSERT_NE(nullptr, mainSession->notifySurfaceBoundsChangeFuncMap_[subSession1->GetPersistentId()]);
    float scaleX = 0.5f;
    float scaleY = 0.5f;
    mainSession->SetScale(scaleX, scaleY, 0.5f, 0.5f);
    WSRect rect = { 100, 100, 400, 400 };
    subSession1->isFollowParentLayout_ = true;
    subSession2->isFollowParentLayout_ = true;
    sptr<SceneSession::SpecificSessionCallback> callBack = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    mainSession->specificCallback_ = callBack;
    auto getSessionCallBack = [&subSession1, &subSession2](int32_t persistentId) {
        if (subSession1->GetPersistentId() == persistentId) {
            return subSession1;
        } else {
            return subSession2;
        }
    };
    callBack->onGetSceneSessionByIdCallback_ = getSessionCallBack;
    mainSession->NotifySubAndDialogFollowRectChange(rect, false, false);
    EXPECT_EQ(rect, updateRect1);
    EXPECT_EQ(rect, updateRect2);

    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    mainSession->property_->SetCompatibleModeProperty(compatibleModeProperty);
    WSRect resultRect = { 100, 100, 200, 200 };
    mainSession->callingPid_ = 1;
    subSession1->callingPid_ = 1;
    subSession2->callingPid_ = 2;
    mainSession->NotifySubAndDialogFollowRectChange(rect, false, false);
    EXPECT_NE(resultRect, updateRect1);
    EXPECT_EQ(resultRect, updateRect2);
}

/**
 * @tc.name: NotifyIsFullScreenInForceSplitMode
 * @tc.desc: NotifyIsFullScreenInForceSplitMode test
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, NotifyIsFullScreenInForceSplitMode, TestSize.Level3)
{
    SessionInfo info;
    info.abilityName_ = "NotifyIsFullScreenInForceSplitMode";
    info.bundleName_ = "NotifyIsFullScreenInForceSplitMode";
    sptr<MainSession> testSession = sptr<MainSession>::MakeSptr(info, nullptr);
    auto ret = testSession->NotifyIsFullScreenInForceSplitMode(true);
    EXPECT_EQ(ret, WSError::WS_OK);
    testSession->RegisterForceSplitFullScreenChangeCallback([](uint32_t uid, bool isFullScreen) {});
    ret = testSession->NotifyIsFullScreenInForceSplitMode(true);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: RestoreAspectRatioTest
 * @tc.desc: Verify RestoreAspectRatio behavior under different conditions.
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, RestoreAspectRatioTest, TestSize.Level1)
{
    SessionInfo info;
    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    session->Session::SetAspectRatio(0.0f);

    // Case 1: ratio nearly zero
    {
        float ratio = 0.0f;
        bool ret = session->RestoreAspectRatio(ratio);
        EXPECT_FALSE(ret);
        EXPECT_FLOAT_EQ(session->GetAspectRatio(), 0.0f);
    }

    // Case 2: ratio valid, moveDragController_ is not nullptr
    {
        float ratio = 1.6f;
        bool ret = session->RestoreAspectRatio(ratio);
        EXPECT_TRUE(ret);
        EXPECT_FLOAT_EQ(session->GetAspectRatio(), ratio);
    }

    // Case 3: ratio valid, moveDragController_ is nullptr
    {
        float ratio = 1.33f;
        session->moveDragController_ = nullptr;
        bool ret = session->RestoreAspectRatio(ratio);
        EXPECT_TRUE(ret);
        EXPECT_FLOAT_EQ(session->GetAspectRatio(), ratio);
    }
}

/**
 * @tc.name: IsExitSplitOnBackgroundRecover
 * @tc.desc: Verify IsExitSplitOnBackgroundRecover.
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, IsExitSplitOnBackgroundRecover, TestSize.Level1)
{
    SessionInfo info;
    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);

    ASSERT_NE(session, nullptr);
    int32_t snapShotRecoverValue = 0;
    session->UpdateWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    EXPECT_EQ(session->IsExitSplitOnBackgroundRecover(), false);
    session->UpdateWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    EXPECT_EQ(session->IsExitSplitOnBackgroundRecover(), true);
    session->UpdateWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    EXPECT_EQ(session->IsExitSplitOnBackgroundRecover(), true);
    session->UpdateWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->isExitSplitOnBackground_ = true;
    EXPECT_EQ(session->IsExitSplitOnBackgroundRecover(), true);
}

/**
 * @tc.name: GetAppForceLandscapeConfigEnable01
 * @tc.desc: Test GetAppForceLandscapeConfigEnable when forceSplitEnableFunc_ is null
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, GetAppForceLandscapeConfigEnable01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "testMainSession1";
    info.moduleName_ = "testMainSession2";
    info.bundleName_ = "testMainSession3";
    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    
    bool enableForceSplit = false;
    WMError res = session->GetAppForceLandscapeConfigEnable(enableForceSplit);
    EXPECT_EQ(res, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: GetAppForceLandscapeConfigEnable02
 * @tc.desc: Test GetAppForceLandscapeConfigEnable when forceSplitEnableFunc_ is set
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, GetAppForceLandscapeConfigEnable02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "testMainSession1";
    info.moduleName_ = "testMainSession2";
    info.bundleName_ = "testBundle";
    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    
    bool expectedEnable = true;
    session->RegisterForceSplitEnableListener(
        [expectedEnable](const std::string& bundleName) {
            return expectedEnable;
        });
    
    bool enableForceSplit = false;
    WMError res = session->GetAppForceLandscapeConfigEnable(enableForceSplit);
    EXPECT_EQ(res, WMError::WM_OK);
    EXPECT_EQ(enableForceSplit, expectedEnable);
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigEnableUpdated01
 * @tc.desc: Test NotifyAppForceLandscapeConfigEnableUpdated when sessionStage_ is null
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, NotifyAppForceLandscapeConfigEnableUpdated01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "testMainSession1";
    info.moduleName_ = "testMainSession2";
    info.bundleName_ = "testMainSession3";
    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    session->sessionStage_ = nullptr;
    
    WSError res = session->NotifyAppForceLandscapeConfigEnableUpdated();
    EXPECT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigEnableUpdated02
 * @tc.desc: Test NotifyAppForceLandscapeConfigEnableUpdated when sessionStage_ is set
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, NotifyAppForceLandscapeConfigEnableUpdated02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "testMainSession1";
    info.moduleName_ = "testMainSession2";
    info.bundleName_ = "testMainSession3";
    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    session->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    
    WSError res = session->NotifyAppForceLandscapeConfigEnableUpdated();
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: RegisterForceSplitEnableListener
 * @tc.desc: Test RegisterForceSplitEnableListener
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, RegisterForceSplitEnableListener, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "testMainSession1";
    info.moduleName_ = "testMainSession2";
    info.bundleName_ = "testBundle";
    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    
    bool callbackCalled = false;
    bool callbackResult = true;
    session->RegisterForceSplitEnableListener(
        [&callbackCalled, callbackResult](const std::string& bundleName) {
            callbackCalled = true;
            return callbackResult;
        });
    
    bool enableForceSplit = false;
    WMError res = session->GetAppForceLandscapeConfigEnable(enableForceSplit);
    EXPECT_EQ(res, WMError::WM_OK);
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(enableForceSplit, callbackResult);
}
} // namespace
} // namespace Rosen
} // namespace OHOS