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
 
#ifndef OHOS_ROSEN_SENSOR_PLUGIN_MOCK_IMPL_H
#define OHOS_ROSEN_SENSOR_PLUGIN_MOCK_IMPL_H

#include <map>
#include <mutex>
#include <vector>
#include <cstdint>
#include "screen_sensor_plugin.h"

namespace OHOS {
namespace Rosen {

class SensorPluginMockImpl {
public:
    static SensorPluginMockImpl& GetInstance();

    bool SubscribeCallback(int32_t motionType, OnMotionChangedPtr callback);
    bool UnsubscribeCallback(int32_t motionType, OnMotionChangedPtr callback);

    void TriggerSensorEvent(int32_t motionType, int32_t status, int32_t dataLen, int32_t* data);
    void Reset();

private:
    SensorPluginMockImpl() = default;
    ~SensorPluginMockImpl() = default;

    std::map<int32_t, OnMotionChangedPtr> callbacks_;
    std::mutex mutex_;
};

}  // namespace Rosen
}  // namespace OHOS

#endif  // OHOS_ROSEN_SENSOR_PLUGIN_MOCK_IMPL_H