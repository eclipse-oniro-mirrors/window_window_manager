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

#include <accesstoken_kit.h>
#include <app_mgr_client.h>
#include <bundle_constants.h>
#include <ipc_skeleton.h>
#include <bundle_mgr_proxy.h>
#include <bundle_mgr_interface.h>
#include <system_ability_definition.h>
#include <iservice_registry.h>
#include <tokenid_kit.h>
#ifdef IMF_ENABLE
#include <input_method_controller.h>
#endif // IMF_ENABLE
#include <singleton.h>
#include <singleton_container.h>
#include <pwd.h>
#include "common/include/session_permission.h"
#include "parameters.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "SessionPermission"};
constexpr int32_t FOUNDATION_UID = 5523;

sptr<AppExecFwk::IBundleMgr> GetBundleManagerProxy()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        WLOGFE("Failed to get system ability mgr.");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject
        = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        WLOGFE("Failed to get bundle manager service.");
        return nullptr;
    }
    auto bundleManagerServiceProxy = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (!bundleManagerServiceProxy || !bundleManagerServiceProxy->AsObject()) {
        WLOGFE("Failed to get bundle manager proxy.");
        return nullptr;
    }
    return bundleManagerServiceProxy;
}
}

bool SessionPermission::IsSystemServiceCalling(bool needPrintLog)
{
    const auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (IsTokenNativeOrShellType(tokenId)) {
        TLOGD(WmsLogTag::DEFAULT, "system service calling");
        return true;
    }
    if (needPrintLog) {
        TLOGE(WmsLogTag::DEFAULT, "Not system service calling");
    }
    return false;
}

bool SessionPermission::IsSystemCalling()
{
    const auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (IsTokenNativeOrShellType(tokenId)) {
        return true;
    }
    uint64_t accessTokenIDEx = IPCSkeleton::GetCallingFullTokenID();
    bool isSystemApp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(accessTokenIDEx);
    return isSystemApp;
}

bool SessionPermission::IsSystemAppCall()
{
    uint64_t callingTokenId = IPCSkeleton::GetCallingFullTokenID();
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(callingTokenId);
}

bool SessionPermission::IsSystemAppCallByCallingTokenID(uint32_t callingTokenId)
{
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(callingTokenId);
}

bool SessionPermission::IsSACalling()
{
    const auto tokenId = IPCSkeleton::GetCallingTokenID();
    const auto flag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (flag == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        TLOGD(WmsLogTag::DEFAULT, "SA called, tokenId:%{private}u, flag:%{public}u", tokenId, flag);
        return true;
    }
    TLOGD(WmsLogTag::DEFAULT, "Not SA called, tokenId:%{private}u, flag:%{public}u", tokenId, flag);
    return false;
}

bool SessionPermission::VerifyCallingPermission(const std::string& permissionName)
{
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    TLOGD(WmsLogTag::DEFAULT, "permission %{public}s, callingTokenID:%{private}u",
        permissionName.c_str(), callerToken);
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TLOGE(WmsLogTag::DEFAULT,
            "permission %{public}s: PERMISSION_DENIED, callingTokenID:%{private}u, ret:%{public}d",
            permissionName.c_str(), callerToken, ret);
        return false;
    }
    TLOGD(WmsLogTag::DEFAULT, "Verify AccessToken success. permission %{public}s, callingTokenID:%{private}u",
        permissionName.c_str(), callerToken);
    return true;
}

bool SessionPermission::VerifyPermissionByCallerToken(const uint32_t callerToken, const std::string& permissionName)
{
    TLOGD(WmsLogTag::DEFAULT, "permission %{public}s, callingTokenID:%{private}u",
        permissionName.c_str(), callerToken);
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TLOGE(WmsLogTag::DEFAULT,
            "permission %{public}s: PERMISSION_DENIED, callingTokenID:%{private}u, ret:%{public}d",
            permissionName.c_str(), callerToken, ret);
        return false;
    }
    TLOGD(WmsLogTag::DEFAULT, "Verify AccessToken success. permission %{public}s, callingTokenID:%{private}u",
        permissionName.c_str(), callerToken);
    return true;
}

bool SessionPermission::VerifySessionPermission()
{
    if (IsSACalling()) {
        TLOGD(WmsLogTag::DEFAULT, "Is SA Call, Permission verified success.");
        return true;
    }
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGD(WmsLogTag::DEFAULT, "MANAGE permission verified success.");
        return true;
    }
    WLOGFW("Permission verified failed.");
    return false;
}

bool SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()
{
    if (IsSACalling() || IsShellCall()) {
        return true;
    }
    auto callerToken = IPCSkeleton::GetCallingFullTokenID();
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(callerToken);
}

bool SessionPermission::IsShellCall()
{
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        TLOGD(WmsLogTag::DEFAULT, "TokenType is Shell, verify success");
        return true;
    }
    TLOGD(WmsLogTag::DEFAULT, "Not Shell called. tokenId:%{private}u, type:%{public}u", callerToken, tokenType);
    return false;
}

bool SessionPermission::IsStartByHdcd()
{
    OHOS::Security::AccessToken::NativeTokenInfo info;
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    if (!IsTokenNativeOrShellType(tokenId)) {
        return false;
    }
    if (Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, info) != 0) {
        return false;
    }
    if (info.processName.compare("hdcd") == 0) {
        return true;
    }
    return false;
}

bool SessionPermission::IsStartedByInputMethod()
{
#ifdef IMF_ENABLE
    auto imc = MiscServices::InputMethodController::GetInstance();
    if (!imc) {
        TLOGE(WmsLogTag::DEFAULT, "InputMethodController is nullptr");
        return false;
    }
    int pid = IPCSkeleton::GetCallingPid();
    return imc->IsCurrentImeByPid(pid);
#else
    return false;
#endif // IMF_ENABLE
}

bool SessionPermission::IsKeyboardCallingProcess(int32_t pid, uint32_t callingWindowId)
{
#ifdef IMF_ENABLE
    auto imc = MiscServices::InputMethodController::GetInstance();
    if (!imc) {
        TLOGE(WmsLogTag::DEFAULT, "InputMethodController is nullptr");
        return false;
    }
    return imc->IsKeyboardCallingProcess(pid, callingWindowId);
#else
    return false;
#endif
}

bool SessionPermission::IsSameBundleNameAsCalling(const std::string& bundleName)
{
    if (bundleName == "") {
        return false;
    }
    auto bundleManagerServiceProxy_ = GetBundleManagerProxy();
    if (!bundleManagerServiceProxy_) {
        WLOGFE("failed to get BundleManagerServiceProxy");
        return false;
    }
    int uid = IPCSkeleton::GetCallingUid();
    // reset ipc identity
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    std::string callingBundleName;
    bundleManagerServiceProxy_->GetNameForUid(uid, callingBundleName);
    IPCSkeleton::SetCallingIdentity(identity);
    if (callingBundleName == bundleName) {
        WLOGFD("verify bundle name success");
        return true;
    } else {
        WLOGFE("verify bundle name failed, calling bundle name %{public}s, but window bundle name %{public}s.",
            callingBundleName.c_str(), bundleName.c_str());
        return false;
    }
}

bool SessionPermission::IsSameAppAsCalling(const std::string& bundleName, const std::string& appIdentifier)
{
    if (bundleName == "" || appIdentifier == "") {
        return false;
    }
    auto bundleManagerServiceProxy = GetBundleManagerProxy();
    if (!bundleManagerServiceProxy) {
        TLOGE(WmsLogTag::DEFAULT, "failed to get BundleManagerServiceProxy");
        return false;
    }
    int uid = IPCSkeleton::GetCallingUid();
    // reset ipc identity
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    std::string callingBundleName;
    bundleManagerServiceProxy->GetNameForUid(uid, callingBundleName);
    if (callingBundleName != bundleName) {
        TLOGE(WmsLogTag::DEFAULT, "verify app failed, callingBundleName %{public}s, bundleName %{public}s.",
              callingBundleName.c_str(), bundleName.c_str());
        IPCSkeleton::SetCallingIdentity(identity);
        return false;
    }
    AppExecFwk::BundleInfo bundleInfo;
    int userId = uid / 200000; // 200000 use uid to caculate userId
    bool ret = bundleManagerServiceProxy->GetBundleInfoV9(
        callingBundleName, static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO),
        bundleInfo, userId);
    IPCSkeleton::SetCallingIdentity(identity);

    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed to query app info, callingBundleName:%{public}s, userId:%{public}d",
              callingBundleName.c_str(), userId);
        return false;
    }

    if (bundleInfo.signatureInfo.appIdentifier == appIdentifier) {
        TLOGD(WmsLogTag::DEFAULT, "verify app success");
        return true;
    }

    TLOGE(WmsLogTag::DEFAULT, "verify app failed, callingBundleName %{public}s, bundleName %{public}s.",
          callingBundleName.c_str(), bundleName.c_str());
    return false;
}

bool SessionPermission::IsStartedByUIExtension()
{
    auto bundleManagerServiceProxy = GetBundleManagerProxy();
    if (!bundleManagerServiceProxy) {
        WLOGFE("failed to get BundleManagerServiceProxy");
        return false;
    }

    int uid = IPCSkeleton::GetCallingUid();
    // reset ipc identity
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    std::string bundleName;
    bundleManagerServiceProxy->GetNameForUid(uid, bundleName);
    AppExecFwk::BundleInfo bundleInfo;
    int userId = uid / 200000; // 200000 use uid to caculate userId
    bool result = bundleManagerServiceProxy->GetBundleInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO, bundleInfo, userId);
    // set ipc identity to raw
    IPCSkeleton::SetCallingIdentity(identity);
    if (!result) {
        WLOGFE("failed to query extension ability info, bundleName:%{public}s, userId:%{public}d",
               bundleName.c_str(), userId);
        return false;
    }

    auto extensionInfo = std::find_if(bundleInfo.extensionInfos.begin(), bundleInfo.extensionInfos.end(),
        [](AppExecFwk::ExtensionAbilityInfo extensionInfo) {
            return (extensionInfo.type == AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI);
        });
    return extensionInfo != bundleInfo.extensionInfos.end();
}

bool SessionPermission::CheckCallingIsUserTestMode(pid_t pid)
{
    TLOGD(WmsLogTag::DEFAULT, "Calling proxy func");
    bool isUserTestMode = false;
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "AppMgrClient is null!");
        return false;
    }
    // reset ipc identity
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = appMgrClient->CheckCallingIsUserTestMode(pid, isUserTestMode);
    // set ipc identity to raw
    IPCSkeleton::SetCallingIdentity(identity);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::DEFAULT, "Permission denied! ret=%{public}d", ret);
        return false;
    }
    return isUserTestMode;
}

bool SessionPermission::IsBetaVersion()
{
    std::string betaName = OHOS::system::GetParameter("const.logsystem.versiontype", "");
    return betaName.find("beta") != std::string::npos;
}

bool SessionPermission::IsFoundationCall()
{
    return IPCSkeleton::GetCallingUid() == FOUNDATION_UID;
}

std::string SessionPermission::GetCallingBundleName()
{
    auto bundleManagerServiceProxy = GetBundleManagerProxy();
    if (!bundleManagerServiceProxy) {
        WLOGFE("failed to get BundleManagerServiceProxy");
        return "";
    }
    int uid = IPCSkeleton::GetCallingUid();
    // reset ipc identity
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    std::string callingBundleName;
    bundleManagerServiceProxy->GetNameForUid(uid, callingBundleName);
    // if bundlename is empty, fill in pw_name
    if (callingBundleName.empty()) {
        if (struct passwd* user = getpwuid(uid)) {
            callingBundleName = user->pw_name;
        }
    }
    IPCSkeleton::SetCallingIdentity(identity);
    return callingBundleName;
}

bool SessionPermission::VerifyPermissionByBundleName(
    const std::string& bundleName, const std::string& permissionName, uint32_t userId)
{
    auto bundleManagerServiceProxy = GetBundleManagerProxy();
    if (!bundleManagerServiceProxy) {
        TLOGW(WmsLogTag::DEFAULT, "failed to get BundleManagerServiceProxy");
        return false;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    AppExecFwk::BundleInfo bundleInfo;
    bool result = bundleManagerServiceProxy->GetBundleInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId);
    IPCSkeleton::SetCallingIdentity(identity);
    if (!result) {
        TLOGW(WmsLogTag::DEFAULT, "get bundle info failed!");
        return false;
    }
    return VerifyPermissionByCallerToken(bundleInfo.applicationInfo.accessTokenId, permissionName);
}

bool SessionPermission::IsTokenNativeOrShellType(uint32_t tokenId)
{
    const auto flag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    TLOGD(WmsLogTag::DEFAULT, "tokenId:%{private}u, flag:%{public}u", tokenId, flag);
    if (flag != Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE &&
        flag != Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        return false;
    }
    return true;
}
} // namespace Rosen
} // namespace OHOS