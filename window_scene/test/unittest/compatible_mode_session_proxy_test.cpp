/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "iremote_object_mocker.h"
#include "mock_message_parcel.h"
#include "session_proxy.h"
#include "ws_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class CompatibleModeSessionProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
};

void CompatibleModeSessionProxyTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "setup test case";
    MockMessageParcel::ClearAllErrorFlag();
}

void CompatibleModeSessionProxyTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "teardown test case";
}

void CompatibleModeSessionProxyTest::SetUp()
{
    GTEST_LOG_(INFO) << "setup";
}

void CompatibleModeSessionProxyTest::TearDown()
{
    GTEST_LOG_(INFO) << "teardown";
    MockMessageParcel::ClearAllErrorFlag();
}

namespace {
/**
 * @tc.name: NotifyCompatibleModeChange
 * @tc.desc: NotifyCompatibleModeChangeWithWriteTokenFail
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeSessionProxyTest, NotifyCompatibleModeChangeWithWriteTokenFail, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithWriteTokenFail test start";
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    CompatibleStyleMode mode = CompatibleStyleMode::LANDSCAPE_DEFAULT;
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    auto ret = sessionProxy->NotifyCompatibleModeChange(mode);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithWriteTokenFail test end";
}

/**
 * @tc.name: NotifyCompatibleModeChange
 * @tc.desc: NotifyCompatibleModeChangeWithWriteModeFail
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeSessionProxyTest, NotifyCompatibleModeChangeWithWriteModeFail, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithWriteModeFail test start";
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    CompatibleStyleMode mode = CompatibleStyleMode::LANDSCAPE_18_9;
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    auto ret = sessionProxy->NotifyCompatibleModeChange(mode);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithWriteModeFail test end";
}

/**
 * @tc.name: NotifyCompatibleModeChange
 * @tc.desc: NotifyCompatibleModeChangeWithNullRemote
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeSessionProxyTest, NotifyCompatibleModeChangeWithNullRemote, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithNullRemote test start";
    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    CompatibleStyleMode mode = CompatibleStyleMode::LANDSCAPE_18_9;
    auto ret = nullProxy->NotifyCompatibleModeChange(mode);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithNullRemote test end";
}

/**
 * @tc.name: NotifyCompatibleModeChange
 * @tc.desc: NotifyCompatibleModeChangeWithFailSendRequest
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeSessionProxyTest, NotifyCompatibleModeChangeWithFailSendRequest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithFailSendRequest test start";
    CompatibleStyleMode mode = CompatibleStyleMode::LANDSCAPE_18_9;
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    mockRemote->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionProxy> failProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    auto ret = failProxy->NotifyCompatibleModeChange(mode);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithFailSendRequest test end";
}

/**
 * @tc.name: NotifyCompatibleModeChange
 * @tc.desc: NotifyCompatibleModeChangeWithReturnOK
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeSessionProxyTest, NotifyCompatibleModeChangeWithReturnOK, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithReturnOK test start";
    auto mockRemote = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(mockRemote);
    CompatibleStyleMode mode = CompatibleStyleMode::LANDSCAPE_18_9;
    auto ret = sessionProxy->NotifyCompatibleModeChange(mode);
    EXPECT_EQ(WSError::WS_OK, ret);
    GTEST_LOG_(INFO) << "NotifyCompatibleModeChangeWithReturnOK test end";
}
} // namespace
} // namespace Rosen
} // namespace OHOS