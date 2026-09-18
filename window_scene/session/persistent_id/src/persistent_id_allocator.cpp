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

#include "session/persistent_id/include/persistent_id_allocator.h"

#include <chrono>
#include <string>
#include <thread>
#include <unistd.h>

#include <errors.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "parameters.h"
#include "persistent_storage.h"
#include "imock_session_manager_interface.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "ws_common.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr int32_t MAX_RETRY_TIMES = 3;
constexpr int32_t RETRY_INTERVAL_MS = 50;
constexpr uint32_t STRIDE_DEFAULT = 4;
constexpr uint32_t STRIDE_LARGE_SCENE = 16;
constexpr int32_t KEY_ID_MAX_DEFAULT = 4;
constexpr int32_t KEY_ID_MAX_LARGE_SCENE = 16;
constexpr uint32_t LOCAL_FALLBACK_ID_BASE = 1000000;
constexpr uint32_t LOCAL_FALLBACK_ID_LIMIT = 2000000;
constexpr uint32_t LOCAL_ID_STRIDE = 1000; 
constexpr int32_t FIRST_RECORD_ID = 2;
const std::string DEVICE_TYPE_CAR = "car";
const std::string DEVICE_TYPE_PC = "pc";
const std::string DEVICE_TYPE_2IN1 = "2in1";

uint32_t GetSessionIdStride()
{
    static const uint32_t stride = [] {
        const std::string deviceType = system::GetParameter("const.product.devicetype", "");
        if (deviceType == DEVICE_TYPE_CAR || deviceType == DEVICE_TYPE_PC ||
            deviceType == DEVICE_TYPE_2IN1) {
            return STRIDE_LARGE_SCENE;
        }
        return STRIDE_DEFAULT;
    }();
    return stride;
}

int32_t GetMaxKeyId()
{
    static const int32_t maxKeyId = [] {
        const std::string deviceType = system::GetParameter("const.product.devicetype", "");
        if (deviceType == DEVICE_TYPE_CAR || deviceType == DEVICE_TYPE_PC ||
            deviceType == DEVICE_TYPE_2IN1) {
            return KEY_ID_MAX_LARGE_SCENE;
        }
        return KEY_ID_MAX_DEFAULT;
    }();
    return maxKeyId;
}

std::string GetStorageKey(int32_t userId)
{
    return "key_id_" + std::to_string(userId);
}

sptr<IMockSessionManagerInterface> GetMockSmsProxy()
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "get system ability manager failed");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(WINDOW_MANAGER_SERVICE_ID);
    if (remoteObject == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "get window manager service failed");
        return nullptr;
    }
    sptr<IMockSessionManagerInterface> proxy = iface_cast<IMockSessionManagerInterface>(remoteObject);
    if (proxy == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "iface_cast window manager service proxy failed");
    }
    return proxy;
}

void RetryWait(int32_t attempt)
{
    if (attempt + 1 < MAX_RETRY_TIMES) {
        std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_INTERVAL_MS));
    }
}

bool AcquireKeyIdWithRetry(int32_t userId, int32_t& keyId)
{
    for (int32_t attempt = 0; attempt < MAX_RETRY_TIMES; ++attempt) {
        auto proxy = GetMockSmsProxy();
        if (proxy != nullptr) {
            int32_t acquiredKeyId = INVALID_KEY_ID;
            ErrCode err = proxy->AcquireKeyId(userId, acquiredKeyId);
            if (err == ERR_OK) {
                keyId = acquiredKeyId;
                return true;
            }
            TLOGW(WmsLogTag::WMS_LIFE, "AcquireKeyId err: %{public}d, attempt: %{public}d", err, attempt);
        }
        RetryWait(attempt);
    }
    return false;
}

bool SyncKeyIdWithRetry(int32_t userId, int32_t keyId, bool& isOccupied)
{
    for (int32_t attempt = 0; attempt < MAX_RETRY_TIMES; ++attempt) {
        auto proxy = GetMockSmsProxy();
        if (proxy != nullptr) {
            ErrCode err = proxy->SyncKeyId(userId, keyId, isOccupied);
            if (err == ERR_OK) {
                return true;
            }
            TLOGW(WmsLogTag::WMS_LIFE, "SyncKeyId err: %{public}d, attempt: %{public}d", err, attempt);
        }
        RetryWait(attempt);
    }
    return false;
}
} // namespace

WM_IMPLEMENT_SINGLE_INSTANCE(PersistentIdAllocator)

int32_t PersistentIdAllocator::ComposeSessionId(uint32_t recordId, uint32_t keyId, uint32_t stride)
{
    return static_cast<int32_t>(recordId * stride + keyId);
}

int32_t PersistentIdAllocator::ComposeLocalSessionId(uint32_t recordId, uint32_t uid)
{
    return static_cast<int32_t>(LOCAL_FALLBACK_ID_BASE + uid % LOCAL_ID_STRIDE * LOCAL_ID_STRIDE + recordId);
}

int32_t PersistentIdAllocator::GetUserIdLocked() const
{
    return GetUserIdByUid(static_cast<int32_t>(getuid()));
}

void PersistentIdAllocator::MarkUsedLocked(int32_t persistentId)
{
    usedIds_.insert(persistentId);
}

bool PersistentIdAllocator::EnsureKeyIdLocked()
{
    if (keyIdState_ == KeyIdState::AVAILABLE) {
        return true;
    }

    const int32_t userId = GetUserIdLocked();
    const int32_t maxKeyId = GetMaxKeyId();
    int32_t persistedKeyId = INVALID_KEY_ID;
    PersistentStorage::Get(GetStorageKey(userId), persistedKeyId, PersistentStorageType::KEY_ID);
    if (persistedKeyId > INVALID_KEY_ID && persistedKeyId <= maxKeyId) {
        bool isOccupied = false;
        if (SyncKeyIdWithRetry(userId, persistedKeyId, isOccupied) && isOccupied) {
            keyId_ = persistedKeyId;
            keyIdState_ = KeyIdState::AVAILABLE;
            TLOGI(WmsLogTag::WMS_LIFE, "restore keyId: %{public}d from storage, userId: %{public}d",
                keyId_, userId);
            return true;
        }
        TLOGW(WmsLogTag::WMS_LIFE, "sync persisted keyId: %{public}d failed, try allocating a new one",
            persistedKeyId);
    }
    int32_t keyId = INVALID_KEY_ID;
    if (AcquireKeyIdWithRetry(userId, keyId) && keyId > INVALID_KEY_ID && keyId <= maxKeyId) {
        keyId_ = keyId;
        keyIdState_ = KeyIdState::AVAILABLE;
        PersistentStorage::Insert(GetStorageKey(userId), keyId_, PersistentStorageType::KEY_ID);
        TLOGI(WmsLogTag::WMS_LIFE, "acquire keyId: %{public}d from foundation, userId: %{public}d",
            keyId_, userId);
        return true;
    }
    TLOGW(WmsLogTag::WMS_LIFE, "acquire keyId failed, use local fallback, userId: %{public}d", userId);
    return false;
}

int32_t PersistentIdAllocator::GenerateSessionIdLocked()
{
    const uint32_t stride = GetSessionIdStride();
    if (sessionRecordId_ < FIRST_RECORD_ID) {
        sessionRecordId_ = FIRST_RECORD_ID - 1;
    }
    while (true) {
        sessionRecordId_++;
        int32_t persistentId = ComposeSessionId(sessionRecordId_, keyId_, stride);
        if (persistentId >= static_cast<int32_t>(LOCAL_FALLBACK_ID_BASE) &&
            persistentId < static_cast<int32_t>(LOCAL_FALLBACK_ID_LIMIT)) {
            sessionRecordId_ = (LOCAL_FALLBACK_ID_LIMIT - keyId_) / stride;
            continue;
        }
        if (usedIds_.count(persistentId) == 0) {
            MarkUsedLocked(persistentId);
            return persistentId;
        }
    }
}

int32_t PersistentIdAllocator::GenerateExtensionLocalLocked()
{
    if (extensionRecordId_ < FIRST_RECORD_ID) {
        extensionRecordId_ = FIRST_RECORD_ID - 1;
    }
    extensionRecordId_++;
    while (usedIds_.count(extensionRecordId_)) {
        extensionRecordId_++;
    }
    constexpr uint32_t pidLength = 18;
    constexpr uint32_t pidMask = (1 << pidLength) - 1;
    constexpr uint32_t persistentIdLength = 12;
    constexpr uint32_t persistentIdMask = (1 << persistentIdLength) - 1;
    uint32_t assembledPersistentId = ((static_cast<uint32_t>(getpid()) & pidMask) << persistentIdLength) |
        (static_cast<uint32_t>(extensionRecordId_) & persistentIdMask);
    MarkUsedLocked(extensionRecordId_);
    return static_cast<int32_t>(assembledPersistentId | 0x40000000);
}

int32_t PersistentIdAllocator::GenerateLocalFallbackLocked()
{
    const uint32_t uid = static_cast<uint32_t>(getuid());
    while (true) {
        int32_t persistentId = ComposeLocalSessionId(fallbackRecordId_, uid);
        if (usedIds_.count(persistentId) == 0) {
            MarkUsedLocked(persistentId);
            fallbackRecordId_++;
            return persistentId;
        }
        fallbackRecordId_++;
    }
}

int32_t PersistentIdAllocator::Acquire(bool isExtension, int32_t specifiedId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (specifiedId != INVALID_SESSION_ID && usedIds_.count(specifiedId) == 0) {
        MarkUsedLocked(specifiedId);
        return specifiedId;
    }
    if (isExtension) {
        return GenerateExtensionLocalLocked();
    }
    if (EnsureKeyIdLocked()) {
        return GenerateSessionIdLocked();
    }
    return GenerateLocalFallbackLocked();
}
} // namespace Rosen
} // namespace OHOS
