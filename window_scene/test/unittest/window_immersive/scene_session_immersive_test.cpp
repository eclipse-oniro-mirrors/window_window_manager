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
}
}
}