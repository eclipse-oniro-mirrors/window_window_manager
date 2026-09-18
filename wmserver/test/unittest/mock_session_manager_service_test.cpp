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

#include <cstdint>
#include <gtest/gtest.h>
#include <errors.h>
#include "mock_session_manager_service.h"
#include "display_manager.h"
#include "persistent_id_manager.h"
#include "string_ex.h"
#include "window_agent.h"
#include "window_impl.h"
#include "window_manager_hilog.h"
#include "window_property.h"
#include "window_root.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "MockSessionManagerServiceTest"};

    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
}

class MockSessionManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MockSessionManagerServiceTest::SetUpTestCase()
{
}

void MockSessionManagerServiceTest::TearDownTestCase()
{
}

void MockSessionManagerServiceTest::SetUp()
{
}

void MockSessionManagerServiceTest::TearDown()
{
}

namespace {
/**
 * @tc.name: OnRemoteDied
 * @tc.desc: OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, OnRemoteDied, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    WLOGI("OnRemoteDied");
    MockSessionManagerService::SMSDeathRecipient smsDeathRecipient(100);
    wptr<IRemoteObject> object = nullptr;
    smsDeathRecipient.OnRemoteDied(object);
    EXPECT_TRUE(g_errLog.find("sessionManagerService is null") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: OnRemoteDied
 * @tc.desc: OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, OnRemoteDied1, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    GTEST_LOG_(INFO) << "MockSessionManagerServiceTest: OnRemoteDied1 start";
    WLOGI("OnRemoteDied");
    MockSessionManagerService::SMSDeathRecipient* mService = new MockSessionManagerService::SMSDeathRecipient(100);
    wptr<IRemoteObject> object = nullptr;
    mService->OnRemoteDied(object);
    EXPECT_FALSE(g_errLog.find("SessionManagerService died!") != std::string::npos);
    LOG_SetCallback(nullptr);
    delete mService;
    mService = nullptr;
    GTEST_LOG_(INFO) << "MockSessionManagerServiceTest: OnRemoteDied1 end";
}

/**
 * @tc.name: SetSessionManagerService
 * @tc.desc: set session manager service
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, SetSessionManagerService, TestSize.Level1)
{
    WLOGI("SetSessionManagerService");
    sptr<IRemoteObject> sessionManagerService = nullptr;
    MockSessionManagerService::GetInstance().SetSessionManagerService(sessionManagerService);
    ASSERT_EQ(false, MockSessionManagerService::GetInstance().SetSessionManagerService(sessionManagerService));
}

/**
 * @tc.name: GetSessionManagerService
 * @tc.desc: get session manager service
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, GetSessionManagerService, TestSize.Level1)
{
    WLOGI("GetSessionManagerService");
    sptr<IRemoteObject> sessionManagerService = nullptr;
    MockSessionManagerService::GetInstance().GetSessionManagerService(sessionManagerService);
    ASSERT_EQ(nullptr, sessionManagerService);
}

/**
 * @tc.name: onStart
 * @tc.desc: on start
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, OnStart, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    WLOGI("onStart");
    MockSessionManagerService::GetInstance().OnStart();
    EXPECT_FALSE(g_errLog.find("OnStart begin") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: onStart
 * @tc.desc: on start
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, OnStart1, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    GTEST_LOG_(INFO) << "MockSessionManagerServiceTest: OnStart1 start";
    MockSessionManagerService* mService = new MockSessionManagerService();
    WLOGI("onStart");
    mService->OnStart();
    EXPECT_FALSE(g_errLog.find("OnStart begin") != std::string::npos);
    LOG_SetCallback(nullptr);
    delete mService;
    mService = nullptr;
    GTEST_LOG_(INFO) << "MockSessionManagerServiceTest: OnStart1 end";
}

/**
 * @tc.name: Dump
 * @tc.desc: Dump
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, Dump, TestSize.Level1)
{
    MockSessionManagerService* mService = new MockSessionManagerService();
    WLOGI("onStart");
    std::vector<std::u16string> args;
    ASSERT_EQ(-1, mService->Dump(-2, args));
}

/**
 * @tc.name: DumpWithUserArg
 * @tc.desc: Dump with -user argument
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, DumpWithUserArg, TestSize.Level1)
{
    MockSessionManagerService* mService = new MockSessionManagerService();
    std::vector<std::u16string> args;
    args.push_back(Str8ToStr16("-user"));
    ASSERT_NE(0, mService->Dump(1, args));

    args.clear();
    args.push_back(Str8ToStr16("-user"));
    args.push_back(Str8ToStr16("abc"));
    args.push_back(Str8ToStr16("-a"));
    ASSERT_NE(0, mService->Dump(1, args));

    args.clear();
    args.push_back(Str8ToStr16("-user"));
    args.push_back(Str8ToStr16("all"));
    args.push_back(Str8ToStr16("-a"));
    ASSERT_NE(0, mService->Dump(1, args));

    args.clear();
    args.push_back(Str8ToStr16("-a"));
    ASSERT_NE(0, mService->Dump(1, args));

    delete mService;
}

/**
 * @tc.name: SetSessionManagerService
 * @tc.desc: SetSessionManagerService
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, SetSessionManagerService2, TestSize.Level1)
{
    sptr<IRemoteObject> sessionManagerService = nullptr;
    ASSERT_EQ(false, MockSessionManagerService::GetInstance().SetSessionManagerService(sessionManagerService));
    MockSessionManagerService::GetInstance().NotifyWMSConnected(100, 0, true);
    MockSessionManagerService::GetInstance().NotifyWMSConnected(100, 0, false);
}

/**
 * @tc.name: GetSMSDeathRecipientByUserId
 * @tc.desc: GetSMSDeathRecipientByUserId/RemoveSMSDeathRecipientByUserId
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, GetSMSDeathRecipientByUserId, TestSize.Level1)
{
    auto smsDeathRecipient = MockSessionManagerService::GetInstance().GetSMSDeathRecipientByUserId(100);
    ASSERT_EQ(nullptr, smsDeathRecipient);
    MockSessionManagerService::GetInstance().RemoveSMSDeathRecipientByUserId(100);
}

/**
 * @tc.name: GetSessionManagerServiceInner
 * @tc.desc: GetSessionManagerServiceInner/RemoveSessionManagerServiceByUserId
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, GetSessionManagerServiceInner, TestSize.Level1)
{
    auto sessionManagerService = MockSessionManagerService::GetInstance().GetSessionManagerServiceInner(100);
    ASSERT_EQ(nullptr, sessionManagerService);
    MockSessionManagerService::GetInstance().RemoveSessionManagerServiceByUserId(100);
}

/**
 * @tc.name: GetProcessSurfaceNodeIdByPersistentId
 * @tc.desc: GetProcessSurfaceNodeIdByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, GetProcessSurfaceNodeIdByPersistentId, TestSize.Level1)
{
    int32_t pid = 123;
    std::vector<uint64_t> persistentIds = {1, 2, 3};
    std::vector<uint64_t> surfaceNodeIds;
    MockSessionManagerService::GetInstance().GetProcessSurfaceNodeIdByPersistentId(
        pid, persistentIds, surfaceNodeIds);
    ASSERT_EQ(0, surfaceNodeIds.size());
}
}

/**
 * @tc.name: AcquireKeyId01
 * @tc.desc: The IPC entry returns ERR_OK and hands out a valid keyId; syncing the
 *           same keyId for the same userId succeeds.
 * @tc.type: FUNC
 */
HWTEST_F(MockSessionManagerServiceTest, AcquireKeyId01, TestSize.Level1)
{
    int32_t keyId = INVALID_KEY_ID;
    ErrCode ret = MockSessionManagerService::GetInstance().AcquireKeyId(5001, keyId);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_GT(keyId, INVALID_KEY_ID);
    bool isSuccess = false;
    ret = MockSessionManagerService::GetInstance().SyncKeyId(5001, keyId, isSuccess);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(isSuccess);
}

/*
 * Tests for PersistentIdManager. The manager is a singleton, so its occupation map
 * grows across the cases below (they run in declaration order, like the rest of
 * this suite). The cases avoid depending on the concrete keyId space size, which
 * follows the product device type (4 by default, 16 on car/pc products).
 */
class PersistentIdManagerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}

    static constexpr int32_t USER_A = 3001;
    static constexpr int32_t USER_B = 3002;
};

/**
 * @tc.name: AcquireKeyId01
 * @tc.desc: Two different sceneboard users get two different valid keyIds.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdManagerTest, AcquireKeyId01, TestSize.Level1)
{
    int32_t keyIdA = PersistentIdManager::GetInstance().AcquireKeyId(USER_A);
    EXPECT_GT(keyIdA, INVALID_KEY_ID);
    int32_t keyIdB = PersistentIdManager::GetInstance().AcquireKeyId(USER_B);
    EXPECT_GT(keyIdB, INVALID_KEY_ID);
    EXPECT_NE(keyIdA, keyIdB);
}

/**
 * @tc.name: AcquireKeyId02
 * @tc.desc: Acquiring is idempotent for the same userId.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdManagerTest, AcquireKeyId02, TestSize.Level1)
{
    int32_t keyId = PersistentIdManager::GetInstance().AcquireKeyId(USER_A);
    EXPECT_GT(keyId, INVALID_KEY_ID);
    EXPECT_EQ(keyId, PersistentIdManager::GetInstance().AcquireKeyId(USER_A));
}

/**
 * @tc.name: AcquireKeyId03
 * @tc.desc: Illegal user ids are rejected.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdManagerTest, AcquireKeyId03, TestSize.Level1)
{
    EXPECT_EQ(PersistentIdManager::GetInstance().AcquireKeyId(0), INVALID_KEY_ID);
    EXPECT_EQ(PersistentIdManager::GetInstance().AcquireKeyId(-1), INVALID_KEY_ID);
}

/**
 * @tc.name: SyncKeyId01
 * @tc.desc: Syncing a keyId occupied by the same userId is idempotent; a keyId
 *           occupied by another userId is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdManagerTest, SyncKeyId01, TestSize.Level1)
{
    int32_t occupied = PersistentIdManager::GetInstance().AcquireKeyId(3003);
    ASSERT_GT(occupied, INVALID_KEY_ID);
    EXPECT_TRUE(PersistentIdManager::GetInstance().SyncKeyId(3003, occupied));
    EXPECT_TRUE(PersistentIdManager::GetInstance().SyncKeyId(3003, occupied));
    EXPECT_FALSE(PersistentIdManager::GetInstance().SyncKeyId(3004, occupied));
}

/**
 * @tc.name: SyncKeyId02
 * @tc.desc: Illegal arguments are rejected.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdManagerTest, SyncKeyId02, TestSize.Level1)
{
    EXPECT_FALSE(PersistentIdManager::GetInstance().SyncKeyId(3005, 0));
    EXPECT_FALSE(PersistentIdManager::GetInstance().SyncKeyId(3005, -1));
    EXPECT_FALSE(PersistentIdManager::GetInstance().SyncKeyId(3005, 17)); // above the largest keyId space
    EXPECT_FALSE(PersistentIdManager::GetInstance().SyncKeyId(0, 1));
    EXPECT_FALSE(PersistentIdManager::GetInstance().SyncKeyId(-1, 1));
}

/**
 * @tc.name: ReuseWhenExhausted01
 * @tc.desc: When the keyId space is exhausted, a new sceneboard still gets a usable
 *           (already occupied, round-robin reused) keyId instead of an invalid one.
 *           Runs late because it fills the singleton map.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdManagerTest, ReuseWhenExhausted01, TestSize.Level1)
{
    int32_t userId = 4000;
    int32_t allocated = 0;
    int32_t keyId = PersistentIdManager::GetInstance().AcquireKeyId(userId);
    while (allocated < 20 && keyId > INVALID_KEY_ID) {
        EXPECT_GT(keyId, INVALID_KEY_ID);
        EXPECT_LE(keyId, 16); // the largest keyId space has 16 slots
        ++allocated;
        ++userId;
        keyId = PersistentIdManager::GetInstance().AcquireKeyId(userId);
    }
    EXPECT_GT(allocated, 0);
    // Even past exhaustion the answer stays a usable keyId inside the space.
    EXPECT_GT(keyId, INVALID_KEY_ID);
    EXPECT_LE(keyId, 16);
}
} // namespace Rosen
} // namespace OHOS