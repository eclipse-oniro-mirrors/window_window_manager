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

#include <gtest/gtest.h>
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/scene_session.h"
#include "session_info.h"
#include "wm_common.h"
#include "mock/mock_accesstoken_kit.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class SceneSessionManagerTest13 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    
    static sptr<SceneSession> CreateSceneSession(int32_t persistentId, const std::string& bundleName);
    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerTest13::ssm_ = nullptr;

void SceneSessionManagerTest13::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerTest13::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest13::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerTest13::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    MockAccesstokenKit::ChangeMockStateToInit();
    ssm_->sceneSessionMap_.clear();
}

sptr<SceneSession> SceneSessionManagerTest13::CreateSceneSession(int32_t persistentId, const std::string& bundleName)
{
    SessionInfo info;
    info.persistentId_ = persistentId;
    info.bundleName_ = bundleName;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    return sceneSession;
}

/**
 * @tc.name: GetAppWindowShowingInfosByBundleName_001
 * @tc.desc: Test GetAppWindowShowingInfosByBundleName with non-SA calling
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest13, GetAppWindowShowingInfosByBundleName_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    
    MockAccesstokenKit::MockIsSACalling(false);
    
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.app";
    appInfo.appIndex = 0;
    appInfo.appInstanceKey = "";
    
    std::vector<AppWindowShowingInfo> windowInfos;
    WMError result = ssm_->GetAppWindowShowingInfosByBundleName(appInfo, windowInfos);
    
    EXPECT_EQ(result, WMError::WM_ERROR_NOT_SYSTEM_APP);
    EXPECT_EQ(windowInfos.size(), 0);
}

/**
 * @tc.name: GetAppWindowShowingInfosByBundleName_002
 * @tc.desc: Test GetAppWindowShowingInfosByBundleName with empty bundleName
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest13, GetAppWindowShowingInfosByBundleName_002, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    
    MockAccesstokenKit::MockIsSACalling(true);
    
    ApplicationInfo appInfo;
    appInfo.bundleName = "";
    appInfo.appIndex = 0;
    appInfo.appInstanceKey = "";
    
    std::vector<AppWindowShowingInfo> windowInfos;
    WMError result = ssm_->GetAppWindowShowingInfosByBundleName(appInfo, windowInfos);
    
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);
    EXPECT_EQ(windowInfos.size(), 0);
}

/**
 * @tc.name: GetAppWindowShowingInfosByBundleName_003
 * @tc.desc: Test GetAppWindowShowingInfosByBundleName with valid params and no sessions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest13, GetAppWindowShowingInfosByBundleName_003, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    
    MockAccesstokenKit::MockIsSACalling(true);
    
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.app";
    appInfo.appIndex = 0;
    appInfo.appInstanceKey = "";
    
    std::vector<AppWindowShowingInfo> windowInfos;
    WMError result = ssm_->GetAppWindowShowingInfosByBundleName(appInfo, windowInfos);
    
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(windowInfos.size(), 0);
}

/**
 * @tc.name: GetAppWindowShowingInfosByBundleName_004
 * @tc.desc: Test GetAppWindowShowingInfosByBundleName with valid params and sessions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest13, GetAppWindowShowingInfosByBundleName_004, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    
    MockAccesstokenKit::MockIsSACalling(true);
    
    sptr<SceneSession> session1 = CreateSceneSession(1, "com.test.app");
    ASSERT_NE(session1, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, session1));
    
    sptr<SceneSession> session2 = CreateSceneSession(2, "com.test.app");
    ASSERT_NE(session2, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, session2));
    
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.app";
    appInfo.appIndex = 0;
    appInfo.appInstanceKey = "";
    
    std::vector<AppWindowShowingInfo> windowInfos;
    WMError result = ssm_->GetAppWindowShowingInfosByBundleName(appInfo, windowInfos);
    
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(windowInfos.size(), 2);
}

/**
 * @tc.name: GetAppWindowShowingInfosByBundleName_005
 * @tc.desc: Test GetAppWindowShowingInfosByBundleName with different bundleName
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest13, GetAppWindowShowingInfosByBundleName_005, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    
    MockAccesstokenKit::MockIsSACalling(true);
    
    sptr<SceneSession> session1 = CreateSceneSession(101, "com.test.app1");
    ASSERT_NE(session1, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(101, session1));
    
    sptr<SceneSession> session2 = CreateSceneSession(102, "com.test.app2");
    ASSERT_NE(session2, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(102, session2));
    
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.app1";
    appInfo.appIndex = 0;
    appInfo.appInstanceKey = "";
    
    std::vector<AppWindowShowingInfo> windowInfos;
    WMError result = ssm_->GetAppWindowShowingInfosByBundleName(appInfo, windowInfos);
    
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(windowInfos.size(), 1);
    if (windowInfos.size() > 0) {
        EXPECT_EQ(windowInfos[0].persistentId, 101);
    }
}

/**
 * @tc.name: UpdateShowOnDockByPersistentIds_001
 * @tc.desc: Test UpdateShowOnDockByPersistentIds with empty list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest13, UpdateShowOnDockByPersistentIds_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    
    sptr<SceneSession> session1 = CreateSceneSession(201, "com.test.app");
    ASSERT_NE(session1, nullptr);
    session1->SetIsShowOnDock(true);
    ssm_->sceneSessionMap_.insert(std::make_pair(201, session1));
    
    sptr<SceneSession> session2 = CreateSceneSession(202, "com.test.app");
    ASSERT_NE(session2, nullptr);
    session2->SetIsShowOnDock(true);
    ssm_->sceneSessionMap_.insert(std::make_pair(202, session2));
    
    std::vector<int32_t> persistentIds;
    ssm_->UpdateShowOnDockByPersistentIds(persistentIds);
    
    EXPECT_EQ(session1->GetIsShowOnDock(), false);
    EXPECT_EQ(session2->GetIsShowOnDock(), false);
}

/**
 * @tc.name: UpdateShowOnDockByPersistentIds_002
 * @tc.desc: Test UpdateShowOnDockByPersistentIds with valid list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest13, UpdateShowOnDockByPersistentIds_002, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    
    sptr<SceneSession> session1 = CreateSceneSession(301, "com.test.app");
    ASSERT_NE(session1, nullptr);
    session1->SetIsShowOnDock(false);
    ssm_->sceneSessionMap_.insert(std::make_pair(301, session1));
    
    sptr<SceneSession> session2 = CreateSceneSession(302, "com.test.app");
    ASSERT_NE(session2, nullptr);
    session2->SetIsShowOnDock(false);
    ssm_->sceneSessionMap_.insert(std::make_pair(302, session2));
    
    sptr<SceneSession> session3 = CreateSceneSession(303, "com.test.app");
    ASSERT_NE(session3, nullptr);
    session3->SetIsShowOnDock(true);
    ssm_->sceneSessionMap_.insert(std::make_pair(303, session3));
    
    std::vector<int32_t> persistentIds = {301, 302};
    ssm_->UpdateShowOnDockByPersistentIds(persistentIds);
    
    EXPECT_EQ(session1->GetIsShowOnDock(), true);
    EXPECT_EQ(session2->GetIsShowOnDock(), true);
    EXPECT_EQ(session3->GetIsShowOnDock(), false);
}

/**
 * @tc.name: UpdateShowOnDockByPersistentIds_003
 * @tc.desc: Test UpdateShowOnDockByPersistentIds with partial match
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest13, UpdateShowOnDockByPersistentIds_003, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    
    sptr<SceneSession> session1 = CreateSceneSession(401, "com.test.app");
    ASSERT_NE(session1, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(401, session1));
    
    sptr<SceneSession> session2 = CreateSceneSession(402, "com.test.app");
    ASSERT_NE(session2, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(402, session2));
    
    sptr<SceneSession> session3 = CreateSceneSession(403, "com.test.app");
    ASSERT_NE(session3, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(403, session3));
    
    std::vector<int32_t> persistentIds = {401, 403};
    ssm_->UpdateShowOnDockByPersistentIds(persistentIds);
    
    EXPECT_EQ(session1->GetIsShowOnDock(), true);
    EXPECT_EQ(session2->GetIsShowOnDock(), false);
    EXPECT_EQ(session3->GetIsShowOnDock(), true);
}

/**
 * @tc.name: UpdateShowOnDockByPersistentIds_004
 * @tc.desc: Test UpdateShowOnDockByPersistentIds with nullptr session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest13, UpdateShowOnDockByPersistentIds_004, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    
    sptr<SceneSession> session1 = CreateSceneSession(501, "com.test.app");
    ASSERT_NE(session1, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(501, session1));
    
    ssm_->sceneSessionMap_.insert(std::make_pair(502, nullptr));
    
    sptr<SceneSession> session3 = CreateSceneSession(503, "com.test.app");
    ASSERT_NE(session3, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(503, session3));
    
    std::vector<int32_t> persistentIds = {501, 502, 503};
    ssm_->UpdateShowOnDockByPersistentIds(persistentIds);
    
    EXPECT_EQ(session1->GetIsShowOnDock(), true);
    EXPECT_EQ(session3->GetIsShowOnDock(), true);
}

/**
 * @tc.name: UpdateShowOnDockByPersistentIds_005
 * @tc.desc: Test UpdateShowOnDockByPersistentIds with no sessions in map
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest13, UpdateShowOnDockByPersistentIds_005, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    
    ssm_->sceneSessionMap_.clear();
    
    std::vector<int32_t> persistentIds = {1, 2, 3};
    ssm_->UpdateShowOnDockByPersistentIds(persistentIds);
    
    EXPECT_EQ(ssm_->sceneSessionMap_.size(), 0);
}

} // namespace Rosen
} // namespace OHOS