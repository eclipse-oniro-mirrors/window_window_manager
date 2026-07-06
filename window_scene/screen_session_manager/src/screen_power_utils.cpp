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

#include "screen_power_utils.h"
#include "power_mgr_client.h"
#ifdef WM_SCREENLOCK_MANAGER_ENABLED
#include "screenlock_manager.h"
#endif
#include "screen_session_manager.h"
#include "window_manager_hilog.h"

using namespace OHOS::PowerMgr;

namespace OHOS {
namespace Rosen {

std::mutex ScreenPowerUtils::powerTimingMutex_;
#ifdef WM_SCREENLOCK_MANAGER_ENABLED
std::mutex ScreenPowerUtils::powerLockMutex_;
#endif
bool ScreenPowerUtils::isEnablePowerForceTimingOut_ = false;

void ScreenPowerUtils::EnablePowerForceTimingOut()
{
    std::lock_guard<std::mutex> lock(powerTimingMutex_);
    TLOGI(WmsLogTag::DMS, "Enable power timeout begin.");
    if (isEnablePowerForceTimingOut_) {
        TLOGI(WmsLogTag::DMS, "Already enable.");
        return;
    }

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerErrors setRet = powerMgrClient.SetForceTimingOut(true);
    if (setRet != PowerErrors::ERR_OK) {
        TLOGE(WmsLogTag::DMS, "SetForceTimingOut failed: %{public}d", setRet);
        return;
    }

    PowerErrors lockRet = powerMgrClient.LockScreenAfterTimingOut(true, true, false);
    if (lockRet != PowerErrors::ERR_OK) {
        setRet = powerMgrClient.SetForceTimingOut(false);
        TLOGE(WmsLogTag::DMS, "LockScreenAfterTimingOut failed, lockRet: %{public}d, setRet: %{public}d",
            lockRet, setRet);
        return;
    }
    isEnablePowerForceTimingOut_ = true;
}

void ScreenPowerUtils::DisablePowerForceTimingOut()
{
    std::lock_guard<std::mutex> lock(powerTimingMutex_);
    TLOGI(WmsLogTag::DMS, "Disable power timeout begin.");
    if (!isEnablePowerForceTimingOut_) {
        TLOGI(WmsLogTag::DMS, "Already disable.");
        return;
    }
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    PowerErrors setRet = powerMgrClient.SetForceTimingOut(false);
    if (setRet != PowerErrors::ERR_OK) {
        TLOGE(WmsLogTag::DMS, "SetForceTimingOut failed: %{public}d", setRet);
    }

    PowerErrors lockRet = powerMgrClient.LockScreenAfterTimingOut(true, false, true);
    if (lockRet != PowerErrors::ERR_OK) {
        TLOGE(WmsLogTag::DMS, "LockScreenAfterTimingOut failed, lockRet: %{public}d", lockRet);
    }
    isEnablePowerForceTimingOut_ = false;
}

void ScreenPowerUtils::LightAndLockScreen(const std::string& detail)
{
#ifndef WM_SCREENLOCK_MANAGER_ENABLED
    return;
#else
    std::lock_guard<std::mutex> lock(powerLockMutex_);
    auto& powerMgrClient = PowerMgrClient::GetInstance();
    auto lockManager = OHOS::ScreenLock::ScreenLockManager::GetInstance();
    if (lockManager == nullptr) {
        TLOGE(WmsLogTag::DMS, "lockManager is nullptr");
        return;
    }
    bool isScreenLocked = lockManager->IsScreenLocked();
    bool isScreenOn = powerMgrClient.IsScreenOn();
    TLOGI(WmsLogTag::DMS, "isScreenLocked: %{public}d, isScreenOn: %{public}d", isScreenLocked, isScreenOn);
    if (!isScreenOn && !isScreenLocked) {
        PowerErrors wakeupRet = powerMgrClient.WakeupDevice(WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, detail);
        auto userId = ScreenSessionManager::GetInstance().GetCurrentUserId();
        std::string identity = IPCSkeleton::ResetCallingIdentity();
        int32_t lockRet = lockManager->Lock(userId);
        IPCSkeleton::SetCallingIdentity(identity);
        TLOGI(WmsLogTag::DMS, "wakeupRet: %{public}d, lockRet: %{public}d", wakeupRet, lockRet);
    }
#endif
}
bool ScreenPowerUtils::GetEnablePowerForceTimingOut()
{
    return isEnablePowerForceTimingOut_;
}
} // Rosen
} // OHOS