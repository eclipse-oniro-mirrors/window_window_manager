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
#include <message_option.h>
#include <message_parcel.h>

#include "session/host/include/scene_session.h"
#include "session/host/include/zidl/session_interface.h"
#include "session/host/include/zidl/session_ipc_interface_code.h"
#include "session/host/include/zidl/session_stub.h"
#include "mock/mock_message_parcel.h"
#include "mock/mock_session_stub.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class SessionStubEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<SessionStub> stub_;
};

void SessionStubEventTest::SetUpTestCase()
{
}

void SessionStubEventTest::TearDownTestCase()
{
}

void SessionStubEventTest::SetUp()
{
    stub_ = sptr<SessionStubMocker>::MakeSptr();
    EXPECT_NE(nullptr, stub_);
}

void SessionStubEventTest::TearDown()
{
}

namespace {
/**
 * @tc.name: ProcessRemoteRequest
 * @tc.desc: test ProcessRemoteRequest
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubEventTest, ProcessRemoteRequest, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    // Failed to obtain the command.
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    int res = stub_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SEND_COMMAND_EVENT),
        data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: HandleSendCommonEvent
 * @tc.desc: test HandleSendCommonEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubEventTest, HandleSendCommonEvent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    
    // Failed to obtain the command.
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    int res = stub_->HandleSendCommonEvent(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    // Failed to obtain the length.
    MockMessageParcel::AddInt32Cache(static_cast<uint32_t>(CommonEventCommand::LOCK_CURSOR));
    res = stub_->HandleSendCommonEvent(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    // Failed to obtain parameters.
    MockMessageParcel::AddInt32Cache(static_cast<uint32_t>(CommonEventCommand::LOCK_CURSOR));
    MockMessageParcel::AddInt32Cache(LOCK_CURSOR_LENGTH);
    res = stub_->HandleSendCommonEvent(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    // Failed to lock cursor.
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    MockMessageParcel::AddInt32Cache(static_cast<uint32_t>(CommonEventCommand::LOCK_CURSOR));
    MockMessageParcel::AddInt32Cache(LOCK_CURSOR_LENGTH);
    MockMessageParcel::AddInt32Cache(1);
    MockMessageParcel::AddInt32Cache(1);
    res = stub_->HandleSendCommonEvent(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    // out of range (<0)
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    MockMessageParcel::AddInt32Cache(static_cast<uint32_t>(CommonEventCommand::LOCK_CURSOR));
    MockMessageParcel::AddInt32Cache(-1);
    res = stub_->HandleSendCommonEvent(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    // out of range (>max)
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    MockMessageParcel::AddInt32Cache(static_cast<uint32_t>(CommonEventCommand::LOCK_CURSOR));
    MockMessageParcel::AddInt32Cache(COMMON_EVENT_COMMAND_MAX_LENGTH + 1);
    res = stub_->HandleSendCommonEvent(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    // lock cursor.
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    MockMessageParcel::AddInt32Cache(static_cast<uint32_t>(CommonEventCommand::LOCK_CURSOR));
    MockMessageParcel::AddInt32Cache(LOCK_CURSOR_LENGTH);
    MockMessageParcel::AddInt32Cache(1);
    MockMessageParcel::AddInt32Cache(1);
    res = stub_->HandleSendCommonEvent(data, reply);
    EXPECT_EQ(res, ERR_NONE);

    // unlock cursor.
    MockMessageParcel::AddInt32Cache(static_cast<uint32_t>(CommonEventCommand::UNLOCK_CURSOR));
    MockMessageParcel::AddInt32Cache(UNLOCK_CURSOR_LENGTH);
    MockMessageParcel::AddInt32Cache(1);
    res = stub_->HandleSendCommonEvent(data, reply);
    EXPECT_EQ(res, ERR_NONE);

    // Perform default behavior.
    MockMessageParcel::AddInt32Cache(10000);
    MockMessageParcel::AddInt32Cache(0);
    res = stub_->HandleSendCommonEvent(data, reply);
    EXPECT_EQ(res, ERR_NONE);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: HandleRecoverWindowEffect
 * @tc.desc: HandleRecoverWindowEffect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubEventTest, HandleRecoverWindowEffect, TestSize.Level1)
{
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RECOVER_WINDOW_EFFECT);
    MessageOption option;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(session->ProcessRemoteRequest(code, data, reply, option), ERR_NONE);
    MockMessageParcel::SetReadBoolErrorFlag(true);
    EXPECT_EQ(session->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    MockMessageParcel::ClearAllErrorFlag();
}
} // namespace
} // namespace Rosen
} // namespace OHOS

