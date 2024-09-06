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

#ifndef OHOS_ROSEN_FOLD_SCREEN_SENSOR_MANAGER_H
#define OHOS_ROSEN_FOLD_SCREEN_SENSOR_MANAGER_H

#ifdef SENSOR_ENABLE
#include <functional>
#include <mutex>
#include <climits>

#include "fold_screen_controller.h"
#include "fold_screen_controller/sensor_fold_state_manager/sensor_fold_state_manager.h"
#include "refbase.h"
#include "wm_single_instance.h"
#include "sensor_agent.h"
#include "sensor_agent_type.h"

namespace OHOS {
namespace Rosen {

class FoldScreenSensorManager : public RefBase {
    WM_DECLARE_SINGLE_INSTANCE_BASE(FoldScreenSensorManager);

public:
    void RegisterPostureCallback();

    void RegisterHallCallback();

    void RegisterApplicationStateObserver();

    void UnRegisterPostureCallback();

    void UnRegisterHallCallback();

    void SetFoldScreenPolicy(sptr<FoldScreenPolicy> foldScreenPolicy);

    void SetSensorFoldStateManager(sptr<SensorFoldStateManager> sensorFoldStateManager);

    void HandlePostureData(const SensorEvent * const event);

    void HandleHallData(const SensorEvent * const event);

    void TriggerDisplaySwitch();

protected:
    FoldStatus GetCurrentState();

private:
    sptr<FoldScreenPolicy> foldScreenPolicy_;

    sptr<SensorFoldStateManager> sensorFoldStateManager_;

    std::recursive_mutex mutex_;

    FoldStatus mState_ = FoldStatus::UNKNOWN;

    SensorUser postureUser {};

    SensorUser hallUser {};

    void notifyFoldAngleChanged(float foldAngle);

    FoldScreenSensorManager();

    ~FoldScreenSensorManager() = default;

    float globalAngle = -1.0F;

    uint16_t globalHall = USHRT_MAX;

    typedef struct EXTHALLData {
        float flag = 0.0;
        float hall = 0.0;
    } ExtHallData;
};
} // namespace Rosen
} // namespace OHOS
#endif
#endif