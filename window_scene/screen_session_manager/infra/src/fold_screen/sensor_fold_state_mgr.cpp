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

#include "sensor_fold_state_mgr.h"
#include <mutex>
#include <hisysevent.h>
#include <parameters.h>
#include <chrono>
#include "app_mgr_client.h"
#include "fold_screen_controller/fold_screen_controller_config.h"
#include "fold_screen_state_internel.h"
#include "window_manager_hilog.h"
#include "screen_session_manager.h"
#include "fold_screen_base_policy.h"
#include "task_sequence_process.h"
#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

namespace OHOS::Rosen::DMS {

namespace {
constexpr int32_t LARGER_BOUNDARY_FLAG = 1;
constexpr int32_t SMALLER_BOUNDARY_FLAG = 0;
constexpr int32_t HALL_FOLDED_THRESHOLD = 0;
constexpr uint16_t DEFAULT_AXIS_SIZE = 1;
constexpr int32_t HALL_THRESHOLD = 1;
constexpr float TENT_MODE_EXIT_MIN_THRESHOLD = 5.0F;
constexpr float TENT_MODE_EXIT_MAX_THRESHOLD = 175.0F;
constexpr int32_t TENT_MODE_OFF = 0;
constexpr int32_t TENT_MODE_ON = 1;
constexpr int32_t MAX_QUEUE_SIZE = 1;
constexpr uint64_t MAX_TIME_INTERVAL_MS = 200;
TaskSequenceProcess g_taskProcessor(MAX_QUEUE_SIZE, MAX_TIME_INTERVAL_MS, "newfoldStatusProcessor");
std::chrono::time_point<std::chrono::system_clock> g_lastUpdateTime = std::chrono::system_clock::now();
}  // namespace

SensorFoldStateMgr& SensorFoldStateMgr::GetInstance()
{
    static std::mutex singletonMutex_;
    static SensorFoldStateMgr* instance_ = nullptr;
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(singletonMutex_);
        if (instance_ == nullptr) {
            instance_ = new SensorFoldStateMgr();
        }
    }
    return *instance_;
}

SensorFoldStateMgr::SensorFoldStateMgr()
{
    currentFoldStatus_ = {FoldStatus::UNKNOWN};
    foldAlgorithmStrategy_ = {0, 0};
}

void SensorFoldStateMgr::HandleSensorEvent(const SensorStatus& sensorStatus)
{
    if (!CheckInputSensorStatus(sensorStatus)) {
        return;
    }

    currentSensorStatus_ = sensorStatus;
    if (sensorStatus.updateSensorType_ == DmsSensorType::SENSOR_TYPE_TENT) {
        HandleTentChange(sensorStatus);
        return;
    }

    if (IsTentMode() && IsSupportTentMode()) {
        TentModeHandleSensorChange(sensorStatus);
        return;
    }

    FoldStatus nextStatus = GetNextFoldStatus(sensorStatus);
    HandleSensorChange(nextStatus);
}

FoldStatus SensorFoldStateMgr::GetNextFoldStatus(const SensorStatus& sensorStatus)
{
    UpdateFoldAlgorithmStrategy(sensorStatus.axis_);
    std::vector<FoldStatus> nextFoldStatus;
    for (size_t i = 0; i < sensorStatus.axis_.size(); ++i) {
        nextFoldStatus.emplace_back(
            GetNextFoldStatusByAxis(sensorStatus.axis_[i], currentFoldStatus_[i], foldAlgorithmStrategy_[i]));
    }
    currentFoldStatus_ = nextFoldStatus;
    return GetNextGlobalFoldStatus(nextFoldStatus);
}

FoldStatus SensorFoldStateMgr::GetNextFoldStatusByAxis(
    const ScreenAxis& axis, FoldStatus currentStatus, int32_t algorithmStrategy)
{
    if (std::isless(axis.angle_, ANGLE_MIN_VAL)) {
        return currentStatus;
    }

    float angle = axis.angle_;
    int32_t hall = axis.hall_;
    FoldStatus foldStatus;
    if (algorithmStrategy == SMALLER_BOUNDARY_FLAG) {
        if (std::islessequal(angle, OPEN_HALF_FOLDED_MIN_THRESHOLD) && hall == HALL_FOLDED_THRESHOLD) {
            foldStatus = FoldStatus::FOLDED;
        } else if (std::isgreaterequal(angle, OPEN_HALF_FOLDED_MIN_THRESHOLD + HALF_FOLDED_BUFFER) &&
                   hall == HALL_FOLDED_THRESHOLD) {
            foldStatus = FoldStatus::HALF_FOLD;
        } else if (std::islessequal(angle, HALF_FOLDED_MAX_THRESHOLD - HALF_FOLDED_BUFFER) && hall == HALL_THRESHOLD) {
            foldStatus = FoldStatus::HALF_FOLD;
        } else if (std::isgreaterequal(angle, HALF_FOLDED_MAX_THRESHOLD)) {
            foldStatus = FoldStatus::EXPAND;
        } else {
            foldStatus = currentStatus;
            if (foldStatus == FoldStatus::UNKNOWN) {
                foldStatus = FoldStatus::HALF_FOLD;
            }
        }
        return foldStatus;
    }

    if (hall == HALL_THRESHOLD && (std::fabs(angle - OPEN_HALF_FOLDED_MIN_THRESHOLD) < 1e-3)) {
        foldStatus = currentStatus;
    } else if (std::islessequal(angle, CLOSE_HALF_FOLDED_MIN_THRESHOLD)) {
        foldStatus = FoldStatus::FOLDED;
    } else if (std::islessequal(angle, HALF_FOLDED_MAX_THRESHOLD - HALF_FOLDED_BUFFER) &&
               std::isgreater(angle, CLOSE_HALF_FOLDED_MIN_THRESHOLD + HALF_FOLDED_BUFFER)) {
        foldStatus = FoldStatus::HALF_FOLD;
    } else if (std::isgreaterequal(angle, HALF_FOLDED_MAX_THRESHOLD)) {
        foldStatus = FoldStatus::EXPAND;
    } else {
        foldStatus = currentStatus;
        if (foldStatus == FoldStatus::UNKNOWN) {
            foldStatus = FoldStatus::HALF_FOLD;
        }
    }
    return foldStatus;
}

FoldStatus SensorFoldStateMgr::GetNextGlobalFoldStatus(const std::vector<FoldStatus>& foldStatus)
{
    if (foldStatus.empty()) {
        return FoldStatus::UNKNOWN;
    }
    return foldStatus[0];
}

void SensorFoldStateMgr::RegisterApplicationStateObserver()
{
    applicationStateObserver_ = GetAppStateObserver();
    if (applicationStateObserver_ == nullptr) {
        TLOGI(WmsLogTag::DMS, "applicationStateObserver_ is nullptr.");
        return;
    }

    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        TLOGE(WmsLogTag::DMS, "appMgrClient_ is nullptr.");
        return;
    }

    auto flag = static_cast<int32_t>(
        appMgrClient->RegisterApplicationStateObserver(applicationStateObserver_, getHallSwitchAppList()));
    if (flag != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "Register app debug listener failed.");
    } else {
        TLOGI(WmsLogTag::DMS, "Register app debug listener success.");
    }
}

sptr<AppExecFwk::IApplicationStateObserver> SensorFoldStateMgr::GetAppStateObserver()
{
    return nullptr;
}

std::vector<std::string> SensorFoldStateMgr::getHallSwitchAppList()
{
    auto stringListConfig = ScreenSceneConfig::GetStringListConfig();
    if (stringListConfig.count("hallSwitchApp") != 0) {
        return stringListConfig["hallSwitchApp"];
    }
    return {};
}

void SensorFoldStateMgr::HandleSensorChange(FoldStatus nextStatus)
{
    std::lock_guard<std::recursive_mutex> lock(statusMutex_);
    if (nextStatus == FoldStatus::UNKNOWN) {
        TLOGW(WmsLogTag::DMS, "fold state is UNKNOWN");
        return;
    }
    auto task = [=] {
        if (globalFoldStatus_ != nextStatus) {
            TLOGI(WmsLogTag::DMS, "current state: %{public}d, next state: %{public}d.", globalFoldStatus_, nextStatus);
            ReportNotifyFoldStatusChange((int32_t)nextStatus);
            PowerMgr::PowerMgrClient::GetInstance().RefreshActivity();

            NotifyReportFoldStatusToScb((int32_t)nextStatus);

            globalFoldStatus_ = nextStatus;

            FoldScreenBasePolicy::GetInstance().SetFoldStatus(globalFoldStatus_);
            ScreenSessionManager::GetInstance().NotifyFoldStatusChanged(globalFoldStatus_);
            if (!FoldScreenBasePolicy::GetInstance().GetLockDisplayStatus()) {
                FoldScreenBasePolicy::GetInstance().SendSensorResult(globalFoldStatus_);
            }
        } else {
            TLOGD(WmsLogTag::DMS, "fold state doesn't change, foldState = %{public}d.", globalFoldStatus_);
        }
        if (FoldScreenBasePolicy::GetInstance().GetdisplayModeRunningStatus() == false) {
            FinishTaskSequence();
        }
    };
    g_taskProcessor.AddTask(task);
}

void SensorFoldStateMgr::FinishTaskSequence()
{
    TLOGI(WmsLogTag::DMS, "TaskSequenceProcess SensorFoldStateMgr::FinishTaskSequence");
    g_taskProcessor.FinishTask();
}

void SensorFoldStateMgr::UpdateFoldAlgorithmStrategy(const std::vector<ScreenAxis>& axis)
{
    for (size_t i = 0; i < axis.size(); ++i) {
        if (i >= foldAlgorithmStrategy_.size()) {
            return;
        }
        if (axis[i].hall_ == HALL_FOLDED_THRESHOLD) {
            foldAlgorithmStrategy_[i] = SMALLER_BOUNDARY_FLAG;
        } else if (axis[i].angle_ >= LARGER_BOUNDARY_FOR_THRESHOLD) {
            foldAlgorithmStrategy_[i] = LARGER_BOUNDARY_FLAG;
        }
    }
}

void SensorFoldStateMgr::ReportNotifyFoldStatusChange(int32_t nextStatus)
{
    if (currentSensorStatus_.axis_.empty()) {
        return;
    }
    int32_t currentStatus = (int32_t)globalFoldStatus_;
    std::string postureStr;
    std::vector<float> angles;
    for (auto& it : currentSensorStatus_.axis_) {
        angles.emplace_back(it.angle_);
    }
    if (angles.size() == DEFAULT_AXIS_SIZE) {
        postureStr = std::to_string(angles[0]);
    } else {
        angles.emplace_back(currentSensorStatus_.reflexionAngle_);
        postureStr = FoldScreenStateInternel::TransVec2Str(angles, "postureAngles").c_str();
    }
    TLOGI(WmsLogTag::DMS,
        "ReportNotifyFoldStatusChange currentStatus: %{public}d, nextStatus: %{public}d, "
        "postureAngle: %{public}s",
        currentStatus, nextStatus, postureStr.c_str());
    int32_t ret = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER, "NOTIFY_FOLD_STATE_CHANGE",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "CURRENT_FOLD_STATUS", currentStatus, "NEXT_FOLD_STATUS",
        nextStatus, "SENSOR_POSTURE", postureStr);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "ReportNotifyFoldStatusChange Write HiSysEvent error, ret: %{public}d", ret);
    }
}

void SensorFoldStateMgr::NotifyReportFoldStatusToScb(int32_t nextStatus)
{
    if (currentSensorStatus_.axis_.empty()) {
        return;
    }
    int32_t currentStatus = (int32_t)globalFoldStatus_;
    std::chrono::time_point<std::chrono::system_clock> timeNow = std::chrono::system_clock::now();
    int32_t duration =
        static_cast<int32_t>(std::chrono::duration_cast<std::chrono::seconds>(timeNow - g_lastUpdateTime).count());
    g_lastUpdateTime = timeNow;

    std::vector<std::string> screenFoldInfo{
        std::to_string(static_cast<int32_t>(currentStatus)), std::to_string(nextStatus), std::to_string(duration)};

    for (auto& it : currentSensorStatus_.axis_) {
        screenFoldInfo.emplace_back(std::to_string(it.angle_));
    }
    ScreenSessionManager::GetInstance().ReportFoldStatusToScb(screenFoldInfo);
}

void SensorFoldStateMgr::HandleTentChange(const SensorStatus& sensorStatus)
{
    SensorStatus tmpSensorStatus = sensorStatus;
    TentSensorInfo& tentSensor = tmpSensorStatus.tentSensorInfo_;
    if (tentSensor.tentType_ == tentModeType_) {
        TLOGI(WmsLogTag::DMS, "Repeat reporting tent mode:%{public}d, no processing", tentModeType_);
        return;
    }

    SetTentMode(tentSensor.tentType_);
    if (tentSensor.tentType_ == TENT_MODE_ON) {
        ReportTentStatusChange(ReportTentModeStatus::NORMAL_ENTER_TENT_MODE);
        HandleSensorChange(FoldStatus::FOLDED);
        FoldScreenBasePolicy::GetInstance().ChangeOnTentMode(FoldStatus::FOLDED);
        SetDeviceStatusAndParam(static_cast<uint32_t>(DMDeviceStatus::STATUS_TENT));
        ScreenRotationProperty::HandleHoverStatusEventInput(DeviceHoverStatus::TENT_STATUS);
    } else {
        if (tentSensor.hall_ == HALL_FOLDED_THRESHOLD) {
            tmpSensorStatus.axis_[0].angle_ = ANGLE_MIN_VAL;
        }
        FoldStatus nextStatus = FoldStatus::UNKNOWN;
        if (tentSensor.hall_ == -1) {
            nextStatus = GetNextFoldStatus(tmpSensorStatus);
        } else {
            tmpSensorStatus.axis_[0].hall_ = tentSensor.hall_;
            nextStatus = GetNextFoldStatus(tmpSensorStatus);
        }
        if (nextStatus == FoldStatus::FOLDED) {
            SetDeviceStatusAndParam(static_cast<uint32_t>(DMDeviceStatus::STATUS_FOLDED));
        } else {
            SetDeviceStatusAndParam(static_cast<uint32_t>(DMDeviceStatus::UNKNOWN));
        }
        HandleSensorChange(nextStatus);
        ReportTentStatusChange(ReportTentModeStatus::NORMAL_EXIT_TENT_MODE);
        FoldScreenBasePolicy::GetInstance().ChangeOffTentMode();
        ScreenRotationProperty::HandleHoverStatusEventInput(DeviceHoverStatus::TENT_STATUS_CANCEL);
    }
}

bool SensorFoldStateMgr::IsTentMode()
{
    return tentModeType_ != 0;
}

void SensorFoldStateMgr::ReportTentStatusChange(ReportTentModeStatus tentStatus)
{
    int32_t status = static_cast<int32_t>(tentStatus);
    TLOGI(WmsLogTag::DMS, "report tentStatus: %{public}d", status);
    int32_t ret = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER, "FOLD_TENT_MODE",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "FOLD_TENT_STATUS", status);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "Write HiSysEvent error, ret: %{public}d", ret);
    }
}

bool SensorFoldStateMgr::IsSupportTentMode()
{
    return true;
}

bool SensorFoldStateMgr::CheckInputSensorStatus(const SensorStatus& sensorStatus)
{
    if (sensorStatus.axis_.size() != DEFAULT_AXIS_SIZE) {
        TLOGI(WmsLogTag::DMS, "invalid sensor status, axis size: %{public}u", sensorStatus.axis_.size());
        return false;
    }
    return true;
}

bool SensorFoldStateMgr::TriggerTentExit(const ScreenAxis& axis)
{
    if (axis.hall_ == HALL_FOLDED_THRESHOLD) {
        ReportTentStatusChange(ReportTentModeStatus::ABNORMAL_EXIT_TENT_MODE_DUE_TO_HALL);
        TLOGI(WmsLogTag::DMS, "Exit tent mode due to hall sensor report folded");
        return true;
    }

    if (std::isless(axis.angle_, TENT_MODE_EXIT_MIN_THRESHOLD) ||
        std::isgreater(axis.angle_, TENT_MODE_EXIT_MAX_THRESHOLD)) {
        ReportTentStatusChange(ReportTentModeStatus::ABNORMAL_EXIT_TENT_MODE_DUE_TO_ANGLE);
        TLOGI(WmsLogTag::DMS, "Exit tent mode due to angle sensor report angle:%{public}f", axis.angle_);
        return true;
    }

    return false;
}

void SensorFoldStateMgr::TentModeHandleSensorChange(const SensorStatus& sensorStatus)
{
    const ScreenAxis& axis = sensorStatus.axis_[0];
    if (TriggerTentExit(axis)) {
        FoldStatus nextStatus = GetNextFoldStatus(sensorStatus);
        HandleSensorChange(nextStatus);
        TLOGI(WmsLogTag::DMS, "exit tent mode. angle: %{public}f, hall: %{public}d", axis.angle_, axis.hall_);
        SetTentMode(TENT_MODE_OFF);
        if (nextStatus == FoldStatus::FOLDED) {
            SetDeviceStatusAndParam(static_cast<uint32_t>(DMDeviceStatus::STATUS_FOLDED));
        } else {
            SetDeviceStatusAndParam(static_cast<uint32_t>(DMDeviceStatus::UNKNOWN));
        }
        ScreenRotationProperty::HandleHoverStatusEventInput(DeviceHoverStatus::TENT_STATUS_CANCEL);
    }
}

void SensorFoldStateMgr::SetDeviceStatusAndParam(uint32_t deviceStatus)
{
    TLOGI(WmsLogTag::DMS, "Set device status to: %{public}u", deviceStatus);
    SetDeviceStatus(deviceStatus);
    system::SetParameter("persist.dms.device.status", std::to_string(deviceStatus));
}

void SensorFoldStateMgr::SetTentMode(int tentType)
{
    TLOGI(WmsLogTag::DMS, "tent mode changing: %{public}d -> %{public}d", tentModeType_, tentType);
    tentModeType_ = tentType;
}

}  // namespace OHOS::Rosen::DMS