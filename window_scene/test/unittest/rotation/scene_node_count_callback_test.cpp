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

#include "mock/mock_session_stage.h"
#include "session/host/include/scene_node_count_callback.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SceneNodeCountCallbackTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneNodeCountCallbackTest::SetUpTestCase() {}

void SceneNodeCountCallbackTest::TearDownTestCase() {}

void SceneNodeCountCallbackTest::SetUp() {}

void SceneNodeCountCallbackTest::TearDown() {}

/**
 * @tc.name: SceneNodeCountCallback_OnSceneNodeCount
 * @tc.desc: Test SceneNodeCountCallback::OnSceneNodeCount with various node counts
 * @tc.type: FUNC
 */
HWTEST_F(SceneNodeCountCallbackTest, OnSceneNodeCount, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneNodeCountCallbackTest: OnSceneNodeCount start";

    auto callback = sptr<SceneNodeCountCallback>::MakeSptr();

    // Case 1: Node count is zero
    callback->ResetResult();
    callback->OnSceneNodeCount(0);
    uint32_t result = callback->GetResult(1000);
    EXPECT_EQ(0u, result);

    // Case 2: Node count is positive
    callback->ResetResult();
    uint32_t testCount = 100;
    callback->OnSceneNodeCount(testCount);
    result = callback->GetResult(1000);
    EXPECT_EQ(testCount, result);

    // Case 3: Large node count
    callback->ResetResult();
    testCount = 10000;
    callback->OnSceneNodeCount(testCount);
    result = callback->GetResult(1000);
    EXPECT_EQ(testCount, result);

    GTEST_LOG_(INFO) << "SceneNodeCountCallbackTest: OnSceneNodeCount end";
}

/**
 * @tc.name: SceneNodeCountCallback_GetResult
 * @tc.desc: Test SceneNodeCountCallback::GetResult with timeout scenarios
 * @tc.type: FUNC
 */
HWTEST_F(SceneNodeCountCallbackTest, GetResult, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneNodeCountCallbackTest: GetResult start";

    auto callback = sptr<SceneNodeCountCallback>::MakeSptr();

    // Case 1: Get result before timeout - should return value immediately
    callback->ResetResult();
    callback->OnSceneNodeCount(42);
    uint32_t result = callback->GetResult(1000);
    EXPECT_EQ(42u, result);

    // Case 2: Get result after timeout - should handle timeout gracefully
    callback->ResetResult();
    // Don't call OnSceneNodeCount, simulate timeout scenario
    result = callback->GetResult(100);
    EXPECT_EQ(0u, result);

    GTEST_LOG_(INFO) << "SceneNodeCountCallbackTest: GetResult end";
}

/**
 * @tc.name: SceneNodeCountCallback_ResetResult
 * @tc.desc: Test SceneNodeCountCallback::ResetResult
 * @tc.type: FUNC
 */
HWTEST_F(SceneNodeCountCallbackTest, ResetResult, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneNodeCountCallbackTest: ResetResult start";

    auto callback = sptr<SceneNodeCountCallback>::MakeSptr();

    // Set initial value
    callback->OnSceneNodeCount(100);
    uint32_t result = callback->GetResult(1000);
    EXPECT_EQ(100u, result);

    // Reset and verify value is cleared
    callback->ResetResult();
    result = callback->GetResult(100);
    EXPECT_EQ(0u, result);

    // Set new value after reset
    callback->OnSceneNodeCount(200);
    result = callback->GetResult(1000);
    EXPECT_EQ(200u, result);

    GTEST_LOG_(INFO) << "SceneNodeCountCallbackTest: ResetResult end";
}
} // namespace Rosen
} // namespace OHOS
