/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "sensor_agent.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

int32_t SubscribeSensor(int32_t sensorTypeId, const SensorUser* user)
{
    return 0;
}
int32_t UnsubscribeSensor(int32_t sensorTypeId, const SensorUser* user)
{
    return 0;
}
int32_t SetBatch(int32_t sensorTypeId, const SensorUser* user, int64_t samplingInterval, int64_t reportInterval)
{
    return 0;
}
int32_t ActivateSensor(int32_t sensorTypeId, const SensorUser* user)
{
    return 0;
}
int32_t DeactivateSensor(int32_t sensorTypeId, const SensorUser* user)
{
    return 0;
}
int32_t SetMode(int32_t sensorTypeId, const SensorUser* user, int32_t mode)
{
    return 0;
}

void SetDeviceStatus(uint32_t deviceStatus) {}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

