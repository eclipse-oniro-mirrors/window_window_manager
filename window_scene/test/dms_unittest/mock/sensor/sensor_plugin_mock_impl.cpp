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
 
#include "sensor_plugin_mock_impl.h"
#include <map>
#include <mutex>
#include <vector>

namespace OHOS {
namespace Rosen {

SensorPluginMockImpl& SensorPluginMockImpl::GetInstance()
{
    static SensorPluginMockImpl instance;
    return instance;
}

bool SensorPluginMockImpl::SubscribeCallback(int32_t motionType, OnMotionChangedPtr callback)
{
    if (callback == nullptr) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_[motionType] = callback;
    return true;
}

bool SensorPluginMockImpl::UnsubscribeCallback(int32_t motionType, OnMotionChangedPtr callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = callbacks_.find(motionType);
    if (it != callbacks_.end() && it->second == callback) {
        callbacks_.erase(it);
        return true;
    }
    return false;
}

void SensorPluginMockImpl::TriggerSensorEvent(int32_t motionType, int32_t status, int32_t dataLen, int32_t* data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = callbacks_.find(motionType);
    if (it != callbacks_.end() && it->second != nullptr) {
        MotionSensorEvent event;
        event.type = motionType;
        event.status = status;
        event.dataLen = dataLen;
        event.data = data;
        it->second(event);
    }
}

void SensorPluginMockImpl::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.clear();
}

}  // namespace Rosen
}  // namespace OHOS