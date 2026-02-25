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
#include <gmock/gmock.h>
#include <ipc_types.h>
#include "iremote_object_mocker.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_session_stub.h"
#include "mock_message_parcel.h"
#include "session/host/include/zidl/session_ipc_interface_code.h"
#include "session/host/include/zidl/session_stub.h"
#include "want.h"
#include "wm_common.h"
#include "ws_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class CompatibleModeSessionStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
};

void CompatibleModeSessionStubTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "setup test case";
    MockMessageParcel::ClearAllErrorFlag();
}

void CompatibleModeSessionStubTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "teardown test case";
}

void CompatibleModeSessionStubTest::SetUp()
{
    GTEST_LOG_(INFO) << "setup";
}

void CompatibleModeSessionStubTest::TearDown()
{
    GTEST_LOG_(INFO) << "teardown";
    MockMessageParcel::ClearAllErrorFlag();
}

namespace {
/**
 * @tc.name: HandleNotifyCompatibleModeChange
 * @tc.desc: HandleNotifyCompatibleModeChangeWithReadModeFail
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeSessionStubTest, HandleNotifyCompatibleModeChangeWithReadModeFail, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNotifyCompatibleModeChangeWithReadModeFail test start";
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_COMPATIBLE_MODE_CHANGE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    int ret = session->ProcessRemoteRequest(code, data, reply, option);
    MockMessageParcel::SetReadInt32ErrorFlag(false);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    GTEST_LOG_(INFO) << "HandleNotifyCompatibleModeChangeWithReadModeFail test end";
}

/**
 * @tc.name: HandleNotifyCompatibleModeChange
 * @tc.desc: HandleNotifyCompatibleModeChangeWithWriteCodeFail
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeSessionStubTest, HandleNotifyCompatibleModeChangeWithWriteCodeFail, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNotifyCompatibleModeChangeWithWriteCodeFail test start";
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_COMPATIBLE_MODE_CHANGE);
    MessageParcel data;
    int32_t mode = 1;
    data.WriteInt32(mode);
    MessageParcel reply;
    MessageOption option;
    MockMessageParcel::AddInt32Cache(mode);
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    int ret = session->ProcessRemoteRequest(code, data, reply, option);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    GTEST_LOG_(INFO) << "HandleNotifyCompatibleModeChangeWithWriteCodeFail test end";
}

/**
 * @tc.name: HandleNotifyCompatibleModeChange
 * @tc.desc: HandleNotifyCompatibleModeChangeWithErrNone
 * @tc.type: FUNC
 */
HWTEST_F(CompatibleModeSessionStubTest, HandleNotifyCompatibleModeChangeWithErrNone, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNotifyCompatibleModeChangeWithErrNone test start";
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_COMPATIBLE_MODE_CHANGE);
    MessageParcel data;
    int32_t mode = 1;
    data.WriteInt32(mode);
    MessageParcel reply;
    MessageOption option;
    MockMessageParcel::AddInt32Cache(mode);
    int ret = session->ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_NONE);
    GTEST_LOG_(INFO) << "HandleNotifyCompatibleModeChangeWithErrNone test end";
}
} // namespace
} // namespace Rosen
} // namespace OHOS