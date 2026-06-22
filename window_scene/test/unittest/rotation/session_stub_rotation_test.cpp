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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <ipc_types.h>
#include <message_parcel.h>

#include "mock/mock_session_stub.h"
#include "session/host/include/zidl/session_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionStubRotationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

protected:
    sptr<SessionStub> session_ = nullptr;
};

void SessionStubRotationTest::SetUpTestCase() {}

void SessionStubRotationTest::TearDownTestCase() {}

void SessionStubRotationTest::SetUp()
{
    session_ = sptr<SessionStubMocker>::MakeSptr();
    EXPECT_NE(nullptr, session_);
}

void SessionStubRotationTest::TearDown()
{
    session_ = nullptr;
}

namespace {
/**
 * @tc.name: HandleUpdateRotationChangeListenerRegistered
 * @tc.desc: sessionStub HandleUpdateRotationChangeListenerRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubRotationTest, HandleUpdateRotationChangeListenerRegistered, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0);
    data.WriteBool(false);
    auto result = session_->HandleUpdateRotationChangeListenerRegistered(data, reply);
    ASSERT_EQ(result, ERR_NONE);
    result = session_->HandleUpdateRotationChangeListenerRegistered(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleSetPreferredOrientationWithResult01
 * @tc.desc: 测试正常流程
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubRotationTest, HandleSetPreferredOrientationWithResult01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionStubRotationTest: HandleSetPreferredOrientationWithResult01 start";
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(static_cast<uint32_t>(Orientation::VERTICAL));
    data.WriteUint32(123);
    data.WriteBool(true);
    auto result = session_->HandleSetPreferredOrientationWithResult(data, reply);
    ASSERT_EQ(result, ERR_NONE);
    GTEST_LOG_(INFO) << "SessionStubRotationTest: HandleSetPreferredOrientationWithResult01 end";
}

/**
 * @tc.name: HandleSetPreferredOrientationWithResult02
 * @tc.desc: 测试 ReadUint32 失败
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubRotationTest, HandleSetPreferredOrientationWithResult02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionStubRotationTest: HandleSetPreferredOrientationWithResult02 start";
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleSetPreferredOrientationWithResult(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    GTEST_LOG_(INFO) << "SessionStubRotationTest: HandleSetPreferredOrientationWithResult02 end";
}
} // namespace
} // namespace Rosen
} // namespace OHOS
