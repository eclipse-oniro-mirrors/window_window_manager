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

#ifndef OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_POLICY_H
#define OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_POLICY_H

#include <refbase.h>
#include <mutex>

#include "dm_common.h"
#include "window_manager_hilog.h"
#include "session/screen/include/screen_property.h"
#include "fold_screen_info.h"

namespace OHOS::Rosen {
namespace {
    const uint32_t FOLD_TO_EXPAND_ONBOOTANIMATION_TASK_NUM = 1;
    const uint32_t FOLD_TO_EXPAND_TASK_NUM = 3;
    constexpr uint32_t SECONDARY_FOLD_TO_EXPAND_TASK_NUM = 3;
}

class FoldScreenPolicy : public RefBase {
public:
    FoldScreenPolicy();
    virtual ~FoldScreenPolicy();

    virtual void ChangeScreenDisplayMode(FoldDisplayMode displayMode,
        DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT);
    virtual void SendSensorResult(FoldStatus foldStatus);
    virtual ScreenId GetCurrentScreenId();
    virtual sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion();
    virtual FoldCreaseRegion GetLiveCreaseRegion();
    virtual void LockDisplayStatus(bool locked);
    virtual void SetOnBootAnimation(bool onBootAnimation);
    virtual void UpdateForPhyScreenPropertyChange();
    virtual void ExitCoordination();
    virtual void AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command);
    virtual FoldDisplayMode GetModeMatchStatus() { return FoldDisplayMode::UNKNOWN; }
    virtual FoldDisplayMode GetModeMatchStatus(FoldStatus targetFoldStatus) { return FoldDisplayMode::UNKNOWN; }
    virtual void BootAnimationFinishPowerInit() {};
    virtual void ChangeOnTentMode(FoldStatus currentState);
    virtual void ChangeOffTentMode();
    virtual Drawing::Rect GetScreenSnapshotRect();
    virtual void SetMainScreenRegion(DMRect& mainScreenRegion);
    virtual void SetIsClearingBootAnimation(bool isClearingBootAnimation);
    virtual void GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems) const;
    virtual const std::unordered_set<FoldStatus>& GetSupportedFoldStatus() const;
    virtual bool GetPhysicalFoldLockFlag() const;
    virtual FoldStatus GetForceFoldStatus() const;
    virtual DMError SetFoldStatusAndLockControl(bool isLocked, FoldStatus targetFoldStatus = FoldStatus::UNKNOWN);
    virtual void SetFoldLockFlagAndFoldStatus(bool physicalFoldLockFlag, FoldStatus targetFoldStatus);
    virtual bool IsFoldStatusSupported(const std::unordered_set<FoldStatus>& supportedFoldStatus,
        FoldStatus targetFoldStatus) const;
    FoldDisplayMode GetScreenDisplayMode();
    virtual FoldStatus GetFoldStatus();
    virtual FoldStatus GetPhysicalFoldStatus();
    std::chrono::steady_clock::time_point GetStartTimePoint();
    void ClearState();
    void SetFoldStatus(FoldStatus foldStatus);
    void SetIsFirstFrameCommitReported(bool isFirstFrameCommitReported);
    bool GetIsFirstFrameCommitReported();

    ScreenId screenId_ { SCREEN_ID_INVALID };
    ScreenProperty screenProperty_;
    mutable std::recursive_mutex displayModeMutex_;
    mutable std::mutex liveCreaseRegionMutex_;
    FoldDisplayMode currentDisplayMode_ = FoldDisplayMode::UNKNOWN;
    FoldStatus currentFoldStatus_ = FoldStatus::UNKNOWN;
    FoldDisplayMode lastDisplayMode_ = FoldDisplayMode::UNKNOWN;
    FoldStatus lastFoldStatus_ = FoldStatus::UNKNOWN;
    sptr<FoldCreaseRegion> currentFoldCreaseRegion_ = nullptr;
    FoldCreaseRegion liveCreaseRegion_ = FoldCreaseRegion(0, {});
    bool lockDisplayStatus_ = false;
    std::atomic<bool> physicalFoldLockFlag_ = false;
    std::atomic<FoldStatus> forceFoldStatus_ = FoldStatus::UNKNOWN;
    bool onBootAnimation_ = false;
    std::atomic<bool> isClearingBootAnimation_ = false;
    bool isFirstFrameCommitReported_ = false;
    std::vector<uint32_t> screenParams_ = {};
    /*
     *    Avoid fold to expand process queues public interface
     */
    bool GetModeChangeRunningStatus();
    virtual void SetdisplayModeChangeStatus(bool status, bool isOnBootAnimation = false){};
    virtual void SetSecondaryDisplayModeChangeStatus(bool status){};
    bool GetdisplayModeRunningStatus();
    FoldDisplayMode GetLastCacheDisplayMode();
    virtual std::vector<uint32_t> GetScreenParams() { return screenParams_; };

protected:
    /*
     *    Avoid fold to expand process queues private variable
     */
    std::atomic<int> pengdingTask_{FOLD_TO_EXPAND_TASK_NUM};
    std::atomic<int> secondaryPengdingTask_{SECONDARY_FOLD_TO_EXPAND_TASK_NUM};
    std::atomic<bool> displayModeChangeRunning_ = false;
    std::atomic<FoldDisplayMode> lastCachedisplayMode_ = FoldDisplayMode::UNKNOWN;
    std::chrono::steady_clock::time_point startTimePoint_ = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point endTimePoint_ = std::chrono::steady_clock::now();
    void SetLastCacheDisplayMode(FoldDisplayMode mode);
    int64_t getFoldingElapsedMs();
};
} // namespace OHOS::Rosen
#endif //OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_POLICY_H