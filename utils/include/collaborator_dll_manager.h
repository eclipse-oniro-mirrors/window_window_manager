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
 
#ifndef OHOS_ROSEN_COLLABORATOR_DLL_MANAGER_H
#define OHOS_ROSEN_COLLABORATOR_DLL_MANAGER_H
 
#include <mutex>
 
#include "want.h"
#include "session_info.h"
 
namespace OHOS {
namespace Rosen {
using PreHandleStartAbilityFunc = void (*)(AAFwk::Want&, sptr<AAFwk::SessionInfo>&, int32_t);
 
class CollaboratorDllWrapper {
public:
    CollaboratorDllWrapper() = default;
    ~CollaboratorDllWrapper();
    bool InitDlSymbol(const char* name, const char* funcName);
    PreHandleStartAbilityFunc GetPreHandleStartAbilityFunc();
private:
    void* preHandleStartAbilityHandle_ = nullptr;
    PreHandleStartAbilityFunc preHandleStartAbilityFunc_ = nullptr;
    std::mutex preHandleStartAbilityFuncLock_;
};
 
class CollaboratorDllManager {
public:
    static void PreHandleStartAbility(AAFwk::Want& want, sptr<AAFwk::SessionInfo>& sessionInfo, int32_t userId);
private:
    static CollaboratorDllWrapper& GetCollaboratorDllWrapper();
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_COLLABORATOR_DLL_MANAGER_H