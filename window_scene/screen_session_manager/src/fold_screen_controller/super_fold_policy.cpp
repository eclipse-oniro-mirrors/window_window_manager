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

#include <hisysevent.h>
#include <hitrace_meter.h>
#include "fold_screen_controller/super_fold_policy.h"
#include "window_manager_hilog.h"
#include <power_mgr_client.h>

namespace OHOS::Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(SuperFoldPolicy)
namespace {
constexpr DisplayId DEFAULT_DISPLAY_ID = 0;
constexpr ScreenId SCREEN_ID_DEFAULT = 0;
constexpr ScreenId SCREEN_ID_FULL = 0;
constexpr ScreenId SCREEN_ID_MAIN = 5;
constexpr uint64_t SCREEN_ID_SIZE_MAX = 2;
constexpr uint64_t SCREEN_ID_SIZE_ONE = 1;
constexpr DMRect FULL_SCREEN_RECORD_DMRECT = {0, 0, 0, 0};
constexpr DMRect FULL_SCREEN_RECORD_PHYDMRECT = {0, 0, DISPLAY_A_WIDTH, DISPLAY_A_HEIGHT};
const uint32_t MODE_CHANGE_TIMEOUT_MS = 2000;
const int64_t POWER_INIT_TIME_MS = 50;
#ifdef TP_FEATURE_ENABLE
const int32_t TP_TYPE = 12;
#endif
const std::string MAIN_TP = "1";
const std::string FULL_TP = "0";
}

bool SuperFoldPolicy::IsFakeDisplayExist()
{
    return ScreenSessionManager::GetInstance().GetDisplayInfoById(DISPLAY_ID_FAKE) != nullptr;
}

bool SuperFoldPolicy::IsNeedSetSnapshotRect(DisplayId displayId)
{
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_DEFAULT);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "session nullptr.");
        return false;
    }
    auto rotation = screenSession->GetRotation();
    return ((displayId == DEFAULT_DISPLAY_ID) || (displayId == DISPLAY_ID_FAKE)) &&
        (rotation == Rotation::ROTATION_0 || rotation == Rotation::ROTATION_180);
}

Drawing::Rect SuperFoldPolicy::GetSnapshotRect(DisplayId displayId, bool isCaptureFullOfScreen)
{
    Drawing::Rect snapshotRect = {0, 0, 0, 0};
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_DEFAULT);
    auto defaultInfo = ScreenSessionManager::GetInstance().GetDisplayInfoById(DEFAULT_DISPLAY_ID);
    if (screenSession == nullptr || defaultInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "session or display nullptr.");
        return snapshotRect;
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    auto screenWidth = screenProperty.GetPhyBounds().rect_.GetWidth();
    auto screenHeight = screenProperty.GetPhyBounds().rect_.GetHeight();
    DMRect creaseRect = screenProperty.GetCreaseRect();
    auto fakeInfo =  ScreenSessionManager::GetInstance().GetDisplayInfoById(DISPLAY_ID_FAKE);
    if (displayId == DISPLAY_ID_FAKE && !isCaptureFullOfScreen) {
        if (fakeInfo != nullptr) {
            snapshotRect = {0, defaultInfo->GetHeight() + static_cast<int32_t>(creaseRect.height_),
                screenWidth, screenHeight};
        }
    } else {
        SuperFoldStatus status = SuperFoldStateManager::GetInstance().GetCurrentStatus();
        bool isSystemKeyboardOn = SuperFoldStateManager::GetInstance().GetSystemKeyboardStatus();
        if (status == SuperFoldStatus::KEYBOARD ||
            (fakeInfo != nullptr && !isCaptureFullOfScreen)|| isSystemKeyboardOn) {
            snapshotRect = {0, 0, defaultInfo->GetWidth(), defaultInfo->GetHeight()};
        } else {
            snapshotRect = {0, 0, screenWidth, screenHeight};
        }
    }
    std::ostringstream oss;
    oss << "snapshot displayId: " << static_cast<uint32_t>(displayId)
        << " left: 0"
        << " top:" << snapshotRect.top_
        << " right:" << snapshotRect.right_
        << " bottom:" << snapshotRect.bottom_
        << " isCaptureFullOfScreen:" << isCaptureFullOfScreen;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    return snapshotRect;
}

DMRect SuperFoldPolicy::GetRecordRect(const std::vector<ScreenId>& screenIds)
{
    DMRect recordRect = {0, 0, 0, 0};
    auto screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_DEFAULT);
    auto defaultInfo = ScreenSessionManager::GetInstance().GetDisplayInfoById(DEFAULT_DISPLAY_ID);
    if (screenSession == nullptr || defaultInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "session or display nullptr.");
        return recordRect;
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    DMRect creaseRect = screenProperty.GetCreaseRect();
    SuperFoldStatus status = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    auto fakeInfo = ScreenSessionManager::GetInstance().GetDisplayInfoById(DISPLAY_ID_FAKE);
    if ((std::find(screenIds.begin(), screenIds.end(), DISPLAY_ID_FAKE) != screenIds.end()) && screenIds.size() == 1) {
        // record for fake screensession
        if (fakeInfo != nullptr) {
            recordRect = {0, defaultInfo->GetHeight() + static_cast<int32_t>(creaseRect.height_),
                defaultInfo->GetWidth(), fakeInfo->GetHeight()};
        }
    } else if ((std::find(screenIds.begin(), screenIds.end(), DEFAULT_DISPLAY_ID) != screenIds.end())
        && screenIds.size() == 1) {
        // record for 0 for one display
        bool isSystemKeyboardOn = SuperFoldStateManager::GetInstance().GetSystemKeyboardStatus();
        if (status == SuperFoldStatus::EXPANDED) {
            recordRect = {0, 0, 0, 0};
        } else if (status == SuperFoldStatus::KEYBOARD || fakeInfo != nullptr || isSystemKeyboardOn) {
            recordRect = {0, 0, defaultInfo->GetWidth(), defaultInfo->GetHeight()};
        }
    }
    std::ostringstream oss;
    oss << "snapshot left: 0"
        << " top:" << recordRect.posY_
        << " right:" << recordRect.width_
        << " bottom:" << recordRect.height_;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    return recordRect;
}

DMError SuperFoldPolicy::QueryMultiScreenCapture(const std::vector<ScreenId>& displayIdList, DMRect& rect)
{
    DMError ret = DMError::DM_ERROR_UNKNOWN;
    if (displayIdList.empty() || displayIdList.size() > SCREEN_ID_SIZE_MAX) {
        return DMError::DM_ERROR_INVALID_PARAM;
    } else if (displayIdList.size() == SCREEN_ID_SIZE_MAX) {
        if (std::find(displayIdList.begin(), displayIdList.end(), MAIN_SCREEN_ID_DEFAULT) != displayIdList.end() &&
            std::find(displayIdList.begin(), displayIdList.end(), SCREEN_ID_FAKE) != displayIdList.end()) {
            rect = FULL_SCREEN_RECORD_PHYDMRECT;
            ret = DMError::DM_OK;
        } else {
            return DMError::DM_ERROR_INVALID_PARAM;
        }
    } else {
        auto displayInfo = ScreenSessionManager::GetInstance().GetDisplayInfoById(displayIdList[0]);
        if (displayIdList[0] == SCREEN_ID_FAKE) {
            rect = GetRecordRect(displayIdList);
            ret = DMError::DM_OK;
        } else {
            rect = FULL_SCREEN_RECORD_DMRECT;
            if (displayInfo) {
                rect.width_ = displayInfo->GetWidth();
                rect.height_ = displayInfo->GetHeight();
                ret = DMError::DM_OK;
            } else {
                return DMError::DM_ERROR_INVALID_PARAM;
            }
        }
    }

    std::ostringstream oss;
    oss << "snapshot left: 0"
        << " top:" << rect.posY_
        << " right:" << rect.width_
        << " bottom:" << rect.height_;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    return ret;
}

ScreenId SuperFoldPolicy::GetRealScreenId(const std::vector<ScreenId>& screenIds)
{
    if (screenIds.empty() || screenIds.size() > SCREEN_ID_SIZE_MAX) {
        TLOGE(WmsLogTag::DMS, "mainScreenIds null");
        return SCREEN_ID_INVALID;
    }
    if (screenIds.size() == SCREEN_ID_SIZE_ONE) {
        TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64, static_cast<uint64_t>(screenIds[0]));
        if ((screenIds[0] == SCREEN_ID_DEFAULT || screenIds[0] == DISPLAY_ID_FAKE)) {
            return SCREEN_ID_DEFAULT;
        } else {
            return screenIds[0];
        }
    } else if (std::find(screenIds.begin(), screenIds.end(), SCREEN_ID_DEFAULT) != screenIds.end()
        && std::find(screenIds.begin(), screenIds.end(), DISPLAY_ID_FAKE) != screenIds.end()) {
        TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 ", %{public}" PRIu64,
            static_cast<uint64_t>(screenIds[0]), static_cast<uint64_t>(screenIds[1]));
        return SCREEN_ID_DEFAULT;
    }
    return SCREEN_ID_INVALID;
}

void SuperFoldPolicy::SetOnBootAnimation(bool onBootAnimation)
{
    TLOGI(WmsLogTag::DMS, "onBootAnimation: %{public}d", onBootAnimation);
    onBootAnimation_.store(onBootAnimation);
    if (!onBootAnimation_) {
        TLOGI(WmsLogTag::DMS, "when boot animation finished, change display mode");
        RecoverDisplayMode();
    }
}

void SuperFoldPolicy::BootAnimationFinishPowerInit()
{
    if (GetScreenClosedState() == ScreenClosedState::OPEN) {
        // coordination to full: power off main screen
        TLOGI(WmsLogTag::DMS, "Fold Screen Power main screen off.");
        ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(SCREEN_ID_FULL,
            ScreenPowerStatus::POWER_STATUS_OFF_FAKE);
        ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(SCREEN_ID_MAIN,
            ScreenPowerStatus::POWER_STATUS_ON);
        std::this_thread::sleep_for(std::chrono::milliseconds(POWER_INIT_TIME_MS));
        ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(SCREEN_ID_MAIN,
            ScreenPowerStatus::POWER_STATUS_OFF);
        ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(SCREEN_ID_FULL,
            ScreenPowerStatus::POWER_STATUS_ON);
    } else if (GetScreenClosedState() == ScreenClosedState::CLOSE) {
        // coordination to main: power off both and power on main screen
        TLOGI(WmsLogTag::DMS, "Fold Screen Power all screen off.");
        ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(SCREEN_ID_MAIN,
            ScreenPowerStatus::POWER_STATUS_OFF);
        ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(SCREEN_ID_FULL,
            ScreenPowerStatus::POWER_STATUS_OFF);

        std::this_thread::sleep_for(std::chrono::milliseconds(POWER_INIT_TIME_MS));
        TLOGI(WmsLogTag::DMS, "Fold Screen Power main screen on.");
        ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(SCREEN_ID_MAIN,
            ScreenPowerStatus::POWER_STATUS_ON);
    } else {
        TLOGI(WmsLogTag::DMS, "Fold Screen Power Init, invalid active screen id");
    }
}

void SuperFoldPolicy::RecoverDisplayMode()
{
    ScreenClosedState screenClosedState = GetScreenClosedState();
    FoldDisplayMode displayMode = GetModeMatchStatus(screenClosedState);
    if (displayMode != FoldDisplayMode::UNKNOWN && GetCurrentDisplayMode() != displayMode) {
        TLOGI(WmsLogTag::DMS, "recover displayMode to %{public}d", displayMode);
        ChangeScreenDisplayMode(displayMode);
    }
}

FoldDisplayMode SuperFoldPolicy::GetModeMatchStatus(ScreenClosedState screenClosedState)
{
    FoldDisplayMode foldDisplayMode = FoldDisplayMode::UNKNOWN;
    switch (screenClosedState) {
        case ScreenClosedState::CLOSE: {
            foldDisplayMode = FoldDisplayMode::MAIN;
            break;
        }
        case ScreenClosedState::OPEN: {
            foldDisplayMode = FoldDisplayMode::FULL;
            break;
        }
        default: {
            TLOGE(WmsLogTag::DMS, "invalid screenClosedState");
            break;
        }
    }
    return foldDisplayMode;
}

ScreenClosedState SuperFoldPolicy::GetScreenClosedState() const
{
    return screenClosedState_.load();
}

ScreenId SuperFoldPolicy::GetCurrentScreenId()
{
    std::lock_guard<std::mutex> lock(currentScreenIdMutex_);
    return currentScreenId_;
}

void SuperFoldPolicy::SetCurrentScreenId(ScreenId screenId)
{
    std::lock_guard<std::mutex> lock(currentScreenIdMutex_);
    currentScreenId_ = screenId;
}

void SuperFoldPolicy::SetLastCacheDisplayMode(FoldDisplayMode displayMode)
{
    lastCacheDisplayMode_.store(displayMode);
}

FoldDisplayMode SuperFoldPolicy::GetCurrentDisplayMode()
{
    return currentDisplayMode_.load();
}

void SuperFoldPolicy::SetCurrentDisplayMode(FoldDisplayMode displayMode)
{
    currentDisplayMode_.store(displayMode);
}

void SuperFoldPolicy::LockDisplayMode(bool isLock)
{
    isLockDisplayMode_.store(isLock);
}

DMError SuperFoldPolicy::SetScreenSwitchState(ScreenClosedState screenClosedState, bool isScreenOn)
{
    if (GetScreenClosedState() == screenClosedState) {
        TLOGE(WmsLogTag::DMS, "current screenClosedState aready is %{public}d", screenClosedState);
        return DMError::DM_OK;
    }
    screenClosedState_.store(screenClosedState);
    FoldDisplayMode displayMode = GetModeMatchStatus(screenClosedState);
    if (displayMode == FoldDisplayMode::UNKNOWN) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    ChangeScreenDisplayModeInner(displayMode, isScreenOn);
    // notify foldStatus
    NotifyFoldStatus(screenClosedState);
    ScreenSessionManager::GetInstance().NotifyScreenClosedStateChange(screenClosedState);
    return DMError::DM_OK;
}

bool SuperFoldPolicy::CheckDisplayMode(FoldDisplayMode displayMode)
{
    if (GetCurrentDisplayMode() == displayMode) {
        TLOGW(WmsLogTag::DMS, "ChangeScreenDisplayMode already in displayMode %{public}d", displayMode);
        return false;
    }
    if (onBootAnimation_.load()) {
        TLOGW(WmsLogTag::DMS, "onBootAnimation can not change mode");
        return false;
    }
    if (isLockDisplayMode_.load()) {
        TLOGW(WmsLogTag::DMS, "displayMode is locked");
        return false;
    }
    if (GetModeChangeRunningStatus()) {
        TLOGW(WmsLogTag::DMS, "last process not complete, skip mode: %{public}d", displayMode);
        return false;
    }
    return true;
}

bool SuperFoldPolicy::GetModeChangeRunningStatus()
{
    auto currentTime = std::chrono::steady_clock::now();
    auto intervalMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTimePoint_).count();
    if (intervalMs > MODE_CHANGE_TIMEOUT_MS) {
        TLOGE(WmsLogTag::DMS, "mode change timeout.");
        return false;
    }
    return GetdisplayModeRunningStatus();
}

bool SuperFoldPolicy::GetdisplayModeRunningStatus()
{
    return displayModeChangeRunning_.load();
}

void SuperFoldPolicy::SwitchScreenAndSetScreenPower(ScreenId screenId, bool isScreenOn)
{
    ScreenId offScreenId = GetCurrentScreenId();
    if (screenId == SCREEN_ID_MAIN) {
        offScreenId = SCREEN_ID_FULL;
    } else if (screenId == SCREEN_ID_FULL) {
        offScreenId = SCREEN_ID_MAIN;
    }
    if (isScreenOn) {
        auto task = [=] {
            TLOGNI(WmsLogTag::DMS, "SetScreenPower: off screenId: %{public}" PRIu64"", offScreenId);
            ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(offScreenId,
                ScreenPowerStatus::POWER_STATUS_OFF);
            TLOGNI(WmsLogTag::DMS, "SetScreenPower: on screenId: %{public}" PRIu64"", screenId);
            ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(screenId,
                ScreenPowerStatus::POWER_STATUS_ON);
            SetdisplayModeChangeStatus(false);
        };
        ScreenSessionManager::GetInstance().GetScreenPowerTaskScheduler()->
            PostAsyncTask(task, __func__);
    } else {
        SetdisplayModeChangeStatus(false);
    }
    SetCurrentScreenId(screenId);
}

DMError SuperFoldPolicy::ChangeScreenDisplayMode(FoldDisplayMode displayMode)
{
    bool isScreenOn = PowerMgr::PowerMgrClient::GetInstance().IsFoldScreenOn();
    return ChangeScreenDisplayModeInner(displayMode, isScreenOn);
}

DMError SuperFoldPolicy::ChangeScreenDisplayModeInner(FoldDisplayMode displayMode, bool isScreenOn)
{
    SetLastCacheDisplayMode(displayMode);
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:ChangeScreenDisplayMode(displayMode= %" PRIu64")", displayMode);
    if (!CheckDisplayMode(displayMode)) {
        return DMError::DM_OK;
    }
    SetdisplayModeChangeStatus(true);
    std::string tp = FULL_TP;
    ReportFoldDisplayModeChange(displayMode);
    switch (displayMode) {
        case FoldDisplayMode::MAIN: {
            SwitchScreenAndSetScreenPower(SCREEN_ID_MAIN, isScreenOn);
            tp = MAIN_TP;
            break;
        }
        case FoldDisplayMode::FULL: {
            SwitchScreenAndSetScreenPower(SCREEN_ID_FULL, isScreenOn);
            tp = FULL_TP;
            break;
        }
        case FoldDisplayMode::COORDINATION: {
            ChangeScreenDisplayModeToCoordination(isScreenOn);
            tp = MAIN_TP;
            break;
        }
        default: {
            return DMError::DM_ERROR_INVALID_MODE_ID;
        }
    }
#ifdef TP_FEATURE_ENABLE
    RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE, tp.c_str());
#endif
    SetCurrentDisplayMode(displayMode);
    SetdisplayModeChangeStatus(false);
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
    return DMError::DM_OK;
}

void SuperFoldPolicy::ChangeScreenDisplayModeToCoordination(bool isScreenOn)
{
    if (GetCurrentDisplayMode() != FoldDisplayMode::FULL) {
        TLOGI(WmsLogTag::DMS, "only full can enter coordination");
        return;
    }
    ScreenSessionManager::GetInstance().SetCoordinationFlag(true);
    auto taskCoordination = [=] {
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayMode: on full screenId");
        if (!isScreenOn) {
            PowerMgr::PowerMgrClient::GetInstance().WakeupDeviceAsync();
        }
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayMode: on main screenId");
        ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(SCREEN_ID_MAIN,
            ScreenPowerStatus::POWER_STATUS_ON);
        SetdisplayModeChangeStatus(false);
    };
    ScreenSessionManager::GetInstance().GetScreenPowerTaskScheduler()->
        PostAsyncTask(taskCoordination, __func__);
}

void SuperFoldPolicy::ExitCoordination()
{
    if (GetCurrentDisplayMode() != FoldDisplayMode::COORDINATION) {
        TLOGI(WmsLogTag::DMS, "not in coordination");
        return;
    }
    ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(SCREEN_ID_MAIN,
        ScreenPowerStatus::POWER_STATUS_OFF);
    ScreenSessionManager::GetInstance().SetCoordinationFlag(false);
    RecoverDisplayMode();
}

void SuperFoldPolicy::ReportFoldDisplayModeChange(FoldDisplayMode displayMode)
{
    int32_t mode = static_cast<int32_t>(displayMode);
    TLOGI(WmsLogTag::DMS, "displayMode: %{public}d", mode);
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "DISPLAY_MODE",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "FOLD_DISPLAY_MODE", mode);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "Write HiSysEvent error, ret: %{public}d", ret);
    }
}

void SuperFoldPolicy::SetdisplayModeChangeStatus(bool status, bool isOnBootAnimation)
{
    if (status) {
        pendingTask_ = SWITCH_SCREEN_TASK_NUM;
        startTimePoint_ = std::chrono::steady_clock::now();
        displayModeChangeRunning_ = status;
    } else {
        pendingTask_ --;
        if (pendingTask_ != 0) {
            return;
        }
        displayModeChangeRunning_ = false;
        endTimePoint_ = std::chrono::steady_clock::now();
        if (lastCacheDisplayMode_.load() != GetCurrentDisplayMode()) {
            TLOGI(WmsLogTag::DMS, "start change displaymode to lastest mode");
            ScreenSessionManager::GetInstance().TriggerDisplayModeUpdate(lastCacheDisplayMode_.load());
        }
    }
}

bool SuperFoldPolicy::SetAndCheckFoldStatus(FoldStatus foldStatus)
{
    std::lock_guard<std::mutex> lock(phyFoldStatusMutex_);
    phyFoldStatus_ = foldStatus;
    if (GetScreenClosedState() == ScreenClosedState::CLOSE) {
        return false;
    }
    if (foldStatus == FoldStatus::FOLDED) {
        return false;
    }
    if (lastFoldStatus_ == foldStatus) {
        return false;
    }
    lastFoldStatus_ = foldStatus;
    return true;
}

FoldStatus SuperFoldPolicy::GetPhyFoldStatus()
{
    std::lock_guard<std::mutex> lock(phyFoldStatusMutex_);
    return phyFoldStatus_;
}

void SuperFoldPolicy::NotifyFoldStatus(ScreenClosedState screenClosedState)
{
    FoldStatus foldStatus = FoldStatus::UNKNOWN;
    if (screenClosedState == ScreenClosedState::CLOSE) {
        foldStatus = FoldStatus::FOLDED;
    } else if (screenClosedState == ScreenClosedState::OPEN) {
        FoldStatus phyFoldStatus = GetPhyFoldStatus();
        foldStatus = FoldStatus::HALF_FOLD;
        if (phyFoldStatus == FoldStatus::HALF_FOLD || phyFoldStatus == FoldStatus::EXPAND) {
            foldStatus = phyFoldStatus;
        }
    }
    TLOGI(WmsLogTag::DMS, "ScreenClosedState:%{public}d, foldStatus:%{public}d", screenClosedState, foldStatus);
    if (foldStatus == FoldStatus::UNKNOWN) {
        TLOGE(WmsLogTag::DMS, "invalid foldstatus");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(phyFoldStatusMutex_);
        if (lastFoldStatus_ == foldStatus) {
            return;
        }
        lastFoldStatus_ = foldStatus;
    }
    ScreenSessionManager::GetInstance().NotifyFoldStatusChangedInner(foldStatus);
}
} // namespace OHOS::Rosen
