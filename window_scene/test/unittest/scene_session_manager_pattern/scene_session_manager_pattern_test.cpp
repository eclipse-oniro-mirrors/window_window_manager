/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <regex>

#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "window_manager_agent.h"
#include "session_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
 
class SceneSessionManagerPatternTest : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp() override;

    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;


private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerPatternTest::ssm_ = nullptr;

void SceneSessionManagerPatternTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerPatternTest::TearDownTestCase()
{
    ssm_->sceneSessionMap_.clear();
    ssm_ = nullptr;
}

void SceneSessionManagerPatternTest::SetUp()
{
    ssm_->sceneSessionMap_.clear();

}

void SceneSessionManagerPatternTest::TearDown()
{
    ssm_->sceneSessionMap_.clear();
    usleep(WAIT_SYNC_IN_NS);
}

namespace {

/**
 * @tc.name: VisitSnapshotFromCache
 * @tc.desc: VisitSnapshotFromCache
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerPatternTest, VisitSnapshotFromCache, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.persistentId_ = 30;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    std::string bundleName = "testBundleName";
    int32_t persistentId = 30;
    sceneSession->scenePersistence_ = sptr<ScenePersistence>::MakeSptr(bundleName, persistentId);
    ssm_->sceneSessionMap_.insert({ 30, sceneSession });
    sceneSession->snapshot_ = std::make_shared<Media::PixelMap>();
    for (int32_t id = 30; id < 30 + ssm_->snapshotCapacity_; ++id) {
        ssm_->PutSnapshotToCache(id);
    }
    ssm_->PutSnapshotToCache(30 + ssm_->snapshotCapacity_);
    ssm_->VisitSnapshotFromCache(30);
    ASSERT_EQ(sceneSession->snapshot_, nullptr);
}

} // namespace
} // namespace Rosen
} // namespace OHOS

