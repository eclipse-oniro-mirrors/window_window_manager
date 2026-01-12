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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SINGLE_DISPLAY_POCKET_FOLD_POLICY_H
#define OHOS_ROSEN_WINDOW_SCENE_SINGLE_DISPLAY_POCKET_FOLD_POLICY_H

#include <refbase.h>

#include "common/include/task_scheduler.h"
#include "fold_screen_controller/fold_screen_policy.h"
#include "fold_screen_info.h"
#include "session/screen/include/screen_session.h"

namespace OHOS::Rosen {
class SingleDisplayPocketFoldPolicy : public FoldScreenPolicy {
public:
    SingleDisplayPocketFoldPolicy(std::recursive_mutex& displayInfoMutex,
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler);
    ~SingleDisplayPocketFoldPolicy() = default;
    void ChangeScreenDisplayMode(FoldDisplayMode displayMode,
        DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT) override;
    void SendSensorResult(FoldStatus foldStatus) override;
    sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion() override;
    FoldCreaseRegion GetLiveCreaseRegion() override;
    void LockDisplayStatus(bool locked) override;
    void SetOnBootAnimation(bool onBootAnimation) override;
    void UpdateForPhyScreenPropertyChange() override;
    void ExitCoordination() override;
    void AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command) override;
    FoldDisplayMode GetModeMatchStatus() override;
    FoldDisplayMode GetModeMatchStatus(FoldStatus targetFoldStatus) override;
    void BootAnimationFinishPowerInit() override;
    void GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems) const override;
private:
    void ChangeScreenDisplayModeToMain(sptr<ScreenSession> screenSession,
        DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT);
    void ChangeScreenDisplayModeToFull(sptr<ScreenSession> screenSession,
        DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT);
    void ChangeScreenDisplayModeToCoordination();
    void ChangeScreenDisplayModeProc(sptr<ScreenSession> screenSession, FoldDisplayMode displayMode,
        DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT);
    void ChangeScreenDisplayModeToMainWhenFoldScreenOn(sptr<ScreenSession> screenSession);
    void ChangeScreenDisplayModeToMainWhenFoldScreenOff(sptr<ScreenSession> screenSession);
    void ChangeScreenDisplayModeToFullWhenFoldScreenOn(sptr<ScreenSession> screenSession);
    void ChangeScreenDisplayModeToFullWhenFoldScreenOff(sptr<ScreenSession> screenSession,
        DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT);
    void ChangeScreenDisplayModeToMainOnBootAnimation(sptr<ScreenSession> screenSession);
    void ChangeScreenDisplayModeToFullOnBootAnimation(sptr<ScreenSession> screenSession);
    void ChangeScreenDisplayModePower(ScreenId screenId, ScreenPowerStatus screenPowerStatus);
    void RecoverWhenBootAnimationExit();
    void ReportFoldDisplayModeChange(FoldDisplayMode displayMode);
    void ReportFoldStatusChangeBegin(int32_t offScreen, int32_t onScreen);
    void SendPropertyChangeResult(sptr<ScreenSession> screenSession, ScreenId screenId,
        ScreenPropertyChangeReason reason);
    void SetdisplayModeChangeStatus(bool status, bool isOnBootAnimation = false) override;
    void ChangeOnTentMode(FoldStatus currentState) override;
    void ChangeOffTentMode() override;
    void CloseCoordinationScreen();
    void NotifyRefreshRateEvent(bool isEventStatus);
    FoldCreaseRegion GetFoldCreaseRegion(bool isVertical) const;
    void GetFoldCreaseRect(bool isVertical, const std::vector<int32_t>& foldRect,
        std::vector<DMRect>& foldCreaseRect) const;
    std::recursive_mutex& displayInfoMutex_;
    std::mutex coordinationMutex_;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler_;
};
} // namespace OHOS::Rosen
#endif //OHOS_ROSEN_WINDOW_SCENE_SINGLE_DISPLAY_FOLD_POLICY_H