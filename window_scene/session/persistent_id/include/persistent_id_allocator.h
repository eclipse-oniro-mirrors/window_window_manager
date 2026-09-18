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

#ifndef OHOS_ROSEN_PERSISTENT_ID_ALLOCATOR_H
#define OHOS_ROSEN_PERSISTENT_ID_ALLOCATOR_H

#include <cstdint>
#include <mutex>
#include <set>

#include "wm_common.h"
#include "wm_single_instance.h"

namespace OHOS {
namespace Rosen {

constexpr int32_t INVALID_KEY_ID = 0;

class PersistentIdAllocator {
    WM_DECLARE_SINGLE_INSTANCE(PersistentIdAllocator);

public:
    int32_t Acquire(bool isExtension, int32_t specifiedId);

    static int32_t ComposeSessionId(uint32_t recordId, uint32_t keyId, uint32_t stride);

    static int32_t ComposeLocalSessionId(uint32_t recordId, uint32_t uid);

private:
    enum class KeyIdState {
        NOT_READY,
        AVAILABLE,
    };

    bool EnsureKeyIdLocked();
    int32_t GenerateSessionIdLocked();
    int32_t GenerateExtensionLocalLocked();
    int32_t GenerateLocalFallbackLocked();
    void MarkUsedLocked(int32_t persistentId);
    int32_t GetUserIdLocked() const;

    std::mutex mutex_;
    KeyIdState keyIdState_ = KeyIdState::NOT_READY;
    int32_t keyId_ = INVALID_KEY_ID; // granted by the foundation process, [1, 4] or [1, 16]
    int32_t sessionRecordId_ = 0;   // incremental id for the keyId plan, starts from 2
    int32_t extensionRecordId_ = 0; // incremental id for extension sessions, starts from 2
    int32_t fallbackRecordId_ = 0;  // incremental id for the reserved local fallback, starts from 0
    std::set<int32_t> usedIds_;     // every id handed out in this process (dupe guard)
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_PERSISTENT_ID_ALLOCATOR_H
