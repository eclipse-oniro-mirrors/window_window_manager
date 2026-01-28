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

#include "display_manager.h"
#include "input_event.h"
#include "key_event.h"
#include "mock/mock_accesstoken_kit.h"
#include "mock/mock_parameters.h"
#include "mock/mock_session_stage.h"
#include "pointer_event.h"

#include "session/host/include/main_session.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/sub_session.h"
#include "session/host/include/system_session.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "window_helper.h"
#include "wm_common.h"
#include "ui/rs_surface_node.h"


using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class SceneSessionTest4 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<SceneSession> sceneSession;
    sptr<WindowSessionProperty> property;
    SessionInfo info;
    WSPropertyChangeAction action;
private:
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void SceneSessionTest4::SetUpTestCase()
{
}

void SceneSessionTest4::TearDownTestCase()
{
}

void SceneSessionTest4::SetUp()
{
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    property = sptr<WindowSessionProperty>::MakeSptr();
    action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    if (!handler_) {
        auto runner = AppExecFwk::EventRunner::Create("SceneSessionTest");
        handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    sceneSession->SetEventHandler(handler_, nullptr);
}

void SceneSessionTest4::TearDown()
{
}

namespace {
/**
 * @tc.name: HandleActionUpdateFlags
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, HandleActionUpdateFlags, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    WMError ret = sceneSession->HandleActionUpdateStatusProps(property, action);
    ASSERT_EQ(WMError::WM_OK, ret);
    ret = sceneSession->HandleActionUpdateNavigationProps(property, action);
    ASSERT_EQ(WMError::WM_OK, ret);
    ret = sceneSession->HandleActionUpdateNavigationIndicatorProps(property, action);
    ASSERT_EQ(WMError::WM_OK, ret);
    ret = sceneSession->HandleActionUpdateFlags(property, action);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: HandleActionUpdateTouchHotArea
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, HandleActionUpdateTouchHotArea, TestSize.Level1)
{
    SessionInfo info;
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onWindowInfoUpdate_ = [](int32_t persistentId, WindowUpdateType type) {};
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    std::vector<Rect> hotAreas;
    hotAreas.push_back({1, 2, 3, 4});
    property->SetTouchHotAreas(hotAreas);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    WMError ret = sceneSession->HandleActionUpdateTouchHotArea(property, action);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: HandleActionUpdateDecorEnable
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, HandleActionUpdateDecorEnable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateDecorEnable";
    info.bundleName_ = "HandleActionUpdateDecorEnable";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->isSystemCalling_ = true;
    sceneSession->SetSessionProperty(property);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;

    auto ret = sceneSession->HandleActionUpdateDecorEnable(property, action);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: HandleActionUpdateDecorEnableTest001
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, HandleActionUpdateDecorEnableTest001, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateDecorEnableTest001";
    info.bundleName_ = "HandleActionUpdateDecorEnableTest001";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->isSystemCalling_ = false;
    sceneSession->SetSessionProperty(property);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;

    auto ret = sceneSession->HandleActionUpdateDecorEnable(property, action);
    ASSERT_EQ(WMError::WM_ERROR_NOT_SYSTEM_APP, ret);
}

/**
 * @tc.name: HandleActionUpdateDragenabled
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, HandleActionUpdateDragenabled, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    OHOS::Rosen::Session session(info);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    session.property_ = property;

    WMError res = sceneSession->HandleActionUpdateDragenabled(property, action);
    ASSERT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: HandleActionUpdateRaiseenabled
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, HandleActionUpdateRaiseenabled, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, property);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG;
    sceneSession->HandleActionUpdateRaiseenabled(property, action);

    OHOS::Rosen::WindowSessionProperty windowSessionProperty;
    windowSessionProperty.isSystemCalling_ = {true};
    sceneSession->HandleActionUpdateRaiseenabled(property, action);

    windowSessionProperty.isSystemCalling_ = {true};
    OHOS::Rosen::Session session(info);
    session.property_ = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->HandleActionUpdateRaiseenabled(property, action);

    sceneSession->HandleActionUpdateHideNonSystemFloatingWindows(property, action);
}

/**
 * @tc.name: HandleActionUpdateTextfieldAvoidInfo
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, HandleActionUpdateTextfieldAvoidInfo, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, property);
    sceneSession->HandleActionUpdateTextfieldAvoidInfo(property, action);
    sceneSession->HandleActionUpdateWindowMask(property, action);

    OHOS::Rosen::Session session(info);
    session.property_ = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->HandleActionUpdateTextfieldAvoidInfo(property, action);
    sceneSession->HandleActionUpdateWindowMask(property, action);
    sceneSession->HandleActionUpdateTopmost(property, action);

    SessionInfo info;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    sceneSession1->HandleActionUpdateTextfieldAvoidInfo(property, action);
    sceneSession1->HandleActionUpdateWindowMask(property, action);
}

/**
 * @tc.name: SetWindowFlags
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetWindowFlags, TestSize.Level1)
{
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, property);
    sceneSession->SetWindowFlags(property);

    OHOS::Rosen::Session session(info);
    session.property_ = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->SetWindowFlags(property);
    sceneSession->NotifySessionChangeByActionNotifyManager(property, action);

    session.property_ = nullptr;
    sceneSession->SetWindowFlags(property);
    sceneSession->sessionChangeByActionNotifyManagerFunc_ = [](
        const sptr<SceneSession>& sceneSession,
        const sptr<WindowSessionProperty>& property, WSPropertyChangeAction action
    ){};
    sceneSession->NotifySessionChangeByActionNotifyManager(property, action);
}

/**
 * @tc.name: SetSkipDraw
 * @tc.desc: SetSkipDraw function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetSkipDraw, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSkipDraw";
    info.bundleName_ = "SetSkipDraw";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, session);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->SetSurfaceNode(surfaceNode);
    session->SetLeashWinSurfaceNode(surfaceNode);
    EXPECT_TRUE(session->GetShadowSurfaceNode());
    EXPECT_TRUE(session->GetLeashWinShadowSurfaceNode());
    session->SetSkipDraw(true);
    EXPECT_TRUE(session->GetShadowSurfaceNode()->GetSkipDraw());
    session->SetSkipDraw(false);
    EXPECT_FALSE(session->GetShadowSurfaceNode()->GetSkipDraw());

    session->SetLeashWinSurfaceNode(nullptr);
    session->SetSkipDraw(true);
    EXPECT_FALSE(session->GetLeashWinShadowSurfaceNode());

    session->SetSurfaceNode(nullptr);
    session->SetSkipDraw(true);
    EXPECT_FALSE(session->GetShadowSurfaceNode());
}

/**
 * @tc.name: SetScale
 * @tc.desc: SetScale function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetScale, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetScale";
    info.bundleName_ = "SetScale";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, session);
    session->Session::SetScale(1.0f, 2.0f, 3.0f, 4.0f);
    session->sessionStage_ = nullptr;
    session->SetScale(5.0f, 2.0f, 3.0f, 4.0f);
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    NotifyWindowInfoUpdateCallback func = [](int32_t persistentId, WindowUpdateType type) {
        return;
    };
    specificCallback->onWindowInfoUpdate_ = func;
    session->specificCallback_ = specificCallback;
    session->SetScale(5.0f, 5.0f, 3.0f, 4.0f);
    session->SetScale(5.0f, 5.0f, 5.0f, 4.0f);
    session->SetScale(5.0f, 5.0f, 5.0f, 5.0f);
    session->SetScale(5.0f, 5.0f, 5.0f, 5.0f);
    EXPECT_EQ(5.0f, session->GetScaleX());
    EXPECT_EQ(5.0f, session->GetScaleY());
    EXPECT_EQ(5.0f, session->GetPivotX());
    EXPECT_EQ(5.0f, session->GetPivotY());

    session->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, session->sessionStage_);
    session->SetScale(5.0f, 5.0f, 5.0f, 5.0f);
    EXPECT_EQ(5.0f, session->GetPivotY());
}

/**
 * @tc.name: RequestSessionBack
 * @tc.desc: RequestSessionBack function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, RequestSessionBack, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestSessionBack";
    info.bundleName_ = "RequestSessionBack";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    NotifyBackPressedFunc func = [](const bool needMoveToBackground) {
        return;
    };
    session->backPressedFunc_ = func;
    session->SetLeashWinSurfaceNode(nullptr);
    EXPECT_EQ(nullptr, session->GetLeashWinShadowSurfaceNode());
    EXPECT_EQ(WSError::WS_OK, session->RequestSessionBack(true));

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    EXPECT_NE(nullptr, surfaceNode);
    session->SetLeashWinSurfaceNode(surfaceNode);
    EXPECT_NE(nullptr, session->GetLeashWinShadowSurfaceNode());
    EXPECT_EQ(WSError::WS_OK, session->RequestSessionBack(true));
}

/**
 * @tc.name: SetSurfaceBounds
 * @tc.desc: SetSurfaceBounds function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetSurfaceBounds, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSurfaceBounds";
    info.bundleName_ = "SetSurfaceBounds";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, session);
    WSRect rect;
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = surfaceNode;
    session->SetSurfaceBounds(rect, false);
    session->SetLeashWinSurfaceNode(surfaceNode);
    session->SetSurfaceBounds(rect, false);
    EXPECT_NE(nullptr, session->GetLeashWinSurfaceNode());
}

/**
 * @tc.name: SetFloatingScale
 * @tc.desc: SetFloatingScale function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetFloatingScale, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetFloatingScale";
    info.bundleName_ = "SetFloatingScale";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, session);
    session->floatingScale_ = 3.14f;
    session->SetFloatingScale(2.176f);
    session->SetFloatingScale(3.14f);
    EXPECT_EQ(nullptr, session->specificCallback_);
}

/**
 * @tc.name: SetRequestedOrientation
 * @tc.desc: SetRequestedOrientation function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetRequestedOrientation, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetRequestedOrientation";
    info.bundleName_ = "SetRequestedOrientation";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    Orientation orientation { Orientation::BEGIN };
    session->SetRequestedOrientation(orientation);
    session->onRequestedOrientationChange_ = nullptr;
    session->SetRequestedOrientation(orientation);
    NotifyReqOrientationChangeFunc func = [](uint32_t orientation, bool needAnimation) {
        return;
    };
    session->onRequestedOrientationChange_ = func;
    session->SetRequestedOrientation(orientation);
    EXPECT_NE(nullptr, session->onRequestedOrientationChange_);
}

/**
 * @tc.name: UpdateSessionPropertyByAction
 * @tc.desc: UpdateSessionPropertyByAction function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, UpdateSessionPropertyByAction, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateSessionPropertyByAction";
    info.bundleName_ = "UpdateSessionPropertyByAction";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, sceneSession->UpdateSessionPropertyByAction(nullptr, action));

    sceneSession->SetSessionProperty(property);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, sceneSession->UpdateSessionPropertyByAction(property, action));

    action = WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON;
    EXPECT_EQ(WMError::WM_OK, sceneSession->UpdateSessionPropertyByAction(property, action));
}

/**
 * @tc.name: ProcessUpdatePropertyByAction1
 * @tc.desc: ProcessUpdatePropertyByAction1 function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, ProcessUpdatePropertyByAction1, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ProcessUpdatePropertyByAction1";
    info.bundleName_ = "ProcessUpdatePropertyByAction1";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_FOCUSABLE));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_TOUCHABLE));

    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS));

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->state_ = SessionState::STATE_END;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_SESSION, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS));

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_ORIENTATION));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE));

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    sceneSession->SetSurfaceNode(surfaceNode);
    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP));
}

/**
 * @tc.name: ProcessUpdatePropertyByAction2
 * @tc.desc: ProcessUpdatePropertyByAction2 function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, ProcessUpdatePropertyByAction2, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ProcessUpdatePropertyByAction2";
    info.bundleName_ = "ProcessUpdatePropertyByAction2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_OTHER_PROPS));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_STATUS_PROPS));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_PROPS));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_FLAGS));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_MODE));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA));

    property->SetSystemCalling(false);
    EXPECT_EQ(WMError::WM_ERROR_NOT_SYSTEM_APP, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE));

    property->SetSystemCalling(true);
    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE));
}

/**
 * @tc.name: ProcessUpdatePropertyByAction3
 * @tc.desc: ProcessUpdatePropertyByAction3 function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, ProcessUpdatePropertyByAction3, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ProcessUpdatePropertyByAction3";
    info.bundleName_ = "ProcessUpdatePropertyByAction3";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS));

    property->SetSystemCalling(false);
    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED));

    property->SetSystemCalling(true);
    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED));

    sceneSession->property_ = property;
    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_TEXTFIELD_AVOID_INFO));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_TOPMOST));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_SUB_WINDOW_Z_LEVEL));

    property->SetSystemCalling(false);
    EXPECT_EQ(WMError::WM_ERROR_NOT_SYSTEM_APP, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO));

    property->SetSystemCalling(true);
    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO));

    EXPECT_EQ(WMError::WM_DO_NOTHING, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_RECT));

    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_FLOAT);
    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_AVOID_AREA_OPTION));
}

/**
 * @tc.name: ProcessUpdatePropertyByAction4
 * @tc.desc: ProcessUpdatePropertyByAction4 function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, ProcessUpdatePropertyByAction4, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ProcessUpdatePropertyByAction4";
    info.bundleName_ = "ProcessUpdatePropertyByAction4";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetSystemCalling(true);
    sceneSession->property_ = property;
    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS));

    sceneSession->property_ = nullptr;
    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE));

    EXPECT_EQ(WMError::WM_OK, sceneSession->ProcessUpdatePropertyByAction(property,
        WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS));
}

/**
 * @tc.name: HandleSpecificSystemBarProperty
 * @tc.desc: HandleSpecificSystemBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, HandleSpecificSystemBarProperty, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleSpecificSystemBarProperty";
    info.bundleName_ = "HandleSpecificSystemBarProperty";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WindowType type = WindowType::WINDOW_TYPE_STATUS_BAR;
    sceneSession->HandleSpecificSystemBarProperty(type, property);

    sceneSession->isDisplayStatusBarTemporarily_.store(true);
    sceneSession->HandleSpecificSystemBarProperty(type, property);

    sceneSession->specificCallback_ = nullptr;
    sceneSession->HandleSpecificSystemBarProperty(type, property);

    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(nullptr, specificCallback);
    sceneSession->specificCallback_ = specificCallback;
    sceneSession->HandleSpecificSystemBarProperty(type, property);

    sceneSession->specificCallback_->onUpdateAvoidArea_ = nullptr;
    sceneSession->HandleSpecificSystemBarProperty(type, property);

    UpdateAvoidAreaCallback onUpdateAvoidArea;
    sceneSession->specificCallback_->onUpdateAvoidArea_ = onUpdateAvoidArea;
    sceneSession->HandleSpecificSystemBarProperty(type, property);
}

/**
 * @tc.name: HandleLayoutAvoidAreaUpdate
 * @tc.desc: HandleLayoutAvoidAreaUpdate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, HandleLayoutAvoidAreaUpdate, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleLayoutAvoidAreaUpdate";
    info.bundleName_ = "HandleLayoutAvoidAreaUpdate";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    
    session->isLastFrameLayoutFinishedFunc_ = nullptr;
    session->isAINavigationBarAvoidAreaValid_ = nullptr;
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END));

    session->isLastFrameLayoutFinishedFunc_ = [](bool& isLayoutFinished) {
        isLayoutFinished = false;
        return WSError::WS_ERROR_NULLPTR;
    };
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END));

    session->isLastFrameLayoutFinishedFunc_ = [](bool& isLayoutFinished) {
        isLayoutFinished = true;
        return WSError::WS_OK;
    };
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END));
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_SYSTEM));
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_NAVIGATION_INDICATOR));

    session->isAINavigationBarAvoidAreaValid_ = [](DisplayId displayId,
        const AvoidArea& avoidArea, int32_t sessionBottom) {
        return true;
    };
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END));
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_NAVIGATION_INDICATOR));

    session->isAINavigationBarAvoidAreaValid_ = [](DisplayId displayId,
        const AvoidArea& avoidArea, int32_t sessionBottom) {
        return false;
    };
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END));
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_NAVIGATION_INDICATOR));
}

/**
 * @tc.name: SetWindowFlags1
 * @tc.desc: SetWindowFlags1
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetWindowFlags1, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowFlags1";
    info.bundleName_ = "SetWindowFlags1";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    sptr<WindowSessionProperty> sessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, sessionProperty);
    sceneSession->SetWindowFlags(property);
    sceneSession->property_ = sessionProperty;
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_PARENT_LIMIT));
    sceneSession->property_->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID));
    sceneSession->SetWindowFlags(property);
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE));
    sceneSession->property_->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK));
    sceneSession->SetWindowFlags(property);
}

/**
 * @tc.name: SetGestureBackEnabled
 * @tc.desc: SetGestureBackEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetGestureBackEnabled, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetGestureBackEnabled";
    info.bundleName_ = "SetGestureBackEnabled";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->isEnableGestureBack_ = false;
    EXPECT_EQ(WMError::WM_OK, sceneSession->SetGestureBackEnabled(false));
    sceneSession->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, sceneSession->specificCallback_);
    auto func = [sceneSession](int32_t persistentId) {
        return;
    };
    sceneSession->specificCallback_->onUpdateGestureBackEnabled_ = func;
    EXPECT_EQ(WMError::WM_OK, sceneSession->SetGestureBackEnabled(true));
    EXPECT_EQ(true, sceneSession->GetGestureBackEnabled());
}

/**
 * @tc.name: GetCustomDecorHeight02
 * @tc.desc: GetCustomDecorHeight
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, GetCustomDecorHeight02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetCustomDecorHeight";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetCustomDecorHeight(50);
    sceneSession->SetCustomDecorHeight(20);
    ASSERT_EQ(50, sceneSession->customDecorHeight_);

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession2->SetCustomDecorHeight(50);
    sceneSession2->SetCustomDecorHeight(150);
    ASSERT_EQ(50, sceneSession2->customDecorHeight_);
}

/**
 * @tc.name: SetDefaultDisplayIdIfNeed03
 * @tc.desc: SetDefaultDisplayIdIfNeed
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetDefaultDisplayIdIfNeed03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetDefaultDisplayIdIfNeed03";
    info.bundleName_ = "SetDefaultDisplayIdIfNeed03";
    info.screenId_ = 20;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetDefaultDisplayIdIfNeed();
    ASSERT_EQ(20, sceneSession->GetSessionInfo().screenId_);
}

/**
 * @tc.name: NotifyServerToUpdateRect01
 * @tc.desc: NotifyServerToUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, NotifyServerToUpdateRect01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyServerToUpdateRect";
    info.bundleName_ = "NotifyServerToUpdateRect";
    info.screenId_ = 20;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    SessionUIParam uiParam;
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    sceneSession->SetForegroundInteractiveStatus(false);
    sceneSession->NotifyServerToUpdateRect(uiParam, reason);
    ASSERT_EQ(false, sceneSession->NotifyServerToUpdateRect(uiParam, reason));

    sceneSession->SetForegroundInteractiveStatus(true);
    ASSERT_EQ(false, sceneSession->NotifyServerToUpdateRect(uiParam, reason));

    uiParam.needSync_ = false;
    ASSERT_EQ(false, sceneSession->NotifyServerToUpdateRect(uiParam, reason));

    uiParam.needSync_ = true;
    uiParam.rect_ = {0, 0, 1, 1};

    sceneSession->GetLayoutController()->SetSessionRect({0, 0, 1, 1});
    sceneSession->SetClientRect({0, 0, 1, 1});
    ASSERT_FALSE(sceneSession->NotifyServerToUpdateRect(uiParam, reason));

    sceneSession->GetLayoutController()->SetSessionRect({1, 1, 1, 1});
    ASSERT_TRUE(sceneSession->NotifyServerToUpdateRect(uiParam, reason));

    sceneSession->GetLayoutController()->SetSessionRect({0, 0, 1, 1});
    sceneSession->SetClientRect({1, 1, 1, 1});
    ASSERT_FALSE(sceneSession->NotifyServerToUpdateRect(uiParam, reason));

    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_TRUE(sceneSession->NotifyServerToUpdateRect(uiParam, reason));

    sceneSession->GetLayoutController()->SetSessionRect({1, 1, 1, 1});
    sceneSession->SetClientRect({1, 1, 1, 1});
    ASSERT_TRUE(sceneSession->NotifyServerToUpdateRect(uiParam, reason));

    uiParam.rect_ = {0, 0, 1, 0};
    ASSERT_EQ(false, sceneSession->NotifyServerToUpdateRect(uiParam, reason));
}

/**
 * @tc.name: UpdateVisibilityInner
 * @tc.desc: UpdateVisibilityInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, UpdateVisibilityInner01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateVisibilityInner01";
    info.bundleName_ = "UpdateVisibilityInner01";
    info.screenId_ = 20;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->isVisible_ = false;
    ASSERT_EQ(false, sceneSession->UpdateVisibilityInner(false));

    ASSERT_EQ(true, sceneSession->UpdateVisibilityInner(true));
}

/**
 * @tc.name: UnregisterSessionChangeListeners
 * @tc.desc: UnregisterSessionChangeListeners
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, UnregisterSessionChangeListeners01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UnregisterSessionChangeListeners01";
    info.bundleName_ = "UnregisterSessionChangeListeners01";
    info.screenId_ = 20;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->UnregisterSessionChangeListeners();

    sceneSession->UnregisterSessionChangeListeners();
    NotifyPendingSessionToBackgroundForDelegatorFunc func =[sceneSession](const SessionInfo& info,
        bool shouldBackToCaller) { return; };
    sceneSession->pendingSessionToBackgroundForDelegatorFunc_ = func;
    ASSERT_EQ(WSError::WS_OK, sceneSession->PendingSessionToBackgroundForDelegator(true));
}

/**
 * @tc.name: IsPcOrPadEnableActivation01
 * @tc.desc: IsPcOrPadEnableActivation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, IsPcOrPadEnableActivation01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsPcOrPadEnableActivation01";
    info.bundleName_ = "IsPcOrPadEnableActivation01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->SetSessionProperty(property);
    sceneSession->SetIsPcAppInPad(true);
    ASSERT_EQ(true, sceneSession->IsPcOrPadEnableActivation());
}

/**
 * @tc.name: SetSystemWindowEnableDrag01
 * @tc.desc: SetSystemWindowEnableDrag function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetSystemWindowEnableDrag01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSystemWindowEnableDrag01";
    info.bundleName_ = "SetSystemWindowEnableDrag01";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    session->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    auto ret = session->SetSystemWindowEnableDrag(true);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_CALLING, ret);

    session->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = session->SetSystemWindowEnableDrag(true);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: OnTitleAndDockHoverShowChange01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, OnTitleAndDockHoverShowChange01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnTitleAndDockHoverShowChange01";
    info.bundleName_ = "OnTitleAndDockHoverShowChange01";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    sceneSession->SetSessionProperty(property);
    auto result = sceneSession->OnTitleAndDockHoverShowChange(true, true);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetAutoStartPiP01
 * @tc.desc: SetAutoStartPiP
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetAutoStartPiP01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAutoStartPiP01";
    info.bundleName_ = "SetAutoStartPiP01";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    sceneSession->SetSessionProperty(property);
    sceneSession->isTerminating_ = false;
    auto result = sceneSession->SetAutoStartPiP(true, 0, 1, 1);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetAutoStartPiP02
 * @tc.desc: SetAutoStartPiP
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetAutoStartPiP02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAutoStartPiP02";
    info.bundleName_ = "SetAutoStartPiP02";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    sceneSession->SetSessionProperty(property);
    sceneSession->isTerminating_ = true;
    NotifyAutoStartPiPStatusChangeFunc func = [](bool flag, uint32_t status, uint32_t width, uint32_t height) {
        return;
    };
    sceneSession->autoStartPiPStatusChangeFunc_ = func;
    auto result = sceneSession->SetAutoStartPiP(true, 1, 1, 1);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetAutoStartPiP03
 * @tc.desc: SetAutoStartPiP
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetAutoStartPiP03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAutoStartPiP03";
    info.bundleName_ = "SetAutoStartPiP03";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    sceneSession->SetSessionProperty(property);
    sceneSession->isTerminating_ = true;
    auto result = sceneSession->SetAutoStartPiP(true, 0, 1, 1);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdatePiPControlStatus
 * @tc.desc: UpdatePiPControlStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, UpdatePiPControlStatus, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdatePiPControlStatus";
    info.bundleName_ = "UpdatePiPControlStatus";
    auto sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    auto property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    sceneSession->SetSessionProperty(property);

    auto controlType = WsPiPControlType::VIDEO_PLAY_PAUSE;
    auto status = WsPiPControlStatus::PLAY;
    sceneSession->isTerminating_ = false;
    WSError result = sceneSession->UpdatePiPControlStatus(controlType, status);
    ASSERT_EQ(result, WSError::WS_OK);

    sceneSession->isTerminating_ = true;
    result = sceneSession->UpdatePiPControlStatus(controlType, status);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetSplitButtonVisible01
 * @tc.desc: SetSplitButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetSplitButtonVisible01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSplitButtonVisible01";
    info.bundleName_ = "SetSplitButtonVisible01";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    sceneSession->SetSessionProperty(property);
    sceneSession->sessionStage_ = nullptr;
    auto result = sceneSession->SetSplitButtonVisible(true);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: SetSplitButtonVisible02
 * @tc.desc: SetSplitButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetSplitButtonVisible02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSplitButtonVisible02";
    info.bundleName_ = "SetSplitButtonVisible02";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    sceneSession->SetSessionProperty(property);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    sceneSession->sessionStage_ = mockSessionStage;
    auto result = sceneSession->SetSplitButtonVisible(true);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetMovable01
 * @tc.desc: SetMovable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetMovable01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetMovable01";
    info.bundleName_ = "SetMovable01";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->SetMovable(true);
    sceneSession->leashWinSurfaceNode_ = nullptr;
    SessionEvent event = SessionEvent::EVENT_START_MOVE;
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    sceneSession->SetMovable(true);
    sceneSession->OnSessionEvent(event);
    sceneSession->moveDragController_->isStartDrag_ = true;
    auto result = sceneSession->OnSessionEvent(event);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetWindowFlags01
 * @tc.desc: SetWindowFlags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetWindowFlags01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowFlags01";
    info.bundleName_ = "SetWindowFlags01";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->SetWindowFlags(property);
    ASSERT_EQ(0, property->GetWindowFlags());
}

/**
 * @tc.name: SetWindowFlags02
 * @tc.desc: SetWindowFlags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetWindowFlags02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowFlags02";
    info.bundleName_ = "SetWindowFlags02";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowFlags(2);
    sceneSession->SetSessionProperty(property);

    sptr<WindowSessionProperty> property2 = sptr<WindowSessionProperty>::MakeSptr();
    property2->SetWindowFlags(2);
    property2->SetSystemCalling(false);

    sceneSession->SetWindowFlags(property2);
    ASSERT_EQ(2, sceneSession->GetSessionProperty()->GetWindowFlags());
}

/**
 * @tc.name: SetWindowFlags03
 * @tc.desc: SetWindowFlags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetWindowFlags03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowFlags03";
    info.bundleName_ = "SetWindowFlags03";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowFlags(2);
    sceneSession->SetSessionProperty(property);

    sptr<WindowSessionProperty> property2 = sptr<WindowSessionProperty>::MakeSptr();
    property2->SetWindowFlags(2);
    property2->SetSystemCalling(true);

    sceneSession->SetWindowFlags(property2);
    ASSERT_EQ(2, sceneSession->GetSessionProperty()->GetWindowFlags());
}

/**
 * @tc.name: UpdateSessionPropertyByAction02
 * @tc.desc: UpdateSessionPropertyByAction function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, UpdateSessionPropertyByAction02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateSessionPropertyByAction02";
    info.bundleName_ = "UpdateSessionPropertyByAction02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->SetSessionProperty(property);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_MAIN_WINDOW_TOPMOST;
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, sceneSession->UpdateSessionPropertyByAction(property, action));
}

/**
 * @tc.name: IsMovable01
 * @tc.desc: IsMovable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, isMovable01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "isMovable01";
    info.bundleName_ = "isMovable01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->SetSessionProperty(property);
    ASSERT_EQ(false, sceneSession->IsMovable());
}

/**
 * @tc.name: IsMovable02
 * @tc.desc: IsMovable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, IsMovable02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsMovable02";
    info.bundleName_ = "IsMovable02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->SetSessionProperty(property);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    ASSERT_EQ(WSError::WS_DO_NOTHING, sceneSession->UpdateFocus(false));
    ASSERT_EQ(false, sceneSession->IsMovable());
    ASSERT_EQ(WSError::WS_OK, sceneSession->UpdateFocus(true));
}

/**
 * @tc.name: IsMovable03
 * @tc.desc: IsMovable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, IsMovable03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsMovable03";
    info.bundleName_ = "IsMovable03";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    ASSERT_EQ(WSError::WS_OK, sceneSession->UpdateFocus(true));
    ASSERT_EQ(false, sceneSession->IsMovable());
}

/**
 * @tc.name: SetFrameGravity
 * @tc.desc: SetFrameGravity Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetFrameGravity, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetFrameGravity";
    info.bundleName_ = "SetFrameGravity";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = surfaceNode;
    ASSERT_EQ(true, session->SetFrameGravity(Gravity::RESIZE));
    session->surfaceNode_ = nullptr;
    ASSERT_EQ(false, session->SetFrameGravity(Gravity::TOP_LEFT));
}

/**
 * @tc.name: SetIsLayoutFullScreen
 * @tc.desc: SetIsLayoutFullScreen Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetIsLayoutFullScreen, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetIsLayoutFullScreen";
    info.bundleName_ = "SetIsLayoutFullScreen";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    session->SetIsLayoutFullScreen(true);
    EXPECT_EQ(session->IsLayoutFullScreen(), true);
}

/**
 * @tc.name: IsLayoutFullScreen
 * @tc.desc: IsLayoutFullScreen Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, IsLayoutFullScreen, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsLayoutFullScreen";
    info.bundleName_ = "IsLayoutFullScreen";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    EXPECT_EQ(session->IsLayoutFullScreen(), false);
}

/**
 * @tc.name: UpdateAllModalUIExtensions
 * @tc.desc: UpdateAllModalUIExtensions Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, UpdateAllModalUIExtensions, TestSize.Level1)
{
    WSRect globalRect = { 100, 100, 100, 100 };
    sceneSession->SetSessionGlobalRect(globalRect);

    WSRect newGlobalRect = { 150, 150, 100, 100 };
    sceneSession->UpdateAllModalUIExtensions(newGlobalRect);

    Rect windowRect = { 100, 100, 100, 100 };
    Rect uiExtRect = { 0, 0, 100, 100 };
    ExtensionWindowEventInfo extensionInfo { 1, 1, -1, windowRect, uiExtRect, false };
    ExtensionWindowEventInfo extensionInfo2 { 2, 2, -1, windowRect, uiExtRect, true };
    sceneSession->modalUIExtensionInfoList_.push_back(extensionInfo);
    sceneSession->modalUIExtensionInfoList_.push_back(extensionInfo2);
    sceneSession->UpdateAllModalUIExtensions(newGlobalRect);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(sceneSession->modalUIExtensionInfoList_[1].windowRect.posX_, 150);
    EXPECT_EQ(sceneSession->modalUIExtensionInfoList_[1].windowRect.posY_, 150);
}

/**
 * @tc.name: HandleActionUpdateSubWindowZLevel
 * @tc.desc: HandleActionUpdateSubWindowZLevel Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, HandleActionUpdateSubWindowZLevel, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateSubWindowZLevel";
    info.bundleName_ = "HandleActionUpdateSubWindowZLevel";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sptr<WindowSessionProperty> sessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    sessionProperty->zLevel_ = 0;
    sceneSession->property_ = sessionProperty;
    ASSERT_EQ(0, sceneSession->GetSubWindowZLevel());
    property->zLevel_ = 1;
    WMError ret = sceneSession->HandleActionUpdateSubWindowZLevel(property,
        WSPropertyChangeAction::ACTION_UPDATE_SUB_WINDOW_Z_LEVEL);
    sceneSession->SetSubWindowZLevel(1);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: HandleActionUpdateAvoidAreaOption
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, HandleActionUpdateAvoidAreaOption, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateAvoidAreaOption";
    info.bundleName_ = "HandleActionUpdateAvoidAreaOption";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> sessionProperty = session->GetSessionProperty();

    sessionProperty->SetAvoidAreaOption(1);
    sessionProperty->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_FLOAT);
    WMError ret = session->HandleActionUpdateAvoidAreaOption(sessionProperty, action);
    ASSERT_EQ(WMError::WM_OK, ret);

    sessionProperty->SetAvoidAreaOption(2);
    sessionProperty->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = session->HandleActionUpdateAvoidAreaOption(sessionProperty, action);
    ASSERT_EQ(WMError::WM_OK, ret);

    sessionProperty->SetAvoidAreaOption(3);
    sessionProperty->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    ret = session->HandleActionUpdateAvoidAreaOption(sessionProperty, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_WINDOW, ret);
}

/**
 * @tc.name: GetSystemAvoidArea
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, GetSystemAvoidArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetSystemAvoidArea";
    info.bundleName_ = "GetSystemAvoidArea";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto specificCallback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sceneSession->isActive_ = true;
    SystemSessionConfig systemConfig;
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sceneSession->SetSystemConfig(systemConfig);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    sceneSession->GetSessionProperty()->SetDisplayId(2024);
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(2024, screenSession));
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID));
    sceneSession->SetSessionProperty(property);

    WSRect rect1({0, 0, 10, 10});
    AvoidArea avoidArea;
    sceneSession->GetSystemAvoidArea(rect1, avoidArea);
    WSRect rect2({0, 0, 10, 10});
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionProperty(property);
    sceneSession->GetSystemAvoidArea(rect2, avoidArea);
    ASSERT_EQ(avoidArea.topRect_.posX_, 0);

    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID));
    sceneSession->SetSessionProperty(property);
    ASSERT_EQ(avoidArea.topRect_.posX_, 0);
}

/**
 * @tc.name: CheckGetAvoidAreaAvailable
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, CheckGetAvoidAreaAvailable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CheckGetAvoidAreaAvailable";
    info.bundleName_ = "CheckGetAvoidAreaAvailable";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    SystemSessionConfig systemConfig;
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    session->SetSystemConfig(systemConfig);
    sptr<WindowSessionProperty> property = session->GetSessionProperty();
    
    // main window
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->SetSessionProperty(property);
    EXPECT_EQ(false, session->CheckGetAvoidAreaAvailable(AvoidAreaType::TYPE_NAVIGATION_INDICATOR));

    // pad free multi window mode
    systemConfig.windowUIType_ = WindowUIType::PAD_WINDOW;
    systemConfig.freeMultiWindowEnable_ = true;
    systemConfig.freeMultiWindowSupport_ = true;
    session->SetSystemConfig(systemConfig);
    EXPECT_EQ(false, session->CheckGetAvoidAreaAvailable(AvoidAreaType::TYPE_SYSTEM));

    // phone
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    systemConfig.freeMultiWindowEnable_ = false;
    systemConfig.freeMultiWindowSupport_ = false;
    session->SetSystemConfig(systemConfig);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    session->SetSessionProperty(property);
    EXPECT_EQ(true, session->CheckGetAvoidAreaAvailable(AvoidAreaType::TYPE_SYSTEM));

    // PC
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->SetSessionProperty(property);
    systemConfig.windowUIType_ = WindowUIType::PC_WINDOW;
    session->SetSystemConfig(systemConfig);
    EXPECT_EQ(false, session->CheckGetAvoidAreaAvailable(AvoidAreaType::TYPE_SYSTEM));
}

/**
 * @tc.name: CheckGetAvoidAreaAvailable02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, CheckGetAvoidAreaAvailable02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CheckGetAvoidAreaAvailable02";
    info.bundleName_ = "CheckGetAvoidAreaAvailable02";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    SystemSessionConfig systemConfig;
    sptr<WindowSessionProperty> property = session->GetSessionProperty();
    // system window
    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_FLOAT);
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    session->SetSystemConfig(systemConfig);
    property->SetAvoidAreaOption(static_cast<uint32_t>(AvoidAreaOption::ENABLE_SYSTEM_WINDOW));
    session->SetSessionProperty(property);
    EXPECT_EQ(true, session->CheckGetAvoidAreaAvailable(AvoidAreaType::TYPE_SYSTEM));

    property->SetAvoidAreaOption(0);
    session->SetSessionProperty(property);
    EXPECT_EQ(false, session->CheckGetAvoidAreaAvailable(AvoidAreaType::TYPE_SYSTEM));

    // sub window
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    session->SetSystemConfig(systemConfig);
    property->SetAvoidAreaOption(static_cast<uint32_t>(AvoidAreaOption::ENABLE_APP_SUB_WINDOW));
    session->SetSessionProperty(property);
    EXPECT_EQ(true, session->CheckGetAvoidAreaAvailable(AvoidAreaType::TYPE_SYSTEM));

    property->SetAvoidAreaOption(0);
    session->SetSessionProperty(property);
    EXPECT_EQ(false, session->CheckGetAvoidAreaAvailable(AvoidAreaType::TYPE_SYSTEM));

    // UIExtension
    property->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    session->SetSessionProperty(property);
    EXPECT_EQ(false, session->CheckGetAvoidAreaAvailable(AvoidAreaType::TYPE_SYSTEM));
}

/**
 * @tc.name: CheckGetMainWindowAvoidAreaAvailable
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, CheckGetMainWindowAvoidAreaAvailable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CheckGetMainWindowAvoidAreaAvailable";
    info.bundleName_ = "CheckGetMainWindowAvoidAreaAvailable";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = session->GetSessionProperty();

    SystemSessionConfig systemConfig;
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    session->SetSystemConfig(systemConfig);
    EXPECT_EQ(false, session->CheckGetMainWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
        AvoidAreaType::TYPE_NAVIGATION_INDICATOR));

    // pad free multi window mode
    systemConfig.windowUIType_ = WindowUIType::PAD_WINDOW;
    systemConfig.freeMultiWindowEnable_ = true;
    systemConfig.freeMultiWindowSupport_ = true;
    session->SetSystemConfig(systemConfig);
    EXPECT_EQ(false, session->CheckGetMainWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
        AvoidAreaType::TYPE_SYSTEM));
    
    // pc
    systemConfig.windowUIType_ = WindowUIType::PC_WINDOW;
    systemConfig.freeMultiWindowEnable_ = false;
    systemConfig.freeMultiWindowSupport_ = false;
    session->SetSystemConfig(systemConfig);
    EXPECT_EQ(false, session->CheckGetMainWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
        AvoidAreaType::TYPE_SYSTEM));

    // phone
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    systemConfig.freeMultiWindowEnable_ = false;
    systemConfig.freeMultiWindowSupport_ = false;
    session->SetSystemConfig(systemConfig);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    session->SetSessionProperty(property);
    EXPECT_EQ(true, session->CheckGetMainWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FULLSCREEN,
        AvoidAreaType::TYPE_SYSTEM));
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->SetSessionProperty(property);
    EXPECT_EQ(true, session->CheckGetMainWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
        AvoidAreaType::TYPE_SYSTEM));
}


/**
 * @tc.name: CheckGetMainWindowAvoidAreaAvailable02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, CheckGetMainWindowAvoidAreaAvailable02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CheckGetMainWindowAvoidAreaAvailable";
    info.bundleName_ = "CheckGetMainWindowAvoidAreaAvailable";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = session->GetSessionProperty();

    SystemSessionConfig systemConfig;
    // pad
    systemConfig.windowUIType_ = WindowUIType::PAD_WINDOW;
    systemConfig.freeMultiWindowEnable_ = false;
    systemConfig.freeMultiWindowSupport_ = false;
    session->SetSystemConfig(systemConfig);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->SetSessionProperty(property);
    EXPECT_EQ(true, session->CheckGetMainWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
        AvoidAreaType::TYPE_SYSTEM));

    // PC
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->SetSessionProperty(property);
    systemConfig.windowUIType_ = WindowUIType::PC_WINDOW;
    session->SetSystemConfig(systemConfig);
    EXPECT_EQ(false, session->CheckGetMainWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
        AvoidAreaType::TYPE_SYSTEM));
    
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToImmersive(true);
    property->SetCompatibleModeProperty(compatibleModeProperty);
    session->SetSessionProperty(property);
    EXPECT_EQ(true, session->CheckGetMainWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
        AvoidAreaType::TYPE_SYSTEM));
}

/**
 * @tc.name: CheckGetSubWindowAvoidAreaAvailable
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, CheckGetSubWindowAvoidAreaAvailable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CheckGetSubWindowAvoidAreaAvailable";
    info.bundleName_ = "CheckGetSubWindowAvoidAreaAvailable";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    SystemSessionConfig systemConfig;
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    session->SetSystemConfig(systemConfig);

    sptr<WindowSessionProperty> property = session->GetSessionProperty();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->SetAvoidAreaOption(static_cast<uint32_t>(AvoidAreaOption::ENABLE_APP_SUB_WINDOW));
    session->SetSessionProperty(property);
    EXPECT_EQ(true, session->CheckGetSubWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
            AvoidAreaType::TYPE_SYSTEM));

    property->SetAvoidAreaOption(0);
    session->SetSessionProperty(property);
    EXPECT_EQ(false, session->CheckGetSubWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
        AvoidAreaType::TYPE_SYSTEM));

    systemConfig.windowUIType_ = WindowUIType::PAD_WINDOW;
    systemConfig.freeMultiWindowEnable_ = true;
    systemConfig.freeMultiWindowSupport_ = true;
    session->SetSystemConfig(systemConfig);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->SetSessionProperty(property);
    EXPECT_EQ(false, session->CheckGetSubWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
        AvoidAreaType::TYPE_SYSTEM));
    
    systemConfig.windowUIType_ = WindowUIType::PC_WINDOW;
    systemConfig.freeMultiWindowEnable_ = false;
    systemConfig.freeMultiWindowSupport_ = false;
    session->SetSystemConfig(systemConfig);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->SetSessionProperty(property);
    EXPECT_EQ(false, session->CheckGetSubWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
        AvoidAreaType::TYPE_SYSTEM));

    systemConfig.freeMultiWindowEnable_ = false;
    systemConfig.freeMultiWindowSupport_ = false;
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    session->SetSystemConfig(systemConfig);

    sptr<SceneSession> parentSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    SystemSessionConfig parentSystemConfig;
    parentSystemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    parentSession->SetSystemConfig(parentSystemConfig);
    sptr<WindowSessionProperty> parentProperty = parentSession->GetSessionProperty();
    parentProperty->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    parentProperty->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    parentSession->SetSessionProperty(parentProperty);
    session->SetParentSession(parentSession);
    EXPECT_EQ(false, session->CheckGetSubWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
        AvoidAreaType::TYPE_NAVIGATION_INDICATOR));

    WSRect sessionRect({0, 0, 10, 10});
    WSRect parentSessionRect({0, 0, 100, 100});
    session->SetSessionRect(sessionRect);
    parentSession->SetSessionRect(parentSessionRect);
    EXPECT_EQ(false, session->CheckGetSubWindowAvoidAreaAvailable(WindowMode::WINDOW_MODE_FLOATING,
        AvoidAreaType::TYPE_NAVIGATION_INDICATOR));
}

/**
 * @tc.name: CheckGetSystemWindowAvoidAreaAvailable
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, CheckGetSystemWindowAvoidAreaAvailable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CheckGetSystemWindowAvoidAreaAvailable";
    info.bundleName_ = "CheckGetSystemWindowAvoidAreaAvailable";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    SystemSessionConfig systemConfig;
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    session->SetSystemConfig(systemConfig);

    sptr<WindowSessionProperty> property = session->GetSessionProperty();
    property->SetAvoidAreaOption(static_cast<uint32_t>(AvoidAreaOption::ENABLE_SYSTEM_WINDOW));
    session->SetSessionProperty(property);
    EXPECT_EQ(true, session->CheckGetSystemWindowAvoidAreaAvailable());

    property->SetAvoidAreaOption(0);
    session->SetSessionProperty(property);
    EXPECT_EQ(false, session->CheckGetSystemWindowAvoidAreaAvailable());
}

/**
 * @tc.name: GetAvoidAreaBytype
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, GetAvoidAreaBytype, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetAvoidAreaBytype";
    info.bundleName_ = "GetAvoidAreaBytype";
    WSRect rect({0, 0, 10, 10});
    AvoidArea avoidArea;
    std::vector<sptr<SceneSession>> sessionVector;
    for (int i = 0; i < 1000; i++) {
        sessionVector.push_back(sptr<SceneSession>::MakeSptr(info, nullptr));
        sessionVector[i]->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_FLOAT);
        sessionVector[i]->GetSessionProperty()->SetAvoidAreaOption(
            static_cast<uint32_t>(AvoidAreaOption::ENABLE_SYSTEM_WINDOW));
    }
    for (int i = 0; i < 1000; i++) {
        avoidArea = sessionVector[i]->GetAvoidAreaByType(AvoidAreaType::TYPE_SYSTEM, rect);
        ASSERT_EQ(avoidArea.topRect_.posX_, 0);
    }
}

/**
 * @tc.name: NotifyFrameLayoutFinishFromAppTest
 * @tc.desc: NotifyFrameLayoutFinishFromApp test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, NotifyFrameLayoutFinishFromAppTest, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyFrameLayoutFinishFromAppTest";
    info.bundleName_ = "NotifyFrameLayoutFinishFromAppTest";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    bool notifyListener = true;
    NotifySessionExceptionFunc func = [](const OHOS::Rosen::SessionInfo &sessionInfo,
                                         const OHOS::Rosen::ExceptionInfo &exceptionInfo,
                                         bool needRecover) {};
    session->sessionExceptionFunc_ = func;
    WSRect rect = { 200, 200, 200, 200 };
    WSError res = session->NotifyFrameLayoutFinishFromApp(notifyListener, rect);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: NotifyFrameLayoutFinishFromAppTest001
 * @tc.desc: NotifyFrameLayoutFinishFromApp test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, NotifyFrameLayoutFinishFromAppTest001, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyFrameLayoutFinishFromAppTest001";
    info.bundleName_ = "NotifyFrameLayoutFinishFromAppTest001";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    bool notifyListener = true;
    NotifySessionExceptionFunc func = nullptr;
    session->sessionExceptionFunc_ = func;
    WSRect rect = { 200, 200, 200, 200 };
    WSError res = session->NotifyFrameLayoutFinishFromApp(notifyListener, rect);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: NotifyFrameLayoutFinishFromAppTest002
 * @tc.desc: NotifyFrameLayoutFinishFromApp test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, NotifyFrameLayoutFinishFromAppTest002, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyFrameLayoutFinishFromAppTest002";
    info.bundleName_ = "NotifyFrameLayoutFinishFromAppTest002";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    bool notifyListener = false;
    NotifySessionExceptionFunc func = nullptr;
    session->sessionExceptionFunc_ = func;
    WSRect rect = { 200, 200, 200, 200 };
    WSError res = session->NotifyFrameLayoutFinishFromApp(notifyListener, rect);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: NotifyFrameLayoutFinishFromAppTest003
 * @tc.desc: NotifyFrameLayoutFinishFromApp test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, NotifyFrameLayoutFinishFromAppTest003, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyFrameLayoutFinishFromAppTest003";
    info.bundleName_ = "NotifyFrameLayoutFinishFromAppTest003";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    // Mock 1) anco app; 2) SupportNotifyFrameLayoutFinish; 3) not sa calling
    session->collaboratorType_ = CollaboratorType::RESERVE_TYPE;
    system::SetBoolParameter("prop_to_param.support.notifyFrameLayoutFinish", true);
    MockAccesstokenKit::MockIsSACalling(false);

    WSRect rect = { 200, 200, 200, 200 };
    WSError res = session->NotifyFrameLayoutFinishFromApp(true, rect);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: NotifyFrameLayoutFinishFromAppTest004
 * @tc.desc: NotifyFrameLayoutFinishFromApp test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, NotifyFrameLayoutFinishFromAppTest004, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyFrameLayoutFinishFromAppTest004";
    info.bundleName_ = "NotifyFrameLayoutFinishFromAppTest004";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    // Mock 1) anco app; 2) SupportNotifyFrameLayoutFinish; 3) is sa calling
    session->collaboratorType_ = CollaboratorType::RESERVE_TYPE;
    system::SetBoolParameter("prop_to_param.support.notifyFrameLayoutFinish", true);
    MockAccesstokenKit::MockIsSACalling(true);

    WSRect rect = { 200, 200, 200, 200 };
    WSError res = session->NotifyFrameLayoutFinishFromApp(true, rect);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: NotifyFrameLayoutFinishFromAppTest005
 * @tc.desc: NotifyFrameLayoutFinishFromApp test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, NotifyFrameLayoutFinishFromAppTest005, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyFrameLayoutFinishFromAppTest005";
    info.bundleName_ = "NotifyFrameLayoutFinishFromAppTest005";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    // Mock 1) anco app; 2) not SupportNotifyFrameLayoutFinish
    session->collaboratorType_ = CollaboratorType::RESERVE_TYPE;
    system::SetBoolParameter("prop_to_param.support.notifyFrameLayoutFinish", false);

    WSRect rect = { 200, 200, 200, 200 };
    WSError res = session->NotifyFrameLayoutFinishFromApp(true, rect);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: NotifyFrameLayoutFinishFromAppTest006
 * @tc.desc: NotifyFrameLayoutFinishFromApp test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, NotifyFrameLayoutFinishFromAppTest006, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyFrameLayoutFinishFromAppTest006";
    info.bundleName_ = "NotifyFrameLayoutFinishFromAppTest006";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    // Mock 1) not anco app
    WSRect rect = { 200, 200, 200, 200 };
    WSError res = session->NotifyFrameLayoutFinishFromApp(true, rect);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: UpdateWaterfallModeTest
 * @tc.desc: UpdateWaterfallMode test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, UpdateWaterfallModeTest, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateWaterfallModeTest";
    info.bundleName_ = "UpdateWaterfallModeTest";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->pcFoldScreenController_ = nullptr;
    SessionEvent event = SessionEvent::EVENT_MAXIMIZE_WATERFALL;
    session->UpdateWaterfallMode(event);

    session->pcFoldScreenController_ = sptr<PcFoldScreenController>::MakeSptr(wptr(session),
        session->GetPersistentId());
    event = SessionEvent::EVENT_MAXIMIZE_WATERFALL;
    session->UpdateWaterfallMode(event);

    event = SessionEvent::EVENT_WATERFALL_TO_MAXIMIZE;
    session->UpdateWaterfallMode(event);
    ASSERT_EQ(session->pcFoldScreenController_->isFullScreenWaterfallMode_, false);

    event = SessionEvent::EVENT_END;
    session->UpdateWaterfallMode(event);
    ASSERT_NE(session->pcFoldScreenController_, nullptr);
}

/**
 * @tc.name: SyncSessionEventTest
 * @tc.desc: SyncSessionEvent function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SyncSessionEventTest, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SyncSessionEventTest";
    info.bundleName_ = "SyncSessionEventTest";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    SessionEvent event = SessionEvent::EVENT_END;
    auto result = sceneSession->SyncSessionEvent(event);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: SyncSessionEventTest001
 * @tc.desc: SyncSessionEvent function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SyncSessionEventTest001, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SyncSessionEventTest001";
    info.bundleName_ = "SyncSessionEventTest001";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    property->isSystemCalling_ = true;
    sceneSession->SetSessionProperty(property);
    sceneSession->isActive_ = false;
    sceneSession->moveDragController_ = nullptr;

    SessionEvent event = SessionEvent::EVENT_START_MOVE;
    auto result = sceneSession->SyncSessionEvent(event);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: SyncSessionEventTest002
 * @tc.desc: SyncSessionEvent function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SyncSessionEventTest002, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SyncSessionEventTest002";
    info.bundleName_ = "SyncSessionEventTest002";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    property->isSystemCalling_ = true;
    sceneSession->SetSessionProperty(property);
    sceneSession->isActive_ = false;
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));

    SessionEvent event = SessionEvent::EVENT_END_MOVE;
    sceneSession->moveDragController_->isStartMove_ = true;
    auto result = sceneSession->SyncSessionEvent(event);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SyncSessionEventTest003
 * @tc.desc: SyncSessionEvent function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SyncSessionEventTest003, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SyncSessionEventTest003";
    info.bundleName_ = "SyncSessionEventTest003";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    property->isSystemCalling_ = true;
    sceneSession->SetSessionProperty(property);
    sceneSession->isActive_ = false;
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));

    SessionEvent event = SessionEvent::EVENT_END_MOVE;
    sceneSession->moveDragController_->isStartMove_ = false;
    auto result = sceneSession->SyncSessionEvent(event);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SyncSessionEventTest004
 * @tc.desc: SyncSessionEvent function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SyncSessionEventTest004, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SyncSessionEventTest004";
    info.bundleName_ = "SyncSessionEventTest004";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    property->isSystemCalling_ = true;
    sceneSession->SetSessionProperty(property);
    sceneSession->isActive_ = false;
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));

    SessionEvent event = SessionEvent::EVENT_START_MOVE;
    sceneSession->moveDragController_->isStartMove_ = false;
    auto result = sceneSession->SyncSessionEvent(event);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SyncSessionEventTest005
 * @tc.desc: SyncSessionEvent function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SyncSessionEventTest005, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SyncSessionEventTest005";
    info.bundleName_ = "SyncSessionEventTest005";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    property->isSystemCalling_ = true;
    sceneSession->SetSessionProperty(property);
    sceneSession->isActive_ = false;
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));

    SessionEvent event = SessionEvent::EVENT_START_MOVE;
    sceneSession->moveDragController_->isStartMove_ = true;
    auto result = sceneSession->SyncSessionEvent(event);
    ASSERT_EQ(result, WSError::WS_ERROR_REPEAT_OPERATION);
}

/**
 * @tc.name: SetWinRectWhenUpdateRectTest
 * @tc.desc: SetWinRectWhenUpdateRect function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetWinRectWhenUpdateRectTest, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWinRectWhenUpdateRectTest";
    info.bundleName_ = "SetWinRectWhenUpdateRectTest";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WSRect rect = { 10, 10, 200, 200 };
    sceneSession->isMidScene_ = false;
    sceneSession->SetWinRectWhenUpdateRect(rect);
    EXPECT_EQ(sceneSession->GetSessionRect().posX_, rect.posX_);
    EXPECT_EQ(sceneSession->GetSessionRect().posY_, rect.posY_);
}

/**
 * @tc.name: SetWinRectWhenUpdateRectTest001
 * @tc.desc: SetWinRectWhenUpdateRect function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetWinRectWhenUpdateRectTest001, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWinRectWhenUpdateRectTest001";
    info.bundleName_ = "SetWinRectWhenUpdateRectTest001";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WSRect rect = { 10, 10, 200, 200 };
    sceneSession->isMidScene_ = true;
    sceneSession->SetWinRectWhenUpdateRect(rect);
    EXPECT_EQ(sceneSession->GetSessionRect().posX_, rect.posX_);
    EXPECT_EQ(sceneSession->GetSessionRect().posY_, rect.posY_);
}

/**
 * @tc.name: SetWinRectWhenUpdateRectTest002
 * @tc.desc: SetWinRectWhenUpdateRect function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetWinRectWhenUpdateRectTest002, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWinRectWhenUpdateRectTest002";
    info.bundleName_ = "SetWinRectWhenUpdateRectTest002";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WSRect rect = { 0, 10, 200, 200 };
    sceneSession->isMidScene_ = true;
    sceneSession->SetWinRectWhenUpdateRect(rect);
    EXPECT_EQ(sceneSession->GetSessionRect().posX_, rect.posX_);
    EXPECT_EQ(sceneSession->GetSessionRect().posY_, rect.posY_);
}

/**
 * @tc.name: SetWinRectWhenUpdateRectTest003
 * @tc.desc: SetWinRectWhenUpdateRect function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetWinRectWhenUpdateRectTest003, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWinRectWhenUpdateRectTest003";
    info.bundleName_ = "SetWinRectWhenUpdateRectTest003";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WSRect rect = { 0, 0, 200, 200 };
    sceneSession->isMidScene_ = true;
    sceneSession->SetWinRectWhenUpdateRect(rect);
    EXPECT_EQ(sceneSession->GetSessionRect().width_, rect.width_);
    EXPECT_EQ(sceneSession->GetSessionRect().height_, rect.height_);
}

/**
 * @tc.name: SetPipParentWindowIdTest
 * @tc.desc: SetPipParentWindowId function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, SetPipParentWindowIdTest, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetPipParentWindowId";
    info.bundleName_ = "SetPipParentWindowId";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sceneSession->isTerminating_ = false;
    uint32_t id = 100;
    auto result = sceneSession->SetPipParentWindowId(id);
    ASSERT_EQ(result, WSError::WS_OK);
 
    auto cb = [](uint32_t windowId) {};
    sceneSession->SetPipParentWindowIdCallback(cb);
 
    result = sceneSession->SetPipParentWindowId(id);
    ASSERT_EQ(result, WSError::WS_OK);
 
    sceneSession->isTerminating_ = true;
    result = sceneSession->SetPipParentWindowId(id);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: IsPiPActiveTest
 * @tc.desc: IsPiPActive function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, IsPiPActiveTest, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "IsPiPActive";
    info.bundleName_ = "IsPiPActive";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sceneSession->pipActiveStatus_.store(true);
    bool status = false;
    ASSERT_EQ(sceneSession->IsPiPActive(status), WMError::WM_OK);
    ASSERT_EQ(status, true);
}

/**
 * @tc.name: UpdatePiPTemplateInfoTest
 * @tc.desc: UpdatePiPTemplateInfo function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, UpdatePiPTemplateInfoTest, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "UpdatePiPTemplateInfo";
    info.bundleName_ = "UpdatePiPTemplateInfo";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sceneSession->isTerminating_ = false;
    PiPTemplateInfo templateInfo;
    auto result = sceneSession->UpdatePiPTemplateInfo(templateInfo);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: RegisterGetIsDockAutoHideFunc
 * @tc.desc: RegisterGetIsDockAutoHideFunc function test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest4, RegisterGetIsDockAutoHideFunc, Function | SmallTest | Level1)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->RegisterGetIsDockAutoHideFunc([](){ return false; });
    EXPECT_NE(sceneSession->onGetIsDockAutoHideFunc_, nullptr);
}
}
}
}