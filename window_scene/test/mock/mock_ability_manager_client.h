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

#ifndef MOCK_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H
#define MOCK_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H

#include <ability_manager_client.h>

namespace OHOS {
namespace AAFwk {
class MockAbilityManagerClient {
public:
    static void ClearAll();
    static void SetUIExtensionRootHostInfo(const UIExtensionHostInfo& hostInfo);
    static void SetUIExtensionSessionInfo(const UIExtensionSessionInfo& sessionInfo);
    static void SetStartUIExtensionAbilityResult(ErrCode result);
    static sptr<SessionInfo> GetLastStartUIExtensionAbilityInfo();
};
}
}
#endif // MOCK_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H