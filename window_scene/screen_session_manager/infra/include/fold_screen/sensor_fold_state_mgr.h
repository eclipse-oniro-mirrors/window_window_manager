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

#ifndef OHOS_ROSEN_DMS_FOLD_SENSOR_MGR_H
#define OHOS_ROSEN_DMS_FOLD_SENSOR_MGR_H

#include <vector>
#include "iapplication_state_observer.h"
#include "fold_screen_common.h"

namespace OHOS {
namespace Rosen {
namespace DMS {
enum class ReportTentModeStatus : int32_t {
    NORMAL_EXIT_TENT_MODE = 0,
    NORMAL_ENTER_TENT_MODE = 1,
    ABNORMAL_EXIT_TENT_MODE_DUE_TO_ANGLE = 2,
    ABNORMAL_EXIT_TENT_MODE_DUE_TO_HALL = 3,
};

class SensorFoldStateMgr {
    DISALLOW_COPY_AND_MOVE(SensorFoldStateMgr);

public:
    static SensorFoldStateMgr& GetInstance();
    void HandleSensorEvent(const SensorStatus& sensorStatus);
    void RegisterApplicationStateObserver();
    bool IsTentMode();

protected:
    SensorFoldStateMgr();
    FoldStatus GetNextFoldStatus(const SensorStatus& sensorStatus);
    virtual FoldStatus GetNextFoldStatusByAxis(
        const ScreenAxis& axis, FoldStatus currentStatus, int32_t algorithmStrategy);
    virtual FoldStatus GetNextGlobalFoldStatus(const std::vector<FoldStatus>& foldStatus);
    virtual sptr<AppExecFwk::IApplicationStateObserver> GetAppStateObserver();
    virtual void HandleTentChange(const SensorStatus& sensorStatus);
    virtual void TentModeHandleSensorChange(const SensorStatus& sensorStatus);
    virtual bool TriggerTentExit(const ScreenAxis& axis);
    virtual bool IsSupportTentMode();
    virtual bool CheckInputSensorStatus(const SensorStatus& sensorStatus);
    void HandleSensorChange(FoldStatus nextStatus);
    void UpdateFoldAlgorithmStrategy(const std::vector<ScreenAxis>& axis);
    void ReportTentStatusChange(ReportTentModeStatus tentStatus);
    void SetTentMode(int tentType);

    sptr<AppExecFwk::IApplicationStateObserver> applicationStateObserver_;
    SensorStatus currentSensorStatus_;
    std::vector<FoldStatus> currentFoldStatus_;
    int32_t tentModeType_ = 0;

private:
    std::vector<std::string> getHallSwitchAppList();
    void NotifyReportFoldStatusToScb(int32_t nextStatus);
    void ReportNotifyFoldStatusChange(int32_t nextStatus);
    void SetDeviceStatusAndParam(uint32_t deviceStatus);

    std::vector<int32_t> foldAlgorithmStrategy_;
    std::recursive_mutex statusMutex_;
    FoldStatus globalFoldStatus_ = FoldStatus::UNKNOWN;
};

}  // namespace DMS
}  // namespace Rosen
}  // namespace OHOS

#endif