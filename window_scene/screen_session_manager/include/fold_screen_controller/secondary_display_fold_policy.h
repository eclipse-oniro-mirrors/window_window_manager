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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SECONDARY_DISPLAY_FOLD_POLICY_H
#define OHOS_ROSEN_WINDOW_SCENE_SECONDARY_DISPLAY_FOLD_POLICY_H

#include <refbase.h>

#include "common/include/task_scheduler.h"
#include "fold_screen_controller/fold_screen_policy.h"
#include "fold_screen_info.h"
#include "session/screen/include/screen_session.h"

namespace OHOS::Rosen {
class SecondaryDisplayFoldPolicy : public FoldScreenPolicy {
public:
    SecondaryDisplayFoldPolicy(std::recursive_mutex& displayInfoMutex,
        std::shared_ptr<TaskScheduler> screenPowerTaskScheduler);
    ~SecondaryDisplayFoldPolicy() = default;
    void ChangeScreenDisplayMode(FoldDisplayMode displayMode,
        DisplayModeChangeReason reason = DisplayModeChangeReason::DEFAULT) override;
    void SendSensorResult(FoldStatus foldStatus) override;
    sptr<FoldCreaseRegion> GetCurrentFoldCreaseRegion() override;
    FoldCreaseRegion GetLiveCreaseRegion() override;
    void SetOnBootAnimation(bool onBootAnimation) override;
    FoldDisplayMode GetModeMatchStatus() override;
    FoldDisplayMode GetModeMatchStatus(FoldStatus targetFoldStatus) override;
    std::vector<uint32_t> GetScreenParams() override;
    void ExitCoordination() override;
    void SetSecondaryDisplayModeChangeStatus(bool status) override;
    void GetAllCreaseRegion(std::vector<FoldCreaseRegionItem>& foldCreaseRegionItems) const override;
    void AddOrRemoveDisplayNodeToTree(ScreenId screenId, int32_t command) override;
    const std::unordered_set<FoldStatus>& GetSupportedFoldStatus() const override;
private:
    void HandlePropertyChange(sptr<ScreenSession> screenSession, ScreenProperty& ScreenProperty,
        ScreenPropertyChangeReason reason, FoldDisplayMode displayMode, bool isNeedNotifyFoldProperty);
    void ChangeSuperScreenDisplayMode(sptr<ScreenSession> screenSession,
        FoldDisplayMode displayMode);
    void CloseCoordinationScreen();
    void ChangeScreenDisplayModeToCoordination();
    void UpdateDisplayNodeBasedOnScreenId(ScreenId screenId, std::shared_ptr<RSDisplayNode> displayNode);
    void InitPositionZInfos();
    void UpdatePositionZForDualDisplayNode();
    void RecoverWhenBootAnimationExit();
    void ReportFoldDisplayModeChange(FoldDisplayMode displayMode);
    void SendPropertyChangeResult(sptr<ScreenSession> screenSession, ScreenId screenId,
        ScreenPropertyChangeReason reason, FoldDisplayMode displayMode);
    void SetStatusConditionalActiveRectAndTpFeature(ScreenProperty &screenProperty);
    void SetStatusFullActiveRectAndTpFeature(const sptr<ScreenSession>& screenSession, ScreenProperty& screenProperty,
        bool isNeedToSetSwitch = true);
    void SetStatusMainActiveRectAndTpFeature(const sptr<ScreenSession>& screenSession, ScreenProperty &screenProperty);
    void SetStatusGlobalFullActiveRectAndTpFeature(const sptr<ScreenSession>& screenSession,
        ScreenProperty &screenProperty);
    void InitScreenParams();
    FoldCreaseRegion GetStatusFullFoldCreaseRegion(bool isVertical) const;
    void GetStatusFullFoldCreaseRect(bool isVertical, const std::vector<int32_t>& foldRect,
        std::vector<DMRect>& foldCreaseRect) const;
    FoldCreaseRegion GetStatusGlobalFullFoldCreaseRegion(bool isVertical) const;
    void GetStatusGlobalFullFoldCreaseRect(bool isVertical, const std::vector<int32_t>& foldRect,
        std::vector<DMRect>& foldCreaseRect) const;
    std::recursive_mutex& displayInfoMutex_;
    std::mutex coordinationMutex_;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler_;
    std::vector<uint32_t> screenParams_;
    bool isChangeScreenWhenBootCompleted = false;
    std::map<ScreenId, float> dualDisplayNodePositionZ_;
};
} // namespace OHOS::Rosen
#endif //OHOS_ROSEN_WINDOW_SCENE_SECONDARY_DISPLAY_FOLD_POLICY_H
