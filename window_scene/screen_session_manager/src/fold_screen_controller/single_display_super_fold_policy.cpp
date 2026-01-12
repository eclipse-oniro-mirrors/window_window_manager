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
#include "fold_screen_controller/single_display_super_fold_policy.h"
#include <hisysevent.h>
#include <hitrace_meter.h>
#include <transaction/rs_interfaces.h>
#include <parameters.h>
#include "session/screen/include/screen_session.h"
#include "screen_session_manager.h"
#include "fold_screen_state_internel.h"
#include "rs_adapter.h"
#include "sensor_agent.h"
#include "sensor_agent_type.h"

#include "window_manager_hilog.h"

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

namespace OHOS::Rosen {
namespace {
const ScreenId SCREEN_ID_FULL = 0;
const ScreenId SCREEN_ID_MAIN = 5;
constexpr int32_t FOLD_CREASE_RECT_SIZE = 4; //numbers of parameter on the current device is 4
const std::string g_FoldScreenRect = system::GetParameter("const.display.foldscreen.crease_region", "");
const std::string FOLD_CREASE_DELIMITER = ",;";
const int32_t REMOVE_DISPLAY_NODE = 0;
const int32_t ADD_DISPLAY_NODE = 1;

const int32_t RECT_POS_X_INDEX = 0;
const int32_t RECT_POS_Y_INDEX = 1;
const int32_t RECT_POS_WIDTH_INDEX = 2;
const int32_t RECT_POS_HEIGHT_INDEX = 3;

#ifdef TP_FEATURE_ENABLE
const int32_t TP_TYPE = 12;
const int32_t TP_TYPE_POWER_CTRL = 18;
const std::string FULL_TP = "0";
const std::string MAIN_TP = "1";
const std::string MAIN_TP_OFF = "1,1";
const std::string FULL_TP_OFF = "0,1";
#endif

static const std::map<FoldDisplayMode, DMDeviceStatus> DISPLAYMODE_DEVICESTATUS_MAPPING = {
    {FoldDisplayMode::MAIN, DMDeviceStatus::STATUS_FOLDED},
    {FoldDisplayMode::FULL, DMDeviceStatus::STATUS_EXPAND},
    {FoldDisplayMode::GLOBAL_FULL, DMDeviceStatus::STATUS_GLOBAL_FULL}
};
} // namespace

SingleDisplaySuperFoldPolicy::SingleDisplaySuperFoldPolicy(std::recursive_mutex& displayInfoMutex,
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler)
    : displayInfoMutex_(displayInfoMutex), screenPowerTaskScheduler_(screenPowerTaskScheduler)
{
    TLOGI(WmsLogTag::DMS, "SingleDisplaySuperFoldPolicy created");
    currentFoldCreaseRegion_ = sptr<FoldCreaseRegion>::MakeSptr(SCREEN_ID_FULL, GetFoldCreaseRegionRect(true));
}

FoldCreaseRegion SingleDisplaySuperFoldPolicy::GetFoldCreaseRegion(bool isVertical) const
{
    std::vector<DMRect> foldCreaseRect = GetFoldCreaseRegionRect(isVertical);
    return FoldCreaseRegion(SCREEN_ID_FULL, foldCreaseRect);
}

std::vector<DMRect> SingleDisplaySuperFoldPolicy::GetFoldCreaseRegionRect(bool isVertical) const
{
    std::vector<DMRect> foldCreaseRect = {};
    std::vector<int32_t> foldRect = FoldScreenStateInternel::StringFoldRectSplitToInt(g_FoldScreenRect,
        FOLD_CREASE_DELIMITER);
    if (foldRect.size() != FOLD_CREASE_RECT_SIZE) {
        TLOGE(WmsLogTag::DMS, "foldRect is invalid");
        return foldCreaseRect;
    }
    GetFoldCreaseRect(isVertical, foldRect, foldCreaseRect);
    return foldCreaseRect;
}

void SingleDisplaySuperFoldPolicy::GetFoldCreaseRect(bool isVertical,
    const std::vector<int32_t>& foldRect, std::vector<DMRect>& foldCreaseRect) const
{
    int32_t liveCreaseRegionPosX; // live Crease Region PosX
    int32_t liveCreaseRegionPosY; // live Crease Region PosY
    uint32_t liveCreaseRegionPosWidth; // live Crease Region PosWidth
    uint32_t liveCreaseRegionPosHeight; // live Crease Region PosHeight
    if (isVertical) {
        TLOGI(WmsLogTag::DMS, "the current FoldCreaseRect is vertical");
        liveCreaseRegionPosX = foldRect[RECT_POS_X_INDEX];
        liveCreaseRegionPosY = foldRect[RECT_POS_Y_INDEX];
        liveCreaseRegionPosWidth = static_cast<uint32_t>(foldRect[RECT_POS_WIDTH_INDEX]);
        liveCreaseRegionPosHeight = static_cast<uint32_t>(foldRect[RECT_POS_HEIGHT_INDEX]);
    } else {
        TLOGI(WmsLogTag::DMS, "the current FoldCreaseRect is horizontal");
        liveCreaseRegionPosX = foldRect[RECT_POS_Y_INDEX];
        liveCreaseRegionPosY = foldRect[RECT_POS_X_INDEX];
        liveCreaseRegionPosWidth = static_cast<uint32_t>(foldRect[RECT_POS_HEIGHT_INDEX]);
        liveCreaseRegionPosHeight = static_cast<uint32_t>(foldRect[RECT_POS_WIDTH_INDEX]);
    }
    foldCreaseRect = {
        {
            liveCreaseRegionPosX, liveCreaseRegionPosY,
            liveCreaseRegionPosWidth, liveCreaseRegionPosHeight
        }
    };
    return;
}

void SingleDisplaySuperFoldPolicy::SetdisplayModeChangeStatus(bool status, bool isOnBootAnimation)
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

bool SingleDisplaySuperFoldPolicy::CheckDisplayModeChange(FoldDisplayMode displayMode, bool isForce,
    DisplayModeChangeReason reason)
{
    if (GetPhysicalFoldLockFlag() && reason != DisplayModeChangeReason::FORCE_SET) {
        TLOGI(WmsLogTag::DMS, "Fold status is locked, can't change to display mode: %{public}d", displayMode);
        return false;
    }
    if (isForce) {
        TLOGI(WmsLogTag::DMS, "force change displayMode");
        SetLastCacheDisplayMode(displayMode);
        return true;
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

    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:`(displayMode = %" PRIu64")", displayMode);
    {
        std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
        if (currentDisplayMode_ == displayMode) {
            TLOGW(WmsLogTag::DMS, "ChangeScreenDisplayMode already in displayMode %{public}d", displayMode);
            return false;
        }
    }
    return true;
}

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayMode(FoldDisplayMode displayMode, DisplayModeChangeReason reason)
{
    if (!CheckDisplayModeChange(displayMode, false, reason)) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "start change displaymode: %{public}d, reason: %{public}d", displayMode, reason);
    ChangeScreenDisplayModeInner(displayMode, reason);
    UpdateDeviceStatus(displayMode);
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
    ScreenSessionManager::GetInstance().SwitchScrollParam(displayMode);
    TLOGD(WmsLogTag::DMS, "end change displaymode: %{public}d, reason: %{public}d", displayMode, reason);
    return;
}

void SingleDisplaySuperFoldPolicy::UpdateDeviceStatus(FoldDisplayMode displayMode)
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

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayMode(FoldDisplayMode displayMode, bool isForce,
    DisplayModeChangeReason reason)
{
    if (!CheckDisplayModeChange(displayMode, isForce)) {
        return;
    }
    ChangeScreenDisplayModeInner(displayMode, reason);
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
    ScreenSessionManager::GetInstance().SwitchScrollParam(displayMode);
}

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayModeInner(FoldDisplayMode displayMode,
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
            SetdisplayModeChangeStatus(true);
            ChangeScreenDisplayModeToMain(screenSession);
            break;
        }
        case FoldDisplayMode::FULL: {
            if (currentDisplayMode_ == FoldDisplayMode::COORDINATION) {
                CloseCoordinationScreen();
            } else {
                SetdisplayModeChangeStatus(true);
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

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayModeToCoordination()
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
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOnMainOn, "ScreenToCoordinationTask");
    AddOrRemoveDisplayNodeToTree(SCREEN_ID_MAIN, ADD_DISPLAY_NODE);
}

void SingleDisplaySuperFoldPolicy::CloseCoordinationScreen()
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
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOnMainOFF, "CloseCoordinationScreenTask");
    AddOrRemoveDisplayNodeToTree(SCREEN_ID_MAIN, REMOVE_DISPLAY_NODE);

    ScreenSessionManager::GetInstance().OnScreenChange(SCREEN_ID_MAIN, ScreenEvent::DISCONNECTED);
    ScreenSessionManager::GetInstance().SetCoordinationFlag(false);
}

void SingleDisplaySuperFoldPolicy::ExitCoordination()
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
    currentDisplayMode_ = displayMode;
    lastDisplayMode_ = displayMode;
    TLOGI(WmsLogTag::DMS, "Exit coordination, current display mode:%{public}d", displayMode);
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
}

void SingleDisplaySuperFoldPolicy::ChangeOnTentMode(FoldStatus currentState)
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

void SingleDisplaySuperFoldPolicy::ChangeOffTentMode()
{
    PowerMgr::PowerMgrClient::GetInstance().WakeupDeviceAsync();
    FoldDisplayMode displayMode = GetModeMatchStatus();
    TLOGW(WmsLogTag::DMS, "CurrentDisplayMode:%{public}d, CurrentFoldStatus:%{public}d",
        currentDisplayMode_, currentFoldStatus_);
    ChangeScreenDisplayMode(displayMode);
}

void SingleDisplaySuperFoldPolicy::NotifyRefreshRateEvent(bool isEventStatus)
{
    EventInfo eventInfo = {
        .eventName = "VOTER_MULTISELFOWNEDSCREEN",
        .eventStatus = isEventStatus,
        .minRefreshRate = 60,
        .maxRefreshRate = 60,
    };
    RSInterfaces::GetInstance().NotifyRefreshRateEvent(eventInfo);
}

void SingleDisplaySuperFoldPolicy::AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command)
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

void SingleDisplaySuperFoldPolicy::SendSensorResult(FoldStatus foldStatus)
{
    TLOGI(WmsLogTag::DMS, "SendSensorResult FoldStatus: %{public}d", foldStatus);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    ChangeScreenDisplayMode(displayMode);
}

sptr<FoldCreaseRegion> SingleDisplaySuperFoldPolicy::GetCurrentFoldCreaseRegion()
{
    TLOGI(WmsLogTag::DMS, "GetCurrentFoldCreaseRegion");
    return currentFoldCreaseRegion_;
}

FoldCreaseRegion SingleDisplaySuperFoldPolicy::GetLiveCreaseRegion()
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
                return FoldCreaseRegion(0, {});
            }
        }
    }
    return liveCreaseRegion_;
}

void SingleDisplaySuperFoldPolicy::GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems) const
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

void SingleDisplaySuperFoldPolicy::LockDisplayStatus(bool locked)
{
    TLOGI(WmsLogTag::DMS, "LockDisplayStatus locked: %{public}d", locked);
    lockDisplayStatus_ = locked;
}

void SingleDisplaySuperFoldPolicy::SetOnBootAnimation(bool onBootAnimation)
{
    TLOGI(WmsLogTag::DMS, "SetOnBootAnimation onBootAnimation: %{public}d", onBootAnimation);
    onBootAnimation_ = onBootAnimation;
    if (!onBootAnimation_) {
        TLOGI(WmsLogTag::DMS, "SetOnBootAnimation when boot animation finished, change display mode");
        RecoverWhenBootAnimationExit();
    }
}

void SingleDisplaySuperFoldPolicy::RecoverWhenBootAnimationExit()
{
    TLOGI(WmsLogTag::DMS, "CurrentScreen(%{public}" PRIu64 ")", screenId_);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    ChangeScreenDisplayMode(displayMode, true);
}

void SingleDisplaySuperFoldPolicy::UpdateForPhyScreenPropertyChange()
{
    TLOGI(WmsLogTag::DMS, "CurrentScreen(%{public}" PRIu64 ")", screenId_);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    if (currentDisplayMode_ != displayMode) {
        ChangeScreenDisplayMode(displayMode);
    }
}

FoldDisplayMode SingleDisplaySuperFoldPolicy::GetModeMatchStatus()
{
    return GetModeMatchStatus(currentFoldStatus_);
}

FoldDisplayMode SingleDisplaySuperFoldPolicy::GetModeMatchStatus(FoldStatus targetFoldStatus)
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

void SingleDisplaySuperFoldPolicy::ReportFoldDisplayModeChange(FoldDisplayMode displayMode)
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

void SingleDisplaySuperFoldPolicy::ReportFoldStatusChangeBegin(int32_t offScreen, int32_t onScreen)
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

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayModeToMainWhenFoldScreenOn(sptr<ScreenSession> screenSession)
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
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOnMain, "screenOnMainTask");
    SendPropertyChangeResult(screenSession, SCREEN_ID_MAIN, ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING);
}

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayModeToMainWhenFoldScreenOff(sptr<ScreenSession> screenSession)
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
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOnMainChangeScreenId, "taskScreenOnMainChangeScreenId");
}

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayModeToMain(sptr<ScreenSession> screenSession,
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

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayModeToFullWhenFoldScreenOn(sptr<ScreenSession> screenSession)
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

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayModeToFullWhenFoldScreenOff(sptr<ScreenSession> screenSession,
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

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayModeToFull(sptr<ScreenSession> screenSession,
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

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayModePower(ScreenId screenId, ScreenPowerStatus screenPowerStatus)
{
    ScreenSessionManager::GetInstance().SetKeyguardDrawnDoneFlag(false);
    ScreenSessionManager::GetInstance().SetScreenPowerForFold(screenId, screenPowerStatus);
}

void SingleDisplaySuperFoldPolicy::SendPropertyChangeResult(sptr<ScreenSession> screenSession, ScreenId screenId,
    ScreenPropertyChangeReason reason)
{
    std::lock_guard<std::recursive_mutex> lock_info(displayInfoMutex_);
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

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayModeToMainOnBootAnimation(sptr<ScreenSession> screenSession)
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

void SingleDisplaySuperFoldPolicy::ChangeScreenDisplayModeToFullOnBootAnimation(sptr<ScreenSession> screenSession)
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

void SingleDisplaySuperFoldPolicy::SetIsClearingBootAnimation(bool isClearingBootAnimation)
{
    TLOGI(WmsLogTag::DMS, "isClearingBootAnimation: %{public}d", isClearingBootAnimation);
    isClearingBootAnimation_ = isClearingBootAnimation;
}
} // namespace OHOS::Rosen