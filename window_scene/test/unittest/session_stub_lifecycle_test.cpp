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
#include <ipc_types.h>
#include <pointer_event.h>
#include "iremote_object_mocker.h"
#include "mock/mock_session_stub.h"
#include "session/host/include/zidl/session_stub.h"
#include "ability_start_setting.h"
#include "session/host/include/zidl/session_ipc_interface_code.h"
#include "want.h"
#include "ws_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class SessionStubLifecycleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    sptr <SessionStub> session_ = nullptr;
};

void SessionStubLifecycleTest::SetUpTestCase()
{
}

void SessionStubLifecycleTest::TearDownTestCase()
{
}

void SessionStubLifecycleTest::SetUp()
{
    session_ = new (std::nothrow) SessionStubMocker();
    EXPECT_NE(nullptr, session_);
}

void SessionStubLifecycleTest::TearDown()
{
    session_ = nullptr;
}

namespace {

/**
 * @tc.name: HandleShow009
 * @tc.desc: sessionStub SessionStubLifecycleTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubLifecycleTest, HandleShow009, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(false);
    auto res = session_->HandleShow(data, reply);
    ASSERT_EQ(0, res);
}

/**
 * @tc.name: HandleHide010
 * @tc.desc: sessionStub SessionStubLifecycleTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubLifecycleTest, HandleHide010, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    auto res = session_->HandleHide(data, reply);
    ASSERT_EQ(0, res);
}

/**
 * @tc.name: HandleDrawingCompleted
 * @tc.desc: sessionStub SessionStubLifecycleTest
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubLifecycleTest, HandleDrawingCompleted, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_NE(session_, nullptr);
    auto res = session_->HandleDrawingCompleted(data, reply);
    ASSERT_EQ(0, res);
}

/**
 * @tc.name: HandlePendingSessionActivation011
 * @tc.desc: sessionStub SessionStubLifecycleTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubLifecycleTest, HandlePendingSessionActivation011, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    AAFwk::Want options;
    EXPECT_NE(data.WriteString("HandlePendingSessionActivation"), false);
    EXPECT_NE(data.WriteParcelable(&options), false);
    ASSERT_EQ(data.WriteUint32(1), true);
    ASSERT_EQ(data.WriteUint64(2), true);
    auto res = session_->HandlePendingSessionActivation(data, reply);
    ASSERT_EQ(5, res);
}
}
}
}