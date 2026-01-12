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

#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>
#include <gtest/gtest.h>
#include <regex>
#include "context.h"
#include "interfaces/include/ws_common.h"
#include "mock/mock_accesstoken_kit.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "mock/mock_ibundle_mgr.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "session_info.h"
#include "session_manager.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_manager_agent.h"
#include "window_manager_hilog.h"
#include "zidl/window_manager_agent_interface.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string EMPTY_DEVICE_ID = "";
using ConfigItem = WindowSceneConfig::ConfigItem;
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
} // namespace
class SceneSessionManagerTest6 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static bool gestureNavigationEnabled_;
    static ProcessGestureNavigationEnabledChangeFunc callbackFunc_;
    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerTest6::ssm_ = nullptr;

bool SceneSessionManagerTest6::gestureNavigationEnabled_ = true;
ProcessGestureNavigationEnabledChangeFunc SceneSessionManagerTest6::callbackFunc_ =
    [](bool enable, const std::string& bundleName, GestureBackType type) { gestureNavigationEnabled_ = enable; };

void WindowChangedFuncTest6(int32_t persistentId, WindowUpdateType type) {}

void ProcessStatusBarEnabledChangeFuncTest(bool enable) {}

void SceneSessionManagerTest6::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerTest6::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest6::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerTest6::TearDown()
{
    MockAccesstokenKit::ChangeMockStateToInit();
    usleep(WAIT_SYNC_IN_NS);
    ssm_->sceneSessionMap_.clear();
}

namespace {
/**
 * @tc.name: MissionChanged
 * @tc.desc: MissionChanged
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, MissionChanged, TestSize.Level1)
{
    sptr<SceneSession> prevSession = nullptr;
    sptr<SceneSession> currSession = nullptr;
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->MissionChanged(prevSession, currSession);
    EXPECT_EQ(false, ret);
    SessionInfo sessionInfoFirst;
    sessionInfoFirst.bundleName_ = "privacy.test.first";
    sessionInfoFirst.abilityName_ = "privacyAbilityName";
    prevSession = sptr<SceneSession>::MakeSptr(sessionInfoFirst, nullptr);
    ASSERT_NE(nullptr, prevSession);
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->MissionChanged(prevSession, currSession);
    EXPECT_EQ(true, ret);
    SessionInfo sessionInfoSecond;
    sessionInfoSecond.bundleName_ = "privacy.test.second";
    sessionInfoSecond.abilityName_ = "privacyAbilityName";
    currSession = sptr<SceneSession>::MakeSptr(sessionInfoSecond, nullptr);
    ASSERT_NE(nullptr, currSession);
    prevSession->persistentId_ = 0;
    currSession->persistentId_ = 0;
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->MissionChanged(prevSession, currSession);
    EXPECT_EQ(false, ret);
    prevSession = nullptr;
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->MissionChanged(prevSession, currSession);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: UpdateSecSurfaceInfo
 * @tc.desc: UpdateSecSurfaceInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, UpdateSecSurfaceInfo, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    std::map<NodeId, std::vector<SecSurfaceInfo>> callbackData;
    std::shared_ptr<RSUIExtensionData> secExtData = std::make_shared<RSUIExtensionData>(callbackData);
    ssm_->currentUserId_ = 101;
    ssm_->UpdateSecSurfaceInfo(secExtData, 100);

    ssm_->currentUserId_ = 100;
    ssm_->UpdateSecSurfaceInfo(secExtData, 100);
}

/**
 * @tc.name: GetWindowLayerChangeInfo
 * @tc.desc: Simulate window Layer change
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetWindowLayerChangeInfo, TestSize.Level1)
{
    std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
    currVisibleData.push_back(std::make_pair(0, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION));
    currVisibleData.push_back(std::make_pair(1, WindowVisibilityState::WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION));
    currVisibleData.push_back(std::make_pair(2, WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION));
    currVisibleData.push_back(std::make_pair(3, WindowVisibilityState::WINDOW_LAYER_STATE_MAX));
    std::vector<std::pair<uint64_t, bool>> currDrawingContentData;
    currDrawingContentData.push_back(std::make_pair(0, true));
    currDrawingContentData.push_back(std::make_pair(1, false));
    VisibleData visibleData;
    visibleData.push_back(std::make_pair(0, WINDOW_LAYER_INFO_TYPE::ALL_VISIBLE));
    visibleData.push_back(std::make_pair(1, WINDOW_LAYER_INFO_TYPE::SEMI_VISIBLE));
    visibleData.push_back(std::make_pair(2, WINDOW_LAYER_INFO_TYPE::INVISIBLE));
    visibleData.push_back(std::make_pair(3, WINDOW_LAYER_INFO_TYPE::WINDOW_LAYER_DYNAMIC_STATUS));
    visibleData.push_back(std::make_pair(4, WINDOW_LAYER_INFO_TYPE::WINDOW_LAYER_STATIC_STATUS));
    visibleData.push_back(std::make_pair(5, WINDOW_LAYER_INFO_TYPE::WINDOW_LAYER_UNKNOWN_TYPE));
    std::shared_ptr<RSOcclusionData> occlusionDataPtr = std::make_shared<RSOcclusionData>(visibleData);
    ASSERT_NE(nullptr, occlusionDataPtr);
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    struct RSSurfaceNodeConfig config;
    sceneSession02->surfaceNode_ = RSSurfaceNode::Create(config);
    ASSERT_NE(sceneSession02->surfaceNode_, nullptr);
    sceneSession02->surfaceNode_->SetId(2);
    sceneSession02->hidingStartWindow_ = true;
    auto oldSessionMap = ssm_->sceneSessionMap_;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    ssm_->GetWindowLayerChangeInfo(occlusionDataPtr, currVisibleData, currDrawingContentData);
    ASSERT_EQ(currVisibleData.size(), 7);
    ASSERT_EQ(currDrawingContentData.size(), 4);
    ssm_->sceneSessionMap_ = oldSessionMap;
}

/**
 * @tc.name: GetWindowVisibilityChangeInfo01
 * @tc.desc: GetWindowVisibilityChangeInfo01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetWindowVisibilityChangeInfo01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->lastVisibleData_.clear();
    std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfos;
    currVisibleData.push_back(std::make_pair(1, WindowVisibilityState::WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION));
    currVisibleData.push_back(std::make_pair(2, WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION));
    currVisibleData.push_back(std::make_pair(3, WindowVisibilityState::WINDOW_LAYER_STATE_MAX));
    ssm_->lastVisibleData_.push_back(std::make_pair(0, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION));
    visibilityChangeInfos = ssm_->GetWindowVisibilityChangeInfo(currVisibleData);
    ASSERT_EQ(visibilityChangeInfos.size(), 3);
}

/**
 * @tc.name: GetWindowVisibilityChangeInfo02
 * @tc.desc: GetWindowVisibilityChangeInfo02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetWindowVisibilityChangeInfo02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->lastVisibleData_.clear();
    std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfos;
    currVisibleData.push_back(std::make_pair(0, WindowVisibilityState::WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION));
    ssm_->lastVisibleData_.push_back(std::make_pair(1, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION));
    visibilityChangeInfos = ssm_->GetWindowVisibilityChangeInfo(currVisibleData);
    ASSERT_EQ(visibilityChangeInfos.size(), 2);
}

/**
 * @tc.name: GetWindowVisibilityChangeInfo03
 * @tc.desc: GetWindowVisibilityChangeInfo03
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetWindowVisibilityChangeInfo03, TestSize.Level0)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->lastVisibleData_.clear();
    std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfos;
    currVisibleData.push_back(std::make_pair(1, WindowVisibilityState::WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION));
    currVisibleData.push_back(std::make_pair(2, WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION));
    ssm_->lastVisibleData_.push_back(
        std::make_pair(1, WindowVisibilityState::WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION));
    ssm_->lastVisibleData_.push_back(
        std::make_pair(2, WindowVisibilityState::WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION));
    visibilityChangeInfos = ssm_->GetWindowVisibilityChangeInfo(currVisibleData);
    ASSERT_EQ(visibilityChangeInfos.size(), 1);
    currVisibleData.clear();
    ssm_->lastVisibleData_.clear();
    currVisibleData.push_back(std::make_pair(1, WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION));
    ssm_->lastVisibleData_.push_back(std::make_pair(2, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION));
    visibilityChangeInfos = ssm_->GetWindowVisibilityChangeInfo(currVisibleData);
    ASSERT_EQ(visibilityChangeInfos.size(), 1);
    ASSERT_EQ(visibilityChangeInfos[0].first, 2);
}

/**
 * @tc.name: GetWindowVisibilityChangeInfo04
 * @tc.desc: GetWindowVisibilityChangeInfo04
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetWindowVisibilityChangeInfo04, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->lastVisibleData_.clear();
    std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfos;
    ssm_->lastVisibleData_.push_back(std::make_pair(0, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION));
    currVisibleData.push_back(std::make_pair(0, WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION));
    currVisibleData.push_back(std::make_pair(1, WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION));
    visibilityChangeInfos = ssm_->GetWindowVisibilityChangeInfo(currVisibleData);
    ASSERT_EQ(visibilityChangeInfos.size(), 1);

    currVisibleData.clear();
    currVisibleData.push_back(std::make_pair(1, WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION));
    visibilityChangeInfos = ssm_->GetWindowVisibilityChangeInfo(currVisibleData);
    ASSERT_EQ(visibilityChangeInfos.size(), 0);
}

/**
 * @tc.name: DealwithVisibilityChange01
 * @tc.desc: DealwithVisibilityChange01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, DealwithVisibilityChange01, TestSize.Level0)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession1->GetPersistentId(), sceneSession1));
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession2->GetPersistentId(), sceneSession2));
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode1 = RSSurfaceNode::Create(config);
    std::shared_ptr<RSSurfaceNode> surfaceNode2 = RSSurfaceNode::Create(config);
    ASSERT_NE(nullptr, surfaceNode1);
    ASSERT_NE(nullptr, surfaceNode2);
    surfaceNode1->SetId(1);
    surfaceNode2->SetId(2);
    ASSERT_NE(nullptr, sceneSession1);
    ASSERT_NE(nullptr, sceneSession2);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession1->surfaceNode_ = surfaceNode1;
    sceneSession1->SetCallingPid(1);
    sceneSession2->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession2->SetParentSession(sceneSession1);
    sceneSession2->surfaceNode_ = surfaceNode2;
    sceneSession2->SetCallingPid(2);
    ASSERT_NE(nullptr, sceneSession1->property_);
    ASSERT_NE(nullptr, sceneSession2->property_);
    sceneSession1->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession2->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession1->property_->SetWindowName("visibility1");
    sceneSession2->property_->SetWindowName("visibility2");
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfos;
    visibilityChangeInfos.push_back(std::make_pair(1, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION));
    std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
    currVisibleData.push_back(std::make_pair(1, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION));
    ssm_->DealwithVisibilityChange(visibilityChangeInfos, currVisibleData);
    ASSERT_EQ(sceneSession1->GetRSVisible(), true);
    ASSERT_EQ(sceneSession2->GetRSVisible(), false);
    sceneSession2->SetSessionState(SessionState::STATE_BACKGROUND);
    sceneSession1->SetRSVisible(false);
    sceneSession2->SetRSVisible(false);
    ssm_->DealwithVisibilityChange(visibilityChangeInfos, currVisibleData);
    ASSERT_EQ(sceneSession1->GetRSVisible(), true);
    ASSERT_EQ(sceneSession2->GetRSVisible(), false);
    ssm_->taskScheduler_ ->PostAsyncTaskToExportHandler([]() {}, "testNotifyMemMgr");
    ssm_->taskScheduler_ ->PostAsyncTaskToExportHandler([]() {}, "testNotifyMemMgr", 100);
}

/**
 * @tc.name: DealwithVisibilityChange02
 * @tc.desc: DealwithVisibilityChange02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, DealwithVisibilityChange02, TestSize.Level0)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession1->GetPersistentId(), sceneSession1));
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession2->GetPersistentId(), sceneSession2));
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode1 = RSSurfaceNode::Create(config);
    std::shared_ptr<RSSurfaceNode> surfaceNode2 = RSSurfaceNode::Create(config);
    ASSERT_NE(nullptr, surfaceNode1);
    ASSERT_NE(nullptr, surfaceNode2);
    surfaceNode1->SetId(1);
    surfaceNode2->SetId(2);
    ASSERT_NE(nullptr, sceneSession1);
    ASSERT_NE(nullptr, sceneSession2);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession1->surfaceNode_ = surfaceNode1;
    sceneSession2->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession2->SetParentSession(sceneSession1);
    sceneSession2->surfaceNode_ = surfaceNode2;
    ASSERT_NE(nullptr, sceneSession1->property_);
    ASSERT_NE(nullptr, sceneSession2->property_);
    sceneSession1->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession2->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession1->property_->SetWindowName("visibility1");
    sceneSession2->property_->SetWindowName("visibility2");
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfos;
    visibilityChangeInfos.push_back(std::make_pair(2, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION));
    std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
    currVisibleData.push_back(std::make_pair(3, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION));
    sceneSession1->SetRSVisible(true);
    ssm_->DealwithVisibilityChange(visibilityChangeInfos, currVisibleData);
    ASSERT_EQ(sceneSession2->GetRSVisible(), true);
    sceneSession2->SetSessionState(SessionState::STATE_BACKGROUND);
    sceneSession1->SetRSVisible(false);
    sceneSession2->SetRSVisible(false);
    sceneSession1->SetSessionState(SessionState::STATE_BACKGROUND);
    ssm_->DealwithVisibilityChange(visibilityChangeInfos, currVisibleData);
    ASSERT_EQ(sceneSession2->GetRSVisible(), false);
}

/**
 * @tc.name: UpdateWindowMode
 * @tc.desc: UpdateWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, UpdateWindowMode, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession));
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->UpdateWindowMode(0, 0);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_WINDOW, ret);
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->UpdateWindowMode(2, 0);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: GetTopNavDestinationName
 * @tc.desc: test GetTopNavDestinationName whether get the top nav destination name.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetTopNavDestinationName, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    ssm_->sceneSessionMap_.clear();

    std::string topNavDestName;
    EXPECT_EQ(ssm_->GetTopNavDestinationName(1000, topNavDestName), WMError::WM_ERROR_INVALID_WINDOW);

    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession));
    EXPECT_EQ(ssm_->GetTopNavDestinationName(2, topNavDestName), WMError::WM_ERROR_INVALID_OPERATION);

    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->sessionStage_ = nullptr;
    EXPECT_EQ(ssm_->GetTopNavDestinationName(2, topNavDestName), WMError::WM_ERROR_SYSTEM_ABNORMALLY);

    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_EQ(ssm_->GetTopNavDestinationName(2, topNavDestName), WMError::WM_OK);
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
}

/**
 * @tc.name: SetWatermarkImageForApp
 * @tc.desc: cancel the watermark
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, SetWatermarkImageForApp1, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::string watermarkName;
    EXPECT_EQ(ssm_->SetWatermarkImageForApp(nullptr, watermarkName), WMError::WM_OK);
    EXPECT_EQ(watermarkName, "");

    int32_t pid = IPCSkeleton::GetCallingRealPid();
    ssm_->appWatermarkPidMap_.clear();
    ssm_->appWatermarkPidMap_[pid] = "watermarkName#1";
    EXPECT_EQ(ssm_->SetWatermarkImageForApp(nullptr, watermarkName), WMError::WM_OK);
    EXPECT_EQ(watermarkName, "");
    EXPECT_EQ(ssm_->appWatermarkPidMap_.size(), 0);
}

/**
 * @tc.name: SetWatermarkImageForApp
 * @tc.desc: set watermark for app
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, SetWatermarkImageForApp2, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    int32_t pid = IPCSkeleton::GetCallingRealPid();
    std::string watermarkName;
    std::string bundleName = "setAppWatermark";
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    ssm_->appWatermarkPidMap_.clear();
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(0, nullptr));

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = bundleName;
    sessionInfo1.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);
    auto session1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    session1->SetCallingPid(pid + 1000);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, session1));

    SessionInfo sessionInfo2;
    sessionInfo2.bundleName_ = bundleName;
    sessionInfo2.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    auto session2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session2->SetSurfaceNode(surfaceNode);
    session2->SetCallingPid(pid);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, session2));

    SessionInfo sessionInfo3;
    sessionInfo3.bundleName_ = bundleName;
    sessionInfo3.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_PIP);
    auto session3 = sptr<SceneSession>::MakeSptr(sessionInfo3, nullptr);
    session3->SetSurfaceNode(nullptr);
    session3->SetCallingPid(pid);
    ssm_->sceneSessionMap_.insert(std::make_pair(3, session3));

    ssm_->appWatermarkPidMap_[pid] = "watermarkName#1";
    EXPECT_EQ(ssm_->SetWatermarkImageForApp(pixelMap, watermarkName), WMError::WM_ERROR_SYSTEM_ABNORMALLY);
    EXPECT_EQ(watermarkName, "");
    EXPECT_EQ(ssm_->appWatermarkPidMap_.size(), 1);

    ssm_->appWatermarkPidMap_.clear();
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
}

/**
 * @tc.name: RecoverWatermarkImageForApp
 * @tc.desc: recover watermark for app
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, RecoverWatermarkImageForApp, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::string watermarkName = "watermark#1";
    EXPECT_EQ(ssm_->RecoverWatermarkImageForApp(watermarkName), WMError::WM_OK);
    EXPECT_EQ(ssm_->RecoverWatermarkImageForApp(""), WMError::WM_OK);
    EXPECT_EQ(ssm_->RecoverWatermarkImageForApp(watermarkName), WMError::WM_OK);
    ssm_->appWatermarkPidMap_.clear();
}

/**
 * @tc.name: SetWatermarkForSession
 * @tc.desc: set watermark for session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, SetWatermarkForSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    int32_t pid = IPCSkeleton::GetCallingRealPid();
    std::string bundleName = "setAppWatermark";
    struct RSSurfaceNodeConfig config;
    ssm_->appWatermarkPidMap_.clear();

    sptr<SceneSession> session0 = nullptr;
    ssm_->SetWatermarkForSession(session0);
    EXPECT_EQ(ssm_->appWatermarkPidMap_.size(), 0);

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = bundleName;
    sessionInfo1.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);
    auto session1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    session1->SetCallingPid(pid + 100);
    ssm_->SetWatermarkForSession(session1);
    EXPECT_EQ(ssm_->appWatermarkPidMap_.size(), 0);

    ssm_->appWatermarkPidMap_[pid] = "watermarkName#1";

    SessionInfo sessionInfo2;
    sessionInfo2.bundleName_ = bundleName;
    sessionInfo2.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    auto session2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session2->SetSurfaceNode(surfaceNode);
    session2->SetCallingPid(pid);
    ssm_->SetWatermarkForSession(session2);
    EXPECT_EQ(ssm_->appWatermarkPidMap_.size(), 1);

    SessionInfo sessionInfo3;
    sessionInfo3.bundleName_ = bundleName;
    sessionInfo3.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_PIP);
    auto session3 = sptr<SceneSession>::MakeSptr(sessionInfo3, nullptr);
    session3->SetSurfaceNode(nullptr);
    session3->SetCallingPid(pid);
    ssm_->SetWatermarkForSession(session3);
    EXPECT_EQ(ssm_->appWatermarkPidMap_.size(), 1);

    ssm_->appWatermarkPidMap_.clear();
}

/**
 * @tc.name: ClearWatermarkForSession
 * @tc.desc: cancel watermark for session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, ClearWatermarkForSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    int32_t pid = IPCSkeleton::GetCallingRealPid();
    std::string bundleName = "setAppWatermark";
    struct RSSurfaceNodeConfig config;
    ssm_->appWatermarkPidMap_.clear();

    sptr<SceneSession> session0 = nullptr;
    ssm_->ClearWatermarkForSession(session0);
    EXPECT_EQ(ssm_->appWatermarkPidMap_.size(), 0);

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = bundleName;
    sessionInfo1.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);
    auto session1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    session1->SetCallingPid(pid + 100);
    ssm_->ClearWatermarkForSession(session1);
    EXPECT_EQ(ssm_->appWatermarkPidMap_.size(), 0);

    ssm_->appWatermarkPidMap_[pid] = "watermarkName#1";

    SessionInfo sessionInfo2;
    sessionInfo2.bundleName_ = bundleName;
    sessionInfo2.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    auto session2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session2->SetSurfaceNode(surfaceNode);
    session2->SetCallingPid(pid);
    ssm_->ClearWatermarkForSession(session2);
    EXPECT_EQ(ssm_->appWatermarkPidMap_.size(), 1);

    SessionInfo sessionInfo3;
    sessionInfo3.bundleName_ = bundleName;
    sessionInfo3.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_PIP);
    auto session3 = sptr<SceneSession>::MakeSptr(sessionInfo3, nullptr);
    session3->SetSurfaceNode(nullptr);
    session3->SetCallingPid(pid);
    ssm_->ClearWatermarkForSession(session3);
    EXPECT_EQ(ssm_->appWatermarkPidMap_.size(), 1);

    ssm_->appWatermarkPidMap_.clear();
}

/**
 * @tc.name: ClearProcessRecordWhenAppExit
 * @tc.desc: clare process record for app
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, ClearProcessRecordWhenAppExit, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    int32_t pid = 100;
    std::string bundleName = "setAppWatermark";
    ssm_->appWatermarkPidMap_.clear();
    ssm_->appWatermarkPidMap_[pid] = "watermarkName#1";
    ssm_->ClearProcessRecordWhenAppExit({});
    EXPECT_EQ(ssm_->appWatermarkPidMap_.size(), 1);
    ssm_->appWatermarkPidMap_.clear();
}

/**
 * @tc.name: RegisterAppStateObserver
 * @tc.desc: register app state ovserver
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, RegisterAppStateObserver, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->appStateObserver_ = nullptr;
    ssm_->RegisterAppStateObserver();
    ASSERT_NE(ssm_->appStateObserver_, nullptr);
    ssm_->appStateObserver_->OnProcessDied({});
    ssm_->appStateObserver_->RegisterProcessDiedNotifyFunc(nullptr);
    ssm_->appStateObserver_->OnProcessDied({});
    ssm_->appStateObserver_ = nullptr;
}

/**
 * @tc.name: SetScreenLocked && IsScreenLocked
 * @tc.desc: SceneSesionManager update screen locked state
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, IsScreenLocked, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->SetScreenLocked(true);
    sleep(1);
    ASSERT_NE(nullptr, ssm_);
    EXPECT_TRUE(ssm_->IsScreenLocked());
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessWindowModeType();
    ASSERT_NE(nullptr, ssm_);
    ssm_->SetScreenLocked(false);
    sleep(1);
    ASSERT_NE(nullptr, ssm_);
    EXPECT_FALSE(ssm_->IsScreenLocked());
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessWindowModeType();
}

/**
 * @tc.name: CheckWindowModeType
 * @tc.desc: CheckWindowModeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckWindowModeType, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    auto ret = ssm_->CheckWindowModeType();
    EXPECT_EQ(WindowModeType::WINDOW_MODE_OTHER, ret);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "privacy.test.first";
    sessionInfo.abilityName_ = "privacyAbilityName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    DisplayId displayId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    sceneSession->property_->SetDisplayId(displayId);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->CheckWindowModeType();
    EXPECT_EQ(WindowModeType::WINDOW_MODE_OTHER, ret);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->isVisible_ = false;
    sceneSession->isRSVisible_ = false;
    sceneSession->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->CheckWindowModeType();
    EXPECT_EQ(WindowModeType::WINDOW_MODE_OTHER, ret);
}

/**
 * @tc.name: CheckWindowModeType01
 * @tc.desc: CheckWindowModeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckWindowModeType01, TestSize.Level1)
{
    DisplayId displayId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "privacy.test.first";
    sessionInfo.abilityName_ = "privacyAbilityName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    sceneSession->property_->SetDisplayId(displayId);
    sceneSession->isVisible_ = true;
    sceneSession->isRSVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "privacy.test.first";
    sessionInfo1.abilityName_ = "privacyAbilityName";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    ASSERT_NE(nullptr, sceneSession1->property_);
    sceneSession1->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_NE(nullptr, sceneSession1->property_);
    sceneSession1->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession1->property_->SetDisplayId(displayId);
    sceneSession1->isVisible_ = true;
    sceneSession1->isRSVisible_ = true;
    sceneSession1->state_ = SessionState::STATE_ACTIVE;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession1));
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->CheckWindowModeType();
    EXPECT_EQ(WindowModeType::WINDOW_MODE_SPLIT_FLOATING, ret);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->CheckWindowModeType();
    EXPECT_EQ(WindowModeType::WINDOW_MODE_FULLSCREEN_FLOATING, ret);
}

/**
 * @tc.name: CheckWindowModeType02
 * @tc.desc: CheckWindowModeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckWindowModeType02, TestSize.Level1)
{
    DisplayId displayId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "privacy.test.first";
    sessionInfo.abilityName_ = "privacyAbilityName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    sceneSession->property_->SetDisplayId(displayId);
    sceneSession->isVisible_ = true;
    sceneSession->isRSVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ASSERT_NE(nullptr, ssm_);
    ssm_->lastWindowModeType_ = WindowModeType::WINDOW_MODE_FULLSCREEN;
    auto ret = ssm_->CheckWindowModeType();
    EXPECT_EQ(WindowModeType::WINDOW_MODE_FLOATING, ret);
    ASSERT_NE(nullptr, ssm_);
    ssm_->NotifyRSSWindowModeTypeUpdate();
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->CheckWindowModeType();
    EXPECT_EQ(WindowModeType::WINDOW_MODE_FULLSCREEN, ret);
    ASSERT_NE(nullptr, ssm_);
    ssm_->NotifyRSSWindowModeTypeUpdate();
}

/**
 * @tc.name: CheckWindowModeType03
 * @tc.desc: CheckWindowModeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckWindowModeType03, TestSize.Level1)
{
    DisplayId displayId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "privacy.test.first";
    sessionInfo.abilityName_ = "privacyAbilityName";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->property_->SetDisplayId(displayId);
    sceneSession->isVisible_ = true;
    sceneSession->isRSVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    sceneSession->isVisible_ = true;
    sceneSession->isRSVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->CheckWindowModeType();
    EXPECT_EQ(WindowModeType::WINDOW_MODE_SPLIT, ret);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->CheckWindowModeType();
    EXPECT_EQ(WindowModeType::WINDOW_MODE_SPLIT, ret);
}

/**
 * @tc.name: GetSceneSessionPrivacyModeBundles
 * @tc.desc: GetSceneSessionPrivacyModeBundles
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetSceneSessionPrivacyModeBundles, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    DisplayId displayId = 0;
    std::unordered_map<DisplayId, std::unordered_set<std::string>> privacyBundles;
    ssm_->GetSceneSessionPrivacyModeBundles(displayId, privacyBundles);
    SessionInfo sessionInfoFirst;
    sessionInfoFirst.bundleName_ = "";
    sessionInfoFirst.abilityName_ = "privacyAbilityName";
    sptr<SceneSession> sceneSessionFirst = sptr<SceneSession>::MakeSptr(sessionInfoFirst, nullptr);
    ASSERT_NE(sceneSessionFirst, nullptr);
    sceneSessionFirst->property_ = nullptr;
    ASSERT_NE(nullptr, ssm_);
    ssm_->GetSceneSessionPrivacyModeBundles(displayId, privacyBundles);
    sceneSessionFirst->property_ = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, sceneSessionFirst->property_);
    sceneSessionFirst->property_->SetDisplayId(0);
    ASSERT_NE(nullptr, ssm_);
    ssm_->GetSceneSessionPrivacyModeBundles(displayId, privacyBundles);
    sessionInfoFirst.bundleName_ = "privacy.test.first";
    sceneSessionFirst->state_ = SessionState::STATE_FOREGROUND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->GetSceneSessionPrivacyModeBundles(displayId, privacyBundles);
    sceneSessionFirst->state_ = SessionState::STATE_CONNECT;
    ASSERT_NE(nullptr, ssm_);
    ssm_->GetSceneSessionPrivacyModeBundles(displayId, privacyBundles);
}

/**
 * @tc.name: GetSceneSessionPrivacyModeBundles01
 * @tc.desc: GetSceneSessionPrivacyModeBundles
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetSceneSessionPrivacyModeBundles01, TestSize.Level1)
{
    DisplayId displayId = 0;
    std::unordered_map<DisplayId, std::unordered_set<std::string>> privacyBundles;
    ssm_->GetSceneSessionPrivacyModeBundles(displayId, privacyBundles);
    SessionInfo sessionInfoFirst;
    sessionInfoFirst.bundleName_ = "privacy.test.first";
    sessionInfoFirst.abilityName_ = "privacyAbilityName";
    sptr<SceneSession> sceneSessionFirst = sptr<SceneSession>::MakeSptr(sessionInfoFirst, nullptr);
    ASSERT_NE(sceneSessionFirst, nullptr);
    sceneSessionFirst->property_ = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, sceneSessionFirst->property_);
    sceneSessionFirst->property_->SetDisplayId(0);
    sceneSessionFirst->state_ = SessionState::STATE_ACTIVE;
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfoSecond;
    sessionInfoSecond.bundleName_ = "privacy.test.second";
    sessionInfoSecond.abilityName_ = "privacyAbilityName";
    sptr<SceneSession> sceneSessionSecond = sptr<SceneSession>::MakeSptr(sessionInfoSecond, nullptr);
    ASSERT_NE(nullptr, sceneSessionSecond);
    ssm_->sceneSessionMap_.insert({ sceneSessionSecond->GetPersistentId(), sceneSessionSecond });
    ASSERT_NE(nullptr, sceneSessionSecond->property_);
    sceneSessionSecond->property_->displayId_ = 1;
    sceneSessionSecond->state_ = SessionState::STATE_ACTIVE;
    sceneSessionSecond->parentSession_ = sceneSessionFirst;
    ASSERT_NE(nullptr, ssm_);
    ssm_->GetSceneSessionPrivacyModeBundles(displayId, privacyBundles);
    sceneSessionSecond->state_ = SessionState::STATE_FOREGROUND;
    sceneSessionSecond->state_ = SessionState::STATE_CONNECT;
    ASSERT_NE(nullptr, ssm_);
    ssm_->GetSceneSessionPrivacyModeBundles(displayId, privacyBundles);
}

/**
 * @tc.name: GetSceneSessionPrivacyModeBundles02
 * @tc.desc: GetSceneSessionPrivacyModeBundles
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetSceneSessionPrivacyModeBundles02, TestSize.Level1)
{
    DisplayId displayId = 0;
    std::unordered_map<DisplayId, std::unordered_set<std::string>> privacyBundles;
    ASSERT_NE(nullptr, ssm_);
    ssm_->GetSceneSessionPrivacyModeBundles(displayId, privacyBundles);
    SessionInfo sessionInfoFirst;
    sessionInfoFirst.bundleName_ = "privacy.test.first";
    sessionInfoFirst.abilityName_ = "privacyAbilityName";
    sptr<SceneSession> sceneSessionFirst = sptr<SceneSession>::MakeSptr(sessionInfoFirst, nullptr);
    ASSERT_NE(sceneSessionFirst, nullptr);
    sceneSessionFirst->property_ = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, sceneSessionFirst->property_);
    sceneSessionFirst->property_->SetDisplayId(0);
    sceneSessionFirst->state_ = SessionState::STATE_ACTIVE;
    sceneSessionFirst->property_->isPrivacyMode_ = false;
    ASSERT_NE(nullptr, ssm_);
    ssm_->GetSceneSessionPrivacyModeBundles(displayId, privacyBundles);
}

/**
 * @tc.name: RegisterWindowManagerAgent
 * @tc.desc: RegisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, RegisterWindowManagerAgent, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_SYSTEM_BAR;
    sptr<IWindowManagerAgent> windowManagerAgent = nullptr;
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GESTURE_NAVIGATION_ENABLED;
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WATER_MARK_FLAG;
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY;
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE;
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_VISIBLE_WINDOW_NUM;
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_FLOAT;
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: OnSessionStateChange
 * @tc.desc: OnSessionStateChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, OnSessionStateChange, TestSize.Level1)
{
    SessionState state = SessionState::STATE_FOREGROUND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->OnSessionStateChange(1, state);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->OnSessionStateChange(1, state);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(nullptr, ssm_);
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = true;
    ssm_->OnSessionStateChange(1, state);
    ASSERT_NE(nullptr, ssm_);
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = false;
    ssm_->OnSessionStateChange(1, state);
    focusGroup->SetFocusedSessionId(0);
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
}

/**
 * @tc.name: OnSessionStateChange01
 * @tc.desc: OnSessionStateChange01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, OnSessionStateChange01, TestSize.Level1)
{
    SessionState state = SessionState::STATE_BACKGROUND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
    state = SessionState::STATE_END;
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
}

/**
 * @tc.name: OnSessionStateChange02
 * @tc.desc: OnSessionStateChange02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, OnSessionStateChange02, TestSize.Level1)
{
    SessionState state = SessionState::STATE_FOREGROUND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    sceneSession->SetFocusedOnShow(true);
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
    sceneSession->SetFocusedOnShow(false);
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
}

/**
 * @tc.name: ProcessModalTopmostRequestFocusImmediately
 * @tc.desc: ProcessModalTopmostRequestFocusImmediately
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, ProcessModalTopmostRequestFocusImmediately, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->ProcessModalTopmostRequestFocusImmediately(sceneSession);
    EXPECT_EQ(WSError::WS_DO_NOTHING, ret);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->ProcessModalTopmostRequestFocusImmediately(sceneSession);
    EXPECT_EQ(WSError::WS_DO_NOTHING, ret);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->ProcessModalTopmostRequestFocusImmediately(sceneSession);
    EXPECT_EQ(WSError::WS_DO_NOTHING, ret);
}

/**
 * @tc.name: GetAbilityInfosFromBundleInfo
 * @tc.desc: GetAbilityInfosFromBundleInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetAbilityInfosFromBundleInfo, TestSize.Level1)
{
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    std::vector<SCBAbilityInfo> scbAbilityInfos;
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->GetAbilityInfosFromBundleInfo(bundleInfos, scbAbilityInfos);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
    OHOS::AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = "com.ix.residentservcie";
    bundleInfo.isKeepAlive = true;
    bundleInfo.applicationInfo.process = "com.ix.residentservcie";
    OHOS::AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.isModuleJson = true;
    hapModuleInfo.mainElementName = "residentServiceAbility";
    hapModuleInfo.process = "com.ix.residentservcie";
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);
    bundleInfos.emplace_back(bundleInfo);
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->GetAbilityInfosFromBundleInfo(bundleInfos, scbAbilityInfos);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: GetCollaboratorAbilityInfos01
 * @tc.desc: GetCollaboratorAbilityInfos01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetCollaboratorAbilityInfos01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    std::vector<SCBAbilityInfo> scbAbilityInfos;
    int32_t userId = 0;
    ssm_->GetCollaboratorAbilityInfos(bundleInfos, scbAbilityInfos, userId);
    EXPECT_EQ(scbAbilityInfos.size(), 0);
}

/**
 * @tc.name: GetCollaboratorAbilityInfos02
 * @tc.desc: GetCollaboratorAbilityInfos02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetCollaboratorAbilityInfos02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::string launcherBundleName = "launcherBundleName";
    std::string launcherModuleName = "launcherModuleName";
    std::string launcherAbilityName = "launcherAbilityName";
    AppExecFwk::AbilityInfo launcherAbility;
    launcherAbility.bundleName = launcherBundleName;
    launcherAbility.moduleName = launcherModuleName;
    launcherAbility.name = launcherAbilityName;
    sptr<IBundleMgrMocker> bundleMgrMocker = sptr<IBundleMgrMocker>::MakeSptr();
    EXPECT_CALL(*bundleMgrMocker, QueryLauncherAbilityInfos(_, _, _))
        .WillOnce([launcherAbility](const AAFwk::Want &want, int32_t userId,
            std::vector<AppExecFwk::AbilityInfo>& abilityInfos) {
            abilityInfos.emplace_back(launcherAbility);
            return 0;
        });
    ssm_->bundleMgr_ = bundleMgrMocker;
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = launcherBundleName;
    AppExecFwk::HapModuleInfo hapModuleInfo;
    AppExecFwk::AbilityInfo abilityInfo;
    hapModuleInfo.abilityInfos.emplace_back(abilityInfo);
    hapModuleInfo.abilityInfos.emplace_back(launcherAbility);
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);
    bundleInfos.emplace_back(bundleInfo);
    std::vector<SCBAbilityInfo> scbAbilityInfos;
    int32_t userId = 0;
    ssm_->GetCollaboratorAbilityInfos(bundleInfos, scbAbilityInfos, userId);
    EXPECT_EQ(scbAbilityInfos.size(), 1);
    EXPECT_EQ(scbAbilityInfos[0].abilityInfo_.moduleName, launcherModuleName);
    EXPECT_EQ(scbAbilityInfos[0].abilityInfo_.name, launcherAbilityName);
}

/**
 * @tc.name: GetCollaboratorAbilityInfos03
 * @tc.desc: GetCollaboratorAbilityInfos03
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetCollaboratorAbilityInfos03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<IBundleMgrMocker> bundleMgrMocker = sptr<IBundleMgrMocker>::MakeSptr();
    EXPECT_CALL(*bundleMgrMocker, QueryLauncherAbilityInfos(_, _, _))
        .WillOnce([](const AAFwk::Want &want, int32_t userId,
            std::vector<AppExecFwk::AbilityInfo> &abilityInfos) {
            return 0;
        });
    ssm_->bundleMgr_ = bundleMgrMocker;
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::HapModuleInfo hapModuleInfo;
    std::string abilityName1 = "testAbilityName1";
    AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo1.name = abilityName1;
    hapModuleInfo.abilityInfos.emplace_back(abilityInfo1);
    std::string abilityName2 = "testAbilityName2";
    AppExecFwk::AbilityInfo abilityInfo2;
    abilityInfo2.name = abilityName2;
    hapModuleInfo.abilityInfos.emplace_back(abilityInfo2);
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);
    bundleInfos.emplace_back(bundleInfo);
    std::vector<SCBAbilityInfo> scbAbilityInfos;
    int32_t userId = 0;
    ssm_->GetCollaboratorAbilityInfos(bundleInfos, scbAbilityInfos, userId);
    EXPECT_EQ(scbAbilityInfos.size(), 1);
    EXPECT_EQ(scbAbilityInfos[0].abilityInfo_.name, abilityName1);
}

/**
 * @tc.name: GetOrientationFromResourceManager
 * @tc.desc: GetOrientationFromResourceManager
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetOrientationFromResourceManager, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = "testBundleName";
    abilityInfo.moduleName = "testModuleName";
    abilityInfo.orientationId = 123456;
    ssm_->GetOrientationFromResourceManager(abilityInfo);
    EXPECT_EQ(OHOS::AppExecFwk::DisplayOrientation::UNSPECIFIED, abilityInfo.orientation);
}

/**
 * @tc.name: NotifyCompleteFirstFrameDrawing
 * @tc.desc: NotifyCompleteFirstFrameDrawing
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, NotifyCompleteFirstFrameDrawing, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sessionInfo.abilityInfo = nullptr;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ASSERT_NE(nullptr, ssm_);
    ssm_->NotifyCompleteFirstFrameDrawing(1);
    sessionInfo.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(nullptr, sessionInfo.abilityInfo);
    ASSERT_NE(nullptr, ssm_);
    ssm_->eventHandler_ = nullptr;
    ssm_->NotifyCompleteFirstFrameDrawing(1);
    ssm_->eventHandler_ = std::make_shared<AppExecFwk::EventHandler>();
    ASSERT_NE(nullptr, ssm_->eventHandler_);
    ASSERT_NE(nullptr, ssm_);
    ssm_->NotifyCompleteFirstFrameDrawing(1);
    ASSERT_NE(nullptr, ssm_);
    ssm_->taskScheduler_ = nullptr;
    ssm_->NotifyCompleteFirstFrameDrawing(1);
    ssm_->taskScheduler_ = std::make_shared<TaskScheduler>("OS_SceneSessionManager");
    ASSERT_NE(nullptr, ssm_->taskScheduler_);
    ASSERT_NE(nullptr, ssm_);
    ssm_->NotifyCompleteFirstFrameDrawing(1);
}

/**
 * @tc.name: NotifyCompleteFirstFrameDrawing02
 * @tc.desc: NotifyCompleteFirstFrameDrawing02:AtomicService free-install start.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, NotifyCompleteFirstFrameDrawing02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sessionInfo.abilityInfo = nullptr;
    sessionInfo.isAtomicService_ = true;
    sessionInfo.isBackTransition_ = false;
    unsigned int flags = 11111111;
    sessionInfo.want = std::make_shared<AAFwk::Want>();
    ASSERT_NE(nullptr, sessionInfo.want);
    sessionInfo.want->SetFlags(flags);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->NotifyCompleteFirstFrameDrawing(1);
    EXPECT_FALSE(g_logMsg.find("sceneSession is nullptr.") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: InitSceneSession01
 * @tc.desc: InitSceneSession01:AtomicService free-install start.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, InitSceneSession01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sessionInfo.abilityInfo = nullptr;
    sessionInfo.isAtomicService_ = true;
    sessionInfo.isBackTransition_ = false;
    sessionInfo.screenId_ = 100;
    unsigned int flags = 11111111;
    sessionInfo.want = std::make_shared<AAFwk::Want>();
    ASSERT_NE(nullptr, sessionInfo.want);
    sessionInfo.want->SetFlags(flags);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    ssm_->InitSceneSession(sceneSession, sessionInfo, nullptr);
    ASSERT_EQ(100, sceneSession->GetSessionInfo().screenId_);
}

/**
 * @tc.name: InitSceneSession02
 * @tc.desc: InitSceneSession02:in pc or pcmode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, InitSceneSession02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "InitSceneSession02";
    sessionInfo.abilityName_ = "InitSceneSession02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    auto oldUIType = ssm_->systemConfig_.windowUIType_;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    ssm_->InitSceneSession(sceneSession, sessionInfo, nullptr);
    EXPECT_NE(sceneSession->getStartWindowConfigFunc_, nullptr);

    sceneSession->getStartWindowConfigFunc_ = nullptr;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(sceneSession->getStartWindowConfigFunc_, nullptr);
    ssm_->systemConfig_.windowUIType_ = oldUIType;
}

/**
 * @tc.name: CheckAndNotifyWaterMarkChangedResult
 * @tc.desc: CheckAndNotifyWaterMarkChangedResult
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckAndNotifyWaterMarkChangedResult, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->CheckAndNotifyWaterMarkChangedResult();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->flags_ = 1 << 4;
    sceneSession->isRSVisible_ = true;
    sceneSession->combinedExtWindowFlags_.waterMarkFlag = true;
    ssm_->CheckAndNotifyWaterMarkChangedResult();
    sceneSession->isRSVisible_ = false;
    ssm_->CheckAndNotifyWaterMarkChangedResult();
    sceneSession->property_->flags_ = 0;
    sceneSession->isRSVisible_ = false;
    ssm_->CheckAndNotifyWaterMarkChangedResult();
    sceneSession->isRSVisible_ = true;
    ssm_->CheckAndNotifyWaterMarkChangedResult();
}

/**
 * @tc.name: CheckAndNotifyWaterMarkChangedResult01
 * @tc.desc: CheckAndNotifyWaterMarkChangedResult01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckAndNotifyWaterMarkChangedResult01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->flags_ = 1 << 4;
    sceneSession->isRSVisible_ = true;
    sceneSession->combinedExtWindowFlags_.waterMarkFlag = false;
    ASSERT_NE(nullptr, ssm_);
    ssm_->CheckAndNotifyWaterMarkChangedResult();
    sceneSession->isRSVisible_ = false;
    ASSERT_NE(nullptr, ssm_);
    ssm_->CheckAndNotifyWaterMarkChangedResult();
    sceneSession->property_->flags_ = 0;
    sceneSession->isRSVisible_ = false;
    ASSERT_NE(nullptr, ssm_);
    ssm_->CheckAndNotifyWaterMarkChangedResult();
    sceneSession->isRSVisible_ = true;
    ASSERT_NE(nullptr, ssm_);
    ssm_->CheckAndNotifyWaterMarkChangedResult();
}

/**
 * @tc.name: CheckAndNotifyWaterMarkChangedResult02
 * @tc.desc: CheckAndNotifyWaterMarkChangedResult02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckAndNotifyWaterMarkChangedResult02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->lastWaterMarkShowState_ = true;
    ssm_->CheckAndNotifyWaterMarkChangedResult();
    ASSERT_NE(nullptr, ssm_);
    ssm_->lastWaterMarkShowState_ = false;
    ssm_->CheckAndNotifyWaterMarkChangedResult();
}

/**
 * @tc.name: FillWindowInfo01
 * @tc.desc: FillWindowInfo01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, FillWindowInfo01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    sptr<SceneSession> sceneSession = nullptr;
    auto ret = ssm_->FillWindowInfo(infos, sceneSession);
    EXPECT_EQ(false, ret);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "FillWindowInfo01";
    sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ret = ssm_->FillWindowInfo(infos, sceneSession);
    EXPECT_EQ(true, ret);
    EXPECT_EQ(1, infos.size());
    sceneSession->hidingStartWindow_ = true;
    ret = ssm_->FillWindowInfo(infos, sceneSession);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FillWindowInfo02
 * @tc.desc: FillWindowInfo02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, FillWindowInfo02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SCBGestureBack";
    sessionInfo.abilityName_ = "FillWindowInfo02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    auto ret = ssm_->FillWindowInfo(infos, sceneSession);
    EXPECT_EQ(false, ret);
    EXPECT_EQ(0, infos.size());
}

/**
 * @tc.name: FillWindowInfo03
 * @tc.desc: FillWindowInfo03
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, FillWindowInfo03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "FillWindowInfo03";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    auto ret = ssm_->FillWindowInfo(infos, sceneSession);
    EXPECT_EQ(true, ret);
    EXPECT_EQ(1, infos.size());
    EXPECT_EQ(1, infos[0]->wid_);
}

/**
 * @tc.name: FillWindowInfo04
 * @tc.desc: FillWindowInfo04
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, FillWindowInfo04, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "FillWindowInfo04";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->property_->SetDisplayId(1);
    auto ret = ssm_->FillWindowInfo(infos, sceneSession);
    EXPECT_EQ(true, ret);
    EXPECT_EQ(1, infos.size());
    EXPECT_EQ(1, infos[0]->displayId_);
}

/**
 * @tc.name: FillWindowInfo05
 * @tc.desc: FillWindowInfo05
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, FillWindowInfo05, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "FillWindowInfo05";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->GetSessionProperty()->SetIsSystemKeyboard(true);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::HALF_FOLDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1649, 2472, 40 });
    WSRect area = { 0, 1690, 2472, 1648 };
    sceneSession->SetSessionGlobalRect(area);
    auto ret = ssm_->FillWindowInfo(infos, sceneSession);
    EXPECT_EQ(true, ret);
    EXPECT_EQ(1, infos.size());
    EXPECT_EQ(0, infos[0]->displayId_);

    infos.clear();
    sceneSession->GetSessionProperty()->SetIsSystemKeyboard(false);
    ret = ssm_->FillWindowInfo(infos, sceneSession);
    EXPECT_EQ(true, ret);
    EXPECT_EQ(1, infos.size());
    EXPECT_EQ(999, infos[0]->displayId_);
}

/**
 * @tc.name: SetSessionVisibilityInfo01
 * @tc.desc: SetSessionVisibilityInfo01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, SetSessionVisibilityInfo01, TestSize.Level1)
{
    sptr<SceneSession> session = nullptr;
    WindowVisibilityState visibleState = WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION;
    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
    std::string visibilityInfo = "";
    ASSERT_NE(nullptr, ssm_);
    ssm_->SetSessionVisibilityInfo(session, visibleState, windowVisibilityInfos, visibilityInfo);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    session = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    ASSERT_NE(nullptr, session);
    session->persistentId_ = 1;
    ssm_->windowVisibilityListenerSessionSet_.clear();
    ssm_->SetSessionVisibilityInfo(session, visibleState, windowVisibilityInfos, visibilityInfo);
    ssm_->windowVisibilityListenerSessionSet_.insert(1);
    ssm_->SetSessionVisibilityInfo(session, visibleState, windowVisibilityInfos, visibilityInfo);
}

/**
 * @tc.name: SetSessionVisibilityInfo02
 * @tc.desc: SetSessionVisibilityInfo02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, SetSessionVisibilityInfo02, TestSize.Level1)
{
    WindowVisibilityState visibleState = WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION;
    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
    std::string visibilityInfo = "";
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sessionInfo.callerPersistentId_ = 2;
    auto session1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    auto session2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    session1->persistentId_ = 1;
    session2->persistentId_ = 2;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert({ 1, session1 });
    ssm_->sceneSessionMap_.insert({ 2, session2 });
    ssm_->windowVisibilityListenerSessionSet_.clear();
    ssm_->windowVisibilityListenerSessionSet_.insert(1);
    ssm_->occlusionStateListenerSessionSet_.clear();
    ssm_->occlusionStateListenerSessionSet_.insert(1);
    ssm_->SetSessionVisibilityInfo(session1, visibleState, windowVisibilityInfos, visibilityInfo);
    EXPECT_NE(windowVisibilityInfos.size(), 0);
    ssm_->sceneSessionMap_.clear();
    ssm_->occlusionStateListenerSessionSet_.clear();
}

/**
 * @tc.name: UpdateSessionOcclusionStateListener
 * @tc.desc: update window occlusion state
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, UpdateSessionOcclusionStateListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    ssm_->sceneSessionMap_.clear();
    ssm_->occlusionStateListenerSessionSet_.clear();
    auto result = ssm_->UpdateSessionOcclusionStateListener(100, false);
    EXPECT_EQ(result, WMError::WM_DO_NOTHING);
    EXPECT_EQ(ssm_->occlusionStateListenerSessionSet_.size(), 0);

    SessionInfo sessionInfo1;
    auto session1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    session1->persistentId_ = 1;
    session1->sessionStage_ = nullptr;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, session1));
    result = ssm_->UpdateSessionOcclusionStateListener(session1->persistentId_, true);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(ssm_->occlusionStateListenerSessionSet_.size(), 1);

    SessionInfo sessionInfo2;
    sessionInfo2.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    auto session2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    session2->persistentId_ = 2;
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    session2->sessionStage_ = mockSessionStage;
    ssm_->sceneSessionMap_.insert(std::make_pair(2, session2));
    result = ssm_->UpdateSessionOcclusionStateListener(session2->persistentId_, true);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(ssm_->occlusionStateListenerSessionSet_.size(), 2);

    result = ssm_->UpdateSessionOcclusionStateListener(session1->persistentId_, false);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(ssm_->occlusionStateListenerSessionSet_.size(), 1);

    ssm_->occlusionStateListenerSessionSet_.clear();
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
}

/**
 * @tc.name: SendTouchEvent
 * @tc.desc: SendTouchEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, SendTouchEvent, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->SendTouchEvent(pointerEvent, 0);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.pointerId_ = 0;
    pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    pointerEvent->pointerId_ = 0;
    pointerEvent->AddPointerItem(pointerItem);
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->SendTouchEvent(pointerEvent, 0);
    EXPECT_EQ(WSError::WS_OK, ret);
    pointerEvent->pointerId_ = 1;
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->SendTouchEvent(pointerEvent, 0);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
    ASSERT_NE(nullptr, ssm_);
    ssm_->RegisterWindowChanged(WindowChangedFuncTest6);
}

/**
 * @tc.name: JudgeNeedNotifyPrivacyInfo
 * @tc.desc: JudgeNeedNotifyPrivacyInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, JudgeNeedNotifyPrivacyInfo, TestSize.Level1)
{
    DisplayId displayId = 1;
    std::unordered_set<std::string> privacyBundles;
    ssm_->privacyBundleMap_.clear();
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->JudgeNeedNotifyPrivacyInfo(displayId, privacyBundles);
    EXPECT_EQ(true, ret);
    privacyBundles.insert("bundle1");
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->JudgeNeedNotifyPrivacyInfo(displayId, privacyBundles);
    EXPECT_EQ(true, ret);
    std::unordered_set<std::string> privacyBundles1;
    privacyBundles1.insert("bundle2");
    ASSERT_NE(nullptr, ssm_);
    ssm_->privacyBundleMap_.insert({ displayId, privacyBundles1 });
    ret = ssm_->JudgeNeedNotifyPrivacyInfo(displayId, privacyBundles);
    EXPECT_EQ(true, ret);
    privacyBundles.insert("bundle2");
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->JudgeNeedNotifyPrivacyInfo(displayId, privacyBundles);
    EXPECT_EQ(true, ret);
    ASSERT_NE(nullptr, ssm_);
    ssm_->InitPersistentStorage();
    ASSERT_NE(nullptr, ssm_);
    ssm_->UpdateCameraFloatWindowStatus(0, true);
    ASSERT_NE(nullptr, ssm_);
    ssm_->UpdateCameraWindowStatus(0, true);
}

/**
 * @tc.name: UpdatePrivateStateAndNotify
 * @tc.desc: UpdatePrivateStateAndNotify
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, UpdatePrivateStateAndNotify, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->privacyBundleMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->UpdatePrivateStateAndNotify(1);
    ASSERT_EQ(ssm_->privacyBundleMap_[1].size(), 0);
    ssm_->UpdatePrivateStateAndNotifyForAllScreens();
}

/**
 * @tc.name: UpdatePrivateStateAndNotify2
 * @tc.desc: UpdatePrivateStateAndNotify2
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, UpdatePrivateStateAndNotify2, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->privacyBundleMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetPrivacyMode(true);
    sceneSession->property_->SetDisplayId(1);
    sceneSession->isVisible_ = false;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession));
    ssm_->UpdatePrivateStateAndNotify(1);
    ASSERT_EQ(ssm_->privacyBundleMap_[1].size(), 1);
    sceneSession->SetSessionState(SessionState::STATE_BACKGROUND);
    sceneSession->property_->SetPrivacyMode(true);
    sceneSession->property_->SetDisplayId(2);
    sceneSession->isVisible_ = true;
    ssm_->UpdatePrivateStateAndNotify(2);
    ASSERT_EQ(ssm_->privacyBundleMap_[2].size(), 1);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->property_->SetPrivacyMode(false);
    sceneSession->property_->SetDisplayId(3);
    ssm_->UpdatePrivateStateAndNotify(3);
    ASSERT_EQ(ssm_->privacyBundleMap_[3].size(), 0);
}

/**
 * @tc.name: UpdatePrivateStateAndNotify3
 * @tc.desc: UpdatePrivateStateAndNotify3
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, UpdatePrivateStateAndNotify3, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->privacyBundleMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->isVisible_ = true;
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetPrivacyMode(true);
    sceneSession->property_->SetDisplayId(1);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession));
    ssm_->UpdatePrivateStateAndNotify(1);
    ASSERT_EQ(ssm_->privacyBundleMap_[1].size(), 1);
    sceneSession->property_->SetPrivacyMode(false);
    sceneSession->combinedExtWindowFlags_.privacyModeFlag = true;
    sceneSession->property_->SetDisplayId(2);
    ssm_->UpdatePrivateStateAndNotify(2);
    ASSERT_EQ(ssm_->privacyBundleMap_[2].size(), 1);
}

/**
 * @tc.name: UpdatePrivateStateAndNotify4
 * @tc.desc: UpdatePrivateStateAndNotify4
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, UpdatePrivateStateAndNotify4, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->privacyBundleMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession2);
    sceneSession2->SetSessionState(SessionState::STATE_FOREGROUND);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetParentSession(sceneSession2);
    ASSERT_NE(nullptr, sceneSession->GetParentSession());
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetPrivacyMode(true);
    sceneSession->property_->SetDisplayId(1);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession));
    ssm_->UpdatePrivateStateAndNotify(1);
    ASSERT_EQ(ssm_->privacyBundleMap_[1].size(), 1);
    sceneSession->property_->SetDisplayId(2);
    sceneSession->GetParentSession()->SetSessionState(SessionState::STATE_BACKGROUND);
    ssm_->UpdatePrivateStateAndNotify(2);
    ASSERT_EQ(ssm_->privacyBundleMap_[2].size(), 0);
}

/**
 * @tc.name: GetCollaboratorByType
 * @tc.desc: GetCollaboratorByType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetCollaboratorByType, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->collaboratorMap_.clear();
    auto ret = ssm_->GetCollaboratorByType(0);
    EXPECT_EQ(nullptr, ret);
    sptr<AAFwk::IAbilityManagerCollaborator> collaborator = nullptr;
    ASSERT_NE(nullptr, ssm_);
    ssm_->collaboratorMap_.insert(std::make_pair(1, collaborator));
    ret = ssm_->GetCollaboratorByType(1);
    EXPECT_EQ(nullptr, ret);
}

/**
 * @tc.name: RegisterGetStateFromManagerFunc
 * @tc.desc: RegisterGetStateFromManagerFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, RegisterGetStateFromManagerFunc, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->RegisterGetStateFromManagerFunc(sceneSession);
    sceneSession = nullptr;
    ASSERT_NE(nullptr, ssm_);
    ssm_->RegisterGetStateFromManagerFunc(sceneSession);
}

/**
 * @tc.name: ProcessDialogRequestFocusImmediately
 * @tc.desc: ProcessDialogRequestFocusImmediately
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, ProcessDialogRequestFocusImmediately, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->ProcessDialogRequestFocusImmediately(sceneSession);
    EXPECT_EQ(WSError::WS_DO_NOTHING, ret);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->ProcessDialogRequestFocusImmediately(sceneSession);
    EXPECT_EQ(WSError::WS_DO_NOTHING, ret);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    ASSERT_NE(nullptr, ssm_);
    ret = ssm_->ProcessDialogRequestFocusImmediately(sceneSession);
    EXPECT_EQ(WSError::WS_DO_NOTHING, ret);
}

/**
 * @tc.name: IsValidSessionIds
 * @tc.desc: IsValidSessionIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, IsValidSessionIds, TestSize.Level1)
{
    std::vector<int32_t> sessionIds = { 1, 2, 3, 4 };
    std::vector<bool> results;
    results.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    sptr<SceneSession> sceneSession1 = nullptr;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession1));
    ssm_->IsValidSessionIds(sessionIds, results);
    EXPECT_FALSE(results.empty());
    DisplayChangeListener listener;
    std::vector<uint64_t> missionIds;
    std::vector<uint64_t> surfaceNodeIds;
    listener.OnGetSurfaceNodeIdsFromMissionIds(missionIds, surfaceNodeIds);
}

/**
*@tc.name:GetSurfaceNodeIdsFromMissionIds01
*@tc.desc:IsValidSessionIds
*@tc.type:FUNC
*/
HWTEST_F(SceneSessionManagerTest6, GetSurfaceNodeIdsFromMissionIds01, TestSize.Level1)
{
    std::vector<int32_t> sessionIds = {1, 2, 3, 4, 5};
    // sessionInfo是闪控球的id 1并且有surfaceNode
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_FB);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    // 设置surfaceNode
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    sceneSession->SetSurfaceNode(surfaceNode);

    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    // sessionInfo1是空指针
    sptr<SceneSession> sceneSession1 = nullptr;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession1));

    // sessionInfo3是pip类型的id 3 surfaceNode为空
    SessionInfo sessionInfo3;
    sessionInfo3.bundleName_ = "SceneSessionManagerTest3";;
    sessionInfo3.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_PIP);
    auto session3 = sptr<SceneSession>::MakeSptr(sessionInfo3, nullptr);
    session3->SetSurfaceNode(nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(3, session3));

    // sessionInfo5是selection的id 5并且有surfacenNode
    SessionInfo sessionInfo5;
    sessionInfo5.bundleName_ = "SceneSessionManagerTest5";
    sessionInfo5.abilityName_ = "DumpSessionWithId5";
    sessionInfo5.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_SELECTION);
    sptr<SceneSession> sceneSession5 = sptr<SceneSession>::MakeSptr(sessionInfo5, nullptr);
    ASSERT_NE(nullptr, sceneSession5);
    // 设置surfaceNode
    struct RSSurfaceNodeConfig config5;
    std::shared_ptr<RSSurfaceNode> surfaceNode5 = RSSurfaceNode::Create(config5);
    sceneSession5->SetSurfaceNode(surfaceNode5);
    ssm_->sceneSessionMap_.insert(std::make_pair(5, sceneSession5));
    // 设置公共参数
    const std::vector<uint32_t> needWindowTypeList = { 2145 };
    WMError error = WMError::WM_OK;

    // 测试正常过滤的流程
    std::vector<uint64_t> surfaceNodesListNormal;
    std::vector<uint64_t> missionIdNormal = {1};
    error = ssm_->GetSurfaceNodeIdsFromMissionIds(missionIdNormal, surfaceNodesListNormal,
        needWindowTypeList);
    EXPECT_EQ(error, WMError::WM_OK);

    // 测试不存在的ID的流程的流程
    std::vector<uint64_t> surfaceNodesListNoExist;
    std::vector<uint64_t> missionIdNoExist = {100};
    error = ssm_->GetSurfaceNodeIdsFromMissionIds(missionIdNoExist, surfaceNodesListNoExist,
        needWindowTypeList, true);
    EXPECT_EQ(error, WMError::WM_ERROR_INVALID_WINDOW);
    error = ssm_->GetSurfaceNodeIdsFromMissionIds(missionIdNoExist, surfaceNodesListNoExist,
        needWindowTypeList, false);
    EXPECT_EQ(error, WMError::WM_OK);

    // 测试scensession为空的流程的流程
    std::vector<uint64_t> surfaceNodesListSceneNull;
    std::vector<uint64_t> missionIdSceneNull = {2};
    error = ssm_->GetSurfaceNodeIdsFromMissionIds(missionIdSceneNull, surfaceNodesListSceneNull,
        needWindowTypeList, true);
    EXPECT_EQ(error, WMError::WM_ERROR_INVALID_WINDOW);
    error = ssm_->GetSurfaceNodeIdsFromMissionIds(missionIdSceneNull, surfaceNodesListSceneNull,
        needWindowTypeList, false);
    EXPECT_EQ(error, WMError::WM_OK);

    // 测试GetSurfaceNode为空的流程的流程
    std::vector<uint64_t> surfaceNodesListSurfaceNull;
    std::vector<uint64_t> missionIdSurfaceNull = {3};
    error = ssm_->GetSurfaceNodeIdsFromMissionIds(missionIdSurfaceNull, surfaceNodesListSurfaceNull,
        needWindowTypeList, true);
    EXPECT_EQ(error, WMError::WM_ERROR_INVALID_WINDOW);
    error = ssm_->GetSurfaceNodeIdsFromMissionIds(missionIdSurfaceNull, surfaceNodesListSurfaceNull,
        needWindowTypeList, false);
    EXPECT_EQ(error, WMError::WM_OK);

    // 测试list为空正常的流程的流程
    std::vector<uint64_t> surfaceNodesListOKEmpty;
    std::vector<uint64_t> missionIdOKEmpty = {1};
    error = ssm_->GetSurfaceNodeIdsFromMissionIds(missionIdOKEmpty, surfaceNodesListOKEmpty);
    EXPECT_EQ(error, WMError::WM_OK);

    // 测试转换闪控球为空的流程的流程
    std::vector<uint64_t> surfaceNodesListFBAndAnother;
    std::vector<uint64_t> missionIdFBAndAnother = {1, 5};
    error = ssm_->GetSurfaceNodeIdsFromMissionIds(missionIdFBAndAnother, surfaceNodesListFBAndAnother,
        needWindowTypeList, true);
    EXPECT_EQ(error, WMError::WM_ERROR_INVALID_WINDOW);
    error = ssm_->GetSurfaceNodeIdsFromMissionIds(missionIdFBAndAnother, surfaceNodesListFBAndAnother,
        needWindowTypeList, false);
    EXPECT_EQ(error, WMError::WM_OK);
}

/**
 * @tc.name: DeleteStateDetectTask
 * @tc.desc: DeleteStateDetectTask
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, DeleteStateDetectTask, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->SetScreenLocked(true);
    sleep(1);
    EXPECT_EQ(true, ssm_->isScreenLocked_);
    ssm_->sceneSessionMap_.clear();
    ASSERT_NE(nullptr, ssm_);
    ssm_->DeleteStateDetectTask();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest2";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->detectTaskInfo_.taskState = DetectTaskState::NO_TASK;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ASSERT_NE(nullptr, ssm_);
    ssm_->DeleteStateDetectTask();
    sceneSession->detectTaskInfo_.taskState = DetectTaskState::ATTACH_TASK;
    ASSERT_NE(nullptr, ssm_);
    ssm_->DeleteStateDetectTask();
}

/**
 * @tc.name: GetWindowStyleType
 * @tc.desc: GetWindowStyleType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetWindowStyleType, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ASSERT_NE(nullptr, ssm_);
    WindowStyleType windowModeType = Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT;
    auto ret = ssm_->GetWindowStyleType(windowModeType);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
    EXPECT_TRUE(g_logMsg.find("permission denied!") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: TerminateSessionByPersistentId
 * @tc.desc: Success to terminate session by persistentId.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, TerminateSessionByPersistentId001, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_WINDOW_BASE);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession->GetPersistentId(), sceneSession));
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    auto result = ssm_->TerminateSessionByPersistentId(sceneSession->GetPersistentId());
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: TerminateSessionByPersistentId
 * @tc.desc: Fail to terminate session by persistentId, invalid persistentId.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, TerminateSessionByPersistentId002, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_WINDOW_BASE);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession->GetPersistentId(), sceneSession));
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    auto result = ssm_->TerminateSessionByPersistentId(INVALID_SESSION_ID);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: SetRootSceneProcessBackEventFunc
 * @tc.desc: test function : SetRootSceneProcessBackEventFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, SetRootSceneProcessBackEventFunc, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
    sessionInfo.abilityName_ = "SetRootSceneProcessBackEventFunc";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::APP_WINDOW_BASE);
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession->GetPersistentId(), sceneSession));
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(sceneSession->GetPersistentId());
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = false;
    ssm_->ProcessBackEvent();

    RootSceneProcessBackEventFunc func = []() {};
    ssm_->SetRootSceneProcessBackEventFunc(func);
    ssm_->ProcessBackEvent();
}

/**
 * @tc.name: RequestInputMethodCloseKeyboard
 * @tc.desc: RequestInputMethodCloseKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, RequestInputMethodCloseKeyboard, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback = nullptr;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    ssm_->sceneSessionMap_.insert({ 0, sceneSession });
    int32_t persistentId = 10;
    ssm_->RequestInputMethodCloseKeyboard(persistentId);

    persistentId = 0;
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session->property_ = nullptr;
    ssm_->RequestInputMethodCloseKeyboard(persistentId);

    bool enable = true;
    auto result = ssm_->GetFreeMultiWindowEnableState(enable);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: RequestSceneSession
 * @tc.desc: RequestSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, RequestSceneSession, TestSize.Level0)
{
    SessionInfo info1;
    info1.persistentId_ = 1;
    info1.isPersistentRecover_ = false;
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();

    SessionInfo info2;
    info2.abilityName_ = "RequestSceneSession";
    info2.bundleName_ = "RequestSceneSession";
    info2.persistentId_ = 1;

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    sptr<SceneSession> getSceneSession1 = ssm_->RequestSceneSession(info1, windowSessionProperty);
    ASSERT_EQ(info1.bundleName_, getSceneSession1->GetSessionInfo().bundleName_);

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ssm_->sceneSessionMap_.insert({ 2, sceneSession2 });
    sptr<SceneSession> getSceneSession2 = ssm_->RequestSceneSession(info2, windowSessionProperty);
    ASSERT_NE(info2.bundleName_, getSceneSession2->GetSessionInfo().bundleName_);
}

/**
 * @tc.name: GetSceneSessionBySessionInfo
 * @tc.desc: GetSceneSessionBySessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetSceneSessionBySessionInfo, TestSize.Level1)
{
    SessionInfo info1;
    info1.persistentId_ = 1;
    info1.isPersistentRecover_ = false;
    info1.windowType_ = 1000;
    info1.appInstanceKey_ = "";
    ASSERT_EQ(ssm_->GetSceneSessionBySessionInfo(info1), nullptr);

    SessionInfo info2;
    info2.persistentId_ = 1;
    info2.isPersistentRecover_ = false;
    info2.windowType_ = 1;
    info2.bundleName_ = "GetSceneSessionBySessionInfoBundle";
    info2.abilityName_ = "GetSceneSessionBySessionInfoAbility";
    info2.appInstanceKey_ = "";
    info2.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(nullptr, info2.abilityInfo);
    info2.abilityInfo->launchMode = AppExecFwk::LaunchMode::SINGLETON;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    sptr<SceneSession> getSceneSession = ssm_->GetSceneSessionBySessionInfo(info2);
    ASSERT_EQ(sceneSession, getSceneSession);

    SessionInfo info3;
    info3.persistentId_ = 2;
    info3.isPersistentRecover_ = false;
    info3.windowType_ = 1;
    info3.bundleName_ = "GetSceneSessionBySessionInfoBundle2";
    info3.abilityName_ = "GetSceneSessionBySessionInfoAbility2";
    info3.appInstanceKey_ = "";
    info3.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    info3.abilityInfo->launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info3, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    ssm_->sceneSessionMap_.insert({ 2, sceneSession2 });
    info3.persistentId_ = 1000;
    ASSERT_EQ(ssm_->GetSceneSessionBySessionInfo(info3), nullptr);

    SessionInfo info4;
    info4.persistentId_ = 0;
    info4.isPersistentRecover_ = false;
    ASSERT_EQ(ssm_->GetSceneSessionBySessionInfo(info4), nullptr);

    SessionInfo info5;
    info5.persistentId_ = 5;
    info5.isPersistentRecover_ = true;
    ASSERT_EQ(ssm_->GetSceneSessionBySessionInfo(info5), nullptr);
}

/**
 * @tc.name: RequestSceneSessionDestruction
 * @tc.desc: RequestSceneSessionDestruction
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, RequestSceneSessionDestruction, TestSize.Level1)
{
    sptr<SceneSession> sceneSession;
    ASSERT_EQ(sceneSession, nullptr);
    bool needRemoveSession = true;
    bool isSaveSnapshot = true;
    bool isForceClean = true;
    ASSERT_EQ(WSError::WS_OK,
              ssm_->RequestSceneSessionDestruction(sceneSession, needRemoveSession, isSaveSnapshot, isForceClean));

    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback = nullptr;
    sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_EQ(WSError::WS_OK,
              ssm_->RequestSceneSessionDestruction(sceneSession, needRemoveSession, isSaveSnapshot, isForceClean));
}

/**
 * @tc.name: NotifySessionAINavigationBarChange
 * @tc.desc: NotifySessionAINavigationBarChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, NotifySessionAINavigationBarChange, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    int32_t persistentId = 1;
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback = nullptr;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    ssm_->sceneSessionMap_.insert({ 0, sceneSession });
    ssm_->NotifySessionAINavigationBarChange(persistentId);

    persistentId = 0;
    Session session(info);
    session.isVisible_ = true;
    session.state_ = SessionState::STATE_FOREGROUND;
    ssm_->NotifySessionAINavigationBarChange(persistentId);
}

/**
 * @tc.name: GetProcessSurfaceNodeIdByPersistentId
 * @tc.desc: GetProcessSurfaceNodeIdByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetProcessSurfaceNodeIdByPersistentId, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback = nullptr;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, specificCallback);
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info, specificCallback);
    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(info, specificCallback);
    sceneSession1->SetCallingPid(123);
    sceneSession2->SetCallingPid(123);
    sceneSession3->SetCallingPid(111);

    int32_t pid = 123;
    std::vector<int32_t> persistentIds;
    std::vector<uint64_t> surfaceNodeIds;
    persistentIds.push_back(sceneSession1->GetPersistentId());
    persistentIds.push_back(sceneSession2->GetPersistentId());
    persistentIds.push_back(sceneSession3->GetPersistentId());
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });
    ssm_->sceneSessionMap_.insert({ sceneSession3->GetPersistentId(), sceneSession3 });

    ASSERT_EQ(WMError::WM_OK, ssm_->GetProcessSurfaceNodeIdByPersistentId(pid, persistentIds, surfaceNodeIds));
    ASSERT_EQ(0, surfaceNodeIds.size());
}

/**
 * @tc.name: OnScreenFoldStatusChanged
 * @tc.desc: OnScreenFoldStatusChanged
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, OnScreenFoldStatusChanged, TestSize.Level1)
{
    std::vector<std::string> screenFoldInfo;
    sptr<IDisplayChangeListener> listener = sptr<DisplayChangeListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    listener->OnScreenFoldStatusChanged(screenFoldInfo);
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->UpdateDisplayHookInfo(0, 50, 50, 0.0f, true);
    EXPECT_EQ(ret, WMError::WM_OK);
    ssm_->CheckSceneZOrder();
}

/**
 * @tc.name: NotifySessionForeground
 * @tc.desc: NotifySessionForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, NotifySessionForeground, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
    sessionInfo.abilityName_ = "NotifySessionForeground";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    uint32_t reason = 0;
    bool withAnimation = false;
    ASSERT_NE(nullptr, ssm_);
    ssm_->NotifySessionForeground(sceneSession, reason, withAnimation);
    WSRect area = { 0, 0, 0, 0 };
    uint32_t type = 0;
    uint64_t displayId = 0;
    ssm_->AddWindowDragHotArea(displayId, type, area);
    ssm_->currAINavigationBarAreaMap_.clear();
    ssm_->currAINavigationBarAreaMap_.insert(std::make_pair(displayId, area));
    auto ret = ssm_->GetAINavigationBarArea(1);
    EXPECT_TRUE(ret.IsEmpty());
    ret = ssm_->GetAINavigationBarArea(displayId);
    EXPECT_EQ(ret, area);
}

/**
 * @tc.name: OnDisplayStateChange
 * @tc.desc: OnDisplayStateChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, OnDisplayStateChange, TestSize.Level1)
{
    DisplayChangeListener listener;
    DisplayId displayId = 0;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(nullptr, displayInfo);
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    displayInfoMap.insert(std::make_pair(displayId, displayInfo));
    DisplayStateChangeType type = DisplayStateChangeType::VIRTUAL_PIXEL_RATIO_CHANGE;
    listener.OnDisplayStateChange(displayId, displayInfo, displayInfoMap, type);
    type = DisplayStateChangeType::UPDATE_ROTATION;
    listener.OnDisplayStateChange(displayId, displayInfo, displayInfoMap, type);
    type = DisplayStateChangeType::UPDATE_SCALE;
    listener.OnDisplayStateChange(displayId, displayInfo, displayInfoMap, type);
    type = DisplayStateChangeType::UNKNOWN;
    listener.OnDisplayStateChange(displayId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: CheckIfReuseSession
 * @tc.desc: CheckIfReuseSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckIfReuseSession, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
    sessionInfo.abilityName_ = "CheckIfReuseSession";
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->CheckIfReuseSession(sessionInfo);
    EXPECT_EQ(ret, BrokerStates::BROKER_UNKOWN);
    ScreenId screenId = 0;
    std::unordered_map<int32_t, SessionUIParam> uiParams;
    ssm_->FlushUIParams(screenId, std::move(uiParams));
}

/**
 * @tc.name: CheckIfReuseSession02
 * @tc.desc: Test if CollaboratorType not exist and collaboratorMap_ not exist
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckIfReuseSession02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->bundleMgr_ = ssm_->GetBundleManager();
    ssm_->currentUserId_ = 123;

    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "SceneSessionManager";
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
    sessionInfo.abilityName_ = "CheckIfReuseSession02";
    sessionInfo.want = std::make_shared<AAFwk::Want>();

    SceneSessionManager::SessionInfoList list = { .uid_ = 123,
                                                  .bundleName_ = "SceneSessionManagerTest6",
                                                  .abilityName_ = "CheckIfReuseSession02",
                                                  .moduleName_ = "SceneSessionManager" };

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(abilityInfo, nullptr);
    ssm_->abilityInfoMap_[list] = abilityInfo;
    auto ret1 = ssm_->CheckIfReuseSession(sessionInfo);
    ASSERT_EQ(ret1, BrokerStates::BROKER_UNKOWN);
    ssm_->abilityInfoMap_.erase(list);
}

/**
 * @tc.name: CheckIfReuseSession03
 * @tc.desc: Test if CollaboratorType is RESERVE_TYPE and collaboratorMap_ not exist
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckIfReuseSession03, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->bundleMgr_ = ssm_->GetBundleManager();
    ssm_->currentUserId_ = 123;

    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "SceneSessionManager";
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
    sessionInfo.abilityName_ = "CheckIfReuseSession03";
    sessionInfo.want = std::make_shared<AAFwk::Want>();

    SceneSessionManager::SessionInfoList list = { .uid_ = 123,
                                                  .bundleName_ = "SceneSessionManagerTest6",
                                                  .abilityName_ = "CheckIfReuseSession03",
                                                  .moduleName_ = "SceneSessionManager" };

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(abilityInfo, nullptr);
    abilityInfo->applicationInfo.codePath = std::to_string(CollaboratorType::RESERVE_TYPE);
    ssm_->abilityInfoMap_[list] = abilityInfo;
    auto ret2 = ssm_->CheckIfReuseSession(sessionInfo);
    ASSERT_EQ(ret2, BrokerStates::BROKER_UNKOWN);
    ssm_->abilityInfoMap_.erase(list);
}

/**
 * @tc.name: CheckIfReuseSession04
 * @tc.desc: Test if CollaboratorType is RESERVE_TYPE and collaboratorMap_ exist
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckIfReuseSession04, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->bundleMgr_ = ssm_->GetBundleManager();
    ssm_->currentUserId_ = 123;

    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "SceneSessionManager";
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
    sessionInfo.abilityName_ = "CheckIfReuseSession04";
    sessionInfo.want = std::make_shared<AAFwk::Want>();

    SceneSessionManager::SessionInfoList list = { .uid_ = 123,
                                                  .bundleName_ = "SceneSessionManagerTest6",
                                                  .abilityName_ = "CheckIfReuseSession04",
                                                  .moduleName_ = "SceneSessionManager" };

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(abilityInfo, nullptr);
    abilityInfo->applicationInfo.codePath = std::to_string(CollaboratorType::RESERVE_TYPE);
    ssm_->abilityInfoMap_[list] = abilityInfo;

    sptr<AAFwk::IAbilityManagerCollaborator> collaborator = iface_cast<AAFwk::IAbilityManagerCollaborator>(nullptr);
    ssm_->collaboratorMap_.insert(std::make_pair(1, collaborator));
    auto ret3 = ssm_->CheckIfReuseSession(sessionInfo);
    ASSERT_EQ(ret3, BrokerStates::BROKER_UNKOWN);
    ssm_->abilityInfoMap_.erase(list);
    ssm_->collaboratorMap_.erase(1);
}

/**
 * @tc.name: CheckIfReuseSession05
 * @tc.desc: Test if CollaboratorType is OTHERS_TYPE and collaboratorMap_ exist
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckIfReuseSession05, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->bundleMgr_ = ssm_->GetBundleManager();
    ssm_->currentUserId_ = 123;

    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "SceneSessionManager";
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
    sessionInfo.abilityName_ = "CheckIfReuseSession05";
    sessionInfo.want = std::make_shared<AAFwk::Want>();

    SceneSessionManager::SessionInfoList list = { .uid_ = 123,
                                                  .bundleName_ = "SceneSessionManagerTest6",
                                                  .abilityName_ = "CheckIfReuseSession05",
                                                  .moduleName_ = "SceneSessionManager" };

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(abilityInfo, nullptr);
    abilityInfo->applicationInfo.codePath = std::to_string(CollaboratorType::OTHERS_TYPE);
    ssm_->abilityInfoMap_[list] = abilityInfo;

    sptr<AAFwk::IAbilityManagerCollaborator> collaborator = iface_cast<AAFwk::IAbilityManagerCollaborator>(nullptr);
    ssm_->collaboratorMap_.insert(std::make_pair(1, collaborator));
    auto ret4 = ssm_->CheckIfReuseSession(sessionInfo);
    ASSERT_EQ(ret4, BrokerStates::BROKER_UNKOWN);
    ssm_->abilityInfoMap_.erase(list);
    ssm_->collaboratorMap_.erase(1);
}

/**
 * @tc.name: CheckIfReuseSession06
 * @tc.desc: Test if CollaboratorType is RESERVE_TYPE and collaboratorMap_ not exist
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, CheckIfReuseSession06, TestSize.Level1)
{
    EXPECT_NE(ssm_, nullptr);
    ssm_->bundleMgr_ = ssm_->GetBundleManager();
    ssm_->currentUserId_ = 123;

    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "SceneSessionManager";
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
    sessionInfo.abilityName_ = "CheckIfReuseSession06";
    sessionInfo.want = std::make_shared<AAFwk::Want>();

    SceneSessionManager::SessionInfoList list = { .uid_ = 123,
                                                  .bundleName_ = "SceneSessionManagerTest6",
                                                  .abilityName_ = "CheckIfReuseSession06",
                                                  .moduleName_ = "SceneSessionManager" };

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->applicationInfo.codePath = std::to_string(CollaboratorType::REDIRECT_TYPE);
    ssm_->abilityInfoMap_[list] = abilityInfo;
    auto ret2 = ssm_->CheckIfReuseSession(sessionInfo);
    EXPECT_EQ(ret2, BrokerStates::BROKER_UNKOWN);
    ssm_->abilityInfoMap_.erase(list);
}

/**
 * @tc.name: UpdateAvoidArea
 * @tc.desc: UpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, UpdateAvoidArea, TestSize.Level1)
{
    int32_t persistentId = 0;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr();
    ssm_->UpdateAvoidArea(persistentId);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
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
 * @tc.name: UpdateMaximizeMode
 * @tc.desc: UpdateMaximizeMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, UpdateMaximizeMode, TestSize.Level1)
{
    int32_t persistentId = 0;
    bool isMaximize = true;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    auto ret = ssm_->UpdateMaximizeMode(persistentId, isMaximize);
    EXPECT_EQ(ret, WSError::WS_OK);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
    sessionInfo.abilityName_ = "UpdateMaximizeMode";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(persistentId, sceneSession));
    EXPECT_EQ(ret, WSError::WS_OK);
    sptr<DisplayInfo> displayInfo = nullptr;
    ssm_->ProcessDisplayScale(displayInfo);
    displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(nullptr, displayInfo);
    ssm_->ProcessDisplayScale(displayInfo);
    ProcessVirtualPixelRatioChangeFunc func = nullptr;
    ssm_->SetVirtualPixelRatioChangeListener(func);
}

/**
 * @tc.name: WindowDestroyNotifyVisibility
 * @tc.desc: WindowDestroyNotifyVisibility
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, WindowDestroyNotifyVisibility, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
    sessionInfo.abilityName_ = "WindowDestroyNotifyVisibility";
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->WindowDestroyNotifyVisibility(sceneSession);
    sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetRSVisible(false);
    ssm_->WindowDestroyNotifyVisibility(sceneSession);
    sceneSession->SetRSVisible(true);
    ASSERT_NE(nullptr, ssm_);
    ssm_->WindowDestroyNotifyVisibility(sceneSession);
    ASSERT_FALSE(sceneSession->GetRSVisible());
}

/**
 * @tc.name: GetApplicationInfo
 * @tc.desc: GetApplicationInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest6, GetApplicationInfo, TestSize.Level1)
{
    std::string bundleName = "com.ohos.sceneboard";
    SCBApplicationInfo applicationInfo;
    ASSERT_NE(nullptr, ssm_);
    WSError ret = ssm_->GetApplicationInfo(bundleName, applicationInfo);
    EXPECT_EQ(WSError::WS_OK, ret);
}
} // namespace
} // namespace Rosen
} // namespace OHOS