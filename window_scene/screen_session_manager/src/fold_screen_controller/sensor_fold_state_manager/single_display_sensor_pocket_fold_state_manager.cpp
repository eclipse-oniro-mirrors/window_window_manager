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

#include <parameters.h>
#include <hisysevent.h>

#include "fold_screen_controller/fold_screen_policy.h"
#include "fold_screen_controller/sensor_fold_state_manager/single_display_sensor_pocket_fold_state_manager.h"
#include "fold_screen_controller/sensor_fold_state_manager/sensor_fold_state_manager.h"
#include "session/screen/include/screen_session.h"
#include "screen_scene_config.h"

#include "window_manager_hilog.h"
#include "app_mgr_client.h"
#include "screen_session_manager/include/screen_rotation_property.h"
#include "screen_session_manager.h"

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

namespace OHOS::Rosen {
using OHOS::AppExecFwk::AppStateData;
using OHOS::AppExecFwk::ApplicationState;
namespace {
const std::string CAMERA_NAME = "camera";
constexpr float ANGLE_MIN_VAL = 0.0F;
constexpr float ALTA_HALF_FOLDED_MAX_THRESHOLD = 140.0F;
constexpr float CLOSE_ALTA_HALF_FOLDED_MIN_THRESHOLD = 70.0F;
constexpr float OPEN_ALTA_HALF_FOLDED_MIN_THRESHOLD = 25.0F;
constexpr float ALTA_HALF_FOLDED_BUFFER = 10.0F;
constexpr float LARGER_BOUNDARY_FOR_ALTA_THRESHOLD = 90.0F;
constexpr int32_t LARGER_BOUNDARY_FLAG = 1;
constexpr int32_t SMALLER_BOUNDARY_FLAG = 0;
constexpr int32_t HALL_THRESHOLD = 1;
constexpr int32_t HALL_FOLDED_THRESHOLD = 0;
constexpr float TENT_MODE_EXIT_MIN_THRESHOLD = 5.0F;
constexpr float TENT_MODE_EXIT_MAX_THRESHOLD = 175.0F;
constexpr int32_t TENT_MODE_OFF = 0;
constexpr int32_t TENT_MODE_ON = 1;
constexpr int32_t TENT_MODE_HOVER_ON = 2;
} // namespace

SingleDisplaySensorPocketFoldStateManager::SingleDisplaySensorPocketFoldStateManager()
{
    auto stringListConfig = ScreenSceneConfig::GetStringListConfig();
    if (stringListConfig.count("hallSwitchApp") != 0) {
        hallSwitchPackageNameList_ = stringListConfig["hallSwitchApp"];
    }
}
SingleDisplaySensorPocketFoldStateManager::~SingleDisplaySensorPocketFoldStateManager() {}

void SingleDisplaySensorPocketFoldStateManager::HandleAngleChange(float angle, int hall,
    sptr<FoldScreenPolicy> foldScreenPolicy)
{
    currentAngle = angle;
    if (IsTentMode()) {
        return TentModeHandleSensorChange(angle, hall, foldScreenPolicy);
    }
    FoldStatus nextState = GetNextFoldState(angle, hall);
    HandleSensorChange(nextState, angle, foldScreenPolicy);
}

void SingleDisplaySensorPocketFoldStateManager::HandleHallChange(float angle, int hall,
    sptr<FoldScreenPolicy> foldScreenPolicy)
{
    currentHall = hall;
    if (IsTentMode()) {
        return TentModeHandleSensorChange(angle, hall, foldScreenPolicy);
    }
    FoldStatus nextState = GetNextFoldState(angle, hall);
    HandleSensorChange(nextState, angle, foldScreenPolicy);
}

void SingleDisplaySensorPocketFoldStateManager::UpdateSwitchScreenBoundaryForLargeFoldDevice(float angle, int hall)
{
    if (hall == HALL_FOLDED_THRESHOLD) {
        allowUserSensorForLargeFoldDevice = SMALLER_BOUNDARY_FLAG;
    } else if (angle >= LARGER_BOUNDARY_FOR_ALTA_THRESHOLD) {
        allowUserSensorForLargeFoldDevice = LARGER_BOUNDARY_FLAG;
    }
}

FoldStatus SingleDisplaySensorPocketFoldStateManager::GetNextFoldState(float angle, int hall)
{
    UpdateSwitchScreenBoundaryForLargeFoldDevice(angle, hall);
    FoldStatus currentState = GetCurrentState();
    if (std::isless(angle, ANGLE_MIN_VAL)) {
        return currentState;
    }
    FoldStatus state;

    if (allowUserSensorForLargeFoldDevice == SMALLER_BOUNDARY_FLAG) {
        if (std::islessequal(angle, OPEN_ALTA_HALF_FOLDED_MIN_THRESHOLD) && hall == HALL_FOLDED_THRESHOLD) {
            state = FoldStatus::FOLDED;
        } else if (std::isgreaterequal(angle, OPEN_ALTA_HALF_FOLDED_MIN_THRESHOLD + ALTA_HALF_FOLDED_BUFFER) &&
            hall == HALL_FOLDED_THRESHOLD) {
            state = FoldStatus::HALF_FOLD;
        } else if (std::islessequal(angle, ALTA_HALF_FOLDED_MAX_THRESHOLD - ALTA_HALF_FOLDED_BUFFER) &&
            hall == HALL_THRESHOLD) {
            state = FoldStatus::HALF_FOLD;
        } else if (std::isgreaterequal(angle, ALTA_HALF_FOLDED_MAX_THRESHOLD)) {
            state = FoldStatus::EXPAND;
        } else {
            state = currentState;
            if (state == FoldStatus::UNKNOWN) {
                state = FoldStatus::HALF_FOLD;
            }
        }
        return state;
    }

    if (hall == HALL_THRESHOLD && angle == OPEN_ALTA_HALF_FOLDED_MIN_THRESHOLD) {
        state = currentState;
    } else if (std::islessequal(angle, CLOSE_ALTA_HALF_FOLDED_MIN_THRESHOLD)) {
        state = FoldStatus::FOLDED;
    } else if (std::islessequal(angle, ALTA_HALF_FOLDED_MAX_THRESHOLD - ALTA_HALF_FOLDED_BUFFER) &&
        std::isgreater(angle, CLOSE_ALTA_HALF_FOLDED_MIN_THRESHOLD + ALTA_HALF_FOLDED_BUFFER)) {
        state = FoldStatus::HALF_FOLD;
    } else if (std::isgreaterequal(angle, ALTA_HALF_FOLDED_MAX_THRESHOLD)) {
        state = FoldStatus::EXPAND;
    } else {
        state = currentState;
        if (state == FoldStatus::UNKNOWN) {
            state = FoldStatus::HALF_FOLD;
        }
    }
    return state;
}

void SingleDisplaySensorPocketFoldStateManager::RegisterApplicationStateObserver()
{
    applicationStateObserver_ = new (std::nothrow) ApplicationStatePocketObserver();
    auto appMgrClient_ = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (applicationStateObserver_ == nullptr) {
        TLOGI(WmsLogTag::DMS_SSM, "applicationStateObserver_ is nullptr.");
        return;
    }
    if (appMgrClient_ == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "appMgrClient_ is nullptr.");
    } else {
        auto flag = static_cast<int32_t>(
            appMgrClient_->RegisterApplicationStateObserver(applicationStateObserver_, hallSwitchPackageNameList_));
        if (flag != ERR_OK) {
            TLOGE(WmsLogTag::DMS_SSM, "Register app debug listener failed.");
        } else {
            TLOGI(WmsLogTag::DMS_SSM, "Register app debug listener success.");
        }
    }
}

void SingleDisplaySensorPocketFoldStateManager::HandleTentChange(int tentType,
    sptr<FoldScreenPolicy> foldScreenPolicy, int32_t hall)
{
    if (tentType == tentModeType_) {
        TLOGI(WmsLogTag::DMS_SSM, "Repeat reporting tent mode:%{public}d, no processing", tentModeType_);
        return;
    }

    SetTentMode(tentType);
    if (foldScreenPolicy == nullptr) {
        TLOGE(WmsLogTag::DMS_SSM, "foldScreenPolicy is nullptr");
        return;
    }
    if (tentType != TENT_MODE_OFF) {
        ReportTentStatusChange(ReportTentModeStatus::NORMAL_ENTER_TENT_MODE);
        HandleSensorChange(FoldStatus::FOLDED, currentAngle, foldScreenPolicy);
        foldScreenPolicy->ChangeOnTentMode(FoldStatus::FOLDED);
        if (tentType == TENT_MODE_ON) {
            ScreenRotationProperty::HandleHoverStatusEventInput(DeviceHoverStatus::TENT_STATUS);
        } else if (tentType == TENT_MODE_HOVER_ON) {
            ScreenRotationProperty::HandleHoverStatusEventInput(DeviceHoverStatus::TENT_STATUS_HOVER);
        }
    } else {
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
        ReportTentStatusChange(ReportTentModeStatus::NORMAL_EXIT_TENT_MODE);
        foldScreenPolicy->ChangeOffTentMode();
        ScreenRotationProperty::HandleHoverStatusEventInput(DeviceHoverStatus::TENT_STATUS_CANCEL);
    }
}

bool SingleDisplaySensorPocketFoldStateManager::TriggerTentExit(float angle, int hall)
{
    if (hall == HALL_FOLDED_THRESHOLD) {
        ReportTentStatusChange(ReportTentModeStatus::ABNORMAL_EXIT_TENT_MODE_DUE_TO_HALL);
        TLOGI(WmsLogTag::DMS_SSM, "Exit tent mode due to hall sensor report folded");
        return true;
    }

    if (std::isless(angle, TENT_MODE_EXIT_MIN_THRESHOLD) || std::isgreater(angle, TENT_MODE_EXIT_MAX_THRESHOLD)) {
        ReportTentStatusChange(ReportTentModeStatus::ABNORMAL_EXIT_TENT_MODE_DUE_TO_ANGLE);
        TLOGI(WmsLogTag::DMS_SSM, "Exit tent mode due to angle sensor report angle:%{public}f", angle);
        return true;
    }

    return false;
}

void SingleDisplaySensorPocketFoldStateManager::TentModeHandleSensorChange(float angle, int hall,
    sptr<FoldScreenPolicy> foldScreenPolicy)
{
    if (TriggerTentExit(angle, hall)) {
        FoldStatus nextState = GetNextFoldState(angle, hall);
        HandleSensorChange(nextState, angle, foldScreenPolicy);
        TLOGI(WmsLogTag::DMS_SSM, "exit tent mode. angle: %{public}f, hall: %{public}d", angle, hall);
        SetTentMode(TENT_MODE_OFF);
        ScreenRotationProperty::HandleHoverStatusEventInput(DeviceHoverStatus::TENT_STATUS_CANCEL);
    }
}

void SingleDisplaySensorPocketFoldStateManager::ReportTentStatusChange(ReportTentModeStatus tentStatus)
{
    int32_t status = static_cast<int32_t>(tentStatus);
    TLOGI(WmsLogTag::DMS_SSM, "report tentStatus: %{public}d", status);
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "FOLD_TENT_MODE",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "FOLD_TENT_STATUS", status);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS_SSM, "Write HiSysEvent error, ret: %{public}d", ret);
    }
}

bool SingleDisplaySensorPocketFoldStateManager::IsCameraMode()
{
    return false;
}

ApplicationStatePocketObserver::ApplicationStatePocketObserver() {}

void ApplicationStatePocketObserver::RegisterCameraForegroundChanged(std::function<void()> callback)
{
    onCameraForegroundChanged_ = callback;
}

void ApplicationStatePocketObserver::OnForegroundApplicationChanged(const AppStateData &appStateData)
{
    if (appStateData.state == static_cast<int32_t>(ApplicationState::APP_STATE_FOREGROUND)) {
        if (appStateData.bundleName.find(CAMERA_NAME) != std::string::npos) {
            isCameraForeground_ = true;
        }
        foregroundBundleName_ = appStateData.bundleName;
    }
    if (appStateData.state == static_cast<int32_t>(ApplicationState::APP_STATE_BACKGROUND)) {
        if (appStateData.bundleName.find(CAMERA_NAME) != std::string::npos) {
            isCameraForeground_ = false;
        }
        if (foregroundBundleName_.compare(appStateData.bundleName) == 0) {
            foregroundBundleName_ = "" ;
        }
    }
    if (appStateData.bundleName.find(CAMERA_NAME) != std::string::npos) {
        if (onCameraForegroundChanged_ == nullptr) {
            TLOGE(WmsLogTag::DMS_SSM, "onCameraForegroundChanged_ not register");
            return;
        }
    }
}

bool ApplicationStatePocketObserver::IsCameraForeground()
{
    return isCameraForeground_;
}

std::string ApplicationStatePocketObserver::GetForegroundApp()
{
    return foregroundBundleName_;
}
} // namespace OHOS::Rosen