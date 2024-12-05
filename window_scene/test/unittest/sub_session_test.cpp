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
#include "session/host/include/session.h"
#include "session/host/include/main_session.h"
#include "session/host/include/system_session.h"
#include <ui/rs_surface_node.h>
#include "window_event_channel_base.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "window_property.h"
#include "window_session_property.h"
#include "mock_sub_session.h"

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

void SubSessionTest::SetUpTestCase()
{
}

void SubSessionTest::TearDownTestCase()
{
}

void SubSessionTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testMainSession1";
    info.moduleName_ = "testMainSession2";
    info.bundleName_ = "testMainSession3";
    subSession_ = new SubSession(info, specificCallback);
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
HWTEST_F(SubSessionTest, TransferKeyEvent01, Function | SmallTest | Level1)
{
    subSession_->state_ = SessionState::STATE_END;

    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, subSession_->TransferKeyEvent(nullptr));
}

/**
 * @tc.name: TransferKeyEvent02
 * @tc.desc: check func TransferKeyEvent
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, TransferKeyEvent02, Function | SmallTest | Level1)
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
HWTEST_F(SubSessionTest, TransferKeyEvent03, Function | SmallTest | Level1)
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
HWTEST_F(SubSessionTest, TransferKeyEvent04, Function | SmallTest | Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "TransferKeyEvent04";
    sessionInfo.moduleName_ = "TransferKeyEvent04";
    sessionInfo.bundleName_ = "TransferKeyEvent04";
    sptr<SubSession> session = new SubSession(sessionInfo, specificCallback);
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
HWTEST_F(SubSessionTest, IsTopmost01, Function | SmallTest | Level1)
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
HWTEST_F(SubSessionTest, IsTopmost02, Function | SmallTest | Level1)
{
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
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
HWTEST_F(SubSessionTest, CheckPointerEventDispatch01, Function | SmallTest | Level1)
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
HWTEST_F(SubSessionTest, CheckPointerEventDispatch02, Function | SmallTest | Level1)
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
HWTEST_F(SubSessionTest, CheckPointerEventDispatch03, Function | SmallTest | Level1)
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
HWTEST_F(SubSessionTest, CheckPointerEventDispatch04, Function | SmallTest | Level1)
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
HWTEST_F(SubSessionTest, CheckPointerEventDispatch05, Function | SmallTest | Level1)
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
HWTEST_F(SubSessionTest, IsModal, Function | SmallTest | Level1)
{
    ASSERT_FALSE(subSession_->IsModal());

    subSession_->SetSessionProperty(nullptr);
    ASSERT_TRUE(subSession_->GetSessionProperty() == nullptr);

    ASSERT_FALSE(subSession_->IsModal());
}

/**
 * @tc.name: IsVisibleForeground01
 * @tc.desc: check func IsVisibleForeground
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, IsVisibleForeground01, Function | SmallTest | Level1)
{
    ASSERT_FALSE(subSession_->IsVisibleForeground());

    SessionInfo info;
    info.abilityName_ = "testMainSession1";
    info.moduleName_ = "testMainSession2";
    info.bundleName_ = "testMainSession3";
    auto parentSession = new SubSession(info, specificCallback);

    subSession_->SetParentSession(parentSession);
    ASSERT_FALSE(subSession_->IsVisibleForeground());
}

/**
 * @tc.name: RectCheck
 * @tc.desc: test function : RectCheck
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, RectCheck, Function | SmallTest | Level1)
{
    ASSERT_NE(subSession_, nullptr);
    SessionInfo info;
    info.abilityName_ = "testRectCheck";
    info.moduleName_ = "testRectCheck";
    info.bundleName_ = "testRectCheck";
    sptr<Session> session = new (std::nothrow) Session(info);
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
HWTEST_F(SubSessionTest, IsModal01, Function | SmallTest | Level2)
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
HWTEST_F(SubSessionTest, IsApplicationModal, Function | SmallTest | Level2)
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
 * @tc.name: CheckDisplayAndMove
 * @tc.desc: CheckDisplayAndMove function01
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, CheckDisplayAndMove01, Function | SmallTest | Level2)
{
    SessionInfo info;
    sptr<Session> mainSession = sptr<Session>::MakeSptr(info);
    sptr<SubSession::SpecificSessionCallback> specificCallback = sptr<SubSession::SpecificSessionCallback>::MakeSptr();
    sptr<SubSessionMocker> subSession = sptr<SubSessionMocker>::MakeSptr(info, specificCallback);
    // if main window not exist
    EXPECT_CALL(*subSession, SetScreenId(_)).Times(0);
    subSession->CheckParentDisplayIdAndMove();
    // if main window and sub window has same display id
    subSession->parentSession_ = mainSession;
    DisplayId mainDisplayId = 123;
    mainSession->GetSessionProperty()->SetDisplayId(mainDisplayId);
    subSession->GetSessionProperty()->SetDisplayId(mainDisplayId);
    EXPECT_CALL(*subSession, SetScreenId(_)).Times(0);
    subSession->CheckParentDisplayIdAndMove();
    // if main window and sub window has different display id
    DisplayId subDisplayId = 234;
    subSession->GetSessionProperty()->SetDisplayId(subDisplayId);
    EXPECT_CALL(*subSession, SetScreenId(mainDisplayId)).Times(1);
    subSession->CheckParentDisplayIdAndMove();
    ASSERT_EQ(subSession->property_->displayId_, mainDisplayId);
}

/**
 * @tc.name: NotifySessionRectChange
 * @tc.desc: NotifySessionRectChange function01
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, NotifySessionRectChange01, Function | SmallTest | Level2)
{
    subSession_->shouldFollowParentWhenShow_.store(true);
    WSRect rect;
    subSession_->NotifySessionRectChange(rect, SizeChangeReason::UNDEFINED, DISPLAY_ID_INVALID);
    ASSERT_EQ(subSession_->shouldFollowParentWhenShow_.load(), true);
    subSession_->NotifySessionRectChange(rect, SizeChangeReason::DRAG_END, DISPLAY_ID_INVALID);
    ASSERT_EQ(subSession_->shouldFollowParentWhenShow_.load(), false);
}

/**
 * @tc.name: UpdateSessionRectInner
 * @tc.desc: UpdateSessionRectInner function01
 * @tc.type: FUNC
 */
HWTEST_F(SubSessionTest, UpdateSessionRectInner01, Function | SmallTest | Level2)
{
    subSession_->shouldFollowParentWhenShow_.store(true);
    WSRect rect;
    MoveConfiguration config;
    config.displayId = DISPLAY_ID_INVALID;
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::UNDEFINED, config);
    ASSERT_EQ(subSession_->shouldFollowParentWhenShow_.load(), true);
    config.displayId = 123;
    subSession_->UpdateSessionRectInner(rect, SizeChangeReason::DRAG_END, config);
    ASSERT_EQ(subSession_->shouldFollowParentWhenShow_.load(), false);
}
}
}
}