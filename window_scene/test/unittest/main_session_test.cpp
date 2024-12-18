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
};

void MainSessionTest::SetUpTestCase()
{
}

void MainSessionTest::TearDownTestCase()
{
}

void MainSessionTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testMainSession1";
    info.moduleName_ = "testMainSession2";
    info.bundleName_ = "testMainSession3";
    mainSession_ = new (std::nothrow) MainSession(info, specificCallback);
    EXPECT_NE(nullptr, mainSession_);
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
HWTEST_F(MainSessionTest, MainSession01, Function | SmallTest | Level1)
{
    MainSession* pMainSession = nullptr;
    sptr<MainSession::SpecificSessionCallback> pSpecificCallback = nullptr;

    SessionInfo info;
    info.persistentId_ = -1;
    info.abilityName_ = "";
    info.moduleName_ = "";
    info.bundleName_ = "";
    pMainSession = new (std::nothrow) MainSession(info, pSpecificCallback);
    EXPECT_NE(nullptr, pMainSession);

    info.persistentId_ = 0;
    pMainSession = new (std::nothrow) MainSession(info, pSpecificCallback);
    EXPECT_NE(nullptr, pMainSession);

    info.persistentId_ = -1;
    info.abilityName_ = "MainSession01";
    info.moduleName_ = "MainSession02";
    info.bundleName_ = "MainSession03";
    pSpecificCallback = new(std::nothrow) MainSession::SpecificSessionCallback;
    pMainSession = new (std::nothrow) MainSession(info, pSpecificCallback);
    EXPECT_NE(nullptr, pMainSession);

    info.persistentId_ = 0;
    pMainSession = new (std::nothrow) MainSession(info, pSpecificCallback);
    EXPECT_NE(nullptr, pMainSession);
}

/**
 * @tc.name: TransferKeyEvent01
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, TransferKeyEvent01, Function | SmallTest | Level1)
{
    mainSession_->state_ = SessionState::STATE_END;

    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, mainSession_->TransferKeyEvent(nullptr));
}

/**
 * @tc.name: TransferKeyEvent02
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, TransferKeyEvent02, Function | SmallTest | Level1)
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
HWTEST_F(MainSessionTest, TransferKeyEvent03, Function | SmallTest | Level1)
{
    mainSession_->state_ = SessionState::STATE_CONNECT;
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    SessionInfo info;
    info.abilityName_ = "testDialogSession1";
    info.moduleName_ = "testDialogSession2";
    info.bundleName_ = "testDialogSession3";
    sptr<Session> dialogSession = new (std::nothrow) SystemSession(info, nullptr);
    dialogSession->SetSessionState(SessionState::STATE_ACTIVE);
    mainSession_->BindDialogToParentSession(dialogSession);

    ASSERT_EQ(WSError::WS_ERROR_INVALID_PERMISSION, mainSession_->TransferKeyEvent(keyEvent));
}


/**
 * @tc.name: ProcessPointDownSession01
 * @tc.desc: check func ProcessPointDownSession
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, ProcessPointDownSession01, Function | SmallTest | Level1)
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
HWTEST_F(MainSessionTest, ProcessPointDownSession02, Function | SmallTest | Level1)
{
    mainSession_->BindDialogToParentSession(mainSession_);
    EXPECT_EQ(WSError::WS_OK, mainSession_->ProcessPointDownSession(10, 20));
}

/**
 * @tc.name: SetTopmost01
 * @tc.desc: check func SetTopmost
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, SetTopmost01, Function | SmallTest | Level1)
{
    EXPECT_EQ(WSError::WS_OK, mainSession_->SetTopmost(true));
    EXPECT_EQ(WSError::WS_OK, mainSession_->SetTopmost(false));
}
/**
 * @tc.name: SetTopmost02
 * @tc.desc: check func SetTopmost
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, SetTopmost02, Function | SmallTest | Level1)
{
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    mainSession_->SetSessionProperty(property);
    ASSERT_TRUE(mainSession_->GetSessionProperty() != nullptr);
    EXPECT_EQ(WSError::WS_OK, mainSession_->SetTopmost(true));

    sptr<SceneSession::SessionChangeCallback> sessionChangeCallback = new SceneSession::SessionChangeCallback();
    ASSERT_TRUE(sessionChangeCallback != nullptr);

    mainSession_->RegisterSessionChangeCallback(sessionChangeCallback);
    EXPECT_EQ(WSError::WS_OK, mainSession_->SetTopmost(false));
}

/**
 * @tc.name: UpdatePointerArea01
 * @tc.desc: check func UpdatePointerArea
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, UpdatePointerArea01, Function | SmallTest | Level1)
{
    WSRect Rect={0, 0, 50, 50};
    mainSession_->UpdateWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    mainSession_->UpdatePointerArea(Rect);
    mainSession_->UpdateWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    mainSession_->UpdatePointerArea(Rect);
}

/**
 * @tc.name: CheckPointerEventDispatch03
 * @tc.desc: check func CheckPointerEventDispatch
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, CheckPointerEventDispatch03, Function | SmallTest | Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();

    mainSession_->SetSessionState(SessionState::STATE_FOREGROUND);
    mainSession_->CheckPointerEventDispatch(pointerEvent);

    mainSession_->SetSessionState(SessionState::STATE_ACTIVE);
    mainSession_->CheckPointerEventDispatch(pointerEvent);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_LEAVE_WINDOW);
    mainSession_->SetSessionState(SessionState::STATE_DISCONNECT);
    mainSession_->CheckPointerEventDispatch(pointerEvent);

    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_PULL_DOWN);
    mainSession_->SetSessionState(SessionState::STATE_DISCONNECT);
    mainSession_->CheckPointerEventDispatch(pointerEvent);
}

/**
 * @tc.name: RectCheck03
 * @tc.desc: check func RectCheck
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, RectCheck03, Function | SmallTest | Level1)
{
    ASSERT_NE(mainSession_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testMainSessionRectCheck";
    info.moduleName_ = "testMainSessionRectCheck";
    info.bundleName_ = "testMainSessionRectCheck";
    sptr<Session> session = new (std::nothrow) Session(info);
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
 * @tc.name: SetExitSplitOnBackground
 * @tc.desc: check func SetExitSplitOnBackground
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, SetExitSplitOnBackground, Function | SmallTest | Level1)
{
    bool isExitSplitOnBackground = true;
    mainSession_->SetExitSplitOnBackground(isExitSplitOnBackground);
    ASSERT_EQ(true, isExitSplitOnBackground);
}

/**
 * @tc.name: IsExitSplitOnBackground01
 * @tc.desc: check func IsExitSplitOnBackground
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, IsExitSplitOnBackground01, Function | SmallTest | Level1)
{
    bool isExitSplitOnBackground = true;
    mainSession_->SetExitSplitOnBackground(isExitSplitOnBackground);
    bool ret = mainSession_->IsExitSplitOnBackground();
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: IsExitSplitOnBackground02
 * @tc.desc: check func IsExitSplitOnBackground
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, IsExitSplitOnBackground02, Function | SmallTest | Level1)
{
    bool isExitSplitOnBackground = false;
    mainSession_->SetExitSplitOnBackground(isExitSplitOnBackground);
    bool ret = mainSession_->IsExitSplitOnBackground();
    ASSERT_EQ(false, ret);
}

/**
 * @tc.name: OnTitleAndDockHoverShowChange
 * @tc.desc: OnTitleAndDockHoverShowChange
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, OnTitleAndDockHoverShowChange, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "OnTitleAndDockHoverShowChange";
    info.bundleName_ = "OnTitleAndDockHoverShowChange";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetTitleAndDockHoverShowChangeCallback([](bool isTitleHoverShown, bool isDockHoverShown) {
        return;
    });
    EXPECT_NE(sceneSession->onTitleAndDockHoverShowChangeFunc_, nullptr);
    EXPECT_EQ(sceneSession->OnTitleAndDockHoverShowChange(true, true), WSError::WS_OK);
}

/**
 * @tc.name: OnRestoreMainWindow
 * @tc.desc: OnRestoreMainWindow function01
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, OnRestoreMainWindow, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "OnRestoreMainWindow";
    info.bundleName_ = "OnRestoreMainWindow";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    EXPECT_EQ(WSError::WS_OK, session->OnRestoreMainWindow());

    session->onRestoreMainWindowFunc_ = nullptr;
    EXPECT_EQ(WSError::WS_OK, session->OnRestoreMainWindow());

    NotifyRestoreMainWindowFunc func = []() {
        return;
    };
    session->onRestoreMainWindowFunc_ = func;
    EXPECT_EQ(WSError::WS_OK, session->OnRestoreMainWindow());
}

/**
 * @tc.name: OnSetWindowRectAutoSave
 * @tc.desc: OnSetWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(MainSessionTest, OnSetWindowRectAutoSave, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "OnSetWindowRectAutoSave";
    info.bundleName_ = "OnSetWindowRectAutoSave";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    EXPECT_EQ(WSError::WS_OK, session->OnSetWindowRectAutoSave(true));

    session->onSetWindowRectAutoSaveFunc_ = nullptr;
    EXPECT_EQ(WSError::WS_OK, session->OnSetWindowRectAutoSave(true));

    NotifySetWindowRectAutoSaveFunc func = [](bool enabled) {
        return;
    };
    session->onSetWindowRectAutoSaveFunc_ = func;
    EXPECT_EQ(WSError::WS_OK, session->OnSetWindowRectAutoSave(true));
}
}
}
}