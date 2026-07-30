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

#include "bundle_info_helper.h"
#include "window_manager_hilog.h"
#include "session_permission.h"

#include <ipc_skeleton.h>

namespace OHOS {
namespace Rosen {
const std::string NO_EXIST_BUNDLE_NAME = "null";

std::string BundleInfoHelper::GetCurBundleName()
{
    std::string bundleName = NO_EXIST_BUNDLE_NAME;
    if (SessionPermission::IsSACalling()) {
        return bundleName;
    }
    int32_t currentPid = IPCSkeleton::GetCallingPid();
    auto bundleInfo = SysCapUtil::GetBundleInfo(currentPid);
    if (bundleInfo != nullptr) {
        bundleName = bundleInfo->name_;
    } else {
        TLOGE(WmsLogTag::DMS, "bundleInfo null");
        bundleName = SysCapUtil::GetBundleName();
    }
    return bundleName;
}

std::string BundleInfoHelper::GetBundleNameByPid(const int32_t agentPid)
{
    std::string bundleName = NO_EXIST_BUNDLE_NAME;
    auto bundleInfo = SysCapUtil::GetBundleInfo(agentPid);
    if (bundleInfo != nullptr) {
        bundleName = bundleInfo->name_;
    }
    return bundleName;
}
} // namespace Rosen
} // namespace OHOS