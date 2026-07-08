/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef SESSION_SENSOR_PLUGIN_H
#define SESSION_SENSOR_PLUGIN_H

#include <string>
#include <stdio.h>
#include <stdlib.h>

#include <dlfcn.h>
#include <unistd.h>

#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
typedef struct SessionMotionSensorEvent {
    int32_t type = -1;
    int32_t status = -1;
    int32_t dataLen = -1;
    int32_t *data = nullptr;
} SessionMotionSensorEvent;

using SessionOnMotionChangedPtr = void (*)(const SessionMotionSensorEvent&);
using SessionMotionSubscribeCallbackPtr = bool (*)(int32_t, SessionOnMotionChangedPtr);
using SessionMotionUnsubscribeCallbackPtr = bool (*)(int32_t, SessionOnMotionChangedPtr);

bool SessionLoadMotionSensor(void);
void SessionUnloadMotionSensor(void);
bool SessionSubscribeCallback(int32_t motionType, SessionOnMotionChangedPtr callback);
bool SessionUnsubscribeCallback(int32_t motionType, SessionOnMotionChangedPtr callback);
}
}
#endif /* SESSION_SENSOR_PLUGIN_H */
