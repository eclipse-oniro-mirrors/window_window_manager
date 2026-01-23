/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_WINDOW_SCENE_JS_SCREEN_UTILS_H
#define OHOS_WINDOW_SCENE_JS_SCREEN_UTILS_H

#include <native_engine/native_engine.h>
#include <native_engine/native_value.h>

#include "dm_common.h"
#include "session/screen/include/screen_property.h"

namespace OHOS::Rosen {
bool ConvertRRectFromJs(napi_env env, napi_value jsObject, RRect& bound);
bool ConvertDMRectFromJs(napi_env env, napi_value jsObject, DMRect& rect);
bool ConvertScreenDirectionInfoFromJs(napi_env env, napi_value jsObject, ScreenDirectionInfo& directionInfo);
napi_value NapiGetUndefined(napi_env env);
bool NapiIsCallable(napi_env env, napi_value value);
class JsScreenUtils {
public:
    static napi_value CreateJsScreenProperty(napi_env env, const ScreenProperty& screenProperty);
    static napi_value CreateJsRRect(napi_env env, const RRect& rrect);
    static napi_value CreateJsScreenConnectChangeType(napi_env env);
    static napi_value CreateJsScreenPropertyChangeReason(napi_env env);
    static napi_value CreateJsFoldStatus(napi_env env);
    static napi_value CreateJsScreenPropertyChangeType(napi_env env);
    static napi_value CreateJsScreenPowerState(napi_env env);
    static napi_value CreateJsSuperFoldStatus(napi_env env);
    static napi_value CreateJsExtendScreenConnectStatus(napi_env env);
    static napi_value CreateJsScreenModeChangeEvent(napi_env env);
    static napi_value CreateJsFoldDisplayMode(napi_env env);
};
} // namespace OHOS::Rosen

#endif // OHOS_WINDOW_SCENE_JS_SCREEN_UTILS_H
