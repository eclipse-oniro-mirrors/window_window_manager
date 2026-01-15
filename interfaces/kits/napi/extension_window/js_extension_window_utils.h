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

#ifndef OHOS_JS_EXTENSION_WINDOW_UTILS_H
#define OHOS_JS_EXTENSION_WINDOW_UTILS_H

#include <map>
#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "window.h"
#include "window_option.h"
#include "wm_common.h"

#ifndef WINDOW_PREVIEW
#include "window_manager.h"
#else
#include "mock/window_manager.h"
#endif

namespace OHOS {
namespace Rosen {
    enum class ComponentRectChangeReason : uint32_t {
        HOST_WINDOW_RECT_CHANGE = 1,
    };
    napi_value GetRectAndConvertToJsValue(napi_env env, const Rect& rect);
    napi_value CreateJsExtensionWindowPropertiesObject(napi_env env, sptr<Window>& window);
    napi_value CreateJsExtensionWindowProperties(napi_env env, sptr<Window>& window);
    napi_value GetHostStatusBarPropertyObject(napi_env env, sptr<Window>& window);
    bool NapiIsCallable(napi_env env, napi_value value);
    napi_value NapiGetUndefined(napi_env env);
    napi_value NapiThrowError(napi_env env, WmErrorCode errCode);
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_JS_EXTENSION_WINDOW_UTILS_H
