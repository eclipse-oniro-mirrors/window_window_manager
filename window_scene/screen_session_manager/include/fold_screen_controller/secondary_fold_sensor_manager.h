/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_SECONDARY_FOLD_SENSOR_MANAGER_H
#define OHOS_ROSEN_SECONDARY_FOLD_SENSOR_MANAGER_H

#ifdef SENSOR_ENABLE
#include <functional>
#include <mutex>
#include <climits>

#include "fold_screen_controller.h"
#include "fold_screen_controller/sensor_fold_state_manager/sensor_fold_state_manager.h"
#include "fold_screen_controller/fold_screen_sensor_manager.h"
#include "refbase.h"
#include "wm_single_instance.h"
#include "sensor_agent.h"
#include "sensor_agent_type.h"

namespace OHOS {
namespace Rosen {

class SecondaryFoldSensorManager : public RefBase {
    WM_DECLARE_SINGLE_INSTANCE_BASE(SecondaryFoldSensorManager);
public:
    void SetFoldScreenPolicy(sptr<FoldScreenPolicy> foldScreenPolicy);
    void SetSensorFoldStateManager(sptr<SensorFoldStateManager> sensorFoldStateManager);
    void RegisterPostureCallback();
    void RegisterHallCallback();
    void UnRegisterPostureCallback();
    void UnRegisterHallCallback();
    void HandlePostureData(const SensorEvent * const event);
    void HandleHallDataExt(const SensorEvent * const event);
    void PowerKeySetScreenActiveRect();
    bool IsPostureUserCallbackInvalid() const;
    bool IsHallUserCallbackInvalid() const;
    std::vector<float> GetGlobalAngle() const;
    std::vector<uint16_t> GetGlobalHall() const;
    bool isPowerRectExe_ = false;
private:
    sptr<FoldScreenPolicy> foldScreenPolicy_;
    sptr<SensorFoldStateManager> sensorFoldStateManager_;

    std::vector<float> globalAngle_ = {-1.0F, -1.0F, -1.0F};
    std::vector<uint16_t> globalHall_ = {USHRT_MAX, USHRT_MAX};
    bool registerPosture_ = false;

    SecondaryFoldSensorManager() = default;
    ~SecondaryFoldSensorManager() = default;
    void NotifyFoldAngleChanged(const std::vector<float> &angles);
    bool GetPostureInner(const SensorEvent * const event, float &valueBc, float &valueAb,
        float &valueAbAnti);
    bool GetHallInner(const SensorEvent * const event, uint16_t &valueBc, uint16_t &valueAb);
    bool IsDataBeyondBoundary();
};
} // namespace Rosen
} // namespace OHOS
#endif
#endif