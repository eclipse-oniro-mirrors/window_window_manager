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
 
#include "get_snapshot_callback_proxy.h"
#include "hilog/log.h"
#include "iremote_object_mocker.h"
#include "../../../window_scene/test/mock/mock_message_parcel.h"
 
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
class GetSnapshotCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::vector<std::shared_ptr<OHOS::Media::PixelMap>> pixelMaps_;
};
 
void GetSnapshotCallbackProxyTest::SetUpTestCase()
{
}
 
void GetSnapshotCallbackProxyTest::TearDownTestCase()
{
}
 
void GetSnapshotCallbackProxyTest::SetUp()
{
    // create pixelMap
    const uint32_t colors[1] = { 0x6f0000ff };
    uint32_t colorsLength = sizeof(colors) / sizeof(colors[0]);
    const int32_t offset = 0;
    Media::InitializationOptions opts;
    opts.size.width = 200;  // 200 test width
    opts.size.height = 300; // 300 test height
    opts.pixelFormat = Media::PixelFormat::RGBA_8888;
    opts.alphaType = Media::AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    int32_t stride = opts.size.width;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(colors, colorsLength, offset, stride, opts);
    pixelMaps_.emplace_back(nullptr);
    pixelMaps_.emplace_back(pixelMap);
}
 
void GetSnapshotCallbackProxyTest::TearDown()
{
}
 
namespace {
/**
 * @tc.name: OnReceived
 * @tc.desc: OnReceived Test
 * @tc.type: FUNC
 */
HWTEST_F(GetSnapshotCallbackProxyTest, OnReceived, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<MockIRemoteObject> iRemoteObjectMocker = nullptr;
    sptr<GetSnapshotCallbackProxy> sProxy = sptr<GetSnapshotCallbackProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);

    WMError errCode = WMError::WM_OK;
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    sProxy->OnReceived(errCode, pixelMaps_);
    iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sProxy = sptr<GetSnapshotCallbackProxy>::MakeSptr(iRemoteObjectMocker);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sProxy->OnReceived(errCode, pixelMaps_);
    EXPECT_TRUE(logMsg.find("Write interface token failed") != std::string::npos);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    sProxy->OnReceived(errCode, pixelMaps_);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: OnReceived01
 * @tc.desc: OnReceived01 Test
 * @tc.type: FUNC
 */
HWTEST_F(GetSnapshotCallbackProxyTest, OnReceived01, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<GetSnapshotCallbackProxy> sProxy = sptr<GetSnapshotCallbackProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);

    WMError errCode = WMError::WM_OK;
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    iRemoteObjectMocker->SetRequestResult(ERR_INVALID_DATA);
    sProxy->OnReceived(errCode, pixelMaps_);
    EXPECT_TRUE(logMsg.find("SendRequest failed") != std::string::npos);
    iRemoteObjectMocker->SetRequestResult(ERR_NONE);
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    sProxy->OnReceived(errCode, pixelMaps_);
    EXPECT_TRUE(logMsg.find("write error code failed") != std::string::npos);
    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: WritePixelMapData
 * @tc.desc: WritePixelMapData Test
 * @tc.type: FUNC
 */
HWTEST_F(GetSnapshotCallbackProxyTest, WritePixelMapData, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<GetSnapshotCallbackProxy> sProxy = sptr<GetSnapshotCallbackProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);

    WMError errCode = WMError::WM_OK;
    sProxy->OnReceived(errCode, pixelMaps_);
    EXPECT_TRUE(logMsg.find("write pixelMap failed") == std::string::npos);
    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
    LOG_SetCallback(nullptr);
}
}
} // namespace Rosen
} // namespace OHOS