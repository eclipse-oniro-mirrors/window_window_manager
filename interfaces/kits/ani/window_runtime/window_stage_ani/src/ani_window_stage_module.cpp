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

#include "ani_window_stage.h"

#include <memory>

#include "ani.h"
#include "ani_window.h"
#include "ani_window_manager.h"
#include "ani_window_utils.h"
#include "ani_transition_controller.h"
#include "permission.h"
#include "pixel_map.h"
#include "pixel_map_taihe_ani.h"
#include "window_manager_hilog.h"
#include "window_scene.h"
#include "interop_js/arkts_esvalue.h"
#include "interop_js/arkts_interop_js_api.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "js_window_stage.h"
#include "ani_extension_window_config.h"

using OHOS::Rosen::WindowScene;
using namespace OHOS::Rosen;
static ani_object WindowStageCreate(ani_env* env, ani_long scene)
{
    std::shared_ptr<WindowScene> scenePtr;
    return CreateAniWindowStage(env, scenePtr); // just for test
}

static ani_ref WindowGetMainWindow(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[ANI]");
    AniWindowStage* windowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (windowStage == nullptr || windowStage->GetWindowScene().lock() == nullptr) {
        TLOGD(WmsLogTag::DEFAULT, "[ANI] windowStage is nullptr");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    return windowStage->GetMainWindow(env);
}

static ani_ref CreateSubWindow(ani_env* env, ani_object obj, ani_long nativeObj, ani_string name)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    AniWindowStage* windowStage = reinterpret_cast<AniWindowStage*>(nativeObj);
    if (windowStage == nullptr || windowStage->GetWindowScene().lock() == nullptr) {
        TLOGD(WmsLogTag::DEFAULT, "[ANI] windowStage is nullptr");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    return windowStage->OnCreateSubWindow(env, name);
}

extern "C" {
ANI_EXPORT ani_status ExtensionWindowConfig_ANI_Constructor(ani_vm *vm, uint32_t *result);
std::array g_methods = {
    ani_native_function {"loadContentSync",
        "lC{std.core.String}C{arkui.stateManagement.storage.localStorage.LocalStorage}:",
        reinterpret_cast<void *>(AniWindowStage::LoadContent)},
    ani_native_function {"loadContentByNameSync",
        "lC{std.core.String}C{arkui.stateManagement.storage.localStorage.LocalStorage}:",
        reinterpret_cast<void *>(AniWindowStage::LoadContentByName)},
    ani_native_function {"disableWindowDecorSync", nullptr,
        reinterpret_cast<void *>(AniWindowStage::DisableWindowDecor)},
    ani_native_function {"setWindowRectAutoSave", "lzz:",
        reinterpret_cast<void *>(AniWindowStage::SetWindowRectAutoSave)},
    ani_native_function {"isWindowRectAutoSave", "l:z",
        reinterpret_cast<void *>(AniWindowStage::IsWindowRectAutoSave)},
    ani_native_function {"setShowOnLockScreenSync",
        nullptr, reinterpret_cast<void *>(AniWindowStage::SetShowOnLockScreen)},
    ani_native_function {"setWindowModal", "lz:",
        reinterpret_cast<void *>(AniWindowStage::SetWindowModal)},
    ani_native_function {"getMainWindowSync", "l:C{@ohos.window.window.Window}",
        reinterpret_cast<void *>(WindowGetMainWindow)},
    ani_native_function {"createSubWindowSync", "lC{std.core.String}:C{@ohos.window.window.Window}",
        reinterpret_cast<void *>(CreateSubWindow)},
    ani_native_function {"getSubWindowSync", "l:C{std.core.Array}",
        reinterpret_cast<void *>(AniWindowStage::GetSubWindow)},
    ani_native_function {"createSubWindowWithOptionsSync",
        "lC{std.core.String}C{@ohos.window.window.SubWindowOptions}:C{@ohos.window.window.Window}",
        reinterpret_cast<void *>(AniWindowStage::CreateSubWindowWithOptions)},
    ani_native_function {"onSync", nullptr,
        reinterpret_cast<void *>(AniWindowStage::RegisterWindowCallback)},
    ani_native_function {"offSync", nullptr,
        reinterpret_cast<void *>(AniWindowStage::UnregisterWindowCallback)},
    ani_native_function {"setImageForRecentSync", "lX{C{std.core.Long}C{@ohos.multimedia.image.image.PixelMap}}i:",
        reinterpret_cast<void *>(AniWindowStage::SetImageForRecent)},
    ani_native_function {"removeImageForRecentSync", "l:",
        reinterpret_cast<void *>(AniWindowStage::RemoveImageForRecent)},
    ani_native_function {"setCustomDensitySync", "ldz:",
        reinterpret_cast<void *>(AniWindowStage::SetCustomDensity)},
    ani_native_function {"setDefaultDensityEnabledSync", "lz:",
        reinterpret_cast<void *>(AniWindowStage::SetDefaultDensityEnabled)},
    ani_native_function {"removeStartingWindowSync", "l:",
        reinterpret_cast<void *>(AniWindowStage::RemoveStartingWindow)},
    ani_native_function {"setSupportedWindowModes", "lC{std.core.Array}:",
        reinterpret_cast<void *>(AniWindowStage::SetSupportedWindowModes)},
    ani_native_function {"setSupportedWindowModes", "lC{std.core.Array}z:",
        reinterpret_cast<void *>(AniWindowStage::SetSupportedWindowModesWithGrayOutMaximizeButton)},
};

std::array s_methods = {
    ani_native_function {"nativeTransferStatic", "C{std.interop.ESValue}:C{std.core.Object}",
        reinterpret_cast<void *>(AniWindowStage::NativeTransferStatic)},
    ani_native_function {"nativeTransferDynamic", "l:C{std.interop.ESValue}",
        reinterpret_cast<void *>(AniWindowStage::NativeTransferDynamic)},
};
std::array g_functions = {
    ani_native_function {"CreateWindowStage", "l:C{@ohos.window.window.WindowStageInternal}",
        reinterpret_cast<void *>(WindowStageCreate)},
    ani_native_function {"getLastWindowSync", nullptr, reinterpret_cast<void *>(AniWindowManager::GetLastWindow)},
    ani_native_function {"shiftAppWindowFocusSync", "lii:",
        reinterpret_cast<void *>(AniWindowManager::ShiftAppWindowFocus)},
    ani_native_function {"getAllMainWindowInfo", "l:C{std.core.Array}",
        reinterpret_cast<void *>(AniWindowManager::GetAllMainWindowInfo)},
    ani_native_function {"getMainWindowSnapshot",
        "lC{std.core.Array}C{@ohos.window.window.WindowSnapshotConfiguration}:C{std.core.Array}",
        reinterpret_cast<void *>(AniWindowManager::GetMainWindowSnapshot)},
    ani_native_function {"createWindowSync",
        "lC{@ohos.window.window.Configuration}:C{@ohos.window.window.Window}",
        reinterpret_cast<void *>(AniWindowManager::CreateWindow)},
    ani_native_function {"findWindowSync",
        "lC{std.core.String}:C{@ohos.window.window.Window}",
        reinterpret_cast<void *>(AniWindowManager::FindWindow)},
    ani_native_function {"minimizeAllSync", "ll:", reinterpret_cast<void *>(AniWindowManager::MinimizeAll)},
    ani_native_function {"minimizeAllWithExclusionSync", "lli:",
        reinterpret_cast<void *>(AniWindowManager::MinimizeAllWithExclusion)},
    ani_native_function {"destroyAniExtConfig", nullptr, reinterpret_cast<void *>(AniExtensionWindowConfig::Finalizer)},
};

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_status ret;
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] null env");
        return ANI_NOT_FOUND;
    }

    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.window.WindowStageInternal", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] can't find class %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    for (auto method : g_methods) {
        if ((ret = env->Class_BindNativeMethods(cls, &method, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] bind window static method fail %{public}u, %{public}s, %{public}s",
                ret, method.name, method.signature);
            return ANI_NOT_FOUND;
        }
    }
    for (auto method : s_methods) {
        if ((ret = env->Class_BindStaticNativeMethods(cls, &method, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] bind window static method fail %{public}u, %{public}s, %{public}s",
                ret, method.name, method.signature);
            return ANI_NOT_FOUND;
        }
    }
    *result = ANI_VERSION_1;

    // just for test
    ani_namespace ns;
    if ((ret = env->FindNamespace("@ohos.window.window", &ns)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] find ns %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    for (auto method : g_functions) {
        if ((ret = env->Namespace_BindNativeFunctions(ns, &method, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] bind window static method fail %{public}u, %{public}s, %{public}s",
                ret, method.name, method.signature);
            return ANI_NOT_FOUND;
        }
    }
    AniWindowManager::AniWindowManagerInit(env, ns);
    OHOS::Rosen::ANI_Transition_Controller_Constructor(vm, result);
    OHOS::Rosen::ANI_Window_Constructor(vm, result);
    ExtensionWindowConfig_ANI_Constructor(vm, result);
    return ANI_OK;
}
}