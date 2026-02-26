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
#include "session/host/include/sub_session.h"

#include "common/include/session_permission.h"
#include "key_event.h"
#include "mock/mock_session_stage.h"
#include "mock_sub_session.h"
#include "pointer_event.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/host/include/main_session.h"
#include "session/host/include/session.h"
#include "session/host/include/system_session.h"
#include "session/screen/include/screen_session.h"
#include "window_event_channel_base.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "window_property.h"
#include "window_session_property.h"
#include <ui/rs_surface_node.h>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SubSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    SessionInfo info;
    sptr<SubSession::SpecificSessionCallback> specificCallback = nullptr;

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    sptr<SubSession> subSession_;
    SystemSessionConfig systemConfig_;
};

void SubSessionTest::SetUpTestCase() {}

void SubSessionTest::TearDownTestCase() {}

void SubSessionTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testMainSession1";
    info.moduleName_ = "testMainSession2";
    info.bundleName_ = "testMainSession3";
    subSession_ = sptr<SubSession>::MakeSptr(info, specificCallback);
    EXPECT_NE(nullptr, subSession_);
}

void SubSessionTest::TearDown()
{
    subSession_ = nullptr;
}

RSSurfaceNode::SharedPtr SubSessionTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    return surfaceNode;
}

namespace {

/**
 * @tc.name: TransferKeyEvent01
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, TransferKeyEvent01, TestSize.Level1)
{
    subSession_->state_ = SessionState::STATE_END;

    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, subSession_->TransferKeyEvent(nullptr));
}

/**
 * @tc.name: TransferKeyEvent02
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, TransferKeyEvent02, TestSize.Level1)
{
    subSession_->state_ = SessionState::STATE_CONNECT;
    std::shared_ptr<MMI::KeyEvent> keyEvent = nullptr;

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, subSession_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: TransferKeyEvent03
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, TransferKeyEvent03, TestSize.Level1)
{
    ASSERT_NE(subSession_, nullptr);
    subSession_->state_ = SessionState::STATE_CONNECT;
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    subSession_->SetParentSession(nullptr);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, subSession_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: TransferKeyEvent04
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, TransferKeyEvent04, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "TransferKeyEvent04";
    sessionInfo.moduleName_ = "TransferKeyEvent04";
    sessionInfo.bundleName_ = "TransferKeyEvent04";
    sptr<SubSession> session = sptr<SubSession>::MakeSptr(sessionInfo, specificCallback);
    ASSERT_NE(session, nullptr);
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);

    subSession_->SetParentSession(session);
    subSession_->SetSessionState(SessionState::STATE_ACTIVE);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, subSession_->TransferKeyEvent(keyEvent));
}

/**
 * @tc.name: IsTopmost01
 * @tc.desc: check func IsTopmost
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, IsTopmost01, TestSize.Level1)
{
    subSession_->GetSessionProperty()->SetTopmost(false);
    ASSERT_EQ(false, subSession_->IsTopmost());

    subSession_->GetSessionProperty()->SetTopmost(true);
    ASSERT_EQ(true, subSession_->IsTopmost());
}

/**
 * @tc.name: IsTopmost02
 * @tc.desc: check func IsTopmost
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, IsTopmost02, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    subSession_->SetSessionProperty(property);
    ASSERT_TRUE(subSession_->GetSessionProperty() != nullptr);

    subSession_->GetSessionProperty()->SetTopmost(true);
    ASSERT_EQ(true, subSession_->IsTopmost());
}

/**
 * @tc.name: CheckPointerEventDispatch01
 * @tc.desc: check func CheckPointerEventDispatch
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, CheckPointerEventDispatch01, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;

    ASSERT_TRUE(subSession_ != nullptr);
    auto result = subSession_->CheckPointerEventDispatch(pointerEvent);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: CheckPointerEventDispatch02
 * @tc.desc: check func CheckPointerEventDispatch
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, CheckPointerEventDispatch02, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    ASSERT_TRUE(subSession_ != nullptr);
    subSession_->SetSessionState(SessionState::STATE_FOREGROUND);
    auto result = subSession_->CheckPointerEventDispatch(pointerEvent);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: CheckPointerEventDispatch03
 * @tc.desc: check func CheckPointerEventDispatch
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, CheckPointerEventDispatch03, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    ASSERT_TRUE(subSession_ != nullptr);
    subSession_->SetSessionState(SessionState::STATE_BACKGROUND);
    subSession_->UpdateSessionState(SessionState::STATE_ACTIVE);
    auto result = subSession_->CheckPointerEventDispatch(pointerEvent);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: CheckPointerEventDispatch04
 * @tc.desc: check func CheckPointerEventDispatch
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, CheckPointerEventDispatch04, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    ASSERT_TRUE(subSession_ != nullptr);
    subSession_->SetSessionState(SessionState::STATE_BACKGROUND);
    subSession_->UpdateSessionState(SessionState::STATE_INACTIVE);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_LEAVE_WINDOW);
    auto result = subSession_->CheckPointerEventDispatch(pointerEvent);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: CheckPointerEventDispatch05
 * @tc.desc: check func CheckPointerEventDispatch
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, CheckPointerEventDispatch05, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    ASSERT_TRUE(subSession_ != nullptr);
    subSession_->SetSessionState(SessionState::STATE_BACKGROUND);
    subSession_->UpdateSessionState(SessionState::STATE_INACTIVE);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    auto result = subSession_->CheckPointerEventDispatch(pointerEvent);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: IsModal01
 * @tc.desc: check func IsModal
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, IsModal, TestSize.Level1)
{
    ASSERT_FALSE(subSession_->IsModal());
}

/**
 * @tc.name: IsVisibleForeground01
 * @tc.desc: check func IsVisibleForeground
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, IsVisibleForeground01, TestSize.Level1)
{
    ASSERT_FALSE(subSession_->IsVisibleForeground());

    SessionInfo info;
    info.abilityName_ = "testMainSession1";
    info.moduleName_ = "testMainSession2";
    info.bundleName_ = "testMainSession3";
    auto parentSession = sptr<SubSession>::MakeSptr(info, specificCallback);

    subSession_->SetParentSession(parentSession);
    ASSERT_FALSE(subSession_->IsVisibleForeground());
}

/**
 * @tc.name: RectCheck
 * @tc.desc: test function : RectCheck
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, RectCheck, TestSize.Level1)
{
    ASSERT_NE(subSession_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testRectCheck";
    info.moduleName_ = "testRectCheck";
    info.bundleName_ = "testRectCheck";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    EXPECT_NE(nullptr, session);
    subSession_->parentSession_ = session;
    uint32_t curWidth = 100;
    uint32_t curHeight = 200;
    subSession_->RectCheck(curWidth, curHeight);

    curWidth = 300;
    curHeight = 200;
    subSession_->RectCheck(curWidth, curHeight);

    curWidth = 1930;
    curHeight = 200;
    subSession_->RectCheck(curWidth, curHeight);

    curWidth = 330;
    curHeight = 200;
    subSession_->RectCheck(curWidth, curHeight);

    curWidth = 330;
    curHeight = 1930;
    subSession_->RectCheck(curWidth, curHeight);
}

/**
 * @tc.name: IsModal
 * @tc.desc: IsModal function01
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, IsModal01, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    EXPECT_EQ(subSession_->IsModal(), false);
    subSession_->SetSessionProperty(property);
    EXPECT_EQ(subSession_->IsModal(), true);
}

/**
 * @tc.name: IsApplicationModal
 * @tc.desc: IsApplicationModal function01
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, IsApplicationModal, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    property->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL);
    EXPECT_EQ(subSession_->IsApplicationModal(), false);
    subSession_->SetSessionProperty(property);
    EXPECT_EQ(subSession_->IsApplicationModal(), true);
}

/**
 * @tc.name: NotifySessionRectChange
 * @tc.desc: NotifySessionRectChange function01
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, NotifySessionRectChange01, TestSize.Level1)
{
    subSession_->shouldFollowParentWhenShow_ = true;
    WSRect rect;
    subSession_->NotifySessionRectChange(rect, SizeChangeReason::UNDEFINED, DISPLAY_ID_INVALID);
    ASSERT_EQ(subSession_->shouldFollowParentWhenShow_, true);
    subSession_->NotifySessionRectChange(rect, SizeChangeReason::DRAG_END, DISPLAY_ID_INVALID);
    ASSERT_EQ(subSession_->shouldFollowParentWhenShow_, false);
}

/**
 * @tc.name: UpdateSessionRectInner
 * @tc.desc: UpdateSessionRectInner function01
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, UpdateSessionRectInner01, TestSize.Level1)
{
    subSession_->shouldFollowParentWhenShow_ = true;
    WSRect rect;
    MoveConfiguration config;
    config.displayId = DISPLAY_ID_INVALID;
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::UNDEFINED, config);
    ASSERT_EQ(subSession_->shouldFollowParentWhenShow_, true);
    config.displayId = 123;
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::DRAG_END, config);
    ASSERT_EQ(subSession_->shouldFollowParentWhenShow_, false);
}

/**
 * @tc.name: IsVisibleForeground
 * @tc.desc: IsVisibleForeground Test
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, IsVisibleForeground, TestSize.Level1)
{
    systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    subSession_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(subSession_->IsVisibleForeground(), false);
    subSession_->SetSessionState(SessionState::STATE_FOREGROUND);
    EXPECT_EQ(subSession_->IsVisibleForeground(), false);
    subSession_->isVisible_ = true;
    EXPECT_EQ(subSession_->IsVisibleForeground(), true);

    SessionInfo info;
    info.abilityName_ = "IsVisibleForeground";
    info.moduleName_ = "IsVisibleForeground";
    info.bundleName_ = "IsVisibleForeground";
    sptr<Session> parentSession = sptr<Session>::MakeSptr(info);
    parentSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    subSession_->SetParentSession(parentSession);
    EXPECT_EQ(subSession_->IsVisibleForeground(), false);
    parentSession->SetSessionState(SessionState::STATE_FOREGROUND);
    EXPECT_EQ(subSession_->IsVisibleForeground(), false);
    parentSession->isVisible_ = true;
    EXPECT_EQ(subSession_->IsVisibleForeground(), true);
}

/**
 * @tc.name: SetParentSessionCallback
 * @tc.desc: SetParentSessionCallback
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, SetParentSessionCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetParentSessionCallback";
    info.bundleName_ = "SetParentSessionCallback";
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);
    subSession->SetParentSessionCallback(nullptr);
    EXPECT_EQ(subSession->setParentSessionFunc_, nullptr);

    NotifySetParentSessionFunc func = [](int32_t oldParentWindowId, int32_t newParentWindowId) { return; };
    subSession->SetParentSessionCallback(std::move(func));
    EXPECT_NE(subSession->setParentSessionFunc_, nullptr);
}

/**
 * @tc.name: NotifySetParentSession
 * @tc.desc: NotifySetParentSession
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, NotifySetParentSession, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySetParentSession";
    info.bundleName_ = "NotifySetParentSession";
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);
    int32_t oldParentWindowId = 1;
    int32_t newParentWindowId = 2;
    auto res = subSession->NotifySetParentSession(oldParentWindowId, newParentWindowId);
    EXPECT_EQ(res, WMError::WM_OK);

    NotifySetParentSessionFunc func = [](int32_t oldParentWindowId, int32_t newParentWindowId) { return; };
    subSession->SetParentSessionCallback(std::move(func));
    res = subSession->NotifySetParentSession(oldParentWindowId, newParentWindowId);
    EXPECT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: NotifyFollowParentMultiScreenPolicy
 * @tc.desc: NotifyFollowParentMultiScreenPolicy
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, NotifyFollowParentMultiScreenPolicy, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subSession_->SetSessionProperty(property);
    EXPECT_EQ(subSession_->NotifyFollowParentMultiScreenPolicy(true), WSError::WS_OK);
    EXPECT_EQ(subSession_->NotifyFollowParentMultiScreenPolicy(false), WSError::WS_OK);
}

/**
 * @tc.name: IsFollowParentMultiScreenPolicy
 * @tc.desc: IsFollowParentMultiScreenPolicy
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, IsFollowParentMultiScreenPolicy, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subSession_->SetSessionProperty(property);
    EXPECT_EQ(subSession_->IsFollowParentMultiScreenPolicy(), false);
    EXPECT_EQ(subSession_->NotifyFollowParentMultiScreenPolicy(true), WSError::WS_OK);
    EXPECT_EQ(subSession_->IsFollowParentMultiScreenPolicy(), true);
    EXPECT_EQ(subSession_->NotifyFollowParentMultiScreenPolicy(false), WSError::WS_OK);
    EXPECT_EQ(subSession_->IsFollowParentMultiScreenPolicy(), false);
}

/**
 * @tc.name: UpdateSessionRectInner02
 * @tc.desc: UpdateSessionRectInner Test
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, UpdateSessionRectInner02, TestSize.Level1)
{
    subSession_->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    SessionInfo info;
    info.abilityName_ = "UpdateSessionRectInner02";
    info.bundleName_ = "UpdateSessionRectInner02";
    sptr<SceneSession> parentSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    parentSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    parentSession->moveDragController_ =
        sptr<MoveDragController>::MakeSptr(wptr(parentSession));
    parentSession->subSession_.emplace_back(subSession_);
    subSession_->SetParentSession(parentSession);

    parentSession->moveDragController_->isStartMove_ = false;
    parentSession->moveDragController_->isStartDrag_ = false;
    WSRect defaultRect;
    WSRect rect = { 50, 50, 800, 800 };
    MoveConfiguration config;
    config.displayId = DISPLAY_ID_INVALID;
    subSession_->NotifyFollowParentMultiScreenPolicy(true);
    parentSession->moveDragController_->isStartMove_ = true;
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::MOVE, config);
    ASSERT_EQ(50, subSession_->GetRequestRectWhenFollowParent().posX_);
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::RESIZE, config);
    ASSERT_EQ(800, subSession_->GetRequestRectWhenFollowParent().width_);

    subSession_->SetRequestRectWhenFollowParent(defaultRect);
    parentSession->moveDragController_->isStartMove_ = false;
    parentSession->moveDragController_->isStartDrag_ = true;
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::MOVE, config);
    ASSERT_EQ(50, subSession_->GetRequestRectWhenFollowParent().posX_);
    rect.width_ = 0;
    rect.height_ = 0;
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::RESIZE, config);
    ASSERT_NE(0, subSession_->GetRequestRectWhenFollowParent().width_);
}

/**
 * @tc.name: UpdateSessionRectInner03
 * @tc.desc: UpdateSessionRectInner Test
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, UpdateSessionRectInner03, TestSize.Level1)
{
    MoveConfiguration config;
    WSRect rect = { 1, 1, 1, 1 };
    subSession_->GetLayoutController()->SetSessionRect({ 0, 0, 0, 0 });

    subSession_->isSubWindowResizingOrMoving_ = false;
    subSession_->SetSessionState(SessionState::STATE_BACKGROUND);
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::MOVE, config);
    ASSERT_EQ(subSession_->isSubWindowResizingOrMoving_, false);

    subSession_->SetSessionState(SessionState::STATE_FOREGROUND);
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::UNDEFINED, config);
    ASSERT_EQ(subSession_->isSubWindowResizingOrMoving_, false);

    subSession_->isSubWindowResizingOrMoving_ = false;
    subSession_->GetLayoutController()->SetSessionRect({ 0, 0, 0, 0 });
    rect = { 0, 0, 0, 0 };
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::MOVE, config);
    ASSERT_EQ(subSession_->isSubWindowResizingOrMoving_, false);

    subSession_->isSubWindowResizingOrMoving_ = false;
    subSession_->GetLayoutController()->SetSessionRect({ 0, 0, 0, 0 });
    rect = { 1, 0, 0, 0 };
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::MOVE, config);
    ASSERT_EQ(subSession_->isSubWindowResizingOrMoving_, true);

    subSession_->isSubWindowResizingOrMoving_ = false;
    subSession_->GetLayoutController()->SetSessionRect({ 0, 0, 0, 0 });
    rect = { 0, 1, 0, 0 };
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::MOVE, config);
    ASSERT_EQ(subSession_->isSubWindowResizingOrMoving_, true);

    subSession_->isSubWindowResizingOrMoving_ = false;
    subSession_->GetLayoutController()->SetSessionRect({ 0, 0, 0, 0 });
    rect = { 0, 0, 0, 0 };
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::RESIZE, config);
    ASSERT_EQ(subSession_->isSubWindowResizingOrMoving_, false);

    subSession_->isSubWindowResizingOrMoving_ = false;
    subSession_->GetLayoutController()->SetSessionRect({ 0, 0, 0, 0 });
    rect = { 0, 0, 1, 0 };
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::RESIZE, config);
    ASSERT_EQ(subSession_->isSubWindowResizingOrMoving_, true);

    subSession_->isSubWindowResizingOrMoving_ = false;
    subSession_->GetLayoutController()->SetSessionRect({ 0, 0, 0, 0 });
    rect = { 0, 0, 0, 1 };
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::RESIZE, config);
    ASSERT_EQ(subSession_->isSubWindowResizingOrMoving_, true);
}

/**
 * @tc.name: HandleCrossMoveToSurfaceNode
 * @tc.desc: HandleCrossMoveToSurfaceNode
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, HandleCrossMoveToSurfaceNode, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleCrossMoveToSurfaceNode";
    info.bundleName_ = "HandleCrossMoveToSurfaceNode";
    sptr<SubSession> sceneSession = sptr<SubSession>::MakeSptr(info, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->displayIdSetDuringMoveTo_.insert(0);
    WSRect rect = { 50, 50, 800, 800 };
    sceneSession->HandleCrossMoveToSurfaceNode(rect);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = info.abilityName_;
    RSSurfaceNodeType rsSurfaceNodeType = RSSurfaceNodeType::DEFAULT;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, rsSurfaceNodeType);
    ASSERT_NE(surfaceNode, nullptr);
    sceneSession->SetSurfaceNode(surfaceNode);
    sceneSession->HandleCrossMoveToSurfaceNode(rect);
    ASSERT_NE(0, sceneSession->displayIdSetDuringMoveTo_.size());
    sceneSession->SetZOrder(101);
    rect = { 900, 900, 800, 800};
    surfaceNode->SetPositionZ(0);
    //Constructing screen rect information
    auto screenId = 1001;
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);
    ScreenProperty screenProperty;
    screenProperty.SetStartX(1000);
    screenProperty.SetStartY(1000);
    screenProperty.SetBounds({{0, 0, 1000, 1000}, 10.0f, 10.0f});
    screenProperty.SetScreenType(ScreenType::REAL);
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    ASSERT_NE(screenSession, nullptr);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(ScreenId, screenSession);
    //Set screen type
    screenSession->GetScreenProperty().SetScreenType(ScreenType::REAL);
    //test can not find drag move mounted node
    sceneSession->HandleCrossMoveToSurfaceNode(rect);
    EXPECT_EQ(surfaceNode->GetStagingProperties().GetPositionZ(), 0);
    //Register lookup node function
    sceneSession->SetFindScenePanelRsNodeByZOrderFunc([this](uint64_t screenId, uint32_t targetZOrder) {
        return CreateRSSurfaceNode();
    });
    //test find drag move mounted node
    int32_t curCloneNodeCount = sceneSession->cloneNodeCount_;
    sceneSession->displayIdSetDuringMoveTo_.clear();
    sceneSession->HandleCrossMoveToSurfaceNode(rect);
    EXPECT_EQ(sceneSession->cloneNodeCount_, curCloneNodeCount + 1);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
}

/**
 * @tc.name: AddSurfaceNodeToScreen
 * @tc.desc: AddSurfaceNodeToScreen
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, AddSurfaceNodeToScreen, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "AddSurfaceNodeToScreen";
    info.bundleName_ = "AddSurfaceNodeToScreen";
    sptr<SubSession> sceneSession = sptr<SubSession>::MakeSptr(info, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->moveDragController_ =
        sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    sceneSession->AddSurfaceNodeToScreen(0);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = info.abilityName_;
    RSSurfaceNodeType rsSurfaceNodeType = RSSurfaceNodeType::DEFAULT;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, rsSurfaceNodeType);
    ASSERT_NE(surfaceNode, nullptr);
    sceneSession->SetSurfaceNode(surfaceNode);
    sceneSession->SetScreenId(0);
    WSRect rect = {900, 900, 800, 800};
    sceneSession->SetZOrder(101);
    sceneSession->SetSessionRect(rect);
    surfaceNode->SetPositionZ(0);
    auto originScreenId = 0;
    ScreenProperty originScreenProperty;
    originScreenProperty.SetStartX(0);
    originScreenProperty.SetStartY(0);
    originScreenProperty.SetBounds({{0, 0, 1000, 1000}, 10.0f, 10.0f});
    originScreenProperty.SetScreenType(ScreenType::REAL);
    sptr<ScreenSession> originScreenSession =
        sptr<ScreenSession>::MakeSptr(originScreenId, originScreenProperty, originScreenId);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(0, originScreenSession);
    auto screenId = 1001;
    ScreenProperty screenProperty;
    screenProperty.SetStartX(1000);
    screenProperty.SetStartY(1000);
    screenProperty.SetBounds({{0, 0, 1000, 1000}, 10.0f, 10.0f});
    screenProperty.SetScreenType(ScreenType::REAL);
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(screenId, screenSession);
    screenSession->GetScreenProperty().SetScreenType(ScreenType::REAL);
    sceneSession->AddSurfaceNodeToScreen(100);
    EXPECT_EQ(surfaceNode->GetStagingProperties().GetPositionZ(), 0);
    sceneSession->SetFindScenePanelRsNodeByZOrderFunc([this](uint64_t screenId, uint32_t targetZOrder) {
        return CreateRSSurfaceNode();
    });
    int32_t curCloneNodeCount = sceneSession->cloneNodeCount_;
    sceneSession->displayIdSetDuringMoveTo_.clear();
    sceneSession->AddSurfaceNodeToScreen(100);
    EXPECT_EQ(sceneSession->cloneNodeCount_, curCloneNodeCount + 2);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
}

/**
 * @tc.name: RemoveSurfaceNodeFromScreen
 * @tc.desc: RemoveSurfaceNodeFromScreen
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, RemoveSurfaceNodeFromScreen, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RemoveSurfaceNodeFromScreen";
    info.bundleName_ = "RemoveSurfaceNodeFromScreen";
    sptr<SubSession> sceneSession = sptr<SubSession>::MakeSptr(info, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->RemoveSurfaceNodeFromScreen();
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = info.abilityName_;
    RSSurfaceNodeType rsSurfaceNodeType = RSSurfaceNodeType::DEFAULT;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, rsSurfaceNodeType);
    ASSERT_NE(surfaceNode, nullptr);
    sceneSession->SetSurfaceNode(surfaceNode);
    sceneSession->displayIdSetDuringMoveTo_.insert(0);
    sceneSession->displayIdSetDuringMoveTo_.insert(888);
    sceneSession->RemoveSurfaceNodeFromScreen();
    EXPECT_EQ(0, sceneSession->cloneNodeCount_);

    // Constructing screen rect information
    auto screenId = 1001;
    ScreenProperty screenProperty;
    screenProperty.SetStartX(1000);
    screenProperty.SetStartY(1000);
    screenProperty.SetBounds({{0, 0, 1000, 1000}, 10.0f, 10.0f});
    screenProperty.SetScreenType(ScreenType::REAL);
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(screenId, screenProperty, screenId);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(screenId, screenSession);
    sceneSession->displayIdSetDuringMoveTo_.insert(screenId);
    int32_t curCloneNodeCount = sceneSession->cloneNodeCount_;
    sceneSession->SetFindScenePanelRsNodeByZOrderFunc([](uint64_t screenId, uint32_t targetZOrder) {
        return nullptr;
    });
    sceneSession->RemoveSurfaceNodeFromScreen();
    EXPECT_EQ(sceneSession->cloneNodeCount_, curCloneNodeCount);
    // Register Lookup Node Function
    sceneSession->SetFindScenePanelRsNodeByZOrderFunc([this](uint64_t screenId, uint32_t targetZOrder) {
        return CreateRSSurfaceNode();
    });
    sceneSession->RemoveSurfaceNodeFromScreen();
    EXPECT_EQ(sceneSession->cloneNodeCount_, curCloneNodeCount - 1);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
}

/**
 * @tc.name: SetSubWindowZLevel
 * @tc.desc: SetSubWindowZLevel
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, SetSubWindowZLevel, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSubWindowZLevel";
    info.bundleName_ = "SetSubWindowZLevel";
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);
    int32_t testZLevel = 0;
    subSession->onSubSessionZLevelChange_ = [&testZLevel](int32_t zLevel) { testZLevel = zLevel; };
    subSession->property_->zLevel_ = 0;
    WSError ret = subSession->SetSubWindowZLevel(1);
    EXPECT_EQ(1, subSession->property_->zLevel_);
    EXPECT_EQ(1, testZLevel);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: GetSubWindowZLevel
 * @tc.desc: GetSubWindowZLevel
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, GetSubWindowZLevel, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetSubWindowZLevel";
    info.bundleName_ = "GetSubWindowZLevel";
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);
    subSession->property_->zLevel_ = 1;
    EXPECT_EQ(1, subSession->GetSubWindowZLevel());
}

/**
 * @tc.name: ProcessPointDownSession01
 * @tc.desc: ProcessPointDownSession with non-modal window and RaiseEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, ProcessPointDownSession01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ProcessPointDownSession01";
    info.bundleName_ = "ProcessPointDownSession01";
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);

    subSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subSession->GetSessionProperty()->SetRaiseEnabled(true);
    subSession->SetSessionState(SessionState::STATE_FOREGROUND);

    auto ret = subSession->ProcessPointDownSession(100, 200);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ProcessPointDownSession02
 * @tc.desc: ProcessPointDownSession with modal window and RaiseEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, ProcessPointDownSession02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ProcessPointDownSession02";
    info.bundleName_ = "ProcessPointDownSession02";
    sptr<SubSession> subSession = sptr<SubSession>::MakeSptr(info, nullptr);
    ASSERT_NE(subSession, nullptr);

    subSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subSession->GetSessionProperty()->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    subSession->GetSessionProperty()->SetRaiseEnabled(true);

    auto ret = subSession->ProcessPointDownSession(100, 200);
    EXPECT_EQ(WSError::WS_OK, ret);

    SessionInfo mainInfo;
    mainInfo.abilityName_ = "ProcessPointDownSession02_main";
    mainInfo.bundleName_ = "ProcessPointDownSession02_main";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(mainInfo, nullptr);
    subSession->SetParentSession(mainSession);

    ret = subSession->ProcessPointDownSession(100, 200);
    EXPECT_EQ(WSError::WS_OK, ret);
}
} // namespace
} // namespace Rosen
} // namespace OHOS