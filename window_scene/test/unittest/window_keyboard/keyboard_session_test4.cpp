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

static sptr<ScenePersistence> scenePersistence = sptr<ScenePersistence>::MakeSptr("testBundleName", 1423);

void KeyboardSessionTest4::SetUpTestCase() {}

void KeyboardSessionTest4::TearDownTestCase()
{
    scenePersistence = nullptr;
}

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
    CallingWindowInfoData callingWindowInfoData;

    // isKeyBoardSyncTransactionOpen_ is false
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, isKeyboardShow, animationInfo,
        callingWindowInfoData);
    keyboardSession->OpenKeyboardSyncTransaction();

    // isKeyBoardSyncTransactionOpen_ is true
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, isKeyboardShow, animationInfo,
        callingWindowInfoData);
    keyboardSession->OpenKeyboardSyncTransaction();
    ASSERT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, true);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, isKeyboardShow, animationInfo,
        callingWindowInfoData);
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
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    keyboardSession->OpenKeyboardSyncTransaction();
    keyboardSession->isKeyboardSyncTransactionOpen_.store(false);
    keyboardSession->OpenKeyboardSyncTransaction();
    WSRect keyboardPanelRect = {0, 0, 0, 0};
    WindowAnimationInfo animationInfo;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo, callingWindowInfoData);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo, callingWindowInfoData);
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
    CallingWindowInfoData callingWindowInfoData;

    keyboardSession->specificCallback_->onUpdateAvoidArea_ = [](uint32_t callingSessionId) {};
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    // isKeyBoardSyncTransactionOpen_ is true
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, isKeyboardShow, animationInfo,
        callingWindowInfoData);
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
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, isKeyboardShow, animationInfo,
        callingWindowInfoData);
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
    CallingWindowInfoData callingWindowInfoData;

    keyboardSession->isKeyboardSyncTransactionOpen_.store(false);
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    keyboardSession->property_->SetCallingSessionId(1);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo, callingWindowInfoData);
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
    CallingWindowInfoData callingWindowInfoData;
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        new (std::nothrow) KeyboardSession::KeyboardSessionCallback();
    keyboardSession->keyboardCallback_ = keyboardCallback;

    ASSERT_NE(keyboardSession->GetSessionProperty(), nullptr);
    keyboardSession->GetSessionProperty()->SetCallingSessionId(1);
    animationInfo.isGravityChanged = true;
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo, callingWindowInfoData);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->GetCallingSessionId(), 1);

    animationInfo.isGravityChanged = false;
    keyboardSession->keyboardCallback_->isLastFrameLayoutFinished = []() { return true; };
    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo, callingWindowInfoData);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->GetCallingSessionId(), 1);

    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo, callingWindowInfoData);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->GetCallingSessionId(), INVALID_WINDOW_ID);

    animationInfo.isGravityChanged = true;
    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    keyboardSession->GetSessionProperty()->SetCallingSessionId(2);
    keyboardSession->keyboardCallback_->isLastFrameLayoutFinished = []() { return false; };
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo, callingWindowInfoData);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->GetCallingSessionId(), 2);

    animationInfo.isGravityChanged = false;
    keyboardSession->state_ = SessionState::STATE_BACKGROUND;
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo, callingWindowInfoData);
    EXPECT_EQ(keyboardSession->GetSessionProperty()->GetCallingSessionId(), 2);

    keyboardSession->state_ = SessionState::STATE_ACTIVE;
    keyboardSession->GetSessionProperty()->SetCallingSessionId(INVALID_WINDOW_ID);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo, callingWindowInfoData);
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
    CallingWindowInfoData callingWindowInfoData;
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        new (std::nothrow) KeyboardSession::KeyboardSessionCallback();
    keyboardSession->keyboardCallback_ = keyboardCallback;

    animationInfo.isGravityChanged = false;
    keyboardSession->keyboardCallback_->isLastFrameLayoutFinished = []() { return true; };
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo, callingWindowInfoData);
    EXPECT_EQ(false, keyboardSession->stateChanged_);

    keyboardSession->keyboardCallback_->isLastFrameLayoutFinished = []() { return false; };
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo, callingWindowInfoData);
    EXPECT_EQ(true, keyboardSession->stateChanged_);

    keyboardSession->stateChanged_ = false;
    animationInfo.isGravityChanged = true;
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo, callingWindowInfoData);
    EXPECT_EQ(false, keyboardSession->stateChanged_);
}

/**
 * @tc.name: CallingWindowStateChange
 * @tc.desc: test function: CallingWindowStateChange
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CallingWindowStateChange, TestSize.Level1)
{
    std::string abilityName = "CallingWindowStateChange";
    std::string bundleName = "CallingWindowStateChange";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, bundleName);
    CallingWindowInfoData callingWindowInfoData;
    callingWindowInfoData.callingWindowState = CallingWindowState::WINDOW_IN_NORMAL;
    keyboardSession->CallingWindowStateChange(callingWindowInfoData);
    EXPECT_EQ(CallingWindowState::WINDOW_IN_NORMAL, keyboardSession->callingWindowInfoData_.callingWindowState);

    callingWindowInfoData.scaleX = 2;
    keyboardSession->CallingWindowStateChange(callingWindowInfoData);
    EXPECT_TRUE(std::abs(keyboardSession->callingWindowInfoData_.scaleX - 2) < 0.001f);

    callingWindowInfoData.scaleY = 2;
    keyboardSession->CallingWindowStateChange(callingWindowInfoData);
    EXPECT_TRUE(std::abs(keyboardSession->callingWindowInfoData_.scaleY - 2) < 0.001f);
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
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession("TestIsLandscapeWithValidSession",
        "TestIsLandscapeWithValidSession");
    bool isLandscape = true;
    const ScreenId screenId1 = 11;
    const ScreenId invalidScreenId1 = 12;
    sptr<ScreenSession> screenSession1 = sptr<ScreenSession>::MakeSptr(screenId1,
        ScreenProperty(), screenId1);
    screenSessionManagerClient_->screenSessionMap_.clear();
    screenSessionManagerClient_->screenSessionMap_[screenId1] = screenSession1;
    screenSession1->property_.bounds_.rect_ = {1, 2, 300, 400};
    EXPECT_EQ(keyboardSession->IsLandscape(screenId1, isLandscape), WMError::WM_OK);
    EXPECT_FALSE(isLandscape);
    EXPECT_EQ(keyboardSession->IsLandscape(invalidScreenId1, isLandscape), WMError::WM_OK);
    EXPECT_FALSE(isLandscape);
    screenSessionManagerClient_->screenSessionMap_.clear();
    EXPECT_EQ(keyboardSession->IsLandscape(invalidScreenId1, isLandscape), WMError::WM_ERROR_INVALID_DISPLAY);
    const ScreenId screenId2 = 21;
    const ScreenId invalidScreenId2 = 22;
    sptr<ScreenSession> screenSession2 = sptr<ScreenSession>::MakeSptr(screenId2,
        ScreenProperty(), screenId2);
    screenSessionManagerClient_->screenSessionMap_.clear();
    screenSessionManagerClient_->screenSessionMap_[screenId2] = screenSession2;
    screenSession2->property_.bounds_.rect_ = {1, 2, 400, 300};
    EXPECT_EQ(keyboardSession->IsLandscape(screenId2, isLandscape), WMError::WM_OK);
    EXPECT_TRUE(isLandscape);
    EXPECT_EQ(keyboardSession->IsLandscape(invalidScreenId2, isLandscape), WMError::WM_OK);
    EXPECT_TRUE(isLandscape);
    screenSessionManagerClient_->screenSessionMap_.clear();
    EXPECT_EQ(keyboardSession->IsLandscape(invalidScreenId2, isLandscape), WMError::WM_ERROR_INVALID_DISPLAY);
}

/**
 * @tc.name: NotifyKeyboardAnimationWillBegin01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, NotifyKeyboardAnimationWillBegin01, TestSize.Level1)
{
    bool isKeyboardShow = true;
    WindowAnimationInfo animationInfo;
    sptr<SceneSession> sceneSession = GetSceneSession("TestSceneSession", "TestSceneSession");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->persistentId_ = 100;
    animationInfo.callingId = 100;
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession("NotifyKeyboardAnimationWillBegin",
        "NotifyKeyboardAnimationWillBegin");
    keyboardSession->NotifyKeyboardAnimationWillBegin(isKeyboardShow, animationInfo);
}

/**
 * @tc.name: NotifyKeyboardAnimationWillBegin02
 * @tc.desc: test function: NotifyKeyboardAnimationWillBegin02
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, NotifyKeyboardAnimationWillBegin02, TestSize.Level1)
{
    std::string abilityName = "NotifyKeyboardAnimationWillBegin02";
    std::string bundleName = "NotifyKeyboardAnimationWillBegin02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, bundleName);
    ASSERT_NE(keyboardSession, nullptr);

    bool isKeyboardShow = true;
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 100;

    keyboardSession->specificCallback_->onUpdateAvoidArea_ = [](uint32_t callingSessionId) {};
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    keyboardSession->NotifyKeyboardAnimationWillBegin(isKeyboardShow, animationInfo);
    usleep(WAIT_ASYNC_US);
    ASSERT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, true);

    ASSERT_NE(keyboardSession->keyboardCallback_, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo callingSessionInfo;
        callingSessionInfo.abilityName_ = "CallingSession";
        callingSessionInfo.bundleName_ = "CallingSession";
        auto callingSession = sptr<SceneSession>::MakeSptr(callingSessionInfo, nullptr);
        callingSession->persistentId_ = persistentId;
        return callingSession;
    };
    keyboardSession->NotifyKeyboardAnimationWillBegin(isKeyboardShow, animationInfo);
    ASSERT_NE(keyboardSession->GetSceneSession(animationInfo.callingId), nullptr);
}

/**
 * @tc.name: BindKeyboardPanelSession02
 * @tc.desc: test function: BindKeyboardPanelSession with valid panel and verify getter
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, BindKeyboardPanelSession02, TestSize.Level0)
{
    std::string abilityName = "BindKeyboardPanelSession02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_EQ(keyboardSession->GetKeyboardPanelSession(), nullptr);
    sptr<SceneSession> panelSession = GetSceneSession(abilityName, abilityName);
    ASSERT_NE(panelSession, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    sptr<SceneSession> getPanelSession = keyboardSession->GetKeyboardPanelSession();
    EXPECT_EQ(getPanelSession, panelSession);
    keyboardSession->BindKeyboardPanelSession(nullptr);
    EXPECT_NE(keyboardSession->GetKeyboardPanelSession(), nullptr);
}

/**
 * @tc.name: BindKeyboardPanelSession03
 * @tc.desc: test function: BindKeyboardPanelSession rebind different panel
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, BindKeyboardPanelSession03, TestSize.Level0)
{
    std::string abilityName = "BindKeyboardPanelSession03";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    sptr<SceneSession> firstPanel = GetSceneSession("FirstPanel", "FirstPanel");
    sptr<SceneSession> secondPanel = GetSceneSession("SecondPanel", "SecondPanel");
    ASSERT_NE(firstPanel, nullptr);
    ASSERT_NE(secondPanel, nullptr);
    keyboardSession->BindKeyboardPanelSession(firstPanel);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), firstPanel);
    keyboardSession->BindKeyboardPanelSession(secondPanel);
    EXPECT_EQ(keyboardSession->GetKeyboardPanelSession(), secondPanel);
    EXPECT_NE(keyboardSession->GetKeyboardPanelSession(), firstPanel);
}

/**
 * @tc.name: GetKeyboardPanelSession01
 * @tc.desc: test function: GetKeyboardPanelSession returns nullptr without binding
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetKeyboardPanelSession01, TestSize.Level0)
{
    std::string abilityName = "GetKeyboardPanelSession01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    sptr<SceneSession> panelSession = keyboardSession->GetKeyboardPanelSession();
    EXPECT_EQ(panelSession, nullptr);
}

/**
 * @tc.name: GetKeyboardPanelSession02
 * @tc.desc: test function: GetKeyboardPanelSession returns bound session
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetKeyboardPanelSession02, TestSize.Level0)
{
    std::string abilityName = "GetKeyboardPanelSession02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    sptr<SceneSession> panelSession = GetSceneSession(abilityName, abilityName);
    ASSERT_NE(panelSession, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    sptr<SceneSession> ret = keyboardSession->GetKeyboardPanelSession();
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ret, panelSession);
}

/**
 * @tc.name: GetKeyboardGravity02
 * @tc.desc: test function: GetKeyboardGravity returns FLOAT gravity
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetKeyboardGravity02, TestSize.Level0)
{
    std::string abilityName = "GetKeyboardGravity02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    auto ret = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(SessionGravity::SESSION_GRAVITY_FLOAT, ret);
}

/**
 * @tc.name: GetKeyboardGravity03
 * @tc.desc: test function: GetKeyboardGravity returns DEFAULT gravity
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetKeyboardGravity03, TestSize.Level0)
{
    std::string abilityName = "GetKeyboardGravity03";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_DEFAULT;
    auto ret = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(SessionGravity::SESSION_GRAVITY_DEFAULT, ret);
}

/**
 * @tc.name: GetKeyboardGravity04
 * @tc.desc: test function: GetKeyboardGravity default value is BOTTOM
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetKeyboardGravity04, TestSize.Level0)
{
    std::string abilityName = "GetKeyboardGravity04";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_NE(keyboardSession->property_, nullptr);
    auto ret = keyboardSession->GetKeyboardGravity();
    EXPECT_EQ(SessionGravity::SESSION_GRAVITY_BOTTOM, ret);
}

/**
 * @tc.name: GetKeyboardGravity05
 * @tc.desc: test function: GetKeyboardGravity after switching gravity multiple times
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetKeyboardGravity05, TestSize.Level0)
{
    std::string abilityName = "GetKeyboardGravity05";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    EXPECT_EQ(SessionGravity::SESSION_GRAVITY_FLOAT, keyboardSession->GetKeyboardGravity());
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    EXPECT_EQ(SessionGravity::SESSION_GRAVITY_BOTTOM, keyboardSession->GetKeyboardGravity());
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_DEFAULT;
    EXPECT_EQ(SessionGravity::SESSION_GRAVITY_DEFAULT, keyboardSession->GetKeyboardGravity());
}

/**
 * @tc.name: NotifyKeyboardPanelInfoChange01
 * @tc.desc: test function: NotifyKeyboardPanelInfoChange when panel disabled
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, NotifyKeyboardPanelInfoChange01, TestSize.Level1)
{
    std::string abilityName = "NotifyKeyboardPanelInfoChange01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = false;
    WSRect rect = { 100, 100, 200, 200 };
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
    EXPECT_FALSE(keyboardSession->isKeyboardPanelEnabled_);
}

/**
 * @tc.name: NotifyKeyboardPanelInfoChange02
 * @tc.desc: test function: NotifyKeyboardPanelInfoChange when sessionStage is null
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, NotifyKeyboardPanelInfoChange02, TestSize.Level1)
{
    std::string abilityName = "NotifyKeyboardPanelInfoChange02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    keyboardSession->sessionStage_ = nullptr;
    WSRect rect = { 50, 50, 100, 100 };
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, false);
    EXPECT_EQ(keyboardSession->sessionStage_, nullptr);
}

/**
 * @tc.name: NotifyKeyboardPanelInfoChange03
 * @tc.desc: test function: NotifyKeyboardPanelInfoChange with valid sessionStage
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, NotifyKeyboardPanelInfoChange03, TestSize.Level1)
{
    std::string abilityName = "NotifyKeyboardPanelInfoChange03";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    keyboardSession->sessionStage_ = mockSessionStage;
    WSRect rect = { 0, 0, 800, 600 };
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, true);
    keyboardSession->NotifyKeyboardPanelInfoChange(rect, false);
    EXPECT_NE(keyboardSession->sessionStage_, nullptr);
}

/**
 * @tc.name: Hide02
 * @tc.desc: test function: Hide when state is disconnect returns ok
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, Hide02, TestSize.Level0)
{
    std::string abilityName = "Hide02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->state_ = SessionState::STATE_DISCONNECT;
    EXPECT_EQ(WSError::WS_OK, keyboardSession->Hide());
}

/**
 * @tc.name: Hide03
 * @tc.desc: test function: Hide on phone window with active session
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, Hide03, TestSize.Level0)
{
    std::string abilityName = "Hide03";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->state_ = SessionState::STATE_CONNECT;
    keyboardSession->isActive_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(keyboardSession->sessionStage_, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(WSError::WS_OK, keyboardSession->Hide());
}

/**
 * @tc.name: Hide04
 * @tc.desc: test function: Hide on pc window with system keyboard
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, Hide04, TestSize.Level0)
{
    std::string abilityName = "Hide04";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->state_ = SessionState::STATE_CONNECT;
    keyboardSession->isActive_ = true;
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(keyboardSession->sessionStage_, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(false, keyboardSession->IsSystemKeyboard());
    keyboardSession->SetIsSystemKeyboard(true);
    EXPECT_EQ(true, keyboardSession->IsSystemKeyboard());
    EXPECT_EQ(WSError::WS_OK, keyboardSession->Hide());
}

/**
 * @tc.name: OpenKeyboardSyncTransaction02
 * @tc.desc: test function: OpenKeyboardSyncTransaction toggles flag multiple times
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, OpenKeyboardSyncTransaction02, TestSize.Level1)
{
    std::string abilityName = "OpenKeyboardSyncTransaction02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(false);
    keyboardSession->OpenKeyboardSyncTransaction();
    keyboardSession->OpenKeyboardSyncTransaction();
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_.load());
    WSRect keyboardPanelRect = { 0, 0, 0, 0 };
    WindowAnimationInfo animationInfo;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo, callingWindowInfoData);
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_.load());
}

/**
 * @tc.name: OpenKeyboardSyncTransaction03
 * @tc.desc: test function: OpenKeyboardSyncTransaction when flag already open
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, OpenKeyboardSyncTransaction03, TestSize.Level1)
{
    std::string abilityName = "OpenKeyboardSyncTransaction03";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    keyboardSession->OpenKeyboardSyncTransaction();
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_.load());
}

/**
 * @tc.name: CloseKeyboardSyncTransaction05
 * @tc.desc: test function: CloseKeyboardSyncTransaction with isKeyboardShow true
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CloseKeyboardSyncTransaction05, TestSize.Level1)
{
    std::string abilityName = "CloseKeyboardSyncTransaction05";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    WSRect keyboardPanelRect = { 0, 0, 0, 0 };
    WindowAnimationInfo animationInfo;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    keyboardSession->keyboardCallback_->isLastFrameLayoutFinished = []() { return true; };
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_.load());
}

/**
 * @tc.name: CloseKeyboardSyncTransaction06
 * @tc.desc: test function: CloseKeyboardSyncTransaction with isKeyboardShow false
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CloseKeyboardSyncTransaction06, TestSize.Level1)
{
    std::string abilityName = "CloseKeyboardSyncTransaction06";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    WSRect keyboardPanelRect = { 0, 0, 0, 0 };
    WindowAnimationInfo animationInfo;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->isKeyboardSyncTransactionOpen_.store(false);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, false, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_.load());
}

/**
 * @tc.name: CloseKeyboardSyncTransaction07
 * @tc.desc: test function: CloseKeyboardSyncTransaction with gravity changed
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CloseKeyboardSyncTransaction07, TestSize.Level1)
{
    std::string abilityName = "CloseKeyboardSyncTransaction07";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    WSRect keyboardPanelRect = { 0, 0, 0, 0 };
    WindowAnimationInfo animationInfo;
    animationInfo.isGravityChanged = true;
    animationInfo.callingId = 5;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->keyboardCallback_->isLastFrameLayoutFinished = []() { return true; };
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_.load());
    EXPECT_FALSE(keyboardSession->stateChanged_);
}

/**
 * @tc.name: CloseKeyboardSyncTransaction08
 * @tc.desc: test function: CloseKeyboardSyncTransaction with layout not finished
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CloseKeyboardSyncTransaction08, TestSize.Level1)
{
    std::string abilityName = "CloseKeyboardSyncTransaction08";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    WSRect keyboardPanelRect = { 0, 0, 0, 0 };
    WindowAnimationInfo animationInfo;
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    keyboardSession->keyboardCallback_->isLastFrameLayoutFinished = []() { return false; };
    keyboardSession->CloseKeyboardSyncTransaction(keyboardPanelRect, true, animationInfo, callingWindowInfoData);
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->stateChanged_);
}

/**
 * @tc.name: CallingWindowStateChange01
 * @tc.desc: test function: CallingWindowStateChange with normal state
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CallingWindowStateChange01, TestSize.Level1)
{
    std::string abilityName = "CallingWindowStateChange01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    CallingWindowInfoData callingWindowInfoData;
    callingWindowInfoData.callingWindowState = CallingWindowState::WINDOW_IN_NORMAL;
    keyboardSession->CallingWindowStateChange(callingWindowInfoData);
    EXPECT_EQ(CallingWindowState::WINDOW_IN_NORMAL, keyboardSession->callingWindowInfoData_.callingWindowState);
}

/**
 * @tc.name: CallingWindowStateChange02
 * @tc.desc: test function: CallingWindowStateChange with AI state
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CallingWindowStateChange02, TestSize.Level1)
{
    std::string abilityName = "CallingWindowStateChange02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    CallingWindowInfoData callingWindowInfoData;
    callingWindowInfoData.callingWindowState = CallingWindowState::WINDOW_IN_AI;
    keyboardSession->CallingWindowStateChange(callingWindowInfoData);
    EXPECT_EQ(CallingWindowState::WINDOW_IN_AI, keyboardSession->callingWindowInfoData_.callingWindowState);
}

/**
 * @tc.name: CallingWindowStateChange03
 * @tc.desc: test function: CallingWindowStateChange with scale values
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CallingWindowStateChange03, TestSize.Level1)
{
    std::string abilityName = "CallingWindowStateChange03";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    CallingWindowInfoData callingWindowInfoData;
    callingWindowInfoData.scaleX = 1.5;
    callingWindowInfoData.scaleY = 0.5;
    keyboardSession->CallingWindowStateChange(callingWindowInfoData);
    EXPECT_TRUE(std::abs(keyboardSession->callingWindowInfoData_.scaleX - 1.5) < 0.001f);
    EXPECT_TRUE(std::abs(keyboardSession->callingWindowInfoData_.scaleY - 0.5) < 0.001f);
}

/**
 * @tc.name: EnableCallingSessionAvoidArea02
 * @tc.desc: test function: EnableCallingSessionAvoidArea with null callback
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, EnableCallingSessionAvoidArea02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo info;
    info.abilityName_ = "EnableCallingSessionAvoidArea02";
    info.bundleName_ = "EnableCallingSessionAvoidArea02";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->EnableCallingSessionAvoidArea();
    EXPECT_TRUE(g_logMsg.find("Calling session is null") != std::string::npos);
}

/**
 * @tc.name: EnableCallingSessionAvoidArea03
 * @tc.desc: test function: EnableCallingSessionAvoidArea with valid calling session
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, EnableCallingSessionAvoidArea03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    keyboardCallback->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo callingSessionInfo;
        callingSessionInfo.abilityName_ = "EnableCallingSessionAvoidArea03";
        callingSessionInfo.bundleName_ = "EnableCallingSessionAvoidArea03";
        auto callingSession = sptr<SceneSession>::MakeSptr(callingSessionInfo, nullptr);
        callingSession->persistentId_ = persistentId;
        return callingSession;
    };
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    SessionInfo info;
    info.abilityName_ = "EnableCallingSessionAvoidArea03";
    info.bundleName_ = "EnableCallingSessionAvoidArea03";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCallback, keyboardCallback);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->EnableCallingSessionAvoidArea();
    EXPECT_TRUE(g_logMsg.find("Calling session is null") == std::string::npos);
}

/**
 * @tc.name: HandleActionUpdateKeyboardTouchHotArea03
 * @tc.desc: test function: HandleActionUpdateKeyboardTouchHotArea with invalid type
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, HandleActionUpdateKeyboardTouchHotArea03, TestSize.Level1)
{
    std::string abilityName = "HandleActionUpdateKeyboardTouchHotArea03";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sptr<SceneSession> keyboardPanelSession = GetSceneSession(abilityName, abilityName);
    keyboardSession->BindKeyboardPanelSession(keyboardPanelSession);
    keyboardSession->GetSessionProperty()->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    WMError ret = keyboardSession->HandleActionUpdateKeyboardTouchHotArea(property, action);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_TYPE, ret);
}

/**
 * @tc.name: TestIsLandscapeNoScreenSession
 * @tc.desc: test function: IsLandscape with empty screen map returns invalid display
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, TestIsLandscapeNoScreenSession, TestSize.Level1)
{
    std::string abilityName = "TestIsLandscapeNoScreenSession";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    bool isLandscape = false;
    screenSessionManagerClient_->screenSessionMap_.clear();
    WMError ret = keyboardSession->IsLandscape(999, isLandscape);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_DISPLAY, ret);
}

/**
 * @tc.name: TestIsLandscapePortrait
 * @tc.desc: test function: IsLandscape with portrait screen
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, TestIsLandscapePortrait, TestSize.Level1)
{
    std::string abilityName = "TestIsLandscapePortrait";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    const ScreenId screenId = 31;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, ScreenProperty(), screenId);
    screenSessionManagerClient_->screenSessionMap_.clear();
    screenSessionManagerClient_->screenSessionMap_[screenId] = screenSession;
    screenSession->property_.bounds_.rect_ = { 0, 0, 300, 400 };
    bool isLandscape = true;
    EXPECT_EQ(keyboardSession->IsLandscape(screenId, isLandscape), WMError::WM_OK);
    EXPECT_FALSE(isLandscape);
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: TestIsLandscapeLandscape
 * @tc.desc: test function: IsLandscape with landscape screen
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, TestIsLandscapeLandscape, TestSize.Level1)
{
    std::string abilityName = "TestIsLandscapeLandscape";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    const ScreenId screenId = 41;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, ScreenProperty(), screenId);
    screenSessionManagerClient_->screenSessionMap_.clear();
    screenSessionManagerClient_->screenSessionMap_[screenId] = screenSession;
    screenSession->property_.bounds_.rect_ = { 0, 0, 400, 300 };
    bool isLandscape = false;
    EXPECT_EQ(keyboardSession->IsLandscape(screenId, isLandscape), WMError::WM_OK);
    EXPECT_TRUE(isLandscape);
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: TestIsLandscapeSquareScreen
 * @tc.desc: test function: IsLandscape with square screen returns false
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, TestIsLandscapeSquareScreen, TestSize.Level1)
{
    std::string abilityName = "TestIsLandscapeSquareScreen";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    const ScreenId screenId = 51;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, ScreenProperty(), screenId);
    screenSessionManagerClient_->screenSessionMap_.clear();
    screenSessionManagerClient_->screenSessionMap_[screenId] = screenSession;
    screenSession->property_.bounds_.rect_ = { 0, 0, 200, 200 };
    bool isLandscape = true;
    EXPECT_EQ(keyboardSession->IsLandscape(screenId, isLandscape), WMError::WM_OK);
    EXPECT_FALSE(isLandscape);
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: NotifyKeyboardAnimationWillBegin03
 * @tc.desc: test function: NotifyKeyboardAnimationWillBegin with calling session
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, NotifyKeyboardAnimationWillBegin03, TestSize.Level1)
{
    std::string abilityName = "NotifyKeyboardAnimationWillBegin03";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    sptr<SceneSession> sceneSession = GetSceneSession("CallingSession03", "CallingSession03");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->persistentId_ = 200;
    keyboardSession->keyboardCallback_->onGetSceneSession = [sceneSession](uint32_t persistentId) {
        return sceneSession;
    };
    bool isKeyboardShow = true;
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 200;
    keyboardSession->NotifyKeyboardAnimationWillBegin(isKeyboardShow, animationInfo);
    usleep(WAIT_ASYNC_US);
    ASSERT_NE(keyboardSession->GetSceneSession(animationInfo.callingId), nullptr);
}

/**
 * @tc.name: NotifyKeyboardAnimationWillBegin04
 * @tc.desc: test function: NotifyKeyboardAnimationWillBegin with sync transaction open
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, NotifyKeyboardAnimationWillBegin04, TestSize.Level1)
{
    std::string abilityName = "NotifyKeyboardAnimationWillBegin04";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->specificCallback_->onUpdateAvoidArea_ = [](uint32_t callingSessionId) {};
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    bool isKeyboardShow = false;
    WindowAnimationInfo animationInfo;
    animationInfo.callingId = 300;
    keyboardSession->NotifyKeyboardAnimationWillBegin(isKeyboardShow, animationInfo);
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(keyboardSession->isKeyboardSyncTransactionOpen_.load());
}

/**
 * @tc.name: GetCallingSessionId01
 * @tc.desc: test function: GetCallingSessionId default invalid
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetCallingSessionId01, TestSize.Level0)
{
    std::string abilityName = "GetCallingSessionId01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_NE(keyboardSession->property_, nullptr);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, INVALID_WINDOW_ID);
}

/**
 * @tc.name: GetCallingSessionId02
 * @tc.desc: test function: GetCallingSessionId after setting
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetCallingSessionId02, TestSize.Level0)
{
    std::string abilityName = "GetCallingSessionId02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_NE(keyboardSession->property_, nullptr);
    uint32_t expectedId = 12345;
    keyboardSession->property_->SetCallingSessionId(expectedId);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, expectedId);
}

/**
 * @tc.name: GetCallingSessionId03
 * @tc.desc: test function: GetCallingSessionId after reset to invalid
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetCallingSessionId03, TestSize.Level0)
{
    std::string abilityName = "GetCallingSessionId03";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->property_->SetCallingSessionId(100);
    keyboardSession->property_->SetCallingSessionId(INVALID_WINDOW_ID);
    uint32_t callingId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(callingId, INVALID_WINDOW_ID);
}

/**
 * @tc.name: GetIsKeyboardSyncTransactionOpen01
 * @tc.desc: test function: GetIsKeyboardSyncTransactionOpen default false
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetIsKeyboardSyncTransactionOpen01, TestSize.Level0)
{
    std::string abilityName = "GetIsKeyboardSyncTransactionOpen01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_FALSE(keyboardSession->GetIsKeyboardSyncTransactionOpen());
}

/**
 * @tc.name: GetIsKeyboardSyncTransactionOpen02
 * @tc.desc: test function: GetIsKeyboardSyncTransactionOpen after open
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetIsKeyboardSyncTransactionOpen02, TestSize.Level0)
{
    std::string abilityName = "GetIsKeyboardSyncTransactionOpen02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    EXPECT_TRUE(keyboardSession->GetIsKeyboardSyncTransactionOpen());
    keyboardSession->isKeyboardSyncTransactionOpen_.store(false);
    EXPECT_FALSE(keyboardSession->GetIsKeyboardSyncTransactionOpen());
}

/**
 * @tc.name: SetSkipSelfWhenShowOnVirtualScreen01
 * @tc.desc: test function: SetSkipSelfWhenShowOnVirtualScreen with true
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, SetSkipSelfWhenShowOnVirtualScreen01, TestSize.Level1)
{
    std::string abilityName = "SetSkipSelfWhenShowOnVirtualScreen01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(keyboardSession->isSkipSelfWhenShowOnVirtualScreen_.load());
}

/**
 * @tc.name: SetSkipSelfWhenShowOnVirtualScreen02
 * @tc.desc: test function: SetSkipSelfWhenShowOnVirtualScreen with callback set but no surface node
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, SetSkipSelfWhenShowOnVirtualScreen02, TestSize.Level1)
{
    std::string abilityName = "SetSkipSelfWhenShowOnVirtualScreen02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    bool capturedSkip = false;
    keyboardSession->specificCallback_->onSetSkipSelfWhenShowOnVirtualScreen_ =
        [&capturedSkip](uint64_t nodeId, bool isSkip) {
            capturedSkip = isSkip;
        };
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    usleep(WAIT_ASYNC_US);
    // SurfaceNode is null, callback is not invoked and flag remains default
    EXPECT_FALSE(keyboardSession->isSkipSelfWhenShowOnVirtualScreen_.load());
    EXPECT_FALSE(capturedSkip);
}

/**
 * @tc.name: SetSkipEventOnCastPlus01
 * @tc.desc: test function: SetSkipEventOnCastPlus with true
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, SetSkipEventOnCastPlus01, TestSize.Level1)
{
    std::string abilityName = "SetSkipEventOnCastPlus01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->SetSkipEventOnCastPlus(true);
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
}

/**
 * @tc.name: SetSkipEventOnCastPlus02
 * @tc.desc: test function: SetSkipEventOnCastPlus with callback set
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, SetSkipEventOnCastPlus02, TestSize.Level1)
{
    std::string abilityName = "SetSkipEventOnCastPlus02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    bool capturedSkip = false;
    keyboardSession->specificCallback_->onSetSkipEventOnCastPlus_ =
        [&capturedSkip](int32_t persistentId, bool isSkip) {
            capturedSkip = isSkip;
        };
    keyboardSession->SetSkipEventOnCastPlus(true);
    usleep(WAIT_ASYNC_US);
    EXPECT_TRUE(capturedSkip);
    keyboardSession->SetSkipEventOnCastPlus(false);
    usleep(WAIT_ASYNC_US);
    EXPECT_FALSE(capturedSkip);
}

/**
 * @tc.name: UpdateSizeChangeReason01
 * @tc.desc: test function: UpdateSizeChangeReason with undefined reason
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, UpdateSizeChangeReason01, TestSize.Level1)
{
    std::string abilityName = "UpdateSizeChangeReason01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_EQ(WSError::WS_OK, keyboardSession->UpdateSizeChangeReason(SizeChangeReason::UNDEFINED));
}

/**
 * @tc.name: UpdateSizeChangeReason02
 * @tc.desc: test function: UpdateSizeChangeReason with drag reasons
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, UpdateSizeChangeReason02, TestSize.Level1)
{
    std::string abilityName = "UpdateSizeChangeReason02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_EQ(WSError::WS_OK, keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_START));
    EXPECT_EQ(WSError::WS_OK, keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_MOVE));
    EXPECT_EQ(WSError::WS_OK, keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_END));
}

/**
 * @tc.name: UpdateSizeChangeReason03
 * @tc.desc: test function: UpdateSizeChangeReason with move reason and panel
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, UpdateSizeChangeReason03, TestSize.Level1)
{
    std::string abilityName = "UpdateSizeChangeReason03";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    sptr<SceneSession> panelSession = GetSceneSession(abilityName, abilityName);
    ASSERT_NE(panelSession, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    EXPECT_EQ(WSError::WS_OK, keyboardSession->UpdateSizeChangeReason(SizeChangeReason::MOVE));
}

/**
 * @tc.name: ChangeKeyboardEffectOption01
 * @tc.desc: test function: ChangeKeyboardEffectOption with valid option
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, ChangeKeyboardEffectOption01, TestSize.Level1)
{
    std::string abilityName = "ChangeKeyboardEffectOption01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    KeyboardEffectOption effectOption;
    effectOption.blurHeight_ = 100;
    EXPECT_EQ(WSError::WS_OK, keyboardSession->ChangeKeyboardEffectOption(effectOption));
    usleep(WAIT_ASYNC_US);
    ASSERT_NE(keyboardSession->property_, nullptr);
    EXPECT_EQ(keyboardSession->property_->GetKeyboardEffectOption().blurHeight_, effectOption.blurHeight_);
}

/**
 * @tc.name: ChangeKeyboardEffectOption02
 * @tc.desc: test function: ChangeKeyboardEffectOption with listener set
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, ChangeKeyboardEffectOption02, TestSize.Level1)
{
    std::string abilityName = "ChangeKeyboardEffectOption02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    KeyboardEffectOption capturedOption;
    capturedOption.blurHeight_ = 0;
    keyboardSession->SetKeyboardEffectOptionChangeListener(
        [&capturedOption](const KeyboardEffectOption& option) {
            capturedOption = option;
        });
    usleep(WAIT_ASYNC_US);
    KeyboardEffectOption effectOption;
    effectOption.blurHeight_ = 200;
    keyboardSession->ChangeKeyboardEffectOption(effectOption);
    usleep(WAIT_ASYNC_US);
    EXPECT_EQ(capturedOption.blurHeight_, effectOption.blurHeight_);
}

/**
 * @tc.name: SetKeyboardEffectOptionChangeListener01
 * @tc.desc: test function: SetKeyboardEffectOptionChangeListener registers func
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, SetKeyboardEffectOptionChangeListener01, TestSize.Level1)
{
    std::string abilityName = "SetKeyboardEffectOptionChangeListener01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    EXPECT_EQ(keyboardSession->changeKeyboardEffectOptionFunc_, nullptr);
    bool called = false;
    keyboardSession->SetKeyboardEffectOptionChangeListener(
        [&called](const KeyboardEffectOption& option) { called = true; });
    usleep(WAIT_ASYNC_US);
    EXPECT_NE(keyboardSession->changeKeyboardEffectOptionFunc_, nullptr);
}

/**
 * @tc.name: IsVisibleForeground01
 * @tc.desc: test function: IsVisibleForeground default false
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, IsVisibleForeground01, TestSize.Level0)
{
    std::string abilityName = "IsVisibleForeground01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isVisible_.store(false);
    EXPECT_FALSE(keyboardSession->IsVisibleForeground());
}

/**
 * @tc.name: IsVisibleForeground02
 * @tc.desc: test function: IsVisibleForeground after set true
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, IsVisibleForeground02, TestSize.Level0)
{
    std::string abilityName = "IsVisibleForeground02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isVisible_.store(true);
    EXPECT_TRUE(keyboardSession->IsVisibleForeground());
    keyboardSession->isVisible_.store(false);
    EXPECT_FALSE(keyboardSession->IsVisibleForeground());
}

/**
 * @tc.name: IsVisibleNotBackground01
 * @tc.desc: test function: IsVisibleNotBackground default false
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, IsVisibleNotBackground01, TestSize.Level0)
{
    std::string abilityName = "IsVisibleNotBackground01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isVisible_.store(false);
    EXPECT_FALSE(keyboardSession->IsVisibleNotBackground());
}

/**
 * @tc.name: IsVisibleNotBackground02
 * @tc.desc: test function: IsVisibleNotBackground after set true
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, IsVisibleNotBackground02, TestSize.Level0)
{
    std::string abilityName = "IsVisibleNotBackground02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isVisible_.store(true);
    EXPECT_TRUE(keyboardSession->IsVisibleNotBackground());
}

/**
 * @tc.name: RecalculatePanelRectForAvoidArea01
 * @tc.desc: test function: RecalculatePanelRectForAvoidArea with default rect
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, RecalculatePanelRectForAvoidArea01, TestSize.Level1)
{
    std::string abilityName = "RecalculatePanelRectForAvoidArea01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.landscapeAvoidHeight_ = -1;
    keyboardSession->property_->keyboardLayoutParams_.portraitAvoidHeight_ = -1;
    WSRect panelRect = { 0, 0, 100, 200 };
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.width_, 100);
    EXPECT_EQ(panelRect.height_, 200);
}

/**
 * @tc.name: RecalculatePanelRectForAvoidArea02
 * @tc.desc: test function: RecalculatePanelRectForAvoidArea with valid landscape avoid height
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, RecalculatePanelRectForAvoidArea02, TestSize.Level1)
{
    std::string abilityName = "RecalculatePanelRectForAvoidArea02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->property_->keyboardLayoutParams_.landscapeAvoidHeight_ = 50;
    keyboardSession->property_->keyboardLayoutParams_.portraitAvoidHeight_ = 60;
    keyboardSession->property_->keyboardLayoutParams_.LandscapePanelRect_.width_ = 100;
    keyboardSession->property_->keyboardLayoutParams_.PortraitPanelRect_.width_ = 200;
    WSRect panelRect = { 0, 0, 100, 200 };
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.height_, 50);
}

/**
 * @tc.name: ForceProcessKeyboardOccupiedAreaInfo01
 * @tc.desc: test function: ForceProcessKeyboardOccupiedAreaInfo runs without crash
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, ForceProcessKeyboardOccupiedAreaInfo01, TestSize.Level1)
{
    std::string abilityName = "ForceProcessKeyboardOccupiedAreaInfo01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->ForceProcessKeyboardOccupiedAreaInfo();
    EXPECT_NE(keyboardSession->specificCallback_, nullptr);
}

/**
 * @tc.name: GetPanelRect01
 * @tc.desc: test function: GetPanelRect returns zero rect when panel is null
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetPanelRect01, TestSize.Level1)
{
    std::string abilityName = "GetPanelRect01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    ASSERT_EQ(keyboardSession->GetKeyboardPanelSession(), nullptr);
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.posX_, 0);
    EXPECT_EQ(panelRect.posY_, 0);
    EXPECT_EQ(panelRect.width_, 0);
    EXPECT_EQ(panelRect.height_, 0);
}

/**
 * @tc.name: GetPanelRect02
 * @tc.desc: test function: GetPanelRect returns panel session rect
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, GetPanelRect02, TestSize.Level1)
{
    std::string abilityName = "GetPanelRect02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    sptr<SceneSession> panelSession = GetSceneSession(abilityName, abilityName);
    ASSERT_NE(panelSession, nullptr);
    WSRect rect = { 10, 20, 300, 400 };
    panelSession->SetSessionRect(rect);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    WSRect panelRect = keyboardSession->GetPanelRect();
    EXPECT_EQ(panelRect.posX_, 10);
    EXPECT_EQ(panelRect.posY_, 20);
    EXPECT_EQ(panelRect.width_, 300);
    EXPECT_EQ(panelRect.height_, 400);
}

/**
 * @tc.name: CloseRSTransaction01
 * @tc.desc: test function: CloseRSTransaction when flag closed
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CloseRSTransaction01, TestSize.Level1)
{
    std::string abilityName = "CloseRSTransaction01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(false);
    keyboardSession->CloseRSTransaction();
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_.load());
}

/**
 * @tc.name: CloseRSTransaction02
 * @tc.desc: test function: CloseRSTransaction when flag open
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CloseRSTransaction02, TestSize.Level1)
{
    std::string abilityName = "CloseRSTransaction02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->isKeyboardSyncTransactionOpen_.store(true);
    keyboardSession->CloseRSTransaction();
    EXPECT_FALSE(keyboardSession->isKeyboardSyncTransactionOpen_.load());
}

/**
 * @tc.name: NotifySystemKeyboardAvoidChange01
 * @tc.desc: test function: NotifySystemKeyboardAvoidChange on non-pc window
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, NotifySystemKeyboardAvoidChange01, TestSize.Level1)
{
    std::string abilityName = "NotifySystemKeyboardAvoidChange01";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW);
    EXPECT_FALSE(keyboardSession->systemConfig_.IsPcWindow());
}

/**
 * @tc.name: NotifySystemKeyboardAvoidChange02
 * @tc.desc: test function: NotifySystemKeyboardAvoidChange on pc without system keyboard
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, NotifySystemKeyboardAvoidChange02, TestSize.Level1)
{
    std::string abilityName = "NotifySystemKeyboardAvoidChange02";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    keyboardSession->SetIsSystemKeyboard(false);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_HIDE);
    EXPECT_FALSE(keyboardSession->IsSystemKeyboard());
}

/**
 * @tc.name: NotifySystemKeyboardAvoidChange03
 * @tc.desc: test function: NotifySystemKeyboardAvoidChange with null callback
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, NotifySystemKeyboardAvoidChange03, TestSize.Level1)
{
    std::string abilityName = "NotifySystemKeyboardAvoidChange03";
    SessionInfo info;
    info.abilityName_ = abilityName;
    info.bundleName_ = abilityName;
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
    EXPECT_TRUE(keyboardSession->IsSystemKeyboard());
    EXPECT_EQ(keyboardSession->keyboardCallback_, nullptr);
}

/**
 * @tc.name: CallingWindowStateChange04
 * @tc.desc: test function: CallingWindowStateChange default scale values
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, CallingWindowStateChange04, TestSize.Level1)
{
    std::string abilityName = "CallingWindowStateChange04";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    ASSERT_NE(keyboardSession, nullptr);
    CallingWindowInfoData callingWindowInfoData;
    keyboardSession->CallingWindowStateChange(callingWindowInfoData);
    EXPECT_EQ(keyboardSession->callingWindowInfoData_.scaleX, 1);
    EXPECT_EQ(keyboardSession->callingWindowInfoData_.scaleY, 1);
    EXPECT_EQ(keyboardSession->callingWindowInfoData_.callingWindowState, CallingWindowState::WINDOW_IN_NORMAL);
}

/**
 * @tc.name: HandleActionUpdateKeyboardTouchHotArea05
 * @tc.desc: test function: HandleActionUpdateKeyboardTouchHotArea with null panel
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, HandleActionUpdateKeyboardTouchHotArea05, TestSize.Level1)
{
    std::string abilityName = "HandleActionUpdateKeyboardTouchHotArea05";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    ASSERT_EQ(keyboardSession->GetKeyboardPanelSession(), nullptr);
    WMError ret = keyboardSession->HandleActionUpdateKeyboardTouchHotArea(property, action);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: HandleActionUpdateKeyboardTouchHotArea06
 * @tc.desc: test function: HandleActionUpdateKeyboardTouchHotArea with full hot areas
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest4, HandleActionUpdateKeyboardTouchHotArea06, TestSize.Level1)
{
    std::string abilityName = "HandleActionUpdateKeyboardTouchHotArea06";
    sptr<KeyboardSession> keyboardSession = GetKeyboardSession(abilityName, abilityName);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    KeyboardTouchHotAreas keyboardTouchHotAreas;
    Rect rect = { 0, 0, 100, 100 };
    keyboardTouchHotAreas.landscapeKeyboardHotAreas_.push_back(rect);
    keyboardTouchHotAreas.portraitKeyboardHotAreas_.push_back(rect);
    keyboardTouchHotAreas.landscapePanelHotAreas_.push_back(rect);
    keyboardTouchHotAreas.portraitPanelHotAreas_.push_back(rect);
    property->SetKeyboardTouchHotAreas(keyboardTouchHotAreas);
    sptr<SceneSession> keyboardPanelSession = GetSceneSession(abilityName, abilityName);
    keyboardSession->BindKeyboardPanelSession(keyboardPanelSession);
    keyboardSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    const ScreenId screenId = 61;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(screenId, ScreenProperty(), screenId);
    screenSessionManagerClient_->screenSessionMap_.clear();
    screenSessionManagerClient_->screenSessionMap_[screenId] = screenSession;
    screenSession->property_.bounds_.rect_ = { 0, 0, 400, 300 };
    keyboardSession->property_->SetDisplayId(screenId);
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ASPECT_RATIO;
    WMError ret = keyboardSession->HandleActionUpdateKeyboardTouchHotArea(property, action);
    EXPECT_EQ(WMError::WM_OK, ret);
    screenSessionManagerClient_->screenSessionMap_.clear();
}
} // namespace
} // namespace Rosen
} // namespace OHOS
