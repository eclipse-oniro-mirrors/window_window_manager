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

#include "session_sensor_plugin.h"
#include <map>
#include <mutex>

namespace OHOS {
namespace Rosen {

namespace {
std::map<int32_t, SessionOnMotionChangedPtr> g_callbackMap;
std::mutex g_mutex;
bool g_loaded = false;
}

bool SessionLoadMotionSensor(void)
{
    g_loaded = true;
    return true;
}

void SessionUnloadMotionSensor(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_callbackMap.clear();
    g_loaded = false;
}

bool SessionSubscribeCallback(int32_t motionType, SessionOnMotionChangedPtr callback)
{
    if (callback == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(g_mutex);
    g_callbackMap[motionType] = callback;
    return true;
}

bool SessionUnsubscribeCallback(int32_t motionType, SessionOnMotionChangedPtr callback)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_callbackMap.find(motionType);
    if (it != g_callbackMap.end() && it->second == callback) {
        g_callbackMap.erase(it);
        return true;
    }
    return false;
}

void TriggerSessionMotionEvent(int32_t motionType, int32_t status)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_callbackMap.find(motionType);
    if (it != g_callbackMap.end() && it->second != nullptr) {
        SessionMotionSensorEvent event;
        event.type = motionType;
        event.status = status;
        event.dataLen = 0;
        event.data = nullptr;
        it->second(event);
    }
}

bool IsSessionMotionSensorLoaded()
{
    return g_loaded;
}

bool IsSessionMotionSensorSubscribed(int32_t motionType)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_callbackMap.find(motionType) != g_callbackMap.end();
}

}
}
