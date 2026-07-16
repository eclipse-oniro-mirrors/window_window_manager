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

#define PRIVATE public
#include "session/host/include/keyboard_session.h"
#undef PRIVATE

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

constexpr int WAIT_ASYNC_US = 1000000;

class KeyboardSessionTest6 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<KeyboardSession> GetKeyboardSession(const std::string& abilityName, const std::string& bundleName);
    sptr<SceneSession> GetSceneSession(const std::string& abilityName, const std::string& bundleName);
    ScreenSessionManagerClient* screenSessionManagerClient_;
};

void KeyboardSessionTest6::SetUpTestCase() {}
void KeyboardSessionTest6::TearDownTestCase() {}
void KeyboardSessionTest6::SetUp() {}
void KeyboardSessionTest6::TearDown() {}

sptr<KeyboardSession> KeyboardSessionTest6::GetKeyboardSession(const std::string& abilityName,
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

sptr<SceneSession> KeyboardSessionTest6::GetSceneSession(const std::string& abilityName, const std::string& bundleName)
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

HWTEST_F(KeyboardSessionTest6, ConstructorWithAllCallbacks, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ConstructorWithAllCallbacks";
    info.bundleName_ = "ConstructorWithAllCallbacks";
    info.persistentId_ = 200;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_NE(keyboardSession->keyboardCallback_, nullptr);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
}

HWTEST_F(KeyboardSessionTest6, ShowWithNullProperty, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowNullProperty", "ShowNullProperty");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_ = nullptr;
    sptr<WindowSessionProperty> property = nullptr;
    EXPECT_EQ(keyboardSession->Show(property), WSError::WS_ERROR_NULLPTR);
}

HWTEST_F(KeyboardSessionTest6, ShowWithValidPropertyAndSurfaceNode, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowValidProperty", "ShowValidProperty");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    keyboardSession->SetSurfaceNode(surfaceNode);
    EXPECT_EQ(keyboardSession->Show(property), WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, ShowWithKeyboardLayoutParamsSet, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowWithParams", "ShowWithParams");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    params.landscapeAvoidHeight_ = 100;
    params.portraitAvoidHeight_ = 150;
    property->SetKeyboardLayoutParams(params);
    keyboardSession->property_ = property;
    EXPECT_EQ(keyboardSession->Show(property), WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, ShowWithPhoneWindowUIType, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowPhoneUI", "ShowPhoneUI");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_EQ(keyboardSession->Show(property), WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, ShowWithPadWindowUIType, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ShowPadUI", "ShowPadUI");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_EQ(keyboardSession->Show(property), WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, HideWithSessionStageNull, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideStageNull", "HideStageNull");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->sessionStage_ = nullptr;
    EXPECT_EQ(keyboardSession->Hide(), WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, HideWithSystemKeyboardTrue, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HideSystemKB", "HideSystemKB");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    EXPECT_EQ(keyboardSession->Hide(), WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, DisconnectWithKeyboardPanelSession, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectPanel";
    info.bundleName_ = "DisconnectPanel";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    SessionInfo panelInfo;
    panelInfo.abilityName_ = "PanelSession";
    panelInfo.bundleName_ = "PanelSession";
    sptr<SceneSession> panelSession = sptr<SceneSession>::MakeSptr(panelInfo, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    WSError result = keyboardSession->Disconnect();
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, DisconnectWithAnimationInfo, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisconnectAnim";
    info.bundleName_ = "DisconnectAnim";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    WindowAnimationInfo animationInfo;
    CallingWindowInfoData callingWindowInfoData;
    WSRect rect = {0, 0, 0, 0};
    keyboardSession->CloseKeyboardSyncTransaction(rect, false, animationInfo, callingWindowInfoData);
    WSError result = keyboardSession->Disconnect();
    ASSERT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, GetSceneSessionWithCallback, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSceneCb", "GetSceneCb");
    ASSERT_NE(keyboardSession, nullptr);
    auto sceneSession = GetSceneSession("TestScene", "TestScene");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->persistentId_ = 300;
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [sceneSession](uint32_t persistentId) -> sptr<SceneSession> {
        if (persistentId == 300) {
            return sceneSession;
        }
        return nullptr;
    };
    sptr<SceneSession> result = keyboardSession->GetSceneSession(300);
    EXPECT_EQ(result, sceneSession);
}

HWTEST_F(KeyboardSessionTest6, GetSceneSessionWithInvalidId, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSceneInvalid", "GetSceneInvalid");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    sptr<SceneSession> result = keyboardSession->GetSceneSession(INVALID_WINDOW_ID);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(KeyboardSessionTest6, GetFocusedSessionIdWithCallback, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocusedCb", "GetFocusedCb");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() -> int32_t { return 500; };
    int32_t result = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(result, 500);
}

HWTEST_F(KeyboardSessionTest6, GetFocusedSessionIdWithNullCallback, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetFocusedNull", "GetFocusedNull");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_ = nullptr;
    int32_t result = keyboardSession->GetFocusedSessionId();
    EXPECT_EQ(result, INVALID_WINDOW_ID);
}

HWTEST_F(KeyboardSessionTest6, SetCallingSessionIdWithValidSession, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingValid", "SetCallingValid");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->persistentId_ = 600;
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [callingSession](int32_t persistentId) -> sptr<SceneSession> {
        if (persistentId == 600) {
            return callingSession;
        }
        return nullptr;
    };
    keyboardSession->keyboardCallback_->onCallingSessionIdChange = [](int32_t id) {};
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->SetCallingSessionId(600);
    EXPECT_EQ(keyboardSession->GetCallingSessionId(), 600);
}

HWTEST_F(KeyboardSessionTest6, SetCallingSessionIdWithNullCallback, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetCallingNullCb", "SetCallingNullCb");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    keyboardSession->SetCallingSessionId(100);
    EXPECT_EQ(keyboardSession->GetCallingSessionId(), INVALID_SESSION_ID);
}

HWTEST_F(KeyboardSessionTest6, UseFocusIdIfCallingSessionIdInvalid01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UseFocus01", "UseFocus01");
    ASSERT_NE(keyboardSession, nullptr);
    auto sceneSession = GetSceneSession("Scene", "Scene");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->persistentId_ = 700;
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [sceneSession](uint32_t id) -> sptr<SceneSession> {
        if (id == 700) {
            return sceneSession;
        }
        return nullptr;
    };
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() -> int32_t { return 700; };
    keyboardSession->GetSessionProperty()->SetCallingSessionId(INVALID_WINDOW_ID);
    keyboardSession->UseFocusIdIfCallingSessionIdInvalid(700);
    EXPECT_EQ(keyboardSession->GetCallingSessionId(), 700);
}

HWTEST_F(KeyboardSessionTest6, UseFocusIdIfCallingSessionIdInvalid02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UseFocus02", "UseFocus02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    keyboardSession->GetSessionProperty()->SetCallingSessionId(INVALID_WINDOW_ID);
    keyboardSession->UseFocusIdIfCallingSessionIdInvalid(INVALID_WINDOW_ID);
    EXPECT_EQ(keyboardSession->GetCallingSessionId(), INVALID_WINDOW_ID);
}

HWTEST_F(KeyboardSessionTest6, NotifySystemKeyboardAvoidChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid01", "NotifySysAvoid01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    keyboardSession->keyboardCallback_->onSystemKeyboardAvoidChange =
        [](DisplayId id, SystemKeyboardAvoidChangeReason reason) {};
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
    EXPECT_EQ(keyboardSession->keyboardAvoidAreaActive_, true);
}

HWTEST_F(KeyboardSessionTest6, NotifySystemKeyboardAvoidChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid02", "NotifySysAvoid02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetIsSystemKeyboard(false);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
    EXPECT_EQ(keyboardSession->keyboardAvoidAreaActive_, true);
}

HWTEST_F(KeyboardSessionTest6, NotifySystemKeyboardAvoidChange03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifySysAvoid03", "NotifySysAvoid03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_ = nullptr;
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
    EXPECT_EQ(keyboardSession->keyboardAvoidAreaActive_, true);
}

HWTEST_F(KeyboardSessionTest6, NotifyRootSceneOccupiedAreaChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRoot01", "NotifyRoot01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onNotifyOccupiedAreaChange = nullptr;
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    ASSERT_NE(info, nullptr);
    keyboardSession->NotifyRootSceneOccupiedAreaChange(info);
}

HWTEST_F(KeyboardSessionTest6, NotifyRootSceneOccupiedAreaChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRoot02", "NotifyRoot02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onNotifyOccupiedAreaChange =
        [](const sptr<OccupiedAreaChangeInfo>& info) {};
    keyboardSession->GetSessionProperty()->SetDisplayId(100);
    sptr<OccupiedAreaChangeInfo> info = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    ASSERT_NE(info, nullptr);
    keyboardSession->NotifyRootSceneOccupiedAreaChange(info);
}

HWTEST_F(KeyboardSessionTest6, NotifyRootSceneOccupiedAreaChange03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRoot03", "NotifyRoot03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_ = nullptr;
    sptr<OccupiedAreaChangeInfo> info = nullptr;
    keyboardSession->NotifyRootSceneOccupiedAreaChange(info);
}

HWTEST_F(KeyboardSessionTest6, RestoreCallingSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore01", "Restore01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = true;
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->SetOriPosYBeforeRaisedByKeyboard(100);
    callingSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    WSRect lastSafeRect = {10, 20, 30, 40};
    callingSession->SetLastSafeRect(lastSafeRect);
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [callingSession](int32_t id) -> sptr<SceneSession> { return callingSession; };
    keyboardSession->RestoreCallingSession(100, nullptr);
}

HWTEST_F(KeyboardSessionTest6, RestoreCallingSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore02", "Restore02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = false;
    keyboardSession->RestoreCallingSession(100, nullptr);
}

HWTEST_F(KeyboardSessionTest6, RestoreCallingSession03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Restore03", "Restore03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardAvoidAreaActive_ = true;
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    keyboardSession->RestoreCallingSession(100, nullptr);
}

HWTEST_F(KeyboardSessionTest6, ChangeKeyboardEffectOption01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect01", "ChangeEffect01");
    keyboardSession->changeKeyboardEffectOptionFunc_ = nullptr;
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::NON_IMMERSIVE_MODE;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, ChangeKeyboardEffectOption02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ChangeEffect02", "ChangeEffect02");
    bool callbackInvoked = false;
    keyboardSession->changeKeyboardEffectOptionFunc_ =
        [&callbackInvoked](const KeyboardEffectOption& option) { callbackInvoked = true; };
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::DARK_IMMERSIVE_MODE;
    WSError result = keyboardSession->ChangeKeyboardEffectOption(effectOption);
    EXPECT_EQ(result, WSError::WS_OK);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(callbackInvoked, true);
}

HWTEST_F(KeyboardSessionTest6, AdjustKeyboardLayout01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Adjust01", "Adjust01");
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    params.landscapeAvoidHeight_ = 200;
    params.portraitAvoidHeight_ = 250;
    keyboardSession->adjustKeyboardLayoutFunc_ = nullptr;
    WSError result = keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, AdjustKeyboardLayout02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Adjust02", "Adjust02");
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    keyboardSession->adjustKeyboardLayoutFunc_ = [](const KeyboardLayoutParams& params) {};
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    WSError result = keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, AdjustKeyboardLayout03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Adjust03", "Adjust03");
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    params.landscapeAvoidHeight_ = -1;
    params.portraitAvoidHeight_ = -1;
    WSError result = keyboardSession->AdjustKeyboardLayout(params);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, CheckIfNeedRaiseCallingSession01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise01", "CheckRaise01");
    sptr<SceneSession> callingSession = nullptr;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, true);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, CheckIfNeedRaiseCallingSession02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise02", "CheckRaise02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, true);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, CheckIfNeedRaiseCallingSession03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise03", "CheckRaise03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->GetSessionProperty()->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    callingSession->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, false);
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest6, CheckIfNeedRaiseCallingSession04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CheckRaise04", "CheckRaise04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->GetSessionProperty()->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    callingSession->systemConfig_.freeMultiWindowEnable_ = true;
    callingSession->systemConfig_.freeMultiWindowSupport_ = true;
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    bool result = keyboardSession->CheckIfNeedRaiseCallingSession(callingSession, true);
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest6, IsNeedRaiseSubWindow01, TestSize.Level1)
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

HWTEST_F(KeyboardSessionTest6, IsNeedRaiseSubWindow02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsNeedRaiseSub02", "IsNeedRaiseSub02");
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

HWTEST_F(KeyboardSessionTest6, CallingWindowStateChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CallingState01", "CallingState01");
    ASSERT_NE(keyboardSession, nullptr);
    CallingWindowInfoData callingWindowInfoData;
    callingWindowInfoData.callingWindowState = CallingWindowState::WINDOW_IN_NORMAL;
    callingWindowInfoData.scaleX = 1.0f;
    callingWindowInfoData.scaleY = 1.0f;
    keyboardSession->CallingWindowStateChange(callingWindowInfoData);
    EXPECT_EQ(keyboardSession->callingWindowInfoData_.callingWindowState, CallingWindowState::WINDOW_IN_NORMAL);
}

HWTEST_F(KeyboardSessionTest6, CallingWindowStateChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CallingState02", "CallingState02");
    ASSERT_NE(keyboardSession, nullptr);
    CallingWindowInfoData callingWindowInfoData;
    callingWindowInfoData.callingWindowState = CallingWindowState::WINDOW_IN_AI;
    callingWindowInfoData.scaleX = 0.73f;
    callingWindowInfoData.scaleY = 0.73f;
    keyboardSession->CallingWindowStateChange(callingWindowInfoData);
    EXPECT_EQ(keyboardSession->callingWindowInfoData_.callingWindowState, CallingWindowState::WINDOW_IN_AI);
}

HWTEST_F(KeyboardSessionTest6, OpenKeyboardSyncTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("OpenSync01", "OpenSync01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(false);
    keyboardSession->OpenKeyboardSyncTransaction();
    EXPECT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, true);
}

HWTEST_F(KeyboardSessionTest6, OpenKeyboardSyncTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("OpenSync02", "OpenSync02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    keyboardSession->OpenKeyboardSyncTransaction();
    EXPECT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, true);
}

HWTEST_F(KeyboardSessionTest6, CloseKeyboardSyncTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseSync01", "CloseSync01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    WSRect rect = {0, 0, 0, 0};
    WindowAnimationInfo animationInfo;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CloseKeyboardSyncTransaction(rect, true, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);
}

HWTEST_F(KeyboardSessionTest6, CloseKeyboardSyncTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseSync02", "CloseSync02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(false);
    WSRect rect = {100, 200, 500, 300};
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 10;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CloseKeyboardSyncTransaction(rect, false, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);
}

HWTEST_F(KeyboardSessionTest6, RecalculatePanelRectForAvoidArea01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Recalc01", "Recalc01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.landscapeAvoidHeight_ = 100;
    keyboardSession->property_->keyboardLayoutParams_.portraitAvoidHeight_ = 150;
    WSRect panelRect = {0, 500, 1000, 300};
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.posY_, 650);
}

HWTEST_F(KeyboardSessionTest6, RecalculatePanelRectForAvoidArea02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("Recalc02", "Recalc02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.landscapeAvoidHeight_ = -1;
    keyboardSession->property_->keyboardLayoutParams_.portraitAvoidHeight_ = -1;
    WSRect panelRect = {0, 500, 1000, 300};
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.height_, 300);
}

HWTEST_F(KeyboardSessionTest6, UpdateSizeChangeReason01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason01", "UpdateReason01");
    ASSERT_NE(keyboardSession, nullptr);
    auto panelSession = GetSceneSession("Panel", "Panel");
    ASSERT_NE(panelSession, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_START);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, UpdateSizeChangeReason02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateReason02", "UpdateReason02");
    ASSERT_NE(keyboardSession, nullptr);
    auto panelSession = GetSceneSession("Panel", "Panel");
    ASSERT_NE(panelSession, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    WSError result = keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_END);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, GetPanelRect01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect01", "GetPanelRect01");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.posX_, 0);
    EXPECT_EQ(panelRect.posY_, 0);
}

HWTEST_F(KeyboardSessionTest6, GetPanelRect02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect02", "GetPanelRect02");
    ASSERT_NE(keyboardSession, nullptr);
    auto panelSession = GetSceneSession("Panel", "Panel");
    ASSERT_NE(panelSession, nullptr);
    WSRect rect = {100, 200, 500, 300};
    panelSession->SetSessionRect(rect);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.posX_, 100);
    EXPECT_EQ(panelRect.posY_, 200);
}

HWTEST_F(KeyboardSessionTest6, SetSurfaceBounds01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds01", "SetBounds01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    WSRect rect = {0, 0, 500, 300};
    keyboardSession->SetSurfaceBounds(rect, false);
}

HWTEST_F(KeyboardSessionTest6, SetSurfaceBounds02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds02", "SetBounds02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    keyboardSession->keyboardPanelSession_ = nullptr;
    WSRect rect = {0, 0, 500, 300};
    keyboardSession->SetSurfaceBounds(rect, false);
}

HWTEST_F(KeyboardSessionTest6, SetSurfaceBounds03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetBounds03", "SetBounds03");
    ASSERT_NE(keyboardSession, nullptr);
    auto panelSession = GetSceneSession("Panel", "Panel");
    ASSERT_NE(panelSession, nullptr);
    struct RSSurfaceNodeConfig config;
    panelSession->surfaceNode_ = RSSurfaceNode::Create(config);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    WSRect rect = {0, 0, 500, 300};
    keyboardSession->SetSurfaceBounds(rect, false, false);
}

HWTEST_F(KeyboardSessionTest6, NotifyKeyboardPanelInfoChange01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel01", "NotifyPanel01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = false;
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
}

HWTEST_F(KeyboardSessionTest6, NotifyKeyboardPanelInfoChange02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyPanel02", "NotifyPanel02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(keyboardSession->sessionStage_, nullptr);
    WSRect rect = {100, 200, 500, 300};
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
}

HWTEST_F(KeyboardSessionTest6, ProcessKeyboardOccupiedAreaInfo01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ProcessOccupied01", "ProcessOccupied01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(100, true, false);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest6, ProcessKeyboardOccupiedAreaInfo02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ProcessOccupied02", "ProcessOccupied02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "Calling";
        info.bundleName_ = "Calling";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        return session;
    };
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(100, false, true);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest6, ForceProcessKeyboardOccupiedAreaInfo01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ForceProcess01", "ForceProcess01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isVisible_ = false;
    keyboardSession->ForceProcessKeyboardOccupiedAreaInfo();
}

HWTEST_F(KeyboardSessionTest6, ForceProcessKeyboardOccupiedAreaInfo02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ForceProcess02", "ForceProcess02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isVisible_ = true;
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    keyboardSession->ForceProcessKeyboardOccupiedAreaInfo();
}

HWTEST_F(KeyboardSessionTest6, EnableCallingSessionAvoidArea01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid01", "EnableAvoid01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = nullptr;
    keyboardSession->EnableCallingSessionAvoidArea();
}

HWTEST_F(KeyboardSessionTest6, EnableCallingSessionAvoidArea02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("EnableAvoid02", "EnableAvoid02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "Calling";
        info.bundleName_ = "Calling";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        return session;
    };
    keyboardSession->property_->SetCallingSessionId(100);
    keyboardSession->EnableCallingSessionAvoidArea();
}

HWTEST_F(KeyboardSessionTest6, SetKeyboardEffectOptionChangeListener01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetListener01", "SetListener01");
    ASSERT_NE(keyboardSession, nullptr);
    NotifyKeyboarEffectOptionChangeFunc func = [](const KeyboardEffectOption& option) {};
    keyboardSession->SetKeyboardEffectOptionChangeListener(func);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest6, SetKeyboardEffectOptionChangeListener02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetListener02", "SetListener02");
    ASSERT_NE(keyboardSession, nullptr);
    NotifyKeyboarEffectOptionChangeFunc func = nullptr;
    keyboardSession->SetKeyboardEffectOptionChangeListener(func);
    usleep(WAIT_ASYNC_US);
}

HWTEST_F(KeyboardSessionTest6, SetSkipSelfWhenShowOnVirtualScreen01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipVirtual01", "SkipVirtual01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->isSkipSelfWhenShowOnVirtualScreen_.load(), false);
}

HWTEST_F(KeyboardSessionTest6, SetSkipSelfWhenShowOnVirtualScreen02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipVirtual02", "SkipVirtual02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(false);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->isSkipSelfWhenShowOnVirtualScreen_.load(), false);
}

HWTEST_F(KeyboardSessionTest6, SetSkipEventOnCastPlus01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipCast01", "SkipCast01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetSkipEventOnCastPlus(true);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->isSkipEventOnCastPlus_, false);
}

HWTEST_F(KeyboardSessionTest6, SetSkipEventOnCastPlus02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SkipCast02", "SkipCast02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetSkipEventOnCastPlus(false);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->isSkipEventOnCastPlus_, false);
}

HWTEST_F(KeyboardSessionTest6, HandleKeyboardMoveDragEnd01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleDrag01", "HandleDrag01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->adjustKeyboardLayoutFunc_ = nullptr;
    WSRect rect = {50, 50, 900, 900};
    keyboardSession->HandleKeyboardMoveDragEnd(rect, SizeChangeReason::DRAG_END, -1);
}

HWTEST_F(KeyboardSessionTest6, HandleKeyboardMoveDragEnd02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleDrag02", "HandleDrag02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->adjustKeyboardLayoutFunc_ = [](const KeyboardLayoutParams& params) {};
    keyboardSession->sessionRectChangeFunc_ = [](const WSRect& rect, SizeChangeReason reason, DisplayId id) {};
    WSRect rect = {50, 50, 900, 900};
    keyboardSession->HandleKeyboardMoveDragEnd(rect, SizeChangeReason::DRAG_END, 11);
}

HWTEST_F(KeyboardSessionTest6, NotifyKeyboardAnimationWillBegin01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyAnim01", "NotifyAnim01");
    ASSERT_NE(keyboardSession, nullptr);
    bool isKeyboardShow = true;
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 100;
    keyboardSession->NotifyKeyboardAnimationWillBegin(isKeyboardShow, animationInfo);
}

HWTEST_F(KeyboardSessionTest6, NotifyKeyboardAnimationWillBegin02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyAnim02", "NotifyAnim02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo info;
        info.abilityName_ = "Calling";
        info.bundleName_ = "Calling";
        auto session = sptr<SceneSession>::MakeSptr(info, nullptr);
        return session;
    };
    bool isKeyboardShow = false;
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 200;
    keyboardSession->NotifyKeyboardAnimationWillBegin(isKeyboardShow, animationInfo);
}

HWTEST_F(KeyboardSessionTest6, HandleActionUpdateKeyboardTouchHotArea01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleTouch01", "HandleTouch01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    KeyboardTouchHotAreas keyboardTouchHotAreas;
    Rect rect = {800, 800, 1200, 1200};
    keyboardTouchHotAreas.landscapeKeyboardHotAreas_.push_back(rect);
    keyboardTouchHotAreas.portraitKeyboardHotAreas_.push_back(rect);
    property->SetKeyboardTouchHotAreas(keyboardTouchHotAreas);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    WMError result = keyboardSession->HandleActionUpdateKeyboardTouchHotArea(property, action);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_DISPLAY);
}

HWTEST_F(KeyboardSessionTest6, CalculateOccupiedAreaAfterUIRefresh01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcRefresh01", "CalcRefresh01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->dirtyFlags_ = 0;
    keyboardSession->isVisible_ = false;
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
}

HWTEST_F(KeyboardSessionTest6, CalculateOccupiedAreaAfterUIRefresh02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CalcRefresh02", "CalcRefresh02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t id) { return nullptr; };
    keyboardSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    keyboardSession->isVisible_ = true;
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
}

HWTEST_F(KeyboardSessionTest6, NotifyOccupiedAreaChanged01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyOccupied01", "NotifyOccupied01");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->sessionInfo_.isSystem_ = true;
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    ASSERT_NE(occupiedAreaInfo, nullptr);
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, occupiedAreaInfo, false, nullptr);
}

HWTEST_F(KeyboardSessionTest6, NotifyOccupiedAreaChanged02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyOccupied02", "NotifyOccupied02");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->sessionInfo_.isSystem_ = false;
    callingSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(callingSession->sessionStage_, nullptr);
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    ASSERT_NE(occupiedAreaInfo, nullptr);
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, occupiedAreaInfo, false, nullptr);
}

HWTEST_F(KeyboardSessionTest6, GetSessionScreenName01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName01", "GetScreenName01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_ = nullptr;
    std::string result = keyboardSession->GetSessionScreenName();
    EXPECT_EQ(result, "");
}

HWTEST_F(KeyboardSessionTest6, GetSessionScreenName02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetScreenName02", "GetScreenName02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->displayId_ = 100;
    std::string result = keyboardSession->GetSessionScreenName();
}

HWTEST_F(KeyboardSessionTest6, GetRSTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetRSTrans01", "GetRSTrans01");
    ASSERT_NE(keyboardSession, nullptr);
    auto rsTransaction = keyboardSession->GetRSTransaction();
    EXPECT_EQ(rsTransaction, nullptr);
}

HWTEST_F(KeyboardSessionTest6, CloseRSTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTrans01", "CloseRSTrans01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(false);
    keyboardSession->CloseRSTransaction();
    EXPECT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);
}

HWTEST_F(KeyboardSessionTest6, CloseRSTransaction02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTrans02", "CloseRSTrans02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    keyboardSession->CloseRSTransaction();
    EXPECT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);
}

HWTEST_F(KeyboardSessionTest6, PostKeyboardAnimationSyncTimeoutTask01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("PostTimeout01", "PostTimeout01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
}

HWTEST_F(KeyboardSessionTest6, IsVisibleForeground01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsVisibleFore01", "IsVisibleFore01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isVisible_ = true;
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    bool result = keyboardSession->IsVisibleForeground();
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest6, IsVisibleForeground02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsVisibleFore02", "IsVisibleFore02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isVisible_ = false;
    bool result = keyboardSession->IsVisibleForeground();
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, IsVisibleNotBackground01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsVisibleNotBack01", "IsVisibleNotBack01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isVisible_ = true;
    keyboardSession->state_ = SessionState::STATE_BACKGROUND;
    bool result = keyboardSession->IsVisibleNotBackground();
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest6, IsVisibleNotBackground02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsVisibleNotBack02", "IsVisibleNotBack02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isVisible_ = false;
    bool result = keyboardSession->IsVisibleNotBackground();
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, GetIsKeyboardSyncTransactionOpen01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSyncOpen01", "GetSyncOpen01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    bool result = keyboardSession->GetIsKeyboardSyncTransactionOpen();
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest6, GetIsKeyboardSyncTransactionOpen02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSyncOpen02", "GetSyncOpen02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(false);
    bool result = keyboardSession->GetIsKeyboardSyncTransactionOpen();
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, NotifyClientToUpdateRect01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRect01", "NotifyRect01");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {800, 800, 1200, 1200};
    WSError result = keyboardSession->NotifyClientToUpdateRect("test", rect, nullptr);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, NotifyClientToUpdateRect02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("NotifyRect02", "NotifyRect02");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = {800, 800, 1200, 1200};
    std::shared_ptr<RSTransaction> rsTransaction = nullptr;
    WSError result = keyboardSession->NotifyClientToUpdateRect("update", rect, rsTransaction);
    EXPECT_EQ(result, WSError::WS_OK);
}

HWTEST_F(KeyboardSessionTest6, SetSessionBlackListWhenShowAdd01, TestSize.Level1)
{
    SessionInfo keyboardInfo;
    keyboardInfo.abilityName_ = "BlackListAdd01";
    keyboardInfo.bundleName_ = "BlackListAdd01";
    auto keyboardSpecificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(keyboardSpecificCb, nullptr);
    auto keyboardCb = sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    ASSERT_NE(keyboardCb, nullptr);
    auto keyboardSession = sptr<KeyboardSession>::MakeSptr(keyboardInfo, keyboardSpecificCb, keyboardCb);
    ASSERT_NE(keyboardSession, nullptr);
    auto panelSession = GetSceneSession("Panel", "Panel");
    ASSERT_NE(panelSession, nullptr);
    keyboardSpecificCb->onAddSessionBlackList_ = [](int32_t id,
        const std::unordered_set<std::string>& tags) { return WMError::WM_OK; };
    keyboardSession->SetSessionBlackListWhenShow(true, panelSession);
}

HWTEST_F(KeyboardSessionTest6, SetSessionBlackListWhenShowRemove01, TestSize.Level1)
{
    SessionInfo keyboardInfo;
    keyboardInfo.abilityName_ = "BlackListRemove01";
    keyboardInfo.bundleName_ = "BlackListRemove01";
    auto keyboardSpecificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(keyboardSpecificCb, nullptr);
    auto keyboardCb = sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    ASSERT_NE(keyboardCb, nullptr);
    auto keyboardSession = sptr<KeyboardSession>::MakeSptr(keyboardInfo, keyboardSpecificCb, keyboardCb);
    ASSERT_NE(keyboardSession, nullptr);
    auto panelSession = GetSceneSession("Panel", "Panel");
    ASSERT_NE(panelSession, nullptr);
    keyboardSpecificCb->onRemoveSessionBlackList_ = [](int32_t id,
        const std::unordered_set<std::string>& tags) { return WMError::WM_OK; };
    keyboardSession->SetSessionBlackListWhenShow(false, panelSession);
}

HWTEST_F(KeyboardSessionTest6, GetSkipFlagForCallingSessionNull01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSkipNull01", "GetSkipNull01");
    ASSERT_NE(keyboardSession, nullptr);
    bool result = keyboardSession->GetSkipFlagForCallingSession(nullptr);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, GetSkipFlagForCallingSessionMain01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSkipMain01", "GetSkipMain01");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    callingSession->isSkipSelfWhenShowOnVirtualScreen_.store(true);
    bool result = keyboardSession->GetSkipFlagForCallingSession(callingSession);
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest6, HandleMoveDragSurfaceNode01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleMove01", "HandleMove01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG);
}

HWTEST_F(KeyboardSessionTest6, HandleMoveDragSurfaceNode02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleMove02", "HandleMove02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_MOVE);
}

HWTEST_F(KeyboardSessionTest6, HandleMoveDragSurfaceNode03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleMove03", "HandleMove03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::MOVE);
}

HWTEST_F(KeyboardSessionTest6, HandleMoveDragSurfaceNode04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleMove04", "HandleMove04");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::DRAG_END);
}

HWTEST_F(KeyboardSessionTest6, HandleMoveDragSurfaceNode05, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleMove05", "HandleMove05");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->HandleMoveDragSurfaceNode(SizeChangeReason::UNDEFINED);
}

HWTEST_F(KeyboardSessionTest6, IsNeedProcessKeyboardOccupiedAreaInfo01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsNeedProcess01", "IsNeedProcess01");
    ASSERT_NE(keyboardSession, nullptr);
    KeyboardLayoutParams params;
    KeyboardLayoutParams lastParams;
    lastParams.landscapeAvoidHeight_ = 100;
    lastParams.portraitAvoidHeight_ = 150;
    keyboardSession->property_->SetKeyboardLayoutParams(lastParams);
    params.landscapeAvoidHeight_ = 100;
    params.portraitAvoidHeight_ = 150;
    bool result = keyboardSession->isNeedProcessKeyboardOccupiedAreaInfo(params, lastParams);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, IsNeedProcessKeyboardOccupiedAreaInfo02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsNeedProcess02", "IsNeedProcess02");
    ASSERT_NE(keyboardSession, nullptr);
    KeyboardLayoutParams params;
    KeyboardLayoutParams lastParams;
    lastParams.landscapeAvoidHeight_ = 100;
    lastParams.portraitAvoidHeight_ = 150;
    keyboardSession->property_->SetKeyboardLayoutParams(lastParams);
    params.landscapeAvoidHeight_ = 200;
    params.portraitAvoidHeight_ = 250;
    bool result = keyboardSession->isNeedProcessKeyboardOccupiedAreaInfo(params, lastParams);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, IsNeedProcessKeyboardOccupiedAreaInfo03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsNeedProcess03", "IsNeedProcess03");
    ASSERT_NE(keyboardSession, nullptr);
    KeyboardLayoutParams params;
    KeyboardLayoutParams lastParams;
    lastParams.landscapeAvoidHeight_ = -1;
    lastParams.portraitAvoidHeight_ = -1;
    keyboardSession->property_->SetKeyboardLayoutParams(lastParams);
    params.landscapeAvoidHeight_ = 100;
    params.portraitAvoidHeight_ = 150;
    bool result = keyboardSession->isNeedProcessKeyboardOccupiedAreaInfo(params, lastParams);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, IsNeedProcessKeyboardOccupiedAreaInfo04, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsNeedProcess04", "IsNeedProcess04");
    ASSERT_NE(keyboardSession, nullptr);
    KeyboardLayoutParams params;
    params.landscapeAvoidHeight_ = -1;
    params.portraitAvoidHeight_ = -1;
    KeyboardLayoutParams lastParams;
    lastParams.landscapeAvoidHeight_ = 100;
    lastParams.portraitAvoidHeight_ = 150;
    keyboardSession->property_->SetKeyboardLayoutParams(lastParams);
    bool result = keyboardSession->isNeedProcessKeyboardOccupiedAreaInfo(params, lastParams);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, ForceNotifyKeyboardOccupiedArea01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ForceNotify01", "ForceNotify01");
    ASSERT_NE(keyboardSession, nullptr);
    auto sceneSession = GetSceneSession("Scene", "Scene");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->ForceNotifyKeyboardOccupiedArea();
}

HWTEST_F(KeyboardSessionTest6, ForceNotifyKeyboardOccupiedArea02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ForceNotify02", "ForceNotify02");
    ASSERT_NE(keyboardSession, nullptr);
    auto sceneSession = GetSceneSession("Scene", "Scene");
    ASSERT_NE(sceneSession, nullptr);
    bool callbackInvoked = false;
    sceneSession->RegisterNotifyOccupiedAreaChangeCallback([&](DisplayId displayId) { callbackInvoked = true; });
    sceneSession->ForceNotifyKeyboardOccupiedArea();
    EXPECT_EQ(callbackInvoked, true);
}

HWTEST_F(KeyboardSessionTest6, ForceNotifyKeyboardOccupiedArea03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("ForceNotify03", "ForceNotify03");
    ASSERT_NE(keyboardSession, nullptr);
    auto sceneSession = GetSceneSession("Scene", "Scene");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->RegisterNotifyOccupiedAreaChangeCallback(nullptr);
    sceneSession->ForceNotifyKeyboardOccupiedArea();
}

HWTEST_F(KeyboardSessionTest6, GetKeyboardGravity01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetGravity01", "GetGravity01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    SessionGravity gravity = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(gravity, SessionGravity::SESSION_GRAVITY_BOTTOM);
}

HWTEST_F(KeyboardSessionTest6, GetKeyboardGravity02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetGravity02", "GetGravity02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    SessionGravity gravity = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(gravity, SessionGravity::SESSION_GRAVITY_FLOAT);
}

HWTEST_F(KeyboardSessionTest6, GetKeyboardGravity03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetGravity03", "GetGravity03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_DEFAULT;
    SessionGravity gravity = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(gravity, SessionGravity::SESSION_GRAVITY_DEFAULT);
}

HWTEST_F(KeyboardSessionTest6, IsSystemKeyboard01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsSystemKB01", "IsSystemKB01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetIsSystemKeyboard(true);
    EXPECT_EQ(keyboardSession->IsSystemKeyboard(), true);
}

HWTEST_F(KeyboardSessionTest6, IsSystemKeyboard02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsSystemKB02", "IsSystemKB02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetIsSystemKeyboard(false);
    EXPECT_EQ(keyboardSession->IsSystemKeyboard(), false);
}

HWTEST_F(KeyboardSessionTest6, IsSystemKeyboard03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsSystemKB03", "IsSystemKB03");
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_EQ(keyboardSession->IsSystemKeyboard(), false);
}

HWTEST_F(KeyboardSessionTest6, SetScreenId01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetScreenId01", "SetScreenId01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(100);
    EXPECT_EQ(keyboardSession->property_->GetDisplayId(), 100);
}

HWTEST_F(KeyboardSessionTest6, SetScreenId02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetScreenId02", "SetScreenId02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(0);
    EXPECT_EQ(keyboardSession->property_->GetDisplayId(), 0);
}

HWTEST_F(KeyboardSessionTest6, SetScreenId03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetScreenId03", "SetScreenId03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_->SetDisplayId(UINT64_MAX);
    EXPECT_EQ(keyboardSession->property_->GetDisplayId(), UINT64_MAX);
}

HWTEST_F(KeyboardSessionTest6, GetSizeChangeReason01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSizeReason01", "GetSizeReason01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG);
    SizeChangeReason reason = keyboardSession->GetSizeChangeReason();
    EXPECT_EQ(reason, SizeChangeReason::DRAG);
}

HWTEST_F(KeyboardSessionTest6, GetSizeChangeReason02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSizeReason02", "GetSizeReason02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    SizeChangeReason reason = keyboardSession->GetSizeChangeReason();
    EXPECT_EQ(reason, SizeChangeReason::UNDEFINED);
}

HWTEST_F(KeyboardSessionTest6, GetSizeChangeReason03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSizeReason03", "GetSizeReason03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->UpdateSizeChangeReason(SizeChangeReason::MOVE);
    SizeChangeReason reason = keyboardSession->GetSizeChangeReason();
    EXPECT_EQ(reason, SizeChangeReason::MOVE);
}

HWTEST_F(KeyboardSessionTest6, GetCallingWindowInfoData01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCallingInfo01", "GetCallingInfo01");
    ASSERT_NE(keyboardSession, nullptr);
    CallingWindowInfoData info;
    info.callingWindowState = CallingWindowState::WINDOW_IN_NORMAL;
    info.scaleX = 1.0f;
    info.scaleY = 1.0f;
    keyboardSession->CallingWindowStateChange(info);
    CallingWindowInfoData result = keyboardSession->callingWindowInfoData_;
    EXPECT_EQ(result.callingWindowState, CallingWindowState::WINDOW_IN_NORMAL);
}

HWTEST_F(KeyboardSessionTest6, GetCallingWindowInfoData02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCallingInfo02", "GetCallingInfo02");
    ASSERT_NE(keyboardSession, nullptr);
    CallingWindowInfoData result = keyboardSession->callingWindowInfoData_;
    EXPECT_EQ(result.callingWindowState, CallingWindowState::WINDOW_IN_NORMAL);
}

HWTEST_F(KeyboardSessionTest6, GetCallingWindowInfoData03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetCallingInfo03", "GetCallingInfo03");
    ASSERT_NE(keyboardSession, nullptr);
    CallingWindowInfoData info;
    info.callingWindowState = CallingWindowState::WINDOW_IN_AI;
    info.scaleX = 0.5f;
    info.scaleY = 0.5f;
    keyboardSession->CallingWindowStateChange(info);
    CallingWindowInfoData result = keyboardSession->callingWindowInfoData_;
    EXPECT_EQ(result.callingWindowState, CallingWindowState::WINDOW_IN_AI);
}

HWTEST_F(KeyboardSessionTest6, IsKeyboardPanelEnabled01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsPanelEnabled01", "IsPanelEnabled01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    EXPECT_EQ(keyboardSession->isKeyboardPanelEnabled_, true);
}

HWTEST_F(KeyboardSessionTest6, IsKeyboardPanelEnabled02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsPanelEnabled02", "IsPanelEnabled02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = false;
    EXPECT_EQ(keyboardSession->isKeyboardPanelEnabled_, false);
}

HWTEST_F(KeyboardSessionTest6, SetEventHandler01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetHandler01", "SetHandler01");
    ASSERT_NE(keyboardSession, nullptr);
    auto runner = AppExecFwk::EventRunner::Create("TestHandler01");
    auto handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    keyboardSession->SetEventHandler(handler, nullptr);
}

HWTEST_F(KeyboardSessionTest6, SetEventHandler02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetHandler02", "SetHandler02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetEventHandler(nullptr, nullptr);
}

HWTEST_F(KeyboardSessionTest6, SetKeyboardLayoutParams01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetLayout01", "SetLayout01");
    ASSERT_NE(keyboardSession, nullptr);
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    params.landscapeAvoidHeight_ = 100;
    params.portraitAvoidHeight_ = 150;
    keyboardSession->property_->SetKeyboardLayoutParams(params);
    KeyboardLayoutParams result = keyboardSession->property_->GetKeyboardLayoutParams();
    EXPECT_EQ(result.gravity_, WindowGravity::WINDOW_GRAVITY_BOTTOM);
}

HWTEST_F(KeyboardSessionTest6, SetKeyboardLayoutParams02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetLayout02", "SetLayout02");
    ASSERT_NE(keyboardSession, nullptr);
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    params.landscapeAvoidHeight_ = 200;
    params.portraitAvoidHeight_ = 250;
    keyboardSession->property_->SetKeyboardLayoutParams(params);
    KeyboardLayoutParams result = keyboardSession->property_->GetKeyboardLayoutParams();
    EXPECT_EQ(result.gravity_, WindowGravity::WINDOW_GRAVITY_FLOAT);
}

HWTEST_F(KeyboardSessionTest6, SetKeyboardLayoutParams03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetLayout03", "SetLayout03");
    ASSERT_NE(keyboardSession, nullptr);
    KeyboardLayoutParams params;
    params.gravity_ = WindowGravity::WINDOW_GRAVITY_DEFAULT;
    params.landscapeAvoidHeight_ = 0;
    params.portraitAvoidHeight_ = 0;
    keyboardSession->property_->SetKeyboardLayoutParams(params);
    KeyboardLayoutParams result = keyboardSession->property_->GetKeyboardLayoutParams();
    EXPECT_EQ(result.landscapeAvoidHeight_, 0);
}

HWTEST_F(KeyboardSessionTest6, HandleActionUpdateTouchHotArea01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleTouchHot01", "HandleTouchHot01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    WMError result = keyboardSession->HandleActionUpdateTouchHotArea(property, action);
    EXPECT_EQ(result, WMError::WM_OK);
}

HWTEST_F(KeyboardSessionTest6, HandleActionUpdateTouchHotArea02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleTouchHot02", "HandleTouchHot02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    WMError result = keyboardSession->HandleActionUpdateTouchHotArea(property, action);
    EXPECT_EQ(result, WMError::WM_OK);
}

HWTEST_F(KeyboardSessionTest6, HandleActionUpdateTouchHotArea03, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleTouchHot03", "HandleTouchHot03");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->GetSessionProperty()->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    WMError result = keyboardSession->HandleActionUpdateTouchHotArea(property, action);
    EXPECT_EQ(result, WMError::WM_OK);
}

HWTEST_F(KeyboardSessionTest6, GetSkipFlagForCallingSessionSub01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSkipSub01", "GetSkipSub01");
    ASSERT_NE(keyboardSession, nullptr);
    auto mainSession = GetSceneSession("Main", "Main");
    ASSERT_NE(mainSession, nullptr);
    mainSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    mainSession->isSkipSelfWhenShowOnVirtualScreen_.store(true);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    callingSession->SetParentSession(mainSession);
    callingSession->isSkipSelfWhenShowOnVirtualScreen_.store(false);
    bool result = keyboardSession->GetSkipFlagForCallingSession(callingSession);
    EXPECT_EQ(result, true);
}

HWTEST_F(KeyboardSessionTest6, GetSkipFlagForCallingSessionSub02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSkipSub02", "GetSkipSub02");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    callingSession->SetParentSession(nullptr);
    callingSession->isSkipSelfWhenShowOnVirtualScreen_.store(true);
    bool result = keyboardSession->GetSkipFlagForCallingSession(callingSession);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, GetSkipFlagForCallingSessionNoMain01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSkipNoMain01", "GetSkipNoMain01");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    callingSession->isSkipSelfWhenShowOnVirtualScreen_.store(true);
    bool result = keyboardSession->GetSkipFlagForCallingSession(callingSession);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, GetSkipFlagForCallingSessionFalse01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSkipFalse01", "GetSkipFalse01");
    ASSERT_NE(keyboardSession, nullptr);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    callingSession->isSkipSelfWhenShowOnVirtualScreen_.store(false);
    bool result = keyboardSession->GetSkipFlagForCallingSession(callingSession);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, GetSkipFlagForCallingSessionFalse02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSkipFalse02", "GetSkipFalse02");
    ASSERT_NE(keyboardSession, nullptr);
    auto mainSession = GetSceneSession("Main", "Main");
    ASSERT_NE(mainSession, nullptr);
    mainSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    mainSession->isSkipSelfWhenShowOnVirtualScreen_.store(false);
    auto callingSession = GetSceneSession("Calling", "Calling");
    ASSERT_NE(callingSession, nullptr);
    callingSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    callingSession->SetParentSession(mainSession);
    bool result = keyboardSession->GetSkipFlagForCallingSession(callingSession);
    EXPECT_EQ(result, false);
}

HWTEST_F(KeyboardSessionTest6, IsCalculateOccupiedAreaWaitUntilDragEnd01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsCalcWait01", "IsCalcWait01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isCalculateOccupiedAreaWaitUntilDragEnd_ = true;
    EXPECT_EQ(keyboardSession->isCalculateOccupiedAreaWaitUntilDragEnd_, true);
}

HWTEST_F(KeyboardSessionTest6, IsCalculateOccupiedAreaWaitUntilDragEnd02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("IsCalcWait02", "IsCalcWait02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isCalculateOccupiedAreaWaitUntilDragEnd_ = false;
    EXPECT_EQ(keyboardSession->isCalculateOccupiedAreaWaitUntilDragEnd_, false);
}

HWTEST_F(KeyboardSessionTest6, GetStateChanged01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetStateChanged01", "GetStateChanged01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->stateChanged_ = true;
    EXPECT_EQ(keyboardSession->stateChanged_, true);
}

HWTEST_F(KeyboardSessionTest6, GetStateChanged02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetStateChanged02", "GetStateChanged02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->stateChanged_ = false;
    EXPECT_EQ(keyboardSession->stateChanged_, false);
}

HWTEST_F(KeyboardSessionTest6, SetSurfaceNode01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetSurface01", "SetSurface01");
    ASSERT_NE(keyboardSession, nullptr);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    keyboardSession->SetSurfaceNode(surfaceNode);
    EXPECT_NE(keyboardSession->surfaceNode_, nullptr);
}

HWTEST_F(KeyboardSessionTest6, SetSurfaceNode02, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetSurface02", "SetSurface02");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetSurfaceNode(nullptr);
    EXPECT_EQ(keyboardSession->surfaceNode_, nullptr);
}
} // namespace
} // namespace Rosen
} // namespace OHOS