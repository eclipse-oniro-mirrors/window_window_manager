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

class KeyboardSessionTest5 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<KeyboardSession> GetKeyboardSession(const std::string& abilityName, const std::string& bundleName);
    sptr<SceneSession> GetSceneSession(const std::string& abilityName, const std::string& bundleName);
};

void KeyboardSessionTest5::SetUpTestCase() {}
void KeyboardSessionTest5::TearDownTestCase() {}
void KeyboardSessionTest5::SetUp() {}
void KeyboardSessionTest5::TearDown() {}

sptr<KeyboardSession> KeyboardSessionTest5::GetKeyboardSession(const std::string& abilityName,
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

sptr<SceneSession> KeyboardSessionTest5::GetSceneSession(const std::string& abilityName, const std::string& bundleName)
{
    SessionInfo info;
    info.abilityName_ = abilityName;
    info.bundleName_ = bundleName;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    return sceneSession;
}

namespace {

HWTEST_F(KeyboardSessionTest5, Constructor01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor01";
    info.bundleName_ = "Constructor01";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
}

HWTEST_F(KeyboardSessionTest5, Constructor02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Constructor02";
    info.bundleName_ = "Constructor02";
    info.persistentId_ = 100;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
}

HWTEST_F(KeyboardSessionTest5, Constructor03, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest5, Show01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Show01", "Show01");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = nullptr;
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

HWTEST_F(KeyboardSessionTest5, Show02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Show02", "Show02");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, Show03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Show03", "Show03");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetKeyboardEffectOption(KeyboardEffectOption());
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, Show04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Show04", "Show04");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, Show05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Show05", "Show05");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSError result = keyboardSession->Show(property);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, Hide01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Hide01", "Hide01");
    keyboardSession->state_ = SessionState::STATE_DISCONNECT;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, Hide02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Hide02", "Hide02");
    keyboardSession->state_ = SessionState::STATE_CONNECT;
    keyboardSession->isActive_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, Hide03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Hide03", "Hide03");
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, Hide04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Hide04", "Hide04");
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, Hide05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Hide05", "Hide05");
    keyboardSession->SetIsSystemKeyboard(false);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WSError result = keyboardSession->Hide();
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, Disconnect01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Disconnect01";
    info.bundleName_ = "Disconnect01";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    WSError result = keyboardSession->Disconnect(false);
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, Disconnect02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Disconnect02";
    info.bundleName_ = "Disconnect02";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    WSError result = keyboardSession->Disconnect(true);
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, Disconnect03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Disconnect03";
    info.bundleName_ = "Disconnect03";
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

HWTEST_F(KeyboardSessionTest5, Disconnect04, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Disconnect04";
    info.bundleName_ = "Disconnect04";
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

HWTEST_F(KeyboardSessionTest5, Disconnect05, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Disconnect05";
    info.bundleName_ = "Disconnect05";
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

HWTEST_F(KeyboardSessionTest5, BindKeyboardPanelSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Bind01", "Bind01");
    sptr<SceneSession> panelSession = nullptr;
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest5, BindKeyboardPanelSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Bind02", "Bind02");
    auto panelSession = GetSceneSession("Panel01", "Panel01");
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), panelSession);
}

HWTEST_F(KeyboardSessionTest5, BindKeyboardPanelSession03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Bind03", "Bind03");
    auto panelSession1 = GetSceneSession("Panel01", "Panel01");
    auto panelSession2 = GetSceneSession("Panel02", "Panel02");
    keyboardSession->BindKeyboardPanelSession(panelSession1);
    keyboardSession->BindKeyboardPanelSession(panelSession2);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), panelSession2);
}

HWTEST_F(KeyboardSessionTest5, GetKeyboardPanelSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanel01", "GetPanel01");
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest5, GetKeyboardPanelSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanel02", "GetPanel02");
    auto panelSession = GetSceneSession("Panel", "Panel");
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_NE(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

HWTEST_F(KeyboardSessionTest5, GetKeyboardGravity01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Gravity01", "Gravity01");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    SessionGravity gravity = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(gravity, SessionGravity::SESSION_GRAVITY_BOTTOM);
}

HWTEST_F(KeyboardSessionTest5, GetKeyboardGravity02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Gravity02", "Gravity02");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    SessionGravity gravity = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(gravity, SessionGravity::SESSION_GRAVITY_FLOAT);
}

HWTEST_F(KeyboardSessionTest5, GetCallingSessionId01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling01", "GetCalling01");
    keyboardSession->property_->SetCallingSessionId(100);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, 100);
}

HWTEST_F(KeyboardSessionTest5, GetCallingSessionId02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling02", "GetCalling02");
    keyboardSession->property_->SetCallingSessionId(INVALID_WINDOW_ID);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, INVALID_WINDOW_ID);
}

HWTEST_F(KeyboardSessionTest5, GetCallingSessionId03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling03", "GetCalling03");
    keyboardSession->property_->SetCallingSessionId(UINT32_MAX);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, UINT32_MAX);
}

HWTEST_F(KeyboardSessionTest5, GetCallingSessionId04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCalling04", "GetCalling04");
    keyboardSession->property_ = nullptr;
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, INVALID_SESSION_ID);
}

HWTEST_F(KeyboardSessionTest5, NotifyClientToUpdateRect01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRect01", "NotifyRect01");
    WSRect rect = {800, 800, 1200, 1200};
    WSError result = keyboardSession->NotifyClientToUpdateRect("test", rect, nullptr);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, NotifyClientToUpdateRect02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRect02", "NotifyRect02");
    WSRect rect = {800, 800, 1200, 1200};
    std::shared_ptr<RSTransaction> rsTransaction = nullptr;
    WSError result = keyboardSession->NotifyClientToUpdateRect("update", rect, rsTransaction);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, NotifyClientToUpdateRect03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRect03", "NotifyRect03");
    WSRect rect = {800, 800, 1200, 1200};
    WSError result = keyboardSession->NotifyClientToUpdateRect("", rect, nullptr);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, IsVisibleForeground01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Visible01", "Visible01");
    keyboardSession->isVisible_ = true;
    EXPECT_EQ(keyboardSession->IsVisibleForeground(), true);
}

HWTEST_F(KeyboardSessionTest5, IsVisibleForeground02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Visible02", "Visible02");
    keyboardSession->isVisible_ = false;
    EXPECT_EQ(keyboardSession->IsVisibleForeground(), false);
}

HWTEST_F(KeyboardSessionTest5, IsVisibleNotBackground01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotBack01", "NotBack01");
    keyboardSession->isVisible_ = true;
    EXPECT_EQ(keyboardSession->IsVisibleNotBackground(), true);
}

HWTEST_F(KeyboardSessionTest5, IsVisibleNotBackground02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotBack02", "NotBack02");
    keyboardSession->isVisible_ = false;
    EXPECT_EQ(keyboardSession->IsVisibleNotBackground(), false);
}

HWTEST_F(KeyboardSessionTest5, GetIsKeyboardSyncTransactionOpen01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SyncOpen01", "SyncOpen01");
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    EXPECT_EQ(keyboardSession->GetIsKeyboardSyncTransactionOpen(), false);
}

HWTEST_F(KeyboardSessionTest5, GetIsKeyboardSyncTransactionOpen02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SyncOpen02", "SyncOpen02");
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    EXPECT_EQ(keyboardSession->GetIsKeyboardSyncTransactionOpen(), true);
}

HWTEST_F(KeyboardSessionTest5, GetIsKeyboardSyncTransactionOpen03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SyncOpen03", "SyncOpen03");
    EXPECT_EQ(keyboardSession->GetIsKeyboardSyncTransactionOpen(), false);
}

HWTEST_F(KeyboardSessionTest5, ChangeKeyboardEffectOption01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect01", "ChangeEffect01");
    KeyboardEffectOption effectOption;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, ChangeKeyboardEffectOption02, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest5, ChangeKeyboardEffectOption03, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest5, ChangeKeyboardEffectOption04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect04", "ChangeEffect04");
    keyboardSession->changeKeyboardEffectOptionFunc_ = [](const KeyboardEffectOption& option) {};
    KeyboardEffectOption effectOption;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, ChangeKeyboardEffectOption05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect05", "ChangeEffect05");
    keyboardSession->changeKeyboardEffectOptionFunc_ = nullptr;
    KeyboardEffectOption effectOption;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, SetKeyboardEffectOptionChangeListener01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetListener01", "SetListener01");
    NotifyKeyboarEffectOptionChangeFunc func = [](const KeyboardEffectOption& option) {};
    keyboardSession->SetKeyboardEffectOptionChangeListener(func);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, SetKeyboardEffectOptionChangeListener02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetListener02", "SetListener02");
    NotifyKeyboarEffectOptionChangeFunc func = nullptr;
    keyboardSession->SetKeyboardEffectOptionChangeListener(func);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, SetSkipSelfWhenShowOnVirtualScreen01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Skip01", "Skip01");
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, SetSkipSelfWhenShowOnVirtualScreen02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Skip02", "Skip02");
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, SetSkipEventOnCastPlus01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipEvent01", "SkipEvent01");
    keyboardSession->SetSkipEventOnCastPlus(true);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, SetSkipEventOnCastPlus02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipEvent02", "SkipEvent02");
    keyboardSession->SetSkipEventOnCastPlus(false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, UpdateSizeChangeReason01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason01", "UpdateReason01");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, UpdateSizeChangeReason02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason02", "UpdateReason02");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_START);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, UpdateSizeChangeReason03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason03", "UpdateReason03");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_MOVE);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, UpdateSizeChangeReason04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason04", "UpdateReason04");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_END);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, UpdateSizeChangeReason05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason05", "UpdateReason05");
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, GetPanelRect01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect01", "GetPanelRect01");
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.posX_, 0);
    EXPECT_EQ(panelRect.posY_, 0);
    EXPECT_EQ(panelRect.width_, 0);
    EXPECT_EQ(panelRect.height_, 0);
}

HWTEST_F(KeyboardSessionTest5, GetPanelRect02, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest5, RecalculatePanelRectForAvoidArea01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Recalc01", "Recalc01");
    keyboardSession->property_->keyboardLayoutParams_.landscapeAvoidHeight_ = -1;
    keyboardSession->property_->keyboardLayoutParams_.portraitAvoidHeight_ = -1;
    WSRect panelRect = {0, 500, 1000, 300};
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.posY_, 500);
}

HWTEST_F(KeyboardSessionTest5, RecalculatePanelRectForAvoidArea02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Recalc02", "Recalc02");
    keyboardSession->property_->keyboardLayoutParams_.landscapeAvoidHeight_ = 100;
    keyboardSession->property_->keyboardLayoutParams_.portraitAvoidHeight_ = 150;
    WSRect panelRect = {0, 500, 1000, 300};
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
}

HWTEST_F(KeyboardSessionTest5, RecalculatePanelRectForAvoidArea03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Recalc03", "Recalc03");
    keyboardSession->property_->keyboardLayoutParams_.landscapeAvoidHeight_ = 200;
    keyboardSession->property_->keyboardLayoutParams_.portraitAvoidHeight_ = 250;
    keyboardSession->property_->keyboardLayoutParams_.LandscapePanelRect_.width_ = 1000;
    keyboardSession->property_->keyboardLayoutParams_.PortraitPanelRect_.width_ = 500;
    WSRect panelRect = {0, 500, 1000, 300};
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
}

HWTEST_F(KeyboardSessionTest5, RecalculatePanelRectForAvoidArea04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Recalc04", "Recalc04");
    keyboardSession->property_->keyboardLayoutParams_.landscapeAvoidHeight_ = 0;
    keyboardSession->property_->keyboardLayoutParams_.portraitAvoidHeight_ = 0;
    WSRect panelRect = {0, 500, 1000, 300};
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
}

HWTEST_F(KeyboardSessionTest5, RecalculatePanelRectForAvoidArea05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Recalc05", "Recalc05");
    keyboardSession->property_->keyboardLayoutParams_.landscapeAvoidHeight_ = 50;
    keyboardSession->property_->keyboardLayoutParams_.portraitAvoidHeight_ = 80;
    keyboardSession->property_->keyboardLayoutParams_.LandscapePanelRect_.width_ = 500;
    keyboardSession->property_->keyboardLayoutParams_.PortraitPanelRect_.width_ = 500;
    WSRect panelRect = {0, 500, 500, 300};
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
}

HWTEST_F(KeyboardSessionTest5, OpenKeyboardSyncTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("OpenSync01", "OpenSync01");
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->OpenKeyboardSyncTransaction();
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, OpenKeyboardSyncTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("OpenSync02", "OpenSync02");
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->OpenKeyboardSyncTransaction();
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, CloseKeyboardSyncTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseSync01", "CloseSync01");
    WSRect rect = {0, 0, 0, 0};
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 1;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CloseKeyboardSyncTransaction(rect, true, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, CloseKeyboardSyncTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseSync02", "CloseSync02");
    WSRect rect = {0, 0, 0, 0};
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = INVALID_WINDOW_ID;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CloseKeyboardSyncTransaction(rect, false, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, CloseKeyboardSyncTransaction03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseSync03", "CloseSync03");
    WSRect rect = {100, 200, 500, 300};
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 5;
    animationInfo.isGravityChanged = true;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CloseKeyboardSyncTransaction(rect, true, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, CloseKeyboardSyncTransaction04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseSync04", "CloseSync04");
    WSRect rect = {0, 0, 0, 0};
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 10;
    animationInfo.isGravityChanged = false;
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CloseKeyboardSyncTransaction(rect, true, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, CloseKeyboardSyncTransaction05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseSync05", "CloseSync05");
    keyboardSession->keyboardCallback_->isLastFrameLayoutFinished = []() { return true; };
    WSRect rect = {0, 0, 0, 0};
    WindowAnimationInfo animationInfo;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CloseKeyboardSyncTransaction(rect, true, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, EnableCallingSessionAvoidArea01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid01", "EnableAvoid01");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->EnableCallingSessionAvoidArea();
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, EnableCallingSessionAvoidArea02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid02", "EnableAvoid02");
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
}

HWTEST_F(KeyboardSessionTest5, EnableCallingSessionAvoidArea03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid03", "EnableAvoid03");
    keyboardSession->keyboardCallback_ = nullptr;
    keyboardSession->EnableCallingSessionAvoidArea();
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, EnableCallingSessionAvoidArea04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid04", "EnableAvoid04");
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    keyboardSession->EnableCallingSessionAvoidArea();
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, CalculateCenterScaledRect01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcScaled01", "CalcScaled01");
    WSRect rect = {500, 500, 2000, 2000};
    WSRect scaledRect = keyboardSession->CalculateCenterScaledRect(rect, 1.0f, 1.0f);
    EXPECT_EQ(scaledRect.posX_, 500);
    EXPECT_EQ(scaledRect.posY_, 500);
    EXPECT_EQ(scaledRect.width_, 2000);
    EXPECT_EQ(scaledRect.height_, 2000);
}

HWTEST_F(KeyboardSessionTest5, CalculateCenterScaledRect02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcScaled02", "CalcScaled02");
    WSRect rect = {500, 500, 2000, 2000};
    WSRect scaledRect = keyboardSession->CalculateCenterScaledRect(rect, 1.5f, 1.0f);
    EXPECT_EQ(scaledRect.width_, 3000);
    EXPECT_EQ(scaledRect.height_, 2000);
}

HWTEST_F(KeyboardSessionTest5, CalculateCenterScaledRect03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcScaled03", "CalcScaled03");
    WSRect rect = {500, 500, 2000, 2000};
    WSRect scaledRect = keyboardSession->CalculateCenterScaledRect(rect, 1.0f, 1.5f);
    EXPECT_EQ(scaledRect.width_, 2000);
    EXPECT_EQ(scaledRect.height_, 3000);
}

HWTEST_F(KeyboardSessionTest5, CalculateCenterScaledRect04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcScaled04", "CalcScaled04");
    WSRect rect = {0, 0, 1000, 1000};
    WSRect scaledRect = keyboardSession->CalculateCenterScaledRect(rect, 2.0f, 2.0f);
    EXPECT_EQ(scaledRect.width_, 2000);
    EXPECT_EQ(scaledRect.height_, 2000);
}

HWTEST_F(KeyboardSessionTest5, CalculateCenterScaledRect05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcScaled05", "CalcScaled05");
    WSRect rect = {100, 100, 500, 500};
    WSRect scaledRect = keyboardSession->CalculateCenterScaledRect(rect, 0.5f, 0.5f);
    EXPECT_EQ(scaledRect.posX_, 100);
    EXPECT_EQ(scaledRect.posY_, 100);
}

HWTEST_F(KeyboardSessionTest5, CalculateCenterScaledRect06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcScaled06", "CalcScaled06");
    WSRect rect = {922, 277, 1274, 1387};
    WSRect scaledRect = keyboardSession->CalculateCenterScaledRect(rect, 1.399529f, 1.399423f);
    EXPECT_GT(scaledRect.width_, rect.width_);
    EXPECT_GT(scaledRect.height_, rect.height_);
}

HWTEST_F(KeyboardSessionTest5, CalculateCenterScaledRect07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcScaled07", "CalcScaled07");
    WSRect rect = {0, 0, 100, 100};
    WSRect scaledRect = keyboardSession->CalculateCenterScaledRect(rect, 10.0f, 10.0f);
    EXPECT_EQ(scaledRect.width_, 1000);
    EXPECT_EQ(scaledRect.height_, 1000);
}

HWTEST_F(KeyboardSessionTest5, NotifyKeyboardPanelInfoChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel01", "NotifyPanel01");
    keyboardSession->isKeyboardPanelEnabled_ = false;
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
}

HWTEST_F(KeyboardSessionTest5, NotifyKeyboardPanelInfoChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel02", "NotifyPanel02");
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = nullptr;
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
}

HWTEST_F(KeyboardSessionTest5, NotifyKeyboardPanelInfoChange03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel03", "NotifyPanel03");
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
}

HWTEST_F(KeyboardSessionTest5, NotifyKeyboardPanelInfoChange04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel04", "NotifyPanel04");
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    WSRect rect = {0, 0, 0, 0};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, false);
}

HWTEST_F(KeyboardSessionTest5, NotifyKeyboardPanelInfoChange05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel05", "NotifyPanel05");
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    WSRect rect = {500, 600, 800, 400};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
}

HWTEST_F(KeyboardSessionTest5, CheckIfNeedRaiseCallingSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise01", "CheckRaise01");
    sptr<SceneSession> callingSession = nullptr;
    bool isFloating = true;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, isFloating);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest5, CheckIfNeedRaiseCallingSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise02", "CheckRaise02");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    auto callingSession = GetSceneSession("Calling", "Calling");
    bool isFloating = true;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, isFloating);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest5, CheckIfNeedRaiseCallingSession03, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest5, CheckIfNeedRaiseCallingSession04, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest5, CheckIfNeedRaiseCallingSession05, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest5, CheckIfNeedRaiseCallingSession06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise06", "CheckRaise06");
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    auto callingSession = GetSceneSession("Calling", "Calling");
    callingSession->SetIsMidScene(true);
    bool isFloating = true;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, isFloating);
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest5, ProcessKeyboardOccupiedAreaInfo01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ProcessOccupied01", "ProcessOccupied01");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(100, true, false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, ProcessKeyboardOccupiedAreaInfo02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ProcessOccupied02", "ProcessOccupied02");
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
}

HWTEST_F(KeyboardSessionTest5, ProcessKeyboardOccupiedAreaInfo03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ProcessOccupied03", "ProcessOccupied03");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "Calling";
        info.bundleName_ = "Calling";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        return session;
    };
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(INVALID_WINDOW_ID, true, false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, RestoreCallingSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore01", "Restore01");
    keyboardSession->keyboardAvoidAreaActive_ = false;
    keyboardSession->RestoreCallingSession(100, nullptr);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, RestoreCallingSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore02", "Restore02");
    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->RestoreCallingSession(100, nullptr);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, RestoreCallingSession03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore03", "Restore03");
    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "Calling";
        info.bundleName_ = "Calling";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        session->SetOriPosYBeforeRaisedByKeyboard(500);
        session->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
        return session;
    };
    keyboardSession->RestoreCallingSession(100, nullptr);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, RestoreCallingSession04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore04", "Restore04");
    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "Calling";
        info.bundleName_ = "Calling";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        session->SetOriPosYBeforeRaisedByKeyboard(0);
        return session;
    };
    keyboardSession->RestoreCallingSession(100, nullptr);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, NotifySystemKeyboardAvoidChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid01", "NotifySysAvoid01");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW);
}

HWTEST_F(KeyboardSessionTest5, NotifySystemKeyboardAvoidChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid02", "NotifySysAvoid02");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    keyboardSession->SetIsSystemKeyboard(false);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW);
}

HWTEST_F(KeyboardSessionTest5, NotifySystemKeyboardAvoidChange03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid03", "NotifySysAvoid03");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->keyboardCallback_->onSystemKeyboardAvoidChange = nullptr;
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW);
}

HWTEST_F(KeyboardSessionTest5, NotifySystemKeyboardAvoidChange04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid04", "NotifySysAvoid04");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->keyboardCallback_->onSystemKeyboardAvoidChange =
        [](DisplayId id, SystemKeyboardAvoidChangeReason reason) {};
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_HIDE);
}

HWTEST_F(KeyboardSessionTest5, NotifySystemKeyboardAvoidChange05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid05", "NotifySysAvoid05");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_GRAVITY_BOTTOM);
}

HWTEST_F(KeyboardSessionTest5, NotifySystemKeyboardAvoidChange06, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid06", "NotifySysAvoid06");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_DISCONNECT);
}

HWTEST_F(KeyboardSessionTest5, NotifySystemKeyboardAvoidChange07, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid07", "NotifySysAvoid07");
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_GRAVITY_FLOAT);
}

HWTEST_F(KeyboardSessionTest5, NotifyRootSceneOccupiedAreaChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRoot01", "NotifyRoot01");
    keyboardSession->property_->SetDisplayId(100);
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    keyboardSession->NotifyRootSceneOccupiedAreaChange(info);
}

HWTEST_F(KeyboardSessionTest5, NotifyRootSceneOccupiedAreaChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRoot02", "NotifyRoot02");
    keyboardSession->keyboardCallback_->onNotifyOccupiedAreaChange = nullptr;
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    keyboardSession->NotifyRootSceneOccupiedAreaChange(info);
}

HWTEST_F(KeyboardSessionTest5, NotifyRootSceneOccupiedAreaChange03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRoot03", "NotifyRoot03");
    keyboardSession->keyboardCallback_->onNotifyOccupiedAreaChange = [](const sptr<OccupiedAreaChangeInfo>& info) {};
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    keyboardSession->NotifyRootSceneOccupiedAreaChange(info);
}

HWTEST_F(KeyboardSessionTest5, IsNeedRaiseSubWindow01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsNeedRaiseSub01", "IsNeedRaiseSub01");
    auto callingSession = GetSceneSession("Calling", "Calling");
    callingSession->GetSessionProperty()->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    WSRect rect = {0, 0, 500, 500};
    bool result = keyboardSession->IsNeedRaiseSubWindow(callingSession, rect);
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest5, IsNeedRaiseSubWindow02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsNeedRaiseSub02", "IsNeedRaiseSub02");
    auto callingSession = GetSceneSession("Calling", "Calling");
    callingSession->GetSessionProperty()->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    WSRect rect = {0, 0, 500, 500};
    bool result = keyboardSession->IsNeedRaiseSubWindow(callingSession, rect);
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest5, IsNeedRaiseSubWindow03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsNeedRaiseSub03", "IsNeedRaiseSub03");
    auto callingSession = GetSceneSession("Calling", "Calling");
    callingSession->GetSessionProperty()->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    auto mainSession = GetSceneSession("Main", "Main");
    mainSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    callingSession->parentSession_ = mainSession;
    WSRect rect = {0, 0, 500, 500};
    mainSession->SetSessionRect(rect);
    bool result = keyboardSession->IsNeedRaiseSubWindow(callingSession, rect);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest5, HandleMoveDragSurfaceNode01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleMove01", "HandleMove01");
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG);
}

HWTEST_F(KeyboardSessionTest5, HandleMoveDragSurfaceNode02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleMove02", "HandleMove02");
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_MOVE);
}

HWTEST_F(KeyboardSessionTest5, HandleMoveDragSurfaceNode03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleMove03", "HandleMove03");
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_END);
}

HWTEST_F(KeyboardSessionTest5, HandleMoveDragSurfaceNode04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleMove04", "HandleMove04");
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::UNDEFINED);
}

HWTEST_F(KeyboardSessionTest5, HandleMoveDragSurfaceNode05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleMove05", "HandleMove05");
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::MOVE);
}

HWTEST_F(KeyboardSessionTest5, AdjustKeyboardLayout01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Adjust01", "Adjust01");
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    WSError result = keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, AdjustKeyboardLayout02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Adjust02", "Adjust02");
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    WSError result = keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, AdjustKeyboardLayout03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Adjust03", "Adjust03");
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    params.landscapeAvoidHeight_ = 100;
    params.portraitAvoidHeight_ = 150;
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    WSError result = keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, AdjustKeyboardLayout04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Adjust04", "Adjust04");
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    keyboardSession->adjustKeyboardLayoutFunc_ = [](const KeyboardLayoutParams& p) {};
    WSError result = keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, AdjustKeyboardLayout05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Adjust05", "Adjust05");
    KeyboardLayoutParams params;
    params.landscapeAvoidHeight_ = -1;
    params.portraitAvoidHeight_ = -1;
    WSError result = keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest5, NotifySessionRectChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRect01", "NotifyRect01");
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->NotifySessionRectChange(rect, SizeChangeReason::UNDEFINED);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, NotifySessionRectChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRect02", "NotifyRect02");
    WSRect rect = {0, 0, 1000, 800};
    keyboardSession->NotifySessionRectChange(rect, SizeChangeReason::MOVE);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, NotifySessionRectChange03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRect03", "NotifyRect03");
    WSRect rect = {50, 50, 600, 400};
    keyboardSession->NotifySessionRectChange(rect, SizeChangeReason::RESIZE);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, NotifySessionRectChange04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRect04", "NotifyRect04");
    WSRect rect = {0, 0, 800, 600};
    DisplayId displayId = 1;
    keyboardSession->NotifySessionRectChange(rect, SizeChangeReason::UNDEFINED, displayId);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, GetSceneSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScene01", "GetScene01");
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    sptr<SceneSession> session = keyboardSession->GetSceneSession(100);
    EXPECT_EQ(session, nullptr);
}

HWTEST_F(KeyboardSessionTest5, GetSceneSession02, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest5, GetSceneSession03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScene03", "GetScene03");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    sptr<SceneSession> session = keyboardSession->GetSceneSession(INVALID_WINDOW_ID);
    EXPECT_EQ(session, nullptr);
}

HWTEST_F(KeyboardSessionTest5, GetFocusedSessionId01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused01", "GetFocused01");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = nullptr;
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, INVALID_WINDOW_ID);
}

HWTEST_F(KeyboardSessionTest5, GetFocusedSessionId02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused02", "GetFocused02");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return 100; };
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, 100);
}

HWTEST_F(KeyboardSessionTest5, GetFocusedSessionId03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocused03", "GetFocused03");
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return INVALID_WINDOW_ID; };
    int32_t focusedId = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(focusedId, INVALID_WINDOW_ID);
}

HWTEST_F(KeyboardSessionTest5, SetSurfaceBounds01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds01", "SetBounds01");
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->SetSurfaceBounds(rect, false, false);
}

HWTEST_F(KeyboardSessionTest5, SetSurfaceBounds02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds02", "SetBounds02");
    keyboardSession->keyboardPanelSession_ = nullptr;
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->SetSurfaceBounds(rect, true, true);
}

HWTEST_F(KeyboardSessionTest5, SetSurfaceBounds03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds03", "SetBounds03");
    WSRect rect = {0, 0, 0, 0};
    keyboardSession->SetSurfaceBounds(rect, false, false);
}

HWTEST_F(KeyboardSessionTest5, SetSurfaceBounds04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds04", "SetBounds04");
    WSRect rect = {500, 600, 800, 400};
    keyboardSession->SetSurfaceBounds(rect, true, false);
}

HWTEST_F(KeyboardSessionTest5, GetRSTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetRSTrans01", "GetRSTrans01");
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    std::shared_ptr<RSTransaction> transaction = keyboardSession->GetRSTransaction();
    EXPECT_EQ(transaction, nullptr);
}

HWTEST_F(KeyboardSessionTest5, GetRSTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetRSTrans02", "GetRSTrans02");
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    std::shared_ptr<RSTransaction> transaction = keyboardSession->GetRSTransaction();
}

HWTEST_F(KeyboardSessionTest5, CloseRSTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTrans01", "CloseRSTrans01");
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->CloseRSTransaction();
}

HWTEST_F(KeyboardSessionTest5, CloseRSTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTrans02", "CloseRSTrans02");
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->CloseRSTransaction();
    EXPECT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);
}

HWTEST_F(KeyboardSessionTest5, CloseRSTransaction03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTrans03", "CloseRSTrans03");
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->CloseRSTransaction();
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, GetSessionScreenName01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName01", "GetScreenName01");
    keyboardSession->property_ = nullptr;
    std::string screenName = keyboardSession->GetSessionScreenName();
    EXPECT_EQ(screenName, "");
}

HWTEST_F(KeyboardSessionTest5, GetSessionScreenName02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName02", "GetScreenName02");
    keyboardSession->property_->SetDisplayId(DISPLAY_ID_INVALID);
    std::string screenName = keyboardSession->GetSessionScreenName();
    EXPECT_EQ(screenName, "");
}

HWTEST_F(KeyboardSessionTest5, CalculateOccupiedAreaAfterUIRefresh01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcOccupiedUI01", "CalcOccupiedUI01");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, CalculateOccupiedAreaAfterUIRefresh02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcOccupiedUI02", "CalcOccupiedUI02");
    keyboardSession->stateChanged_ = true;
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, CalculateOccupiedAreaAfterUIRefresh03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcOccupiedUI03", "CalcOccupiedUI03");
    keyboardSession->isVisible_ = true;
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "Calling";
        info.bundleName_ = "Calling";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        return session;
    };
    keyboardSession->property_->SetCallingSessionId(100);
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, CalculateOccupiedAreaAfterUIRefresh04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcOccupiedUI04", "CalcOccupiedUI04");
    keyboardSession->stateChanged_ = false;
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, CalculateOccupiedAreaAfterUIRefresh05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcOccupiedUI05", "CalcOccupiedUI05");
    keyboardSession->isVisible_ = false;
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, SetCallingSessionId01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingId01", "SetCallingId01");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return INVALID_WINDOW_ID; };
    keyboardSession->SetCallingSessionId(100);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, SetCallingSessionId02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingId02", "SetCallingId02");
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
}

HWTEST_F(KeyboardSessionTest5, SetCallingSessionId03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingId03", "SetCallingId03");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "Calling";
        info.bundleName_ = "Calling";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        session->persistentId_ = persistentId;
        return session;
    };
    keyboardSession->keyboardCallback_->onCallingSessionIdChange = [](uint32_t id) {};
    keyboardSession->property_->SetCallingSessionId(50);
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->SetCallingSessionId(100);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, NotifyOccupiedAreaChanged01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyOccupied01", "NotifyOccupied01");
    auto callingSession = GetSceneSession("Calling", "Calling");
    callingSession->sessionInfo_.isSystem_ = true;
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, info, false, nullptr);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, NotifyOccupiedAreaChanged02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyOccupied02", "NotifyOccupied02");
    auto callingSession = GetSceneSession("Calling", "Calling");
    callingSession->sessionInfo_.isSystem_ = false;
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, info, true, nullptr);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, CalculateOccupiedArea01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcOccupied01", "CalcOccupied01");
    sptr<SceneSession> callingSession = nullptr;
    WSRect callingRect = {0, 0, 500, 500};
    WSRect panelRect = {0, 400, 500, 200};
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    bool result = keyboardSession->CalculateOccupiedArea(callingSession, callingRect, panelRect, info);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest5, CalculateOccupiedArea02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcOccupied02", "CalcOccupied02");
    auto callingSession = GetSceneSession("Calling", "Calling");
    callingSession->SetLastSafeRect({0, 0, 100, 100});
    WSRect callingRect = {0, 300, 500, 500};
    WSRect panelRect = {0, 400, 500, 200};
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    bool result = keyboardSession->CalculateOccupiedArea(callingSession, callingRect, panelRect, info);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, RaiseCallingSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("RaiseCalling01", "RaiseCalling01");
    keyboardSession->keyboardAvoidAreaActive_ = false;
    auto callingSession = GetSceneSession("Calling", "Calling");
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    bool result = keyboardSession->RaiseCallingSession(callingSession, info);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest5, RaiseCallingSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("RaiseCalling02", "RaiseCalling02");
    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->state_ = SessionState::STATE_BACKGROUND;
    auto callingSession = GetSceneSession("Calling", "Calling");
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    bool result = keyboardSession->RaiseCallingSession(callingSession, info);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest5, UseFocusIdIfCallingSessionIdInvalid01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UseFocus01", "UseFocus01");
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() { return INVALID_WINDOW_ID; };
    keyboardSession->property_->SetCallingSessionId(INVALID_WINDOW_ID);
    uint32_t callingSessionId = 100;
    keyboardSession->UseFocusIdIfCallingSessionIdInvalid(callingSessionId);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, UseFocusIdIfCallingSessionIdInvalid02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UseFocus02", "UseFocus02");
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
}

HWTEST_F(KeyboardSessionTest5, HandleCrossScreenChild01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CrossScreen01", "CrossScreen01");
    keyboardSession->moveDragController_ = nullptr;
    keyboardSession->HandleCrossScreenChild(true);
}

HWTEST_F(KeyboardSessionTest5, HandleCrossScreenChild02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CrossScreen02", "CrossScreen02");
    keyboardSession->moveDragController_ = nullptr;
    keyboardSession->HandleCrossScreenChild(false);
}

HWTEST_F(KeyboardSessionTest5, PostKeyboardAnimationSyncTimeoutTask01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("PostTimeout01", "PostTimeout01");
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest5, PostKeyboardAnimationSyncTimeoutTask02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("PostTimeout02", "PostTimeout02");
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
    usleep(WAIT_ASYNC_US);
}

} // namespace
} // namespace Rosen
} // namespace OHOS