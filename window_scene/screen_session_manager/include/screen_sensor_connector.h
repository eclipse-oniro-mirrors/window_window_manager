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

#ifndef OHOS_ROSEN_SCREEN_SENSOR_CONNECTOR_H
#define OHOS_ROSEN_SCREEN_SENSOR_CONNECTOR_H

#include <map>
#include <refbase.h>
#include "dm_common.h"
#include "screen_rotation_property.h"
#include "screen_tent_property.h"
#ifdef SENSOR_ENABLE
#include "sensor_agent.h"
#endif
#include "window_manager_hilog.h"
#include "screen_sensor_plugin.h"

namespace OHOS {
namespace Rosen {
class ScreenSensorConnector : public RefBase {
public:
    ScreenSensorConnector() = delete;
    ~ScreenSensorConnector() = default;

    static void SubscribeRotationSensor();
    static void UnsubscribeRotationSensor();
    static void SubscribeTentSensor();
    static void UnsubscribeTentSensor();
};

#ifdef WM_SUBSCRIBE_MOTION_ENABLE
class MotionSubscriber {
friend ScreenSensorConnector;
public:
    MotionSubscriber() = delete;
    ~MotionSubscriber() = default;
private:
    static void SubscribeMotionSensor();
    static void UnsubscribeMotionSensor();

    static bool isMotionSensorSubscribed_;
};

class MotionTentSubscriber {
friend ScreenSensorConnector;
public:
    MotionTentSubscriber() = delete;
    ~MotionTentSubscriber() = default;
private:
    static void SubscribeMotionSensor();
    static void UnsubscribeMotionSensor();

    static bool isMotionSensorSubscribed_;
};
#endif
} // Rosen
} // OHOS
#endif // OHOS_ROSEN_SCREEN_CONNECTOR_H
