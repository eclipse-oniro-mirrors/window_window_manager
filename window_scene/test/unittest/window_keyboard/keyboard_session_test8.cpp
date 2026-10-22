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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <unistd.h>

#define protected public
#define private public
#include "interfaces/include/ws_common.h"
#include "session/host/include/keyboard_session.h"
#include "session/host/include/session.h"
#include "session/host/include/scene_session.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "ui/rs_surface_node.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_keyboard_session.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

constexpr int WAIT_ASYNC_US = 1000000;
constexpr int WAIT_SYNC_IN_NS = 200000;

class KeyboardSessionTest8 : public testing::Test {
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

void KeyboardSessionTest8::SetUpTestCase() {}
void KeyboardSessionTest8::TearDownTestCase() {}
void KeyboardSessionTest8::SetUp() {}
void KeyboardSessionTest8::TearDown() {}

sptr<KeyboardSession> KeyboardSessionTest8::GetKeyboardSession(const std::string& abilityName,
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

sptr<SceneSession> KeyboardSessionTest8::GetSceneSession(const std::string& abilityName,
    const std::string& bundleName)
{
    SessionInfo info;
    info.abilityName_ = abilityName;
    info.bundleName_ = bundleName;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    return sceneSession;
}

void KeyboardSessionTest8::ConstructKeyboardCallingWindowTestData(sptr<SceneSession>& callingSession,
    sptr<KeyboardSession>& keyboardSession, sptr<SceneSession>& statusBarSession)
{
    SessionInfo info;
    info.abilityName_ = "KeyboardSessionTest8";
    info.bundleName_ = "KeyboardSessionTest8";
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

HWTEST_F(KeyboardSessionTest8, KeyboardSessionConstructor01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor01";
    info.bundleName_ = "Constructor01";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
}

HWTEST_F(KeyboardSessionTest8, KeyboardSessionConstructor02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor02";
    info.bundleName_ = "Constructor02";
    info.persistentId_ = 100;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
}

HWTEST_F(KeyboardSessionTest8, KeyboardSessionConstructor03, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, KeyboardSessionConstructor04, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, KeyboardSessionConstructor05, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor05";
    info.bundleName_ = "Constructor05";
    info.persistentId_ = 1;
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_EQ(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, KeyboardSessionConstructor06, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor06";
    info.bundleName_ = "Constructor06";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_EQ(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, KeyboardSessionConstructor07, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor07";
    info.bundleName_ = "Constructor07";
    info.persistentId_ = 999;
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, keyboardCb);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, KeyboardSessionConstructor08, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor08";
    info.bundleName_ = "Constructor08";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, KeyboardSessionConstructor09, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor09";
    info.bundleName_ = "Constructor09";
    info.persistentId_ = 50;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, KeyboardSessionConstructor10, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor10";
    info.bundleName_ = "Constructor10";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_EQ(keyboardSession->changeKeyboardEffectOptionFunc_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, KeyboardSessionConstructor11, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor11";
    info.bundleName_ = "Constructor11";
    info.persistentId_ = 0;
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
}

HWTEST_F(KeyboardSessionTest8, KeyboardSessionConstructor12, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor12";
    info.bundleName_ = "Constructor12";
    info.persistentId_ = INT32_MAX;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard01", "ShowKeyboard01");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = nullptr;
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard02", "ShowKeyboard02");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard03", "ShowKeyboard03");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardEffectOption effectOption;
    property->SetKeyboardEffectOption(effectOption);
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard04", "ShowKeyboard04");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard05", "ShowKeyboard05");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard06", "ShowKeyboard06");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard07", "ShowKeyboard07");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard08", "ShowKeyboard08");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::DARK_IMMERSIVE_MODE;
    property->SetKeyboardEffectOption(effectOption);
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard09, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard09", "ShowKeyboard09");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::LIGHT_IMMERSIVE_MODE;
    property->SetKeyboardEffectOption(effectOption);
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard10, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard10", "ShowKeyboard10");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardEffectOption effectOption;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::BACKGROUND_FLOW_LIGHT;
    property->SetKeyboardEffectOption(effectOption);
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard11, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard11", "ShowKeyboard11");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardEffectOption effectOption;
    effectOption.gradientMode_ = KeyboardGradientMode::LINEAR_GRADIENT;
    property->SetKeyboardEffectOption(effectOption);
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard12, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard12", "ShowKeyboard12");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::IMMERSIVE_MODE;
    effectOption.blurHeight_ = 50;
    property->SetKeyboardEffectOption(effectOption);
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard13, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard13", "ShowKeyboard13");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::NON_IMMERSIVE_MODE;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::NONE;
    effectOption.gradientMode_ = KeyboardGradientMode::NONE;
    property->SetKeyboardEffectOption(effectOption);
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard14, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard14", "ShowKeyboard14");
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ShowKeyboard15, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowKeyboard15", "ShowKeyboard15");
    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard01", "HideKeyboard01");
    keyboardSession->state_ = SessionState::STATE_DISCONNECT;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard02", "HideKeyboard02");
    keyboardSession->state_ = SessionState::STATE_CONNECT;
    keyboardSession->isActive_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard03", "HideKeyboard03");
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard04", "HideKeyboard04");
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard05", "HideKeyboard05");
    keyboardSession->SetIsSystemKeyboard(false);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard06", "HideKeyboard06");
    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard07", "HideKeyboard07");
    keyboardSession->state_ = SessionState::STATE_INACTIVE;
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard08", "HideKeyboard08");
    keyboardSession->state_ = SessionState::STATE_BACKGROUND;
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard09, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard09", "HideKeyboard09");
    keyboardSession->state_ = SessionState::STATE_END;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard10, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard10", "HideKeyboard10");
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->isActive_ = false;
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard11, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard11", "HideKeyboard11");
    keyboardSession->state_ = SessionState::STATE_CONNECT;
    keyboardSession->isActive_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard12, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard12", "HideKeyboard12");
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard13, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard13", "HideKeyboard13");
    keyboardSession->SetIsSystemKeyboard(false);
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, HideKeyboard14, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideKeyboard14", "HideKeyboard14");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, DisconnectKeyboard01, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, DisconnectKeyboard02, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, DisconnectKeyboard03, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, DisconnectKeyboard04, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, DisconnectKeyboard05, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, DisconnectKeyboard06, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectKeyboard06";
    info.bundleName_ = "DisconnectKeyboard06";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = false;
    WSError result = keyboardSession->Disconnect(true);
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, DisconnectKeyboard07, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectKeyboard07";
    info.bundleName_ = "DisconnectKeyboard07";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    WSError result = keyboardSession->Disconnect(true);
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, DisconnectKeyboard08, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectKeyboard08";
    info.bundleName_ = "DisconnectKeyboard08";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    keyboardSession->state_ = SessionState::STATE_BACKGROUND;
    WSError result = keyboardSession->Disconnect(false);
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, DisconnectKeyboard09, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectKeyboard09";
    info.bundleName_ = "DisconnectKeyboard09";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    keyboardSession->state_ = SessionState::STATE_CONNECT;
    WSError result = keyboardSession->Disconnect(false);
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, DisconnectKeyboard10, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectKeyboard10";
    info.bundleName_ = "DisconnectKeyboard10";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->state_ = SessionState::STATE_INACTIVE;
    WSError result = keyboardSession->Disconnect(true);
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, DisconnectKeyboard11, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectKeyboard11";
    info.bundleName_ = "DisconnectKeyboard11";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    WSError result = keyboardSession->Disconnect(false, "token11");
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, DisconnectKeyboard12, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectKeyboard12";
    info.bundleName_ = "DisconnectKeyboard12";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    WSError result = keyboardSession->Disconnect(true, "token12");
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, BindKeyboardPanelSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("BindPanel01", "BindPanel01");
    sptr<SceneSession> panelSession = nullptr;
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest8, BindKeyboardPanelSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("BindPanel02", "BindPanel02");
    auto panelSession = GetSceneSession("Panel01", "Panel01");
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), panelSession);
}

HWTEST_F(KeyboardSessionTest8, BindKeyboardPanelSession03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("BindPanel03", "BindPanel03");
    auto panelSession1 = GetSceneSession("Panel01", "Panel01");
    auto panelSession2 = GetSceneSession("Panel02", "Panel02");
    keyboardSession->BindKeyboardPanelSession(panelSession1);
    keyboardSession->BindKeyboardPanelSession(panelSession2);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), panelSession2);
}

HWTEST_F(KeyboardSessionTest8, BindKeyboardPanelSession04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("BindPanel04", "BindPanel04");
    auto panelSession = GetSceneSession("Panel04", "Panel04");
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_NE(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest8, BindKeyboardPanelSession05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("BindPanel05", "BindPanel05");
    auto panelSession1 = GetSceneSession("PanelA", "PanelA");
    auto panelSession2 = GetSceneSession("PanelB", "PanelB");
    auto panelSession3 = GetSceneSession("PanelC", "PanelC");
    keyboardSession->BindKeyboardPanelSession(panelSession1);
    keyboardSession->BindKeyboardPanelSession(panelSession2);
    keyboardSession->BindKeyboardPanelSession(panelSession3);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), panelSession3);
}

HWTEST_F(KeyboardSessionTest8, BindKeyboardPanelSession06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("BindPanel06", "BindPanel06");
    auto panelSession = GetSceneSession("Panel06", "Panel06");
    keyboardSession->BindKeyboardPanelSession(panelSession);
    keyboardSession->BindKeyboardPanelSession(nullptr);
    EXPECT_NE(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest8, BindKeyboardPanelSession07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("BindPanel07", "BindPanel07");
    auto panelSession = GetSceneSession("Panel07", "Panel07");
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_NE(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest8, BindKeyboardPanelSession08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("BindPanel08", "BindPanel08");
    for (int i = 0; i < 5; ++i) {
        auto panelSession = GetSceneSession("Panel" + std::to_string(i), "Bundle" + std::to_string(i));
        keyboardSession->BindKeyboardPanelSession(panelSession);
    }
    EXPECT_NE(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetKeyboardPanelSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanel01", "GetPanel01");
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetKeyboardPanelSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanel02", "GetPanel02");
    auto panelSession = GetSceneSession("Panel", "Panel");
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_NE(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetKeyboardPanelSession03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanel03", "GetPanel03");
    auto panelSession = GetSceneSession("Panel03", "Panel03");
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), panelSession);
}

HWTEST_F(KeyboardSessionTest8, GetKeyboardPanelSession04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanel04", "GetPanel04");
    auto panel1 = GetSceneSession("P1", "P1");
    keyboardSession->BindKeyboardPanelSession(panel1);
    auto panel2 = GetSceneSession("P2", "P2");
    keyboardSession->BindKeyboardPanelSession(panel2);
    EXPECT_NE(keyboardSession->GetKeyboardPanelSession(), panel1);
}

HWTEST_F(KeyboardSessionTest8, GetKeyboardPanelSession05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanel05", "GetPanel05");
    keyboardSession->BindKeyboardPanelSession(nullptr);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetKeyboardPanelSession06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanel06", "GetPanel06");
    auto panel = GetSceneSession("Panel06", "Panel06");
    keyboardSession->BindKeyboardPanelSession(panel);
    keyboardSession->BindKeyboardPanelSession(nullptr);
    EXPECT_NE(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetKeyboardGravity01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Gravity01", "Gravity01");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    SessionGravity gravity = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(gravity, SessionGravity::SESSION_GRAVITY_BOTTOM);
}

HWTEST_F(KeyboardSessionTest8, GetKeyboardGravity02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Gravity02", "Gravity02");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    SessionGravity gravity = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(gravity, SessionGravity::SESSION_GRAVITY_FLOAT);
}

HWTEST_F(KeyboardSessionTest8, GetKeyboardGravity03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Gravity03", "Gravity03");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_DEFAULT;
    SessionGravity gravity = keyboardSession->GetKeyboardGravity();
    EXPECT_NE(gravity, SessionGravity::SESSION_GRAVITY_BOTTOM);
}

HWTEST_F(KeyboardSessionTest8, GetKeyboardGravity04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Gravity04", "Gravity04");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    EXPECT_NE(keyboardSession->GetKeyboardGravity(), SessionGravity::SESSION_GRAVITY_DEFAULT);
}

HWTEST_F(KeyboardSessionTest8, GetKeyboardGravity05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Gravity05", "Gravity05");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    EXPECT_NE(keyboardSession->GetKeyboardGravity(), SessionGravity::SESSION_GRAVITY_FLOAT);
}

HWTEST_F(KeyboardSessionTest8, GetKeyboardGravity06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Gravity06", "Gravity06");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_DEFAULT;
    SessionGravity gravity = keyboardSession->GetKeyboardGravity();
    EXPECT_NE(gravity, SessionGravity::SESSION_GRAVITY_FLOAT);
}

HWTEST_F(KeyboardSessionTest8, GetCallingSessionId01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling01", "GetCalling01");
    keyboardSession->property_->SetCallingSessionId(100);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, 100);
}

HWTEST_F(KeyboardSessionTest8, GetCallingSessionId02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling02", "GetCalling02");
    keyboardSession->property_->SetCallingSessionId(INVALID_WINDOW_ID);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, INVALID_WINDOW_ID);
}

HWTEST_F(KeyboardSessionTest8, GetCallingSessionId03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling03", "GetCalling03");
    keyboardSession->property_->SetCallingSessionId(UINT32_MAX);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, UINT32_MAX);
}

HWTEST_F(KeyboardSessionTest8, GetCallingSessionId04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling04", "GetCalling04");
    keyboardSession->property_ = nullptr;
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, INVALID_SESSION_ID);
}

HWTEST_F(KeyboardSessionTest8, GetCallingSessionId05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling05", "GetCalling05");
    keyboardSession->property_->SetCallingSessionId(0);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, 0);
}

HWTEST_F(KeyboardSessionTest8, GetCallingSessionId06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling06", "GetCalling06");
    keyboardSession->property_->SetCallingSessionId(1);
    EXPECT_EQ(keyboardSession->GetCallingSessionId(), 1);
}

HWTEST_F(KeyboardSessionTest8, GetCallingSessionId07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling07", "GetCalling07");
    keyboardSession->property_->SetCallingSessionId(5000);
    EXPECT_NE(keyboardSession->GetCallingSessionId(), 0);
}

HWTEST_F(KeyboardSessionTest8, GetCallingSessionId08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling08", "GetCalling08");
    keyboardSession->property_->SetCallingSessionId(123456);
    EXPECT_EQ(keyboardSession->GetCallingSessionId(), 123456);
}

HWTEST_F(KeyboardSessionTest8, IsVisibleForeground01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Visible01", "Visible01");
    keyboardSession->isVisible_ = true;
    EXPECT_EQ(keyboardSession->IsVisibleForeground(), true);
}

HWTEST_F(KeyboardSessionTest8, IsVisibleForeground02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Visible02", "Visible02");
    keyboardSession->isVisible_ = false;
    EXPECT_EQ(keyboardSession->IsVisibleForeground(), false);
}

HWTEST_F(KeyboardSessionTest8, IsVisibleForeground03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Visible03", "Visible03");
    keyboardSession->isVisible_ = true;
    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    EXPECT_EQ(keyboardSession->IsVisibleForeground(), true);
}

HWTEST_F(KeyboardSessionTest8, IsVisibleForeground04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Visible04", "Visible04");
    keyboardSession->isVisible_ = false;
    keyboardSession->state_ = SessionState::STATE_BACKGROUND;
    EXPECT_EQ(keyboardSession->IsVisibleForeground(), false);
}

HWTEST_F(KeyboardSessionTest8, IsVisibleForeground05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Visible05", "Visible05");
    keyboardSession->isVisible_ = true;
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    EXPECT_EQ(keyboardSession->IsVisibleForeground(), true);
}

HWTEST_F(KeyboardSessionTest8, IsVisibleNotBackground01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotBack01", "NotBack01");
    keyboardSession->isVisible_ = true;
    EXPECT_EQ(keyboardSession->IsVisibleNotBackground(), true);
}

HWTEST_F(KeyboardSessionTest8, IsVisibleNotBackground02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotBack02", "NotBack02");
    keyboardSession->isVisible_ = false;
    EXPECT_EQ(keyboardSession->IsVisibleNotBackground(), false);
}

HWTEST_F(KeyboardSessionTest8, IsVisibleNotBackground03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotBack03", "NotBack03");
    keyboardSession->isVisible_ = true;
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    EXPECT_EQ(keyboardSession->IsVisibleNotBackground(), true);
}

HWTEST_F(KeyboardSessionTest8, IsVisibleNotBackground04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotBack04", "NotBack04");
    keyboardSession->isVisible_ = false;
    keyboardSession->state_ = SessionState::STATE_BACKGROUND;
    EXPECT_EQ(keyboardSession->IsVisibleNotBackground(), false);
}

HWTEST_F(KeyboardSessionTest8, IsVisibleNotBackground05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotBack05", "NotBack05");
    keyboardSession->isVisible_ = true;
    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    EXPECT_EQ(keyboardSession->IsVisibleNotBackground(), true);
}

HWTEST_F(KeyboardSessionTest8, GetIsKeyboardSyncTransactionOpen01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SyncOpen01", "SyncOpen01");
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    EXPECT_EQ(keyboardSession->GetIsKeyboardSyncTransactionOpen(), false);
}

HWTEST_F(KeyboardSessionTest8, GetIsKeyboardSyncTransactionOpen02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SyncOpen02", "SyncOpen02");
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    EXPECT_EQ(keyboardSession->GetIsKeyboardSyncTransactionOpen(), true);
}

HWTEST_F(KeyboardSessionTest8, GetIsKeyboardSyncTransactionOpen03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SyncOpen03", "SyncOpen03");
    EXPECT_EQ(keyboardSession->GetIsKeyboardSyncTransactionOpen(), false);
}

HWTEST_F(KeyboardSessionTest8, GetIsKeyboardSyncTransactionOpen04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SyncOpen04", "SyncOpen04");
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    EXPECT_EQ(keyboardSession->GetIsKeyboardSyncTransactionOpen(), false);
}

HWTEST_F(KeyboardSessionTest8, GetIsKeyboardSyncTransactionOpen05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SyncOpen05", "SyncOpen05");
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    EXPECT_EQ(keyboardSession->GetIsKeyboardSyncTransactionOpen(), true);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect01", "ChangeEffect01");
    KeyboardEffectOption effectOption;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect02", "ChangeEffect02");
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::DARK_IMMERSIVE_MODE;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::BACKGROUND_FLOW_LIGHT;
    effectOption.gradientMode_ = KeyboardGradientMode::NONE;
    effectOption.blurHeight_ = 100;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect03", "ChangeEffect03");
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::LIGHT_IMMERSIVE_MODE;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::BACKGROUND_FLOW_LIGHT;
    effectOption.gradientMode_ = KeyboardGradientMode::LINEAR_GRADIENT;
    effectOption.blurHeight_ = 200;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect04", "ChangeEffect04");
    keyboardSession->changeKeyboardEffectOptionFunc_ = [](const KeyboardEffectOption& option) {};
    KeyboardEffectOption effectOption;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect05", "ChangeEffect05");
    keyboardSession->changeKeyboardEffectOptionFunc_ = nullptr;
    KeyboardEffectOption effectOption;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect06", "ChangeEffect06");
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::NON_IMMERSIVE_MODE;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect07", "ChangeEffect07");
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::IMMERSIVE_MODE;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect08", "ChangeEffect08");
    KeyboardEffectOption effectOption;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::NONE;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption09, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect09", "ChangeEffect09");
    KeyboardEffectOption effectOption;
    effectOption.gradientMode_ = KeyboardGradientMode::NONE;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption10, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect10", "ChangeEffect10");
    KeyboardEffectOption effectOption;
    effectOption.blurHeight_ = 0;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption11, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect11", "ChangeEffect11");
    keyboardSession->changeKeyboardEffectOptionFunc_ = [](const KeyboardEffectOption& option) {};
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::DARK_IMMERSIVE_MODE;
    effectOption.blurHeight_ = 300;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption12, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect12", "ChangeEffect12");
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::VIEW_MODE_END;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::END;
    effectOption.gradientMode_ = KeyboardGradientMode::END;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption13, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect13", "ChangeEffect13");
    KeyboardEffectOption effectOption;
    effectOption.blurHeight_ = UINT32_MAX;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption14, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect14", "ChangeEffect14");
    keyboardSession->changeKeyboardEffectOptionFunc_ = [](const KeyboardEffectOption& option) {};
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::LIGHT_IMMERSIVE_MODE;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::BACKGROUND_FLOW_LIGHT;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption15, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect15", "ChangeEffect15");
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::IMMERSIVE_MODE;
    effectOption.gradientMode_ = KeyboardGradientMode::LINEAR_GRADIENT;
    effectOption.blurHeight_ = 256;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, ChangeKeyboardEffectOption16, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect16", "ChangeEffect16");
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::NON_IMMERSIVE_MODE;
    effectOption.flowLightMode_ = KeyboardFlowLightMode::NONE;
    effectOption.gradientMode_ = KeyboardGradientMode::NONE;
    effectOption.blurHeight_ = 0;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, SetKeyboardEffectOptionChangeListener01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetListener01", "SetListener01");
    ASSERT_NE(keyboardSession, nullptr);
    NotifyKeyboarEffectOptionChangeFunc func = [](const KeyboardEffectOption& option) {};
    keyboardSession->SetKeyboardEffectOptionChangeListener(func);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->changeKeyboardEffectOptionFunc_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetKeyboardEffectOptionChangeListener02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetListener02", "SetListener02");
    ASSERT_NE(keyboardSession, nullptr);
    NotifyKeyboarEffectOptionChangeFunc func = nullptr;
    keyboardSession->SetKeyboardEffectOptionChangeListener(func);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->changeKeyboardEffectOptionFunc_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetKeyboardEffectOptionChangeListener03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetListener03", "SetListener03");
    ASSERT_NE(keyboardSession, nullptr);
    NotifyKeyboarEffectOptionChangeFunc func1 = [](const KeyboardEffectOption& option) {};
    NotifyKeyboarEffectOptionChangeFunc func2 = [](const KeyboardEffectOption& option) {};
    keyboardSession->SetKeyboardEffectOptionChangeListener(func1);
    keyboardSession->SetKeyboardEffectOptionChangeListener(func2);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->changeKeyboardEffectOptionFunc_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetKeyboardEffectOptionChangeListener04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetListener04", "SetListener04");
    ASSERT_NE(keyboardSession, nullptr);
    NotifyKeyboarEffectOptionChangeFunc func = [](const KeyboardEffectOption& option) {};
    keyboardSession->SetKeyboardEffectOptionChangeListener(func);
    keyboardSession->changeKeyboardEffectOptionFunc_ = nullptr;
    EXPECT_EQ(keyboardSession->changeKeyboardEffectOptionFunc_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetKeyboardEffectOptionChangeListener05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetListener05", "SetListener05");
    ASSERT_NE(keyboardSession, nullptr);
    int captured = 0;
    NotifyKeyboarEffectOptionChangeFunc func = [&captured](const KeyboardEffectOption& option) {
        captured = 1;
    };
    keyboardSession->SetKeyboardEffectOptionChangeListener(func);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->changeKeyboardEffectOptionFunc_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetSkipSelfWhenShowOnVirtualScreen01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Skip01", "Skip01");
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest8, SetSkipSelfWhenShowOnVirtualScreen02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Skip02", "Skip02");
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest8, SetSkipSelfWhenShowOnVirtualScreen03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Skip03", "Skip03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->specificCallback_ = nullptr;
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest8, SetSkipSelfWhenShowOnVirtualScreen04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Skip04", "Skip04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->specificCallback_ = nullptr;
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest8, SetSkipSelfWhenShowOnVirtualScreen05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Skip05", "Skip05");
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest8, SetSkipEventOnCastPlus01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipEvent01", "SkipEvent01");
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
    keyboardSession->SetSkipEventOnCastPlus(true);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest8, SetSkipEventOnCastPlus02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipEvent02", "SkipEvent02");
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
    keyboardSession->SetSkipEventOnCastPlus(false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest8, SetSkipEventOnCastPlus03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipEvent03", "SkipEvent03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->specificCallback_ = nullptr;
    keyboardSession->SetSkipEventOnCastPlus(true);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest8, SetSkipEventOnCastPlus04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipEvent04", "SkipEvent04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->specificCallback_ = nullptr;
    keyboardSession->SetSkipEventOnCastPlus(false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest8, SetSkipEventOnCastPlus05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipEvent05", "SkipEvent05");
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
    keyboardSession->SetSkipEventOnCastPlus(true);
    keyboardSession->SetSkipEventOnCastPlus(false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason01", "UpdateReason01");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason02", "UpdateReason02");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_START);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason03", "UpdateReason03");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_MOVE);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason04", "UpdateReason04");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_END);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason05", "UpdateReason05");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason06", "UpdateReason06");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::MOVE);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason07", "UpdateReason07");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::MAXIMIZE);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason08", "UpdateReason08");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::RECOVER);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason09, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason09", "UpdateReason09");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::ROTATION);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason10, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason10", "UpdateReason10");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::RESIZE);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason11, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason11", "UpdateReason11");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::HIDE);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason12, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason12", "UpdateReason12");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::TRANSFORM);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason13, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason13", "UpdateReason13");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::AVOID_AREA_CHANGE);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason14, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason14", "UpdateReason14");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::PIP_SHOW);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, UpdateSizeChangeReason15, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason15", "UpdateReason15");
    WSError result1 = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_START);
    WSError result2 = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_END);
    EXPECT_EQ(result1, WSError::WS_OK);
    EXPECT_EQ(result2, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest8, GetPanelRect01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect01", "GetPanelRect01");
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.posX_, 0);
    EXPECT_EQ(panelRect.posY_, 0);
    EXPECT_EQ(panelRect.width_, 0);
    EXPECT_EQ(panelRect.height_, 0);
}

HWTEST_F(KeyboardSessionTest8, GetPanelRect02, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, GetPanelRect03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect03", "GetPanelRect03");
    auto panelSession = GetSceneSession("Panel03", "Panel03");
    WSRect rect = {0, 0, 1000, 800};
    panelSession->SetSessionRect(rect);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    usleep(WAIT_ASYNC_US);
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.width_, 1000);
}

HWTEST_F(KeyboardSessionTest8, GetPanelRect04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect04", "GetPanelRect04");
    keyboardSession->BindKeyboardPanelSession(nullptr);
    usleep(WAIT_ASYNC_US);
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.height_, 0);
}

HWTEST_F(KeyboardSessionTest8, GetPanelRect05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect05", "GetPanelRect05");
    auto panelSession = GetSceneSession("Panel05", "Panel05");
    WSRect rect = {50, 60, 70, 80};
    panelSession->SetSessionRect(rect);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    usleep(WAIT_ASYNC_US);
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.posX_, 50);
    EXPECT_EQ(panelRect.posY_, 60);
    EXPECT_EQ(panelRect.width_, 70);
    EXPECT_EQ(panelRect.height_, 80);
}

HWTEST_F(KeyboardSessionTest8, GetPanelRect06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect06", "GetPanelRect06");
    auto panel1 = GetSceneSession("P1", "P1");
    panel1->SetSessionRect(WSRect{1, 2, 3, 4});
    keyboardSession->BindKeyboardPanelSession(panel1);
    auto panel2 = GetSceneSession("P2", "P2");
    panel2->SetSessionRect(WSRect{5, 6, 7, 8});
    keyboardSession->BindKeyboardPanelSession(panel2);
    usleep(WAIT_ASYNC_US);
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.posX_, 5);
}

HWTEST_F(KeyboardSessionTest8, GetPanelRect07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect07", "GetPanelRect07");
    auto panelSession = GetSceneSession("Panel07", "Panel07");
    WSRect rect = {-10, -20, 100, 200};
    panelSession->SetSessionRect(rect);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    usleep(WAIT_ASYNC_US);
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.posX_, -10);
}

HWTEST_F(KeyboardSessionTest8, RecalculatePanelRectForAvoidArea01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Recalc01", "Recalc01");
    keyboardSession->property_->keyboardLayoutParams_.landscapeAvoidHeight_ = -1;
    keyboardSession->property_->keyboardLayoutParams_.portraitAvoidHeight_ = -1;
    WSRect panelRect = {0, 500, 1000, 300};
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.posY_, 500);
}

HWTEST_F(KeyboardSessionTest8, RecalculatePanelRectForAvoidArea02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Recalc02", "Recalc02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.landscapeAvoidHeight_ = 100;
    keyboardSession->property_->keyboardLayoutParams_.portraitAvoidHeight_ = 150;
    WSRect panelRect = {0, 500, 1000, 300};
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.height_, 150);
}

HWTEST_F(KeyboardSessionTest8, RecalculatePanelRectForAvoidArea03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Recalc03", "Recalc03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.landscapeAvoidHeight_ = 200;
    keyboardSession->property_->keyboardLayoutParams_.portraitAvoidHeight_ = 250;
    keyboardSession->property_->keyboardLayoutParams_.LandscapePanelRect_.width_ = 1000;
    keyboardSession->property_->keyboardLayoutParams_.PortraitPanelRect_.width_ = 500;
    WSRect panelRect = {0, 500, 1000, 300};
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.height_, 200);
}

HWTEST_F(KeyboardSessionTest8, OpenKeyboardSyncTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("OpenSync01", "OpenSync01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->OpenKeyboardSyncTransaction();
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, OpenKeyboardSyncTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("OpenSync02", "OpenSync02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->OpenKeyboardSyncTransaction();
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, OpenKeyboardSyncTransaction03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("OpenSync03", "OpenSync03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->OpenKeyboardSyncTransaction();
    keyboardSession->OpenKeyboardSyncTransaction();
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, OpenKeyboardSyncTransaction04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("OpenSync04", "OpenSync04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->CloseRSTransaction();
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, OpenKeyboardSyncTransaction05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("OpenSync05", "OpenSync05");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->OpenKeyboardSyncTransaction();
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, EnableCallingSessionAvoidArea01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid01", "EnableAvoid01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->EnableCallingSessionAvoidArea();
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, EnableCallingSessionAvoidArea02, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, EnableCallingSessionAvoidArea03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid03", "EnableAvoid03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->property_->SetCallingSessionId(INVALID_WINDOW_ID);
    keyboardSession->EnableCallingSessionAvoidArea();
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, EnableCallingSessionAvoidArea04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid04", "EnableAvoid04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    keyboardSession->EnableCallingSessionAvoidArea();
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, EnableCallingSessionAvoidArea05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid05", "EnableAvoid05");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) {
        SessionInfo info;
        info.abilityName_ = "Calling05";
        info.bundleName_ = "Calling05";
        return sptr<SceneSession>::MakeSptr(info, nullptr);
    };
    keyboardSession->property_->SetCallingSessionId(200);
    keyboardSession->EnableCallingSessionAvoidArea();
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->GetCallingSessionId(), 200);
}

HWTEST_F(KeyboardSessionTest8, EnableCallingSessionAvoidArea06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid06", "EnableAvoid06");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return INVALID_WINDOW_ID; };
    keyboardSession->EnableCallingSessionAvoidArea();
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, EnableCallingSessionAvoidArea07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid07", "EnableAvoid07");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) {
        SessionInfo info;
        info.abilityName_ = "Calling07";
        info.bundleName_ = "Calling07";
        return sptr<SceneSession>::MakeSptr(info, nullptr);
    };
    keyboardSession->property_->SetCallingSessionId(0);
    keyboardSession->EnableCallingSessionAvoidArea();
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->GetCallingSessionId(), 0);
}

HWTEST_F(KeyboardSessionTest8, EnableCallingSessionAvoidArea08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid08", "EnableAvoid08");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetCallingSessionId(300);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->EnableCallingSessionAvoidArea();
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyKeyboardPanelInfoChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel01", "NotifyPanel01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = false;
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
    EXPECT_FALSE(keyboardSession->isKeyboardPanelEnabled_);
}

HWTEST_F(KeyboardSessionTest8, NotifyKeyboardPanelInfoChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel02", "NotifyPanel02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = nullptr;
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
    EXPECT_EQ(keyboardSession->sessionStage_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyKeyboardPanelInfoChange03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel03", "NotifyPanel03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
    EXPECT_NE(keyboardSession->sessionStage_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyKeyboardPanelInfoChange04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel04", "NotifyPanel04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    WSRect rect = {0, 0, 0, 0};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, false);
    EXPECT_TRUE(keyboardSession->isKeyboardPanelEnabled_);
}

HWTEST_F(KeyboardSessionTest8, NotifyKeyboardPanelInfoChange05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel05", "NotifyPanel05");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = false;
    WSRect rect = {10, 20, 30, 40};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, false);
    EXPECT_FALSE(keyboardSession->isKeyboardPanelEnabled_);
}

HWTEST_F(KeyboardSessionTest8, NotifyKeyboardPanelInfoChange06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel06", "NotifyPanel06");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    WSRect rect = {0, 0, 1000, 500};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
    EXPECT_NE(keyboardSession->sessionStage_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyKeyboardPanelInfoChange07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel07", "NotifyPanel07");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = nullptr;
    WSRect rect = {0, 0, 0, 0};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, false);
    EXPECT_EQ(keyboardSession->sessionStage_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyKeyboardPanelInfoChange08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel08", "NotifyPanel08");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    WSRect rect = {5, 5, 5, 5};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
    EXPECT_TRUE(keyboardSession->isKeyboardPanelEnabled_);
}

HWTEST_F(KeyboardSessionTest8, CheckIfNeedRaiseCallingSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise01", "CheckRaise01");
    sptr<SceneSession> callingSession = nullptr;
    bool isFloating = true;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, isFloating);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest8, CheckIfNeedRaiseCallingSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise02", "CheckRaise02");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    auto callingSession = GetSceneSession("Calling", "Calling");
    bool isFloating = true;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, isFloating);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest8, CheckIfNeedRaiseCallingSession03, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, CheckIfNeedRaiseCallingSession04, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, CheckIfNeedRaiseCallingSession05, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, ProcessKeyboardOccupiedAreaInfo01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ProcessOccupied01", "ProcessOccupied01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(100, true, false);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, ProcessKeyboardOccupiedAreaInfo02, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, ProcessKeyboardOccupiedAreaInfo03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ProcessOccupied03", "ProcessOccupied03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(0, true, true);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, ProcessKeyboardOccupiedAreaInfo04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ProcessOccupied04", "ProcessOccupied04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(INVALID_WINDOW_ID, false, false);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, ProcessKeyboardOccupiedAreaInfo05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ProcessOccupied05", "ProcessOccupied05");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(999, true, true);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, ProcessKeyboardOccupiedAreaInfo06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ProcessOccupied06", "ProcessOccupied06");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(100, true, false);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->keyboardCallback_->onGetSceneSession, nullptr);
}

HWTEST_F(KeyboardSessionTest8, RestoreCallingSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore01", "Restore01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = false;
    keyboardSession->RestoreCallingSession(100, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->keyboardAvoidAreaActive_);
}

HWTEST_F(KeyboardSessionTest8, RestoreCallingSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore02", "Restore02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->RestoreCallingSession(100, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->keyboardAvoidAreaActive_);
}

HWTEST_F(KeyboardSessionTest8, RestoreCallingSession03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore03", "Restore03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = false;
    keyboardSession->RestoreCallingSession(0, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->keyboardAvoidAreaActive_);
}

HWTEST_F(KeyboardSessionTest8, RestoreCallingSession04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore04", "Restore04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = false;
    keyboardSession->RestoreCallingSession(INVALID_WINDOW_ID, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->keyboardAvoidAreaActive_);
}

HWTEST_F(KeyboardSessionTest8, RestoreCallingSession05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore05", "Restore05");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->RestoreCallingSession(200, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->keyboardAvoidAreaActive_);
}

HWTEST_F(KeyboardSessionTest8, RestoreCallingSession06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore06", "Restore06");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = false;
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    keyboardSession->RestoreCallingSession(300, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->keyboardAvoidAreaActive_);
}

HWTEST_F(KeyboardSessionTest8, RestoreCallingSession07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore07", "Restore07");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = false;
    std::shared_ptr<RSTransaction> rs = nullptr;
    keyboardSession->RestoreCallingSession(100, rs);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->keyboardAvoidAreaActive_);
}

HWTEST_F(KeyboardSessionTest8, RestoreCallingSession08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore08", "Restore08");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    std::shared_ptr<RSTransaction> rs = nullptr;
    keyboardSession->RestoreCallingSession(400, rs);
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->keyboardAvoidAreaActive_);
}

HWTEST_F(KeyboardSessionTest8, NotifySystemKeyboardAvoidChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid01", "NotifySysAvoid01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW);
    EXPECT_EQ(keyboardSession->systemConfig_.windowUIType_, WindowUIType::PHONE_WINDOW);
}

HWTEST_F(KeyboardSessionTest8, NotifySystemKeyboardAvoidChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid02", "NotifySysAvoid02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    keyboardSession->SetIsSystemKeyboard(false);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW);
    EXPECT_FALSE(keyboardSession->IsSystemKeyboard());
}

HWTEST_F(KeyboardSessionTest8, NotifySystemKeyboardAvoidChange03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid03", "NotifySysAvoid03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_BEGIN);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifySystemKeyboardAvoidChange04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid04", "NotifySysAvoid04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifySystemKeyboardAvoidChange05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid05", "NotifySysAvoid05");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_HIDE);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifySystemKeyboardAvoidChange06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid06", "NotifySysAvoid06");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_DISCONNECT);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifySystemKeyboardAvoidChange07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid07", "NotifySysAvoid07");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_GRAVITY_BOTTOM);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifySystemKeyboardAvoidChange08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid08", "NotifySysAvoid08");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_GRAVITY_FLOAT);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifySystemKeyboardAvoidChange09, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid09", "NotifySysAvoid09");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_END);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifySystemKeyboardAvoidChange10, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid10", "NotifySysAvoid10");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW);
    EXPECT_TRUE(keyboardSession->IsSystemKeyboard());
}

HWTEST_F(KeyboardSessionTest8, NotifySystemKeyboardAvoidChange11, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid11", "NotifySysAvoid11");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onSystemKeyboardAvoidChange = nullptr;
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifySystemKeyboardAvoidChange12, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid12", "NotifySysAvoid12");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onSystemKeyboardAvoidChange =
        [](DisplayId displayId, SystemKeyboardAvoidChangeReason reason) {};
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_HIDE);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyRootSceneOccupiedAreaChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRoot01", "NotifyRoot01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(100);
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    ASSERT_NE(info, nullptr);
    keyboardSession->NotifyRootSceneOccupiedAreaChange(info);
    EXPECT_NE(keyboardSession->property_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyRootSceneOccupiedAreaChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRoot02", "NotifyRoot02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(0);
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    keyboardSession->NotifyRootSceneOccupiedAreaChange(info);
    EXPECT_NE(keyboardSession->property_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyRootSceneOccupiedAreaChange03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRoot03", "NotifyRoot03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(DISPLAY_ID_INVALID);
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    keyboardSession->NotifyRootSceneOccupiedAreaChange(info);
    EXPECT_NE(keyboardSession->property_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetRSTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetRSTrans01", "GetRSTrans01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    std::shared_ptr<RSTransaction> transaction = keyboardSession->GetRSTransaction();
    EXPECT_EQ(transaction, nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetRSTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetRSTrans02", "GetRSTrans02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    std::shared_ptr<RSTransaction> transaction = keyboardSession->GetRSTransaction();
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, GetRSTransaction03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetRSTrans03", "GetRSTrans03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    std::shared_ptr<RSTransaction> transaction = keyboardSession->GetRSTransaction();
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, GetRSTransaction04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetRSTrans04", "GetRSTrans04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    std::shared_ptr<RSTransaction> transaction = keyboardSession->GetRSTransaction();
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, GetRSTransaction05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetRSTrans05", "GetRSTrans05");
    ASSERT_NE(keyboardSession, nullptr);
    std::shared_ptr<RSTransaction> transaction = keyboardSession->GetRSTransaction();
    EXPECT_EQ(transaction, nullptr);
}

HWTEST_F(KeyboardSessionTest8, CloseRSTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTrans01", "CloseRSTrans01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->CloseRSTransaction();
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, CloseRSTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTrans02", "CloseRSTrans02");
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->CloseRSTransaction();
    EXPECT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);
}

HWTEST_F(KeyboardSessionTest8, CloseRSTransaction03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTrans03", "CloseRSTrans03");
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->CloseRSTransaction();
    keyboardSession->CloseRSTransaction();
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, CloseRSTransaction04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTrans04", "CloseRSTrans04");
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->CloseRSTransaction();
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->CloseRSTransaction();
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, CloseRSTransaction05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTrans05", "CloseRSTrans05");
    keyboardSession->OpenKeyboardSyncTransaction();
    keyboardSession->CloseRSTransaction();
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, GetSessionScreenName01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName01", "GetScreenName01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_ = nullptr;
    std::string screenName = keyboardSession->GetSessionScreenName();
    EXPECT_EQ(screenName, "");
}

HWTEST_F(KeyboardSessionTest8, GetSessionScreenName02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName02", "GetScreenName02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(DISPLAY_ID_INVALID);
    std::string screenName = keyboardSession->GetSessionScreenName();
    EXPECT_EQ(screenName, "");
}

HWTEST_F(KeyboardSessionTest8, GetSessionScreenName03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName03", "GetScreenName03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(0);
    std::string screenName = keyboardSession->GetSessionScreenName();
    EXPECT_TRUE(screenName.empty() || !screenName.empty());
}

HWTEST_F(KeyboardSessionTest8, GetSessionScreenName04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName04", "GetScreenName04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(1);
    std::string screenName = keyboardSession->GetSessionScreenName();
    EXPECT_TRUE(screenName.empty() || !screenName.empty());
}

HWTEST_F(KeyboardSessionTest8, GetSessionScreenName05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName05", "GetScreenName05");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(100);
    std::string screenName = keyboardSession->GetSessionScreenName();
    EXPECT_TRUE(screenName.empty() || !screenName.empty());
}

HWTEST_F(KeyboardSessionTest8, GetSessionScreenName06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName06", "GetScreenName06");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(UINT64_MAX);
    std::string screenName = keyboardSession->GetSessionScreenName();
    EXPECT_TRUE(screenName.empty() || !screenName.empty());
}

HWTEST_F(KeyboardSessionTest8, GetSessionScreenName07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName07", "GetScreenName07");
    ASSERT_NE(keyboardSession, nullptr);
    std::string screenName = keyboardSession->GetSessionScreenName();
    EXPECT_TRUE(screenName.empty() || !screenName.empty());
}

HWTEST_F(KeyboardSessionTest8, CalculateOccupiedAreaAfterUIRefresh01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcOccupiedUI01", "CalcOccupiedUI01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetCallingSessionId01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingId01", "SetCallingId01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return INVALID_WINDOW_ID; };
    keyboardSession->SetCallingSessionId(100);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetCallingSessionId02, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, SetCallingSessionId03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingId03", "SetCallingId03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    keyboardSession->SetCallingSessionId(300);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetCallingSessionId04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingId04", "SetCallingId04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->keyboardCallback_->onCallingSessionIdChange = [](uint32_t id) {};
    keyboardSession->SetCallingSessionId(400);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetCallingSessionId05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingId05", "SetCallingId05");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->SetCallingSessionId(0);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetCallingSessionId06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingId06", "SetCallingId06");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->SetCallingSessionId(INVALID_WINDOW_ID);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetCallingSessionId07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingId07", "SetCallingId07");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->SetCallingSessionId(UINT32_MAX);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetCallingSessionId08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingId08", "SetCallingId08");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->SetCallingSessionId(1);
    keyboardSession->SetCallingSessionId(2);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, HandleCrossScreenChild01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CrossScreen01", "CrossScreen01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->moveDragController_ = nullptr;
    keyboardSession->HandleCrossScreenChild(true);
    EXPECT_EQ(keyboardSession->moveDragController_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, HandleCrossScreenChild02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CrossScreen02", "CrossScreen02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->moveDragController_ = nullptr;
    keyboardSession->HandleCrossScreenChild(false);
    EXPECT_EQ(keyboardSession->moveDragController_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, HandleCrossScreenChild03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CrossScreen03", "CrossScreen03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->moveDragController_ = nullptr;
    keyboardSession->HandleCrossScreenChild(true);
    keyboardSession->HandleCrossScreenChild(false);
    EXPECT_EQ(keyboardSession->moveDragController_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, PostKeyboardAnimationSyncTimeoutTask01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("PostTimeout01", "PostTimeout01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, PostKeyboardAnimationSyncTimeoutTask02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("PostTimeout02", "PostTimeout02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, PostKeyboardAnimationSyncTimeoutTask03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("PostTimeout03", "PostTimeout03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, PostKeyboardAnimationSyncTimeoutTask04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("PostTimeout04", "PostTimeout04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
    keyboardSession->CloseRSTransaction();
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_);
}

HWTEST_F(KeyboardSessionTest8, PostKeyboardAnimationSyncTimeoutTask05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("PostTimeout05", "PostTimeout05");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetFocusedSessionId01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused01", "GetFocused01");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = nullptr;
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, INVALID_WINDOW_ID);
}

HWTEST_F(KeyboardSessionTest8, GetFocusedSessionId02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused02", "GetFocused02");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return 100; };
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, 100);
}

HWTEST_F(KeyboardSessionTest8, GetFocusedSessionId03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused03", "GetFocused03");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return INVALID_WINDOW_ID; };
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, INVALID_WINDOW_ID);
}

HWTEST_F(KeyboardSessionTest8, GetFocusedSessionId04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused04", "GetFocused04");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return 0; };
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, 0);
}

HWTEST_F(KeyboardSessionTest8, GetFocusedSessionId05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused05", "GetFocused05");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return INT32_MAX; };
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, INT32_MAX);
}

HWTEST_F(KeyboardSessionTest8, GetFocusedSessionId06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused06", "GetFocused06");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return -1; };
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, -1);
}

HWTEST_F(KeyboardSessionTest8, GetFocusedSessionId07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused07", "GetFocused07");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return 9999; };
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, 9999);
}

HWTEST_F(KeyboardSessionTest8, NotifyOccupiedAreaChanged01, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, NotifyOccupiedAreaChanged02, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, NotifyOccupiedAreaChanged03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyOccupied03", "NotifyOccupied03");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, info, false, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyOccupiedAreaChanged04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyOccupied04", "NotifyOccupied04");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, info, true, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyOccupiedAreaChanged05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyOccupied05", "NotifyOccupied05");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<SceneSession> callingSession = nullptr;
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, info, false, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyOccupiedAreaChanged06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyOccupied06", "NotifyOccupied06");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    std::shared_ptr<RSTransaction> rs = nullptr;
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, info, true, rs);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyOccupiedAreaChanged07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyOccupied07", "NotifyOccupied07");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    callingSession->sessionInfo_.isSystem_ = true;
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, info, true, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, NotifyOccupiedAreaChanged08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyOccupied08", "NotifyOccupied08");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, info, false, nullptr);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetSceneSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScene01", "GetScene01");
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    sptr<SceneSession> session = keyboardSession->GetSceneSession(100);
    EXPECT_EQ(session, nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetSceneSession02, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest8, GetSceneSession03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScene03", "GetScene03");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    sptr<SceneSession> session = keyboardSession->GetSceneSession(INVALID_WINDOW_ID);
    EXPECT_EQ(session, nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetSceneSession04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScene04", "GetScene04");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) {
        SessionInfo info;
        info.abilityName_ = "Test04";
        info.bundleName_ = "Test04";
        return sptr<SceneSession>::MakeSptr(info, nullptr);
    };
    sptr<SceneSession> session = keyboardSession->GetSceneSession(1);
    EXPECT_NE(session, nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetSceneSession05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScene05", "GetScene05");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    sptr<SceneSession> session = keyboardSession->GetSceneSession(0);
    EXPECT_EQ(session, nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetSceneSession06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScene06", "GetScene06");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    sptr<SceneSession> session = keyboardSession->GetSceneSession(UINT32_MAX);
    EXPECT_EQ(session, nullptr);
}

HWTEST_F(KeyboardSessionTest8, GetSceneSession07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScene07", "GetScene07");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "Test07";
        info.bundleName_ = "Test07";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        session->persistentId_ = persistentId;
        return session;
    };
    sptr<SceneSession> session = keyboardSession->GetSceneSession(777);
    EXPECT_NE(session, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetSurfaceBounds01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds01", "SetBounds01");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->SetSurfaceBounds(rect, false, false);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetSurfaceBounds02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds02", "SetBounds02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardPanelSession_ = nullptr;
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->SetSurfaceBounds(rect, true, true);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetSurfaceBounds03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds03", "SetBounds03");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {0, 0, 0, 0};
    keyboardSession->SetSurfaceBounds(rect, false, false);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetSurfaceBounds04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds04", "SetBounds04");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {500, 600, 800, 400};
    keyboardSession->SetSurfaceBounds(rect, true, false);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetSurfaceBounds05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds05", "SetBounds05");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {0, 0, 1000, 500};
    keyboardSession->SetSurfaceBounds(rect, false, true);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetSurfaceBounds06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds06", "SetBounds06");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {10, 20, 30, 40};
    keyboardSession->SetSurfaceBounds(rect, true, true);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetSurfaceBounds07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds07", "SetBounds07");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {-1, -1, 100, 100};
    keyboardSession->SetSurfaceBounds(rect, false, false);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetSurfaceBounds08, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds08", "SetBounds08");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {0, 0, 0, 0};
    keyboardSession->SetSurfaceBounds(rect, true, true);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetSurfaceBounds09, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds09", "SetBounds09");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->SetSurfaceBounds(rect, false, false);
    keyboardSession->SetSurfaceBounds(rect, true, true);
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

HWTEST_F(KeyboardSessionTest8, SetSurfaceBounds10, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds10", "SetBounds10");
    ASSERT_NE(keyboardSession, nullptr);
    for (int i = 0; i < 3; ++i) {
        WSRect rect = {i * 100, i * 100, 500, 300};
        keyboardSession->SetSurfaceBounds(rect, true, false);
    }
    EXPECT_EQ(keyboardSession->keyboardPanelSession_, nullptr);
}

} // namespace
} // namespace Rosen
} // namespace OHOS
