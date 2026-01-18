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
#include <regex>

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

class WindowImmersiveAvoidAreaTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;

private:
    void CreateSession(SessionInfo sessionInfo, int32_t persistentId);
};

sptr<SceneSessionManager> WindowImmersiveAvoidAreaTest::ssm_ = nullptr;

void WindowImmersiveAvoidAreaTest::SetUpTestCase()
{
    ssm_ = new SceneSessionManager();
    ssm_->Init();
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr();
    ssm_->rootSceneSession_->property_ = sptr<WindowSessionProperty>::MakeSptr();
}

void WindowImmersiveAvoidAreaTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void WindowImmersiveAvoidAreaTest::SetUp()
{
}

void WindowImmersiveAvoidAreaTest::TearDown()
{
}

void WindowImmersiveAvoidAreaTest::CreateSession(SessionInfo sessionInfo, int32_t persistentId)
{
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });
    ASSERT_NE(ssm_->GetSceneSession(persistentId), nullptr);
}

namespace {

/**
 * @tc.name: UpdateSessionAvoidAreaListener
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateSessionAvoidAreaListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    {
        std::unique_lock<std::shared_mutex> lock(ssm_->sceneSessionMapMutex_);
        ssm_->sceneSessionMap_.clear();
    }
    int32_t persistentId = 100;
    ssm_->UpdateSessionAvoidAreaListener(persistentId, true);

    SessionInfo info;
    info.abilityName_ = "BackgroundTask02";
    info.bundleName_ = "BackgroundTask02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({100, sceneSession});
    ssm_->UpdateSessionAvoidAreaListener(persistentId, true);
    ssm_->UpdateSessionAvoidAreaListener(persistentId, false);
}

/**
 * @tc.name: UpdateRootSceneAvoidArea
 * @tc.desc: call UpdateRootSceneAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateRootSceneAvoidArea, TestSize.Level1)
{
    EXPECT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "testbundleName";
    sessionInfo.abilityName_ = "testabilityName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ssm_->rootSceneSession_->specificCallback_ = nullptr;
    ssm_->UpdateRootSceneAvoidArea();
    auto res = ssm_->rootSceneSession_->GetPersistentId();
    EXPECT_NE(res, 0);
    sleep(1);
}

/**
 * @tc.name: UpdateRootSceneSessionAvoidArea
 * @tc.desc: UpdateRootSceneSessionAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateRootSceneSessionAvoidArea, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    int32_t persistentId = 1;
    ASSERT_NE(nullptr, ssm_->rootSceneSession_);
    bool needUpdate = false;
    ssm_->UpdateRootSceneSessionAvoidArea(persistentId, needUpdate);
    sleep(1);
}

/**
 * @tc.name: UpdateAvoidSessionAvoidArea
 * @tc.desc: UpdateAvoidSessionAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateAvoidSessionAvoidArea, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->avoidAreaListenerSessionSet_.insert(0);
    ssm_->avoidAreaListenerSessionSet_.insert(1);
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    WindowType type = WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT;
    ssm_->UpdateAvoidSessionAvoidArea(type);

    ASSERT_NE(sceneSession->property_, nullptr);
    sceneSession->property_->type_ = WindowType::APP_MAIN_WINDOW_END;
    sceneSession->isVisible_ = true;
    ssm_->UpdateAvoidSessionAvoidArea(type);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, ssm_->HandleSecureSessionShouldHide(nullptr));
    sleep(1);
}

/**
 * @tc.name: PreloadInLakeApp、UpdateSessionAvoidAreaListener
 * @tc.desc: PreloadInLakeApp、UpdateSessionAvoidAreaListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, PreloadInLakeApp, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    int32_t persistentId = 0;

    ssm_->PreloadInLakeApp("");
    sptr<SceneSession> sceneSession = nullptr;

    ssm_->UpdateSessionAvoidAreaListener(persistentId, true);
    sceneSession = ssm_->CreateSceneSession(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});
    ssm_->UpdateSessionAvoidAreaListener(persistentId, true);
}

/**
 * @tc.name: UpdateNormalSessionAvoidArea_01
 * @tc.desc: UpdateNormalSessionAvoidArea_01
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateNormalSessionAvoidArea_01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    int32_t persistentId = 1;
    sptr<SceneSession> sceneSession = nullptr;
    bool needUpdate = true;
    ssm_->UpdateNormalSessionAvoidArea(persistentId, sceneSession, needUpdate);
    EXPECT_EQ(needUpdate, false);

    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ASSERT_NE(sceneSession->property_, nullptr);
    sceneSession->property_->type_ = WindowType::APP_SUB_WINDOW_BASE;
    needUpdate = true;
    ssm_->UpdateNormalSessionAvoidArea(persistentId, sceneSession, needUpdate);
    EXPECT_EQ(needUpdate, false);

    sceneSession->property_->type_ = WindowType::APP_SUB_WINDOW_END;
    sceneSession->isVisible_ = true;
    needUpdate = true;
    ssm_->UpdateNormalSessionAvoidArea(persistentId, sceneSession, needUpdate);
    EXPECT_EQ(needUpdate, false);

    ssm_->avoidAreaListenerSessionSet_.insert(1);
    ssm_->UpdateNormalSessionAvoidArea(persistentId, sceneSession, needUpdate);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, ssm_->HandleSecureSessionShouldHide(nullptr));
    sleep(1);
}

/**
 * @tc.name: UpdateNormalSessionAvoidArea_02
 * @tc.desc: UpdateNormalSessionAvoidArea_02
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateNormalSessionAvoidArea_02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "WindowImmersiveAvoidAreaTest";
    sessionInfo.abilityName_ = "UpdateNormalSessionAvoidArea";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    int32_t persistentId = 1;
    bool needUpdate = true;
    ASSERT_NE(nullptr, ssm_);
    ssm_->avoidAreaListenerSessionSet_.clear();
    ssm_->UpdateNormalSessionAvoidArea(persistentId, sceneSession, needUpdate);
    sceneSession->isVisible_ = false;
    ssm_->UpdateNormalSessionAvoidArea(persistentId, sceneSession, needUpdate);
    sleep(1);
}

/**
 * @tc.name: UpdateNormalSessionAvoidArea_03
 * @tc.desc: UpdateNormalSessionAvoidArea_03
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateNormalSessionAvoidArea_03, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "UpdateNormalSessionAvoidArea_03";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    int32_t persistentId = 1;
    bool needUpdate = true;
    ASSERT_NE(nullptr, ssm_);
    ssm_->avoidAreaListenerSessionSet_.clear();
    ssm_->avoidAreaListenerSessionSet_.insert(persistentId);
    ssm_->UpdateNormalSessionAvoidArea(persistentId, sceneSession, needUpdate);
    sceneSession = nullptr;
    ssm_->UpdateNormalSessionAvoidArea(persistentId, sceneSession, needUpdate);
    sleep(1);
}

/**
 * @tc.name: UpdateNormalSessionAvoidArea_04
 * @tc.desc: UpdateNormalSessionAvoidArea_04
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateNormalSessionAvoidArea_04, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "UpdateNormalSessionAvoidArea_04";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->GetLayoutController()->SetSessionRect({ 1, 1, 1, 1 });
    int32_t persistentId = 1;
    bool needUpdate = true;
    ASSERT_NE(nullptr, ssm_);
    ssm_->avoidAreaListenerSessionSet_.clear();
    ssm_->avoidAreaListenerSessionSet_.insert(persistentId);
    ssm_->UpdateNormalSessionAvoidArea(persistentId, sceneSession, needUpdate);
    sleep(1);
}

/**
 * @tc.name: UpdateAvoidArea_01
 * @tc.desc: UpdateAvoidArea_01
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateAvoidArea_01, TestSize.Level1)
{
    int32_t persistentId = 0;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->UpdateAvoidArea(persistentId);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "UpdateAvoidArea_01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_STATUS_BAR);
    ssm_->sceneSessionMap_.insert(std::make_pair(persistentId, sceneSession));
    ssm_->UpdateAvoidArea(persistentId);
    sleep(1);
}

/**
 * @tc.name: UpdateAvoidArea_02
 * @tc.desc: UpdateAvoidArea_02
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateAvoidArea_02, TestSize.Level1)
{
    int32_t persistentId = 0;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->UpdateAvoidArea(persistentId);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "UpdateAvoidArea_02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_WINDOW_BASE);
    ssm_->sceneSessionMap_.insert(std::make_pair(persistentId, sceneSession));
    ssm_->UpdateAvoidArea(persistentId);
    sleep(1);
}

/**
 * @tc.name: UpdateAvoidArea_03
 * @tc.desc: UpdateAvoidArea_03
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateAvoidArea_03, TestSize.Level1)
{
    int32_t persistentId = 0;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->UpdateAvoidArea(persistentId);
    sleep(1);
}

/**
 * @tc.name: NotifyNextAvoidRectInfo_01
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_01
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, NotifyNextAvoidRectInfo_01, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    WSRect portraitRect = { 0, 0, 1260, 123 };
    WSRect landspaceRect = { 0, 0, 2720, 123 };
    auto ret = ssm_->NotifyNextAvoidRectInfo(AvoidAreaType::TYPE_SYSTEM, portraitRect, landspaceRect, 0);
    ASSERT_EQ(ret, WSError::WS_OK);
    std::pair<WSRect, WSRect> nextSystemBarAvoidAreaRectInfo;
    ret = ssm_->GetNextAvoidRectInfo(0, AvoidAreaType::TYPE_SYSTEM, nextSystemBarAvoidAreaRectInfo);
    ASSERT_EQ(ret, WSError::WS_OK);
    ret = ssm_->GetNextAvoidRectInfo(0, AvoidAreaType::TYPE_NAVIGATION_INDICATOR, nextSystemBarAvoidAreaRectInfo);
    ASSERT_EQ(ret, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: NotifyNextAvoidRectInfo_02
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_02
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, NotifyNextAvoidRectInfo_02, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    WSRect portraitRect = { 0, 0, 1260, 123 };
    WSRect landspaceRect = { 0, 0, 2720, 123 };
    auto ret = ssm_->NotifyNextAvoidRectInfo(AvoidAreaType::TYPE_SYSTEM, portraitRect, landspaceRect, 0);
    ASSERT_EQ(ret, WSError::WS_OK);
    SessionInfo info;
    info.abilityName_ = "NotifyNextAvoidRectInfo";
    info.bundleName_ = "NotifyNextAvoidRectInfo";
    info.screenId_ = 0;
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onGetNextAvoidAreaRectInfo_ = [](
        DisplayId displayId, AvoidAreaType type, std::pair<WSRect, WSRect>& nextSystemBarAvoidAreaRectInfo) {
        return ssm_->GetNextAvoidRectInfo(displayId, type, nextSystemBarAvoidAreaRectInfo);
    };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->property_->SetPersistentId(1);
    sceneSession->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 2720 });
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    std::pair<WSRect, WSRect> nextSystemBarAvoidAreaRectInfo;
    ret = sceneSession->specificCallback_->onGetNextAvoidAreaRectInfo_(
        0, AvoidAreaType::TYPE_SYSTEM, nextSystemBarAvoidAreaRectInfo);
    ASSERT_EQ(ret, WSError::WS_OK);
    ASSERT_EQ(nextSystemBarAvoidAreaRectInfo.first, portraitRect);
    ASSERT_EQ(nextSystemBarAvoidAreaRectInfo.second, landspaceRect);
    ssm_->sceneSessionMap_.clear();
}

/*
 * @tc.name: GetScaleInLSState
 * @tc.desc: SceneSesion test GetScaleInLSState
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, GetScaleInLSState, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "testbundleName";
    sessionInfo.abilityName_ = "testabilityName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    float scaleX = 1;
    float scaleY = 1;
    WSRect winRect = { 0, 0, 0, 0 };
    Rect avoidAreaRect = { 0, 0, 0, 0 };
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->layoutController_ = sptr<LayoutController>::MakeSptr(property);
    sceneSession->Session::SetScale(-1, -1, -1, -1);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    EXPECT_EQ(sceneSession->GetScaleInLSState(scaleX, scaleY), WSError::WS_DO_NOTHING);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    EXPECT_EQ(sceneSession->GetScaleInLSState(scaleX, scaleY), WSError::WS_DO_NOTHING);
    sceneSession->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_EQ(sceneSession->GetScaleInLSState(scaleX, scaleY), WSError::WS_DO_NOTHING);
    sceneSession->specificCallback_->onGetLSState_ = []() { return false; };
    EXPECT_EQ(sceneSession->GetScaleInLSState(scaleX, scaleY), WSError::WS_DO_NOTHING);
    sceneSession->specificCallback_->onGetLSState_ = []() { return true; };
    EXPECT_EQ(sceneSession->GetScaleInLSState(scaleX, scaleY), WSError::WS_ERROR_INVALID_PARAM);
    sceneSession->Session::SetScale(1, -1, -1, -1);
    EXPECT_EQ(sceneSession->GetScaleInLSState(scaleX, scaleY), WSError::WS_ERROR_INVALID_PARAM);
    sceneSession->CalculateWindowRectByScale(winRect);
    sceneSession->CalculateAvoidAreaByScale(avoidAreaRect);
    sceneSession->Session::SetScale(1, 1, -1, -1);
    EXPECT_EQ(sceneSession->GetScaleInLSState(scaleX, scaleY), WSError::WS_OK);
    sceneSession->CalculateWindowRectByScale(winRect);
    sceneSession->CalculateAvoidAreaByScale(avoidAreaRect);
}

/**
 * @tc.name: UpdateAvoidAreaForLSStateChange
 * @tc.desc: SceneSesionManager test UpdateAvoidAreaForLSStateChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateAvoidAreaForLSStateChange, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->UpdateAvoidAreaForLSStateChange(1, 1);
    ssm_->sceneSessionMap_.clear();
    ssm_->UpdateAvoidAreaForLSStateChange(1, 2);
    SessionInfo sessionInfo1;
    SessionInfo sessionInfo2;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sceneSession->isScbCoreEnabled_ = true;
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession2->property_ = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession2->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->sceneSessionMap_.insert({ 2, nullptr });
    ssm_->sceneSessionMap_.insert({ 2, sceneSession2 });
    ssm_->UpdateAvoidAreaForLSStateChange(1, 2);
    sleep(1);
}

/**
 * @tc.name: UpdateAvoidArea
 * @tc.desc: UpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateAvoidArea, TestSize.Level1)
{
    int32_t persistentId = 0;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr();
    ssm_->UpdateAvoidArea(persistentId);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "WindowImmersiveAvoidAreaTest";
    sessionInfo.abilityName_ = "UpdateAvoidArea";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(persistentId, sceneSession));
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_STATUS_BAR);
    ssm_->UpdateAvoidArea(persistentId);
    sceneSession->property_->SetWindowType(WindowType::APP_WINDOW_BASE);
    ssm_->UpdateAvoidArea(persistentId);
}

/**
 * @tc.name: UpdateNormalSessionAvoidArea02
 * @tc.desc: UpdateNormalSessionAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, UpdateNormalSessionAvoidArea02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "WindowImmersiveAvoidAreaTest";
    sessionInfo.abilityName_ = "UpdateNormalSessionAvoidArea02";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->GetLayoutController()->SetSessionRect({ 1, 1, 1, 1 });
    int32_t persistentId = 1;
    bool needUpdate = true;
    ASSERT_NE(nullptr, ssm_);
    ssm_->avoidAreaListenerSessionSet_.clear();
    ssm_->avoidAreaListenerSessionSet_.insert(persistentId);
    ssm_->UpdateNormalSessionAvoidArea(persistentId, sceneSession, needUpdate);
}

/**
 * @tc.name: NotifyNextAvoidRectInfo_statusBar
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_statusBar
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, NotifyNextAvoidRectInfo_statusBar, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    WSRect portraitRect = { 0, 0, 1260, 123 };
    WSRect landspaceRect = { 0, 0, 2720, 123 };
    auto ret = ssm_->NotifyNextAvoidRectInfo(AvoidAreaType::TYPE_SYSTEM, portraitRect, landspaceRect, 0);
    ASSERT_EQ(ret, WSError::WS_OK);
    SessionInfo info;
    info.abilityName_ = "NotifyNextAvoidRectInfo_statusBar";
    info.bundleName_ = "NotifyNextAvoidRectInfo_statusBar";
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
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_0, { 0, 0, 1260, 2720 }, properties, avoidAreas);
    Rect rect = { 0, 0, 1260, 123 };
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_SYSTEM].topRect_, rect);
    properties[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = false;
    rect = { 0, 0, 0, 0 };
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_0, { 0, 0, 1260, 123 }, properties, avoidAreas);
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_SYSTEM].topRect_, rect);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyNextAvoidRectInfo_statusBar_01
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_statusBar_01
 * @tc.type: FUNC
 */
HWTEST_F(WindowImmersiveAvoidAreaTest, NotifyNextAvoidRectInfo_statusBar_01, TestSize.Level0)
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
HWTEST_F(WindowImmersiveAvoidAreaTest, NotifyNextAvoidRectInfo_keyboard, TestSize.Level0)
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
HWTEST_F(WindowImmersiveAvoidAreaTest, NotifyNextAvoidRectInfo_keyboard_01, TestSize.Level0)
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
HWTEST_F(WindowImmersiveAvoidAreaTest, NotifyNextAvoidRectInfo_AIBar, TestSize.Level0)
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
HWTEST_F(WindowImmersiveAvoidAreaTest, GetKeyboardAvoidArea, TestSize.Level1)
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
HWTEST_F(WindowImmersiveAvoidAreaTest, GetCutoutAvoidArea, TestSize.Level1)
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
HWTEST_F(WindowImmersiveAvoidAreaTest, GetAvoidAreaByType, TestSize.Level1)
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
HWTEST_F(WindowImmersiveAvoidAreaTest, GetAvoidAreaByTypeIgnoringVisibility, TestSize.Level1)
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
HWTEST_F(WindowImmersiveAvoidAreaTest, CalculateAvoidAreaRect, TestSize.Level1)
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
}
}
}