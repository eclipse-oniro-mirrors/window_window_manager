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
#include "js_float_view_listener.h"

#include "js_float_view_utils.h"
#include "window_manager_hilog.h"

#include "js_runtime_utils.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

static napi_value CallJsFunction(napi_env env, napi_value method, napi_value const* argv, size_t argc)
{
    if (env == nullptr || method == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "env nullptr or method is nullptr");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value callResult = nullptr;
    napi_get_undefined(env, &result);
    napi_get_undefined(env, &callResult);
    napi_call_function(env, result, method, argc, argv, &callResult);
    TLOGD(WmsLogTag::WMS_SYSTEM, "called.");
    return callResult;
}

JsFloatViewListener::~JsFloatViewListener()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "~JsFloatViewListener");
}

std::shared_ptr<NativeReference> JsFloatViewListener::GetCallbackRef() const
{
    return jsCallBack_;
}

void JsFloatViewListener::OnStateChange(const FloatViewState& state, const std::string& stopReason)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "state: %{public}d", static_cast<int32_t>(state));
    auto napiTask = [jsCallback = jsCallBack_, state, stopReason, env = env_]() {
        if (jsCallback == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "js callback is null");
            return;
        }
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to get object");
            return;
        }
        napi_set_named_property(env, objValue, "state", CreateJsValue(env, static_cast<uint32_t>(state)));
        napi_set_named_property(env, objValue, "stopReason", CreateJsValue(env, stopReason));

        napi_value argv[] = { objValue };
        CallJsFunction(env, jsCallback->GetNapiValue(), argv, ArraySize(argv));
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnStateChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to SendEvent");
        }
    } else {
        TLOGE(WmsLogTag::WMS_SYSTEM, "env is nullptr");
    }
}

void JsFloatViewListener::OnRectangleChange(const Rect& window, double scale, const std::string& reason)
{
    auto napiTask = [jsCallback = jsCallBack_, window, scale, reason, env = env_]() {
        if (jsCallback == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "js callback is null");
            return;
        }
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to get object");
            return;
        }
        napi_set_named_property(env, objValue, "windowRect", GetRectAndConvertToJsValue(env, window));
        napi_set_named_property(env, objValue, "windowScale", CreateJsValue(env, scale));
        napi_set_named_property(env, objValue, "reason", CreateJsValue(env, reason));

        napi_value argv[] = { objValue };
        CallJsFunction(env, jsCallback->GetNapiValue(), argv, ArraySize(argv));
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnRectangleChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to SendEvent");
        }
    } else {
        TLOGE(WmsLogTag::WMS_SYSTEM, "env is nullptr");
    }
}

void JsFloatViewListener::OnLimitsChange(const FloatViewLimits& limits)
{
    auto napiTask = [jsCallback = jsCallBack_, limits, env = env_]() {
        if (jsCallback == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "js callback is null");
            return;
        }
        auto jsLimitsInfo = CreateJsFloatViewLimitsObject(env, limits);
        if (jsLimitsInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to get jsLimitsInfo");
            return;
        }
        napi_value argv[] = { jsLimitsInfo };
        CallJsFunction(env, jsCallback->GetNapiValue(), argv, ArraySize(argv));
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnLimitsChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to SendEvent");
        }
    } else {
        TLOGE(WmsLogTag::WMS_SYSTEM, "env is nullptr");
    }
}
} // Rosen
} // OHOS