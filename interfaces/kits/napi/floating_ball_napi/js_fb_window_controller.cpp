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
#include "js_fb_window_controller.h"

#include <memory>

#include "color_parser.h"
#include "floating_ball_manager.h"
#include "js_err_utils.h"
#include "js_fb_utils.h"
#include "napi_common_want.h"
#include "pixel_map_napi.h"
#include "permission.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
const std::string STATE_CHANGE_CB = "stateChange";
const std::string CLICK_EVENT = "click";
const std::string FLOATING_BALL_PERMISSION = "ohos.permission.USE_FLOAT_BALL";
constexpr int32_t NUMBER_TWO = 2;
const char* ARKUI_WINDOW_FB_STARTFLOATINGBALL = "ArkUI.window.fb.startFloatingBall";
const char* ARKUI_WINDOW_FB_UPDATEFLOATINGBALL = "ArkUI.window.fb.updateFloatingBall";
const char* ARKUI_WINDOW_FB_STOPFLOATINGBALL = "ArkUI.window.fb.stopFloatingBall";
const char* ARKUI_WINDOW_FB_ONRESTOREMAINWINDOW = "ArkUI.window.fb.onRestoreMainWindow";
const char* ARKUI_WINDOW_FB_ONSTATECHANGE = "ArkUI.window.fb.onStateChange";
const char* ARKUI_WINDOW_FB_OFFSTATECHANGE = "ArkUI.window.fb.offStateChange";
const char* ARKUI_WINDOW_FB_ONCLICK = "ArkUI.window.fb.onClick";
const char* ARKUI_WINDOW_FB_OFFCLICK = "ArkUI.window.fb.offClick";
const char* ARKUI_WINDOW_FB_GETFLOATINGBALLWINDOWINFO = "ArkUI.window.fb.getFloatingBallWindowInfo";
}

void BindFunctions(napi_env env, napi_value object, const char* moduleName)
{
    BindNativeFunction(env, object, "startFloatingBall", moduleName, JsFbController::StartFloatingBall);
    BindNativeFunction(env, object, "updateFloatingBall", moduleName, JsFbController::UpdateFloatingBall);
    BindNativeFunction(env, object, "stopFloatingBall", moduleName, JsFbController::StopFloatingBall);
    BindNativeFunction(env, object, "restoreMainWindow", moduleName, JsFbController::RestoreMainWindow);
    BindNativeFunction(env, object, "on", moduleName, JsFbController::RegisterCallback);
    BindNativeFunction(env, object, "off", moduleName, JsFbController::UnregisterCallback);
    BindNativeFunction(env, object, "getFloatingBallWindowInfo", moduleName, JsFbController::GetFloatingBallWindowInfo);
}

napi_value CreateJsFbControllerObject(napi_env env, const sptr<FloatingBallController>& fbController)
{
    napi_value objValue = nullptr;
    napi_status status = napi_create_object(env, &objValue);
    if (status != napi_ok || objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "failed to create js obj, error:%{public}d", status);
        return NapiGetUndefined(env);
    }

    TLOGI(WmsLogTag::WMS_SYSTEM, "CreateJsFbController");
    std::unique_ptr<JsFbController> jsFbController = std::make_unique<JsFbController>(fbController);
    napi_wrap(env, objValue, jsFbController.release(), JsFbController::Finalizer, nullptr, nullptr);

    BindFunctions(env, objValue, "JsFbController");
    return objValue;
}

JsFbController::JsFbController(const sptr<FloatingBallController>& fbController)
    : fbController_(fbController)
{
}

JsFbController::~JsFbController()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "JsFbController release");
}

void JsFbController::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "Finalizer is called");
    std::unique_ptr<JsFbController>(static_cast<JsFbController*>(data));
}

napi_value JsFbController::StartFloatingBall(napi_env env, napi_callback_info info)
{
    JsFbController* me = CheckParamsAndGetThis<JsFbController>(env, info);
    return (me != nullptr) ? me->OnStartFloatingBall(env, info) : nullptr;
}

napi_value JsFbController::OnStartFloatingBall(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnStartFloatingBall is called");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_STARTFLOATINGBALL, WmErrorCode::WM_ERROR_FB_PARAM_INVALID);
        return NapiThrowInvalidParam(env, "Missing args when start floating ball");
    }

    napi_value config = argv[0];
    if (config == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "config is null");
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_STARTFLOATINGBALL, WmErrorCode::WM_ERROR_FB_PARAM_INVALID);
        return NapiThrowInvalidParam(env,
            "Failed to convert object to FloatingBallOption or FloatingBallOption is null");
    }

    FbOption option;
    if (GetFloatingBallOptionFromJs(env, config, option) == nullptr) {
        return NapiGetUndefined(env);
    }

    std::string errMsg = "";
    if (!option.IsValid(errMsg)) {
        napi_throw(env, AbilityRuntime::CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_FB_PARAM_INVALID), errMsg));
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_STARTFLOATINGBALL, WmErrorCode::WM_ERROR_FB_PARAM_INVALID);
        return NapiGetUndefined(env);
    }

    option.SetTextUpdateAnimationType(static_cast<uint32_t>(FloatingBallTextUpdateAnimationType::ANIMATION_NONE));
    return StartFloatingBallTask(env, option);
}

napi_value JsFbController::StartFloatingBallTask(napi_env env, const FbOption& option)
{
    wptr<FloatingBallController> weakController(fbController_);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakController, env, option, task = napiAsyncTask] {
        if (!Permission::CheckCallingPermission(FLOATING_BALL_PERMISSION)) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_NO_PERMISSION,
                "no permission."));
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_STARTFLOATINGBALL,
                                             WmErrorCode::WM_ERROR_NO_PERMISSION);
            return;
        }
        auto fbController = weakController.promote();
        if (fbController == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY,
                "controller is nullptr."));
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_STARTFLOATINGBALL,
                                             WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY);
            return;
        }
        sptr<FbOption> optionPtr = sptr<FbOption>::MakeSptr(option);
        auto errCode = ConvertErrorToCode(fbController->StartFloatingBall(optionPtr));
        if (errCode != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode,
                "JsFbController::OnStartFloatingBall failed."));
            return;
        }
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_STARTFLOATINGBALL, WmErrorCode::WM_OK);

        task->Resolve(env, NapiGetUndefined(env));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnStartFloatingBall") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsFbController::UpdateFloatingBall(napi_env env, napi_callback_info info)
{
    JsFbController* me = CheckParamsAndGetThis<JsFbController>(env, info);
    return (me != nullptr) ? me->OnUpdateFloatingBall(env, info) : nullptr;
}

napi_value JsFbController::OnUpdateFloatingBall(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnUpdateFloatingBall is called");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_UPDATEFLOATINGBALL,
                                         WmErrorCode::WM_ERROR_FB_PARAM_INVALID);
        return NapiThrowInvalidParam(env, "Missing args when update floating ball");
    }

    napi_value config = argv[0];
    if (config == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "config is null");
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_UPDATEFLOATINGBALL,
                                         WmErrorCode::WM_ERROR_FB_PARAM_INVALID);
        return NapiThrowInvalidParam(env,
            "Failed to convert object to FloatingBallOption or FloatingBallOption is null");
    }

    FbOption option;
    if (GetFloatingBallOptionFromJs(env, config, option) == nullptr) {
        return NapiGetUndefined(env);
    }

    std::string errMsg = "";
    if (!option.IsValid(errMsg)) {
        napi_throw(env, AbilityRuntime::CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_FB_PARAM_INVALID), errMsg));
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_UPDATEFLOATINGBALL,
                                         WmErrorCode::WM_ERROR_FB_PARAM_INVALID);
        return NapiGetUndefined(env);
    }
    return UpdateFloatingBallTask(env, option);
}

napi_value JsFbController::UpdateFloatingBallTask(napi_env env, const FbOption& option)
{
    wptr<FloatingBallController> weakController(fbController_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [weakController, option, errCodePtr] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto fbController = weakController.promote();
        if (fbController == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_UPDATEFLOATINGBALL,
                                             WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY);
            return;
        }
        sptr<FbOption> optionPtr = sptr<FbOption>::MakeSptr(option);
        *errCodePtr = ConvertErrorToCode(fbController->UpdateFloatingBall(optionPtr));
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                    "JsFbController::OnUpdateFloatingBall failed."));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFbController::OnUpdateFloatingBall",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsFbController::StopFloatingBall(napi_env env, napi_callback_info info)
{
    JsFbController* me = CheckParamsAndGetThis<JsFbController>(env, info);
    return (me != nullptr) ? me->OnStopFloatingBall(env, info) : nullptr;
}

napi_value JsFbController::OnStopFloatingBall(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnStopFloatingBall");
    wptr<FloatingBallController> weakController(fbController_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [weakController, errCodePtr] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto fbController = weakController.promote();
        if (fbController == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_STOPFLOATINGBALL,
                                             WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY);
            return;
        }
        *errCodePtr = ConvertErrorToCode(fbController->StopFloatingBallFromClient());
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_STOPFLOATINGBALL, WmErrorCode::WM_OK);
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                    "JsFbController::OnStopFloatingBall failed."));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFbController::OnStopFloatingBall",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsFbController::RestoreMainWindow(napi_env env, napi_callback_info info)
{
    JsFbController* me = CheckParamsAndGetThis<JsFbController>(env, info);
    return (me != nullptr) ? me->OnRestoreMainWindow(env, info) : nullptr;
}

napi_value JsFbController::OnRestoreMainWindow(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnRestoreMainWindow");
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_ONRESTOREMAINWINDOW,
                                         WmErrorCode::WM_ERROR_FB_PARAM_INVALID);
        return NapiThrowInvalidParam(env, "Missing args when restore main window");
    }

    napi_value wantValue = argv[0];
    if (wantValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "want is null");
        HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_ONRESTOREMAINWINDOW,
                                         WmErrorCode::WM_ERROR_FB_PARAM_INVALID);
        return NapiThrowInvalidParam(env, "want is null");
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantValue, want)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "unWrap want failed.");
        return NapiThrowInvalidParam(env, "Incorrect parameter, parameter must be want.");
    }

    std::shared_ptr<AAFwk::Want> abilityWant = std::make_shared<AAFwk::Want>(want);
    wptr<FloatingBallController> weakController(fbController_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [weakController, abilityWant, errCodePtr] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto fbController = weakController.promote();
        if (fbController == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_ONRESTOREMAINWINDOW,
                                             WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY);
            return;
        }
        *errCodePtr = ConvertErrorToCode(fbController->RestoreMainWindow(abilityWant));
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_ONRESTOREMAINWINDOW, WmErrorCode::WM_OK);
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                    "JsFbController::OnRestoreMainWindow failed."));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFbController::OnRestoreMainWindow",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsFbController::GetFloatingBallOptionFromJs(napi_env env, napi_value optionObject, FbOption& option)
{
    napi_value templateValue = nullptr;
    napi_value titleValue = nullptr;
    napi_value contentValue = nullptr;
    napi_value colorValue = nullptr;
    napi_value textUpdateAnimationTypeValue = nullptr;

    uint32_t templateType = 0;
    std::string title = "";
    std::string content = "";
    std::string color = "";
    uint32_t textUpdateAnimationType = 0;
    bool hasProperty = false;
    napi_has_named_property(env, optionObject, "template", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, optionObject, "template", &templateValue);
        ConvertFromJsValue(env, templateValue, templateType);
        option.SetTemplate(templateType);
    }
    napi_has_named_property(env, optionObject, "title", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, optionObject, "title", &titleValue);
        ConvertFromJsValue(env, titleValue, title);
        option.SetTitle(title);
    }
    napi_has_named_property(env, optionObject, "content", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, optionObject, "content", &contentValue);
        ConvertFromJsValue(env, contentValue, content);
        option.SetContent(content);
    }
    napi_has_named_property(env, optionObject, "backgroundColor", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, optionObject, "backgroundColor", &colorValue);
        ConvertFromJsValue(env, colorValue, color);
        option.SetBackgroundColor(color);
    }
    napi_has_named_property(env, optionObject, "textUpdateAnimationType", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, optionObject, "textUpdateAnimationType", &textUpdateAnimationTypeValue);
        ConvertFromJsValue(env, textUpdateAnimationTypeValue, textUpdateAnimationType);
        option.SetTextUpdateAnimationType(textUpdateAnimationType);
    }
    if (GetIcon(env, optionObject, option) == nullptr) {
        napi_throw(env, AbilityRuntime::CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_FB_PARAM_INVALID), "Invalid icon object"));
        return nullptr;
    }
    return NapiGetUndefined(env);
}

napi_value JsFbController::GetIcon(napi_env env, const napi_value value, FbOption& option)
{
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    napi_has_named_property(env, value, "icon", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, value, "icon", &result);
        napi_typeof(env, result, &valuetype);
        if (valuetype != napi_object) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Wrong argument type. Object expected.");
            return nullptr;
        }
        std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
        pixelMap = Media::PixelMapNapi::GetPixelMap(env, result);
        if (pixelMap == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Invalid object pixelMap");
            return nullptr;
        }
        option.SetIcon(pixelMap);
    }
    return NapiGetUndefined(env);
}

napi_value JsFbController::RegisterCallback(napi_env env, napi_callback_info info)
{
    JsFbController* me = CheckParamsAndGetThis<JsFbController>(env, info);
    return (me != nullptr) ? me->OnRegisterCallback(env, info) : nullptr;
}

napi_value JsFbController::OnRegisterCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "OnRegisterCallback is called");
    size_t argc = NUMBER_TWO;
    napi_value argv[NUMBER_TWO] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < NUMBER_TWO) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "OnRegisterCallback Params not match: %{public}zu", argc);
        return NapiThrowInvalidParam(env, "OnRegisterCallback Params not match");
    }
    std::string cbType = "";
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to convert parameter to callbackType");
        return NapiThrowInvalidParam(env, "Failed to convert parameter to callbackType");
    }
    napi_value value = argv[1];
    if (value == nullptr || !NapiIsCallable(env, value)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Callback is nullptr or not callable");
        return NapiThrowInvalidParam(env, "Callback is nullptr or not callable");
    }
    return RegisterListenerWithType(env, cbType, value);
}

napi_value JsFbController::RegisterListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    std::lock_guard<std::mutex> lock(callbBackMutex_);
    if (IsCallbackRegistered(env, type, value)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Callback already registered");
        napi_throw(env, AbilityRuntime::CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_FB_REPEAT_OPERATION), "Callback already registered"));
        return NapiGetUndefined(env);
    }
    std::shared_ptr<NativeReference> callbackRef;
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    auto fbWindowListener = sptr<JsFbWindowListener>::MakeSptr(env, callbackRef);
    if (fbWindowListener == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "New JsFbWindowListener failed");
        napi_throw(env, AbilityRuntime::CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR), "New JsFbWindowListener failed"));
        return NapiGetUndefined(env);
    }
    WMError ret = WMError::WM_OK;
    if (type == STATE_CHANGE_CB) {
        ret = ProcessStateChangeRegister(fbWindowListener);
        auto it = WM_JS_TO_ERROR_CODE_MAP.find(ret);
        if (it != WM_JS_TO_ERROR_CODE_MAP.end()) {
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_ONSTATECHANGE, it->second);
        }
    }
    if (type == CLICK_EVENT) {
        ret = ProcessClickEventRegister(fbWindowListener);
        auto it = WM_JS_TO_ERROR_CODE_MAP.find(ret);
        if (it != WM_JS_TO_ERROR_CODE_MAP.end()) {
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_ONCLICK, it->second);
        }
    }
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "register failed");
        napi_throw(env, AbilityRuntime::CreateJsError(env,
            static_cast<int32_t>(ConvertErrorToCode(ret)), "register failed"));
        return NapiGetUndefined(env);
    }
    jsCbMap_[type].insert(fbWindowListener);
    TLOGI(WmsLogTag::WMS_SYSTEM, "Register type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    return NapiGetUndefined(env);
}

bool JsFbController::IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    if (jsCbMap_.empty()) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "callback empty, methodName %{public}s not registered", type.c_str());
        return false;
    }
    auto callback = jsCbMap_.find(type);
    if (callback == jsCbMap_.end()) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "methodName %{public}s not registered", type.c_str());
        return false;
    }
    for (auto& listener : callback->second) {
        if (listener == nullptr || listener->GetCallbackRef() == nullptr) {
            TLOGI(WmsLogTag::WMS_SYSTEM, "listener or listener callback is null");
            continue;
        }
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, listener->GetCallbackRef()->GetNapiValue(), &isEquals);
        if (isEquals) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Callback already registered");
            return true;
        }
    }
    return false;
}

WMError JsFbController::ProcessStateChangeRegister(const sptr<JsFbWindowListener>& listener)
{
    if (fbController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "controller is nullptr");
        return WMError::WM_ERROR_FB_STATE_ABNORMALLY;
    }
    return fbController_->RegisterFbLifecycle(listener);
}

WMError JsFbController::ProcessClickEventRegister(const sptr<JsFbWindowListener>& listener)
{
    if (fbController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "controller is nullptr");
        return WMError::WM_ERROR_FB_STATE_ABNORMALLY;
    }
    return fbController_->RegisterFbClickObserver(listener);
}

napi_value JsFbController::UnregisterCallback(napi_env env, napi_callback_info info)
{
    JsFbController* me = CheckParamsAndGetThis<JsFbController>(env, info);
    return (me != nullptr) ? me->OnUnregisterCallback(env, info) : nullptr;
}

napi_value JsFbController::OnUnregisterCallback(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_TWO;
    napi_value argv[NUMBER_TWO] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == 0) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "JsFbController Params not match: %{public}zu", argc);
        return NapiThrowInvalidParam(env, "Params num not match");
    }
    std::string cbType = "";
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to convert parameter to string");
        return NapiThrowInvalidParam(env, "Failed to convert parameter to string");
    }
    if (argc == 1) {
        return UnRegisterListenerWithType(env, cbType, nullptr);
    }
    napi_value value = argv[1];
    if (value == nullptr || !NapiIsCallable(env, value)) {
        return NapiThrowInvalidParam(env, "callBack is invalid");
    }
    return UnRegisterListenerWithType(env, cbType, value);
}

napi_value JsFbController::UnRegisterListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    std::lock_guard<std::mutex> lock(callbBackMutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "methodName %{public}s not registered", type.c_str());
        return NapiGetUndefined(env);
    }

    if (value == nullptr) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "methodName %{public}s start unregister all callback", type.c_str());
        for (auto& listener : jsCbMap_[type]) {
            WmErrorCode ret = UnRegisterListener(type, listener);
            if (ret != WmErrorCode::WM_OK) {
                TLOGE(WmsLogTag::WMS_SYSTEM, "Unregister type %{public}s failed, no value", type.c_str());
                napi_throw(env, AbilityRuntime::CreateJsError(env,
                    static_cast<int32_t>(ret), "unRegister failed"));
                return NapiGetUndefined(env);
            }
        }
        jsCbMap_.erase(type);
    } else {
        TLOGI(WmsLogTag::WMS_SYSTEM, "methodName %{public}s start unregister one callback", type.c_str());
        bool foundCallbackValue = false;
        for (auto& listener : jsCbMap_[type]) {
            bool isEquals = false;
            napi_strict_equals(env, value, listener->GetCallbackRef()->GetNapiValue(), &isEquals);
            if (!isEquals) {
                continue;
            }
            foundCallbackValue = true;
            WmErrorCode ret = UnRegisterListener(type, listener);
            if (ret != WmErrorCode::WM_OK) {
                TLOGE(WmsLogTag::WMS_SYSTEM, "Unregister type %{public}s failed", type.c_str());
                napi_throw(env, AbilityRuntime::CreateJsError(env, static_cast<int32_t>(ret), "unRegister failed"));
                return NapiGetUndefined(env);
            }
            jsCbMap_[type].erase(listener);
            break;
        }
        if (!foundCallbackValue) {
            TLOGW(WmsLogTag::WMS_SYSTEM, "Unregister type %{public}s failed because not found callback", type.c_str());
            return NapiGetUndefined(env);
        }
        if (jsCbMap_[type].empty()) {
            jsCbMap_.erase(type);
        }
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "Unregister type %{public}s success", type.c_str());
    return NapiGetUndefined(env);
}

WmErrorCode JsFbController::UnRegisterListener(const std::string& type,
    const sptr<JsFbWindowListener>& fbWindowListener)
{
    WMError ret = WMError::WM_OK;
    if (type == STATE_CHANGE_CB) {
        ret = ProcessStateChangeUnRegister(fbWindowListener);
        auto it = WM_JS_TO_ERROR_CODE_MAP.find(ret);
        if (it != WM_JS_TO_ERROR_CODE_MAP.end()) {
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_OFFSTATECHANGE, it->second);
        }
    }
    if (type == CLICK_EVENT) {
        ret = ProcessClickEventUnRegister(fbWindowListener);
        auto it = WM_JS_TO_ERROR_CODE_MAP.find(ret);
        if (it != WM_JS_TO_ERROR_CODE_MAP.end()) {
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_OFFCLICK, it->second);
        }
    }
    return ConvertErrorToCode(ret);
}

WMError JsFbController::ProcessStateChangeUnRegister(const sptr<JsFbWindowListener>& listener)
{
    if (fbController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "controller is nullptr");
        return WMError::WM_ERROR_FB_STATE_ABNORMALLY;
    }
    return fbController_->UnRegisterFbLifecycle(listener);
}

WMError JsFbController::ProcessClickEventUnRegister(const sptr<JsFbWindowListener>& listener)
{
    if (fbController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "controller is nullptr");
        return WMError::WM_ERROR_FB_STATE_ABNORMALLY;
    }
    return fbController_->UnRegisterFbClickObserver(listener);
}

napi_value JsFbController::GetFloatingBallWindowInfo(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "GetFloatingBallWindowInfo start");
    JsFbController* me = CheckParamsAndGetThis<JsFbController>(env, info);
    return (me != nullptr) ? me->OnGetFloatingBallWindowInfo(env, info) : nullptr;
}

napi_value CreateJsFbWindowInfoObject(napi_env env, const std::shared_ptr<uint32_t>& windowIdPtr)
{
    napi_value objValue = nullptr;
    napi_status status = napi_create_object(env, &objValue);
    if (status != napi_ok || objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "failed to create js obj, error:%{public}d", status);
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "CreateJsFbWindowInfo");
    napi_set_named_property(env, objValue, "windowId", CreateJsValue(env, *windowIdPtr));
    return objValue;
}

napi_value JsFbController::OnGetFloatingBallWindowInfo(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "NAPI OnGetFloatingBallWindowInfo");
    wptr<FloatingBallController> weakController(fbController_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    std::shared_ptr<uint32_t> windowIdPtr = std::make_shared<uint32_t>(0);
    NapiAsyncTask::ExecuteCallback execute = [weakController, windowIdPtr, errCodePtr] {
        if (errCodePtr == nullptr || windowIdPtr == nullptr) {
            return;
        }
        auto fbController = weakController.promote();
        if (fbController == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY;
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_GETFLOATINGBALLWINDOWINFO,
                                             WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY);
            return;
        }
        uint32_t windowId = 0;
        *errCodePtr = ConvertErrorToCode(fbController->GetFloatingBallWindowInfo(windowId));
        *windowIdPtr = windowId;
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr, windowIdPtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr || windowIdPtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, CreateJsFbWindowInfoObject(env, windowIdPtr));
            HISTOGRAM_ENUMERATION_ERROR_CODE(ARKUI_WINDOW_FB_GETFLOATINGBALLWINDOWINFO, WmErrorCode::WM_OK);
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                    "JsFbController::OnGetFloatingBallWindowInfo failed."));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsFbController::OnGetFloatingBallWindowInfo",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

sptr<FloatingBallController> JsFbController::GetController() const
{
    return fbController_;
}
} // namespace Rosen
} // namespace OHOS
