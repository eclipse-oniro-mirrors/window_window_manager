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
#include "mock/mock_message_parcel.h"
#include "session_manager/include/zidl/session_lifecycle_listener_proxy.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionLifecycleListenerProxyTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

namespace {
HWTEST_F(SessionLifecycleListenerProxyTest, OnLifecycleEvent_NullRemote, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(nullptr);
    ASSERT_NE(proxy, nullptr);
    proxy->OnLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, payload);
    SUCCEED();
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnBatchLifecycleEvent_NullRemote, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;
    std::vector<ISessionLifecycleListener::LifecycleEventPayload> payloads = { payload };

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(nullptr);
    ASSERT_NE(proxy, nullptr);
    proxy->OnBatchLifecycleEvent(payloads);
    SUCCEED();
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnAppInstanceLifecycleEvent_NullRemote, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(nullptr);
    ASSERT_NE(proxy, nullptr);
    proxy->OnAppInstanceLifecycleEvent(payload);
    SUCCEED();
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnLifecycleEvent_Normal, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;

    auto remoteObj = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remoteObj, nullptr);
    EXPECT_CALL(*remoteObj, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteObj);
    ASSERT_NE(proxy, nullptr);
    MockMessageParcel::ClearAllErrorFlag();
    proxy->OnLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, payload);
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnBatchLifecycleEvent_Normal, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;
    std::vector<ISessionLifecycleListener::LifecycleEventPayload> payloads = { payload };

    auto remoteObj = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remoteObj, nullptr);
    EXPECT_CALL(*remoteObj, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteObj);
    ASSERT_NE(proxy, nullptr);
    MockMessageParcel::ClearAllErrorFlag();
    proxy->OnBatchLifecycleEvent(payloads);
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnAppInstanceLifecycleEvent_Normal, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;

    auto remoteObj = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remoteObj, nullptr);
    EXPECT_CALL(*remoteObj, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteObj);
    ASSERT_NE(proxy, nullptr);
    MockMessageParcel::ClearAllErrorFlag();
    proxy->OnAppInstanceLifecycleEvent(payload);
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnLifecycleEvent_WriteInterfaceTokenFailed, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;

    auto remoteObj = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remoteObj, nullptr);
    EXPECT_CALL(*remoteObj, SendRequest(_, _, _, _)).Times(0);

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteObj);
    ASSERT_NE(proxy, nullptr);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    proxy->OnLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, payload);
    MockMessageParcel::ClearAllErrorFlag();
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnBatchLifecycleEvent_WriteInterfaceTokenFailed, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;
    std::vector<ISessionLifecycleListener::LifecycleEventPayload> payloads = { payload };

    auto remoteObj = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remoteObj, nullptr);
    EXPECT_CALL(*remoteObj, SendRequest(_, _, _, _)).Times(0);

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteObj);
    ASSERT_NE(proxy, nullptr);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    proxy->OnBatchLifecycleEvent(payloads);
    MockMessageParcel::ClearAllErrorFlag();
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnAppInstanceLifecycleEvent_WriteInterfaceTokenFailed, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;

    auto remoteObj = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remoteObj, nullptr);
    EXPECT_CALL(*remoteObj, SendRequest(_, _, _, _)).Times(0);

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteObj);
    ASSERT_NE(proxy, nullptr);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    proxy->OnAppInstanceLifecycleEvent(payload);
    MockMessageParcel::ClearAllErrorFlag();
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnLifecycleEvent_WriteParcelableFailed, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;

    auto remoteObj = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remoteObj, nullptr);
    EXPECT_CALL(*remoteObj, SendRequest(_, _, _, _)).Times(0);

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteObj);
    ASSERT_NE(proxy, nullptr);
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    proxy->OnLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, payload);
    MockMessageParcel::ClearAllErrorFlag();
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnBatchLifecycleEvent_WriteParcelableFailed, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;
    std::vector<ISessionLifecycleListener::LifecycleEventPayload> payloads = { payload };

    auto remoteObj = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remoteObj, nullptr);
    EXPECT_CALL(*remoteObj, SendRequest(_, _, _, _)).Times(0);

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteObj);
    ASSERT_NE(proxy, nullptr);
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    proxy->OnBatchLifecycleEvent(payloads);
    MockMessageParcel::ClearAllErrorFlag();
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnAppInstanceLifecycleEvent_WriteParcelableFailed, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;

    auto remoteObj = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remoteObj, nullptr);
    EXPECT_CALL(*remoteObj, SendRequest(_, _, _, _)).Times(0);

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteObj);
    ASSERT_NE(proxy, nullptr);
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    proxy->OnAppInstanceLifecycleEvent(payload);
    MockMessageParcel::ClearAllErrorFlag();
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnLifecycleEvent_RemoteRequestFailed, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;

    auto remoteObj = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remoteObj, nullptr);
    EXPECT_CALL(*remoteObj, SendRequest(_, _, _, _)).WillOnce(Return(ERR_INVALID_DATA));

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteObj);
    ASSERT_NE(proxy, nullptr);
    MockMessageParcel::ClearAllErrorFlag();
    proxy->OnLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, payload);
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnBatchLifecycleEvent_RemoteRequestFailed, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;
    std::vector<ISessionLifecycleListener::LifecycleEventPayload> payloads = { payload };

    auto remoteObj = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remoteObj, nullptr);
    EXPECT_CALL(*remoteObj, SendRequest(_, _, _, _)).WillOnce(Return(ERR_INVALID_DATA));

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteObj);
    ASSERT_NE(proxy, nullptr);
    MockMessageParcel::ClearAllErrorFlag();
    proxy->OnBatchLifecycleEvent(payloads);
}

HWTEST_F(SessionLifecycleListenerProxyTest, OnAppInstanceLifecycleEvent_RemoteRequestFailed, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;

    auto remoteObj = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remoteObj, nullptr);
    EXPECT_CALL(*remoteObj, SendRequest(_, _, _, _)).WillOnce(Return(ERR_INVALID_DATA));

    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteObj);
    ASSERT_NE(proxy, nullptr);
    MockMessageParcel::ClearAllErrorFlag();
    proxy->OnAppInstanceLifecycleEvent(payload);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
