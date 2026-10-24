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

#include "persistent_id_manager.h"

#include <string>

#include "parameters.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr int32_t DEFAULT_MAX_KEY_ID = 4;
constexpr int32_t LARGE_SCENE_MAX_KEY_ID = 16;
const std::string DEVICE_TYPE_CAR = "car";
} // namespace

WM_IMPLEMENT_SINGLE_INSTANCE(PersistentIdManager)

int32_t PersistentIdManager::MaxKeyId()
{
    static const int32_t maxKeyId = [] {
        const std::string deviceType = system::GetParameter("const.product.devicetype", "");
        if (deviceType == DEVICE_TYPE_CAR) {
            return LARGE_SCENE_MAX_KEY_ID;
        }
        return DEFAULT_MAX_KEY_ID;
    }();
    return maxKeyId;
}

int32_t PersistentIdManager::AcquireKeyId(int32_t userId)
{
    if (userId <= 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "illegal userId: %{public}d", userId);
        return INVALID_KEY_ID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = userIdToKeyId_.find(userId);
    if (it != userIdToKeyId_.end()) {
        TLOGI(WmsLogTag::WMS_LIFE, "userId: %{public}d already owns keyId: %{public}d", userId, it->second);
        return it->second;
    }
    const int32_t maxKeyId = MaxKeyId();
    for (int32_t candidate = 1; candidate <= maxKeyId; ++candidate) {
        bool occupied = false;
        for (const auto& entry : userIdToKeyId_) {
            if (entry.second == candidate) {
                occupied = true;
                break;
            }
        }
        if (!occupied) {
            userIdToKeyId_[userId] = candidate;
            TLOGI(WmsLogTag::WMS_LIFE, "allocate keyId: %{public}d to userId: %{public}d, max: %{public}d",
                candidate, userId, maxKeyId);
            return candidate;
        }
    }
    sharedKeyIdCursor_ = sharedKeyIdCursor_ % maxKeyId + 1;
    TLOGW(WmsLogTag::WMS_LIFE, "keyId space exhausted, reuse keyId: %{public}d for userId: %{public}d",
        sharedKeyIdCursor_, userId);
    return sharedKeyIdCursor_;
}

bool PersistentIdManager::SyncKeyId(int32_t userId, int32_t keyId)
{
    if (userId <= 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "illegal userId: %{public}d", userId);
        return false;
    }
    const int32_t maxKeyId = MaxKeyId();
    if (keyId <= INVALID_KEY_ID || keyId > maxKeyId) {
        TLOGE(WmsLogTag::WMS_LIFE, "illegal keyId: %{public}d, max: %{public}d, userId: %{public}d",
            keyId, maxKeyId, userId);
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& entry : userIdToKeyId_) {
        if (entry.second != keyId) {
            continue;
        }
        if (entry.first == userId) {
            TLOGI(WmsLogTag::WMS_LIFE, "keyId: %{public}d already occupied by userId: %{public}d",
                keyId, userId);
            return true;
        }
        TLOGE(WmsLogTag::WMS_LIFE, "keyId: %{public}d occupied by userId: %{public}d, reject userId: %{public}d",
            keyId, entry.first, userId);
        return false;
    }
    userIdToKeyId_[userId] = keyId;
    TLOGI(WmsLogTag::WMS_LIFE, "sync keyId: %{public}d occupied by userId: %{public}d", keyId, userId);
    return true;
}
} // namespace Rosen
} // namespace OHOS
