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
#include "iremote_object_mocker.h"
#include "mock_message_parcel.h"
#include "session_manager_service_recover_proxy.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionManagerServiceRecoverProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<IRemoteObject> iRemoteObjectMocker_;
    sptr<SessionManagerServiceRecoverProxy> sessionManagerServiceRecoverProxy_;
};

void SessionManagerServiceRecoverProxyTest::SetUpTestCase() {}

void SessionManagerServiceRecoverProxyTest::TearDownTestCase() {}

void SessionManagerServiceRecoverProxyTest::SetUp() {}

void SessionManagerServiceRecoverProxyTest::TearDown() {}

namespace {
/**
 * @tc.name: OnSessionManagerServiceRecover01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerServiceRecoverProxyTest, OnSessionManagerServiceRecover01, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sessionManagerServiceRecoverProxy_ = sptr<SessionManagerServiceRecoverProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sessionManagerServiceRecoverProxy_, nullptr);
    sptr<IRemoteObject> sessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(sessionManagerService, nullptr);
    sessionManagerServiceRecoverProxy_->OnSessionManagerServiceRecover(sessionManagerService);
}

/**
 * @tc.name: OnSessionManagerServiceRecover02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerServiceRecoverProxyTest, OnSessionManagerServiceRecover02, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sessionManagerServiceRecoverProxy_ = sptr<SessionManagerServiceRecoverProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sessionManagerServiceRecoverProxy_, nullptr);
    sptr<IRemoteObject> sessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(sessionManagerService, nullptr);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sessionManagerServiceRecoverProxy_->OnSessionManagerServiceRecover(sessionManagerService);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: OnSessionManagerServiceRecover03
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerServiceRecoverProxyTest, OnSessionManagerServiceRecover03, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sessionManagerServiceRecoverProxy_ = sptr<SessionManagerServiceRecoverProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sessionManagerServiceRecoverProxy_, nullptr);
    sptr<IRemoteObject> sessionManagerService = nullptr;
    sessionManagerServiceRecoverProxy_->OnSessionManagerServiceRecover(sessionManagerService);
}

/**
 * @tc.name: OnSessionManagerServiceRecover04
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerServiceRecoverProxyTest, OnSessionManagerServiceRecover04, TestSize.Level1)
{
    sptr<MockIRemoteObject> mockIRemoteObject = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(mockIRemoteObject, nullptr);
    mockIRemoteObject->sendRequestResult_ = 1;
    sessionManagerServiceRecoverProxy_ = sptr<SessionManagerServiceRecoverProxy>::MakeSptr(mockIRemoteObject);
    ASSERT_NE(sessionManagerServiceRecoverProxy_, nullptr);
    sptr<IRemoteObject> sessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(sessionManagerService, nullptr);
    sessionManagerServiceRecoverProxy_->OnSessionManagerServiceRecover(sessionManagerService);
}

/**
 * @tc.name: OnWMSConnectionChanged01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerServiceRecoverProxyTest, OnWMSConnectionChanged01, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sessionManagerServiceRecoverProxy_ = sptr<SessionManagerServiceRecoverProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sessionManagerServiceRecoverProxy_, nullptr);
    sptr<IRemoteObject> sessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(sessionManagerService, nullptr);
    sessionManagerServiceRecoverProxy_->OnWMSConnectionChanged(1, 0, true, sessionManagerService, 9999);
}

/**
 * @tc.name: OnWMSConnectionChanged02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerServiceRecoverProxyTest, OnWMSConnectionChanged02, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sessionManagerServiceRecoverProxy_ = sptr<SessionManagerServiceRecoverProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sessionManagerServiceRecoverProxy_, nullptr);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sptr<IRemoteObject> sessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(sessionManagerService, nullptr);
    sessionManagerServiceRecoverProxy_->OnWMSConnectionChanged(1, 0, true, sessionManagerService, 8888);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: OnWMSConnectionChanged03
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerServiceRecoverProxyTest, OnWMSConnectionChanged03, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sessionManagerServiceRecoverProxy_ = sptr<SessionManagerServiceRecoverProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sessionManagerServiceRecoverProxy_, nullptr);
    sptr<IRemoteObject> sessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(sessionManagerService, nullptr);
    sessionManagerServiceRecoverProxy_->OnWMSConnectionChanged(-1, 0, true, sessionManagerService, 7777);
}

/**
 * @tc.name: OnWMSConnectionChanged04
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerServiceRecoverProxyTest, OnWMSConnectionChanged04, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sessionManagerServiceRecoverProxy_ = sptr<SessionManagerServiceRecoverProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sessionManagerServiceRecoverProxy_, nullptr);
    sptr<IRemoteObject> sessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(sessionManagerService, nullptr);
    sessionManagerServiceRecoverProxy_->OnWMSConnectionChanged(1, -1, true, sessionManagerService, 6666);
}

/**
 * @tc.name: OnWMSConnectionChanged05
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerServiceRecoverProxyTest, OnWMSConnectionChanged05, TestSize.Level1)
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sessionManagerServiceRecoverProxy_ = sptr<SessionManagerServiceRecoverProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sessionManagerServiceRecoverProxy_, nullptr);
    sptr<IRemoteObject> sessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(sessionManagerService, nullptr);
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    sessionManagerServiceRecoverProxy_->OnWMSConnectionChanged(1, 1, true, sessionManagerService, 5555);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: OnWMSConnectionChanged06
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerServiceRecoverProxyTest, OnWMSConnectionChanged06, TestSize.Level1)
{
    sptr<MockIRemoteObject> mockIRemoteObject = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(mockIRemoteObject, nullptr);
    mockIRemoteObject->sendRequestResult_ = 1;
    sessionManagerServiceRecoverProxy_ = sptr<SessionManagerServiceRecoverProxy>::MakeSptr(mockIRemoteObject);
    ASSERT_NE(sessionManagerServiceRecoverProxy_, nullptr);
    sptr<IRemoteObject> sessionManagerService = nullptr;
    sessionManagerServiceRecoverProxy_->OnWMSConnectionChanged(1, 1, true, sessionManagerService, 4444);
}

/**
 * @tc.name: OnWMSConnectionChanged07
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerServiceRecoverProxyTest, OnWMSConnectionChanged07, TestSize.Level1)
{
    sptr<MockIRemoteObject> mockIRemoteObject = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(mockIRemoteObject, nullptr);
    mockIRemoteObject->sendRequestResult_ = 1;
    sessionManagerServiceRecoverProxy_ = sptr<SessionManagerServiceRecoverProxy>::MakeSptr(mockIRemoteObject);
    ASSERT_NE(sessionManagerServiceRecoverProxy_, nullptr);
    sptr<IRemoteObject> sessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(sessionManagerService, nullptr);
    sessionManagerServiceRecoverProxy_->OnWMSConnectionChanged(1, 1, true, sessionManagerService, 3333);
}
} // namespace
} // namespace Rosen
} // namespace OHOS