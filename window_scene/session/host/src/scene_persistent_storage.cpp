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

#include "session/host/include/scene_persistent_storage.h"

#include <filesystem>

namespace OHOS {
namespace Rosen {

std::string ScenePersistentStorage::saveDir_;
std::map<ScenePersistentStorageType, std::string> ScenePersistentStorage::storagePath_;
std::shared_mutex ScenePersistentStorage::storageMutex_;

bool ScenePersistentStorage::HasKey(const std::string& key, ScenePersistentStorageType storageType)
{
    bool res = false;
    std::shared_lock<std::shared_mutex> lock(storageMutex_);
    auto pref = GetPreference(storageType);
    if (!pref) {
        TLOGD(WmsLogTag::DEFAULT, "[ScenePersistentStorage] Preferences is nullptr");
        return res;
    }
    res = pref->HasKey(key);
    TLOGD(WmsLogTag::DEFAULT, "[ScenePersistentStorage] %{public}s %{public}s",
        (res ? "Has persisted key:" : "Don't have persisted key:"), key.c_str());
    return res;
}

void ScenePersistentStorage::Delete(const std::string& key, ScenePersistentStorageType storageType)
{
    std::unique_lock<std::shared_mutex> lock(storageMutex_);
    auto pref = GetPreference(storageType);
    if (!pref) {
        WLOGE("[ScenePersistentStorage] Preferences is nullptr");
        return;
    }
    pref->Delete(key);
    pref->Flush();
    WLOGD("[ScenePersistentStorage] Delete key %{public}s", key.c_str());
}

std::shared_ptr<PersistentPerference> ScenePersistentStorage::GetPreference(ScenePersistentStorageType storageType)
{
    auto iter = storagePath_.find(storageType);
    if (iter == storagePath_.end()) {
        return nullptr;
    }
    auto fileName = storagePath_[storageType];
    int errCode;
    auto pref = NativePreferences::PreferencesHelper::GetPreferences(fileName, errCode);
    WLOGD("[ScenePersistentStorage] GetPreference file: %{public}s, errCode: %{public}d", fileName.c_str(), errCode);
    return pref;
}

void ScenePersistentStorage::InitDir(std::string dir)
{
    saveDir_ = dir;
    std::filesystem::path fileDir {saveDir_};
    if (!std::filesystem::exists(fileDir)) {
        std::error_code errorCode;
        if (!std::filesystem::create_directories(fileDir, errorCode)) {
            TLOGE(WmsLogTag::DEFAULT, "Create dir failed, errorCode: %{public}d, msg: %{public}s",
                errorCode.value(), errorCode.message().c_str());
            abort();
        }
        std::filesystem::permissions(fileDir, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write |
            std::filesystem::perms::group_read | std::filesystem::perms::group_write);
    }
    storagePath_ = {
        { ScenePersistentStorageType::ASPECT_RATIO, saveDir_ + "/session_window_aspect_ratio" },
        { ScenePersistentStorageType::MAXIMIZE_STATE, saveDir_ + "/session_window_maximize_state" },
    };
}

void ScenePersistentStorage::RenameKeys(const std::map<std::string, std::string>& renameMap,
    ScenePersistentStorageType storageType)
{
    if (renameMap.empty()) {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(storageMutex_);
    auto pref = GetPreference(storageType);
    if (!pref) {
        TLOGD(WmsLogTag::DEFAULT, "[ScenePersistentStorage] Preferences is nullptr");
        return;
    }
    for (const auto& [oldKey, newKey] : renameMap) {
        if (oldKey.empty() || newKey.empty() || oldKey == newKey) {
            continue;
        }
        if (!pref->HasKey(oldKey)) {
            TLOGD(WmsLogTag::DEFAULT, "[ScenePersistentStorage] OldKey does not exist: %{public}s", oldKey.c_str());
            continue;
        }
        switch (storageType) {
            case ScenePersistentStorageType::ASPECT_RATIO: {
                auto floatValue = pref->GetFloat(oldKey);
                pref->PutFloat(newKey, floatValue);
                pref->Delete(oldKey);
                TLOGD(WmsLogTag::DEFAULT, "[ScenePersistentStorage] Rename OldKey: %{public}s to newKey: %{public}s",
                    oldKey.c_str(), newKey.c_str());
                break;
            }
            case ScenePersistentStorageType::MAXIMIZE_STATE: {
                auto intValue = pref->GetInt(oldKey);
                pref->PutInt(newKey, intValue);
                pref->Delete(oldKey);
                TLOGD(WmsLogTag::DEFAULT, "[ScenePersistentStorage] Rename OldKey: %{public}s to newKey: %{public}s",
                    oldKey.c_str(), newKey.c_str());
                break;
            }
            default:
                TLOGW(WmsLogTag::DEFAULT, "[ScenePersistentStorage] Unknown storage type");
        }
    }
    pref->Flush();
}
} // namespace Rosen
} // namespace OHOS
