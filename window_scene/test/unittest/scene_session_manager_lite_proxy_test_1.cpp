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

#include "iremote_object_mocker.h"
#include "mock/mock_session.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_message_parcel.h"
#include "pointer_event.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/zidl/pip_change_listener_stub.h"
#include "session_manager/include/zidl/scene_session_manager_lite_interface.h"
#include "session_manager/include/zidl/scene_session_manager_lite_proxy.h"
#include "session_manager/include/zidl/session_lifecycle_listener_stub.h"
#include "session_manager/include/zidl/session_router_stack_listener_stub.h"
#include "window_manager_agent.h"
#include "ws_common.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class sceneSessionManagerLiteProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<IRemoteObject> iRemoteObjectMocker;

private:
    sptr<MockIRemoteObject> iRemoteObjectMocker_ {nullptr};
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy_ {nullptr};
};

void sceneSessionManagerLiteProxyTest::SetUpTestCase()
{
}

void sceneSessionManagerLiteProxyTest::TearDownTestCase()
{
}

void sceneSessionManagerLiteProxyTest::SetUp()
{
    iRemoteObjectMocker_ = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker_, nullptr);
    sceneSessionManagerLiteProxy_ = sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker_);
    ASSERT_NE(sceneSessionManagerLiteProxy_, nullptr);
}

void sceneSessionManagerLiteProxyTest::TearDown()
{
    iRemoteObjectMocker_ = nullptr;
    sceneSessionManagerLiteProxy_ = nullptr;
    MockMessageParcel::ClearAllErrorFlag();
}

namespace {
class MockSessionLifecycleListenerForLiteProxyTest : public SessionLifecycleListenerStub {
public:
    void OnLifecycleEvent(SessionLifecycleEvent event, const LifecycleEventPayload& payload) override {}
    void OnBatchLifecycleEvent(const std::vector<LifecycleEventPayload>& payloads) override {}
    void OnAppInstanceLifecycleEvent(const LifecycleEventPayload& payload) override {}
};

HWTEST_F(sceneSessionManagerLiteProxyTest, GetSessionInfoWithDisplay, TestSize.Level1)
{
    SessionInfoBean sessionInfo;
    AAFwk::DisplayInfo displayInfo;

    sptr<SceneSessionManagerLiteProxy> nullProxy = sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    ASSERT_NE(nullProxy, nullptr);
    auto ret = nullProxy->GetSessionInfo("", 1, sessionInfo, displayInfo);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = sceneSessionManagerLiteProxy_->GetSessionInfo("", 1, sessionInfo, displayInfo);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteString16ErrorFlag(true);
    ret = sceneSessionManagerLiteProxy_->GetSessionInfo("", 1, sessionInfo, displayInfo);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = sceneSessionManagerLiteProxy_->GetSessionInfo("", 1, sessionInfo, displayInfo);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    iRemoteObjectMocker_->SetRequestResult(1);
    ret = sceneSessionManagerLiteProxy_->GetSessionInfo("", 1, sessionInfo, displayInfo);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
    iRemoteObjectMocker_->SetRequestResult(0);

    MockMessageParcel::ClearAllErrorFlag();
    ret = sceneSessionManagerLiteProxy_->GetSessionInfo("", 1, sessionInfo, displayInfo);
    EXPECT_EQ(ret, WSError::WS_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, RegisterSessionLifecycleListenerByAppInstance, TestSize.Level1)
{
    sptr<ISessionLifecycleListener> listener = sptr<MockSessionLifecycleListenerForLiteProxyTest>::MakeSptr();
    ASSERT_NE(listener, nullptr);

    MockMessageParcel::ClearAllErrorFlag();
    auto ret = sceneSessionManagerLiteProxy_->RegisterSessionLifecycleListenerByAppInstance(
        nullptr, "bundle", 1, "key");
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    ret = sceneSessionManagerLiteProxy_->RegisterSessionLifecycleListenerByAppInstance(
        listener, "bundle", 1, "key");
    EXPECT_EQ(ret, WMError::WM_OK);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = sceneSessionManagerLiteProxy_->RegisterSessionLifecycleListenerByAppInstance(
        listener, "bundle", 1, "key");
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteRemoteObjectErrorFlag(true);
    ret = sceneSessionManagerLiteProxy_->RegisterSessionLifecycleListenerByAppInstance(
        listener, "bundle", 1, "key");
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteStringErrorFlag(true);
    ret = sceneSessionManagerLiteProxy_->RegisterSessionLifecycleListenerByAppInstance(
        listener, "bundle", 1, "key");
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = sceneSessionManagerLiteProxy_->RegisterSessionLifecycleListenerByAppInstance(
        listener, "bundle", 1, "key");
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    iRemoteObjectMocker_->SetRequestResult(1);
    ret = sceneSessionManagerLiteProxy_->RegisterSessionLifecycleListenerByAppInstance(
        listener, "bundle", 1, "key");
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker_->SetRequestResult(0);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    ret = sceneSessionManagerLiteProxy_->RegisterSessionLifecycleListenerByAppInstance(
        listener, "bundle", 1, "key");
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
}
}
}
}