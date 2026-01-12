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

#ifndef OHOS_ROSEN_DMS_SCREEN_SENSOR_MGR_H
#define OHOS_ROSEN_DMS_SCREEN_SENSOR_MGR_H

#ifdef SENSOR_ENABLE
#include "sensor_agent.h"
#include "sensor_agent_type.h"
#include "nocopyable.h"
#include "fold_screen_common.h"

namespace OHOS {
namespace Rosen {
namespace DMS {

typedef struct {
    RecordSensorCallback taskCallback;
    int64_t interval;
} SensorCallbackEntry;

class ScreenSensorMgr {
    DISALLOW_COPY_AND_MOVE(ScreenSensorMgr);

public:
    static ScreenSensorMgr& GetInstance();
    void HandleSensorData(SensorEvent* event);
    void HandleTentSensorData(int32_t tentType, int32_t hall);
    void RegisterPostureCallback();
    void RegisterHallCallback();
    void RegisterApplicationStateObserver();
    void UnRegisterPostureCallback();
    void UnRegisterHallCallback();
    void HandlePostureData(const SensorEvent* const event);
    void HandleHallData(const SensorEvent* const event);
    int32_t SubscribeSensorCallback(int32_t sensorTypeId, int64_t interval, const RecordSensorCallback taskCallback);
    int32_t UnSubscribeSensorCallback(int32_t sensorTypeId);
    int32_t UpdateSensorInterval(int32_t sensorTypeId, int64_t interval);
    bool HasSubscribedSensor(int32_t sensorTypeId);
    bool GetSensorRegisterStatus(void);

protected:
    ScreenSensorMgr();
    virtual bool ParsePostureData(const SensorEvent* const event);
    virtual bool ParseHallData(const SensorEvent* const event);
    virtual void HandleAbnormalAngle();
    virtual SensorStatus GetSensorStatus(DmsSensorType updateSensorType);
    void NotifyFoldAngleChanged();
    bool checkCurrentPostureAndHall();
    template <typename T>
    T* GetSensorData(const SensorEvent* const event);

    std::vector<float> angle_;
    std::vector<uint16_t> hall_;
    float reflexionAngle_ = -1.0F;
    bool registerPosture_ = false;
    bool registerHall_ = false;
    std::vector<float> oldAngle_;
    TentSensorInfo tentSensorInfo_;

private:
    void CleanupCallback(int32_t sensorTypeId);

    std::map<int32_t, SensorCallbackEntry> sensorCallbacks_;
    std::unordered_map<int32_t, SensorUser> users_;
};

} // namespace DMS
} // namespace Rosen
} // namespace OHOS

#endif
#endif