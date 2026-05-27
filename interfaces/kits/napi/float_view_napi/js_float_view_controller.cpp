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
#include "js_float_view_controller.h"

#include "js_float_view_utils.h"

#include "../../../../utils/include/window_histogram_management.h"
#include "js_err_utils.h"
#include "napi_common_want.h"
#include "permission.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

namespace {
const std::string FLOAT_VIEW_PERMISSION = "ohos.permission.FLOAT_VIEW";
constexpr size_t ARG_COUNT_ZERO = 0;
constexpr size_t ARG_COUNT_ONE = 1;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t ONE_PARAMS_SIZE = 1;
const char* ARKUI_WINDOW_FV_START = "ArkUI.window.fv.start";
const char* ARKUI_WINDOW_FV_SETUICONTEXT = "ArkUI.window.fv.setUIContext";
const char* ARKUI_WINDOW_FV_SETUICONTEXTBYNAME = "ArkUI.window.fv.setUIContextByName";
const char* ARKUI_WINDOW_FV_SETFLOATVIEWVISIBILITYINAPP = "ArkUI.window.fv.setFloatViewVisibilityInApp";
const char* ARKUI_WINDOW_FV_SETWINDOWSIZE = "ArkUI.window.fv.setWindowSize";
const char* ARKUI_WINDOW_FV_SETTEMPLATETYPE = "ArkUI.window.fv.setTemplateType";
const char* ARKUI_WINDOW_FV_GETWINDOWPROPERTIES = "ArkUI.window.fv.getWindowProperties";
const char* ARKUI_WINDOW_FV_RESTOREMAINWINDOW = "ArkUI.window.fv.restoreMainWindow";
const char* ARKUI_WINDOW_FV_ONCHANGE = "ArkUI.window.fv.onChange";
const char* ARKUI_WINDOW_FV_OFFCHANGE = "ArkUI.window.fv.offChange";

const char* ARKUI_WINDOW_FV_START_BOOL = "ArkUI.window.fv.start.bool";
const char* ARKUI_WINDOW_FV_SETUICONTEXT_BOOL = "ArkUI.window.fv.setUIContext.bool";
const char* ARKUI_WINDOW_FV_SETUICONTEXTBYNAME_BOOL = "ArkUI.window.fv.setUIContextByName.bool";
const char* ARKUI_WINDOW_FV_SETFLOATVIEWVISIBILITYINAPP_BOOL = "ArkUI.window.fv.setFloatViewVisibilityInApp.bool";
const char* ARKUI_WINDOW_FV_SETWINDOWSIZE_BOOL = "ArkUI.window.fv.setWindowSize.bool";
const char* ARKUI_WINDOW_FV_SETTEMPLATETYPE_BOOL = "ArkUI.window.fv.setTemplateType.bool";
const char* ARKUI_WINDOW_FV_GETWINDOWPROPERTIES_BOOL = "ArkUI.window.fv.getWindowProperties.bool";
const char* ARKUI_WINDOW_FV_RESTOREMAINWINDOW_BOOL = "ArkUI.window.fv.restoreMainWindow.bool";
const char* ARKUI_WINDOW_FV_ONCHANGE_BOOL = "ArkUI.window.fv.onChange.bool";
const char* ARKUI_WINDOW_FV_OFFCHANGE_BOOL = "ArkUI.window.fv.offChange.bool";
}

void BindFunctions(napi_env env, napi_value object, const char* moduleName)
{
    BindNativeFunction(env, object, "start", moduleName, JsFloatViewController::Start);
    BindNativeFunction(env, object, "stop", moduleName, JsFloatViewController::Stop);
    BindNativeFunction(env, object, "setUIContext", moduleName, JsFloatViewController::SetUIContext);
    BindNativeFunction(env, object, "setUIContextByName", moduleName, JsFloatViewController::SetUIContextByName);
    BindNativeFunction(env, object, "setFloatViewVisibilityInApp", moduleName,
        JsFloatViewController::SetFloatViewVisibilityInApp);
    BindNativeFunction(env, object, "setWindowSize", moduleName, JsFloatViewController::SetWindowSize);
    BindNativeFunction(env, object, "switchTemplate", moduleName, JsFloatViewController::SwitchTemplate);
    BindNativeFunction(env, object, "getWindowProperties", moduleName, JsFloatViewController::GetWindowProperties);
    BindNativeFunction(env, object, "restoreMainWindow", moduleName, JsFloatViewController::RestoreMainWindow);

    BindNativeFunction(env, object, "onStateChange", moduleName, JsFloatViewController::OnStateChange);
    BindNativeFunction(env, object, "offStateChange", moduleName, JsFloatViewController::OffStateChange);
    BindNativeFunction(env, object, "onRectChange", moduleName, JsFloatViewController::OnRectChange);
    BindNativeFunction(env, object, "offRectChange", moduleName, JsFloatViewController::OffRectChange);
    BindNativeFunction(env, object, "onLimitsChange", moduleName, JsFloatViewController::OnLimitsChange);
    BindNativeFunction(env, object, "offLimitsChange", moduleName, JsFloatViewController::OffLimitsChange);
}

napi_value CreateJsFloatViewControllerObject(napi_env env, const sptr<FloatViewController>& floatViewController)
{
    napi_value objValue = nullptr;
    napi_status status = napi_create_object(env, &objValue);
    if (status != napi_ok || objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "failed to create js obj, error:%{public}d", status);
        return NapiGetUndefined(env);
    }

    TLOGI(WmsLogTag::WMS_SYSTEM, "CreateJsFloatViewControllerObject");
    std::unique_ptr<JsFloatViewController> jsFloatViewController =
        std::make_unique<JsFloatViewController>(floatViewController);
    napi_wrap(env, objValue, jsFloatViewController.release(), JsFloatViewController::Finalizer, nullptr, nullptr);

    BindFunctions(env, objValue, "JsFloatViewController");
    return objValue;
}

JsFloatViewController::JsFloatViewController(const sptr<FloatViewController>& floatViewController)
    : fvController_(floatViewController)
{
}

JsFloatViewController::~JsFloatViewController()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "JsFloatViewController release");
}

void JsFloatViewController::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "Finalizer is called");
    std::unique_ptr<JsFloatViewController>(static_cast<JsFloatViewController*>(data));
}

napi_value JsFloatViewController::Start(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->OnStartFloatView(env, info) : nullptr;
}

napi_value JsFloatViewController::OnStartFloatView(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnStartFloatView is called");
    wptr<FloatViewController> weakController(fvController_);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakController, env, task = napiAsyncTask] {
        if (!Permission::CheckCallingPermission(FLOAT_VIEW_PERMISSION)) {
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_START, WmErrorCode::WM_ERROR_NO_PERMISSION);
            HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_START_BOOL, 0);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_NO_PERMISSION,
                "no permission."));
            return;
        }
        auto fvController = weakController.promote();
        if (fvController == nullptr) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "Controller is nullptr");
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_START, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
            HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_START_BOOL, 0);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "The controller is nullptr."));
            return;
        }
        auto errCode = ConvertErrorToCode(fvController->StartFloatView());
        if (errCode != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "Failed to start float view."));
            HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_START_BOOL, 0);
            return;
        } else {
            HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_START_BOOL, 1);
        }
        task->Resolve(env, NapiGetUndefined(env));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "StartFloatViewTask") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsFloatViewController::Stop(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->OnStopFloatView(env, info) : nullptr;
}

napi_value JsFloatViewController::OnStopFloatView(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnStopFloatView");
    wptr<FloatViewController> weakController(fvController_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [weakController, errCodePtr] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto fvController = weakController.promote();
        if (fvController == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = ConvertErrorToCode(fvController->StopFloatViewFromClient());
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "Failed to stop float view."));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFloatViewController::OnStopFloatView",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsFloatViewController::SetUIContext(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->OnSetUIContext(env, info, false) : nullptr;
}

napi_value JsFloatViewController::SetUIContextByName(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->OnSetUIContext(env, info, true) : nullptr;
}

napi_value JsFloatViewController::OnSetUIContext(napi_env env, napi_callback_info info, bool isLoadByName)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnSetUIContext, %{public}d", isLoadByName);
    auto name = ARKUI_WINDOW_FV_SETUICONTEXT;
    auto boolName = ARKUI_WINDOW_FV_SETUICONTEXT_BOOL;
    if (isLoadByName) {
        name = ARKUI_WINDOW_FV_SETUICONTEXTBYNAME;
        boolName = ARKUI_WINDOW_FV_SETUICONTEXTBYNAME_BOOL;
    }
    size_t argc = 2;
    napi_value argv[2] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "param num verification failed.",
            name, boolName);
    }
    std::string contextUrl;
    if (!ConvertFromJsValue(env, argv[0], contextUrl)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to convert parameter to context url");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "Failed to convert parameter to context url", name, boolName);
    }
    napi_value storage = nullptr;
    if (argc == 2) { // 2: num of params
        storage = argv[1];
    }
    std::shared_ptr<NativeReference> contentStorage = nullptr;
    if (storage != nullptr) {
        napi_ref result = nullptr;
        napi_create_reference(env, storage, 1, &result);
        contentStorage = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(result));
    }
    return SetUIContextTask(env, contextUrl, contentStorage, isLoadByName);
}

napi_value JsFloatViewController::SetUIContextTask(napi_env env, const std::string contextUrl,
    const std::shared_ptr<NativeReference>& contentStorage, bool isLoadByName)
{
    wptr<FloatViewController> weakController(fvController_);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakController, contentStorage, contextUrl, isLoadByName, env, task = napiAsyncTask] {
        if (contextUrl.empty()) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "The ui path is empty");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM,
                "The ui path is empty."));
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_SETUICONTEXT, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
            HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_SETUICONTEXT_BOOL, 0);
            return;
        }
        auto fvController = weakController.promote();
        if (fvController == nullptr) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "Controller is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "The controller is nullptr."));
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_SETUICONTEXT, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
            HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_SETUICONTEXT_BOOL, 0);
            return;
        }
        auto errCode = ConvertErrorToCode(fvController->SetUIContext(contextUrl, contentStorage, isLoadByName));
        if (errCode != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "Failed to set UI content."));
            HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_SETUICONTEXT_BOOL, 0);
            return;
        } else {
            HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_SETUICONTEXT_BOOL, 1);
        }
        task->Resolve(env, NapiGetUndefined(env));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "SetUIContextTask") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsFloatViewController::SetFloatViewVisibilityInApp(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->OnSetFloatViewVisibilityInApp(env, info) : nullptr;
}

napi_value JsFloatViewController::OnSetFloatViewVisibilityInApp(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnSetFloatViewVisibilityInApp");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "param num verification failed.",
            ARKUI_WINDOW_FV_SETFLOATVIEWVISIBILITYINAPP, ARKUI_WINDOW_FV_SETFLOATVIEWVISIBILITYINAPP_BOOL);
    }
    bool visibleInApp = true;
    if (!ConvertFromJsValue(env, argv[0], visibleInApp)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to convert parameter to visibility");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "Failed to convert parameter to visibility", ARKUI_WINDOW_FV_SETFLOATVIEWVISIBILITYINAPP,
            ARKUI_WINDOW_FV_SETFLOATVIEWVISIBILITYINAPP_BOOL);
    }
    wptr<FloatViewController> weakController(fvController_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [weakController, errCodePtr, visibleInApp] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto fvController = weakController.promote();
        if (fvController == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = ConvertErrorToCode(fvController->SetVisibilityInApp(visibleInApp));
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
                HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_SETFLOATVIEWVISIBILITYINAPP_BOOL, 1);
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                    "Failed to set float view visibility in app."));
                HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_SETFLOATVIEWVISIBILITYINAPP_BOOL, 0);
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFloatViewController::OnSetFloatViewVisibilityInApp",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsFloatViewController::SetWindowSize(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->OnSetWindowSize(env, info) : nullptr;
}

napi_value JsFloatViewController::OnSetWindowSize(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnSetWindowSize");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "param num verification failed.", ARKUI_WINDOW_FV_SETWINDOWSIZE, ARKUI_WINDOW_FV_SETWINDOWSIZE_BOOL);
    }
    napi_value jsSize = argv[0];
    if (jsSize == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "jsSize is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "Failed to convert object to Size or size is null", ARKUI_WINDOW_FV_SETWINDOWSIZE,
            ARKUI_WINDOW_FV_SETWINDOWSIZE_BOOL);
    }
    napi_value jsWidth = nullptr;
    napi_value jsHeight = nullptr;

    int32_t width = 0;
    int32_t height = 0;
    bool hasProperty = false;
    napi_has_named_property(env, jsSize, "width", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, jsSize, "width", &jsWidth);
        ConvertFromJsValue(env, jsWidth, width);
    }
    napi_has_named_property(env, jsSize, "height", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, jsSize, "height", &jsHeight);
        ConvertFromJsValue(env, jsHeight, height);
    }
    return OnSetWindowSizeTask(env, width, height);
}

napi_value JsFloatViewController::OnSetWindowSizeTask(napi_env env, int32_t width, int32_t height)
{
    wptr<FloatViewController> weakController(fvController_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [weakController, errCodePtr, width, height] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto fvController = weakController.promote();
        if (fvController == nullptr) {
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_SETWINDOWSIZE, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        if (width <= 0 || height <= 0) {
            *errCodePtr = WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_SETWINDOWSIZE, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
            return;
        }
        Rect rect = {0, 0, static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        *errCodePtr = ConvertErrorToCode(fvController->SetWindowSize(rect));
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
                HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_SETWINDOWSIZE_BOOL, 1);
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "Failed to set window size."));
                HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_SETWINDOWSIZE_BOOL, 0);
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFloatViewController::OnSetWindowSizeTask",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsFloatViewController::SwitchTemplate(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->OnSwitchTemplate(env, info) : nullptr;
}

napi_value JsFloatViewController::OnSwitchTemplate(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnSwitchTemplate");
    size_t argc = ARG_COUNT_ONE;
    napi_value argv[ARG_COUNT_ONE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "Param num verification failed.", ARKUI_WINDOW_FV_SETTEMPLATETYPE, ARKUI_WINDOW_FV_SETTEMPLATETYPE_BOOL);
    }
    napi_value jsTemplateProperty = argv[INDEX_ZERO];
    if (jsTemplateProperty == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "jsTemplateProperty is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "Failed to convert object to TemplateProperty or input is null", ARKUI_WINDOW_FV_SETTEMPLATETYPE,
            ARKUI_WINDOW_FV_SETTEMPLATETYPE_BOOL);
    }
    std::string errorMsg = "";
    WmErrorCode errorCode = WmErrorCode::WM_OK;
    auto templateProperty = ConvertJsValueToTemplateProperty(env, jsTemplateProperty, errorMsg, errorCode);
    if (!templateProperty) {
        return NapiThrowError(env, errorCode, errorMsg, ARKUI_WINDOW_FV_SETTEMPLATETYPE,
            ARKUI_WINDOW_FV_SETTEMPLATETYPE_BOOL);
    }

    return OnSwitchTemplateTask(env, templateProperty);
}

napi_value JsFloatViewController::OnSwitchTemplateTask(napi_env env, std::shared_ptr<TemplateProperty> templateProperty)
{
    wptr<FloatViewController> weakController(fvController_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [weakController, errCodePtr, templateProperty] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto fvController = weakController.promote();
        if (fvController == nullptr) {
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_SETTEMPLATETYPE, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        if (!templateProperty->CheckLegal()) {
            *errCodePtr = WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_SETTEMPLATETYPE, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
            return;
        }
        *errCodePtr = ConvertErrorToCode(fvController->SetTemplateTypeAndSize(templateProperty));
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
                HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_SETTEMPLATETYPE_BOOL, 1);
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "Failed to set template type."));
                HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_SETTEMPLATETYPE_BOOL, 0);
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFloatViewController::OnSwitchTemplateTask",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsFloatViewController::GetWindowProperties(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->OnGetWindowProperties(env, info) : nullptr;
}

napi_value JsFloatViewController::OnGetWindowProperties(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnGetWindowProperties");
    wptr<FloatViewController> weakController(fvController_);
    auto fvController = weakController.promote();
    if (fvController == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "controller is null.",
            ARKUI_WINDOW_FV_GETWINDOWPROPERTIES, ARKUI_WINDOW_FV_GETWINDOWPROPERTIES_BOOL);
    }
    auto templateType = static_cast<uint32_t>(fvController->GetTemplateType());
    auto state = fvController->GetCurState();
    if (!fvController->IsStateWithWindow(state)) {
        return NapiThrowError(
            env, WmErrorCode::WM_ERROR_FV_INVALID_STATE, "current state do not support get window properties.",
            ARKUI_WINDOW_FV_GETWINDOWPROPERTIES, ARKUI_WINDOW_FV_GETWINDOWPROPERTIES_BOOL);
    }
    auto fvWindow = fvController->GetWindow();
    if (fvWindow == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "window is null.",
            ARKUI_WINDOW_FV_GETWINDOWPROPERTIES, ARKUI_WINDOW_FV_GETWINDOWPROPERTIES_BOOL);
    }
    auto windowInfo = fvController->GetWindowInfo();
    auto jsObject = CreateJsFloatViewPropertiesObject(env, templateType, fvWindow, windowInfo, state);
    if (jsObject == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Failed to create js object.",
            ARKUI_WINDOW_FV_GETWINDOWPROPERTIES, ARKUI_WINDOW_FV_GETWINDOWPROPERTIES_BOOL);
    }
    HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_GETWINDOWPROPERTIES_BOOL, 1);
    return jsObject;
}

napi_value JsFloatViewController::RestoreMainWindow(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->OnRestoreMainWindow(env, info) : nullptr;
}

napi_value JsFloatViewController::OnRestoreMainWindow(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_ONE;
    napi_value argv[ONE_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    AAFwk::WantParams wantParams;
    if (argc > ARG_COUNT_ZERO) {
        napi_value wantValue = argv[INDEX_ZERO];
        if (wantValue != nullptr && !AppExecFwk::UnwrapWantParams(env, wantValue, wantParams)) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to convert parameters to wantParameters");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                "Failed to convert parameters to wantParameters.", ARKUI_WINDOW_FV_RESTOREMAINWINDOW,
                ARKUI_WINDOW_FV_RESTOREMAINWINDOW_BOOL);
        }
    }
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    wptr<FloatViewController> weakController(fvController_);
    std::shared_ptr<AAFwk::WantParams> parameters = std::make_shared<AAFwk::WantParams>(wantParams);
    NapiAsyncTask::ExecuteCallback execute = [weakController, errCodePtr, parameters] {
        if (errCodePtr == nullptr || parameters == nullptr) {
            return;
        }
        auto controller = weakController.promote();
        if (controller == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = ConvertErrorToCode(controller->RestoreMainWindow(parameters));
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr, parameters](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr || parameters == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FV_RESTOREMAINWINDOW,
                    WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
                HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_RESTOREMAINWINDOW_BOOL, 1);
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "Failed to restore main window."));
                HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_RESTOREMAINWINDOW_BOOL, 0);
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFloatViewController::OnRestoreMainWindow",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

sptr<FloatViewController> JsFloatViewController::GetController() const
{
    return fvController_;
}

bool JsFloatViewController::IsCallbackRegistered(napi_env env, const CallbackType& type, napi_value jsCallback)
{
    if (jsCallbackMap_.empty() || jsCallbackMap_.find(type) == jsCallbackMap_.end()) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "callback type %{public}d not registered!", type);
        return false;
    }
    for (auto& listener : jsCallbackMap_[type]) {
        bool isEquals = false;
        napi_strict_equals(env, jsCallback, listener->GetCallbackRef()->GetNapiValue(), &isEquals);
        if (isEquals) {
            return true;
        }
    }
    return false;
}

napi_value JsFloatViewController::RegisterCallbackWithType(
    const CallbackType& callbackType, napi_env env, napi_callback_info info)
{
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    TLOGI(WmsLogTag::WMS_SYSTEM, "RegisterCallbackWithType is called, type: %{public}d", callbackType);
    size_t argc = ARG_COUNT_ONE;
    napi_value argv[ONE_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "RegisterCallbackWithType Params not match: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "No enough params.",
            ARKUI_WINDOW_FV_ONCHANGE, ARKUI_WINDOW_FV_ONCHANGE_BOOL);
    }
    napi_value value = argv[INDEX_ZERO];
    if (value == nullptr || !NapiIsCallable(env, value)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Callback is nullptr or not callable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "Callback is nullptr or not callable.",
            ARKUI_WINDOW_FV_ONCHANGE, ARKUI_WINDOW_FV_ONCHANGE_BOOL);
    }
    if (fvController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "controller is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Controller not exists.",
            ARKUI_WINDOW_FV_ONCHANGE, ARKUI_WINDOW_FV_ONCHANGE_BOOL);
    }
    if (IsCallbackRegistered(env, callbackType, value)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Callback already registered!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_FV_REPEAT_OPERATION, "Callback already registered.",
            ARKUI_WINDOW_FV_ONCHANGE, ARKUI_WINDOW_FV_ONCHANGE_BOOL);
    }

    std::shared_ptr<NativeReference> callbackRef;
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    auto fvWindowListener = sptr<JsFloatViewListener>::MakeSptr(env, callbackRef);
    if (fvWindowListener == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "New JsFloatViewListener failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "New JsFloatViewListener failed.",
            ARKUI_WINDOW_FV_ONCHANGE, ARKUI_WINDOW_FV_ONCHANGE_BOOL);
    }
    WMError errCode = DoRegisterCallbackWithType(callbackType, fvWindowListener);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Register callback failed, type: %{public}d", callbackType);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Failed to register callback.",
            ARKUI_WINDOW_FV_ONCHANGE, ARKUI_WINDOW_FV_ONCHANGE_BOOL);
    }
    jsCallbackMap_[callbackType].insert(fvWindowListener);
    TLOGI(WmsLogTag::WMS_SYSTEM, "Register type %{public}d success", callbackType);
    HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_ONCHANGE_BOOL, 1);
    return NapiGetUndefined(env);
}

napi_value JsFloatViewController::UnregisterCallbackWithType(
    const CallbackType& callbackType, napi_env env, napi_callback_info info)
{
    std::lock_guard<std::mutex> lock(jsCallbackMutex_);
    TLOGI(WmsLogTag::WMS_SYSTEM, "UnegisterCallbackWithType is called, type: %{public}d", callbackType);
    size_t argc = ARG_COUNT_ONE;
    napi_value argv[ONE_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_ONE) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "no callback specific, unregister all");
        for (auto& callback : jsCallbackMap_[callbackType]) {
            WMError ret = DoUnregisterCallbackWithType(callbackType, callback);
            if (ret != WMError::WM_OK) {
                TLOGE(WmsLogTag::WMS_SYSTEM, "Unregister type %{public}d failed, no value", callbackType);
                return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Failed to unregister callback.",
                    ARKUI_WINDOW_FV_OFFCHANGE, ARKUI_WINDOW_FV_OFFCHANGE_BOOL);
            }
        }
        jsCallbackMap_.erase(callbackType);
        TLOGI(WmsLogTag::WMS_SYSTEM, "Unregister type %{public}d success", callbackType);
        return NapiGetUndefined(env);
    }
    napi_value value = argv[INDEX_ZERO];
    for (auto& callback : jsCallbackMap_[callbackType]) {
        bool isEquals = false;
        napi_strict_equals(env, value, callback->GetCallbackRef()->GetNapiValue(), &isEquals);
        if (!isEquals) {
            continue;
        }
        WMError ret = DoUnregisterCallbackWithType(callbackType, callback);
        if (ret != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Unregister type %{public}d failed, no value", callbackType);
            return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Failed to unregister callback.",
                ARKUI_WINDOW_FV_OFFCHANGE, ARKUI_WINDOW_FV_OFFCHANGE_BOOL);
        }
        jsCallbackMap_[callbackType].erase(callback);
        break;
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "Unregister type %{public}d success", callbackType);
    HISTOGRAM_BOOLEAN(ARKUI_WINDOW_FV_OFFCHANGE_BOOL, 1);
    return NapiGetUndefined(env);
}

WMError JsFloatViewController::DoRegisterCallbackWithType(
    const CallbackType& callbackType, const sptr<JsFloatViewListener>& listener)
{
    WMError errCode = WMError::WM_OK;
    switch (callbackType) {
        case CallbackType::STATE_CHANGE:
            errCode = fvController_->RegisterStateChangeListener(listener);
            break;
        case CallbackType::RECT_CHANGE:
            errCode = fvController_->RegisterRectChangeListener(listener);
            break;
        case CallbackType::LIMITS_CHANGE:
            errCode = fvController_->RegisterLimitsChangeListener(listener);
            break;
        default:
            break;
    }
    return errCode;
}

WMError JsFloatViewController::DoUnregisterCallbackWithType(
    const CallbackType& callbackType, const sptr<JsFloatViewListener>& listener)
{
    WMError errCode = WMError::WM_OK;
    switch (callbackType) {
        case CallbackType::STATE_CHANGE:
            errCode = fvController_->UnregisterStateChangeListener(listener);
            break;
        case CallbackType::RECT_CHANGE:
            errCode = fvController_->UnregisterRectChangeListener(listener);
            break;
        case CallbackType::LIMITS_CHANGE:
            errCode = fvController_->UnregisterLimitsChangeListener(listener);
            break;
        default:
            break;
    }
    return errCode;
}

napi_value JsFloatViewController::OnStateChange(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->RegisterCallbackWithType(CallbackType::STATE_CHANGE, env, info) : nullptr;
}

napi_value JsFloatViewController::OffStateChange(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->UnregisterCallbackWithType(CallbackType::STATE_CHANGE, env, info) : nullptr;
}

napi_value JsFloatViewController::OnRectChange(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->RegisterCallbackWithType(CallbackType::RECT_CHANGE, env, info) : nullptr;
}

napi_value JsFloatViewController::OffRectChange(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->UnregisterCallbackWithType(CallbackType::RECT_CHANGE, env, info) : nullptr;
}

napi_value JsFloatViewController::OnLimitsChange(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->RegisterCallbackWithType(CallbackType::LIMITS_CHANGE, env, info) : nullptr;
}

napi_value JsFloatViewController::OffLimitsChange(napi_env env, napi_callback_info info)
{
    JsFloatViewController* me = CheckParamsAndGetThis<JsFloatViewController>(env, info);
    return (me != nullptr) ? me->UnregisterCallbackWithType(CallbackType::LIMITS_CHANGE, env, info) : nullptr;
}

} // namespace Rosen
} // namespace OHOS
