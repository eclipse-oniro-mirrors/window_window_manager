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

#include "parameter.h"
#include "parameters.h"
#include <iostream>

int WatchParameter(const char* keyprefix, ParameterChgPtr callback, void* context)
{
    return 0;
}

namespace OHOS {
namespace system {

DeviceParamMock& DeviceParamMock::GetInstance()
{
    static DeviceParamMock instance;
    return instance;
}

std::map<std::string, std::string> DeviceParamMock::GetDeviceParam()
{
    std::unique_lock<std::mutex> lock(paramMutex_);
    if (!deviceParam_.empty()) {
        return deviceParam_;
    }

    if (currentDeviceType_ == "PLR") {
        deviceParam_ = {{"const.logsystem.versiontype", "beta"},
                        {"const.product.devicetype", "phone"},
                        {"const.product.real_device_radius", "beta"}};
    }
    return deviceParam_;
}

void DeviceParamMock::SetDeviceParam(const std::string& key, const std::string& value)
{
    std::unique_lock<std::mutex> lock(paramMutex_);
    deviceParam_[key] = value;
}

std::string GetParameter(const std::string& key, const std::string& def)
{
    const auto& config = DeviceParamMock::GetInstance().GetDeviceParam();
    auto findIt = config.find(key);
    if (findIt != config.end()) {
        return findIt->second;
    }
    return def;
}

bool GetBoolParameter(const std::string& key, bool def)
{
    return true;
}

template <typename T>
T GetIntParameter(const std::string& key, T def, T min, T max)
{
    return def;
}

template <typename T>
T GetUintParameter(const std::string& key, T def, T max)
{
    return def;
}

bool SetParameter(const std::string& key, const std::string& value)
{
    DeviceParamMock::GetInstance().SetDeviceParam(key, value);
    return true;
}

std::string GetDeviceType(void)
{
    return "";
}
}  // namespace system
}  // namespace OHOS