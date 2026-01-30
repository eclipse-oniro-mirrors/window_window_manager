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
#include "js_ui_effect_controller.h"

#include "js_runtime_utils.h"
#include "js_window_animation_utils.h"
#include "js_window.h"
#include "js_err_utils.h"
#include "ui_effect_controller_common.h"
#include "window_adapter.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t FOUR_PARAMS_SIZE = 4;
using namespace AbilityRuntime;

napi_status JsUIEffectController::CreateJsObject(napi_env env, JsUIEffectController* filter, napi_value& obj)
{
    NAPI_CHECK(napi_create_object(env, &obj), "ui effect obj");
    napi_status status = napi_wrap(env, obj, filter, JsUIEffectController::Finalizer, nullptr, nullptr);
    if (status != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "napi func failed, with reason %{public}d", status);
        delete filter;
        return status;
    }
    BindNativeFunctions(env, obj, "JsUIEffectController");
    return napi_status::napi_ok;
}

void JsUIEffectController::Finalizer(napi_env env, void* data, void* hint)
{
    JsUIEffectController* controller = static_cast<JsUIEffectController*>(data);
    if (controller != nullptr) {
        delete controller;
    }
}

JsUIEffectController::JsUIEffectController(): client_(sptr<UIEffectControllerClient>::MakeSptr())
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "Constructor");
};

JsUIEffectController::~JsUIEffectController()
{
    SingletonContainer::Get<WindowAdapter>().UnregisterUIEffectRecoverCallbackFunc(client_->GetId());
    TLOGI(WmsLogTag::WMS_ANIMATION, "Destrutor, id: %{public}d", client_->GetId());
}

WMError JsUIEffectController::Init()
{
    WMError err = InitClientAndServer();
    if (err != WMError::WM_OK) {
        TLOGI(WmsLogTag::WMS_ANIMATION, "failed to init");
        return err;
    }
    SingletonContainer::Get<WindowAdapter>().RegisterUIEffectRecoverCallbackFunc(client_->GetId(), [this]()-> WMError {
        return InitClientAndServer();
    });
    return WMError::WM_OK;
}

WMError JsUIEffectController::InitClientAndServer()
{
    int32_t controllerId = UIEFFECT_INVALID_ID;
    WMError err = SingletonContainer::Get<WindowAdapter>().CreateUIEffectController(client_, server_, controllerId);
    if (err != WMError::WM_OK) {
        return err;
    }
    client_->SetId(controllerId);
    return WMError::WM_OK;
}

void JsUIEffectController::BindNativeFunctions(napi_env env, napi_value object, const char* moduleName)
{
    BindNativeFunction(env, object, "setParams", moduleName, JsUIEffectController::SetParams);
    BindNativeFunction(env, object, "animateToUIEffect", moduleName, JsUIEffectController::AnimateTo);
}

napi_value JsUIEffectController::SetParams(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "NapiFunc");
    JsUIEffectController* me = CheckParamsAndGetThis<JsUIEffectController>(env, info);
    return (me != nullptr) ? me->OnSetParams(env, info) : nullptr;
}

napi_value JsUIEffectController::AnimateTo(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "NapiFunc");
    JsUIEffectController* me = CheckParamsAndGetThis<JsUIEffectController>(env, info);
    return (me != nullptr) ? me->OnAnimateTo(env, info) : nullptr;
}

napi_value JsUIEffectController::OnSetParams(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    sptr<UIEffectParams> params = sptr<UIEffectParams>::MakeSptr();
    if (params->ConvertFromJsValue(env, argv[INDEX_ZERO]) != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "parse ui effect params failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (params->IsEmpty()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "do not have any parameters");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
    }
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    NapiAsyncTask::ExecuteCallback execute = [params, client = client_, server = server_, errCodePtr] {
        if (errCodePtr == nullptr || client == nullptr || server == nullptr) {
            return;
        }
        WMError ret = server->SetParams(params);
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(ret);
        TLOGI(WmsLogTag::WMS_ANIMATION, "ui effect %{public}d set filter params exec end with err code %{public}d",
            client->GetId(), ret);
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "System abnormal."));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsUIEffect::SetParams",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsUIEffectController::OnAnimateTo(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGC_THREE || argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    struct AnimationInfoList {
        sptr<UIEffectParams> params = sptr<UIEffectParams>::MakeSptr();
        sptr<WindowAnimationOption> option = sptr<WindowAnimationOption>::MakeSptr();
        sptr<WindowAnimationOption> interruptOption = nullptr;
        WmErrorCode errCode = WmErrorCode::WM_OK;
    };
    std::shared_ptr<AnimationInfoList> lists = std::make_shared<AnimationInfoList>();
    WmErrorCode err = WmErrorCode::WM_OK;
    if (!ConvertWindowAnimationOptionFromJsValue(env, argv[INDEX_ZERO], *lists->option, err)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "parse window animation config failed");
        return NapiThrowError(env, err);
    }
    if (!CheckWindowAnimationOption(env, *lists->option, err)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "check window animation config failed");
        return NapiThrowError(env, err);
    }
    if (lists->params->ConvertFromJsValue(env, argv[INDEX_ONE]) != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "parse ui effect params failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (lists->params->IsEmpty()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "do not have any parameters");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
    }
    if (argc == ARGC_THREE) {
        lists->interruptOption = sptr<WindowAnimationOption>::MakeSptr();
        if (!ConvertWindowAnimationOptionFromJsValue(env, argv[INDEX_TWO], *lists->interruptOption, err)) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "parse window animation config failed");
            return NapiThrowError(env, err);
        }
        if (!CheckWindowAnimationOption(env, *lists->interruptOption, err)) {
            TLOGE(WmsLogTag::WMS_ANIMATION, "check window animation config failed");
            return NapiThrowError(env, err);
        }
    }
    NapiAsyncTask::ExecuteCallback execute = [lists, client = client_, server = server_] {
        WMError ret = server->AnimateTo(lists->params, lists->option, lists->interruptOption);
        lists->errCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
        TLOGI(WmsLogTag::WMS_ANIMATION, "ui effect %{public}d animateTo, err code %{public}d",
            client->GetId(), ret);
    };
    NapiAsyncTask::CompleteCallback complete =
        [lists](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (lists == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "System abnormal."));
                return;
            }
            if (lists->errCode == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, lists->errCode));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsUIEffect::AnimateTo",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}
}