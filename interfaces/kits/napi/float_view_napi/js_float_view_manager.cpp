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
#include "js_float_view_manager.h"

#include "js_err_utils.h"
#include "js_float_view_utils.h"
#include "float_view_controller.h"
#include "float_view_manager.h"
#include "floating_ball_manager.h"
#include "float_window_manager.h"
#include "window_manager_hilog.h"
#include "window_manager.h"
#include "singleton_container.h"
#include "permission.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
const std::string FLOATING_BALL_PERMISSION = "ohos.permission.USE_FLOAT_BALL";
const std::string FLOAT_VIEW_PERMISSION = "ohos.permission.FLOAT_VIEW";
constexpr size_t ARG_COUNT_ONE = 1;
constexpr size_t ARG_COUNT_TWO = 2;
constexpr size_t ARG_COUNT_THREE = 3;
const char* ARKUI_WINDOW_FV_CREATE = "ArkUI.window.fv.create";
const char* ARKUI_WINDOW_FV_ISFLOATINGVIEWENABLED = "ArkUI.window.fv.isFloatingViewEnabled";
const char* ARKUI_WINDOW_FV_GETFLOATVIEWLIMITS = "ArkUI.window.fv.getFloatViewLimits";
const char* ARKUI_WINDOW_FV_BIND = "ArkUI.window.fv.bind";
const char* ARKUI_WINDOW_FV_UNBIND = "ArkUI.window.fv.unBind";
}
JsFloatViewManager::JsFloatViewManager()
{
}

JsFloatViewManager::~JsFloatViewManager()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "JsFloatViewManager release");
}

napi_value JsFloatViewWMInit(napi_env env, napi_value exportObj)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "JsFloatViewWMInit");
    if (env == nullptr || exportObj == nullptr) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "JsFloatViewWMInit env or exportObj is nullptr");
        return nullptr;
    }
    std::unique_ptr<JsFloatViewManager> jsFvManager = std::make_unique<JsFloatViewManager>();
    napi_wrap(env, exportObj, jsFvManager.release(), JsFloatViewManager::Finalizer, nullptr, nullptr);

    const char* moduleName = "JsFloatViewManager";
    BindNativeFunction(env, exportObj, "create", moduleName, JsFloatViewManager::CreateFloatViewController);
    BindNativeFunction(env, exportObj, "isFloatViewEnabled", moduleName, JsFloatViewManager::IsFloatViewEnabled);
    BindNativeFunction(env, exportObj, "getFloatViewLimits", moduleName, JsFloatViewManager::GetFloatViewLimits);
    BindNativeFunction(env, exportObj, "bind", moduleName, JsFloatViewManager::Bind);
    BindNativeFunction(env, exportObj, "unbind", moduleName, JsFloatViewManager::UnBind);
    InitFvEnums(env, exportObj);
    return NapiGetUndefined(env);
}

void JsFloatViewManager::Finalizer(napi_env env, void* data, void* hint)
{
    std::unique_ptr<JsFloatViewManager>(static_cast<JsFloatViewManager*>(data));
}

napi_value JsFloatViewManager::CreateFloatViewController(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "JsFloatViewManager create controller");
    JsFloatViewManager* me = CheckParamsAndGetThis<JsFloatViewManager>(env, info);
    return (me != nullptr) ? me->OnCreateFloatViewController(env, info) : nullptr;
}

napi_value JsFloatViewManager::OnCreateFloatViewController(napi_env env, napi_callback_info info)
{
    FvOption option;
    auto errMsg = CheckAndGetParam(env, info, option);
    if (!errMsg.empty()) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_CREATE, WmErrorCode::WM_ERROR_FB_PARAM_INVALID);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, errMsg);
    }
    return CreateFloatViewControllerTask(env, option);
}

napi_value JsFloatViewManager::CreateFloatViewControllerTask(napi_env env, const FvOption& option)
{
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    sptr<FloatViewController> floatViewController = sptr<FloatViewController>::MakeSptr(option, env);
    NapiAsyncTask::ExecuteCallback execute = [errCodePtr, floatViewController, option] {
        if (errCodePtr == nullptr) {
            return;
        }
        if (!FloatViewManager::isSupportFloatView_) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Device do not support float view");
            *errCodePtr = WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_CREATE, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
            return;
        }

        if (option.GetTemplate() >= static_cast<uint32_t>(FloatViewTemplate::END)) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "template type is invalid");
            *errCodePtr = WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_CREATE, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
            return;
        }

        auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(option.GetContext());
        if (context == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_CREATE, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
            return;
        }
        if (floatViewController == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_CREATE, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
            return;
        }
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr, floatViewController](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, CreateJsFloatViewControllerObject(env, floatViewController));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                    "JsFloatViewManager::OnCreateFloatViewController failed."));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFloatViewManager::OnCreateFloatViewController",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

std::string JsFloatViewManager::CheckAndGetParam(napi_env env, napi_callback_info info, FvOption& option)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    // 解析应用传参FloatViewConfiguration对象
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        return "Missing args when creating fvController";
    }

    napi_value config = argv[0];
    if (config == nullptr) {
        return "Failed to convert object to fvConfiguration or fvConfiguration is null";
    }
    bool hasContext = false;
    bool hasTemplate = false;
    napi_has_named_property(env, config, "context", &hasContext);
    napi_has_named_property(env, config, "templateType", &hasTemplate);
    if (!hasContext || !hasTemplate) {
        return "Missing context or templateType in configuration.";
    }
    napi_value contextPtrValue = nullptr;
    napi_get_named_property(env, config, "context", &contextPtrValue);
    void* contextPtr = nullptr;
    napi_unwrap(env, contextPtrValue, &contextPtr);
    if (contextPtr == nullptr) {
        return "Context is null.";
    }
    option.SetContext(contextPtr);

    napi_value templateTypeValue = nullptr;
    napi_get_named_property(env, config, "templateType", &templateTypeValue);
    if (templateTypeValue == nullptr) {
        return "Failed to convert object to templateType or templateType is null";
    }
    uint32_t templateType = 0;
    ConvertFromJsValue(env, templateTypeValue, templateType);
    option.SetTemplate(templateType);
    return "";
}

napi_value JsFloatViewManager::IsFloatViewEnabled(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "JsFloatViewManager check is enabled");
    JsFloatViewManager* me = CheckParamsAndGetThis<JsFloatViewManager>(env, info);
    return (me != nullptr) ? me->OnIsFloatViewEnabled(env, info) : nullptr;
}

napi_value JsFloatViewManager::OnIsFloatViewEnabled(napi_env env, napi_callback_info info)
{
    bool isSupportFloatView = FloatViewManager::isSupportFloatView_;
    if (!isSupportFloatView) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_ISFLOATINGVIEWENABLED,
                                         WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
    } else {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_ISFLOATINGVIEWENABLED, WmErrorCode::WM_OK);
    }
    return CreateJsValue(env, isSupportFloatView);
}

napi_value JsFloatViewManager::GetFloatViewLimits(napi_env env, napi_callback_info info)
{
    JsFloatViewManager* me = CheckParamsAndGetThis<JsFloatViewManager>(env, info);
    return (me != nullptr) ? me->OnGetFloatViewLimits(env, info) : nullptr;
}

napi_value JsFloatViewManager::OnGetFloatViewLimits(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnGetFloatViewLimits");
    if (!FloatViewManager::isSupportFloatView_) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Device do not support float view");
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_GETFLOATVIEWLIMITS,
                                         WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
            "Device do not support float view.");
    }
    size_t argc = ARG_COUNT_ONE;
    napi_value argv[ARG_COUNT_ONE] = { nullptr };
    // 解析应用传参FloatViewConfiguration对象
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_ONE) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "Missing args.");
    }
    uint32_t templateType = 0;
    if (!ConvertFromJsValue(env, argv[0], templateType)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to convert parameter to template type");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "Failed to convert parameter to template type");
    }
    if (templateType >= static_cast<uint32_t>(FloatViewTemplate::END)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "template type is invalid");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM, "template type is invalid");
    }

    FloatViewLimits limits;
    WMError errCode = SingletonContainer::Get<WindowManager>().GetFloatViewLimits(templateType, limits);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to get global float view limits, code: %{public}d", errCode);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
            "Failed to get global float view limits.");
    }
    auto jsObject = CreateJsFloatViewLimitsObject(env, limits);
    if (jsObject == nullptr) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_GETFLOATVIEWLIMITS,
                                         WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Failed to create js object.");
    }
    return jsObject;
}

napi_value JsFloatViewManager::Bind(napi_env env, napi_callback_info info)
{
    JsFloatViewManager* me = CheckParamsAndGetThis<JsFloatViewManager>(env, info);
    return (me != nullptr) ? me->OnBind(env, info) : nullptr;
}

napi_value JsFloatViewManager::OnBind(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnBind");
    size_t argc = ARG_COUNT_THREE;
    napi_value argv[ARG_COUNT_THREE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_THREE) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_BIND, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "Missing args when bind.");
    }

    auto controllerPair = GetBindControllers(env, argv);
    if (controllerPair.first == nullptr || controllerPair.second == nullptr) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_BIND, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "Failed to get controllers from js objects.");
    }

    auto jsFvController = static_cast<JsFloatViewController*>(controllerPair.first);
    if (jsFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get floatViewController from js object");
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_BIND, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "Failed to get floatViewController from js object.");
    }

    auto jsFbController = static_cast<JsFbController*>(controllerPair.second);
    if (jsFbController == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get floatingBallController from js object");
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_BIND, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "Failed to get floatingBallController from js object.");
    }

    napi_value jsFbOption = argv[2];
    if (jsFbOption == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get floating ball param");
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_BIND, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "Failed to get floating ball param.");
    }

    FbOption fbOption;
    if (jsFbController->GetFloatingBallOptionFromJs(env, jsFbOption, fbOption) == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert floating ball param");
        return NapiGetUndefined(env);
    }
    auto fvControllerSptr = jsFvController->GetController();
    auto fbControllerSptr = jsFbController->GetController();
    return BindTask(env, fvControllerSptr, fbControllerSptr, fbOption);
}

std::pair<void*, void*> JsFloatViewManager::GetBindControllers(napi_env env, napi_value argv[])
{
    napi_value fvController = argv[0];
    if (fvController == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get floatViewController");
        return std::make_pair(nullptr, nullptr);
    }
    void* fvControllerPtr = nullptr;
    napi_unwrap(env, fvController, &fvControllerPtr);
    napi_value fbController = argv[1];
    if (fbController == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get floatingBallController");
        return std::make_pair(nullptr, nullptr);
    }
    void* fbControllerPtr = nullptr;
    napi_unwrap(env, fbController, &fbControllerPtr);
    return std::make_pair(fvControllerPtr, fbControllerPtr);
}

napi_value JsFloatViewManager::BindTask(napi_env env, const sptr<FloatViewController> &fvController,
    const sptr<FloatingBallController> &fbController, const FbOption &option)
{
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [env, errCodePtr, fvController, fbController, option] {
        if (errCodePtr == nullptr) {
            return;
        }
        if (!FloatViewManager::isSupportFloatView_ || !FloatingBallManager::IsSupportFloatingBall()) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Device do not support float view");
            *errCodePtr = WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_BIND, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
            return;
        }
        if (!Permission::CheckCallingPermission(FLOATING_BALL_PERMISSION) ||
            !Permission::CheckCallingPermission(FLOAT_VIEW_PERMISSION)) {
            *errCodePtr = WmErrorCode::WM_ERROR_NO_PERMISSION;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_BIND, WmErrorCode::WM_ERROR_NO_PERMISSION);
            return;
        }
        if (fvController == nullptr || fbController == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "fvController or fbController is null");
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_BIND, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
            return;
        }
        std::string errMsg = "";
        if (!option.IsValid(errMsg)) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "check floating ball param failed, %{public}s", errMsg.c_str());
            *errCodePtr = WmErrorCode::WM_ERROR_FB_PARAM_INVALID;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_BIND, WmErrorCode::WM_ERROR_FB_PARAM_INVALID);
            return;
        }
        *errCodePtr = ConvertErrorToCode(FloatWindowManager::Bind(fvController, fbController, option));
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "JsFloatViewManager::BindTask failed."));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFloatViewManager::BindTask",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsFloatViewManager::UnBind(napi_env env, napi_callback_info info)
{
    JsFloatViewManager* me = CheckParamsAndGetThis<JsFloatViewManager>(env, info);
    return (me != nullptr) ? me->OnUnBind(env, info) : nullptr;
}

napi_value JsFloatViewManager::OnUnBind(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnUnBind");
    size_t argc = ARG_COUNT_TWO;
    napi_value argv[ARG_COUNT_TWO] = { nullptr };
    // 解析应用传参FloatViewConfiguration对象
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_TWO) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_UNBIND, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "Missing args when bind.");
    }
    auto controllerPair = GetBindControllers(env, argv);
    if (controllerPair.first == nullptr || controllerPair.second == nullptr) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_UNBIND, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "Failed to get controllers from js objects.");
    }

    auto jsFvController = static_cast<JsFloatViewController*>(controllerPair.first);
    if (jsFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get floatViewController from js object");
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_UNBIND, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "Failed to get floatViewController from js object.");
    }

    auto jsFbController = static_cast<JsFbController*>(controllerPair.second);
    if (jsFbController == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get floatingBallController from js object");
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_UNBIND, WmErrorCode::WM_ERROR_INVALID_PARAM);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "Failed to get floatingBallController from js object.");
    }
    auto fvControllerSptr = jsFvController->GetController();
    auto fbControllerSptr = jsFbController->GetController();
    return UnBindTask(env, fvControllerSptr, fbControllerSptr);
}

napi_value JsFloatViewManager::UnBindTask(napi_env env, const sptr<FloatViewController> &fvController,
    const sptr<FloatingBallController> &fbController)
{
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [env, errCodePtr, fvController, fbController] {
        if (errCodePtr == nullptr) {
            return;
        }
        if (!FloatViewManager::isSupportFloatView_ || !FloatingBallManager::IsSupportFloatingBall()) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Device do not support float view");
            *errCodePtr = WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_UNBIND, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
            return;
        }
        if (fvController == nullptr || fbController == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "fvController or fbController is null");
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_UNBIND, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
            return;
        }
        *errCodePtr = ConvertErrorToCode(FloatWindowManager::UnBind(fvController, fbController));
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "JsFloatViewManager::UnBindTask failed."));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFloatViewManager::UnBindTask",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}
} // namespace Rosen
} // namespace OHOS
