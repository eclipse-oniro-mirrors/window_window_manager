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

#include "fold_screen_base_controller.h"
#include "sensor_fold_state_mgr.h"
#include "screen_sensor_mgr.h"
#include "window_manager_hilog.h"
#include "product_config.h"

namespace OHOS::Rosen::DMS {
FoldScreenBaseController::FoldScreenBaseController() {}

FoldScreenBaseController::~FoldScreenBaseController()
{
    TLOGI(WmsLogTag::DMS, "FoldScreenBaseController is destructed");
}

nlohmann::ordered_json FoldScreenBaseController::GetFoldCreaseRegionJson()
{
    nlohmann::ordered_json ret = nlohmann::ordered_json::array();
    if (foldCreaseRegionItems_.size() == 0) {
        FoldScreenBasePolicy::GetInstance().GetAllCreaseRegion(foldCreaseRegionItems_);
    }

    for (const auto& foldCreaseRegionItem : foldCreaseRegionItems_) {
        nlohmann::ordered_json capabilityInfo;
        capabilityInfo["foldDisplayMode"] =
            std::to_string(static_cast<int32_t>(foldCreaseRegionItem.foldDisplayMode_));
        capabilityInfo["displayOrientation"] =
            std::to_string(static_cast<int32_t>(foldCreaseRegionItem.orientation_));
        capabilityInfo["creaseRects"]["displayId"] =
            std::to_string(static_cast<int32_t>(foldCreaseRegionItem.region_.GetDisplayId()));
        auto creaseRects = foldCreaseRegionItem.region_.GetCreaseRects();
        capabilityInfo["creaseRects"]["rects"] = nlohmann::ordered_json::array();
        for (const auto& creaseRect : creaseRects) {
            capabilityInfo["creaseRects"]["rects"].push_back({
                {"posX", creaseRect.posX_},
                {"posY", creaseRect.posY_},
                {"width", creaseRect.width_},
                {"height", creaseRect.height_}
            });
        }
        ret.push_back(capabilityInfo);
    }
    return ret;
}

void FoldScreenBaseController::SetDisplayMode(const FoldDisplayMode displayMode)
{
    if (ProductConfig::GetInstance().IsSecondaryDisplayFoldDevice()) {
        FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayMode(displayMode, DisplayModeChangeReason::SETMODE);
    } else {
        FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayMode(displayMode);
    }
}

void FoldScreenBaseController::RecoverDisplayMode()
{
    FoldDisplayMode displayMode = FoldScreenBasePolicy::GetInstance().GetModeMatchStatus();
    FoldDisplayMode currentDisplayMode = FoldScreenBasePolicy::GetInstance().GetScreenDisplayMode();
    if (displayMode == currentDisplayMode) {
        TLOGI(WmsLogTag::DMS, "current displayMode is correct, skip");
        return;
    }
    if (!ProductConfig::GetInstance().IsSingleDisplayFoldDevice() &&
        !ProductConfig::GetInstance().IsSingleDisplayPocketFoldDevice() &&
        !ProductConfig::GetInstance().IsSecondaryDisplayFoldDevice() &&
        !ProductConfig::GetInstance().IsDualDisplayFoldDevice() &&
        !ProductConfig::GetInstance().IsSingleDisplaySuperFoldDevice()) {
        TLOGI(WmsLogTag::DMS, "current fold device do not need recover, skip");
        return;
    }
    TLOGI(WmsLogTag::DMS, "%{public}d -> %{public}d", currentDisplayMode, displayMode);
    FoldScreenBasePolicy::GetInstance().ChangeScreenDisplayMode(displayMode, DisplayModeChangeReason::RECOVER);
}

void FoldScreenBaseController::LockDisplayStatus(bool locked)
{
    FoldScreenBasePolicy::GetInstance().LockDisplayStatus(locked);
}

DMError FoldScreenBaseController::ForceSetFoldStatusAndLock(FoldStatus targetFoldStatus)
{
    return FoldScreenBasePolicy::GetInstance().SetFoldStatusAndLockControl(true, targetFoldStatus);
}

DMError FoldScreenBaseController::RestorePhysicalFoldStatus()
{
    return FoldScreenBasePolicy::GetInstance().SetFoldStatusAndLockControl(false, FoldStatus::UNKNOWN);
}

bool FoldScreenBaseController::GetPhysicalFoldLockFlag() const
{
    return FoldScreenBasePolicy::GetInstance().GetPhysicalFoldLockFlag();
}

FoldStatus FoldScreenBaseController::GetPhysicalFoldStatus() const
{
    return FoldScreenBasePolicy::GetInstance().GetPhysicalFoldStatus();
}

FoldDisplayMode FoldScreenBaseController::GetDisplayMode()
{
    return FoldScreenBasePolicy::GetInstance().GetScreenDisplayMode();
}

bool FoldScreenBaseController::IsFoldable()
{
    return true;
}

FoldStatus FoldScreenBaseController::GetFoldStatus()
{
    return FoldScreenBasePolicy::GetInstance().GetFoldStatus();
}

void FoldScreenBaseController::SetFoldStatus(FoldStatus foldStatus)
{
    FoldScreenBasePolicy::GetInstance().SetFoldStatus(foldStatus);
}

FoldDisplayMode FoldScreenBaseController::GetModeMatchStatus()
{
    return FoldScreenBasePolicy::GetInstance().GetModeMatchStatus();
}

bool FoldScreenBaseController::GetTentMode()
{
    return DMS::SensorFoldStateMgr::GetInstance().IsTentMode();
}

void FoldScreenBaseController::OnTentModeChanged(int tentType, int32_t hall)
{
    return DMS::ScreenSensorMgr::GetInstance().HandleTentSensorData(tentType, hall);
}

sptr<FoldCreaseRegion> FoldScreenBaseController::GetCurrentFoldCreaseRegion()
{
    return FoldScreenBasePolicy::GetInstance().GetCurrentFoldCreaseRegion();
}

FoldCreaseRegion FoldScreenBaseController::GetLiveCreaseRegion() const
{
    return FoldScreenBasePolicy::GetInstance().GetLiveCreaseRegion();
}

ScreenId FoldScreenBaseController::GetCurrentScreenId()
{
    return FoldScreenBasePolicy::GetInstance().GetCurrentScreenId();
}

void FoldScreenBaseController::BootAnimationFinishPowerInit()
{
    FoldScreenBasePolicy::GetInstance().BootAnimationFinishPowerInit();
    FoldScreenBasePolicy::GetInstance().SetCurrentDisplayMode(FoldDisplayMode::UNKNOWN);
}

void FoldScreenBaseController::SetOnBootAnimation(bool onBootAnimation)
{
    FoldScreenBasePolicy::GetInstance().SetOnBootAnimation(onBootAnimation);
}

void FoldScreenBaseController::UpdateForPhyScreenPropertyChange()
{
    FoldScreenBasePolicy::GetInstance().UpdateForPhyScreenPropertyChange();
}

void FoldScreenBaseController::ExitCoordination()
{
    FoldScreenBasePolicy::GetInstance().ExitCoordination();
}

bool FoldScreenBaseController::GetModeChangeRunningStatus()
{
    return FoldScreenBasePolicy::GetInstance().GetModeChangeRunningStatus();
}

void FoldScreenBaseController::SetdisplayModeChangeStatus(bool status)
{
    if (ProductConfig::GetInstance().IsSecondaryDisplayFoldDevice()) {
        FoldScreenBasePolicy::GetInstance().SetSecondaryDisplayModeChangeStatus(status);
    } else {
        FoldScreenBasePolicy::GetInstance().SetdisplayModeChangeStatus(status);
    }
}

bool FoldScreenBaseController::GetdisplayModeRunningStatus()
{
    return FoldScreenBasePolicy::GetInstance().GetdisplayModeRunningStatus();
}

FoldDisplayMode FoldScreenBaseController::GetLastCacheDisplayMode()
{
    return FoldScreenBasePolicy::GetInstance().GetLastCacheDisplayMode();
}

void FoldScreenBaseController::AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command)
{
    FoldScreenBasePolicy::GetInstance().AddOrRemoveDisplayNodeToTree(screenId, command);
}

Drawing::Rect FoldScreenBaseController::GetScreenSnapshotRect()
{
    return FoldScreenBasePolicy::GetInstance().GetScreenSnapshotRect();
}

void FoldScreenBaseController::SetMainScreenRegion(DMRect& mainScreenRegion)
{
    FoldScreenBasePolicy::GetInstance().SetMainScreenRegion(mainScreenRegion);
}

std::chrono::steady_clock::time_point FoldScreenBaseController::GetStartTimePoint()
{
    return FoldScreenBasePolicy::GetInstance().GetStartTimePoint();
}

bool FoldScreenBaseController::GetIsFirstFrameCommitReported()
{
    return FoldScreenBasePolicy::GetInstance().GetIsFirstFrameCommitReported();
}

void FoldScreenBaseController::SetIsFirstFrameCommitReported(bool isFirstFrameCommitReported)
{
    FoldScreenBasePolicy::GetInstance().SetIsFirstFrameCommitReported(isFirstFrameCommitReported);
}

void FoldScreenBaseController::SetIsClearingBootAnimation(bool isClearingBootAnimation)
{
    FoldScreenBasePolicy::GetInstance().SetIsClearingBootAnimation(isClearingBootAnimation);
}

void FoldScreenBaseController::NotifyRunSensorFoldStateManager()
{
    TLOGI(WmsLogTag::DMS, "TaskSequenceProcess FoldScreenBaseController::NotifyRunSensorFoldStateManager");
    DMS::SensorFoldStateMgr::GetInstance().FinishTaskSequence();
}
} // namespace OHOS::Rosen