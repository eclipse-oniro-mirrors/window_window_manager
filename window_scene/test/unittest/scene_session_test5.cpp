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
#include "proxy/include/window_info.h"

#include "common/include/session_permission.h"
#include "display_manager.h"
#include "input_event.h"
#include "key_event.h"

#include "mock/mock_session_stage.h"
#include "mock/mock_scene_session.h"
#include "mock_vsync_station.h"
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
#include "transaction/rs_transaction.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
}
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
private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
};

void SceneSessionTest5::SetUpTestCase() {}

void SceneSessionTest5::TearDownTestCase() {}

void SceneSessionTest5::SetUp() {}

void SceneSessionTest5::TearDown() {}

RSSurfaceNode::SharedPtr SceneSessionTest5::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTestSurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    return surfaceNode;
}

namespace {

/**
 * @tc.name: GetSystemAvoidArea
 * @tc.desc: GetSystemAvoidArea function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, GetSystemAvoidArea, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, HookAvoidAreaInCompatibleMode, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HookAvoidAreaInCompatibleMode";
    info.bundleName_ = "HookAvoidAreaInCompatibleMode";

    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    WSRect rect = { 800, 100, 1000, 1000 };
    AvoidArea avoidArea;
    avoidArea.topRect_ = { -1, -1, -1, -1 };
    avoidArea.bottomRect_ = { -1, -1, -1, -1 };
    Rect invalidRect = { -1, -1, -1, -1 };
    // hook Func only support compatibleMode
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToImmersive(false);
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->HookAvoidAreaInCompatibleMode(rect, AvoidAreaType::TYPE_SYSTEM, avoidArea);
    EXPECT_TRUE(avoidArea.topRect_ == invalidRect);
    compatibleModeProperty->SetIsAdaptToImmersive(true);
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    session->HookAvoidAreaInCompatibleMode(rect, AvoidAreaType::TYPE_SYSTEM, avoidArea);
    EXPECT_TRUE(avoidArea.topRect_ == invalidRect);

    // test top system avoidArea
    session->property_->GetCompatibleModeProperty()->SetIsAdaptToImmersive(true);
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->HookAvoidAreaInCompatibleMode(rect, AvoidAreaType::TYPE_SYSTEM, avoidArea);
    auto vpr = 3.5f;
    Rect targetRect = { 0, 0, rect.width_, 40 * vpr };
    EXPECT_TRUE(avoidArea.topRect_ == targetRect);

    // test buttom aiBar avoidArea
    session->HookAvoidAreaInCompatibleMode(rect, AvoidAreaType::TYPE_NAVIGATION_INDICATOR, avoidArea);
    targetRect = { 0, rect.height_ - 28 * vpr, rect.width_, 28 * vpr };
    EXPECT_TRUE(avoidArea.bottomRect_ == targetRect);
}

/**
 * @tc.name: GetSystemAvoidArea01
 * @tc.desc: GetSystemAvoidArea01 function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, GetSystemAvoidArea01, TestSize.Level1)
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
    rect.width_ = 4;
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
HWTEST_F(SceneSessionTest5, NotifyOutsideDownEvent, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyOutsideDownEvent";
    info.bundleName_ = "NotifyOutsideDownEvent";
    info.windowInputType_ = static_cast<uint32_t>(MMI::WindowInputType::NORMAL);

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
    OnOutsideDownEvent func = [](int32_t x, int32_t y) { return; };
    session->specificCallback_->onOutsideDownEvent_ = func;
    session->NotifyOutsideDownEvent(pointerEvent);
    auto res = pointerEvent->GetPointerItem(2024, pointerItem);
    EXPECT_EQ(true, res);

    pointerEvent->SetPointerAction(5);
    session->NotifyOutsideDownEvent(pointerEvent);
    pointerEvent->RemoveAllPointerItems();

    info.windowInputType_ = static_cast<uint32_t>(MMI::WindowInputType::TRANSMIT_ALL);
    sptr<SceneSession> session1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    session1->NotifyOutsideDownEvent(pointerEvent);
}

/**
 * @tc.name: TransferPointerEventInnerTest
 * @tc.desc: TransferPointerEventInner function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, TransferPointerEventInner, TestSize.Level1)
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

    NotifySessionTouchOutsideCallback func = [](int32_t persistentId, DisplayId displayId) { return; };
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
HWTEST_F(SceneSessionTest5, TransferPointerEventInnerTest001, TestSize.Level1)
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
    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));
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
HWTEST_F(SceneSessionTest5, SetSurfaceBounds01, TestSize.Level1)
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
    session->SetGetRsCmdBlockingCountFunc([] {
        return 1;
    });
    session->SetSessionRect(preRect);
    session->SetSurfaceBounds(rect, false);
    session->SetGetRsCmdBlockingCountFunc([] {
        return 0;
    });
    session->surfaceNode_ = surfaceNode;
    session->SetSurfaceBounds(rect, false);
    EXPECT_EQ(preRect, session->GetSessionRect());
}

/**
 * @tc.name: OnLayoutFullScreenChange
 * @tc.desc: OnLayoutFullScreenChange function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, OnLayoutFullScreenChange, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, RegisterLayoutFullScreenChangeCallback, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, OnDefaultDensityEnabled, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, UpdateSessionPropertyByAction, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateSessionPropertyByAction";
    info.bundleName_ = "UpdateSessionPropertyByAction";
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR,
              session->UpdateSessionPropertyByAction(nullptr, WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE));
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION,
              session->UpdateSessionPropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE));
}

/**
 * @tc.name: SetSessionRectChangeCallback
 * @tc.desc: SetSessionRectChangeCallback function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSessionRectChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSessionRectChangeCallback";
    info.bundleName_ = "SetSessionRectChangeCallback";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    WSRect rec = { 1, 1, 1, 1 };
    NotifySessionRectChangeFunc func = [](const WSRect& rect,
                                          SizeChangeReason reason,
                                          DisplayId displayId,
                                          const RectAnimationConfig& rectAnimationConfig) { return; };
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
HWTEST_F(SceneSessionTest5, SetSessionRectChangeCallback02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSessionRectChangeCallback02";
    info.bundleName_ = "SetSessionRectChangeCallback02";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    WSRect rec = { 1, 1, 1, 1 };
    NotifySessionRectChangeFunc func = [](const WSRect& rect,
                                          SizeChangeReason reason,
                                          DisplayId displayId,
                                          const RectAnimationConfig& rectAnimationConfig) { return; };
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
HWTEST_F(SceneSessionTest5, SetSessionRectChangeCallback03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSessionRectChangeCallback03";
    info.bundleName_ = "SetSessionRectChangeCallback03";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->SetSessionProperty(property);
    WSRect rec = { 1, 1, 1, 1 };
    NotifySessionRectChangeFunc func = [](const WSRect& rect,
                                          const SizeChangeReason reason,
                                          DisplayId displayId,
                                          const RectAnimationConfig& rectAnimationConfig) { return; };
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
HWTEST_F(SceneSessionTest5, GetSystemAvoidArea02, TestSize.Level1)
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
    rect.width_ = 1;
    session->GetSystemAvoidArea(rect, avoidArea);
    rect.height_ = 1;
    session->GetSystemAvoidArea(rect, avoidArea);
}

/**
 * @tc.name: OnMoveDragCallback
 * @tc.desc: OnMoveDragCallback function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, OnMoveDragCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnMoveDragCallback";
    info.bundleName_ = "OnMoveDragCallback";
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    session->SetVsyncStation(mockVsyncStation);
    EXPECT_NE(nullptr, session->vsyncStation_);
    session->moveDragController_ = nullptr;
    SizeChangeReason reason = { SizeChangeReason::DRAG };
    session->OnMoveDragCallback(reason);
    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));
    session->OnMoveDragCallback(reason);

    reason = SizeChangeReason::DRAG_END;
    session->OnMoveDragCallback(reason);

    reason = SizeChangeReason::DRAG_MOVE;
    session->OnMoveDragCallback(reason);

    reason = SizeChangeReason::DRAG_START;
    session->OnMoveDragCallback(reason);
    EXPECT_EQ(WSError::WS_OK, session->UpdateSizeChangeReason(reason));

    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));
    EXPECT_NE(session->moveDragController_, nullptr);
}

/**
 * @tc.name: OnMoveDragCallback02
 * @tc.desc: OnMoveDragCallback for DragResizeWhenEndFilter
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, OnMoveDragCallback02, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "DragResizeWhenEnd";
    info.bundleName_ = "DragResizeWhenEnd";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, session);
    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));
    EXPECT_NE(nullptr, session->moveDragController_);
    SizeChangeReason reason = { SizeChangeReason::DRAG };
    session->OnMoveDragCallback(reason);
    session->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    session->dragResizeTypeDuringDrag_ = DragResizeType::RESIZE_WHEN_DRAG_END;
    session->moveDragController_->isStartDrag_ = true;
    EXPECT_EQ(session->DragResizeWhenEndFilter(reason), true);
    session->OnMoveDragCallback(reason);
}

/**
 * @tc.name: OnMoveDragCallback03
 * @tc.desc: OnMoveDragCallback for DragResizeInCompatibleMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, OnMoveDragCallback03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DragResizeInCompatibleMode";
    info.bundleName_ = "DragResizeInCompatibleMode";
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, session);
    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));
    EXPECT_NE(nullptr, session->moveDragController_);
    SizeChangeReason reason = { SizeChangeReason::DRAG };
    WSRect windowRect = { 1, 10, 3, 4 };
    WSRect systemBarRect = { 1, 2, 10, 11 };
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    EXPECT_NE(compatibleModeProperty, nullptr);
    session->isVisible_ = true;
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCallback->onGetSceneSessionVectorByTypeAndDisplayId_ = nullptr;
    session->specificCallback_ = specificCallback;
    GetSceneSessionVectorByTypeAndDisplayIdCallback func =
        [session](WindowType type, uint64_t displayId)->std::vector<sptr<SceneSession>>
    {
        std::vector<sptr<SceneSession>> vSession;
        vSession.push_back(session);
        return vSession;
    };
    specificCallback->onGetSceneSessionVectorByTypeAndDisplayId_ = func;
    compatibleModeProperty->SetIsAdaptToDragScale(true);
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);
    session->moveDragController_->UpdateTargetRect(reason, windowRect);
    session->GetLayoutController()->SetSessionRect(systemBarRect);
    session->OnMoveDragCallback(reason);
    WSRect newRect = session->moveDragController_->GetTargetRect(
        MoveDragController::TargetRectCoordinate::RELATED_TO_START_DISPLAY);
    ASSERT_EQ(4, newRect.height_);
    ASSERT_EQ(10, newRect.posY_);

    compatibleModeProperty->SetIsAdaptToDragScale(false);
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);
    session->moveDragController_->UpdateTargetRect(reason, windowRect);
    session->GetLayoutController()->SetSessionRect(systemBarRect);
    session->OnMoveDragCallback(reason);
    newRect = session->moveDragController_->GetTargetRect(
        MoveDragController::TargetRectCoordinate::RELATED_TO_START_DISPLAY);
    ASSERT_NE(4, newRect.height_);
    ASSERT_NE(10, newRect.posY_);
}

/**
 * @tc.name: DragResizeWhenEndFilter
 * @tc.desc: DragResizeWhenEndFilter function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, DragResizeWhenEndFilter, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "DragResizeWhenEnd";
    info.bundleName_ = "DragResizeWhenEnd";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, session);
    auto oriProperty = session->GetSessionProperty();
    EXPECT_NE(nullptr, oriProperty);
    auto moveDragController = sptr<MoveDragController>::MakeSptr(wptr(session));
    EXPECT_NE(nullptr, moveDragController);
    SizeChangeReason reason = { SizeChangeReason::DRAG };
    // null
    session->moveDragController_ = nullptr;
    EXPECT_EQ(session->DragResizeWhenEndFilter(reason), true);
    session->moveDragController_ = moveDragController;
    session->property_ = nullptr;
    EXPECT_EQ(session->DragResizeWhenEndFilter(reason), true);
    session->property_ = oriProperty;
    // invalid
    EXPECT_EQ(session->DragResizeWhenEndFilter(SizeChangeReason::DRAG_START), false);
    session->moveDragController_->isStartDrag_ = false;
    EXPECT_EQ(session->DragResizeWhenEndFilter(reason), false);
    session->moveDragController_->isStartDrag_ = true;
    EXPECT_EQ(session->DragResizeWhenEndFilter(reason), false);
    session->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    session->systemConfig_.freeMultiWindowSupport_ = true;
    session->systemConfig_.freeMultiWindowEnable_ = false;
    session->dragResizeTypeDuringDrag_ = DragResizeType::RESIZE_WHEN_DRAG_END;
    EXPECT_EQ(session->DragResizeWhenEndFilter(reason), false);
    session->systemConfig_.freeMultiWindowEnable_ = true;
    EXPECT_EQ(session->DragResizeWhenEndFilter(reason), true);
    session->systemConfig_.freeMultiWindowEnable_ = false;
    session->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(session->DragResizeWhenEndFilter(reason), true);
    EXPECT_EQ(session->DragResizeWhenEndFilter(SizeChangeReason::DRAG_END), true);
    session->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(session->DragResizeWhenEndFilter(reason), false);
}

/**
 * @tc.name: HandleSessionDragEvent
 * @tc.desc: HandleSessionDragEvent function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleSessionDragEvent, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleSessionDragEvent";
    info.bundleName_ = "HandleSessionDragEvent";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
 
    session->appDragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    session->moveDragController_ = nullptr;
    session->HandleSessionDragEvent(SessionEvent::EVENT_END_MOVE);
    EXPECT_EQ(session->GetDragResizeTypeDuringDrag(), DragResizeType::RESIZE_TYPE_UNDEFINED);
 
    auto oriProperty = session->GetSessionProperty();
    session->property_ = oriProperty;
    auto moveDragController = sptr<MoveDragController>::MakeSptr(wptr(session));
    session->moveDragController_ = moveDragController;
    SessionEvent event = { SessionEvent::EVENT_DRAG };
 
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    ASSERT_NE(compatibleModeProperty, nullptr);
 
    session->moveDragController_->isStartDrag_ = false;
    compatibleModeProperty->SetIsAdaptToDragScale(false);
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);
    session->HandleSessionDragEvent(event);
 
    compatibleModeProperty->SetIsAdaptToDragScale(true);
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);
    session->HandleSessionDragEvent(event);
 
    event = { SessionEvent::EVENT_DRAG_START };
    session->moveDragController_->isStartDrag_ = true;
 
    compatibleModeProperty->SetIsAdaptToDragScale(false);
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);
    session->HandleSessionDragEvent(event);
    EXPECT_EQ(session->GetDragResizeTypeDuringDrag(), session->GetAppDragResizeType());
 
    compatibleModeProperty->SetIsAdaptToDragScale(true);
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);
    session->HandleSessionDragEvent(event);

    session->HandleSessionDragEvent(SessionEvent::EVENT_END_MOVE);
    EXPECT_EQ(session->GetDragResizeTypeDuringDrag(), DragResizeType::RESIZE_TYPE_UNDEFINED);

    session->HandleSessionDragEvent(SessionEvent::EVENT_MAXIMIZE);
    EXPECT_EQ(session->GetDragResizeTypeDuringDrag(), DragResizeType::RESIZE_TYPE_UNDEFINED);
}

/**
 * @tc.name: IsCompatibleModeDirtyDragScaleWindow
 * @tc.desc: IsCompatibleModeDirtyDragScaleWindow function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, IsCompatibleModeDirtyDragScaleWindow, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "IsCompatibleModeDirtyDragScaleWindow";
    info.bundleName_ = "IsCompatibleModeDirtyDragScaleWindow";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    session->compatibleDragScaleFlags_ = false;
    EXPECT_EQ(session->IsCompatibleModeDirtyDragScaleWindow(), false);
    session->compatibleDragScaleFlags_ = true;
    EXPECT_EQ(session->IsCompatibleModeDirtyDragScaleWindow(), true);
}
 
/**
 * @tc.name: ResetCompatibleModeDragScaleFlags
 * @tc.desc: ResetCompatibleModeDragScaleFlags function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, ResetCompatibleModeDragScaleFlags, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ResetCompatibleModeDragScaleFlags";
    info.bundleName_ = "ResetCompatibleModeDragScaleFlags";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    session->ResetCompatibleModeDragScaleFlags();
    EXPECT_EQ(session->IsCompatibleModeDirtyDragScaleWindow(), false);
}

/**
 * @tc.name: UpdateKeyFrameCloneNode
 * @tc.desc: UpdateKeyFrameCloneNode function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateKeyFrameCloneNode, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "keyframe";
    info.bundleName_ = "keyframe";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    auto sessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, sessionStage);
    auto rsKeyFrameNode = RSWindowKeyFrameNode::Create();
    EXPECT_NE(nullptr, rsKeyFrameNode);
    std::shared_ptr<RSWindowKeyFrameNode> rsKeyFrameNodeNull = nullptr;
    auto rsTransaction = std::make_shared<RSTransaction>();
    EXPECT_NE(nullptr, rsTransaction);
    std::shared_ptr<RSTransaction> rsTransactionNull = nullptr;

    session->keyFrameCloneNode_ = rsKeyFrameNode;
    EXPECT_EQ(session->UpdateKeyFrameCloneNode(rsKeyFrameNode, rsTransaction), WSError::WS_OK);
    session->keyFrameCloneNode_ = nullptr;
    EXPECT_EQ(session->UpdateKeyFrameCloneNode(rsKeyFrameNodeNull, rsTransaction), WSError::WS_ERROR_NULLPTR);
    EXPECT_EQ(session->UpdateKeyFrameCloneNode(rsKeyFrameNode, rsTransaction), WSError::WS_ERROR_NULLPTR);
    session->sessionStage_ = sessionStage;
    EXPECT_EQ(session->UpdateKeyFrameCloneNode(rsKeyFrameNode, rsTransactionNull), WSError::WS_OK);
    EXPECT_EQ(session->UpdateKeyFrameCloneNode(rsKeyFrameNode, rsTransaction), WSError::WS_OK);
}

/**
 * @tc.name: UpdateKeyFrameState
 * @tc.desc: UpdateKeyFrameState function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateKeyFrameState, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "keyframe";
    info.bundleName_ = "keyframe";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    auto moveDragController = sptr<MoveDragController>::MakeSptr(wptr(session));
    EXPECT_NE(nullptr, moveDragController);
    auto sessionStage = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, sessionStage);
    auto rsKeyFrameNode = RSWindowKeyFrameNode::Create();
    EXPECT_NE(nullptr, rsKeyFrameNode);
    auto rsTransaction = std::make_shared<RSTransaction>();
    EXPECT_NE(nullptr, rsTransaction);

    session->moveDragController_ = nullptr;
    session->sessionStage_ = nullptr;
    SizeChangeReason reason = { SizeChangeReason::DRAG_START };
    WSRect rect;
    KeyFramePolicy keyFramePolicy;
    keyFramePolicy.dragResizeType_ = DragResizeType::RESIZE_KEY_FRAME;
    session->UpdateKeyFrameState(reason, rect);
    EXPECT_EQ(session->keyFramePolicy_.running_, false);
    session->moveDragController_ = moveDragController;
    session->UpdateKeyFrameState(reason, rect);
    EXPECT_EQ(session->keyFramePolicy_.running_, false);
    session->sessionStage_ = sessionStage;
    session->UpdateKeyFrameState(reason, rect);
    EXPECT_EQ(session->keyFramePolicy_.running_, false);
    session->moveDragController_->isStartDrag_ = true;
    session->UpdateKeyFrameState(reason, rect);
    session->SetDragKeyFramePolicy(keyFramePolicy);
    session->UpdateKeyFrameState(reason, rect);
    EXPECT_EQ(session->keyFramePolicy_.running_, true);
    session->SetAppDragResizeType(DragResizeType::RESIZE_WHEN_DRAG_END);
    session->UpdateKeyFrameState(reason, rect);
    EXPECT_EQ(session->keyFramePolicy_.running_, false);
    session->SetAppDragResizeType(DragResizeType::RESIZE_TYPE_UNDEFINED);
    session->UpdateKeyFrameState(reason, rect);
    EXPECT_EQ(session->keyFramePolicy_.running_, true);
    session->keyFrameCloneNode_ = rsKeyFrameNode;
    reason = SizeChangeReason::DRAG;
    session->UpdateKeyFrameState(reason, rect);
    EXPECT_EQ(session->lastKeyFrameDragRect_, rect);
    reason = SizeChangeReason::DRAG_END;
    session->UpdateKeyFrameState(reason, rect);
    EXPECT_EQ(session->keyFramePolicy_.running_, false);
}

/**
 * @tc.name: RequestKeyFrameNextVsync
 * @tc.desc: RequestKeyFrameNextVsync function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, RequestKeyFrameNextVsync, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "keyframe";
    info.bundleName_ = "keyframe";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    session->handler_ = nullptr;
    uint64_t requestStamp = 0;
    uint64_t count = 0;
    uint32_t triggerCount = 0;

    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    EXPECT_CALL(*mockVsyncStation, RequestVsync(_))
        .WillRepeatedly(Invoke([&](const std::shared_ptr<VsyncCallback>& cb) {
            ASSERT_NE(cb, nullptr);
            // Trigger the vsync callback only once to avoid recursive re-entry in tests.
            if (++triggerCount <= 1) {
                cb->onCallback(0, 0);
            }
        }));
    session->SetVsyncStation(mockVsyncStation);

    session->RequestKeyFrameNextVsync(requestStamp, count);
    session->keyFramePolicy_.running_ = true;
    session->RequestKeyFrameNextVsync(requestStamp, count);
    session->keyFrameVsyncRequestStamp_ = requestStamp;
    session->RequestKeyFrameNextVsync(requestStamp, count);
}

/**
 * @tc.name: OnKeyFrameNextVsync
 * @tc.desc: OnKeyFrameNextVsync function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, OnKeyFrameNextVsync, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "keyframe";
    info.bundleName_ = "keyframe";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    uint64_t count = 0;
    session->lastKeyFrameDragStamp_ = 0;
    session->keyFrameDragPauseNoticed_ = true;
    session->keyFrameAnimating_ = true;
    session->OnKeyFrameNextVsync(count);
    EXPECT_EQ(session->keyFrameDragPauseNoticed_, true);
    session->keyFrameDragPauseNoticed_ = false;
    session->OnKeyFrameNextVsync(count);
    EXPECT_EQ(session->keyFrameDragPauseNoticed_, false);
    session->keyFrameAnimating_ = false;
    session->OnKeyFrameNextVsync(count);
    EXPECT_EQ(session->keyFrameDragPauseNoticed_, true);
    session->lastKeyFrameDragStamp_ = 0;
    session->keyFrameDragPauseNoticed_ = false;
    session->keyFrameAnimating_ = false;
    session->OnKeyFrameNextVsync(count);
    EXPECT_EQ(session->keyFrameDragPauseNoticed_, true);
    session->keyFrameDragPauseNoticed_ = false;
    session->keyFrameAnimating_ = false;
    uint64_t nowTimeStamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    session->lastKeyFrameDragStamp_ = nowTimeStamp;
    session->OnKeyFrameNextVsync(count);
    EXPECT_EQ(session->keyFrameDragPauseNoticed_, false);
}

/**
 * @tc.name: KeyFrameNotifyFilter
 * @tc.desc: KeyFrameNotifyFilter function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, KeyFrameNotifyFilter, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "keyframe";
    info.bundleName_ = "keyframe";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    SizeChangeReason reason = { SizeChangeReason::DRAG };
    WSRect rect = { 0, 0, 10, 10 };
    WSRect rectNew = { 100, 100, 100, 100 };
    session->lastKeyFrameRect_ = rect;
    session->lastKeyFrameStamp_ = 0;
    // no running
    EXPECT_EQ(session->KeyFrameNotifyFilter(rectNew, reason), false);
    session->keyFramePolicy_.running_ = true;
    // other reason
    EXPECT_EQ(session->KeyFrameNotifyFilter(rectNew, SizeChangeReason::DRAG_START), true);
    EXPECT_EQ(session->KeyFrameNotifyFilter(rectNew, SizeChangeReason::DRAG_END), false);
    session->keyFrameAnimating_ = true;
    EXPECT_EQ(session->KeyFrameNotifyFilter(rectNew, reason), true);
    session->keyFrameAnimating_ = false;
    session->keyFrameCloneNode_ = nullptr;
    EXPECT_EQ(session->KeyFrameNotifyFilter(rectNew, reason), true);
    session->keyFrameCloneNode_ = RSWindowKeyFrameNode::Create();
    // for same rect
    session->keyFrameAnimating_ = false;
    session->lastKeyFrameRect_ = rect;
    session->lastKeyFrameStamp_ = 0;
    EXPECT_EQ(session->KeyFrameNotifyFilter(rect, reason), true);
    session->keyFrameAnimating_ = false;
    session->lastKeyFrameRect_ = rect;
    session->lastKeyFrameStamp_ = 0;
    EXPECT_EQ(session->KeyFrameNotifyFilter(rectNew, reason), false);
    // not meet time condition
    session->keyFrameAnimating_ = false;
    session->lastKeyFrameRect_ = rect;
    EXPECT_EQ(session->KeyFrameNotifyFilter(rectNew, reason), true);
    // for distance condition
    WSRect moveToRect = {
        0, 0, static_cast<int>(session->keyFramePolicy_.distance_), static_cast<int>(session->keyFramePolicy_.distance_)
    };
    session->keyFrameAnimating_ = false;
    EXPECT_EQ(session->KeyFrameNotifyFilter(moveToRect, reason), false);
    session->keyFramePolicy_.distance_ = 0;
    session->lastKeyFrameRect_ = rect;
    session->keyFrameAnimating_ = false;
    EXPECT_EQ(session->KeyFrameNotifyFilter(moveToRect, reason), true);
}

/**
 * @tc.name: KeyFrameRectAlmostSame
 * @tc.desc: KeyFrameRectAlmostSame function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, KeyFrameRectAlmostSame, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "keyframe";
    info.bundleName_ = "keyframe";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    WSRect rect = { 10, 10, 10, 10 };
    const int32_t DIFF_TEST = 13;
    EXPECT_EQ(session->KeyFrameRectAlmostSame(rect, { DIFF_TEST + 1, 10, 10, 10 }), false);
    EXPECT_EQ(session->KeyFrameRectAlmostSame(rect, { 10, DIFF_TEST + 1, 10, 10 }), false);
    EXPECT_EQ(session->KeyFrameRectAlmostSame(rect, { 10, 10, DIFF_TEST + 1, 10 }), false);
    EXPECT_EQ(session->KeyFrameRectAlmostSame(rect, { 10, 10, 10, DIFF_TEST + 1 }), false);
    EXPECT_EQ(session->KeyFrameRectAlmostSame(rect, { DIFF_TEST, DIFF_TEST, DIFF_TEST, DIFF_TEST }), true);
}

/**
 * @tc.name: KeyFrameAnimateEnd
 * @tc.desc: KeyFrameAnimateEnd function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, KeyFrameAnimateEnd, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "keyframe";
    info.bundleName_ = "keyframe";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    EXPECT_EQ(session->KeyFrameAnimateEnd(), WSError::WS_OK);
}

/**
 * @tc.name: No
 * @tc.desc: SetDragKeyFramePolicy function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetDragKeyFramePolicy, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "keyframe";
    info.bundleName_ = "keyframe";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    auto moveDragController = sptr<MoveDragController>::MakeSptr(wptr(session));
    KeyFramePolicy keyFramePolicy;
    session->keyFramePolicy_.stopping_ = true;
    // running
    session->keyFramePolicy_.running_ = true;
    session->keyFramePolicy_.dragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    session->moveDragController_ = nullptr;
    session->SetAppDragResizeType(DragResizeType::RESIZE_WHEN_DRAG_END);
    session->SetDragKeyFramePolicy(keyFramePolicy);
    EXPECT_EQ(session->GetKeyFramePolicy().stopping_, true);
    EXPECT_EQ(session->GetKeyFramePolicy().dragResizeType_, keyFramePolicy.dragResizeType_);
    EXPECT_EQ(session->GetKeyFramePolicy().interval_, keyFramePolicy.interval_);
    EXPECT_EQ(session->GetKeyFramePolicy().distance_, keyFramePolicy.distance_);
    EXPECT_EQ(session->GetKeyFramePolicy().animationDuration_, keyFramePolicy.animationDuration_);
    EXPECT_EQ(session->GetKeyFramePolicy().animationDelay_, keyFramePolicy.animationDelay_);
    // enable during resize
    session->keyFramePolicy_.running_ = false;
    session->SetDragKeyFramePolicy(keyFramePolicy);
    EXPECT_EQ(session->GetKeyFramePolicy().stopping_, true);
    keyFramePolicy.dragResizeType_ = DragResizeType::RESIZE_KEY_FRAME;
    session->SetDragKeyFramePolicy(keyFramePolicy);
    EXPECT_EQ(session->GetKeyFramePolicy().stopping_, true);
    session->moveDragController_ = moveDragController;
    session->moveDragController_->isStartDrag_ = false;
    session->SetDragKeyFramePolicy(keyFramePolicy);
    EXPECT_EQ(session->GetKeyFramePolicy().stopping_, true);
    session->moveDragController_->isStartDrag_ = true;
    session->SetDragKeyFramePolicy(keyFramePolicy);
    EXPECT_EQ(session->GetKeyFramePolicy().stopping_, true);
    session->SetAppDragResizeType(DragResizeType::RESIZE_TYPE_UNDEFINED);
    session->SetDragKeyFramePolicy(keyFramePolicy);
    EXPECT_EQ(session->GetKeyFramePolicy().stopping_, false);
}

/**
 * @tc.name: NotifyServerToUpdateRect_KeyFrame
 * @tc.desc: NotifyServerToUpdateRect_KeyFrame test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, NotifyServerToUpdateRect_KeyFrame, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "keyframe";
    info.bundleName_ = "keyframe";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->foregroundInteractiveStatus_.store(true);
    SessionUIParam uiParam;
    session->keyFramePolicy_.running_ = true;
    EXPECT_EQ(session->NotifyServerToUpdateRect(uiParam, SizeChangeReason::UNDEFINED), false);
}

/**
 * @tc.name: UpdateWinRectForSystemBar
 * @tc.desc: UpdateWinRectForSystemBar function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateWinRectForSystemBar, TestSize.Level1)
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
    GetSceneSessionVectorByTypeAndDisplayIdCallback func =
        [session](WindowType type, uint64_t displayId) -> std::vector<sptr<SceneSession>> {
        std::vector<sptr<SceneSession>> vSession;
        vSession.push_back(session);
        return vSession;
    };
    specificCallback->onGetSceneSessionVectorByTypeAndDisplayId_ = func;
    session->UpdateWinRectForSystemBar(rect);

    session->isVisible_ = true;
    session->GetLayoutController()->SetSessionRect(rect);
    session->UpdateWinRectForSystemBar(rect);

    WSRect rect1 = { 1, 2, 10, 4 };
    session->GetLayoutController()->SetSessionRect(rect1);
    session->UpdateWinRectForSystemBar(rect);
    EXPECT_EQ(4, session->GetSessionRect().height_);

    WSRect rect2 = { 1, 2, 10, 8 };
    session->GetLayoutController()->SetSessionRect(rect2);
    session->UpdateWinRectForSystemBar(rect);
}

/**
 * @tc.name: UpdateNativeVisibility
 * @tc.desc: UpdateNativeVisibility function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateNativeVisibility, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateNativeVisibility";
    info.bundleName_ = "UpdateNativeVisibility";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    NotifyWindowInfoUpdateCallback dateFunc = [](int32_t persistentId, WindowUpdateType type) { return; };
    UpdateAvoidAreaCallback areaFunc = [](const int32_t persistentId) { return; };
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
HWTEST_F(SceneSessionTest5, SetPrivacyMode, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, SetSnapshotSkip, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, SetWatermarkEnabled, TestSize.Level1)
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
    session->SetSurfaceNode(surfaceNode);
    session->SetWatermarkEnabled(watermarkName, true);
}

/**
 * @tc.name: UIExtSurfaceNodeIdCache
 * @tc.desc: UIExtSurfaceNodeIdCache
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UIExtSurfaceNodeIdCache, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, SetSystemSceneOcclusionAlpha, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, ResetOcclusionAlpha, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, SetSystemSceneForceUIFirst, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, UpdateWindowAnimationFlag, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateWindowAnimationFlag";
    info.bundleName_ = "UpdateWindowAnimationFlag";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    EXPECT_EQ(WSError::WS_OK, session->UpdateWindowAnimationFlag(true));

    session->onWindowAnimationFlagChange_ = nullptr;
    EXPECT_EQ(WSError::WS_OK, session->UpdateWindowAnimationFlag(true));

    NotifyWindowAnimationFlagChangeFunc func = [](const bool flag) { return; };
    session->onWindowAnimationFlagChange_ = func;
    EXPECT_EQ(WSError::WS_OK, session->UpdateWindowAnimationFlag(true));
}

/**
 * @tc.name: SetForegroundInteractiveStatus
 * @tc.desc: SetForegroundInteractiveStatus function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetForegroundInteractiveStatus, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, HandleUpdatePropertyByAction, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, SetSystemWindowEnableDrag, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, SetWindowEnableDragBySystem, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowEnableDrag";
    info.bundleName_ = "SetWindowEnableDrag";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto ret = session->SetWindowEnableDragBySystem(true);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: HandleActionUpdateSetBrightness01
 * @tc.desc: HandleActionUpdateSetBrightness function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdateSetBrightness01, TestSize.Level1)
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
 * @tc.name: HandleActionUpdateSetBrightness02
 * @tc.desc: WINDOW_TYPE_WALLET_SWIPE_CARD
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdateSetBrightness02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateSetBrightness";
    info.bundleName_ = "HandleActionUpdateSetBrightness";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD);
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    session->SetSessionState(SessionState::STATE_CONNECT);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS;
    auto res = session->HandleActionUpdateSetBrightness(property, action);
    EXPECT_EQ(WMError::WM_OK, res);
}

/**
 * @tc.name: HandleActionUpdateMaximizeState
 * @tc.desc: HandleActionUpdateMaximizeState function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdateMaximizeState, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, SetUniqueDensityDpi, TestSize.Level1)
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
 * @tc.name: UpdateAnimationSpeed
 * @tc.desc: UpdateAnimationSpeed function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateAnimationSpeed, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateAnimationSpeed";
    info.bundleName_ = "UpdateAnimationSpeed";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    session->sessionStage_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_SESSION, session->UpdateAnimationSpeed(2.0f));
    session->sessionInfo_.isSystem_ = false;
    session->state_ = SessionState::STATE_DISCONNECT;
    EXPECT_EQ(WMError::WM_ERROR_INVALID_SESSION, session->UpdateAnimationSpeed(2.0f));
    session->state_ = SessionState::STATE_CONNECT;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, session->UpdateAnimationSpeed(2.0f));

    session->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_NE(nullptr, session->sessionStage_);
    EXPECT_EQ(WMError::WM_OK, session->UpdateAnimationSpeed(2.0f));
}

/**
 * @tc.name: HandleActionUpdateWindowModeSupportType
 * @tc.desc: HandleActionUpdateWindowModeSupportType function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdateWindowModeSupportType, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateWindowModeSupportType";
    info.bundleName_ = "HandleActionUpdateWindowModeSupportType";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->isSystemCalling_ = false;
    session->SetSessionProperty(property);
    ASSERT_EQ(WMError::WM_ERROR_NOT_SYSTEM_APP,
              session->HandleActionUpdateWindowModeSupportType(property, WSPropertyChangeAction::ACTION_UPDATE_RECT));
}

/**
 * @tc.name: UpdateUIParam
 * @tc.desc: UpdateUIParam function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateUIParam, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, UpdateVisibilityInner, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, UpdateInteractiveInner, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, IsAnco, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, ProcessUpdatePropertyByAction, TestSize.Level1)
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
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_OTHER_PROPS);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_STATUS_PROPS);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_FLAGS);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_MODE);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED);
    session->ProcessUpdatePropertyByAction(property,
                                           WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_TEXTFIELD_AVOID_INFO);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_TOPMOST);
    session->ProcessUpdatePropertyByAction(property, WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO);
}

/**
 * @tc.name: HandleActionUpdateTurnScreenOn
 * @tc.desc: HandleActionUpdateTurnScreenOn function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdateTurnScreenOn, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateTurnScreenOn";
    info.bundleName_ = "HandleActionUpdateTurnScreenOn";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);

    auto res = session->HandleActionUpdateTurnScreenOn(property, WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON);
    EXPECT_EQ(res, WMError::WM_OK);

    property->SetTurnScreenOn(true);
    res = session->HandleActionUpdateTurnScreenOn(property, WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON);
    EXPECT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: HandleActionUpdatePrivacyMode1
 * @tc.desc: HandleActionUpdatePrivacyMode1
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdatePrivacyMode1, TestSize.Level0)
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
    session->SetSurfaceNode(surfaceNode);
    session->property_->SetPrivacyMode(false);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPrivacyMode(true);
    auto res = session->HandleActionUpdatePrivacyMode(property, WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE);
    EXPECT_EQ(WMError::WM_OK, res);
    EXPECT_EQ(true, session->property_->GetPrivacyMode());
}

/**
 * @tc.name: HandleActionUpdatePrivacyMode2
 * @tc.desc: HandleActionUpdatePrivacyMode2
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleActionUpdatePrivacyMode2, TestSize.Level1)
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
    session->SetSurfaceNode(surfaceNode);
    session->property_->SetPrivacyMode(true);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetPrivacyMode(false);
    auto res = session->HandleActionUpdatePrivacyMode(property, WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE);
    EXPECT_EQ(WMError::WM_OK, res);
    EXPECT_EQ(false, session->property_->GetPrivacyMode());
}

/**
 * @tc.name: UpdateClientRect01
 * @tc.desc: UpdateClientRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateClientRect01, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, ResetSizeChangeReasonIfDirty, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, HandleMoveDragSurfaceNode, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleMoveDragSurfaceNode";
    info.bundleName_ = "HandleMoveDragSurfaceNode";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);

    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));
    EXPECT_NE(session->moveDragController_, nullptr);

    session->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_START);
    session->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG);
    session->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_MOVE);
    session->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_END);
}

/**
 * @tc.name: HandleMoveDragSurfaceNodeRemoveCloneNode
 * @tc.desc: HandleMoveDragSurfaceNodeRemoveCloneNode Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleMoveDragSurfaceNodeRemoveCloneNode, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleMoveDragSurfaceNodeRemoveCloneNode";
    info.bundleName_ = "HandleMoveDragSurfaceNodeRemoveCloneNode";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    EXPECT_NE(sceneSession->moveDragController_, nullptr);
    sceneSession->moveDragController_->moveDragStartDisplayId_ = 0;
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    // create surfacenode
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = info.abilityName_;
    RSSurfaceNodeType rsSurfaceNodeType = RSSurfaceNodeType::DEFAULT;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig, rsSurfaceNodeType);
    sceneSession->SetSurfaceNode(surfaceNode);
    // set displayId to moveDrag map
    sceneSession->moveDragController_->displayIdSetDuringMoveDrag_.insert(0);
    sceneSession->moveDragController_->displayIdSetDuringMoveDrag_.insert(1001);
    // register FindScenePanelRsNodeByZOrderFunc to get drag mounted node
    sceneSession->SetFindScenePanelRsNodeByZOrderFunc([this](uint64_t screenId, uint32_t targetZOrder) {
        return CreateRSSurfaceNode();
    });
    
    // Constructing origin screen rect information
    auto originScreenId = 0;
    ScreenProperty originScreenProperty;
    originScreenProperty.SetStartX(0);
    originScreenProperty.SetStartY(0);
    originScreenProperty.SetBounds({{0, 0, 1000, 1000}, 10.0f, 10.0f});
    originScreenProperty.SetScreenType(ScreenType::REAL);
    sptr<ScreenSession> originScreenSession =
        sptr<ScreenSession>::MakeSptr(originScreenId, originScreenProperty, originScreenId);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(0, originScreenSession);

    // set original position Z
    sceneSession->moveDragController_->originalPositionZ_ = 100;
    // set same screen
    sceneSession->moveDragController_->moveDragEndDisplayId_ = 0;
    // test func
    sceneSession->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_END);
    EXPECT_EQ(surfaceNode->GetStagingProperties().GetPositionZ(), 100);

    // set original position Z
    sceneSession->moveDragController_->originalPositionZ_ = 200;
    // set different screen
    sceneSession->moveDragController_->moveDragEndDisplayId_ = 1001;
    // test func
    sceneSession->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_END);
    EXPECT_EQ(surfaceNode->GetStagingProperties().GetPositionZ(), 200);
}

/**
 * @tc.name: HandleMoveDragSurfaceBounds
 * @tc.desc: HandleMoveDragSurfaceBounds Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleMoveDragSurfaceBounds, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleMoveDragSurfaceBounds";
    info.bundleName_ = "HandleMoveDragSurfaceBounds";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    WSRect preRect = { 0, 0, 50, 50 };
    WSRect rect = { 0, 0, 100, 100 };
    WSRect globalRect = { 0, 0, 100, 100 };

    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    EXPECT_CALL(*mockVsyncStation, RequestVsync(_))
        .WillRepeatedly(Invoke([](const std::shared_ptr<VsyncCallback>& callback) {
            ASSERT_NE(callback, nullptr);
            callback->onCallback(1, 1);
        }));
    session->SetVsyncStation(mockVsyncStation);
    ASSERT_NE(nullptr, session->vsyncStation_);

    session->SetSessionRect(preRect);
    EXPECT_EQ(preRect, session->GetSessionRect());
    session->keyFramePolicy_.running_ = true;
    session->keyFrameCloneNode_ = RSWindowKeyFrameNode::Create();
    session->HandleMoveDragSurfaceBounds(rect, globalRect, SizeChangeReason::DRAG_MOVE);
    EXPECT_EQ(preRect, session->GetSessionRect());
    session->HandleMoveDragSurfaceBounds(rect, globalRect, SizeChangeReason::DRAG);
    EXPECT_EQ(rect, session->GetSessionRect());

    session->keyFramePolicy_.running_ = false;
    session->HandleMoveDragSurfaceBounds(rect, globalRect, SizeChangeReason::DRAG_END);
    session->HandleMoveDragSurfaceBounds(rect, globalRect, SizeChangeReason::DRAG);
    EXPECT_EQ(rect, session->GetSessionRect());
}

/**
 * @tc.name: HandleMoveDragSurfaceBounds02
 * @tc.desc: HandleMoveDragSurfaceBounds for interrupted
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleMoveDragSurfaceBounds02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleMoveDragSurfaceBounds02";
    info.bundleName_ = "HandleMoveDragSurfaceBounds02";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSRect preRect = { 0, 0, 50, 50 };
    WSRect rect = { 0, 0, 100, 100 };
    WSRect globalRect = { 0, 0, 100, 100 };

    auto mockVsyncStation = std::make_shared<MockVsyncStation>();
    EXPECT_CALL(*mockVsyncStation, RequestVsync(_))
        .WillRepeatedly(Invoke([](const std::shared_ptr<VsyncCallback>& callback) {
            ASSERT_NE(callback, nullptr);
            callback->onCallback(1, 1);
        }));
    session->SetVsyncStation(mockVsyncStation);
    ASSERT_NE(nullptr, session->vsyncStation_);

    session->SetSessionRect(preRect);
    EXPECT_EQ(preRect, session->GetSessionRect());
    session->keyFramePolicy_.running_ = false;
    session->moveDragController_->SetStartDragFlag(false);
    session->HandleMoveDragSurfaceBounds(rect, globalRect, SizeChangeReason::DRAG_END);
    session->HandleMoveDragSurfaceBounds(rect, globalRect, SizeChangeReason::DRAG);
    EXPECT_EQ(rect, session->GetSessionRect());
    EXPECT_EQ(false, session->moveDragController_->GetStartDragFlag());

    session->moveDragController_->SetStartDragFlag(true);
    session->HandleMoveDragSurfaceBounds(rect, globalRect, SizeChangeReason::DRAG);
    EXPECT_EQ(rect, session->GetSessionRect());
    session->HandleMoveDragSurfaceBounds(rect, globalRect, SizeChangeReason::DRAG_END);
    EXPECT_EQ(true, session->moveDragController_->GetStartDragFlag());
}

/**
 * @tc.name: HandleMoveDragSurfaceBounds03
 * @tc.desc: HandleMoveDragSurfaceBounds for fold pc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HandleMoveDragSurfaceBounds03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleMoveDragSurfaceBounds03";
    info.bundleName_ = "HandleMoveDragSurfaceBounds03";
    sptr<MainSession> session = sptr<MainSession>::MakeSptr(info, nullptr);
    session->SetScreenId(0);
    session->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(session));
    session->moveDragController_->moveDragProperty_.pointerType_ = MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    session->moveDragController_->SetStartDragFlag(true);
    ASSERT_NE(session->pcFoldScreenController_, nullptr);
    auto oldDisplayId = PcFoldScreenManager::GetInstance().displayId_;
    auto oldScreenFoldStatus = PcFoldScreenManager::GetInstance().screenFoldStatus_;
    PcFoldScreenManager::GetInstance().displayId_ = session->GetScreenId();
    PcFoldScreenManager::GetInstance().screenFoldStatus_ = SuperFoldStatus::HALF_FOLDED;
    WSRect rect = { 0, 0, 100, 100 };
    WSRect globalRect = { 0, 0, 100, 100 };
    session->HandleMoveDragSurfaceBounds(rect, globalRect, SizeChangeReason::DRAG_END);
    EXPECT_EQ(session->moveDragController_->GetStartDragFlag(), true);
    session->HandleMoveDragSurfaceBounds(rect, globalRect, SizeChangeReason::DRAG_MOVE);
    EXPECT_EQ(session->moveDragController_->GetStartDragFlag(), true);
    PcFoldScreenManager::GetInstance().displayId_ = oldDisplayId;
    PcFoldScreenManager::GetInstance().screenFoldStatus_ = oldScreenFoldStatus;
}

/**
 * @tc.name: OnNextVsyncReceivedWhenDrag
 * @tc.desc: OnNextVsyncReceivedWhenDrag
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, OnNextVsyncReceivedWhenDrag, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnNextVsyncReceivedWhenDrag";
    info.bundleName_ = "OnNextVsyncReceivedWhenDrag";
    LOG_SetCallback(MyLogCallback);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->moveDragController_ = nullptr;
    WSRect globalRect = { 0, 0, 100, 100 };
    bool isGlobal = true;
    bool needFlush = true;
    bool needSetBoundsNextVsync = true;
    session->UpdateRectForDrag(globalRect);
    session->OnNextVsyncReceivedWhenDrag(globalRect, isGlobal, needFlush, needSetBoundsNextVsync);
    EXPECT_TRUE(g_errLog.find("session moveDragController is null") != std::string::npos);
}

/**
 * @tc.name: SetNotifyVisibleChangeFunc
 * @tc.desc: SetNotifyVisibleChangeFunc Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetNotifyVisibleChangeFunc, TestSize.Level1)
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
 * @tc.name: NotifyServerToUpdateRect01
 * @tc.desc: NotifyServerToUpdateRect01 Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, NotifyServerToUpdateRect01, TestSize.Level1)
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

    session->SetClientRect(session->GetSessionRect());
    EXPECT_EQ(session->NotifyServerToUpdateRect(uiParam, SizeChangeReason::UNDEFINED), false); // skip same rect
}

/**
 * @tc.name: MoveUnderInteriaAndNotifyRectChange
 * @tc.desc: test func: MoveUnderInteriaAndNotifyRectChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, MoveUnderInteriaAndNotifyRectChange, TestSize.Level1)
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
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::HALF_FOLDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
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
 * @tc.name: WindowScaleTransfer01
 * @tc.desc: WindowScaleTransfer01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, WindowScaleTransfer01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "WindowScaleTransfer01";
    info.bundleName_ = "WindowScaleTransfer01";
    info.screenId_ = 0;
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    WSRect rect = { 100, 100, 400, 400 };
    WSRect resultRect = { 100, 100, 200, 200 };
    float scaleX = 0.5f;
    float scaleY = 0.5f;
    mainSession->GetLayoutController()->SetSessionRect(rect);
    mainSession->SetScale(scaleX, scaleY, 0.5f, 0.5f);
    mainSession->WindowScaleTransfer(mainSession->GetLayoutController()->winRect_, scaleX, scaleY);
    EXPECT_EQ(mainSession->GetSessionRect(), resultRect);
}

/**
 * @tc.name: WindowScaleTransfer02
 * @tc.desc: WindowScaleTransfer02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, WindowScaleTransfer02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "WindowScaleTransfer02";
    info.bundleName_ = "WindowScaleTransfer02";
    info.screenId_ = 0;
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    WSRect rect = { 200, 200, 200, 200 };
    WSRect resultRect = { 200, 200, 400, 400 };
    float scaleX = 2.0f;
    float scaleY = 2.0f;
    mainSession->GetLayoutController()->SetSessionRect(rect);
    mainSession->SetScale(scaleX, scaleY, 0.5f, 0.5f);
    mainSession->WindowScaleTransfer(mainSession->GetLayoutController()->winRect_, scaleX, scaleY);
    EXPECT_EQ(mainSession->GetSessionRect(), resultRect);
}

/**
 * @tc.name: IsCompatibilityModeScale01
 * @tc.desc: IsCompatibilityModeScale01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, IsCompatibilityModeScale01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsCompatibilityModeScale01";
    info.bundleName_ = "IsCompatibilityModeScale01";
    info.screenId_ = 0;
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    auto property = mainSession->GetSessionProperty();
    float scaleX = 2.0f;
    float scaleY = 2.0f;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    mainSession->property_->SetCompatibleModeProperty(compatibleModeProperty);
    bool res = mainSession->IsCompatibilityModeScale(scaleX, scaleY);
    EXPECT_EQ(res, true);
    compatibleModeProperty->SetIsAdaptToProportionalScale(false);
    mainSession->property_->SetCompatibleModeProperty(compatibleModeProperty);
    res = mainSession->IsCompatibilityModeScale(scaleX, scaleY);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: IsCompatibilityModeScale02
 * @tc.desc: IsCompatibilityModeScale02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, IsCompatibilityModeScale02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsCompatibilityModeScale02";
    info.bundleName_ = "IsCompatibilityModeScale02";
    info.screenId_ = 0;
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    auto property = mainSession->GetSessionProperty();
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    mainSession->property_->SetCompatibleModeProperty(compatibleModeProperty);
    bool res = mainSession->IsCompatibilityModeScale(scaleX, scaleY);
    EXPECT_EQ(res, false);
    compatibleModeProperty->SetIsAdaptToProportionalScale(false);
    mainSession->property_->SetCompatibleModeProperty(compatibleModeProperty);
    res = mainSession->IsCompatibilityModeScale(scaleX, scaleY);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: HookStartMoveRect
 * @tc.desc: HookStartMoveRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, HookStartMoveRect, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HookStartMoveRect";
    info.bundleName_ = "HookStartMoveRect";
    info.screenId_ = 0;
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    WSRect preRect = { 100, 100, 400, 400 };
    WSRect resultRect = { 100, 100, 200, 200 };
    float scaleX = 0.5f;
    float scaleY = 0.5f;
    mainSession->SetScale(scaleX, scaleY, 0.5f, 0.5f);
    mainSession->SetSessionRect(preRect);
    EXPECT_EQ(preRect, mainSession->GetSessionRect());
    auto property = mainSession->GetSessionProperty();
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(false);
    mainSession->property_->SetCompatibleModeProperty(compatibleModeProperty);
    WSRect currRect;
    mainSession->HookStartMoveRect(currRect, mainSession->GetSessionRect());
    EXPECT_EQ(preRect, currRect);
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    mainSession->property_->SetCompatibleModeProperty(compatibleModeProperty);
    mainSession->HookStartMoveRect(currRect, mainSession->GetSessionRect());
    EXPECT_EQ(resultRect, currRect);
}

/**
 * @tc.name: CompatibilityModeWindowScaleTransfer
 * @tc.desc: CompatibilityModeWindowScaleTransfer
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, CompatibilityModeWindowScaleTransfer, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CompatibilityModeWindowScaleTransfer";
    info.bundleName_ = "CompatibilityModeWindowScaleTransfer";
    info.screenId_ = 0;
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    WSRect preRect = { 100, 100, 400, 400 };
    WSRect noChangeRect = { 100, 100, 400, 400 };
    WSRect resultRect = { 100, 100, 200, 200 };
    float scaleX = 0.5f;
    float scaleY = 0.5f;
    bool isScale = true;
    mainSession->SetScale(scaleX, scaleY, 0.5f, 0.5f);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    compatibleModeProperty->SetIsAdaptToProportionalScale(false);
    mainSession->property_->SetCompatibleModeProperty(compatibleModeProperty);
    mainSession->CompatibilityModeWindowScaleTransfer(preRect, isScale);
    EXPECT_EQ(noChangeRect, preRect);
    compatibleModeProperty->SetIsAdaptToProportionalScale(true);
    mainSession->property_->SetCompatibleModeProperty(compatibleModeProperty);
    mainSession->CompatibilityModeWindowScaleTransfer(preRect, isScale);
    EXPECT_EQ(resultRect, preRect);
    isScale = false;
    mainSession->CompatibilityModeWindowScaleTransfer(preRect, isScale);
    EXPECT_EQ(noChangeRect, preRect);
}

/**
 * @tc.name: ThrowSlipDirectly
 * @tc.desc: ThrowSlipDirectly
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, ThrowSlipDirectly, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ThrowSlipDirectly";
    info.bundleName_ = "ThrowSlipDirectly";
    info.screenId_ = 0;
    sptr<MainSession> mainSession = sptr<MainSession>::MakeSptr(info, nullptr);
    WSRect rect = { 100, 100, 400, 400 };
    mainSession->GetLayoutController()->SetSessionRect(rect);
    mainSession->ThrowSlipDirectly(ThrowSlipMode::THREE_FINGERS_SWIPE, WSRectF{ 0.0f, 0.0f, 0.0f, 0.0f });
    EXPECT_EQ(mainSession->GetSessionRect(), rect);
}

/**
 * @tc.name: MarkSystemSceneUIFirst
 * @tc.desc: MarkSystemSceneUIFirst function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, MarkSystemSceneUIFirst, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, IsMissionHighlighted, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, SetSessionDisplayIdChangeCallback, TestSize.Level1)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetSessionDisplayIdChangeCallback([](uint64_t displayId) { return; });
    ASSERT_NE(sceneSession->sessionDisplayIdChangeFunc_, nullptr);
}

/**
 * @tc.name: NotifySessionDisplayIdChange
 * @tc.desc: NotifySessionDisplayIdChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, NotifySessionDisplayIdChange, TestSize.Level1)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    uint64_t checkDisplayId = 345;
    uint64_t moveDisplayId = 456;
    sceneSession->sessionDisplayIdChangeFunc_ = [&checkDisplayId](uint64_t displayId) { checkDisplayId = displayId; };
    sceneSession->NotifySessionDisplayIdChange(moveDisplayId);
    ASSERT_EQ(moveDisplayId, checkDisplayId);
}

/**
 * @tc.name: CheckAndMoveDisplayIdRecursively
 * @tc.desc: CheckAndMoveDisplayIdRecursively
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, CheckAndMoveDisplayIdRecursively, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, SetShouldFollowParentWhenShow, TestSize.Level1)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetShouldFollowParentWhenShow(false);
    ASSERT_EQ(sceneSession->shouldFollowParentWhenShow_, false);
}

HWTEST_F(SceneSessionTest5, CheckSubSessionShouldFollowParent, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, ActivateKeyboardAvoidArea01, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, IsSameMainSession, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, HandleActionUpdateExclusivelyHighlighted, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, SetHighlightChangeNotifyFunc, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, StartMovingWithCoordinate_01, TestSize.Level1)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->moveDragController_ = nullptr;
    WSError result = sceneSession->StartMovingWithCoordinate(100, 50, 300, 500, 0);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: StartMovingWithCoordinate_02
 * @tc.desc: StartMovingWithCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, StartMovingWithCoordinate_02, TestSize.Level1)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    sceneSession->moveDragController_->isStartMove_ = true;
    WSError result = sceneSession->StartMovingWithCoordinate(100, 50, 300, 500, 0);
    EXPECT_EQ(result, WSError::WS_ERROR_REPEAT_OPERATION);
}

/**
 * @tc.name: StartMovingWithCoordinate_03
 * @tc.desc: StartMovingWithCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, StartMovingWithCoordinate_03, TestSize.Level1)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    sceneSession->moveDragController_->isStartMove_ = false;
    WSError result = sceneSession->StartMovingWithCoordinate(100, 50, 300, 500, 0);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: StartMovingWithCoordinate_04
 * @tc.desc: StartMovingWithCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, StartMovingWithCoordinate_04, TestSize.Level1)
{
    const SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    sceneSession->moveDragController_->isStartMove_ = false;
    WSError result = sceneSession->StartMovingWithCoordinate(100, 50, 300, 500, 999);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetColorSpace
 * @tc.desc: SetColorSpace function01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetColorSpace, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, UpdateCrossAxisOfLayout, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, UpdateCrossAxis, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, GetCrossAxisState, TestSize.Level1)
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
HWTEST_F(SceneSessionTest5, GetWaterfallMode, TestSize.Level1)
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

/**
 * @tc.name: UpdateDensity
 * @tc.desc: UpdateDensity Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, UpdateDensity, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "updateDensity";
    info.bundleName_ = "updateDensity";
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_END);
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);
    SystemSessionConfig systemConfig;
    systemConfig.windowUIType_ = WindowUIType::PC_WINDOW;
    session->SetSystemConfig(systemConfig);
    session->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    session->property_->SetLastLimitsVpr(1.9f);
    session->property_->SetDisplayId(0);

    WSRect rect = { 10, 10, 800, 600 };
    session->GetLayoutController()->SetSessionRect(rect);
    DMRect availableArea = { 0, 0, 1000, 800 };
    ScreenProperty screenProperty;
    screenProperty.SetAvailableArea(availableArea);
    ScreenSessionConfig config = {
        .screenId = 0,
        .rsId = 0,
        .name = "updateDensity",
        .property = screenProperty,
    };
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(nullptr, screenSession);

    session->UpdateDensity();
    WSRect resultRect = { 10, 10, 800, 600 };
    EXPECT_EQ(session->GetSessionRect(), resultRect);
}

/**
 * @tc.name: NotifyRotationChange
 * @tc.desc: NotifyRotationChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, NotifyRotationChange, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSessionMocker> session = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    session->sessionStage_ = nullptr;
    session->isRotationChangeCallbackRegistered = false;
    RotationChangeInfo rotationInfo = { RotationChangeType::WINDOW_WILL_ROTATE, 0, 0, { 0, 0, 2720, 1270 } };
    RotationChangeResult res = session->NotifyRotationChange(rotationInfo);
    EXPECT_EQ(res.windowRect_.width_, 0);

    session->isRotationChangeCallbackRegistered = true;
    res = session->NotifyRotationChange(rotationInfo);
    EXPECT_EQ(res.windowRect_.width_, 0);

    sptr<SessionStageMocker> sessionStageMocker = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStageMocker, nullptr);
    session->sessionStage_ = sessionStageMocker;
    res = session->NotifyRotationChange(rotationInfo);
    EXPECT_EQ(res.windowRect_.width_, 0);
}

/**
 * @tc.name: NotifyRotationChange_IsRestrictNotify_SystemWindow
 * @tc.desc: NotifyRotationChange_IsRestrictNotify_SystemWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, NotifyRotationChange_IsRestrictNotify_SystemWindow, Function | SmallTest | Level2)
{
    sptr<SessionStageMocker> sessionStageMocker = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStageMocker, nullptr);
    RotationChangeInfo rotationInfo = { RotationChangeType::WINDOW_WILL_ROTATE, 0, 0, { 0, 0, 2720, 1270 } };
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    SessionInfo info;
    property->SetWindowType(WindowType::WINDOW_TYPE_DESKTOP);
    sptr<SceneSessionMocker> session = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    session->isRotationChangeCallbackRegistered = true;
    session->sessionStage_ = sessionStageMocker;
    bool isRestrictNotify = false;
    property->isSystemCalling_ = true;
    session->SetSessionProperty(property);
    RotationChangeResult res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);

    property->isSystemCalling_ = false;
    session->SetSessionProperty(property);
    res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);

    isRestrictNotify = true;
    property->isSystemCalling_ = true;
    session->SetSessionProperty(property);
    res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);

    property->isSystemCalling_ = false;
    session->SetSessionProperty(property);
    res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);
}

/**
 * @tc.name: NotifyRotationChange_IsRestrictNotify_NotSystemWindow
 * @tc.desc: NotifyRotationChange_IsRestrictNotify_NotSystemWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, NotifyRotationChange_IsRestrictNotify_NotSystemWindow, Function | SmallTest | Level2)
{
    sptr<SessionStageMocker> sessionStageMocker = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStageMocker, nullptr);
    RotationChangeInfo rotationInfo = { RotationChangeType::WINDOW_WILL_ROTATE, 0, 0, { 0, 0, 2720, 1270 } };
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    SessionInfo info;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSessionMocker> session = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    session->isRotationChangeCallbackRegistered = true;
    session->sessionStage_ = sessionStageMocker;
    bool isRestrictNotify = false;
    property->isSystemCalling_ = true;
    session->SetSessionProperty(property);
    RotationChangeResult res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);

    property->isSystemCalling_ = false;
    session->SetSessionProperty(property);
    res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);

    isRestrictNotify = true;
    property->isSystemCalling_ = true;
    session->SetSessionProperty(property);
    res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);

    property->isSystemCalling_ = false;
    session->SetSessionProperty(property);
    res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);
}

/**
 * @tc.name: SetSessionGetTargetOrientationConfigInfoCallback
 * @tc.desc: SetSessionGetTargetOrientationConfigInfoCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSessionGetTargetOrientationConfigInfoCallback, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "SetSessionGetTargetOrientationConfigInfoCallback";
    info.bundleName_ = "SetSessionGetTargetOrientationConfigInfoCallback";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    sceneSession->SetSessionGetTargetOrientationConfigInfoCallback([](uint32_t targetOrientation) { return; });
    EXPECT_NE(sceneSession->sessionGetTargetOrientationConfigInfoFunc_, nullptr);
}

/**
 * @tc.name: GetTargetOrientationConfigInfo
 * @tc.desc: GetTargetOrientationConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, GetTargetOrientationConfigInfo, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "GetTargetOrientationConfigInfo";
    info.bundleName_ = "GetTargetOrientationConfigInfo";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    Orientation orientation = Orientation::USER_ROTATION_PORTRAIT;
    std::map<Rosen::WindowType, Rosen::SystemBarProperty> targetProperties;
    std::map<Rosen::WindowType, Rosen::SystemBarProperty> currentProperties;
    NotifySessionGetTargetOrientationConfigInfoFunc func = [](uint32_t targetOrientation) {};
    sceneSession->sessionGetTargetOrientationConfigInfoFunc_ = func;
    sceneSession->GetTargetOrientationConfigInfo(orientation, targetProperties, currentProperties);
    EXPECT_NE(sceneSession->sessionGetTargetOrientationConfigInfoFunc_, nullptr);
}

/**
 * @tc.name: NotifyRotationProperty
 * @tc.desc: NotifyRotationProperty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, NotifyRotationProperty, Function | SmallTest | Level2)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo info;
    info.abilityName_ = "NotifyRotationProperty";
    info.bundleName_ = "NotifyRotationProperty";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    WSError result = sceneSession->NotifyRotationProperty(0, 0, 0);
    EXPECT_EQ(result, WSError::WS_OK);
    WSError result1 = sceneSession->NotifyRotationProperty(0, 0, 1);
    EXPECT_EQ(result1, WSError::WS_OK);
    WSError result2 = sceneSession->NotifyRotationProperty(0, 1, 1);
    EXPECT_EQ(result2, WSError::WS_OK);
    WSError result3 = sceneSession->NotifyRotationProperty(90, 1, 1);
    EXPECT_EQ(result3, WSError::WS_OK);
    sceneSession->GetSessionProperty()->SetDisplayId(1024);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1024, nullptr);
    sceneSession->NotifyRotationProperty(90, 1, 1);
    usleep(100000);
    EXPECT_TRUE(g_errLog.find("failed to convert Rotation to Orientation") != std::string::npos);
}

/**
 * @tc.name: ConvertRotationToOrientation
 * @tc.desc: ConvertRotationToOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, ConvertRotationToOrientation, Function | SmallTest | Level2)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo info;
    info.abilityName_ = "ConvertRotationToOrientation";
    info.bundleName_ = "ConvertRotationToOrientation";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    uint32_t orientation = 0;
    WSError result = sceneSession->ConvertRotationToOrientation(90, 1, 1, orientation);
    EXPECT_EQ(result, WSError::WS_OK);
    
    sceneSession->GetSessionProperty()->SetDisplayId(1024);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1024, nullptr);
    uint32_t orientation1 = 0;
    WSError result1 = sceneSession->ConvertRotationToOrientation(90, 1, 1, orientation1);
    EXPECT_EQ(result1, WSError::WS_ERROR_INVALID_DISPLAY);
    EXPECT_TRUE(g_errLog.find("Screen session is null") != std::string::npos);
}

/**
 * @tc.name: HandleActionUpdateViewKeepScreenOn
 * @tc.desc: HandleActionUpdateViewKeepScreenOn
 * @tc.type: FUNC
 */
 HWTEST_F(SceneSessionTest5, HandleActionUpdateViewKeepScreenOn, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateViewKeepScreenOn";
    info.bundleName_ = "HandleActionUpdateViewKeepScreenOn";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetViewKeepScreenOn(true);
    WMError result = sceneSession->HandleActionUpdateViewKeepScreenOn(
        property, WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON);
    EXPECT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: SetSkipSelfWhenShowOnVirtualScreen
 * @tc.desc: SetSkipSelfWhenShowOnVirtualScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSkipSelfWhenShowOnVirtualScreen, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSkipSelfWhenShowOnVirtualScreen";
    info.bundleName_ = "SetSkipSelfWhenShowOnVirtualScreen";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    EXPECT_EQ(sceneSession->GetSessionProperty()->GetSkipSelfWhenShowOnVirtualScreen(), true);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    ASSERT_NE(surfaceNode, nullptr);
    sceneSession->surfaceNode_ = surfaceNode;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    sceneSession->SetSessionProperty(property);

    uint64_t persistentId = 0;
    bool isSkip = false;
    sptr<SceneSession::SpecificSessionCallback> specificCallBack =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCallBack->onSetSkipSelfWhenShowOnVirtualScreen_ =
        [&persistentId, &isSkip](uint64_t id, bool skip) {
            persistentId = id;
            isSkip = skip;
        };
    sceneSession->specificCallback_ = specificCallBack;

    sceneSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    EXPECT_EQ(persistentId, surfaceNode->GetId());
    EXPECT_EQ(isSkip, true);

    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionProperty(property);
    sceneSession->SetSkipSelfWhenShowOnVirtualScreen(false);
    EXPECT_EQ(persistentId, sceneSession->GetMissionId());
    EXPECT_EQ(isSkip, false);
}

/**
 * @tc.name: SetSkipEventOnCastPlus
 * @tc.desc: SetSkipEventOnCastPlus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, SetSkipEventOnCastPlus, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSkipEventOnCastPlus";
    info.bundleName_ = "SetSkipEventOnCastPlus";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    int32_t persistentId = 0;
    bool isSkip = false;
    sptr<SceneSession::SpecificSessionCallback> specificCallBack =
         sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCallBack->onSetSkipEventOnCastPlus_ =
        [&persistentId, &isSkip](int32_t id, bool skip) {
            persistentId = id;
            isSkip = skip;
        };
    sceneSession->specificCallback_ = specificCallBack;
    sceneSession->SetSkipEventOnCastPlus(true);
    EXPECT_EQ(persistentId, sceneSession->GetMissionId());
    EXPECT_EQ(isSkip, true);
}

/**
 * @tc.name: GetSystemBarPropertyForRotation
 * @tc.desc: GetSystemBarPropertyForRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest5, GetSystemBarPropertyForRotation, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "GetSystemBarPropertyForRotation";
    info.bundleName_ = "GetSystemBarPropertyForRotation";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    std::map<Rosen::WindowType, Rosen::SystemBarProperty> properties;
    sceneSession->SetSystemBarPropertyForRotation(properties);
    EXPECT_EQ(sceneSession->GetSystemBarPropertyForRotation(), properties);
}
} // namespace
} // namespace Rosen
} // namespace OHOS