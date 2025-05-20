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

#ifndef OHOS_ROSEN_WINDOW_SCENE_ABILITY_INFO_MANAGER_H
#define OHOS_ROSEN_WINDOW_SCENE_ABILITY_INFO_MANAGER_H

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

#include <refbase.h>

#include "wm_single_instance.h"

namespace OHOS::AppExecFwk {
class IBundleMgr;
struct AbilityInfo;
struct BundleInfo;
} // namespace OHOS::AppExecFwk

namespace OHOS::Rosen {
class AbilityInfoManager {
WM_DECLARE_SINGLE_INSTANCE(AbilityInfoManager);
public:
    static bool FindAbilityInfo(const AppExecFwk::BundleInfo& bundleInfo,
        const std::string& moduleName, const std::string& abilityName, AppExecFwk::AbilityInfo& abilityInfo);

    void Init(const sptr<AppExecFwk::IBundleMgr>& bundleMgr);
    void SetCurrentUserId(int32_t userId);

    // Locks applicationInfoMutex_
    void RemoveAppInfo(const std::string& bundleName);
    bool IsAnco(const std::string& bundleName, const std::string& abilityName, const std::string& moduleName);
    // Above guarded by applicationInfoMutex_

private:
    std::mutex applicationInfoMutex_;
    std::unordered_map<std::string, std::string> applicationInfoMap_;
    int32_t userId_ = 0;
    // Above guarded by applicationInfoMutex_

    sptr<AppExecFwk::IBundleMgr> bundleMgr_;
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_ABILITY_INFO_MANAGER_H