/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ability_info_manager.h"

#include <bundlemgr/launcher_service.h>

#include "ws_common.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {

AbilityInfoManager& AbilityInfoManager::GetInstance()
{
    static AbilityInfoManager instance;
    return instance;
}

// LCOV_EXCL_START
bool AbilityInfoManager::FindAbilityInfo(const AppExecFwk::BundleInfo& bundleInfo,
    const std::string& moduleName, const std::string& abilityName, AppExecFwk::AbilityInfo& abilityInfo)
{
    auto& hapModulesList = bundleInfo.hapModuleInfos;
    for (auto& hapModule : hapModulesList) {
        auto& abilityInfoList = hapModule.abilityInfos;
        for (auto& ability : abilityInfoList) {
            if (ability.moduleName == moduleName && ability.name == abilityName) {
                abilityInfo = ability;
                return true;
            }
        }
    }
    TLOGW(WmsLogTag::DEFAULT, "ability info not found, bundle:%{public}s", bundleInfo.name.c_str());
    return false;
}
// LCOV_EXCL_STOP

void AbilityInfoManager::Init(const sptr<AppExecFwk::IBundleMgr>& bundleMgr)
{
    if (bundleMgr == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "bundleMgr is nullptr");
        return;
    }
    bundleMgr_ = bundleMgr;
}

void AbilityInfoManager::SetCurrentUserId(int32_t userId)
{
    TLOGI(WmsLogTag::WMS_LIFE, "userId: %{public}d", userId);
    std::unique_lock<std::mutex> lock(applicationInfoMutex_);
    userId_ = userId;
}

// LCOV_EXCL_START
bool AbilityInfoManager::IsAnco(const std::string& bundleName, const std::string& abilityName,
    const std::string& moduleName)
{
    bool isAnco = false;
    std::unique_lock<std::mutex> lock(applicationInfoMutex_);
    auto iter = applicationInfoMap_.find(bundleName);
    if (iter == applicationInfoMap_.end()) {
        if (bundleMgr_ == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "bundleMgr is nullptr!");
            return isAnco;
        }
        AAFwk::Want want;
        want.SetElementName("", bundleName, abilityName, moduleName);
        auto abilityInfoFlag = AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION;
        AppExecFwk::AbilityInfo abilityInfo;
        bool ret = bundleMgr_->QueryAbilityInfo(want, abilityInfoFlag, userId_, abilityInfo);
        if (!ret) {
            TLOGE(WmsLogTag::WMS_LIFE, "Get ability info from BMS failed!");
            return isAnco;
        }
        applicationInfoMap_[bundleName] = abilityInfo.applicationInfo.codePath;
        TLOGI(WmsLogTag::WMS_LIFE, "bundleName: %{public}s, abilityName: %{public}s, moduleName: %{public}s, "
            "userId: %{public}d, abilityInfoFlag: %{public}d, codePath: %{public}s", bundleName.c_str(),
            abilityName.c_str(), moduleName.c_str(), userId_, abilityInfoFlag,
            abilityInfo.applicationInfo.codePath.c_str());
        isAnco = abilityInfo.applicationInfo.codePath == std::to_string(CollaboratorType::RESERVE_TYPE) ||
                 abilityInfo.applicationInfo.codePath == std::to_string(CollaboratorType::OTHERS_TYPE);
    } else {
        TLOGI(WmsLogTag::WMS_LIFE, "applicationInfo already in applicationInfoMap_, codePath: %{public}s",
            iter->second.c_str());
        isAnco = iter->second == std::to_string(CollaboratorType::RESERVE_TYPE) ||
                 iter->second == std::to_string(CollaboratorType::OTHERS_TYPE);
    }
    return isAnco;
}

void AbilityInfoManager::RefreshAppInfo(const std::string& bundleName)
{
    std::unique_lock<std::mutex> lock(applicationInfoMutex_);
    applicationInfoMap_.erase(bundleName);
}
// LCOV_EXCL_STOP
} // namespace OHOS::Rosen