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

#include "ani.h"
#include <hitrace_meter.h>

#include "window_manager_hilog.h"
#include "floating_ball_manager.h"
#include "floating_ball_option.h"
#include "ani_fb_window.h"
#include "ani_fb_window_controller.h"
#include "ani_fb_window_utils.h"

namespace OHOS {
namespace Rosen {
AniFbWindow::AniFbWindow() {TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]AniFbWindow generate");}
AniFbWindow::~AniFbWindow() {TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]~AniFbWindow destroy");}

ani_status AniFbWindow::Init(ani_env* env, ani_namespace nsp)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]null env");
        return ANI_ERROR;
    }
    ani_function setObjFunc = nullptr;
    ani_status ret = env->Namespace_FindFunction(nsp, "setNativeObj", "l:", &setObjFunc);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]find setNativeObj func failed %{public}u", ret);
        return ret;
    }
    std::unique_ptr<AniFbWindow> aniFbWindow = std::make_unique<AniFbWindow>();
    ret = env->Function_Call_Void(setObjFunc, aniFbWindow.release());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]call setNativeObj func failed %{public}u", ret);
        return ret;
    }
    return ret;
}

void AniFbWindow::AddAniFbControllerObj(AniFbController* aniFbController, ani_object obj)
{
    std::lock_guard<std::mutex> lock(mtxLocalObjs_);
    localObjs.insert(std::pair(aniFbController, obj));
}

ani_status AniFbWindow::DelAniFbControllerObj(AniFbController* aniFbController)
{
    std::lock_guard<std::mutex> lock(mtxLocalObjs_);
    auto it = localObjs.find(aniFbController);
    if (it != localObjs.end()) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]del aniFbController");
        delete aniFbController;
        AniFbWindow::localObjs.erase(it);
        return ANI_OK;
    }
    TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]del failed");
    return ANI_ERROR;
}

ani_ref AniFbWindow::Create(ani_env* env, ani_long nativeObj, ani_object floatingBallConfigurations)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    AniFbWindow* fbWindowAni = reinterpret_cast<AniFbWindow*>(nativeObj);
    return fbWindowAni != nullptr ? fbWindowAni->OnCreate(env, floatingBallConfigurations) : nullptr;
}

ani_ref AniFbWindow::OnCreate(ani_env* env, ani_object floatingBallConfigurations)
{
    if (floatingBallConfigurations == nullptr) {
        return AniThrowError<WMError>(env, WMError::WM_ERROR_INVALID_PARAM, "[FB]floatingBallConfigurations is null");
    }
    void* contextPtr = nullptr;
    if (GetContextPtr(env, floatingBallConfigurations, contextPtr) == ANI_ERROR) {
        return AniThrowError<WMError>(env, WMError::WM_ERROR_INVALID_PARAM,
            "[FB]Invalid parameters in config, please check if context is null");
    }
    return AniFbWindow::CreateFbController(env, contextPtr);
}

ani_ref AniFbWindow::CreateFbController(ani_env* env, void* contextPtr)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");

    if (!AniFbWindow::IsFloatingBallEnabledAni(env)) {
        return AniThrowError<WMError>(env, WMError::WM_ERROR_DEVICE_NOT_SUPPORT,
            "[FB]device not support floatingBall");
    }

    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
    if (context == nullptr) {
        return AniThrowError<WMError>(env, WMError::WM_ERROR_FB_INTERNAL_ERROR, "[FB]Invalid context");
    }

    sptr<Window> mainWindow = Window::GetMainWindowWithContext(context->lock());
    if (mainWindow == nullptr) {
        return AniThrowError<WMError>(env, WMError::WM_ERROR_FB_INTERNAL_ERROR, "[FB]Invalid mainWindow");
    }
    
    sptr<FloatingBallController> fbController =
        new FloatingBallController(mainWindow, mainWindow->GetWindowId(), contextPtr);
    return CreateAniFbControllerObject(env, fbController);
}

ani_boolean AniFbWindow::IsFloatingBallEnabledAni(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    bool isSupportFloatingBall = FloatingBallManager::IsSupportFloatingBall();
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]IsFloatingBallEnabledAni = %{public}d", isSupportFloatingBall);
    return static_cast<ani_boolean>(isSupportFloatingBall);
}
}  // namespace Rosen
}  // namespace OHOS


// total entrance
extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    // ets ANI Construct
    using namespace OHOS::Rosen;
    ani_status ret;
    ani_env* env;
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]ANI_Constructor start");
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]null env");
        return ANI_NOT_FOUND;
    }
    // find namespace from ets file
    ani_namespace nsp;
    if ((ret = env->FindNamespace("@ohos.window.floatingBall.floatingBall", &nsp)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]ANI_Constructor failed to find namespace, %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    // Bind ets and cpp
    std::array funcs = {
        ani_native_function{"isFloatingBallEnabled", ":z",
            reinterpret_cast<void*>(AniFbWindow::IsFloatingBallEnabledAni)},
        ani_native_function{"createSync", nullptr,
            reinterpret_cast<void*>(AniFbWindow::Create)},
        ani_native_function{"fbControllerFinalizerCallback", "l:",
            reinterpret_cast<void*>(AniFbController::Finalizer)},
    };
    // Bind each function in the array
    for (const auto& func : funcs) {
        if ((ret = env->Namespace_BindNativeFunctions(nsp, &func, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]bind namespace fail %{public}u, %{public}s, %{public}s",
                ret, func.name, func.signature);
            return ANI_NOT_FOUND;
        }
    }
    // register APIs for ani_fb_window_controller.cpp
    ANI_Controller_Constructor(vm, result);
    // Init and log
    if ((ret = AniFbWindow::Init(env, nsp)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]AniFbWindow Init failed");
        return ANI_ERROR;
    }
    *result = ANI_VERSION_1;
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]ANI_Constructor Init finish");
    return ANI_OK;
    }
}  // extern "C"