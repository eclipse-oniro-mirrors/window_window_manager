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
} // namespace
} // namespace Rosen
} // namespace OHOS
