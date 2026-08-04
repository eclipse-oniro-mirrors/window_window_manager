/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

constexpr int WAIT_ASYNC_US = 1000000;
constexpr int WAIT_SYNC_IN_NS = 200000;

class KeyboardSessionTest7 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<KeyboardSession> GetKeyboardSession(const std::string& abilityName, const std::string& bundleName);
    sptr<SceneSession> GetSceneSession(const std::string& abilityName, const std::string& bundleName);
    void ConstructKeyboardCallingWindowTestData(sptr<SceneSession>& callingSession,
        sptr<KeyboardSession>& keyboardSession, sptr<SceneSession>& statusBarSession);
};

void KeyboardSessionTest7::SetUpTestCase() {}
void KeyboardSessionTest7::TearDownTestCase() {}
void KeyboardSessionTest7::SetUp() {}
void KeyboardSessionTest7::TearDown() {}

sptr<KeyboardSession> KeyboardSessionTest7::GetKeyboardSession(const std::string& abilityName,
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

sptr<SceneSession> KeyboardSessionTest7::GetSceneSession(const std::string& abilityName, const std::string& bundleName)
{
    SessionInfo info;
    info.abilityName_ = abilityName;
    info.bundleName_ = bundleName;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    return sceneSession;
}

void KeyboardSessionTest7::ConstructKeyboardCallingWindowTestData(sptr<SceneSession>& callingSession,
    sptr<KeyboardSession>& keyboardSession, sptr<SceneSession>& statusBarSession)
{
    SessionInfo info;
    info.abilityName_ = "KeyboardSessionTest7";
    info.bundleName_ = "KeyboardSessionTest7";
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

namespace {

HWTEST_F(KeyboardSessionTest7, KeyboardSessionConstructor01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor01";
    info.bundleName_ = "Constructor01";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
}

HWTEST_F(KeyboardSessionTest7, KeyboardSessionConstructor02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor02";
    info.bundleName_ = "Constructor02";
    info.persistentId_ = 100;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
}

HWTEST_F(KeyboardSessionTest7, KeyboardSessionConstructor03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor03";
    info.bundleName_ = "Constructor03";
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, keyboardCb);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, KeyboardSessionConstructor04, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor04";
    info.bundleName_ = "Constructor04";
    info.persistentId_ = 200;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, ShowKeyboard01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard01", "ShowKeyboard01");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = nullptr;
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

HWTEST_F(KeyboardSessionTest7, ShowKeyboard02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard02", "ShowKeyboard02");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, ShowKeyboard03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard03", "ShowKeyboard03");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetKeyboardEffectOption(KeyboardEffectOption());
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, ShowKeyboard04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard04", "ShowKeyboard04");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, ShowKeyboard05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard05", "ShowKeyboard05");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, ShowKeyboard06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard06", "ShowKeyboard06");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, HideKeyboard01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard01", "HideKeyboard01");
    keyboardSession->state_ = SessionState::STATE_DISCONNECT;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, HideKeyboard02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard02", "HideKeyboard02");
    keyboardSession->state_ = SessionState::STATE_CONNECT;
    keyboardSession->isActive_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, HideKeyboard03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard03", "HideKeyboard03");
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, HideKeyboard04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard04", "HideKeyboard04");
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, HideKeyboard05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard05", "HideKeyboard05");
    keyboardSession->SetIsSystemKeyboard(false);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, HideKeyboard06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard06", "HideKeyboard06");
    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, DisconnectKeyboard01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectKeyboard01";
    info.bundleName_ = "DisconnectKeyboard01";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    WSError result = keyboardSession->Disconnect(false);
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, DisconnectKeyboard02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectKeyboard02";
    info.bundleName_ = "DisconnectKeyboard02";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    WSError result = keyboardSession->Disconnect(true);
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, DisconnectKeyboard03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectKeyboard03";
    info.bundleName_ = "DisconnectKeyboard03";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    keyboardSession->SetIsSystemKeyboard(true);
    WSError result = keyboardSession->Disconnect(false);
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, DisconnectKeyboard04, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectKeyboard04";
    info.bundleName_ = "DisconnectKeyboard04";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    keyboardSession->SetIsSystemKeyboard(false);
    WSError result = keyboardSession->Disconnect(true);
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, DisconnectKeyboard05, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectKeyboard05";
    info.bundleName_ = "DisconnectKeyboard05";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    WSError result = keyboardSession->Disconnect(false);
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, BindKeyboardPanelSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("BindPanel01", "BindPanel01");
    sptr<SceneSession> panelSession = nullptr;
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest7, BindKeyboardPanelSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("BindPanel02", "BindPanel02");
    auto panelSession = GetSceneSession("Panel01", "Panel01");
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), panelSession);
}

HWTEST_F(KeyboardSessionTest7, BindKeyboardPanelSession03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("BindPanel03", "BindPanel03");
    auto panelSession1 = GetSceneSession("Panel01", "Panel01");
    auto panelSession2 = GetSceneSession("Panel02", "Panel02");
    keyboardSession->BindKeyboardPanelSession(panelSession1);
    keyboardSession->BindKeyboardPanelSession(panelSession2);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), panelSession2);
}

HWTEST_F(KeyboardSessionTest7, GetKeyboardPanelSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanel01", "GetPanel01");
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest7, GetKeyboardPanelSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanel02", "GetPanel02");
    auto panelSession = GetSceneSession("Panel", "Panel");
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_NE(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest7, GetKeyboardGravity01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Gravity01", "Gravity01");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    SessionGravity gravity = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(gravity, SessionGravity::SESSION_GRAVITY_BOTTOM);
}

HWTEST_F(KeyboardSessionTest7, GetKeyboardGravity02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Gravity02", "Gravity02");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    SessionGravity gravity = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(gravity, SessionGravity::SESSION_GRAVITY_FLOAT);
}

HWTEST_F(KeyboardSessionTest7, GetCallingSessionId01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling01", "GetCalling01");
    keyboardSession->property_->SetCallingSessionId(100);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, 100);
}

HWTEST_F(KeyboardSessionTest7, GetCallingSessionId02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling02", "GetCalling02");
    keyboardSession->property_->SetCallingSessionId(INVALID_WINDOW_ID);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, INVALID_WINDOW_ID);
}

HWTEST_F(KeyboardSessionTest7, GetCallingSessionId03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling03", "GetCalling03");
    keyboardSession->property_->SetCallingSessionId(UINT32_MAX);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, UINT32_MAX);
}

HWTEST_F(KeyboardSessionTest7, GetCallingSessionId04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling04", "GetCalling04");
    keyboardSession->property_ = nullptr;
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, INVALID_SESSION_ID);
}

HWTEST_F(KeyboardSessionTest7, NotifyClientToUpdateRect01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRect01", "NotifyRect01");
    WSRect rect = {800, 800, 1200, 1200};
    WSError result = keyboardSession->NotifyClientToUpdateRect("test", rect, nullptr);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, NotifyClientToUpdateRect02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRect02", "NotifyRect02");
    WSRect rect = {800, 800, 1200, 1200};
    std::shared_ptr<RSTransaction> rsTransaction = nullptr;
    WSError result = keyboardSession->NotifyClientToUpdateRect("update", rect, rsTransaction);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, NotifyClientToUpdateRect03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRect03", "NotifyRect03");
    WSRect rect = {800, 800, 1200, 1200};
    WSError result = keyboardSession->NotifyClientToUpdateRect("", rect, nullptr);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, IsVisibleForeground01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Visible01", "Visible01");
    keyboardSession->isVisible_ = true;
    EXPECT_EQ(keyboardSession->IsVisibleForeground(), true);
}

HWTEST_F(KeyboardSessionTest7, IsVisibleForeground02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Visible02", "Visible02");
    keyboardSession->isVisible_ = false;
    EXPECT_EQ(keyboardSession->IsVisibleForeground(), false);
}

HWTEST_F(KeyboardSessionTest7, IsVisibleNotBackground01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotBack01", "NotBack01");
    keyboardSession->isVisible_ = true;
    EXPECT_EQ(keyboardSession->IsVisibleNotBackground(), true);
}

HWTEST_F(KeyboardSessionTest7, IsVisibleNotBackground02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotBack02", "NotBack02");
    keyboardSession->isVisible_ = false;
    EXPECT_EQ(keyboardSession->IsVisibleNotBackground(), false);
}

HWTEST_F(KeyboardSessionTest7, GetIsKeyboardSyncTransactionOpen01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SyncOpen01", "SyncOpen01");
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    EXPECT_EQ(keyboardSession->GetIsKeyboardSyncTransactionOpen(), false);
}

HWTEST_F(KeyboardSessionTest7, GetIsKeyboardSyncTransactionOpen02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SyncOpen02", "SyncOpen02");
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    EXPECT_EQ(keyboardSession->GetIsKeyboardSyncTransactionOpen(), true);
}

HWTEST_F(KeyboardSessionTest7, GetIsKeyboardSyncTransactionOpen03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SyncOpen03", "SyncOpen03");
    EXPECT_EQ(keyboardSession->GetIsKeyboardSyncTransactionOpen(), false);
}

HWTEST_F(KeyboardSessionTest7, ChangeKeyboardEffectOption01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect01", "ChangeEffect01");
    KeyboardEffectOption effectOption;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, ChangeKeyboardEffectOption02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect02", "ChangeEffect02");
    KeyboardEffectOption effectOption(
        KeyboardViewMode::DARK_IMMERSIVE_MODE,
        KeyboardFlowLightMode::BACKGROUND_FLOW_LIGHT,
        KeyboardGradientMode::NONE,
        100);
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, ChangeKeyboardEffectOption03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect03", "ChangeEffect03");
    KeyboardEffectOption effectOption(
        KeyboardViewMode::LIGHT_IMMERSIVE_MODE,
        KeyboardFlowLightMode::BACKGROUND_FLOW_LIGHT,
        KeyboardGradientMode::LINEAR_GRADIENT,
        200);
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, ChangeKeyboardEffectOption04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect04", "ChangeEffect04");
    keyboardSession->changeKeyboardEffectOptionFunc_ = [](const KeyboardEffectOption& option) {};
    KeyboardEffectOption effectOption;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, ChangeKeyboardEffectOption05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect05", "ChangeEffect05");
    keyboardSession->changeKeyboardEffectOptionFunc_ = nullptr;
    KeyboardEffectOption effectOption;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, SetKeyboardEffectOptionChangeListener01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetListener01", "SetListener01");
    ASSERT_NE(keyboardSession, nullptr);
    NotifyKeyboarEffectOptionChangeFunc func = [](const KeyboardEffectOption& option) {};
    keyboardSession->SetKeyboardEffectOptionChangeListener(func);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->changeKeyboardEffectOptionFunc_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, SetKeyboardEffectOptionChangeListener02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetListener02", "SetListener02");
    ASSERT_NE(keyboardSession, nullptr);
    NotifyKeyboarEffectOptionChangeFunc func = nullptr;
    keyboardSession->SetKeyboardEffectOptionChangeListener(func);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->changeKeyboardEffectOptionFunc_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, SetSkipSelfWhenShowOnVirtualScreen01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Skip01", "Skip01");
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest7, SetSkipSelfWhenShowOnVirtualScreen02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Skip02", "Skip02");
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest7, SetSkipEventOnCastPlus01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipEvent01", "SkipEvent01");
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
    keyboardSession->SetSkipEventOnCastPlus(true);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest7, SetSkipEventOnCastPlus02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipEvent02", "SkipEvent02");
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
    keyboardSession->SetSkipEventOnCastPlus(false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest7, UpdateSizeChangeReason01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason01", "UpdateReason01");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, UpdateSizeChangeReason02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason02", "UpdateReason02");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_START);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, UpdateSizeChangeReason03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason03", "UpdateReason03");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_MOVE);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, UpdateSizeChangeReason04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason04", "UpdateReason04");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_END);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, UpdateSizeChangeReason05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason05", "UpdateReason05");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, GetPanelRect01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect01", "GetPanelRect01");
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.posX_, 0);
    EXPECT_EQ(panelRect.posY_, 0);
    EXPECT_EQ(panelRect.width_, 0);
    EXPECT_EQ(panelRect.height_, 0);
}

HWTEST_F(KeyboardSessionTest7, GetPanelRect02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect02", "GetPanelRect02");
    auto panelSession = GetSceneSession("Panel", "Panel");
    WSRect rect = {100, 200, 500, 300};
    panelSession->SetSessionRect(rect);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    usleep(WAIT_ASYNC_US);
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.posX_, 100);
    EXPECT_EQ(panelRect.posY_, 200);
}

HWTEST_F(KeyboardSessionTest7, OpenKeyboardSyncTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("OpenSync01", "OpenSync01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->OpenKeyboardSyncTransaction();
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest7, OpenKeyboardSyncTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("OpenSync02", "OpenSync02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->OpenKeyboardSyncTransaction();
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest7, CloseKeyboardSyncTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseSync01", "CloseSync01");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {0, 0, 0, 0};
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 1;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CloseKeyboardSyncTransaction(rect, true, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest7, CloseKeyboardSyncTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseSync02", "CloseSync02");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {0, 0, 0, 0};
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = INVALID_WINDOW_ID;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CloseKeyboardSyncTransaction(rect, false, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest7, CloseKeyboardSyncTransaction03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseSync03", "CloseSync03");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {100, 200, 500, 300};
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 5;
    animationInfo.isGravityChanged = true;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CloseKeyboardSyncTransaction(rect, true, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest7, EnableCallingSessionAvoidArea01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid01", "EnableAvoid01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->EnableCallingSessionAvoidArea();
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, EnableCallingSessionAvoidArea02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid02", "EnableAvoid02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "CallingSession";
        info.bundleName_ = "CallingSession";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        session->persistentId_ = persistentId;
        return session;
    };
    keyboardSession->property_->SetCallingSessionId(100);
    keyboardSession->EnableCallingSessionAvoidArea();
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->GetCallingSessionId(), 100);
}

HWTEST_F(KeyboardSessionTest7, NotifyKeyboardPanelInfoChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel01", "NotifyPanel01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = false;
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
    EXPECT_FALSE(keyboardSession->isKeyboardPanelEnabled_);
}

HWTEST_F(KeyboardSessionTest7, NotifyKeyboardPanelInfoChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel02", "NotifyPanel02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = nullptr;
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
    EXPECT_EQ(keyboardSession->sessionStage_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, NotifyKeyboardPanelInfoChange03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel03", "NotifyPanel03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
    EXPECT_NE(keyboardSession->sessionStage_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, CheckIfNeedRaiseCallingSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise01", "CheckRaise01");
    sptr<SceneSession> callingSession = nullptr;
    bool isFloating = true;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, isFloating);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest7, CheckIfNeedRaiseCallingSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise02", "CheckRaise02");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    auto callingSession = GetSceneSession("Calling", "Calling");
    bool isFloating = true;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, isFloating);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest7, CheckIfNeedRaiseCallingSession03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise03", "CheckRaise03");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    auto callingSession = GetSceneSession("Calling", "Calling");
    callingSession->isSubWindowResizingOrMoving_ = true;
    callingSession->GetSessionProperty()->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    bool isFloating = false;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, isFloating);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest7, CheckIfNeedRaiseCallingSession04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise04", "CheckRaise04");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    auto callingSession = GetSceneSession("Calling", "Calling");
    callingSession->GetSessionProperty()->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    callingSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    bool isFloating = true;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, isFloating);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest7, CheckIfNeedRaiseCallingSession05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise05", "CheckRaise05");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    auto callingSession = GetSceneSession("Calling", "Calling");
    callingSession->GetSessionProperty()->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    callingSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    bool isFloating = false;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, isFloating);
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest7, ProcessKeyboardOccupiedAreaInfo01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ProcessOccupied01", "ProcessOccupied01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(100, true, false);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, ProcessKeyboardOccupiedAreaInfo02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ProcessOccupied02", "ProcessOccupied02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "Calling";
        info.bundleName_ = "Calling";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        session->persistentId_ = persistentId;
        return session;
    };
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(100, false, false);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, RestoreCallingSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore01", "Restore01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = false;
    keyboardSession->RestoreCallingSession(100, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->keyboardAvoidAreaActive_);
}

HWTEST_F(KeyboardSessionTest7, RestoreCallingSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore02", "Restore02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->RestoreCallingSession(100, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->keyboardAvoidAreaActive_);
}

HWTEST_F(KeyboardSessionTest7, NotifySystemKeyboardAvoidChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid01", "NotifySysAvoid01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW);
    EXPECT_EQ(keyboardSession->systemConfig_.windowUIType_, WindowUIType::PHONE_WINDOW);
}

HWTEST_F(KeyboardSessionTest7, NotifySystemKeyboardAvoidChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid02", "NotifySysAvoid02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    keyboardSession->SetIsSystemKeyboard(false);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW);
    EXPECT_FALSE(keyboardSession->IsSystemKeyboard());
}

HWTEST_F(KeyboardSessionTest7, NotifyRootSceneOccupiedAreaChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRoot01", "NotifyRoot01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(100);
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    ASSERT_NE(info, nullptr);
    keyboardSession->NotifyRootSceneOccupiedAreaChange(info);
    EXPECT_NE(keyboardSession->property_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, AdjustKeyboardLayout01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Adjust01", "Adjust01");
    ASSERT_NE(keyboardSession, nullptr);
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    WSError result = keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, AdjustKeyboardLayout02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Adjust02", "Adjust02");
    ASSERT_NE(keyboardSession, nullptr);
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    WSError result = keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, AdjustKeyboardLayout03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Adjust03", "Adjust03");
    ASSERT_NE(keyboardSession, nullptr);
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    params.landscapeAvoidHeight_ = 100;
    params.portraitAvoidHeight_ = 150;
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    WSError result = keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest7, IsNeedRaiseSubWindow01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsNeedRaiseSub01", "IsNeedRaiseSub01");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->GetSessionProperty()->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    WSRect rect = {0, 0, 500, 500};
    bool result = keyboardSession->IsNeedRaiseSubWindow(callingSession, rect);
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest7, IsNeedRaiseSubWindow02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsNeedRaiseSub02", "IsNeedRaiseSub02");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->GetSessionProperty()->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    WSRect rect = {0, 0, 500, 500};
    bool result = keyboardSession->IsNeedRaiseSubWindow(callingSession, rect);
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest7, IsNeedRaiseSubWindow03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsNeedRaiseSub03", "IsNeedRaiseSub03");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->GetSessionProperty()->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    auto mainSession = GetSceneSession("Main", "Main");
    ASSERT_NE(mainSession, nullptr);
    mainSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    callingSession->parentSession_ = mainSession;
    WSRect rect = {0, 0, 500, 500};
    mainSession->SetSessionRect(rect);
    bool result = keyboardSession->IsNeedRaiseSubWindow(callingSession, rect);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest7, RaiseCallingSession01, TestSize.Level1)
{
    sptr<SceneSession> callingSession = nullptr;
    sptr<KeyboardSession> keyboardSession = nullptr;
    sptr<SceneSession> statusBarSession = nullptr;
    ConstructKeyboardCallingWindowTestData(callingSession, keyboardSession, statusBarSession);
    keyboardSession->keyboardAvoidAreaActive_ = false;
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    bool result = keyboardSession->RaiseCallingSession(callingSession, info);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest7, RaiseCallingSession02, TestSize.Level1)
{
    sptr<SceneSession> callingSession = nullptr;
    sptr<KeyboardSession> keyboardSession = nullptr;
    sptr<SceneSession> statusBarSession = nullptr;
    ConstructKeyboardCallingWindowTestData(callingSession, keyboardSession, statusBarSession);
    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->SetSessionState(SessionState::STATE_BACKGROUND);
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    bool result = keyboardSession->RaiseCallingSession(callingSession, info);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest7, GetRSTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetRSTrans01", "GetRSTrans01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    std::shared_ptr<RSTransaction> transaction = keyboardSession->GetRSTransaction();
    EXPECT_EQ(transaction, nullptr);
}

HWTEST_F(KeyboardSessionTest7, GetRSTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetRSTrans02", "GetRSTrans02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    std::shared_ptr<RSTransaction> transaction = keyboardSession->GetRSTransaction();
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest7, CloseRSTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTrans01", "CloseRSTrans01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->CloseRSTransaction();
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest7, CloseRSTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTrans02", "CloseRSTrans02");
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->CloseRSTransaction();
    EXPECT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);
}

HWTEST_F(KeyboardSessionTest7, GetSessionScreenName01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName01", "GetScreenName01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_ = nullptr;
    std::string screenName = keyboardSession->GetSessionScreenName();
    EXPECT_EQ(screenName, "");
}

HWTEST_F(KeyboardSessionTest7, GetSessionScreenName02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName02", "GetScreenName02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(DISPLAY_ID_INVALID);
    std::string screenName = keyboardSession->GetSessionScreenName();
    EXPECT_EQ(screenName, "");
}

HWTEST_F(KeyboardSessionTest7, SetCallingSessionId01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingId01", "SetCallingId01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return INVALID_WINDOW_ID; };
    keyboardSession->SetCallingSessionId(100);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, SetCallingSessionId02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingId02", "SetCallingId02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "Calling";
        info.bundleName_ = "Calling";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        session->persistentId_ = persistentId;
        return session;
    };
    keyboardSession->keyboardCallback_->onCallingSessionIdChange = nullptr;
    keyboardSession->SetCallingSessionId(200);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->keyboardCallback_->onCallingSessionIdChange, nullptr);
}

HWTEST_F(KeyboardSessionTest7, UseFocusIdIfCallingSessionIdInvalid01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UseFocus01", "UseFocus01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return INVALID_WINDOW_ID; };
    keyboardSession->property_->SetCallingSessionId(INVALID_WINDOW_ID);
    uint32_t callingSessionId = 100;
    keyboardSession->UseFocusIdIfCallingSessionIdInvalid(callingSessionId);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->GetCallingSessionId(), INVALID_WINDOW_ID);
}

HWTEST_F(KeyboardSessionTest7, UseFocusIdIfCallingSessionIdInvalid02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UseFocus02", "UseFocus02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) {
        SessionInfo info;
        info.abilityName_ = "Test";
        info.bundleName_ = "Test";
        return sptr<SceneSession>::MakeSptr(info, nullptr);
    };
    uint32_t callingSessionId = 100;
    keyboardSession->property_->SetCallingSessionId(callingSessionId);
    keyboardSession->UseFocusIdIfCallingSessionIdInvalid(callingSessionId);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->GetCallingSessionId(), 100);
}

HWTEST_F(KeyboardSessionTest7, HandleCrossScreenChild01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CrossScreen01", "CrossScreen01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->moveDragController_ = nullptr;
    keyboardSession->HandleCrossScreenChild(true);
    EXPECT_EQ(keyboardSession->moveDragController_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, HandleCrossScreenChild02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CrossScreen02", "CrossScreen02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->moveDragController_ = nullptr;
    keyboardSession->HandleCrossScreenChild(false);
    EXPECT_EQ(keyboardSession->moveDragController_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, PostKeyboardAnimationSyncTimeoutTask01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("PostTimeout01", "PostTimeout01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest7, PostKeyboardAnimationSyncTimeoutTask02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("PostTimeout02", "PostTimeout02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest7, GetFocusedSessionId01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused01", "GetFocused01");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = nullptr;
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, INVALID_WINDOW_ID);
}

HWTEST_F(KeyboardSessionTest7, GetFocusedSessionId02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused02", "GetFocused02");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return 100; };
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, 100);
}

HWTEST_F(KeyboardSessionTest7, GetFocusedSessionId03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused03", "GetFocused03");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return INVALID_WINDOW_ID; };
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, INVALID_WINDOW_ID);
}

HWTEST_F(KeyboardSessionTest7, NotifyOccupiedAreaChanged01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyOccupied01", "NotifyOccupied01");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->sessionInfo_.isSystem_ = true;
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, info, false, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(callingSession->sessionInfo_.isSystem_);
}

HWTEST_F(KeyboardSessionTest7, NotifyOccupiedAreaChanged02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyOccupied02", "NotifyOccupied02");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->sessionInfo_.isSystem_ = false;
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, info, true, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(callingSession->sessionInfo_.isSystem_);
}

HWTEST_F(KeyboardSessionTest7, GetSceneSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScene01", "GetScene01");
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    sptr<SceneSession> session = keyboardSession->GetSceneSession(100);
    EXPECT_EQ(session, nullptr);
}

HWTEST_F(KeyboardSessionTest7, GetSceneSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScene02", "GetScene02");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "Test";
        info.bundleName_ = "Test";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        session->persistentId_ = persistentId;
        return session;
    };
    sptr<SceneSession> session = keyboardSession->GetSceneSession(100);
    EXPECT_NE(session, nullptr);
}

HWTEST_F(KeyboardSessionTest7, GetSceneSession03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScene03", "GetScene03");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    sptr<SceneSession> session = keyboardSession->GetSceneSession(INVALID_WINDOW_ID);
    EXPECT_EQ(session, nullptr);
}

HWTEST_F(KeyboardSessionTest7, SetSurfaceBounds01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds01", "SetBounds01");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->SetSurfaceBounds(rect, false, false);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, SetSurfaceBounds02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds02", "SetBounds02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardPanelSession_ = nullptr;
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->SetSurfaceBounds(rect, true, true);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, SetSurfaceBounds03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds03", "SetBounds03");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {0, 0, 0, 0};
    keyboardSession->SetSurfaceBounds(rect, false, false);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

HWTEST_F(KeyboardSessionTest7, SetSurfaceBounds04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds04", "SetBounds04");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {500, 600, 800, 400};
    keyboardSession->SetSurfaceBounds(rect, true, false);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

} // namespace
} // namespace Rosen
} // namespace OHOS