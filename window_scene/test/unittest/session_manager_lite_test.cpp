/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
class SessionManagerLiteTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    sptr<SessionManagerLite> sml_ = nullptr;
    int32_t userId_ = 100;
};

void SessionManagerLiteTest::SetUpTestCase() {}

void SessionManagerLiteTest::TearDownTestCase() {}

void SessionManagerLiteTest::SetUp()
{
    sml_ = &SessionManagerLite::GetInstance(userId_);
}

void SessionManagerLiteTest::TearDown()
{
    sml_ = nullptr;
}

namespace {
/**
 * @tc.name: RecoverSessionManagerService
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, RecoverSessionManagerService, TestSize.Level1)
{
    bool funcInvoked = false;
    sml_->RecoverSessionManagerService(nullptr);

    sml_->userSwitchCallbackFunc_ = [&]() { funcInvoked = true; };
    sml_->RecoverSessionManagerService(nullptr);
    EXPECT_NE(funcInvoked, false);
    sml_->userSwitchCallbackFunc_ = nullptr;
}

/**
 * @tc.name: ReregisterSessionListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, ReregisterSessionListener, TestSize.Level1)
{
    sml_->recoverListenerRegistered_ = true;
    sml_->GetSceneSessionManagerLiteProxy();
    sml_->ReregisterSessionListener();
    ASSERT_NE(nullptr, sml_);
}

/**
 * @tc.name: FoundationDeathRecipientLite::OnRemoteDied
 * @tc.desc: foundation died
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, OnRemoteDied1, TestSize.Level1)
{
    sptr<FoundationDeathRecipientLite> deathRecipient = new FoundationDeathRecipientLite(userId_);
    sptr<IRemoteObject> sptrDeath = sptr<IRemoteObjectMocker>::MakeSptr();
    wptr<IRemoteObject> wptrDeath = wptr(sptrDeath);
    deathRecipient->OnRemoteDied(wptrDeath);
    ASSERT_NE(nullptr, sml_);
}

/**
 * @tc.name: OnRemoteDied2
 * @tc.desc: scb died
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, OnRemoteDied2, TestSize.Level1)
{
    sptr<SSMDeathRecipientLite> deathRecipient = new SSMDeathRecipientLite(userId_);
    sptr<IRemoteObject> sptrDeath = sptr<IRemoteObjectMocker>::MakeSptr();
    wptr<IRemoteObject> wptrDeath = wptr(sptrDeath);
    deathRecipient->OnRemoteDied(wptrDeath);
    ASSERT_NE(nullptr, sml_);
}

/**
 * @tc.name: OnFoundationDied
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, OnFoundationDied, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->OnFoundationDied();
    ASSERT_EQ(false, sml_->isWMSConnected_);
    ASSERT_EQ(false, sml_->isFoundationListenerRegistered_);
    ASSERT_EQ(false, sml_->recoverListenerRegistered_);
    ASSERT_EQ(nullptr, sml_->mockSessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sml_->sessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, sml_->sceneSessionManagerLiteProxy_);
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener
 * @tc.desc: WMSConnectionChangedCallbackFunc is null
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, RegisterWMSConnectionChangedListener, TestSize.Level1)
{
    auto ret = sml_->RegisterWMSConnectionChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener1
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, RegisterWMSConnectionChangedListener1, TestSize.Level1)
{
    sml_->recoverListenerRegistered_ = true;
    sml_->currentWMSUserId_ = 100;
    sml_->currentScreenId_ = 0;
    sml_->isWMSConnected_ = true;
    auto callbackFunc = [](int32_t userId, int32_t screenId, bool isConnected) {};
    auto ret = sml_->RegisterWMSConnectionChangedListener(callbackFunc);
    ASSERT_NE(nullptr, sml_->wmsConnectionChangedFunc_);
}

/**
 * @tc.name: RegisterSMSRecoverListener1
 * @tc.desc: mockSessionManagerServiceProxy_ is null
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, RegisterSMSRecoverListener1, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->recoverListenerRegistered_ = false;
    sml_->mockSessionManagerServiceProxy_ = nullptr;
    sml_->RegisterSMSRecoverListener();
    ASSERT_EQ(sml_->recoverListenerRegistered_, false);
}

/**
 * @tc.name: RegisterSMSRecoverListener2
 * @tc.desc: normal test
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, RegisterSMSRecoverListener2, TestSize.Level1)
{
    ASSERT_NE(nullptr, sml_);
    sml_->recoverListenerRegistered_ = false;

    // lite init mock not success now
    sml_->InitMockSMSProxy();
    sml_->RegisterSMSRecoverListener();
}

/**
 * @tc.name: SessionManagerServiceLiteRecoverListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, SMSRecoverListener1, TestSize.Level1)
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;
    int32_t ret;
    uint32_t code = 0;
    auto listener = sptr<SessionManagerServiceLiteRecoverListener>::MakeSptr(userId_);

    // branch 1
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);

    // branch 2: TRANS_ID_ON_SESSION_MANAGER_SERVICE_RECOVER
    data.WriteInterfaceToken(SessionManagerServiceLiteRecoverListener::GetDescriptor());
    code = static_cast<uint32_t>(OHOS::Rosen::ISessionManagerServiceRecoverListener::
                                     SessionManagerServiceRecoverMessage::TRANS_ID_ON_SESSION_MANAGER_SERVICE_RECOVER);
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_NONE);
}

/**
 * @tc.name: SessionManagerServiceLiteRecoverListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, SMSRecoverListener2, TestSize.Level1)
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;
    int32_t ret;
    uint32_t code = 0;
    int32_t wmsUserId = INVALID_USER_ID;
    int32_t screenId = DEFAULT_SCREEN_ID;
    bool isConnected = false;
    auto listener = sptr<SessionManagerServiceLiteRecoverListener>::MakeSptr(userId_);

    // branch 3: TRANS_ID_ON_WMS_CONNECTION_CHANGED
    code = static_cast<uint32_t>(OHOS::Rosen::ISessionManagerServiceRecoverListener::
                                     SessionManagerServiceRecoverMessage::TRANS_ID_ON_WMS_CONNECTION_CHANGED);
    // branch 3-1
    data.WriteInterfaceToken(SessionManagerServiceLiteRecoverListener::GetDescriptor());
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);

    // branch 3-2
    // Note: data should be override every time.
    data.WriteInterfaceToken(SessionManagerServiceLiteRecoverListener::GetDescriptor());
    data.WriteInt32(wmsUserId);
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);

    // branch 3-3
    data.WriteInterfaceToken(SessionManagerServiceLiteRecoverListener::GetDescriptor());
    data.WriteInt32(wmsUserId);
    data.WriteInt32(screenId);
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_TRANSACTION_FAILED);
}

/**
 * @tc.name: SessionManagerServiceLiteRecoverListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, SMSRecoverListener3, TestSize.Level1)
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;
    int32_t ret;
    uint32_t code = 0;
    int32_t wmsUserId = INVALID_USER_ID;
    int32_t screenId = DEFAULT_SCREEN_ID;
    bool isConnected = false;
    auto listener = sptr<SessionManagerServiceLiteRecoverListener>::MakeSptr(userId_);

    // branch 4: TRANS_ID_ON_WMS_CONNECTION_CHANGED
    code = static_cast<uint32_t>(OHOS::Rosen::ISessionManagerServiceRecoverListener::
                                     SessionManagerServiceRecoverMessage::TRANS_ID_ON_WMS_CONNECTION_CHANGED);
    data.WriteInterfaceToken(SessionManagerServiceLiteRecoverListener::GetDescriptor());
    data.WriteInt32(wmsUserId);
    data.WriteInt32(screenId);
    data.WriteBool(isConnected);
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_NONE);

    // branch 5
    isConnected = true;
    data.WriteInterfaceToken(SessionManagerServiceLiteRecoverListener::GetDescriptor());
    data.WriteInt32(wmsUserId);
    data.WriteInt32(screenId);
    data.WriteBool(isConnected);
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_NONE);

    // branch 6: default
    code = 10;
    data.WriteInterfaceToken(SessionManagerServiceLiteRecoverListener::GetDescriptor());
    listener->OnRemoteRequest(code, data, reply, option);
    ASSERT_NE(nullptr, listener);
}

/**
 * @tc.name: SessionManagerLite::GetInstance(int32_t userId)
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, GetInstanceMulti, TestSize.Level1)
{
    sptr<SessionManagerLite> instance = nullptr;
    int32_t userId = 101;

    instance = &SessionManagerLite::GetInstance(userId);
    ASSERT_NE(nullptr, instance);

    // branch overried
    instance = &SessionManagerLite::GetInstance(userId);
    ASSERT_NE(nullptr, instance);
}

/**
 * @tc.name: SessionManagerLite::InitSessionManagerServiceProxy()
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, InitSessionManagerServiceProxy, TestSize.Level1)
{
    sptr<SessionManagerLite> instance = &SessionManagerLite::GetInstance(100);
    auto ret = instance->InitMockSMSProxy();

    ASSERT_EQ(WMError::WM_OK, ret);
    instance->InitSessionManagerServiceProxy();
}
} // namespace
} // namespace Rosen
} // namespace OHOS
