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

#ifndef SYSTEM_PARAMETERS_H
#define SYSTEM_PARAMETERS_H

#include <limits>
#include <string>
#include <map>
#include <mutex>

namespace OHOS {
namespace system {

class DeviceParamMock {
public:
    DeviceParamMock() = default;
    ~DeviceParamMock() = default;
    static DeviceParamMock& GetInstance();
    std::map<std::string, std::string> GetDeviceParam();
    void SetDeviceParam(const std::string& key, const std::string& value);
    static std::string currentDeviceType_;

private:
    std::map<std::string, std::string> deviceParam_;
    std::mutex paramMutex_;
};

std::string GetParameter(const std::string& key, const std::string& def);

bool GetBoolParameter(const std::string& key, bool def);

template <typename T>
T GetIntParameter(const std::string& key, T def, T min = std::numeric_limits<T>::min(),
                  T max = std::numeric_limits<T>::max());

template <typename T>
T GetUintParameter(const std::string& key, T def, T max = std::numeric_limits<T>::max());

bool SetParameter(const std::string& key, const std::string& value);
std::string GetDeviceType(void);

}  // namespace system
}  // namespace OHOS

#endif  // SYSTEM_PARAMETERS_H