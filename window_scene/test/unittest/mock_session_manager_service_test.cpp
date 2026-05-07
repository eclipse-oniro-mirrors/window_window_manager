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

#define private public
#define protected public
#include "mock_session_manager_service.h"
#undef private
#undef protected

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "iremote_object_mocker.h"

#include "mock_accesstoken_kit.h"
#include "scene_session_manager.h"
#include "scene_session_manager_lite.h"
#include "session_manager_service_proxy.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class MockMockSessionManagerService : public MockSessionManagerService {
public:
    MOCK_METHOD(sptr<IRemoteObject>, GetSceneSessionManagerByUserId, (int32_t), (override));
    MOCK_METHOD(ErrCode,
                NotifySCBSnapshotSkipByUserIdAndBundleNames,
                (int32_t, const std::vector<std::string>&, const sptr<IRemoteObject>&),
                (override));
    MOCK_METHOD(int32_t, GetUserIdByCallingUid, (), (override));
    MOCK_METHOD(sptr<IRemoteObject>, GetSessionManagerServiceInner, (int32_t), (override));
    MOCK_METHOD(ErrCode, CheckClientIsSystemUser, (), (override));
    MOCK_METHOD(sptr<IRemoteObject>, GetSceneSessionManagerInner, (int32_t, bool), (override));
    MOCK_METHOD(sptr<IRemoteObject>, GetSceneSessionManager, (), (override));
};

class MockRecoverListener : public ISessionManagerServiceRecoverListener {
public:
    MOCK_METHOD0(AsObject, sptr<IRemoteObject>());
    MOCK_METHOD1(OnSessionManagerServiceRecover, void(const sptr<IRemoteObject>&));
    MOCK_METHOD5(OnWMSConnectionChanged,
                 void(int32_t userId,
                      int32_t screenId,
                      bool isConnected,
                      const sptr<IRemoteObject>& sessionManagerService,
                      int32_t pid));
};

namespace {
std::string g_errLog;
void MyLogCallback(const LogType type,
                   const LogLevel level,
                   const unsigned int domain,
                   const char* tag,
                   const char* msg)
{
    g_errLog += msg;
}

/**
 * @tc.name: SetSnapshotSkipByUserIdAndBundleNamesInner
 * @tc.desc: test the core function of SetSnapshotSkipByUserIdAndBundleNames
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, SetSnapshotSkipByUserIdAndBundleNamesInner, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    sptr<IRemoteObject> mockRemoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerByUserId(_))
        .Times(3)
        .WillOnce(Return(nullptr))
        .WillRepeatedly(Return(mockRemoteObject));
    EXPECT_CALL(mockMockSms, NotifySCBSnapshotSkipByUserIdAndBundleNames(_, _, _))
        .Times(2)
        .WillOnce(Return(ERR_INVALID_OPERATION))
        .WillRepeatedly(Return(ERR_OK));

    ErrCode ret = mockMockSms.SetSnapshotSkipByUserIdAndBundleNamesInner(100, { "notepad" });
    EXPECT_EQ(ERR_OK, ret);

    ret = mockMockSms.SetSnapshotSkipByUserIdAndBundleNamesInner(100, { "notepad" });
    EXPECT_EQ(ERR_INVALID_OPERATION, ret);

    ret = mockMockSms.SetSnapshotSkipByUserIdAndBundleNamesInner(100, { "notepad" });
    EXPECT_EQ(ERR_OK, ret);
}

/**
 * @tc.name: SetSnapshotSkipByIdNamesMapInner
 * @tc.desc: test the core function of SetSnapshotSkipByIdNamesMap
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, SetSnapshotSkipByIdNamesMapInner, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    sptr<IRemoteObject> mockRemoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerByUserId(_))
        .Times(3)
        .WillOnce(Return(nullptr))
        .WillRepeatedly(Return(mockRemoteObject));
    EXPECT_CALL(mockMockSms, NotifySCBSnapshotSkipByUserIdAndBundleNames(_, _, _))
        .Times(2)
        .WillOnce(Return(ERR_INVALID_OPERATION))
        .WillRepeatedly(Return(ERR_OK));

    int32_t ret = mockMockSms.SetSnapshotSkipByIdNamesMapInner({ { 100, { "notepad" } } });
    EXPECT_EQ(ERR_OK, ret);

    ret = mockMockSms.SetSnapshotSkipByIdNamesMapInner({ { 100, { "notepad" } } });
    EXPECT_EQ(ERR_INVALID_OPERATION, ret);

    ret = mockMockSms.SetSnapshotSkipByIdNamesMapInner({ { 100, { "notepad" } } });
    EXPECT_EQ(ERR_OK, ret);
}

/**
 * @tc.name: RecoverSCBSnapshotSkipByUserId
 * @tc.desc: test the function of RecoverSCBSnapshotSkipByUserId
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, RecoverSCBSnapshotSkipByUserId, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    sptr<IRemoteObject> mockRemoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerByUserId(_))
        .Times(4)
        .WillOnce(Return(mockRemoteObject))
        .WillOnce(Return(nullptr))
        .WillRepeatedly(Return(mockRemoteObject));
    EXPECT_CALL(mockMockSms, NotifySCBSnapshotSkipByUserIdAndBundleNames(_, _, _))
        .Times(3)
        .WillOnce(Return(ERR_OK))
        .WillOnce(Return(ERR_INVALID_OPERATION))
        .WillOnce(Return(ERR_OK));

    ErrCode ret = mockMockSms.SetSnapshotSkipByUserIdAndBundleNamesInner(100, { "notepad" });
    EXPECT_EQ(ERR_OK, ret);

    ret = mockMockSms.RecoverSCBSnapshotSkipByUserId(-1);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);

    ret = mockMockSms.RecoverSCBSnapshotSkipByUserId(100);
    EXPECT_EQ(ERR_DEAD_OBJECT, ret);

    ret = mockMockSms.RecoverSCBSnapshotSkipByUserId(100);
    EXPECT_EQ(ERR_INVALID_OPERATION, ret);

    ret = mockMockSms.RecoverSCBSnapshotSkipByUserId(100);
    EXPECT_EQ(ERR_OK, ret);
}

/**
 * @tc.name: SetSessionManagerService
 * @tc.desc: test the function of SetSessionManagerService
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, SetSessionManagerService, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    bool ret;
    MockMockSessionManagerService mockMockSms;
    // Mock SessionManagerService
    auto service = sptr<RemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(*service, AddDeathRecipient(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*service, IsProxyObject()).WillRepeatedly(Return(true));

    // branch 1: null service
    ret = mockMockSms.SetSessionManagerService(nullptr);
    EXPECT_EQ(false, ret);

    // branch 2: mock wms userId return -1
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).Times(1).WillOnce(Return(-1));
    ret = mockMockSms.SetSessionManagerService(service);
    EXPECT_EQ(false, ret);

    // branch 3: mock wms userId return 100
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).Times(1).WillOnce(Return(100));
    mockMockSms.defaultWMSUserId_ = INVALID_USER_ID;
    ret = mockMockSms.SetSessionManagerService(service);
    EXPECT_TRUE(g_errLog.find("Set defaultWMSUserId_") != std::string::npos);
    EXPECT_EQ(false, ret);

    // branch 4: cover the remaining branches
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).Times(1).WillOnce(Return(100));
    mockMockSms.defaultWMSUserId_ = 100;
    ret = mockMockSms.SetSessionManagerService(service);
    EXPECT_EQ(false, ret);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetSessionManagerService
 * @tc.desc: test the function of GetSessionManagerService
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetSessionManagerService, TestSize.Level1)
{
    sptr<IRemoteObject> sessionManagerService = nullptr;
    MockMockSessionManagerService mockMockSms;
    // branch 1: Mock GetUserIdByCallingUid() 返回非法用户id -1
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).Times(1).WillOnce(Return(-1));
    ErrCode ret = mockMockSms.GetSessionManagerService(sessionManagerService);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    // branch 2: Mock GetUserIdByCallingUid() 返回系统用户id 0
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).Times(1).WillOnce(Return(0));
    ret = mockMockSms.GetSessionManagerService(sessionManagerService);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    // branch 3: return ok
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).Times(1).WillOnce(Return(0));
    auto service = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(nullptr, service);
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillOnce(Return(service));
    ret = mockMockSms.GetSessionManagerService(sessionManagerService);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetSessionManagerServiceByUserId
 * @tc.desc: test the function of GetSessionManagerServiceByUserId
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetSessionManagerServiceByUserId, TestSize.Level1)
{
    int32_t userId = 100;
    MockMockSessionManagerService mockMockSms;
    sptr<IRemoteObject> sessionManagerService = nullptr;
    ErrCode ret;

    // branch 1: clientUserId <= INVALID_USER_ID
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).WillOnce(Return(-1));
    ret = mockMockSms.GetSessionManagerServiceByUserId(userId, sessionManagerService);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    // branch 2: clientUserId != SYSTEM_USERID
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).WillOnce(Return(200));
    ret = mockMockSms.GetSessionManagerServiceByUserId(userId, sessionManagerService);
    EXPECT_EQ(ret, ERR_WOULD_BLOCK);

    // branch 3: SYSTEM_USERID
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).WillRepeatedly(Return(0));
    ret = mockMockSms.GetSessionManagerServiceByUserId(userId, sessionManagerService);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    // branch 4: SYSTEM_USERID
    auto service = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(nullptr, service);
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillOnce(Return(service));
    ret = mockMockSms.GetSessionManagerServiceByUserId(userId, sessionManagerService);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RegisterSMSRecoverListener
 * @tc.desc: test the function of RegisterSMSRecoverListener
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, RegisterSMSRecoverListener, TestSize.Level1)
{
    ErrCode ret;
    int32_t userId = 100;
    bool isLite = false;
    sptr<IRemoteObject> listener = nullptr;
    MockMockSessionManagerService mockMockSms;

    // branch 1: nullptr
    ret = mockMockSms.RegisterSMSRecoverListener(userId, isLite, listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    // branch 2: clientUserId is INVALID_USER_ID
    listener = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).Times(1).WillOnce(Return(-1));
    ret = mockMockSms.RegisterSMSRecoverListener(userId, isLite, listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    // branch 3: clientUserId > INVALID_USER_ID
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).WillRepeatedly(Return(100));
    ret = mockMockSms.RegisterSMSRecoverListener(userId, isLite, listener);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnregisterSMSRecoverListener
 * @tc.desc: test the function of UnregisterSMSRecoverListener
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, UnregisterSMSRecoverListener, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 101;
    bool isLite = false;
    ErrCode ret;

    // branch 1
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).WillOnce(Return(-1));
    ret = mockMockSms.UnregisterSMSRecoverListener(userId, isLite);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).WillRepeatedly(Return(0));
    ret = mockMockSms.UnregisterSMSRecoverListener(userId, isLite);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: NotifySceneBoardAvailable
 * @tc.desc: test the function of NotifySceneBoardAvailable
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, NotifySceneBoardAvailable, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(false);
    ErrCode resultCode = mockMockSms.NotifySceneBoardAvailable();
    EXPECT_EQ(resultCode, ERR_PERMISSION_DENIED);

    MockAccesstokenKit::MockIsSystemApp(true);
    MockAccesstokenKit::MockIsSACalling(true);
    // Mock GetUserIdByCallingUid() 返回非法用户-1
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).Times(1).WillOnce(Return(-1));
    resultCode = mockMockSms.NotifySceneBoardAvailable();
    EXPECT_EQ(resultCode, ERR_INVALID_VALUE);

    MockAccesstokenKit::MockIsSystemApp(true);
    MockAccesstokenKit::MockIsSACalling(true);
    // Mock GetUserIdByCallingUid() 返回合法用户100
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid()).Times(1).WillOnce(Return(100));
    resultCode = mockMockSms.NotifySceneBoardAvailable();
    EXPECT_EQ(resultCode, ERR_OK);
}

/**
 * @tc.name: NotifySceneBoardAvailableToSystemAppClient
 * @tc.desc: test the function of NotifySceneBoardAvailableToSystemAppClient
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, NotifySceneBoardAvailableToSystemAppClient, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int32_t userId = 100;
    bool isLite = false;
    MockMockSessionManagerService mockMockSms;

    // branch 1: sessionManagerService is null
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillRepeatedly(Return(nullptr));
    mockMockSms.NotifySceneBoardAvailableToSystemAppClient(userId, isLite);
    EXPECT_TRUE(g_errLog.find("SessionManagerService is null") != std::string::npos);

    // branch 2: 没有监听器，正常返回
    g_errLog.clear();
    auto service = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillRepeatedly(Return(service));
    mockMockSms.NotifySceneBoardAvailableToSystemAppClient(userId, isLite);

    // branch 3: 有监听器，调用 OnSessionManagerServiceRecover
    g_errLog.clear();
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillRepeatedly(Return(service));
    
    auto listener = sptr<MockRecoverListener>::MakeSptr();
    mockMockSms.AddSMSRecoverListener(SYSTEM_USERID, 11111, userId, isLite, listener);
    
    mockMockSms.NotifySceneBoardAvailableToSystemAppClient(userId, isLite);
    EXPECT_TRUE(g_errLog.find("Call OnSessionManagerServiceRecover") != std::string::npos);

    // branch 4: userId == defaultWMSUserId，同时收集 INVALID_USER_ID 的监听器
    g_errLog.clear();
    mockMockSms.defaultWMSUserId_ = userId;
    auto listener2 = sptr<MockRecoverListener>::MakeSptr();
    mockMockSms.AddSMSRecoverListener(SYSTEM_USERID, 22222, INVALID_USER_ID, isLite, listener2);
    
    mockMockSms.NotifySceneBoardAvailableToSystemAppClient(userId, isLite);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifySceneBoardAvailableToClient
 * @tc.desc: test the function of NotifySceneBoardAvailableToClient
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, NotifySceneBoardAvailableToClient, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int32_t userId = 100;
    bool isLite = false;
    MockMockSessionManagerService mockMockSms;

    // branch 1: sessionManagerService is null
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillOnce(Return(nullptr));
    mockMockSms.NotifySceneBoardAvailableToClient(userId, isLite);
    EXPECT_TRUE(g_errLog.find("SessionManagerService is null") != std::string::npos);

    // branch 2: 没有监听器，正常返回
    g_errLog.clear();
    auto service = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillOnce(Return(service));
    mockMockSms.NotifySceneBoardAvailableToClient(userId, isLite);

    // branch 3: 有监听器，调用 OnSessionManagerServiceRecover
    g_errLog.clear();
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillOnce(Return(service));
    
    auto listener = sptr<MockRecoverListener>::MakeSptr();
    mockMockSms.AddSMSRecoverListener(userId, 11111, 100, isLite, listener);
    
    mockMockSms.NotifySceneBoardAvailableToClient(userId, isLite);
    EXPECT_TRUE(g_errLog.find("Call OnSessionManagerServiceRecover") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifyWMSConnected
 * @tc.desc: test the function of NotifyWMSConnected
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, NotifyWMSConnected, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int32_t userId = 100;
    DisplayId screenId = 0;
    bool isColdStart = true;
    MockMockSessionManagerService mockMockSms;

    // branch 1
    mockMockSms.defaultScreenId_ = screenId;
    mockMockSms.NotifyWMSConnected(userId, screenId, isColdStart);
    EXPECT_TRUE(g_errLog.find("Set defaultWMSUserId_") != std::string::npos);

    // branch 2
    isColdStart = false;
    screenId = 1;
    mockMockSms.NotifyWMSConnected(userId, screenId, isColdStart);
    EXPECT_TRUE(g_errLog.find("User switched") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifyWMSConnectionChangedToClient
 * @tc.desc: test the function of NotifyWMSConnectionChangedToClient
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, NotifyWMSConnectionChangedToClient, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    int32_t wmsUserId = 100;
    DisplayId screenId = 0;
    bool isConnected = true;
    bool isLite = false;
    int32_t wmsPid = 12345;
    auto service = sptr<IRemoteObjectMocker>::MakeSptr();
    MockMockSessionManagerService mockMockSms;

    // branch 1: sessionManagerService is null
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillOnce(Return(nullptr));
    mockMockSms.NotifyWMSConnectionChangedToClient(wmsUserId, screenId, isConnected, isLite, wmsPid);
    EXPECT_TRUE(g_errLog.find("sessionManagerService is null") != std::string::npos);

    // branch 2: 没有监听器，正常返回
    g_errLog.clear();
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillOnce(Return(service));
    mockMockSms.NotifyWMSConnectionChangedToClient(wmsUserId, screenId, isConnected, isLite, wmsPid);

    // branch 3: 有监听器，调用 OnWMSConnectionChanged
    g_errLog.clear();
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillOnce(Return(service));
    
    auto listener = sptr<MockRecoverListener>::MakeSptr();
    mockMockSms.AddSMSRecoverListener(SYSTEM_USERID, 11111, 100, isLite, listener);
    
    mockMockSms.NotifyWMSConnectionChangedToClient(wmsUserId, screenId, isConnected, isLite, wmsPid);
    EXPECT_TRUE(g_errLog.find("OnWMSConnectionChanged") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: DefaultSceneSessionManager
 * @tc.desc: test the function of DefaultSceneSessionManager
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, DefaultSceneSessionManager, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    mockMockSms.defaultWMSUserId_ = 100;
    mockMockSms.sessionManagerServiceMap_[100] = sptr<IRemoteObjectMocker>::MakeSptr();
    mockMockSms.defaultSceneSessionManager_ = sptr<IRemoteObjectMocker>::MakeSptr();
    ON_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillByDefault(Invoke([&mockMockSms]() {
        return mockMockSms.MockSessionManagerService::GetSessionManagerServiceInner(100);
    }));
    std::vector<int32_t> persistentIds = { 111, 222 };
    std::vector<string> privacyWindowTags = { "tag1", "tag2" };
    mockMockSms.AddSkipSelfWhenShowOnVirtualScreenList(persistentIds, 100);
    mockMockSms.RemoveSkipSelfWhenShowOnVirtualScreenList(persistentIds, 100);
    mockMockSms.SetScreenPrivacyWindowTagSwitch(0, privacyWindowTags, false);
}

/**
 * @tc.name: NotifyWMSConnectionStatus
 * @tc.desc: test the function of NotifyWMSConnectionStatus
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, NotifyWMSConnectionStatus, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    mockMockSms.wmsConnectionStatusMap_[userId] = true;
    
    sptr<IRemoteObject> mockRemoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<ISessionManagerServiceRecoverListener> smsListener =
        iface_cast<ISessionManagerServiceRecoverListener>(mockRemoteObject);
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillRepeatedly(Invoke([&mockMockSms, userId]() {
        return mockMockSms.MockSessionManagerService::GetSessionManagerServiceInner(userId);
    }));
    ErrCode errCode = mockMockSms.NotifyWMSConnectionStatus(userId, smsListener);
    EXPECT_EQ(errCode, ERR_OK);

    mockMockSms.sessionManagerServiceMap_[userId] = sptr<IRemoteObjectMocker>::MakeSptr();
    mockMockSms.userId2ScreenIdMap_[userId] = 0;
    errCode = mockMockSms.NotifyWMSConnectionStatus(userId, smsListener);
    EXPECT_EQ(errCode, ERR_OK);
}

/**
 * @tc.name: CheckClientIsSystemUser
 * @tc.desc: test the function of CheckClientIsSystemUser
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, CheckClientIsSystemUser, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t SYSTEM_USERID = 0;
    int32_t INVALID_USER_ID = -1;
    int32_t clientUserId = 100;
    // 1. Mock GetUserIdByCallingUid() 返回U0用户id 0
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(3)
        .WillOnce(Return(SYSTEM_USERID))
        .WillOnce(Return(INVALID_USER_ID))
        .WillOnce(Return(clientUserId));

    ErrCode result = mockMockSms.MockSessionManagerService::CheckClientIsSystemUser();
    EXPECT_EQ(result, ERR_OK);

    // 2. Mock GetUserIdByCallingUid() 返回非法用户id -1
    result = mockMockSms.MockSessionManagerService::CheckClientIsSystemUser();
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    // 3. Mock GetUserIdByCallingUid() 返回非U0用户id 100
    result = mockMockSms.MockSessionManagerService::CheckClientIsSystemUser();
    EXPECT_EQ(result, ERR_WOULD_BLOCK);
}

/**
 * @tc.name: UpdateSceneSessionManagerFromCache
 * @tc.desc: test the function of UpdateSceneSessionManagerFromCache
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, UpdateSceneSessionManagerFromCache, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    bool isLite = true;
    // sceneSessionManager 为 nullptr，函数直接返回
    sptr<IRemoteObject> sceneSessionManager = nullptr;
    mockMockSms.UpdateSceneSessionManagerFromCache(userId, isLite, sceneSessionManager);
    EXPECT_TRUE(mockMockSms.sceneSessionManagerLiteMap_.empty());
    EXPECT_TRUE(mockMockSms.sceneSessionManagerMap_.empty());
    // isLite 为 true，sceneSessionManagerLiteMap_ 被更新
    sceneSessionManager = sptr<IRemoteObjectMocker>::MakeSptr();
    mockMockSms.UpdateSceneSessionManagerFromCache(userId, isLite, sceneSessionManager);
    EXPECT_EQ(mockMockSms.sceneSessionManagerLiteMap_.count(userId), 1); // which is 0
    EXPECT_EQ(mockMockSms.sceneSessionManagerMap_.count(userId), 0);
    // isLite 为 false，sceneSessionManagerMap_ 被更新
    isLite = false;
    mockMockSms.UpdateSceneSessionManagerFromCache(userId, isLite, sceneSessionManager);
    EXPECT_EQ(mockMockSms.sceneSessionManagerLiteMap_.count(userId), 1);
    EXPECT_EQ(mockMockSms.sceneSessionManagerMap_.count(userId), 1);
}

/**
 * @tc.name: GetSceneSessionManagerFromCache
 * @tc.desc: test the function of GetSceneSessionManagerFromCache
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetSceneSessionManagerFromCache, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    // userId 不存在于缓存中，返回nullptr
    int32_t userId = 100;
    bool isLite = true;
    sptr<IRemoteObject> result = mockMockSms.GetSceneSessionManagerFromCache(userId, isLite);
    EXPECT_EQ(result, nullptr);
    // userId 存在于缓存中，isLite 为 true，返回正确对象
    sptr<IRemoteObject> mockRemoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sceneSessionManagerLiteMapMutex_);
        mockMockSms.sceneSessionManagerLiteMap_[userId] = mockRemoteObject;
    }
    result = mockMockSms.GetSceneSessionManagerFromCache(userId, isLite);
    EXPECT_EQ(result, mockRemoteObject);
    // userId 存在于缓存中，isLite 为 false，返回正确对象
    isLite = false;
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sceneSessionManagerMapMutex_);
        mockMockSms.sceneSessionManagerMap_[userId] = mockRemoteObject;
    }
    result = mockMockSms.GetSceneSessionManagerFromCache(userId, isLite);
    EXPECT_EQ(result, mockRemoteObject);
}

/**
 * @tc.name: GetSceneSessionManagerInner
 * @tc.desc: test the function of GetSceneSessionManagerInner
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetSceneSessionManagerInner, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 606;
    bool isLite = true;
    sptr<IRemoteObject> result;
    // 缓存命中，直接返回缓存对象
    sptr<IRemoteObject> mockRemoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sceneSessionManagerLiteMapMutex_);
        mockMockSms.sceneSessionManagerLiteMap_[userId] = mockRemoteObject;
    }
    result = mockMockSms.MockSessionManagerService::GetSceneSessionManagerInner(userId, isLite);
    mockMockSms.sceneSessionManagerLiteMap_.clear();
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name: GetSceneSessionManagerInner02
 * @tc.desc: test the function of GetSceneSessionManagerInner
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetSceneSessionManagerInner02, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    bool isLite = true;
    mockMockSms.sceneSessionManagerLiteMap_.clear();
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(userId)).WillOnce(Return(nullptr));
    sptr<IRemoteObject> result = mockMockSms.MockSessionManagerService::GetSceneSessionManagerInner(userId, isLite);
    EXPECT_EQ(result, nullptr);

    // GetSessionManagerServiceInner 不为 nullptr，但 iface_cast 失败
    sptr<IRemoteObject> mockSessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(userId)).WillOnce(Return(mockSessionManagerService));
    result = mockMockSms.MockSessionManagerService::GetSceneSessionManagerInner(userId, isLite);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetSceneSessionManagerByUserIdImpl
 * @tc.desc: test the function of GetSceneSessionManagerByUserIdImpl
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetSceneSessionManagerByUserIdImpl, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    bool isLite = true;
    bool checkClient = true;
    sptr<ISceneSessionManagerLite> result;
    // checkClient 为 true， CheckClientIsSystemUser 返回错误码
    EXPECT_CALL(mockMockSms, CheckClientIsSystemUser()).WillOnce(Return(ERR_INVALID_VALUE));
    ErrCode err = mockMockSms.GetSceneSessionManagerByUserIdImpl(userId, isLite, checkClient, result);
    EXPECT_EQ(err, ERR_INVALID_VALUE);
    EXPECT_EQ(result, nullptr);
    checkClient = false;

    // GetSceneSessionManagerInner 返回 nullptr
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, isLite)).WillOnce(Return(nullptr));
    err = mockMockSms.GetSceneSessionManagerByUserIdImpl(userId, isLite, checkClient, result);
    EXPECT_EQ(err, ERR_DEAD_OBJECT);
    EXPECT_EQ(result, nullptr);

    sptr<IRemoteObject> mockRemoteObject = SceneSessionManagerLite::GetInstance().AsObject();
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, isLite)).WillOnce(Return(mockRemoteObject));
    err = mockMockSms.GetSceneSessionManagerByUserIdImpl<ISceneSessionManagerLite>(userId, isLite, checkClient, result);
    EXPECT_EQ(err, ERR_OK);
}

/**
 * @tc.name: GetSceneSessionManagerByClient
 * @tc.desc: test the function of GetSceneSessionManagerByClient
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetSceneSessionManagerByClient, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    sptr<IRemoteObject> sceneSessionManager = nullptr;
    // 调用者不是系统用户， 返回错误码
    EXPECT_CALL(mockMockSms, CheckClientIsSystemUser()).WillOnce(Return(ERR_INVALID_VALUE));
    ErrCode result = mockMockSms.GetSceneSessionManagerByClient(userId, sceneSessionManager);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(sceneSessionManager, nullptr);
    // 调用者是系统用户， GetSceneSessionManagerInner 返回 nullptr
    EXPECT_CALL(mockMockSms, CheckClientIsSystemUser()).WillOnce(Return(ERR_OK));

    sptr<IRemoteObject> mockRemoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, false)).WillOnce(Return(mockRemoteObject));
    result = mockMockSms.GetSceneSessionManagerByClient(userId, sceneSessionManager);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_NE(sceneSessionManager, nullptr);
}

/**
 * @tc.name: GetSceneSessionManagerLiteByClient
 * @tc.desc: test the function of GetSceneSessionManagerLiteByClient
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetSceneSessionManagerLiteByClient, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    sptr<IRemoteObject> sceneSessionManager = nullptr;
    // 调用者不是系统用户， 返回错误码
    EXPECT_CALL(mockMockSms, CheckClientIsSystemUser()).WillOnce(Return(ERR_INVALID_VALUE));
    ErrCode result = mockMockSms.GetSceneSessionManagerLiteByClient(userId, sceneSessionManager);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(sceneSessionManager, nullptr);
    // 调用者是系统用户， GetSceneSessionManagerLiteByClient 返回 nullptr
    EXPECT_CALL(mockMockSms, CheckClientIsSystemUser()).WillOnce(Return(ERR_OK));
    sptr<IRemoteObject> mockRemoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, true)).WillOnce(Return(mockRemoteObject));
    result = mockMockSms.GetSceneSessionManagerLiteByClient(userId, sceneSessionManager);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_NE(sceneSessionManager, nullptr);
}

/**
 * @tc.name: RemoveSessionManagerServiceByUserId01
 * @tc.desc: test the function of RemoveSessionManagerServiceByUserId01
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, RemoveSessionManagerServiceByUserId01, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 666;

    mockMockSms.sessionManagerServiceMap_[userId] = nullptr;
    mockMockSms.sceneSessionManagerLiteMap_[userId] = nullptr;
    mockMockSms.sceneSessionManagerMap_[userId] = nullptr;
    mockMockSms.RemoveSessionManagerServiceByUserId(userId);
    EXPECT_TRUE(g_errLog.find("userId:") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RemoveSessionManagerServiceByUserId02
 * @tc.desc: test the function of RemoveSessionManagerServiceByUserId02
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, RemoveSessionManagerServiceByUserId02, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 666;
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sessionManagerServiceMapMutex_);
        mockMockSms.sessionManagerServiceMap_.clear();
    }
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sceneSessionManagerLiteMapMutex_);
        mockMockSms.sceneSessionManagerLiteMap_.clear();
    }
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sceneSessionManagerMapMutex_);
        mockMockSms.sceneSessionManagerMap_.clear();
    }
    mockMockSms.RemoveSessionManagerServiceByUserId(userId);
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sessionManagerServiceMapMutex_);
        EXPECT_TRUE(mockMockSms.sessionManagerServiceMap_.empty());
    }
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sceneSessionManagerLiteMapMutex_);
        EXPECT_TRUE(mockMockSms.sceneSessionManagerLiteMap_.empty());
    }
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sceneSessionManagerMapMutex_);
        EXPECT_TRUE(mockMockSms.sceneSessionManagerMap_.empty());
    }
}

/**
 * @tc.name: GetSceneSessionManagerLiteBySA
 * @tc.desc: test the function of GetSceneSessionManagerLiteBySA
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetSceneSessionManagerLiteBySA, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, true)).WillOnce(Return(nullptr));
    auto result = mockMockSms.GetSceneSessionManagerLiteBySA(userId);
    EXPECT_EQ(result, nullptr);
    sptr<IRemoteObject> mockRemoteObject = SceneSessionManagerLite::GetInstance().AsObject();
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, true)).WillOnce(Return(mockRemoteObject));
    result = mockMockSms.GetSceneSessionManagerLiteBySA(userId);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name: GetSceneSessionManagerBySA
 * @tc.desc: test the function of GetSceneSessionManagerBySA
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetSceneSessionManagerBySA, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, false)).WillOnce(Return(nullptr));
    auto result = mockMockSms.GetSceneSessionManagerBySA(userId);
    EXPECT_EQ(result, nullptr);
    sptr<IRemoteObject> mockRemoteObject = SceneSessionManager::GetInstance().AsObject();
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, false)).WillOnce(Return(mockRemoteObject));
    result = mockMockSms.GetSceneSessionManagerBySA(userId);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name: RemoveFromMap
 * @tc.desc: test the function of RemoveFromMap
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, RemoveFromMap, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    sptr<IRemoteObject> mockObject = nullptr;
    std::map<int32_t, sptr<IRemoteObject>> testMap_{};
    std::mutex testMutex_;
    testMap_[userId] = mockObject;
    mockMockSms.RemoveFromMap(testMap_, testMutex_, userId);
    EXPECT_EQ(testMap_.find(userId), testMap_.end());

    userId = 101;
    size_t initialSize = testMap_.size();
    mockMockSms.RemoveFromMap(testMap_, testMutex_, userId);
    EXPECT_EQ(testMap_.size(), initialSize);
}

/**
 * @tc.name: RemoveSMSDeathRecipientByUserId
 * @tc.desc: test the function of RemoveSMSDeathRecipientByUserId
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, RemoveSMSDeathRecipientByUserId, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);

    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    sptr<IRemoteObject> mockObject = nullptr;
    mockMockSms.smsDeathRecipientMap_[userId] = nullptr;
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(userId)).WillOnce(Return(nullptr));
    mockMockSms.RemoveSMSDeathRecipientByUserId(userId);
    EXPECT_TRUE(g_errLog.find("userId:") != std::string::npos);

    g_errLog.clear();
    sptr<IRemoteObject> mockSessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(userId)).WillRepeatedly(Return(mockSessionManagerService));
    mockMockSms.smsDeathRecipientMap_.clear();
    mockMockSms.RemoveSMSDeathRecipientByUserId(userId);
    EXPECT_TRUE(g_errLog.find("userId:") != std::string::npos);

    g_errLog.clear();
    mockMockSms.smsDeathRecipientMap_[userId] = nullptr;
    mockMockSms.RemoveSMSDeathRecipientByUserId(userId);
    EXPECT_TRUE(g_errLog.find("userId:") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: DumpSessionInfo
 * @tc.desc: test the function of DumpSessionInfo
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, DumpSessionInfo, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    std::vector<std::string> args;
    std::string info;
    int ret;

    // branch 1
    ret = mockMockSms.DumpSessionInfo(args, info);
    EXPECT_EQ(-1, ret);

    // branch 2
    args.emplace_back("test");
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillOnce(Return(nullptr));
    ret = mockMockSms.DumpSessionInfo(args, info);
    EXPECT_EQ(-1, ret);

    // branch 3
    auto mockSessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillRepeatedly(Return(mockSessionManagerService));
    EXPECT_CALL(mockMockSms, GetSceneSessionManager()).WillOnce(Return(nullptr));
    ret = mockMockSms.DumpSessionInfo(args, info);
    EXPECT_EQ(-1, ret);

    // branch 4：set defaultSSMRemote is not null
    auto sceneSessionManagerRemote = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_CALL(mockMockSms, GetSceneSessionManager()).WillOnce(Return(sceneSessionManagerRemote));
    ret = mockMockSms.DumpSessionInfo(args, info);
    EXPECT_EQ(-1, ret);
}

/**
 * @tc.name: GetSceneSessionManager
 * @tc.desc: test the function of GetSceneSessionManager
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetSceneSessionManager, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockSessionManagerService mockSms;
    mockSms.defaultWMSUserId_ = 100;
    sptr<IRemoteObject> sessionManagerService = nullptr;

    // branch 1: sessionManagerService is nullptr
    mockSms.GetSceneSessionManager();
    EXPECT_TRUE(g_errLog.find("Get session mgr service remote failed") != std::string::npos);

    // branch 2: set userId is not INVALID_USER_ID
    g_errLog.clear();
    sessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    mockSms.sessionManagerServiceMap_[100] = sessionManagerService;
    EXPECT_FALSE(g_errLog.find("Get session mgr service remote failed") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetProcessSurfaceNodeIdByPersistentId
 * @tc.desc: test the function of GetProcessSurfaceNodeIdByPersistentId
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetProcessSurfaceNodeIdByPersistentId, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    MockMockSessionManagerService mockMockSms;
    int32_t pid = 100;
    std::vector<uint64_t> windowIdList = {1, 2, 3};
    std::vector<uint64_t> surfaceNodeIds = {1, 2, 3};
    auto service = sptr<IRemoteObjectMocker>::MakeSptr();;

    // branch 1: mock GetSessionManagerServiceInner() return nullptr
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillOnce(Return(nullptr));
    mockMockSms.GetProcessSurfaceNodeIdByPersistentId(pid, windowIdList, surfaceNodeIds);
    EXPECT_TRUE(g_errLog.find("Get session manager remote failed") != std::string::npos);

    // branch 2: mock GetSceneSessionManager() return nullptr
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillOnce(Return(service));
    EXPECT_CALL(mockMockSms, GetSceneSessionManager()).WillOnce(Return(nullptr));
    mockMockSms.GetProcessSurfaceNodeIdByPersistentId(pid, windowIdList, surfaceNodeIds);
    EXPECT_TRUE(g_errLog.find("Get scene session manager remote failed") != std::string::npos);

    // branch 3: cover the other branches
    g_errLog.clear();
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_)).WillOnce(Return(service));
    EXPECT_CALL(mockMockSms, GetSceneSessionManager()).WillOnce(Return(service));
    mockMockSms.GetProcessSurfaceNodeIdByPersistentId(pid, windowIdList, surfaceNodeIds);
    EXPECT_FALSE(g_errLog.find("Get scene session manager remote failed") != std::string::npos);

    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: AddSMSRecoverListener
 * @tc.desc: test the function of AddSMSRecoverListener
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, AddSMSRecoverListener, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t clientUserId = 100;
    int32_t pid = 12345;
    int32_t instanceUserId = 100;
    bool isLite = false;
    
    auto listener = sptr<MockRecoverListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);
    
    mockMockSms.AddSMSRecoverListener(clientUserId, pid, instanceUserId, isLite, listener);
    
    auto listeners = mockMockSms.CollectListenersByClientUserId(clientUserId, isLite);
    EXPECT_EQ(listeners.size(), 1u);
    EXPECT_EQ(listeners[0].first, pid);
}

/**
 * @tc.name: UpdateUserId2PidMapping
 * @tc.desc: test the function of UpdateUserId2PidMapping
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, UpdateUserId2PidMapping, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    int32_t pid = 12345;
    
    mockMockSms.UpdateUserId2PidMapping(userId, pid);
    int32_t retrievedPid = mockMockSms.GetWmsPidByUserId(userId);
    EXPECT_EQ(retrievedPid, pid);
}

/**
 * @tc.name: GetWmsPidByUserId
 * @tc.desc: test the function of GetWmsPidByUserId
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetWmsPidByUserId, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    int32_t pid = 12345;
    
    int32_t retrievedPid = mockMockSms.GetWmsPidByUserId(999);
    EXPECT_EQ(retrievedPid, INVALID_PID);
    
    mockMockSms.UpdateUserId2PidMapping(userId, pid);
    retrievedPid = mockMockSms.GetWmsPidByUserId(userId);
    EXPECT_EQ(retrievedPid, pid);
}

/**
 * @tc.name: RemoveUserId2PidMapping
 * @tc.desc: test the function of RemoveUserId2PidMapping
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, RemoveUserId2PidMapping, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t userId = 100;
    int32_t pid = 12345;
    
    mockMockSms.UpdateUserId2PidMapping(userId, pid);
    mockMockSms.RemoveUserId2PidMapping(userId);
    
    int32_t retrievedPid = mockMockSms.GetWmsPidByUserId(userId);
    EXPECT_EQ(retrievedPid, INVALID_PID);
}

/**
 * @tc.name: GetDefaultWMSUserId
 * @tc.desc: test the function of GetDefaultWMSUserId
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetDefaultWMSUserId, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    
    int32_t defaultUserId = mockMockSms.GetDefaultWMSUserId();
    EXPECT_EQ(defaultUserId, INVALID_USER_ID);
    
    mockMockSms.defaultWMSUserId_ = 100;
    defaultUserId = mockMockSms.GetDefaultWMSUserId();
    EXPECT_EQ(defaultUserId, 100);
}

/**
 * @tc.name: RemoveSMSRecoverListener
 * @tc.desc: test the function of RemoveSMSRecoverListener
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, RemoveSMSRecoverListener, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t clientUserId = 100;
    int32_t pid = 12345;
    int32_t instanceUserId = 100;
    bool isLite = false;
    
    auto listener = sptr<MockRecoverListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);
    
    mockMockSms.AddSMSRecoverListener(clientUserId, pid, instanceUserId, isLite, listener);
    auto listeners = mockMockSms.CollectListenersByClientUserId(clientUserId, isLite);
    EXPECT_EQ(listeners.size(), 1u);
    
    mockMockSms.RemoveSMSRecoverListener(clientUserId, pid, instanceUserId, isLite);
    listeners = mockMockSms.CollectListenersByClientUserId(clientUserId, isLite);
    EXPECT_EQ(listeners.size(), 0u);
}

/**
 * @tc.name: CollectListenersByClientUserId
 * @tc.desc: test the function of CollectListenersByClientUserId
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, CollectListenersByClientUserId, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t clientUserId = 100;
    bool isLite = false;
    auto listener1 = sptr<MockRecoverListener>::MakeSptr();
    auto listener2 = sptr<MockRecoverListener>::MakeSptr();
    
    mockMockSms.AddSMSRecoverListener(clientUserId, 11111, 100, isLite, listener1);
    mockMockSms.AddSMSRecoverListener(clientUserId, 22222, 101, isLite, listener2);
    
    auto listeners = mockMockSms.CollectListenersByClientUserId(clientUserId, isLite);
    EXPECT_EQ(listeners.size(), 2u);
    
    listeners = mockMockSms.CollectListenersByClientUserId(999, isLite);
    EXPECT_EQ(listeners.size(), 0u);
}

/**
 * @tc.name: CollectListenersByInstanceUserId
 * @tc.desc: test the function of CollectListenersByInstanceUserId
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, CollectListenersByInstanceUserId, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t clientUserId = 100;
    int32_t instanceUserId = 100;
    bool isLite = false;
    
    auto listener1 = sptr<MockRecoverListener>::MakeSptr();
    auto listener2 = sptr<MockRecoverListener>::MakeSptr();
    mockMockSms.AddSMSRecoverListener(clientUserId, 11111, instanceUserId, isLite, listener1);
    mockMockSms.AddSMSRecoverListener(clientUserId, 22222, 200, isLite, listener2);
    
    auto listeners = mockMockSms.CollectListenersByInstanceUserId(clientUserId, instanceUserId, isLite);
    EXPECT_EQ(listeners.size(), 1u);
    listeners = mockMockSms.CollectListenersByInstanceUserId(clientUserId, 999, isLite);
    EXPECT_EQ(listeners.size(), 0u);
}

/**
 * @tc.name: AddClientDeathRecipient
 * @tc.desc: test the function of AddClientDeathRecipient
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, AddClientDeathRecipient, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    int32_t clientUserId = 100;
    int32_t instanceUserId = 100;
    int32_t pid = 12345;
    bool isLite = false;
    
    auto service = sptr<IRemoteObjectMocker>::MakeSptr();
    bool ret = mockMockSms.AddClientDeathRecipient(service, clientUserId, instanceUserId, pid, isLite);
    EXPECT_EQ(ret, true);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
