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

#include "fold_screen_controller/sensor_fold_state_manager/single_display_sensor_fold_state_manager.h"
#include <parameters.h>

#include "fold_screen_controller/fold_screen_policy.h"
#include "fold_screen_controller/fold_screen_controller_config.h"
#include "fold_screen_controller/sensor_fold_state_manager/sensor_fold_state_manager.h"
#include "session/screen/include/screen_session.h"

#include "window_manager_hilog.h"

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

namespace OHOS::Rosen {
namespace {
constexpr int32_t LARGER_BOUNDARY_FLAG = 1;
constexpr int32_t SMALLER_BOUNDARY_FLAG = 0;
constexpr int32_t HALL_THRESHOLD = 1;
constexpr int32_t HALL_FOLDED_THRESHOLD = 0;
} // namespace

SingleDisplaySensorFoldStateManager::SingleDisplaySensorFoldStateManager() {}
SingleDisplaySensorFoldStateManager::~SingleDisplaySensorFoldStateManager() {}

void SingleDisplaySensorFoldStateManager::HandleAngleChange(float angle, int hall,
    sptr<FoldScreenPolicy> foldScreenPolicy)
{
    FoldStatus nextState = GetNextFoldState(angle, hall);
    HandleSensorChange(nextState, angle, foldScreenPolicy);
}

void SingleDisplaySensorFoldStateManager::HandleHallChange(float angle, int hall,
    sptr<FoldScreenPolicy> foldScreenPolicy)
{
    FoldStatus nextState = GetNextFoldState(angle, hall);
    HandleSensorChange(nextState, angle, foldScreenPolicy);
}

void SingleDisplaySensorFoldStateManager::UpdateSwitchScreenBoundaryForLargeFoldDevice(float angle, int hall)
{
    if (hall == HALL_FOLDED_THRESHOLD) {
        allowUserSensorForLargeFoldDevice = SMALLER_BOUNDARY_FLAG;
    } else if (angle >= LARGER_BOUNDARY_FOR_THRESHOLD) {
        allowUserSensorForLargeFoldDevice = LARGER_BOUNDARY_FLAG;
    }
}


FoldStatus SingleDisplaySensorFoldStateManager::GetNextFoldState(float angle, int hall)
{
    UpdateSwitchScreenBoundaryForLargeFoldDevice(angle, hall);
    FoldStatus currentState = GetCurrentState();
    if (std::isless(angle, ANGLE_MIN_VAL)) {
        return currentState;
    }
    FoldStatus state;

    if (allowUserSensorForLargeFoldDevice == SMALLER_BOUNDARY_FLAG) {
        if (std::islessequal(angle, OPEN_HALF_FOLDED_MIN_THRESHOLD) && hall == HALL_FOLDED_THRESHOLD) {
            state = FoldStatus::FOLDED;
        } else if (std::isgreaterequal(angle, OPEN_HALF_FOLDED_MIN_THRESHOLD + HALF_FOLDED_BUFFER) &&
            hall == HALL_FOLDED_THRESHOLD) {
            state = FoldStatus::HALF_FOLD;
        } else if (std::islessequal(angle, HALF_FOLDED_MAX_THRESHOLD - HALF_FOLDED_BUFFER) &&
            hall == HALL_THRESHOLD) {
            state = FoldStatus::HALF_FOLD;
        } else if (std::isgreaterequal(angle, HALF_FOLDED_MAX_THRESHOLD)) {
            state = FoldStatus::EXPAND;
        } else {
            state = currentState;
            if (state == FoldStatus::UNKNOWN) {
                state = FoldStatus::HALF_FOLD;
            }
        }
        return state;
    }

    if (hall == HALL_THRESHOLD && (std::fabs(angle - OPEN_HALF_FOLDED_MIN_THRESHOLD) < 1e-3)) {
        state = currentState;
    } else if (std::islessequal(angle, CLOSE_HALF_FOLDED_MIN_THRESHOLD)) {
        state = FoldStatus::FOLDED;
    } else if (std::islessequal(angle, HALF_FOLDED_MAX_THRESHOLD - HALF_FOLDED_BUFFER) &&
        std::isgreater(angle, CLOSE_HALF_FOLDED_MIN_THRESHOLD + HALF_FOLDED_BUFFER)) {
        state = FoldStatus::HALF_FOLD;
    } else if (std::isgreaterequal(angle, HALF_FOLDED_MAX_THRESHOLD)) {
        state = FoldStatus::EXPAND;
    } else {
        state = currentState;
        if (state == FoldStatus::UNKNOWN) {
            state = FoldStatus::HALF_FOLD;
        }
    }
    return state;
}

void SingleDisplaySensorFoldStateManager::RegisterApplicationStateObserver() {}
} // namespace OHOS::Rosen