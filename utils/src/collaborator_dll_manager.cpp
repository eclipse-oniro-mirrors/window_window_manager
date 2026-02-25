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
 
#include <dlfcn.h>
 
#include "collaborator_dll_manager.h"
#include "ipc_skeleton.h"
#include "window_manager_hilog.h"
 
namespace OHOS {
namespace Rosen {
namespace {
constexpr const char* URI_CHECK_BROKER_SO_NAME = "libams_broker_ext.z.so";
constexpr const char* URI_CHECK_BROKER_FUNC_NAME = "PreHandleStartAbility";
}
 
CollaboratorDllWrapper::~CollaboratorDllWrapper()
{
    if (preHandleStartAbilityHandle_ != nullptr) {
        dlclose(preHandleStartAbilityHandle_);
    }
    preHandleStartAbilityFunc_ = nullptr;
}
 
bool CollaboratorDllWrapper::InitDlSymbol(const char* name, const char* funcName)
{
    std::lock_guard<std::mutex> lock(preHandleStartAbilityFuncLock_);
    if (preHandleStartAbilityFunc_ != nullptr) {
        TLOGD(WmsLogTag::WMS_SCB, "preHandleStartAbilityFunc is valid.");
        return true;
    }
    if (preHandleStartAbilityHandle_ == nullptr) {
        preHandleStartAbilityHandle_ = dlopen(name, RTLD_NOW);
        if (preHandleStartAbilityHandle_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SCB, "dlopen failed %{public}s, %{public}s", name, dlerror());
            return false;
        }
    }
    preHandleStartAbilityFunc_ = reinterpret_cast<PreHandleStartAbilityFunc>(dlsym(preHandleStartAbilityHandle_,
        funcName));
    if (preHandleStartAbilityFunc_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "dlsym failed %{public}s, %{public}s", funcName, dlerror());
        dlclose(preHandleStartAbilityHandle_);
        preHandleStartAbilityHandle_ = nullptr;
        return false;
    }
    return true;
}
 
PreHandleStartAbilityFunc CollaboratorDllWrapper::GetPreHandleStartAbilityFunc()
{
    std::lock_guard<std::mutex> lock(preHandleStartAbilityFuncLock_);
    return preHandleStartAbilityFunc_;
}
 
CollaboratorDllWrapper& CollaboratorDllManager::GetCollaboratorDllWrapper()
{
    static CollaboratorDllWrapper dll;
    return dll;
}
 
void CollaboratorDllManager::PreHandleStartAbility(AAFwk::Want& want, sptr<AAFwk::SessionInfo>& sessionInfo,
    int32_t userId)
{
    auto& dll = GetCollaboratorDllWrapper();
    (void)dll.InitDlSymbol(URI_CHECK_BROKER_SO_NAME, URI_CHECK_BROKER_FUNC_NAME);
    
    PreHandleStartAbilityFunc func = dll.GetPreHandleStartAbilityFunc();
    if (func != nullptr) {
        func(want, sessionInfo, userId);
    }
}
}
}