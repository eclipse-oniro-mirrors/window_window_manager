/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <transaction/rs_interfaces.h>
#include "fold_screen_controller/single_display_fold_policy.h"
#include "session/screen/include/screen_session.h"
#include "screen_session_manager.h"
#include "fold_screen_state_internel.h"

#include "window_manager_hilog.h"

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

namespace OHOS::Rosen {
namespace {
const ScreenId SCREEN_ID_FULL = 0;
const ScreenId SCREEN_ID_MAIN = 5;
const int32_t CREASE_REGION_POS_Y = 994;
const int32_t CREASE_REGION_POS_WIDTH = 1320;
const int32_t CREASE_REGION_POS_HEIGHT = 132;
constexpr int32_t FOLD_CREASE_RECT_SIZE = 4; //numbers of parameter on the current device is 4
const std::string g_FoldScreenRect = system::GetParameter("const.display.foldscreen.crease_region", "");
const std::string FOLD_CREASE_DELIMITER = ",;";

#ifdef TP_FEATURE_ENABLE
const int32_t TP_TYPE = 12;
const int32_t TP_TYPE_POWER_CTRL = 18;
const std::string FULL_TP = "0";
const std::string MAIN_TP = "1";
const std::string MAIN_TP_OFF = "1,1";
const std::string FULL_TP_OFF = "0,1";
#endif
} // namespace

SingleDisplayFoldPolicy::SingleDisplayFoldPolicy(std::recursive_mutex& displayInfoMutex,
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler)
    : displayInfoMutex_(displayInfoMutex), screenPowerTaskScheduler_(screenPowerTaskScheduler)
{
    TLOGI(WmsLogTag::DMS, "SingleDisplayFoldPolicy created");

    ScreenId screenIdFull = 0;
    int32_t foldCreaseRegionPosX = 0;
    int32_t foldCreaseRegionPosY = CREASE_REGION_POS_Y;
    int32_t foldCreaseRegionPosWidth = CREASE_REGION_POS_WIDTH;
    int32_t foldCreaseRegionPosHeight = CREASE_REGION_POS_HEIGHT;

    std::vector<DMRect> rect = {
        {
            foldCreaseRegionPosX, foldCreaseRegionPosY,
            foldCreaseRegionPosWidth, foldCreaseRegionPosHeight
        }
    };
    currentFoldCreaseRegion_ = new FoldCreaseRegion(screenIdFull, rect);
}

FoldCreaseRegion SingleDisplayFoldPolicy::GetFoldCreaseRegion(bool isVertical) const
{
    std::vector<int32_t> foldRect = FoldScreenStateInternel::StringFoldRectSplitToInt(g_FoldScreenRect,
        FOLD_CREASE_DELIMITER);
    if (foldRect.size() != FOLD_CREASE_RECT_SIZE) {
        TLOGE(WmsLogTag::DMS, "foldRect is invalid");
        return FoldCreaseRegion(0, {});
    }

    ScreenId screenIdFull = 0;
    std::vector<DMRect> foldCreaseRect;
    GetFoldCreaseRect(isVertical, foldRect, foldCreaseRect);
    return FoldCreaseRegion(screenIdFull, foldCreaseRect);
}

void SingleDisplayFoldPolicy::GetFoldCreaseRect(bool isVertical,
    const std::vector<int32_t>& foldRect, std::vector<DMRect>& foldCreaseRect) const
{
    int32_t liveCreaseRegionPosX; // live Crease Region PosX
    int32_t liveCreaseRegionPosY; // live Crease Region PosY
    uint32_t liveCreaseRegionPosWidth; // live Crease Region PosWidth
    uint32_t liveCreaseRegionPosHeight; // live Crease Region PosHeight
    if (isVertical) {
        TLOGI(WmsLogTag::DMS, "the current FoldCreaseRect is vertical");
        liveCreaseRegionPosX = foldRect[0];
        liveCreaseRegionPosY = foldRect[1];
        liveCreaseRegionPosWidth = static_cast<uint32_t>(foldRect[2]);
        liveCreaseRegionPosHeight = static_cast<uint32_t>(foldRect[3]);
    } else {
        TLOGI(WmsLogTag::DMS, "the current FoldCreaseRect is horizontal");
        liveCreaseRegionPosX = foldRect[1];
        liveCreaseRegionPosY = foldRect[0];
        liveCreaseRegionPosWidth = static_cast<uint32_t>(foldRect[3]);
        liveCreaseRegionPosHeight = static_cast<uint32_t>(foldRect[2]);
    }
    foldCreaseRect = {
        {
            liveCreaseRegionPosX, liveCreaseRegionPosY,
            liveCreaseRegionPosWidth, liveCreaseRegionPosHeight
        }
    };
    return;
}

void SingleDisplayFoldPolicy::SetdisplayModeChangeStatus(bool status, bool isOnBootAnimation)
{
    if (status) {
        pengdingTask_ = isOnBootAnimation ? FOLD_TO_EXPAND_ONBOOTANIMATION_TASK_NUM : FOLD_TO_EXPAND_TASK_NUM;
        startTimePoint_ = std::chrono::steady_clock::now();
        displayModeChangeRunning_ = status;
    } else {
        pengdingTask_ --;
        if (pengdingTask_ != 0) {
            return;
        }
        displayModeChangeRunning_ = false;
        endTimePoint_ = std::chrono::steady_clock::now();
        if (lastCachedisplayMode_.load() != GetScreenDisplayMode()) {
            ScreenSessionManager::GetInstance().TriggerDisplayModeUpdate(lastCachedisplayMode_.load());
        }
    }
}

bool SingleDisplayFoldPolicy::CheckDisplayModeChange(FoldDisplayMode displayMode, bool isForce,
    DisplayModeChangeReason reason)
{
    if (isForce) {
        TLOGI(WmsLogTag::DMS, "force change displayMode");
        return true;
    }
    if (GetPhysicalFoldLockFlag() && reason != DisplayModeChangeReason::FORCE_SET) {
        TLOGI(WmsLogTag::DMS, "Fold status is locked, can't change to display mode: %{public}d", displayMode);
        return false;
    }
    if (isClearingBootAnimation_) {
        TLOGI(WmsLogTag::DMS, "clearing bootAnimation not change displayMode");
        return false;
    }
    SetLastCacheDisplayMode(displayMode);
    if (GetModeChangeRunningStatus()) {
        TLOGW(WmsLogTag::DMS, "last process not complete, skip mode: %{public}d", displayMode);
        return false;
    }
    TLOGI(WmsLogTag::DMS, "start change displaymode: %{public}d, lastElapsedMs: %{public}" PRId64 "ms",
        displayMode, getFoldingElapsedMs());

    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:ChangeScreenDisplayMode(displayMode = %" PRIu64")", displayMode);
    {
        std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
        if (currentDisplayMode_ == displayMode) {
            TLOGW(WmsLogTag::DMS, "ChangeScreenDisplayMode already in displayMode %{public}d", displayMode);
            return false;
        }
    }
    return true;
}

void SingleDisplayFoldPolicy::ChangeScreenDisplayMode(FoldDisplayMode displayMode, DisplayModeChangeReason reason)
{
    if (!CheckDisplayModeChange(displayMode, false, reason)) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "start change displaymode: %{public}d, reason: %{public}d", displayMode, reason);
    ChangeScreenDisplayModeInner(displayMode, reason);
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
    ScreenSessionManager::GetInstance().SwitchScrollParam(displayMode);
    TLOGD(WmsLogTag::DMS, "end change displaymode: %{public}d, reason: %{public}d", displayMode, reason);
    return;
}

void SingleDisplayFoldPolicy::ChangeScreenDisplayMode(FoldDisplayMode displayMode, bool isForce,
    DisplayModeChangeReason reason)
{
    if (!CheckDisplayModeChange(displayMode, isForce)) {
        return;
    }
    ChangeScreenDisplayModeInner(displayMode, reason);
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
    ScreenSessionManager::GetInstance().SwitchScrollParam(displayMode);
}

void SingleDisplayFoldPolicy::ChangeScreenDisplayModeInner(FoldDisplayMode displayMode, DisplayModeChangeReason reason)
{
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_FULL);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "default screenSession is null");
        return;
    }
    SetdisplayModeChangeStatus(true);
    {
        std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
        lastDisplayMode_ = displayMode;
    }
    ReportFoldDisplayModeChange(displayMode);
    switch (displayMode) {
        case FoldDisplayMode::MAIN: {
            ChangeScreenDisplayModeToMain(screenSession, reason);
            break;
        }
        case FoldDisplayMode::FULL: {
            ChangeScreenDisplayModeToFull(screenSession, reason);
            break;
        }
        case FoldDisplayMode::UNKNOWN: {
            TLOGI(WmsLogTag::DMS, "displayMode is unknown");
            break;
        }
        default: {
            TLOGI(WmsLogTag::DMS, "displayMode is invalid");
            break;
        }
    }
    {
        std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
        currentDisplayMode_ = displayMode;
    }
}

void SingleDisplayFoldPolicy::SendSensorResult(FoldStatus foldStatus)
{
    TLOGI(WmsLogTag::DMS, "SendSensorResult FoldStatus: %{public}d", foldStatus);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    ChangeScreenDisplayMode(displayMode);
}

sptr<FoldCreaseRegion> SingleDisplayFoldPolicy::GetCurrentFoldCreaseRegion()
{
    TLOGI(WmsLogTag::DMS, "GetCurrentFoldCreaseRegion");
    return currentFoldCreaseRegion_;
}

FoldCreaseRegion SingleDisplayFoldPolicy::GetLiveCreaseRegion()
{
    TLOGI(WmsLogTag::DMS, "enter");
    std::lock_guard<std::mutex> lock_mode(liveCreaseRegionMutex_);
    FoldDisplayMode displayMode = GetScreenDisplayMode();
    if (displayMode == FoldDisplayMode::UNKNOWN || displayMode == FoldDisplayMode::MAIN) {
        return FoldCreaseRegion(0, {});
    }
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_FULL);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "default screenSession is null");
        return FoldCreaseRegion(0, {});
    }
    DisplayOrientation displayOrientation = screenSession->GetScreenProperty().GetDisplayOrientation();
    if (displayMode == FoldDisplayMode::FULL) {
        switch (displayOrientation) {
            case DisplayOrientation::PORTRAIT:
            case DisplayOrientation::PORTRAIT_INVERTED: {
                liveCreaseRegion_ = GetFoldCreaseRegion(true);
                break;
            }
            case DisplayOrientation::LANDSCAPE:
            case DisplayOrientation::LANDSCAPE_INVERTED: {
                liveCreaseRegion_ = GetFoldCreaseRegion(false);
                break;
            }
            default: {
                TLOGE(WmsLogTag::DMS, "displayOrientation is invalid");
            }
        }
    }
    return liveCreaseRegion_;
}

void SingleDisplayFoldPolicy::GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems) const
{
    FoldCreaseRegionItem MCreaseItem{DisplayOrientation::LANDSCAPE, FoldDisplayMode::MAIN,
        FoldCreaseRegion(0, {})};
    FoldCreaseRegionItem FPorCreaseItem{DisplayOrientation::PORTRAIT, FoldDisplayMode::FULL,
        GetFoldCreaseRegion(true)};
    FoldCreaseRegionItem FLandCreaseItem{DisplayOrientation::LANDSCAPE, FoldDisplayMode::FULL,
        GetFoldCreaseRegion(false)};
    foldCreaseRegionItems.push_back(MCreaseItem);
    foldCreaseRegionItems.push_back(FPorCreaseItem);
    foldCreaseRegionItems.push_back(FLandCreaseItem);
}

void SingleDisplayFoldPolicy::LockDisplayStatus(bool locked)
{
    TLOGI(WmsLogTag::DMS, "LockDisplayStatus locked: %{public}d", locked);
    lockDisplayStatus_ = locked;
}

void SingleDisplayFoldPolicy::SetOnBootAnimation(bool onBootAnimation)
{
    TLOGI(WmsLogTag::DMS, "onBootAnimation: %{public}d", onBootAnimation);
    onBootAnimation_ = onBootAnimation;
    if (!onBootAnimation_) {
        TLOGI(WmsLogTag::DMS, "when boot animation finished, change display mode");
        RecoverWhenBootAnimationExit();
    }
}

void SingleDisplayFoldPolicy::RecoverWhenBootAnimationExit()
{
    TLOGI(WmsLogTag::DMS, "CurrentScreen(%{public}" PRIu64 ")", screenId_);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    ChangeScreenDisplayMode(displayMode);
}

void SingleDisplayFoldPolicy::UpdateForPhyScreenPropertyChange()
{
    TLOGI(WmsLogTag::DMS, "CurrentScreen(%{public}" PRIu64 ")", screenId_);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    if (currentDisplayMode_ != displayMode) {
        ChangeScreenDisplayMode(displayMode);
    }
}

FoldDisplayMode SingleDisplayFoldPolicy::GetModeMatchStatus()
{
    return GetModeMatchStatus(currentFoldStatus_);
}

FoldDisplayMode SingleDisplayFoldPolicy::GetModeMatchStatus(FoldStatus targetFoldStatus)
{
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    switch (targetFoldStatus) {
        case FoldStatus::EXPAND: {
            displayMode = FoldDisplayMode::FULL;
            break;
        }
        case FoldStatus::FOLDED: {
            displayMode = FoldDisplayMode::MAIN;
            break;
        }
        case FoldStatus::HALF_FOLD: {
            displayMode = FoldDisplayMode::FULL;
            break;
        }
        default: {
            TLOGI(WmsLogTag::DMS, "GetModeMatchStatus FoldStatus is invalid");
        }
    }
    return displayMode;
}

void SingleDisplayFoldPolicy::ReportFoldDisplayModeChange(FoldDisplayMode displayMode)
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

void SingleDisplayFoldPolicy::ReportFoldStatusChangeBegin(int32_t offScreen, int32_t onScreen)
{
    TLOGI(WmsLogTag::DMS, "offScreen: %{public}d, onScreen: %{public}d",
        offScreen, onScreen);
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "FOLD_STATE_CHANGE_BEGIN",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "POWER_OFF_SCREEN", offScreen,
        "POWER_ON_SCREEN", onScreen);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "Write HiSysEvent error, ret: %{public}d", ret);
    }
}

void SingleDisplayFoldPolicy::ChangeScreenDisplayModeToMainWhenFoldScreenOn(sptr<ScreenSession> screenSession)
{
    TLOGI(WmsLogTag::DMS, "IsFoldScreenOn is true, begin.");
    ReportFoldStatusChangeBegin(static_cast<int32_t>(SCREEN_ID_FULL),
        static_cast<int32_t>(SCREEN_ID_MAIN));
    auto taskScreenOnMain = [=] {
        // off full screen
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayModeToMain: IsFoldScreenOn is true, screenIdFull OFF.");
        screenId_ = SCREEN_ID_FULL;
        ChangeScreenDisplayModePower(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF);
        SetdisplayModeChangeStatus(false);

        // on main screen
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayModeToMain: IsFoldScreenOn is true, screenIdMain ON.");
        screenId_ = SCREEN_ID_MAIN;
        ChangeScreenDisplayModePower(SCREEN_ID_MAIN, ScreenPowerStatus::POWER_STATUS_ON);
        SetdisplayModeChangeStatus(false);
    };
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOnMain, "screenOnMainTask");
    SendPropertyChangeResult(screenSession, SCREEN_ID_MAIN, ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING);
}

void SingleDisplayFoldPolicy::ChangeScreenDisplayModeToMainWhenFoldScreenOff(sptr<ScreenSession> screenSession)
{
    TLOGI(WmsLogTag::DMS, "IsFoldScreenOn is false, begin.");
    // off full screen
    auto taskScreenOffMainOff = [=] {
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayModeToMain: IsFoldScreenOn is false, screenIdFull OFF.");
        screenId_ = SCREEN_ID_FULL;
        ChangeScreenDisplayModePower(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF);
        SetdisplayModeChangeStatus(false);
    };
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOffMainOff, "screenOffMainOffTask");
    SendPropertyChangeResult(screenSession, SCREEN_ID_MAIN, ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING);
    auto taskScreenOnMainChangeScreenId = [=] {
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayModeToMain: IsFoldScreenOn is false, Change ScreenId to Main.");
        screenId_ = SCREEN_ID_MAIN;
#ifdef TP_FEATURE_ENABLE
        RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE_POWER_CTRL, MAIN_TP_OFF.c_str());
#endif
        SetdisplayModeChangeStatus(false);
    };
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOnMainChangeScreenId, "taskScreenOnMainChangeScreenId");
}

void SingleDisplayFoldPolicy::ChangeScreenDisplayModeToMain(sptr<ScreenSession> screenSession,
    DisplayModeChangeReason reason)
{
    if (onBootAnimation_) {
        SetdisplayModeChangeStatus(true, true);
        ChangeScreenDisplayModeToMainOnBootAnimation(screenSession);
        return;
    }
    RSInterfaces::GetInstance().NotifyScreenSwitched();
#ifdef TP_FEATURE_ENABLE
    RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE, MAIN_TP.c_str());
#endif
    if (PowerMgr::PowerMgrClient::GetInstance().IsFoldScreenOn() ||
        ScreenSessionManager::GetInstance().GetCancelSuspendStatus()) {
        ChangeScreenDisplayModeToMainWhenFoldScreenOn(screenSession);
    } else { // When the screen is off and folded, it is not powered on
        ScreenSessionManager::GetInstance().ForceSkipScreenOffAnimation();
        ChangeScreenDisplayModeToMainWhenFoldScreenOff(screenSession);
    }
}

void SingleDisplayFoldPolicy::ChangeScreenDisplayModeToFullWhenFoldScreenOn(sptr<ScreenSession> screenSession)
{
    TLOGI(WmsLogTag::DMS, "IsFoldScreenOn is true, begin.");
    auto taskScreenOnFull = [=] {
        // off main screen
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayModeToFull: IsFoldScreenOn is true, screenIdMain OFF.");
        screenId_ = SCREEN_ID_MAIN;
        ChangeScreenDisplayModePower(SCREEN_ID_MAIN, ScreenPowerStatus::POWER_STATUS_OFF);
        SetdisplayModeChangeStatus(false);

        // on full screen
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayModeToFull: IsFoldScreenOn is true, screenIdFull ON.");
        screenId_ = SCREEN_ID_FULL;
        ChangeScreenDisplayModePower(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_ON);
        SetdisplayModeChangeStatus(false);
    };
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOnFull, "screenOnFullTask");
    SendPropertyChangeResult(screenSession, SCREEN_ID_FULL, ScreenPropertyChangeReason::FOLD_SCREEN_EXPAND);
}

void SingleDisplayFoldPolicy::ChangeScreenDisplayModeToFullWhenFoldScreenOff(sptr<ScreenSession> screenSession,
    DisplayModeChangeReason reason)
{
    TLOGI(WmsLogTag::DMS, "IsFoldScreenOn is false, begin.");
    // off main screen
    auto taskScreenOffFullOff = [=] {
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayModeToFull: IsFoldScreenOn is false, screenIdMain OFF.");
        screenId_ = SCREEN_ID_MAIN;
        ChangeScreenDisplayModePower(SCREEN_ID_MAIN, ScreenPowerStatus::POWER_STATUS_OFF);
        SetdisplayModeChangeStatus(false);
    };
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOffFullOff, "screenOffFullOffTask");
    SendPropertyChangeResult(screenSession, SCREEN_ID_FULL, ScreenPropertyChangeReason::FOLD_SCREEN_EXPAND);
    // on full screen
    auto taskScreenOnFullOn = [=] {
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayModeToFull: IsFoldScreenOn is false, screenIdFull ON.");
        screenId_ = SCREEN_ID_FULL;
        if (reason == DisplayModeChangeReason::RECOVER) {
#ifdef TP_FEATURE_ENABLE
            RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE_POWER_CTRL, FULL_TP_OFF.c_str());
#endif
        } else {
            PowerMgr::PowerMgrClient::GetInstance().WakeupDeviceAsync();
        }
        SetdisplayModeChangeStatus(false);
    };
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOnFullOn, "screenOnFullOnTask");
}

void SingleDisplayFoldPolicy::ChangeScreenDisplayModeToFull(sptr<ScreenSession> screenSession,
    DisplayModeChangeReason reason)
{
    if (onBootAnimation_) {
        SetdisplayModeChangeStatus(true, true);
        ChangeScreenDisplayModeToFullOnBootAnimation(screenSession);
        return;
    }
    RSInterfaces::GetInstance().NotifyScreenSwitched();
    ReportFoldStatusChangeBegin((int32_t)SCREEN_ID_MAIN, (int32_t)SCREEN_ID_FULL);
#ifdef TP_FEATURE_ENABLE
    RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE, FULL_TP.c_str());
#endif
    if (PowerMgr::PowerMgrClient::GetInstance().IsFoldScreenOn()) {
        ChangeScreenDisplayModeToFullWhenFoldScreenOn(screenSession);
    } else { //AOD scene
        if (ScreenSessionManager::GetInstance().TryToCancelScreenOff()) {
            ChangeScreenDisplayModeToFullWhenFoldScreenOn(screenSession);
        } else {
            ChangeScreenDisplayModeToFullWhenFoldScreenOff(screenSession, reason);
        }
    }
}

void SingleDisplayFoldPolicy::ChangeScreenDisplayModePower(ScreenId screenId, ScreenPowerStatus screenPowerStatus)
{
    ScreenSessionManager::GetInstance().SetKeyguardDrawnDoneFlag(false);
    ScreenSessionManager::GetInstance().SetScreenPowerForFold(screenId, screenPowerStatus);
}

void SingleDisplayFoldPolicy::SendPropertyChangeResult(sptr<ScreenSession> screenSession, ScreenId screenId,
    ScreenPropertyChangeReason reason)
{
    std::lock_guard<std::recursive_mutex> lock_info(displayInfoMutex_);
    screenProperty_ = ScreenSessionManager::GetInstance().GetPhyScreenProperty(screenId);
    screenSession->SetPhyScreenId(screenId);
    if (!ScreenSessionManager::GetInstance().GetClientProxy()) {
        ScreenProperty property = screenSession->UpdatePropertyByFoldControl(screenProperty_);

        screenSession->PropertyChange(property, reason);

        // To avoid the app get an incorrect and unready property after a display mode change notification, we
        // temporarily modify the rotation value. The rotation will be corrected to its accurate value after SCB's
        // property change.
        screenSession->SetRotationAndScreenRotationOnly(Rotation::ROTATION_0);
        TLOGI(WmsLogTag::DMS, "screenBounds : width_= %{public}f, height_= %{public}f",
            screenSession->GetScreenProperty().GetBounds().rect_.width_,
            screenSession->GetScreenProperty().GetBounds().rect_.height_);
        ScreenSessionManager::GetInstance().NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(),
            DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    } else {
        screenSession->NotifyFoldPropertyChange(screenProperty_, reason, GetScreenDisplayMode());
    }
}

void SingleDisplayFoldPolicy::ChangeScreenDisplayModeToMainOnBootAnimation(sptr<ScreenSession> screenSession)
{
    TLOGI(WmsLogTag::DMS, "enter!");
    screenProperty_ = ScreenSessionManager::GetInstance().GetPhyScreenProperty(SCREEN_ID_MAIN);
    screenSession->UpdatePropertyByFoldControl(screenProperty_);
    screenSession->PropertyChange(screenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING);
    TLOGI(WmsLogTag::DMS, "screenBounds : width_= %{public}f, height_= %{public}f",
        screenSession->GetScreenProperty().GetBounds().rect_.width_,
        screenSession->GetScreenProperty().GetBounds().rect_.height_);
    screenId_ = SCREEN_ID_MAIN;
}

void SingleDisplayFoldPolicy::ChangeScreenDisplayModeToFullOnBootAnimation(sptr<ScreenSession> screenSession)
{
    TLOGI(WmsLogTag::DMS, "enter!");
    screenProperty_ = ScreenSessionManager::GetInstance().GetPhyScreenProperty(SCREEN_ID_FULL);
    screenSession->UpdatePropertyByFoldControl(screenProperty_);
    screenSession->PropertyChange(screenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::FOLD_SCREEN_EXPAND);
    TLOGI(WmsLogTag::DMS, "screenBounds : width_= %{public}f, height_= %{public}f",
        screenSession->GetScreenProperty().GetBounds().rect_.width_,
        screenSession->GetScreenProperty().GetBounds().rect_.height_);
    screenId_ = SCREEN_ID_FULL;
}

void SingleDisplayFoldPolicy::SetIsClearingBootAnimation(bool isClearingBootAnimation)
{
    TLOGI(WmsLogTag::DMS, "isClearingBootAnimation: %{public}d", isClearingBootAnimation);
    isClearingBootAnimation_ = isClearingBootAnimation;
}
} // namespace OHOS::Rosen