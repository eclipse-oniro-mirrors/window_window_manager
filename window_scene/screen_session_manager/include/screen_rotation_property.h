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

#ifndef OHOS_ROSEN_SCREEN_ROTATION_PROPERTY_H
#define OHOS_ROSEN_SCREEN_ROTATION_PROPERTY_H

#include <map>
#include <refbase.h>

#ifdef SENSOR_ENABLE
#include "sensor_agent.h"
#endif

#include "dm_common.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
enum class DeviceRotation: int32_t {
    INVALID = -1,
    ROTATION_PORTRAIT = 0,
    ROTATION_LANDSCAPE,
    ROTATION_PORTRAIT_INVERTED,
    ROTATION_LANDSCAPE_INVERTED,
};

enum class DeviceHoverStatus: int32_t {
    INVALID = -1,
    TENT_STATUS = 0,
    TENT_STATUS_CANCEL,
    CAMERA_STATUS,
    CAMERA_STATUS_CANCEL,
    TENT_STATUS_HOVER,
};

class ScreenRotationProperty : public RefBase {
public:
    ScreenRotationProperty() = delete;
    ~ScreenRotationProperty() = default;
    static void HandleSensorEventInput(DeviceRotation deviceRotation);
    static void HandleHoverStatusEventInput(DeviceHoverStatus hoverStatus, bool needRotate = true);

    static bool isDeviceHorizontal();

    // Get the live device orientation via the screen session's GetDeviceRotation
    static bool IsDeviceHorizontalByScreenSession();
private:
    static float ConvertDeviceToFloat(DeviceRotation deviceRotation);
    
    static bool isDeviceHorizontal_;
};
} // Rosen
} // OHOS
#endif // OHOS_ROSEN_SCREEN_ROTATION_CONTROLLER_H