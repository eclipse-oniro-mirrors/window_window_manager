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

#include "js_window_extension_context.h"
#include "js_window_extension.h"

#include <js_runtime_utils.h>
#include <js_extension_context.h>

#include "napi_common_start_options.h"
#include "napi_common_want.h"
#include "permission.h"
#include "service_extension_context.h"
#include "start_options.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JSWindowExtensionContext"};
std::shared_ptr<AppExecFwk::EventHandler> handler_;
constexpr int32_t MIN_ARG_COUNT = 2;
}

napi_value NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

class JsWindowExtensionContext final {
public:
    explicit JsWindowExtensionContext(
        const std::shared_ptr<WindowExtensionContext>& context) : context_(context) {}
    ~JsWindowExtensionContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        WLOGI("JsWindowExtensionContext::Finalizer is called");
        std::unique_ptr<JsWindowExtensionContext>(static_cast<JsWindowExtensionContext*>(data));
    }

    static napi_value StartAbility(napi_env env, napi_callback_info info)
    {
        JsWindowExtensionContext* me = CheckParamsAndGetThis<JsWindowExtensionContext>(env, info);
        return (me != nullptr) ? me->OnStartAbility(env, info) : nullptr;
    }
private:
    std::weak_ptr<WindowExtensionContext> context_;

    bool CheckStartAbilityInputParam(
        napi_env env, napi_callback_info info,
        AAFwk::Want& want, AAFwk::StartOptions& startOptions) const
    {
        size_t argc = 2; // 2 : max param number
        napi_value argv[2] = {nullptr}; // 2 : max param number
        napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
        if (argc < 1) {
            return false;
        }
        // Check input want
        if (!CheckWantParam(env, argv[0], want)) {
            return false;
        }
        if (argc > 1 && GetType(env, argv[1]) == napi_object) {
            WLOGI("OnStartAbility start options is used.");
            AppExecFwk::UnwrapStartOptions(env, argv[1], startOptions);
        }
        return true;
    }

    bool CheckWantParam(napi_env env, napi_value value, AAFwk::Want& want) const
    {
        if (!OHOS::AppExecFwk::UnwrapWant(env, value, want)) {
            WLOGFE("The input want is invalid.");
            return false;
        }
        WLOGI("UnwrapWant, BundleName: %{public}s, AbilityName: %{public}s.",
            want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        return true;
    }

    napi_value OnStartAbility(napi_env env, napi_callback_info info)
    {
        if (!Permission::IsSystemCalling()) {
            TLOGE(WmsLogTag::DEFAULT, "Permission denied!");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP)));
            return NapiGetUndefined(env);
        }
        WLOGI("OnStartAbility is called");
        size_t argc = 4;
        napi_value argv[4] = {nullptr};
        napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
        if (argc < MIN_ARG_COUNT) { // at least two argc
            WLOGFE("Start ability failed, not enough params.");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
            return NapiGetUndefined(env);
        }

        AAFwk::Want want;
        AAFwk::StartOptions startOptions;
        if (!CheckStartAbilityInputParam(env, info, want, startOptions)) {
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
            return NapiGetUndefined(env);
        }

        napi_value lastParam = (argc <= 2) ? nullptr : argv[2]; // at least two argc
        napi_value result = nullptr;
        std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [weak = context_, want, startOptions, env, task = napiAsyncTask] {
            WLOGI("startAbility begin");
            auto context = weak.lock();
            if (!context) {
                WLOGW("context is released");
                task->Reject(env, CreateJsError(env,
                    static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
                return;
            }
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(context->StartAbility(want, startOptions));
            if (ret == WmErrorCode::WM_OK) {
                task->Resolve(env, NapiGetUndefined(env));
            } else {
                task->Reject(env, CreateJsError(env, static_cast<int32_t>(ret)));
            }
        };
        if (napi_send_event(env, asyncTask, napi_eprio_high, "OnStartAbility") != napi_status::napi_ok) {
            napiAsyncTask->Reject(env, CreateJsError(env,
                static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
        }
        return result;
    }
};

void JsWindowConnectCallback::OnAbilityConnectDone(const AppExecFwk::ElementName& element,
                                                   const sptr<IRemoteObject>& remoteObject, int resultCode)
{
    WLOGI("called");
}

void JsWindowConnectCallback::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode)
{
    WLOGI("called");
}

napi_value CreateJsWindowExtensionContext(napi_env env, const std::shared_ptr<WindowExtensionContext>& context)
{
    WLOGI("CreateJsWindowExtensionContext begin");
    napi_value objValue = CreateJsExtensionContext(env, context);

    std::unique_ptr<JsWindowExtensionContext> jsContext
        = std::make_unique<JsWindowExtensionContext>(context);
    napi_wrap(env, objValue, jsContext.release(), JsWindowExtensionContext::Finalizer, nullptr, nullptr);

    // make handler
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());

    const char *moduleName = "JsWindowExtensionContext";
    BindNativeFunction(env, objValue, "startAbility", moduleName, JsWindowExtensionContext::StartAbility);

    return objValue;
}
} // namespace Rosen
} // namespace OHOS