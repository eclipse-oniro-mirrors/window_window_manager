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
#ifndef OHOS_ROSEN_WINDOW_SCENE_DUAL_DISPLAY_POLICY_H
#define OHOS_ROSEN_WINDOW_SCENE_DUAL_DISPLAY_POLICY_H

#include <refbase.h>

#include "common/include/task_scheduler.h"
#include "fold_screen_controller/fold_screen_policy.h"
#include "fold_screen_info.h"
#include "session/screen/include/screen_session.h"

namespace OHOS::Rosen {
class DualDisplayFoldPolicy : public FoldScreenPolicy {
public:
    DualDisplayFoldPolicy(std::recursive_mutex& displayInfoMutex,
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler);
    ~DualDisplayFoldPolicy() = default;
    void ChangeScreenDisplayMode(FoldDisplayMode displayMode) override;
    void SendSensorResult(FoldStatus foldStatus) override;
    sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion() override;
    void LockDisplayStatus(bool locked) override;
    void SetOnBootAnimation(bool onBootAnimation) override;
    void UpdateForPhyScreenPropertyChange() override;

private:
    void ChangeScreenDisplayModeInner(sptr<ScreenSession> screenSession, ScreenId offScreenId, ScreenId onScreenId);
    void ChangeScreenDisplayModeOnBootAnimation(sptr<ScreenSession> screenSession, ScreenId screenId);
    void ChangeScreenDisplayModeToCoordination();
    void RecoverWhenBootAnimationExit();
    void TriggerScreenDisplayModeUpdate(FoldDisplayMode displayMode);
    FoldDisplayMode GetModeMatchStatus();
    void ReportFoldDisplayModeChange(FoldDisplayMode displayMode);
    void ReportFoldStatusChangeBegin(int32_t offScreen, int32_t onScreen);
    void SendPropertyChangeResult(sptr<ScreenSession> screenSession, ScreenId screenId,
        ScreenPropertyChangeReason reason);
    void AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command);
    std::recursive_mutex& displayInfoMutex_;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler_;
};
} // namespace OHOS::Rosen
#endif //OHOS_ROSEN_WINDOW_SCENE_DUAL_DISPLAY_DEVICE_POLICY_H