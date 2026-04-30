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

#include "rs_mock_impl.h"

namespace OHOS {
namespace Rosen {
std::shared_ptr<RsMockImpl> RsMockImpl::instance_ = nullptr;
std::mutex RsMockImpl::mutex_;

RsMockImpl& RsMockImpl::GetInstance()
{
    if (instance_ != nullptr) {
        return *instance_;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    if (instance_ != nullptr) {
        return *instance_;
    }
    instance_ = std::make_shared<RsMockImpl>();
    return *instance_;
}

void RsMockImpl::Rest()
{
    std::unique_lock<std::mutex> lock(mutex_);
    instance_ == nullptr;
}

void RsMockImpl::Init(const ProductConfigMock& data)
{
    productConfig_ = data;
}

int32_t RsMockImpl::SetScreenChangeCallback(const ScreenChangeCallback& callback)
{
    screenChangeCallback_ = callback;
    return 0;
}

void RsMockImpl::TriggerScreenChange(ScreenId screenId, ScreenEvent screenEvent, ScreenChangeReason reason)
{
    if (screenChangeCallback_ != nullptr) {
        screenChangeCallback_(screenId, screenEvent, reason, nullptr);
    }
}

RSScreenModeInfo RsMockImpl::GetScreenActiveMode(ScreenId id)
{
    auto modelIdIt = productConfig_.screenActiveModeId_.find(id);
    if (modelIdIt == productConfig_.screenActiveModeId_.end()) {
        return RSScreenModeInfo();
    }

    auto modelInfoIt = productConfig_.screenModeInfo_.find(id);
    if (modelInfoIt != productConfig_.screenModeInfo_.end() && modelInfoIt->second.size() > modelIdIt->second) {
        return modelInfoIt->second[modelIdIt->second];
    }
    return RSScreenModeInfo();
}

RSScreenCapability RsMockImpl::GetScreenCapability(ScreenId id)
{
    auto capIt = productConfig_.screenCapability_.find(id);
    if (capIt == productConfig_.screenCapability_.end()) {
        return RSScreenCapability();
    }
    return capIt->second;
}

int32_t RsMockImpl::SetScreenCorrection(ScreenId id, ScreenRotation screenRotation)
{
    screenCorrectionMap_[id] = screenRotation;
    return 0;
}

std::vector<RSScreenModeInfo> RsMockImpl::GetScreenSupportedModes(ScreenId id)
{
    auto modelInfoIt = productConfig_.screenModeInfo_.find(id);
    if (modelInfoIt != productConfig_.screenModeInfo_.end()) {
        return modelInfoIt->second;
    }
    return std::vector<RSScreenModeInfo>();
}

int32_t RsMockImpl::GetScreenSupportedHDRFormats(ScreenId id, std::vector<ScreenHDRFormat>& hdrFormats)
{
    auto it = productConfig_.screenHdrFormat_.find(id);
    if (it != productConfig_.screenHdrFormat_.end()) {
        hdrFormats = it->second;
    }

    return 0;
}

int32_t RsMockImpl::GetScreenSupportedColorSpaces(ScreenId id, std::vector<GraphicCM_ColorSpaceType>& colorSpaces)
{
    auto it = productConfig_.screenColorSpace_.find(id);
    if (it != productConfig_.screenColorSpace_.end()) {
        colorSpaces = it->second;
    }

    return 0;
}

int32_t RsMockImpl::RegisterHgmRefreshRateUpdateCallback(const HgmRefreshRateUpdateCallback& callback)
{
    hgmRefreshRateUpdateCallback_ = callback;
    return 0;
}

}  // namespace Rosen
}  // namespace OHOS