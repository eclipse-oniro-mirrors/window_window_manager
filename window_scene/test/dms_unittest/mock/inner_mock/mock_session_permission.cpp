/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "session_permission.h"

namespace OHOS {
namespace Rosen {

bool SessionPermission::IsSystemServiceCalling(bool needPrintLog)
{
    return true;
}

bool SessionPermission::IsSystemCalling()
{
    return true;
}

bool SessionPermission::IsSystemAppCall()
{
    return true;
}

bool SessionPermission::IsSystemAppCallByCallingTokenID(uint32_t callingTokenId)
{
    return true;
}

bool SessionPermission::IsSACalling()
{
    return true;
}

bool SessionPermission::VerifyCallingPermission(const std::string& permissionName)
{
    return true;
}

bool SessionPermission::VerifyPermissionByCallerToken(const uint32_t callerToken, const std::string& permissionName)
{
    return true;
}

bool SessionPermission::VerifySessionPermission()
{
    return true;
}

bool SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()
{
    return true;
}

bool SessionPermission::IsShellCall()
{
    return true;
}

bool SessionPermission::IsStartByHdcd()
{
    return true;
}

bool SessionPermission::IsStartedByInputMethod()
{
    return true;
}

bool SessionPermission::IsKeyboardCallingProcess(int32_t pid, uint32_t callingWindowId)
{
    return true;
}

bool SessionPermission::IsSameBundleNameAsCalling(const std::string& bundleName)
{
    return true;
}

bool SessionPermission::IsSameAppAsCalling(const std::string& bundleName, const std::string& appIdentifier)
{
    return true;
}

bool SessionPermission::IsStartedByUIExtension()
{
    return true;
}

bool SessionPermission::CheckCallingIsUserTestMode(pid_t pid)
{
    return true;
}

bool SessionPermission::IsBetaVersion()
{
    return true;
}

bool SessionPermission::IsFoundationCall()
{
    return true;
}

std::string SessionPermission::GetCallingBundleName()
{
    return "";
}

bool SessionPermission::VerifyPermissionByBundleName(
    const std::string& bundleName, const std::string& permissionName, uint32_t userId)
{
    return true;
}

bool SessionPermission::IsTokenNativeOrShellType(uint32_t tokenId)
{
    return true;
}

}  // Rosen
}  // OHOS