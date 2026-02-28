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
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>
#include "iremote_object_mocker.h"
#include "scene_board_judgement.h"
#include "session_manager.h"
#include "session_manager_lite.h"
#include "session_manager_service_recover_interface.h"
#include "singleton_delegator.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    sptr<SessionManager> sm_ = nullptr;
    int32_t userId_ = 100;
};

void SessionManagerTest::SetUpTestCase() {}

void SessionManagerTest::TearDownTestCase() {}

void SessionManagerTest::SetUp()
{
    sm_ = &SessionManager::GetInstance(userId_);
}

void SessionManagerTest::TearDown()
{
    sm_ = nullptr;
}

namespace {
/**
 * @tc.name: GetSceneSessionManagerProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, GetSceneSessionManagerProxy, TestSize.Level1)
{
    sm_->RemoveSSMDeathRecipient();
    sm_->ClearSessionManagerProxy();
    auto sceneSessionManagerProxy = sm_->GetSceneSessionManagerProxy();
    ASSERT_NE(nullptr, sceneSessionManagerProxy);

    sm_->ClearSessionManagerProxy();
    sm_->sessionManagerServiceProxy_ = SessionManagerLite::GetInstance().GetSessionManagerServiceProxy();
    sceneSessionManagerProxy = sm_->GetSceneSessionManagerProxy();
    ASSERT_NE(nullptr, sceneSessionManagerProxy);
}

/**
 * @tc.name: ClearSessionManagerProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, ClearSessionManagerProxy, TestSize.Level1)
{
    // branch 1
    sm_->InitSessionManagerServiceProxy();
    ASSERT_NE(nullptr, sm_->sessionManagerServiceProxy_);
    sm_->ClearSessionManagerProxy();
    ASSERT_EQ(sm_->sessionManagerServiceProxy_, nullptr);

    // branch 2
    sm_->ClearSessionManagerProxy();
    ASSERT_EQ(sm_->sessionManagerServiceProxy_, nullptr);
}

/**
 * @tc.name: OnRemoteDied1 | FoundationDeathRecipient
 * @tc.desc: foundation died
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, OnRemoteDied1, TestSize.Level1)
{
    sptr<IRemoteObject> sptrDeath = sptr<IRemoteObjectMocker>::MakeSptr();
    wptr<IRemoteObject> wptrDeath = wptr(sptrDeath);
    auto deathRecipient = sptr<FoundationDeathRecipient>::MakeSptr(userId_);
    deathRecipient->OnRemoteDied(wptrDeath);
    ASSERT_EQ(false, sm_->isWMSConnected_);
    ASSERT_EQ(false, sm_->isRecoverListenerRegistered_);
    ASSERT_EQ(nullptr, sm_->mockSessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sm_->sessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sm_->sceneSessionManagerProxy_);
}

/**
 * @tc.name: OnRemoteDied2 | SSMDeathRecipient
 * @tc.desc: scb died
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, OnRemoteDied2, TestSize.Level1)
{
    sptr<IRemoteObject> sptrDeath = sptr<IRemoteObjectMocker>::MakeSptr();
    wptr<IRemoteObject> wptrDeath = wptr(sptrDeath);
    auto deathRecipient = sptr<SSMDeathRecipient>::MakeSptr(userId_);

    deathRecipient->OnRemoteDied(wptrDeath);
    ASSERT_EQ(nullptr, sm_->sessionManagerServiceProxy_);
}

/**
 * @tc.name: OnFoundationDied
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, OnFoundationDied, TestSize.Level1)
{
    sm_->OnFoundationDied();
    ASSERT_EQ(false, sm_->isWMSConnected_);
    ASSERT_EQ(false, sm_->isRecoverListenerRegistered_);
    ASSERT_EQ(nullptr, sm_->mockSessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sm_->sessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sm_->sceneSessionManagerProxy_);
}

/**
 * @tc.name: RegisterWindowManagerRecoverCallbackFunc
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, RegisterWindowManagerRecoverCallbackFunc, TestSize.Level1)
{
    auto testFunc = []() { return; };
    sm_->RegisterWindowManagerRecoverCallbackFunc(testFunc);
    ASSERT_NE(sm_->windowManagerRecoverFunc_, nullptr);
}

/**
 * @tc.name: SessionManager::GetInstance(int32_t userId)
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, GetInstanceMulti, TestSize.Level1)
{
    sptr<SessionManager> instance = nullptr;
    int32_t userId = -1;
    instance = &SessionManager::GetInstance(userId);
    ASSERT_NE(nullptr, instance);

    userId = 101;
    instance = &SessionManager::GetInstance(userId);
    ASSERT_NE(nullptr, instance);

    // branch overried
    instance = &SessionManager::GetInstance(userId);
    ASSERT_NE(nullptr, instance);
}

/**
 * @tc.name: RemoveSSMDeathRecipient
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, RemoveSSMDeathRecipient, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    // branch 1
    sm_->sceneSessionManagerProxy_ = nullptr;
    sm_->RemoveSSMDeathRecipient();

    // branch 2
    auto sceneProxy = sm_->GetSceneSessionManagerProxy();
    ASSERT_NE(nullptr, sceneProxy);
    sm_->RemoveSSMDeathRecipient();
}

/**
 * @tc.name: InitSessionManagerServiceProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, InitSessionManagerServiceProxy, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    auto ret = sm_->InitMockSMSProxy();
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 1
    sptr<IRemoteObject> object = new IRemoteObjectMocker();
    sptr<ISessionManagerService> smsProxy = iface_cast<ISessionManagerService>(object);
    ASSERT_NE(nullptr, smsProxy);

    sm_->sessionManagerServiceProxy_ = smsProxy;
    sm_->InitSessionManagerServiceProxy();
    ASSERT_NE(nullptr, sm_->sessionManagerServiceProxy_);

    // branch 2
    sm_->ClearSessionManagerProxy();
    sm_->InitSessionManagerServiceProxy();
    ASSERT_NE(nullptr, sm_->sessionManagerServiceProxy_);
}

/**
 * @tc.name: InitSceneSessionManagerProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, InitSceneSessionManagerProxy, TestSize.Level1)
{
    ASSERT_NE(nullptr, sm_);
    // branch 1
    sptr<IRemoteObject> object = new IRemoteObjectMocker();
    sptr<ISceneSessionManager> sceneProxy = iface_cast<ISceneSessionManager>(object);
    sm_->sceneSessionManagerProxy_ = sceneProxy;
    sm_->InitSceneSessionManagerProxy();

    // branch 2
    sm_->ClearSessionManagerProxy();
    sm_->InitSceneSessionManagerProxy();

    // branch 3, fake sessionManagerServiceProxy_
    sm_->ClearSessionManagerProxy();
    sptr<ISessionManagerService> sessionProxy = iface_cast<ISessionManagerService>(object);
    sm_->sessionManagerServiceProxy_ = sessionProxy;
    sm_->InitSceneSessionManagerProxy();

    // branch 4
    sm_->ClearSessionManagerProxy();
    sm_->InitSessionManagerServiceProxy();
    ASSERT_NE(nullptr, sm_->sessionManagerServiceProxy_);
    sm_->InitSceneSessionManagerProxy();
    ASSERT_NE(nullptr, sm_->sceneSessionManagerProxy_);
}

/**
 * @tc.name: RecoverSessionManagerService
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, RecoverSessionManagerService, TestSize.Level1)
{
    bool funcInvoked = false;
    sm_->RecoverSessionManagerService(nullptr);

    sm_->windowManagerRecoverFunc_ = [&]() { funcInvoked = true; };
    sm_->RecoverSessionManagerService(nullptr);
    EXPECT_NE(funcInvoked, false);
    sm_->windowManagerRecoverFunc_ = nullptr;
}

/**
 * @tc.name: SessionManagerServiceRecoverListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, SMSRecoverListener1, TestSize.Level1)
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;
    int32_t ret;
    uint32_t code = 0;
    auto listener = sptr<SessionManagerServiceRecoverListener>::MakeSptr(userId_);

    // branch 1
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);

    // branch 2: TRANS_ID_ON_SESSION_MANAGER_SERVICE_RECOVER
    data.WriteInterfaceToken(SessionManagerServiceRecoverListener::GetDescriptor());
    code = static_cast<uint32_t>(OHOS::Rosen::ISessionManagerServiceRecoverListener::
                                     SessionManagerServiceRecoverMessage::TRANS_ID_ON_SESSION_MANAGER_SERVICE_RECOVER);
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_NONE);
}

/**
 * @tc.name: SessionManagerServiceRecoverListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, SMSRecoverListener2, TestSize.Level1)
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;
    int32_t ret;
    uint32_t code = 0;
    int32_t wmsUserId = INVALID_USER_ID;
    int32_t screenId = DEFAULT_SCREEN_ID;
    bool isConnected = false;
    auto listener = sptr<SessionManagerServiceRecoverListener>::MakeSptr(userId_);

    // branch 3: TRANS_ID_ON_WMS_CONNECTION_CHANGED
    code = static_cast<uint32_t>(OHOS::Rosen::ISessionManagerServiceRecoverListener::
                                     SessionManagerServiceRecoverMessage::TRANS_ID_ON_WMS_CONNECTION_CHANGED);
    // branch 3-1
    data.WriteInterfaceToken(SessionManagerServiceRecoverListener::GetDescriptor());
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);

    // branch 3-2
    // Note: data should be override every time.
    data.WriteInterfaceToken(SessionManagerServiceRecoverListener::GetDescriptor());
    data.WriteInt32(wmsUserId);
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);

    // branch 3-3
    data.WriteInterfaceToken(SessionManagerServiceRecoverListener::GetDescriptor());
    data.WriteInt32(wmsUserId);
    data.WriteInt32(screenId);
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);
}

/**
 * @tc.name: SessionManagerServiceRecoverListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerTest, SMSRecoverListener3, TestSize.Level1)
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;
    int32_t ret;
    uint32_t code = 0;
    int32_t wmsUserId = INVALID_USER_ID;
    int32_t screenId = DEFAULT_SCREEN_ID;
    bool isConnected = false;
    auto listener = sptr<SessionManagerServiceRecoverListener>::MakeSptr(userId_);

    // branch 4: TRANS_ID_ON_WMS_CONNECTION_CHANGED
    code = static_cast<uint32_t>(OHOS::Rosen::ISessionManagerServiceRecoverListener::
                                     SessionManagerServiceRecoverMessage::TRANS_ID_ON_WMS_CONNECTION_CHANGED);
    data.WriteInterfaceToken(SessionManagerServiceRecoverListener::GetDescriptor());
    data.WriteInt32(wmsUserId);
    data.WriteInt32(screenId);
    data.WriteBool(isConnected);
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_NONE);

    // branch 5
    isConnected = true;
    data.WriteInterfaceToken(SessionManagerServiceRecoverListener::GetDescriptor());
    data.WriteInt32(wmsUserId);
    data.WriteInt32(screenId);
    data.WriteBool(isConnected);
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_NONE);

    // branch 6: default
    code = 10;
    data.WriteInterfaceToken(SessionManagerServiceRecoverListener::GetDescriptor());
    listener->OnRemoteRequest(code, data, reply, option);
    ASSERT_NE(nullptr, listener);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
