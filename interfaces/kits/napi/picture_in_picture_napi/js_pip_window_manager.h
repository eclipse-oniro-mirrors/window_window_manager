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

#ifndef OHOS_JS_PIP_WINDOW_MANAGER_H
#define OHOS_JS_PIP_WINDOW_MANAGER_H

#include "js_runtime_utils.h"
#include "picture_in_picture_manager.h"

namespace OHOS {
namespace Rosen {
napi_value JsPipWindowManagerInit(napi_env env, napi_value exportObj);
class JsPipWindowManager {
public:
    JsPipWindowManager();
    ~JsPipWindowManager();
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value IsPipEnabled(napi_env env, napi_callback_info info);
    static napi_value CreatePipController(napi_env env, napi_callback_info info);
private:
    napi_value OnIsPipEnabled(napi_env env, napi_callback_info info);
    napi_value OnCreatePipController(napi_env env, napi_callback_info info);
    napi_value NapiSendTask(napi_env env, PipOption& pipOption);
    std::unique_ptr<AbilityRuntime::NapiAsyncTask> CreateEmptyAsyncTask(napi_env env, napi_value* result);
};

enum DefaultWindowSizeType : uint32_t {
    NOT_SET,
    SMALL,
    LARGE,
};
}
}
#endif //OHOS_JS_PIP_WINDOW_MANAGER_H
