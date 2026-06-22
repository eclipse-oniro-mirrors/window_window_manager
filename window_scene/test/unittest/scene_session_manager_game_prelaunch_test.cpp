/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "interfaces/include/ws_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class SceneSessionManagerGamePrelaunchTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionManagerGamePrelaunchTest::SetUpTestCase() {}

void SceneSessionManagerGamePrelaunchTest::TearDownTestCase() {}

void SceneSessionManagerGamePrelaunchTest::SetUp() {}

void SceneSessionManagerGamePrelaunchTest::TearDown() {}

namespace {
/**
 * @tc.name: SessionInfoWithGamePrelaunch01
 * @tc.desc: Test SessionInfo with isGamePrelaunch_ set to true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerGamePrelaunchTest, SessionInfoWithGamePrelaunch01, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "TestBundle";
    sessionInfo.abilityName_ = "TestAbility";
    sessionInfo.isGamePrelaunch_ = true;
    
    EXPECT_TRUE(sessionInfo.isGamePrelaunch_);
    EXPECT_EQ(sessionInfo.bundleName_, "TestBundle");
    EXPECT_EQ(sessionInfo.abilityName_, "TestAbility");
}

/**
 * @tc.name: SessionInfoWithGamePrelaunch02
 * @tc.desc: Test SessionInfo with isGamePrelaunch_ default value
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerGamePrelaunchTest, SessionInfoWithGamePrelaunch02, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "TestBundle";
    sessionInfo.abilityName_ = "TestAbility";
    
    EXPECT_FALSE(sessionInfo.isGamePrelaunch_);
}

/**
 * @tc.name: SessionInfoCopyWithGamePrelaunch
 * @tc.desc: Test SessionInfo copy constructor with isGamePrelaunch_
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerGamePrelaunchTest, SessionInfoCopyWithGamePrelaunch, TestSize.Level0)
{
    SessionInfo originalInfo;
    originalInfo.bundleName_ = "TestBundle";
    originalInfo.abilityName_ = "TestAbility";
    originalInfo.isGamePrelaunch_ = true;
    
    SessionInfo copiedInfo = originalInfo;
    
    EXPECT_TRUE(copiedInfo.isGamePrelaunch_);
    EXPECT_EQ(copiedInfo.bundleName_, originalInfo.bundleName_);
    EXPECT_EQ(copiedInfo.abilityName_, originalInfo.abilityName_);
}

/**
 * @tc.name: SessionInfoSetGamePrelaunch
 * @tc.desc: Test setting isGamePrelaunch_ to different values
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerGamePrelaunchTest, SessionInfoSetGamePrelaunch, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "TestBundle";
    sessionInfo.abilityName_ = "TestAbility";
    
    sessionInfo.isGamePrelaunch_ = true;
    EXPECT_TRUE(sessionInfo.isGamePrelaunch_);
    
    sessionInfo.isGamePrelaunch_ = false;
    EXPECT_FALSE(sessionInfo.isGamePrelaunch_);
    
    sessionInfo.isGamePrelaunch_ = true;
    EXPECT_TRUE(sessionInfo.isGamePrelaunch_);
}

/**
 * @tc.name: LifeCycleChangeReasonEnumValues
 * @tc.desc: Test LifeCycleChangeReason enum values
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerGamePrelaunchTest, LifeCycleChangeReasonEnumValues, TestSize.Level0)
{
    EXPECT_EQ(static_cast<int32_t>(LifeCycleChangeReason::DEFAULT), 0);
    
    int32_t quickBatchBackgroundValue = static_cast<int32_t>(LifeCycleChangeReason::QUICK_BATCH_BACKGROUND);
    int32_t gamePrelaunchBackgroundValue = static_cast<int32_t>(LifeCycleChangeReason::GAME_PRELAUNCH_BACKGROUND);
    
    EXPECT_EQ(gamePrelaunchBackgroundValue, quickBatchBackgroundValue + 1);
}

/**
 * @tc.name: SessionInfoAssignmentWithGamePrelaunch
 * @tc.desc: Test SessionInfo assignment operator with isGamePrelaunch_
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerGamePrelaunchTest, SessionInfoAssignmentWithGamePrelaunch, TestSize.Level0)
{
    SessionInfo originalInfo;
    originalInfo.bundleName_ = "TestBundle";
    originalInfo.abilityName_ = "TestAbility";
    originalInfo.isGamePrelaunch_ = true;
    
    SessionInfo assignedInfo;
    assignedInfo = originalInfo;
    
    EXPECT_TRUE(assignedInfo.isGamePrelaunch_);
    EXPECT_EQ(assignedInfo.bundleName_, originalInfo.bundleName_);
    EXPECT_EQ(assignedInfo.abilityName_, originalInfo.abilityName_);
}

/**
 * @tc.name: SessionInfoMultipleInstancesWithGamePrelaunch
 * @tc.desc: Test multiple SessionInfo instances with different isGamePrelaunch_ values
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerGamePrelaunchTest, SessionInfoMultipleInstancesWithGamePrelaunch, TestSize.Level0)
{
    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "Bundle1";
    sessionInfo1.abilityName_ = "Ability1";
    sessionInfo1.isGamePrelaunch_ = true;
    
    SessionInfo sessionInfo2;
    sessionInfo2.bundleName_ = "Bundle2";
    sessionInfo2.abilityName_ = "Ability2";
    sessionInfo2.isGamePrelaunch_ = false;
    
    EXPECT_TRUE(sessionInfo1.isGamePrelaunch_);
    EXPECT_FALSE(sessionInfo2.isGamePrelaunch_);
    
    EXPECT_EQ(sessionInfo1.bundleName_, "Bundle1");
    EXPECT_EQ(sessionInfo2.bundleName_, "Bundle2");
}

/**
 * @tc.name: SessionInfoGamePrelaunchToggle
 * @tc.desc: Test toggling isGamePrelaunch_ value
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerGamePrelaunchTest, SessionInfoGamePrelaunchToggle, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "TestBundle";
    sessionInfo.abilityName_ = "TestAbility";
    
    EXPECT_FALSE(sessionInfo.isGamePrelaunch_);
    
    sessionInfo.isGamePrelaunch_ = true;
    EXPECT_TRUE(sessionInfo.isGamePrelaunch_);
    
    sessionInfo.isGamePrelaunch_ = false;
    EXPECT_FALSE(sessionInfo.isGamePrelaunch_);
    
    sessionInfo.isGamePrelaunch_ = true;
    EXPECT_TRUE(sessionInfo.isGamePrelaunch_);
}

/**
 * @tc.name: SessionInfoGamePrelaunchWithOtherFields
 * @tc.cdesc: Test isGamePrelaunch_ with other SessionInfo fields
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerGamePrelaunchTest, SessionInfoGamePrelaunchWithOtherFields, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "TestBundle";
    sessionInfo.abilityName_ = "TestAbility";
    sessionInfo.moduleName_ = "TestModule";
    sessionInfo.persistentId_ = 12345;
    sessionInfo.isGamePrelaunch_ = true;
    
    EXPECT_TRUE(sessionInfo.isGamePrelaunch_);
    EXPECT_EQ(sessionInfo.bundleName_, "TestBundle");
    EXPECT_EQ(sessionInfo.abilityName_, "TestAbility");
    EXPECT_EQ(sessionInfo.moduleName_, "TestModule");
    EXPECT_EQ(sessionInfo.persistentId_, 12345);
}
}
}
}
