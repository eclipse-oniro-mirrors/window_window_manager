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

#include "common/include/session_permission.h"
#include "display_manager.h"
#include "input_event.h"
#include "key_event.h"

#include "mock/mock_session_stage.h"
#include "mock/mock_scene_session.h"
#include "pointer_event.h"

#include "screen_manager.h"
#include "session/host/include/sub_session.h"
#include "session/host/include/main_session.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/system_session.h"
#include "session/screen/include/screen_session.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "wm_common.h"
#include "window_helper.h"
#include "ui/rs_surface_node.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {

class GetKeyboardGravitySceneSession : public SceneSession {
public:
    GetKeyboardGravitySceneSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback)
        : SceneSession(info, specificCallback)
    {
    }

    SessionGravity GetKeyboardGravity() const override
    {
        return SessionGravity::SESSION_GRAVITY_FLOAT;
    }
};

class SceneSessionTest5 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionTest5::SetUpTestCase()
{
}

void SceneSessionTest5::TearDownTestCase()
{
}

void SceneSessionTest5::SetUp()
{
}

void SceneSessionTest5::TearDown()
{
}

namespace {

/**
 * @tc.name: GetSystemAvoidArea
 * @tc.desc: GetSystemAvoidArea function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, GetSystemAvoidArea, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "GetSystemAvoidArea";
    info.bundleName_ = "GetSystemAvoidArea";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    WSRect rect;
    AvoidArea avoidArea;
    session->property_->SetWindowFlags(0);
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    session->isDisplayStatusBarTemporarily_.store(true);
    session->GetSystemAvoidArea(rect, avoidArea);
    ASSERT_NE(session->GetSessionProperty(), nullptr);
    EXPECT_EQ(WindowMode::WINDOW_MODE_UNDEFINED, session->GetSessionProperty()->GetWindowMode());

    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_END);
    session->isDisplayStatusBarTemporarily_.store(false);
    session->GetSystemAvoidArea(rect, avoidArea);

    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    SystemSessionConfig systemConfig;
    systemConfig.windowUIType_ = WindowUIType::PC_WINDOW;
    session->SetSystemConfig(systemConfig);
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    session->specificCallback_ = specificCallback;
    session->specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ = nullptr;
    session->GetSystemAvoidArea(rect, avoidArea);

    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    GetSceneSessionVectorByTypeAndDisplayIdCallback func = [&session](WindowType type, uint64_t displayId) {
        std::vector<sptr<SceneSession>> vSession;
        vSession.push_back(session);
        return vSession;
    };
    session->property_->SetDisplayId(2024);
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(2024, screenSession));
    session->specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ = func;
    session->GetSystemAvoidArea(rect, avoidArea);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
}

/**
 * @tc.name: HookAvoidAreaInCompatibleMode
 * @tc.desc: HookAvoidAreaInCompatibleMode function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HookAvoidAreaInCompatibleMode, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HookAvoidAreaInCompatibleMode";
    info.bundleName_ = "HookAvoidAreaInCompatibleMode";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    WSRect rect = {800, 100, 1000, 1000};
    AvoidArea avoidArea;
    avoidArea.topRect_ = {-1, -1, -1, -1};
    avoidArea.bottomRect_ = {-1, -1, -1, -1};
    Rect invalidRect = {-1, -1, -1, -1};
    // hook Func only support compatibleMode
    session->SetCompatibleModeInPc(false, true);
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->HookAvoidAreaInCompatibleMode(rect, avoidArea, AvoidAreaType::TYPE_SYSTEM);
    EXPECT_TRUE(avoidArea.topRect_ == invalidRect);
    session->SetCompatibleModeInPc(true, true);
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    session->HookAvoidAreaInCompatibleMode(rect, avoidArea, AvoidAreaType::TYPE_SYSTEM);
    EXPECT_TRUE(avoidArea.topRect_ == invalidRect);

    // test top system avoidArea
    session->SetCompatibleModeInPc(true, true);
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->HookAvoidAreaInCompatibleMode(rect, avoidArea, AvoidAreaType::TYPE_SYSTEM);
    auto vpr = 3.5f;
    Rect targetRect = {rect.posX_, rect.posY_, 40 * vpr, rect.width_};
    EXPECT_FALSE(avoidArea.topRect_ == targetRect);

    // test buttom aiBar avoidArea
    session->HookAvoidAreaInCompatibleMode(rect, avoidArea, AvoidAreaType::TYPE_NAVIGATION_INDICATOR);
    targetRect = {rect.posX_, rect.posY_ + rect.height_ - 28 * vpr, rect.width_, 28 * vpr};
    EXPECT_TRUE(avoidArea.bottomRect_ == targetRect);
}

/**
 * @tc.name: GetSystemAvoidArea01
 * @tc.desc: GetSystemAvoidArea01 function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, GetSystemAvoidArea01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "GetSystemAvoidArea01";
    info.bundleName_ = "GetSystemAvoidArea01";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    WSRect rect;
    AvoidArea avoidArea;
    session->property_->SetWindowFlags(0);

    session->isDisplayStatusBarTemporarily_.store(false);

    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    SystemSessionConfig systemConfig;

    session->SetSystemConfig(systemConfig);
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    session->specificCallback_ = specificCallback;

    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    GetSceneSessionVectorByTypeAndDisplayIdCallback func = [&session](WindowType type, uint64_t displayId) {
        std::vector<sptr<SceneSession>> vSession;
        vSession.push_back(session);
        return vSession;
    };
    session->property_->SetDisplayId(2024);
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(2024, screenSession));
    session->specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ = func;
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();

    session->property_->SetDisplayId(1024);
    session->Session::SetFloatingScale(0.0f);

    session->Session::SetFloatingScale(0.5f);
    EXPECT_EQ(0.5f, session->Session::GetFloatingScale());
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    session->GetSystemAvoidArea(rect, avoidArea);
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->GetSystemAvoidArea(rect, avoidArea);
    rect.height_ = 3;
    rect.width_ =4;
    session->GetSystemAvoidArea(rect, avoidArea);

    session->isVisible_ = true;
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    session->GetSystemAvoidArea(rect, avoidArea);
}

/**
 * @tc.name: NotifyOutsideDownEvent
 * @tc.desc: NotifyOutsideDownEvent function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, NotifyOutsideDownEvent, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "NotifyOutsideDownEvent";
    info.bundleName_ = "NotifyOutsideDownEvent";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->SetPointerAction(2);
    pointerEvent->RemoveAllPointerItems();
    session->NotifyOutsideDownEvent(pointerEvent);
    pointerEvent->SetPointerAction(8);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(2024);
    pointerEvent->AddPointerItem(pointerItem);
    pointerEvent->SetPointerId(2024);
    session->NotifyOutsideDownEvent(pointerEvent);
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    session->specificCallback_ = specificCallback;
    session->specificCallback_->onOutsideDownEvent_ = nullptr;
    session->NotifyOutsideDownEvent(pointerEvent);
    OnOutsideDownEvent func = [](int32_t x, int32_t y) {
        return;
    };
    session->specificCallback_->onOutsideDownEvent_ = func;
    session->NotifyOutsideDownEvent(pointerEvent);
    auto res = pointerEvent->GetPointerItem(2024, pointerItem);
    EXPECT_EQ(true, res);

    pointerEvent->SetPointerAction(5);
    session->NotifyOutsideDownEvent(pointerEvent);
    pointerEvent->RemoveAllPointerItems();
}

/**
 * @tc.name: TransferPointerEventInnerTest
 * @tc.desc: TransferPointerEventInner function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, TransferPointerEventInner, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "TransferPointerEventInner";
    info.bundleName_ = "TransferPointerEventInner";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();

    info.isSystem_ = false;
    pointerEvent->SetPointerAction(9);

    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    session->specificCallback_ = specificCallback;
    session->specificCallback_->onSessionTouchOutside_ = nullptr;
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, session->TransferPointerEventInner(pointerEvent, false));

    NotifySessionTouchOutsideCallback func = [](int32_t persistentId) {
        return;
    };
    session->specificCallback_->onSessionTouchOutside_ = func;
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, session->TransferPointerEventInner(pointerEvent, false));
    pointerEvent->SetPointerAction(2);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, session->TransferPointerEventInner(pointerEvent, false));
}

/**
 * @tc.name: TransferPointerEventInnerTest001
 * @tc.desc: TransferPointerEventInner function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, TransferPointerEventInnerTest001, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "TransferPointerEventInnerTest001";
    info.bundleName_ = "TransferPointerEventInnerTest001";
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    session->property_->SetMaximizeMode(MaximizeMode::MODE_RECOVER);
    session->ClearDialogVector();
    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(2024, session->GetWindowType());
    SystemSessionConfig systemConfig;
    systemConfig.isSystemDecorEnable_ = false;
    systemConfig.decorWindowModeSupportType_ = 2;
    session->SetSystemConfig(systemConfig);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, session->TransferPointerEventInner(pointerEvent, false));

    session->BindDialogToParentSession(session);
    session->SetSessionState(SessionState::STATE_ACTIVE);
    pointerEvent->SetPointerAction(5);
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->property_->SetDragEnabled(false);
    systemConfig.isSystemDecorEnable_ = true;

    session->moveDragController_->isStartDrag_ = false;
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, session->TransferPointerEventInner(pointerEvent, false));

    pointerEvent->SetPointerAction(2);
    EXPECT_EQ(WSError::WS_OK, session->TransferPointerEventInner(pointerEvent, false));

    session->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    session->property_->SetDecorEnable(false);
    EXPECT_EQ(WSError::WS_OK, session->TransferPointerEventInner(pointerEvent, false));

    pointerEvent->SetPointerAction(5);
    session->property_->SetDragEnabled(true);
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    systemConfig.freeMultiWindowSupport_ = false;
    session->moveDragController_->isStartDrag_ = true;
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, session->TransferPointerEventInner(pointerEvent, false));

    systemConfig.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, session->TransferPointerEventInner(pointerEvent, false));
    session->ClearDialogVector();
}

/**
 * @tc.name: SetSurfaceBounds01
 * @tc.desc: SetSurfaceBounds function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSurfaceBounds01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetSurfaceBounds01";
    info.bundleName_ = "SetSurfaceBounds01";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_FLOAT);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = nullptr;
    WSRect preRect = { 20, 20, 800, 800 };
    WSRect rect = { 30, 30, 900, 900 };
    session->SetSessionRect(preRect);
    session->SetSurfaceBounds(rect, false);

    session->surfaceNode_ = surfaceNode;
    session->SetSurfaceBounds(rect, false);
    EXPECT_EQ(preRect, session->GetSessionRect());
}

/**
 * @tc.name: OnLayoutFullScreenChange
 * @tc.desc: OnLayoutFullScreenChange function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, OnLayoutFullScreenChange, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "OnLayoutFullScreenChange";
    info.bundleName_ = "OnLayoutFullScreenChange";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    NotifyLayoutFullScreenChangeFunc func = [](bool isLayoutFullScreen) {};
    session->onLayoutFullScreenChangeFunc_ = func;
    EXPECT_EQ(WSError::WS_OK, session->OnLayoutFullScreenChange(true));
}

/**
 * @tc.name: RegisterLayoutFullScreenChangeCallback
 * @tc.desc: test RegisterLayoutFullScreenChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, RegisterLayoutFullScreenChangeCallback, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "RegisterLayoutFullScreenChangeCallback";
    info.bundleName_ = "RegisterLayoutFullScreenChangeCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->onLayoutFullScreenChangeFunc_ = nullptr;
    NotifyLayoutFullScreenChangeFunc func = [](bool isLayoutFullScreen) {};

    sceneSession->RegisterLayoutFullScreenChangeCallback(std::move(func));
    ASSERT_NE(sceneSession->onLayoutFullScreenChangeFunc_, nullptr);
}

/**
 * @tc.name: OnDefaultDensityEnabled
 * @tc.desc: OnDefaultDensityEnabled function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, OnDefaultDensityEnabled, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "OnDefaultDensityEnabled";
    info.bundleName_ = "OnDefaultDensityEnabled";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    EXPECT_EQ(WSError::WS_OK, session->OnDefaultDensityEnabled(true));

    session->onDefaultDensityEnabledFunc_ = nullptr;
    EXPECT_EQ(WSError::WS_OK, session->OnDefaultDensityEnabled(true));

    NotifyDefaultDensityEnabledFunc func = [](bool isLDefaultDensityEnabled) {};
    session->onDefaultDensityEnabledFunc_ = func;
    EXPECT_EQ(WSError::WS_OK, session->OnDefaultDensityEnabled(true));
}

/**
 * @tc.name: UpdateSessionPropertyByAction
 * @tc.desc: UpdateSessionPropertyByAction function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateSessionPropertyByAction, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "UpdateSessionPropertyByAction";
    info.bundleName_ = "UpdateSessionPropertyByAction";
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, session->UpdateSessionPropertyByAction
        (nullptr, WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, session->UpdateSessionPropertyByAction
        (property, WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE));
}

/**
 * @tc.name: SetSessionRectChangeCallback
 * @tc.desc: SetSessionRectChangeCallback function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSessionRectChangeCallback, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetSessionRectChangeCallback";
    info.bundleName_ = "SetSessionRectChangeCallback";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    WSRect rec = { 1, 1, 1, 1 };
    NotifySessionRectChangeFunc func = [](const WSRect& rect, SizeChangeReason reason,
        DisplayId displayId, const RectAnimationConfig& rectAnimationConfig) {
        return;
    };
    session->SetSessionRectChangeCallback(nullptr);
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session->SetSessionRectChangeCallback(func);
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    session->SetSessionRectChangeCallback(func);
    rec.width_ = 0;
    session->SetSessionRectChangeCallback(func);
    rec.height_ = 0;
    session->SetSessionRectChangeCallback(func);
    EXPECT_EQ(WindowType::APP_MAIN_WINDOW_BASE, session->GetWindowType());
}

/**
 * @tc.name: SetSessionRectChangeCallback02
 * @tc.desc: SetSessionRectChangeCallback02 function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSessionRectChangeCallback02, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetSessionRectChangeCallback02";
    info.bundleName_ = "SetSessionRectChangeCallback02";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    WSRect rec = { 1, 1, 1, 1 };
    NotifySessionRectChangeFunc func = [](const WSRect& rect, SizeChangeReason reason,
        DisplayId displayId, const RectAnimationConfig& rectAnimationConfig) {
        return;
    };
    session->SetSessionRectChangeCallback(nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session->SetSessionProperty(property);
    session->SetSessionRectChangeCallback(func);

    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    session->SetSessionProperty(property);
    session->SetSessionRequestRect(rec);
    session->SetSessionRectChangeCallback(func);

    rec.width_ = 0;
    session->SetSessionRequestRect(rec);
    session->SetSessionRectChangeCallback(func);

    rec.height_ = 0;
    session->SetSessionRequestRect(rec);
    session->SetSessionRectChangeCallback(func);
    EXPECT_EQ(WindowType::APP_MAIN_WINDOW_BASE, session->GetWindowType());
}

/**
 * @tc.name: SetSessionRectChangeCallback03
 * @tc.desc: SetSessionRectChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSessionRectChangeCallback03, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetSessionRectChangeCallback03";
    info.bundleName_ = "SetSessionRectChangeCallback03";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->SetSessionProperty(property);
    WSRect rec = { 1, 1, 1, 1 };
    NotifySessionRectChangeFunc func = [](const WSRect& rect, const SizeChangeReason reason,
        DisplayId displayId, const RectAnimationConfig& rectAnimationConfig) {
        return;
    };
    session->SetSessionRequestRect(rec);
    session->SetSessionRectChangeCallback(nullptr);

    rec.width_ = 0;
    session->SetSessionRequestRect(rec);
    session->SetSessionRectChangeCallback(nullptr);

    rec.height_ = 0;
    rec.width_ = 1;
    session->SetSessionRequestRect(rec);
    session->SetSessionRectChangeCallback(nullptr);

    rec.height_ = 0;
    rec.width_ = 0;
    session->SetSessionRequestRect(rec);
    session->SetSessionRectChangeCallback(nullptr);
    EXPECT_EQ(WindowType::APP_MAIN_WINDOW_BASE, session->GetWindowType());
}

/**
 * @tc.name: GetSystemAvoidArea02
 * @tc.desc: GetSystemAvoidArea02 function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, GetSystemAvoidArea02, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "GetSystemAvoidArea02";
    info.bundleName_ = "GetSystemAvoidArea02";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    ASSERT_NE(session->GetSessionProperty(), nullptr);
    session->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);

    SystemSessionConfig systemConfig;
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    session->SetSystemConfig(systemConfig);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    session->GetSessionProperty()->SetDisplayId(1664);
    session->Session::SetFloatingScale(0.02f);
    WSRect rect;
    AvoidArea avoidArea;
    session->GetSystemAvoidArea(rect, avoidArea);
    EXPECT_EQ(0.02f, session->Session::GetFloatingScale());

    session->Session::SetFloatingScale(0.5f);
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    session->GetSystemAvoidArea(rect, avoidArea);
    session->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    rect.height_ = 2;
    rect.width_ =1 ;
    session->GetSystemAvoidArea(rect, avoidArea);
    rect.height_ = 1;
    session->GetSystemAvoidArea(rect, avoidArea);
}

/**
 * @tc.name: AdjustRectByAspectRatio
 * @tc.desc: AdjustRectByAspectRatio function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, AdjustRectByAspectRatio, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "AdjustRectByAspectRatio";
    info.bundleName_ = "AdjustRectByAspectRatio";
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    WSRect rect;
    EXPECT_EQ(false, session->AdjustRectByAspectRatio(rect));
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    EXPECT_EQ(false, session->AdjustRectByAspectRatio(rect));
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    EXPECT_EQ(false, session->AdjustRectByAspectRatio(rect));
    session->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    EXPECT_EQ(false, session->AdjustRectByAspectRatio(rect));
}

/**
 * @tc.name: AdjustRectByAspectRatio01
 * @tc.desc: AdjustRectByAspectRatio function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, AdjustRectByAspectRatio01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "AdjustRectByAspectRatio01";
    info.bundleName_ = "AdjustRectByAspectRatio01";
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    WSRect rect;
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    session->aspectRatio_ = 0.5f;
    EXPECT_NE(nullptr, DisplayManager::GetInstance().GetDefaultDisplay());

    SystemSessionConfig systemConfig;
    systemConfig.isSystemDecorEnable_ = true;
    systemConfig.decorWindowModeSupportType_ = 2;
    session->SetSystemConfig(systemConfig);
    EXPECT_EQ(true, session->AdjustRectByAspectRatio(rect));

    systemConfig.isSystemDecorEnable_ = false;
    EXPECT_EQ(false, session->AdjustRectByAspectRatio(rect));
}

/**
 * @tc.name: OnMoveDragCallback
 * @tc.desc: OnMoveDragCallback function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, OnMoveDragCallback, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "OnMoveDragCallback";
    info.bundleName_ = "OnMoveDragCallback";
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->SetRequestNextVsyncFunc([](const std::shared_ptr<VsyncCallback>& callback) {});
    EXPECT_NE(nullptr, session->requestNextVsyncFunc_);
    session->moveDragController_ = nullptr;
    SizeChangeReason reason = { SizeChangeReason::DRAG };
    session->OnMoveDragCallback(reason);
    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(2024, session->GetWindowType());
    session->OnMoveDragCallback(reason);

    reason = SizeChangeReason::DRAG_END;
    session->OnMoveDragCallback(reason);

    reason = SizeChangeReason::DRAG_MOVE;
    session->OnMoveDragCallback(reason);

    reason = SizeChangeReason::DRAG_START;
    session->OnMoveDragCallback(reason);
    EXPECT_EQ(WSError::WS_OK, session->UpdateSizeChangeReason(reason));

    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(2024, session->GetWindowType());
    EXPECT_NE(session->moveDragController_, nullptr);
    session->property_->compatibleModeInPc_ = true;
    session->OnMoveDragCallback(reason);
    EXPECT_EQ(WSError::WS_OK, session->UpdateSizeChangeReason(reason));
}

/**
 * @tc.name: UpdateWinRectForSystemBar
 * @tc.desc: UpdateWinRectForSystemBar function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateWinRectForSystemBar, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "UpdateWinRectForSystemBar";
    info.bundleName_ = "UpdateWinRectForSystemBar";
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCallback->onGetSceneSessionVectorByTypeAndDisplayId_ = nullptr;
    session->specificCallback_ = specificCallback;
    WSRect rect = { 1, 10, 3, 4 };
    session->UpdateWinRectForSystemBar(rect);
    GetSceneSessionVectorByTypeAndDisplayIdCallback func = [session](WindowType type, uint64_t displayId)->
        std::vector<sptr<SceneSession>>
    {
        std::vector<sptr<SceneSession>> vSession;
        vSession.push_back(session);
        return vSession;
    };
    specificCallback->onGetSceneSessionVectorByTypeAndDisplayId_ = func;
    session->UpdateWinRectForSystemBar(rect);

    session->isVisible_ = true;
    session->winRect_ = rect;
    session->UpdateWinRectForSystemBar(rect);

    WSRect rect1 = { 1, 2, 10, 4 };
    session->winRect_ = rect1;
    session->UpdateWinRectForSystemBar(rect);
    EXPECT_EQ(4, session->GetSessionRect().height_);

    WSRect rect2 = { 1, 2, 10, 8 };
    session->winRect_ = rect2;
    session->UpdateWinRectForSystemBar(rect);
}

/**
 * @tc.name: UpdateNativeVisibility
 * @tc.desc: UpdateNativeVisibility function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateNativeVisibility, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "UpdateNativeVisibility";
    info.bundleName_ = "UpdateNativeVisibility";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    NotifyWindowInfoUpdateCallback dateFunc = [](int32_t persistentId, WindowUpdateType type) {
        return;
    };
    UpdateAvoidAreaCallback areaFunc = [](const int32_t persistentId) {
        return;
    };
    specificCallback->onWindowInfoUpdate_ = dateFunc;
    specificCallback->onUpdateAvoidArea_ = areaFunc;
    session->specificCallback_ = specificCallback;
    session->UpdateNativeVisibility(true);
    session->UpdateNativeVisibility(false);
}

/**
 * @tc.name: SetPrivacyMode
 * @tc.desc: SetPrivacyMode function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetPrivacyMode, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetPrivacyMode";
    info.bundleName_ = "SetPrivacyMode";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = surfaceNode;
    ASSERT_NE(session->GetSessionProperty(), nullptr);
    session->GetSessionProperty()->SetPrivacyMode(true);
    EXPECT_EQ(true, session->GetSessionProperty()->GetPrivacyMode());
    session->leashWinSurfaceNode_ = nullptr;
    session->SetPrivacyMode(false);
    session->leashWinSurfaceNode_ = surfaceNode;
    session->SetPrivacyMode(true);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetPrivacyMode(true);
    session->SetSessionProperty(property);
    session->SetPrivacyMode(true);
}

/**
 * @tc.name: SetSnapshotSkip
 * @tc.desc: SetSnapshotSkip function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSnapshotSkip, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetSnapshotSkip";
    info.bundleName_ = "SetSnapshotSkip";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = surfaceNode;
    ASSERT_NE(session->GetSessionProperty(), nullptr);
    session->GetSessionProperty()->SetSnapshotSkip(true);
    EXPECT_EQ(true, session->GetSessionProperty()->GetSnapshotSkip());
    session->leashWinSurfaceNode_ = nullptr;
    session->SetSnapshotSkip(false);
    session->leashWinSurfaceNode_ = surfaceNode;
    session->SetSnapshotSkip(false);
    session->SetSnapshotSkip(true);
    session->surfaceNode_ = nullptr;
    EXPECT_NE(nullptr, session->GetLeashWinSurfaceNode());
    session->SetSnapshotSkip(true);
}

/**
 * @tc.name: SetWatermarkEnabled
 * @tc.desc: SetWatermarkEnabled function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetWatermarkEnabled, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetWatermarkEnabled";
    info.bundleName_ = "SetWatermarkEnabled";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    std::string watermarkName = "watermarkNameTest";
    session->SetWatermarkEnabled(watermarkName, true);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = surfaceNode;
    session->SetWatermarkEnabled(watermarkName, true);
}

/**
 * @tc.name: UIExtSurfaceNodeIdCache
 * @tc.desc: UIExtSurfaceNodeIdCache
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UIExtSurfaceNodeIdCache, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "UIExtSurfaceNodeIdCache";
    info.bundleName_ = "UIExtSurfaceNodeIdCache";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    session->AddUIExtSurfaceNodeId(1, 2);
    EXPECT_EQ(session->GetUIExtPersistentIdBySurfaceNodeId(1), 2);

    session->RemoveUIExtSurfaceNodeId(2);
    EXPECT_EQ(session->GetUIExtPersistentIdBySurfaceNodeId(1), 0);
}

/**
 * @tc.name: SetSystemSceneOcclusionAlpha
 * @tc.desc: SetSystemSceneOcclusionAlpha function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSystemSceneOcclusionAlpha, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetSystemSceneOcclusionAlpha";
    info.bundleName_ = "SetSystemSceneOcclusionAlpha";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    session->SetSystemSceneOcclusionAlpha(-0.325);
    session->SetSystemSceneOcclusionAlpha(3.14125);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = surfaceNode;
    session->leashWinSurfaceNode_ = nullptr;
    session->SetSystemSceneOcclusionAlpha(0.14125);
    session->leashWinSurfaceNode_ = surfaceNode;
    session->SetSystemSceneOcclusionAlpha(0.14125);
    EXPECT_NE(nullptr, session->GetLeashWinSurfaceNode());
}

/**
 * @tc.name: ResetOcclusionAlpha
 * @tc.desc: ResetOcclusionAlpha function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, ResetOcclusionAlpha, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ResetOcclusionAlpha";
    info.bundleName_ = "ResetOcclusionAlpha";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = surfaceNode;
    session->leashWinSurfaceNode_ = nullptr;
    session->ResetOcclusionAlpha();
    EXPECT_EQ(nullptr, session->GetLeashWinSurfaceNode());
    session->leashWinSurfaceNode_ = surfaceNode;
    session->ResetOcclusionAlpha();
    EXPECT_NE(nullptr, session->GetLeashWinSurfaceNode());
}

/**
 * @tc.name: SetSystemSceneForceUIFirst
 * @tc.desc: SetSystemSceneForceUIFirst function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSystemSceneForceUIFirst, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetSystemSceneForceUIFirst";
    info.bundleName_ = "SetSystemSceneForceUIFirst";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    session->SetSystemSceneForceUIFirst(true);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = surfaceNode;
    session->leashWinSurfaceNode_ = nullptr;
    session->SetSystemSceneForceUIFirst(true);
    session->leashWinSurfaceNode_ = surfaceNode;
    session->SetSystemSceneForceUIFirst(true);
    EXPECT_NE(nullptr, session->GetLeashWinSurfaceNode());
}

/**
 * @tc.name: UpdateWindowAnimationFlag
 * @tc.desc: UpdateWindowAnimationFlag function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateWindowAnimationFlag, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "UpdateWindowAnimationFlag";
    info.bundleName_ = "UpdateWindowAnimationFlag";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    EXPECT_EQ(WSError::WS_OK, session->UpdateWindowAnimationFlag(true));

    session->onWindowAnimationFlagChange_ = nullptr;
    EXPECT_EQ(WSError::WS_OK, session->UpdateWindowAnimationFlag(true));

    NotifyWindowAnimationFlagChangeFunc func = [](const bool flag) {
        return;
    };
    session->onWindowAnimationFlagChange_ = func;
    EXPECT_EQ(WSError::WS_OK, session->UpdateWindowAnimationFlag(true));
}

/**
 * @tc.name: SetForegroundInteractiveStatus
 * @tc.desc: SetForegroundInteractiveStatus function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetForegroundInteractiveStatus, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetForegroundInteractiveStatus";
    info.bundleName_ = "SetForegroundInteractiveStatus";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->SetSessionState(SessionState::STATE_FOREGROUND);
    EXPECT_NE(session, nullptr);
    session->toastSession_.clear();
    session->toastSession_.push_back(session);
    session->SetForegroundInteractiveStatus(false);
    session->toastSession_.clear();
    session->SetSessionState(SessionState::STATE_ACTIVE);
    session->toastSession_.push_back(session);
    session->SetForegroundInteractiveStatus(false);
    session->toastSession_.clear();
    session->SetSessionState(SessionState::STATE_CONNECT);
    session->toastSession_.push_back(session);
    session->SetForegroundInteractiveStatus(false);
    session->toastSession_.clear();
    session->SetSessionState(SessionState::STATE_ACTIVE);
    session->toastSession_.push_back(nullptr);
    session->SetForegroundInteractiveStatus(false);
    session->toastSession_.clear();
}

/**
 * @tc.name: HandleUpdatePropertyByAction
 * @tc.desc: HandleUpdatePropertyByAction function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleUpdatePropertyByAction, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HandleUpdatePropertyByAction";
    info.bundleName_ = "HandleUpdatePropertyByAction";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_RECT;
    auto res = session->HandleUpdatePropertyByAction(nullptr, action);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, res);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    res = session->HandleUpdatePropertyByAction(property, action);
    EXPECT_EQ(WMError::WM_DO_NOTHING, res);
    action = WSPropertyChangeAction::ACTION_UPDATE_FLAGS;
    res = session->HandleUpdatePropertyByAction(property, action);
    EXPECT_EQ(WMError::WM_OK, res);
    auto prop = session->GetSessionProperty();
    EXPECT_EQ(prop->GetSystemBarProperty(), property->GetSystemBarProperty());
}

/**
 * @tc.name: SetSystemWindowEnableDrag
 * @tc.desc: SetSystemWindowEnableDrag function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSystemWindowEnableDrag, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetSystemWindowEnableDrag";
    info.bundleName_ = "SetSystemWindowEnableDrag";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_DESKTOP);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto ret = session->SetWindowEnableDragBySystem(true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetWindowEnableDragBySystem
 * @tc.desc: SetWindowEnableDragBySystem function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetWindowEnableDragBySystem, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowEnableDrag";
    info.bundleName_ = "SetWindowEnableDrag";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto ret = session->SetWindowEnableDragBySystem(true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: HandleActionUpdateSetBrightness
 * @tc.desc: HandleActionUpdateSetBrightness function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdateSetBrightness, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateSetBrightness";
    info.bundleName_ = "HandleActionUpdateSetBrightness";
    info.windowType_ = static_cast<uint32_t>(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE);
    info.isSystem_ = true;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_RECT;
    auto res = session->HandleActionUpdateSetBrightness(property, action);
    EXPECT_EQ(WMError::WM_OK, res);

    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> session1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    res = session1->HandleActionUpdateSetBrightness(property, action);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_SESSION, res);

    info.isSystem_ = false;
    sptr<SceneSession> session2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    session2->SetSessionState(SessionState::STATE_CONNECT);
    res = session2->HandleActionUpdateSetBrightness(property, action);
    EXPECT_EQ(WMError::WM_OK, res);

    sptr<SceneSession> session3 = sptr<SceneSession>::MakeSptr(info, nullptr);
    session3->SetSessionState(SessionState::STATE_CONNECT);
    float brightness = 1.0;
    property->SetBrightness(brightness);
    EXPECT_EQ(brightness, property->GetBrightness());
    EXPECT_EQ(WSError::WS_OK, session3->SetBrightness(brightness));
    res = session3->HandleActionUpdateSetBrightness(property, action);
    EXPECT_EQ(WMError::WM_OK, res);
    EXPECT_EQ(brightness, session3->GetBrightness());

    sptr<SceneSession> session4 = sptr<SceneSession>::MakeSptr(info, nullptr);
    session4->SetSessionState(SessionState::STATE_CONNECT);
    brightness = 0.8;
    property->SetBrightness(brightness);
    EXPECT_EQ(brightness, property->GetBrightness());
    res = session4->HandleActionUpdateSetBrightness(property, action);
    EXPECT_EQ(WMError::WM_OK, res);
    EXPECT_EQ(brightness, session4->GetBrightness());
    brightness = 1.0;
    EXPECT_EQ(WSError::WS_OK, session4->SetBrightness(brightness));
    EXPECT_EQ(brightness, session4->GetBrightness());
}

/**
 * @tc.name: HandleActionUpdateMaximizeState
 * @tc.desc: HandleActionUpdateMaximizeState function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdateMaximizeState, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateMaximizeState";
    info.bundleName_ = "HandleActionUpdateMaximizeState";
    info.windowType_ = static_cast<uint32_t>(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_RECT;

    auto res = session->HandleActionUpdateMaximizeState(property, action);
    EXPECT_EQ(WMError::WM_OK, res);
    res = session->HandleActionUpdateMode(property, action);
    EXPECT_EQ(WMError::WM_OK, res);
    res = session->HandleActionUpdateAnimationFlag(property, action);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: SetUniqueDensityDpi
 * @tc.desc: SetUniqueDensityDpi function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetUniqueDensityDpi, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetUniqueDensityDpi";
    info.bundleName_ = "SetUniqueDensityDpi";
    info.windowType_ = static_cast<uint32_t>(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE);
    info.isSystem_ = true;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    session->sessionStage_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_SESSION, session->SetUniqueDensityDpi(true, 520));
    session->sessionInfo_.isSystem_ = false;
    session->state_ = SessionState::STATE_DISCONNECT;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_SESSION, session->SetUniqueDensityDpi(true, 520));
    session->state_ = SessionState::STATE_CONNECT;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, session->SetUniqueDensityDpi(true, 520));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, session->SetUniqueDensityDpi(true, 79));
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, session->SetUniqueDensityDpi(false, 79));

    session->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, session->sessionStage_);
}

/**
 * @tc.name: HandleActionUpdateWindowModeSupportType
 * @tc.desc: HandleActionUpdateWindowModeSupportType function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdateWindowModeSupportType, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateWindowModeSupportType";
    info.bundleName_ = "HandleActionUpdateWindowModeSupportType";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->isSystemCalling_ = false;
    session->SetSessionProperty(property);
    ASSERT_EQ(WMError::WM_ERROR_NOT_SYSTEM_APP, session->HandleActionUpdateWindowModeSupportType(property,
        WSPropertyChangeAction::ACTION_UPDATE_RECT));
}

/**
 * @tc.name: UpdateUIParam
 * @tc.desc: UpdateUIParam function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateUIParam, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "UpdateUIParam";
    info.bundleName_ = "UpdateUIParam";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    session->isFocused_ = true;
    session->isVisible_ = false;
    uint32_t res = session->UpdateUIParam();
    ASSERT_EQ(0, res);
    ASSERT_EQ(false, session->postProcessFocusState_.enabled_);

    session->isFocused_ = true;
    session->isVisible_ = true;
    uint32_t res1 = session->UpdateUIParam();
    ASSERT_EQ(1, res1);
    ASSERT_EQ(true, session->postProcessFocusState_.enabled_);
}

/**
 * @tc.name: UpdateVisibilityInner
 * @tc.desc: UpdateVisibilityInner function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateVisibilityInner, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "UpdateVisibilityInner";
    info.bundleName_ = "UpdateVisibilityInner";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    session->isVisible_ = true;
    ASSERT_EQ(false, session->UpdateVisibilityInner(true));
}

/**
 * @tc.name: UpdateInteractiveInner
 * @tc.desc: UpdateInteractiveInner function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateInteractiveInner, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "UpdateInteractiveInner";
    info.bundleName_ = "UpdateInteractiveInner";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    session->foregroundInteractiveStatus_.store(true);
    ASSERT_EQ(false, session->UpdateInteractiveInner(true));
    ASSERT_EQ(true, session->UpdateInteractiveInner(false));
}

/**
 * @tc.name: IsAnco
 * @tc.desc: IsAnco function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, IsAnco, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "IsAnco";
    info.bundleName_ = "IsAnco";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    bool res = session->IsAnco();
    EXPECT_EQ(res, false);

    session->collaboratorType_ = CollaboratorType::RESERVE_TYPE;
    res = session->IsAnco();
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: ProcessUpdatePropertyByAction
 * @tc.desc: ProcessUpdatePropertyByAction function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, ProcessUpdatePropertyByAction, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ProcessUpdatePropertyByAction";
    info.bundleName_ = "ProcessUpdatePropertyByAction";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);

    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_FOCUSABLE);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_TOUCHABLE);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_ORIENTATION);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE);
    session->ProcessUpdatePropertyByAction(
        property, WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_OTHER_PROPS);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_STATUS_PROPS);
    session->ProcessUpdatePropertyByAction(
        property, WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_FLAGS);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_MODE);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED);
    session->ProcessUpdatePropertyByAction(
        property, WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS);
    session->ProcessUpdatePropertyByAction(
        property, WSPropertyChangeAction::ACTION_UPDATE_TEXTFIELD_AVOID_INFO);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_TOPMOST);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO);
}

/**
 * @tc.name: HandleActionUpdateTurnScreenOn
 * @tc.desc: HandleActionUpdateTurnScreenOn function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdateTurnScreenOn, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateTurnScreenOn";
    info.bundleName_ = "HandleActionUpdateTurnScreenOn";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);

    auto res = session->HandleActionUpdateTurnScreenOn(
        property, WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON);
    EXPECT_EQ(res, WMError::WM_OK);

    property->SetTurnScreenOn(true);
    res = session->HandleActionUpdateTurnScreenOn(
        property, WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON);
    EXPECT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: HandleActionUpdatePrivacyMode1
 * @tc.desc: HandleActionUpdatePrivacyMode1
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdatePrivacyMode1, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdatePrivacyMode1";
    info.bundleName_ = "HandleActionUpdatePrivacyMode1";
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    EXPECT_NE(session->property_, nullptr);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = surfaceNode;
    session->property_->SetPrivacyMode(false);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPrivacyMode(true);
    auto res =
        session->HandleActionUpdatePrivacyMode(property, WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE);
    EXPECT_EQ(WMError::WM_OK, res);
    EXPECT_EQ(true, session->property_->GetPrivacyMode());
}

/**
 * @tc.name: HandleActionUpdatePrivacyMode2
 * @tc.desc: HandleActionUpdatePrivacyMode2
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdatePrivacyMode2, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdatePrivacyMode2";
    info.bundleName_ = "HandleActionUpdatePrivacyMode2";
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    EXPECT_NE(session->property_, nullptr);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = surfaceNode;
    session->property_->SetPrivacyMode(true);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPrivacyMode(false);
    auto res =
        session->HandleActionUpdatePrivacyMode(property, WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE);
    EXPECT_EQ(WMError::WM_OK, res);
    EXPECT_EQ(false, session->property_->GetPrivacyMode());
}

/**
 * @tc.name: UpdateClientRect01
 * @tc.desc: UpdateClientRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateClientRect01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "UpdateClientRect01";
    info.bundleName_ = "UpdateClientRect01";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    WSRect rect = { 0, 0, 0, 0 };
    EXPECT_EQ(session->UpdateClientRect(rect), WSError::WS_OK);

    session->SetClientRect(rect);
    rect.posX_ = 100;
    rect.posY_ = 100;
    rect.width_ = 800;
    rect.height_ = 800;
    EXPECT_EQ(session->UpdateClientRect(rect), WSError::WS_OK);

    session->SetClientRect(rect);
    EXPECT_EQ(session->UpdateClientRect(rect), WSError::WS_OK);
}

/**
 * @tc.name: ResetSizeChangeReasonIfDirty
 * @tc.desc: ResetSizeChangeReasonIfDirty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, ResetSizeChangeReasonIfDirty, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ResetSizeChangeReasonIfDirty";
    info.bundleName_ = "ResetSizeChangeReasonIfDirty";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    session->UpdateSizeChangeReason(SizeChangeReason::DRAG);
    session->ResetDirtyFlags();
    session->ResetSizeChangeReasonIfDirty();
    EXPECT_EQ(session->GetSizeChangeReason(), SizeChangeReason::DRAG);

    session->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    session->ResetSizeChangeReasonIfDirty();
    EXPECT_EQ(session->GetSizeChangeReason(), SizeChangeReason::DRAG);

    session->UpdateSizeChangeReason(SizeChangeReason::MOVE);
    session->ResetSizeChangeReasonIfDirty();
    EXPECT_EQ(session->GetSizeChangeReason(), SizeChangeReason::UNDEFINED);
}

/**
 * @tc.name: HandleMoveDragSurfaceNode
 * @tc.desc: HandleMoveDragSurfaceNode Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleMoveDragSurfaceNode, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HandleMoveDragSurfaceNode";
    info.bundleName_ = "HandleMoveDragSurfaceNode";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);

    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(2024, session->GetWindowType());
    EXPECT_NE(session->moveDragController_, nullptr);

    session->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_START);
    session->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG);
    session->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_MOVE);
    session->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_END);
}

/**
 * @tc.name: SetNotifyVisibleChangeFunc
 * @tc.desc: SetNotifyVisibleChangeFunc Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetNotifyVisibleChangeFunc, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    session->SetNotifyVisibleChangeFunc([](int32_t persistentId) {});
    EXPECT_NE(session->notifyVisibleChangeFunc_, nullptr);
}

/**
 * @tc.name: SetRequestNextVsyncFunc
 * @tc.desc: SetRequestNextVsyncFunc01 Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetRequestNextVsyncFunc01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);

    session->SetRequestNextVsyncFunc(nullptr);
    ASSERT_EQ(nullptr, session->requestNextVsyncFunc_);

    session->SetRequestNextVsyncFunc([](const std::shared_ptr<VsyncCallback>& callback) {
        SessionInfo info1;
        info1.abilityName_ = "test2";
        info1.bundleName_ = "test2";
    });
    ASSERT_NE(nullptr, session->requestNextVsyncFunc_);
}

/**
 * @tc.name: NotifyServerToUpdateRect01
 * @tc.desc: NotifyServerToUpdateRect01 Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, NotifyServerToUpdateRect01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "NotifyServerToUpdateRect01";
    info.bundleName_ = "NotifyServerToUpdateRect01";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    session->foregroundInteractiveStatus_.store(true);
    EXPECT_EQ(session->GetForegroundInteractiveStatus(), true);

    SessionUIParam uiParam;
    EXPECT_EQ(session->NotifyServerToUpdateRect(uiParam, SizeChangeReason::UNDEFINED), false);

    uiParam.rect_ = { 100, 100, 200, 200 };
    session->SetNeedSyncSessionRect(false);
    EXPECT_EQ(session->NotifyServerToUpdateRect(uiParam, SizeChangeReason::UNDEFINED), false); // not sync
    EXPECT_NE(session->GetSessionRect(), uiParam.rect_);

    uiParam.needSync_ = false;
    EXPECT_EQ(session->NotifyServerToUpdateRect(uiParam, SizeChangeReason::UNDEFINED), false); // not sync
    EXPECT_NE(session->GetSessionRect(), uiParam.rect_);

    uiParam.needSync_ = true;
    session->SetNeedSyncSessionRect(true); // sync first
    EXPECT_EQ(session->NotifyServerToUpdateRect(uiParam, SizeChangeReason::UNDEFINED), true);
    EXPECT_EQ(session->GetSessionRect(), uiParam.rect_);

    session->clientRect_ = session->winRect_;
    EXPECT_EQ(session->NotifyServerToUpdateRect(uiParam, SizeChangeReason::UNDEFINED), false); // skip same rect
}

/**
 * @tc.name: MoveUnderInteriaAndNotifyRectChange
 * @tc.desc: test func: MoveUnderInteriaAndNotifyRectChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, MoveUnderInteriaAndNotifyRectChange, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "MoveUnderInteriaAndNotifyRectChange";
    info.bundleName_ = "MoveUnderInteriaAndNotifyRectChange";
    info.screenId_ = 0;
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(mainSession->pcFoldScreenController_, nullptr);
    auto controller = mainSession->pcFoldScreenController_;
    WSRect rect = { 0, 0, 100, 100 };
    EXPECT_FALSE(mainSession->MoveUnderInteriaAndNotifyRectChange(rect, SizeChangeReason::DRAG_END));
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(0, SuperFoldStatus::HALF_FOLDED,
        { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    PcFoldScreenManager::GetInstance().vpr_ = 1.7f;

    WSRect rect0 = { 100, 100, 400, 400 };
    WSRect rect1 = { 100, 500, 400, 400 };

    // throw
    controller->RecordStartMoveRect(rect0, false);
    controller->RecordMoveRects(rect0);
    usleep(10000);
    rect = rect1;
    controller->RecordMoveRects(rect);
    EXPECT_TRUE(mainSession->MoveUnderInteriaAndNotifyRectChange(rect, SizeChangeReason::DRAG_END));

    // throw full screen
    usleep(100000);
    controller->RecordStartMoveRect(rect0, true);
    controller->RecordMoveRects(rect0);
    usleep(10000);
    rect = rect1;
    controller->RecordMoveRects(rect);
    EXPECT_TRUE(mainSession->MoveUnderInteriaAndNotifyRectChange(rect, SizeChangeReason::DRAG_END));
}

/**
 * @tc.name: ThrowSlipDirectly
 * @tc.desc: ThrowSlipDirectly
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, ThrowSlipDirectly, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ThrowSlipDirectly";
    info.bundleName_ = "ThrowSlipDirectly";
    info.screenId_ = 0;
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    WSRect rect = { 100, 100, 400, 400 };
    mainSession->winRect_ = rect;
    mainSession->ThrowSlipDirectly(WSRectF { 0.0f, 0.0f, 0.0f, 0.0f });
    EXPECT_EQ(mainSession->winRect_, rect);
}

/**
 * @tc.name: SetBehindWindowFilterEnabled
 * @tc.desc: SetBehindWindowFilterEnabled test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetBehindWindowFilterEnabled, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetBehindWindowFilterEnabled";
    info.bundleName_ = "SetBehindWindowFilterEnabled";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    session->SetBehindWindowFilterEnabled(false);
    session->SetBehindWindowFilterEnabled(true);
}

/**
 * @tc.name: MarkSystemSceneUIFirst
 * @tc.desc: MarkSystemSceneUIFirst function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, MarkSystemSceneUIFirst, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "MarkSystemSceneUIFirst";
    info.bundleName_ = "MarkSystemSceneUIFirst";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    session->MarkSystemSceneUIFirst(true, true);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->surfaceNode_ = surfaceNode;
    session->leashWinSurfaceNode_ = nullptr;
    session->MarkSystemSceneUIFirst(true, true);
    session->leashWinSurfaceNode_ = surfaceNode;
    session->MarkSystemSceneUIFirst(true, true);
    EXPECT_NE(nullptr, session->GetLeashWinSurfaceNode());
}

/**
 * @tc.name: IsMissionHighlighted
 * @tc.desc: IsMissionHighlighted
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, IsMissionHighlighted, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "IsMissionHighlighted";
    info.bundleName_ = "IsMissionHighlighted";
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    mainSession->isFocused_ = true;
    EXPECT_TRUE(mainSession->IsMissionHighlighted());
    mainSession->isFocused_ = false;

    SessionInfo subInfo;
    subInfo.abilityName_ = "IsMissionHighlightedSub";
    subInfo.bundleName_ = "IsMissionHighlightedSub";
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subInfo, nullptr);
    mainSession->subSession_.push_back(subSession);
    subSession->isFocused_ = true;
    EXPECT_TRUE(mainSession->IsMissionHighlighted());
    subSession->isFocused_ = false;
    EXPECT_FALSE(mainSession->IsMissionHighlighted());
}

/**
 * @tc.name: SetSessionDisplayIdChangeCallback
 * @tc.desc: SetSessionDisplayIdChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSessionDisplayIdChangeCallback, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetSessionDisplayIdChangeCallback([] (uint64_t displayId) {
        return;
    });
    ASSERT_NE(sceneSession->sessionDisplayIdChangeFunc_, nullptr);
}

/**
 * @tc.name: NotifySessionDisplayIdChange
 * @tc.desc: NotifySessionDisplayIdChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, NotifySessionDisplayIdChange, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    uint64_t checkDisplayId = 345;
    uint64_t moveDisplayId = 456;
    sceneSession->sessionDisplayIdChangeFunc_ = [&checkDisplayId] (uint64_t displayId) {
        checkDisplayId = displayId;
    };
    sceneSession->NotifySessionDisplayIdChange(moveDisplayId);
    ASSERT_EQ(moveDisplayId, checkDisplayId);
}

/**
 * @tc.name: CheckAndMoveDisplayIdRecursively
 * @tc.desc: CheckAndMoveDisplayIdRecursively
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, CheckAndMoveDisplayIdRecursively, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSessionMocker> sceneSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    uint64_t displayId = 234;
    sptr<SceneSessionMocker> subSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    sceneSession->subSession_.push_back(subSession);
    EXPECT_CALL(*sceneSession, CheckAndMoveDisplayIdRecursively(displayId))
        .WillRepeatedly([weakThis = wptr(sceneSession)](uint64_t displayId) {
            auto session = weakThis.promote();
            if (session) {
                return weakThis->SceneSession::CheckAndMoveDisplayIdRecursively(displayId);
            } else {
                GTEST_LOG_(INFO) << "SceneSessionMocker:NULL";
                return;
            }
    });
    sceneSession->property_->SetDisplayId(displayId);
    sceneSession->shouldFollowParentWhenShow_ = true;
    EXPECT_CALL(*sceneSession, SetScreenId(displayId)).Times(0);
    sceneSession->CheckAndMoveDisplayIdRecursively(displayId);
    sceneSession->property_->SetDisplayId(123);
    sceneSession->shouldFollowParentWhenShow_ = false;
    EXPECT_CALL(*sceneSession, SetScreenId(displayId)).Times(0);
    sceneSession->CheckAndMoveDisplayIdRecursively(displayId);
    sceneSession->property_->SetDisplayId(123);
    sceneSession->shouldFollowParentWhenShow_ = true;
    EXPECT_CALL(*sceneSession, SetScreenId(displayId)).Times(1);
    EXPECT_CALL(*subSession, CheckAndMoveDisplayIdRecursively(displayId)).Times(1);
    sceneSession->CheckAndMoveDisplayIdRecursively(displayId);
    ASSERT_EQ(sceneSession->property_->GetDisplayId(), displayId);
}

/**
 * @tc.name: SetShouldFollowParentWhenShow
 * @tc.desc: SetShouldFollowParentWhenShow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetShouldFollowParentWhenShow, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetShouldFollowParentWhenShow(false);
    ASSERT_EQ(sceneSession->shouldFollowParentWhenShow_, false);
}

HWTEST_F(SceneSessionTest5, CheckSubSessionShouldFollowParent, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->subSession_.push_back(subSession);
    subSession->state_ = SessionState::STATE_ACTIVE;
    uint64_t displayIdBase = 123;
    uint64_t displayIdDiff = 345;
    subSession->property_->SetDisplayId(displayIdBase);
    sceneSession->CheckSubSessionShouldFollowParent(displayIdBase);
    sceneSession->CheckSubSessionShouldFollowParent(displayIdBase);
    EXPECT_EQ(subSession->shouldFollowParentWhenShow_, true);
    sceneSession->CheckSubSessionShouldFollowParent(displayIdDiff);
    EXPECT_EQ(subSession->shouldFollowParentWhenShow_, false);
}

/**
 * @tc.name: ActivateKeyboardAvoidArea01
 * @tc.desc: test ActivateKeyboardAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, ActivateKeyboardAvoidArea01, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.bundleName_ = "ActivateKeyboardAvoidArea01";
    info.abilityName_ = "ActivateKeyboardAvoidArea01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_EQ(true, sceneSession->IsKeyboardAvoidAreaActive());
    sceneSession->ActivateKeyboardAvoidArea(false, true);
    ASSERT_EQ(false, sceneSession->IsKeyboardAvoidAreaActive());
    sceneSession->ActivateKeyboardAvoidArea(false, false);
    ASSERT_EQ(false, sceneSession->IsKeyboardAvoidAreaActive());
    sceneSession->ActivateKeyboardAvoidArea(true, true);
    ASSERT_EQ(true, sceneSession->IsKeyboardAvoidAreaActive());
    sceneSession->ActivateKeyboardAvoidArea(true, false);
    ASSERT_EQ(true, sceneSession->IsKeyboardAvoidAreaActive());
}

/**
 * @tc.name: IsSameMainSession
 * @tc.desc: test IsSameMainSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, IsSameMainSession, Function | SmallTest | Level2)
{
    SessionInfo info1;
    info1.abilityName_ = "abilityName_test1";
    info1.bundleName_ = "bundleName_test1";
    SessionInfo info2;
    info2.abilityName_ = "abilityName_test2";
    info2.bundleName_ = "bundleName_test2";
    sptr<SceneSession> preSceneSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    sptr<SceneSession> currSceneSession = sptr<SceneSession>::MakeSptr(info2, nullptr);
    preSceneSession->persistentId_ = 1;
    currSceneSession->persistentId_ = 1;
    ASSERT_EQ(true, currSceneSession->IsSameMainSession(preSceneSession));
    currSceneSession->persistentId_ = 2;
    ASSERT_EQ(false, currSceneSession->IsSameMainSession(preSceneSession));

    sptr<SceneSession> subSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    sptr<SceneSession> subSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    subSession1->SetParentSession(preSceneSession);
    subSession2->SetParentSession(currSceneSession);
    currSceneSession->persistentId_ = 1;
    subSession1->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    subSession2->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ASSERT_EQ(true, subSession1->IsSameMainSession(subSession1));
    currSceneSession->persistentId_ = 2;
    ASSERT_EQ(false, subSession1->IsSameMainSession(subSession2));
}

/**
 * @tc.name: HandleActionUpdateExclusivelyHighlighted
 * @tc.desc: test HandleActionUpdateExclusivelyHighlighted
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdateExclusivelyHighlighted, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateExclusivelyHighlighted";
    info.bundleName_ = "HandleActionUpdateExclusivelyHighlighted";
    info.isSystem_ = true;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetExclusivelyHighlighted(true);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_EXCLUSIVE_HIGHLIGHTED;
    auto res = session->HandleActionUpdateExclusivelyHighlighted(property, action);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: SetHighlightChangeNotifyFunc
 * @tc.desc: SetHighlightChangeNotifyFunc Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetHighlightChangeNotifyFunc, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    session->SetHighlightChangeNotifyFunc([](int32_t persistentId) {});
    EXPECT_NE(session->highlightChangeFunc_, nullptr);
}

/**
 * @tc.name: StartMovingWithCoordinate_01
 * @tc.desc: StartMovingWithCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, StartMovingWithCoordinate_01, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->moveDragController_ = nullptr;
    WSError result = sceneSession->StartMovingWithCoordinate(100, 50, 300, 500);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: StartMovingWithCoordinate_02
 * @tc.desc: StartMovingWithCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, StartMovingWithCoordinate_02, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(2024, sceneSession->GetWindowType());
    sceneSession->moveDragController_->isStartMove_ = true;
    WSError result = sceneSession->StartMovingWithCoordinate(100, 50, 300, 500);
    EXPECT_EQ(result, WSError::WS_ERROR_REPEAT_OPERATION);
}

/**
 * @tc.name: StartMovingWithCoordinate_03
 * @tc.desc: StartMovingWithCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, StartMovingWithCoordinate_03, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(2024, sceneSession->GetWindowType());
    sceneSession->moveDragController_->isStartMove_ = false;
    WSError result = sceneSession->StartMovingWithCoordinate(100, 50, 300, 500);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetColorSpace
 * @tc.desc: SetColorSpace function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetColorSpace, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetColorSpace";
    info.bundleName_ = "SetColorSpace";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session->SetColorSpace(ColorSpace::COLOR_SPACE_WIDE_GAMUT);
    EXPECT_EQ(nullptr, session->GetSurfaceNode());
    session->surfaceNode_ = surfaceNode;
    session->SetColorSpace(ColorSpace::COLOR_SPACE_WIDE_GAMUT);
    EXPECT_NE(nullptr, session->GetSurfaceNode());
}

/**
 * @tc.name: UpdateCrossAxisOfLayout
 * @tc.desc: UpdateCrossAxisOfLayout
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateCrossAxisOfLayout, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSessionMocker> sceneSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    WSRect rect;
    EXPECT_CALL(*sceneSession, UpdateCrossAxis()).Times(1);
    sceneSession->SceneSession::UpdateCrossAxisOfLayout(rect);
}

/**
 * @tc.name: UpdateCrossAxis
 * @tc.desc: UpdateCrossAxis
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateCrossAxis, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto sessionStageMocker = sptr<SessionStageMocker>::MakeSptr();
    sceneSession->sessionStage_ = sessionStageMocker;
    // always have ovelap with axis
    sceneSession->isCrossAxisOfLayout_ = true;
    // not fold screen
    PcFoldScreenManager::GetInstance().displayId_ = SCREEN_ID_INVALID;
    EXPECT_CALL(*sessionStageMocker, NotifyWindowCrossAxisChange(_)).Times(0);
    sceneSession->UpdateCrossAxis();
    // fold screen, but fold status unknown
    PcFoldScreenManager::GetInstance().displayId_ = 0;
    sceneSession->GetSessionProperty()->displayId_ = 0;
    PcFoldScreenManager::GetInstance().screenFoldStatus_ = SuperFoldStatus::UNKNOWN;
    sceneSession->crossAxisState_ = 100;
    EXPECT_CALL(*sessionStageMocker, NotifyWindowCrossAxisChange(CrossAxisState::STATE_INVALID)).Times(1);
    sceneSession->UpdateCrossAxis();
    // state: half folded
    sceneSession->crossAxisState_ = 100;
    PcFoldScreenManager::GetInstance().screenFoldStatus_ = SuperFoldStatus::HALF_FOLDED;
    EXPECT_CALL(*sessionStageMocker, NotifyWindowCrossAxisChange(CrossAxisState::STATE_CROSS)).Times(1);
    sceneSession->UpdateCrossAxis();
    // state: other
    sceneSession->crossAxisState_ = 100;
    PcFoldScreenManager::GetInstance().screenFoldStatus_ = SuperFoldStatus::EXPANDED;
    EXPECT_CALL(*sessionStageMocker, NotifyWindowCrossAxisChange(CrossAxisState::STATE_NO_CROSS)).Times(1);
    sceneSession->UpdateCrossAxis();
    // sessionStage is nullptr
    sceneSession->crossAxisState_ = 100;
    sceneSession->sessionStage_ = nullptr;
    EXPECT_CALL(*sessionStageMocker, NotifyWindowCrossAxisChange(_)).Times(0);
    sceneSession->UpdateCrossAxis();
}

/**
 * @tc.name: GetCrossAxisState
 * @tc.desc: GetCrossAxisState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, GetCrossAxisState, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSessionMocker> sceneSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    sceneSession->crossAxisState_ = 1;
    CrossAxisState state = CrossAxisState::STATE_INVALID;
    sceneSession->GetCrossAxisState(state);
    EXPECT_EQ(state, CrossAxisState::STATE_CROSS);
}

/**
 * @tc.name: GetWaterfallMode
 * @tc.desc: GetWaterfallMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, GetWaterfallMode, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "GetWaterfallMode";
    info.bundleName_ = "GetWaterfallMode";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    bool isWaterfallMode = true;
    session->GetWaterfallMode(isWaterfallMode);
    EXPECT_EQ(isWaterfallMode, false);
}
}
} // namespace Rosen
} // namespace OHOS