/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under Apache License, Version 2.0 (the "License");
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

#include "iremote_object_mocker.h"
#include "mock/mock_message_parcel.h"
#include "session/host/include/zidl/scene_node_count_callback_proxy.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SceneNodeCountCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneNodeCountCallbackProxyTest::SetUpTestCase() {}

void SceneNodeCountCallbackProxyTest::TearDownTestCase() {}

void SceneNodeCountCallbackProxyTest::SetUp() {}

void SceneNodeCountCallbackProxyTest::TearDown() {}

/**
 * @tc.name: OnSceneNodeCount_Success
 * @tc.desc: Test OnSceneNodeCount with normal flow
 * @tc.type: FUNC
 */
HWTEST_F(SceneNodeCountCallbackProxyTest, OnSceneNodeCount_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneNodeCountCallbackProxyTest: OnSceneNodeCount_Success start";
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneNodeCountCallbackProxy> callbackProxy =
        sptr<SceneNodeCountCallbackProxy>::MakeSptr(remoteMocker);

    // Set remote to return success
    remoteMocker->SetRequestResult(ERR_NONE);

    uint32_t testNodeCount = 42;
    callbackProxy->OnSceneNodeCount(testNodeCount);

    // Verify success (no return value means success)
    GTEST_LOG_(INFO) << "SceneNodeCountCallbackProxyTest: OnSceneNodeCount_Success end";
}

/**
 * @tc.name: OnSceneNodeCount
 * @tc.desc: test function : OnSceneNodeCount
 * @tc.type: FUNC
 */
HWTEST_F(SceneNodeCountCallbackProxyTest, OnSceneNodeCount, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneNodeCountCallbackProxy> sceneNodeCountCallbackProxy
        = sptr<SceneNodeCountCallbackProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(sceneNodeCountCallbackProxy, nullptr);

    // Case 1: Failed to write interface token
    uint32_t testNodeCount = 42;
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sceneNodeCountCallbackProxy->OnSceneNodeCount(testNodeCount);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Case 2: Failed to write nodeCount
    MockMessageParcel::SetReadUint32ErrorFlag(true);
    sceneNodeCountCallbackProxy->OnSceneNodeCount(testNodeCount);
    MockMessageParcel::SetReadUint32ErrorFlag(false);

    // Case 3: remote is nullptr
    sptr<SceneNodeCountCallbackProxy> nullProxy = sptr<SceneNodeCountCallbackProxy>::MakeSptr(nullptr);
    nullProxy->OnSceneNodeCount(testNodeCount);

    // Case 4: Failed to send request
    remoteMocker->SetRequestResult(1);
    sceneNodeCountCallbackProxy->OnSceneNodeCount(testNodeCount);
    remoteMocker->SetRequestResult(0);

    MockMessageParcel::ClearAllErrorFlag();
    GTEST_LOG_(INFO) << "SessionStageProxyRotationTest: GetSceneNodeCount_Callback end";
}
} // namespace Rosen
} // namespace OHOS
