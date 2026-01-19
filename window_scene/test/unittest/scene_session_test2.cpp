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

#include "window_helper.h"
#include "display_manager.h"
#include "pointer_event.h"
#include "proxy/include/window_info.h"

#include <gtest/gtest.h>
#include "key_event.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/sub_session.h"
#include "session/host/include/system_session.h"
#include "session/host/include/main_session.h"
#include "wm_common.h"
#include "mock/mock_session_stage.h"
#include "mock_vsync_station.h"
#include "input_event.h"
#include <pointer_event.h>
#include "ui/rs_surface_node.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class SceneSessionTest2 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionTest2::SetUpTestCase() {}

void SceneSessionTest2::TearDownTestCase() {}

void SceneSessionTest2::SetUp() {}

void SceneSessionTest2::TearDown() {}

namespace {
/**
 * @tc.name: RaiseAboveTarget
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, RaiseAboveTarget, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RaiseAboveTarget";
    info.bundleName_ = "RaiseAboveTarget";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    property->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;

    sceneSession->SetSessionProperty(property);
    WSError result = sceneSession->RaiseAboveTarget(0);
    ASSERT_EQ(result, WSError::WS_OK);

    sptr<SceneSession> tempSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    int32_t persistentId = 1;
    tempSession->persistentId_ = persistentId;
    int32_t callingPid = 2;
    tempSession->callingPid_ = callingPid;
    sceneSession->subSession_.push_back(tempSession);
    int32_t subWindowId = 1;
    result = sceneSession->RaiseAboveTarget(subWindowId);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_CALLING);

    sceneSession->onRaiseAboveTarget_ = nullptr;
    result = sceneSession->RaiseAboveTarget(0);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: RaiseMainWindowAboveTarget
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, RaiseMainWindowAboveTarget, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RaiseMainWindowAboveTarget";
    info.bundleName_ = "RaiseMainWindowAboveTarget";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);

    sptr<SceneSession> sourceSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sourceSession, nullptr);

    sourceSession->isActive_ = true;
    sourceSession->callingPid_ = 1;
    sourceSession->zOrder_ = 2001;
    sptr<WindowSessionProperty> property1 = sptr<WindowSessionProperty>::MakeSptr();
    property1->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    property1->SetDisplayId(0);
    sourceSession->SetSessionProperty(property1);

    auto result = sourceSession->RaiseMainWindowAboveTarget(0);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);

    sptr<SceneSession> targetSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(targetSession, nullptr);

    targetSession->isActive_ = true;
    targetSession->callingPid_ = 2;
    targetSession->zOrder_ = 102;
    sptr<WindowSessionProperty> property2 = sptr<WindowSessionProperty>::MakeSptr();
    property2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    property2->SetDisplayId(1);
    targetSession->SetSessionProperty(property2);

    sptr<SceneSession::SpecificSessionCallback> callBack =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, callBack);
    sourceSession->specificCallback_ = callBack;
    auto task = [&targetSession](int32_t persistentId) { return targetSession; };
    callBack->onGetSceneSessionByIdCallback_ = task;
    result = sourceSession->RaiseMainWindowAboveTarget(0);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_CALLING);

    targetSession->callingPid_ = 1;
    result = sourceSession->RaiseMainWindowAboveTarget(0);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_CALLING);

    property2->SetDisplayId(0);
    targetSession->SetSessionProperty(property2);
    result = sourceSession->RaiseMainWindowAboveTarget(0);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_CALLING);
}

/**
 * @tc.name: NotifyPropertyWhenConnect1
 * @tc.desc: NotifyPropertyWhenConnect1
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, NotifyPropertyWhenConnect1, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ability";
    info.bundleName_ = "bundle";
    info.moduleName_ = "module";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    std::string key = info.bundleName_ + info.moduleName_ + info.abilityName_;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    sceneSession->NotifyPropertyWhenConnect();
}

/**
 * @tc.name: GetSystemAvoidArea
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, GetSystemAvoidArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetSystemAvoidArea";
    info.bundleName_ = "GetSystemAvoidArea";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    int32_t p = 10;
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID));

    sceneSession->SetSessionProperty(property);
    WSRect rect({ 1, 1, 1, 1 });
    AvoidArea avoidArea;
    sceneSession->GetSystemAvoidArea(rect, avoidArea);
    ASSERT_EQ(p, 10);
    sceneSession->GetSystemAvoidArea(rect, avoidArea, true);
    ASSERT_EQ(p, 10);
}

/**
 * @tc.name: TransferPointerEvent
 * @tc.desc: TransferPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, TransferPointerEvent01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    ASSERT_EQ(sceneSession->TransferPointerEvent(pointerEvent), WSError::WS_OK);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->SetMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    property->SetPersistentId(11);
    sceneSession->property_ = property;

    std::shared_ptr<MMI::PointerEvent> pointerEvent_ = MMI::PointerEvent::Create();
    pointerEvent_->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW);
    ASSERT_EQ(sceneSession->TransferPointerEvent(pointerEvent_), WSError::WS_OK);

    sceneSession->sessionInfo_.isSystem_ = true;
    pointerEvent_->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    ASSERT_EQ(sceneSession->TransferPointerEvent(pointerEvent_), WSError::WS_OK);

    pointerEvent_->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW);
    ASSERT_EQ(sceneSession->TransferPointerEvent(pointerEvent_), WSError::WS_OK);

    sceneSession->sessionInfo_.isSystem_ = false;
    pointerEvent_->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    ASSERT_EQ(sceneSession->TransferPointerEvent(pointerEvent_), WSError::WS_OK);

    pointerEvent_->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW);
    ASSERT_EQ(sceneSession->TransferPointerEvent(pointerEvent_), WSError::WS_OK);
}

/**
 * @tc.name: TransferPointerEvent
 * @tc.desc: TransferPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, TransferPointerEvent02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsSubWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    ASSERT_EQ(sceneSession->TransferPointerEvent(pointerEvent), WSError::WS_OK);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->SetMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->SetPersistentId(11);
    sceneSession->property_ = property;

    std::shared_ptr<MMI::PointerEvent> pointerEvent_ = MMI::PointerEvent::Create();
    pointerEvent_->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    ASSERT_EQ(sceneSession->TransferPointerEvent(pointerEvent_), WSError::WS_OK);
}

/**
 * @tc.name: RequestSessionBack
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, RequestSessionBack, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestSessionBack";
    info.bundleName_ = "RequestSessionBack";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    property->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    sceneSession->SetSessionProperty(property);

    WSError result = sceneSession->RequestSessionBack(true);
    ASSERT_EQ(result, WSError::WS_OK);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    EXPECT_NE(nullptr, surfaceNode);
    sceneSession->SetLeashWinSurfaceNode(surfaceNode);
    result = sceneSession->RequestSessionBack(true);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetParentPersistentId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetParentPersistentId, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetParentPersistentId";
    info.bundleName_ = "SetParentPersistentId";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    sceneSession->SetParentPersistentId(0);
    int32_t result = sceneSession->GetParentPersistentId();
    ASSERT_EQ(result, 0);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);

    sceneSession->SetSessionProperty(property);
    sceneSession->SetParentPersistentId(0);
    result = sceneSession->GetParentPersistentId();
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: GetMainSessionId
 * @tc.desc: GetMainSessionId Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, GetMainSessionId, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetMainSessionId";
    info.moduleName_ = "GetMainSessionId";
    info.bundleName_ = "GetMainSessionId";

    sptr<Session> session = sptr<Session>::MakeSptr(info);
    EXPECT_NE(session, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    property->SetPersistentId(0);
    session->SetSessionProperty(property);

    sptr<Session> subSession = sptr<Session>::MakeSptr(info);
    EXPECT_NE(subSession, nullptr);
    subSession->SetParentSession(session);
    sptr<WindowSessionProperty> subProperty = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(subProperty, nullptr);
    subProperty->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subProperty->SetPersistentId(1);
    subSession->SetSessionProperty(subProperty);

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->SetParentSession(subSession);
    sptr<WindowSessionProperty> sceneProperty = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(sceneProperty, nullptr);
    sceneProperty->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sceneProperty->SetPersistentId(2);
    sceneSession->SetSessionProperty(sceneProperty);
    auto result = sceneSession->GetMainSessionId();
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: UpdateSizeChangeReason01
 * @tc.desc: UpdateSizeChangeReason01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, UpdateSizeChangeReason01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateSizeChangeReason01";
    info.bundleName_ = "UpdateSizeChangeReason01";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    sceneSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    sceneSession->sessionStage_ = mockSessionStage;
    sceneSession->UpdateSizeChangeReason(SizeChangeReason::ROTATION);
    ASSERT_EQ(sceneSession->GetSizeChangeReason(), SizeChangeReason::ROTATION);
    ASSERT_EQ(WSError::WS_OK, sceneSession->UpdateSizeChangeReason(SizeChangeReason::UNDEFINED));
}

/**
 * @tc.name: UpdatePiPRect01
 * @tc.desc: UpdatePiPRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, UpdatePiPRect, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdatePiPRect01";
    info.bundleName_ = "UpdatePiPRect";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    sceneSession->SetSessionProperty(property);

    Rect rect = { 0, 0, 800, 600 };
    SizeChangeReason reason = SizeChangeReason::PIP_START;
    WSError result = sceneSession->UpdatePiPRect(rect, reason);
    ASSERT_EQ(result, WSError::WS_OK);

    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sceneSession->SetSessionProperty(property);
    result = sceneSession->UpdatePiPRect(rect, reason);
    ASSERT_EQ(result, WSError::WS_DO_NOTHING);

    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    sceneSession->SetSessionProperty(property);
    sceneSession->isTerminating_ = true;
    result = sceneSession->UpdatePiPRect(rect, reason);
    ASSERT_EQ(result, WSError::WS_OK);

    sceneSession->isTerminating_ = false;
    result = sceneSession->UpdatePiPRect(rect, reason);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdatePiPControlStatus
 * @tc.desc: UpdatePiPControlStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, UpdatePiPControlStatus, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdatePiPControlStatus";
    info.bundleName_ = "UpdatePiPControlStatus";
    auto sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    auto property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    sceneSession->SetSessionProperty(property);

    auto controlType = WsPiPControlType::VIDEO_PLAY_PAUSE;
    auto status = WsPiPControlStatus::PLAY;
    WSError result = sceneSession->UpdatePiPControlStatus(controlType, status);
    ASSERT_EQ(result, WSError::WS_OK);

    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sceneSession->SetSessionProperty(property);
    result = sceneSession->UpdatePiPControlStatus(controlType, status);
    ASSERT_EQ(result, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: SetScale
 * @tc.desc: SetScale
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetScale, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetScale";
    info.bundleName_ = "SetScale";
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->SetScale(1.0f, 1.0f, 0.0f, 0.0f);
}

/**
 * @tc.name: SetIsDisplayStatusBarTemporarily
 * @tc.desc: SetIsDisplayStatusBarTemporarily
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetIsDisplayStatusBarTemporarily, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetIsDisplayStatusBarTemporarily";
    info.bundleName_ = "SetIsDisplayStatusBarTemporarily";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->SetIsDisplayStatusBarTemporarily(true);
    ASSERT_EQ(true, sceneSession->GetIsDisplayStatusBarTemporarily());
    sceneSession->SetIsDisplayStatusBarTemporarily(false);
    ASSERT_EQ(false, sceneSession->GetIsDisplayStatusBarTemporarily());
}

/**
 * @tc.name: UpdateAvoidArea
 * @tc.desc: UpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, UpdateAvoidArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateAvoidArea";
    info.bundleName_ = "UpdateAvoidArea";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->sessionStage_ = nullptr;
    WSError result = sceneSession->UpdateAvoidArea(nullptr, AvoidAreaType::TYPE_SYSTEM);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, result);

    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, sceneSession->sessionStage_);
    result = sceneSession->UpdateAvoidArea(nullptr, AvoidAreaType::TYPE_SYSTEM);
    EXPECT_EQ(WSError::WS_OK, result);
    sceneSession->foregroundInteractiveStatus_.store(false);
    result = sceneSession->UpdateAvoidArea(nullptr, AvoidAreaType::TYPE_SYSTEM);
    EXPECT_EQ(WSError::WS_DO_NOTHING, result);
}

/**
 * @tc.name: ChangeSessionVisibilityWithStatusBar
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, ChangeSessionVisibilityWithStatusBar, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ChangeSessionVisibilityWithStatusBar";
    info.bundleName_ = "ChangeSessionVisibilityWithStatusBar";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    sptr<AAFwk::SessionInfo> info1 = nullptr;
    WSError result = sceneSession->ChangeSessionVisibilityWithStatusBar(info1, false);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);

    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    result = sceneSession->ChangeSessionVisibilityWithStatusBar(abilitySessionInfo, false);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);

    sceneSession->isTerminating_ = true;
    sptr<AAFwk::SessionInfo> terminatingMock = sptr<AAFwk::SessionInfo>::MakeSptr();
    result = sceneSession->ChangeSessionVisibilityWithStatusBar(abilitySessionInfo, false);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetShouldHideNonSecureWindows
 * @tc.desc: SetShouldHideNonSecureWindows
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetShouldHideNonSecureWindows, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetShouldHideNonSecureWindows";
    info.bundleName_ = "SetShouldHideNonSecureWindows";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    EXPECT_FALSE(sceneSession->shouldHideNonSecureWindows_.load());
    sceneSession->SetShouldHideNonSecureWindows(true);
    EXPECT_TRUE(sceneSession->shouldHideNonSecureWindows_.load());
}

/**
 * @tc.name: UpdateExtWindowFlags
 * @tc.desc: update uiextension window flags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, UpdateExtWindowFlags, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateExtWindowFlags";
    info.bundleName_ = "UpdateExtWindowFlags";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    EXPECT_TRUE(sceneSession->extWindowFlagsMap_.empty());
    int32_t persistentId = 12345;
    ExtensionWindowFlags flags(7);
    ExtensionWindowFlags actions(7);
    sceneSession->UpdateExtWindowFlags(persistentId, flags, actions);
    EXPECT_EQ(sceneSession->extWindowFlagsMap_.size(), 1);
    EXPECT_EQ(sceneSession->extWindowFlagsMap_.begin()->first, persistentId);
    EXPECT_EQ(sceneSession->extWindowFlagsMap_.begin()->second.bitData, 7);
    flags.bitData = 0;
    actions.bitData = 3;
    sceneSession->UpdateExtWindowFlags(persistentId, flags, actions);
    EXPECT_EQ(sceneSession->extWindowFlagsMap_.size(), 1);
    EXPECT_EQ(sceneSession->extWindowFlagsMap_.begin()->first, persistentId);
    EXPECT_EQ(sceneSession->extWindowFlagsMap_.begin()->second.bitData, 4);
    actions.bitData = 4;
    sceneSession->UpdateExtWindowFlags(persistentId, flags, actions);
    EXPECT_TRUE(sceneSession->extWindowFlagsMap_.empty());
}

/**
 * @tc.name: GetCombinedExtWindowFlags
 * @tc.desc: get combined uiextension window flags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, GetCombinedExtWindowFlags, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetCombinedExtWindowFlags";
    info.bundleName_ = "GetCombinedExtWindowFlags";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    auto combinedExtWindowFlags = sceneSession->GetCombinedExtWindowFlags();
    EXPECT_EQ(combinedExtWindowFlags.bitData, 0);
    sceneSession->UpdateExtWindowFlags(1234, ExtensionWindowFlags(3), ExtensionWindowFlags(3));
    sceneSession->UpdateExtWindowFlags(5678, ExtensionWindowFlags(4), ExtensionWindowFlags(4));

    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    combinedExtWindowFlags = sceneSession->GetCombinedExtWindowFlags();
    EXPECT_EQ(combinedExtWindowFlags.bitData, 7);

    sceneSession->state_ = SessionState::STATE_BACKGROUND;
    combinedExtWindowFlags = sceneSession->GetCombinedExtWindowFlags();
    EXPECT_EQ(combinedExtWindowFlags.bitData, 6);
}

/**
 * @tc.name: RemoveExtWindowFlags
 * @tc.desc: remove uiextension window flags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, RemoveExtWindowFlags, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RemoveExtWindowFlags";
    info.bundleName_ = "RemoveExtWindowFlags";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    EXPECT_TRUE(sceneSession->extWindowFlagsMap_.empty());
    sceneSession->UpdateExtWindowFlags(1234, ExtensionWindowFlags(3), ExtensionWindowFlags(3));
    EXPECT_EQ(sceneSession->extWindowFlagsMap_.size(), 1);
    sceneSession->RemoveExtWindowFlags(1234);
    EXPECT_TRUE(sceneSession->extWindowFlagsMap_.empty());
}

/**
 * @tc.name: ClearExtWindowFlags
 * @tc.desc: clear uiextension window flags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, ClearExtWindowFlags, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ClearExtWindowFlags";
    info.bundleName_ = "ClearExtWindowFlags";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    EXPECT_TRUE(sceneSession->extWindowFlagsMap_.empty());
    sceneSession->UpdateExtWindowFlags(1234, ExtensionWindowFlags(3), ExtensionWindowFlags(3));
    sceneSession->UpdateExtWindowFlags(5678, ExtensionWindowFlags(4), ExtensionWindowFlags(4));
    EXPECT_EQ(sceneSession->extWindowFlagsMap_.size(), 2);
    sceneSession->ClearExtWindowFlags();
    EXPECT_TRUE(sceneSession->extWindowFlagsMap_.empty());
}

/**
 * @tc.name: CalculateCombinedExtWindowFlags
 * @tc.desc: calculate combined uiextension window flags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, CalculateCombinedExtWindowFlags, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CalculateCombinedExtWindowFlags";
    info.bundleName_ = "CalculateCombinedExtWindowFlags";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    EXPECT_EQ(sceneSession->combinedExtWindowFlags_.bitData, 0);
    sceneSession->UpdateExtWindowFlags(1234, ExtensionWindowFlags(3), ExtensionWindowFlags(3));
    sceneSession->UpdateExtWindowFlags(5678, ExtensionWindowFlags(4), ExtensionWindowFlags(4));
    sceneSession->CalculateCombinedExtWindowFlags();
    EXPECT_EQ(sceneSession->combinedExtWindowFlags_.bitData, 7);
}

/**
 * @tc.name: SaveUpdatedIcon
 * @tc.desc: SaveUpdatedIcon
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SaveUpdatedIcon, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SaveUpdatedIcon";
    info.bundleName_ = "SaveUpdatedIcon";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->scenePersistence_ = sptr<ScenePersistence>::MakeSptr("OpenHarmony", 1);
    EXPECT_NE(nullptr, sceneSession->scenePersistence_);
    sceneSession->SaveUpdatedIcon(nullptr);
}

/**
 * @tc.name: NotifyTouchOutside
 * @tc.desc: NotifyTouchOutside
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, NotifyTouchOutside, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyTouchOutside";
    info.bundleName_ = "NotifyTouchOutside";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, sceneSession->sessionStage_);
    sceneSession->NotifyTouchOutside();

    auto func = [sceneSession]() { sceneSession->SaveUpdatedIcon(nullptr); };
    sceneSession->onTouchOutside_ = func;
    EXPECT_NE(nullptr, &func);
    sceneSession->sessionStage_ = nullptr;
    sceneSession->NotifyTouchOutside();

    sceneSession->onTouchOutside_ = nullptr;
    sceneSession->sessionStage_ = nullptr;
    sceneSession->NotifyTouchOutside();
}

/**
 * @tc.name: CheckTouchOutsideCallbackRegistered
 * @tc.desc: CheckTouchOutsideCallbackRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, CheckTouchOutsideCallbackRegistered, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CheckTouchOutsideCallbackRegistered";
    info.bundleName_ = "CheckTouchOutsideCallbackRegistered";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    auto func = [sceneSession]() { sceneSession->NotifyWindowVisibility(); };
    sceneSession->onTouchOutside_ = func;
    bool result = sceneSession->CheckTouchOutsideCallbackRegistered();
    EXPECT_EQ(true, result);

    sceneSession->onTouchOutside_ = nullptr;
    result = sceneSession->CheckTouchOutsideCallbackRegistered();
    EXPECT_EQ(false, result);
}

/**
 * @tc.name: RegisterTouchOutsideCallback
 * @tc.desc: test RegisterTouchOutsideCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, RegisterTouchOutsideCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RegisterTouchOutsideCallback";
    info.bundleName_ = "RegisterTouchOutsideCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->onTouchOutside_ = nullptr;
    NotifyTouchOutsideFunc func = []() {};
    sceneSession->RegisterTouchOutsideCallback(std::move(func));

    ASSERT_NE(sceneSession->onTouchOutside_, nullptr);
}

/**
 * @tc.name: UpdateRotationAvoidArea
 * @tc.desc: UpdateRotationAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, UpdateRotationAvoidArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateRotationAvoidArea";
    info.bundleName_ = "UpdateRotationAvoidArea";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, sceneSession->specificCallback_);
    auto func = [sceneSession](const int32_t& persistentId) {
        sceneSession->SetParentPersistentId(persistentId);
        sceneSession->GetParentPersistentId();
    };
    sceneSession->specificCallback_->onUpdateAvoidArea_ = func;
    sptr<Session> session;
    session = sptr<Session>::MakeSptr(info);
    int result = session->GetPersistentId();
    EXPECT_EQ(0, result);
    sceneSession->UpdateRotationAvoidArea();

    sceneSession->specificCallback_ = nullptr;
    sceneSession->UpdateRotationAvoidArea();
}

/**
 * @tc.name: NotifyForceHideChange
 * @tc.desc: NotifyForceHideChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, NotifyForceHideChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyForceHideChange";
    info.bundleName_ = "NotifyForceHideChange";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->NotifyForceHideChange(true);

    sptr<Session> session = sptr<Session>::MakeSptr(info);
    auto func = [sceneSession](bool hide) { sceneSession->SetPrivacyMode(hide); };
    sceneSession->onForceHideChangeFunc_ = func;
    EXPECT_NE(nullptr, &func);
    sceneSession->NotifyForceHideChange(true);
}

/**
 * @tc.name: SendPointerEventToUI
 * @tc.desc: SendPointerEventToUI
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SendPointerEventToUI, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SendPointerEventToUI";
    info.bundleName_ = "SendPointerEventToUI";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    auto pointerEventFunc = [sceneSession](std::shared_ptr<MMI::PointerEvent> pointerEvent) {
        sceneSession->NotifyOutsideDownEvent(pointerEvent);
    };
    sceneSession->systemSessionPointerEventFunc_ = pointerEventFunc;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    sceneSession->SendPointerEventToUI(pointerEvent);
    EXPECT_NE(nullptr, pointerEvent);
}

/**
 * @tc.name: SetFloatingScale
 * @tc.desc: SetFloatingScale
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetFloatingScale, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetFloatingScale";
    info.bundleName_ = "SetFloatingScale";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    auto windowInfoUpdateFun = [sceneSession](int32_t persistentId, WindowUpdateType type) {
        if (WindowUpdateType::WINDOW_UPDATE_PROPERTY == type) {
            sceneSession->SetCollaboratorType(persistentId);
            return;
        } else {
            persistentId++;
            sceneSession->SetParentPersistentId(persistentId);
            return;
        }
    };
    auto updateAvoidAreaFun = [sceneSession](const int32_t& persistentId) {
        bool result = sceneSession->RemoveSubSession(persistentId);
        sceneSession->SetWindowAnimationFlag(result);
    };
    sceneSession->specificCallback_->onWindowInfoUpdate_ = windowInfoUpdateFun;
    sceneSession->specificCallback_->onUpdateAvoidArea_ = updateAvoidAreaFun;
    sceneSession->SetFloatingScale(3.14f);
    EXPECT_EQ(3.14f, sceneSession->floatingScale_);

    sceneSession->floatingScale_ = 3.0f;
    sceneSession->SetFloatingScale(3.0f);
    EXPECT_EQ(3.0f, sceneSession->floatingScale_);
}

/**
 * @tc.name: ProcessPointDownSession
 * @tc.desc: ProcessPointDownSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, ProcessPointDownSession, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ProcessPointDownSession";
    info.bundleName_ = "ProcessPointDownSession";
    info.windowInputType_ = static_cast<uint32_t>(MMI::WindowInputType::NORMAL);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, sceneSession->specificCallback_);
    auto sessionTouchOutsideFun = [sceneSession](int32_t persistentId, DisplayId displayId) {
        sceneSession->SetCollaboratorType(persistentId);
    };
    auto outsideDownEventFun = [sceneSession](int32_t x, int32_t y) {
        int z = x + y;
        sceneSession->SetCollaboratorType(z);
    };
    sceneSession->specificCallback_->onSessionTouchOutside_ = sessionTouchOutsideFun;
    sceneSession->specificCallback_->onOutsideDownEvent_ = outsideDownEventFun;
    EXPECT_EQ(WSError::WS_OK, sceneSession->ProcessPointDownSession(3, 4));

    sceneSession->specificCallback_->onSessionTouchOutside_ = nullptr;
    EXPECT_EQ(WSError::WS_OK, sceneSession->ProcessPointDownSession(3, 4));

    sceneSession->sessionInfo_.bundleName_ = "SCBGestureBack";
    sceneSession->specificCallback_->onOutsideDownEvent_ = nullptr;
    EXPECT_EQ(WSError::WS_OK, sceneSession->ProcessPointDownSession(3, 4));

    info.windowInputType_ = static_cast<uint32_t>(MMI::WindowInputType::TRANSMIT_ALL);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession1->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, sceneSession1->specificCallback_);
    sceneSession1->specificCallback_->onOutsideDownEvent_ = nullptr;
    EXPECT_EQ(WSError::WS_ERROR_INVALID_TYPE, sceneSession1->ProcessPointDownSession(3, 4));
}

/**
 * @tc.name: SetSessionInfoWindowInputType
 * @tc.desc: SetSessionInfoWindowInputType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetSessionInfoWindowInputType, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSessionInfoWindowInputType";
    info.bundleName_ = "SetSessionInfoWindowInputType";
    info.windowInputType_ = static_cast<uint32_t>(MMI::WindowInputType::NORMAL);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->isActive_ = true;
    sceneSession->SetSessionInfoWindowInputType(static_cast<uint32_t>(MMI::WindowInputType::TRANSMIT_AXIS_MOVE));

    SessionInfo sessionInfo = sceneSession->GetSessionInfo();
    ASSERT_EQ(sessionInfo.windowInputType_, static_cast<uint32_t>(MMI::WindowInputType::TRANSMIT_AXIS_MOVE));

    sceneSession->SetSessionInfoWindowInputType(static_cast<uint32_t>(MMI::WindowInputType::TRANSMIT_ALL));
    sessionInfo = sceneSession->GetSessionInfo();
    ASSERT_EQ(sessionInfo.windowInputType_, static_cast<uint32_t>(MMI::WindowInputType::TRANSMIT_ALL));
}

/**
 * @tc.name: SetSessionInfoExpandInputFlag
 * @tc.desc: SetSessionInfoExpandInputFlag
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetSessionInfoExpandInputFlag, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSessionInfoExpandInputFlag";
    info.bundleName_ = "SetSessionInfoExpandInputFlag";
    info.expandInputFlag_ = 0;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetSessionInfoExpandInputFlag(1);

    SessionInfo sessionInfo = sceneSession->GetSessionInfo();
    ASSERT_EQ(sessionInfo.expandInputFlag_, 1);

    sceneSession->SetSessionInfoExpandInputFlag(0);
    sessionInfo = sceneSession->GetSessionInfo();
    ASSERT_EQ(sessionInfo.expandInputFlag_, 0);
}

/**
 * @tc.name: GetSessionInfoExpandInputFlag
 * @tc.desc: GetSessionInfoExpandInputFlag
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, GetSessionInfoExpandInputFlag, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetSessionInfoExpandInputFlag";
    info.bundleName_ = "GetSessionInfoExpandInputFlag";
    info.expandInputFlag_ = 0;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetSessionInfoExpandInputFlag(1);

    ASSERT_EQ(sceneSession->GetSessionInfoExpandInputFlag(), 1);
}

/**
 * @tc.name: SetSelfToken
 * @tc.desc: SetSelfToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetSelfToken, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSelfToken";
    info.bundleName_ = "SetSelfToken";
    sptr<SceneSession> sceneSession;
    sptr<SceneSession::SpecificSessionCallback> specificSession =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificSession);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->SetSessionState(SessionState::STATE_DISCONNECT);
    sceneSession->UpdateSessionState(SessionState::STATE_CONNECT);
    sceneSession->UpdateSessionState(SessionState::STATE_ACTIVE);
    sceneSession->isVisible_ = true;
    EXPECT_EQ(true, sceneSession->IsVisibleForAccessibility());
    sceneSession->SetSystemTouchable(false);
    EXPECT_EQ(false, sceneSession->IsVisibleForAccessibility());
    sceneSession->SetForegroundInteractiveStatus(true);
    sceneSession->NotifyAccessibilityVisibilityChange();
    auto windowInfoUpdateFun = [sceneSession](int32_t persistentId, WindowUpdateType type) {
        if (WindowUpdateType::WINDOW_UPDATE_PROPERTY == type) {
            sceneSession->SetCollaboratorType(persistentId);
            return;
        } else {
            persistentId++;
            sceneSession->SetParentPersistentId(persistentId);
            return;
        }
    };
    sceneSession->specificCallback_->onWindowInfoUpdate_ = windowInfoUpdateFun;
    sceneSession->NotifyAccessibilityVisibilityChange();
    EXPECT_NE(nullptr, sceneSession->specificCallback_);
}

/**
 * @tc.name: SetLastSafeRect
 * @tc.desc: SetLastSafeRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetLastSafeRect, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetLastSafeRect";
    info.bundleName_ = "SetLastSafeRect";
    sptr<SceneSession> sceneSession;
    sptr<SceneSession::SpecificSessionCallback> specificSession =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificSession);
    WSRect rect = { 3, 4, 5, 6 };
    sceneSession->SetLastSafeRect(rect);
    WSRect result = sceneSession->GetLastSafeRect();
    EXPECT_EQ(3, result.posX_);
    EXPECT_EQ(4, result.posY_);
    EXPECT_EQ(5, result.width_);
    EXPECT_EQ(6, result.height_);
}

/**
 * @tc.name: GetSessionTargetRectByDisplayId
 * @tc.desc: GetSessionTargetRectByDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, GetSessionTargetRectByDisplayId, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetSubSession";
    info.bundleName_ = "GetSessionTargetRectByDisplayId";
    sptr<SceneSession> sceneSession;
    sptr<SceneSession::SpecificSessionCallback> specificSession =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificSession);
    bool res = sceneSession->AddSubSession(sceneSession);
    EXPECT_EQ(true, res);
    EXPECT_EQ(sceneSession, (sceneSession->GetSubSession())[0]);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    WSRect rectResult = sceneSession->GetSessionTargetRectByDisplayId(0);
    EXPECT_EQ(0, rectResult.posX_);
    EXPECT_EQ(0, rectResult.width_);
    auto dragHotAreaFunc = [sceneSession](DisplayId displayId, int32_t type, SizeChangeReason reason) {
        if (SizeChangeReason::END == reason) {
            GTEST_LOG_(INFO) << "type = " << type;
        }
        return;
    };
    sceneSession->SetWindowDragHotAreaListener(dragHotAreaFunc);
    EXPECT_NE(nullptr, sceneSession->moveDragController_);
    sceneSession->moveDragController_ = nullptr;
    rectResult = sceneSession->GetSessionTargetRectByDisplayId(0);
    sceneSession->SetWindowDragHotAreaListener(dragHotAreaFunc);
    ASSERT_EQ(0, rectResult.width_);
}

/**
 * @tc.name: SetPipActionEvent
 * @tc.desc: SetPipActionEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetPipActionEvent, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetPipActionEvent";
    info.bundleName_ = "SetPipActionEvent";
    auto sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    WSError res = sceneSession->SetPipActionEvent("close", 0);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
    auto mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    sceneSession->sessionStage_ = mockSessionStage;
    res = sceneSession->SetPipActionEvent("close", 0);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: SetPiPControlEvent
 * @tc.desc: SetPiPControlEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetPiPControlEvent, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetPiPControlEvent";
    info.bundleName_ = "SetPiPControlEvent";
    auto sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    auto property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionProperty(property);
    auto controlType = WsPiPControlType::VIDEO_PLAY_PAUSE;
    auto status = WsPiPControlStatus::PLAY;
    WSError res = sceneSession->SetPiPControlEvent(controlType, status);
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_TYPE);

    auto sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStage, nullptr);

    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    property->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    sceneSession->SetSessionProperty(property);
    res = sceneSession->SetPiPControlEvent(controlType, status);
    ASSERT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: SetForceHideState
 * @tc.desc: SetForceHideState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetForceHideState, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetForceHideState";
    info.bundleName_ = "SetForceHideState";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    sceneSession->SetForceHideState(ForceHideState::NOT_HIDDEN);
    bool hide = sceneSession->GetForceHideState();
    ASSERT_EQ(hide, ForceHideState::NOT_HIDDEN);
    sceneSession->SetForceHideState(ForceHideState::HIDDEN_WHEN_FOCUSED);
    hide = sceneSession->GetForceHideState();
    ASSERT_EQ(hide, ForceHideState::HIDDEN_WHEN_FOCUSED);
}

/**
 * @tc.name: OnSessionEvent01
 * @tc.desc: OnSessionEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, OnSessionEvent01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnSessionEvent01";
    info.bundleName_ = "OnSessionEvent01";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    sceneSession->leashWinSurfaceNode_ = nullptr;
    SessionEvent event = SessionEvent::EVENT_START_MOVE;
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    sceneSession->OnSessionEvent(event);

    sceneSession->moveDragController_->isStartDrag_ = true;
    auto result = sceneSession->OnSessionEvent(event);
    ASSERT_EQ(result, WSError::WS_OK);
    event = SessionEvent::EVENT_DRAG_START;
    ASSERT_EQ(sceneSession->OnSessionEvent(event), WSError::WS_OK);
}

/**
 * @tc.name: SetSessionRectChangeCallback
 * @tc.desc: SetSessionRectChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetSessionRectChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSessionRectChangeCallback";
    info.bundleName_ = "SetSessionRectChangeCallback";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    NotifySessionRectChangeFunc func;
    sceneSession->SetSessionRectChangeCallback(func);

    sceneSession->RaiseToAppTop();
    sceneSession = nullptr;
    GTEST_LOG_(INFO) << "2";
    sceneSession->BindDialogSessionTarget(sceneSession);

    Session ssession(info);
    ssession.property_ = nullptr;
}

/**
 * @tc.name: SetSessionPiPControlStatusChangeCallback
 * @tc.desc: SetSessionPiPControlStatusChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetSessionPiPControlStatusChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSessionPiPControlStatusChangeCallback";
    info.bundleName_ = "SetSessionPiPControlStatusChangeCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    NotifySessionPiPControlStatusChangeFunc func;
    sceneSession->SetSessionPiPControlStatusChangeCallback(func);
}

/**
 * @tc.name: SetAutoStartPiPStatusChangeCallback
 * @tc.desc: SetAutoStartPiPStatusChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetAutoStartPiPStatusChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAutoStartPiPStatusChangeCallback";
    info.bundleName_ = "SetAutoStartPiPStatusChangeCallback";
    auto sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    NotifyAutoStartPiPStatusChangeFunc func;
    sceneSession->SetAutoStartPiPStatusChangeCallback(func);
}

/**
 * @tc.name: RaiseAppMainWindowToTop
 * @tc.desc: RaiseAppMainWindowToTop
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, RaiseAppMainWindowToTop, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RaiseAppMainWindowToTop";
    info.bundleName_ = "RaiseAppMainWindowToTop";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    WSError result = sceneSession->RaiseAppMainWindowToTop();
    EXPECT_EQ(WSError::WS_OK, result);
    bool status = true;
    sceneSession->OnNeedAvoid(status);

    bool showWhenLocked = true;
    sceneSession->OnShowWhenLocked(showWhenLocked);
    sceneSession->NotifyPropertyWhenConnect();

    sceneSession->focusedOnShow_ = false;
    result = sceneSession->RaiseAppMainWindowToTop();
    EXPECT_EQ(WSError::WS_OK, result);
}

/**
 * @tc.name: GetCutoutAvoidArea
 * @tc.desc: GetCutoutAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, GetCutoutAvoidArea01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetCutoutAvoidArea";
    info.bundleName_ = "GetCutoutAvoidArea";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    WSRect rect;
    AvoidArea avoidArea;
    DisplayManager displayManager;
    Session ssession(info);
    auto display = DisplayManager::GetInstance().GetDisplayById(ssession.GetSessionProperty()->GetDisplayId());
    sceneSession->GetCutoutAvoidArea(rect, avoidArea);
}

/**
 * @tc.name: GetAINavigationBarArea
 * @tc.desc: GetAINavigationBarArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, GetAINavigationBarArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetAINavigationBarArea";
    info.bundleName_ = "GetAINavigationBarArea";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    WSRect rect;
    AvoidArea avoidArea;
    sceneSession->GetAINavigationBarArea(rect, avoidArea);

    sceneSession->SetIsDisplayStatusBarTemporarily(true);
    sceneSession->GetAINavigationBarArea(rect, avoidArea);
    ASSERT_EQ(sceneSession->GetIsDisplayStatusBarTemporarily(), true);

    auto property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    sceneSession->property_ = property;
    sceneSession->GetAINavigationBarArea(rect, avoidArea);

    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->SetSessionProperty(property);
    sceneSession->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(nullptr, sceneSession->specificCallback_);
    sceneSession->specificCallback_->onGetAINavigationBarArea_ = [](uint64_t displayId, bool ignoreVisibility) {
        WSRect rect = { 1, 1, 1, 1 };
        return rect;
    };
    sceneSession->GetAINavigationBarArea(rect, avoidArea);
    sceneSession->GetAINavigationBarArea(rect, avoidArea, true);
}

/**
 * @tc.name: TransferPointerEvent
 * @tc.desc: TransferPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, TransferPointerEvent03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "TransferPointerEvent";
    info.bundleName_ = "TransferPointerEvent";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    sceneSession->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();

    sceneSession->TransferPointerEvent(pointerEvent);

    WindowLimits limits;
    WSRect rect;
    float ratio = 0.0;
    bool isDecor = true;
    float vpr = 0.0;
    sceneSession->SetPipActionEvent("pointerEvent", 0);

    auto property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sceneSession->SetSessionProperty(property);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, sceneSession->SetPipActionEvent("pointerEvent", 0));

    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    property->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    sceneSession->SetSessionProperty(property);
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetPipActionEvent("pointerEvent", 0));
}

/**
 * @tc.name: OnMoveDragCallback
 * @tc.desc: OnMoveDragCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, OnMoveDragCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnMoveDragCallback";
    info.bundleName_ = "OnMoveDragCallback";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    Session session(info);
    WSRect rect;
    sceneSession->UpdateWinRectForSystemBar(rect);
    sceneSession->SetSurfaceBounds(rect, false);
    sceneSession->GetWindowNameAllType();
    session.scenePersistence_ = sptr<ScenePersistence>::MakeSptr("aa", 0);
    sceneSession->GetUpdatedIconPath();

    bool visible = true;
    sceneSession->UpdateNativeVisibility(visible);

    sceneSession->scenePersistence_ = sptr<ScenePersistence>::MakeSptr(info.bundleName_, 0);
    EXPECT_NE(sceneSession->scenePersistence_, nullptr);
    sceneSession->GetUpdatedIconPath();
}

/**
 * @tc.name: OnMoveDragCallback
 * @tc.desc: OnMoveDragCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, OnMoveDragCallback01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnMoveDragCallback01";
    info.bundleName_ = "OnMoveDragCallback01";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    double alpha = 0.5;
    Session session(info);

    sceneSession->SetSystemSceneOcclusionAlpha(alpha);
    sceneSession->IsNeedDefaultAnimation();
    bool isPlaying = true;
    sceneSession->NotifyIsCustomAnimationPlaying(isPlaying);

    sptr<AAFwk::SessionInfo> abilitySessionInfo = nullptr;
    bool visible = true;
    sceneSession->ChangeSessionVisibilityWithStatusBar(abilitySessionInfo, visible);
}

/**
 * @tc.name: OnMoveDragCallback
 * @tc.desc: OnMoveDragCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, OnMoveDragCallback02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnMoveDragCallback02";
    info.bundleName_ = "OnMoveDragCallback02";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sptr<SceneSession> ssession = nullptr;
    sceneSession->HandleCastScreenConnection(info, ssession);

    Session session(info);
    sptr<AAFwk::SessionInfo> abilitySessionInfo = nullptr;
    ExceptionInfo exceptionInfo;
    NotifySessionExceptionFunc func = [](const SessionInfo& info, const ExceptionInfo& exceInfo, bool startFail) {};
    session.sessionExceptionFunc_ = func;
    NotifySessionExceptionFunc func1 = [](const SessionInfo& info, const ExceptionInfo& exceInfo, bool startFail) {};
    session.jsSceneSessionExceptionFunc_ = func1;
    sceneSession->NotifySessionException(abilitySessionInfo, exceptionInfo);

    sceneSession->NotifyPiPWindowPrepareClose();

    bool isLandscapeMultiWindow = true;
    sceneSession->SetLandscapeMultiWindow(isLandscapeMultiWindow);

    std::shared_ptr<MMI::KeyEvent> keyEvent = std::make_shared<MMI::KeyEvent>(MMI::KeyEvent::KEYCODE_BACK);
    bool isPreImeEvent = true;
    sceneSession->SendKeyEventToUI(keyEvent, isPreImeEvent);
    sceneSession->IsDirtyWindow();
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    sceneSession->NotifyUILostFocus();
}

/**
 * @tc.name: IsStartMoving
 * @tc.desc: IsStartMoving
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, IsStartMoving, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsStartMoving";
    info.bundleName_ = "IsStartMoving";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    sceneSession->IsStartMoving();
    bool startMoving = true;
    sceneSession->SetIsStartMoving(startMoving);
    DisplayId from = 0;
    DisplayId to = 0;
    sceneSession->NotifyDisplayMove(from, to);
    sceneSession->ClearExtWindowFlags();
    bool isRegister = true;
    sceneSession->UpdateRectChangeListenerRegistered(isRegister);

    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, sceneSession->sessionStage_);
    sceneSession->NotifyDisplayMove(from, to);
}

/**
 * @tc.name: SetTemporarilyShowWhenLocked
 * @tc.desc: SetTemporarilyShowWhenLocked
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetTemporarilyShowWhenLocked, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetTemporarilyShowWhenLocked";
    info.bundleName_ = "SetTemporarilyShowWhenLocked";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    bool isTemporarilyShowWhenLocked = sceneSession->IsTemporarilyShowWhenLocked();
    ASSERT_EQ(isTemporarilyShowWhenLocked, false);
    sceneSession->SetTemporarilyShowWhenLocked(true);
    isTemporarilyShowWhenLocked = sceneSession->IsTemporarilyShowWhenLocked();
    ASSERT_EQ(isTemporarilyShowWhenLocked, true);
    sceneSession->SetTemporarilyShowWhenLocked(false);
    isTemporarilyShowWhenLocked = sceneSession->IsTemporarilyShowWhenLocked();
    ASSERT_EQ(isTemporarilyShowWhenLocked, false);

    sceneSession->isTemporarilyShowWhenLocked_.store(true);
    sceneSession->SetTemporarilyShowWhenLocked(true);
    ASSERT_EQ(sceneSession->IsTemporarilyShowWhenLocked(), true);
}

/**
 * @tc.name: GetShowWhenLockedFlagValue
 * @tc.desc: GetShowWhenLockedFlagValue
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, GetShowWhenLockedFlagValue, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetShowWhenLockedFlagValue";
    info.bundleName_ = "GetShowWhenLockedFlagValue";
    info.windowType_ = 1;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(sceneSession->GetShowWhenLockedFlagValue(), false);
    sceneSession->property_ = property;
    property->SetWindowFlags(4);
    ASSERT_EQ(sceneSession->GetShowWhenLockedFlagValue(), true);
}

/**
 * @tc.name: SetClientIdentityToken
 * @tc.desc: SetClientIdentityToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetClientIdentityToken, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetClientIdentityToken";
    info.bundleName_ = "SetClientIdentityToken";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    std::string token = "testToken";
    sceneSession->SetClientIdentityToken(token);
    ASSERT_EQ(sceneSession->GetClientIdentityToken(), token);
}

/**
 * @tc.name: SetSkipDraw
 * @tc.desc: SetSkipDraw
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetSkipDraw, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSkipDraw";
    info.bundleName_ = "SetSkipDraw";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->SetSkipDraw(true);
    sceneSession->SetSkipDraw(false);
}

/**
 * @tc.name: GetWindowDragHotAreaType
 * @tc.desc: GetWindowDragHotAreaType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, GetWindowDragHotAreaType, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HotAreaType";
    info.bundleName_ = "HotAreaType";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    WSRect rect = { 0, 0, 10, 10 };
    sceneSession->AddOrUpdateWindowDragHotArea(0, 1, rect);
    sceneSession->AddOrUpdateWindowDragHotArea(0, 1, rect);
    auto type = sceneSession->GetWindowDragHotAreaType(0, 1, 2, 2);
    ASSERT_EQ(type, 1);
}

/**
 * @tc.name: RegisterSubModalTypeChangeCallback
 * @tc.desc: RegisterSubModalTypeChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, RegisterSubModalTypeChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RegisterSubModalTypeChangeCallback";
    info.bundleName_ = "RegisterSubModalTypeChangeCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->RegisterSubModalTypeChangeCallback([](SubWindowModalType subWindowModalType) { return; });
    EXPECT_NE(sceneSession->onSubModalTypeChange_, nullptr);
}

/**
 * @tc.name: NotifySubModalTypeChange
 * @tc.desc: NotifySubModalTypeChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, NotifySubModalTypeChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySubModalTypeChange";
    info.bundleName_ = "NotifySubModalTypeChange";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->RegisterSubModalTypeChangeCallback([](SubWindowModalType subWindowModalType) { return; });
    EXPECT_NE(sceneSession->onSubModalTypeChange_, nullptr);
    EXPECT_EQ(sceneSession->NotifySubModalTypeChange(SubWindowModalType::TYPE_WINDOW_MODALITY), WSError::WS_OK);
}

/**
 * @tc.name: RegisterMainModalTypeChangeCallback
 * @tc.desc: RegisterMainModalTypeChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, RegisterMainModalTypeChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RegisterMainModalTypeChangeCallback";
    info.bundleName_ = "RegisterMainModalTypeChangeCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->RegisterMainModalTypeChangeCallback([](bool isModal) { return; });
    EXPECT_NE(sceneSession->onMainModalTypeChange_, nullptr);
}

/**
 * @tc.name: GetSubWindowModalType
 * @tc.desc: GetSubWindowModalType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, GetSubWindowModalType, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ModalType";
    info.bundleName_ = "ModalType";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sceneSession->SetSessionProperty(property);
    auto result = sceneSession->GetSubWindowModalType();
    ASSERT_EQ(result, SubWindowModalType::TYPE_DIALOG);
}

/**
 * @tc.name: SetWindowAnimationFlag
 * @tc.desc: SetWindowAnimationFlag
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetWindowAnimationFlag, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowAnimationFlag";
    info.bundleName_ = "SetWindowAnimationFlag";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    sceneSession->onWindowAnimationFlagChange_ = [](bool isNeedDefaultAnimationFlag) {};
    sceneSession->SetWindowAnimationFlag(true);
    ASSERT_EQ(true, sceneSession->needDefaultAnimationFlag_);
}

/**
 * @tc.name: IsFullScreenMovable
 * @tc.desc: IsFullScreenMovable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, IsFullScreenMovable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsFullScreenMovable";
    info.bundleName_ = "IsFullScreenMovable";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    auto result = sceneSession->IsFullScreenMovable();
    ASSERT_EQ(true, result);
}

/**
 * @tc.name: IsSplitMovable
 * @tc.desc: IsSplitMovable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, IsSplitMovable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsSplitMovable";
    info.bundleName_ = "IsSplitMovable";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    auto result = sceneSession->IsSplitMovable();
    ASSERT_EQ(false, result);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    sceneSession->property_ = property;
    result = sceneSession->IsSplitMovable();
    ASSERT_EQ(true, result);
}

/**
 * @tc.name: SetTitleAndDockHoverShowChangeCallback
 * @tc.desc: SetTitleAndDockHoverShowChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, SetTitleAndDockHoverShowChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetTitleAndDockHoverShowChangeCallback";
    info.bundleName_ = "SetTitleAndDockHoverShowChangeCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetTitleAndDockHoverShowChangeCallback([](bool isTitleHoverShown, bool isDockHoverShown) { return; });
    EXPECT_NE(sceneSession->onTitleAndDockHoverShowChangeFunc_, nullptr);
}

/**
 * @tc.name: NotifyDragEventOnNextVsync
 * @tc.desc: NotifyDragEventOnNextVsync function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, NotifyDragEventOnNextVsync, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyDragEventOnNextVsync";
    info.bundleName_ = "NotifyDragEventOnNextVsync";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    SizeChangeReason reason = { SizeChangeReason::DRAG };
    session->NotifyDragEventOnNextVsync(reason);
    SessionEvent event = { SessionEvent::EVENT_DRAG };
    EXPECT_EQ(WSError::WS_OK, session->OnSessionEvent(event));
 
    auto moveDragController = sptr<MoveDragController>::MakeSptr(wptr(session));
    session->moveDragController_ = moveDragController;
    session->moveDragController_->isStartDrag_ = true;
    session->UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    session->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    session->systemConfig_.freeMultiWindowSupport_ = true;
    session->systemConfig_.freeMultiWindowEnable_ = true;
    session->dragResizeTypeDuringDrag_ = DragResizeType::RESIZE_SCALE;
 
    session->SetVsyncStation(nullptr);
    ASSERT_EQ(nullptr, session->vsyncStation_);
    session->NotifyDragEventOnNextVsync(reason);

    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    session->SetVsyncStation(mockVsyncStation);
    ASSERT_NE(nullptr, session->vsyncStation_);
    session->NotifyDragEventOnNextVsync(reason);
    EXPECT_EQ(WSError::WS_OK, session->OnSessionEvent(event));

    session->UpdateSizeChangeReason(reason);
    session->NotifyDragEventOnNextVsync(reason);
    EXPECT_EQ(WSError::WS_OK, session->OnSessionEvent(event));
}
 
/**
 * @tc.name: IsDragResizeScale
 * @tc.desc: IsDragResizeScale function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest2, IsDragResizeScale, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "IsDragResizeScale";
    info.bundleName_ = "IsDragResizeScale";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    auto oriProperty = session->GetSessionProperty();
    session->property_ = nullptr;
    SizeChangeReason reason = { SizeChangeReason::DRAG_START };
    EXPECT_EQ(session->IsDragResizeScale(reason), false);
 
    reason = { SizeChangeReason::DRAG_END };
    EXPECT_EQ(session->IsDragResizeScale(reason), false);
 
    session->property_ = oriProperty;
    reason = { SizeChangeReason::DRAG };
    auto moveDragController = sptr<MoveDragController>::MakeSptr(wptr(session));
    session->moveDragController_ = moveDragController;
 
    session->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    session->systemConfig_.freeMultiWindowSupport_ = true;
    session->systemConfig_.freeMultiWindowEnable_ = true;
    session->dragResizeTypeDuringDrag_ = DragResizeType::RESIZE_SCALE;
    session->moveDragController_->isStartDrag_ = true;
    session->needNotifyDragEventOnNextVsync_ = true;
    EXPECT_EQ(session->IsDragResizeScale(reason), true);
}
} // namespace
} // namespace Rosen
} // namespace OHOS