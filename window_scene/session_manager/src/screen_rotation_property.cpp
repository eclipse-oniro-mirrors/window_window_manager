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

#include "screen_rotation_property.h"
#include "screen_session_manager.h"
#include "fold_screen_state_internel.h"

namespace OHOS {
namespace Rosen {
void ScreenRotationProperty::HandleSensorEventInput(DeviceRotation deviceRotation)
{
    static DeviceRotation lastSensorRotationConverted_ = DeviceRotation::INVALID;
    if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice() &&
        ScreenSessionManager::GetInstance().GetFoldStatus() == FoldStatus::FOLDED) {
        deviceRotation = ConvertSinglePocketOuterRotation(deviceRotation);
    }
    TLOGI(WmsLogTag::DMS, "DeviceRotation: %{public}d, "
        "lastSensorRotationConverted: %{public}d", deviceRotation, lastSensorRotationConverted_);

    if (deviceRotation != DeviceRotation::INVALID && lastSensorRotationConverted_ != deviceRotation) {
        lastSensorRotationConverted_ = deviceRotation;
    }
    auto screenSession = ScreenSessionManager::GetInstance().GetDefaultScreenSession();
    if (!screenSession) {
        TLOGW(WmsLogTag::DMS, "screenSession is null, sensor rotation status handle failed");
        return;
    }
    screenSession->HandleSensorRotation(ConvertDeviceToFloat(deviceRotation));
}

DeviceRotation ScreenRotationProperty::ConvertSinglePocketOuterRotation(DeviceRotation deviceRotation)
{
    DeviceRotation sensorRotation = deviceRotation;
    switch (deviceRotation) {
        case DeviceRotation::ROTATION_LANDSCAPE:
            sensorRotation = DeviceRotation::ROTATION_LANDSCAPE_INVERTED;
            break;
        case DeviceRotation::ROTATION_LANDSCAPE_INVERTED:
            sensorRotation = DeviceRotation::ROTATION_LANDSCAPE;
            break;
        default:
            TLOGW(WmsLogTag::DMS, "no need to covert, rotation: %{public}d", deviceRotation);
    }
    return sensorRotation;
}

float ScreenRotationProperty::ConvertDeviceToFloat(DeviceRotation deviceRotation)
{
    float sensorRotation = -1.0f; // -1 mean keep before degree
    switch (deviceRotation) {
        case DeviceRotation::ROTATION_PORTRAIT:
            sensorRotation = 0.0f; // degree 0
            break;
        case DeviceRotation::ROTATION_LANDSCAPE:
            sensorRotation = 90.0f; // degree 90
            break;
        case DeviceRotation::ROTATION_PORTRAIT_INVERTED:
            sensorRotation = 180.0f; // degree 180
            break;
        case DeviceRotation::ROTATION_LANDSCAPE_INVERTED:
            sensorRotation = 270.0f; // degree 270
            break;
        case DeviceRotation::INVALID:
            sensorRotation = -1.0f; // keep before degree
            break;
        default:
            TLOGW(WmsLogTag::DMS, "invalid device rotation: %{public}d", deviceRotation);
    }
    return sensorRotation;
}
} // Rosen
} // OHOS