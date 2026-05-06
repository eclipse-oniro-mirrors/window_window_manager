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

#ifndef OHOS_ANI_WINDOW_ANIMATION_UTILS_H
#define OHOS_ANI_WINDOW_ANIMATION_UTILS_H

#include "ani.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {

inline bool GreatNotEqual(double left, double right)
{
    static constexpr double eps = 0.001f;
    return (left - right) > eps;
}

ani_object ConvertTransitionAnimationToAniValue(ani_env* env,
    std::shared_ptr<TransitionAnimation> transitionAnimation);
ani_object ConvertWindowAnimationOptionToAniValue(ani_env* env,
    const WindowAnimationOption& animationConfig);
ani_object ConvertStartAnimationOptionsToAniValue(ani_env* env,
    std::shared_ptr<StartAnimationOptions> startAnimationOptions);
ani_object ConvertStartAnimationSystemOptionsToAniValue(ani_env* env,
    std::shared_ptr<StartAnimationSystemOptions> startAnimationSystemOptions);
bool ConvertTransitionAnimationFromAniValue(ani_env* env, ani_object aniObject,
    TransitionAnimation& transitionAnimation, WmErrorCode& result);
bool ConvertStartAnimationOptionsFromAniValue(ani_env* env, ani_object aniObject,
    StartAnimationOptions& startAnimationOptions);
bool ConvertStartAnimationSystemOptionsFromAniValue(ani_env* env, ani_object aniObject,
    StartAnimationSystemOptions& startAnimationSystemOptions);
bool ConvertWindowAnimationOptionFromAniValue(ani_env* env, ani_object config,
    WindowAnimationOption& animationConfig, WmErrorCode& result);
bool ConvertWindowCreateParamsFromAniValue(ani_env* env, ani_object aniObject, WindowCreateParams& windowCreateParams);
bool ConvertSplitRatioPreferenceFromAniValue(
    ani_env* env, ani_object aniObject, SplitRatioPreference& splitRatioPreference);
bool CheckWindowAnimationOption(WindowAnimationOption& animationConfig, WmErrorCode& result);
}
}
#endif
