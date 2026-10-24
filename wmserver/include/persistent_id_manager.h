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

#ifndef OHOS_ROSEN_PERSISTENT_ID_MANAGER_H
#define OHOS_ROSEN_PERSISTENT_ID_MANAGER_H

#include <cstdint>
#include <map>
#include <mutex>

#include "wm_single_instance.h"

namespace OHOS {
namespace Rosen {

constexpr int32_t INVALID_KEY_ID = 0;

class PersistentIdManager {
    WM_DECLARE_SINGLE_INSTANCE(PersistentIdManager);

public:
    int32_t AcquireKeyId(int32_t userId);

    bool SyncKeyId(int32_t userId, int32_t keyId);

private:
    static int32_t MaxKeyId();

    std::mutex mutex_;
    std::map<int32_t, int32_t> userIdToKeyId_;
    int32_t sharedKeyIdCursor_ = 0; // round-robin cursor for reuse when the space is exhausted
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_PERSISTENT_ID_MANAGER_H
