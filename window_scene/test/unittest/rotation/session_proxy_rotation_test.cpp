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

#include "session_proxy.h"

#include <gtest/gtest.h>

#include "mock_message_parcel.h"
#include "iremote_object_mocker.h"
#include "ws_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionProxyTest : public testing::Test {
public:
    SessionProxyTest() {}
    ~SessionProxyTest() {}
};

namespace {
/**
 * @tc.name: UpdateRotationChangeRegistered
 * @tc.desc: UpdateRotationChangeRegistered test
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, UpdateRotationChangeRegistered, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateRotationChangeRegistered start";
    auto iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    auto sProxy = sptr<SessionProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sProxy, nullptr);
    WSError res = sProxy->UpdateRotationChangeRegistered(0, false);
    ASSERT_EQ(res, WSError::WS_OK);
    GTEST_LOG_(INFO) << "SessionProxyTest: UpdateRotationChangeRegistered end";
}

/**
 * @tc.name: SetPreferredOrientationWithResult
 * @tc.desc: 测试 SetPreferredOrientationWithResult 的各种场景
 * @tc.type: FUNC
 */
HWTEST_F(SessionProxyTest, SetPreferredOrientationWithResult, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionProxyTest: SetPreferredOrientationWithResult start";
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SessionProxy> sessionProxy = sptr<SessionProxy>::MakeSptr(remoteMocker);

    Orientation orientation = Orientation::VERTICAL;
    uint32_t promiseId = 123;
    bool needAnimation = true;

    WMError errCode = sessionProxy->SetPreferredOrientationWithResult(orientation, promiseId, needAnimation);
    EXPECT_EQ(errCode, WMError::WM_OK);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    errCode = sessionProxy->SetPreferredOrientationWithResult(orientation, promiseId, needAnimation);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    errCode = sessionProxy->SetPreferredOrientationWithResult(orientation, promiseId, needAnimation);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    remoteMocker->SetRequestResult(1);
    errCode = sessionProxy->SetPreferredOrientationWithResult(orientation, promiseId, needAnimation);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(0);

    sptr<SessionProxy> nullProxy = sptr<SessionProxy>::MakeSptr(nullptr);
    errCode = nullProxy->SetPreferredOrientationWithResult(orientation, promiseId, needAnimation);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    GTEST_LOG_(INFO) << "SessionProxyTest: SetPreferredOrientationWithResult end";
}
} // namespace
} // namespace Rosen
} // namespace OHOS
