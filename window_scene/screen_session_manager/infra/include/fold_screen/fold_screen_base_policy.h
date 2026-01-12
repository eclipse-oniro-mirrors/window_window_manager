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

#ifndef OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_BASE_POLICY_H
#define OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_BASE_POLICY_H

#include <mutex>

#include "dm_common.h"
#include "session/screen/include/screen_property.h"
#include "common/include/task_scheduler.h"
#include "session/screen/include/screen_session.h"
#include "fold_screen_controller/fold_screen_policy.h"
#include "fold_screen_info.h"

namespace OHOS::Rosen::DMS {
namespace {
    static const uint32_t FOLD_TASK_NUM = 3;
    static const uint32_t FOLD_TASK_NUM_ONBOOTANIMATION = 1;
}
class FoldScreenBasePolicy {
public:
    static FoldScreenBasePolicy& GetInstance();
    virtual ScreenId GetCurrentScreenId();
    virtual Drawing::Rect GetScreenSnapshotRect();

    void ClearState();
    FoldDisplayMode GetScreenDisplayMode();
    virtual FoldStatus GetFoldStatus();
    void SetFoldStatus(FoldStatus foldStatus);
    std::chrono::steady_clock::time_point GetStartTimePoint();
    bool GetIsFirstFrameCommitReported();
    void SetIsFirstFrameCommitReported(bool isFirstFrameCommitReported);

    // Avoid fold to expand process queues public interface
    virtual bool GetModeChangeRunningStatus();
    virtual void SetSecondaryDisplayModeChangeStatus(bool status){};
    bool GetdisplayModeRunningStatus();
    FoldDisplayMode GetLastCacheDisplayMode();
    virtual std::vector<uint32_t> GetScreenParams() { return screenParams_; };

    // tentMode
    void ChangeOnTentMode(FoldStatus currentState);
    void ChangeOffTentMode();
    // coordinationMode
    void ChangeScreenDisplayModeToCoordination();
    void CloseCoordinationScreen();
    void ExitCoordination();
    void NotifyRefreshRateEvent(bool isEventStatus);
    void AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command);
    // report
    void ReportFoldDisplayModeChange(FoldDisplayMode displayMode);
    void ReportFoldStatusChangeBegin(int32_t offScreen, int32_t onScreen);
    // bootAnimation
    void SetOnBootAnimation(bool onBootAnimation);
    void RecoverWhenBootAnimationExit();
    void UpdateForPhyScreenPropertyChange();
    void ChangeScreenDisplayModeToMainOnBootAnimation(sptr<ScreenSession> screenSession);
    void ChangeScreenDisplayModeToFullOnBootAnimation(sptr<ScreenSession> screenSession);
    virtual void SetIsClearingBootAnimation(bool isClearingBootAnimation);
    virtual void BootAnimationFinishPowerInit() {};
    //fold or expand
    bool CheckDisplayModeChange(FoldDisplayMode displayMode, bool isForce,
        DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT);
    void ChangeScreenDisplayMode(FoldDisplayMode displayMode, bool isForce,
        DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT);
    virtual void ChangeScreenDisplayMode(FoldDisplayMode displayMode,
        DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT);
    void SendSensorResult(FoldStatus foldStatus);
    void UpdateDeviceStatus(FoldDisplayMode displayMode);
    void ChangeScreenDisplayModeInner(FoldDisplayMode displayMode, DisplayModeChangeReason reason);
    void ChangeScreenDisplayModeToMain(sptr<ScreenSession> screenSession,
        DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT);
    void ChangeScreenDisplayModeToFull(sptr<ScreenSession> screenSession,
        DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT);
    void ChangeScreenDisplayModeToMainWhenFoldScreenOn(sptr<ScreenSession> screenSession);
    void ChangeScreenDisplayModeToMainWhenFoldScreenOff(sptr<ScreenSession> screenSession);
    void ChangeScreenDisplayModeToFullWhenFoldScreenOn(sptr<ScreenSession> screenSession);
    void ChangeScreenDisplayModeToFullWhenFoldScreenOff(sptr<ScreenSession> screenSession,
        DisplayModeChangeReason reason);
    void ChangeScreenDisplayModePower(ScreenId screenId, ScreenPowerStatus screenPowerStatus);
    void SendPropertyChangeResult(sptr<ScreenSession> screenSession, ScreenId screenId,
        ScreenPropertyChangeReason reason);
    void SetdisplayModeChangeStatus(bool status, bool isOnBootAnimation = false);
    // common
    void LockDisplayStatus(bool locked);
    virtual FoldDisplayMode GetModeMatchStatus();
    virtual FoldDisplayMode GetModeMatchStatus(FoldStatus targetFoldStatus);
    // FoldCreaseRegion
    sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion();
    FoldCreaseRegion GetLiveCreaseRegion() const;
    void GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems) const;
    virtual void SetMainScreenRegion(DMRect& mainScreenRegion) {};
    bool GetLockDisplayStatus() const;
    void SetCurrentDisplayMode(FoldDisplayMode mode);
    // Lock target fold status
    virtual const std::unordered_set<FoldStatus>& GetSupportedFoldStatus() const;
    virtual bool GetPhysicalFoldLockFlag() const;
    virtual FoldStatus GetForceFoldStatus() const;
    virtual DMError SetFoldStatusAndLockControl(bool isLocked, FoldStatus targetFoldStatus = FoldStatus::UNKNOWN);
    virtual void SetFoldLockFlagAndFoldStatus(bool physicalFoldLockFlag, FoldStatus targetFoldStatus);
    virtual FoldStatus GetPhysicalFoldStatus();
    bool IsFoldStatusSupported(const std::unordered_set<FoldStatus>& supportedFoldStatus,
        FoldStatus targetFoldStatus) const;

protected:
    FoldScreenBasePolicy();
    virtual ~FoldScreenBasePolicy();

    // Avoid fold to expand process queues private variable
    std::atomic<int> pendingTask_{FOLD_TASK_NUM};
    std::atomic<bool> displayModeChangeRunning_ = false;
    std::atomic<FoldDisplayMode> lastCachedisplayMode_ = FoldDisplayMode::UNKNOWN;
    std::chrono::steady_clock::time_point startTimePoint_ = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point endTimePoint_ = std::chrono::steady_clock::now();
    void SetLastCacheDisplayMode(FoldDisplayMode mode);
    int64_t getFoldingElapsedMs();

    std::atomic<bool> physicalFoldLockFlag_ = false;
    std::atomic<FoldStatus> forceFoldStatus_ = FoldStatus::UNKNOWN;

    std::mutex coordinationMutex_;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler_;

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
    bool onBootAnimation_ = false;
    std::atomic<bool> isClearingBootAnimation_ = false;
    bool isFirstFrameCommitReported_ = false;
    std::vector<uint32_t> screenParams_ = {};
};
} // namespace OHOS::Rosen
#endif //OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_BASE_POLICY_H