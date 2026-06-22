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

#ifndef OHOS_ROSEN_PRODUCT_CONFIG_MOCK_H
#define OHOS_ROSEN_PRODUCT_CONFIG_MOCK_H

#include <mutex>
#include "transaction/rs_interfaces.h"

namespace OHOS {
namespace Rosen {
class ProductConfigMock {
public:
    ProductConfigMock() = default;
    ~ProductConfigMock() = default;
    static ProductConfigMock& GetCurrentConfig();
    static const ProductConfigMock& ResetAndGetDeviceDefaultConfig();
    static void ResetToPLR();

    std::string deviceType_;
    std::vector<ScreenId> innerScreenId_;
    std::map<ScreenId, int32_t> screenActiveModeId_;
    std::map<ScreenId, std::vector<RSScreenModeInfo>> screenModeInfo_;
    std::map<ScreenId, RSScreenCapability> screenCapability_;
    std::map<ScreenId, std::vector<ScreenHDRFormat>> screenHdrFormat_;
    std::map<ScreenId, std::vector<GraphicCM_ColorSpaceType>> screenColorSpace_;

private:
    static ProductConfigMock currentConfig_;
    static std::mutex configMutex_;
    static std::string currentDeviceType_;
};

}  // namespace OHOS
}  // namespace OHOS

#endif