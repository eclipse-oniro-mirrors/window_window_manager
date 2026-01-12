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

#include "fold_screen_controller/fold_screen_policy.h"
#include "window_manager_hilog.h"
#include "screen_session_manager.h"

namespace OHOS::Rosen {
    const uint32_t MODE_CHANGE_TIMEOUT_MS = 2000;
    static const std::unordered_set<FoldStatus> SUPPORTED_FOLD_STATUS = {
        FoldStatus::EXPAND,
        FoldStatus::FOLDED,
        FoldStatus::HALF_FOLD
};
FoldScreenPolicy::FoldScreenPolicy() = default;
FoldScreenPolicy::~FoldScreenPolicy() = default;

void FoldScreenPolicy::ChangeScreenDisplayMode(FoldDisplayMode displayMode, DisplayModeChangeReason reason) {}

void FoldScreenPolicy::LockDisplayStatus(bool locked)
{
    lockDisplayStatus_ = locked;
}

bool FoldScreenPolicy::IsFoldStatusSupported(const std::unordered_set<FoldStatus>& supportedFoldStatus,
    FoldStatus targetFoldStatus) const
{
    return supportedFoldStatus.find(targetFoldStatus) != supportedFoldStatus.end();
}

bool FoldScreenPolicy::GetPhysicalFoldLockFlag() const
{
    return physicalFoldLockFlag_.load(std::memory_order_relaxed);
}

FoldStatus FoldScreenPolicy::GetFoldStatus()
{
    if (!GetPhysicalFoldLockFlag()) {
        return lastFoldStatus_;
    }
    return GetForceFoldStatus();
}

void FoldScreenPolicy::SendSensorResult(FoldStatus foldStatus) {}

ScreenId FoldScreenPolicy::GetCurrentScreenId()
{
    return screenId_;
}
sptr<FoldCreaseRegion> FoldScreenPolicy::GetCurrentFoldCreaseRegion()
{
    return currentFoldCreaseRegion_;
}

FoldCreaseRegion FoldScreenPolicy::GetLiveCreaseRegion()
{
    std::lock_guard<std::mutex> lock_mode(liveCreaseRegionMutex_);
    return liveCreaseRegion_;
}

void FoldScreenPolicy::SetOnBootAnimation(bool onBootAnimation)
{
    onBootAnimation_ = onBootAnimation;
}

void FoldScreenPolicy::UpdateForPhyScreenPropertyChange() {}

FoldDisplayMode FoldScreenPolicy::GetScreenDisplayMode()
{
    std::lock_guard<std::recursive_mutex> lock_mode(displayModeMutex_);
    return lastDisplayMode_;
}

FoldStatus FoldScreenPolicy::GetPhysicalFoldStatus()
{
    return lastFoldStatus_;
}

FoldStatus FoldScreenPolicy::GetForceFoldStatus() const
{
    return forceFoldStatus_.load(std::memory_order_relaxed);
}

void FoldScreenPolicy::SetFoldLockFlagAndFoldStatus(bool physicalFoldLockFlag, FoldStatus targetFoldStatus)
{
    TLOGI(WmsLogTag::DMS, "Set physicalFoldLockFlag as %{public}d, forceFoldStatus as %{public}d",
        physicalFoldLockFlag, targetFoldStatus);
    physicalFoldLockFlag_.store(physicalFoldLockFlag, std::memory_order_relaxed);
    forceFoldStatus_.store(targetFoldStatus, std::memory_order_relaxed);
}

DMError FoldScreenPolicy::SetFoldStatusAndLockControl(bool isLocked, FoldStatus targetFoldStatus)
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

std::chrono::steady_clock::time_point FoldScreenPolicy::GetStartTimePoint()
{
    return startTimePoint_;
}

bool FoldScreenPolicy::GetIsFirstFrameCommitReported()
{
    return isFirstFrameCommitReported_;
}

void FoldScreenPolicy::SetIsFirstFrameCommitReported(bool isFirstFrameCommitReported)
{
    isFirstFrameCommitReported_ = isFirstFrameCommitReported;
}

void FoldScreenPolicy::ClearState()
{
    currentDisplayMode_ = FoldDisplayMode::UNKNOWN;
    currentFoldStatus_ = FoldStatus::UNKNOWN;
}

void FoldScreenPolicy::SetFoldStatus(FoldStatus foldStatus)
{
    TLOGI(WmsLogTag::DMS, "SetFoldStatus: %{public}d", foldStatus);
    currentFoldStatus_ = foldStatus;
    lastFoldStatus_ = foldStatus;
}

void FoldScreenPolicy::ExitCoordination() {};

void FoldScreenPolicy::ChangeOnTentMode(FoldStatus currentState) {}

void FoldScreenPolicy::ChangeOffTentMode() {}

bool FoldScreenPolicy::GetModeChangeRunningStatus()
{
    TLOGE(WmsLogTag::DMS, "GetdisplayModeRunningStatus: %{public}d", GetdisplayModeRunningStatus());
    auto currentTime = std::chrono::steady_clock::now();
    auto intervalMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTimePoint_).count();
    if (intervalMs > MODE_CHANGE_TIMEOUT_MS) {
        TLOGE(WmsLogTag::DMS, "mode change timeout.");
        return false;
    }
    return GetdisplayModeRunningStatus();
}

bool FoldScreenPolicy::GetdisplayModeRunningStatus()
{
    return displayModeChangeRunning_.load();
}

FoldDisplayMode FoldScreenPolicy::GetLastCacheDisplayMode()
{
    return lastCachedisplayMode_.load();
}

void FoldScreenPolicy::SetLastCacheDisplayMode(FoldDisplayMode mode)
{
    lastCachedisplayMode_ = mode;
}

int64_t FoldScreenPolicy::getFoldingElapsedMs()
{
    if (endTimePoint_ < startTimePoint_) {
        TLOGE(WmsLogTag::DMS, "invalid timepoint. endTimePoint less startTimePoint");
        return 0;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTimePoint_ - startTimePoint_).count();
    return static_cast<int64_t>(elapsed);
}

void FoldScreenPolicy::AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command) {};

Drawing::Rect FoldScreenPolicy::GetScreenSnapshotRect()
{
    Drawing::Rect snapshotRect = {0, 0, 0, 0};
    return snapshotRect;
}

void FoldScreenPolicy::SetMainScreenRegion(DMRect& mainScreenRegion) {}

void FoldScreenPolicy::SetIsClearingBootAnimation(bool isClearingBootAnimation) {}

void FoldScreenPolicy::GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems) const {}

const std::unordered_set<FoldStatus>& FoldScreenPolicy::GetSupportedFoldStatus() const
{
    return SUPPORTED_FOLD_STATUS;
}
} // namespace OHOS::Rosen