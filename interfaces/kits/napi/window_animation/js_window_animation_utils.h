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

#ifndef OHOS_JS_WINDOW_ANIMATION_UTILS_H
#define OHOS_JS_WINDOW_ANIMATION_UTILS_H
#include "napi/native_api.h"
#include "native_engine/native_engine.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {

#define CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue) \
    do {                                                       \
        napi_create_object((env), &(objValue));                \
        if ((objValue) == nullptr) {                           \
            TLOGE(WmsLogTag::DEFAULT, "Failed to get object"); \
            return nullptr;                                    \
        }                                                      \
    } while (0)

#define NAPI_CHECK_RETURN_IF_NULL(func, msg)                                                            \
    do {                                                                                                \
        napi_status status = func;                                                                      \
        if (status != napi_ok) {                                                                        \
            TLOGE(WmsLogTag::DEFAULT, "Failed with reason %{public}s, code: %{public}d", msg, status);  \
            return nullptr;                                                                             \
        }                                                                                               \
    } while (0)

    
inline bool GreatNotEqual(double left, double right)
{
    static constexpr double eps = 0.001f;
    return (left - right) > eps;
}

napi_value ConvertTransitionAnimationToJsValue(napi_env env,
    std::shared_ptr<TransitionAnimation> transitionAnimation);
napi_value ConvertWindowAnimationOptionToJsValue(napi_env env,
    const WindowAnimationOption& animationConfig);
napi_value ConvertStartAnimationOptionsToJsValue(napi_env env,
    std::shared_ptr<StartAnimationOptions> startAnimationOptions);
napi_value ConvertStartAnimationSystemOptionsToJsValue(napi_env env,
    std::shared_ptr<StartAnimationSystemOptions> startAnimationSystemOptions);
bool ConvertTransitionAnimationFromJsValue(napi_env env, napi_value jsObject,
    TransitionAnimation& transitionAnimation, WmErrorCode& result);
bool ConvertStartAnimationOptionsFromJsValue(napi_env env, napi_value jsObject,
    StartAnimationOptions& startAnimationOptions);
bool ConvertStartAnimationSystemOptionsFromJsValue(napi_env env, napi_value jsObject,
    StartAnimationSystemOptions& startAnimationSystemOptions);
bool ConvertWindowAnimationOptionFromJsValue(napi_env env, napi_value config,
    WindowAnimationOption& animationConfig, WmErrorCode& result);
bool ConvertWindowCreateParamsFromJsValue(napi_env env, napi_value jsObject, WindowCreateParams& windowCreateParams);
bool ConvertSplitRatioPreferenceFromJsValue(
    napi_env env, napi_value jsObject, SplitRatioPreference& splitRatioPreference);
bool CheckWindowAnimationOption(napi_env env, WindowAnimationOption& animationConfig, WmErrorCode& result);
}
}
#endif
