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

#ifndef OHOS_ROSEN_SMALL_DEVICE_SCREEN_SENSOR_MANAGER_H
#define OHOS_ROSEN_SMALL_DEVICE_SCREEN_SENSOR_MANAGER_H

#include <mutex>
#include <refbase.h>
#include <chrono>
#include "common/include/task_scheduler.h"
#include "dm_common.h"
#include "fold_screen_controller/fold_screen_policy.h"
namespace OHOS::Rosen {
enum class ReportTentModeStatus : int32_t {
    NORMAL_EXIT_TENT_MODE = 0,
    NORMAL_ENTER_TENT_MODE = 1,
    ABNORMAL_EXIT_TENT_MODE_DUE_TO_ANGLE = 2,
    ABNORMAL_EXIT_TENT_MODE_DUE_TO_HALL = 3,
};
class SensorFoldStateManager : public RefBase {
public:
    SensorFoldStateManager();
    virtual ~SensorFoldStateManager();

    virtual void HandleAngleChange(float angle, int hall, sptr<FoldScreenPolicy> foldScreenPolicy);
    virtual void HandleHallChange(float angle, int hall, sptr<FoldScreenPolicy> foldScreenPolicy);
    virtual void HandleTentChange(int tentType, sptr<FoldScreenPolicy> foldScreenPolicy, int32_t hall = -1);
    virtual void HandleAngleOrHallChange(const std::vector<float> &angles, const std::vector<uint16_t> &halls,
        sptr<FoldScreenPolicy> foldScreenPolicy, bool isPostureRegistered);
    virtual void RegisterApplicationStateObserver();
    void ClearState(sptr<FoldScreenPolicy> foldScreenPolicy);
    bool IsTentMode();
    void FinishTaskSequence();
protected:
    virtual FoldStatus HandleSecondaryOneStep(FoldStatus currentStatus, FoldStatus nextStatus,
        const std::vector<float>& previousAngles, const std::vector<uint16_t>& previousHalls) { return nextStatus; }
    void HandleSensorChange(FoldStatus nextState, float angle, sptr<FoldScreenPolicy> foldScreenPolicy);
    void HandleSensorChange(FoldStatus nextState, const std::vector<float>& angles,
        const std::vector<uint16_t>& halls, sptr<FoldScreenPolicy> foldScreenPolicy);
    FoldStatus GetCurrentState();
    void SetTentMode(int tentType);
    std::recursive_mutex mStateMutex_;
    int tentModeType_ = 0;
    inline static bool isInOneStep_ = false;
    inline static std::condition_variable oneStep_;
    inline static std::mutex oneStepMutex_;

private:
    void ReportNotifyFoldStatusChange(int32_t currentStatus, int32_t nextStatus, float postureAngle);
    void ReportNotifyFoldStatusChange(int32_t currentStatus, int32_t nextStatus,
        const std::vector<float> &postureAngles);
    FoldStatus mState_ = FoldStatus::UNKNOWN;

    std::chrono::time_point<std::chrono::system_clock> mLastStateClock_ = std::chrono::system_clock::now();
    void NotifyReportFoldStatusToScb(FoldStatus currentStatus, FoldStatus nextStatus, float postureAngle);
    void NotifyReportFoldStatusToScb(FoldStatus currentStatus, FoldStatus nextStatus,
        const std::vector<float> &postureAngles);

    void ProcessNotifyFoldStatusChange(FoldStatus currentStatus, FoldStatus nextStatus,
        const std::vector<float>& angles, sptr<FoldScreenPolicy> foldScreenPolicy);
};
} // namespace OHOS::Rosen
#endif //OHOS_ROSEN_SMALL_DEVICE_SCREEN_SENSOR_MANAGER_H