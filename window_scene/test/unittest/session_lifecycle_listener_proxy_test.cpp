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
/**
 * @tc.name: SessionLifecycleListenerProxy_Branches
 * @tc.desc: cover SessionLifecycleListenerProxy branch paths
 * @tc.type: FUNC
 */
HWTEST_F(SessionLifecycleListenerProxyTest, SessionLifecycleListenerProxy_Branches, TestSize.Level1)
{
    ISessionLifecycleListener::LifecycleEventPayload payload;
    payload.persistentId_ = 1;
    payload.bundleName_ = "bundle";
    payload.appIndex_ = 1;
    payload.appInstanceKey_ = "appKey";
    payload.sessionState_ = SessionState::STATE_ACTIVE;
    std::vector<ISessionLifecycleListener::LifecycleEventPayload> payloads = { payload };

    sptr<SessionLifecycleListenerProxy> nullProxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(nullptr);
    ASSERT_NE(nullProxy, nullptr);
    nullProxy->OnLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, payload);
    nullProxy->OnBatchLifecycleEvent(payloads);
    nullProxy->OnAppInstanceLifecycleEvent(payload);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(remoteMocker, nullptr);
    sptr<SessionLifecycleListenerProxy> proxy = sptr<SessionLifecycleListenerProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::ClearAllErrorFlag();
    proxy->OnLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, payload);
    proxy->OnBatchLifecycleEvent(payloads);
    proxy->OnAppInstanceLifecycleEvent(payload);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    proxy->OnLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, payload);
    proxy->OnBatchLifecycleEvent(payloads);
    proxy->OnAppInstanceLifecycleEvent(payload);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    proxy->OnLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, payload);
    proxy->OnBatchLifecycleEvent(payloads);
    proxy->OnAppInstanceLifecycleEvent(payload);

    MockMessageParcel::ClearAllErrorFlag();
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    proxy->OnLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, payload);
    proxy->OnBatchLifecycleEvent(payloads);
    proxy->OnAppInstanceLifecycleEvent(payload);
    remoteMocker->SetRequestResult(ERR_NONE);
    MockMessageParcel::ClearAllErrorFlag();
    SUCCEED();
}
} // namespace
} // namespace Rosen
} // namespace OHOS