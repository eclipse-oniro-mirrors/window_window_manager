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

#include <application_context.h>
#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>
#include <gtest/gtest.h>
#include <regex>

#include "context.h"
#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session_info.h"
#include "session_manager.h"
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/main_session.h"
#include "session/host/include/pc_fold_screen_controller.h"
#include "session/host/include/scene_session.h"
#include "window_manager_agent.h"
#include "zidl/window_manager_agent_interface.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_logMsg = msg;
    }
}
class SceneSessionManagerTest4 : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp() override;

    void TearDown() override;

    static bool gestureNavigationEnabled_;

    static ProcessGestureNavigationEnabledChangeFunc callbackFunc_;
    static sptr<SceneSessionManager> ssm_;
    ScreenSessionManagerClient* screenSessionManagerClient_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerTest4::ssm_ = nullptr;
bool SceneSessionManagerTest4::gestureNavigationEnabled_ = true;

ProcessGestureNavigationEnabledChangeFunc SceneSessionManagerTest4::callbackFunc_ =
    [](bool enable, const std::string& bundleName, GestureBackType type) { gestureNavigationEnabled_ = enable; };

void SceneSessionManagerTest4::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerTest4::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest4::SetUp()
{
    ssm_->sceneSessionMap_.clear();
    screenSessionManagerClient_ = &ScreenSessionManagerClient::GetInstance();
}

void SceneSessionManagerTest4::TearDown()
{
    ssm_->sceneSessionMap_.clear();
    screenSessionManagerClient_ = nullptr;
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: UpdateSceneSessionWant01
 * @tc.desc: SceneSesionManager test UpdateSceneSessionWant
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateSceneSessionWant01, TestSize.Level1)
{
    SessionInfo info;
    info.persistentId_ = 0;
    ssm_->UpdateSceneSessionWant(info);
    ASSERT_NE(ssm_, nullptr);
}

/**
 * @tc.name: UpdateSceneSessionWant02
 * @tc.desc: SceneSesionManager test UpdateSceneSessionWant
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateSceneSessionWant02, TestSize.Level1)
{
    SessionInfo info;
    info.persistentId_ = 1;
    ssm_->UpdateSceneSessionWant(info);
    ASSERT_NE(ssm_, nullptr);
}

/**
 * @tc.name: UpdateSceneSessionWant03
 * @tc.desc: SceneSesionManager test UpdateSceneSessionWant
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateSceneSessionWant03, TestSize.Level1)
{
    SessionInfo info;
    info.persistentId_ = 1;
    auto sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->UpdateSceneSessionWant(info);
    ASSERT_NE(ssm_, nullptr);
}

/**
 * @tc.name: UpdateSceneSessionWant04
 * @tc.desc: SceneSesionManager test UpdateSceneSessionWant
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateSceneSessionWant04, TestSize.Level1)
{
    SessionInfo info;
    info.persistentId_ = 1;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    info.want = want;
    auto sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->UpdateSceneSessionWant(info);
    ASSERT_NE(ssm_, nullptr);
}

/**
 * @tc.name: UpdateSceneSessionWant05
 * @tc.desc: SceneSesionManager test UpdateSceneSessionWant
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateSceneSessionWant05, TestSize.Level1)
{
    SessionInfo info;
    info.persistentId_ = 1;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    info.want = want;
    auto sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    int32_t collaboratorType = CollaboratorType::RESERVE_TYPE;
    sceneSession->SetCollaboratorType(collaboratorType);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->UpdateSceneSessionWant(info);
    ASSERT_NE(ssm_, nullptr);
}

/**
 * @tc.name: UpdateSceneSessionWant06
 * @tc.desc: SceneSesionManager test UpdateSceneSessionWant
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateSceneSessionWant06, TestSize.Level1)
{
    SessionInfo info;
    info.persistentId_ = 1;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    info.want = want;
    auto sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    int32_t collaboratorType = -1;
    sceneSession->SetCollaboratorType(collaboratorType);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->UpdateSceneSessionWant(info);
    ASSERT_NE(ssm_, nullptr);
}

/**
 * @tc.name: GetSceneSessionByIdentityInfo01
 * @tc.desc: SceneSesionManager test GetSceneSessionByIdentityInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetSceneSessionByIdentityInfo01, TestSize.Level1)
{
    std::string abilityName = "test1";
    std::string bundleName = "test2";
    std::string moduleName = "test3";
    int32_t appIndex = 10;
    SessionInfo info;
    info.abilityName_ = abilityName;
    info.bundleName_ = bundleName;
    info.moduleName_ = moduleName;
    info.appIndex_ = appIndex;
    info.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    SessionIdentityInfo identityInfo = { bundleName, moduleName, abilityName, appIndex };
    ASSERT_NE(ssm_->GetSceneSessionByIdentityInfo(identityInfo), nullptr);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSession01
 * @tc.desc: SceneSesionManager test DestroyAndDisconnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, DestroyAndDisconnectSpecificSession01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DestroyAndDisconnectSpecificSession";
    info.bundleName_ = "DestroyAndDisconnectSpecificSession";
    info.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ASSERT_NE(ssm_->DestroyAndDisconnectSpecificSession(1), WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyEnterRecentTask02
 * @tc.desc: NotifyEnterRecentTask
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, NotifyEnterRecentTask02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    EXPECT_EQ(ssm_->NotifyEnterRecentTask(false), WSError::WS_OK);
}

/**
 * @tc.name: GetWindowStatus
 * @tc.desc: GetWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetWindowStatus, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    WindowMode mode = WindowMode::WINDOW_MODE_FLOATING;
    SessionState sessionState = SessionState::STATE_FOREGROUND;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    auto result = ssm_->GetWindowStatus(mode, sessionState, nullptr);
    EXPECT_EQ(result, WindowStatus::WINDOW_STATUS_UNDEFINED);
}

/**
 * @tc.name: GetWindowStatus02
 * @tc.desc: GetWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetWindowStatus02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    WindowMode mode = WindowMode::WINDOW_MODE_FLOATING;
    SessionState sessionState = SessionState::STATE_FOREGROUND;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    auto result = ssm_->GetWindowStatus(mode, sessionState, nullptr);
    property->SetMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
    result = ssm_->GetWindowStatus(mode, sessionState, property);
    EXPECT_EQ(result, WindowStatus::WINDOW_STATUS_MAXIMIZE);
}

/**
 * @tc.name: GetWindowStatus03
 * @tc.desc: GetWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetWindowStatus03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    WindowMode mode = WindowMode::WINDOW_MODE_FLOATING;
    SessionState sessionState = SessionState::STATE_FOREGROUND;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    auto result = ssm_->GetWindowStatus(mode, sessionState, nullptr);
    property->SetMaximizeMode(MaximizeMode::MODE_FULL_FILL);
    result = ssm_->GetWindowStatus(mode, sessionState, property);
    EXPECT_EQ(result, WindowStatus::WINDOW_STATUS_FLOATING);
}

/**
 * @tc.name: GetWindowStatus04
 * @tc.desc: GetWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetWindowStatus04, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    WindowMode mode = WindowMode::WINDOW_MODE_FLOATING;
    SessionState sessionState = SessionState::STATE_FOREGROUND;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    mode = WindowMode::WINDOW_MODE_SPLIT_PRIMARY;
    auto result = ssm_->GetWindowStatus(mode, sessionState, property);
    EXPECT_EQ(result, WindowStatus::WINDOW_STATUS_SPLITSCREEN);
}

/**
 * @tc.name: UpdateDisplayRegion
 * @tc.desc: UpdateDisplayRegion
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateDisplayRegion, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->UpdateDisplayRegion(nullptr);
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(displayInfo, nullptr);

    constexpr DisplayId displayId = 0;
    constexpr int32_t width = 100;
    displayInfo->SetDisplayId(displayId);
    displayInfo->SetWidth(width);
    ssm_->UpdateDisplayRegion(displayInfo);

    constexpr int32_t height = 200;
    displayInfo->SetWidth(width);
    displayInfo->SetHeight(height);
    ssm_->UpdateDisplayRegion(displayInfo);

    displayInfo->SetHeight(height);
    PcFoldScreenManager::GetInstance().SetDisplayInfo(displayId, SuperFoldStatus::HALF_FOLDED);
    PcFoldScreenManager::GetInstance().SetDisplayRects(WSRect::EMPTY_RECT, { 0, 0, width, height }, WSRect::EMPTY_RECT);
    ssm_->UpdateDisplayRegion(displayInfo);
    ASSERT_NE(ssm_->displayRegionMap_.count(displayId), 0);
    auto region = ssm_->displayRegionMap_[displayId];
    ASSERT_NE(region, nullptr);
    const SkIRect& rect = region->getBounds();
    EXPECT_EQ(rect.fLeft, 0);
    EXPECT_EQ(rect.fTop, 0);
    EXPECT_EQ(rect.fRight, width);
    EXPECT_EQ(rect.fBottom, height);
}

/**
 * @tc.name: GetDisplayRegion
 * @tc.desc: GetDisplayRegion
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetDisplayRegion, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    constexpr DisplayId displayId = 0;
    auto region = ssm_->GetDisplayRegion(displayId);
    EXPECT_NE(region, nullptr);

    constexpr int32_t left = 0;
    constexpr int32_t top = 0;
    constexpr int32_t right = 100;
    constexpr int32_t bottom = 200;
    SkIRect rect{ .fLeft = left, .fTop = top, .fRight = right, .fBottom = bottom };
    ssm_->displayRegionMap_[displayId] = std::make_shared<SkRegion>(rect);
    auto region1 = ssm_->GetDisplayRegion(displayId);
    ASSERT_NE(region1, nullptr);
    const SkIRect& rect1 = region1->getBounds();
    EXPECT_EQ(rect1.fLeft, 0);
    EXPECT_EQ(rect1.fTop, 0);
    EXPECT_EQ(rect1.fRight, right);
    EXPECT_EQ(rect1.fBottom, bottom);
}

/**
 * @tc.name: GetCustomDecorHeight
 * @tc.desc: GetCustomDecorHeight
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetCustomDecorHeight, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetCustomDecorHeight(50);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, nullptr));
    EXPECT_EQ(0, ssm_->GetCustomDecorHeight(2));
    EXPECT_EQ(50, ssm_->GetCustomDecorHeight(1));
}

/**
 * @tc.name: IsVectorSame
 * @tc.desc: IsVectorSame
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, IsVectorSame, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<VisibleWindowNumInfo> lastInfo;
    std::vector<VisibleWindowNumInfo> currentInfo;
    VisibleWindowNumInfo num;
    lastInfo.push_back(num);
    EXPECT_EQ(false, ssm_->IsVectorSame(lastInfo, currentInfo));
    lastInfo.clear();
}

/**
 * @tc.name: IsVectorSame01
 * @tc.desc: IsVectorSame
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, IsVectorSame01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<VisibleWindowNumInfo> lastInfo;
    std::vector<VisibleWindowNumInfo> currentInfo;
    lastInfo.reserve(2);
    VisibleWindowNumInfo oneNum;
    oneNum.displayId = 0;
    oneNum.visibleWindowNum = 3;
    lastInfo.push_back(oneNum);
    currentInfo.push_back(oneNum);
    EXPECT_EQ(true, ssm_->IsVectorSame(lastInfo, currentInfo));
}

/**
 * @tc.name: IsVectorSame03
 * @tc.desc: IsVectorSame
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, IsVectorSame03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<VisibleWindowNumInfo> lastInfo;
    std::vector<VisibleWindowNumInfo> currentInfo;
    lastInfo.clear();
    currentInfo.clear();
    VisibleWindowNumInfo twoNum;
    twoNum.displayId = 0;
    twoNum.visibleWindowNum = 2;
    currentInfo.push_back(twoNum);
    EXPECT_EQ(false, ssm_->IsVectorSame(lastInfo, currentInfo));
}

/**
 * @tc.name: IsVectorSame04
 * @tc.desc: IsVectorSame
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, IsVectorSame04, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<VisibleWindowNumInfo> lastInfo;
    std::vector<VisibleWindowNumInfo> currentInfo;
    VisibleWindowNumInfo twoNum;
    currentInfo.clear();
    twoNum.displayId = 1;
    twoNum.visibleWindowNum = 3;
    currentInfo.push_back(twoNum);
    EXPECT_EQ(false, ssm_->IsVectorSame(lastInfo, currentInfo));
}

/**
 * @tc.name: CacVisibleWindowNum
 * @tc.desc: CacVisibleWindowNum
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, CacVisibleWindowNum, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession03 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession04 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ASSERT_NE(sceneSession03, nullptr);
    ASSERT_NE(sceneSession04, nullptr);
    ASSERT_NE(sceneSession01->property_, nullptr);
    sceneSession01->property_->type_ = WindowType::WINDOW_TYPE_WALLPAPER;
    sceneSession01->SetRSVisible(true);
    sceneSession02->state_ = SessionState::STATE_BACKGROUND;
    ASSERT_NE(sceneSession03->property_, nullptr);
    sceneSession03->property_->type_ = WindowType::APP_MAIN_WINDOW_END;
    ASSERT_NE(sceneSession04->property_, nullptr);
    sceneSession04->property_->type_ = WindowType::WINDOW_TYPE_WALLPAPER;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession03));
    ssm_->sceneSessionMap_.insert(std::make_pair(4, sceneSession04));
    ssm_->sceneSessionMap_.insert(std::make_pair(5, nullptr));

    ssm_->CacVisibleWindowNum();
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, ssm_->HandleSecureSessionShouldHide(nullptr));
}

/**
 * @tc.name: GetAppMainSceneSession
 * @tc.desc: GetAppMainSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetAppMainSceneSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession;
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(0, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    EXPECT_EQ(ssm_->GetAppMainSceneSession(0, sceneSession), WSError::WS_ERROR_INVALID_SESSION);

    EXPECT_EQ(ssm_->GetAppMainSceneSession(1, sceneSession01), WSError::WS_OK);

    ASSERT_NE(sceneSession01->property_, nullptr);
    sceneSession01->property_->type_ = WindowType::WINDOW_TYPE_MEDIA;
    EXPECT_EQ(ssm_->GetAppMainSceneSession(1, sceneSession01), WSError::WS_ERROR_INVALID_CALLING);

    sceneSession01->property_->type_ = WindowType::WINDOW_TYPE_APP_SUB_WINDOW;
    sceneSession01->property_->SetParentPersistentId(2);
    EXPECT_EQ(ssm_->GetAppMainSceneSession(1, sceneSession01), WSError::WS_OK);
}

/**
 * @tc.name: GetImmersiveState01
 * @tc.desc: GetImmersiveState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetImmersiveState01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    SessionInfo info;
    info.abilityName_ = "GetImmersiveState01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    sceneSession->property_->type_ = WindowType::APP_MAIN_WINDOW_BASE;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(false, ssm_->GetImmersiveState(0u));
    sceneSession->SetIsLayoutFullScreen(true);
    EXPECT_EQ(true, ssm_->GetImmersiveState(0u));
}

/**
 * @tc.name: GetImmersiveState02
 * @tc.desc: GetImmersiveState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetImmersiveState02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;

    SessionInfo info;
    info.abilityName_ = "GetImmersiveState02";
    sptr<SceneSession> sceneSession01;
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_EQ(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));

    EXPECT_EQ(false, ssm_->GetImmersiveState(0u));
    sceneSession02->property_->type_ = WindowType::APP_MAIN_WINDOW_END;
    EXPECT_EQ(false, ssm_->GetImmersiveState(0u));
    sceneSession02->property_->type_ = WindowType::APP_MAIN_WINDOW_BASE;
    EXPECT_EQ(false, ssm_->GetImmersiveState(0u));
    sceneSession02->state_ = SessionState::STATE_ACTIVE;
    EXPECT_EQ(false, ssm_->GetImmersiveState(0u));
    sceneSession02->state_ = SessionState::STATE_FOREGROUND;
    EXPECT_EQ(false, ssm_->GetImmersiveState(0u));
    sceneSession02->property_->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    EXPECT_EQ(false, ssm_->GetImmersiveState(0u));
    sceneSession02->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sceneSession02->property_->sysBarPropMap_[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = false;
    EXPECT_EQ(true, ssm_->GetImmersiveState(0u));
    sceneSession02->property_->sysBarPropMap_[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = true;
    EXPECT_EQ(false, ssm_->GetImmersiveState(0u));
}

/**
 * @tc.name: UpdateSessionDisplayId
 * @tc.desc: UpdateSessionDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateSessionDisplayId, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto result = ssm_->UpdateSessionDisplayId(0, 0);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_WINDOW);

    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    sceneSession->sessionInfo_.screenId_ = 6;
    sceneSession->SetPropertyDirtyFlags(0);
    result = ssm_->UpdateSessionDisplayId(1, 2);
    EXPECT_EQ(result, WSError::WS_OK);
    EXPECT_EQ(sceneSession->GetPropertyDirtyFlags(), static_cast<uint32_t>(SessionPropertyFlag::DISPLAY_ID));
}

/**
 * @tc.name: UpdateSessionWindowVisibilityListener02
 * @tc.desc: UpdateSessionWindowVisibilityListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateSessionWindowVisibilityListener02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    int32_t persistentId = 1;
    auto result = ssm_->UpdateSessionWindowVisibilityListener(persistentId, true);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: NotifyScreenshotEvent
 * @tc.desc: test WS_OK
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, NotifyScreenshotEvent, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->NotifyScreenshotEvent(ScreenshotEventType::SCROLL_SHOT_START);
    EXPECT_EQ(ret, WMError::WM_OK);

    ssm_->screenshotAppEventListenerSessionSet_.insert(1);
    ret = ssm_->NotifyScreenshotEvent(ScreenshotEventType::SCROLL_SHOT_START);
    EXPECT_EQ(ret, WMError::WM_OK);

    SessionInfo info;
    info.abilityName_ = "NotifyScreenshotEvent";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetPersistentId(1);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ret = ssm_->NotifyScreenshotEvent(ScreenshotEventType::SCROLL_SHOT_START);
    EXPECT_EQ(ret, WMError::WM_OK);

    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    ret = ssm_->NotifyScreenshotEvent(ScreenshotEventType::SCROLL_SHOT_START);
    EXPECT_EQ(ret, WMError::WM_OK);

    sceneSession->SetSessionState(SessionState::STATE_BACKGROUND);
    ret = ssm_->NotifyScreenshotEvent(ScreenshotEventType::SCROLL_SHOT_START);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: UpdateSessionScreenshotAppEventListener01
 * @tc.desc: test WM_ERROR_NULLPTR
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateSessionScreenshotAppEventListener01, TestSize.Level1)
{
    int32_t persistentId = 10086;
    bool haveListener = true;
    WMError ret = ssm_->UpdateSessionScreenshotAppEventListener(persistentId, haveListener);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateSessionScreenshotAppEventListener02
 * @tc.desc: test WS_OK
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateSessionScreenshotAppEventListener02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "UpdateSessionScreenshotAppEventListener";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    int32_t persistentId = 1;
    auto ret = ssm_->UpdateSessionScreenshotAppEventListener(persistentId, true);
    EXPECT_EQ(ret, WMError::WM_OK);
    ret = ssm_->UpdateSessionScreenshotAppEventListener(persistentId, false);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: UpdateDarkColorModeToRS
 * @tc.desc: UpdateDarkColorModeToRS
 * @tc.type: FUNC
 * @tc.require: issueIB1N43
 */
HWTEST_F(SceneSessionManagerTest4, UpdateDarkColorModeToRS, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    AbilityRuntime::ApplicationContext::applicationContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    ASSERT_NE(nullptr, AbilityRuntime::ApplicationContext::applicationContext_);
    AbilityRuntime::ApplicationContext::applicationContext_->contextImpl_ =
        std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(nullptr, AbilityRuntime::ApplicationContext::applicationContext_->contextImpl_);
    AbilityRuntime::ApplicationContext::applicationContext_->contextImpl_->config_ =
        std::make_shared<AppExecFwk::Configuration>();
    ASSERT_NE(nullptr, AbilityRuntime::ApplicationContext::applicationContext_->GetConfiguration());
    ssm_->UpdateDarkColorModeToRS();
}

/**
 * @tc.name: NotifySessionAINavigationBarChange
 * @tc.desc: NotifySessionAINavigationBarChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, NotifySessionAINavigationBarChange, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->NotifySessionAINavigationBarChange(0);
    ssm_->NotifySessionAINavigationBarChange(1);

    ASSERT_NE(sceneSession->property_, nullptr);
    sceneSession->property_->type_ = WindowType::APP_SUB_WINDOW_END;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    ssm_->NotifySessionAINavigationBarChange(1);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, ssm_->HandleSecureSessionShouldHide(nullptr));

    ssm_->sceneSessionMap_.clear();
    auto persistentId = sceneSession->GetPersistentId();
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });

    sceneSession->SetScbCoreEnabled(true);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ssm_->NotifySessionAINavigationBarChange(persistentId);
    EXPECT_EQ(WSError::WS_OK, ssm_->HandleSecureSessionShouldHide(sceneSession));
}

/**
 * @tc.name: FindSessionByAffinity
 * @tc.desc: FindSessionByAffinity
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, FindSessionByAffinity, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::string affinity = "";
    sptr<SceneSession> sceneSession = ssm_->FindSessionByAffinity(affinity);
    EXPECT_EQ(sceneSession, nullptr);
}

/**
 * @tc.name: SetSystemAnimatedScenes
 * @tc.desc: SetSystemAnimatedScenes
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, SetSystemAnimatedScenes, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SystemAnimatedSceneType sceneType = SystemAnimatedSceneType::SCENE_ENTER_MISSION_CENTER;
    auto result = ssm_->SetSystemAnimatedScenes(sceneType);
    EXPECT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: GetProcessDrawingState
 * @tc.desc: GetProcessDrawingState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetProcessDrawingState, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    uint64_t windowId = 10;
    int32_t pid = 1;
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession01 = nullptr;
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession03 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession04 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ASSERT_NE(sceneSession03, nullptr);
    ASSERT_NE(sceneSession04, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession03));
    ssm_->sceneSessionMap_.insert(std::make_pair(4, sceneSession04));
    sceneSession02->SetCallingPid(pid);
    struct RSSurfaceNodeConfig config;
    sceneSession02->surfaceNode_ = RSSurfaceNode::Create(config);
    ASSERT_NE(sceneSession02->surfaceNode_, nullptr);
    sceneSession03->SetCallingPid(pid);
    sceneSession03->surfaceNode_ = nullptr;
    sceneSession04->SetCallingPid(6);
    auto result = ssm_->GetProcessDrawingState(windowId, pid);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GetPreWindowDrawingState
 * @tc.desc: GetPreWindowDrawingState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetPreWindowDrawingState, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    uint64_t surfaceId = 0;
    int32_t pid = 10;
    bool result = ssm_->GetPreWindowDrawingState(surfaceId, true, pid);
    EXPECT_EQ(result, false);

    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession01, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    struct RSSurfaceNodeConfig config;
    sceneSession01->surfaceNode_ = RSSurfaceNode::Create(config);
    ASSERT_NE(sceneSession01->surfaceNode_, nullptr);
    sceneSession01->surfaceNode_->id_ = 10;
    surfaceId = 10;
    result = ssm_->GetPreWindowDrawingState(surfaceId, true, pid);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetWindowDrawingContentChangeInfo
 * @tc.desc: GetWindowDrawingContentChangeInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetWindowDrawingContentChangeInfo, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(0, nullptr));
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    std::vector<std::pair<uint64_t, bool>> currDrawingContentData;
    currDrawingContentData.push_back(std::make_pair(0, false));
    currDrawingContentData.push_back(std::make_pair(1, true));

    struct RSSurfaceNodeConfig config;
    sceneSession->surfaceNode_ = RSSurfaceNode::Create(config);
    ASSERT_NE(sceneSession->surfaceNode_, nullptr);
    sceneSession->surfaceNode_->id_ = 0;
    sceneSession->SetCallingPid(0);
    sceneSession->SetDrawingContentState(true);

    auto result = ssm_->GetWindowDrawingContentChangeInfo(currDrawingContentData);
    EXPECT_EQ(result, currDrawingContentData);

    sceneSession->SetCallingPid(2);
    result = ssm_->GetWindowDrawingContentChangeInfo(currDrawingContentData);
    EXPECT_NE(result, currDrawingContentData);
}

/**
 * @tc.name: GetWindowDrawingContentChangeInfo02
 * @tc.desc: GetWindowDrawingContentChangeInfo02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetWindowDrawingContentChangeInfo02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "GetWindowDrawingContentChangeInfo02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(0, nullptr));

    std::vector<std::pair<uint64_t, bool>> currDrawingContentData;
    currDrawingContentData.push_back(std::make_pair(0, false));
    currDrawingContentData.push_back(std::make_pair(1, true));

    auto result = ssm_->GetWindowDrawingContentChangeInfo(currDrawingContentData);
    EXPECT_EQ(result, currDrawingContentData);
}

/**
 * @tc.name: DealwithDrawingContentChange
 * @tc.desc: DealwithDrawingContentChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, DealwithDrawingContentChange, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(0, nullptr));

    std::vector<std::pair<uint64_t, bool>> drawingContentChangeInfo;
    drawingContentChangeInfo.push_back(std::make_pair(0, true));
    drawingContentChangeInfo.push_back(std::make_pair(1, true));
    ssm_->DealwithDrawingContentChange(drawingContentChangeInfo);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    struct RSSurfaceNodeConfig config;
    sceneSession->surfaceNode_ = RSSurfaceNode::Create(config);
    ASSERT_NE(sceneSession->surfaceNode_, nullptr);
    sceneSession->surfaceNode_->id_ = 1;
    ssm_->DealwithDrawingContentChange(drawingContentChangeInfo);

    ssm_->openDebugTrace_ = true;
    ssm_->DealwithDrawingContentChange(drawingContentChangeInfo);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, ssm_->HandleSecureSessionShouldHide(nullptr));
}

/**
 * @tc.name: GetSubSceneSession
 * @tc.desc: GetSubSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetSubSceneSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "GetSubSceneSession";
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession03 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<Session> session04 = sptr<Session>::MakeSptr(info);
    sptr<Session> session05 = sptr<Session>::MakeSptr(info);
    ASSERT_NE(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ASSERT_NE(sceneSession03, nullptr);
    ASSERT_NE(session04, nullptr);
    ASSERT_NE(session05, nullptr);
    sceneSession01->persistentId_ = 1;
    sceneSession02->persistentId_ = 2;
    sceneSession03->persistentId_ = 3;
    session04->persistentId_ = 4;
    session05->persistentId_ = 5;
    ssm_->sceneSessionMap_.insert(std::make_pair(0, nullptr));
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession01->GetPersistentId(), sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession02->GetPersistentId(), sceneSession02));
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession03->GetPersistentId(), sceneSession03));
    session04->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession01->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession01->parentSession_ = session04;
    session05->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession02->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession02->parentSession_ = session05;
    sceneSession03->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession03->parentSession_ = sceneSession01;
    std::vector<sptr<SceneSession>> subSessions = ssm_->GetSubSceneSession(session04->GetPersistentId());
    EXPECT_EQ(subSessions.size(), 2);
}

/**
 * @tc.name: RemoveDuplicateSubSession
 * @tc.desc: RemoveDuplicateSubSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, RemoveDuplicateSubSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfo;
    std::vector<sptr<SceneSession>> subSessions;
    visibilityChangeInfo.push_back(std::make_pair(0, WINDOW_VISIBILITY_STATE_NO_OCCLUSION));
    visibilityChangeInfo.push_back(std::make_pair(1, WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION));

    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession03 = nullptr;
    ASSERT_NE(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    struct RSSurfaceNodeConfig config;
    sceneSession01->surfaceNode_ = RSSurfaceNode::Create(config);
    ASSERT_NE(sceneSession01->surfaceNode_, nullptr);
    sceneSession01->surfaceNode_->id_ = 0;

    ssm_->sceneSessionMap_.insert(std::make_pair(0, nullptr));
    ssm_->RemoveDuplicateSubSession(visibilityChangeInfo, subSessions);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    sceneSession02->persistentId_ = 2;
    subSessions.push_back(sceneSession01);
    subSessions.push_back(sceneSession02);
    subSessions.push_back(sceneSession03);
    ssm_->RemoveDuplicateSubSession(visibilityChangeInfo, subSessions);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, ssm_->HandleSecureSessionShouldHide(nullptr));
}

/**
 * @tc.name: UpdateSubWindowVisibility
 * @tc.desc: UpdateSubWindowVisibility
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateSubWindowVisibility, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    WindowVisibilityState visibleState = WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION;
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfo;
    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
    std::string visibilityInfo = "";
    std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
    ssm_->UpdateSubWindowVisibility(
        sceneSession, visibleState, visibilityChangeInfo, windowVisibilityInfos, visibilityInfo, currVisibleData);

    ASSERT_NE(sceneSession->property_, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ssm_->UpdateSubWindowVisibility(
        sceneSession, visibleState, visibilityChangeInfo, windowVisibilityInfos, visibilityInfo, currVisibleData);

    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    visibleState = WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION;
    ssm_->sceneSessionMap_.insert(std::make_pair(0, nullptr));
    ssm_->UpdateSubWindowVisibility(
        sceneSession, visibleState, visibilityChangeInfo, windowVisibilityInfos, visibilityInfo, currVisibleData);

    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession03 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<Session> session03 = sptr<Session>::MakeSptr(info);
    ASSERT_NE(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ASSERT_NE(sceneSession03, nullptr);
    ASSERT_NE(session03, nullptr);
    sceneSession01->SetParentSession(session03);
    sceneSession02->SetParentSession(session03);
    sceneSession03->SetParentSession(session03);
    sceneSession02->SetCallingPid(2);
    sceneSession03->SetCallingPid(3);
    sceneSession03->state_ = SessionState::STATE_FOREGROUND;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession03));
    ssm_->UpdateSubWindowVisibility(
        sceneSession, visibleState, visibilityChangeInfo, windowVisibilityInfos, visibilityInfo, currVisibleData);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, ssm_->HandleSecureSessionShouldHide(nullptr));
}

/**
 * @tc.name: SelectSesssionFromMap
 * @tc.desc: SelectSesssionFromMap
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, SelectSesssionFromMap, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    uint64_t surfaceId = 6;
    uint64_t surfaceId02 = 7;
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession;
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(0, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    sceneSession01->surfaceNode_ = nullptr;
    struct RSSurfaceNodeConfig config;
    sceneSession02->surfaceNode_ = RSSurfaceNode::Create(config);
    ASSERT_NE(sceneSession02->surfaceNode_, nullptr);
    NodeId id = 6;
    sceneSession02->surfaceNode_->SetId(id);
    EXPECT_EQ(sceneSession02->surfaceNode_->id_, 6);

    sptr<SceneSession> result = ssm_->SelectSesssionFromMap(surfaceId);
    EXPECT_EQ(result, sceneSession02);
    result = ssm_->SelectSesssionFromMap(surfaceId02);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetAccessibilityWindowInfo
 * @tc.desc: GetAccessibilityWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetAccessibilityWindowInfo, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession;
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession03 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession04 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ASSERT_NE(sceneSession03, nullptr);
    ASSERT_NE(sceneSession04, nullptr);
    sceneSession01->SetForceTouchable(true);
    sceneSession01->isVisible_ = true;
    ASSERT_NE(sceneSession01->property_, nullptr);
    sceneSession01->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession01->property_->SetParentPersistentId(4);
    sceneSession02->SetForceTouchable(false);
    sceneSession03->SetForceTouchable(true);
    sceneSession03->isVisible_ = true;
    ASSERT_NE(sceneSession03->property_, nullptr);
    sceneSession03->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession03->property_->SetParentPersistentId(6);
    ssm_->sceneSessionMap_.insert(std::make_pair(0, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession03));
    ssm_->sceneSessionMap_.insert(std::make_pair(4, sceneSession04));
    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;
    auto result = ssm_->GetAccessibilityWindowInfo(accessibilityInfo);
    EXPECT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: ShiftAppWindowFocus02
 * @tc.desc: ShiftAppWindowFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, ShiftAppWindowFocus02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->SetFocusedSessionId(INVALID_SESSION_ID, DEFAULT_DISPLAY_ID);
    int32_t sourcePersistentId = INVALID_SESSION_ID;
    int32_t targetPersistentId = 1;

    auto result = ssm_->ShiftAppWindowFocus(sourcePersistentId, targetPersistentId);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    SessionInfo info;
    info.abilityName_ = "abilityName";
    info.bundleName_ = "bundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(INVALID_SESSION_ID, sceneSession));
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    result = ssm_->ShiftAppWindowFocus(sourcePersistentId, targetPersistentId);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    SessionInfo info01;
    info01.abilityName_ = "abilityName01";
    info01.bundleName_ = "bundleName01";
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info01, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    sceneSession01->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    result = ssm_->ShiftAppWindowFocus(sourcePersistentId, targetPersistentId);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_CALLING);
}

/**
 * @tc.name: UpdateTitleInTargetPos
 * @tc.desc: UpdateTitleInTargetPos
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateTitleInTargetPos, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "abilityName";
    info.bundleName_ = "bundleName";
    info.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(0, sceneSession));
    auto result = ssm_->UpdateTitleInTargetPos(1, true, 1);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_WINDOW);

    result = ssm_->UpdateTitleInTargetPos(0, true, 1);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: GetIsLayoutFullScreen
 * @tc.desc: GetIsLayoutFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetIsLayoutFullScreen, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "abilityName";
    info.bundleName_ = "bundleName";
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession03 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession04 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ASSERT_NE(sceneSession03, nullptr);
    ASSERT_NE(sceneSession04, nullptr);
    ASSERT_NE(sceneSession01->property_, nullptr);
    sceneSession01->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);

    ASSERT_NE(sceneSession02->property_, nullptr);
    sceneSession02->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession02->SetSessionState(SessionState::STATE_DISCONNECT);

    ASSERT_NE(sceneSession03->property_, nullptr);
    sceneSession03->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession03->SetSessionState(SessionState::STATE_ACTIVE);
    sceneSession03->property_->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);

    ASSERT_NE(sceneSession04->property_, nullptr);
    sceneSession04->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession04->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession04->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sceneSession04->property_->SetIsLayoutFullScreen(true);

    ASSERT_NE(sceneSession04->property_, nullptr);
    sceneSession04->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession04->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession04->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sceneSession04->property_->SetIsLayoutFullScreen(false);

    ssm_->sceneSessionMap_.insert(std::make_pair(0, nullptr));
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession03));
    bool isLayoutFullScreen = true;
    auto result = ssm_->GetIsLayoutFullScreen(isLayoutFullScreen);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UpdateExtWindowFlags
 * @tc.desc: UpdateExtWindowFlags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateExtWindowFlags, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    uint32_t extWindowFlags = 0;
    uint32_t extWindowActions = 0;
    auto ret = ssm_->UpdateExtWindowFlags(nullptr, extWindowFlags, extWindowActions);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: AddOrRemoveSecureSession02
 * @tc.desc: AddOrRemoveSecureSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, AddOrRemoveSecureSession02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t persistentId = 1;
    bool shouldHide = true;
    SessionInfo info;
    info.abilityName_ = "secureSession";
    info.bundleName_ = "secureSession";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(0, nullptr));
    ssm_->sceneSessionMap_.insert(std::make_pair(persistentId, sceneSession));
    auto result = ssm_->AddOrRemoveSecureSession(0, shouldHide);
    EXPECT_EQ(result, WSError::WS_OK);
    result = ssm_->AddOrRemoveSecureSession(persistentId, shouldHide);
    EXPECT_EQ(result, WSError::WS_OK);
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: HandleHideNonSystemFloatingWindows
 * @tc.desc: HandleHideNonSystemFloatingWindows
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, HandleHideNonSystemFloatingWindows, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "abilityName";
    info.bundleName_ = "bundleName";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    sceneSession->property_->hideNonSystemFloatingWindows_ = true;
    property->SetHideNonSystemFloatingWindows(false);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ssm_->HandleHideNonSystemFloatingWindows(property, sceneSession);

    sceneSession->property_->hideNonSystemFloatingWindows_ = false;
    property->SetHideNonSystemFloatingWindows(true);
    ssm_->HandleHideNonSystemFloatingWindows(property, sceneSession);

    sceneSession->isVisible_ = false;
    ssm_->HandleHideNonSystemFloatingWindows(property, sceneSession);
    EXPECT_EQ(Session::IsScbCoreEnabled(), true);
}

/**
 * @tc.name: GetAllClearableSessions
 * @tc.desc: GetAllClearableSessions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetAllClearableSessions, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.abilityName_ = "abilityName";
    sessionInfo.abilityInfo = nullptr;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession));
    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "bundleName";
    sessionInfo1.abilityName_ = "abilityName";
    sessionInfo1.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(nullptr, sessionInfo1.abilityInfo);
    sessionInfo1.abilityInfo->excludeFromMissions = false;
    sessionInfo1.abilityInfo->unclearableMission = false;
    sessionInfo1.isSystem_ = false;
    sessionInfo1.lockedState = false;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession1));
    std::vector<sptr<SceneSession>> sessionVector;
    sessionVector.clear();
    ssm_->GetAllClearableSessions(sessionVector);
    EXPECT_FALSE(sessionVector.empty());
}

/**
 * @tc.name: UpdateBrightness
 * @tc.desc: UpdateBrightness
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, UpdateBrightness, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sessionInfo.bundleName_ = "bundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ASSERT_NE(sceneSession->property_, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    WSError result = ssm_->UpdateBrightness(1);
    EXPECT_EQ(result, WSError::WS_DO_NOTHING);

    SessionInfo sessionInfo02;
    sessionInfo02.isSystem_ = true;
    sessionInfo02.bundleName_ = "bundleName";
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(sessionInfo02, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ASSERT_NE(sceneSession02->property_, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    sceneSession02->property_->SetBrightness(50.f);
    ssm_->SetDisplayBrightness(40.f);
    result = ssm_->UpdateBrightness(2);
    EXPECT_EQ(ssm_->displayBrightness_, 50);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: RegisterSessionExceptionFunc
 * @tc.desc: RegisterSessionExceptionFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, RegisterSessionExceptionFunc, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.persistentId_ = 1;
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(sessionInfo.persistentId_, sceneSession));
    std::shared_ptr<SessionListenerController> listenerController = std::make_shared<SessionListenerController>();
    ssm_->listenerController_ = listenerController;
    ASSERT_NE(ssm_->listenerController_, nullptr);
    ExceptionInfo exceptionInfo;
    ssm_->RegisterSessionExceptionFunc(sceneSession);

    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    ASSERT_NE(abilitySessionInfo, nullptr);
    WSError result = sceneSession->NotifySessionExceptionInner(abilitySessionInfo, exceptionInfo, false);
    EXPECT_EQ(result, WSError::WS_OK);

    abilitySessionInfo->errorCode = 1;
    result = sceneSession->NotifySessionExceptionInner(abilitySessionInfo, exceptionInfo, false);
    EXPECT_EQ(result, WSError::WS_OK);

    abilitySessionInfo->errorCode = 0;
    result = sceneSession->NotifySessionExceptionInner(abilitySessionInfo, exceptionInfo, false);
    EXPECT_EQ(result, WSError::WS_OK);

    sessionInfo.isSystem_ = false;
    result = sceneSession->NotifySessionExceptionInner(abilitySessionInfo, exceptionInfo, false);
    EXPECT_EQ(result, WSError::WS_OK);

    ssm_->listenerController_ = nullptr;
    result = sceneSession->NotifySessionExceptionInner(abilitySessionInfo, exceptionInfo, false);
    EXPECT_EQ(result, WSError::WS_OK);

    sessionInfo.persistentId_ = 2;
    result = sceneSession->NotifySessionExceptionInner(abilitySessionInfo, exceptionInfo, false);
    EXPECT_EQ(result, WSError::WS_OK);

    result = sceneSession->NotifySessionExceptionWithOptions(abilitySessionInfo, "sessionExceptionTest", false);
    EXPECT_EQ(result, WSError::WS_OK);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: RegisterSessionSnapshotFunc
 * @tc.desc: RegisterSessionSnapshotFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, RegisterSessionSnapshotFunc, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.persistentId_ = 1;
    sessionInfo.isSystem_ = false;
    sessionInfo.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(sessionInfo.abilityInfo, nullptr);
    sessionInfo.abilityInfo->excludeFromMissions = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->scenePersistence_ = sptr<ScenePersistence>::MakeSptr("bundleName", 1);
    ASSERT_NE(sceneSession->scenePersistence_, nullptr);
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    sceneSession->surfaceNode_ = std::make_shared<RSSurfaceNode>(rsSurfaceNodeConfig, true, 0);
    ASSERT_NE(sceneSession->surfaceNode_, nullptr);
    sceneSession->surfaceNode_->bufferAvailable_ = true;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->listenerController_ = std::make_shared<SessionListenerController>();
    ASSERT_NE(ssm_->listenerController_, nullptr);
    ssm_->RegisterSessionSnapshotFunc(sceneSession);
    EXPECT_EQ(sceneSession->Snapshot(1.f), nullptr);

    sessionInfo.abilityInfo->excludeFromMissions = false;
    EXPECT_EQ(sceneSession->Snapshot(1.f), nullptr);

    ssm_->listenerController_ = nullptr;
    EXPECT_EQ(sceneSession->Snapshot(1.f), nullptr);

    sessionInfo.abilityInfo = nullptr;
    EXPECT_EQ(sceneSession->Snapshot(1.f), nullptr);

    sessionInfo.isSystem_ = true;
    EXPECT_EQ(sceneSession->Snapshot(1.f), nullptr);

    sessionInfo.persistentId_ = 2;
    EXPECT_EQ(sceneSession->Snapshot(1.f), nullptr);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: IsParentSessionVisible
 * @tc.desc: IsParentSessionVisible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, IsParentSessionVisible, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ASSERT_NE(sceneSession->property_, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession->property_->SetParentPersistentId(1);
    sceneSession->isVisible_ = false;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    bool result = ssm_->IsParentSessionVisible(sceneSession);
    EXPECT_EQ(result, false);

    bool testRet = sceneSession->IsScbCoreEnabled();
    EXPECT_EQ(testRet, true);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sceneSession->isVisible_ = true;
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    result = ssm_->IsParentSessionVisible(sceneSession);
    EXPECT_EQ(result, true);

    sceneSession->property_->SetParentPersistentId(2);
    sceneSession->SetSessionState(SessionState::STATE_DISCONNECT);
    result = ssm_->IsParentSessionVisible(sceneSession);
    EXPECT_EQ(result, true);

    sceneSession->property_->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    result = ssm_->IsParentSessionVisible(sceneSession);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GetTopFocusableNonAppSession
 * @tc.desc: GetTopFocusableNonAppSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetTopFocusableNonAppSession, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ASSERT_NE(sceneSession->property_, nullptr);

    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    sceneSession->property_->SetFocusable(true);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    sptr<SceneSession> result = ssm_->GetTopFocusableNonAppSession();
    EXPECT_EQ(result, sceneSession);

    sceneSession->isVisible_ = false;
    result = ssm_->GetTopFocusableNonAppSession();
    EXPECT_EQ(result, nullptr);

    sceneSession->property_->SetFocusable(false);
    result = ssm_->GetTopFocusableNonAppSession();
    EXPECT_EQ(result, nullptr);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    result = ssm_->GetTopFocusableNonAppSession();
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetTopFocusableNonAppSession01
 * @tc.desc: GetTopFocusableNonAppSession01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetTopFocusableNonAppSession01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    sptr<SceneSession> wallpaper = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    wallpaper->property_->SetWindowType(WindowType::WINDOW_TYPE_WALLPAPER);
    wallpaper->zOrder_ = 1;
    sptr<SceneSession> desktop = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    desktop->property_->SetWindowType(WindowType::WINDOW_TYPE_DESKTOP);
    desktop->zOrder_ = 2;
    desktop->property_->SetFocusable(true);
    desktop->isVisible_ = true;
    desktop->state_ = SessionState::STATE_FOREGROUND;

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    sceneSession->zOrder_ = 3;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    sceneSession1->zOrder_ = 0;
    ssm_->sceneSessionMap_.insert(std::make_pair(0, sceneSession1));
    ssm_->sceneSessionMap_.insert(std::make_pair(1, wallpaper));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, desktop));
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession));
    sptr<SceneSession> result = ssm_->GetTopFocusableNonAppSession();
    EXPECT_EQ(result, desktop);
}

/**
 * @tc.name: GetTopNearestBlockingFocusSession
 * @tc.desc: GetTopNearestBlockingFocusSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetTopNearestBlockingFocusSession, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> sceneSession03 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> sceneSession04 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> parentSceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ASSERT_NE(sceneSession03, nullptr);
    ASSERT_NE(sceneSession04, nullptr);
    ASSERT_NE(parentSceneSession, nullptr);

    sceneSession01->SetZOrder(1);
    sceneSession01->isVisible_ = true;
    sceneSession01->SetSessionState(SessionState::STATE_FOREGROUND);

    sceneSession02->SetZOrder(2);
    sceneSession02->isVisible_ = true;
    sceneSession02->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession02->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession02->blockingFocus_ = false;

    sceneSession03->SetZOrder(3);
    sceneSession03->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    parentSceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession03->property_->SetParentPersistentId(8);

    sceneSession04->SetZOrder(4);
    sceneSession04->property_->SetParentPersistentId(1);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession03));
    ssm_->sceneSessionMap_.insert(std::make_pair(4, sceneSession04));
    ssm_->sceneSessionMap_.insert(std::make_pair(8, parentSceneSession));

    sptr<SceneSession> ret = ssm_->GetTopNearestBlockingFocusSession(DEFAULT_DISPLAY_ID, 0, true);
    EXPECT_EQ(ret, sceneSession01);

    ret = ssm_->GetTopNearestBlockingFocusSession(DEFAULT_DISPLAY_ID, 10, true);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetTopNearestBlockingFocusSession01
 * @tc.desc: GetTopNearestBlockingFocusSession01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, GetTopNearestBlockingFocusSession01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession01->SetZOrder(1);
    sceneSession01->persistentId_ = 1;
    sceneSession01->isVisible_ = true;
    sceneSession01->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession02->SetZOrder(3);
    sceneSession02->isVisible_ = true;
    sceneSession02->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession02->SetParentSession(sceneSession01);
    sceneSession02->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    sptr<SceneSession> ret = ssm_->GetTopNearestBlockingFocusSession(DEFAULT_DISPLAY_ID, 0, true);
    EXPECT_EQ(ret, sceneSession01);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sceneSession02->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ret = ssm_->GetTopNearestBlockingFocusSession(DEFAULT_DISPLAY_ID, 2, true);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CheckBlockingFocus
 * @tc.desc: CheckBlockingFocus
 * 1.session with blockfocus porperty need be blocked
 * 2.main window need be blocked
 * 3.WINDOW_TYPE_VOICE_INTERACTION window type need be blocked
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, CheckBlockingFocus, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    bool ret = ssm_->CheckBlockingFocus(sceneSession, false);
    EXPECT_EQ(ret, false);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = ssm_->CheckBlockingFocus(sceneSession, true);
    EXPECT_EQ(ret, true);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD);
    ret = ssm_->CheckBlockingFocus(sceneSession, false);
    EXPECT_EQ(ret, true);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_VOICE_INTERACTION);
    ret = ssm_->CheckBlockingFocus(sceneSession, false);
    EXPECT_EQ(ret, true);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->sessionInfo_.isSystem_ = true;
    sceneSession->blockingFocus_ = true;
    ret = ssm_->CheckBlockingFocus(sceneSession, false);
    EXPECT_EQ(ret, true);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    sceneSession->blockingFocus_ = false;
    sceneSession01->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sceneSession->SetParentSession(sceneSession01);
    ret = ssm_->CheckBlockingFocus(sceneSession, false);
    EXPECT_EQ(ret, false);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ret = ssm_->CheckBlockingFocus(sceneSession, true);
    EXPECT_EQ(ret, false);

    sceneSession->isMidScene_ = true;
    ret = ssm_->CheckBlockingFocus(sceneSession, true);
    EXPECT_EQ(ret, true);
    sceneSession->isMidScene_ = false;
}

/**
 * @tc.name: ProcessModalExtensionPointDown
 * @tc.desc: ProcessModalExtensionPointDown
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest4, ProcessModalExtensionPointDown, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(token, nullptr);
    int32_t posX = 1;
    int32_t posY = 1;
    ExtensionWindowAbilityInfo extensionWindowAbilityInfo;
    extensionWindowAbilityInfo.persistentId = 1;
    extensionWindowAbilityInfo.parentId = 2;
    ssm_->extSessionInfoMap_.insert(std::make_pair(token, extensionWindowAbilityInfo));

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession));
    ssm_->ProcessModalExtensionPointDown(token, posX, posY);

    ExtensionWindowEventInfo eventInfo;
    eventInfo.pid = 0;
    eventInfo.persistentId = 1;
    sceneSession->modalUIExtensionInfoList_.push_back(eventInfo);
    ssm_->ProcessModalExtensionPointDown(token, posX, posY);

    eventInfo.persistentId = 4;
    ssm_->ProcessModalExtensionPointDown(token, posX, posY);

    eventInfo.pid = 4;
    ssm_->ProcessModalExtensionPointDown(token, posX, posY);

    sceneSession->modalUIExtensionInfoList_.clear();
    ssm_->ProcessModalExtensionPointDown(token, posX, posY);
    bool result = true;
    EXPECT_EQ(WSError::WS_OK, ssm_->GetFreeMultiWindowEnableState(result));
    usleep(WAIT_SYNC_IN_NS);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
