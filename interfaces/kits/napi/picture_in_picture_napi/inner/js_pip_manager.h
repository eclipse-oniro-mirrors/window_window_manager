/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_PIP_MANAGER_H
#define OHOS_JS_PIP_MANAGER_H

#include "js_runtime_utils.h"

namespace OHOS {
namespace Rosen {
napi_value JsPipManagerInit(napi_env env, napi_value exportObj);
class JsPipManager {
public:
    JsPipManager();
    ~JsPipManager();
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value InitXComponentController(napi_env env, napi_callback_info info);
    static napi_value GetCustomUIController(napi_env env, napi_callback_info info);
    static napi_value GetTypeNode(napi_env env, napi_callback_info info);
    static napi_value RegisterCallback(napi_env env, napi_callback_info info);
    static napi_value UnregisterCallback(napi_env env, napi_callback_info info);
    static napi_value SetTypeNodeEnabled(napi_env env, napi_callback_info info);
private:
    napi_value OnInitXComponentController(napi_env env, napi_callback_info info);
    napi_value OnGetCustomUIController(napi_env env, napi_callback_info info);
    napi_value OnGetTypeNode(napi_env env, napi_callback_info info);
    napi_value OnRegisterCallback(napi_env env, napi_callback_info info);
    napi_value OnUnregisterCallback(napi_env env, napi_callback_info info);
    napi_value OnSetTypeNodeEnabled(napi_env env, napi_callback_info info);
};
} // namespace Rosen
} // namespace OHOS
#endif //OHOS_JS_PIP_MANAGER_H
