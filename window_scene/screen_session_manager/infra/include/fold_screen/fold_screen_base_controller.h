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
 
#ifndef OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_BASE_CONTROLLER_H
#define OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_BASE_CONTROLLER_H
 
#include <refbase.h>
 
#include "common/include/task_scheduler.h"
#include "fold_screen_base_policy.h"
#include "fold_screen_controller/fold_screen_state_machine.h"
#include "fold_screen_controller/sensor_fold_state_manager/sensor_fold_state_manager.h"
#include "fold_screen_info.h"

namespace OHOS::Rosen::DMS {
class FoldScreenBaseController : public RefBase {
public:
    FoldScreenBaseController();
    virtual ~FoldScreenBaseController();

    virtual void BootAnimationFinishPowerInit();
    virtual void SetDisplayMode(const FoldDisplayMode displayMode);
    virtual void RecoverDisplayMode();
    virtual FoldDisplayMode GetDisplayMode();
    virtual bool IsFoldable();
    virtual FoldStatus GetFoldStatus();
    virtual bool GetTentMode();
    virtual FoldDisplayMode GetModeMatchStatus();
    virtual void SetFoldStatus(FoldStatus foldStatus);
    virtual void OnTentModeChanged(int tentType, int32_t hall = -1);
    virtual sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion();
    virtual FoldCreaseRegion GetLiveCreaseRegion() const;
    virtual ScreenId GetCurrentScreenId();
    virtual void LockDisplayStatus(bool locked);
    virtual DMError ForceSetFoldStatusAndLock(FoldStatus targetFoldStatus);
    virtual DMError RestorePhysicalFoldStatus();
    virtual bool GetPhysicalFoldLockFlag() const;
    virtual FoldStatus GetPhysicalFoldStatus() const;
    virtual void SetOnBootAnimation(bool onBootAnimation);
    virtual void UpdateForPhyScreenPropertyChange();
    virtual void ExitCoordination();
    virtual void AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command);
    virtual Drawing::Rect GetScreenSnapshotRect();
    virtual void SetMainScreenRegion(DMRect& mainScreenRegion);
    virtual std::chrono::steady_clock::time_point GetStartTimePoint();
    virtual bool GetIsFirstFrameCommitReported();
    virtual void SetIsFirstFrameCommitReported(bool isFirstFrameCommitReported);
    /*
     *    Avoid fold to expand process queues public interface
     */
    virtual bool GetModeChangeRunningStatus();
    virtual void SetdisplayModeChangeStatus(bool status);
    virtual bool GetdisplayModeRunningStatus();
    virtual FoldDisplayMode GetLastCacheDisplayMode();
    virtual void SetIsClearingBootAnimation(bool isClearingBootAnimation);
    virtual nlohmann::ordered_json GetFoldCreaseRegionJson();
    virtual void NotifyRunSensorFoldStateManager();
private:
    std::vector<FoldCreaseRegionItem> foldCreaseRegionItems_;
};
} // namespace OHOS::Rosen
#endif //OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_BASE_CONTROLLER_H