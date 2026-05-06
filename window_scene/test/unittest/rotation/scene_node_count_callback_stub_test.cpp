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

#include "session/host/include/zidl/scene_node_count_callback_stub.h"
#include "session/host/include/scene_node_count_callback.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SceneNodeCountCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneNodeCountCallbackStubTest::SetUpTestCase() {}

void SceneNodeCountCallbackStubTest::TearDownTestCase() {}

void SceneNodeCountCallbackStubTest::SetUp() {}

void SceneNodeCountCallbackStubTest::TearDown() {}

/**
 * @tc.name: OnRemoteRequest_Success
 * @tc.desc: Test OnRemoteRequest with normal flow for TRANS_ON_SCENE_NODE_COUNT
 * @tc.type: FUNC
 */
HWTEST_F(SceneNodeCountCallbackStubTest, OnRemoteRequest_Success, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneNodeCountCallbackStubTest: OnRemoteRequest_Success start";
    sptr<SceneNodeCountCallback> stub = sptr<SceneNodeCountCallback>::MakeSptr();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneNodeCountCallbackStub::GetDescriptor());
    uint32_t nodeCount = 42;
    data.WriteUint32(nodeCount);

    int result = stub->OnRemoteRequest(
        static_cast<uint32_t>(ISceneNodeCountCallback::SceneNodeCountCallbackMessage::TRANS_ON_SCENE_NODE_COUNT),
        data, reply, option);
    EXPECT_EQ(ERR_NONE, result);

    GTEST_LOG_(INFO) << "SceneNodeCountCallbackStubTest: OnRemoteRequest_Success end";
}

/**
 * @tc.name: OnRemoteRequest_InvalidCode
 * @tc.desc: Test OnRemoteRequest with unknown message code
 * @tc.type: FUNC
 */
HWTEST_F(SceneNodeCountCallbackStubTest, OnRemoteRequest_InvalidCode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneNodeCountCallbackStubTest: OnRemoteRequest_InvalidCode start";
    sptr<SceneNodeCountCallback> stub = sptr<SceneNodeCountCallback>::MakeSptr();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneNodeCountCallbackStub::GetDescriptor());
    uint32_t invalidCode = 9999;
    data.WriteUint32(invalidCode);

    // Should delegate to parent class for unknown codes
    int result = stub->OnRemoteRequest(
        static_cast<uint32_t>(ISceneNodeCountCallback::SceneNodeCountCallbackMessage::TRANS_ON_SCENE_NODE_INVALID),
        data, reply, option);
    EXPECT_NE(ERR_NONE, result);

    GTEST_LOG_(INFO) << "SceneNodeCountCallbackStubTest: OnRemoteRequest_InvalidCode end";
}

/**
 * @tc.name: OnRemoteRequest_InterfaceTokenMismatch
 * @tc.desc: Test OnRemoteRequest when interface token doesn't match
 * @tc.type: FUNC
 */
HWTEST_F(SceneNodeCountCallbackStubTest, OnRemoteRequest_InterfaceTokenMismatch, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneNodeCountCallbackStubTest: OnRemoteRequest_InterfaceTokenMismatch start";
    sptr<SceneNodeCountCallback> stub = sptr<SceneNodeCountCallback>::MakeSptr();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    // Write invalid interface token
    data.WriteInterfaceToken(u"OHOS.ISceneNodeCountCallback.INVALID");

    int result = stub->OnRemoteRequest(
        static_cast<uint32_t>(ISceneNodeCountCallback::SceneNodeCountCallbackMessage::TRANS_ON_SCENE_NODE_COUNT),
        data, reply, option);

    EXPECT_EQ(ERR_TRANSACTION_FAILED, result);

    GTEST_LOG_(INFO) << "SceneNodeCountCallbackStubTest: OnRemoteRequest_InterfaceTokenMismatch end";
}

/**
 * @tc.name: OnRemoteRequest_ReadUint32Failed
 * @tc.desc: Test OnRemoteRequest when reading uint32 fails
 * @tc.type: FUNC
 */
HWTEST_F(SceneNodeCountCallbackStubTest, OnRemoteRequest_ReadUint32Failed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneNodeCountCallbackStubTest: OnRemoteRequest_ReadUint32Failed start";
    sptr<SceneNodeCountCallback> stub = sptr<SceneNodeCountCallback>::MakeSptr();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SceneNodeCountCallbackStub::GetDescriptor());
    // Don't write Uint32, causing ReadUint32 to fail

    int result = stub->OnRemoteRequest(
        static_cast<uint32_t>(ISceneNodeCountCallback::SceneNodeCountCallbackMessage::TRANS_ON_SCENE_NODE_COUNT),
        data, reply, option);

    EXPECT_EQ(ERR_INVALID_DATA, result);

    GTEST_LOG_(INFO) << "SceneNodeCountCallbackStubTest: OnRemoteRequest_ReadUint32Failed end";
}
} // namespace Rosen
} // namespace OHOS
