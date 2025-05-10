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

#include "fold_screen_controller/fold_screen_controller.h"
#include "fold_screen_controller/single_display_fold_policy.h"
#include "fold_screen_controller/secondary_display_fold_policy.h"
#include "fold_screen_controller/single_display_pocket_fold_policy.h"
#include "fold_screen_controller/dual_display_fold_policy.h"
#include "fold_screen_controller/fold_screen_sensor_manager.h"
#include "fold_screen_controller/sensor_fold_state_manager/single_display_sensor_fold_state_manager.h"
#include "fold_screen_controller/sensor_fold_state_manager/single_display_sensor_pocket_fold_state_manager.h"
#include "fold_screen_controller/sensor_fold_state_manager/dual_display_sensor_fold_state_manager.h"
#include "fold_screen_controller/sensor_fold_state_manager/secondary_display_sensor_fold_state_manager.h"
#include "fold_screen_controller/secondary_fold_sensor_manager.h"
#include "fold_screen_state_internel.h"

#include "window_manager_hilog.h"

namespace OHOS::Rosen {
FoldScreenController::FoldScreenController(std::recursive_mutex& displayInfoMutex,
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler)
    : displayInfoMutex_(displayInfoMutex), screenPowerTaskScheduler_(screenPowerTaskScheduler)
{
    if (FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
        foldScreenPolicy_ = GetFoldScreenPolicy(DisplayDeviceType::DOUBLE_DISPLAY_DEVICE);
        sensorFoldStateManager_ = new DualDisplaySensorFoldStateManager();
        TLOGI(WmsLogTag::DMS, "fold polocy: DOUBLE_DISPLAY_DEVICE");
    } else if (FoldScreenStateInternel::IsSingleDisplayFoldDevice()) {
        foldScreenPolicy_ = GetFoldScreenPolicy(DisplayDeviceType::SINGLE_DISPLAY_DEVICE);
        sensorFoldStateManager_ = new SingleDisplaySensorFoldStateManager();
        TLOGI(WmsLogTag::DMS, "fold polocy: SINGLE_DISPLAY_DEVICE");
    } else if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        foldScreenPolicy_ = GetFoldScreenPolicy(DisplayDeviceType::SINGLE_DISPLAY_POCKET_DEVICE);
        sensorFoldStateManager_ = new SingleDisplaySensorPocketFoldStateManager();
        TLOGI(WmsLogTag::DMS, "fold polocy: SINGLE_DISPLAY_POCKET_DEVICE");
    } else if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        foldScreenPolicy_ = GetFoldScreenPolicy(DisplayDeviceType::SECONDARY_DISPLAY_DEVICE);
        sensorFoldStateManager_ = new SecondaryDisplaySensorFoldStateManager();
        TLOGI(WmsLogTag::DMS, "fold polocy: SECONDARY_DISPLAY_DEVICE");
    }

    if (foldScreenPolicy_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "FoldScreenPolicy is null");
        return;
    }
#ifdef SENSOR_ENABLE
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        SecondaryFoldSensorManager::GetInstance().SetFoldScreenPolicy(foldScreenPolicy_);
        SecondaryFoldSensorManager::GetInstance().SetSensorFoldStateManager(sensorFoldStateManager_);
    } else {
        FoldScreenSensorManager::GetInstance().SetFoldScreenPolicy(foldScreenPolicy_);
        FoldScreenSensorManager::GetInstance().SetSensorFoldStateManager(sensorFoldStateManager_);
    }
#endif
}

FoldScreenController::~FoldScreenController()
{
    TLOGI(WmsLogTag::DMS, "FoldScreenController is destructed");
}

sptr<FoldScreenPolicy> FoldScreenController::GetFoldScreenPolicy(DisplayDeviceType productType)
{
    sptr<FoldScreenPolicy> tempPolicy = nullptr;
    switch (productType) {
        case DisplayDeviceType::SINGLE_DISPLAY_DEVICE: {
            tempPolicy = new SingleDisplayFoldPolicy(displayInfoMutex_, screenPowerTaskScheduler_);
            break;
        }
        case DisplayDeviceType::DOUBLE_DISPLAY_DEVICE: {
            tempPolicy = new DualDisplayFoldPolicy(displayInfoMutex_, screenPowerTaskScheduler_);
            break;
        }
        case DisplayDeviceType::SINGLE_DISPLAY_POCKET_DEVICE:{
            tempPolicy = new SingleDisplayPocketFoldPolicy(displayInfoMutex_, screenPowerTaskScheduler_);
            break;
        }
        case DisplayDeviceType::SECONDARY_DISPLAY_DEVICE:{
            tempPolicy = new SecondaryDisplayFoldPolicy(displayInfoMutex_, screenPowerTaskScheduler_);
            break;
        }
        default: {
            TLOGE(WmsLogTag::DMS, "GetFoldScreenPolicy DisplayDeviceType is invalid");
            break;
        }
    }

    return tempPolicy;
}

void FoldScreenController::SetDisplayMode(const FoldDisplayMode displayMode)
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "SetDisplayMode: foldScreenPolicy_ is null");
        return;
    }
    foldScreenPolicy_->ChangeScreenDisplayMode(displayMode);
}

void FoldScreenController::RecoverDisplayMode()
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "foldScreenPolicy_ is null");
        return;
    }
    FoldDisplayMode displayMode = foldScreenPolicy_->GetModeMatchStatus();
    FoldDisplayMode currentDisplayMode = foldScreenPolicy_->GetScreenDisplayMode();
    if (displayMode == currentDisplayMode) {
        TLOGI(WmsLogTag::DMS, "current displayMode is correct, skip");
        return;
    }
    if (!FoldScreenStateInternel::IsSingleDisplayFoldDevice() &&
        !FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        TLOGI(WmsLogTag::DMS, "not single display fold (pocket) device, skip");
        return;
    }
    TLOGI(WmsLogTag::DMS, "%{public}d -> %{public}d", currentDisplayMode, displayMode);
    foldScreenPolicy_->ChangeScreenDisplayMode(displayMode, DisplayModeChangeReason::RECOVER);
}

void FoldScreenController::LockDisplayStatus(bool locked)
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "LockDisplayStatus: foldScreenPolicy_ is null");
        return;
    }
    foldScreenPolicy_->LockDisplayStatus(locked);
}

FoldDisplayMode FoldScreenController::GetDisplayMode()
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "GetDisplayMode: foldScreenPolicy_ is null");
        return FoldDisplayMode::UNKNOWN;
    }
    return foldScreenPolicy_->GetScreenDisplayMode();
}

bool FoldScreenController::IsFoldable()
{
    return true;
}

FoldStatus FoldScreenController::GetFoldStatus()
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "GetFoldStatus: foldScreenPolicy_ is null");
        return FoldStatus::UNKNOWN;
    }
    return foldScreenPolicy_->GetFoldStatus();
}

void FoldScreenController::SetFoldStatus(FoldStatus foldStatus)
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "SetFoldStatus: foldScreenPolicy_ is null");
        return;
    }
    foldScreenPolicy_->SetFoldStatus(foldStatus);
}

FoldDisplayMode FoldScreenController::GetModeMatchStatus()
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "foldScreenPolicy is null");
        return FoldDisplayMode::UNKNOWN;
    }
    return foldScreenPolicy_->GetModeMatchStatus();
}

bool FoldScreenController::GetTentMode()
{
    if (sensorFoldStateManager_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "GetTentMode: sensorFoldStateManager_ is null");
        return false;
    }
    return sensorFoldStateManager_->IsTentMode();
}

bool FoldScreenController::GetCameraMode()
{
    if (sensorFoldStateManager_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "GetCameraMode: sensorFoldStateManager_ is null");
        return false;
    }
    return sensorFoldStateManager_->IsCameraMode();
}

void FoldScreenController::OnTentModeChanged(int tentType, int32_t hall)
{
    if (sensorFoldStateManager_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "OnTentModeChanged: sensorFoldStateManager_ is null");
        return;
    }
    return sensorFoldStateManager_->HandleTentChange(tentType, foldScreenPolicy_, hall);
}

sptr<FoldCreaseRegion> FoldScreenController::GetCurrentFoldCreaseRegion()
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "GetFoldStatus: foldScreenPolicy_ is null");
        return nullptr;
    }
    return foldScreenPolicy_->GetCurrentFoldCreaseRegion();
}

ScreenId FoldScreenController::GetCurrentScreenId()
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "GetCurrentScreenId: foldScreenPolicy_ is null");
        return 0;
    }
    return foldScreenPolicy_->GetCurrentScreenId();
}

void FoldScreenController::BootAnimationFinishPowerInit()
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "foldScreenPolicy_ is null");
        return;
    }
    foldScreenPolicy_->BootAnimationFinishPowerInit();
    foldScreenPolicy_->currentDisplayMode_ = FoldDisplayMode::UNKNOWN;
}

void FoldScreenController::SetOnBootAnimation(bool onBootAnimation)
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "SetOnBootAnimation: foldScreenPolicy_ is null");
        return;
    }
    foldScreenPolicy_->SetOnBootAnimation(onBootAnimation);
}

void FoldScreenController::UpdateForPhyScreenPropertyChange()
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "UpdateForPhyScreenPropertyChange: foldScreenPolicy_ is null");
        return;
    }
    foldScreenPolicy_->UpdateForPhyScreenPropertyChange();
}

void FoldScreenController::ExitCoordination()
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "foldScreenPolicy_ is null");
        return;
    }
    foldScreenPolicy_->ExitCoordination();
}

bool FoldScreenController::GetModeChangeRunningStatus()
{
    return foldScreenPolicy_->GetModeChangeRunningStatus();
}

void FoldScreenController::SetdisplayModeChangeStatus(bool status)
{
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        foldScreenPolicy_->SetSecondaryDisplayModeChangeStatus(status);
    } else {
        foldScreenPolicy_->SetdisplayModeChangeStatus(status);
    }
}

bool FoldScreenController::GetdisplayModeRunningStatus()
{
    return foldScreenPolicy_->GetdisplayModeRunningStatus();
}

FoldDisplayMode FoldScreenController::GetLastCacheDisplayMode()
{
    return foldScreenPolicy_->GetLastCacheDisplayMode();
}

void FoldScreenController::AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command)
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "foldScreenPolicy_ is null");
        return;
    }
    foldScreenPolicy_->AddOrRemoveDisplayNodeToTree(screenId, command);
}

Drawing::Rect FoldScreenController::GetScreenSnapshotRect()
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "foldScreenPolicy_ is null");
        Drawing::Rect snapshotRect = {0, 0, 0, 0};
        return snapshotRect;
    }
    return foldScreenPolicy_->GetScreenSnapshotRect();
}

void FoldScreenController::SetMainScreenRegion(DMRect& mainScreenRegion)
{
    if (foldScreenPolicy_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "foldScreenPolicy_ is null");
        return;
    }
    foldScreenPolicy_->SetMainScreenRegion(mainScreenRegion);
}

std::chrono::steady_clock::time_point FoldScreenController::GetStartTimePoint()
{
    return foldScreenPolicy_->GetStartTimePoint();
}

bool FoldScreenController::GetIsFirstFrameCommitReported()
{
    return foldScreenPolicy_->GetIsFirstFrameCommitReported();
}

void FoldScreenController::SetIsFirstFrameCommitReported(bool isFirstFrameCommitReported)
{
    foldScreenPolicy_->SetIsFirstFrameCommitReported(isFirstFrameCommitReported);
}
} // namespace OHOS::Rosen