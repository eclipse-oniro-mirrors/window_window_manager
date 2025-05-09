/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "dm_common.h"
#include "fold_screen_controller/dual_display_fold_policy.h"
#include "session/screen/include/screen_session.h"
#include "screen_session_manager.h"

#include "window_manager_hilog.h"
#include "parameters.h"

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

namespace OHOS::Rosen {
namespace {
const ScreenId SCREEN_ID_MAIN = 0;
const ScreenId SCREEN_ID_SUB = 5;
const bool IS_COORDINATION_SUPPORT =
    OHOS::system::GetBoolParameter("const.window.foldabledevice.is_coordination_support", false);
#ifdef TP_FEATURE_ENABLE
const int32_t TP_TYPE = 12;
#endif
const std::string MAIN_TP = "0";
const std::string SUB_TP = "1";
const int32_t REMOVE_DISPLAY_NODE = 0;
const int32_t ADD_DISPLAY_NODE = 1;
const uint32_t CHANGE_MODE_TASK_NUM = 3;
} // namespace

DualDisplayFoldPolicy::DualDisplayFoldPolicy(std::recursive_mutex& displayInfoMutex,
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler): screenPowerTaskScheduler_(screenPowerTaskScheduler)
{
    TLOGI(WmsLogTag::DMS, "DualDisplayFoldPolicy created");

    ScreenId screenIdMain = 0;
    int32_t foldCreaseRegionPosX = 0;
    int32_t foldCreaseRegionPosY = 1256;
    int32_t foldCreaseRegionPosWidth = 1136;
    int32_t foldCreaseRegionPosHeight = 184;

    std::vector<DMRect> rect = {
        {
            foldCreaseRegionPosX, foldCreaseRegionPosY,
            foldCreaseRegionPosWidth, foldCreaseRegionPosHeight
        }
    };
    currentFoldCreaseRegion_ = new FoldCreaseRegion(screenIdMain, rect);
}

void DualDisplayFoldPolicy::SetdisplayModeChangeStatus(bool status, bool isOnBootAnimation)
{
    if (status) {
        pengdingTask_ = CHANGE_MODE_TASK_NUM;
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
            TLOGI(WmsLogTag::DMS, "start change displaymode to lastest mode");
            ChangeScreenDisplayMode(lastCachedisplayMode_.load());
        }
    }
}

bool DualDisplayFoldPolicy::CheckDisplayMode(FoldDisplayMode displayMode)
{
    if (displayMode == FoldDisplayMode::COORDINATION && !IS_COORDINATION_SUPPORT) {
        TLOGI(WmsLogTag::DMS, "Current device is not support coordination");
        return false;
    }
    if (currentDisplayMode_ == displayMode) {
        TLOGW(WmsLogTag::DMS, "ChangeScreenDisplayMode already in displayMode %{public}d", displayMode);
        return false;
    }
    return true;
}

ScreenId DualDisplayFoldPolicy::GetScreenIdByDisplayMode(FoldDisplayMode displayMode)
{
    ScreenId screenId = SCREEN_ID_MAIN;
    if (displayMode == FoldDisplayMode::SUB) {
        screenId = SCREEN_ID_SUB;
    }
    return screenId;
}

void DualDisplayFoldPolicy::ChangeScreenDisplayMode(FoldDisplayMode displayMode, DisplayModeChangeReason reason)
{
    SetLastCacheDisplayMode(displayMode);
    if (GetModeChangeRunningStatus()) {
        TLOGW(WmsLogTag::DMS, "last process not complete, skip mode: %{public}d", displayMode);
        return;
    }
    TLOGI(WmsLogTag::DMS, "start change displaymode: %{public}d, lastElapsedMs: %{public}" PRId64 "ms",
        displayMode, getFoldingElapsedMs());
    ScreenId screenId = GetScreenIdByDisplayMode(displayMode);
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "default screenSession is null");
        return;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:ChangeScreenDisplayMode(displayMode= %" PRIu64")", displayMode);
    {
        std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
        if (!CheckDisplayMode(displayMode)) {
            return;
        }
    }
    SetdisplayModeChangeStatus(true);
    {
        std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
        lastDisplayMode_ = displayMode;
    }
    ReportFoldDisplayModeChange(displayMode);
    ScreenSessionManager::GetInstance().SwitchScrollParam(displayMode);
    ChangeScreenDisplayModeProc(screenSession, displayMode);
    {
        std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
        currentDisplayMode_ = displayMode;
    }
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
    SetdisplayModeChangeStatus(false);
}

void DualDisplayFoldPolicy::ChangeScreenDisplayModeProc(sptr<ScreenSession> screenSession,
    FoldDisplayMode displayMode)
{
    switch (displayMode) {
        case FoldDisplayMode::SUB: {
            ChangeScreenDisplayModeInner(screenSession, SCREEN_ID_MAIN, SCREEN_ID_SUB);
            break;
        }
        case FoldDisplayMode::MAIN: {
            ChangeScreenDisplayModeInner(screenSession, SCREEN_ID_SUB, SCREEN_ID_MAIN);
            break;
        }
        case FoldDisplayMode::COORDINATION: {
            ChangeScreenDisplayModeToCoordination();
            break;
        }
        default: {
            break;
        }
    }
}

void DualDisplayFoldPolicy::SendSensorResult(FoldStatus foldStatus)
{
    TLOGI(WmsLogTag::DMS, "SendSensorResult FoldStatus: %{public}d", foldStatus);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    bool isScreenOn = PowerMgr::PowerMgrClient::GetInstance().IsFoldScreenOn();
    if (currentDisplayMode_ == FoldDisplayMode::COORDINATION && isScreenOn &&
        displayMode == FoldDisplayMode::MAIN) {
        TLOGI(WmsLogTag::DMS, "CurrentDisplayMode is coordination, HalfFold no need to change displaympde");
        return;
    }
    ChangeScreenDisplayMode(displayMode);
}

sptr<FoldCreaseRegion> DualDisplayFoldPolicy::GetCurrentFoldCreaseRegion()
{
    TLOGI(WmsLogTag::DMS, "GetCurrentFoldCreaseRegion");
    return currentFoldCreaseRegion_;
}

void DualDisplayFoldPolicy::LockDisplayStatus(bool locked)
{
    TLOGI(WmsLogTag::DMS, "LockDisplayStatus locked: %{public}d", locked);
    lockDisplayStatus_ = locked;
}

void DualDisplayFoldPolicy::SetOnBootAnimation(bool onBootAnimation)
{
    TLOGI(WmsLogTag::DMS, "SetOnBootAnimation onBootAnimation: %{public}d", onBootAnimation);
    onBootAnimation_ = onBootAnimation;
    if (!onBootAnimation_) {
        TLOGI(WmsLogTag::DMS, "SetOnBootAnimation when boot animation finished, change display mode");
        RecoverWhenBootAnimationExit();
    }
}

void DualDisplayFoldPolicy::RecoverWhenBootAnimationExit()
{
    TLOGI(WmsLogTag::DMS, "CurrentScreen(%{public}" PRIu64 ")", screenId_);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    if (currentDisplayMode_ != displayMode) {
        ChangeScreenDisplayMode(displayMode);
    } else {
        TriggerScreenDisplayModeUpdate(displayMode);
    }
}

void DualDisplayFoldPolicy::TriggerScreenDisplayModeUpdate(FoldDisplayMode displayMode)
{
    TLOGI(WmsLogTag::DMS, "TriggerScreenDisplayModeUpdate displayMode = %{public}d", displayMode);
    sptr<ScreenSession> screenSession = nullptr;
    if (displayMode == FoldDisplayMode::SUB) {
        screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_SUB);
    } else {
        screenSession = ScreenSessionManager::GetInstance().GetScreenSession(SCREEN_ID_MAIN);
    }
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "TriggerScreenDisplayModeUpdate default screenSession is null");
        return;
    }
    switch (displayMode) {
        case FoldDisplayMode::SUB: {
            ChangeScreenDisplayModeInner(screenSession, SCREEN_ID_MAIN, SCREEN_ID_SUB);
            break;
        }
        case FoldDisplayMode::MAIN: {
            ChangeScreenDisplayModeInner(screenSession, SCREEN_ID_SUB, SCREEN_ID_MAIN);
            break;
        }
        case FoldDisplayMode::UNKNOWN: {
            TLOGI(WmsLogTag::DMS, "TriggerScreenDisplayModeUpdate displayMode is unknown");
            break;
        }
        default: {
            TLOGI(WmsLogTag::DMS, "TriggerScreenDisplayModeUpdate displayMode is invalid");
            break;
        }
    }
}

void DualDisplayFoldPolicy::UpdateForPhyScreenPropertyChange()
{
    TLOGI(WmsLogTag::DMS, "UpdateForPhyScreenPropertyChange currentScreen(%{public}" PRIu64 ")", screenId_);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    if (currentDisplayMode_ != displayMode) {
        ChangeScreenDisplayMode(displayMode);
    }
}

FoldDisplayMode DualDisplayFoldPolicy::GetModeMatchStatus()
{
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    switch (currentFoldStatus_) {
        case FoldStatus::EXPAND: {
            displayMode = FoldDisplayMode::MAIN;
            break;
        }
        case FoldStatus::FOLDED: {
            displayMode = FoldDisplayMode::SUB;
            break;
        }
        case FoldStatus::HALF_FOLD: {
            displayMode = FoldDisplayMode::MAIN;
            break;
        }
        default: {
            TLOGI(WmsLogTag::DMS, "GetModeMatchStatus FoldStatus is invalid");
        }
    }
    return displayMode;
}

void DualDisplayFoldPolicy::ReportFoldDisplayModeChange(FoldDisplayMode displayMode)
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

void DualDisplayFoldPolicy::ReportFoldStatusChangeBegin(int32_t offScreen, int32_t onScreen)
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

void DualDisplayFoldPolicy::ChangeScreenDisplayModeInner(sptr<ScreenSession> screenSession, ScreenId offScreenId,
    ScreenId onScreenId)
{
    if (onBootAnimation_) {
        ChangeScreenDisplayModeOnBootAnimation(screenSession, onScreenId);
        return;
    }
    std::string tp = MAIN_TP;
    if (onScreenId == SCREEN_ID_SUB) {
        tp = SUB_TP;
    }
#ifdef TP_FEATURE_ENABLE
    RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE, tp.c_str());
#endif
    ReportFoldStatusChangeBegin((int32_t)SCREEN_ID_MAIN, (int32_t)SCREEN_ID_SUB);
    bool isScreenOn = PowerMgr::PowerMgrClient::GetInstance().IsFoldScreenOn();
    TLOGI(WmsLogTag::DMS, "ChangeScreenDisplayModeToCoordination, isScreenOn= %{public}d", isScreenOn);
    auto taskScreenOff = [=] {
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayMode: off screenId: %{public}" PRIu64 "", offScreenId);
        screenId_ = offScreenId;
        ScreenSessionManager::GetInstance().SetKeyguardDrawnDoneFlag(false);
        ScreenSessionManager::GetInstance().SetScreenPowerForFold(ScreenPowerStatus::POWER_STATUS_OFF);
        SetdisplayModeChangeStatus(false);
    };
    if (screenPowerTaskScheduler_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenPowerTaskScheduler_ is nullpter");
        return;
    }
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOff, "screenOffTask");
    AddOrRemoveDisplayNodeToTree(offScreenId, REMOVE_DISPLAY_NODE);

    auto taskScreenOn = [=] {
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayMode: on screenId: %{public}" PRIu64 "", onScreenId);
        screenId_ = onScreenId;
        if (isScreenOn) {
            ScreenSessionManager::GetInstance().SetKeyguardDrawnDoneFlag(false);
            ScreenSessionManager::GetInstance().SetScreenPowerForFold(ScreenPowerStatus::POWER_STATUS_ON);
        } else {
            PowerMgr::PowerMgrClient::GetInstance().WakeupDeviceAsync();
        }
        SetdisplayModeChangeStatus(false);
    };
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOn, "screenOnTask");
    AddOrRemoveDisplayNodeToTree(onScreenId, ADD_DISPLAY_NODE);
}

void DualDisplayFoldPolicy::ChangeScreenDisplayModeToCoordination()
{
    bool isScreenOn = PowerMgr::PowerMgrClient::GetInstance().IsFoldScreenOn();
    TLOGI(WmsLogTag::DMS, "ChangeScreenDisplayModeToCoordination, isScreenOn= %{public}d", isScreenOn);
#ifdef TP_FEATURE_ENABLE
    RSInterfaces::GetInstance().SetTpFeatureConfig(TP_TYPE, MAIN_TP.c_str());
#endif
    // on main screen
    auto taskScreenOnMain = [=] {
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayMode: on main screenId");
        screenId_ = SCREEN_ID_MAIN;
        if (isScreenOn) {
            ScreenSessionManager::GetInstance().SetKeyguardDrawnDoneFlag(false);
            ScreenSessionManager::GetInstance().SetScreenPower(ScreenPowerStatus::POWER_STATUS_ON,
                PowerStateChangeReason::STATE_CHANGE_REASON_DISPLAY_SWITCH);
        } else {
            PowerMgr::PowerMgrClient::GetInstance().WakeupDeviceAsync();
        }
        SetdisplayModeChangeStatus(false);
    };
    if (screenPowerTaskScheduler_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenPowerTaskScheduler_ is nullpter");
        return;
    }
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOnMain, "taskScreenOnMain");
    // on sub screen
    auto taskScreenOnSub = [=] {
        TLOGNI(WmsLogTag::DMS, "ChangeScreenDisplayMode: on sub screenId");
        if (isScreenOn) {
            ScreenSessionManager::GetInstance().SetKeyguardDrawnDoneFlag(false);
            ScreenSessionManager::GetInstance().SetScreenPowerForFold(SCREEN_ID_SUB,
                ScreenPowerStatus::POWER_STATUS_ON);
        }
        SetdisplayModeChangeStatus(false);
    };
    screenPowerTaskScheduler_->PostAsyncTask(taskScreenOnSub, "taskScreenOnSub");
    AddOrRemoveDisplayNodeToTree(SCREEN_ID_SUB, ADD_DISPLAY_NODE);
}

void DualDisplayFoldPolicy::ChangeScreenDisplayModeOnBootAnimation(sptr<ScreenSession> screenSession, ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "ChangeScreenDisplayModeToFullOnBootAnimation");
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "ChangeScreenDisplayModeOnBootAnimation, ScreenSession is nullpter");
        return;
    }
    screenProperty_ = ScreenSessionManager::GetInstance().GetPhyScreenProperty(screenId);
    ScreenPropertyChangeReason reason = ScreenPropertyChangeReason::FOLD_SCREEN_EXPAND;
    if (screenId == SCREEN_ID_SUB) {
        reason = ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING;
    }
    screenSession->UpdatePropertyByFoldControl(screenProperty_);
    screenSession->PropertyChange(screenSession->GetScreenProperty(), reason);
    TLOGI(WmsLogTag::DMS, "screenBounds : width_= %{public}f, height_= %{public}f",
        screenSession->GetScreenProperty().GetBounds().rect_.width_,
        screenSession->GetScreenProperty().GetBounds().rect_.height_);
    screenId_ = screenId;
}

void DualDisplayFoldPolicy::AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command)
{
    TLOGI(WmsLogTag::DMS, "AddOrRemoveDisplayNodeToTree, screenId: %{public}" PRIu64 ", command: %{public}d",
        screenId, command);
    sptr<ScreenSession> screenSession = ScreenSessionManager::GetInstance().GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "AddOrRemoveDisplayNodeToTree, screenSession is null");
        return;
    }
    std::shared_ptr<RSDisplayNode> displayNode = screenSession->GetDisplayNode();
    if (displayNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "AddOrRemoveDisplayNodeToTree, displayNode is null");
        return;
    }
    if (command == ADD_DISPLAY_NODE) {
        displayNode->AddDisplayNodeToTree();
    } else if (command == REMOVE_DISPLAY_NODE) {
        displayNode->RemoveDisplayNodeFromTree();
    }
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        TLOGI(WmsLogTag::DMS, "add or remove displayNode");
        transactionProxy->FlushImplicitTransaction();
    }
}

void DualDisplayFoldPolicy::ExitCoordination()
{
    ScreenSessionManager::GetInstance().SetScreenPowerForFold(SCREEN_ID_SUB,
        ScreenPowerStatus::POWER_STATUS_OFF);
    AddOrRemoveDisplayNodeToTree(SCREEN_ID_SUB, REMOVE_DISPLAY_NODE);
    FoldDisplayMode displayMode = GetModeMatchStatus();
    currentDisplayMode_ = displayMode;
    lastDisplayMode_ = displayMode;
    TLOGI(WmsLogTag::DMS, "CurrentDisplayMode:%{public}d", displayMode);
    ScreenSessionManager::GetInstance().NotifyDisplayModeChanged(displayMode);
}
} // namespace OHOS::Rosen