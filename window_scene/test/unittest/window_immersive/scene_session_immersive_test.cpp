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

#include <gtest/gtest.h>

#include "context.h"
#include "interfaces/include/ws_common.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/session.h"
#include "session_info.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
} // namespace

class SceneSessionImmersiveTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;

private:
    void CreateSession(SessionInfo sessionInfo, int32_t persistentId);
};

sptr<SceneSessionManager> SceneSessionImmersiveTest::ssm_ = nullptr;

void SceneSessionImmersiveTest::SetUpTestCase()
{
    ssm_ = new SceneSessionManager();
    ssm_->Init();
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr();
    ssm_->rootSceneSession_->property_ = sptr<WindowSessionProperty>::MakeSptr();
}

void SceneSessionImmersiveTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionImmersiveTest::SetUp()
{
}

void SceneSessionImmersiveTest::TearDown()
{
}

void SceneSessionImmersiveTest::CreateSession(SessionInfo sessionInfo, int32_t persistentId)
{
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });
    ASSERT_NE(ssm_->GetSceneSession(persistentId), nullptr);
}

namespace {

/**
 * @tc.name: NotifyNextAvoidRectInfo_statusBar_01
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_statusBar_01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, NotifyNextAvoidRectInfo_statusBar_01, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    WSRect portraitRect = { 0, 0, 1260, 123 };
    WSRect landspaceRect = { 0, 0, 2720, 123 };
    auto ret = ssm_->NotifyNextAvoidRectInfo(AvoidAreaType::TYPE_SYSTEM, portraitRect, landspaceRect, 0);
    ASSERT_EQ(ret, WSError::WS_OK);
    SessionInfo info;
    info.abilityName_ = "NotifyNextAvoidRectInfo_statusBar_01";
    info.bundleName_ = "NotifyNextAvoidRectInfo_statusBar_01";
    info.screenId_ = 0;
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onGetNextAvoidAreaRectInfo_ =
        [](DisplayId displayId, AvoidAreaType type, std::pair<WSRect, WSRect>& nextSystemBarAvoidAreaRectInfo) {
            return ssm_->GetNextAvoidRectInfo(displayId, type, nextSystemBarAvoidAreaRectInfo);
        };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->property_->SetPersistentId(1);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sceneSession->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 2720 });
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    std::map<WindowType, SystemBarProperty> properties;
    properties[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarProperty();
    properties[WindowType::WINDOW_TYPE_STATUS_BAR].settingFlag_ = SystemBarSettingFlag::ENABLE_SETTING;
    properties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR] = SystemBarProperty();
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_90, { 0, 0, 2720, 1260 }, properties, avoidAreas);
    Rect rect = { 0, 0, 2720, 123 };
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_SYSTEM].topRect_, rect);
    properties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enable_ = false;
    rect = { 0, 0, 0, 0 };
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_90, { 0, 0, 2720, 1260 }, properties, avoidAreas);
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].bottomRect_, rect);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyNextAvoidRectInfo_keyboard
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_keyboard
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, NotifyNextAvoidRectInfo_keyboard, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "NotifyNextAvoidRectInfo_keyboard";
    info.bundleName_ = "NotifyNextAvoidRectInfo_keyboard";
    info.screenId_ = 0;
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onKeyboardRotationChange_ =
        [](int32_t persistentId, Rotation rotation, std::vector<std::pair<bool, WSRect>>& avoidAreas) {
            ssm_->GetKeyboardOccupiedAreaWithRotation(persistentId, rotation, avoidAreas);
        };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->property_->SetPersistentId(1);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    AvoidArea avoidArea;
    sceneSession->GetKeyboardAvoidAreaByRotation(Rotation::ROTATION_0, { 0, 0, 1260, 2720 }, avoidArea);
    Rect rect = { 0, 0, 0, 0 };
    ASSERT_EQ(avoidArea.bottomRect_, rect);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyNextAvoidRectInfo_keyboard_01
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_keyboard_01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, NotifyNextAvoidRectInfo_keyboard_01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "NotifyNextAvoidRectInfo_keyboard_01";
    info.bundleName_ = "NotifyNextAvoidRectInfo_keyboard_01";
    info.screenId_ = 0;
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onKeyboardRotationChange_ =
        [](int32_t persistentId, Rotation rotation, std::vector<std::pair<bool, WSRect>>& avoidAreas) {
            ssm_->GetKeyboardOccupiedAreaWithRotation(persistentId, rotation, avoidAreas);
        };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->property_->SetPersistentId(1);
    SessionInfo keyboardSessionInfo;
    keyboardSessionInfo.abilityName_ = "keyboard";
    keyboardSessionInfo.bundleName_ = "keyboard";
    keyboardSessionInfo.screenId_ = 0;
    sptr<SceneSession> keyboardSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->property_->type_ = WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT;
    keyboardSession->property_->keyboardLayoutParams_.PortraitPanelRect_ = { 0, 1700, 1260, 1020 };
    keyboardSession->property_->keyboardLayoutParams_.LandscapePanelRect_ = { 0, 538, 2720, 722 };
    keyboardSession->property_->SetPersistentId(2);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->sceneSessionMap_.insert({ keyboardSession->GetPersistentId(), keyboardSession });
    auto uiType = ssm_->systemConfig_.windowUIType_;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    AvoidArea avoidArea;
    sceneSession->GetKeyboardAvoidAreaByRotation(Rotation::ROTATION_0, { 0, 0, 1260, 2720 }, avoidArea);
    Rect rect = { 0, 1700, 1260, 1020 };
    ASSERT_EQ(avoidArea.bottomRect_, rect);
    sceneSession->GetKeyboardAvoidAreaByRotation(Rotation::ROTATION_90, { 0, 0, 2720, 1260 }, avoidArea);
    rect = { 0, 538, 2720, 722 };
    ASSERT_EQ(avoidArea.bottomRect_, rect);
    keyboardSession->state_ = SessionState::STATE_BACKGROUND;
    rect = { 0, 0, 0, 0 };
    avoidArea.bottomRect_ = rect;
    sceneSession->GetKeyboardAvoidAreaByRotation(Rotation::ROTATION_180, { 0, 0, 1260, 2720 }, avoidArea);
    ASSERT_EQ(avoidArea.bottomRect_, rect);
    ssm_->systemConfig_.windowUIType_ = uiType;
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyNextAvoidRectInfo_AIBar
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_AIBar
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, NotifyNextAvoidRectInfo_AIBar, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    WSRect portraitRect = { 409, 2629, 442, 91 };
    WSRect landspaceRect = { 884, 1169, 952, 91 };
    auto ret = ssm_->NotifyNextAvoidRectInfo(AvoidAreaType::TYPE_NAVIGATION_INDICATOR, portraitRect, landspaceRect, 0);
    ASSERT_EQ(ret, WSError::WS_OK);
    SessionInfo info;
    info.abilityName_ = "NotifyNextAvoidRectInfo_AIBar";
    info.bundleName_ = "NotifyNextAvoidRectInfo_AIBar";
    info.screenId_ = 0;
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onGetNextAvoidAreaRectInfo_ =
        [](DisplayId displayId, AvoidAreaType type, std::pair<WSRect, WSRect>& nextSystemBarAvoidAreaRectInfo) {
            return ssm_->GetNextAvoidRectInfo(displayId, type, nextSystemBarAvoidAreaRectInfo);
        };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->property_->SetPersistentId(1);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sceneSession->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 2720 });
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    std::map<WindowType, SystemBarProperty> properties;
    properties[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarProperty();
    properties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR] = SystemBarProperty();
    properties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].settingFlag_ = SystemBarSettingFlag::ENABLE_SETTING;
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_0, { 0, 0, 1260, 2720 }, properties, avoidAreas);
    Rect rect = { 409, 2629, 442, 91 };
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].bottomRect_, rect);
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_90, { 0, 0, 2720, 1260 }, properties, avoidAreas);
    rect = { 884, 1169, 952, 91 };
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].bottomRect_, rect);
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_180, { 0, 0, 1260, 2720 }, properties, avoidAreas);
    rect = { 409, 2629, 442, 91 };
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].bottomRect_, rect);
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_270, { 0, 0, 2720, 1260 }, properties, avoidAreas);
    rect = { 884, 1169, 952, 91 };
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].bottomRect_, rect);
    properties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enable_ = false;
    rect = { 0, 0, 0, 0 };
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_0, { 0, 0, 2720, 1260 }, properties, avoidAreas);
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].topRect_, rect);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetKeyboardAvoidArea
 * @tc.desc: GetKeyboardAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, GetKeyboardAvoidArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    specificCallback_->onGetSceneSessionVectorByType_ = [](WindowType type) -> std::vector<sptr<SceneSession>> {
        std::vector<sptr<SceneSession>> backgroundSession;
        return backgroundSession;
    };

    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    WSRect overlapRect = { 0, 0, 0, 0 };
    AvoidArea avoidArea;
    sceneSession->GetKeyboardAvoidArea(overlapRect, avoidArea);
    ASSERT_EQ(true, overlapRect.IsEmpty());
    ASSERT_EQ(true, sceneSession->keyboardAvoidAreaActive_);
    sceneSession->keyboardAvoidAreaActive_ = false;
    sceneSession->GetKeyboardAvoidArea(overlapRect, avoidArea);
    ASSERT_EQ(false, sceneSession->keyboardAvoidAreaActive_);
    ASSERT_EQ(true, overlapRect.IsEmpty());
}

/**
 * @tc.name: GetCutoutAvoidArea
 * @tc.desc: GetCutoutAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, GetCutoutAvoidArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    WSRect overlapRect = { 0, 0, 0, 0 };
    AvoidArea avoidArea;
    sceneSession->GetCutoutAvoidArea(overlapRect, avoidArea);
}

/**
 * @tc.name: GetAvoidAreaByType
 * @tc.desc: GetAvoidAreaByType
 * @tc.type: FUNC ok
 */
HWTEST_F(SceneSessionImmersiveTest, GetAvoidAreaByType, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ =
        [](WindowType type, uint64_t displayId) -> std::vector<sptr<SceneSession>> {
        SessionInfo info_;
        info_.abilityName_ = "Background01";
        info_.bundleName_ = "IsFloatingWindowAppType";
        std::vector<sptr<SceneSession>> backgroundSession;
        sptr<SceneSession> session2 = sptr<SceneSession>::MakeSptr(info_, nullptr);
        backgroundSession.push_back(session2);
        return backgroundSession;
    };
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    WSRect rect = { 0, 0, 320, 240 }; // width: 320, height: 240
    sceneSession->SetSessionRect(rect);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->property_ = property;
    AvoidArea avoidArea;
    sceneSession->GetAvoidAreaByType(AvoidAreaType::TYPE_CUTOUT);
    sceneSession->GetAvoidAreaByType(AvoidAreaType::TYPE_SYSTEM);
    sceneSession->GetAvoidAreaByType(AvoidAreaType::TYPE_KEYBOARD);
    sceneSession->GetAvoidAreaByType(AvoidAreaType::TYPE_SYSTEM_GESTURE);
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: GetAvoidAreaByTypeIgnoringVisibility
 * @tc.desc: GetAvoidAreaByTypeIgnoringVisibility
 * @tc.type: FUNC ok
 */
HWTEST_F(SceneSessionImmersiveTest, GetAvoidAreaByTypeIgnoringVisibility, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "GetAvoidAreaByTypeIgnoringVisibility";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    specificCallback_->onGetSceneSessionVectorByTypeAndDisplayId_ =
        [](WindowType type, uint64_t displayId) -> std::vector<sptr<SceneSession>> {
        SessionInfo info_;
        info_.abilityName_ = "Background01";
        info_.bundleName_ = "GetAvoidAreaByTypeIgnoringVisibility";
        std::vector<sptr<SceneSession>> backgroundSession;
        sptr<SceneSession> session2 = sptr<SceneSession>::MakeSptr(info_, nullptr);
        backgroundSession.push_back(session2);
        return backgroundSession;
    };
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    WSRect rect = { 0, 0, 320, 240 }; // width: 320, height: 240
    sceneSession->SetSessionRect(rect);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->property_ = property;
    using T = std::underlying_type_t<AvoidAreaType>;
    for (T avoidAreaType = static_cast<T>(AvoidAreaType::TYPE_START);
        avoidAreaType < static_cast<T>(AvoidAreaType::TYPE_END); avoidAreaType++) {
        auto type = static_cast<AvoidAreaType>(avoidAreaType);
        sceneSession->GetAvoidAreaByTypeIgnoringVisibility(type);
    }
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: CalculateAvoidAreaRect
 * @tc.desc: CalculateAvoidAreaRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, CalculateAvoidAreaRect, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "Background01";
    info.bundleName_ = "IsFloatingWindowAppType";
    info.windowType_ = 1;
    sptr<Rosen::ISession> session_;
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback_);
    EXPECT_NE(sceneSession, nullptr);
    WSRect overlapRect = { 0, 0, 0, 0 };
    WSRect avoidRect = { 0, 0, 0, 0 };
    AvoidArea avoidArea;
    sceneSession->CalculateAvoidAreaRect(overlapRect, avoidRect, avoidArea);
    WSRect overlapRect_ = { 1, 1, 1, 1 };
    WSRect avoidRect_ = { 1, 1, 1, 1 };
    sceneSession->CalculateAvoidAreaRect(overlapRect_, avoidRect_, avoidArea);
}


/**
 * @tc.name: NotifyClientToUpdateAvoidArea
 * @tc.desc: check func NotifyClientToUpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, NotifyClientToUpdateAvoidArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyClientToUpdateAvoidArea";
    info.bundleName_ = "NotifyClientToUpdateAvoidArea";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);

    sceneSession->NotifyClientToUpdateAvoidArea();
    EXPECT_EQ(nullptr, sceneSession->specificCallback_);

    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sceneSession = sptr<SceneSession>::MakeSptr(info, callback);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->persistentId_ = 6;
    callback->onUpdateAvoidArea_ = nullptr;
    sceneSession->NotifyClientToUpdateAvoidArea();

    UpdateAvoidAreaCallback callbackFun = [&sceneSession](int32_t persistentId) {
        sceneSession->RemoveToastSession(persistentId);
        return;
    };
    callback->onUpdateAvoidArea_ = callbackFun;
    sceneSession->NotifyClientToUpdateAvoidArea();
    EXPECT_EQ(6, sceneSession->GetPersistentId());
}

/**
 * @tc.name: HandleLayoutAvoidAreaUpdate
 * @tc.desc: HandleLayoutAvoidAreaUpdate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, HandleLayoutAvoidAreaUpdate, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleLayoutAvoidAreaUpdate";
    info.bundleName_ = "HandleLayoutAvoidAreaUpdate";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    
    session->isLastFrameLayoutFinishedFunc_ = nullptr;
    session->isAINavigationBarAvoidAreaValid_ = nullptr;
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END));

    session->isLastFrameLayoutFinishedFunc_ = [](bool& isLayoutFinished) {
        isLayoutFinished = false;
        return WSError::WS_ERROR_NULLPTR;
    };
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END));

    session->isLastFrameLayoutFinishedFunc_ = [](bool& isLayoutFinished) {
        isLayoutFinished = true;
        return WSError::WS_OK;
    };

    session->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    session->specificCallback_->onGetLSState_ = []() { return true; };
    session->Session::SetRsScale(0, 0);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END));
    session->Session::SetRsScale(1, 0);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END));
    session->Session::SetRsScale(1, 1);
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END));
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_SYSTEM));
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_NAVIGATION_INDICATOR));

    session->isAINavigationBarAvoidAreaValid_ = [](DisplayId displayId,
        const AvoidArea& avoidArea, int32_t sessionBottom) {
        return true;
    };
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END));
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_NAVIGATION_INDICATOR));

    session->isAINavigationBarAvoidAreaValid_ = [](DisplayId displayId,
        const AvoidArea& avoidArea, int32_t sessionBottom) {
        return false;
    };
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END));
    EXPECT_EQ(WSError::WS_OK, session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_NAVIGATION_INDICATOR));
}

/**
 * @tc.name: GetSystemAvoidArea
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, GetSystemAvoidArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetSystemAvoidArea";
    info.bundleName_ = "GetSystemAvoidArea";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto specificCallback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sceneSession->isActive_ = true;
    SystemSessionConfig systemConfig;
    systemConfig.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sceneSession->SetSystemConfig(systemConfig);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    sceneSession->GetSessionProperty()->SetDisplayId(2024);
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(2024, screenSession));
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID));
    sceneSession->SetSessionProperty(property);

    WSRect rect1({0, 0, 10, 10});
    AvoidArea avoidArea;
    sceneSession->GetSystemAvoidArea(rect1, avoidArea);
    WSRect rect2({0, 0, 10, 10});
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetSessionProperty(property);
    sceneSession->GetSystemAvoidArea(rect2, avoidArea);
    ASSERT_EQ(avoidArea.topRect_.posX_, 0);

    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID));
    sceneSession->SetSessionProperty(property);
    ASSERT_EQ(avoidArea.topRect_.posX_, 0);
}

/**
 * @tc.name: GetAllAvoidAreas
 * @tc.desc: GetAllAvoidAreas
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, GetAllAvoidAreas, TestSize.Level1)
{
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    SessionInfo info;
    info.abilityName_ = "GetAllAvoidAreas";
    info.bundleName_ = "GetAllAvoidAreas";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_EQ(sceneSession->GetAllAvoidAreas(avoidAreas), WSError::WS_OK);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sceneSession->specificCallback_->onGetLSState_ = []() { return true; };
    sceneSession->Session::SetRsScale(0, 0);
    EXPECT_EQ(sceneSession->GetAllAvoidAreas(avoidAreas), WSError::WS_ERROR_INVALID_PARAM);
    sceneSession->Session::SetRsScale(1, 0);
    EXPECT_EQ(sceneSession->GetAllAvoidAreas(avoidAreas), WSError::WS_ERROR_INVALID_PARAM);
    sceneSession->Session::SetRsScale(1, 1);
    EXPECT_EQ(sceneSession->GetAllAvoidAreas(avoidAreas), WSError::WS_OK);
}

/*
 * @tc.name: NotifyClientToUpdateRectTask
 * @tc.desc: NotifyClientToUpdateRectTask
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, NotifyClientToUpdateRectTask, TestSize.Level1)
{
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    SessionInfo info;
    info.abilityName_ = "NotifyClientToUpdateRectTask";
    info.bundleName_ = "NotifyClientToUpdateRectTask";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->property_ = sptr<WindowSessionProperty>::MakeSptr();
    session->layoutController_ = sptr<LayoutController>::MakeSptr(session->property_);
    session->layoutController_->reason_ = SizeChangeReason::AVOID_AREA_CHANGE;
    session->foregroundInteractiveStatus_ = false;
    session->NotifyClientToUpdateRectTask("OnBoundsChanged", std::nullopt, nullptr);
    session->foregroundInteractiveStatus_ = true;
    session->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session->NotifyClientToUpdateRectTask("OnBounds", std::nullopt, nullptr);
    session->property_->SetWindowType(WindowType::WINDOW_TYPE_STATUS_BAR);
    session->NotifyClientToUpdateRectTask("OnBounds", std::nullopt, nullptr);
    session->NotifyClientToUpdateRectTask("OnBoundsChanged", std::nullopt, nullptr);
    EXPECT_EQ(session->GetSizeChangeReason(), SizeChangeReason::AVOID_AREA_CHANGE);
}

/*
 * @tc.name: GetFloatNavigationAvoidAreaForRoot
 * @tc.desc: GetFloatNavigationAvoidAreaForRoot
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, GetFloatNavigationAvoidAreaForRoot, TestSize.Level1)
{
    sptr<RootSceneSession> session = sptr<RootSceneSession>::MakeSptr();
    session->specificCallback_ = nullptr;
    WSRect rect = { 0, 0, 0, 0 };
    AvoidArea area;
    session->GetFloatNavigationAvoidAreaForRoot(rect, area, false);
    session->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    session->specificCallback_->onGetFloatNavagationInfo_ = nullptr;
    session->GetFloatNavigationAvoidAreaForRoot(rect, area, false);
    session->specificCallback_->onGetFloatNavagationInfo_ = [] (DisplayId displayId,
        std::tuple<bool, WSRect, WSRect>& floatNavagationInfo) {
        WSRect rect1;
        floatNavagationInfo = std::tuple<bool, WSRect, WSRect>(true, rect1, rect1);
        return WSError::WS_OK;
    };
    session->GetFloatNavigationAvoidAreaForRoot(rect, area, false);
    session->GetFloatNavigationAvoidAreaForRoot(rect, area, true);
    session->specificCallback_->onGetFloatNavagationInfo_ = [] (DisplayId displayId,
        std::tuple<bool, WSRect, WSRect>& floatNavagationInfo) {
        WSRect rect1;
        floatNavagationInfo = std::tuple<bool, WSRect, WSRect>(false, rect1, rect1);
        return WSError::WS_OK;
    };
    session->GetFloatNavigationAvoidAreaForRoot(rect, area, false);
    session->GetFloatNavigationAvoidAreaForRoot(rect, area, true);
    EXPECT_EQ(session->GetAvoidAreaByTypeInner(AvoidAreaType::TYPE_FLOAT_NAVIGATION), AvoidArea());
    EXPECT_EQ(session->GetAvoidAreaByTypeInner(AvoidAreaType::TYPE_NAVIGATION_INDICATOR), AvoidArea());
    session->property_ = sptr<WindowSessionProperty>::MakeSptr();
    session->property_->SetDisplayId(-1);
    session->GetFloatNavigationAvoidArea(rect, area, true);
}

/*
 * @tc.name: GetFloatNavigationAvoidArea
 * @tc.desc: GetFloatNavigationAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, GetFloatNavigationAvoidArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetFloatNavigationAvoidArea";
    info.bundleName_ = "GetFloatNavigationAvoidArea";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->specificCallback_ = nullptr;
    WSRect rect = { 0, 0, 0, 0 };
    AvoidArea area;
    session->GetFloatNavigationAvoidArea(rect, area, false);
    session->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    session->specificCallback_->onGetFloatNavagationInfo_ = nullptr;
    session->GetFloatNavigationAvoidArea(rect, area, false);
    session->specificCallback_->onGetFloatNavagationInfo_ = [] (DisplayId displayId,
        std::tuple<bool, WSRect, WSRect>& floatNavagationInfo) {
        WSRect rect1;
        floatNavagationInfo = std::tuple<bool, WSRect, WSRect>(true, rect1, rect1);
        return WSError::WS_DO_NOTHING;
    };
    session->GetFloatNavigationAvoidArea(rect, area, false);
    session->GetFloatNavigationAvoidArea(rect, area, true);
    session->specificCallback_->onGetFloatNavagationInfo_ = [] (DisplayId displayId,
        std::tuple<bool, WSRect, WSRect>& floatNavagationInfo) {
        WSRect rect1;
        floatNavagationInfo = std::tuple<bool, WSRect, WSRect>(false, rect1, rect1);
        return WSError::WS_OK;
    };
    session->GetFloatNavigationAvoidArea(rect, area, false);
    session->GetFloatNavigationAvoidArea(rect, area, true);
    session->property_ = sptr<WindowSessionProperty>::MakeSptr();
    session->property_->SetDisplayId(-1);
    session->GetFloatNavigationAvoidArea(rect, area, true);
    std::tuple<bool, WSRect, WSRect> info1;
    EXPECT_EQ(session->specificCallback_->onGetFloatNavagationInfo_(1, info1), WSError::WS_OK);
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    session->GetAvoidAreaByTypeInner(AvoidAreaType::TYPE_NAVIGATION_INDICATOR, rect);
    session->GetAvoidAreaByType(AvoidAreaType::TYPE_NAVIGATION_INDICATOR, rect);
    session->GetAvoidAreaByTypeIgnoringVisibility(AvoidAreaType::TYPE_NAVIGATION_INDICATOR, rect);
    session->GetAllAvoidAreas(avoidAreas);

    session->GetAvoidAreaByTypeInner(AvoidAreaType::TYPE_FLOAT_NAVIGATION, rect);
    session->GetAvoidAreaByType(AvoidAreaType::TYPE_FLOAT_NAVIGATION, rect);
    session->GetAvoidAreaByTypeIgnoringVisibility(AvoidAreaType::TYPE_FLOAT_NAVIGATION, rect);
    session->GetAllAvoidAreas(avoidAreas);

    session->SetFloatNavigationEnabled(true);
    session->GetAvoidAreaByTypeInner(AvoidAreaType::TYPE_FLOAT_NAVIGATION, rect);
    session->GetAvoidAreaByType(AvoidAreaType::TYPE_FLOAT_NAVIGATION, rect);
    session->GetAvoidAreaByTypeIgnoringVisibility(AvoidAreaType::TYPE_FLOAT_NAVIGATION, rect);
    session->GetAllAvoidAreas(avoidAreas);
}

/*
 * @tc.name: NotifyFloatNavigationInfo
 * @tc.desc: NotifyFloatNavigationInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, NotifyFloatNavigationInfo, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->floatNavagationInfoMap_.clear();
    WSRect rect;
    ssm_->NotifyFloatNavigationInfo(0, true, rect, rect);
    sleep(1);
    std::tuple<bool, WSRect, WSRect> info(true, rect, rect);
    EXPECT_EQ(ssm_->floatNavagationInfoMap_[0], info);
    ssm_->avoidAreaListenerSessionSet_.insert(6);
    ssm_->NotifyFloatNavigationInfo(0, false, rect, rect);
    sleep(1);
    std::tuple<bool, WSRect, WSRect> info1(false, rect, rect);
    EXPECT_EQ(ssm_->floatNavagationInfoMap_[0], info1);
    ssm_->NotifyFloatNavigationInfo(0, false, rect, rect);
    sleep(1);
}

/*
 * @tc.name: GetFloatNavagationInfo
 * @tc.desc: GetFloatNavagationInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, GetFloatNavagationInfo, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->floatNavagationInfoMap_.clear();
    WSRect rect;
    std::tuple<bool, WSRect, WSRect> info(true, rect, rect);
    ssm_->GetFloatNavagationInfo(0, info);
    EXPECT_EQ(ssm_->GetFloatNavagationInfo(0, info), WSError::WS_DO_NOTHING);
    ssm_->floatNavagationInfoMap_[0] = info;
    EXPECT_EQ(ssm_->GetFloatNavagationInfo(0, info), WSError::WS_OK);
}

/*
 * @tc.name: SetFloatNavigationEnabled
 * @tc.desc: SetFloatNavigationEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionImmersiveTest, SetFloatNavigationEnabled, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetFloatNavigationEnabled";
    info.bundleName_ = "SetFloatNavigationEnabled";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_EQ(session->SetFloatNavigationEnabled(true), WMError::WM_OK);
    EXPECT_EQ(session->SetFloatNavigationEnabled(false), WMError::WM_OK);
}
}
}
}