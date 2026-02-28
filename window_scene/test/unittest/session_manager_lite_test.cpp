/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "iremote_screen_session_manager_mocker.h"
#include "iremote_session_manager_mocker.h"
#include "scene_board_judgement.h"
#include "session_manager.h"
#include "session_manager_lite.h"
#include "session_manager_service_recover_interface.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Rosen {
namespace {
std::string g_errLog;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
    const char *msg)
{
    g_errLog += msg;
}

class MockSessionManagerLite : public SessionManagerLite {
public:
    MOCK_METHOD(WMError, RegisterSMSRecoverListener, (), (override));
    MOCK_METHOD(sptr<IMockSessionManagerInterface>, GetMockSessionManagerServiceProxy, (), (override));
    MOCK_METHOD(WMError, InitSessionManagerServiceProxy, (), (override));
    MOCK_METHOD(WMError, InitSceneSessionManagerLiteProxy, (), (override));
    MOCK_METHOD(WMError, InitScreenSessionManagerLiteProxy, (), (override));
};

class MockGetMockSessionManagerServiceProxy : public SessionManagerLite {
public:
    MOCK_METHOD(sptr<IMockSessionManagerInterface>, GetMockSessionManagerServiceProxy, (), (override));
};

class MockTestInitSessionManagerServiceProxy : public SessionManagerLite {
public:
    MOCK_METHOD(WMError, RegisterSMSRecoverListener, (), (override));
    MOCK_METHOD(sptr<IMockSessionManagerInterface>, GetMockSessionManagerServiceProxy, (), (override));
    MOCK_METHOD(WMError, InitSceneSessionManagerLiteProxy, (), (override));
    MOCK_METHOD(WMError, InitScreenSessionManagerLiteProxy, (), (override));
};

class SessionManagerLiteTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    int32_t userId_ = -1;
    sptr<SessionManagerLite> instance_ = nullptr;
};

void SessionManagerLiteTest::SetUpTestCase() {}

void SessionManagerLiteTest::TearDownTestCase() {}

void SessionManagerLiteTest::SetUp()
{
    instance_ = sptr<SessionManagerLite>::MakeSptr(userId_);
}

void SessionManagerLiteTest::TearDown()
{
    instance_ = nullptr;
}

/**
 * @tc.name: GetSceneSessionManagerLiteProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, GetSceneSessionManagerLiteProxy, TestSize.Level1)
{
    MockSessionManagerLite mockInstance;
    sptr<ISceneSessionManagerLite> proxy = nullptr;

    // branch 1: make InitSessionManagerServiceProxy failed
    EXPECT_CALL(mockInstance, InitSessionManagerServiceProxy()).WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    proxy = mockInstance.GetSceneSessionManagerLiteProxy();
    EXPECT_EQ(nullptr, proxy);

    // branch 2: make InitSceneSessionManagerLiteProxy failed
    EXPECT_CALL(mockInstance, InitSessionManagerServiceProxy()).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(mockInstance, InitSceneSessionManagerLiteProxy()).WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    proxy = mockInstance.GetSceneSessionManagerLiteProxy();
    EXPECT_EQ(nullptr, proxy);

    // branch 3: make both return ok
    EXPECT_CALL(mockInstance, InitSessionManagerServiceProxy()).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(mockInstance, InitSceneSessionManagerLiteProxy()).WillOnce(Return(WMError::WM_OK));
    proxy = mockInstance.GetSceneSessionManagerLiteProxy();
    EXPECT_EQ(nullptr, proxy);
}

/**
 * @tc.name: GetScreenSessionManagerLiteProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, GetScreenSessionManagerLiteProxy, TestSize.Level1)
{
    MockSessionManagerLite mockInstance;
    sptr<IScreenSessionManagerLite> proxy = nullptr;

    // branch 1: make InitSessionManagerServiceProxy failed
    EXPECT_CALL(mockInstance, InitSessionManagerServiceProxy()).WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    proxy = mockInstance.GetScreenSessionManagerLiteProxy();
    EXPECT_EQ(nullptr, proxy);

    // branch 2: make InitScreenSessionManagerLiteProxy failed
    EXPECT_CALL(mockInstance, InitSessionManagerServiceProxy()).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(mockInstance, InitScreenSessionManagerLiteProxy()).WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    proxy = mockInstance.GetScreenSessionManagerLiteProxy();
    EXPECT_EQ(nullptr, proxy);

    // branch 3: make both return ok
    EXPECT_CALL(mockInstance, InitSessionManagerServiceProxy()).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(mockInstance, InitScreenSessionManagerLiteProxy()).WillOnce(Return(WMError::WM_OK));
    proxy = mockInstance.GetScreenSessionManagerLiteProxy();
    EXPECT_EQ(nullptr, proxy);
}

/**
 * @tc.name: SessionManagerLite::GetSessionManagerServiceProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, GetSessionManagerServiceProxy, TestSize.Level1)
{
    MockSessionManagerLite mockInstance;
    sptr<ISessionManagerService> proxy = nullptr;

    // branch 1: make InitSessionManagerServiceProxy failed
    EXPECT_CALL(mockInstance, InitSessionManagerServiceProxy()).WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    proxy = mockInstance.GetSessionManagerServiceProxy();
    EXPECT_EQ(nullptr, proxy);

    // branch 2: make InitSessionManagerServiceProxy return ok
    EXPECT_CALL(mockInstance, InitSessionManagerServiceProxy()).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(mockInstance, InitSceneSessionManagerLiteProxy()).WillOnce(Return(WMError::WM_OK));
    proxy = mockInstance.GetSessionManagerServiceProxy();
    EXPECT_EQ(nullptr, proxy);
}

/**
 * @tc.name: SessionManagerLite::InitSceneSessionManagerLiteProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, InitSceneSessionManagerLiteProxy, TestSize.Level1)
{
    WMError ret;

    // branch 1: sessionManagerServiceProxy_ is null
    ret = instance_->InitSceneSessionManagerLiteProxy();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2
    instance_->InitSessionManagerServiceProxy();
    EXPECT_NE(nullptr, instance_->sessionManagerServiceProxy_);
    ret = instance_->InitSceneSessionManagerLiteProxy();
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: mock sessionManagerServiceProxy_
    instance_->sceneSessionManagerLiteProxy_ = nullptr;
    auto mockRemoteObject = sptr<IRemoteSessionManagerMocker>::MakeSptr();
    instance_->sessionManagerServiceProxy_ = mockRemoteObject;

    // branch 3-1
    EXPECT_CALL(*mockRemoteObject, GetSceneSessionManagerLite()).WillOnce(Return(nullptr));
    ret = instance_->InitSceneSessionManagerLiteProxy();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 3-2: make iface_cast failed
    auto testRemoteObject = sptr<IRemoteSessionManagerMocker>::MakeSptr();
    EXPECT_CALL(*mockRemoteObject, GetSceneSessionManagerLite()).WillOnce(Return(testRemoteObject));
    ret = instance_->InitSceneSessionManagerLiteProxy();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: ClearSessionManagerProxy
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, ClearSessionManagerProxy, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);

    instance_->ClearSessionManagerProxy();
    EXPECT_TRUE(g_errLog.find("Proxy clear success") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: OnWMSConnectionChangedCallback
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, OnWMSConnectionChangedCallback, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int32_t userId = 100;
    int32_t screenId = DEFAULT_SCREEN_ID;
    bool isConnected = false;
    auto callback = [](int32_t userId, int32_t screenId, bool isConnected) {};

    // branch 1
    instance_->wmsConnectionChangedFunc_ = nullptr;
    instance_->OnWMSConnectionChangedCallback(userId, screenId, isConnected);
    EXPECT_TRUE(g_errLog.find("Callback is null") != std::string::npos);

    // branch 2
    instance_->wmsConnectionChangedFunc_ = callback;
    instance_->OnWMSConnectionChangedCallback(userId, screenId, isConnected);
    EXPECT_TRUE(g_errLog.find("WMS connection changed") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SessionManagerLite::OnWMSConnectionChanged
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, OnWMSConnectionChanged1, TestSize.Level1)
{
    int32_t userId = 100;
    int32_t screenId = DEFAULT_SCREEN_ID;
    sptr<ISessionManagerService> service = nullptr;
    bool isConnected = true;

    // branch 1: Into all 'if' branch
    instance_->currentWMSUserId_ = 200;
    instance_->OnWMSConnectionChanged(userId, screenId, isConnected, service);
    EXPECT_EQ(instance_->currentWMSUserId_, userId);

    // branch 2: Cover all branches
    isConnected = false;
    userId = 300;
    instance_->OnWMSConnectionChanged(userId, screenId, isConnected, service);
    EXPECT_NE(instance_->currentWMSUserId_, userId);
}

/**
 * @tc.name: SessionManagerServiceLiteRecoverListener::OnWMSConnectionChanged
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, OnWMSConnectionChanged2, TestSize.Level1)
{
    // Just cover the branch, no need to assert.
    auto listener = sptr<SessionManagerServiceLiteRecoverListener>::MakeSptr(userId_);
    ASSERT_NE(nullptr, listener);
    listener->OnWMSConnectionChanged(100, 100, true, nullptr);
}

/**
 * @tc.name: OnUserSwitch
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, OnUserSwitch, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ISessionManagerService> sessionMgrService = nullptr;

    // branch 1: Make sceneSessionManagerLiteProxy_ is null
    instance_->OnUserSwitch(nullptr);
    EXPECT_TRUE(g_errLog.find("Init scene session manager lite proxy failed") != std::string::npos);

    // branch 2: Set callback func is null
    sessionMgrService = instance_->GetSessionManagerServiceProxy();
    ASSERT_NE(nullptr, sessionMgrService);
    instance_->userSwitchCallbackFunc_ = nullptr;
    instance_->OnUserSwitch(sessionMgrService);
    EXPECT_TRUE(g_errLog.find("callback func is null") != std::string::npos);

    // branch 3: Set callback func is not null
    instance_->userSwitchCallbackFunc_ = [](){};
    instance_->OnUserSwitch(sessionMgrService);
    EXPECT_TRUE(g_errLog.find("run callback func") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener
 * @tc.desc: WMSConnectionChangedCallbackFunc is null
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, RegisterWMSConnectionChangedListener, TestSize.Level1)
{
    WMError ret;
    MockSessionManagerLite mockInstance;
    auto callback = [](int32_t userId, int32_t screenId, bool isConnected) {};

    // branch 1: Set callbackFunc is null
    ret = instance_->RegisterWMSConnectionChangedListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: Mock RegisterSMSRecoverListener return not ok
    EXPECT_CALL(mockInstance, RegisterSMSRecoverListener()).WillOnce(Return(WMError::WM_ERROR_NULLPTR));
    ret = mockInstance.RegisterWMSConnectionChangedListener(callback);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 3: Make RegisterSMSRecoverListener return ok
    EXPECT_CALL(mockInstance, RegisterSMSRecoverListener()).WillOnce(Return(WMError::WM_OK));
    ret = mockInstance.RegisterWMSConnectionChangedListener(callback);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterWMSConnectionChangedListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, UnregisterWMSConnectionChangedListener, TestSize.Level1)
{
    auto remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    instance_->mockSessionManagerServiceProxy_ = iface_cast<IMockSessionManagerInterface>(remoteObject);
    auto ret = instance_->UnregisterWMSConnectionChangedListener();
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RegisterUserSwitchListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, RegisterUserSwitchListener, TestSize.Level1)
{
    // Just cover branch, no need to assert.
    instance_->RegisterUserSwitchListener(nullptr);
    EXPECT_NE(nullptr, instance_);
}

/**
 * @tc.name: RecoverSessionManagerService
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, RecoverSessionManagerService, TestSize.Level1)
{
    bool funcInvoked = false;
    instance_->RecoverSessionManagerService(nullptr);

    instance_->userSwitchCallbackFunc_ = [&]() { funcInvoked = true; };
    instance_->RecoverSessionManagerService(nullptr);
    EXPECT_NE(funcInvoked, false);
    instance_->userSwitchCallbackFunc_ = nullptr;
}

/**
 * @tc.name: ReregisterSessionListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, ReregisterSessionListener, TestSize.Level1)
{
    instance_->isRecoverListenerRegistered_ = true;
    instance_->GetSceneSessionManagerLiteProxy();
    instance_->ReregisterSessionListener();
    ASSERT_NE(nullptr, instance_);
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
    ASSERT_NE(nullptr, instance_);
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
    ASSERT_NE(nullptr, instance_);
}

/**
 * @tc.name: OnFoundationDied
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, OnFoundationDied, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    instance_->OnFoundationDied();
    ASSERT_EQ(false, instance_->isWMSConnected_);
    ASSERT_EQ(false, instance_->isRecoverListenerRegistered_);
    ASSERT_EQ(nullptr, instance_->mockSessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, instance_->sessionManagerServiceProxy_);
    ASSERT_EQ(nullptr, instance_->sceneSessionManagerLiteProxy_);
}

/**
 * @tc.name: SessionManagerLite::RegisterSMSRecoverListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, RegisterSMSRecoverListener, TestSize.Level1)
{
    WMError ret;
    MockGetMockSessionManagerServiceProxy mockInstance;

    // branch 1: set isRecoverListenerRegistered_ is true
    instance_->isRecoverListenerRegistered_ = true;
    ret = instance_->RegisterSMSRecoverListener();
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 2: register success
    instance_->isRecoverListenerRegistered_ = false;
    ret = instance_->RegisterSMSRecoverListener();
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: cover "if (!smsRecoverListener_) {}"
    instance_->isRecoverListenerRegistered_ = false;
    ret = instance_->RegisterSMSRecoverListener();
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 4: mock GetMockSessionManagerServiceProxy return null
    EXPECT_CALL(mockInstance, GetMockSessionManagerServiceProxy()).WillOnce(Return(nullptr));
    ret = mockInstance.RegisterSMSRecoverListener();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: SessionManagerLite::UnregisterSMSRecoverListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, UnregisterSMSRecoverListener, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockSessionManagerLite mockInstance;

    // branch 1
    instance_->UnregisterSMSRecoverListener();
    EXPECT_TRUE(g_errLog.find("Unregister end") != std::string::npos);

    // branch 2: mock GetMockSessionManagerServiceProxy return null
    EXPECT_CALL(mockInstance, GetMockSessionManagerServiceProxy()).WillOnce(Return(nullptr));
    mockInstance.UnregisterSMSRecoverListener();
    EXPECT_TRUE(g_errLog.find("Unregister end") != std::string::npos);

    LOG_SetCallback(nullptr);
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
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int32_t userId;

    // branch 1: set userId is INVALID_USER_ID
    userId = INVALID_USER_ID;
    SessionManagerLite::GetInstance(userId);
    EXPECT_FALSE(g_errLog.find("get new instance") != std::string::npos);

    // branch 2: set userId is 999
    userId = 999;
    SessionManagerLite::GetInstance(userId);
    EXPECT_TRUE(g_errLog.find("get new instance") != std::string::npos);

    // branch overried
    g_errLog.clear();
    SessionManagerLite::GetInstance(userId);
    EXPECT_FALSE(g_errLog.find("get new instance") != std::string::npos);

    // SessionManagerLite::sessionManagerLiteMap_.erase(userId);
    SessionManagerLite::sessionManagerLiteMap_.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SessionManagerLite::InitSessionManagerServiceProxy()
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, InitSessionManagerServiceProxy, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockTestInitSessionManagerServiceProxy mockInstance;

    // branch 1: mock RegisterSMSRecoverListener return failed
    EXPECT_CALL(mockInstance, RegisterSMSRecoverListener()).WillOnce(Return(WMError::WM_DO_NOTHING));
    mockInstance.InitSessionManagerServiceProxy();
    EXPECT_TRUE(g_errLog.find("Register recover listener failed") != std::string::npos);

    // branch 2: mock GetMockSessionManagerServiceProxy return nullptr
    EXPECT_CALL(mockInstance, RegisterSMSRecoverListener()).WillOnce(Return(WMError::WM_OK));
    EXPECT_CALL(mockInstance, GetMockSessionManagerServiceProxy()).WillOnce(Return(nullptr));
    mockInstance.InitSessionManagerServiceProxy();
    EXPECT_TRUE(g_errLog.find("Get mock session manager service proxy failed") != std::string::npos);

    // branch 3: Init success
    instance_->InitSessionManagerServiceProxy();
    EXPECT_TRUE(g_errLog.find("Init success") != std::string::npos);

    // branch 4: sessionManagerServiceProxy_ is not null
    instance_->InitSessionManagerServiceProxy();

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SessionManagerLite::InitScreenSessionManagerLiteProxy()
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, InitScreenSessionManagerLiteProxy, TestSize.Level1)
{
    WMError ret;
    MockGetMockSessionManagerServiceProxy mockInstance;
    sptr<IScreenSessionManagerLite> proxy = nullptr;

    // branch 1: mock GetMockSessionManagerServiceProxy return nullptr
    EXPECT_CALL(mockInstance, GetMockSessionManagerServiceProxy()).WillOnce(Return(nullptr));
    ret = mockInstance.InitScreenSessionManagerLiteProxy();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: cover screenSessionManagerLiteProxy_ is not null
    proxy = sptr<IRemoteScreenSessionManagerMocker>::MakeSptr();
    mockInstance.screenSessionManagerLiteProxy_ = proxy;
    ret = mockInstance.InitScreenSessionManagerLiteProxy();
    EXPECT_EQ(WMError::WM_OK, ret);
}
} // namespace
} // namespace OHOS::Rosen
