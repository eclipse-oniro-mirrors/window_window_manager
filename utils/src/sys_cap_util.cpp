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

#include "permission.h"
#include "sys_cap_util.h"
#include "string_util.h"
#include "window_manager_hilog.h"

#include <accesstoken_kit.h>
#include <bundle_mgr_interface.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "SysCapUtil"};
const uint32_t API_VERSION_MOD = 1000;
static constexpr int32_t INVALID_PID = -1;
}
std::shared_mutex SysCapUtil::pidBundleNameMutex_;
std::map<uint32_t, std::shared_ptr<BundleInfo>>& SysCapUtil::GetPidBundleInfoMap()
{
    static std::map<uint32_t, std::shared_ptr<BundleInfo>>* map =
        new std::map<uint32_t, std::shared_ptr<BundleInfo>>();
    return *map;
}

std::map<sptr<IRemoteObject>, uint32_t>& SysCapUtil::GetAgentPidMap()
{
    static std::map<sptr<IRemoteObject>, uint32_t>* map =
        new std::map<sptr<IRemoteObject>, uint32_t>();
    return *map;
}

std::string SysCapUtil::GetClientName()
{
    std::string bn = GetBundleName();
    if (!bn.empty()) {
        WLOGFD("bundle name [%{public}s]", bn.c_str());
        return bn;
    }

    std::string pn = GetProcessName();
    if (!pn.empty()) {
        WLOGFD("process name [%{public}s]", pn.c_str());
        return pn;
    }

    WLOGFD("unknown name");
    return "unknown";
}

std::string SysCapUtil::GetBundleName()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetSystemAbilityManager null");
        return "";
    }
    OHOS::sptr<OHOS::IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    sptr<AppExecFwk::IBundleMgr> iBundleMgr = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (iBundleMgr == nullptr) {
        WLOGFW("IBundleMgr is null");
        return "";
    }

    std::string bundleName = "";
    AppExecFwk::BundleInfo bundleInfo;
    if (iBundleMgr->GetBundleInfoForSelf(0, bundleInfo) == ERR_OK) {
        bundleName = bundleInfo.name;
    } else {
        TLOGD(WmsLogTag::DEFAULT, "Failed");
    }
    return StringUtil::Trim(bundleName);
}

uint32_t SysCapUtil::GetApiCompatibleVersion()
{
    uint32_t apiCompatibleVersion = 0;
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetSystemAbilityManager null");
        return apiCompatibleVersion;
    }
    OHOS::sptr<OHOS::IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    sptr<AppExecFwk::IBundleMgr> iBundleMgr = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (iBundleMgr == nullptr) {
        WLOGFW("IBundleMgr is null");
        return apiCompatibleVersion;
    }
    AppExecFwk::BundleInfo bundleInfo;
    if (iBundleMgr->GetBundleInfoForSelf(0, bundleInfo) == ERR_OK) {
        apiCompatibleVersion = bundleInfo.targetVersion % API_VERSION_MOD;
        WLOGFD("targetVersion: [%{public}u], apiCompatibleVersion: [%{public}u]", bundleInfo.targetVersion,
            apiCompatibleVersion);
    } else {
        TLOGD(WmsLogTag::DEFAULT, "Failed");
    }
    return apiCompatibleVersion;
}

std::string SysCapUtil::GetProcessName()
{
    OHOS::Security::AccessToken::NativeTokenInfo info;
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    if (!Permission::IsTokenNativeOrShellType(tokenId)) {
        return "";
    }
    if (Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, info) != 0) {
        WLOGFW("get token info failed");
        return "";
    }
    return StringUtil::Trim(info.processName);
}

std::shared_ptr<BundleInfo> SysCapUtil::UpdateBundleInfo(uint32_t pid)
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    sptr<AppExecFwk::IBundleMgr> iBundleMgr = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (iBundleMgr == nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "IBundleMgr is null");
        return nullptr;
    }
    AppExecFwk::BundleInfo bundleInfo;
    if (iBundleMgr->GetBundleInfoForSelf(0, bundleInfo) == ERR_OK) {
        std::unique_lock<std::shared_mutex> lock(pidBundleNameMutex_);
        std::shared_ptr<BundleInfo> bundleInfoPtr = std::make_shared<BundleInfo>();
        bundleInfoPtr->name_ = bundleInfo.name;
        bundleInfoPtr->apiVersion_ = bundleInfo.targetVersion % API_VERSION_MOD;
        GetPidBundleInfoMap().emplace(pid, bundleInfoPtr);
        TLOGD(WmsLogTag::DEFAULT, "pid: %{public}u, bundle name: %{public}s",
            pid, bundleInfo.name.c_str());
        return bundleInfoPtr;
    } else {
        TLOGE(WmsLogTag::DEFAULT, "Failed");
        return nullptr;
    }
}
      
std::shared_ptr<BundleInfo> SysCapUtil::UpdateBundleInfo(uint32_t pid, sptr<IRemoteObject> agent)
{
    TLOGD(WmsLogTag::DEFAULT, "update pid:%{public}d bundle info", pid);
    auto bundleInfo = UpdateBundleInfo(pid);
    if (bundleInfo == nullptr) {
        return nullptr;
    }
    std::unique_lock<std::shared_mutex> lock(pidBundleNameMutex_);
    GetAgentPidMap().emplace(agent, pid);
    return bundleInfo;
}
      
std::shared_ptr<BundleInfo> SysCapUtil::GetBundleInfo(uint32_t pid)
{
    TLOGD(WmsLogTag::DEFAULT, "get pid:%{public}d bundle info", pid);
    std::shared_lock<std::shared_mutex> lock(pidBundleNameMutex_);
    auto& map = GetPidBundleInfoMap();
    auto it = map.find(pid);
    if (it == map.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}

void SysCapUtil::RemoveBundleInfo(sptr<IRemoteObject> agent)
{
    int32_t pid;
    std::unique_lock<std::shared_mutex> lock(pidBundleNameMutex_);
    auto& agentPidMap = GetAgentPidMap();
    auto it = agentPidMap.find(agent);
    if (it == agentPidMap.end()) {
        return;
    }
    pid = it->second;
    if (pid == INVALID_PID) {
        return;
    }
    agentPidMap.erase(it);
    TLOGI(WmsLogTag::DEFAULT, "remove pid:%{public}d bundle info", pid);
    GetPidBundleInfoMap().erase(pid);
}
} // namespace Rosen
} // namespace OHOS