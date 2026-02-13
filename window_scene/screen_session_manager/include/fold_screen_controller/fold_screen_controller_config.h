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

#ifndef OHOS_ROSEN_FOLD_SCREEN_CONTROLLER_CONFIG_H
#define OHOS_ROSEN_FOLD_SCREEN_CONTROLLER_CONFIG_H

#include "screen_scene_config.h"

namespace OHOS::Rosen {
namespace {
static inline const float ANGLE_MIN_VAL =
    static_cast<float>(system::GetIntParameter<int32_t>("const.dms.min_fold_angle", 0));

static inline const float HALF_FOLDED_MAX_THRESHOLD =
    static_cast<float>(ScreenSceneConfig::GetNumberConfigValue("halfFoldMaxThreshold", 140));
static inline const float CLOSE_HALF_FOLDED_MIN_THRESHOLD =
    static_cast<float>(ScreenSceneConfig::GetNumberConfigValue("closeHalfFoldedMinThreshold", 70));
static inline const float OPEN_HALF_FOLDED_MIN_THRESHOLD =
    static_cast<float>(ScreenSceneConfig::GetNumberConfigValue("openHalfFoldedMinThreshold", 25));
static inline const float HALF_FOLDED_BUFFER =
    static_cast<float>(ScreenSceneConfig::GetNumberConfigValue("halfFoldedBuffer", 10));
static inline const float LARGER_BOUNDARY_FOR_THRESHOLD =
    static_cast<float>(ScreenSceneConfig::GetNumberConfigValue("largerBoundaryForThreshold", 90));

static inline const size_t POSTURE_SIZE = ScreenSceneConfig::GetNumberConfigValue("postureSize", 3);
static inline const size_t HALL_SIZE = ScreenSceneConfig::GetNumberConfigValue("hallSize", 2);
}
}
#endif // OHOS_ROSEN_FOLD_SCREEN_CONTROLLER_CONFIG_H