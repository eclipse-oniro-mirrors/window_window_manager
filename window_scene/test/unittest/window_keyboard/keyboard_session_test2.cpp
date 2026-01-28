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
    g_logMsg += msg;
}
}

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
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();

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
    callingSession = sptr<SceneSession>::MakeSptr(info, specCallback);
    keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    keyboardSession->property_ = windowSessionProperty;
    SessionInfo info1;
    info1.abilityName_ = "BindKeyboardPanelSession";
    info1.bundleName_ = "BindKeyboardPanelSession";
    sptr<SceneSession> panelSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    sptr<SceneSession> getPanelSession = keyboardSession->GetKeyboardPanelSession();
    SessionInfo info2;
    info2.abilityName_ = "GetStatusBarHeight";
    info2.bundleName_ = "GetStatusBarHeight";
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    statusBarSession = sptr<SceneSession>::MakeSptr(info2, specificCallback_);
    WSRect rect({ 0, 0, 0, 10 });
    statusBarSession->GetLayoutController()->SetSessionRect(rect);
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

RSSurfaceNode::SharedPtr KeyboardSessionTest2::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "KeyboardSessionTest2SurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    if (surfaceNode == nullptr) {
        GTEST_LOG_(INFO) << "KeyboardSessionTest2::CreateRSSurfaceNode surfaceNode is nullptr";
    }
    return surfaceNode;
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
 * @tc.name: AdjustKeyboardLayout02
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
 * @tc.name: AdjustKeyboardLayout04
 * @tc.desc: AdjustKeyboardLayout test
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, AdjustKeyboardLayout04, Function | SmallTest | Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(KeyboardSessionTest2Callback);
    SessionInfo info;
    info.abilityName_ = "AdjustKeyboardLayout04";
    info.bundleName_ = "AdjustKeyboardLayout04";
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<SceneSession::SpecificSessionCallback> specificCb =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);

    //all conditions met, expect: need recalculate occupiedArea.
    KeyboardLayoutParams lastParams;
    lastParams.landscapeAvoidHeight_ = 300;
    lastParams.portraitAvoidHeight_ = 700;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetKeyboardLayoutParams(lastParams);
    keyboardSession->SetSessionProperty(property);
    KeyboardLayoutParams params;
    params.landscapeAvoidHeight_ = 200;
    params.portraitAvoidHeight_ = 600;

    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_TRUE(g_logMsg.find("Keyboard avoidHeight is set") != std::string::npos);

    //params.landscapeAvoidHeight_ < 0, expect: skip recalculate occupiedArea.
    g_logMsg.clear();
    params.landscapeAvoidHeight_ = -100;
    keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_TRUE(g_logMsg.find("Keyboard avoidHeight is set") == std::string::npos);

    //params.portraitAvoidHeight_ < 0, expect: skip recalculate occupiedArea.
    params.landscapeAvoidHeight_ = 200;
    params.portraitAvoidHeight_ = -100;
    keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_TRUE(g_logMsg.find("Keyboard avoidHeight is set") == std::string::npos);
    LOG_SetCallback(nullptr);

    params.landscapeAvoidHeight_ = 0;
    keyboardSession->state_ = SessionState::STATE_BACKGROUND;
    keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_TRUE(g_logMsg.find("Keyboard avoidHeight is set") == std::string::npos);

    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_TRUE(g_logMsg.find("Keyboard avoidHeight is set") == std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: AdjustKeyboardLayout05
 * @tc.desc: AdjustKeyboardLayout test
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, AdjustKeyboardLayout05, Function | SmallTest | Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(KeyboardSessionTest2Callback);
    SessionInfo info;
    info.abilityName_ = "AdjustKeyboardLayout05";
    info.bundleName_ = "AdjustKeyboardLayout05";
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<SceneSession::SpecificSessionCallback> specificCb =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);

    //params == lastParams, expect: skip recalculate occupiedArea.
    KeyboardLayoutParams lastParams;
    lastParams.landscapeAvoidHeight_ = 300;
    lastParams.portraitAvoidHeight_ = 700;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetKeyboardLayoutParams(lastParams);
    keyboardSession->SetSessionProperty(property);
    KeyboardLayoutParams params;
    params.landscapeAvoidHeight_ = 300;
    params.portraitAvoidHeight_ = 700;

    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_TRUE(g_logMsg.find("Keyboard avoidHeight is set") == std::string::npos);

    //lastParams.landscapeAvoidHeight_  < 0, expect: skip recalculate occupiedArea.
    lastParams.landscapeAvoidHeight_ = -300;
    lastParams.portraitAvoidHeight_ = 700;
    property->SetKeyboardLayoutParams(lastParams);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_TRUE(g_logMsg.find("Keyboard avoidHeight is set") == std::string::npos);

    //lastParams.portraitAvoidHeight_  < 0, expect: skip recalculate occupiedArea.
    lastParams.landscapeAvoidHeight_ = 300;
    lastParams.portraitAvoidHeight_ = -700;
    property->SetKeyboardLayoutParams(lastParams);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_TRUE(g_logMsg.find("Keyboard avoidHeight is set") == std::string::npos);
    LOG_SetCallback(nullptr);
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
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    keyboardSession->SetSessionProperty(property);
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    keyboardSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    callingSession->systemConfig_.freeMultiWindowSupport_ = true;
    callingSession->systemConfig_.freeMultiWindowEnable_ = true;
    auto ret = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, true);
    EXPECT_EQ(ret, true);
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
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, false);

    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->SetSessionState(SessionState::STATE_BACKGROUND);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, false);

    callingSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
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
    callingSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

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
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, true);

    lastSafeRect = { 0, 0, 0, 0 };
    callingSession->SetLastSafeRect(lastSafeRect);
    rect1 = { 1, 1, 100, 100 };
    callingSession->SetSessionRect(rect1);
    rect2 = { 200, 200, 100, 100 };
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
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
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, true);
    callingSession->SetIsMidScene(false);
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    callingSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    callingSession->SetOriPosYBeforeRaisedByKeyboard(10);
    lastSafeRect = { 50, 50, 51, 51 };
    callingSession->SetLastSafeRect(lastSafeRect);
    callingSession->SetSessionRect(rect1);
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, false);
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    callingSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    callingSession->SetOriPosYBeforeRaisedByKeyboard(10);
    lastSafeRect = { 1, 50, 100, 100 };
    callingSession->SetLastSafeRect(lastSafeRect);
    callingSession->SetSessionRect(rect1);
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
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
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, true);
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    lastSafeRect = { 1, statusHeight-1, 10, 9 };
    callingSession->SetLastSafeRect(lastSafeRect);
    rect1 = { 1, statusHeight-1, 10, 10 };
    callingSession->SetSessionRect(rect1);
    rect2 = { 1, statusHeight-2, 10, 10 };
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, false);
    callingSession->SetLastSafeRect(lastSafeRect);
    rect1 = { 1, statusHeight, 100, 100 };
    callingSession->SetSessionRect(rect1);
    rect2 = { 50, statusHeight-1, 100, 100 };
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
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
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, true);
    lastSafeRect = { 1, 1, 1, 1 };
    callingSession->SetLastSafeRect(lastSafeRect);
    rect1 = { 1, 1, 10, 10 };
    callingSession->SetSessionRect(rect1);
    rect2 = { 5, 5, 10, 10 };
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, true);
    lastSafeRect = { 0, 0, 0, 0 };
    callingSession->SetLastSafeRect(lastSafeRect);
    rect1 = { 1, 1, 1, 1 };
    callingSession->SetSessionRect(rect1);
    rect2 = { 10, 10, 1, 1 };
    panelSession->SetSessionRect(rect2);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: RaiseCallingSession06
 * @tc.desc: RaiseCallingSession06
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, RaiseCallingSession06, TestSize.Level1)
{
    sptr<SceneSession> callingSession = nullptr;
    sptr<KeyboardSession> keyboardSession = nullptr;
    sptr<SceneSession> statusBarSession = nullptr;
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;
    ConstructKeyboardCallingWindowTestData(callingSession, keyboardSession, statusBarSession);
    sptr<SceneSession> panelSession = keyboardSession->GetKeyboardPanelSession();

    callingSession->property_->SetDisplayId(999);
    keyboardSession->SetScreenId(0);
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: RaiseCallingSession07
 * @tc.desc: RaiseCallingSession07
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, RaiseCallingSession07, TestSize.Level1)
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
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, false);

    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    keyboardSession->property_->SetKeyboardLayoutParams(params);
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    callingSession->SetIsMidScene(false);
    callingSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    callingSession->systemConfig_.freeMultiWindowEnable_ = false;
    callingSession->systemConfig_.freeMultiWindowSupport_ = false;
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: RaiseCallingSession08
 * @tc.desc: RaiseCallingSession08
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, RaiseCallingSession08, TestSize.Level1)
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
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: RaiseCallingSession09
 * @tc.desc: RaiseCallingSession09
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, RaiseCallingSession09, TestSize.Level1)
{
    sptr<SceneSession> callingSession = nullptr;
    sptr<KeyboardSession> keyboardSession = nullptr;
    sptr<SceneSession> statusBarSession = nullptr;
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;
    ConstructKeyboardCallingWindowTestData(callingSession, keyboardSession, statusBarSession);
    sptr<SceneSession> panelSession = keyboardSession->GetKeyboardPanelSession();
    keyboardSession->property_->SetIsSystemKeyboard(false);
    keyboardSession->SetSessionState(SessionState::STATE_FOREGROUND);

    callingSession->layoutController_->SetScale(1.0, 1.0, 0.5, 0.5);
    callingSession->layoutController_->SetSessionRect({500, 0, 2000, 2000});
    callingSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    callingSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);

    callingSession->SetLastSafeRect({0, 0, 0, 0});
    panelSession->layoutController_->SetSessionRect({0, 1990, 3000, 1000});
    auto ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, true);
    ASSERT_NE(nullptr, occupiedAreaInfo);
    EXPECT_EQ(10, occupiedAreaInfo->safeHeight_);

    callingSession->SetLastSafeRect({0, 0, 0, 0});
    callingSession->layoutController_->SetSessionRect({500, 30, 2000, 2000});
    panelSession->layoutController_->SetSessionRect({0, 1980, 3000, 1000});
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, true);
    ASSERT_NE(nullptr, occupiedAreaInfo);
    EXPECT_EQ(20, occupiedAreaInfo->safeHeight_);

    callingSession->layoutController_->SetScale(1.5, 1.5, 0.5, 0.5);
    callingSession->layoutController_->SetSessionRect({500, 500, 2000, 2000});
    panelSession->layoutController_->SetSessionRect({0, 3000, 4000, 1000});
    callingSession->SetOriPosYBeforeRaisedByKeyboard(0);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, true);
    ASSERT_NE(nullptr, occupiedAreaInfo);
    EXPECT_EQ(0, occupiedAreaInfo->safeHeight_);

    callingSession->layoutController_->SetSessionRect({500, 550, 2000, 2000});
    panelSession->layoutController_->SetSessionRect({0, 2990, 4000, 1000});
    callingSession->SetOriPosYBeforeRaisedByKeyboard(0);
    ret = keyboardSession->RaiseCallingSession(callingSession, occupiedAreaInfo);
    EXPECT_EQ(ret, true);
    ASSERT_NE(nullptr, occupiedAreaInfo);
    EXPECT_EQ(60, occupiedAreaInfo->safeHeight_);
}

/**
 * @tc.name: isNeedProcessKeyboardOccupiedAreaInfo
 * @tc.desc: isNeedProcessKeyboardOccupiedAreaInfo test
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest2, isNeedProcessKeyboardOccupiedAreaInfo, Function | SmallTest | Level1)
{
    SessionInfo info;
    info.abilityName_ = "isNeedProcessKeyboardOccupiedAreaInfo";
    info.bundleName_ = "isNeedProcessKeyboardOccupiedAreaInfo";
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<SceneSession::SpecificSessionCallback> specificCb =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);
    KeyboardLayoutParams params;
    KeyboardLayoutParams lastParams = params;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    EXPECT_FALSE(keyboardSession->isNeedProcessKeyboardOccupiedAreaInfo(params, lastParams));
}
} // namespace
} // namespace Rosen
} // namespace OHOS
