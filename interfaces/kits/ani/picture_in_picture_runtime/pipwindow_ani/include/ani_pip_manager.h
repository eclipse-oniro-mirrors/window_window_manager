/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef ANI_PIP_MANAGER_H
#define ANI_PIP_MANAGER_H

#include "picture_in_picture_manager.h"
#include "ani.h"

namespace OHOS {
namespace Rosen {
// use for PiPContent.ets only
class AniPipManager {
public:
    AniPipManager();
    ~AniPipManager();
    static ani_status Init(ani_env* env, ani_namespace nsp);
    static void InitXComponentController(ani_env* env, ani_long nativeObj, ani_object xComponentController);
    static ani_ref GetCustomUIController(ani_env* env, ani_long nativeObj);
    static ani_ref GetTypeNode(ani_env* env, ani_long nativeObj);
    static void SetTypeNodeEnabled(ani_env* env, ani_long nativeObj);
    static void SetPipNodeType(ani_env* env, ani_long nativeObj, ani_object typeNode, ani_boolean markPip);
    static void RegisterCallback(ani_env* env, ani_long nativeObj, ani_string type, ani_ref callback);
    static void UnregisterCallback(ani_env* env, ani_long nativeObj, ani_string type);

private:

    void OnInitXComponentController(ani_env* env, ani_object xComponentController);
    ani_ref OnGetCustomUIController(ani_env* env);
    ani_ref OnGetTypeNode(ani_env* env);
    void OnRegisterCallback(ani_env* env, ani_string type, ani_ref callback);
    void OnUnregisterCallback(ani_env* env, ani_string type);
    void OnSetPipNodeType(ani_env* env, ani_object typeNode, ani_boolean markPip);
    void OnSetTypeNodeEnabled(ani_env* env);
};

ani_status ANI_Manager_Constructor(ani_vm *vm, uint32_t *result);
} // namespace Rosen
} // namespace OHOS
#endif // ANI_PIP_MANAGER_H