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

    void RegisterApplicationStateObserver();

    void UnRegisterPostureCallback();

    void UnRegisterHallCallback();

    void RegisterHallCallback();

    void SetFoldScreenPolicy(sptr<FoldScreenPolicy> foldScreenPolicy);

    void SetSensorFoldStateManager(sptr<SensorFoldStateManager> sensorFoldStateManager);

    void HandlePostureData(const SensorEvent* const event);

    void HandleHallData(const SensorEvent* const event);

    void TriggerDisplaySwitch();

    bool GetSensorRegisterStatus(void);

    float GetGlobalAngle() const;
    void SetGlobalAngle(float angle);
    uint16_t GetGlobalHall() const;
    void SetGlobalHall(uint16_t hall);

protected:
    FoldStatus GetCurrentState();

private:
    sptr<FoldScreenPolicy> foldScreenPolicy_;

    sptr<SensorFoldStateManager> sensorFoldStateManager_;

    void NotifyFoldAngleChanged(float foldAngle);
    bool HandleAbnormalAngle();

    FoldScreenSensorManager();

    ~FoldScreenSensorManager() = default;

    float globalAngle = -1.0F;

    uint16_t globalHall = USHRT_MAX;

    bool registerPosture_ = false;

    bool registerHall_ = false;
};
} // namespace Rosen
} // namespace OHOS
#endif
#endif