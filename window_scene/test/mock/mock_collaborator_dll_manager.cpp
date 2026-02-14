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
 
#include "mock_collaborator_dll_manager.h"
#include "collaborator_dll_manager.h"
 
namespace OHOS::Rosen {
namespace {
int32_t g_callerTypeForAnco = 0;
}
 
void MockCollaboratorDllManager::MockPreHandleStartAbility(int32_t callerTypeForAnco)
{
    g_callerTypeForAnco = callerTypeForAnco;
}
 
void CollaboratorDllManager::PreHandleStartAbility(AAFwk::Want& want, sptr<AAFwk::SessionInfo>& sessionInfo,
    int32_t userId)
{
    sessionInfo->callerTypeForAnco = g_callerTypeForAnco;
}
 
CollaboratorDllWrapper& CollaboratorDllManager::GetCollaboratorDllWrapper()
{
    static CollaboratorDllWrapper dll;
    return dll;
}
}