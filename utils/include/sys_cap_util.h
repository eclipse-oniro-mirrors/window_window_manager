/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef WM_SYS_CAP_UTIL_H
#define WM_SYS_CAP_UTIL_H

#include <bundle_mgr_interface.h>
#include <shared_mutex>
#include <string>

namespace OHOS {
namespace Rosen {
struct BundleInfo {
    std::string name_;
    int32_t apiVersion_;
};

class SysCapUtil {
public:
    static std::string GetClientName();
    static std::string GetBundleName();
    static uint32_t GetApiCompatibleVersion();
    static std::shared_ptr<BundleInfo> GetBundleInfo(uint32_t pid);
    static std::shared_ptr<BundleInfo> UpdateBundleInfo(uint32_t pid);
    static std::shared_ptr<BundleInfo> UpdateBundleInfo(uint32_t pid, sptr<IRemoteObject> agent);
    static void RemoveBundleInfo(sptr<IRemoteObject> agent);
private:
    static std::string GetProcessName();
    static std::shared_mutex pidBundleNameMutex_;
    static std::map<uint32_t, std::shared_ptr<BundleInfo>> pidBundleInfoMap_;
    static std::map<sptr<IRemoteObject>, uint32_t> agentPidMap_;
};
} // Rosen
} // OHOS
#endif // WM_BUNDLE_MGR_UTIL_H