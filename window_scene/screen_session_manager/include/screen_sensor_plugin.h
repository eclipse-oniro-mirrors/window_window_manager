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

#ifndef SCREEN_SENSOR_PLUGIN_H
#define SCREEN_SENSOR_PLUGIN_H

#include <string>
#include <stdio.h>
#include <stdlib.h>

#include <dlfcn.h>
#include <unistd.h>

#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
#if (defined(__aarch64__) || defined(__x86_64__))
const std::string PLUGIN_SO_PATH = "/system/lib64/platformsdk/libmotion_agent.z.so";
#else
const std::string PLUGIN_SO_PATH = "/system/lib/platformsdk/libmotion_agent.z.so";
#endif

typedef struct MotionSensorEvent {
    int32_t type = -1;
    int32_t status = -1;
    int32_t dataLen = -1;
    int32_t *data = nullptr;
} MotionSensorEvent;

using OnMotionChangedPtr = void (*)(const MotionSensorEvent&);
using MotionSubscribeCallbackPtr =  bool (*)(int32_t, OnMotionChangedPtr);
using MotionUnsubscribeCallbackPtr = bool (*)(int32_t, OnMotionChangedPtr);

bool LoadMotionSensor(void);
void UnloadMotionSensor(void);
bool SubscribeCallback(int32_t motionType, OnMotionChangedPtr callback);
bool UnsubscribeCallback(int32_t motionType, OnMotionChangedPtr callback);
}
}
#endif /* SCREEN_SENSOR_PLUGIN_H */