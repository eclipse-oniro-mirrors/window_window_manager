/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "ani.h"
#include "ani_embeddable_window_stage.h"
#include "window_manager_hilog.h"

extern "C" {
ANI_EXPORT ani_status ExtensionWindow_ANI_Constructor(ani_vm *vm, uint32_t* result);
ANI_EXPORT ani_status ExtensionWindowHost_ANI_Constructor(ani_vm *vm, uint32_t* result);
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t* result)
{
    using namespace OHOS::Rosen;
    ani_status ret;
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] null env");
        return ANI_NOT_FOUND;
    }

    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.window.WindowStage", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] null env %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    *result = ANI_VERSION_1;

    ret = ExtensionWindow_ANI_Constructor(vm, result);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] Constructor ExtensionWindow failed: %{public}u", ret);
        return ret;
    }
    ret = ExtensionWindowHost_ANI_Constructor(vm, result);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI] Constructor ExtensionWindowHost failed: %{public}u", ret);
        return ret;
    }
    return ANI_OK;
}
}
