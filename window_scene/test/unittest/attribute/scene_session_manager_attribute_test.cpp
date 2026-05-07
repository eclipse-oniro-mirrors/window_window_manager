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
#include "pointer_event.h"
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
class SceneSessionManagerAttributeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t waitSyncInNs = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerAttributeTest::ssm_ = nullptr;

void SceneSessionManagerAttributeTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerAttributeTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerAttributeTest::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerAttributeTest::TearDown()
{
    MockAccesstokenKit::ChangeMockStateToInit();
    usleep(waitSyncInNs);
    ssm_->sceneSessionMap_.clear();
}

namespace {
/**
 * @tc.name: GetTopNavDestinationName
 * @tc.desc: test GetTopNavDestinationName whether get the top nav destination name.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, GetTopNavDestinationName, TestSize.Level1)
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
 * @tc.name: GetRealSessionState
 * @tc.desc: test get the real session state
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, GetRealSessionState, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    ssm_->sceneSessionMap_.clear();
    EXPECT_EQ(ssm_->GetRealSessionState(nullptr), SessionState::STATE_DISCONNECT);

    SessionInfo parentInfo;
    sptr<SceneSession> parentSession = sptr<SceneSession>::MakeSptr(parentInfo, nullptr);
    ASSERT_NE(parentSession, nullptr);
    parentSession->persistentId_ = 1;
    parentSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    parentSession->SetSessionState(SessionState::STATE_ACTIVE);
    ssm_->sceneSessionMap_.insert(std::make_pair(parentSession->GetPersistentId(), parentSession));
    EXPECT_EQ(ssm_->GetRealSessionState(parentSession), SessionState::STATE_ACTIVE);

    SessionInfo childInfo;
    sptr<SceneSession> childSession = sptr<SceneSession>::MakeSptr(childInfo, nullptr);
    ASSERT_NE(childSession, nullptr);
    childSession->persistentId_ = 2;
    childSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    childSession->SetParentPersistentId(parentSession->GetPersistentId());
    ssm_->sceneSessionMap_.insert(std::make_pair(childSession->GetPersistentId(), childSession));

    childSession->SetSessionState(SessionState::STATE_BACKGROUND);
    EXPECT_EQ(ssm_->GetRealSessionState(childSession), SessionState::STATE_BACKGROUND);

    childSession->SetSessionState(SessionState::STATE_FOREGROUND);
    EXPECT_EQ(ssm_->GetRealSessionState(childSession), SessionState::STATE_ACTIVE);

    parentSession->SetSessionState(SessionState::STATE_BACKGROUND);
    EXPECT_EQ(ssm_->GetRealSessionState(childSession), SessionState::STATE_BACKGROUND);

    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
}

/**
 * @tc.name: IsNeedNotifyScreenshotEvent
 * @tc.desc: test IsNeedNotifyScreenshotEvent.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, IsNeedNotifyScreenshotEvent, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    auto oldScreenshotEventListenerSessionSet = ssm_->screenshotAppEventListenerSessionSet_;
    auto oldScreenshotListenerSessionSet = ssm_->screenshotListenerSessionSet_;
    ssm_->screenshotListenerSessionSet_.clear();
    ssm_->sceneSessionMap_.clear();
    ssm_->screenshotAppEventListenerSessionSet_.clear();
    EXPECT_EQ(ssm_->IsNeedNotifyScreenshotEvent(nullptr), false);

    SessionInfo sessionInfo;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->property_->SetPersistentId(100);
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession->GetPersistentId(), sceneSession));
    sceneSession->SetSessionState(SessionState::STATE_DISCONNECT);
    EXPECT_EQ(ssm_->IsNeedNotifyScreenshotEvent(sceneSession), false);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    EXPECT_EQ(ssm_->IsNeedNotifyScreenshotEvent(sceneSession), true);

    SessionInfo sessionInfo2;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subSession->property_->SetPersistentId(101);
    subSession->property_->SetParentPersistentId(sceneSession->GetPersistentId());
    subSession->SetParentSession(sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(subSession->GetPersistentId(), subSession));
    ssm_->screenshotAppEventListenerSessionSet_.insert(subSession->GetPersistentId());

    subSession->SetSessionState(SessionState::STATE_DISCONNECT);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    EXPECT_EQ(ssm_->IsNeedNotifyScreenshotEvent(subSession), true);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    EXPECT_NE(ssm_->UpdateSessionScreenshotListener(1100, false), WMError::WM_OK);
    EXPECT_EQ(ssm_->UpdateSessionScreenshotListener(sceneSession->GetPersistentId(), true), WMError::WM_OK);
    ssm_->OnScreenshot(0);
    auto ret = ssm_->NotifyScreenshotEvent(ScreenshotEventType::SCROLL_SHOT_START);
    EXPECT_NE(ret, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_EQ(ssm_->IsNeedNotifyScreenshotEvent(subSession), true);
    EXPECT_EQ(ssm_->UpdateSessionScreenshotListener(sceneSession->GetPersistentId(), false), WMError::WM_OK);

    ssm_->sceneSessionMap_.clear();
    ssm_->screenshotAppEventListenerSessionSet_.clear();
    ssm_->screenshotListenerSessionSet_.clear();
    ssm_->screenshotListenerSessionSet_ = oldScreenshotListenerSessionSet;
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
    ssm_->screenshotAppEventListenerSessionSet_ = oldScreenshotEventListenerSessionSet;
}

/**
 * @tc.name: SetWindowSnapshotSkip
 * @tc.desc: test SetWindowSnapshotSkip
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, SetWindowSnapshotSkip, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    ssm_->sceneSessionMap_.clear();
    EXPECT_NE(ssm_->SetWindowSnapshotSkip(1, true), WMError::WM_ERROR_INVALID_CALLING);
    SessionInfo sessionInfo;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->property_->SetPersistentId(100);
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession->GetPersistentId(), sceneSession));
    EXPECT_NE(ssm_->SetWindowSnapshotSkip(sceneSession->GetPersistentId(), false), WMError::WM_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
}

/**
 * @tc.name: SetScreenWatermarkImage001
 * @tc.desc: test SetScreenWatermarkImage with null pixelMap
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, SetScreenWatermarkImage001, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::string bundleName;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    EXPECT_EQ(ssm_->SetScreenWatermarkImage(pixelMap, 1, bundleName), WMError::WM_ERROR_ILLEGAL_PARAM);
}

/**
 * @tc.name: SetScreenWatermarkImage002
 * @tc.desc: test SetScreenWatermarkImage without system permission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, SetScreenWatermarkImage002, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    std::string bundleName;
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(false);
    EXPECT_EQ(ssm_->SetScreenWatermarkImage(pixelMap, 1, bundleName), WMError::WM_ERROR_NOT_SYSTEM_APP);
    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: SetScreenWatermarkImage003
 * @tc.desc: test SetScreenWatermarkImage with system permission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, SetScreenWatermarkImage003, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldScreenWatermarkBundleName = ssm_->screenWatermarkBundleName_;
    auto oldScreenWatermarkPriority = ssm_->screenWatermarkPriority_;
    ssm_->screenWatermarkBundleName_ = "";
    ssm_->screenWatermarkPriority_ = 0;
    std::string bundleName;
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    MockAccesstokenKit::MockIsSystemApp(true);
    MockAccesstokenKit::MockIsSACalling(false);
    EXPECT_EQ(ssm_->SetScreenWatermarkImage(pixelMap, 1, bundleName), WMError::WM_OK);
    EXPECT_FALSE(bundleName.empty());
    ssm_->screenWatermarkBundleName_ = oldScreenWatermarkBundleName;
    ssm_->screenWatermarkPriority_ = oldScreenWatermarkPriority;
    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: SetScreenWatermarkImage004
 * @tc.desc: test SetScreenWatermarkImage with lower priority
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, SetScreenWatermarkImage004, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldScreenWatermarkBundleName = ssm_->screenWatermarkBundleName_;
    auto oldScreenWatermarkPriority = ssm_->screenWatermarkPriority_;
    ssm_->screenWatermarkBundleName_ = "test.bundle";
    ssm_->screenWatermarkPriority_ = 1;
    std::string bundleName;
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(true);
    EXPECT_EQ(ssm_->SetScreenWatermarkImage(pixelMap, 2, bundleName), WMError::WM_DO_NOTHING);
    ssm_->screenWatermarkBundleName_ = oldScreenWatermarkBundleName;
    ssm_->screenWatermarkPriority_ = oldScreenWatermarkPriority;
    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: SetScreenWatermarkImage005
 * @tc.desc: test SetScreenWatermarkImage with higher priority
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, SetScreenWatermarkImage005, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldScreenWatermarkBundleName = ssm_->screenWatermarkBundleName_;
    auto oldScreenWatermarkPriority = ssm_->screenWatermarkPriority_;
    ssm_->screenWatermarkBundleName_ = "test.bundle";
    ssm_->screenWatermarkPriority_ = 2;
    std::string bundleName;
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    MockAccesstokenKit::MockIsSystemApp(true);
    MockAccesstokenKit::MockIsSACalling(false);
    EXPECT_EQ(ssm_->SetScreenWatermarkImage(pixelMap, 1, bundleName), WMError::WM_OK);
    ssm_->screenWatermarkBundleName_ = oldScreenWatermarkBundleName;
    ssm_->screenWatermarkPriority_ = oldScreenWatermarkPriority;
    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: CleanScreenWatermarkImage001
 * @tc.desc: test CleanScreenWatermarkImage without system permission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, CleanScreenWatermarkImage001, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    EXPECT_EQ(ssm_->CleanScreenWatermarkImage(nullptr), WMError::WM_ERROR_ILLEGAL_PARAM);
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(false);
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    EXPECT_EQ(ssm_->CleanScreenWatermarkImage(pixelMap), WMError::WM_ERROR_NOT_SYSTEM_APP);
    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: CleanScreenWatermarkImage002
 * @tc.desc: test CleanScreenWatermarkImage when no watermark exists
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, CleanScreenWatermarkImage002, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldScreenWatermarkBundleName = ssm_->screenWatermarkBundleName_;
    auto oldScreenWatermarkPriority = ssm_->screenWatermarkPriority_;
    ssm_->screenWatermarkBundleName_ = "";
    ssm_->screenWatermarkPriority_ = 0;
    MockAccesstokenKit::MockIsSystemApp(true);
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    EXPECT_EQ(ssm_->CleanScreenWatermarkImage(pixelMap), WMError::WM_DO_NOTHING);
    ssm_->screenWatermarkBundleName_ = oldScreenWatermarkBundleName;
    ssm_->screenWatermarkPriority_ = oldScreenWatermarkPriority;
    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: CleanScreenWatermarkImage003
 * @tc.desc: test CleanScreenWatermarkImage when watermark exists but not owner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, CleanScreenWatermarkImage003, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldScreenWatermarkBundleName = ssm_->screenWatermarkBundleName_;
    auto oldScreenWatermarkPriority = ssm_->screenWatermarkPriority_;
    ssm_->screenWatermarkBundleName_ = "test.bundle";
    ssm_->screenWatermarkPriority_ = 1;
    MockAccesstokenKit::MockIsSystemApp(true);
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    EXPECT_EQ(ssm_->CleanScreenWatermarkImage(pixelMap), WMError::WM_DO_NOTHING);
    ssm_->screenWatermarkBundleName_ = oldScreenWatermarkBundleName;
    ssm_->screenWatermarkPriority_ = oldScreenWatermarkPriority;
    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: CleanScreenWatermarkImage004
 * @tc.desc: test CleanScreenWatermarkImage when watermark exists and is owner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, CleanScreenWatermarkImage004, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    auto oldScreenWatermarkBundleName = ssm_->screenWatermarkBundleName_;
    auto oldScreenWatermarkPriority = ssm_->screenWatermarkPriority_;
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "test.bundle";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetPersistentId(100);
    sceneSession->SetCallingPid(IPCSkeleton::GetCallingRealPid());
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession->GetPersistentId(), sceneSession));
    ssm_->screenWatermarkBundleName_ = "test.bundle";
    ssm_->screenWatermarkPriority_ = 1;
    MockAccesstokenKit::MockIsSystemApp(true);
    std::shared_ptr<Media::PixelMap> pixelMap = std::make_shared<Media::PixelMap>();
    EXPECT_EQ(ssm_->CleanScreenWatermarkImage(pixelMap), WMError::WM_OK);
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
    ssm_->screenWatermarkBundleName_ = oldScreenWatermarkBundleName;
    ssm_->screenWatermarkPriority_ = oldScreenWatermarkPriority;
    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: RecoverScreenWatermarkImage001
 * @tc.desc: test RecoverScreenWatermarkImage with empty bundleName
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, RecoverScreenWatermarkImage001, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldScreenWatermarkBundleName = ssm_->screenWatermarkBundleName_;
    auto oldScreenWatermarkPriority = ssm_->screenWatermarkPriority_;
    ssm_->screenWatermarkBundleName_ = "";
    ssm_->screenWatermarkPriority_ = 0;
    EXPECT_EQ(ssm_->RecoverScreenWatermarkImage("", 1), WMError::WM_OK);
    ssm_->screenWatermarkBundleName_ = oldScreenWatermarkBundleName;
    ssm_->screenWatermarkPriority_ = oldScreenWatermarkPriority;
}

/**
 * @tc.name: RecoverScreenWatermarkImage002
 * @tc.desc: test RecoverScreenWatermarkImage when no watermark exists
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, RecoverScreenWatermarkImage002, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldScreenWatermarkBundleName = ssm_->screenWatermarkBundleName_;
    auto oldScreenWatermarkPriority = ssm_->screenWatermarkPriority_;
    ssm_->screenWatermarkBundleName_ = "";
    ssm_->screenWatermarkPriority_ = 0;
    EXPECT_EQ(ssm_->RecoverScreenWatermarkImage("test.bundle", 1), WMError::WM_OK);
    ssm_->screenWatermarkBundleName_ = oldScreenWatermarkBundleName;
    ssm_->screenWatermarkPriority_ = oldScreenWatermarkPriority;
}

/**
 * @tc.name: RecoverScreenWatermarkImage003
 * @tc.desc: test RecoverScreenWatermarkImage when watermark exists
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, RecoverScreenWatermarkImage003, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldScreenWatermarkBundleName = ssm_->screenWatermarkBundleName_;
    auto oldScreenWatermarkPriority = ssm_->screenWatermarkPriority_;
    ssm_->screenWatermarkBundleName_ = "existing.bundle";
    ssm_->screenWatermarkPriority_ = 1;
    EXPECT_EQ(ssm_->RecoverScreenWatermarkImage("test.bundle", 2), WMError::WM_OK);
    ssm_->screenWatermarkBundleName_ = oldScreenWatermarkBundleName;
    ssm_->screenWatermarkPriority_ = oldScreenWatermarkPriority;
}

/**
 * @tc.name: RecoverScreenWatermarkImage004
 * @tc.desc: test RecoverScreenWatermarkImage with different priorities
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, RecoverScreenWatermarkImage004, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldScreen = ssm_->screenWatermarkBundleName_;
    auto oldScreenWatermarkPriority = ssm_->screenWatermarkPriority_;
    ssm_->screenWatermarkBundleName_ = "";
    ssm_->screenWatermarkPriority_ = 0;
    EXPECT_EQ(ssm_->RecoverScreenWatermarkImage("test.bundle", 100), WMError::WM_OK);
    ssm_->screenWatermarkBundleName_ = oldScreen;
    ssm_->screenWatermarkPriority_ = oldScreenWatermarkPriority;
}

/**
 * @tc.name: RecoverProcessWatermark001
 * @tc.desc: test RecoverProcessWatermark
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, RecoverProcessWatermark001, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->RecoverProcessWatermark(123, "RecoverProcessWatermarkName");
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: FilterForGetAllWindowLayoutInfo001
 * @tc.desc: test FilterForGetAllWindowLayoutInfo with empty sessions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, FilterForGetAllWindowLayoutInfo001, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    ssm_->sceneSessionMap_.clear();
    std::vector<sptr<SceneSession>> filteredSessions;
    WindowInfoOptions option;
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions, option);
    EXPECT_EQ(filteredSessions.size(), 0);
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
}

/**
 * @tc.name: FilterForGetAllWindowLayoutInfo002
 * @tc.desc: test FilterForGetAllWindowLayoutInfo with null session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, FilterForGetAllWindowLayoutInfo002, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(1, nullptr));
    std::vector<sptr<SceneSession>> filteredSessions;
    WindowInfoOptions option;
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions, option);
    EXPECT_EQ(filteredSessions.size(), 0);
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
}

/**
 * @tc.name: FilterForGetAllWindowLayoutInfo003
 * @tc.desc: test FilterForGetAllWindowLayoutInfo with invalid rect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, FilterForGetAllWindowLayoutInfo003, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "test.bundle";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetPersistentId(100);
    ssm_->sceneSessionMap_.insert(std::make_pair(100, sceneSession));
    std::vector<sptr<SceneSession>> filteredSessions;
    WindowInfoOptions option;
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions, option);
    EXPECT_EQ(filteredSessions.size(), 0);
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
}

/**
 * @tc.name: FilterForGetAllWindowLayoutInfo004
 * @tc.desc: test FilterForGetAllWindowLayoutInfo with invalid session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, FilterForGetAllWindowLayoutInfo004, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "test.bundle";
    sessionInfo.isSystem_ = false;
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetPersistentId(100);
    sceneSession->persistentId_ = 100;
    WSRect rect = {0, 0, 100, 100};
    sceneSession->SetSessionRect(rect);
    sceneSession->SetSessionGlobalRect(rect);
    sceneSession->SetZOrder(100);
    ssm_->sceneSessionMap_.insert(std::make_pair(100, sceneSession));
    std::vector<sptr<SceneSession>> filteredSessions;
    WindowInfoOptions option;
    option.excludeSystemWindows = true;
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions, option);
    EXPECT_EQ(filteredSessions.size(), 0);
    sceneSession->GetSessionProperty()->SetDisplayId(DEFAULT_DISPLAY_ID);
    sceneSession->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FB);
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions, option);
    EXPECT_EQ(filteredSessions.size(), 1);
    filteredSessions.clear();
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FV);
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions, option);
    EXPECT_EQ(filteredSessions.size(), 1);
    filteredSessions.clear();
    option.excludeSystemWindows = false;
    option.foregroundAboveWindow = 100;
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions, option);
    EXPECT_EQ(filteredSessions.size(), 0);
    option.foregroundAboveWindow = 10;
    option.foregroundBelowWindow = 100;
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions, option);
    EXPECT_EQ(filteredSessions.size(), 0);
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
}
} // namespace
} // namespace Rosen
} // namespace OHOS
