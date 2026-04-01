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

#ifndef OHOS_ROSEN_DUAL_DISPLAY_FOLD_STATE_SENSOR_MANAGER_H
#define OHOS_ROSEN_DUAL_DISPLAY_FOLD_STATE_SENSOR_MANAGER_H

#include "common/include/task_scheduler.h"
#include "dm_common.h"
#include "fold_screen_controller/fold_screen_policy.h"
#include "fold_screen_controller/sensor_fold_state_manager/sensor_fold_state_manager.h"
#include "iapplication_state_observer.h"
#include "ffrt_queue.h"

namespace OHOS {
namespace Rosen {
using OHOS::AppExecFwk::AppStateData;
using OHOS::AppExecFwk::IApplicationStateObserver;
enum class ReportDualTentModeStatus : int32_t {
    NORMAL_EXIT_TENT_MODE = 0,
    NORMAL_ENTER_TENT_MODE = 1,
    ABNORMAL_EXIT_TENT_MODE_DUE_TO_ANGLE = 2,
    ABNORMAL_EXIT_TENT_MODE_DUE_TO_HALL = 3,
};
class ApplicationStateObserver : public IApplicationStateObserver {
public:
    ApplicationStateObserver();
    virtual ~ApplicationStateObserver() = default;
    void OnForegroundApplicationChanged(const AppStateData &appStateData) override;
    void OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override {};
    void OnExtensionStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override {};
    void OnProcessCreated(const AppExecFwk::ProcessData &processData) override {};
    void OnProcessDied(const AppExecFwk::ProcessData &processData) override {};
    void OnApplicationStateChanged(const AppExecFwk::AppStateData &appStateData) override {};
    sptr<IRemoteObject> AsObject() override { return nullptr; };
    std::string GetForegroundApp();

private:
    std::string foregroundBundleName_ {""};
};

class DualDisplaySensorFoldStateManager : public SensorFoldStateManager {
public:
    explicit DualDisplaySensorFoldStateManager(const std::shared_ptr<TaskScheduler>& screenPowerTaskScheduler);
    virtual ~DualDisplaySensorFoldStateManager();

    void HandleAngleChange(float angle, int hall, sptr<FoldScreenPolicy> foldScreenPolicy) override;
    void HandleHallChange(float angle, int hall, sptr<FoldScreenPolicy> foldScreenPolicy) override;
    void RegisterApplicationStateObserver() override;
    void HandleTentChange(int tentType, sptr<FoldScreenPolicy> foldScreenPolicy, int hall = -1) override;
    bool TriggerTentExit(float angle, int hall);
    void TentModeHandleSensorChange(float angle, int hall, sptr<FoldScreenPolicy> foldScreenPolicy);
    void ReportTentStatusChange(ReportDualTentModeStatus tentStatus);

private:
    FoldStatus GetNextFoldState(float angle, int hall);
    void UpdateHallSwitchAppInfo(FoldStatus foldStatus);
    void HandleHallChangeInner(float angle, int hall, const sptr<FoldScreenPolicy>& foldScreenPolicy);
    void SensorReportTimeOutPro(float angle, int hall, const sptr<FoldScreenPolicy>& foldScreenPolicy);
    bool CheckUpdateAngle(float& angle, int hall);
    void HandleAngleChangeInTask(float angle, int hall, const sptr<FoldScreenPolicy>& foldScreenPolicy);
    sptr<ApplicationStateObserver> applicationStateObserver_;
    bool isHallSwitchApp_ = true;
    std::vector<std::string> packageNames_;
    int tentModeType_ = 0;
    float currentAngle_ = -1.0F;
    int32_t currentHall_ = -1;
    std::atomic_bool isInTask_ = false;
    std::shared_ptr<TaskScheduler> screenPowerTaskScheduler_;
    std::shared_ptr<DMS::FfrtQueue> ffrtQueue_ = nullptr;
};
} // namespace Rosen
} // namespace OHOS
#endif