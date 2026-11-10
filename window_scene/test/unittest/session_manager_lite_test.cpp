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
#include <unistd.h>
#include "iremote_object_mocker.h"
#include "iremote_screen_session_manager_mocker.h"
#include "iremote_session_manager_mocker.h"
#include "parameters.h"
#include "scene_board_judgement.h"
#include "scene_session_manager.h"
#include "scene_session_manager_lite.h"
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

// IRemoteObject mock that only observes the transaction code routed by the client:
// the callback proxy (iface_cast<IMockSessionManagerInterface>) forwards every method call to
// SendRequest; GetSessionManagerService is [ipccode 0] and GetSessionManagerServiceByUserId
// (implicit next code) is 1 per wmserver/IMockSessionManagerInterface.idl:20-21.
class MockSmsSendRequestRemote : public IRemoteObject {
public:
    MockSmsSendRequestRemote() : IRemoteObject{u"MockSmsSendRequestRemote"} {}
    ~MockSmsSendRequestRemote() override = default;

    MOCK_METHOD(int, SendRequest, (uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option), (override));
    int32_t GetObjectRefCount() override { return 0; }
    bool IsProxyObject() const override { return true; }
    bool IsObjectDead() const override { return false; }
    bool CheckObjectLegality() const override { return true; }
    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient) override { return true; }
    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient) override { return true; }
    sptr<IRemoteBroker> AsInterface() override { return nullptr; }
    int Dump(int fd, const std::vector<std::u16string>& args) override { return 0; }
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
    std::string isConcurrentuser_;
};

void SessionManagerLiteTest::SetUpTestCase() {}

void SessionManagerLiteTest::TearDownTestCase() {}

void SessionManagerLiteTest::SetUp()
{
    isConcurrentuser_ = OHOS::system::GetParameter("persist.dms.concurrentuser", "");
    OHOS::system::SetParameter("persist.dms.concurrentuser", "true");
    instance_ = sptr<SessionManagerLite>::MakeSptr(userId_);
}

void SessionManagerLiteTest::TearDown()
{
    SessionManagerLite::sessionManagerLiteMap_.clear();
    OHOS::system::SetParameter("persist.dms.concurrentuser", isConcurrentuser_);
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
    int32_t pid = 12345;
    auto callback = [](int32_t userId, int32_t screenId, bool isConnected, int32_t pid) {};

    // branch 1
    instance_->wmsConnectionChangedFunc_ = nullptr;
    instance_->OnWMSConnectionChangedCallback(userId, screenId, isConnected, pid);
    EXPECT_TRUE(g_errLog.find("callback func is null") != std::string::npos);

    // branch 2
    instance_->wmsConnectionChangedFunc_ = callback;
    instance_->OnWMSConnectionChangedCallback(userId, screenId, isConnected, pid);
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
    int32_t pid = 12345;

    // branch 1: Into all 'if' branch
    instance_->currentServer_.userId = 200;
    instance_->OnWMSConnectionChanged(userId, screenId, isConnected, pid, INVALID_USER_ID, INVALID_PID, service);
    EXPECT_EQ(instance_->currentServer_.userId, userId);

    // branch 2: Cover all branches
    isConnected = false;
    userId = 300;
    instance_->OnWMSConnectionChanged(userId, screenId, isConnected, pid, INVALID_USER_ID, INVALID_PID, service);
    EXPECT_NE(instance_->currentServer_.userId, userId);
}

/**
 * @tc.name: SessionManagerServiceLiteRecoverListener::OnWMSConnectionChanged
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, OnWMSConnectionChanged2, TestSize.Level1)
{
    // Just cover branch, no need to assert.
    auto listener = sptr<SessionManagerServiceLiteRecoverListener>::MakeSptr(userId_);
    ASSERT_NE(nullptr, listener);
    listener->OnWMSConnectionChanged(100, 100, true, 12345, INVALID_USER_ID, INVALID_PID, nullptr);
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
    auto callback = [](int32_t userId, int32_t screenId, bool isConnected, int32_t pid) {};

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
    int32_t pid = -1;
    int32_t fromUserId = INVALID_USER_ID;
    int32_t fromPid = INVALID_PID;
    auto listener = sptr<SessionManagerServiceLiteRecoverListener>::MakeSptr(userId_);

    // branch 4: TRANS_ID_ON_WMS_CONNECTION_CHANGED
    code = static_cast<uint32_t>(OHOS::Rosen::ISessionManagerServiceRecoverListener::
                                     SessionManagerServiceRecoverMessage::TRANS_ID_ON_WMS_CONNECTION_CHANGED);
    data.WriteInterfaceToken(SessionManagerServiceLiteRecoverListener::GetDescriptor());
    data.WriteInt32(wmsUserId);
    data.WriteInt32(screenId);
    data.WriteBool(isConnected);
    data.WriteInt32(pid);
    data.WriteInt32(fromUserId);
    data.WriteInt32(fromPid);
    ret = listener->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_NONE);

    // branch 5
    isConnected = true;
    data.WriteInterfaceToken(SessionManagerServiceLiteRecoverListener::GetDescriptor());
    data.WriteInt32(wmsUserId);
    data.WriteInt32(screenId);
    data.WriteBool(isConnected);
    data.WriteInt32(pid);
    data.WriteInt32(fromUserId);
    data.WriteInt32(fromPid);
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
 * @tc.name: SessionManagerLite::InitSessionManagerServiceProxyRouteCrossUserNonU0
 * @tc.desc: Cross-user non-U0 request must stay on GetSessionManagerServiceByUserId
 *           (server :322-325 keeps rejecting non-U0 cross-user with ERR_WOULD_BLOCK). Uses a real
 *           instance so the real guard (getuid-derived user vs userId_) is exercised.
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, InitSessionManagerServiceProxyRouteCrossUserNonU0, TestSize.Level1)
{
    int32_t selfUser = GetUserIdByUid(getuid());
    int32_t crossUser = selfUser + 1024; // anything > 0 and != selfUser, valid in any process user
    auto remote = sptr<MockSmsSendRequestRemote>::MakeSptr();
    ASSERT_NE(nullptr, remote);
    auto instance = sptr<SessionManagerLite>::MakeSptr(crossUser);
    ASSERT_NE(nullptr, instance);
    instance->isRecoverListenerRegistered_ = true;
    instance->mockSessionManagerServiceProxy_ = iface_cast<IMockSessionManagerInterface>(remote);
    ASSERT_NE(nullptr, instance->mockSessionManagerServiceProxy_);

    // Real guard: crossUser != GetUserIdByUid(getuid()) -> false -> ByUserId kept.
    EXPECT_CALL(*remote, SendRequest(0, _, _, _)).Times(0);
    EXPECT_CALL(*remote, SendRequest(1, _, _, _)).Times(1); // ByUserId(crossUser)
    auto ret = instance->InitSessionManagerServiceProxy();
    EXPECT_NE(WMError::WM_OK, ret);
}

/**
 * @tc.name: SessionManagerLite::InitSessionManagerServiceProxyU0ExplicitZeroGuard
 * @tc.desc: Mandatory guard regression: a U0 caller carrying explicit userId_=0 must keep
 *           routing to GetSessionManagerServiceByUserId(0) (server :322-325 U0 branch -> Inner(0)),
 *           never to the no-arg path. Without the guard, 0 == GetUserIdByUid(getuid()) would hold
 *           for a U0 process and mis-route to no-arg, where the server :300-304 redirects
 *           clientUserId == SYSTEM_USERID to defaultWMSUserId_ (multi-user semantic drift).
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, InitSessionManagerServiceProxyU0ExplicitZeroGuard, TestSize.Level1)
{
    auto remote = sptr<MockSmsSendRequestRemote>::MakeSptr();
    ASSERT_NE(nullptr, remote);
    auto instance = sptr<SessionManagerLite>::MakeSptr(SYSTEM_USERID); // U0 explicit userId_=0
    ASSERT_NE(nullptr, instance);
    instance->isRecoverListenerRegistered_ = true;
    instance->mockSessionManagerServiceProxy_ = iface_cast<IMockSessionManagerInterface>(remote);
    ASSERT_NE(nullptr, instance->mockSessionManagerServiceProxy_);

    // Real guard in any process user: userId_ != SYSTEM_USERID is false -> self-route denied.
    EXPECT_CALL(*remote, SendRequest(0, _, _, _)).Times(0); // NOT redirected to no-arg/defaultWMSUserId_
    EXPECT_CALL(*remote, SendRequest(1, _, _, _)).Times(1); // explicit ByUserId(0), U0 branch Inner(0)
    auto ret = instance->InitSessionManagerServiceProxy();
    EXPECT_NE(WMError::WM_OK, ret);
}

/**
 * @tc.name: SessionManagerLite::InitSessionManagerServiceProxyRouteDefaultInstance
 * @tc.desc: Behavior invariance: default instance (userId_ == INVALID_USER_ID) keeps the
 *           baseline first branch -- no-arg GetSessionManagerService(), guard not consulted
 *           (short-circuit).
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, InitSessionManagerServiceProxyRouteDefaultInstance, TestSize.Level1)
{
    auto remote = sptr<MockSmsSendRequestRemote>::MakeSptr();
    ASSERT_NE(nullptr, remote);
    auto instance = sptr<SessionManagerLite>::MakeSptr(INVALID_USER_ID); // default instance
    ASSERT_NE(nullptr, instance);
    instance->isRecoverListenerRegistered_ = true;
    instance->mockSessionManagerServiceProxy_ = iface_cast<IMockSessionManagerInterface>(remote);
    ASSERT_NE(nullptr, instance->mockSessionManagerServiceProxy_);

    EXPECT_CALL(*remote, SendRequest(0, _, _, _)).Times(1); // no-arg GetSessionManagerService
    EXPECT_CALL(*remote, SendRequest(1, _, _, _)).Times(0);
    auto ret = instance->InitSessionManagerServiceProxy();
    EXPECT_NE(WMError::WM_OK, ret);
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

/**
 * @tc.name: NotifyStackRecoverFromAMS01
 * @tc.desc: test NotifyStackRecoverFromAMS with valid persistentId
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, NotifyStackRecoverFromAMS01, TestSize.Level1)
{
    int32_t persistentId = 100;
    bool isStackRecover = true;
    auto ret = SceneSessionManagerLite::GetInstance().NotifyStackRecoverFromAMS(persistentId, isStackRecover);
    auto expectRet = SceneSessionManager::GetInstance().NotifyStackRecoverFromAMS(persistentId, isStackRecover);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: NotifyStackRecoverFromAMS02
 * @tc.desc: test NotifyStackRecoverFromAMS with isStackRecover false
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, NotifyStackRecoverFromAMS02, TestSize.Level1)
{
    int32_t persistentId = 200;
    bool isStackRecover = false;
    auto ret = SceneSessionManagerLite::GetInstance().NotifyStackRecoverFromAMS(persistentId, isStackRecover);
    auto expectRet = SceneSessionManager::GetInstance().NotifyStackRecoverFromAMS(persistentId, isStackRecover);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: NotifyStackRecoverFromAMS03
 * @tc.desc: test NotifyStackRecoverFromAMS with invalid persistentId
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, NotifyStackRecoverFromAMS03, TestSize.Level1)
{
    int32_t persistentId = -1;
    bool isStackRecover = true;
    auto ret = SceneSessionManagerLite::GetInstance().NotifyStackRecoverFromAMS(persistentId, isStackRecover);
    auto expectRet = SceneSessionManager::GetInstance().NotifyStackRecoverFromAMS(persistentId, isStackRecover);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: NotifyStackRecoverFromAMS04
 * @tc.desc: test NotifyStackRecoverFromAMS with zero persistentId
 * @tc.type: FUNC
 */
HWTEST_F(SessionManagerLiteTest, NotifyStackRecoverFromAMS04, TestSize.Level1)
{
    int32_t persistentId = 0;
    bool isStackRecover = false;
    auto ret = SceneSessionManagerLite::GetInstance().NotifyStackRecoverFromAMS(persistentId, isStackRecover);
    auto expectRet = SceneSessionManager::GetInstance().NotifyStackRecoverFromAMS(persistentId, isStackRecover);
    EXPECT_EQ(ret, expectRet);
}
} // namespace
} // namespace OHOS::Rosen
