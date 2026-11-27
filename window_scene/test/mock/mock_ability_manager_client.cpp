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

#include "mock_ability_manager_client.h"

#include <session_info.h>

namespace OHOS {
namespace AAFwk {
static std::shared_ptr<UIExtensionHostInfo> g_uiExtensionRootHostInfo;
static std::shared_ptr<UIExtensionSessionInfo> g_uiExtensionSessionInfo;
static ErrCode g_startUIExtensionAbilityResult = ERR_OK;
static sptr<SessionInfo> g_lastStartUIExtensionAbilityInfo;

void MockAbilityManagerClient::ClearAll()
{
    g_uiExtensionRootHostInfo = nullptr;
    g_uiExtensionSessionInfo = nullptr;
    g_startUIExtensionAbilityResult = ERR_OK;
    g_lastStartUIExtensionAbilityInfo = nullptr;
}

void MockAbilityManagerClient::SetUIExtensionRootHostInfo(const UIExtensionHostInfo& hostInfo)
{
    g_uiExtensionRootHostInfo = std::make_shared<UIExtensionHostInfo>();
    g_uiExtensionRootHostInfo->elementName_.SetBundleName(hostInfo.elementName_.GetBundleName());
}

void MockAbilityManagerClient::SetUIExtensionSessionInfo(const UIExtensionSessionInfo& sessionInfo)
{
    g_uiExtensionSessionInfo = std::make_shared<UIExtensionSessionInfo>();
    g_uiExtensionSessionInfo->hostElementName.SetBundleName(sessionInfo.hostElementName.GetBundleName());
    g_uiExtensionSessionInfo->persistentId = sessionInfo.persistentId;
    g_uiExtensionSessionInfo->hostWindowId = sessionInfo.hostWindowId;
    g_uiExtensionSessionInfo->extensionAbilityType = sessionInfo.extensionAbilityType;
}

void MockAbilityManagerClient::SetStartUIExtensionAbilityResult(ErrCode result)
{
    g_startUIExtensionAbilityResult = result;
}

sptr<SessionInfo> MockAbilityManagerClient::GetLastStartUIExtensionAbilityInfo()
{
    return g_lastStartUIExtensionAbilityInfo;
}

ErrCode AbilityManagerClient::GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token,
    UIExtensionHostInfo& hostInfo, int32_t userId)
{
    if (g_uiExtensionRootHostInfo) {
        hostInfo.elementName_.SetBundleName(g_uiExtensionRootHostInfo->elementName_.GetBundleName());
    }
    return 0;
}

ErrCode AbilityManagerClient::GetUIExtensionSessionInfo(const sptr<IRemoteObject> token,
    UIExtensionSessionInfo& sessionInfo, int32_t userId)
{
    if (g_uiExtensionSessionInfo) {
        sessionInfo.hostElementName.SetBundleName(g_uiExtensionSessionInfo->hostElementName.GetBundleName());
        sessionInfo.persistentId = g_uiExtensionSessionInfo->persistentId;
        sessionInfo.hostWindowId = g_uiExtensionSessionInfo->hostWindowId;
        sessionInfo.extensionAbilityType = g_uiExtensionSessionInfo->extensionAbilityType;
    }
    return 0;
}

ErrCode AbilityManagerClient::StartUIExtensionAbility(sptr<SessionInfo> extensionSessionInfo,
    int32_t userId)
{
    g_lastStartUIExtensionAbilityInfo = extensionSessionInfo;
    return g_startUIExtensionAbilityResult;
}
}
}
