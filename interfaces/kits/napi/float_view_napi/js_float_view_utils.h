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
#ifndef OHOS_JS_FLOAT_VIEW_UTILS_H
#define OHOS_JS_FLOAT_VIEW_UTILS_H

#include "js_runtime_utils.h"

#include "wm_common.h"
#include "window.h"

namespace OHOS {
namespace Rosen {
napi_value NapiGetUndefined(napi_env env);
napi_value NapiThrowError(napi_env env, WmErrorCode errCode, const std::string& msg);
bool NapiIsCallable(napi_env env, napi_value value);
napi_status InitFvEnums(napi_env env, napi_value exports);
napi_value GetRectAndConvertToJsValue(napi_env env, const Rect& rect);
napi_value ConvertAvoidAreaToJsValue(napi_env env, const AvoidArea& avoidArea);
napi_value CreateJsFloatViewPropertiesObject(napi_env env, uint32_t templateType,
    const sptr<Window>& window, const FloatViewWindowInfo &windowInfo, const FvWindowState &state);
napi_value CreateJsFloatViewLimitsObject(napi_env env, const FloatViewLimits& limits);
} // namespace Rosen
} // namespace OHOS
#endif