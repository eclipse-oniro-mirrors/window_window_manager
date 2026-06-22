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

#ifndef OHOS_WINDOW_SCENE_CONFIG_H
#define OHOS_WINDOW_SCENE_CONFIG_H

#include <js_runtime_utils.h>
#include <native_engine/native_engine.h>
#include <native_engine/native_value.h>

#include "interfaces/include/ws_common.h"
#include "window_session_property.h"
namespace OHOS::Rosen {

class JsWindowSceneConfig {
public:
    JsWindowSceneConfig();
    ~JsWindowSceneConfig();
    static napi_value CreateWindowSceneConfig(napi_env env, const AppWindowSceneConfig& config);
    static napi_value CreateFreeMultiWindowConfig(napi_env env, const SystemSessionConfig& config);
    static napi_value CreateSystemConfig(napi_env env, const SystemSessionConfig& config);
    static napi_value CreateWindowLimits(napi_env env, const WindowLimits& windowLimits);
    static napi_value CreateSingleHandCompatibleConfig(napi_env env, const SingleHandCompatibleModeConfig& config);
    static napi_value CreateSingleHandBackgroundLayoutConfig(napi_env env,
        const SingleHandBackgroundLayoutConfig& config);
    static napi_value CreateJsRect(napi_env env, const WSRect& rect);
    static napi_value CreateSingleHandBackgroundTextConfig(napi_env env, const SingleHandBackgroundTextConfig& config);
private:
    static napi_value CreateShadowValue(napi_env env, const AppWindowSceneConfig& config, bool focused);
    static napi_value CreateShadowDarkValue(napi_env env, const AppWindowSceneConfig& config, bool focused);
    static napi_value CreateKeyboardAnimationValue(napi_env env, const KeyboardSceneAnimationConfig& config);
    static napi_value CreateWindowAnimationValue(napi_env env, const AppWindowSceneConfig& config);
    static napi_value CreateSystemUIStatusBarValue(napi_env env, const SystemUIStatusBarConfig& config);
    static napi_value CreateWindowStatusBar(napi_env env, const StatusBarConfig& config);
};
} // namespace OHOS::Rosen

#endif // OHOS_WINDOW_SCENE_CONFIG_H
