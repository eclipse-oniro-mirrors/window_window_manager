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

#ifndef OHOS_ROSEN_SINGLE_DISPLAY_POCKET_FOLD_STATE_SENSOR_MANAGER_H
#define OHOS_ROSEN_SINGLE_DISPLAY_POCKET_FOLD_STATE_SENSOR_MANAGER_H

#include "dm_common.h"
#include "fold_screen_controller/fold_screen_policy.h"
#include "fold_screen_controller/sensor_fold_state_manager/sensor_fold_state_manager.h"
#include "app_state_data.h"
#include "iapplication_state_observer.h"
#include "ability_state_data.h"
#include "process_data.h"

namespace OHOS {
namespace Rosen {
using OHOS::AppExecFwk::AppStateData;
using OHOS::AppExecFwk::IApplicationStateObserver;
enum class ReportTentModeStatus : int32_t {
    NORMAL_EXIT_TENT_MODE = 0,
    NORMAL_ENTER_TENT_MODE = 1,
    ABNORMAL_EXIT_TENT_MODE_DUE_TO_ANGLE = 2,
    ABNORMAL_EXIT_TENT_MODE_DUE_TO_HALL = 3,
};

class ApplicationStatePocketObserver : public IApplicationStateObserver {
public:
    ApplicationStatePocketObserver();
    virtual ~ApplicationStatePocketObserver() = default;
    void OnForegroundApplicationChanged(const AppStateData &appStateData) override;
    void OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override {};
    void OnExtensionStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override {};
    void OnProcessCreated(const AppExecFwk::ProcessData &processData) override {};
    void OnProcessDied(const AppExecFwk::ProcessData &processData) override {};
    void OnApplicationStateChanged(const AppExecFwk::AppStateData &appStateData) override {};
    sptr<IRemoteObject> AsObject() override { return nullptr; };
    std::string GetForegroundApp();
    void RegisterCameraForegroundChanged(std::function<void()> callback);

private:
    std::string foregroundBundleName_ {""};
    std::function<void()> onCameraForegroundChanged_ { nullptr };
};
class SingleDisplaySensorPocketFoldStateManager : public SensorFoldStateManager {
public:
    SingleDisplaySensorPocketFoldStateManager();
    virtual ~SingleDisplaySensorPocketFoldStateManager();

    void HandleAngleChange(float angle, int hall, sptr<FoldScreenPolicy> foldScreenPolicy) override;
    void HandleHallChange(float angle, int hall, sptr<FoldScreenPolicy> foldScreenPolicy) override;
    void HandleTentChange(bool isTent, sptr<FoldScreenPolicy> foldScreenPolicy) override;
    void RegisterApplicationStateObserver() override;

private:
    FoldStatus GetNextFoldState(float angle, int hall);
    void UpdateSwitchScreenBoundaryForLargeFoldDevice(float, int);
    void SetCameraFoldStrategy(float angle);
    void SetCameraRotationStatusChange(float angle, int hall);
    sptr<ApplicationStatePocketObserver> applicationStateObserver_;
    bool isInCameraFoldStrategy_ = false;
    bool isCameraRotationStrategy_ = false;
    std::vector<std::string> hallSwitchPackageNameList_;
    int allowUserSensorForLargeFoldDevice = 0;
    bool TriggerTentExit(float angle, int hall);
    void TentModeHandleSensorChange(float angle, int hall, sptr<FoldScreenPolicy> foldScreenPolicy);
    void ReportTentStatusChange(ReportTentModeStatus tentStatus);
    float currentAngle_ = -1.0F;
    int32_t currentHall_ = -1;
};
} // namespace Rosen
} // namespace OHOS
#endif