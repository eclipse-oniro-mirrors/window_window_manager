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

#ifndef OHOS_JS_FB_UTILS_H
#define OHOS_JS_FB_UTILS_H

#include "js_runtime_utils.h"
#include "window_histogram_management.h"

namespace OHOS {
namespace Rosen {
napi_value NapiGetUndefined(napi_env env);
napi_value NapiThrowInvalidParam(napi_env env, const std::string& msg, const char* enumerationName = "",
    const char* boolName = "");
napi_valuetype GetType(napi_env env, napi_value value);
bool NapiIsCallable(napi_env env, napi_value value);
napi_status InitFbEnums(napi_env env, napi_value exports);
}
}
#endif //OHOS_JS_FB_UTILS_H
