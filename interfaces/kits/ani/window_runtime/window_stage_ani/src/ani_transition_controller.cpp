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

#include "ani_transition_controller.h"
#include "ani_window_utils.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "window_option.h"
#include "permission.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

AniTransitionContext::AniTransitionContext(sptr<Window> window, bool isShownTransContext)
    : windowToken_(window), isShownTransContext_(isShownTransContext)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] constructor");
}

AniTransitionContext::~AniTransitionContext()
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] deContructor");
}

void AniTransitionContext::Finalizer(ani_env* env, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    std::unique_ptr<AniTransitionContext>(reinterpret_cast<AniTransitionContext*>(nativeObj));
}

void AniTransitionContext::CompleteTransition(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_boolean isCompleted)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]In");
    AniTransitionContext* context = reinterpret_cast<AniTransitionContext*>(nativeObj);
    if (context == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Transition context is null");
        return;
    }
    context->OnCompleteTransition(env, isCompleted);
}

void AniTransitionContext::OnCompleteTransition(ani_env* env, ani_boolean isCompleted)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Not system app, permission denied");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
        return;
    }

    WMError ret = WMError::WM_OK;
    auto window = windowToken_.promote();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Native widnow is null");
        AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return;
    }
    auto state = window->GetWindowState();
    if (!isShownTransContext_) {
        if (state != WindowState::STATE_HIDDEN) {
            TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] is not hidden with state %{public}d",
                window->GetWindowId(), window->GetWindowName().c_str(), static_cast<uint32_t>(state));
                return;
        }
        window->UpdateSurfaceNodeAfterCustomAnimation(false); // remove from rs tree after animation
        if (!isCompleted) {
            ret = window->Show();
        }
    } else {
        if (state != WindowState::STATE_SHOWN) {
            TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] is not shown with state %{public}d",
                window->GetWindowId(), window->GetWindowName().c_str(), static_cast<uint32_t>(state));
                return;
        }
        if (!isCompleted) {
            ret = window->Hide();
        }
    }
    if (ret != WMError::WM_OK) {
        AniWindowUtils::AniThrowError(env, WM_JS_TO_ERROR_CODE_MAP.at(ret));
        return;
    }
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] Window [%{public}u, %{public}s] completeTransition %{public}d end",
        window->GetWindowId(), window->GetWindowName().c_str(), isCompleted);
}

ani_object AniTransitionContext::CreateAniObj(ani_env* env, ani_ref aniWindowObj,
    sptr<Window> window, ani_boolean isShownTransContext)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]");
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Null env");
        return nullptr;
    }

    ani_status ret = ANI_OK;
    ani_class cls {};
    if ((ret = env->FindClass("@ohos.window.window.TransitionContextInternal", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Find class TransitionContextInternal failed %{public}u", ret);
        return nullptr;
    }
    std::unique_ptr<AniTransitionContext> transContext = std::make_unique<AniTransitionContext>(window,
        isShownTransContext);
    ani_method initFunc {};
    if ((ret = env->Class_FindMethod(cls, "<ctor>", ":", &initFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get ctor failed %{public}u", ret);
        return nullptr;
    }
    ani_object obj {};
    if ((ret = env->Object_New(cls, initFunc, &obj)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Obj new failed %{public}u", ret);
        return nullptr;
    }
    ani_method objFunc {};
    if ((ret = env->Class_FindMethod(cls, "setNativeObj", "l:", &objFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Find setNativeObj failed %{public}u", ret);
        return nullptr;
    }
    env->Object_CallMethod_Void(obj, objFunc, reinterpret_cast<ani_long>(transContext.release()));
    if (aniWindowObj != nullptr) {
        ret = AniWindowUtils::CallAniMethodVoid(env, obj, cls, "<set>toWindow", nullptr, aniWindowObj);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Set toWindow value failed %{public}u", ret);
            return nullptr;
        }
    }
    TLOGD(WmsLogTag::WMS_ANIMATION, "[ANI] Transition context create success");
    return obj;
}

ani_status AniTransitionContext::AniTransitionContextInit(ani_env* env)
{
    ani_status ret = ANI_OK;
    ani_namespace ns {};
    if ((ret = env->FindNamespace("@ohos.window.window", &ns)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Find ns failed. ret: %{public}u", ret);
        return ret;
    }
    std::array functions = {
        ani_native_function { "transContextFinalizerCallback", nullptr,
            reinterpret_cast<void *>(AniTransitionContext::Finalizer) },
    };
    if ((ret = env->Namespace_BindNativeFunctions(ns, functions.data(), functions.size())) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Bind ns functions failed. ret: %{public}u", ret);
        return ret;
    }

    ani_class cls {};
    if ((ret = env->FindClass("@ohos.window.window.TransitionContextInternal", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Find class TransitionContextInternal failed %{public}u", ret);
        return ret;
    }

    std::array methods = {
        ani_native_function { "completeTransitionSync", "lz:",
            reinterpret_cast<void *>(AniTransitionContext::CompleteTransition) },
    };
    for (auto method: methods) {
        // 1u means the number of native methods in method
        if ((ret = env->Class_BindNativeMethods(cls, &method, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Bind methd failed. %{public}u, %{public}s, %{public}s",
                ret, method.name, method.signature);
            return ret;
        }
    }
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] Transition context init success");
    return ANI_OK;
}

AniTransitionController::AniTransitionController(ani_env* env, ani_ref aniWindowObj, sptr<Window> window)
    : env_(env), aniWindowObj_(aniWindowObj), windowToken_(window), weakRef_(wptr<AniTransitionController>(this))
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "constructor");
    auto mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (!mainRunner) {
        return;
    }
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainRunner);
}

AniTransitionController::~AniTransitionController()
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "deConstructor");
    if (env_ == nullptr || aniTransControllerObj_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "env_ is null");
        return;
    }
    env_->GlobalReference_Delete(aniTransControllerObj_);
    aniTransControllerObj_ = nullptr;
}

void AniTransitionController::AnimationForShown()
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] In");
    auto asyncTask = [self = weakRef_, env = env_, this]() {
        auto thisController = self.promote();
        if (thisController == nullptr) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "This transition controller is null");
            return;
        }
        auto aniWindowObj = thisController->aniWindowObj_;
        auto window = thisController->windowToken_.promote();
        if (aniWindowObj == nullptr || window == nullptr) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Native window or aniWindow is null");
            return;
        }
        auto state = window->GetWindowState();
        if (state != WindowState::STATE_SHOWN) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Animation shown configuration for state %{public}u not support",
                static_cast<uint32_t>(state));
            return;
        }
        ani_object transitionControllerObj = thisController->GetAniTransControllerObj();
        ani_object transContextObj = AniTransitionContext::CreateAniObj(env, aniWindowObj, window, true);
        if (transContextObj == nullptr || transitionControllerObj == nullptr) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Create tansContext failed or aniTransControllerObj_ is null");
            return;
        }
        ani_ref fnRef {};
        auto ret = env->Object_GetPropertyByName_Ref(transitionControllerObj, "animationForShown", &fnRef);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Find animationForShown failed. ret: %{public}u", ret);
            return;
        }
        std::array<ani_ref, 1> args = { transContextObj };
        ani_ref fnReturnVal {};
        ret = env->FunctionalObject_Call(static_cast<ani_fn_object>(fnRef), args.size(), args.data(), &fnReturnVal);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Call animationForShown failed. ret: %{public}u", ret);
        }
        // add to rs tree before animation
        window->UpdateSurfaceNodeAfterCustomAnimation(true);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get main event handler failed");
        return;
    }
    eventHandler_->PostTask(asyncTask, __func__, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniTransitionController::AnimationForHidden()
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] In");
    auto asyncTask = [self = weakRef_, env = env_, this]() {
        auto thisController = self.promote();
        if (thisController == nullptr) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "This transition controller is null");
            return;
        }
        auto aniWindowObj = thisController->aniWindowObj_;
        auto window = thisController->windowToken_.promote();
        if (aniWindowObj == nullptr || window == nullptr) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Native window or aniWindow is null");
            return;
        }
        auto state = window->GetWindowState();
        if (state != WindowState::STATE_HIDDEN) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Animation shown configuration for state %{public}u not support",
                static_cast<uint32_t>(state));
            return;
        }
        ani_object transitionControllerObj = thisController->GetAniTransControllerObj();
        ani_object transContextObj = AniTransitionContext::CreateAniObj(env, aniWindowObj, window, false);
        if (transContextObj == nullptr || transitionControllerObj == nullptr) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Create tansContext failed or aniTransControllerObj_ is null");
            return;
        }
        ani_ref fnRef {};
        auto ret = env->Object_GetPropertyByName_Ref(transitionControllerObj, "animationForHidden", &fnRef);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Find animationForHidden failed. ret: %{public}u", ret);
            return;
        }
        std::array<ani_ref, 1> args = { transContextObj };
        ani_ref fnReturnVal {};
        ret = env->FunctionalObject_Call(static_cast<ani_fn_object>(fnRef), args.size(), args.data(), &fnReturnVal);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Call animationForHidden failed. ret: %{public}u", ret);
        }
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get main event handler failed");
        return;
    }
    eventHandler_->PostTask(asyncTask, __func__, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniTransitionController::SetAniTransControllerObj(const ani_object& aniObj)
{
    ani_ref g_obj {};
    auto ret = env_->GlobalReference_Create(aniObj, &g_obj);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Set global AniTransControllerObj failed. ret: %{public}u", ret);
        return;
    }
    aniTransControllerObj_ = reinterpret_cast<ani_object>(g_obj);
}

ani_object AniTransitionController::GetAniTransControllerObj() const
{
    return aniTransControllerObj_;
}

void AniTransitionController::Finalizer(ani_env* env, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]In");
}

sptr<AniTransitionController> AniTransitionController::CreateAniTransitionController(ani_env* env,
    ani_ref aniWindowObj, sptr<Window> window)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI]In");
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Null env");
        return nullptr;
    }
    ani_status ret = ANI_OK;
    ani_class cls {};
    if ((ret = env->FindClass("@ohos.window.window.TransitionControllerInternal", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Find class TransitionControllerInternal failed %{public}u", ret);
        return nullptr;
    }

    ani_method initFunc {};
    if ((ret = env->Class_FindMethod(cls, "<ctor>", ":", &initFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get ctor failed %{public}u", ret);
        return nullptr;
    }
    ani_object obj {};
    if ((ret = env->Object_New(cls, initFunc, &obj)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Obj new failed %{public}u", ret);
        return nullptr;
    }

    auto controller = sptr<AniTransitionController>::MakeSptr(env, aniWindowObj, window);
    ani_method objFunc {};
    if ((ret = env->Class_FindMethod(cls, "setNativeObj", "l:", &objFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Find setNativeObj failed %{public}u", ret);
        return nullptr;
    }
    ret = env->Object_CallMethod_Void(obj, objFunc, reinterpret_cast<ani_long>(controller.GetRefPtr()));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Call setNativeObj failed %{public}u", ret);
        return nullptr;
    }
    controller->SetAniTransControllerObj(obj);
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] Transition controller create success");
    return controller;
}

ani_status AniTransitionController::AniTransitionControllerInit(ani_env* env)
{
    ani_status ret = ANI_OK;
    ani_namespace ns {};
    if ((ret = env->FindNamespace("@ohos.window.window", &ns)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Find ns failed. ret: %{public}u", ret);
        return ret;
    }
    std::array functions = {
        ani_native_function { "transControllerFinalizerCallback", nullptr,
            reinterpret_cast<void *>(AniTransitionController::Finalizer) },
    };
    if ((ret = env->Namespace_BindNativeFunctions(ns, functions.data(), functions.size())) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Bind ns functions failed. ret: %{public}u", ret);
        return ret;
    }

    ani_class cls {};
    if ((ret = env->FindClass("@ohos.window.window.TransitionControllerInternal", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Find class TransitionControllerInternal failed %{public}u", ret);
        return ret;
    }
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] Transition context init success");
    return ANI_OK;
}

ani_status ANI_Transition_Controller_Constructor(ani_vm* vm, uint32_t *result)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[ANI] In");
    ani_env* env;
    ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get env failed. ret: %{public}u", ret);
        return ret;
    }
    ani_status contextInitRet = AniTransitionContext::AniTransitionContextInit(env);
    if (contextInitRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] transition context init failed with result: %{public}u", contextInitRet);
        return contextInitRet;
    }
    ani_status controllerInitRet = AniTransitionController::AniTransitionControllerInit(env);
    if (controllerInitRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] transition controller init failed with result: %{public}u",
            controllerInitRet);
        return controllerInitRet;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}
} // Rosen
} // OHOS