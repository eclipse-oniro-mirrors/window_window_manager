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

#ifndef OHOS_ROSEN_DMS_SCREEN_FOLD_COMMON_H
#define OHOS_ROSEN_DMS_SCREEN_FOLD_COMMON_H

#include <vector>
#include "dm_common.h"

namespace OHOS {
namespace Rosen {
namespace DMS {

enum class DmsSensorType : int32_t {
    SENSOR_TYPE_POSTURE = 0,
    SENSOR_TYPE_HALL,
    SENSOR_TYPE_TENT,
    SENSOR_TYPE_END,
};

struct ScreenAxis {
    float angle_ = -1.0F;
    int32_t hall_ = -1;
};

struct TentSensorInfo {
    int32_t tentType_ = 0;
    int32_t hall_ = -1;
};

struct SensorStatus {
    std::vector<ScreenAxis> axis_;
    float reflexionAngle_ = -1.0F;
    bool registerPosture_ = false;
    bool registerHall_ = false;
    TentSensorInfo tentSensorInfo_;
    DmsSensorType updateSensorType_ = DmsSensorType::SENSOR_TYPE_END;
};

struct ExtHallData {
    float flag = 0.0F;
    float hall = 0.0F; // BC axis angle
    float hallNfc = 0.0F;
    float hallProtectiveCase = 0.0F;
    float hallAb = 0.0F;
};

struct PostureDataSecondary {
    float postureAccxc = 0.0F;
    float postureAccyc = 0.0F;
    float postureAcczc = 0.0F;
    float postureAccxb = 0.0F;
    float postureAccyb = 0.0F;
    float postureAcczb = 0.0F;
    float postureBc = 0.0F;
    float postureScreen = 0.0F;
    float postureAccxx = 0.0F;
    float postureAccyx = 0.0F;
    float postureAcczx = 0.0F;
    float postureAb = 0.0F;
    float postureAbAnti = 0.0F;
};

} // namespace DMS
} // namespace Rosen
} // namespace OHOS

#endif