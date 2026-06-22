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

#include "product_config_mock.h"
namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t PHY_WIDTH = 1316;
    constexpr uint32_t PHY_HEIGHT = 2832;
}

ProductConfigMock ProductConfigMock::currentConfig_;
std::mutex ProductConfigMock::configMutex_;

ProductConfigMock& ProductConfigMock::GetCurrentConfig()
{
    std::unique_lock<std::mutex> lock(configMutex_);
    return currentConfig_;
}

const ProductConfigMock& ProductConfigMock::ResetAndGetDeviceDefaultConfig()
{
    std::unique_lock<std::mutex> lock(configMutex_);
    if (currentConfig_.deviceType_ == ProductConfigMock::currentDeviceType_) {
        return currentConfig_;
    }

    if (ProductConfigMock::currentDeviceType_ == "PLR") {
        ProductConfigMock::ResetToPLR();
    }
    return currentConfig_;
}

void ProductConfigMock::ResetToPLR()
{
    currentConfig_.deviceType_ = "PLR";
    currentConfig_.innerScreenId_ = {0};
    currentConfig_.screenActiveModeId_ = {{0, 3}};
    currentConfig_.screenModeInfo_ = {{0,
                                       {{1316, 2832, 120, 0},
                                        {1316, 2832, 90, 1},
                                        {1316, 2832, 72, 2},
                                        {1316, 2832, 60, 3},
                                        {1316, 2832, 45, 4},
                                        {1316, 2832, 40, 5},
                                        {1316, 2832, 36, 6},
                                        {1316, 2832, 30, 7}}}};
    RSScreenCapability capability;
    capability.SetPhyWidth(PHY_WIDTH);
    capability.SetPhyHeight(PHY_HEIGHT);
    currentConfig_.screenCapability_[0] = capability;

    currentConfig_.screenHdrFormat_ = {{0, {ScreenHDRFormat::VIDEO_HDR_VIVID, ScreenHDRFormat::IMAGE_HDR_VIVID_DUAL}}};
    currentConfig_.screenColorSpace_ = {
        {0,
         {GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_BT2020_SRGB,
          GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_BT2020_HLG,
          GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_BT2020_PQ,
          GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_P3_PQ}}};
}
}  // namespace Rosen
}  // namespace OHOS