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
 
#ifndef OHOS_ROSEN_SUPER_FOLD_SENSOR_MANAGER_H
#define OHOS_ROSEN_SUPER_FOLD_SENSOR_MANAGER_H
 
#include <deque>
#include <memory>
#include <mutex>
#include <sstream>
#include <functional>
#include <climits>
 
#include "refbase.h"
#include "wm_single_instance.h"
#include "ws_common.h"
#include "sensor_agent.h"
#include "sensor_agent_type.h"
#include "common/include/task_scheduler.h"
#include "session/screen/include/screen_property.h"
#include "dm_common.h"
 
namespace OHOS {

namespace Rosen {
 
class SuperFoldSensorManager : public RefBase {
WM_DECLARE_SINGLE_INSTANCE_BASE(SuperFoldSensorManager);
 
public:
 
    void RegisterPostureCallback(); //折叠角度注册回调
 
    void RegisterHallCallback(); // 磁吸键盘
 
    void UnregisterPostureCallback();
 
    void UnregisterHallCallback();
 
    void HandlePostureData(const SensorEvent * const event);
 
    void HandleHallData(const SensorEvent * const event);

    void HandleSuperSensorChange(SuperFoldStatusChangeEvents events);

    float GetCurAngle();

    void DriveStateMachineToExpand();
    void SetStateMachineToActived();
    void HandleFoldStatusLockedToExpand();
    void HandleFoldStatusUnlocked();
    void SetTaskScheduler(std::shared_ptr<TaskScheduler> scheduler);

private:
    
    std::recursive_mutex mutex_;
 
    float curAngle_ = 170.0F;

    int32_t curInterval_ = 0;

    uint16_t curHall_ = USHRT_MAX;
 
    void NotifyFoldAngleChanged(float foldAngle);
 
    void NotifyHallChanged(uint16_t hall);

    void NotifySoftKeyboardChanged();
 
    SuperFoldSensorManager();
 
    ~SuperFoldSensorManager();

    std::shared_ptr<TaskScheduler> taskScheduler_ = nullptr;
};
}
}

#endif // OHOS_ROSEN_SUPER_FOLD_SENSOR_MANAGER_H