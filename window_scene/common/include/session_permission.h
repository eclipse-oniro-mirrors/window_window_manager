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

#ifndef OHOS_ROSEN_SESSION_PERMISSION_H
#define OHOS_ROSEN_SESSION_PERMISSION_H

#include <string>

namespace OHOS {
namespace Rosen {
namespace PermissionConstants {
constexpr const char* PERMISSION_MANAGE_MISSION = "ohos.permission.MANAGE_MISSIONS";
constexpr const char* PERMISSION_KILL_APP_PROCESS = "ohos.permission.KILL_APP_PROCESSES";
constexpr const char* PERMISSION_MAIN_WINDOW_TOPMOST = "ohos.permission.WINDOW_TOPMOST";
constexpr const char* PERMISSION_CALLED_EXTENSION_ON_LOCK_SCREEN = "ohos.permission.CALLED_UIEXTENSION_ON_LOCK_SCREEN";
constexpr const char* PERMISSION_WRITE_APP_LOCK = "ohos.permission.WRITE_APP_LOCK";
constexpr const char* PERMISSION_SET_ABILITY_INSTANCE_INFO = "ohos.permission.SET_ABILITY_INSTANCE_INFO";
constexpr const char* PERMISSION_VISIBLE_WINDOW_INFO = "ohos.permission.VISIBLE_WINDOW_INFO";
constexpr const char* PERMISSION_WINDOW_TRANSPARENT = "ohos.permission.SET_WINDOW_TRANSPARENT";
constexpr const char* PERMISSION_FLOATING_BALL = "ohos.permission.USE_FLOAT_BALL";
}
class SessionPermission {
public:
    static bool IsSystemServiceCalling(bool needPrintLog = true);
    static bool IsSystemCalling();
    static bool VerifySessionPermission();
    static bool JudgeCallerIsAllowedToUseSystemAPI();
    static bool IsShellCall();
    static bool IsStartByHdcd();
    static bool IsStartedByInputMethod();
    static bool IsKeyboardCallingProcess(int32_t pid, uint32_t callingWindowId);
    static bool IsSACalling();
    static bool VerifyCallingPermission(const std::string& permissionName);
    static bool VerifyPermissionByCallerToken(const uint32_t callerToken, const std::string& permissionName);
    static bool VerifyPermissionByBundleName(
        const std::string& bundleName, const std::string& permissionName, uint32_t userId);
    static bool IsSameBundleNameAsCalling(const std::string& bundleName);
    static bool IsSameAppAsCalling(const std::string& bundleName, const std::string& appIdentifier);
    static bool IsStartedByUIExtension();
    static bool CheckCallingIsUserTestMode(pid_t pid);
    static bool IsBetaVersion();
    static bool IsSystemAppCall();
    static bool IsSystemAppCallByCallingTokenID(uint32_t callingTokenId);
    static bool IsFoundationCall();
    static std::string GetCallingBundleName();
    static bool IsTokenNativeOrShellType(uint32_t tokenId);
};
} // Rosen
} // OHOS
#endif // OHOS_ROSEN_SESSION_PERMISSION_H