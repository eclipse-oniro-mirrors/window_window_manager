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

#include "iremote_object_mocker.h"
#include "mock_message_parcel.h"
#include "session/host/include/zidl/session_ipc_interface_code.h"
#include "session/host/include/zidl/session_proxy.h"
#include "ws_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionProxyLayoutTest : public testing::Test {
public:
    SessionProxyLayoutTest() : iRemoteObjectMocker_(sptr<IRemoteObjectMocker>::MakeSptr()),
        sessionProxy_(sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker_)) {}
    ~SessionProxyLayoutTest() = default;

    sptr<IRemoteObject> iRemoteObjectMocker_;
    sptr<SessionProxy> sessionProxy_;
};

namespace {
/**
 * @tc.name: NotifyAttachedWindowsLimitsChanged01
 * @tc.desc: Test NotifyAttachedWindowsLimitsChanged with valid limits
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyLayoutTest, NotifyAttachedWindowsLimitsChanged01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyLayoutTest: NotifyAttachedWindowsLimitsChanged01 start";
    WindowLimits newLimits = { 200, 1000, 300, 2000, 0.0f, 0.0f, 0.0f, PixelUnit::PX };

    WSError res = sessionProxy_->NotifyAttachedWindowsLimitsChanged(newLimits);
    EXPECT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyLayoutTest: NotifyAttachedWindowsLimitsChanged01 end";
}

/**
 * @tc.name: NotifyAttachedWindowsLimitsChanged02
 * @tc.desc: Test NotifyAttachedWindowsLimitsChanged with WriteInterfaceToken error
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyLayoutTest, NotifyAttachedWindowsLimitsChanged02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyLayoutTest: NotifyAttachedWindowsLimitsChanged02 start";
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);

    WindowLimits newLimits = { 100, 800, 200, 1600, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WSError res = sessionProxy_->NotifyAttachedWindowsLimitsChanged(newLimits);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
    GTEST_LOG_(INFO) << "SessionProxyLayoutTest: NotifyAttachedWindowsLimitsChanged02 end";
}

/**
 * @tc.name: NotifyAttachedWindowsLimitsChanged03
 * @tc.desc: Test NotifyAttachedWindowsLimitsChanged with Marshalling error (WriteUint32 fails)
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyLayoutTest, NotifyAttachedWindowsLimitsChanged03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyLayoutTest: NotifyAttachedWindowsLimitsChanged03 start";
    WindowLimits newLimits = { 150, 900, 250, 1800, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    WSError res = sessionProxy_->NotifyAttachedWindowsLimitsChanged(newLimits);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
    GTEST_LOG_(INFO) << "SessionProxyLayoutTest: NotifyAttachedWindowsLimitsChanged03 end";
}

/**
 * @tc.name: NotifyAttachedWindowsLimitsChanged04
 * @tc.desc: Test NotifyAttachedWindowsLimitsChanged with VP unit limits
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyLayoutTest, NotifyAttachedWindowsLimitsChanged04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyLayoutTest: NotifyAttachedWindowsLimitsChanged04 start";
    WindowLimits newLimits = { 50, 500, 100, 1000, 0.0f, 0.0f, 0.0f, PixelUnit::VP };
    WSError res = sessionProxy_->NotifyAttachedWindowsLimitsChanged(newLimits);
    EXPECT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyLayoutTest: NotifyAttachedWindowsLimitsChanged04 end";
}

/**
 * @tc.name: NotifyAttachedWindowsLimitsChanged05
 * @tc.desc: Test NotifyAttachedWindowsLimitsChanged with null remote object
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyLayoutTest, NotifyAttachedWindowsLimitsChanged05, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyLayoutTest: NotifyAttachedWindowsLimitsChanged05 start";
    // Create proxy with null remote object
    auto sessionProxy = sptr<SessionProxy>::MakeSptr(nullptr);

    WindowLimits newLimits = { 100, 800, 200, 1600, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WSError res = sessionProxy->NotifyAttachedWindowsLimitsChanged(newLimits);
    EXPECT_EQ(res, WSError::WS_ERROR_IPC_FAILED);
    GTEST_LOG_(INFO) << "SessionProxyLayoutTest: NotifyAttachedWindowsLimitsChanged05 end";
}

} // namespace
} // namespace Rosen
} // namespace OHOS
