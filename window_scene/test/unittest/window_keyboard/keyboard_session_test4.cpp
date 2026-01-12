/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
std::string g_logMsg;
void MyLogCallback(const LogType type, const LogLevel level,
    const unsigned int domain, const char *tag, const char *msg)
{
    g_logMsg += msg;
}
}

constexpr int WAIT_ASYNC_US = 1000000;
class KeyboardSessionTest4 : public testing::Test, public IScreenConnectionListener {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void OnScreenConnected(const sptr<ScreenSession>& screenSession) override;
    void OnScreenDisconnected(const sptr<ScreenSession>& screenSession) override;

private:
    sptr<KeyboardSession> GetKeyboardSession(const std::string& abilityName, const std::string& bundleName);
    sptr<SceneSession> GetSceneSession(const std::string& abilityName, const std::string& bundleName);
};

void KeyboardSessionTest4::SetUpTestCase() {}

void KeyboardSessionTest4::TearDownTestCase() {}

void KeyboardSessionTest4::SetUp() {}

void KeyboardSessionTest4::TearDown() {}

void KeyboardSessionTest4::OnScreenConnected(const sptr<ScreenSession>& screenSession) {}

void KeyboardSessionTest4::OnScreenDisconnected(const sptr<ScreenSession>& screenSession) {}

sptr<KeyboardSession> KeyboardSessionTest4::GetKeyboardSession(const std::string& abilityName,
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

sptr<SceneSession> KeyboardSessionTest4::GetSceneSession(const std::string& abilityName, const std::string& bundleName)
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
/**
 * @tc.name: BindKeyboardPanelSession
 * @tc.desc: BindKeyboardPanelSession
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, BindKeyboardPanelSession, TestSize.Level0)
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
HWTEST_F(KeyboardSessionTest4, GetKeyboardGravity01, TestSize.Level0)
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
HWTEST_F(KeyboardSessionTest4, NotifyKeyboardPanelInfoChange, TestSize.Level1)
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
 * @tc.name: Hide01
 * @tc.desc: test function: Hide
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, Hide01, TestSize.Level0)
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
 * @tc.name: OpenKeyboardSyncTransaction
 * @tc.desc: OpenKeyboardSyncTransaction
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, OpenKeyboardSyncTransaction, TestSize.Level1)
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
 * @tc.name: OpenKeyboardSyncTransaction01
 * @tc.desc: OpenKeyboardSyncTransaction
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, OpenKeyboardSyncTransaction01, TestSize.Level1)
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
 * @tc.name: CloseKeyBoardSyncTransaction01
 * @tc.desc: test function: CloseKeyBoardSyncTransaction
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CloseKeyBoardSyncTransaction01, TestSize.Level1)
{
    std::string abilityName = "CloseKeyBoardSyncTransaction01";
    std::string bundleName = "CloseKeyBoardSyncTransaction01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, bundleName);
    ASSERT_NE(keyboardSession, nullptr);

    WSRect keyboardPanelRect;
    bool isKeyboardShow = true;
    WindowAnimationInfo animationInfo;

    keyboardSession->specificCallback_->onUpdateAvoidArea_ = [](uint32_t callingSessionId) {};
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    // isKeyBoardSyncTransactionOpen_ is true
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, isKeyboardShow, animationInfo);
    usleep(WAIT_ASYNC_US);
    ASSERT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);

    ASSERT_NE(keyboardSession->keyboardCallback_, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo callingSessionInfo;
        callingSessionInfo.abilityName_ = "CallingSession";
        callingSessionInfo.bundleName_ = "CallingSession";
        auto callingSession = sptr<SceneSession>::MakeSptr(callingSessionInfo, nullptr);
        callingSession->persistentId_ = persistentId;
        return callingSession;
    };
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, isKeyboardShow, animationInfo);
    ASSERT_NE(keyboardSession->GetSceneSession(animationInfo.callingId), nullptr);
}

/**
 * @tc.name: CloseKeyboardSyncTransaction02
 * @tc.desc: test function: CloseKeyboardSyncTransaction
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CloseKeyboardSyncTransaction02, TestSize.Level1)
{
    std::string abilityName = "CloseKeyboardSyncTransaction02";
    std::string bundleName = "CloseKeyboardSyncTransaction02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, bundleName);
    ASSERT_NE(keyboardSession, nullptr);
    WSRect keyboardPanelRect = { 0, 0, 0, 0 };
    WindowAnimationInfo animationInfo;

    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo);
    usleep(WAIT_ASYNC_US);
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->property_->SetCallingSessionId(1);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo);
    usleep(WAIT_ASYNC_US);
    auto callingSessionId = keyboardSession->property_->GetCallingSessionId();
    ASSERT_EQ(callingSessionId, INVALID_WINDOW_ID);
}

/**
 * @tc.name: CloseKeyboardSyncTransaction03
 * @tc.name: CloseKeyboardSyncTransaction03
 * @tc.desc: test function: CloseKeyboardSyncTransaction
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CloseKeyboardSyncTransaction03, TestSize.Level1)
{
    std::string abilityName = "CloseKeyboardSyncTransaction03";
    std::string bundleName = "CloseKeyboardSyncTransaction03";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, bundleName);
    WSRect keyboardPanelRect = { 0, 0, 0, 0 };
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 3;
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        new (std::nothrow) KeyboardSession::KeyboardSessionCallback();
    keyboardSession->keyboardCallback_ = keyboardCallback;

    ASSERT_NE(keyboardSession->GetSessionProperty(), nullptr);
    keyboardSession->GetSessionProperty()->SetCallingSessionId(1);
    animationInfo.isGravityChanged = true;
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->GetCallingSessionId(), 1);

    animationInfo.isGravityChanged = false;
    keyboardSession->keyboardCallback_->isLastFrameLayoutFinished = []() { return true; };
    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->GetCallingSessionId(), 1);

    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->GetCallingSessionId(), INVALID_WINDOW_ID);

    animationInfo.isGravityChanged = true;
    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    keyboardSession->GetSessionProperty()->SetCallingSessionId(2);
    keyboardSession->keyboardCallback_->isLastFrameLayoutFinished = []() { return false; };
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->GetCallingSessionId(), 2);

    animationInfo.isGravityChanged = false;
    keyboardSession->state_ = SessionState::STATE_BACKGROUND;
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->GetCallingSessionId(), 2);

    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    keyboardSession->GetSessionProperty()->SetCallingSessionId(INVALID_WINDOW_ID);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->GetCallingSessionId(), 3);
}

/**
 * @tc.name: CloseKeyboardSyncTransaction04
 * @tc.desc: test function: CloseKeyboardSyncTransaction
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CloseKeyboardSyncTransaction04, TestSize.Level1)
{
    std::string abilityName = "CloseKeyboardSyncTransaction04";
    std::string bundleName = "CloseKeyboardSyncTransaction04";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, bundleName);
    WSRect keyboardPanelRect = { 0, 0, 0, 0 };
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 3;
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        new (std::nothrow) KeyboardSession::KeyboardSessionCallback();
    keyboardSession->keyboardCallback_ = keyboardCallback;

    animationInfo.isGravityChanged = false;
    keyboardSession->keyboardCallback_->isLastFrameLayoutFinished = []() { return true; };
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo);
    EXPECT_EQ(false, keyboardSession->stateChanged_);

    keyboardSession->keyboardCallback_->isLastFrameLayoutFinished = []() { return false; };
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo);
    EXPECT_EQ(true, keyboardSession->stateChanged_);

    keyboardSession->stateChanged_ = false;
    animationInfo.isGravityChanged = true;
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo);
    EXPECT_EQ(false, keyboardSession->stateChanged_);
}

/**
 * @tc.name: EnableCallingSessionAvoidArea01
 * @tc.desc: test function: EnableCallingSessionAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, EnableCallingSessionAvoidArea01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    keyboardCallback->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo callingSessionInfo;
        callingSessionInfo.abilityName_ = "CallingSession";
        callingSessionInfo.bundleName_ = "CallingSession";
        auto callingSession = sptr<SceneSession>::MakeSptr(callingSessionInfo, nullptr);
        callingSession->persistentId_ = persistentId;
        return callingSession;
    };

    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    SessionInfo info;
    info.abilityName_ = "keyboardSession";
    info.bundleName_ = "keyboardSession";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCallback, keyboardCallback);
    keyboardSession->EnableCallingSessionAvoidArea();
    EXPECT_TRUE(g_logMsg.find("Calling session is null") == std::string::npos);

    keyboardCallback->onGetSceneSession = [](uint32_t persistentId) {
        return nullptr;
    };
    keyboardSession->EnableCallingSessionAvoidArea();
    EXPECT_TRUE(g_logMsg.find("Calling session is null") != std::string::npos);
}

/**
 * @tc.name: CalculateScaledRect01
 * @tc.desc: test function: CalculateScaledRect
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CalculateScaledRect01, TestSize.Level1)
{
    std::string abilityName = "CalculateScaledRect01";
    std::string bundleName = "CalculateScaledRect01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, bundleName);

    WSRect rect = {500, 500, 2000, 2000};
    WSRect scaledRect = keyboardSession->CalculateScaledRect(rect, 1, 1);
    EXPECT_TRUE(scaledRect == rect);

    scaledRect = keyboardSession->CalculateScaledRect(rect, 1, 1.5);
    WSRect result = {500, 0, 2000, 3000};
    EXPECT_TRUE(scaledRect == result);

    scaledRect = keyboardSession->CalculateScaledRect(rect, 1.5, 1);
    result = {0, 500, 3000, 2000};
    EXPECT_TRUE(scaledRect == result);

    rect = {922, 277, 1274, 1387};
    scaledRect = keyboardSession->CalculateScaledRect(rect, 1.399529, 1.399423);
    result = {667, 0, 1783, 1941};
    EXPECT_TRUE(scaledRect == result);
}

/**
 * @tc.name: HandleActionUpdateKeyboardTouchHotArea01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, HandleActionUpdateKeyboardTouchHotArea01, TestSize.Level1)
{
    std::string abilityName = "HandleActionUpdateKeyboardTouchHotArea01";
    std::string bundleName = "HandleActionUpdateKeyboardTouchHotArea01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, bundleName);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardTouchHotAreas keyboardTouchHotAreas;
    Rect rect = {800, 800, 1200, 1200};
    keyboardTouchHotAreas.landscapeKeyboardHotAreas_.push_back(rect);
    keyboardTouchHotAreas.landscapePanelHotAreas_.push_back(rect);
    keyboardTouchHotAreas.portraitKeyboardHotAreas_.push_back(rect);
    keyboardTouchHotAreas.portraitPanelHotAreas_.push_back(rect);
    property->SetKeyboardTouchHotAreas(keyboardTouchHotAreas);
    SessionInfo info;
    info.abilityName_ = "keyboardPanelSession";
    info.bundleName_ = "keyboardPanelSession";
    sptr<SceneSession> keyboardPanelSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    keyboardSession->BindKeyboardPanelSession(keyboardPanelSession);
    keyboardSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    WMError ret = keyboardSession->HandleActionUpdateKeyboardTouchHotArea(property, action);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_TYPE, ret);
    keyboardSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ret = keyboardSession->HandleActionUpdateKeyboardTouchHotArea(property, action);
    ASSERT_EQ(WMError::WM_OK, ret);
    ret = keyboardSession->HandleActionUpdateKeyboardTouchHotArea(property, action);
    ASSERT_EQ(WMError::WM_OK, ret);
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession1 = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCallback);
    keyboardSession1->BindKeyboardPanelSession(keyboardPanelSession);
    keyboardSession1->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ret = keyboardSession1->HandleActionUpdateTouchHotArea(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: HandleActionUpdateKeyboardTouchHotArea02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, HandleActionUpdateKeyboardTouchHotArea02, TestSize.Level1)
{
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateKeyboardTouchHotArea02";
    info.bundleName_ = "HandleActionUpdateKeyboardTouchHotArea02";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCallback);
    sptr<SceneSession> keyboardPanelSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    keyboardSession->BindKeyboardPanelSession(keyboardPanelSession);
    keyboardSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    WMError ret = keyboardSession->HandleActionUpdateKeyboardTouchHotArea(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    RRect bounds;
    bounds.rect_.width_ = 200;
    bounds.rect_.height_ = 100;
    screenSession->GetScreenProperty().SetBounds(bounds);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);
    keyboardSession->property_->SetDisplayId(screenId);
    ret = keyboardSession->HandleActionUpdateKeyboardTouchHotArea(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);
    bounds.rect_.width_ = 100;
    bounds.rect_.height_ = 200;
    screenSession->GetScreenProperty().SetBounds(bounds);
    keyboardSession->property_->SetDisplayId(screenId);
    ret = keyboardSession->HandleActionUpdateKeyboardTouchHotArea(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);
    keyboardSession->property_->SetDisplayId(666);
    ret = keyboardSession->HandleActionUpdateKeyboardTouchHotArea(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ret = keyboardSession->HandleActionUpdateKeyboardTouchHotArea(property, action);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_DISPLAY);
}

/**
 * @tc.name: TestIsLandscapeWithValidSession
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, TestIsLandscapeWithValidSession, TestSize.Level1)
{
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession("IsLandscape", "IsLandscape");
    bool isLandscape = false;
    
    screenSessionManagerClient_->screenSessionMap_.clear();
    EXPECT_EQ(keyboardSession->IsLandscape(1234, isLandscape), WMError::WM_ERROR_INVALID_DISPLAY);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);
    EXPECT_EQ(keyboardSession->IsLandscape(0, isLandscape), WMError::WM_OK);
    screenSessionManagerClient_->screenSessionMap_.clear();
}
} // namespace
} // namespace Rosen
} // namespace OHOS
