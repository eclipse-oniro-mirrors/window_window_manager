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

// Private and protected members are opened by -Dprivate=public/-Dprotected=public
// in the target's cflags (ws_persistent_id_allocator_test in BUILD.gn): an
// in-source "#define private public" trips the UPPER_CASE macro-name lint.

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include <errors.h>
#include <gtest/gtest.h>
#include <iremote_object.h>
#include <iservice_registry.h>
#include <parcel.h>
#include <system_ability_definition.h>

#include "imock_session_manager_interface.h"
#include "session/persistent_id/include/persistent_id_allocator.h"
#include "session/host/include/scene_persistent_storage.h"
#include "wm_common.h"
#include "ws_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t EXTENSION_ID_FLAG = 0x40000000;
constexpr int32_t KEY_ID_STRIDE_DEFAULT = 4;
constexpr int32_t KEY_ID_STRIDE_LARGE_SCENE = 16;
constexpr int32_t LOCAL_FALLBACK_ID_BASE = 1000000;
constexpr int32_t LOCAL_FALLBACK_ID_LIMIT = 2000000;
constexpr int32_t LOCAL_ID_STRIDE = 1000;
constexpr int32_t LOCAL_ID_RECORD_COUNT = 1000;

// Must match GetStorageKey() in persistent_id_allocator.cpp: the persisted key of
// a user's keyId is "key_id_<userId>".
std::string StorageKeyOf(int32_t userId)
{
    return "key_id_" + std::to_string(userId);
}

// Same uid-to-user mapping as PersistentIdAllocator::GetUserId(): EnsureKeyId
// reads the storage key of the calling process's user.
int32_t ProcUserId()
{
    return GetUserIdByUid(static_cast<int32_t>(getuid()));
}

// Points the singleton storage at a per-suite directory under the working dir.
// Only the cases below call it, and they run last in declaration order: the
// cases above depend on the storage staying NOT ready (fast fallback path).
void InitKeyStorage()
{
    static bool initialized = [] {
        ScenePersistentStorage::InitDir("persistent_id_allocator_test");
        return true;
    }();
    (void)initialized;
}

// Removes the on-disk residue of this suite: the preferences directory and the
// regular file the broken-storage case used as an unpersistable "directory".
// Called from TearDownTestCase after the last case; missing paths are ignored,
// so partial runs (--gtest_filter) clean up the same way. Deleting the whole
// directory also neutralizes the preferences cache: a re-flush at process exit
// cannot re-create the file once its parent directory is gone.
void CleanKeyStorageResidue()
{
    InitKeyStorage(); // make sure Delete targets the suite dir, not the broken path
    ScenePersistentStorage::Delete(StorageKeyOf(ProcUserId()), ScenePersistentStorageType::KEY_ID);
    std::error_code ec;
    std::filesystem::remove_all("persistent_id_allocator_test", ec);
    std::filesystem::remove_all("persistent_id_allocator_broken_storage", ec);
}

// ===== Mocked foundation SA for the cases at the end of this file ============
// GetMockSmsProxy() resolves samgr -> GetSystemAbility -> iface_cast. The mocked
// cases replace the samgr client with MockSamgr, whose GetSystemAbility hands
// out a remote whose QueryLocalInterface returns MockSmsInterface, so iface_cast
// bypasses IPC and the allocator talks to a programmable in-process fake. Same
// pattern as LocalInterfaceRemoteMocker in mock_session_manager_service_test.cpp.
class MockSmsInterface : public IMockSessionManagerInterface {
public:
    ErrCode AcquireKeyId(int32_t userId, int32_t& keyId) override
    {
        keyId = acquireKeyId_;
        return acquireErr_;
    }

    ErrCode SyncKeyId(int32_t userId, int32_t keyId, bool& isSuccess) override
    {
        isSuccess = syncSuccess_;
        return syncErr_;
    }

    sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }

    bool Marshalling(Parcel& parcel) const
    {
        return true;
    }

    ErrCode GetSessionManagerService(sptr<IRemoteObject>& sessionManagerService) override
    {
        return ERR_OK;
    }

    ErrCode GetSessionManagerServiceByUserId(int32_t userId, sptr<IRemoteObject>& sessionManagerService) override
    {
        return ERR_OK;
    }

    ErrCode GetScreenSessionManagerLite(sptr<IRemoteObject>& screenSessionManagerLite) override
    {
        return ERR_OK;
    }

    ErrCode NotifySceneBoardAvailable() override
    {
        return ERR_OK;
    }

    ErrCode RegisterSMSRecoverListener(int32_t userId, bool isLite,
        const sptr<IRemoteObject>& listener) override
    {
        return ERR_OK;
    }

    ErrCode UnregisterSMSRecoverListener(int32_t userId, bool isLite) override
    {
        return ERR_OK;
    }

    ErrCode SetSnapshotSkipByUserIdAndBundleNames(int32_t userId,
        const std::vector<std::string>& bundleNameList) override
    {
        return ERR_OK;
    }

    ErrCode SetSnapshotSkipByIdNamesMap(const std::unordered_map<int32_t,
        std::vector<std::string>>& userIdAndBunldeNames) override
    {
        return ERR_OK;
    }

    ErrCode GetSceneSessionManagerLiteByClient(int32_t userId,
        sptr<IRemoteObject>& sceneSessionManagerLite) override
    {
        return ERR_OK;
    }

    ErrCode GetSceneSessionManagerByClient(int32_t userId,
        sptr<IRemoteObject>& sceneSessionManager) override
    {
        return ERR_OK;
    }

    ErrCode NotifySetSpecificWindowZIndex() override
    {
        return ERR_OK;
    }

    ErrCode GetActiveUserIds(std::vector<int32_t>& activeUserIds) override
    {
        return ERR_OK;
    }

    // programmable per case
    ErrCode acquireErr_ = ERR_OK;
    int32_t acquireKeyId_ = 0;
    ErrCode syncErr_ = ERR_OK;
    bool syncSuccess_ = false;
};

// Hands out a same-process broker from QueryLocalInterface, so that iface_cast
// returns the broker instead of building an IPC proxy.
class LocalInterfaceRemoteObject : public IRemoteObject {
public:
    explicit LocalInterfaceRemoteObject(const sptr<IRemoteBroker>& broker)
        : IRemoteObject(u"LocalInterfaceRemoteObject"), broker_(broker)
    {
    }

    sptr<IRemoteBroker> QueryLocalInterface(const std::u16string& descriptor) override
    {
        return broker_;
    }

    int32_t GetObjectRefCount()
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return ERR_INVALID_OPERATION; // unreachable: the local interface bypasses IPC
    }

    bool IsProxyObject() const
    {
        return true;
    }

    bool IsObjectDead() const
    {
        return false;
    }

    bool CheckObjectLegality() const
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface()
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string>& args)
    {
        return 0;
    }

private:
    sptr<IRemoteBroker> broker_;
};

// NOTE(build verification): the assignment in InjectMockSamgr assumes the
// sysroot's iservice_registry.h caches the samgr client in a settable member
//   sptr<ISystemAbilityManager> SystemAbilityManagerClient::systemAbilityManager_
// returned as-is by GetSystemAbilityManager(). If the build fails there, adjust
// only that function against the local header (member name or member type; if
// the member holds the concrete client class, derive MockSamgr from it instead)
// and complete MockSamgr with the pure virtuals the local header declares.
class MockSamgr : public ISystemAbilityManager {
public:
    sptr<IRemoteObject> GetSystemAbility(int32_t said, int32_t deviceId)
    {
        return (said == WINDOW_MANAGER_SERVICE_ID) ? remote_ : nullptr;
    }

    bool AddSystemAbility(int32_t said, const sptr<IRemoteObject>& ability,
        const std::u16string& localAbilityManagerName)
    {
        return false;
    }

    bool RemoveSystemAbility(int32_t said)
    {
        return false;
    }

    sptr<IRemoteObject> CheckSystemAbility(int32_t said, int32_t deviceId)
    {
        return nullptr;
    }

    int32_t AddOnDemandSystemAbilityInfo(int32_t said, const std::u16string& localAbilityManagerName)
    {
        return ERR_OK;
    }

    int32_t SubscribeSystemAbility(int32_t said, const sptr<ISystemAbilityStatusChange>& listener)
    {
        return ERR_OK;
    }

    int32_t UnSubscribeSystemAbility(int32_t said, const sptr<ISystemAbilityStatusChange>& listener)
    {
        return ERR_OK;
    }

    sptr<IRemoteObject> LoadSystemAbility(int32_t said, int32_t timeoutMs)
    {
        return nullptr;
    }

    int32_t UnloadSystemAbility(int32_t said)
    {
        return ERR_OK;
    }

    int32_t CancelUnloadSystemAbility(int32_t said)
    {
        return ERR_OK;
    }

    sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }

    bool Marshalling(Parcel& parcel) const
    {
        return true;
    }

    sptr<IRemoteObject> remote_;
};

sptr<MockSmsInterface>& MockSmsPtr()
{
    static sptr<MockSmsInterface> mock = sptr<MockSmsInterface>::MakeSptr();
    return mock;
}

MockSmsInterface& MockSms()
{
    return *MockSmsPtr();
}

void InjectMockSamgr()
{
    static bool injected = [] {
        sptr<MockSamgr> samgr = sptr<MockSamgr>::MakeSptr();
        samgr->remote_ = sptr<LocalInterfaceRemoteObject>::MakeSptr(MockSmsPtr());
        SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = samgr;
        return true;
    }();
    (void)injected;
}
} // namespace

class PersistentIdAllocatorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase()
    {
        CleanKeyStorageResidue(); // no on-disk residue for other suites or reruns
    }
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: ComposeSessionId01
 * @tc.desc: The keyId plan composes a session id as recordId * stride + keyId; with
 *           a fixed stride, different keyIds never produce the same id rail and one
 *           rail advances by exactly the stride between consecutive recordIds.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, ComposeSessionId01, TestSize.Level1)
{
    // Basics of the formula for both product scenes.
    EXPECT_EQ(PersistentIdAllocator::ComposeSessionId(2, 1, KEY_ID_STRIDE_DEFAULT), 2 * 4 + 1);
    EXPECT_EQ(PersistentIdAllocator::ComposeSessionId(2, 4, KEY_ID_STRIDE_DEFAULT), 2 * 4 + 4);
    EXPECT_EQ(PersistentIdAllocator::ComposeSessionId(2, 1, KEY_ID_STRIDE_LARGE_SCENE), 2 * 16 + 1);
    EXPECT_EQ(PersistentIdAllocator::ComposeSessionId(2, 16, KEY_ID_STRIDE_LARGE_SCENE), 2 * 16 + 16);

    // Different keyIds with the same recordId never collide within one stride space.
    for (uint32_t keyId = 1; keyId <= KEY_ID_STRIDE_DEFAULT; ++keyId) {
        for (uint32_t other = keyId + 1; other <= KEY_ID_STRIDE_DEFAULT; ++other) {
            EXPECT_NE(PersistentIdAllocator::ComposeSessionId(7, keyId, KEY_ID_STRIDE_DEFAULT),
                PersistentIdAllocator::ComposeSessionId(7, other, KEY_ID_STRIDE_DEFAULT));
        }
    }

    // One rail advances by the stride between consecutive recordIds.
    for (uint32_t keyId = 1; keyId <= KEY_ID_STRIDE_LARGE_SCENE; ++keyId) {
        EXPECT_EQ(PersistentIdAllocator::ComposeSessionId(9, keyId, KEY_ID_STRIDE_LARGE_SCENE) -
            PersistentIdAllocator::ComposeSessionId(8, keyId, KEY_ID_STRIDE_LARGE_SCENE),
            KEY_ID_STRIDE_LARGE_SCENE);
    }
}

/**
 * @tc.name: ComposeSessionId02
 * @tc.desc: The last recordId that still composes an id inside the legacy 30-bit
 *           id space does so for the widest stride and the largest keyId.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, ComposeSessionId02, TestSize.Level1)
{
    // Largest legal recordId for stride 16 / keyId 16: (0x3fffffff - keyId) / stride.
    constexpr uint32_t maxRecordId =
        (0x3fffffff - KEY_ID_STRIDE_LARGE_SCENE) / KEY_ID_STRIDE_LARGE_SCENE;
    int32_t persistentId = PersistentIdAllocator::ComposeSessionId(
        maxRecordId, KEY_ID_STRIDE_LARGE_SCENE, KEY_ID_STRIDE_LARGE_SCENE);
    EXPECT_GT(persistentId, 0);
    EXPECT_LE(static_cast<uint32_t>(persistentId), 0x3fffffffu);
}

/**
 * @tc.name: ComposeLocalSessionId01
 * @tc.desc: The fallback rule composes 1000000 + uid % 1000 * 1000 + recordId: ids
 *           stay inside the reserved range [1000000, 2000000) while recordId stays
 *           below 1000, and different uid tags never collide on the same recordId.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, ComposeLocalSessionId01, TestSize.Level1)
{
    EXPECT_EQ(PersistentIdAllocator::ComposeLocalSessionId(0, 0), LOCAL_FALLBACK_ID_BASE);
    EXPECT_EQ(PersistentIdAllocator::ComposeLocalSessionId(999, 999),
        LOCAL_FALLBACK_ID_BASE + 999 * LOCAL_ID_STRIDE + 999); // last id of the reserved range
    EXPECT_EQ(PersistentIdAllocator::ComposeLocalSessionId(0, 1000), LOCAL_FALLBACK_ID_BASE); // uid mod 1000
    for (uint32_t recordId = 0; recordId < LOCAL_ID_RECORD_COUNT; ++recordId) {
        int32_t persistentId = PersistentIdAllocator::ComposeLocalSessionId(recordId, 4242);
        EXPECT_GE(persistentId, LOCAL_FALLBACK_ID_BASE);
        EXPECT_LT(persistentId, LOCAL_FALLBACK_ID_LIMIT);
    }
}

/**
 * @tc.name: ComposeLocalSessionId02
 * @tc.desc: Consecutive recordIds advance by 1 inside the uid-tag block and the
 *           recordId keeps growing past 1000 instead of wrapping, so the fallback
 *           generation never runs out.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, ComposeLocalSessionId02, TestSize.Level1)
{
    EXPECT_EQ(PersistentIdAllocator::ComposeLocalSessionId(1, 77) -
        PersistentIdAllocator::ComposeLocalSessionId(0, 77), 1);
    EXPECT_EQ(PersistentIdAllocator::ComposeLocalSessionId(5, 8) -
        PersistentIdAllocator::ComposeLocalSessionId(5, 7), LOCAL_ID_STRIDE);
    // Past the reserved block the ids keep growing without wrapping.
    EXPECT_EQ(PersistentIdAllocator::ComposeLocalSessionId(1000, 7),
        PersistentIdAllocator::ComposeLocalSessionId(0, 8));
    EXPECT_GT(PersistentIdAllocator::ComposeLocalSessionId(1001, 999),
        LOCAL_FALLBACK_ID_LIMIT - 1);
}

/**
 * @tc.name: ExtensionLocalId01
 * @tc.desc: Extension ids keep the legacy pid-assembled 0x40000000 format and are
 *           generated locally without any IPC.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, ExtensionLocalId01, TestSize.Level1)
{
    for (int32_t i = 0; i < 3; ++i) {
        int32_t persistentId = PersistentIdAllocator::GetInstance().Acquire(true, INVALID_SESSION_ID);
        EXPECT_NE(persistentId, INVALID_SESSION_ID);
        EXPECT_NE(static_cast<uint32_t>(persistentId) & EXTENSION_ID_FLAG, 0u);
    }
}

/**
 * @tc.name: SpecifiedId01
 * @tc.desc: An unused specified id is adopted as-is; requesting the same id again
 *           falls back to the auto generation path (legacy semantics, no
 *           same-process takeover).
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, SpecifiedId01, TestSize.Level1)
{
    constexpr int32_t specifiedId = 50000001; // far away from ids the suite generates itself
    EXPECT_EQ(PersistentIdAllocator::GetInstance().Acquire(false, specifiedId), specifiedId);
    int32_t renumbered = PersistentIdAllocator::GetInstance().Acquire(false, specifiedId);
    EXPECT_NE(renumbered, specifiedId);
    EXPECT_NE(renumbered, INVALID_SESSION_ID);
    EXPECT_EQ(static_cast<uint32_t>(renumbered) & EXTENSION_ID_FLAG, 0u);
}

/**
 * @tc.name: LocalFallback01
 * @tc.desc: In a unit-test environment the foundation service is unreachable, so
 *           the keyId plan is unavailable and non-extension ids fall back to the
 *           reserved local range: valid ids at or above 1000000, without the
 *           extension flag, and never duplicated within this process.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, LocalFallback01, TestSize.Level1)
{
    constexpr int32_t fallbackCount = 5;
    std::set<int32_t> collected;
    for (int32_t i = 0; i < fallbackCount; ++i) {
        int32_t persistentId = PersistentIdAllocator::GetInstance().Acquire(false, INVALID_SESSION_ID);
        EXPECT_NE(persistentId, INVALID_SESSION_ID);
        EXPECT_GE(persistentId, LOCAL_FALLBACK_ID_BASE);
        EXPECT_EQ(static_cast<uint32_t>(persistentId) & EXTENSION_ID_FLAG, 0u);
        collected.insert(persistentId);
    }
    EXPECT_EQ(collected.size(), static_cast<size_t>(fallbackCount));
}

/**
 * @tc.name: ExtensionLocalId02
 * @tc.desc: Locally generated extension ids never repeat inside one process: the
 *           recordId keeps advancing and is recorded in the used set, so every
 *           id carries the extension flag and all ids stay pairwise distinct.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, ExtensionLocalId02, TestSize.Level1)
{
    constexpr int32_t generateCount = 5;
    std::set<int32_t> collected;
    for (int32_t i = 0; i < generateCount; ++i) {
        int32_t persistentId = PersistentIdAllocator::GetInstance().Acquire(true, INVALID_SESSION_ID);
        EXPECT_NE(persistentId, INVALID_SESSION_ID);
        EXPECT_NE(static_cast<uint32_t>(persistentId) & EXTENSION_ID_FLAG, 0u);
        collected.insert(persistentId);
    }
    EXPECT_EQ(collected.size(), static_cast<size_t>(generateCount));
}

/**
 * @tc.name: SpecifiedId02
 * @tc.desc: A free specified id is adopted as-is even for an extension acquire, so
 *           the specified-id rule takes precedence over the extension rule; once
 *           the id is taken, an extension acquire is renumbered into the extension
 *           format instead.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, SpecifiedId02, TestSize.Level1)
{
    constexpr int32_t specifiedId = 50000002; // far away from ids the suite generates itself
    EXPECT_EQ(PersistentIdAllocator::GetInstance().Acquire(true, specifiedId), specifiedId);
    EXPECT_EQ(static_cast<uint32_t>(specifiedId) & EXTENSION_ID_FLAG, 0u);
    int32_t renumbered = PersistentIdAllocator::GetInstance().Acquire(true, specifiedId);
    EXPECT_NE(renumbered, specifiedId);
    EXPECT_NE(static_cast<uint32_t>(renumbered) & EXTENSION_ID_FLAG, 0u);
}

/**
 * @tc.name: LocalFallback02
 * @tc.desc: An id adopted out of the reserved local range stays exclusive: the
 *           local fallback generation skips the taken slot and keeps handing out
 *           ids from the same range.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, LocalFallback02, TestSize.Level1)
{
    int32_t first = PersistentIdAllocator::GetInstance().Acquire(false, INVALID_SESSION_ID);
    EXPECT_GE(first, LOCAL_FALLBACK_ID_BASE);
    EXPECT_LT(first, LOCAL_FALLBACK_ID_LIMIT);
    int32_t nextSlot = first + 1; // the slot the next fallback call would take if free
    EXPECT_EQ(PersistentIdAllocator::GetInstance().Acquire(false, nextSlot), nextSlot);
    int32_t after = PersistentIdAllocator::GetInstance().Acquire(false, INVALID_SESSION_ID);
    EXPECT_NE(after, nextSlot); // the adopted slot is skipped
    EXPECT_GT(after, first); // generation continues with the following slot
    EXPECT_LT(after, LOCAL_FALLBACK_ID_LIMIT);
}

// The cases below drive the branch chain of TryRestoreKeyId/TryAcquireNewKeyId
// through the public Acquire(): EnsureKeyId reads the keyId persisted for the
// test process's user, so each case seeds that storage key first. In a unit
// test the foundation SA is unreachable (no samgr on host; on device the
// sceneboard-only gate denies the test process), so every SA call fails after
// its retries — the branches reachable here are the persisted-value checks and
// the SA-failure degradations. The mocked cases at the end of this file then
// inject a fake samgr to also cover the grant-success branches.
/**
 * @tc.name: TryRestoreKeyId01
 * @tc.desc: Without a persisted keyId (the preference default is 0), restoring
 *           is rejected, the new-keyId acquisition fails without the SA, and
 *           Acquire degrades to a local fallback id.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, TryRestoreKeyId01, TestSize.Level1)
{
    InitKeyStorage();
    ScenePersistentStorage::Delete(StorageKeyOf(ProcUserId()), ScenePersistentStorageType::KEY_ID);
    int32_t persistentId = PersistentIdAllocator::GetInstance().Acquire(false, INVALID_SESSION_ID);
    EXPECT_GE(persistentId, LOCAL_FALLBACK_ID_BASE);
    EXPECT_LT(persistentId, LOCAL_FALLBACK_ID_LIMIT);
}

/**
 * @tc.name: TryRestoreKeyId02
 * @tc.desc: A persisted keyId above the keyId space of any product is not
 *           restored and Acquire still returns a usable local fallback id.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, TryRestoreKeyId02, TestSize.Level1)
{
    InitKeyStorage();
    ScenePersistentStorage::Insert(StorageKeyOf(ProcUserId()), 17, ScenePersistentStorageType::KEY_ID); // > 16
    int32_t persistentId = PersistentIdAllocator::GetInstance().Acquire(false, INVALID_SESSION_ID);
    EXPECT_GE(persistentId, LOCAL_FALLBACK_ID_BASE);
    EXPECT_LT(persistentId, LOCAL_FALLBACK_ID_LIMIT);
}

/**
 * @tc.name: TryRestoreKeyId03
 * @tc.desc: With a valid persisted keyId but the foundation SA unreachable, the
 *           sync retries fail, restoring is rejected and Acquire falls back
 *           locally; the failed acquire must not clobber the persisted value.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, TryRestoreKeyId03, TestSize.Level1)
{
    InitKeyStorage();
    ScenePersistentStorage::Insert(StorageKeyOf(ProcUserId()), 3, ScenePersistentStorageType::KEY_ID);
    int32_t persistentId = PersistentIdAllocator::GetInstance().Acquire(false, INVALID_SESSION_ID);
    EXPECT_GE(persistentId, LOCAL_FALLBACK_ID_BASE);
    EXPECT_LT(persistentId, LOCAL_FALLBACK_ID_LIMIT);
    int32_t persisted = 0;
    ScenePersistentStorage::Get(StorageKeyOf(ProcUserId()), persisted, ScenePersistentStorageType::KEY_ID);
    EXPECT_EQ(persisted, 3);
}

/**
 * @tc.name: TryAcquireNewKeyId01
 * @tc.desc: When acquiring from the foundation SA fails, nothing is granted and
 *           nothing is persisted for the user; Acquire degrades to a local id.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, TryAcquireNewKeyId01, TestSize.Level1)
{
    InitKeyStorage();
    ScenePersistentStorage::Delete(StorageKeyOf(ProcUserId()), ScenePersistentStorageType::KEY_ID);
    int32_t persistentId = PersistentIdAllocator::GetInstance().Acquire(false, INVALID_SESSION_ID);
    EXPECT_GE(persistentId, LOCAL_FALLBACK_ID_BASE);
    EXPECT_LT(persistentId, LOCAL_FALLBACK_ID_LIMIT);
    int32_t persisted = 1; // non-zero sentinel: the read below must overwrite it
    ScenePersistentStorage::Get(StorageKeyOf(ProcUserId()), persisted, ScenePersistentStorageType::KEY_ID);
    EXPECT_EQ(persisted, 0); // key absent: the failure path persists nothing
}

// ===== Simulated foundation SA ===============================================
// The cases below inject the mock samgr from InjectMockSamgr(), so
// GetMockSmsProxy() resolves to the programmable MockSmsInterface and the
// grant-success branches become reachable. They must stay at the END of the
// file: the injection is process-global, and the cases above rely on the real
// (unreachable) SA behavior.

/**
 * @tc.name: TryRestoreKeyId04
 * @tc.desc: With the mocked SA confirming the persisted keyId still belongs to
 *           the user, EnsureKeyId restores it and Acquire returns a keyId-space
 *           session id; a later Acquire takes the AVAILABLE fast path.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, TryRestoreKeyId04, TestSize.Level1)
{
    InitKeyStorage();
    InjectMockSamgr();
    auto& allocator = PersistentIdAllocator::GetInstance();
    allocator.keyIdState_ = PersistentIdAllocator::KeyIdState::NOT_READY;
    allocator.keyId_ = INVALID_KEY_ID;
    ScenePersistentStorage::Insert(StorageKeyOf(ProcUserId()), 3, ScenePersistentStorageType::KEY_ID);
    MockSms().syncErr_ = ERR_OK;
    MockSms().syncSuccess_ = true; // the SA confirms the persisted keyId is still ours
    MockSms().acquireErr_ = ERR_PERMISSION_DENIED; // the acquire path must not be reached

    int32_t id = allocator.Acquire(false, INVALID_SESSION_ID);
    EXPECT_LT(id, LOCAL_FALLBACK_ID_BASE); // a keyId-space id, not a local fallback
    EXPECT_GT(id, 0);
    EXPECT_EQ(allocator.keyId_, 3); // the keyId restored from storage
    int32_t persisted = 0;
    ScenePersistentStorage::Get(StorageKeyOf(ProcUserId()), persisted, ScenePersistentStorageType::KEY_ID);
    EXPECT_EQ(persisted, 3);

    int32_t next = allocator.Acquire(false, INVALID_SESSION_ID); // AVAILABLE fast path
    EXPECT_GT(next, id); // next record, same keyId
    EXPECT_LT(next, LOCAL_FALLBACK_ID_BASE);
}

/**
 * @tc.name: TryRestoreKeyId05
 * @tc.desc: The mocked SA reports the persisted keyId as taken by another user
 *           and denies the acquire: restoring is rejected and Acquire falls
 *           back locally.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, TryRestoreKeyId05, TestSize.Level1)
{
    InitKeyStorage();
    InjectMockSamgr();
    auto& allocator = PersistentIdAllocator::GetInstance();
    allocator.keyIdState_ = PersistentIdAllocator::KeyIdState::NOT_READY;
    ScenePersistentStorage::Insert(StorageKeyOf(ProcUserId()), 3, ScenePersistentStorageType::KEY_ID);
    MockSms().syncErr_ = ERR_OK;
    MockSms().syncSuccess_ = false; // the keyId is occupied by another user
    MockSms().acquireErr_ = ERR_PERMISSION_DENIED;

    int32_t id = allocator.Acquire(false, INVALID_SESSION_ID);
    EXPECT_GE(id, LOCAL_FALLBACK_ID_BASE);
    EXPECT_LT(id, LOCAL_FALLBACK_ID_LIMIT);
    EXPECT_NE(allocator.keyIdState_, PersistentIdAllocator::KeyIdState::AVAILABLE);
}

/**
 * @tc.name: TryAcquireNewKeyId02
 * @tc.desc: A keyId handed out by the mocked SA but outside the keyId space
 *           (above the maximum or invalid) is not granted and not persisted.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, TryAcquireNewKeyId02, TestSize.Level1)
{
    InitKeyStorage();
    InjectMockSamgr();
    auto& allocator = PersistentIdAllocator::GetInstance();
    allocator.keyIdState_ = PersistentIdAllocator::KeyIdState::NOT_READY;
    ScenePersistentStorage::Delete(StorageKeyOf(ProcUserId()), ScenePersistentStorageType::KEY_ID);
    MockSms().acquireErr_ = ERR_OK;

    MockSms().acquireKeyId_ = 17; // above the largest space of any product
    int32_t id = allocator.Acquire(false, INVALID_SESSION_ID);
    EXPECT_GE(id, LOCAL_FALLBACK_ID_BASE);
    EXPECT_LT(id, LOCAL_FALLBACK_ID_LIMIT);

    MockSms().acquireKeyId_ = 0; // invalid
    id = allocator.Acquire(false, INVALID_SESSION_ID);
    EXPECT_GE(id, LOCAL_FALLBACK_ID_BASE);

    int32_t persisted = 1; // non-zero sentinel: the read below must overwrite it
    ScenePersistentStorage::Get(StorageKeyOf(ProcUserId()), persisted, ScenePersistentStorageType::KEY_ID);
    EXPECT_EQ(persisted, 0); // nothing persisted on either rejection
}

/**
 * @tc.name: TryAcquireNewKeyId03
 * @tc.desc: A valid keyId granted by the mocked SA is persisted under the
 *           user's storage key and used for keyId-space session ids.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, TryAcquireNewKeyId03, TestSize.Level1)
{
    InitKeyStorage();
    InjectMockSamgr();
    auto& allocator = PersistentIdAllocator::GetInstance();
    allocator.keyIdState_ = PersistentIdAllocator::KeyIdState::NOT_READY;
    ScenePersistentStorage::Delete(StorageKeyOf(ProcUserId()), ScenePersistentStorageType::KEY_ID);
    MockSms().acquireErr_ = ERR_OK;
    MockSms().acquireKeyId_ = 3;

    int32_t id = allocator.Acquire(false, INVALID_SESSION_ID);
    EXPECT_LT(id, LOCAL_FALLBACK_ID_BASE); // session id composed from the granted keyId
    EXPECT_GT(id, 0);
    EXPECT_EQ(allocator.keyId_, 3);
    int32_t persisted = 0;
    ScenePersistentStorage::Get(StorageKeyOf(ProcUserId()), persisted, ScenePersistentStorageType::KEY_ID);
    EXPECT_EQ(persisted, 3); // the granted keyId is persisted for the user
}

/**
 * @tc.name: TryAcquireNewKeyId04
 * @tc.desc: When the storage cannot persist (its directory is a regular file,
 *           so the preferences fail to load), the read-back check rejects the
 *           grant and Acquire falls back locally.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, TryAcquireNewKeyId04, TestSize.Level1)
{
    InjectMockSamgr();
    auto& allocator = PersistentIdAllocator::GetInstance();
    allocator.keyIdState_ = PersistentIdAllocator::KeyIdState::NOT_READY;
    std::ofstream brokenStorage("persistent_id_allocator_broken_storage");
    ASSERT_TRUE(brokenStorage.good()); // the "directory" exists, but as a file
    brokenStorage.close();
    ScenePersistentStorage::InitDir("persistent_id_allocator_broken_storage");
    MockSms().acquireErr_ = ERR_OK;
    MockSms().acquireKeyId_ = 3;

    int32_t id = allocator.Acquire(false, INVALID_SESSION_ID);
    EXPECT_GE(id, LOCAL_FALLBACK_ID_BASE); // the grant is rejected, local fallback
    EXPECT_LT(id, LOCAL_FALLBACK_ID_LIMIT);
    EXPECT_NE(allocator.keyIdState_, PersistentIdAllocator::KeyIdState::AVAILABLE);

    InitKeyStorage(); // restore the working storage for any later case
}

/**
 * @tc.name: GenerateSessionId01
 * @tc.desc: With a granted keyId the generator covers its branches: record
 *           normalization, skipping a used record, and jumping over the
 *           reserved local band.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, GenerateSessionId01, TestSize.Level1)
{
    auto& allocator = PersistentIdAllocator::GetInstance();
    allocator.keyIdState_ = PersistentIdAllocator::KeyIdState::AVAILABLE;
    allocator.keyId_ = 4; // unused by any earlier case, keeps records collision-free
    allocator.sessionRecordId_ = 0; // forces the FIRST_RECORD_ID normalization

    int32_t first = allocator.Acquire(false, INVALID_SESSION_ID); // record 2
    EXPECT_GT(first, 0);
    EXPECT_LT(first, LOCAL_FALLBACK_ID_BASE);
    const int32_t stride = (first - 4) / 2; // first == 2 * stride + 4
    EXPECT_TRUE(stride == KEY_ID_STRIDE_DEFAULT || stride == KEY_ID_STRIDE_LARGE_SCENE);

    allocator.sessionRecordId_ = 1; // record 2 again, now used
    int32_t skipped = allocator.Acquire(false, INVALID_SESSION_ID);
    EXPECT_EQ(skipped, first + stride); // the used record is skipped by an increment

    // land inside the reserved local band: the record jumps past it
    allocator.sessionRecordId_ = LOCAL_FALLBACK_ID_BASE / stride;
    int32_t jumped = allocator.Acquire(false, INVALID_SESSION_ID);
    EXPECT_EQ(jumped, LOCAL_FALLBACK_ID_LIMIT + 4); // past the band: LIMIT + keyId
}
} // namespace Rosen
} // namespace OHOS
