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
#include "hilog/log.h"
#include "get_snapshot_callback.h"
#include "wm_common.h"
 
using namespace testing;
using namespace testing::ext;
namespace {
    std::string logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        logMsg += msg;
    }
}
namespace OHOS {
namespace Rosen {
class GetSnapshotCallbackTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
 
void GetSnapshotCallbackTest::SetUpTestCase()
{
}
 
void GetSnapshotCallbackTest::TearDownTestCase()
{
}
 
void GetSnapshotCallbackTest::SetUp()
{
}
 
void GetSnapshotCallbackTest::TearDown()
{
}
 
namespace {
/**
 * @tc.name: OnReceived
 * @tc.desc: OnReceived Test
 * @tc.type: FUNC
 */
HWTEST_F(GetSnapshotCallbackTest, OnReceived, TestSize.Level1)
{
    sptr<GetSnapshotCallback> getStub = sptr<GetSnapshotCallback>::MakeSptr();
    int32_t testId= -1;
    int32_t testNum = 10;
    std::vector<std::shared_ptr<OHOS::Media::PixelMap>> pixelMaps;
    WMError errCode = WMError::WM_OK;
    getStub->OnReceived(errCode, pixelMaps);
    NotifyGetMainWindowSnapshotFunc func = [&](
        WMError errCode, std::vector<std::shared_ptr<OHOS::Media::PixelMap>> pixelMaps)
    {
        testId = testNum + testId;
    };

    getStub->RegisterFunc(std::move(func));
    getStub->OnReceived(errCode, pixelMaps);
    EXPECT_EQ(testId, 9);
}

/**
 * @tc.name: OnRemoteRequest
 * @tc.desc: OnRemoteRequest Test
 * @tc.type: FUNC
 */
HWTEST_F(GetSnapshotCallbackTest, OnRemoteRequest, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<GetSnapshotCallback> getStub = sptr<GetSnapshotCallback>::MakeSptr();
    data.WriteInterfaceToken(u"error.GetDescriptor");
    uint32_t code = 1;
    EXPECT_EQ(getStub->OnRemoteRequest(code, data, reply, option), ERR_INVALID_STATE);
    data.WriteInterfaceToken(GetSnapshotCallback::GetDescriptor());
    EXPECT_EQ(getStub->OnRemoteRequest(code, data, reply, option), ERR_NONE);
}

/**
 * @tc.name: HandleOnReceived
 * @tc.desc: HandleOnReceived Test
 * @tc.type: FUNC
 */
HWTEST_F(GetSnapshotCallbackTest, HandleOnReceived, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<GetSnapshotCallback> getStub = sptr<GetSnapshotCallback>::MakeSptr();
    EXPECT_EQ(getStub->HandleOnReceived(data, reply), ERR_NONE);
    data.WriteInt32(0);
    EXPECT_EQ(getStub->HandleOnReceived(data, reply), ERR_NONE);
    data.WriteInt32(0);
    data.WriteInt32(-1);
    EXPECT_EQ(getStub->HandleOnReceived(data, reply), ERR_NONE);
    data.WriteInt32(0);
    data.WriteInt32(1);
    EXPECT_EQ(getStub->HandleOnReceived(data, reply), ERR_NONE);
}

/**
 * @tc.name: HandleOnReceived01
 * @tc.desc: HandleOnReceived01 Test
 * @tc.type: FUNC
 */
HWTEST_F(GetSnapshotCallbackTest, HandleOnReceived01, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MessageParcel data;
    MessageParcel reply;
    sptr<GetSnapshotCallback> getStub = sptr<GetSnapshotCallback>::MakeSptr();
    EXPECT_EQ(getStub->HandleOnReceived(data, reply), ERR_NONE);

    data.WriteInt32(0);
    data.WriteInt32(1);
    getStub->HandleOnReceived(data, reply);
    EXPECT_TRUE(logMsg.find("read nullptrLen failed") != std::string::npos);
    logMsg.clear();
    data.WriteInt32(0);
    data.WriteInt32(2);
    data.WriteInt32(1);
    getStub->HandleOnReceived(data, reply);
    EXPECT_TRUE(logMsg.find("read nullptrLen failed") == std::string::npos);
    EXPECT_TRUE(logMsg.find("pixelMaps size") != std::string::npos);
    LOG_SetCallback(nullptr);
}
}
} // namespace Rosen
} // namespace OHOS