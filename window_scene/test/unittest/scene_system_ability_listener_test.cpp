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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "scene_system_ability_listener.h"
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#include "system_ability_definition.h"
#endif
using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class SceneSystemAbilityListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSystemAbilityListenerTest::SetUpTestCase() {}

void SceneSystemAbilityListenerTest::TearDownTestCase() {}

void SceneSystemAbilityListenerTest::SetUp() {}

void SceneSystemAbilityListenerTest::TearDown() {}

HWTEST_F(SceneSystemAbilityListenerTest, OnAddSystemAbilityTest, TestSize.Level1)
{
    std::string scbUid = std::to_string(12);
    std::string scbPid = std::to_string(34);
    std::string scbTid = std::to_string(56);
    std::string ssmTid = std::to_string(78);
    std::string scbBundleName = "scb";
    std::string ssmThreadName = "ssm";
    SCBThreadInfo threadInfo = { .scbUid_ = scbUid,
                                 .scbPid_ = scbPid,
                                 .scbTid_ = scbTid,
                                 .ssmTid_ = ssmTid,
                                 .scbBundleName_ = scbBundleName,
                                 .ssmThreadName_ = ssmThreadName };
    SceneSystemAbilityListener listener(threadInfo);
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    ResourceSchedule::ResSchedClient::GetInstance().rss_ = nullptr;
    listener.OnAddSystemAbility(RENDER_SERVICE, "");
    EXPECT_EQ(ResourceSchedule::ResSchedClient::GetInstance().rss_, nullptr);
    listener.OnAddSystemAbility(RES_SCHED_SYS_ABILITY_ID, "");
    EXPECT_NE(ResourceSchedule::ResSchedClient::GetInstance().rss_, nullptr);
#else
    EXPECT_EQ(listener.info_, threadInfo);
#endif

#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
    listener.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
    EXPECT_EQ(listener.info_, threadInfo);
#endif
}

HWTEST_F(SceneSystemAbilityListenerTest, OnRemoveSystemAbility, TestSize.Level1)
{
#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
    listener.OnRemoveSystemAbility(POWER_MANAGER_SERVICE_ID, "");
    EXPECT_EQ(listener.info_, threadInfo);
#endif
}
} // namespace Rosen
} // namespace OHOS