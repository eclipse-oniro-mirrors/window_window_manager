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


#ifndef OHOS_ROSEN_DMS_RS_MOCK_IMPL_H
#define OHOS_ROSEN_DMS_RS_MOCK_IMPL_H

#include <mutex>
#include "transaction/rs_interfaces.h"
#include "product_config_mock.h"

namespace OHOS {
namespace Rosen {
class RsMockImpl {
public:
    static RsMockImpl& GetInstance();
    static void Rest();
    RsMockImpl() = default;
    virtual ~RsMockImpl() = default;

    virtual int32_t SetScreenChangeCallback(const ScreenChangeCallback& callback);
    virtual RSScreenModeInfo GetScreenActiveMode(ScreenId id);
    virtual RSScreenCapability GetScreenCapability(ScreenId id);
    virtual int32_t SetScreenCorrection(ScreenId id, ScreenRotation screenRotation);
    virtual std::vector<RSScreenModeInfo> GetScreenSupportedModes(ScreenId id);
    virtual int32_t GetScreenSupportedHDRFormats(ScreenId id, std::vector<ScreenHDRFormat>& hdrFormats);
    virtual int32_t GetScreenSupportedColorSpaces(ScreenId id, std::vector<GraphicCM_ColorSpaceType>& colorSpaces);
    virtual int32_t RegisterHgmRefreshRateUpdateCallback(const HgmRefreshRateUpdateCallback& callback);

    // event trigger
    void TriggerScreenChange(ScreenId screenId, ScreenEvent screenEvent, ScreenChangeReason reason);

    void Init(const ProductConfigMock& data);

private:
    static std::shared_ptr<RsMockImpl> instance_;
    static std::mutex mutex_;
    ScreenChangeCallback screenChangeCallback_;
    HgmRefreshRateUpdateCallback hgmRefreshRateUpdateCallback_;

    ProductConfigMock productConfig_;
    std::map<ScreenId, ScreenRotation> screenCorrectionMap_;
};
}  // namespace Rosen
}  // namespace OHOS
#endif