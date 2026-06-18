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
#ifndef OHOS_JS_FB_WINDOW_MANAGER_H
#define OHOS_JS_FB_WINDOW_MANAGER_H

#include "js_runtime_utils.h"
#include "window_histogram_management.h"


namespace OHOS {
namespace Rosen {
napi_value JsFbWindowManagerInit(napi_env env, napi_value exportObj);
class JsFbWindowManager {
public:
    JsFbWindowManager();
    virtual ~JsFbWindowManager();
    static napi_value CreateFbController(napi_env env, napi_callback_info info);
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value IsFbEnabled(napi_env env, napi_callback_info info);
private:
    napi_value OnCreateFbController(napi_env env, napi_callback_info info);
    napi_value NapiSendTask(napi_env env, void* contextPtr);
    napi_value OnIsFbEnabled(napi_env env, napi_callback_info info);
};
}
}
#endif // OHOS_JS_FB_WINDOW_MANAGER_H