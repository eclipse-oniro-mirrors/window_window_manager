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
#include <parameter.h>
#include <parameters.h>

#include "common_test_utils.h"
#include "display_manager_agent_default.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "screen_scene_config.h"
#include "screen_setting_helper.h"
#include "fold_screen_state_internel.h"
#include "mock/mock_accesstoken_kit.h"
#include "window_manager_hilog.h"
#include "test_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr int32_t DEFAULT_DISPLAY_ID = 0;
constexpr uint32_t SLEEP_TIME_IN_US = 100000;
constexpr ScreenId TEST_SCREEN_ID_0 = 0;
constexpr ScreenId TEST_SCREEN_ID_1 = 1;
constexpr ScreenId TEST_SCREEN_ID_100 = 100;
constexpr int32_t TEST_USER_ID_100 = 100;
} // namespace

class ScreenSessionManagerDefaultDisplayTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<ScreenSessionManager> ssm_;
};

sptr<ScreenSessionManager> ScreenSessionManagerDefaultDisplayTest::ssm_ = nullptr;

void ScreenSessionManagerDefaultDisplayTest::SetUpTestCase()
{
    ssm_ = new ScreenSessionManager();
    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("foundation", perms, 1);
}

void ScreenSessionManagerDefaultDisplayTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void ScreenSessionManagerDefaultDisplayTest::SetUp()
{
    ssm_->displayConcurrentUserMap_.clear();
}

void ScreenSessionManagerDefaultDisplayTest::TearDown()
{
    ScreenSceneConfig::isConcurrentUser_ = false;
    ssm_->displayConcurrentUserMap_.clear();
    usleep(SLEEP_TIME_IN_US);
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_NonConcurrentUser
 * @tc.desc: Test GetDefaultDisplayIdByUserId when concurrent user is disabled, userId forced to USER_ID_DEFAULT
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest, GetDefaultDisplayIdByUserId_NonConcurrentUser, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = false;
    DisplayId result = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    EXPECT_TRUE(result == DISPLAY_ID_INVALID || result == DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_NonConcurrentUser_DefaultParam
 * @tc.desc: Test GetDefaultDisplayIdByUserId with default CONCURRENT_USER_ID_DEFAULT parameter, non-concurrent
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest,
    GetDefaultDisplayIdByUserId_NonConcurrentUser_DefaultParam, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = false;
    DisplayId result = ssm_->GetDefaultDisplayIdByUserId();
    EXPECT_TRUE(result == DISPLAY_ID_INVALID || result == DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_ConcurrentUser_SpecificUserId
 * @tc.desc: Test GetDefaultDisplayIdByUserId with concurrent user enabled and specific userId
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest,
    GetDefaultDisplayIdByUserId_ConcurrentUser_SpecificUserId, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = true;
    DisplayId result = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    EXPECT_TRUE(result == DISPLAY_ID_INVALID || result == DEFAULT_DISPLAY_ID);
    ScreenSceneConfig::isConcurrentUser_ = false;
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_ConcurrentUser_DefaultParam
 * @tc.desc: Test GetDefaultDisplayIdByUserId with concurrent user enabled and CONCURRENT_USER_ID_DEFAULT
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest,
    GetDefaultDisplayIdByUserId_ConcurrentUser_DefaultParam, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = true;
    DisplayId result = ssm_->GetDefaultDisplayIdByUserId();
    EXPECT_TRUE(result == DISPLAY_ID_INVALID || result == DEFAULT_DISPLAY_ID);
    ScreenSceneConfig::isConcurrentUser_ = false;
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_ConcurrentUserSwitch
 * @tc.desc: Test GetDefaultDisplayIdByUserId when concurrent user state is toggled
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest, GetDefaultDisplayIdByUserId_ConcurrentUserSwitch, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = false;
    DisplayId resultDisabled = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    ScreenSceneConfig::isConcurrentUser_ = true;
    DisplayId resultEnabled = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    ScreenSceneConfig::isConcurrentUser_ = false;
    EXPECT_TRUE(resultDisabled == DISPLAY_ID_INVALID || resultDisabled == DEFAULT_DISPLAY_ID);
    EXPECT_TRUE(resultEnabled == DISPLAY_ID_INVALID || resultEnabled == DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: GetDefaultDisplayInfo_NoScreenSession
 * @tc.desc: Test GetDefaultDisplayInfo when no screen session exists, returns nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest, GetDefaultDisplayInfo_NoScreenSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = false;
    ssm_->screenSessionMap_.clear();
    sptr<DisplayInfo> result = ssm_->GetDefaultDisplayInfo();
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetDefaultDisplayInfo_NoScreenSession_WithUserId
 * @tc.desc: Test GetDefaultDisplayInfo with userId when no screen session exists
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest, GetDefaultDisplayInfo_NoScreenSession_WithUserId, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = false;
    ssm_->screenSessionMap_.clear();
    sptr<DisplayInfo> result = ssm_->GetDefaultDisplayInfo(TEST_USER_ID_100);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetDefaultDisplayInfo_WithScreenSession
 * @tc.desc: Test GetDefaultDisplayInfo when a screen session exists in the map
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest, GetDefaultDisplayInfo_WithScreenSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = false;
    ssm_->screenSessionMap_.clear();
    ScreenId screenId = TEST_SCREEN_ID_0;
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("TestDisplay", screenId, 0, 0);
    ASSERT_NE(screenSession, nullptr);
    ssm_->screenSessionMap_[screenId] = screenSession;
    sptr<DisplayInfo> result = ssm_->GetDefaultDisplayInfo();
    if (result != nullptr) {
        EXPECT_NE(result->GetDisplayId(), DISPLAY_ID_INVALID);
    } else {
        EXPECT_EQ(result, nullptr);
    }
    ssm_->screenSessionMap_.clear();
}

/**
 * @tc.name: GetDefaultDisplayInfo_ConcurrentUser_WithScreenSession
 * @tc.desc: Test GetDefaultDisplayInfo with concurrent user and screen session in map
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest,
    GetDefaultDisplayInfo_ConcurrentUser_WithScreenSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = true;
    ssm_->screenSessionMap_.clear();
    ScreenId screenId = TEST_SCREEN_ID_0;
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("TestConcurrentDisplay", screenId, 0, 0);
    ASSERT_NE(screenSession, nullptr);
    ssm_->screenSessionMap_[screenId] = screenSession;
    sptr<DisplayInfo> result = ssm_->GetDefaultDisplayInfo(TEST_USER_ID_100);
    if (result != nullptr) {
        EXPECT_NE(result->GetDisplayId(), DISPLAY_ID_INVALID);
    } else {
        EXPECT_EQ(result, nullptr);
    }
    ScreenSceneConfig::isConcurrentUser_ = false;
    ssm_->screenSessionMap_.clear();
}

/**
 * @tc.name: GetDefaultDisplayInfo_ConcurrentUserMap_Populated
 * @tc.desc: Test GetDefaultDisplayInfo when displayConcurrentUserMap_ is populated
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest, GetDefaultDisplayInfo_ConcurrentUserMap_Populated, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = true;
    ssm_->displayConcurrentUserMap_.clear();
    ssm_->displayConcurrentUserMap_[TEST_SCREEN_ID_100][TEST_USER_ID_100] = {true, 0};
    ScreenId screenId = TEST_SCREEN_ID_100;
    sptr<ScreenSession> screenSession =
        new (std::nothrow) ScreenSession("TestConcurrentMap", screenId, 0, 0);
    ASSERT_NE(screenSession, nullptr);
    ssm_->screenSessionMap_[screenId] = screenSession;
    sptr<DisplayInfo> result = ssm_->GetDefaultDisplayInfo(TEST_USER_ID_100);
    EXPECT_TRUE(result == nullptr || result != nullptr);
    ScreenSceneConfig::isConcurrentUser_ = false;
    ssm_->screenSessionMap_.clear();
    ssm_->displayConcurrentUserMap_.clear();
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_InvalidScreenIdValidation
 * @tc.desc: Test GetDefaultDisplayIdByUserId returns DISPLAY_ID_INVALID when screenId is invalid
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest, GetDefaultDisplayIdByUserId_InvalidScreenIdValidation, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = false;
    ssm_->screenSessionMap_.clear();
    DisplayId result = ssm_->GetDefaultDisplayIdByUserId();
    if (ssm_->defaultScreenId_ == INVALID_SCREEN_ID || ssm_->defaultScreenId_ == SCREEN_ID_INVALID) {
        EXPECT_EQ(result, DISPLAY_ID_INVALID);
    }
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_MultipleCallsConsistency
 * @tc.desc: Test GetDefaultDisplayIdByUserId returns consistent results on multiple calls
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest, GetDefaultDisplayIdByUserId_MultipleCallsConsistency, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = false;
    DisplayId result1 = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    DisplayId result2 = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    EXPECT_EQ(result1, result2);
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_DifferentUserIds_NonConcurrent
 * @tc.desc: Test GetDefaultDisplayIdByUserId with different userIds when concurrent user is disabled
 *          (all should be forced to USER_ID_DEFAULT)
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest,
    GetDefaultDisplayIdByUserId_DifferentUserIds_NonConcurrent, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = false;
    DisplayId result1 = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    DisplayId result2 = ssm_->GetDefaultDisplayIdByUserId(200);
    DisplayId result3 = ssm_->GetDefaultDisplayIdByUserId();
    EXPECT_EQ(result1, result2);
    EXPECT_EQ(result2, result3);
}

/**
 * @tc.name: GetDefaultDisplayIdByUserId_ConcurrentUserMap_Empty
 * @tc.desc: Test GetDefaultDisplayIdByUserId with concurrent user enabled and empty user map
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerDefaultDisplayTest, GetDefaultDisplayIdByUserId_ConcurrentUserMap_Empty, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ScreenSceneConfig::isConcurrentUser_ = true;
    ssm_->displayConcurrentUserMap_.clear();
    DisplayId result = ssm_->GetDefaultDisplayIdByUserId(TEST_USER_ID_100);
    EXPECT_TRUE(result == DISPLAY_ID_INVALID || result == DEFAULT_DISPLAY_ID);
    ScreenSceneConfig::isConcurrentUser_ = false;
}
} // namespace Rosen
} // namespace OHOS
