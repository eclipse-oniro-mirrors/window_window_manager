/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_transition_controller.h"
#include "js_runtime_utils.h"
#include "window.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "window_option.h"
#include "js_window.h"
#include "permission.h"
#include "js_err_utils.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsTransitionController"};
}

static int g_jsTransCxtCtorCnt = 0;
static int g_jsTransCxtDtorCnt = 0;
static int g_jsTransCtrlCtorCnt = 0;
static int g_jsTransCtrlDtorCnt = 0;

JsTransitionContext::JsTransitionContext(sptr<Window> window, bool isShownTransContext)
    : windowToken_(window), isShownTransContext_(isShownTransContext)
{
    WLOGFI("constructorCnt: %{public}d", ++g_jsTransCxtCtorCnt);
}

JsTransitionContext::~JsTransitionContext()
{
    WLOGFI("deConstructorCnt: %{public}d", ++g_jsTransCxtDtorCnt);
}

void JsTransitionContext::Finalizer(napi_env env, void* data, void* hint)
{
    WLOGFI("[NAPI]");
    std::unique_ptr<JsTransitionContext>(static_cast<JsTransitionContext*>(data));
}

napi_value JsTransitionContext::CompleteTransition(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsTransitionContext* me = CheckParamsAndGetThis<JsTransitionContext>(env, info);
    return (me != nullptr) ? me->OnCompleteTransition(env, info) : nullptr;
}

napi_value JsTransitionContext::OnCompleteTransition(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "not system app, permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }

    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool transitionCompleted = false;
    if (!ConvertFromJsValue(env, argv[0], transitionCompleted)) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    WMError ret = WMError::WM_OK;
    auto window = windowToken_.promote();
    if (window == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    auto state = window->GetWindowState();
    if (!isShownTransContext_) {
        if (state != WindowState::STATE_HIDDEN) {
            WLOGI("Window [%{public}u, %{public}s] Hidden context called but window is not hidden: %{public}u",
                window->GetWindowId(), window->GetWindowName().c_str(), static_cast<uint32_t>(state));
            return NapiGetUndefined(env);
        }
        window->UpdateSurfaceNodeAfterCustomAnimation(false); // remove from rs tree after animation
        if (!transitionCompleted) {
            ret = window->Show(); // hide aborted
        }
    } else {
        if (state != WindowState::STATE_SHOWN) {
            WLOGI("Window [%{public}u, %{public}s] shown context called but window is not shown: %{public}u",
                window->GetWindowId(), window->GetWindowName().c_str(), static_cast<uint32_t>(state));
            return NapiGetUndefined(env);
        }
        if (!transitionCompleted) {
            ret = window->Hide(); // show aborted
        }
    }
    if (ret != WMError::WM_OK) {
        return NapiThrowError(env, WM_JS_TO_ERROR_CODE_MAP.at(ret));
    }
    WLOGI("Window [%{public}u, %{public}s] CompleteTransition %{public}d end",
        window->GetWindowId(), window->GetWindowName().c_str(), transitionCompleted);
    return NapiGetUndefined(env);
}

static napi_value CreateJsTransitionContextObject(napi_env env, std::shared_ptr<NativeReference> jsWin,
    sptr<Window> window, bool isShownTransContext)
{
    WLOGFI("[NAPI]");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }
    std::unique_ptr<JsTransitionContext> jsTransContext = std::make_unique<JsTransitionContext>(window,
        isShownTransContext);
    napi_wrap(env, objValue, jsTransContext.release(), JsTransitionContext::Finalizer, nullptr, nullptr);
    if (jsWin != nullptr && jsWin->GetNapiValue() != nullptr) {
        napi_set_named_property(env, objValue, "toWindow", jsWin->GetNapiValue());
    } else {
        WLOGFE("jsWin is nullptr");
        return nullptr;
    }

    const char *moduleName = "JsTransitionContext";
    BindNativeFunction(env, objValue, "completeTransition", moduleName, JsTransitionContext::CompleteTransition);
    return objValue;
}

JsTransitionController::JsTransitionController(napi_env env, std::shared_ptr<NativeReference> jsWin,
    sptr<Window> window)
    : env_(env), jsWin_(jsWin), windowToken_(window), weakRef_(wptr<JsTransitionController> (this))
{
    WLOGFI("constructorCnt: %{public}d", ++g_jsTransCtrlCtorCnt);
}

JsTransitionController::~JsTransitionController()
{
    WLOGFI("deConstructorCnt: %{public}d", ++g_jsTransCtrlDtorCnt);
}

void JsTransitionController::AnimationForShown()
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[NAPI]");
    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env_, lastParam, &result);
    auto asyncTask = [self = weakRef_, task = napiAsyncTask]() {
        auto thisController = self.promote();
        if (thisController == nullptr) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "this transition Controller is null!");
            return;
        }
        HandleScope handleScope(thisController->env_);
        auto jsWin = thisController->jsWin_.lock();
        auto window = thisController->windowToken_.promote();
        if (jsWin == nullptr || window == nullptr) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "native window or jsWindow is null!");
            return;
        }
        auto state = window->GetWindowState();
        if (state != WindowState::STATE_SHOWN) {
            TLOGE(WmsLogTag::WMS_ANIMATION,
                "animation shown configuration for state %{public}u not support!",
                static_cast<uint32_t>(state));
            return;
        }
        napi_value jsTransContextObj = CreateJsTransitionContextObject(
            thisController->env_, jsWin, window, true);
        if (jsTransContextObj == nullptr) {
            return;
        }
        napi_value argv[] = { jsTransContextObj };
        thisController->CallJsMethod("animationForShown", argv, ArraySize(argv));
        // add to rs tree before animation
        window->UpdateSurfaceNodeAfterCustomAnimation(true);
    };
    if (napi_send_event(env_, asyncTask, napi_eprio_high, "AnimationForShown") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to run napiAsyncTask for animationForShown");
    }
}

void JsTransitionController::AnimationForHidden()
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "[NAPI]");
    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env_, lastParam, &result);
    auto asyncTask = [self = weakRef_, task = napiAsyncTask]() {
        auto thisController = self.promote();
        if (thisController == nullptr) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "this transition Controller is null!");
            return;
        }
        HandleScope handleScope(thisController->env_);
        auto jsWin = thisController->jsWin_.lock();
        auto window = thisController->windowToken_.promote();
        if (jsWin == nullptr || window == nullptr) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "native window or jsWindow is null!");
            return;
        }
        auto state = window->GetWindowState();
        if (state != WindowState::STATE_HIDDEN) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "animation hidden configuration for state %{public}u not support!",
                static_cast<uint32_t>(state));
            return;
        }
        napi_value jsTransContextObj = CreateJsTransitionContextObject(
            thisController->env_, jsWin, window, false);
        if (jsTransContextObj == nullptr) {
            return;
        }
        napi_value argv[] = { jsTransContextObj };
        thisController->CallJsMethod("animationForHidden", argv, ArraySize(argv));
    };
    if (napi_send_event(env_, asyncTask, napi_eprio_high, "AnimationForHidden") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to run napiAsyncTask for animationForHidden");
    }
}

void JsTransitionController::CallJsMethod(const std::string& methodName, napi_value const * argv, size_t argc)
{
    WLOGI("Call js function:%{public}s.", methodName.c_str());
    auto self = jsTransControllerObj_.lock();
    if (self == nullptr) {
        WLOGFE("JsController is null!");
        return;
    }
    auto jsControllerObj = self->GetNapiValue();
    if (jsControllerObj == nullptr) {
        WLOGFE("JsControllerObj is null!");
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, jsControllerObj, methodName.c_str(), &method);
    if (method == nullptr || GetType(env_, method) == napi_undefined) {
        WLOGFE("Failed to get %{public}s from object", methodName.c_str());
        return;
    }
    napi_call_function(env_, jsControllerObj, method, argc, argv, nullptr);
}

void JsTransitionController::SetJsController(std::shared_ptr<NativeReference> jsVal)
{
    jsTransControllerObj_ = jsVal;
}
}
}