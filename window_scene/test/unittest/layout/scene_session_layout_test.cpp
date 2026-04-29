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
#include <ui/rs_surface_node.h>

#include "display_manager.h"
#include "input_event.h"
#include "key_event.h"
#include "mock/mock_scene_session.h"
#include "mock/mock_session_stage.h"
#include "mock_vsync_station.h"
#include "pointer_event.h"
#include "session/host/include/main_session.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/sub_session.h"
#include "session/host/include/system_session.h"
#include "session/screen/include/screen_session.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "window_helper.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class SceneSessionLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    sptr<SessionStageMocker> mockSessionStage_ = nullptr;
    static constexpr uint32_t WAIT_SYNC_NS = 200000;
};

void SceneSessionLayoutTest::SetUpTestCase() {}

void SceneSessionLayoutTest::TearDownTestCase() {}

void SceneSessionLayoutTest::SetUp()
{
    mockSessionStage_ = sptr<SessionStageMocker>::MakeSptr();
}

void SceneSessionLayoutTest::TearDown() {}

RSSurfaceNode::SharedPtr SceneSessionLayoutTest::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    return surfaceNode;
}

// Helper: Create a main session with attach info
sptr<MainSession> CreateMainSessionWithAttach(const std::string& name, sptr<SessionStageMocker>& mockStage)
{
    SessionInfo info;
    info.abilityName_ = name;
    info.bundleName_ = name;
    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    session->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    mockStage = sptr<SessionStageMocker>::MakeSptr();
    session->sessionStage_ = mockStage;

    // Main session does not need windowAnchorInfo_ to be set
    // It only checks children's windowAnchorInfo_ when notifying attached windows

    return session;
}

// Helper: Create a child session with attach info
sptr<SubSession> CreateChildSessionWithAttach(const std::string& name,
    bool isHeightLimit = true, bool isWidthLimit = true)
{
    SessionInfo info;
    info.abilityName_ = name;
    info.bundleName_ = name;
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    sptr<SubSession> session = sptr<SubSession>::MakeSptr(info, nullptr);

    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = isHeightLimit;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = isWidthLimit;

    return session;
}

// Helper: Set up parent-child relationship
void SetupParentChild(sptr<MainSession> parent, sptr<SubSession> child)
{
    child->parentSession_ = parent;
    parent->subSession_.push_back(child);
}

namespace {
/**
 * @tc.name: UpdateRect01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, UpdateRect01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateRect01";
    info.bundleName_ = "UpdateRect01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);

    sceneSession->SetSessionProperty(property);
    WSRect rect({ 1, 1, 1, 1 });
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    WSError result = sceneSession->UpdateRect(rect, reason, "SceneSessionLayoutTest");
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdateRect02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, UpdateRect02, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "UpdateRect02";
    info.bundleName_ = "UpdateRect02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);

    sceneSession->SetSessionProperty(property);
    WSRect rect({ 1, 1, 1, 1 });
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    WSError result = sceneSession->UpdateRect(rect, reason, "SceneSessionLayoutTest");
    EXPECT_EQ(result, WSError::WS_OK);

    sceneSession->GetLayoutController()->SetSessionRect(rect);
    result = sceneSession->UpdateRect(rect, reason, "SceneSessionLayoutTest");
    EXPECT_EQ(result, WSError::WS_OK);

    sceneSession->Session::UpdateSizeChangeReason(SizeChangeReason::DRAG_END);
    result = sceneSession->UpdateRect(rect, reason, "SceneSessionLayoutTest");
    EXPECT_EQ(result, WSError::WS_OK);

    WSRect rect2({ 0, 0, 0, 0 });
    result = sceneSession->UpdateRect(rect2, reason, "SceneSessionLayoutTest");
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdateRect03
 * @tc.desc: UpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, UpdateRect03, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "UpdateRect03";
    info.bundleName_ = "UpdateRect03";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;

    WSRect rect = { 200, 200, 200, 200 };
    session->GetLayoutController()->SetSessionRect(rect);
    session->SetClientRect(rect);
    EXPECT_EQ(session->UpdateRect(rect, reason, "SceneSessionLayoutTest"), WSError::WS_OK);

    rect.posX_ = 100;
    rect.posY_ = 100;
    rect.width_ = 800;
    rect.height_ = 800;
    session->GetLayoutController()->SetSessionRect(rect);
    EXPECT_EQ(session->UpdateRect(rect, reason, "SceneSessionLayoutTest"), WSError::WS_OK);
}

/**
 * @tc.name: UpdateRect04
 * @tc.desc: UpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, UpdateRect04, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "UpdateRect04";
    info.bundleName_ = "UpdateRect04";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<SceneSessionMocker> sceneSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    sceneSession->handler_ = nullptr;
    SizeChangeReason reason = SizeChangeReason::RESIZE;

    WSRect sessionRect = { 200, 200, 200, 200 };
    WSRect requestRect = { 100, 100, 800, 800 };
    sceneSession->GetLayoutController()->SetSessionRect(sessionRect);
    sceneSession->SetClientRect(sessionRect);
    auto mockRsTx = std::shared_ptr<RSTransaction>(nullptr);
    // NotifyClientToUpdateRect is expected to be called once
    EXPECT_CALL(*sceneSession, NotifyClientToUpdateRect("SceneSessionLayoutTest", _, mockRsTx))
        .Times(1);
    EXPECT_EQ(sceneSession->UpdateRect(requestRect, reason, "SceneSessionLayoutTest", mockRsTx), WSError::WS_OK);
}

/**
 * @tc.name: UpdateRect05
 * @tc.desc: UpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, UpdateRect05, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "UpdateRect05";
    info.bundleName_ = "UpdateRect05";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<SceneSessionMocker> sceneSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    sceneSession->handler_ = nullptr;
    SizeChangeReason reason = SizeChangeReason::DRAG_END;

    WSRect sessionRect = { 200, 200, 200, 200 };
    WSRect requestRect = { 200, 200, 200, 200 };
    sceneSession->GetLayoutController()->SetSessionRect(sessionRect);
    sceneSession->SetClientRect(sessionRect);
    auto mockRsTx = std::shared_ptr<RSTransaction>(nullptr);
    // NotifyClientToUpdateRect is expected to be called 0 times
    EXPECT_CALL(*sceneSession, NotifyClientToUpdateRect("SceneSessionLayoutTest", _, mockRsTx))
        .Times(0);
    EXPECT_EQ(sceneSession->UpdateRect(requestRect, reason, "SceneSessionLayoutTest", mockRsTx), WSError::WS_OK);
}

/**
 * @tc.name: NotifyClientToUpdateRect01
 * @tc.desc: NotifyClientToUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyClientToUpdateRect01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "NotifyClientToUpdateRect01";
    info.bundleName_ = "NotifyClientToUpdateRect01";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    sceneSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    sceneSession->sessionStage_ = mockSessionStage;
    auto ret = sceneSession->NotifyClientToUpdateRect("SceneSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: UpdateRectInner
 * @tc.desc: UpdateRectInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, UpdateRectInner01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "UpdateRectInner01";
    info.bundleName_ = "UpdateRectInner01";
    info.screenId_ = 20;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    SessionUIParam uiParam;
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    sceneSession->SetForegroundInteractiveStatus(true);

    uiParam.needSync_ = true;
    uiParam.rect_ = { 0, 0, 1, 1 };

    sceneSession->GetLayoutController()->SetSessionRect({ 1, 1, 1, 1 });
    sceneSession->isVisible_ = true;
    ASSERT_EQ(false, sceneSession->UpdateRectInner(uiParam, reason));
}

/**
 * @tc.name: UpdateRectInner02
 * @tc.desc: UpdateRectInner02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, UpdateRectInner02, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "UpdateRectInner02";
    info.bundleName_ = "UpdateRectInner02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    SessionUIParam uiParam;
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    sceneSession->SetForegroundInteractiveStatus(true);

    sceneSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    sceneSession->isSubWindowResizingOrMoving_ = true;
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_EQ(true, sceneSession->UpdateRectInner(uiParam, reason));
    ASSERT_EQ(true, sceneSession->isSubWindowResizingOrMoving_);

    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ASSERT_EQ(true, sceneSession->UpdateRectInner(uiParam, reason));
    ASSERT_EQ(false, sceneSession->isSubWindowResizingOrMoving_);
}

/**
 * @tc.name: UpdateRectInner03
 * @tc.desc: UpdateRectInner03
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, UpdateRectInner03, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "UpdateRectInner03";
    info.bundleName_ = "UpdateRectInner03";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    SessionUIParam uiParam;
    // test drag move reason return false
    SizeChangeReason reason = SizeChangeReason::DRAG_MOVE;
    EXPECT_EQ(false, sceneSession->UpdateRectInner(uiParam, reason));
    // test other reason return false
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    reason = SizeChangeReason::UNDEFINED;
    sceneSession->SetForegroundInteractiveStatus(true);
    sceneSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    sceneSession->isSubWindowResizingOrMoving_ = true;
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    EXPECT_EQ(true, sceneSession->UpdateRectInner(uiParam, reason));
}

/**
 * @tc.name: NotifyClientToUpdateRect
 * @tc.desc: NotifyClientToUpdateRect function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyClientToUpdateRect, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyClientToUpdateRect";
    info.bundleName_ = "NotifyClientToUpdateRect";
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->moveDragController_ = nullptr;
    session->isKeyboardPanelEnabled_ = false;
    session->Session::UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    session->Session::SetSessionState(SessionState::STATE_CONNECT);
    session->specificCallback_ = nullptr;
    session->Session::UpdateSizeChangeReason(SizeChangeReason::DRAG);
    EXPECT_EQ(WSError::WS_OK, session->NotifyClientToUpdateRect("SceneSessionLayoutTest", std::nullopt, nullptr));

    UpdateAvoidAreaCallback func = [](const int32_t& persistentId) { return; };
    auto specificCallback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCallback->onUpdateAvoidArea_ = func;
    session->specificCallback_ = specificCallback;
    session->Session::UpdateSizeChangeReason(SizeChangeReason::RECOVER);
    EXPECT_EQ(WSError::WS_OK, session->NotifyClientToUpdateRect("SceneSessionLayoutTest", std::nullopt, nullptr));
}

/**
 * @tc.name: NotifyClientToUpdateRectTask
 * @tc.desc: NotifyClientToUpdateRectTask function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyClientToUpdateRectTask, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "NotifyClientToUpdateRectTask";
    info.bundleName_ = "NotifyClientToUpdateRectTask";
    info.isSystem_ = true;

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->moveDragController_ = nullptr;
    session->isKeyboardPanelEnabled_ = false;
    session->SetSessionRect({ 0, 0, 800, 800 });

    session->Session::UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION,
              session->NotifyClientToUpdateRectTask("SceneSessionLayoutTest", std::nullopt, nullptr));
    session->Session::UpdateSizeChangeReason(SizeChangeReason::MOVE);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION,
              session->NotifyClientToUpdateRectTask("SceneSessionLayoutTest", std::nullopt, nullptr));
    session->Session::UpdateSizeChangeReason(SizeChangeReason::DRAG_MOVE);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION,
              session->NotifyClientToUpdateRectTask("SceneSessionLayoutTest", std::nullopt, nullptr));
    session->Session::UpdateSizeChangeReason(SizeChangeReason::RESIZE);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION,
              session->NotifyClientToUpdateRectTask("SceneSessionLayoutTest", std::nullopt, nullptr));
    session->Session::UpdateSizeChangeReason(SizeChangeReason::RECOVER);
    EXPECT_EQ(session->GetSizeChangeReason(), SizeChangeReason::RECOVER);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION,
              session->NotifyClientToUpdateRectTask("SceneSessionLayoutTest", std::nullopt, nullptr));

    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));
    session->moveDragController_->isStartDrag_ = true;
    session->moveDragController_->isStartMove_ = true;
    session->Session::UpdateSizeChangeReason(SizeChangeReason::MOVE);
    session->isKeyboardPanelEnabled_ = true;
    info.windowType_ = static_cast<uint32_t>(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION,
              session->NotifyClientToUpdateRectTask("SceneSessionLayoutTest", std::nullopt, nullptr));
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION,
              session->NotifyClientToUpdateRectTask("SceneSessionLayoutTest", std::nullopt, nullptr));

    session->Session::UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    EXPECT_EQ(WSError::WS_DO_NOTHING,
              session->NotifyClientToUpdateRectTask("SceneSessionLayoutTest", std::nullopt, nullptr));

    session->Session::UpdateSizeChangeReason(SizeChangeReason::MOVE);
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION,
              session->NotifyClientToUpdateRectTask("SceneSessionLayoutTest", std::nullopt, nullptr));
    session->Session::UpdateSizeChangeReason(SizeChangeReason::DRAG_MOVE);
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION,
              session->NotifyClientToUpdateRectTask("SceneSessionLayoutTest", std::nullopt, nullptr));
}

/**
 * @tc.name: NotifyGlobalScaledRectChange01
 * @tc.desc: test NotifyGlobalScaledRectChange is called or not
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyGlobalScaledRectChange01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyGlobalScaledRectChange01";
    info.bundleName_ = "NotifyGlobalScaledRectChange01";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto sessionStageMocker = sptr<SessionStageMocker>::MakeSptr();

    // Case1: session is not foreground
    session->Session::SetSessionState(SessionState::STATE_BACKGROUND);
    session->NotifyGlobalScaledRectChange();
    EXPECT_CALL(*sessionStageMocker, NotifyGlobalScaledRectChange(_)).Times(0);

    // Case2: session is foreground but sessionStage is null
    session->Session::SetSessionState(SessionState::STATE_FOREGROUND);
    session->sessionStage_ = nullptr;
    session->NotifyGlobalScaledRectChange();
    EXPECT_CALL(*sessionStageMocker, NotifyGlobalScaledRectChange(_)).Times(0);

    // Case3: session is foreground and sessionStage is not null
    session->sessionStage_ = sessionStageMocker;
    session->NotifyGlobalScaledRectChange();
    EXPECT_CALL(*sessionStageMocker, NotifyGlobalScaledRectChange(_)).Times(1);
}

/**
 * @tc.name: UpdateUIParam01
 * @tc.desc: UpdateUIParam test NotifyGlobalScaledRectChange is called or not
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, UpdateUIParam01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateUIParam01";
    info.bundleName_ = "UpdateUIParam01";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->Session::SetSessionState(SessionState::STATE_FOREGROUND);
    auto sessionStageMocker = sptr<SessionStageMocker>::MakeSptr();
    session->sessionStage_ = sessionStageMocker;
    WSRect oldRect = { 0, 0, 1000, 1000 };
    WSRect newRect = { 100, 100, 1200, 1200 };
    float newFloat = 0.9f;
    SessionUIParam uiParam;
    session->SetSessionGlobalRect(oldRect);

    // Case1: global rect is dirty
    uiParam.rect_ = newRect;
    session->UpdateUIParam(uiParam);
    EXPECT_CALL(*sessionStageMocker, NotifyGlobalScaledRectChange(_)).Times(1);
    session->dirtyFlags_ = 0;

    // Case2: scale is dirty
    uiParam.scaleX_ = newFloat;
    uiParam.scaleY_ = newFloat;
    uiParam.pivotX_ = newFloat;
    uiParam.pivotY_ = newFloat;
    session->UpdateUIParam(uiParam);
    EXPECT_CALL(*sessionStageMocker, NotifyGlobalScaledRectChange(_)).Times(1);
    session->dirtyFlags_ = 0;

    // Case3: global rect and scale are not dirty
    session->UpdateUIParam(uiParam);
    EXPECT_CALL(*sessionStageMocker, NotifyGlobalScaledRectChange(_)).Times(0);
}

/**
 * @tc.name: HandleActionUpdateWindowLimits
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, HandleActionUpdateWindowLimits, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS;
    WMError res = sceneSession->HandleActionUpdateWindowLimits(property, action);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: HandleActionUpdateWindowLimits2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, HandleActionUpdateWindowLimits2, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WindowLimits windowLimitsPX = { 2000, 2000, 2000, 2000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    property->SetWindowLimits(windowLimitsPX);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS;
    WMError res = sceneSession->HandleActionUpdateWindowLimits(property, action);
    EXPECT_EQ(WMError::WM_OK, res);

    WindowLimits windowLimitsVP = { 1000, 1000, 1000, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    property->SetWindowLimitsVP(windowLimitsVP);
    res = sceneSession->HandleActionUpdateWindowLimits(property, action);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: NotifySessionWindowLimitschange
 * @tc.desc: NotifySessionWindowLimitschange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifySessionWindowLimitschange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSessionRectChangeCallback";
    info.bundleName_ = "SetSessionRectChangeCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    WindowLimits expectedMainLimits = { 2000, 2000, 100, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    bool callbackTriggered = false;
    WindowLimits receivedMainLimits;

    // Case 1: sessionWindowLimitsChangeFunc_ is null
    sceneSession->NotifySessionWindowLimitsChange(expectedMainLimits);
    EXPECT_NE(receivedMainLimits.minWidth_, expectedMainLimits.minWidth_);
    EXPECT_NE(receivedMainLimits.minHeight_, expectedMainLimits.minHeight_);
    EXPECT_NE(receivedMainLimits.maxWidth_, expectedMainLimits.maxWidth_);
    EXPECT_NE(receivedMainLimits.maxHeight_, expectedMainLimits.maxHeight_);
    EXPECT_NE(receivedMainLimits.pixelUnit_, expectedMainLimits.pixelUnit_);

    // Case 2: sessionWindowLimitsChangeFunc_ is not null
    sceneSession->SetSessionWindowLimitsChangeCallback(
        [&](const WindowLimits& mainLimits) {
            callbackTriggered = true;
            receivedMainLimits = mainLimits;
        }
    );
    sceneSession->NotifySessionWindowLimitsChange(expectedMainLimits);
    EXPECT_TRUE(callbackTriggered);
    EXPECT_EQ(receivedMainLimits.minWidth_, expectedMainLimits.minWidth_);
    EXPECT_EQ(receivedMainLimits.minHeight_, expectedMainLimits.minHeight_);
    EXPECT_EQ(receivedMainLimits.maxWidth_, expectedMainLimits.maxWidth_);
    EXPECT_EQ(receivedMainLimits.maxHeight_, expectedMainLimits.maxHeight_);
    EXPECT_EQ(receivedMainLimits.pixelUnit_, expectedMainLimits.pixelUnit_);
}

/**
 * @tc.name: SetAspectRatio2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SetAspectRatio2, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "SetAspectRatio2";
    info.bundleName_ = "SetAspectRatio2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->isActive_ = true;

    float ratio = 0.0001;
    sceneSession->moveDragController_ = nullptr;
    auto result = sceneSession->SetAspectRatio(ratio);
    EXPECT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(sceneSession->GetAspectRatio(), ratio);

    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    result = sceneSession->SetAspectRatio(ratio);
    EXPECT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(sceneSession->GetAspectRatio(), ratio);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionProperty(property);
    result = sceneSession->SetAspectRatio(ratio);
    EXPECT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(sceneSession->GetAspectRatio(), ratio);
}

/**
 * @tc.name: SetAspectRatio3
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SetAspectRatio3, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAspectRatio3";
    info.bundleName_ = "SetAspectRatio3";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->isActive_ = true;

    float ratio = 0.1;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionProperty(property);
    WindowLimits limits;
    limits.maxHeight_ = 10;
    limits.minWidth_ = 1;
    limits.minHeight_ = 1;
    property->SetWindowLimits(limits);
    auto result = sceneSession->SetAspectRatio(ratio);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SetAspectRatio4
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SetAspectRatio4, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAspectRatio4";
    info.bundleName_ = "SetAspectRatio4";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->isActive_ = true;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionProperty(property);

    float ratio = 0.1;
    WindowLimits limits;
    limits.maxHeight_ = 10;
    limits.minWidth_ = 10;
    property->SetWindowLimits(limits);
    auto result = sceneSession->SetAspectRatio(ratio);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SetAspectRatio5
 * @tc.desc: test for aspectRatio NearZero
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SetAspectRatio5, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "SetAspectRatio5";
    info.bundleName_ = "SetAspectRatio5";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->isActive_ = true;

    float ratio = 0.0001;
    auto result = sceneSession->SetAspectRatio(ratio);
    EXPECT_EQ(result, WSError::WS_OK);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionProperty(property);
    result = sceneSession->SetAspectRatio(ratio);
    EXPECT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(sceneSession->GetAspectRatio(), ratio);
}

/**
 * @tc.name: SetAspectRatio6
 * @tc.desc: test for aspectRatio is smaller than minWidth/maxHeight
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SetAspectRatio6, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "SetAspectRatio6";
    info.bundleName_ = "SetAspectRatio6";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->isActive_ = true;

    float ratio = 2.5;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    WindowLimits limits;
    limits.maxWidth_ = 3000;
    limits.maxHeight_ = 3000;
    limits.minWidth_ = 2000;
    limits.minHeight_ = 2000;
    property->SetWindowLimits(limits);
    sceneSession->SetSessionProperty(property);
    auto result = sceneSession->SetAspectRatio(ratio);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SetAspectRatio7
 * @tc.desc: test for aspectRatio is smaller than minWidth/maxHeight
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SetAspectRatio7, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "SetAspectRatio7";
    info.bundleName_ = "SetAspectRatio7";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->isActive_ = true;

    float ratio = 0.1;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    WindowLimits limits;
    limits.maxWidth_ = 3000;
    limits.maxHeight_ = 3000;
    limits.minWidth_ = 2000;
    limits.minHeight_ = 2000;
    property->SetWindowLimits(limits);
    sceneSession->SetSessionProperty(property);
    auto result = sceneSession->SetAspectRatio(ratio);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: SaveAspectRatio
 * @tc.desc: SaveAspectRatio
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SaveAspectRatio, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_EQ(true, sceneSession->SaveAspectRatio(0.1));

    sceneSession->sessionInfo_.bundleName_ = "";
    sceneSession->sessionInfo_.moduleName_ = "";
    sceneSession->sessionInfo_.abilityName_ = "";
    ASSERT_EQ(false, sceneSession->SaveAspectRatio(0.1));
}

/**
 * To test the function call
 *
 * @tc.name: ActivateDragBySystem
 * @tc.desc: ActivateDragBySystem function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ActivateDragBySystem, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ActivateDragBySystem";
    info.bundleName_ = "ActivateDragBySystem";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto ret = sceneSession->ActivateDragBySystem(true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * To test the drag activated settings, and validate the draggable results.
 * Expect the results:
 * enableDrag: true, dragActivated: true => true
 * enableDrag: false, dragActivated: true => false
 * enableDrag: true, dragActivated: false => false
 * enableDrag: false, dragActivated: false => false
 *
 * @tc.name: CheckDragActivatedSettings
 * @tc.desc: CheckDragActivatedSettings
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, CheckDragActivatedSettings, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CheckDragActivatedSettings";
    info.bundleName_ = "CheckDragActivatedSettings";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->ActivateDragBySystem(true);
    sceneSession->GetSessionProperty()->SetDragEnabled(true);
    ASSERT_EQ(true, sceneSession->IsDragAccessible());

    sceneSession->GetSessionProperty()->SetDragEnabled(false);
    ASSERT_EQ(false, sceneSession->IsDragAccessible());

    sceneSession->ActivateDragBySystem(false);
    sceneSession->GetSessionProperty()->SetDragEnabled(true);
    ASSERT_EQ(false, sceneSession->IsDragAccessible());

    sceneSession->GetSessionProperty()->SetDragEnabled(false);
    ASSERT_EQ(false, sceneSession->IsDragAccessible());
}

/**
 * @tc.name: NotifySingleHandTransformChange
 * @tc.desc: NotifySingleHandTransformChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifySingleHandTransformChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySingleHandTransformChange";
    info.bundleName_ = "NotifySingleHandTransformChange";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    SingleHandTransform testTransform;
    sceneSession->state_ = SessionState::STATE_BACKGROUND;
    sceneSession->NotifySingleHandTransformChange(testTransform);

    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    mockSessionStage_ = nullptr;
    sceneSession->sessionStage_ = mockSessionStage_;
    sceneSession->NotifySingleHandTransformChange(testTransform);

    mockSessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    sceneSession->sessionStage_ = mockSessionStage_;
    sceneSession->NotifySingleHandTransformChange(testTransform);
    ASSERT_NE(100, testTransform.posX);
}

/**
 * @tc.name: GetSessionGlobalRectWithSingleHandScale
 * @tc.desc: GetSessionGlobalRectWithSingleHandScale
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, GetSessionGlobalRectWithSingleHandScale, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetSessionGlobalRectWithSingleHandScale";
    info.bundleName_ = "GetSessionGlobalRectWithSingleHandScale";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    SingleHandTransform testTransform;
    WSRect globalRect = { 100, 100, 800, 800 };
    sceneSession->SetSessionGlobalRect(globalRect);
    sceneSession->SetSingleHandTransform(testTransform);
    ASSERT_EQ(sceneSession->GetSessionGlobalRect().posX_,
              sceneSession->GetSessionGlobalRectWithSingleHandScale().posX_);

    testTransform.posX = 10;
    testTransform.posY = 10;
    testTransform.scaleX = 0.75f;
    testTransform.scaleY = 0.75f;
    sceneSession->SetSingleHandTransform(testTransform);
    ASSERT_NE(sceneSession->GetSessionGlobalRect().posX_,
              sceneSession->GetSessionGlobalRectWithSingleHandScale().posX_);
}

/**
 * @tc.name: IsNeedConvertToRelativeRect
 * @tc.desc: IsNeedConvertToRelativeRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, IsNeedConvertToRelativeRect, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsNeedConvertToRelativeRect";
    info.bundleName_ = "IsNeedConvertToRelativeRect";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->moveDragController_ =
        sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    info.abilityName_ = "IsNeedConvertToRelativeRect_subSession";
    info.bundleName_ = "IsNeedConvertToRelativeRect_subSession";
    sptr<SceneSession> subSceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->subSession_.emplace_back(subSceneSession);
    subSceneSession->parentSession_ = sceneSession;

    EXPECT_EQ(sceneSession->IsNeedConvertToRelativeRect(SizeChangeReason::MAXIMIZE), false);
    EXPECT_EQ(sceneSession->IsNeedConvertToRelativeRect(SizeChangeReason::DRAG_MOVE), true);

    sceneSession->moveDragController_->isStartMove_ = true;
    sceneSession->moveDragController_->isStartDrag_ = false;
    EXPECT_EQ(subSceneSession->IsNeedConvertToRelativeRect(SizeChangeReason::MAXIMIZE), false);

    sceneSession->moveDragController_->isStartMove_ = false;
    sceneSession->moveDragController_->isStartDrag_ = true;
    EXPECT_EQ(subSceneSession->IsNeedConvertToRelativeRect(SizeChangeReason::MAXIMIZE), false);

    subSceneSession->windowAnchorInfo_.isAnchorEnabled_ = true;
    sceneSession->moveDragController_->isStartMove_ = true;
    sceneSession->moveDragController_->isStartDrag_ = false;
    EXPECT_EQ(subSceneSession->IsNeedConvertToRelativeRect(SizeChangeReason::MAXIMIZE), true);
}

/**
 * @tc.name: IsDraggable
 * @tc.desc: IsDraggable when moveDragController_ is nullptr or not.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, IsDraggable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsDraggable";
    info.bundleName_ = "IsDraggable";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sceneSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->GetSessionProperty()->SetDragEnabled(true);
    sceneSession->SetDragActivated(true);

    sceneSession->moveDragController_ = nullptr;
    EXPECT_EQ(sceneSession->IsDraggable(), false);

    sceneSession->moveDragController_ =
        sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    EXPECT_EQ(sceneSession->IsDraggable(), true);
}

/**
 * @tc.name: IsMovable
 * @tc.desc: Test IsMovable when AnchorEnabled or AnchorDisabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, IsMovable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsMovable";
    info.bundleName_ = "IsMovable";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    EXPECT_EQ(WSError::WS_OK, sceneSession->UpdateFocus(true));
    sceneSession->moveDragController_->isStartMove_ = false;
    sceneSession->moveDragController_->isStartDrag_ = false;
    sceneSession->moveDragController_->hasPointDown_ = true;
    sceneSession->moveDragController_->isMovable_ = true;

    sceneSession->windowAnchorInfo_.isAnchorEnabled_ = true;
    EXPECT_EQ(false, sceneSession->IsMovable());
    sceneSession->windowAnchorInfo_.isAnchorEnabled_ = false;
    EXPECT_EQ(true, sceneSession->IsMovable());
}

/**
 * @tc.name: IsAnyParentSessionDragMoving
 * @tc.desc: IsAnyParentSessionDragMoving
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, IsAnyParentSessionDragMoving, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsAnyParentSessionDragMoving";
    info.bundleName_ = "IsAnyParentSessionDragMoving";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->moveDragController_ =
        sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    info.abilityName_ = "IsAnyParentSessionDragMoving_subSession";
    info.bundleName_ = "IsAnyParentSessionDragMoving_subSession";
    sptr<SceneSession> subSceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    subSceneSession->parentSession_ = nullptr;
    ASSERT_EQ(subSceneSession->IsAnyParentSessionDragMoving(), false);
    sceneSession->moveDragController_->isStartMove_ = true;
    ASSERT_EQ(sceneSession->IsAnyParentSessionDragMoving(), true);

    sceneSession->subSession_.emplace_back(subSceneSession);
    subSceneSession->parentSession_ = sceneSession;
    ASSERT_EQ(subSceneSession->IsAnyParentSessionDragMoving(), true);
}

/**
 * @tc.name: IsAnyParentSessionDragZooming
 * @tc.desc: IsAnyParentSessionDragZooming
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, IsAnyParentSessionDragZooming, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsAnyParentSessionDragZooming";
    info.bundleName_ = "IsAnyParentSessionDragZooming";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->moveDragController_ =
        sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    info.abilityName_ = "IsAnyParentSessionDragZooming_subSession";
    info.bundleName_ = "IsAnyParentSessionDragZooming_subSession";
    sptr<SceneSession> subSceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    subSceneSession->parentSession_ = nullptr;
    ASSERT_EQ(subSceneSession->IsAnyParentSessionDragZooming(), false);
    sceneSession->moveDragController_->isStartDrag_ = true;
    ASSERT_EQ(sceneSession->IsAnyParentSessionDragZooming(), true);

    sceneSession->subSession_.emplace_back(subSceneSession);
    subSceneSession->parentSession_ = sceneSession;
    ASSERT_EQ(subSceneSession->IsAnyParentSessionDragZooming(), true);
}

/**
 * @tc.name: HandleSubSessionSurfaceNode
 * @tc.desc: HandleSubSessionSurfaceNode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, HandleSubSessionSurfaceNode, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleSubSessionSurfaceNode";
    info.bundleName_ = "HandleSubSessionSurfaceNode";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->moveDragController_ =
        sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    info.abilityName_ = "HandleSubSessionSurfaceNode_subSession";
    info.bundleName_ = "HandleSubSessionSurfaceNode_subSession";
    sptr<SubSession> subSceneSession = sptr<SubSession>::MakeSptr(info, nullptr);
    subSceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession->subSession_.emplace_back(subSceneSession);
    subSceneSession->parentSession_ = sceneSession;
    sceneSession->displayIdSetDuringMoveTo_.insert(888);
    subSceneSession->displayIdSetDuringMoveTo_.insert(888);

    subSceneSession->NotifyFollowParentMultiScreenPolicy(false);
    sceneSession->HandleSubSessionSurfaceNode(false, 0);
    ASSERT_NE(0, subSceneSession->displayIdSetDuringMoveTo_.size());
    subSceneSession->NotifyFollowParentMultiScreenPolicy(true);
    sceneSession->HandleSubSessionSurfaceNode(true, 0);
    ASSERT_NE(0, subSceneSession->displayIdSetDuringMoveTo_.size());
    sceneSession->HandleSubSessionSurfaceNode(false, 0);
    ASSERT_EQ(0, subSceneSession->displayIdSetDuringMoveTo_.size());
}

/**
 * @tc.name: HandleSubSessionCrossNode
 * @tc.desc: HandleSubSessionCrossNode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, HandleSubSessionCrossNode, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleSubSessionCrossNode";
    info.bundleName_ = "HandleSubSessionCrossNode";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    info.abilityName_ = "HandleSubSessionCrossNode_subSession";
    info.bundleName_ = "HandleSubSessionCrossNode_subSession";
    sptr<SceneSession> subSceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    sceneSession->SetDragStart(true);
    sceneSession->HandleSubSessionCrossNode(SizeChangeReason::UNDEFINED);
    ASSERT_EQ(sceneSession->IsDragStart(), true);
    sceneSession->HandleSubSessionCrossNode(SizeChangeReason::DRAG);
    ASSERT_EQ(sceneSession->IsDragStart(), true);
    sceneSession->HandleSubSessionCrossNode(SizeChangeReason::DRAG_END);
    ASSERT_EQ(sceneSession->IsDragStart(), false);

    sceneSession->HandleSubSessionCrossNode(SizeChangeReason::DRAG);
    ASSERT_EQ(sceneSession->IsDragStart(), true);
    sceneSession->subSession_.emplace_back(subSceneSession);
    subSceneSession->parentSession_ = sceneSession;
    sceneSession->HandleSubSessionCrossNode(SizeChangeReason::DRAG_END);
    ASSERT_EQ(sceneSession->IsDragStart(), false);
    sceneSession->HandleSubSessionCrossNode(SizeChangeReason::DRAG_MOVE);
    ASSERT_EQ(sceneSession->IsDragStart(), true);
}

/**
 * @tc.name: UpdateWindowModeForUITest01
 * @tc.desc: UpdateWindowModeForUITest
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, UpdateWindowModeForUITest01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateWindowModeForUITest";
    info.bundleName_ = "UpdateWindowModeForUITest";
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);

    const int32_t updateMode = 1;
    WMError errCode = sceneSession->UpdateWindowModeForUITest(updateMode);
    EXPECT_EQ(errCode, WMError::WM_ERROR_NULLPTR);

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    sceneSession->sessionStage_ = mockSessionStage;
    errCode = sceneSession->UpdateWindowModeForUITest(updateMode);
    EXPECT_EQ(errCode, WMError::WM_OK);
}

/**
 * @tc.name: NotifyWindowStatusDidChangeAfterShowWindow
 * @tc.desc: NotifyWindowStatusDidChangeAfterShowWindowForUITest
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyWindowStatusDidChangeAfterShowWindow, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyWindowStatusDidChangeAfterShowWindow";
    info.bundleName_ = "NotifyWindowStatusDidChangeAfterShowWindow";
    auto specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);

    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    EXPECT_CALL(*mockVsyncStation, RequestVsync(_))
        .WillRepeatedly(Invoke([](const std::shared_ptr<VsyncCallback>& cb) {
            ASSERT_NE(cb, nullptr);
            cb->onCallback(0, 0);
        }));
    sceneSession->SetVsyncStation(mockVsyncStation);
    ASSERT_NE(sceneSession->vsyncStation_, nullptr);

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    sceneSession->sessionStage_ = mockSessionStage;
    sceneSession->handler_ = nullptr;
    EXPECT_CALL(*mockSessionStage, NotifyLayoutFinishAfterWindowModeChange(_)).Times(1);
    sceneSession->NotifyWindowStatusDidChangeAfterShowWindow();
    sceneSession->ExecuteWindowStatusChangeNotification("test");
}

/**
 * @tc.name: NotifyNotifySubSessionParentSizeChange
 * @tc.desc: NotifyNotifySubSessionParentSizeChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifySubSessionParentSizeChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySubSessionParentSizeChange";
    info.bundleName_ = "NotifySubSessionParentSizeChange";
    Rect rect = {1, 2, 3, 4};
    auto specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();

    // subSession is null
    sceneSession->sessionStage_ = mockSessionStage;
    sptr<SceneSession> subSceneSession = nullptr;
    sceneSession->subSession_.push_back(subSceneSession);
    sceneSession->handler_ = nullptr;
    sceneSession->NotifySubSessionParentSizeChange(rect);
    EXPECT_CALL(*mockSessionStage, NotifySubWindowAfterParentWindowSizeChange(rect)).Times(0);

    // sessionStage_ is null
    subSceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    subSceneSession->sessionStage_ = nullptr;
    sceneSession->subSession_.push_back(subSceneSession);
    sceneSession->NotifySubSessionParentSizeChange(rect);
    EXPECT_CALL(*mockSessionStage, NotifySubWindowAfterParentWindowSizeChange(rect)).Times(0);

    // subSession->GetWindowAnchorInfo().isAnchoredByAttach is false
    sptr<SessionStageMocker> mockSubSessionStage = sptr<SessionStageMocker>::MakeSptr();
    subSceneSession->sessionStage_ = mockSubSessionStage;
    WindowAnchorInfo anchorInfo = {false, false, WindowAnchor::TOP_START, 0, 0};
    subSceneSession->windowAnchorInfo_ = anchorInfo;
    sceneSession->subSession_.push_back(subSceneSession);
    sceneSession->NotifySubSessionParentSizeChange(rect);
    EXPECT_CALL(*mockSubSessionStage, NotifySubWindowAfterParentWindowSizeChange(rect)).Times(0);

    // subSession->GetWindowAnchorInfo().isAnchoredByAttach is true
    anchorInfo = {true, true, WindowAnchor::TOP_START, 0, 0};
    subSceneSession->windowAnchorInfo_ = anchorInfo;
    sceneSession->subSession_.push_back(subSceneSession);
    sceneSession->NotifySubSessionParentSizeChange(rect);
    EXPECT_CALL(*mockSubSessionStage, NotifySubWindowAfterParentWindowSizeChange(rect)).Times(1);
}

/**
 * @tc.name: NotifyNotifySubSessionParentStatusChange
 * @tc.desc: NotifyNotifySubSessionParentSizeChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifySubSessionParentStatusChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySubSessionParentStatusChange";
    info.bundleName_ = "NotifySubSessionParentStatusChange";
    auto specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    MaximizeMode maximizeMode = MaximizeMode::MODE_AVOID_SYSTEM_BAR;

    WindowMode mode = WindowMode::WINDOW_MODE_FULLSCREEN;
    // subSession is null
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    sceneSession->sessionStage_ = mockSessionStage;
    sceneSession->handler_ = nullptr;
    EXPECT_CALL(*mockSessionStage, NotifySubWindowAfterParentWindowStatusChange(mode, maximizeMode, true)).Times(0);
    sceneSession->NotifySubSessionParentStatusChange(mode);

    // sessionStage_ is null
    sptr<SceneSession> subSceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    sceneSession->subSession_.push_back(subSceneSession);
    sceneSession->sessionStage_ = nullptr;
    EXPECT_CALL(*mockSessionStage, NotifySubWindowAfterParentWindowStatusChange(mode, maximizeMode, true)).Times(0);
    sceneSession->NotifySubSessionParentStatusChange(mode);

    // subSession->GetWindowAnchorInfo().isAnchoredByAttach is false
    sptr<SessionStageMocker> mockSubSessionStage = sptr<SessionStageMocker>::MakeSptr();
    subSceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    subSceneSession->sessionStage_ = mockSubSessionStage;
    WindowAnchorInfo anchorInfo = {false, false, WindowAnchor::TOP_START, 0, 0};
    subSceneSession->windowAnchorInfo_ = anchorInfo;
    sceneSession->subSession_.push_back(subSceneSession);
    EXPECT_CALL(*mockSessionStage, NotifySubWindowAfterParentWindowStatusChange(mode, maximizeMode, true)).Times(0);
    sceneSession->NotifySubSessionParentStatusChange(mode);

    subSceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    mockSubSessionStage = sptr<SessionStageMocker>::MakeSptr();
    subSceneSession->sessionStage_ = mockSubSessionStage;
    anchorInfo = {true, true, WindowAnchor::TOP_START, 0, 0};
    subSceneSession->windowAnchorInfo_ = anchorInfo;
    sceneSession->subSession_.push_back(subSceneSession);
    EXPECT_CALL(*mockSessionStage, NotifySubWindowAfterParentWindowStatusChange(mode, maximizeMode, true)).Times(1);
    sceneSession->NotifySubSessionParentStatusChange(mode);
}

/**
 * @tc.name: NotifyWindowStatusDidChangeIfNeedWhenSessionEvent
 * @tc.desc: NotifyWindowStatusDidChangeIfNeedWhenSessionEventForUITest
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyWindowStatusDidChangeIfNeedWhenSessionEvent, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyWindowStatusDidChangeIfNeedWhenSessionEvent";
    info.bundleName_ = "NotifyWindowStatusDidChangeIfNeedWhenSessionEvent";
    auto specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);

    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    EXPECT_CALL(*mockVsyncStation, RequestVsync(_))
        .WillRepeatedly(Invoke([](const std::shared_ptr<VsyncCallback>& cb) {
            ASSERT_NE(cb, nullptr);
            cb->onCallback(0, 0);
        }));
    sceneSession->SetVsyncStation(mockVsyncStation);

    sceneSession->RegisterGetIsDockAutoHideFunc([](){return true;});

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    sceneSession->sessionStage_ = mockSessionStage;
    sceneSession->handler_ = nullptr;
    sceneSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_CALL(*mockSessionStage, NotifyLayoutFinishAfterWindowModeChange(_)).Times(1);
    sceneSession->NotifyWindowStatusDidChangeIfNeedWhenSessionEvent(SessionEvent::EVENT_MAXIMIZE);
}

/**
 * @tc.name: ExecuteWindowStatusChangeNotification
 * @tc.desc: ExecuteWindowStatusChangeNotificationForUITest
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ExecuteWindowStatusChangeNotification, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ExecuteWindowStatusChangeNotification";
    info.bundleName_ = "ExecuteWindowStatusChangeNotification";
    auto specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);

    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    EXPECT_CALL(*mockVsyncStation, RequestVsync(_))
        .WillRepeatedly(Invoke([](const std::shared_ptr<VsyncCallback>& cb) {
            ASSERT_NE(cb, nullptr);
            cb->onCallback(0, 0);
        }));
    sceneSession->SetVsyncStation(mockVsyncStation);
    ASSERT_NE(sceneSession->vsyncStation_, nullptr);

    sceneSession->RegisterGetIsDockAutoHideFunc([](){return true;});

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_CALL(*mockSessionStage, NotifyLayoutFinishAfterWindowModeChange(_)).Times(0);
    sceneSession->ExecuteWindowStatusChangeNotification("unittest");
    usleep(WAIT_SYNC_NS);

    sceneSession->sessionStage_ = mockSessionStage;
    sceneSession->handler_ = nullptr;
    EXPECT_CALL(*mockSessionStage, NotifyLayoutFinishAfterWindowModeChange(_)).Times(1);
    sceneSession->ExecuteWindowStatusChangeNotification("unittest");
    usleep(WAIT_SYNC_NS);
}

/**
 * @tc.name: NotifyAttachedWindowsLimitsChanged01
 * @tc.desc: Test NotifyAttachedWindowsLimitsChanged with main window and attached children
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyAttachedWindowsLimitsChanged01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyAttachedWindowsLimitsChanged01";
    info.bundleName_ = "NotifyAttachedWindowsLimitsChanged01";

    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    mainSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    // Set up attach info on main session
    mainSession->windowAnchorInfo_.isAnchoredByAttach_ = true;
    mainSession->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    mainSession->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    // Create child session with attach
    sptr<SessionStageMocker> childSessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<SubSession> childSession =
        CreateChildSessionWithAttach("NotifyAttachedWindowsLimitsChanged01_child");
    childSession->sessionStage_ = childSessionStage;
    SetupParentChild(mainSession, childSession);

    WindowLimits newLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    // Main window should notify its children via UpdateAttachedWindowLimits (async operation)
    EXPECT_CALL(*childSessionStage, UpdateAttachedWindowLimits(
        mainSession->GetPersistentId(), testing::_, true, true))
        .Times(1)
        .WillOnce(testing::Return(WSError::WS_OK));
    mainSession->NotifyAttachedWindowsLimitsChanged(newLimits);
    usleep(WAIT_SYNC_NS);

    // Clean up: break circular sptr references
    mainSession->subSession_.clear();
    childSession->parentSession_ = nullptr;
}

/**
 * @tc.name: NotifyAttachedWindowsLimitsChanged02
 * @tc.desc: Test NotifyAttachedWindowsLimitsChanged with child window notifying parent
 *           Parent should then notify all children (including siblings) automatically
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyAttachedWindowsLimitsChanged02, TestSize.Level1)
{
    sptr<SessionStageMocker> mainSessionStage;
    sptr<MainSession> mainSession =
        CreateMainSessionWithAttach("NotifyAttachedWindowsLimitsChanged02_main", mainSessionStage);

    // Create first child session (the one that will trigger notification)
    sptr<SubSession> childSession1 =
        CreateChildSessionWithAttach("NotifyAttachedWindowsLimitsChanged02_child1", true, true);
    SetupParentChild(mainSession, childSession1);

    // Create second child session (sibling that should be notified)
    sptr<SessionStageMocker> childSession2Stage = sptr<SessionStageMocker>::MakeSptr();
    sptr<SubSession> childSession2 =
        CreateChildSessionWithAttach("NotifyAttachedWindowsLimitsChanged02_child2", false, true);
    childSession2->sessionStage_ = childSession2Stage;
    SetupParentChild(mainSession, childSession2);

    WindowLimits newLimits = { 1800, 1200, 100, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    // Child window should notify parent and parent should notify sibling (async operation)
    // Parent Session calls its SessionStage to update own limits (excludePersistentId defaults to INVALID_SESSION_ID)
    EXPECT_CALL(*mainSessionStage, UpdateAttachedWindowLimits(
        childSession1->GetPersistentId(), testing::_, true, true))
        .Times(1)
        .WillOnce(testing::Return(WSError::WS_OK));
    // Parent then propagates to sibling via UpdateAttachedWindowLimits
    EXPECT_CALL(*childSession2Stage, UpdateAttachedWindowLimits(
        childSession1->GetPersistentId(), testing::_, true, true))
        .Times(1)
        .WillOnce(testing::Return(WSError::WS_OK));
    childSession1->NotifyAttachedWindowsLimitsChanged(newLimits);
    usleep(WAIT_SYNC_NS);

    // Clean up: break circular sptr references
    mainSession->subSession_.clear();
    childSession1->parentSession_ = nullptr;
    childSession2->parentSession_ = nullptr;
}

/**
 * @tc.name: NotifyAttachedWindowsLimitsChanged03
 * @tc.desc: Test NotifyAttachedWindowsLimitsChanged with main window notifying multiple children
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyAttachedWindowsLimitsChanged03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyAttachedWindowsLimitsChanged03";
    info.bundleName_ = "NotifyAttachedWindowsLimitsChanged03";

    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    mainSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    // Set up attach info on main session
    mainSession->windowAnchorInfo_.isAnchoredByAttach_ = true;
    mainSession->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    mainSession->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    // Create multiple child sessions with different limit settings
    sptr<SessionStageMocker> childSessionStage1 = sptr<SessionStageMocker>::MakeSptr();
    sptr<SessionStageMocker> childSessionStage2 = sptr<SessionStageMocker>::MakeSptr();
    sptr<SessionStageMocker> childSessionStage3 = sptr<SessionStageMocker>::MakeSptr();

    sptr<SubSession> childSession1 =
        CreateChildSessionWithAttach("NotifyAttachedWindowsLimitsChanged03_child1", true, false);
    childSession1->sessionStage_ = childSessionStage1;
    SetupParentChild(mainSession, childSession1);

    sptr<SubSession> childSession2 =
        CreateChildSessionWithAttach("NotifyAttachedWindowsLimitsChanged03_child2", false, true);
    childSession2->sessionStage_ = childSessionStage2;
    SetupParentChild(mainSession, childSession2);

    sptr<SubSession> childSession3 =
        CreateChildSessionWithAttach("NotifyAttachedWindowsLimitsChanged03_child3", true, false);
    childSession3->sessionStage_ = childSessionStage3;
    SetupParentChild(mainSession, childSession3);

    WindowLimits newLimits = { 1800, 900, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    // Main window should notify all its children via UpdateAttachedWindowLimits (async operation)
    EXPECT_CALL(*childSessionStage1, UpdateAttachedWindowLimits(
        mainSession->GetPersistentId(), testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));
    EXPECT_CALL(*childSessionStage2, UpdateAttachedWindowLimits(
        mainSession->GetPersistentId(), testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));
    EXPECT_CALL(*childSessionStage3, UpdateAttachedWindowLimits(
        mainSession->GetPersistentId(), testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));
    mainSession->NotifyAttachedWindowsLimitsChanged(newLimits);
    usleep(WAIT_SYNC_NS);

    // Clean up: break circular sptr references
    mainSession->subSession_.clear();
    childSession1->parentSession_ = nullptr;
    childSession2->parentSession_ = nullptr;
    childSession3->parentSession_ = nullptr;
}

/**
 * @tc.name: NotifyAttachedWindowsLimitsChanged04
 * @tc.desc: Test NotifyAttachedWindowsLimitsChanged with child window notifying parent
 *           Parent should then notify all other children (siblings) automatically
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyAttachedWindowsLimitsChanged04, TestSize.Level1)
{
    sptr<SessionStageMocker> mainSessionStage;
    sptr<MainSession> mainSession =
        CreateMainSessionWithAttach("NotifyAttachedWindowsLimitsChanged04_main", mainSessionStage);

    // Create the child session that will trigger notification
    sptr<SubSession> childSession1 = CreateChildSessionWithAttach("NotifyAttachedWindowsLimitsChanged04_child1");
    sptr<SessionStageMocker> childSessionStage1 = sptr<SessionStageMocker>::MakeSptr();
    childSession1->sessionStage_ = childSessionStage1;
    SetupParentChild(mainSession, childSession1);

    // Create sibling child sessions with different limit settings
    sptr<SessionStageMocker> childSessionStage2 = sptr<SessionStageMocker>::MakeSptr();
    sptr<SessionStageMocker> childSessionStage3 = sptr<SessionStageMocker>::MakeSptr();

    sptr<SubSession> childSession2 =
        CreateChildSessionWithAttach("NotifyAttachedWindowsLimitsChanged04_child2", true, false);
    childSession2->sessionStage_ = childSessionStage2;
    SetupParentChild(mainSession, childSession2);

    sptr<SubSession> childSession3 =
        CreateChildSessionWithAttach("NotifyAttachedWindowsLimitsChanged04_child3", false, true);
    childSession3->sessionStage_ = childSessionStage3;
    SetupParentChild(mainSession, childSession3);

    WindowLimits newLimits = { 1900, 1100, 120, 280, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    // Set up expectations: child notifies parent
    // Parent Session calls its SessionStage to update own limits
    EXPECT_CALL(*mainSessionStage, UpdateAttachedWindowLimits(
        childSession1->GetPersistentId(), testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Parent then notifies other children via UpdateAttachedWindowLimits
    EXPECT_CALL(*childSessionStage2, UpdateAttachedWindowLimits(
        childSession1->GetPersistentId(), testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));
    EXPECT_CALL(*childSessionStage3, UpdateAttachedWindowLimits(
        childSession1->GetPersistentId(), testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Child window should notify parent, and parent should notify all siblings automatically
    childSession1->NotifyAttachedWindowsLimitsChanged(newLimits);
    usleep(WAIT_SYNC_NS);

    // Clean up: break circular sptr references
    mainSession->subSession_.clear();
    childSession1->parentSession_ = nullptr;
    childSession2->parentSession_ = nullptr;
    childSession3->parentSession_ = nullptr;
}

/**
 * @tc.name: NotifyAttachedWindowsLimitsChanged05
 * @tc.desc: Test NotifyAttachedWindowsLimitsChanged without attach relationship
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyAttachedWindowsLimitsChanged05, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyAttachedWindowsLimitsChanged05";
    info.bundleName_ = "NotifyAttachedWindowsLimitsChanged05";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    // Set up without attach relationship
    session->windowAnchorInfo_.isAnchoredByAttach_ = false;

    WindowLimits newLimits = { 1800, 900, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    // No attach relationship - no parent to notify, so this should complete without errors
    // This tests the path when isAnchoredByAttach_ is false
    auto result = session->NotifyAttachedWindowsLimitsChanged(newLimits);
    usleep(WAIT_SYNC_NS);  // Wait for async task to complete

    // Verify function returns OK and limits were set in property
    EXPECT_EQ(result, WSError::WS_OK);
    WindowLimits savedLimits = session->GetSessionProperty()->GetLimitsForAttachedWindows();
    EXPECT_EQ(savedLimits.maxWidth_, 1800);
    EXPECT_EQ(savedLimits.maxHeight_, 900);
    EXPECT_EQ(savedLimits.minWidth_, 150);
    EXPECT_EQ(savedLimits.minHeight_, 250);
}

/**
 * @tc.name: NotifyAttachedWindowsLimitsChanged06
 * @tc.desc: Test NotifyAttachedWindowsLimitsChanged with sub window but no parent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyAttachedWindowsLimitsChanged06, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyAttachedWindowsLimitsChanged06";
    info.bundleName_ = "NotifyAttachedWindowsLimitsChanged06";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info for sub window
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Deliberately don't set parent - parentSession will be null
    // This should log a warning but not crash
    WindowLimits newLimits = { 1800, 900, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    auto result = session->NotifyAttachedWindowsLimitsChanged(newLimits);
    usleep(WAIT_SYNC_NS);  // Wait for async task to complete

    // Verify function returns OK, limits were set, and no crash occurred
    EXPECT_EQ(result, WSError::WS_OK);
    WindowLimits savedLimits = session->GetSessionProperty()->GetLimitsForAttachedWindows();
    EXPECT_EQ(savedLimits.maxWidth_, 1800);
    EXPECT_EQ(savedLimits.maxHeight_, 900);
    EXPECT_EQ(savedLimits.minWidth_, 150);
    EXPECT_EQ(savedLimits.minHeight_, 250);
}

/**
 * @tc.name: NotifyAttachedWindowsLimitsChanged07
 * @tc.desc: Test NotifyAttachedWindowsLimitsChanged with system modal window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyAttachedWindowsLimitsChanged07, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyAttachedWindowsLimitsChanged07";
    info.bundleName_ = "NotifyAttachedWindowsLimitsChanged07";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_FLOAT);

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info for system modal window
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // System modal window is neither main nor sub window, so it should not trigger any notifications
    WindowLimits newLimits = { 1800, 900, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    auto result = session->NotifyAttachedWindowsLimitsChanged(newLimits);
    usleep(WAIT_SYNC_NS);  // Wait for async task to complete

    // Verify function returns OK and limits were set (even though no notifications are sent)
    EXPECT_EQ(result, WSError::WS_OK);
    WindowLimits savedLimits = session->GetSessionProperty()->GetLimitsForAttachedWindows();
    EXPECT_EQ(savedLimits.maxWidth_, 1800);
    EXPECT_EQ(savedLimits.maxHeight_, 900);
    EXPECT_EQ(savedLimits.minWidth_, 150);
    EXPECT_EQ(savedLimits.minHeight_, 250);
}

/**
 * @tc.name: NotifyRelatedWindowsAttachStateChange01
 * @tc.desc: Test NotifyRelatedWindowsAttachStateChange when isFromAttachOrDetach_ is false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsAttachStateChange01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsAttachStateChange01";
    info.bundleName_ = "NotifyRelatedWindowsAttachStateChange01";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info with isFromAttachOrDetach_ = false
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = false;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    sptr<Session> parentSession = nullptr;

    // Should return early without any operations
    session->NotifyRelatedWindowsAttachStateChange(parentSession, false, true, false, false);

    // Verify no changes to property lists
    EXPECT_TRUE(session->GetSessionProperty()->GetAttachedWindowLimitsList().empty());
    EXPECT_TRUE(session->GetSessionProperty()->GetAttachedLimitOptionsList().empty());
}

/**
 * @tc.name: NotifyRelatedWindowsAttachStateChange02
 * @tc.desc: Test NotifyRelatedWindowsAttachStateChange when effective limits unchanged
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsAttachStateChange02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsAttachStateChange02";
    info.bundleName_ = "NotifyRelatedWindowsAttachStateChange02";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info with intersected limits enabled
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    sptr<Session> parentSession = nullptr;

    // wasAttached=false, isAttached=false
    // Old effective: false && true = false, New effective: false && true = false (no change)
    session->NotifyRelatedWindowsAttachStateChange(parentSession, false, false, true, true);

    // Verify no changes to property lists
    EXPECT_TRUE(session->GetSessionProperty()->GetAttachedWindowLimitsList().empty());
    EXPECT_TRUE(session->GetSessionProperty()->GetAttachedLimitOptionsList().empty());
}

/**
 * @tc.name: NotifyRelatedWindowsAttachStateChange03
 * @tc.desc: Test NotifyRelatedWindowsAttachStateChange attach scenario without parent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsAttachStateChange03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsAttachStateChange03";
    info.bundleName_ = "NotifyRelatedWindowsAttachStateChange03";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    sptr<Session> parentSession = nullptr;

    // wasAttached=false, isAttached=true (attach scenario)
    // Without parent, should not call any request methods
    session->NotifyRelatedWindowsAttachStateChange(parentSession, false, true, false, false);

    // Verify no crash and lists remain empty
    EXPECT_TRUE(session->GetSessionProperty()->GetAttachedWindowLimitsList().empty());
    EXPECT_TRUE(session->GetSessionProperty()->GetAttachedLimitOptionsList().empty());
}

/**
 * @tc.name: NotifyRelatedWindowsAttachStateChange04
 * @tc.desc: Test NotifyRelatedWindowsAttachStateChange attach scenario with parent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsAttachStateChange04, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsAttachStateChange04";
    info.bundleName_ = "NotifyRelatedWindowsAttachStateChange04";

    // Create parent session (MainSession overrides RequestUpdateAttachedWindowLimits)
    sptr<Session> parentSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> parentSceneSession = static_cast<SceneSession*>(parentSession.GetRefPtr());
    sptr<SessionStageMocker> parentSessionStage = sptr<SessionStageMocker>::MakeSptr();
    parentSceneSession->sessionStage_ = parentSessionStage;

    // Create child session
    sptr<SceneSession> childSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    childSession->sessionStage_ = mockSessionStage_;

    // Set up attach info for child
    childSession->windowAnchorInfo_.isAnchoredByAttach_ = true;
    childSession->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    childSession->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    childSession->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Set window limits for notification
    childSession->GetSessionProperty()->SetLimitsForAttachedWindows(
        { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX });

    // Expect parent to receive RequestUpdateAttachedWindowLimits call
    EXPECT_CALL(*parentSessionStage, UpdateAttachedWindowLimits(
        childSession->GetPersistentId(), testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // wasAttached=false, isAttached=true (attach scenario)
    childSession->NotifyRelatedWindowsAttachStateChange(parentSession, false, true, false, false);
}

/**
 * @tc.name: NotifyRelatedWindowsAttachStateChange05
 * @tc.desc: Test NotifyRelatedWindowsAttachStateChange detach scenario without parent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsAttachStateChange05, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsAttachStateChange05";
    info.bundleName_ = "NotifyRelatedWindowsAttachStateChange05";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Add some limits to the lists to verify they get cleared
    auto property = session->GetSessionProperty();
    property->SetAttachedWindowLimits(100, { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    AttachLimitOptions options = { true, true };
    property->SetAttachedLimitOptions(100, options);

    // Verify lists are not empty before
    EXPECT_FALSE(property->GetAttachedWindowLimitsList().empty());
    EXPECT_FALSE(property->GetAttachedLimitOptionsList().empty());

    sptr<Session> parentSession = nullptr;

    // wasAttached=true, isAttached=false (detach scenario)
    session->NotifyRelatedWindowsAttachStateChange(parentSession, true, false, true, true);
}

/**
 * @tc.name: NotifyRelatedWindowsAttachStateChange06
 * @tc.desc: Test NotifyRelatedWindowsAttachStateChange detach scenario with parent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsAttachStateChange06, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsAttachStateChange06";
    info.bundleName_ = "NotifyRelatedWindowsAttachStateChange06";

    // Create parent session (MainSession overrides RequestRemoveAttachedWindowLimits)
    sptr<Session> parentSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> parentSceneSession = static_cast<SceneSession*>(parentSession.GetRefPtr());
    sptr<SessionStageMocker> parentSessionStage = sptr<SessionStageMocker>::MakeSptr();
    parentSceneSession->sessionStage_ = parentSessionStage;

    // Create child session
    sptr<SceneSession> childSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    childSession->sessionStage_ = mockSessionStage_;

    // Set up attach info for child
    childSession->windowAnchorInfo_.isAnchoredByAttach_ = true;
    childSession->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    childSession->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    childSession->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Expect parent to receive RequestRemoveAttachedWindowLimits call
    EXPECT_CALL(*parentSessionStage, RemoveAttachedWindowLimits(childSession->GetPersistentId()))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // wasAttached=true, isAttached=false (detach scenario)
    childSession->NotifyRelatedWindowsAttachStateChange(parentSession, true, false, true, true);

    // Verify lists were cleared
    EXPECT_TRUE(childSession->GetSessionProperty()->GetAttachedWindowLimitsList().empty());
    EXPECT_TRUE(childSession->GetSessionProperty()->GetAttachedLimitOptionsList().empty());
}

/**
 * @tc.name: NotifyRelatedWindowsAttachStateChange07
 * @tc.desc: Test NotifyRelatedWindowsAttachStateChange when limits change without parent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsAttachStateChange07, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsAttachStateChange07";
    info.bundleName_ = "NotifyRelatedWindowsAttachStateChange07";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info with intersected limits
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;  // New value
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    sptr<Session> parentSession = nullptr;

    // wasAttached=true, isAttached=true (attach state unchanged)
    // old height limit was false, new is true (limits changed)
    // Without parent, should not call any request methods
    session->NotifyRelatedWindowsAttachStateChange(parentSession, true, true, true, false);

    // Verify no crash
    EXPECT_TRUE(session->GetWindowAnchorInfo().isAnchoredByAttach_);
}

/**
 * @tc.name: NotifyRelatedWindowsAttachStateChange08
 * @tc.desc: Test NotifyRelatedWindowsAttachStateChange when limits change with parent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsAttachStateChange08, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsAttachStateChange08";
    info.bundleName_ = "NotifyRelatedWindowsAttachStateChange08";

    // Create parent session (MainSession overrides RequestUpdateAttachedWindowLimits)
    sptr<Session> parentSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> parentSceneSession = static_cast<SceneSession*>(parentSession.GetRefPtr());
    sptr<SessionStageMocker> parentSessionStage = sptr<SessionStageMocker>::MakeSptr();
    parentSceneSession->sessionStage_ = parentSessionStage;

    // Create child session
    sptr<SceneSession> childSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    childSession->sessionStage_ = mockSessionStage_;

    // Set up attach info with new limits (height enabled, width enabled)
    childSession->windowAnchorInfo_.isAnchoredByAttach_ = true;
    childSession->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    childSession->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;  // New value (changed from false)
    childSession->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Set window limits for notification
    childSession->GetSessionProperty()->SetLimitsForAttachedWindows(
        { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX });

    // Expect parent to receive RequestUpdateAttachedWindowLimits call
    // wasAttached=true, isAttached=true, but old height limit was false
    // Old effective: true && false = false (height), New effective: true && true = true (height)
    EXPECT_CALL(*parentSessionStage, UpdateAttachedWindowLimits(
        childSession->GetPersistentId(), testing::_, true, true))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Limits changed scenario
    childSession->NotifyRelatedWindowsAttachStateChange(parentSession, true, true, true, false);
}

/**
 * @tc.name: NotifyRelatedWindowsOnDestruction01
 * @tc.desc: Test NotifyRelatedWindowsOnDestruction with main window and attached children
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsOnDestruction01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsOnDestruction01";
    info.bundleName_ = "NotifyRelatedWindowsOnDestruction01";

    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    mainSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    // Set up attach info on main session
    mainSession->windowAnchorInfo_.isAnchoredByAttach_ = true;
    mainSession->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    mainSession->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    // Create child session with attach
    SessionInfo childInfo;
    childInfo.abilityName_ = "NotifyRelatedWindowsOnDestruction01_child";
    childInfo.bundleName_ = "NotifyRelatedWindowsOnDestruction01_child";
    childInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    sptr<SubSession> childSession = sptr<SubSession>::MakeSptr(childInfo, nullptr);

    childSession->windowAnchorInfo_.isAnchoredByAttach_ = true;
    childSession->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    childSession->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Create mock session stage for child
    sptr<SessionStageMocker> childSessionStage = sptr<SessionStageMocker>::MakeSptr();
    childSession->sessionStage_ = childSessionStage;

    // Add child to main session
    mainSession->subSession_.push_back(childSession);
    childSession->parentSession_ = mainSession;

    // Main window being destroyed should call RemoveAttachedWindowLimits on child
    EXPECT_CALL(*childSessionStage, RemoveAttachedWindowLimits(mainSession->GetPersistentId()))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    mainSession->NotifyRelatedWindowsOnDestruction();

    // Clean up: break circular sptr references
    mainSession->subSession_.clear();
    childSession->parentSession_ = nullptr;
}

/**
 * @tc.name: NotifyRelatedWindowsOnDestruction02
 * @tc.desc: Test NotifyRelatedWindowsOnDestruction with child window notifying parent
 *           Parent should then notify other children (siblings) to remove limits
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsOnDestruction02, TestSize.Level1)
{
    sptr<SessionStageMocker> mainSessionStage;
    sptr<MainSession> mainSession =
        CreateMainSessionWithAttach("NotifyRelatedWindowsOnDestruction02_main", mainSessionStage);

    // Create first child session (the one being destroyed)
    sptr<SubSession> childSession1 =
        CreateChildSessionWithAttach("NotifyRelatedWindowsOnDestruction02_child1", true, false);
    SetupParentChild(mainSession, childSession1);

    // Create second child session (sibling that should be notified)
    sptr<SessionStageMocker> childSession2Stage = sptr<SessionStageMocker>::MakeSptr();
    sptr<SubSession> childSession2 =
        CreateChildSessionWithAttach("NotifyRelatedWindowsOnDestruction02_child2", false, true);
    childSession2->sessionStage_ = childSession2Stage;
    SetupParentChild(mainSession, childSession2);

    // Child being destroyed notifies parent via RemoveAttachedWindowLimits
    EXPECT_CALL(*mainSessionStage, RemoveAttachedWindowLimits(childSession1->GetPersistentId()))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Parent then notifies sibling via RemoveAttachedWindowLimits
    EXPECT_CALL(*childSession2Stage, RemoveAttachedWindowLimits(childSession1->GetPersistentId()))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Child window being destroyed should notify parent, and parent should notify sibling
    childSession1->NotifyRelatedWindowsOnDestruction();

    // Clean up: break circular sptr references
    mainSession->subSession_.clear();
    childSession1->parentSession_ = nullptr;
    childSession2->parentSession_ = nullptr;
}

/**
 * @tc.name: NotifyRelatedWindowsOnDestruction03
 * @tc.desc: Test NotifyRelatedWindowsOnDestruction with main window notifying multiple children
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsOnDestruction03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsOnDestruction03";
    info.bundleName_ = "NotifyRelatedWindowsOnDestruction03";

    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    mainSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    // Set up attach info on main session
    mainSession->windowAnchorInfo_.isAnchoredByAttach_ = true;
    mainSession->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    mainSession->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    sptr<SessionStageMocker> mainSessionStage = sptr<SessionStageMocker>::MakeSptr();
    mainSession->sessionStage_ = mainSessionStage;

    // Create multiple child sessions with attach
    sptr<SessionStageMocker> childSessionStage1 = sptr<SessionStageMocker>::MakeSptr();
    sptr<SessionStageMocker> childSessionStage2 = sptr<SessionStageMocker>::MakeSptr();

    sptr<SubSession> childSession1 =
        CreateChildSessionWithAttach("NotifyRelatedWindowsOnDestruction03_child1", true, false);
    childSession1->sessionStage_ = childSessionStage1;
    SetupParentChild(mainSession, childSession1);

    sptr<SubSession> childSession2 =
        CreateChildSessionWithAttach("NotifyRelatedWindowsOnDestruction03_child2", false, true);
    childSession2->sessionStage_ = childSessionStage2;
    SetupParentChild(mainSession, childSession2);

    // Main window being destroyed should call RemoveAttachedWindowLimits on all children
    EXPECT_CALL(*childSessionStage1, RemoveAttachedWindowLimits(mainSession->GetPersistentId()))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));
    EXPECT_CALL(*childSessionStage2, RemoveAttachedWindowLimits(mainSession->GetPersistentId()))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Main window being destroyed should notify all its children
    mainSession->NotifyRelatedWindowsOnDestruction();

    // Clean up: break circular sptr references
    mainSession->subSession_.clear();
    childSession1->parentSession_ = nullptr;
    childSession2->parentSession_ = nullptr;
}

/**
 * @tc.name: NotifyRelatedWindowsOnDestruction04
 * @tc.desc: Test NotifyRelatedWindowsOnDestruction with child window notifying parent
 *           Parent should then notify all other children (multiple siblings) to remove limits
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsOnDestruction04, TestSize.Level1)
{
    sptr<SessionStageMocker> mainSessionStage;
    sptr<MainSession> mainSession =
        CreateMainSessionWithAttach("NotifyRelatedWindowsOnDestruction04_main", mainSessionStage);

    // Create the child session that will be destroyed
    sptr<SubSession> childSession1 = CreateChildSessionWithAttach("NotifyRelatedWindowsOnDestruction04_child1");
    SetupParentChild(mainSession, childSession1);

    // Create sibling child sessions with different limit settings
    sptr<SessionStageMocker> childSessionStage2 = sptr<SessionStageMocker>::MakeSptr();
    sptr<SessionStageMocker> childSessionStage3 = sptr<SessionStageMocker>::MakeSptr();

    sptr<SubSession> childSession2 =
        CreateChildSessionWithAttach("NotifyRelatedWindowsOnDestruction04_child2", true, false);
    childSession2->sessionStage_ = childSessionStage2;
    SetupParentChild(mainSession, childSession2);

    sptr<SubSession> childSession3 =
        CreateChildSessionWithAttach("NotifyRelatedWindowsOnDestruction04_child3", false, true);
    childSession3->sessionStage_ = childSessionStage3;
    SetupParentChild(mainSession, childSession3);

    // Child being destroyed notifies parent via RemoveAttachedWindowLimits
    EXPECT_CALL(*mainSessionStage, RemoveAttachedWindowLimits(childSession1->GetPersistentId()))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Parent then notifies all siblings via RemoveAttachedWindowLimits
    EXPECT_CALL(*childSessionStage2, RemoveAttachedWindowLimits(childSession1->GetPersistentId()))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));
    EXPECT_CALL(*childSessionStage3, RemoveAttachedWindowLimits(childSession1->GetPersistentId()))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    // Child window being destroyed should notify parent, and parent should notify all siblings
    childSession1->NotifyRelatedWindowsOnDestruction();

    // Clean up: break circular sptr references
    mainSession->subSession_.clear();
    childSession1->parentSession_ = nullptr;
    childSession2->parentSession_ = nullptr;
    childSession3->parentSession_ = nullptr;
}

/**
 * @tc.name: NotifyRelatedWindowsOnDestruction05
 * @tc.desc: Test NotifyRelatedWindowsOnDestruction without attach relationship
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsOnDestruction05, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsOnDestruction05";
    info.bundleName_ = "NotifyRelatedWindowsOnDestruction05";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    // Set up without attach - should return early
    session->windowAnchorInfo_.isAnchoredByAttach_ = false;

    session->NotifyRelatedWindowsOnDestruction();

    // Verify early return path - no attach relationship
    EXPECT_FALSE(session->GetWindowAnchorInfo().isAnchoredByAttach_);
}

/**
 * @tc.name: NotifyRelatedWindowsOnDestruction06
 * @tc.desc: Test NotifyRelatedWindowsOnDestruction with attach but no intersected limits
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsOnDestruction06, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsOnDestruction06";
    info.bundleName_ = "NotifyRelatedWindowsOnDestruction06";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    // Set up with attach but no intersected limits - should return early
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = false;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = false;

    session->NotifyRelatedWindowsOnDestruction();

    // Verify early return path - attach exists but no intersected limits
    EXPECT_TRUE(session->GetWindowAnchorInfo().isAnchoredByAttach_);
    EXPECT_FALSE(session->GetWindowAnchorInfo().attachOptions.isIntersectedHeightLimit);
    EXPECT_FALSE(session->GetWindowAnchorInfo().attachOptions.isIntersectedWidthLimit);
}

/**
 * @tc.name: NotifyRelatedWindowsOnDestruction07
 * @tc.desc: Test NotifyRelatedWindowsOnDestruction with sub window but no parent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsOnDestruction07, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsOnDestruction07";
    info.bundleName_ = "NotifyRelatedWindowsOnDestruction07";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info for sub window
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Deliberately don't set parent - parentSession will be null
    // This should log a warning but not crash
    session->NotifyRelatedWindowsOnDestruction();

    // Verify sub window with attach but no parent does not crash
    EXPECT_EQ(info.windowType_, static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW));
    EXPECT_EQ(session->parentSession_, nullptr);
    EXPECT_TRUE(session->GetWindowAnchorInfo().isAnchoredByAttach_);
}

/**
 * @tc.name: NotifyRelatedWindowsOnDestruction08
 * @tc.desc: Test NotifyRelatedWindowsOnDestruction with system modal window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRelatedWindowsOnDestruction08, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRelatedWindowsOnDestruction08";
    info.bundleName_ = "NotifyRelatedWindowsOnDestruction08";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_FLOAT);

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info for system modal window
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // System modal window is neither main nor sub window, so it should not trigger any notifications
    session->NotifyRelatedWindowsOnDestruction();

    // Verify FLOAT window type and attach state (no notifications sent)
    EXPECT_EQ(info.windowType_, static_cast<uint32_t>(WindowType::WINDOW_TYPE_FLOAT));
    EXPECT_TRUE(session->GetWindowAnchorInfo().isAnchoredByAttach_);
}

/**
 * @tc.name: RequestUpdateAttachedWindowLimits_BaseClass
 * @tc.desc: Test SceneSession base class default implementation returns WS_OK
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, RequestUpdateAttachedWindowLimits_BaseClass, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestUpdateAttachedWindowLimits_BaseClass";
    info.bundleName_ = "RequestUpdateAttachedWindowLimits_BaseClass";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    int32_t sourcePersistentId = 1001;
    WindowLimits attachedLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    bool isIntersectedHeightLimit = true;
    bool isIntersectedWidthLimit = true;

    // Base class default implementation should return WS_OK
    WSError ret = session->RequestUpdateAttachedWindowLimits(sourcePersistentId, attachedLimits,
        isIntersectedHeightLimit, isIntersectedWidthLimit);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: RequestRemoveAttachedWindowLimits_BaseClass
 * @tc.desc: Test SceneSession base class default implementation returns WS_OK
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, RequestRemoveAttachedWindowLimits_BaseClass, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestRemoveAttachedWindowLimits_BaseClass";
    info.bundleName_ = "RequestRemoveAttachedWindowLimits_BaseClass";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    int32_t sourcePersistentId = 2001;

    // Base class default implementation should return WS_OK
    WSError ret = session->RequestRemoveAttachedWindowLimits(sourcePersistentId);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ShouldProcessAttachStateChange01
 * @tc.desc: Test ShouldProcessAttachStateChange with valid session and attach info
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ShouldProcessAttachStateChange01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ShouldProcessAttachStateChange01";
    info.bundleName_ = "ShouldProcessAttachStateChange01";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info with intersected limits
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Test with valid session and attach state change (wasAttached=false, isAttached=true)
    bool isDetaching = false;
    bool result = session->ShouldProcessAttachStateChange(false, true, false, false, isDetaching);
    EXPECT_EQ(true, result);  // Should return true as effective limits changed
    EXPECT_EQ(false, isDetaching);  // Not detaching
}

/**
 * @tc.name: ShouldProcessAttachStateChange02
 * @tc.desc: Test ShouldProcessAttachStateChange returns false when isFromAttachOrDetach_ is false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ShouldProcessAttachStateChange02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ShouldProcessAttachStateChange02";
    info.bundleName_ = "ShouldProcessAttachStateChange02";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info with isFromAttachOrDetach_ = false
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = false;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    bool isDetaching = false;
    bool result = session->ShouldProcessAttachStateChange(false, true, false, false, isDetaching);
    EXPECT_EQ(false, result);
}

/**
 * @tc.name: ShouldProcessAttachStateChange03
 * @tc.desc: Test ShouldProcessAttachStateChange returns false when effective limits unchanged
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ShouldProcessAttachStateChange03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ShouldProcessAttachStateChange03";
    info.bundleName_ = "ShouldProcessAttachStateChange03";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info with intersected limits
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Test with unchanged effective limits (wasAttached=false, isAttached=false)
    // Old: false && true = false, New: false && true = false (no change)
    bool isDetaching = false;
    bool result = session->ShouldProcessAttachStateChange(false, false, true, true, isDetaching);
    EXPECT_EQ(false, result);
}

/**
 * @tc.name: ShouldProcessAttachStateChange04
 * @tc.desc: Test ShouldProcessAttachStateChange returns true when effective limits change
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ShouldProcessAttachStateChange04, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ShouldProcessAttachStateChange04";
    info.bundleName_ = "ShouldProcessAttachStateChange04";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info with intersected limits
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Test with changed effective limits (wasAttached=false, isAttached=true)
    // Old: false && false = false, New: true && true = true (changed)
    bool isDetaching = false;
    bool result = session->ShouldProcessAttachStateChange(false, true, false, false, isDetaching);
    EXPECT_EQ(true, result);
    EXPECT_EQ(false, isDetaching);  // Not detaching (wasAttached=false, isAttached=true)
}

/**
 * @tc.name: ShouldProcessAttachStateChange05
 * @tc.desc: Test ShouldProcessAttachStateChange with attach scenario
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ShouldProcessAttachStateChange05, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ShouldProcessAttachStateChange05";
    info.bundleName_ = "ShouldProcessAttachStateChange05";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info with height limit only
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = false;

    // Test attach: wasAttached=false, isAttached=true
    // Old: false && false = false, New: true && true = true (height changed)
    bool isDetaching = false;
    bool result = session->ShouldProcessAttachStateChange(false, true, false, false, isDetaching);
    EXPECT_EQ(true, result);
    EXPECT_EQ(false, isDetaching);  // Not detaching
}

/**
 * @tc.name: ShouldProcessAttachStateChange06
 * @tc.desc: Test ShouldProcessAttachStateChange with detach scenario
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ShouldProcessAttachStateChange06, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ShouldProcessAttachStateChange06";
    info.bundleName_ = "ShouldProcessAttachStateChange06";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info with both limits
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Test detach: wasAttached=true, isAttached=false
    // Old: true && true = true, New: false && true = false (changed)
    bool isDetaching = false;
    bool result = session->ShouldProcessAttachStateChange(true, false, true, true, isDetaching);
    EXPECT_EQ(true, result);
    EXPECT_EQ(true, isDetaching);  // Detaching (wasAttached=true, isAttached=false)
}

/**
 * @tc.name: ShouldProcessAttachStateChange07
 * @tc.desc: Test ShouldProcessAttachStateChange returns false when effective limits unchanged (both true)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ShouldProcessAttachStateChange07, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ShouldProcessAttachStateChange07";
    info.bundleName_ = "ShouldProcessAttachStateChange07";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info with intersected limits
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Test with both old and new effective limits being true (no change)
    // Old: true && true = true, New: true && true = true (no change)
    bool isDetaching = false;
    bool result = session->ShouldProcessAttachStateChange(true, true, true, true, isDetaching);
    EXPECT_EQ(false, result);  // No change in effective limits
}

/**
 * @tc.name: ShouldProcessAttachStateChange08
 * @tc.desc: Test ShouldProcessAttachStateChange when attach state unchanged but limits change
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ShouldProcessAttachStateChange08, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ShouldProcessAttachStateChange08";
    info.bundleName_ = "ShouldProcessAttachStateChange08";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Set up attach info with intersected limits
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;  // New value
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;

    // Test with wasAttached=true, isAttached=true, but old limits changed
    // Old: true && false = false (height), true && true = true (width)
    // New: true && true = true (height), true && true = true (width)
    // Height limit changed from false to true
    bool isDetaching = false;
    bool result = session->ShouldProcessAttachStateChange(true, true, true, false, isDetaching);
    EXPECT_EQ(true, result);  // Height limit changed
    EXPECT_EQ(false, isDetaching);  // Not detaching (wasAttached=true, isAttached=true)
}

/**
 * @tc.name: HandleMoveDragEnd
 * @tc.desc: HandleMoveDragEnd
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, HandleMoveDragEnd, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleMoveDragEnd";
    info.bundleName_ = "HandleMoveDragEnd";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->subSession_.push_back(subSession);
    sceneSession->moveDragController_ =
        sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    subSession->state_ = SessionState::STATE_FOREGROUND;
    const uint64_t startDisplayId = 0;
    const uint64_t endDisplayId = 12;
    WSRect rect = { 0, 0, 1000, 1000 };

    // Case1: Not cross screen
    sceneSession->GetSessionProperty()->SetDisplayId(startDisplayId);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->GetMoveDragController()->winType_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    subSession->GetSessionProperty()->SetDisplayId(startDisplayId);
    sceneSession->GetMoveDragController()->moveDragStartDisplayId_ = startDisplayId;
    sceneSession->GetMoveDragController()->moveDragEndDisplayId_ = startDisplayId;
    subSession->SetShouldFollowParentWhenShow(true);
    sceneSession->HandleMoveDragEnd(rect, SizeChangeReason::UNDEFINED);
    EXPECT_EQ(subSession->GetShouldFollowParentWhenShow(), true);

    // Case2: Not supported across screen
    sceneSession->GetSessionProperty()->SetDisplayId(startDisplayId);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_DESKTOP);
    sceneSession->GetMoveDragController()->winType_ = WindowType::WINDOW_TYPE_DESKTOP;
    subSession->GetSessionProperty()->SetDisplayId(startDisplayId);
    sceneSession->GetMoveDragController()->moveDragStartDisplayId_ = startDisplayId;
    sceneSession->GetMoveDragController()->moveDragEndDisplayId_ = endDisplayId;
    subSession->SetShouldFollowParentWhenShow(true);
    sceneSession->HandleMoveDragEnd(rect, SizeChangeReason::UNDEFINED);
    EXPECT_EQ(subSession->GetShouldFollowParentWhenShow(), true);

    // Case3: Support cross screen and cross screen
    sceneSession->GetSessionProperty()->SetDisplayId(startDisplayId);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->GetMoveDragController()->winType_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    subSession->GetSessionProperty()->SetDisplayId(startDisplayId);
    sceneSession->GetMoveDragController()->moveDragStartDisplayId_ = startDisplayId;
    sceneSession->GetMoveDragController()->moveDragEndDisplayId_ = endDisplayId;
    subSession->SetShouldFollowParentWhenShow(true);
    sceneSession->HandleMoveDragEnd(rect, SizeChangeReason::UNDEFINED);
    EXPECT_EQ(subSession->GetShouldFollowParentWhenShow(), false);

    // Case4: Not cross screen and not supported across screen
    sceneSession->GetSessionProperty()->SetDisplayId(startDisplayId);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_DESKTOP);
    sceneSession->GetMoveDragController()->winType_ = WindowType::WINDOW_TYPE_DESKTOP;
    subSession->GetSessionProperty()->SetDisplayId(startDisplayId);
    sceneSession->GetMoveDragController()->moveDragStartDisplayId_ = startDisplayId;
    sceneSession->GetMoveDragController()->moveDragEndDisplayId_ = startDisplayId;
    subSession->SetShouldFollowParentWhenShow(true);
    sceneSession->HandleMoveDragEnd(rect, SizeChangeReason::UNDEFINED);
    EXPECT_EQ(subSession->GetShouldFollowParentWhenShow(), true);
}

/**
 * @tc.name: SetMoveAvailableArea01
 * @tc.desc: SetMoveAvailableArea01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SetMoveAvailableArea01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetMoveAvailableArea01";
    info.bundleName_ = "SetMoveAvailableArea01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    WSError res = sceneSession->SetMoveAvailableArea(-1);
    EXPECT_EQ(res, WSError::WS_ERROR_INVALID_DISPLAY);

    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    int32_t screenId = 0;
    sceneSession->SetScreenId(screenId);
    res = sceneSession->SetMoveAvailableArea(0);
    EXPECT_EQ(res, WSError::WS_OK);

    sptr<ScreenSession> currentScreenSession =
        ScreenSessionManager::GetInstance().GetOrCreateScreenSession(0);
    ASSERT_NE(currentScreenSession, nullptr);
    uint32_t currentScreenHeight = currentScreenSession->GetScreenProperty().GetBounds().rect_.height_;
    DMRect statusBarRect = sceneSession->CalcRectForStatusBar();
    int32_t dockHeight = sceneSession->GetDockHeight();
    if (PcFoldScreenManager::GetInstance().IsHalfFolded(screenId)) {
        sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
        DMRect availableArea;
        DMError ret = DisplayManager::GetInstance().GetExpandAvailableArea(
            sceneSession->GetSessionProperty()->GetDisplayId(), availableArea);
        if (ret != DMError::DM_OK) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to get available area, ret: %{public}d", ret);
        }
        EXPECT_EQ(sceneSession->moveDragController_->moveAvailableArea_, availableArea);
        EXPECT_EQ(sceneSession->moveDragController_->moveAvailableArea_.height_,
            currentScreenHeight - statusBarRect.height_ - dockHeight);
    }
}

/**
 * @tc.name: SetMoveAvailableArea02
 * @tc.desc: SetMoveAvailableArea02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SetMoveAvailableArea02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetMoveAvailableArea02";
    info.bundleName_ = "SetMoveAvailableArea02";
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);

    WSRect rect({0, 0, 50, 50});
    sceneSession->GetLayoutController()->SetSessionRect(rect);
    sceneSession->specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ =
        [&](WindowType type, uint64_t displayId) -> std::vector<sptr<SceneSession>> {
        std::vector<sptr<SceneSession>> vec;
        vec.push_back(sceneSession);
        return vec;
    };

    SystemSessionConfig systemConfig;
    int32_t screenId = 0;
    sceneSession->SetScreenId(screenId);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionProperty(property);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sceneSession->SetSystemConfig(systemConfig);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    WSError res = sceneSession->SetMoveAvailableArea(0);
    EXPECT_EQ(res, WSError::WS_OK);

    systemConfig.windowUIType_ = WindowUIType::PC_WINDOW;
    sceneSession->SetSystemConfig(systemConfig);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    res = sceneSession->SetMoveAvailableArea(0);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: GetWindowDragMoveMountedNode01
 * @tc.desc: GetWindowDragMoveMountedNode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, GetWindowDragMoveMountedNode01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetWindowDragMoveMountedNode";
    info.bundleName_ = "GetWindowDragMoveMountedNode";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto rsNode = sceneSession->GetWindowDragMoveMountedNode(std::numeric_limits<uint32_t>::max(), 0);
    EXPECT_EQ(rsNode, nullptr);
    sceneSession->SetFindScenePanelRsNodeByZOrderFunc([this](uint64_t screenId, uint32_t targetZOrder) {
        return CreateRSSurfaceNode();
    });
    rsNode = sceneSession->GetWindowDragMoveMountedNode(0, 0);
    EXPECT_NE(rsNode, nullptr);
}

/**
 * @tc.name: ShouldSkipUpdateRect
 * @tc.desc: ShouldSkipUpdateRect function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ShouldSkipUpdateRect, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "ShouldSkipUpdateRect";
    info.bundleName_ = "ShouldSkipUpdateRect";
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->UpdateSizeChangeReason(SizeChangeReason::MAXIMIZE);
    auto property = session->GetSessionProperty();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session->SetSessionRect({ 0, 0, 800, 800});
    WSRect rect = { 0, 0, 800, 800 };

    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    session->sessionStage_ = mockSessionStage;

    session->UpdateSizeChangeReason(SizeChangeReason::DRAG);
    EXPECT_EQ(true, session->ShouldSkipUpdateRect(rect));

    session->UpdateSizeChangeReason(SizeChangeReason::DRAG_END);
    EXPECT_EQ(false, session->ShouldSkipUpdateRect(rect));

    property->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    EXPECT_EQ(false, session->ShouldSkipUpdateRect(rect));

    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    EXPECT_EQ(false, session->ShouldSkipUpdateRect(rect));

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session->UpdateSizeChangeReason(SizeChangeReason::MOVE);
    session->SetClientRect({ 0, 0, 800, 800 });
    session->sessionStage_ = nullptr;
    EXPECT_EQ(true, session->ShouldSkipUpdateRect(rect));
    session->sessionStage_ = mockSessionStage;
    EXPECT_EQ(true, session->ShouldSkipUpdateRect(rect));
    session->SetClientRect({ 0, 0, 1000, 1000 });
    EXPECT_EQ(false, session->ShouldSkipUpdateRect(rect));

    session->SetSessionRect({ 0, 0, 1200, 1200 });
    session->sessionStage_ = nullptr;
    EXPECT_EQ(false, session->ShouldSkipUpdateRect(rect));
    session->sessionStage_ = mockSessionStage;
    EXPECT_EQ(false, session->ShouldSkipUpdateRect(rect));
}
 
/**
 * @tc.name: ShouldSkipUpdateRectNotify
 * @tc.desc: ShouldSkipUpdateRectNotify function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ShouldSkipUpdateRectNotify, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "ShouldSkipUpdateRectNotify";
    info.bundleName_ = "ShouldSkipUpdateRectNotify";
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->UpdateSizeChangeReason(SizeChangeReason::MAXIMIZE);
    session->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session->SetSessionRect({ 0, 0, 800, 800});
    WSRect rect = { 0, 0, 800, 800 };

    session->rectChangeListenerRegistered_ = false;
    EXPECT_EQ(true, session->ShouldSkipUpdateRectNotify(rect));
    session->rectChangeListenerRegistered_ = true;
    EXPECT_EQ(true, session->ShouldSkipUpdateRectNotify(rect));

    session->UpdateSizeChangeReason(SizeChangeReason::DRAG_MOVE);
    session->rectChangeListenerRegistered_ = false;
    EXPECT_EQ(true, session->ShouldSkipUpdateRectNotify(rect));
    session->rectChangeListenerRegistered_ = true;
    EXPECT_EQ(false, session->ShouldSkipUpdateRectNotify(rect));

    session->UpdateSizeChangeReason(SizeChangeReason::MAXIMIZE);
    session->SetSessionRect({ 0, 0, 1000, 800});
    EXPECT_EQ(false, session->ShouldSkipUpdateRectNotify(rect));
    session->SetSessionRect({ 0, 0, 800, 1000});
    EXPECT_EQ(false, session->ShouldSkipUpdateRectNotify(rect));
    session->SetSessionRect({ 0, 0, 800, 800});
    EXPECT_EQ(true, session->ShouldSkipUpdateRectNotify(rect));
}
/**
 * @tc.name: SyncAllAttachedLimitsToAttachingChild01
 * @tc.desc: Test SyncAllAttachedLimitsToAttachingChild with null parentSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SyncAllAttachedLimitsToAttachingChild01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SyncAllAttachedLimitsToAttachingChild01";
    info.bundleName_ = "SyncAllAttachedLimitsToAttachingChild01";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = mockSessionStage_;

    // Call with null parent - should return early
    session->SyncAllAttachedLimitsToAttachingChild(nullptr);

    // Verify no IPC call was made
    auto limitsList = session->GetSessionProperty()->GetAttachedWindowLimitsList();
    EXPECT_TRUE(limitsList.empty());
}

/**
 * @tc.name: SyncAllAttachedLimitsToAttachingChild02
 * @tc.desc: Test SyncAllAttachedLimitsToAttachingChild with null sessionStage
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SyncAllAttachedLimitsToAttachingChild02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SyncAllAttachedLimitsToAttachingChild02";
    info.bundleName_ = "SyncAllAttachedLimitsToAttachingChild02";

    sptr<MainSession> parentSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> childSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    // Don't set sessionStage_ - it stays null

    // Call with null sessionStage - should return early
    childSession->SyncAllAttachedLimitsToAttachingChild(parentSession);

    // Verify no crash
    auto limitsList = childSession->GetSessionProperty()->GetAttachedWindowLimitsList();
    EXPECT_TRUE(limitsList.empty());
}

/**
 * @tc.name: SyncAllAttachedLimitsToAttachingChild03
 * @tc.desc: Test SyncAllAttachedLimitsToAttachingChild with parent that has no attached limits
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SyncAllAttachedLimitsToAttachingChild03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SyncAllAttachedLimitsToAttachingChild03";
    info.bundleName_ = "SyncAllAttachedLimitsToAttachingChild03";

    // Create parent with limits for attached windows
    sptr<MainSession> parentSession = sptr<MainSession>::MakeSptr(info, nullptr);
    WindowLimits parentLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    parentSession->GetSessionProperty()->SetLimitsForAttachedWindows(parentLimits);

    // Create child with mock sessionStage
    sptr<SubSession> childSession = sptr<SubSession>::MakeSptr(info, nullptr);
    sptr<SessionStageMocker> childMockStage = sptr<SessionStageMocker>::MakeSptr();
    childSession->sessionStage_ = childMockStage;

    // Expect IPC call with only parent's own limits (no other attached windows)
    EXPECT_CALL(*childMockStage, SyncAllAttachedLimitsToChild(
        testing::SizeIs(1), testing::SizeIs(1)))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    childSession->SyncAllAttachedLimitsToAttachingChild(parentSession);
}

/**
 * @tc.name: SyncAllAttachedLimitsToAttachingChild04
 * @tc.desc: Test SyncAllAttachedLimitsToAttachingChild with parent that has existing attached limits
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SyncAllAttachedLimitsToAttachingChild04, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SyncAllAttachedLimitsToAttachingChild04";
    info.bundleName_ = "SyncAllAttachedLimitsToAttachingChild04";

    // Create parent with limits for attached windows
    sptr<MainSession> parentSession = sptr<MainSession>::MakeSptr(info, nullptr);
    WindowLimits parentLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    parentSession->GetSessionProperty()->SetLimitsForAttachedWindows(parentLimits);

    // Simulate parent already has an attached sub-window's limits
    WindowLimits subLimits = { 1500, 800, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    parentSession->GetSessionProperty()->SetAttachedWindowLimits(500, subLimits);
    parentSession->GetSessionProperty()->SetAttachedLimitOptions(500, AttachLimitOptions{ true, false });

    // Create child with mock sessionStage
    sptr<SubSession> childSession = sptr<SubSession>::MakeSptr(info, nullptr);
    sptr<SessionStageMocker> childMockStage = sptr<SessionStageMocker>::MakeSptr();
    childSession->sessionStage_ = childMockStage;

    // Expect IPC call with parent's limits + 1 other attached window = 2 entries
    EXPECT_CALL(*childMockStage, SyncAllAttachedLimitsToChild(
        testing::SizeIs(2), testing::SizeIs(2)))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    childSession->SyncAllAttachedLimitsToAttachingChild(parentSession);
}

/**
 * @tc.name: SyncAllAttachedLimitsToAttachingChild05
 * @tc.desc: Test SyncAllAttachedLimitsToAttachingChild with multiple attached sub-windows
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, SyncAllAttachedLimitsToAttachingChild05, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SyncAllAttachedLimitsToAttachingChild05";
    info.bundleName_ = "SyncAllAttachedLimitsToAttachingChild05";

    // Create parent with limits for attached windows
    sptr<MainSession> parentSession = sptr<MainSession>::MakeSptr(info, nullptr);
    WindowLimits parentLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    parentSession->GetSessionProperty()->SetLimitsForAttachedWindows(parentLimits);

    // Simulate parent already has 2 attached sub-windows
    WindowLimits subLimits1 = { 1500, 800, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WindowLimits subLimits2 = { 1800, 900, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    parentSession->GetSessionProperty()->SetAttachedWindowLimits(501, subLimits1);
    parentSession->GetSessionProperty()->SetAttachedLimitOptions(501, AttachLimitOptions{ true, false });
    parentSession->GetSessionProperty()->SetAttachedWindowLimits(502, subLimits2);
    parentSession->GetSessionProperty()->SetAttachedLimitOptions(502, AttachLimitOptions{ false, true });

    // Create child with mock sessionStage
    sptr<SubSession> childSession = sptr<SubSession>::MakeSptr(info, nullptr);
    sptr<SessionStageMocker> childMockStage = sptr<SessionStageMocker>::MakeSptr();
    childSession->sessionStage_ = childMockStage;

    // Expect IPC call with parent's limits + 2 other attached windows = 3 entries
    EXPECT_CALL(*childMockStage, SyncAllAttachedLimitsToChild(
        testing::SizeIs(3), testing::SizeIs(3)))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    childSession->SyncAllAttachedLimitsToAttachingChild(parentSession);
}

/**
 * @tc.name: ResetAttachBindingState01
 * @tc.desc: Reset attach state and verify all fields are cleared
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ResetAttachBindingState01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ResetAttachBindingState01";
    info.bundleName_ = "ResetAttachBindingState01";
    sptr<SubSession> session = sptr<SubSession>::MakeSptr(info, nullptr);

    // Set anchor state
    session->windowAnchorInfo_.isAnchorEnabled_ = true;
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;
    session->windowAnchorInfo_.isFromAttachOrDetach_ = true;
    session->windowAnchorInfo_.windowAnchor_ = WindowAnchor::BOTTOM_END;
    session->windowAnchorInfo_.offsetX_ = 100;
    session->windowAnchorInfo_.offsetY_ = 200;
    session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit = true;
    session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit = true;

    session->ResetAttachBindingState();

    EXPECT_FALSE(session->windowAnchorInfo_.isAnchorEnabled_);
    EXPECT_FALSE(session->windowAnchorInfo_.isAnchoredByAttach_);
    EXPECT_TRUE(session->windowAnchorInfo_.isFromAttachOrDetach_);
    EXPECT_EQ(session->windowAnchorInfo_.windowAnchor_, WindowAnchor::TOP_START);
    EXPECT_EQ(session->windowAnchorInfo_.offsetX_, 0);
    EXPECT_EQ(session->windowAnchorInfo_.offsetY_, 0);
    EXPECT_FALSE(session->windowAnchorInfo_.attachOptions.isIntersectedWidthLimit);
    EXPECT_FALSE(session->windowAnchorInfo_.attachOptions.isIntersectedHeightLimit);
}

/**
 * @tc.name: ResetAttachBindingState02
 * @tc.desc: ResetAttachBindingState triggers onWindowAnchorInfoChangeFunc_ callback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, ResetAttachBindingState02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ResetAttachBindingState02";
    info.bundleName_ = "ResetAttachBindingState02";
    sptr<SubSession> session = sptr<SubSession>::MakeSptr(info, nullptr);

    session->windowAnchorInfo_.isAnchorEnabled_ = true;
    session->windowAnchorInfo_.isAnchoredByAttach_ = true;

    std::shared_ptr<bool> callbackFired = std::make_shared<bool>(false);
    session->onWindowAnchorInfoChangeFunc_ = [callbackFired](const WindowAnchorInfo& info) {
        *callbackFired = true;
        EXPECT_FALSE(info.isAnchorEnabled_);
        EXPECT_FALSE(info.isAnchoredByAttach_);
        EXPECT_TRUE(info.isFromAttachOrDetach_);
    };

    session->ResetAttachBindingState();
    EXPECT_TRUE(*callbackFired);
}

/**
 * @tc.name: NotifyRebindAttachAfterParentChange01
 * @tc.desc: NotifyRebindAttachAfterParentChange with null sessionStage_ → early return
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRebindAttachAfterParentChange01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRebindAttachAfterParentChange01";
    info.bundleName_ = "NotifyRebindAttachAfterParentChange01";
    sptr<SubSession> session = sptr<SubSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);

    // sessionStage_ is null by default → should return early without crash
    session->NotifyRebindAttachAfterParentChange(100);
}

/**
 * @tc.name: NotifyRebindAttachAfterParentChange02
 * @tc.desc: NotifyRebindAttachAfterParentChange calls sessionStage_ IPC
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionLayoutTest, NotifyRebindAttachAfterParentChange02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRebindAttachAfterParentChange02";
    info.bundleName_ = "NotifyRebindAttachAfterParentChange02";
    sptr<SubSession> session = sptr<SubSession>::MakeSptr(info, nullptr);

    sptr<SessionStageMocker> mockStage = sptr<SessionStageMocker>::MakeSptr();
    session->sessionStage_ = mockStage;

    EXPECT_CALL(*mockStage, NotifyRebindAttachAfterParentChange(200))
        .Times(1).WillOnce(testing::Return(WSError::WS_OK));

    session->NotifyRebindAttachAfterParentChange(200);
}

} // namespace
} // namespace Rosen
} // namespace OHOS