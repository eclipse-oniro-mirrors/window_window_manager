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
#include <iremote_broker.h>

#include "iremote_object_mocker.h"
#include "mock_message_parcel.h"
#include "display_manager_lite_proxy.h"

using namespace testing;
using namespace testing::ext;

namespace {
std::string logMsg;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
    const char* msg)
{
    logMsg = msg;
}
}

namespace OHOS ::Rosen {
class DisplayManagerLiteProxyMockTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy_;
};

void DisplayManagerLiteProxyMockTest::SetUp()
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    displayManagerLiteProxy_ = sptr<DisplayManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(displayManagerLiteProxy_, nullptr);
}

void DisplayManagerLiteProxyMockTest::TearDown()
{
    displayManagerLiteProxy_ = nullptr;
}

namespace {
/**
 * @tc.name: SetResolution
 * @tc.desc: SetResolution
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, SetResolution, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId id = 1;
    uint32_t width = 1080;
    uint32_t height = 2400;
    float vpr = 2.8f;

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    displayManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("write screenId/width/height/virtualPixelRatio failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    displayManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("write screenId/width/height/virtualPixelRatio failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    displayManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("write screenId/width/height/virtualPixelRatio failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteFloatErrorFlag(true);
    displayManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("write screenId/width/height/virtualPixelRatio failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    displayManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("SendRequest failed") == std::string::npos);
}

/**
 * @tc.name: IsOnboardDisplay
 * @tc.desc: IsOnboardDisplay
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, IsOnboardDisplay, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    DisplayId displayId = 0;
    bool isOnboardDisplay = false;
    MockMessageParcel::ClearAllErrorFlag();

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerLiteProxy_->IsOnboardDisplay(displayId, isOnboardDisplay);
    EXPECT_TRUE(logMsg.find("write interface token failed") != std::string::npos);

    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    displayManagerLiteProxy_->IsOnboardDisplay(displayId, isOnboardDisplay);
    EXPECT_TRUE(logMsg.find("write displayId failed") != std::string::npos);

    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadBoolErrorFlag(true);
    displayManagerLiteProxy_->IsOnboardDisplay(displayId, isOnboardDisplay);
    EXPECT_TRUE(logMsg.find("read result failed") != std::string::npos);

    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    proxy->IsOnboardDisplay(displayId, isOnboardDisplay);
    EXPECT_TRUE(logMsg.find("send request failed") != std::string::npos);
    remoteMocker->SetRequestResult(ERR_NONE);

    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
    remoteMocker = nullptr;
    proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    proxy->IsOnboardDisplay(displayId, isOnboardDisplay);
    EXPECT_TRUE(logMsg.find("remote is null") != std::string::npos);
    MockMessageParcel::ClearAllErrorFlag();
    LOG_SetCallback(nullptr);
}
} // namespace
} // namespace OHOS::Rosen