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

#ifndef OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_CONTROLLER_H
#define OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_CONTROLLER_H

#include <refbase.h>

#include "common/include/task_scheduler.h"
#include "../infra/include/fold_screen/fold_screen_base_controller.h"
#include "fold_screen_controller/fold_screen_policy.h"
#include "fold_screen_controller/fold_screen_state_machine.h"
#include "fold_screen_controller/sensor_fold_state_manager/sensor_fold_state_manager.h"
#include "fold_screen_info.h"

namespace OHOS::Rosen {
enum class DisplayDeviceType :uint32_t {
    DISPLAY_DEVICE_UNKNOWN = 0,
    SINGLE_DISPLAY_DEVICE,
    DOUBLE_DISPLAY_DEVICE,
    SINGLE_DISPLAY_POCKET_DEVICE,
    SECONDARY_DISPLAY_DEVICE,
    SINGLE_DISPLAY_SUPER_DEVICE,
};

class FoldScreenController : public DMS::FoldScreenBaseController {
public:
    FoldScreenController(std::recursive_mutex& displayInfoMutex,
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler);
    virtual ~FoldScreenController();

    void BootAnimationFinishPowerInit() override;
    void SetDisplayMode(const FoldDisplayMode displayMode) override;
    void RecoverDisplayMode() override;
    FoldDisplayMode GetDisplayMode() override;
    bool IsFoldable() override;
    FoldStatus GetFoldStatus() override;
    bool GetTentMode() override;
    FoldDisplayMode GetModeMatchStatus() override;
    void SetFoldStatus(FoldStatus foldStatus) override;
    void OnTentModeChanged(int tentType, int32_t hall = -1) override;
    sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion() override;
    FoldCreaseRegion GetLiveCreaseRegion() const override;
    ScreenId GetCurrentScreenId() override;
    void LockDisplayStatus(bool locked) override;
    DMError ForceSetFoldStatusAndLock(FoldStatus targetFoldStatus) override;
    DMError RestorePhysicalFoldStatus() override;
    bool GetPhysicalFoldLockFlag() const override;
    FoldStatus GetPhysicalFoldStatus() const override;
    void SetOnBootAnimation(bool onBootAnimation) override;
    void UpdateForPhyScreenPropertyChange() override;
    void ExitCoordination() override;
    Drawing::Rect GetScreenSnapshotRect() override;
    void SetMainScreenRegion(DMRect& mainScreenRegion) override;
    std::chrono::steady_clock::time_point GetStartTimePoint() override;
    bool GetIsFirstFrameCommitReported() override;
    void SetIsFirstFrameCommitReported(bool isFirstFrameCommitReported) override;
    /*
     *    Avoid fold to expand process queues public interface
     */
    bool GetModeChangeRunningStatus() override;
    void SetdisplayModeChangeStatus(bool status) override;
    bool GetdisplayModeRunningStatus() override;
    FoldDisplayMode GetLastCacheDisplayMode() override;
    void AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command) override;
    void SetIsClearingBootAnimation(bool isClearingBootAnimation) override;
    nlohmann::ordered_json GetFoldCreaseRegionJson() override;
    void NotifyRunSensorFoldStateManager() override;
private:
    sptr<FoldScreenPolicy> GetFoldScreenPolicy(DisplayDeviceType productType);
    sptr<FoldScreenPolicy> foldScreenPolicy_;
    sptr<SensorFoldStateManager> sensorFoldStateManager_;
    std::recursive_mutex& displayInfoMutex_;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler_;
    std::vector<FoldCreaseRegionItem> foldCreaseRegionItems_;
};
} // namespace OHOS::Rosen
#endif //OHOS_ROSEN_WINDOW_SCENE_FOLD_SCREEN_CONTROLLER_H