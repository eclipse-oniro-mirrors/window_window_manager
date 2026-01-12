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

#include "fold_screen_base_policy.h"
#include <parameters.h>

#include <hisysevent.h>
#include <hitrace_meter.h>
#include <transaction/rs_interfaces.h>
#include "fold_screen_state_internel.h"
#include "fold_crease_region_controller.h"
#include "screen_session_manager.h"
#include "window_manager_hilog.h"
#include "rs_adapter.h"
#include "sensor_agent.h"
#include "sensor_agent_type.h"

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

namespace OHOS::Rosen::DMS {
namespace {
    const uint32_t MODE_CHANGE_TIMEOUT_MS = 2000;
    const ScreenId SCREEN_ID_FULL = 0;
    const ScreenId SCREEN_ID_MAIN = 5;
    const int32_t REMOVE_DISPLAY_NODE = 0;
    const int32_t ADD_DISPLAY_NODE = 1;

#ifdef TP_FEATURE_ENABLE
    const int32_t TP_TYPE = 12;
    const int32_t TP_TYPE_POWER_CTRL = 18;
    const std::string FULL_TP = "0";
    const std::string MAIN_TP = "1";
    const std::string MAIN_TP_OFF = "1,1";
    const std::string FULL_TP_OFF = "0,1";
#endif
    const std::unordered_set<FoldStatus> SUPPORTED_FOLD_STATUS = {
        FoldStatus::EXPAND,
        FoldStatus::FOLDED,
        FoldStatus::HALF_FOLD
    };
    static const std::map<FoldDisplayMode, DMDeviceStatus> DISPLAYMODE_DEVICESTATUS_MAPPING = {
        {FoldDisplayMode::MAIN, DMDeviceStatus::STATUS_FOLDED},
        {FoldDisplayMode::FULL, DMDeviceStatus::STATUS_EXPAND},
        {FoldDisplayMode::GLOBAL_FULL, DMDeviceStatus::STATUS_GLOBAL_FULL}
    };
} // namespace

FoldScreenBasePolicy::FoldScreenBasePolicy() {}

FoldScreenBasePolicy::~FoldScreenBasePolicy() = default;

FoldScreenBasePolicy& FoldScreenBasePolicy::GetInstance()
{
    static std::mutex singletonMutex_;
    static FoldScreenBasePolicy* instance_ = nullptr;
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(singletonMutex_);
        if (instance_ == nullptr) {
            instance_ = new FoldScreenBasePolicy();
        }
    }
    return *instance_;
}

FoldDisplayMode FoldScreenBasePolicy::GetScreenDisplayMode()
{
    std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
    return lastDisplayMode_;
}

void FoldScreenBasePolicy::LockDisplayStatus(bool locked)
{
    TLOGI(WmsLogTag::DMS, "LockDisplayStatus locked: %{public}d", locked);
    lockDisplayStatus_ = locked;
}

FoldStatus FoldScreenBasePolicy::GetFoldStatus()
{
    if (!GetPhysicalFoldLockFlag()) {
        return lastFoldStatus_;
    }
    return GetForceFoldStatus();
}

void FoldScreenBasePolicy::SetFoldStatus(FoldStatus foldStatus)
{
    TLOGI(WmsLogTag::DMS, "SetFoldStatus FoldStatus: %{public}d", foldStatus);
    currentFoldStatus_ = foldStatus;
    lastFoldStatus_ = foldStatus;
}

ScreenId FoldScreenBasePolicy::GetCurrentScreenId() { return screenId_; }

std::chrono::steady_clock::time_point FoldScreenBasePolicy::GetStartTimePoint()
{
    return startTimePoint_;
}

bool FoldScreenBasePolicy::GetIsFirstFrameCommitReported()
{
    return isFirstFrameCommitReported_;
}

void FoldScreenBasePolicy::SetIsFirstFrameCommitReported(bool isFirstFrameCommitReported)
{
    isFirstFrameCommitReported_ = isFirstFrameCommitReported;
}

void FoldScreenBasePolicy::ClearState()
{
    currentDisplayMode_ = FoldDisplayMode::UNKNOWN;
    currentFoldStatus_ = FoldStatus::UNKNOWN;
}

bool FoldScreenBasePolicy::GetModeChangeRunningStatus()
{
    auto currentTime = std::chrono::steady_clock::now();
    auto intervalMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTimePoint_).count();
    if (intervalMs > MODE_CHANGE_TIMEOUT_MS) {
        TLOGE(WmsLogTag::DMS, "mode change timeout.");
        return false;
    }
    return GetdisplayModeRunningStatus();
}

bool FoldScreenBasePolicy::GetdisplayModeRunningStatus()
{
    return displayModeChangeRunning_.load();
}

FoldDisplayMode FoldScreenBasePolicy::GetLastCacheDisplayMode()
{
    return lastCachedisplayMode_.load();
}

void FoldScreenBasePolicy::SetLastCacheDisplayMode(FoldDisplayMode mode)
{
    lastCachedisplayMode_ = mode;
}

int64_t FoldScreenBasePolicy::getFoldingElapsedMs()
{
    if (endTimePoint_ < startTimePoint_) {
        TLOGE(WmsLogTag::DMS, "invalid timepoint. endTimePoint less startTimePoint");
        return 0;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTimePoint_ - startTimePoint_).count();
    return static_cast<int64_t>(elapsed);
}

Drawing::Rect FoldScreenBasePolicy::GetScreenSnapshotRect()
{
    Drawing::Rect snapshotRect = {0, 0, 0, 0};
    return snapshotRect;
}

/**
 * tentMode start
 */
void FoldScreenBasePolicy::ChangeOnTentMode(FoldStatus currentState)
{
    TLOGI(WmsLogTag::DMS, "Enter tent mode, current state:%{public}d, change display mode to MAIN", currentState);
    if (currentState == FoldStatus::EXPAND || currentState == FoldStatus::HALF_FOLD) {
        ChangeScreenDisplayMode(FoldDisplayMode::MAIN);
    } else if (currentState == FoldStatus::FOLDED) {
        ChangeScreenDisplayMode(FoldDisplayMode::MAIN);
        PowerMgr::PowerMgrClient::GetInstance().WakeupDeviceAsync();
    } else {
        TLOGE(WmsLogTag::DMS, "current state:%{public}d invalid", currentState);
    }
}

void FoldScreenBasePolicy::ChangeOffTentMode()
{
    PowerMgr::PowerMgrClient::GetInstance().WakeupDeviceAsync();
    FoldDisplayMode displayMode = GetModeMatchStatus();
    TLOGW(WmsLogTag::DMS, "CurrentDisplayMode:%{public}d, CurrentFoldStatus:%{public}d",
        currentDisplayMode_, currentFoldStatus_);
    ChangeScreenDisplayMode(displayMode);
}
/**
 * tentMode end
 */
 
 /**
 * coordination start
 */
 void FoldScreenBasePolicy::ChangeScreenDisplayModeToCoordination()
{
    std::lock_guard<std::mutex> lock(coordinationMutex_);
    if (ScreenSessionManager::GetInstance().GetCoordinationFlag()) {
        TLOGW(WmsLogTag::DMS, "change displaymode to coordination skipped, current coordination flag is true");
        return;
    }
    TLOGI(WmsLogTag::DMS, "change displaymode to coordination current mode=%{public}d", currentDisplayMode_);
    ScreenSessionManager::GetInstance().NotifyRSCoordination(true);
    ScreenSessionManager::GetInstance().SetCoordinationFlag(true);
    ScreenSessionManager::GetInstance().OnScreenChange(SCREEN_ID_MAIN, ScreenEvent::CONNECTED);

    // on main screen
    auto taskScreenOnMainOn = [=] {
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayModeToCoordination: screenIdMain ON.");
        NotifyRefreshRateEvent(true);
        ScreenSessionManager::GetInstance().SetKeyguardDrawnDoneFlag(false);
        ScreenSessionManager::GetInstance().SetScreenPowerForFold(SCREEN_ID_MAIN,
            ScreenPowerStatus::POWER_STATUS_ON);
        PowerMgr::PowerMgrClient::GetInstance().RefreshActivity();
    };
    ScreenSessionManager::GetInstance().GetScreenPowerTaskScheduler()->
        PostAsyncTask(taskScreenOnMainOn, "ScreenToCoordinationTask");
    AddOrRemoveDisplayNodeToTree(SCREEN_ID_MAIN, ADD_DISPLAY_NODE);
}

void FoldScreenBasePolicy::CloseCoordinationScreen()
{
    std::lock_guard<std::mutex> lock(coordinationMutex_);
    if (!ScreenSessionManager::GetInstance().GetCoordinationFlag()) {
        TLOGW(WmsLogTag::DMS, "CloseCoordinationScreen skipped, current coordination flag is false");
        return;
    }
    TLOGI(WmsLogTag::DMS, "Close Coordination Screen current mode=%{public}d", currentDisplayMode_);
    ScreenSessionManager::GetInstance().NotifyRSCoordination(false);
    // on main screen
    auto taskScreenOnMainOFF = [=] {
        TLOGNI(WmsLogTag::DMS, "CloseCoordinationScreen: screenIdMain OFF.");
        ScreenSessionManager::GetInstance().SetKeyguardDrawnDoneFlag(false);
        ScreenSessionManager::GetInstance().SetScreenPowerForFold(SCREEN_ID_MAIN,
            ScreenPowerStatus::POWER_STATUS_OFF);
        NotifyRefreshRateEvent(false);
    };
    ScreenSessionManager::GetInstance().GetScreenPowerTaskScheduler()->
        PostAsyncTask(taskScreenOnMainOFF, "CloseCoordinationScreenTask");
    AddOrRemoveDisplayNodeToTree(SCREEN_ID_MAIN, REMOVE_DISPLAY_NODE);

    ScreenSessionManager::GetInstance().OnScreenChange(SCREEN_ID_MAIN, ScreenEvent::DISCONNECTED);
    ScreenSessionManager::GetInstance().SetCoordinationFlag(false);
}

void FoldScreenBasePolicy::ExitCoordination()
{
    std::lock_guard<std::mutex> lock(coordinationMutex_);
    if (!ScreenSessionManager::GetInstance().GetCoordinationFlag()) {
        TLOGW(WmsLogTag::DMS, "ExitCoordination skipped, current coordination flag is false");
        return;
    }
    ScreenSessionManager::GetInstance().NotifyRSCoordination(false);
    ScreenSessionManager::GetInstance().SetKeyguardDrawnDoneFlag(false);
    ScreenSessionManager::GetInstance().SetRSScreenPowerStatusExt(SCREEN_ID_MAIN,
        ScreenPowerStatus::POWER_STATUS_OFF);
    AddOrRemoveDisplayNodeToTree(SCREEN_ID_MAIN, REMOVE_DISPLAY_NODE);
    ScreenSessionManager::GetInstance().OnScreenChange(SCREEN_ID_MAIN, ScreenEvent::DISCONNECTED);
    ScreenSessionManager::GetInstance().SetCoordinationFlag(false);
    NotifyRefreshRateEvent(false);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    {
            std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
            currentDisplayMode_ = displayMode;
            lastDisplayMode_ = displayMode;
    }
    TLOGI(WmsLogTag::DMS, "Exit coordination, current display mode:%{public}d", displayMode);
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
}

void FoldScreenBasePolicy::NotifyRefreshRateEvent(bool isEventStatus)
{
    EventInfo eventInfo = {
        .eventName = "VOTER_MULTISELFOWNEDSCREEN",
        .eventStatus = isEventStatus,
        .minRefreshRate = 60,
        .maxRefreshRate = 60,
    };
    RSInterfaces::GetInstance().NotifyRefreshRateEvent(eventInfo);
}

void FoldScreenBasePolicy::AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 ", command: %{public}d",
        screenId, command);
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    std::shared_ptr<RSDisplayNode> displayNode = screenSession->GetDisplayNode();
    if (displayNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode is null");
        return;
    }
    if (command == ADD_DISPLAY_NODE) {
        displayNode->AddDisplayNodeToTree();
    } else if (command == REMOVE_DISPLAY_NODE) {
        displayNode->RemoveDisplayNodeFromTree();
    }
    displayNode = nullptr;
    TLOGI(WmsLogTag::DMS, "add or remove displayNode");
    RSTransactionAdapter::FlushImplicitTransaction(screenSession->GetRSUIContext());
}
/**
 * coordination end
 */

/**
 * report Fold start
 */

 void FoldScreenBasePolicy::ReportFoldDisplayModeChange(FoldDisplayMode displayMode)
{
    int32_t mode = static_cast<int32_t>(displayMode);
    TLOGI(WmsLogTag::DMS, "ReportFoldDisplayModeChange displayMode: %{public}d", mode);
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "DISPLAY_MODE",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "FOLD_DISPLAY_MODE", mode);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "ReportFoldDisplayModeChange Write HiSysEvent error, ret: %{public}d", ret);
    }
}

void FoldScreenBasePolicy::ReportFoldStatusChangeBegin(int32_t offScreen, int32_t onScreen)
{
    TLOGI(WmsLogTag::DMS, "ReportFoldStatusChangeBegin offScreen: %{public}d, onScreen: %{public}d",
        offScreen, onScreen);
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "FOLD_STATE_CHANGE_BEGIN",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "POWER_OFF_SCREEN", offScreen,
        "POWER_ON_SCREEN", onScreen);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "ReportFoldStatusChangeBegin Write HiSysEvent error, ret: %{public}d", ret);
    }
}
/**
 * report Fold end
 */

 /**
  * bootAnimate start
  */
void FoldScreenBasePolicy::SetOnBootAnimation(bool onBootAnimation)
{
    TLOGI(WmsLogTag::DMS, "SetOnBootAnimation onBootAnimation: %{public}d", onBootAnimation);
    onBootAnimation_ = onBootAnimation;
    if (!onBootAnimation_) {
        TLOGI(WmsLogTag::DMS, "SetOnBootAnimation when boot animation finished, change display mode");
        RecoverWhenBootAnimationExit();
    }
}

void FoldScreenBasePolicy::RecoverWhenBootAnimationExit()
{
    TLOGI(WmsLogTag::DMS, "CurrentScreen(%{public}" PRIu64 ")", screenId_);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    ChangeScreenDisplayMode(displayMode, true);
}

void FoldScreenBasePolicy::UpdateForPhyScreenPropertyChange()
{
    TLOGI(WmsLogTag::DMS, "CurrentScreen(%{public}" PRIu64 ")", screenId_);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    if (currentDisplayMode_ != displayMode) {
        ChangeScreenDisplayMode(displayMode);
    }
}

void FoldScreenBasePolicy::ChangeScreenDisplayModeToMainOnBootAnimation(sptr<ScreenSession> screenSession)
{
    TLOGI(WmsLogTag::DMS, "ChangeScreenDisplayModeToMainOnBootAnimation");
    screenProperty_ = ScreenSessionManager::GetInstance().GetPhyScreenProperty(SCREEN_ID_MAIN);
    screenSession->UpdatePropertyByFoldControl(screenProperty_);
    screenSession->SetValidWidth(screenProperty_.GetBounds().rect_.width_);
    screenSession->SetValidHeight(screenProperty_.GetBounds().rect_.height_);
    screenSession->PropertyChange(screenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING);
    TLOGI(WmsLogTag::DMS, "screenBounds : width_= %{public}f, height_= %{public}f",
        screenSession->GetScreenProperty().GetBounds().rect_.width_,
        screenSession->GetScreenProperty().GetBounds().rect_.height_);
    screenId_ = SCREEN_ID_MAIN;
}

void FoldScreenBasePolicy::ChangeScreenDisplayModeToFullOnBootAnimation(sptr<ScreenSession> screenSession)
{
    TLOGI(WmsLogTag::DMS, "ChangeScreenDisplayModeToFullOnBootAnimation");
    screenProperty_ = ScreenSessionManager::GetInstance().GetPhyScreenProperty(SCREEN_ID_FULL);
    screenSession->UpdatePropertyByFoldControl(screenProperty_);
    screenSession->SetValidWidth(screenProperty_.GetBounds().rect_.width_);
    screenSession->SetValidHeight(screenProperty_.GetBounds().rect_.height_);
    screenSession->PropertyChange(screenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::FOLD_SCREEN_EXPAND);
    TLOGI(WmsLogTag::DMS, "screenBounds : width_= %{public}f, height_= %{public}f",
        screenSession->GetScreenProperty().GetBounds().rect_.width_,
        screenSession->GetScreenProperty().GetBounds().rect_.height_);
    screenId_ = SCREEN_ID_FULL;
}

void FoldScreenBasePolicy::SetIsClearingBootAnimation(bool isClearingBootAnimation)
{
    TLOGI(WmsLogTag::DMS, "isClearingBootAnimation: %{public}d", isClearingBootAnimation);
    isClearingBootAnimation_ = isClearingBootAnimation;
}
/**
  * bootAnimate end
  */

/**
  * fold or expand start
  */
bool FoldScreenBasePolicy::CheckDisplayModeChange(FoldDisplayMode displayMode, bool isForce,
    DisplayModeChangeReason reason)
{
    if (isForce) {
        TLOGI(WmsLogTag::DMS, "force change displayMode");
        SetLastCacheDisplayMode(displayMode);
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

void FoldScreenBasePolicy::ChangeScreenDisplayMode(FoldDisplayMode displayMode, DisplayModeChangeReason reason)
{
    if (!CheckDisplayModeChange(displayMode, false, reason)) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "start change displaymode: %{public}d, reason: %{public}d", displayMode, reason);
    ChangeScreenDisplayModeInner(displayMode, reason);
    UpdateDeviceStatus(displayMode);
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
    ScreenSessionManager::GetInstance().SwitchScrollParam(displayMode);
    return;
}

void FoldScreenBasePolicy::UpdateDeviceStatus(FoldDisplayMode displayMode)
{
    DMDeviceStatus deviceStatus = DMDeviceStatus::UNKNOWN;
    auto iter = DISPLAYMODE_DEVICESTATUS_MAPPING.find(displayMode);
    if (iter != DISPLAYMODE_DEVICESTATUS_MAPPING.end()) {
        deviceStatus = iter->second;
    }
    TLOGI(WmsLogTag::DMS, "Set device status to: %{public}u", deviceStatus);
    SetDeviceStatus(static_cast<uint32_t>(deviceStatus));
    system::SetParameter("persist.dms.device.status", std::to_string(static_cast<uint32_t>(deviceStatus)));
}

void FoldScreenBasePolicy::ChangeScreenDisplayMode(FoldDisplayMode displayMode, bool isForce,
    DisplayModeChangeReason reason)
{
    if (!CheckDisplayModeChange(displayMode, isForce)) {
        return;
    }
    ChangeScreenDisplayModeInner(displayMode, reason);
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
    ScreenSessionManager::GetInstance().SwitchScrollParam(displayMode);
}

void FoldScreenBasePolicy::ChangeScreenDisplayModeInner(FoldDisplayMode displayMode,
    DisplayModeChangeReason reason)
{
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_FULL);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "default screenSession is null");
        return;
    }
    {
        std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
        lastDisplayMode_ = displayMode;
    }
    ReportFoldDisplayModeChange(displayMode);
    switch (displayMode) {
        case FoldDisplayMode::MAIN: {
            if (currentDisplayMode_ == FoldDisplayMode::COORDINATION) {
                CloseCoordinationScreen();
            }
            ChangeScreenDisplayModeToMain(screenSession);
            break;
        }
        case FoldDisplayMode::FULL: {
            if (currentDisplayMode_ == FoldDisplayMode::COORDINATION) {
                CloseCoordinationScreen();
                if (GetModeMatchStatus() != displayMode) {
                    TLOGI(WmsLogTag::DMS, "Exit coordination and recover full");
                    ChangeScreenDisplayModeToFull(screenSession, reason);
                }
            } else {
                ChangeScreenDisplayModeToFull(screenSession, reason);
            }
            break;
        }
        case FoldDisplayMode::COORDINATION: {
            ChangeScreenDisplayModeToCoordination();
            break;
        }
        default: {
            TLOGI(WmsLogTag::DMS, "ChangeScreenDisplayMode displayMode is invalid");
            break;
        }
    }
    {
        std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
        currentDisplayMode_ = displayMode;
    }
}

void FoldScreenBasePolicy::SendSensorResult(FoldStatus foldStatus)
{
    TLOGI(WmsLogTag::DMS, "SendSensorResult FoldStatus: %{public}d", foldStatus);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    ChangeScreenDisplayMode(displayMode);
}

void FoldScreenBasePolicy::ChangeScreenDisplayModeToMainWhenFoldScreenOn(sptr<ScreenSession> screenSession)
{
    TLOGI(WmsLogTag::DMS, "IsFoldScreenOn is true, begin.");
    ReportFoldStatusChangeBegin((int32_t)SCREEN_ID_FULL, (int32_t)SCREEN_ID_MAIN);
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
    ScreenSessionManager::GetInstance().GetScreenPowerTaskScheduler()->
        PostAsyncTask(taskScreenOnMain, "screenOnMainTask");
    SendPropertyChangeResult(screenSession, SCREEN_ID_MAIN, ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING);
}

void FoldScreenBasePolicy::ChangeScreenDisplayModeToMainWhenFoldScreenOff(sptr<ScreenSession> screenSession)
{
    TLOGI(WmsLogTag::DMS, "IsFoldScreenOn is false, begin.");
    // off full screen
    auto taskScreenOffMainOff = [=] {
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayModeToMain: IsFoldScreenOn is false, screenIdFull OFF.");
        screenId_ = SCREEN_ID_FULL;
        ChangeScreenDisplayModePower(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF);
        SetdisplayModeChangeStatus(false);
    };
    ScreenSessionManager::GetInstance().GetScreenPowerTaskScheduler()->
        PostAsyncTask(taskScreenOffMainOff, "screenOffMainOffTask");
    SendPropertyChangeResult(screenSession, SCREEN_ID_MAIN, ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING);
    bool isTentMode = ScreenSessionManager::GetInstance().GetTentMode();
    auto taskScreenOnMainChangeScreenId = [=] {
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayModeToMain: IsFoldScreenOn is false, Change ScreenId to Main.");
        screenId_ = SCREEN_ID_MAIN;
#ifdef TP_FEATURE_ENABLE
        RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE_POWER_CTRL, MAIN_TP_OFF.c_str());
#endif
        if (isTentMode) {
            PowerMgr::PowerMgrClient::GetInstance().WakeupDeviceAsync(
                PowerMgr::WakeupDeviceType::WAKEUP_DEVICE_TENT_MODE_CHANGE);
        }
        SetdisplayModeChangeStatus(false);
    };
    ScreenSessionManager::GetInstance().GetScreenPowerTaskScheduler()->
        PostAsyncTask(taskScreenOnMainChangeScreenId, "taskScreenOnMainChangeScreenId");
}

void FoldScreenBasePolicy::ChangeScreenDisplayModeToMain(sptr<ScreenSession> screenSession,
    DisplayModeChangeReason reason)
{
    SetdisplayModeChangeStatus(true);
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

void FoldScreenBasePolicy::ChangeScreenDisplayModeToFullWhenFoldScreenOn(sptr<ScreenSession> screenSession)
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
    ScreenSessionManager::GetInstance().GetScreenPowerTaskScheduler()->
        PostAsyncTask(taskScreenOnFull, "screenOnFullTask");
    SendPropertyChangeResult(screenSession, SCREEN_ID_FULL, ScreenPropertyChangeReason::FOLD_SCREEN_EXPAND);
}

void FoldScreenBasePolicy::ChangeScreenDisplayModeToFullWhenFoldScreenOff(sptr<ScreenSession> screenSession,
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
    ScreenSessionManager::GetInstance().GetScreenPowerTaskScheduler()->
        PostAsyncTask(taskScreenOffFullOff, "screenOffFullOffTask");
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
    ScreenSessionManager::GetInstance().GetScreenPowerTaskScheduler()->
        PostAsyncTask(taskScreenOnFullOn, "screenOnFullOnTask");
}

void FoldScreenBasePolicy::ChangeScreenDisplayModeToFull(sptr<ScreenSession> screenSession,
    DisplayModeChangeReason reason)
{
    SetdisplayModeChangeStatus(true);
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

void FoldScreenBasePolicy::ChangeScreenDisplayModePower(ScreenId screenId, ScreenPowerStatus screenPowerStatus)
{
    ScreenSessionManager::GetInstance().SetKeyguardDrawnDoneFlag(false);
    ScreenSessionManager::GetInstance().SetScreenPowerForFold(screenId, screenPowerStatus);
}

void FoldScreenBasePolicy::SendPropertyChangeResult(sptr<ScreenSession> screenSession, ScreenId screenId,
    ScreenPropertyChangeReason reason)
{
    screenProperty_ = ScreenSessionManager::GetInstance().GetPhyScreenProperty(screenId);
    screenSession->SetPhyScreenId(screenId);
    if (!ScreenSessionManager::GetInstance().GetClientProxy()) {
        screenSession->UpdatePropertyByFoldControl(screenProperty_);

        screenSession->PropertyChange(screenSession->GetScreenProperty(), reason);
        TLOGI(WmsLogTag::DMS, "screenBounds : width_= %{public}f, height_= %{public}f",
            screenSession->GetScreenProperty().GetBounds().rect_.width_,
            screenSession->GetScreenProperty().GetBounds().rect_.height_);

        screenSession->SetRotationAndScreenRotationOnly(Rotation::ROTATION_0);
        ScreenSessionManager::GetInstance().NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(),
            DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    } else {
        screenSession->NotifyFoldPropertyChange(screenProperty_, reason, GetScreenDisplayMode());
    }
}

void FoldScreenBasePolicy::SetdisplayModeChangeStatus(bool status, bool isOnBootAnimation)
{
    if (status) {
        pendingTask_ = isOnBootAnimation ? FOLD_TASK_NUM_ONBOOTANIMATION : FOLD_TASK_NUM;
        startTimePoint_ = std::chrono::steady_clock::now();
        displayModeChangeRunning_ = status;
    } else {
        pendingTask_ --;
        if (pendingTask_ != 0) {
            return;
        }
        displayModeChangeRunning_ = false;
        endTimePoint_ = std::chrono::steady_clock::now();
        if (lastCachedisplayMode_.load() != GetScreenDisplayMode()) {
            ScreenSessionManager::GetInstance().TriggerDisplayModeUpdate(lastCachedisplayMode_.load());
        }
    }
}

/**
  * fold or expand end
  */

sptr<FoldCreaseRegion> FoldScreenBasePolicy::GetCurrentFoldCreaseRegion()
{
    TLOGI(WmsLogTag::DMS, "GetCurrentFoldCreaseRegion");
    return FoldCreaseRegionController::GetInstance().GetCurrentFoldCreaseRegion();
}

FoldCreaseRegion FoldScreenBasePolicy::GetLiveCreaseRegion() const
{
    return FoldCreaseRegionController::GetInstance().GetLiveCreaseRegion();
}

void FoldScreenBasePolicy::GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems) const
{
    FoldCreaseRegionController::GetInstance().GetAllCreaseRegion(foldCreaseRegionItems);
}

FoldDisplayMode FoldScreenBasePolicy::GetModeMatchStatus()
{
    return GetModeMatchStatus(currentFoldStatus_);
}

FoldDisplayMode FoldScreenBasePolicy::GetModeMatchStatus(FoldStatus targetFoldStatus)
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

void FoldScreenBasePolicy::SetFoldLockFlagAndFoldStatus(bool physicalFoldLockFlag, FoldStatus targetFoldStatus)
{
    TLOGI(WmsLogTag::DMS, "Set physicalFoldLockFlag as %{public}d, forceFoldStatus as %{public}d",
        physicalFoldLockFlag, targetFoldStatus);
    physicalFoldLockFlag_.store(physicalFoldLockFlag, std::memory_order_relaxed);
    forceFoldStatus_.store(targetFoldStatus, std::memory_order_relaxed);
}

const std::unordered_set<FoldStatus>& FoldScreenBasePolicy::GetSupportedFoldStatus() const
{
    return SUPPORTED_FOLD_STATUS;
}

bool FoldScreenBasePolicy::GetPhysicalFoldLockFlag() const
{
    return physicalFoldLockFlag_.load(std::memory_order_relaxed);
}

FoldStatus FoldScreenBasePolicy::GetForceFoldStatus() const
{
    return forceFoldStatus_.load(std::memory_order_relaxed);
}


bool FoldScreenBasePolicy::IsFoldStatusSupported(const std::unordered_set<FoldStatus>& supportedFoldStatus,
    FoldStatus targetFoldStatus) const
{
    return supportedFoldStatus.find(targetFoldStatus) != supportedFoldStatus.end();
}

DMError FoldScreenBasePolicy::SetFoldStatusAndLockControl(bool isLocked, FoldStatus targetFoldStatus)
{
    if (GetModeChangeRunningStatus()) {
        TLOGW(WmsLogTag::DMS, "last process not complete!");
        return DMError::DM_ERROR_DISPLAY_MODE_SWITCH_PENDING;
    }
    if (isLocked && IsFoldStatusSupported(GetSupportedFoldStatus(), targetFoldStatus)) {
        TLOGE(WmsLogTag::DMS, "Current device does not support this fold status: %{public}d", targetFoldStatus);
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    FoldStatus currentFoldStatus = GetFoldStatus();
    FoldStatus changeFoldStatus = isLocked ? targetFoldStatus : GetPhysicalFoldStatus();
    SetFoldLockFlagAndFoldStatus(isLocked, targetFoldStatus);
    if (currentFoldStatus == changeFoldStatus) {
        TLOGW(WmsLogTag::DMS,
            "current fold status: %{public}d equal to change fold status, no need to change", currentFoldStatus);
        return DMError::DM_OK;
    }
    TLOGI(WmsLogTag::DMS, "Change fold status from %{public}d to %{public}d", currentFoldStatus, changeFoldStatus);
    ScreenSessionManager::GetInstance().NotifyFoldStatusChanged(changeFoldStatus);
    FoldDisplayMode targetDisplayMode = GetModeMatchStatus(changeFoldStatus);
    TLOGI(WmsLogTag::DMS,
        "Get fold status: %{public}d, display mode: %{public}d", changeFoldStatus, targetDisplayMode);
    ChangeScreenDisplayMode(targetDisplayMode, DisplayModeChangeReason::FORCE_SET);
    return DMError::DM_OK;
}

FoldStatus FoldScreenBasePolicy::GetPhysicalFoldStatus()
{
    return lastFoldStatus_;
}

bool FoldScreenBasePolicy::GetLockDisplayStatus() const
{
    return lockDisplayStatus_;
}

void FoldScreenBasePolicy::SetCurrentDisplayMode(FoldDisplayMode mode)
{
    std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
    currentDisplayMode_ = mode;
}
} // namespace OHOS::Rosen