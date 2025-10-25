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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "iremote_object_mocker.h"

#include "scene_session_manager.h"
#include "session_manager_service_proxy.h"
#include "mock_accesstoken_kit.h"
#include "wm_common.h"
#include "scene_session_manager_lite.h"
#include "scene_session_manager.h"

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
};

namespace {
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
 * @tc.name: SetSessionManagerServiceFailed
 * @tc.desc: test the function of SetSessionManagerServiceFailed
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, SetSessionManagerServiceFailed, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    sptr<IRemoteObject> nullSessionManagerService = nullptr;
    bool result = mockMockSms.SetSessionManagerService(nullSessionManagerService);
    EXPECT_EQ(result, false);

    sptr<IRemoteObject> mockSessionManagerService = sptr<IRemoteObjectMocker>::MakeSptr();
    // Mock GetUserIdByCallingUid() 返回非法用户id -1
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(-1));
    result = mockMockSms.SetSessionManagerService(mockSessionManagerService);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetSessionManagerService
 * @tc.desc: test the function of GetSessionManagerService
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, GetSessionManagerService, TestSize.Level1)
{
    sptr<IRemoteObject> sessionManagerService;
    MockMockSessionManagerService mockMockSms;
    // 1. Mock GetUserIdByCallingUid() 返回非法用户id -1
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(-1));
    ErrCode resultCode = mockMockSms.GetSessionManagerService(sessionManagerService);
    EXPECT_EQ(resultCode, ERR_INVALID_VALUE);

    // 1. Mock GetUserIdByCallingUid() 返回系统用户id 0
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(mockMockSms, GetSessionManagerServiceInner(_))
        .WillOnce(Invoke([&mockMockSms](){
            return mockMockSms.MockSessionManagerService::GetSessionManagerServiceInner(0);
        }));
    resultCode = mockMockSms.GetSessionManagerService(sessionManagerService);
    EXPECT_EQ(resultCode, ERR_OK);
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
    sptr<IRemoteObject> sessionManagerService;
    // 1. Mock GetUserIdByCallingUid() 返回非法用户id -1
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(-1));
    ErrCode resultCode = mockMockSms.GetSessionManagerServiceByUserId(userId, sessionManagerService);
    EXPECT_EQ(resultCode, ERR_INVALID_VALUE);

    // 2. Mock GetUserIdByCallingUid() 返回非系统用户id 200
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(200));
    resultCode = mockMockSms.GetSessionManagerServiceByUserId(userId, sessionManagerService);
    EXPECT_EQ(resultCode, ERR_WOULD_BLOCK);

    // 2. Mock GetUserIdByCallingUid() 返回系统用户id 0
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(0));
    resultCode = mockMockSms.GetSessionManagerServiceByUserId(userId, sessionManagerService);
    EXPECT_EQ(resultCode, ERR_OK);
}

/**
 * @tc.name: RegisterSMSRecoverListener
 * @tc.desc: test the function of RegisterSMSRecoverListener
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, RegisterSMSRecoverListener, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    sptr<IRemoteObject> listener = nullptr;
    int32_t userId = 100;
    // 1. listener 为 nullptr
    ErrCode resultCode = mockMockSms.RegisterSMSRecoverListener(listener, userId);
    EXPECT_EQ(resultCode, ERR_INVALID_VALUE);

    // 2.Mock GetUserIdByCallingUid() 返回系统用户id 0
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(0));
    listener = sptr<IRemoteObjectMocker>::MakeSptr();
    userId = INVALID_USER_ID;
    resultCode = mockMockSms.RegisterSMSRecoverListener(listener, userId);
    EXPECT_EQ(resultCode, ERR_OK);
    
    // 3.Mock GetUserIdByCallingUid() 返回系统用户id 0
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(0));
    listener = sptr<IRemoteObjectMocker>::MakeSptr();
    userId = 100;
    resultCode = mockMockSms.RegisterSMSRecoverListener(listener, userId);
    EXPECT_EQ(resultCode, 4194312);

    // 4.Mock GetUserIdByCallingUid() 返回非系统用户id 100
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(100));
    listener = sptr<IRemoteObjectMocker>::MakeSptr();
    userId = INVALID_USER_ID;
    resultCode = mockMockSms.RegisterSMSRecoverListener(listener, userId);
    EXPECT_EQ(resultCode, ERR_WOULD_BLOCK);
}

/**
 * @tc.name: UnregisterSMSRecoverListener
 * @tc.desc: test the function of UnregisterSMSRecoverListener
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, UnregisterSMSRecoverListener, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    // 1.Mock GetUserIdByCallingUid() 返回系统用户id 0
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(0));
    int32_t userId = INVALID_USER_ID;
    ErrCode resultCode = mockMockSms.UnregisterSMSRecoverListener(userId);
    EXPECT_EQ(resultCode, ERR_OK);

    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(0));
    std::map<int32_t, sptr<ISessionManagerServiceRecoverListener>> systemAppSmsRecoverListenerMap;
    systemAppSmsRecoverListenerMap[999090] = nullptr;
    mockMockSms.systemAppSmsRecoverListenerMap_[0] = systemAppSmsRecoverListenerMap;
    resultCode = mockMockSms.UnregisterSMSRecoverListener(userId);
    EXPECT_EQ(resultCode, ERR_OK);

    // 2.Mock GetUserIdByCallingUid() 返回系统用户id 0
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(0));
    userId = 100;
    resultCode = mockMockSms.UnregisterSMSRecoverListener(userId);
    EXPECT_EQ(resultCode, 4194312);

    // 3.Mock GetUserIdByCallingUid() 返回非系统用户id 100
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(100));
    userId = INVALID_USER_ID;
    resultCode = mockMockSms.UnregisterSMSRecoverListener(userId);
    EXPECT_EQ(resultCode, ERR_OK);
}

/**
 * @tc.name: RegisterSMSLiteRecoverListener
 * @tc.desc: test the function of RegisterSMSLiteRecoverListener
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, RegisterSMSLiteRecoverListener, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    sptr<IRemoteObject> listener = nullptr;
    int32_t userId = 100;
    // 1. listener 为 nullptr
    ErrCode resultCode = mockMockSms.RegisterSMSLiteRecoverListener(listener, userId);
    EXPECT_EQ(resultCode, ERR_INVALID_VALUE);

    // 2.Mock GetUserIdByCallingUid() 返回系统用户id 0
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(0));
    listener = sptr<IRemoteObjectMocker>::MakeSptr();
    userId = INVALID_USER_ID;
    resultCode = mockMockSms.RegisterSMSLiteRecoverListener(listener, userId);
    EXPECT_EQ(resultCode, ERR_OK);
    
    // 3.Mock GetUserIdByCallingUid() 返回系统用户id 0
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(0));
    listener = sptr<IRemoteObjectMocker>::MakeSptr();
    userId = 100;
    resultCode = mockMockSms.RegisterSMSLiteRecoverListener(listener, userId);
    EXPECT_EQ(resultCode, 4194312);

    // 4.Mock GetUserIdByCallingUid() 返回非系统用户id 100
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(100));
    listener = sptr<IRemoteObjectMocker>::MakeSptr();
    userId = INVALID_USER_ID;
    resultCode = mockMockSms.RegisterSMSLiteRecoverListener(listener, userId);
    EXPECT_EQ(resultCode, ERR_WOULD_BLOCK);
}

/**
 * @tc.name: UnregisterSMSLiteRecoverListener
 * @tc.desc: test the function of UnregisterSMSLiteRecoverListener
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, UnregisterSMSLiteRecoverListener, TestSize.Level1)
{
    MockMockSessionManagerService mockMockSms;
    // 1.Mock GetUserIdByCallingUid() 返回系统用户id 0
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(0));
    int32_t userId = INVALID_USER_ID;
    ErrCode resultCode = mockMockSms.UnregisterSMSLiteRecoverListener(userId);
    EXPECT_EQ(resultCode, ERR_OK);

    std::map<int32_t, sptr<ISessionManagerServiceRecoverListener>> systemAppSmsLiteRecoverListenerMap;
    systemAppSmsLiteRecoverListenerMap[999090] = nullptr;
    mockMockSms.systemAppSmsLiteRecoverListenerMap_[0] = systemAppSmsLiteRecoverListenerMap;
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(0));
    resultCode = mockMockSms.UnregisterSMSLiteRecoverListener(userId);
    EXPECT_EQ(resultCode, ERR_OK);

    // 2.Mock GetUserIdByCallingUid() 返回系统用户id 0
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(0));
    userId = 100;
    resultCode = mockMockSms.UnregisterSMSLiteRecoverListener(userId);
    EXPECT_EQ(resultCode, 4194312);

    // 3.Mock GetUserIdByCallingUid() 返回非系统用户id 100
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(100));
    userId = INVALID_USER_ID;
    resultCode = mockMockSms.UnregisterSMSLiteRecoverListener(userId);
    EXPECT_EQ(resultCode, ERR_OK);
}

/**
 * @tc.name: NotifySceneBoardAvailable
 * @tc.desc: test the function of NotifySceneBoardAvailable
 * @tc.type: FUNC
 */
HWTEST(MockSessionManagerServiceTest, NotifySceneBoardAvailable, TestSize.Level1)
{
    // 1.Mock IsSystemCalling返回fasle
    MockMockSessionManagerService mockMockSms;
    MockAccesstokenKit::MockIsSystemApp(false);
    MockAccesstokenKit::MockIsSACalling(false);
    ErrCode resultCode = mockMockSms.NotifySceneBoardAvailable();
    EXPECT_EQ(resultCode, ERR_PERMISSION_DENIED);

    // 2. Mock IsSystemCalling返回true
    MockAccesstokenKit::MockIsSystemApp(true);
    MockAccesstokenKit::MockIsSACalling(true);
    // Mock GetUserIdByCallingUid() 返回非法用户-1
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(-1));
    resultCode = mockMockSms.NotifySceneBoardAvailable();
    EXPECT_EQ(resultCode, ERR_INVALID_VALUE);

    // 2. Mock IsSystemCalling返回true
    MockAccesstokenKit::MockIsSystemApp(true);
    MockAccesstokenKit::MockIsSACalling(true);
    // Mock GetUserIdByCallingUid() 返回合法用户100
    EXPECT_CALL(mockMockSms, GetUserIdByCallingUid())
        .Times(1)
        .WillOnce(Return(100));
    resultCode = mockMockSms.NotifySceneBoardAvailable();
    EXPECT_EQ(resultCode, ERR_OK);
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
    ON_CALL(mockMockSms, GetSessionManagerServiceInner(_))
        .WillByDefault(Invoke([&mockMockSms](){
            return mockMockSms.MockSessionManagerService::GetSessionManagerServiceInner(100);
        }));
    std::vector<std::string> args = {"arg1"};
    std::string info = "info";
    std::string& dumpInfo = info;
    mockMockSms.DumpSessionInfo(args, dumpInfo);
    std::vector<uint64_t> windowIdList = {111, 222};
    std::vector<uint64_t> surfaceNodeIds = {111, 222};
    std::vector<int32_t> persistentIds = {111, 222};
    std::vector<string> privacyWindowTags = {"tag1", "tag2"};
    mockMockSms.GetProcessSurfaceNodeIdByPersistentId(123, windowIdList, surfaceNodeIds);
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
    EXPECT_EQ(errCode, ERR_DEAD_OBJECT);

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
        .WillOnce(Return (SYSTEM_USERID))
        .WillOnce(Return (INVALID_USER_ID))
        .WillOnce(Return (clientUserId));

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
    ErrCode err = mockMockSms.GetSceneSessionManagerByUserIdImpl(userId, result, isLite, checkClient);
    EXPECT_EQ(err, ERR_INVALID_VALUE);
    EXPECT_EQ(result, nullptr);
    checkClient = false;

    // GetSceneSessionManagerInner 返回 nullptr
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, isLite)).WillOnce(Return(nullptr));
    err = mockMockSms.GetSceneSessionManagerByUserIdImpl(userId, result, isLite, checkClient);
    EXPECT_EQ(err, ERR_DEAD_OBJECT);
    EXPECT_EQ(result, nullptr);

    sptr<IRemoteObject> mockRemoteObject = SceneSessionManagerLite::GetInstance().AsObject();
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, isLite)).WillOnce(Return(mockRemoteObject));
    err = mockMockSms.GetSceneSessionManagerByUserIdImpl<ISceneSessionManagerLite>(userId, result, isLite, checkClient);
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
    MockMockSessionManagerService mockMockSms;
    size_t sessionManagerServiceMapSize = mockMockSms.sessionManagerServiceMap_.size();
    size_t sceneSessionManagerLiteMapSize = mockMockSms.sceneSessionManagerLiteMap_.size();
    size_t sceneSessionManagerMapSize = mockMockSms.sceneSessionManagerMap_.size();
    int32_t userId = 666;
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sessionManagerServiceMapMutex_);
        mockMockSms.sessionManagerServiceMap_[userId] = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sceneSessionManagerLiteMapMutex_);
        mockMockSms.sceneSessionManagerLiteMap_[userId] = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sceneSessionManagerMapMutex_);
        mockMockSms.sceneSessionManagerMap_[userId] = nullptr;
    }
    mockMockSms.RemoveSessionManagerServiceByUserId(userId);
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sessionManagerServiceMapMutex_);
        EXPECT_EQ(mockMockSms.sessionManagerServiceMap_.size(), sessionManagerServiceMapSize);
    }
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sceneSessionManagerLiteMapMutex_);
        EXPECT_EQ(mockMockSms.sceneSessionManagerLiteMap_.size(), sceneSessionManagerLiteMapSize);
    }
    {
        std::lock_guard<std::mutex> lock(mockMockSms.sceneSessionManagerMapMutex_);
        EXPECT_EQ(mockMockSms.sceneSessionManagerMap_.size(), sceneSessionManagerMapSize);
    }
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
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, false)).WillOnce(Return(nullptr));
    auto result = mockMockSms.GetSceneSessionManagerLiteBySA(userId);
    EXPECT_EQ(result, nullptr);
    sptr<IRemoteObject> mockRemoteObject = SceneSessionManagerLite::GetInstance().AsObject();
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, false)).WillOnce(Return(mockRemoteObject));
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
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, true)).WillOnce(Return(nullptr));
    auto result = mockMockSms.GetSceneSessionManagerBySA(userId);
    EXPECT_EQ(result, nullptr);
    sptr<IRemoteObject> mockRemoteObject = SceneSessionManager::GetInstance().AsObject();
    EXPECT_CALL(mockMockSms, GetSceneSessionManagerInner(userId, true)).WillOnce(Return(mockRemoteObject));
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
} // namespace
} // namespace Rosen
} // namespace OHOS