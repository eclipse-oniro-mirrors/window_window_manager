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

#include "fold_screen_controller/sensor_fold_state_manager/dual_display_sensor_fold_state_manager.h"
#include <iservice_registry.h>
#include <parameters.h>
#include <refbase.h>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <hisysevent.h>

#include "app_mgr_client.h"
#include "app_service_manager.h"
#include "app_mgr_constants.h"
#include "app_mgr_interface.h"
#include "app_state_data.h"
#include "ability_state_data.h"
#include "process_data.h"
#include "fold_screen_controller/fold_screen_policy.h"
#include "fold_screen_controller/fold_screen_controller_config.h"
#include "screen_setting_helper.h"


#include "fold_screen_controller/fold_screen_sensor_manager.h"
#include "fold_screen_controller/sensor_fold_state_manager/dual_display_sensor_fold_state_manager.h"
#include "fold_screen_controller/sensor_fold_state_manager/sensor_fold_state_manager.h"
#include "fold_screen_state_internel.h"
#include "session/screen/include/screen_session.h"
#include "screen_scene_config.h"
#include "iremote_object.h"
#include "window_manager_hilog.h"
#include "dms_global_mutex.h"

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

namespace OHOS::Rosen {
using OHOS::AppExecFwk::AppStateData;
using OHOS::AppExecFwk::ApplicationState;
namespace {
const float INWARD_FOLDED_THRESHOLD = static_cast<float>(system::GetIntParameter<int32_t>
    ("const.fold.folded_threshold", 85));
const float INWARD_EXPAND_THRESHOLD = static_cast<float>(system::GetIntParameter<int32_t>
    ("const.fold.expand_threshold", 145));
const float INWARD_HALF_FOLDED_MAX_THRESHOLD = static_cast<float>(system::GetIntParameter<int32_t>
    ("const.half_folded_max_threshold", 135));
const float INWARD_HALF_FOLDED_MIN_THRESHOLD = static_cast<float>(system::GetIntParameter<int32_t>
    ("const.fold.half_folded_min_threshold", 85));
constexpr int32_t HALL_THRESHOLD = 1;
constexpr int32_t HALL_FOLDED_THRESHOLD = 0;
constexpr float INWARD_FOLDED_LOWER_THRESHOLD = 10.0F;
constexpr float INWARD_FOLDED_UPPER_THRESHOLD = 20.0F;
constexpr float ANGLE_BUFFER = 0.001F;
constexpr float HALL_ZERO_INVALID_POSTURE = 170.0F;
constexpr uint32_t FULL_WAIT_TIMES = 300;
std::mutex foldStatusChangeMutex_;
std::condition_variable angleChangeCv_;
constexpr float TENT_MODE_EXIT_MAX_THRESHOLD = 110.0F;
constexpr int32_t TENT_MODE_OFF = 0;
constexpr int32_t TENT_MODE_ON = 1;
} // namespace

DualDisplaySensorFoldStateManager::DualDisplaySensorFoldStateManager(
    const std::shared_ptr<TaskScheduler>& screenPowerTaskScheduler) :
    screenPowerTaskScheduler_(screenPowerTaskScheduler),
    currentAngle_(0),
    currentHall_(0)
{
    auto stringListConfig = ScreenSceneConfig::GetStringListConfig();
    if (stringListConfig.count("hallSwitchApp") != 0) {
        hallSwitchPackageNameList_ = stringListConfig["hallSwitchApp"];
    }
}

DualDisplaySensorFoldStateManager::~DualDisplaySensorFoldStateManager() {}

void DualDisplaySensorFoldStateManager::UpdateHallSwitchAppInfo(FoldStatus foldStatus)
{
    if (foldStatus == FoldStatus::EXPAND || foldStatus == FoldStatus::HALF_FOLD) {
        isHallSwitchApp_ = true;
    }
}

void DualDisplaySensorFoldStateManager::HandleAngleChange(float angle, int hall,
    sptr<FoldScreenPolicy> foldScreenPolicy)
{
    currentAngle_.store(static_cast<int32_t>(angle));
    if (IsTentMode()) {
        return TentModeHandleSensorChange(angle, hall, foldScreenPolicy);
    }
    if (!CheckUpdateAngle(angle, hall)) {
        return;
    }
    {
        std::unique_lock<std::mutex> lock(foldStatusChangeMutex_);
        isInTask_.store(true);
    }
    angleChangeCv_.notify_one();
    FoldStatus nextState = GetNextFoldState(angle, hall);
    if (nextState != GetCurrentState()) {
        TLOGI(WmsLogTag::DMS, "angle: %{public}f, hall: %{public}d.", angle, hall);
    }
    UpdateHallSwitchAppInfo(nextState);
    HandleSensorChange(nextState, angle, foldScreenPolicy);
}

void DualDisplaySensorFoldStateManager::HandleHallChange(float angle, int hall,
    sptr<FoldScreenPolicy> foldScreenPolicy)
{
    TLOGI(WmsLogTag::DMS, "HandleHallChange angle: %{public}f, hall: %{public}d.", angle, hall);
    currentHall_.store(static_cast<int32_t>(hall));
    if (IsTentMode()) {
        return TentModeHandleSensorChange(angle, hall, foldScreenPolicy);
    }
    if (hall == HALL_THRESHOLD || angle < HALL_ZERO_INVALID_POSTURE) {
        {
            std::unique_lock<std::mutex> lock(foldStatusChangeMutex_);
            isInTask_.store(true);
        }
        angleChangeCv_.notify_one();
        TLOGI(WmsLogTag::DMS,
            "angle: %{public}f, hall: %{public}d. hall is threshold or sensor less than 170.", angle, hall);
        HandleHallChangeInner(angle, hall, foldScreenPolicy);
        return;
    }

    auto taskDualChangeFoldStatus = [this, angle, hall, foldScreenPolicy] {
        TLOGI(WmsLogTag::DMS, "prepare go into timer, angle: %{public}f, hall: %{public}d.", angle, hall);
        std::unique_lock<std::mutex> lock(foldStatusChangeMutex_);
        isInTask_.store(false);
        auto condition = [this] {
            return this->isInTask_.load();
        };
        if (!DmUtils::safe_wait_for(angleChangeCv_, lock, std::chrono::milliseconds(FULL_WAIT_TIMES), condition)) {
            SensorReportTimeOutPro(angle, hall, foldScreenPolicy);
            return;
        }
        TLOGI(WmsLogTag::DMS, "taskDualChangeFoldStatus was notified and not change foldStatus by hall.");
    };
    screenPowerTaskScheduler_->PostAsyncTask(taskDualChangeFoldStatus, __func__);
}

void DualDisplaySensorFoldStateManager::SensorReportTimeOutPro(float angle, int hall,
    const sptr<FoldScreenPolicy>& foldScreenPolicy)
{
    uint16_t currentHall = FoldScreenSensorManager::GetInstance().GetGlobalHall();
    float currentAngle = FoldScreenSensorManager::GetInstance().GetGlobalAngle();
    TLOGI(WmsLogTag::DMS, "currentAngle: %{public}f, currentHall: %{public}d.", currentAngle, currentHall);
    if (currentHall == HALL_THRESHOLD) {
        HandleHallChangeInner(angle, hall, foldScreenPolicy);
        isInTask_.store(true);
        return;
    }
    // no new angle upload, continue the process
    if (FoldScreenStateInternel::FloatEqualAbs(currentAngle, angle)) {
        TLOGI(WmsLogTag::DMS,
            "angle: %{public}f, hall: %{public}d. no continuous angle uploads, continue to change foldstatus.",
            currentAngle, currentHall);
        FoldScreenSensorManager::GetInstance().SetGlobalAngle(ANGLE_MIN_VAL);
        HandleHallChangeInner(ANGLE_MIN_VAL, hall, foldScreenPolicy);
        isInTask_.store(true);
        return;
    }
    if (currentAngle < HALL_ZERO_INVALID_POSTURE) {
        HandleAngleChangeInTask(currentAngle, currentHall, foldScreenPolicy);
        isInTask_.store(true);
        return;
    }
    isInTask_.store(true);
    TLOGI(WmsLogTag::DMS, "taskDualChangeFoldStatus time out and exit.");
    return;
}

void DualDisplaySensorFoldStateManager::HandleHallChangeInner(float angle, int hall,
    const sptr<FoldScreenPolicy>& foldScreenPolicy)
{
    if (applicationStateObserver_ != nullptr && hall == HALL_THRESHOLD
        && PowerMgr::PowerMgrClient::GetInstance().IsScreenOn()) {
        if (std::count(hallSwitchPackageNameList_.begin(), hallSwitchPackageNameList_.end(),
            applicationStateObserver_->GetForegroundApp())) {
            isHallSwitchApp_ = false;
            return;
        }
    }
    if (hall == HALL_THRESHOLD) {
        angle = INWARD_HALF_FOLDED_MIN_THRESHOLD + 1.0f;
    }
    FoldStatus nextState = GetNextFoldState(angle, hall);
    if (nextState != GetCurrentState()) {
        TLOGI(WmsLogTag::DMS, "angle: %{public}f, hall: %{public}d.", angle, hall);
    }
    UpdateHallSwitchAppInfo(nextState);
    HandleSensorChange(nextState, angle, foldScreenPolicy);
}

bool DualDisplaySensorFoldStateManager::CheckUpdateAngle(float& angle, int hall)
{
    if (std::islessequal(angle, INWARD_FOLDED_THRESHOLD + ANGLE_BUFFER) && hall == HALL_THRESHOLD) {
        return false;
    }
    if (std::isgreaterequal(angle, HALL_ZERO_INVALID_POSTURE + ANGLE_BUFFER) && hall == HALL_FOLDED_THRESHOLD) {
        return false;
    }
    if (std::isless(angle, ANGLE_MIN_VAL + ANGLE_BUFFER)) {
        return false;
    }
    if (hall == HALL_FOLDED_THRESHOLD) {
        angle = ANGLE_MIN_VAL;
    }
    return true;
}

void DualDisplaySensorFoldStateManager::HandleAngleChangeInTask(float angle, int hall,
    const sptr<FoldScreenPolicy>& foldScreenPolicy)
{
    if (!CheckUpdateAngle(angle, hall)) {
        return;
    }
    FoldStatus nextState = GetNextFoldState(angle, hall);
    if (nextState != GetCurrentState()) {
        TLOGI(WmsLogTag::DMS, "angle: %{public}f, hall: %{public}d.", angle, hall);
    }
    UpdateHallSwitchAppInfo(nextState);
    HandleSensorChange(nextState, angle, foldScreenPolicy);
}

FoldStatus DualDisplaySensorFoldStateManager::GetNextFoldState(float angle, int hall)
{
    FoldStatus state = GetCurrentState();

    // EXPAND
    if (std::isgreaterequal(angle, INWARD_EXPAND_THRESHOLD + ANGLE_BUFFER)) {
        state = FoldStatus::EXPAND;
    }
    // FOLDED
    if (std::islessequal(angle, INWARD_FOLDED_LOWER_THRESHOLD + ANGLE_BUFFER)) {
        state = FoldStatus::FOLDED;
    }
    // HALF_FOLD
    if (isHallSwitchApp_) {
        ProcessHalfFoldState(state, angle, INWARD_FOLDED_UPPER_THRESHOLD, INWARD_HALF_FOLDED_MAX_THRESHOLD);
    } else {
        ProcessHalfFoldState(state, angle, INWARD_HALF_FOLDED_MIN_THRESHOLD, INWARD_HALF_FOLDED_MAX_THRESHOLD);
    }
    return state;
}

void DualDisplaySensorFoldStateManager::ProcessHalfFoldState(FoldStatus& state, float angle,
    float halfFoldMinThreshold, float halfFoldMaxThreshold)
{
    if (std::isgreaterequal(angle, halfFoldMinThreshold + ANGLE_BUFFER)
        && std::islessequal(angle, halfFoldMaxThreshold + ANGLE_BUFFER)) {
        state = FoldStatus::HALF_FOLD;
    }
}

void DualDisplaySensorFoldStateManager::RegisterApplicationStateObserver()
{
    applicationStateObserver_ = new (std::nothrow) ApplicationStateObserver();
    if (applicationStateObserver_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "applicationStateObserver_ is nullptr.");
        return;
    }
    auto appMgrClient_ = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "appMgrClient_ is nullptr.");
        return;
    }
    int32_t flag =
        appMgrClient_->RegisterApplicationStateObserver(applicationStateObserver_, hallSwitchPackageNameList_);
    if (flag != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "Register app state listener failed, flag = %{public}d", flag);
    } else {
        TLOGI(WmsLogTag::DMS, "Register app state listener success.");
    }
}

ApplicationStateObserver::ApplicationStateObserver() {}

void ApplicationStateObserver::OnForegroundApplicationChanged(const AppStateData &appStateData)
{
    if (appStateData.state == static_cast<int32_t>(ApplicationState::APP_STATE_FOREGROUND)) {
        foregroundBundleName_ = appStateData.bundleName;
        TLOGI(WmsLogTag::DMS, "APP_STATE_FOREGROUND, packageName= %{public}s", foregroundBundleName_.c_str());
    }
    if (appStateData.state == static_cast<int32_t>(ApplicationState::APP_STATE_BACKGROUND)
        && foregroundBundleName_.compare(appStateData.bundleName) == 0) {
        foregroundBundleName_ = "" ;
    }
}

std::string ApplicationStateObserver::GetForegroundApp()
{
    return foregroundBundleName_;
}

void DualDisplaySensorFoldStateManager::HandleTentChange(int tentType,
    sptr<FoldScreenPolicy> foldScreenPolicy, int32_t hall)
{
    // 1 open 0 close
    uint32_t isopen = 0;
    if (ScreenSettingHelper::GetSettingValue(isopen, "wallpaperAodDisplay")) {
        TLOGI(WmsLogTag::DMS, "tent mode wallpaperAodDisplay %{public}u", isopen);
    } else {
        TLOGW(WmsLogTag::DMS, "tent mode read setting wallpaperAodDisplay failed ");
    }
    if (isopen == 1) {
        HandleTentChangeInner(tentType, foldScreenPolicy, hall);
    }
}

void DualDisplaySensorFoldStateManager::HandleTentChangeInner(int tentType,
    sptr<FoldScreenPolicy> foldScreenPolicy, int32_t hall)
{
    float currentAngle = static_cast<float>(currentAngle_.load());
    int32_t currentHall = currentHall_.load();
    if (tentType == tentModeType_) {
        TLOGI(WmsLogTag::DMS, "Repeat reporting tent mode:%{public}d, no processing", tentModeType_);
        return;
    }
    if (foldScreenPolicy == nullptr) {
        TLOGE(WmsLogTag::DMS, "foldScreenPolicy is nullptr");
        return;
    }
    SetTentMode(tentType);
    if (tentType == TENT_MODE_ON) {
        ReportTentStatusChange(ReportDualTentModeStatus::NORMAL_ENTER_TENT_MODE);
        HandleSensorChange(FoldStatus::FOLDED, currentAngle, foldScreenPolicy);
        foldScreenPolicy->ChangeOnTentMode(FoldStatus::FOLDED);
    } else if (tentType == TENT_MODE_OFF) {
        if (hall == HALL_FOLDED_THRESHOLD) {
            currentAngle = ANGLE_MIN_VAL;
        }
        FoldStatus nextState = FoldStatus::UNKNOWN;
        if (hall == -1) {
            nextState = GetNextFoldState(currentAngle, currentHall);
        } else {
            nextState = GetNextFoldState(currentAngle, hall);
        }
        HandleSensorChange(nextState, currentAngle, foldScreenPolicy);
        ReportTentStatusChange(ReportDualTentModeStatus::NORMAL_EXIT_TENT_MODE);
        foldScreenPolicy->ChangeOffTentMode();
    }
}
 
bool DualDisplaySensorFoldStateManager::TriggerTentExit(float angle, int hall)
{
    if (hall == HALL_FOLDED_THRESHOLD) {
        ReportTentStatusChange(ReportDualTentModeStatus::ABNORMAL_EXIT_TENT_MODE_DUE_TO_HALL);
        TLOGI(WmsLogTag::DMS, "Exit tent mode due to hall sensor report folded");
        return true;
    }
 
    if (std::isgreater(angle, TENT_MODE_EXIT_MAX_THRESHOLD)) {
        ReportTentStatusChange(ReportDualTentModeStatus::ABNORMAL_EXIT_TENT_MODE_DUE_TO_ANGLE);
        TLOGI(WmsLogTag::DMS, "Exit tent mode due to angle sensor report angle:%{public}f", angle);
        return true;
    }
 
    return false;
}
 
void DualDisplaySensorFoldStateManager::TentModeHandleSensorChange(float angle, int hall,
    sptr<FoldScreenPolicy> foldScreenPolicy)
{
    if (TriggerTentExit(angle, hall)) {
        FoldStatus nextState = GetNextFoldState(angle, hall);
        HandleSensorChange(nextState, angle, foldScreenPolicy);
        TLOGI(WmsLogTag::DMS, "exit tent mode. angle: %{public}f, hall: %{public}d", angle, hall);
        SetTentMode(TENT_MODE_OFF);
        ScreenRotationProperty::HandleHoverStatusEventInput(DeviceHoverStatus::TENT_STATUS_CANCEL,
            hall == HALL_THRESHOLD ? false : true);
    }
}
 
void DualDisplaySensorFoldStateManager::ReportTentStatusChange(ReportDualTentModeStatus tentStatus)
{
    int32_t status = static_cast<int32_t>(tentStatus);
    TLOGI(WmsLogTag::DMS, "report tentStatus: %{public}d", status);
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "FOLD_TENT_MODE",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "FOLD_TENT_STATUS", status);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "Write HiSysEvent error, ret: %{public}d", ret);
    }
}
} // namespace OHOS::Rosen