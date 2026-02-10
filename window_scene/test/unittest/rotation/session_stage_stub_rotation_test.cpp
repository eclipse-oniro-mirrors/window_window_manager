/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <iremote_stub.h>
#include <message_option.h>
#include <message_parcel.h>

#include "mock/mock_session_stage.h"
#include "session_manager/include/zidl/scene_session_manager_interface.h"
#include "session/container/include/zidl/session_stage_stub.h"
#include "session/container/include/zidl/session_stage_ipc_interface_code.h"
#include "window_manager.h"
#include "window_manager_agent.h"
#include "ws_common.h"
#include "zidl/window_manager_agent_interface.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionStageStubRotationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<SessionStageStub> sessionStageStub_ = sptr<SessionStageMocker>::MakeSptr();
};

void SessionStageStubRotationTest::SetUpTestCase() {}

void SessionStageStubRotationTest::TearDownTestCase() {}

void SessionStageStubRotationTest::SetUp() {}

void SessionStageStubRotationTest::TearDown() {}

namespace {
/**
 * @tc.name: HandleSetCurrentRotation
 * @tc.desc: test function : HandleSetCurrentRotation
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubRotationTest, HandleSetCurrentRotation, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_CURRENT_ROTATION);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(ERR_INVALID_VALUE, sessionStageStub_->HandleSetCurrentRotation(data, reply));

    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteInt32(ONE_FOURTH_FULL_CIRCLE_DEGREE);
    EXPECT_EQ(ERR_NONE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));

    data.WriteInt32(ONE_FOURTH_FULL_CIRCLE_DEGREE);
    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleSetCurrentRotation(data, reply));
}

/**
 * @tc.name: HandleNotifyRotationChange
 * @tc.desc: test function : HandleNotifyRotationChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubRotationTest, HandleNotifyRotationChange, Function | SmallTest | Level1)
{
    sptr<SessionStageMocker> sessionStageStub = sptr<SessionStageMocker>::MakeSptr();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    RotationChangeInfo info = { RotationChangeType::WINDOW_WILL_ROTATE, 0, 0, { 0, 0, 2720, 1270 } };
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_ROTATION_CHANGE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(info.type_));
    data.WriteUint32(info.orientation_);
    data.WriteUint64(info.displayId_);
    data.WriteInt32(info.displayRect_.posX_);
    data.WriteInt32(info.displayRect_.posY_);
    data.WriteUint32(info.displayRect_.width_);
    data.WriteUint32(info.displayRect_.height_);
    sessionStageStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(sessionStageStub_->HandleNotifyRotationChange(data, reply), ERR_INVALID_DATA);
}
} // namespace
} // namespace Rosen
} // namespace OHOS