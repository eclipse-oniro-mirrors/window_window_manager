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

#ifndef OHOS_JS_UI_EFFECT_CONTROLLER_H
#define OHOS_JS_UI_EFFECT_CONTROLLER_H
#include <memory>
#include <mutex>

#include "js_runtime_utils.h"
#include "ui_effect_controller_client.h"
#include "ui_effect_controller_proxy.h"

namespace OHOS::Rosen {
class JsUIEffectController final {
public:
    static napi_status CreateJsObject(napi_env env, JsUIEffectController* filter, napi_value& obj);
    static void Finalizer(napi_env env, void* data, void* hint);
    static void BindNativeFunctions(napi_env env, napi_value object, const char* moduleName);
    static napi_value SetParams(napi_env env, napi_callback_info info);
    static napi_value AnimateTo(napi_env env, napi_callback_info info);
    JsUIEffectController();
    ~JsUIEffectController();
    WMError Init();

private:
    napi_value OnSetParams(napi_env env, napi_callback_info info);
    WMError InitClientAndServer();
    napi_value OnAnimateTo(napi_env env, napi_callback_info info);
    struct RecoverGuard {
        std::mutex mutex;
        JsUIEffectController* controller = nullptr;
    };
    std::shared_ptr<RecoverGuard> recoverGuard_ = std::make_shared<RecoverGuard>();
    sptr<UIEffectControllerClient> client_;
    sptr<IUIEffectController> server_;
};
}
#endif