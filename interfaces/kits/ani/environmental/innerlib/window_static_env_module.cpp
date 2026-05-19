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

#include "ani.h"
#include "ani_extension_window.h"
#include "ani_window.h"
#include "ui_content.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace OHOS;
using namespace OHOS::Rosen;
namespace {
static sptr<Window> GetWindowByInstanceId(int32_t instanceId)
{
    int32_t windowId = Ace::UIContent::GetUIContentWindowID(instanceId);
    auto uicontent = Ace::UIContent::GetUIContent(instanceId);
    if (!uicontent) {
        TLOGE(WmsLogTag::DEFAULT, "uicontent nullptr instanceId: %{public}d, windowId: %{public}d",
            instanceId, windowId);
        return nullptr;
    }
    auto window = sptr<Window>(uicontent->GetUIContentWindow());
    if (window == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "window nullptr %{public}d", windowId);
        return nullptr;
    }
    return window;
}

static ani_ref FindWindowById(ani_env* env, ani_long instanceId)
{
    auto window = GetWindowByInstanceId(instanceId);
    if (window == nullptr) {
        return nullptr;
    }
    if (window->GetType() == WindowType::WINDOW_TYPE_UI_EXTENSION) {
        return FindAniExtensionWindowById(env, window->GetWindowPersistentId());
    }
    return CreateAniWindowObject(env, window);
}

static ani_long GetDisplayId(ani_env* env, ani_long instanceId)
{
    auto window = GetWindowByInstanceId(instanceId);
    if (window == nullptr) {
        return static_cast<ani_long>(DISPLAY_ID_INVALID);
    }
    if (window->GetType() == WindowType::WINDOW_TYPE_UI_EXTENSION && !window->GetIsAtomicService()) {
        return static_cast<ani_long>(DISPLAY_ID_INVALID);
    }
    return static_cast<ani_long>(window->GetDisplayId());
}
}


extern "C" {
std::array g_functions = {
    ani_native_function {"findWindowById", "l:C{std.core.Object}",
        reinterpret_cast<void *>(FindWindowById)},
    ani_native_function {"getDisplayId", "l:l",
        reinterpret_cast<void *>(GetDisplayId)},
};

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI] window static env start!");
    ani_status ret;
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] null env");
        return ANI_NOT_FOUND;
    }
    *result = ANI_VERSION_1;
    ani_namespace ns;
    if ((ret = env->FindNamespace("@ohos.windowenv.env_native", &ns)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] find ns %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    for (auto method : g_functions) {
        if ((ret = env->Namespace_BindNativeFunctions(ns, &method, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] bind window static env method fail %{public}u, %{public}s, %{public}s",
                ret, method.name, method.signature);
            return ANI_NOT_FOUND;
        }
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI] window static env end!");
    return ANI_OK;
}
}