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
 
#include "screen_power_mgr.h"
 
#include "product_ext_wrapper.h"
#include "window_manager_hilog.h"
 
#include <transaction/rs_interfaces.h>
 
namespace OHOS::Rosen::DMS {
 
ScreenPowerMgr::ScreenPowerMgr()
{
    TLOGI(WmsLogTag::DMS, "ScreenPowerMgr+");
}
 
ScreenPowerMgr& ScreenPowerMgr::GetInstance()
{
    TLOGI(WmsLogTag::DMS, "ScreenPowerMgr GetInstance+");
    static std::mutex singletonMutex_;
    static ScreenPowerMgr* instance_ = nullptr;
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(singletonMutex_);
        if (instance_ == nullptr) {
            TLOGI(WmsLogTag::DMS, "init ScreenPowerMgr from ext");
            instance_ = ProductExtWrapper::GetExtInstance<ScreenPowerMgr>("GetScreenPowerMgr");
        }
        if (instance_ == nullptr) {
            TLOGI(WmsLogTag::DMS, "init ScreenPowerMgr");
            instance_ = new ScreenPowerMgr();
        }
    }
    return *instance_;
}
 
void ScreenPowerMgr::ClearScreenPowerStatus(ScreenId rsScreenId)
{
    TLOGI(WmsLogTag::DMS, "ScreenPowerMgr ClearScreenPowerStatus+");
    std::lock_guard<std::mutex> lock(screenPowerStatusMapMutex_);
    screenPowerStatusMap_.erase(rsScreenId);
}
 
bool ScreenPowerMgr::IsScreenPowerOn(ScreenId rsScreenId)
{
    TLOGI(WmsLogTag::DMS, "ScreenPowerMgr IsScreenPowerOn+");
    std::lock_guard<std::mutex> lock(screenPowerStatusMapMutex_);
    auto it = screenPowerStatusMap_.find(rsScreenId);
    if (it != screenPowerStatusMap_.end()) {
        TLOGNFI(
            WmsLogTag::DMS, "[UL_POWER]screenId: %{public}" PRIu64 " powerStatus:%{public}u", rsScreenId, it->second);
        if (it->second == ScreenPowerStatus::POWER_STATUS_ON) {
            return true;
        }
    } else {
        TLOGNFI(WmsLogTag::DMS, "[UL_POWER]cannot find screenPowerStatusMap_ id: %{public}" PRIu64, rsScreenId);
    }
    return false;
}
 
void ScreenPowerMgr::DoSetScreenPowerStatus(ScreenId rsScreenId, ScreenPowerStatus status)
{
    TLOGI(WmsLogTag::DMS, "ScreenPowerMgr DoSetScreenPowerStatus+");
    bool setAdvancedOffFlag = false;
    {
        std::lock_guard<std::mutex> lock(screenPowerStatusMapMutex_);
        auto it = screenPowerStatusMap_.find(rsScreenId);
        if (it != screenPowerStatusMap_.end() && it->second == ScreenPowerStatus::POWER_STATUS_ON_ADVANCED &&
            status == ScreenPowerStatus::POWER_STATUS_OFF) {
            setAdvancedOffFlag = true;
        }
    }
 
    // when the power status is ON_ADVANCED and need to be set to OFF，first set the status to OFF_ADVANCED.
    if (setAdvancedOffFlag) {
        TLOGNFW(WmsLogTag::DMS,
            "set the power status to OFF_ADVANCED first, screenId: %{public}" PRIu64 ", status: %{public}d.",
            rsScreenId, status);
        status = ScreenPowerStatus::POWER_STATUS_OFF_ADVANCED;
    }
    RSInterfaces::GetInstance().SetScreenPowerStatus(rsScreenId, status);
    {
        std::lock_guard<std::mutex> lock(screenPowerStatusMapMutex_);
        screenPowerStatusMap_[rsScreenId] = status;
    }
}
 
void ScreenPowerMgr::SetTpFeatureConfig(int32_t tpType, const std::string& tpConfig)
{
    TLOGI(WmsLogTag::DMS, "ScreenPowerMgr SetTpFeatureConfig+");
#ifdef TP_FEATURE_ENABLE
    RSInterfaces::GetInstance().SetTpFeatureConfig(tpType, tpConfig.c_str());
#endif
}
 
}  // namespace OHOS::Rosen::DMS