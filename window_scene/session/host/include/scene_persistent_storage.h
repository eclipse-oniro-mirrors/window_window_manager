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

#ifndef OHOS_ROSEN_WINDOW_PERSISTENT_STORAGE_H
#define OHOS_ROSEN_WINDOW_PERSISTENT_STORAGE_H

#include <shared_mutex>
#include "preferences.h"
#include "preferences_helper.h"

#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using PersistentPerference = NativePreferences::Preferences;

enum class ScenePersistentStorageType : uint32_t {
    UKNOWN = 0,
    ASPECT_RATIO,
    MAXIMIZE_STATE,
};

class ScenePersistentStorage {
public:
    ScenePersistentStorage() = default;
    ~ScenePersistentStorage() = default;

    static constexpr int32_t MAX_KEY_LEN = 80; // 80: max length of preference's key

    template <typename T>
    static void Insert(const std::string& key, const T& value, ScenePersistentStorageType storageType)
    {
        std::unique_lock<std::shared_mutex> lock(storageMutex_);
        auto pref = GetPreference(storageType);
        if (!pref) {
            WLOGE("[ScenePersistentStorage] Preferences is nullptr");
            return;
        }
        switch (storageType) {
            case ScenePersistentStorageType::ASPECT_RATIO: {
                pref->PutFloat(key, value);
                WLOGD("[ScenePersistentStorage] Insert aspect ratio, key %{public}s, value %{public}f",
                    key.c_str(), static_cast<float>(value));
                break;
            }
            case ScenePersistentStorageType::MAXIMIZE_STATE: {
                pref->PutInt(key, value);
                WLOGD("[ScenePersistentStorage] Insert Maximize state, key %{public}s, value %{public}d",
                    key.c_str(), static_cast<int>(value));
                break;
            }
            default:
                WLOGW("[ScenePersistentStorage] Unknown storage type!");
        }
        pref->Flush();
    }

    template <typename T>
    static void Get(const std::string& key, T& value, ScenePersistentStorageType storageType)
    {
        std::shared_lock<std::shared_mutex> lock(storageMutex_);
        auto pref = GetPreference(storageType);
        if (!pref) {
            WLOGE("[ScenePersistentStorage] Preferences is nullptr");
            return;
        }
        switch (storageType) {
            case ScenePersistentStorageType::ASPECT_RATIO: {
                value = pref->GetFloat(key);
                WLOGD("[ScenePersistentStorage] Get aspect ratio, key: %{public}s, value:%{public}f",
                    key.c_str(), static_cast<float>(value));
                break;
            }
            case ScenePersistentStorageType::MAXIMIZE_STATE: {
                value = pref->GetInt(key);
                WLOGD("[ScenePersistentStorage] Get Maximize state, key: %{public}s, value:%{public}d",
                    key.c_str(), static_cast<int>(value));
                break;
            }
            default:
                WLOGW("[ScenePersistentStorage] Unknown storage type!");
        }
    }

    static void RenameKeys(const std::map<std::string, std::string>& renameMap,
        ScenePersistentStorageType storageType);
    static bool HasKey(const std::string& key, ScenePersistentStorageType storageType);
    static void Delete(const std::string& key, ScenePersistentStorageType storageType);
    static void InitDir(std::string dir);

private:
    static constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "ScenePersistentStorage"};
    static std::string saveDir_;
    static std::shared_ptr<PersistentPerference> GetPreference(ScenePersistentStorageType storageType);
    static std::map<ScenePersistentStorageType, std::string> storagePath_;
    static std::shared_mutex storageMutex_;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_WINDOW_PERSISTENT_STORAGE_H
