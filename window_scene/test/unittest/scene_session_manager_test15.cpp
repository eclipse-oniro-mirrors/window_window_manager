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

#include <unistd.h>

#include "display_manager.h"
#include "interfaces/include/ws_common.h"
#include "ipc_skeleton.h"
#include "mock/mock_accesstoken_kit.h"
#include "session/host/include/scene_session.h"
#include "session_info.h"
#include "session_manager/include/scene_session_manager.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const int32_t SLEEP_TIME{ 10000 };
constexpr int32_t DEFAULT_USERID = -1;
constexpr int32_t TEST_USER_ID_100 = 100;
constexpr int32_t TEST_USER_ID_101 = 101;
constexpr DisplayId TEST_DISPLAY_ID_1 = 1001;
constexpr DisplayId TEST_DISPLAY_ID_2 = 1002;
} // namespace

class SceneSessionManagerTest15 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;
};

sptr<SceneSessionManager> SceneSessionManagerTest15::ssm_ = nullptr;

void SceneSessionManagerTest15::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
}

void SceneSessionManagerTest15::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest15::SetUp()
{
    ssm_->userDefaultDisplayIdCache_.clear();
    ssm_->currentUserId_ = DEFAULT_USERID;
}

void SceneSessionManagerTest15::TearDown()
{
    ssm_->userDefaultDisplayIdCache_.clear();
    ssm_->currentUserId_ = DEFAULT_USERID;
    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: GetInstanceUserDefaultDisplay_DefaultUserId
 * @tc.desc: Test GetInstanceUserDefaultDisplay when currentUserId_ is DEFAULT_USERID
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, GetInstanceUserDefaultDisplay_DefaultUserId, TestSize.Level1)
{
    ssm_->currentUserId_ = DEFAULT_USERID;
    DisplayId displayId = ssm_->GetInstanceUserDefaultDisplay();
    EXPECT_EQ(displayId, DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: GetInstanceUserDefaultDisplay_ValidUserId
 * @tc.desc: Test GetInstanceUserDefaultDisplay when currentUserId_ is a valid user id
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, GetInstanceUserDefaultDisplay_ValidUserId, TestSize.Level1)
{
    ssm_->currentUserId_ = TEST_USER_ID_100;
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_100] = TEST_DISPLAY_ID_1;
    DisplayId displayId = ssm_->GetInstanceUserDefaultDisplay();
    EXPECT_EQ(displayId, DEFAULT_DISPLAY_ID);
    ssm_->currentUserId_ = DEFAULT_USERID;
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_MultiInstanceDisabled
 * @tc.desc: Test GetDefaultDisplayIdByUserId when multi-instance is not enabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, GetDefaultDisplayIdByUserId_MultiInstanceDisabled, TestSize.Level1)
{
    DisplayId displayId = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    EXPECT_EQ(displayId, DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_CacheHit
 * @tc.desc: Test GetDefaultDisplayIdByUserId returns cached value when cache contains userId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, GetDefaultDisplayIdByUserId_CacheHit, TestSize.Level1)
{
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_100] = TEST_DISPLAY_ID_1;
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_101] = TEST_DISPLAY_ID_2;
    DisplayId displayId1 = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    EXPECT_EQ(displayId1, DEFAULT_DISPLAY_ID);
    DisplayId displayId2 = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_101);
    EXPECT_EQ(displayId2, DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_CacheMissNoEntry
 * @tc.desc: Test GetDefaultDisplayIdByUserId when cache does not contain userId and multi-instance is disabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, GetDefaultDisplayIdByUserId_CacheMissNoEntry, TestSize.Level1)
{
    EXPECT_TRUE(ssm_->userDefaultDisplayIdCache_.empty());
    DisplayId displayId = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    EXPECT_EQ(displayId, DEFAULT_DISPLAY_ID);
    EXPECT_TRUE(ssm_->userDefaultDisplayIdCache_.empty());
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_MultipleUsersInCache
 * @tc.desc: Test GetDefaultDisplayIdByUserId with multiple users cached simultaneously
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, GetDefaultDisplayIdByUserId_MultipleUsersInCache, TestSize.Level1)
{
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_100] = TEST_DISPLAY_ID_1;
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_101] = TEST_DISPLAY_ID_2;
    EXPECT_EQ(ssm_->userDefaultDisplayIdCache_.size(), 2u);
    DisplayId id1 = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    DisplayId id2 = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_101);
    EXPECT_EQ(id1, id2);
    EXPECT_EQ(id1, DEFAULT_DISPLAY_ID);
    EXPECT_EQ(id2, DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: InvalidateDefaultDisplayIdCache_ClearsCache
 * @tc.desc: Test InvalidateDefaultDisplayIdCache clears all entries from the cache
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, InvalidateDefaultDisplayIdCache_ClearsCache, TestSize.Level1)
{
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_100] = TEST_DISPLAY_ID_1;
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_101] = TEST_DISPLAY_ID_2;
    EXPECT_EQ(ssm_->userDefaultDisplayIdCache_.size(), 2u);
    ssm_->InvalidateDefaultDisplayIdCache();
    EXPECT_TRUE(ssm_->userDefaultDisplayIdCache_.empty());
}

/**
 * @tc.name: InvalidateDefaultDisplayIdCache_EmptyCache
 * @tc.desc: Test InvalidateDefaultDisplayIdCache on already empty cache does not crash
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, InvalidateDefaultDisplayIdCache_EmptyCache, TestSize.Level1)
{
    EXPECT_TRUE(ssm_->userDefaultDisplayIdCache_.empty());
    ssm_->InvalidateDefaultDisplayIdCache();
    EXPECT_TRUE(ssm_->userDefaultDisplayIdCache_.empty());
}

/**
 * @tc.name: InvalidateDefaultDisplayIdCache_SingleEntry
 * @tc.desc: Test InvalidateDefaultDisplayIdCache with a single cache entry
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, InvalidateDefaultDisplayIdCache_SingleEntry, TestSize.Level1)
{
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_100] = TEST_DISPLAY_ID_1;
    EXPECT_EQ(ssm_->userDefaultDisplayIdCache_.size(), 1u);
    ssm_->InvalidateDefaultDisplayIdCache();
    EXPECT_TRUE(ssm_->userDefaultDisplayIdCache_.empty());
}

/**
 * @tc.name: InvalidateDefaultDisplayIdCache_RepopulateAfterInvalidate
 * @tc.desc: Test that cache can be repopulated after invalidation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, InvalidateDefaultDisplayIdCache_RepopulateAfterInvalidate, TestSize.Level1)
{
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_100] = TEST_DISPLAY_ID_1;
    ssm_->InvalidateDefaultDisplayIdCache();
    EXPECT_TRUE(ssm_->userDefaultDisplayIdCache_.empty());
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_101] = TEST_DISPLAY_ID_2;
    EXPECT_EQ(ssm_->userDefaultDisplayIdCache_.size(), 1u);
    DisplayId displayId = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_101);
    EXPECT_EQ(displayId, DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: ResolveDefaultDisplayId_ExplicitDisplayId
 * @tc.desc: Test ResolveDefaultDisplayId returns the explicit displayId unchanged
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, ResolveDefaultDisplayId_ExplicitDisplayId, TestSize.Level1)
{
    DisplayId explicitId = TEST_DISPLAY_ID_1;
    DisplayId result = ssm_->ResolveDefaultDisplayId(explicitId);
    EXPECT_EQ(result, explicitId);
}

/**
 * @tc.name: ResolveDefaultDisplayId_InvalidDisplayId
 * @tc.desc: Test ResolveDefaultDisplayId with DISPLAY_ID_INVALID when multi-instance is disabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, ResolveDefaultDisplayId_InvalidDisplayId, TestSize.Level1)
{
    DisplayId result = ssm_->ResolveDefaultDisplayId(DISPLAY_ID_INVALID);
    EXPECT_EQ(result, DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: ResolveDefaultDisplayId_ZeroDisplayId
 * @tc.desc: Test ResolveDefaultDisplayId with DEFAULT_DISPLAY_ID (0) is not treated as invalid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, ResolveDefaultDisplayId_ZeroDisplayId, TestSize.Level1)
{
    DisplayId result = ssm_->ResolveDefaultDisplayId(DEFAULT_DISPLAY_ID);
    EXPECT_EQ(result, DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: CacheConsistency_AfterInvalidateAndGetDefaultDisplay
 * @tc.desc: Test that after cache invalidation, GetInstanceUserDefaultDisplay uses the cleared cache state
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, CacheConsistency_AfterInvalidateAndGetDefaultDisplay, TestSize.Level1)
{
    ssm_->currentUserId_ = TEST_USER_ID_100;
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_100] = TEST_DISPLAY_ID_1;
    EXPECT_EQ(ssm_->GetInstanceUserDefaultDisplay(), DEFAULT_DISPLAY_ID);
    ssm_->InvalidateDefaultDisplayIdCache();
    DisplayId result = ssm_->GetInstanceUserDefaultDisplay();
    EXPECT_EQ(result, DEFAULT_DISPLAY_ID);
    ssm_->currentUserId_ = DEFAULT_USERID;
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_DefaultUserIdParam
 * @tc.desc: Test GetDefaultDisplayIdByUserId with default CONCURRENT_USER_ID_DEFAULT parameter
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, GetDefaultDisplayIdByUserId_DefaultUserIdParam, TestSize.Level1)
{
    DisplayId displayId = ssm_->GetDefaultDisplayIdByUserId(DEFAULT_USERID);
    EXPECT_EQ(displayId, DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: GetInstanceUserDefaultDisplay_DefaultAfterUserSwitch
 * @tc.desc: Test GetInstanceUserDefaultDisplay after user switch resets to DEFAULT_USERID
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, GetInstanceUserDefaultDisplay_DefaultAfterUserSwitch, TestSize.Level1)
{
    ssm_->currentUserId_ = TEST_USER_ID_100;
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_100] = TEST_DISPLAY_ID_1;
    EXPECT_EQ(ssm_->GetInstanceUserDefaultDisplay(), DEFAULT_DISPLAY_ID);
    ssm_->currentUserId_ = DEFAULT_USERID;
    ssm_->InvalidateDefaultDisplayIdCache();
    EXPECT_EQ(ssm_->GetInstanceUserDefaultDisplay(), DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: ResolveDefaultDisplayId_VariousExplicitIds
 * @tc.desc: Test ResolveDefaultDisplayId with various explicit displayIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, ResolveDefaultDisplayId_VariousExplicitIds, TestSize.Level1)
{
    DisplayId ids[] = { TEST_DISPLAY_ID_1, TEST_DISPLAY_ID_2, DEFAULT_DISPLAY_ID + 1, 99999 };
    for (DisplayId id : ids) {
        DisplayId result = ssm_->ResolveDefaultDisplayId(id);
        EXPECT_EQ(result, id);
    }
}

/**
 * @tc.name: CacheThreadSafety_ConcurrentInvalidateAndQuery
 * @tc.desc: Test that cache mutex protects concurrent invalidate and query operations
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest15, CacheThreadSafety_ConcurrentInvalidateAndQuery, TestSize.Level1)
{
    ssm_->userDefaultDisplayIdCache_[TEST_USER_ID_100] = TEST_DISPLAY_ID_1;
    std::thread invalidateThread([this]() {
        this->ssm_->InvalidateDefaultDisplayIdCache();
    });
    std::thread queryThread([this]() {
        this->ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    });
    invalidateThread.join();
    queryThread.join();
    SUCCEED();
}
} // namespace Rosen
} // namespace OHOS
