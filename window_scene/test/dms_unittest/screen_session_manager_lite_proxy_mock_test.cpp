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

#include <common/rs_rect.h>
#include <gtest/gtest.h>
#include <iremote_broker.h>

#include "iremote_object_mocker.h"
#include "screen_session_manager/include/screen_session_manager_lite.h"
#include "mock_message_parcel.h"
#include "zidl/screen_session_manager_lite_proxy.h"
#include "display_manager_interface_code.h"

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
class ScreenSessionManagerLiteProxyMockTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    sptr<ScreenSessionManagerLiteProxy> screenSessionManagerLiteProxy_;
};

void ScreenSessionManagerLiteProxyMockTest::SetUp()
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    screenSessionManagerLiteProxy_ = sptr<ScreenSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(screenSessionManagerLiteProxy_, nullptr);
}

void ScreenSessionManagerLiteProxyMockTest::TearDown()
{
    screenSessionManagerLiteProxy_ = nullptr;
}

namespace {
/**
 * @tc.name: SetResolution
 * @tc.desc: SetResolution
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerLiteProxyMockTest, SetResolution, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId id = 1;
    uint32_t width = 1080;
    uint32_t height = 2400;
    float vpr = 2.8f;

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    screenSessionManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    screenSessionManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("write screenId/width/height/virtualPixelRatio failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    screenSessionManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("write screenId/width/height/virtualPixelRatio failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    screenSessionManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("write screenId/width/height/virtualPixelRatio failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteFloatErrorFlag(true);
    screenSessionManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("write screenId/width/height/virtualPixelRatio failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    screenSessionManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("SendRequest failed") == std::string::npos);
}
} // namespace
} // namespace OHOS::Rosen