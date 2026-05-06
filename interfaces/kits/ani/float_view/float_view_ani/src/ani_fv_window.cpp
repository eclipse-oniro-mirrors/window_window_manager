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

#include "ani_fv_window.h"

#include "floating_ball_manager.h"
#include "floating_ball_option.h"
#include "float_view_manager.h"
#include "float_view_option.h"
#include "float_window_manager.h"

#include "ani.h"
#include <hitrace_meter.h>
#include "window_manager_hilog.h"
#include "singleton_container.h"
#include "permission.h"
#include "ani_fb_window_controller.h"

namespace OHOS {
namespace Rosen {
namespace {
const std::string FLOATING_BALL_PERMISSION = "ohos.permission.USE_FLOAT_BALL";
const std::string FLOAT_VIEW_PERMISSION = "ohos.permission.FLOAT_VIEW";
}
AniFvWindow::AniFvWindow() {TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]AniFvWindow generate");}
AniFvWindow::~AniFvWindow() {TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]~AniFvWindow destroy");}

ani_status AniFvWindow::Init(ani_env* env, ani_namespace nsp)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]null env");
        return ANI_ERROR;
    }
    ani_function setObjFunc = nullptr;
    ani_status ret = env->Namespace_FindFunction(nsp, "setNativeObj", "l:", &setObjFunc);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]find setNativeObj func failed %{public}u", ret);
        return ret;
    }
    std::unique_ptr<AniFvWindow> aniFvWindow = std::make_unique<AniFvWindow>();
    ret = env->Function_Call_Void(setObjFunc, aniFvWindow.release());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]call setNativeObj func failed %{public}u", ret);
        return ret;
    }
    return ret;
}

void AniFvWindow::AddAniFvControllerObj(AniFvController* aniFvController, ani_object obj)
{
    std::lock_guard<std::mutex> lock(mtxLocalObjs_);
    localObjs.insert(std::pair(aniFvController, obj));
}

ani_status AniFvWindow::DelAniFvControllerObj(AniFvController* aniFvController)
{
    std::lock_guard<std::mutex> lock(mtxLocalObjs_);
    auto it = localObjs.find(aniFvController);
    if (it != localObjs.end()) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]del aniFvController");
        delete aniFvController;
        AniFvWindow::localObjs.erase(it);
        return ANI_OK;
    }
    TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]del failed");
    return ANI_ERROR;
}

ani_ref AniFvWindow::Create(ani_env* env, ani_long nativeObj, ani_object floatViewConfigurations)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]start");
    AniFvWindow* fvWindowAni = reinterpret_cast<AniFvWindow*>(nativeObj);
    return fvWindowAni != nullptr ? fvWindowAni->OnCreate(env, floatViewConfigurations) : nullptr;
}

ani_ref AniFvWindow::OnCreate(ani_env* env, ani_object floatViewConfigurations)
{
    if (floatViewConfigurations == nullptr) {
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_INVALID_PARAM, "floatViewConfigurations is null");
    }
    void* contextPtr = nullptr;
    if (AniFvUtils::GetContextPtr(env, floatViewConfigurations, contextPtr) == ANI_ERROR) {
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_INVALID_PARAM,
            "Invalid parameters in config, please check if context is null");
    }
    uint32_t templateType = 0;
    if (AniFvUtils::GetTemplateType(env, floatViewConfigurations, templateType) == ANI_ERROR) {
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_INVALID_PARAM,
            "Invalid parameters in config, please check if templateType is null");
    }
    return CreateFvController(env, contextPtr, templateType);
}

ani_ref AniFvWindow::CreateFvController(ani_env* env, void* contextPtr, uint32_t templateType)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]start");

    if (!FloatViewManager::isSupportFloatView_) {
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_DEVICE_NOT_SUPPORT, "device not support floatView");
    }

    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
    if (context == nullptr) {
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_NULLPTR, "Invalid context");
    }

    if (templateType >= static_cast<uint32_t>(FloatViewTemplate::END)) {
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_ILLEGAL_PARAM, "template type is invalid");
    }
    
    FvOption option;
    option.SetContext(contextPtr);
    option.SetTemplate(templateType);
    sptr<FloatViewController> fvController = sptr<FloatViewController>::MakeSptr(option, env);
    return CreateAniFvControllerObject(env, fvController);
}

ani_boolean AniFvWindow::IsFloatViewEnabledAni(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]IsFloatViewEnabledAni = %{public}d", FloatViewManager::isSupportFloatView_);
    return static_cast<ani_boolean>(FloatViewManager::isSupportFloatView_);
}

ani_object AniFvWindow::GetFloatViewLimitsAni(ani_env* env, ani_int templateType)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]GetFloatViewLimitsAni start");
    if (!FloatViewManager::isSupportFloatView_) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]Device do not support float view");
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_DEVICE_NOT_SUPPORT,
            "Device do not support float view.");
    }
    uint32_t templateType_ = static_cast<uint32_t>(templateType);
    if (templateType_ >= static_cast<uint32_t>(FloatViewTemplate::END)) {
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_ILLEGAL_PARAM, "template type is invalid");
    }
    FloatViewLimits limits;
    WMError errCode = SingletonContainer::Get<WindowManager>().GetFloatViewLimits(templateType_, limits);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]Failed to get global float view limits");
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_SYSTEM_ABNORMALLY,
            "Failed to get global float view limits.");
    }
    auto jsObject = AniFvUtils::CreateAniFloatViewLimitsObject(env, limits);
    if (jsObject == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]Failed to create js object");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    return jsObject;
}

ani_object AniFvWindow::BindAni(ani_env* env, ani_object fvController, ani_object fbController, ani_object aniFbOption)
{
    if (fbController == nullptr || fvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]null controller exists");
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_ILLEGAL_PARAM, "null controller exists.");
    }
    ani_long fbAddress;
    ani_long fvAddress;
    if (!AniFvUtils::GetNativeAddress(env, fbController,
        "@ohos.window.floatingBall.floatingBall.FbControllerInternal", "nativeObj", fbAddress) ||
        !AniFvUtils::GetNativeAddress(env, fvController,
        "@ohos.window.floatView.floatView.FvControllerInternal", "nativeObj", fvAddress)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]no controller in ani controller");
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_ILLEGAL_PARAM,
            "Failed to get controllers from js objects.");
    }
    auto aniFbController = reinterpret_cast<AniFbController*>(fbAddress);
    auto aniFvController = reinterpret_cast<AniFvController*>(fvAddress);
    FbOption fbOption;
    if (!AniFbController::GetFbOption(env, aniFbOption, fbOption)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]Failed to convert fb option.");
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_ILLEGAL_PARAM, "Failed to convert fb option.");
    }

    auto nFbController = aniFbController->GetController();
    auto nFvController = aniFvController->GetController();
    if (nFbController == nullptr || nFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "one of the controller is null");
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_NULLPTR);
    }
    if (!FloatViewManager::isSupportFloatView_ || !FloatingBallManager::IsSupportFloatingBall()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "device not support");
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    }
    if (!Permission::CheckCallingPermission(FLOATING_BALL_PERMISSION) ||
        !Permission::CheckCallingPermission(FLOAT_VIEW_PERMISSION)) {
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_INVALID_PERMISSION, "no permission.");
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "start bind controller");
    WMError err = FloatWindowManager::Bind(nFvController, nFbController, fbOption);
    if (err != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "bind controller failed");
        return AniFvUtils::AniThrowError(env, err, "bind controller failed.");
    }
    return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
}

ani_object AniFvWindow::UnBindAni(ani_env* env, ani_object fvController, ani_object fbController)
{
    if (fbController == nullptr || fvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]null controller exists");
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_ILLEGAL_PARAM, "null controller exists.");
    }
    ani_long fbAddress;
    ani_long fvAddress;
    if (!AniFvUtils::GetNativeAddress(env, fbController,
        "@ohos.window.floatingBall.floatingBall.FbControllerInternal", "nativeObj", fbAddress) ||
        !AniFvUtils::GetNativeAddress(env, fvController,
        "@ohos.window.floatView.floatView.FvControllerInternal", "nativeObj", fvAddress)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]no controller in ani controller");
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_ILLEGAL_PARAM,
            "Failed to get controllers from js objects.");
    }
    auto aniFbController = reinterpret_cast<AniFbController*>(fbAddress);
    auto aniFvController = reinterpret_cast<AniFvController*>(fvAddress);
    auto nFbController = aniFbController->GetController();
    auto nFvController = aniFvController->GetController();
    if (nFbController == nullptr || nFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "one of the controller is null");
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_NULLPTR);
    }
    if (!FloatViewManager::isSupportFloatView_ || !FloatingBallManager::IsSupportFloatingBall()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "device not support");
        return AniFvUtils::AniThrowError(env, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    }
    WMError err = FloatWindowManager::UnBind(nFvController, nFbController);
    if (err != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "unbind controller failed");
        return AniFvUtils::AniThrowError(env, err, "unbind controller failed.");
    }
    return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
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
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]ANI_Constructor start");
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]null env");
        return ANI_NOT_FOUND;
    }
    // find namespace from ets file
    ani_namespace nsp;
    if ((ret = env->FindNamespace("@ohos.window.floatView.floatView", &nsp)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]ANI_Constructor failed to find namespace, %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    // Bind ets and cpp
    std::array funcs = {
        ani_native_function{"fvControllerFinalizerCallback", nullptr,
            reinterpret_cast<void*>(AniFvController::Finalizer)},
        ani_native_function{"isFloatViewEnabled", nullptr,
            reinterpret_cast<void*>(AniFvWindow::IsFloatViewEnabledAni)},
        ani_native_function{"getFloatViewLimits", nullptr,
            reinterpret_cast<void*>(AniFvWindow::GetFloatViewLimitsAni)},
        ani_native_function{"createSync", nullptr,
            reinterpret_cast<void*>(AniFvWindow::Create)},
        ani_native_function{"bindSync", nullptr,
            reinterpret_cast<void*>(AniFvWindow::BindAni)},
        ani_native_function{"unbindSync", nullptr,
            reinterpret_cast<void*>(AniFvWindow::UnBindAni)},
    };
    // Bind each function in the array
    for (const auto& func : funcs) {
        if ((ret = env->Namespace_BindNativeFunctions(nsp, &func, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]bind namespace fail %{public}u, %{public}s, %{public}s",
                ret, func.name, func.signature);
            return ANI_NOT_FOUND;
        }
    }
    // register APIs for ani_fv_window_controller.cpp
    ANI_Controller_Constructor(vm, result);
    // Init and log
    if ((ret = AniFvWindow::Init(env, nsp)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]AniFvWindow Init failed");
        return ANI_ERROR;
    }
    *result = ANI_VERSION_1;
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]ANI_Constructor Init finish");
    return ANI_OK;
}
}  // extern "C"