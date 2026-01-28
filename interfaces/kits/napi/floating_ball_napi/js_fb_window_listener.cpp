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

#include "js_fb_window_listener.h"

#include "js_fb_window_controller.h"
#include "window_manager_hilog.h"
#include "floating_ball_manager.h"
#include "js_fb_utils.h"

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

JsFbWindowListener::~JsFbWindowListener()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "~JsWindowListener");
}

std::shared_ptr<NativeReference> JsFbWindowListener::GetCallbackRef() const
{
    return jsCallBack_;
}

void JsFbWindowListener::OnFloatingBallStart()
{
    OnFbListenerCallback(FloatingBallState::STARTED);
}

void JsFbWindowListener::OnFloatingBallStop()
{
    OnFbListenerCallback(FloatingBallState::STOPPED);
}

void JsFbWindowListener::OnFbListenerCallback(const FloatingBallState& state)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "state: %{public}d", static_cast<int32_t>(state));
    auto napiTask = [jsCallback = jsCallBack_, state, env = env_]() {
        if (jsCallback == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "js callback is null");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, static_cast<uint32_t>(state))};
        CallJsFunction(env, jsCallback->GetNapiValue(), argv, ArraySize(argv));
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnFbListenerCallback");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to SendEvent");
        }
    } else {
        TLOGE(WmsLogTag::WMS_SYSTEM, "env is nullptr");
    }
}

void JsFbWindowListener::OnClickEvent()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "called, clickEvent");
    auto napiTask = [jsCallback = jsCallBack_, env = env_]() {
        if (jsCallback == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "js callback is null");
            return;
        }
        CallJsFunction(env, jsCallback->GetNapiValue(), {}, 0);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnClickEvent");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to SendEvent");
        }
    } else {
        TLOGE(WmsLogTag::WMS_SYSTEM, "env is nullptr");
    }
}

} // namespace Rosen
} // namespace OHOS