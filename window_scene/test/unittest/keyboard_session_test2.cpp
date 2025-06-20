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

#include "session/host/include/keyboard_session.h"
#include <gtest/gtest.h>

#include "interfaces/include/ws_common.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_keyboard_session.h"
#include "session/host/include/session.h"
#include "session/host/include/scene_session.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "ui/rs_surface_node.h"
#include "window_helper.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "KeyboardSessionTest2" };
std::string g_logMsg;
void KeyboardSessionTest2Callback(const LogType type, const LogLevel level,
    const unsigned int domain, const char *tag, const char *msg)
{
    g_logMsg = msg;
}
}

constexpr int WAIT_ASYNC_US = 1000000;
class KeyboardSessionTest2 : public testing::Test, public IScreenConnectionListener {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void OnScreenConnected(const sptr<ScreenSession>& screenSession) override;
    void OnScreenDisconnected(const sptr<ScreenSession>& screenSession) override;
    void ConstructKeyboardCallingWindowTestData(sptr<SceneSession>& callingSession,
                                                sptr<KeyboardSession>& keyboardSession,
                                                sptr<SceneSession>& statusBarSession);

private:
    sptr<KeyboardSession> GetKeyboardSession(const std::string& abilityName, const std::string& bundleName);
    sptr<SceneSession> GetSceneSession(const std::string& abilityName, const std::string& bundleName);
    sptr<KSSceneSessionMocker> GetSceneSessionMocker(const std::string& abilityName, const std::string& bundleName);
};

void KeyboardSessionTest2::SetUpTestCase() {}

void KeyboardSessionTest2::TearDownTestCase() {}

void KeyboardSessionTest2::SetUp() {}

void KeyboardSessionTest2::TearDown() {}

void KeyboardSessionTest2::OnScreenConnected(const sptr<ScreenSession>& screenSession) {}

void KeyboardSessionTest2::OnScreenDisconnected(const sptr<ScreenSession>& screenSession) {}

void KeyboardSessionTest2::ConstructKeyboardCallingWindowTestData(sptr<SceneSession>& callingSession,
                                                                  sptr<KeyboardSession>& keyboardSession,
                                                                  sptr<SceneSession>& statusBarSession)
{
    SessionInfo info;
    info.abilityName_ = "KeyboardSessionTest2";
    info.bundleName_ = "KeyboardSessionTest2";
    sptr<SceneSession::SpecificSessionCallback> specCallback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(specCallback, nullptr);
    callingSession = sptr<SceneSession>::MakeSptr(info, specCallback);
    ASSERT_NE(callingSession, nullptr);
    keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(windowSessionProperty, nullptr);
    keyboardSession->property_ = windowSessionProperty;
    ASSERT_NE(keyboardSession->property_, nullptr);
    SessionInfo info1;
    info1.abilityName_ = "BindKeyboardPanelSession";
    info1.bundleName_ = "BindKeyboardPanelSession";
    sptr<SceneSession> panelSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(panelSession, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    sptr<SceneSession> getPanelSession = keyboardSession->GetKeyboardPanelSession();
    EXPECT_EQ(getPanelSession, panelSession);
    SessionInfo info2;
    info2.abilityName_ = "GetStatusBarHeight";
    info2.bundleName_ = "GetStatusBarHeight";
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(specificCallback_, nullptr);
    statusBarSession = sptr<SceneSession>::MakeSptr(info2, specificCallback_);
    ASSERT_NE(statusBarSession, nullptr);
    WSRect rect({ 0, 0, 0, 10 });
    statusBarSession->winRect_ = rect;
    specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ = [&](WindowType type,
        uint64_t displayId) -> std::vector<sptr<SceneSession>>{
        std::vector<sptr<SceneSession>> vec;
        vec.push_back(statusBarSession);
        return vec;
    };
}

sptr<KeyboardSession> KeyboardSessionTest2::GetKeyboardSession(const std::string& abilityName,
                                                               const std::string& bundleName)
{
    SessionInfo info;
    info.abilityName_ = abilityName;
    info.bundleName_ = bundleName;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);

    sptr<WindowSessionProperty> keyboardProperty = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(keyboardProperty, nullptr);
    keyboardProperty->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    keyboardSession->SetSessionProperty(keyboardProperty);

    return keyboardSession;
}

sptr<SceneSession> KeyboardSessionTest2::GetSceneSession(const std::string& abilityName, const std::string& bundleName)
{
    SessionInfo info;
    info.abilityName_ = abilityName;
    info.bundleName_ = bundleName;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);

    return sceneSession;
}

sptr<KSSceneSessionMocker> KeyboardSessionTest2::GetSceneSessionMocker(const std::string& abilityName,
                                                                       const std::string& bundleName)
{
    SessionInfo info;
    info.abilityName_ = abilityName;
    info.bundleName_ = bundleName;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KSSceneSessionMocker> mockSession = sptr<KSSceneSessionMocker>::MakeSptr(info, nullptr);

    return mockSession;
}

namespace {
/**
 * @tc.name: AdjustKeyboardLayout01
 * @tc.desc: AdjustKeyboardLayout
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, AdjustKeyboardLayout01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "AdjustKeyboardLayout01";
    info.bundleName_ = "AdjustKeyboardLayout01";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);

    KeyboardLayoutParams params;
    ASSERT_EQ(keyboardSession->AdjustKeyboardLayout(params), WSError::WS_OK);

    keyboardSession->adjustKeyboardLayoutFunc_ = nullptr;
    ASSERT_EQ(keyboardSession->AdjustKeyboardLayout(params), WSError::WS_OK);

    keyboardSession->adjustKeyboardLayoutFunc_ = [](const KeyboardLayoutParams& params) {};
    ASSERT_EQ(keyboardSession->AdjustKeyboardLayout(params), WSError::WS_OK);
}

/**
 * @tc.name: AdjustKeyboardLayout01
 * @tc.desc: AdjustKeyboardLayout
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, AdjustKeyboardLayout02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "AdjustKeyboardLayout02";
    info.bundleName_ = "AdjustKeyboardLayout02";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);

    KeyboardLayoutParams params;
    // params gravity is WINDOW_GRAVITY_BOTTOM
    ASSERT_EQ(keyboardSession->AdjustKeyboardLayout(params), WSError::WS_OK);
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    ASSERT_EQ(keyboardSession->AdjustKeyboardLayout(params), WSError::WS_OK);

    // params gravity is WINDOW_GRAVITY_FLOAT
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    ASSERT_EQ(keyboardSession->AdjustKeyboardLayout(params), WSError::WS_OK);
}

/**
 * @tc.name: AdjustKeyboardLayout03
 * @tc.desc: AdjustKeyboardLayout test
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, AdjustKeyboardLayout03, Function | SmallTest | Level1)
{
    SessionInfo info;
    info.abilityName_ = "AdjustKeyboardLayout03";
    info.bundleName_ = "AdjustKeyboardLayout03";
    sptr<SceneSession::SpecificSessionCallback> specificCb =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    ASSERT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);

    KeyboardLayoutParams params;
    EXPECT_EQ(keyboardSession->AdjustKeyboardLayout(params), WSError::WS_OK);

    keyboardSession->adjustKeyboardLayoutFunc_ = nullptr;
    EXPECT_EQ(keyboardSession->AdjustKeyboardLayout(params), WSError::WS_OK);

    keyboardSession->adjustKeyboardLayoutFunc_ = [](const KeyboardLayoutParams& params){};
    EXPECT_EQ(keyboardSession->AdjustKeyboardLayout(params), WSError::WS_OK);

    params.displayId_ = 100;
    EXPECT_EQ(keyboardSession->AdjustKeyboardLayout(params), WSError::WS_OK);

    keyboardSession->adjustKeyboardLayoutFunc_ = nullptr;
    EXPECT_EQ(keyboardSession->AdjustKeyboardLayout(params), WSError::WS_OK);
}

/**
 * @tc.name: HandleMoveDragSurfaceNode01
 * @tc.desc: HandleMoveDragSurfaceNode test
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, HandleMoveDragSurfaceNode01, Function | SmallTest | Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(KeyboardSessionTest2Callback);
    sptr<KeyboardSession> keyboardSession =
        GetKeyboardSession("HandleMoveDragSurfaceNode01", "HandleMoveDragSurfaceNode01");
    ASSERT_NE(keyboardSession, nullptr);

    ScreenSessionManagerClient::GetInstance().RegisterScreenConnectionListener(this);
    SessionOption option = {
        .rsId_ = 0,
        .isExtend_ = false,
        .screenId_ = 0,
    };
    ScreenEvent screenEvent = ScreenEvent::CONNECTED;
    ScreenSessionManagerClient::GetInstance().OnScreenConnectionChanged(option, screenEvent);

    sptr<MoveDragController> moveDragController = keyboardSession->GetMoveDragController();
    ASSERT_NE(moveDragController, nullptr);
    WSRect rect = { 0, 0, 200, 200 };
    moveDragController->SetTargetRect(rect);
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_MOVE);
    EXPECT_TRUE(g_logMsg.find("keyboard panel session is null") != std::string::npos);

    moveDragController->ResetCrossMoveDragProperty();
    moveDragController->SetTargetRect(rect);
    SessionInfo info;
    info.abilityName_ = "HandleMoveDragSurfaceNode01";
    info.bundleName_ = "HandleMoveDragSurfaceNode01";
    sptr<SceneSession> panelSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(panelSession, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_MOVE);
    EXPECT_TRUE(g_logMsg.find("keyboard panel surface node is null") != std::string::npos);

    moveDragController->ResetCrossMoveDragProperty();
    moveDragController->SetTargetRect(rect);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    panelSession->SetSurfaceNode(surfaceNode);
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_MOVE);
    EXPECT_TRUE(g_logMsg.find("Add widow") != std::string::npos);

    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_END);
    EXPECT_TRUE(g_logMsg.find("Remove window") != std::string::npos);
}

/**
 * @tc.name: CheckIfNeedRaiseCallingSession
 * @tc.desc: CheckIfNeedRaiseCallingSession
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, CheckIfNeedRaiseCallingSession, TestSize.Level1)
{
    WLOGFI("CheckIfNeedRaiseCallingSession begin!");
    SessionInfo info;
    info.abilityName_ = "CheckIfNeedRaiseCallingSession";
    info.bundleName_ = "CheckIfNeedRaiseCallingSession";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    keyboardSession->SetSessionProperty(property);

    ASSERT_FALSE(keyboardSession->CheckIfNeedRaiseCallingSession(nullptr, true));

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    EXPECT_NE(sceneSession, nullptr);

    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    ASSERT_FALSE(keyboardSession->CheckIfNeedRaiseCallingSession(sceneSession, true));

    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    ASSERT_TRUE(keyboardSession->CheckIfNeedRaiseCallingSession(sceneSession, false));

    property->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);

    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_FALSE(keyboardSession->CheckIfNeedRaiseCallingSession(sceneSession, true));

    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    keyboardSession->CheckIfNeedRaiseCallingSession(sceneSession, true);

    WLOGFI("CheckIfNeedRaiseCallingSession end!");
}

/**
 * @tc.name: OpenKeyboardSyncTransaction
 * @tc.desc: OpenKeyboardSyncTransaction
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, OpenKeyboardSyncTransaction, TestSize.Level1)
{
    std::string abilityName = "OpenKeyboardSyncTransaction";
    std::string bundleName = "OpenKeyboardSyncTransaction";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, bundleName);

    WSRect keyboardPanelRect = { 0, 0, 0, 0 };
    bool isKeyboardShow = true;
    WindowAnimationInfo animationInfo;

    // isKeyBoardSyncTransactionOpen_ is false
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, isKeyboardShow, animationInfo);
    keyboardSession->OpenKeyboardSyncTransaction();

    // isKeyBoardSyncTransactionOpen_ is true
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, isKeyboardShow, animationInfo);
    keyboardSession->OpenKeyboardSyncTransaction();
    ASSERT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, true);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, isKeyboardShow, animationInfo);
    ASSERT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);
}

/**
 * @tc.name: BindKeyboardPanelSession
 * @tc.desc: BindKeyboardPanelSession
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, BindKeyboardPanelSession, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "BindKeyboardPanelSession";
    info.bundleName_ = "BindKeyboardPanelSession";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
    sptr<SceneSession> panelSession = nullptr;
    keyboardSession->BindKeyboardPanelSession(panelSession);
    sptr<SceneSession> getPanelSession = keyboardSession->GetKeyboardPanelSession();
    ASSERT_EQ(getPanelSession, nullptr);
    panelSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(panelSession, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    getPanelSession = keyboardSession->GetKeyboardPanelSession();
    EXPECT_EQ(getPanelSession, panelSession);
}

/**
 * @tc.name: GetKeyboardGravity01
 * @tc.desc: GetKeyboardGravity01
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, GetKeyboardGravity01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "GetKeyboardGravity";
    info.bundleName_ = "GetKeyboardGravity";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    keyboardSession->property_ = windowSessionProperty;
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    auto ret = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(SessionGravity::SESSION_GRAVITY_BOTTOM, ret);
}

/**
 * @tc.name: NotifyKeyboardPanelInfoChange
 * @tc.desc: NotifyKeyboardPanelInfoChange
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, NotifyKeyboardPanelInfoChange, TestSize.Level1)
{
    WSRect rect = { 800, 800, 1200, 1200 };
    SessionInfo info;
    info.abilityName_ = "NotifyKeyboardPanelInfoChange";
    info.bundleName_ = "NotifyKeyboardPanelInfoChange";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = false;
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = nullptr;
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    keyboardSession->sessionStage_ = mockSessionStage;
    ASSERT_NE(keyboardSession->sessionStage_, nullptr);
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
}

/**
 * @tc.name: CheckIfNeedRaiseCallingSession01
 * @tc.desc: CheckIfNeedRaiseCallingSession01
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, CheckIfNeedRaiseCallingSession01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CheckIfNeedRaiseCallingSession";
    info.bundleName_ = "CheckIfNeedRaiseCallingSession";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(specificCb, nullptr);
    sptr<SceneSession> callingSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    ASSERT_NE(callingSession, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    keyboardSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    callingSession->systemConfig_.freeMultiWindowSupport_ = true;
    callingSession->systemConfig_.freeMultiWindowEnable_ = true;
    auto ret = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, true);
    EXPECT_EQ(ret, false);
    callingSession->systemConfig_.freeMultiWindowEnable_ = false;
    ret = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, true);
    EXPECT_EQ(ret, false);
    callingSession->systemConfig_.freeMultiWindowEnable_ = true;
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ret = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, true);
    EXPECT_EQ(ret, true);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    callingSession->systemConfig_.freeMultiWindowEnable_ = false;
    ret = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, true);
    EXPECT_EQ(ret, true);
    keyboardSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, true);
    EXPECT_EQ(ret, true);
    ret = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, false);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CheckIfNeedRaiseCallingSession02
 * @tc.desc: CheckIfNeedRaiseCallingSession02
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, CheckIfNeedRaiseCallingSession02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "CheckIfNeedRaiseCallingSession02";
    info.bundleName_ = "CheckIfNeedRaiseCallingSession02";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;


    sptr<SceneSession> callingSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(callingSession, nullptr);
    callingSession->isSubWindowResizingOrMoving_ = true;
    callingSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    auto ret = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, false);
    EXPECT_EQ(ret, true);

    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ret = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, false);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FlushUIParams03
 * @tc.desc: FlushUIParams03
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, FlushUIParams03, TestSize.Level1)
{
    sptr<SceneSession> callingSession = nullptr;
    sptr<KeyboardSession> keyboardSession = nullptr;
    sptr<SceneSession> statusBarSession = nullptr;
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;
    ConstructKeyboardCallingWindowTestData(callingSession, keyboardSession, statusBarSession);
    sptr<SceneSession> panelSession = keyboardSession->GetKeyboardPanelSession();

    callingSession->property_->SetDisplayId(999);
    keyboardSession->SetScreenId(0);
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FlushUIParams04
 * @tc.desc: FlushUIParams04
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, FlushUIParams04, TestSize.Level1)
{
    sptr<SceneSession> callingSession = nullptr;
    sptr<KeyboardSession> keyboardSession = nullptr;
    sptr<SceneSession> statusBarSession = nullptr;
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;
    ConstructKeyboardCallingWindowTestData(callingSession, keyboardSession, statusBarSession);
    sptr<SceneSession> panelSession = keyboardSession->GetKeyboardPanelSession();

    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    keyboardSession->property_->SetKeyboardLayoutParams(params);
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    WSRect lastSafeRect = { 1, 1, 1, 1 };
    callingSession->SetLastSafeRect(lastSafeRect);
    WSRect rect1 = { 1, 1, 100, 100 };
    callingSession->SetSessionRect(rect1);
    WSRect rect2 = { 1, 50, 100, 100 };
    panelSession->SetSessionRect(rect2);
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, false);

    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    keyboardSession->property_->SetKeyboardLayoutParams(params);
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    callingSession->SetIsMidScene(false);
    callingSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    callingSession->systemConfig_.freeMultiWindowEnable_ = false;
    callingSession->systemConfig_.freeMultiWindowSupport_ = false;
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FlushUIParams05
 * @tc.desc: FlushUIParams05
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, FlushUIParams05, TestSize.Level1)
{
    sptr<SceneSession> callingSession = nullptr;
    sptr<KeyboardSession> keyboardSession = nullptr;
    sptr<SceneSession> statusBarSession = nullptr;
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;
    ConstructKeyboardCallingWindowTestData(callingSession, keyboardSession, statusBarSession);
    sptr<SceneSession> panelSession = keyboardSession->GetKeyboardPanelSession();

    KeyboardLayoutParams params;
    params.landscapeAvoidHeight_ = 10;
    keyboardSession->property_->SetKeyboardLayoutParams(params);
    WSRect lastSafeRect = { 1, 1, 1, 1 };
    callingSession->SetLastSafeRect(lastSafeRect);
    WSRect rect1 = { 1, 1, 100, 100 };
    callingSession->SetSessionRect(rect1);
    WSRect rect2 = { 200, 200, 100, 100 };
    panelSession->SetSessionRect(rect2);
    keyboardSession->SetSessionState(SessionState::STATE_FOREGROUND);
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: RaiseCallingSession01
 * @tc.desc: RaiseCallingSession01
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, RaiseCallingSession01, TestSize.Level1)
{
    sptr<SceneSession> callingSession = nullptr;
    sptr<KeyboardSession> keyboardSession = nullptr;
    sptr<SceneSession> statusBarSession = nullptr;
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;
    ConstructKeyboardCallingWindowTestData(callingSession, keyboardSession, statusBarSession);

    keyboardSession->keyboardAvoidAreaActive_ = false;
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, false);
    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->isVisible_ = true;
    keyboardSession->SetSessionState(SessionState::STATE_BACKGROUND);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, true);
    EXPECT_EQ(ret, false);
    keyboardSession->isVisible_ = false;
    keyboardSession->SetSessionState(SessionState::STATE_ACTIVE);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, true);
    EXPECT_EQ(ret, false);
    callingSession = nullptr;
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, true);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: RaiseCallingSession02
 * @tc.desc: RaiseCallingSession02
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, RaiseCallingSession02, TestSize.Level1)
{
    sptr<SceneSession> callingSession = nullptr;
    sptr<KeyboardSession> keyboardSession = nullptr;
    sptr<SceneSession> statusBarSession = nullptr;
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;
    ConstructKeyboardCallingWindowTestData(callingSession, keyboardSession, statusBarSession);
    sptr<SceneSession> panelSession = keyboardSession->GetKeyboardPanelSession();

    keyboardSession->isVisible_ = false;
    keyboardSession->SetSessionState(SessionState::STATE_FOREGROUND);
    KeyboardLayoutParams params;
    params.landscapeAvoidHeight_ = -10;
    keyboardSession->property_->SetKeyboardLayoutParams(params);
    callingSession->SetOriPosYBeforeRaisedByKeyboard(10);
    WSRect lastSafeRect = { 1, 50, 100, 100 };
    callingSession->SetLastSafeRect(lastSafeRect);
    WSRect rect1 = { 1, 1, 100, 100 };
    callingSession->SetSessionRect(rect1);
    WSRect rect2 = { 200, 200, 100, 100 };
    panelSession->SetSessionRect(rect2);
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, true);

    lastSafeRect = { 0, 0, 0, 0 };
    callingSession->SetLastSafeRect(lastSafeRect);
    rect1 = { 1, 1, 100, 100 };
    callingSession->SetSessionRect(rect1);
    rect2 = { 200, 200, 100, 100 };
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: RaiseCallingSession03
 * @tc.desc: RaiseCallingSession03
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, RaiseCallingSession03, TestSize.Level1)
{
    sptr<SceneSession> callingSession = nullptr;
    sptr<KeyboardSession> keyboardSession = nullptr;
    sptr<SceneSession> statusBarSession = nullptr;
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;
    ConstructKeyboardCallingWindowTestData(callingSession, keyboardSession, statusBarSession);
    sptr<SceneSession> panelSession = keyboardSession->GetKeyboardPanelSession();

    keyboardSession->SetSessionState(SessionState::STATE_FOREGROUND);
    keyboardSession->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    int32_t statusHeight = callingSession->GetStatusBarHeight();
    WSRect callingSessionRect = { 0, statusHeight+10, 100, 100 };
    callingSession->SetSessionRect(callingSessionRect);
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    keyboardSession->property_->SetKeyboardLayoutParams(params);
    callingSession->SetIsMidScene(true);
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    callingSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    callingSession->SetOriPosYBeforeRaisedByKeyboard(0);
    WSRect lastSafeRect = { 1, 50, 100, 100 };
    callingSession->SetLastSafeRect(lastSafeRect);
    WSRect rect1 = { 1, 1, 100, 100 };
    callingSession->SetSessionRect(rect1);
    WSRect rect2 = { 50, 50, 100, 100 };
    panelSession->SetSessionRect(rect2);
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, true);
    callingSession->SetIsMidScene(false);
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    callingSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    callingSession->SetOriPosYBeforeRaisedByKeyboard(10);
    lastSafeRect = { 50, 50, 51, 51 };
    callingSession->SetLastSafeRect(lastSafeRect);
    callingSession->SetSessionRect(rect1);
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, false);
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    callingSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    callingSession->SetOriPosYBeforeRaisedByKeyboard(10);
    lastSafeRect = { 1, 50, 100, 100 };
    callingSession->SetLastSafeRect(lastSafeRect);
    callingSession->SetSessionRect(rect1);
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: RaiseCallingSession04
 * @tc.desc: RaiseCallingSession04
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, RaiseCallingSession04, TestSize.Level1)
{
    sptr<SceneSession> callingSession = nullptr;
    sptr<KeyboardSession> keyboardSession = nullptr;
    sptr<SceneSession> statusBarSession = nullptr;
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;
    ConstructKeyboardCallingWindowTestData(callingSession, keyboardSession, statusBarSession);
    sptr<SceneSession> panelSession = keyboardSession->GetKeyboardPanelSession();

    int32_t statusHeight = statusBarSession->GetStatusBarHeight();
    ASSERT_EQ(statusHeight, 10);
    keyboardSession->SetSessionState(SessionState::STATE_FOREGROUND);
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    WSRect lastSafeRect = { 1, 50, 100, 100 };
    callingSession->SetLastSafeRect(lastSafeRect);
    WSRect rect1 = { 1, 1, 100, 100 };
    callingSession->SetSessionRect(rect1);
    WSRect rect2 = { 50, 50, 100, 100 };
    panelSession->SetSessionRect(rect2);
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, true);
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    lastSafeRect = { 1, statusHeight-1, 10, 9 };
    callingSession->SetLastSafeRect(lastSafeRect);
    rect1 = { 1, statusHeight-1, 10, 10 };
    callingSession->SetSessionRect(rect1);
    rect2 = { 1, statusHeight-2, 10, 10 };
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, false);
    callingSession->SetLastSafeRect(lastSafeRect);
    rect1 = { 1, statusHeight, 100, 100 };
    callingSession->SetSessionRect(rect1);
    rect2 = { 50, statusHeight-1, 100, 100 };
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, false);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: RaiseCallingSession05
 * @tc.desc: RaiseCallingSession05
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, RaiseCallingSession05, TestSize.Level1)
{
    sptr<SceneSession> callingSession = nullptr;
    sptr<KeyboardSession> keyboardSession = nullptr;
    sptr<SceneSession> statusBarSession = nullptr;
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;
    ConstructKeyboardCallingWindowTestData(callingSession, keyboardSession, statusBarSession);
    sptr<SceneSession> panelSession = keyboardSession->GetKeyboardPanelSession();

    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    keyboardSession->property_->SetKeyboardLayoutParams(params);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    callingSession->systemConfig_.freeMultiWindowEnable_ = true;
    callingSession->systemConfig_.freeMultiWindowSupport_ = true;
    keyboardSession->SetSessionState(SessionState::STATE_FOREGROUND);
    keyboardSession->isVisible_ = true;
    WSRect lastSafeRect = { 0, 0, 0, 0 };
    callingSession->SetLastSafeRect(lastSafeRect);
    WSRect rect1 = { 1, 1, 10, 10 };
    callingSession->SetSessionRect(rect1);
    WSRect rect2 = { 5, 5, 10, 10 };
    panelSession->SetSessionRect(rect2);
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, true);
    EXPECT_EQ(ret, true);
    lastSafeRect = { 1, 1, 1, 1 };
    callingSession->SetLastSafeRect(lastSafeRect);
    rect1 = { 1, 1, 10, 10 };
    callingSession->SetSessionRect(rect1);
    rect2 = { 5, 5, 10, 10 };
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, true);
    EXPECT_EQ(ret, true);
    lastSafeRect = { 0, 0, 0, 0 };
    callingSession->SetLastSafeRect(lastSafeRect);
    rect1 = { 1, 1, 1, 1 };
    callingSession->SetSessionRect(rect1);
    rect2 = { 10, 10, 1, 1 };
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo, true);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: OpenKeyboardSyncTransaction01
 * @tc.desc: OpenKeyboardSyncTransaction
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, OpenKeyboardSyncTransaction01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OpenKeyboardSyncTransaction01";
    info.bundleName_ = "OpenKeyboardSyncTransaction01";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->OpenKeyboardSyncTransaction();
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->OpenKeyboardSyncTransaction();
    WSRect keyboardPanelRect = {0, 0, 0, 0};
    WindowAnimationInfo animationInfo;
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo);
}

/**
 * @tc.name: Hide01
 * @tc.desc: test function: Hide
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, Hide01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "Hide01";
    info.bundleName_ = "Hide01";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    ASSERT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    ASSERT_NE(keyboardSession, nullptr);

    // setActive false return not ok
    keyboardSession->state_ = SessionState::STATE_DISCONNECT;
    EXPECT_EQ(WSError::WS_OK, keyboardSession->Hide());

    // setActive false return ok and deviceType is phone
    keyboardSession->state_ = SessionState::STATE_CONNECT;
    keyboardSession->isActive_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(keyboardSession->sessionStage_, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(WSError::WS_OK, keyboardSession->Hide());

    // deviceType is pc and property is not nullptr
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(WSError::WS_OK, keyboardSession->Hide());

    // Hide system keyboard
    ASSERT_EQ(false, keyboardSession->IsSystemKeyboard());
    keyboardSession->SetIsSystemKeyboard(true);
    ASSERT_EQ(true, keyboardSession->IsSystemKeyboard());
    ASSERT_EQ(WSError::WS_OK, keyboardSession->Hide());
}

/**
 * @tc.name: CloseKeyBoardSyncTransaction3
 * @tc.desc: test function: CloseKeyBoardSyncTransaction
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, CloseKeyBoardSyncTransaction3, TestSize.Level1)
{
    std::string abilityName = "CloseKeyBoardSyncTransaction3";
    std::string bundleName = "CloseKeyBoardSyncTransaction3";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, bundleName);
    ASSERT_NE(keyboardSession, nullptr);

    WSRect keyboardPanelRect;
    bool isKeyboardShow = true;
    WindowAnimationInfo animationInfo;

    keyboardSession->dirtyFlags_ = 0;
    keyboardSession->specificCallback_->onUpdateAvoidArea_ = [](uint32_t callingSessionId) {};
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    // isKeyBoardSyncTransactionOpen_ is true
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, isKeyboardShow, animationInfo);
    usleep(WAIT_ASYNC_US);
    ASSERT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);
}

/**
 * @tc.name: CloseKeyboardSyncTransaction4
 * @tc.desc: test function: CloseKeyboardSyncTransaction
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, CloseKeyboardSyncTransaction4, TestSize.Level1)
{
    std::string abilityName = "CloseKeyboardSyncTransaction4";
    std::string bundleName = "CloseKeyboardSyncTransaction4";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, bundleName);
    ASSERT_NE(keyboardSession, nullptr);
    WSRect keyboardPanelRect = { 0, 0, 0, 0 };
    WindowAnimationInfo animationInfo;
    keyboardSession->dirtyFlags_ = 0;

    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo);
    usleep(WAIT_ASYNC_US);
    ASSERT_EQ(0, keyboardSession->dirtyFlags_);
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->property_->SetCallingSessionId(1);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo);
    usleep(WAIT_ASYNC_US);
    auto callingSessionId = keyboardSession->property_->GetCallingSessionId();
    ASSERT_EQ(callingSessionId, INVALID_WINDOW_ID);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
