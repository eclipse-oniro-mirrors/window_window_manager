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

#include "fold_screen_controller/sensor_fold_state_manager/secondary_display_sensor_fold_state_manager.h"
#include <parameters.h>

#include <hisysevent.h>
#include "screen_session_manager.h"
#include "fold_screen_controller/fold_screen_policy.h"
#include "fold_screen_controller/sensor_fold_state_manager/sensor_fold_state_manager.h"
#include "session/screen/include/screen_session.h"
#include "fold_screen_state_internel.h"

#include "window_manager_hilog.h"

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

namespace OHOS::Rosen {
namespace {
constexpr uint8_t HALLS_AXIS_SIZE = 2;
constexpr uint8_t ANGLES_AXIS_SIZE = 3;
constexpr float ANGLE_MIN_VAL = 0.0F;
constexpr float GRL_HALF_FOLDED_MAX_THRESHOLD = 140.0F;
constexpr float CLOSE_GRL_HALF_FOLDED_MIN_THRESHOLD = 70.0F;
constexpr float OPEN_GRL_HALF_FOLDED_MIN_THRESHOLD = 45.0F;
constexpr float GRL_HALF_FOLDED_BUFFER = 10.0F;
constexpr float LARGER_BOUNDARY_FOR_GRL_THRESHOLD = 90.0F;
constexpr int32_t LARGER_BOUNDARY_FLAG = 1;
constexpr int32_t SMALLER_BOUNDARY_FLAG = 0;
constexpr int32_t HALL_THRESHOLD = 1;
constexpr int32_t HALL_FOLDED_THRESHOLD = 0;
constexpr int32_t HALF_FOLD_VALUE = 3;
constexpr int32_t REFLEXION_VALUE = 2;
} // namespace

SecondaryDisplaySensorFoldStateManager::SecondaryDisplaySensorFoldStateManager() {}
SecondaryDisplaySensorFoldStateManager::~SecondaryDisplaySensorFoldStateManager() {}

void SecondaryDisplaySensorFoldStateManager::HandleAngleOrHallChange(const std::vector<float> &angles,
    const std::vector<uint16_t> &halls, sptr<FoldScreenPolicy> foldScreenPolicy)
{
    FoldStatus nextState = GetNextFoldState(angles, halls);
    HandleSensorChange(nextState, angles, foldScreenPolicy);
    if (angles.size() != ANGLES_AXIS_SIZE) {
        TLOGE(WmsLogTag::DMS, "angles size is not right, angles size %{public}zu", angles.size());
        return;
    }
    bool isSecondaryReflexion = static_cast<bool>(angles[REFLEXION_VALUE]);
    if (isSecondaryReflexion) {
        TLOGW(WmsLogTag::DMS, "Secondary is reflexion");
        isHasReflexioned = true;
        ReportSecondaryReflexion(static_cast<int32_t>(nextState), static_cast<int32_t>(nextState),
            isSecondaryReflexion);
        SendReflexionResult(isSecondaryReflexion);
    }
    if (isHasReflexioned && !isSecondaryReflexion) {
        TLOGW(WmsLogTag::DMS, "Secondary recover from reflexion");
        isHasReflexioned = false;
        SendReflexionResult(isSecondaryReflexion);
    }
}

FoldStatus SecondaryDisplaySensorFoldStateManager::GetNextFoldState(const std::vector<float> &angles,
    const std::vector<uint16_t> &halls)
{
    TLOGD(WmsLogTag::DMS, "%{public}s, %{public}s",
        FoldScreenStateInternel::TransVec2Str(angles, "angle").c_str(),
        FoldScreenStateInternel::TransVec2Str(halls, "hall").c_str());

    FoldStatus state = FoldStatus::UNKNOWN;
    bool isPowerOn = PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    if (angles.size() != ANGLES_AXIS_SIZE || halls.size() != HALLS_AXIS_SIZE) {
        TLOGE(WmsLogTag::DMS, "angles or halls size is not right, angles size %{public}zu, halls size %{public}zu",
            angles.size(), halls.size());
        return state;
    }
    if (!isPowerOn) {
        state = GetFoldStateUnpower(halls);
        return state;
    }
    float angleAB = angles[1];
    uint16_t hallAB = halls[1];
    float angleBC = angles[0];
    uint16_t hallBC = halls[0];

    FoldStatus nextStateSecondary = UpdateSwitchScreenBoundaryForLargeFoldDeviceAB(angleAB,
        hallAB, mNextStateAB);
    mNextStateAB = nextStateSecondary;

    FoldStatus nextStatePrimary = UpdateSwitchScreenBoundaryForLargeFoldDeviceBC(angleBC,
        hallBC, mNextStateBC);
    mNextStateBC = nextStatePrimary;

    state = GetGlobalFoldState(nextStatePrimary, nextStateSecondary);
    return state;
}

FoldStatus SecondaryDisplaySensorFoldStateManager::UpdateSwitchScreenBoundaryForLargeFoldDeviceAB(float angle,
    uint16_t hall, FoldStatus state)
{
    if (hall == HALL_FOLDED_THRESHOLD) {
        allowUserSensorForLargeFoldDeviceAB = SMALLER_BOUNDARY_FLAG;
    } else if (angle >= LARGER_BOUNDARY_FOR_GRL_THRESHOLD) {
        allowUserSensorForLargeFoldDeviceAB = LARGER_BOUNDARY_FLAG;
    }
    return GetNextFoldStateHalf(angle, hall, state, allowUserSensorForLargeFoldDeviceAB);
}

FoldStatus SecondaryDisplaySensorFoldStateManager::UpdateSwitchScreenBoundaryForLargeFoldDeviceBC(float angle,
    uint16_t hall, FoldStatus state)
{
    if (hall == HALL_FOLDED_THRESHOLD) {
        allowUserSensorForLargeFoldDeviceBC = SMALLER_BOUNDARY_FLAG;
    } else if (angle >= LARGER_BOUNDARY_FOR_GRL_THRESHOLD) {
        allowUserSensorForLargeFoldDeviceBC = LARGER_BOUNDARY_FLAG;
    }
    return GetNextFoldStateHalf(angle, hall, state, allowUserSensorForLargeFoldDeviceBC);
}

FoldStatus SecondaryDisplaySensorFoldStateManager::GetNextFoldStateHalf(float angle,
    uint16_t hall, FoldStatus CurrentState, int32_t allowUserSensorForLargeFoldDevice)
{
    if (std::isless(angle, ANGLE_MIN_VAL)) {
        return CurrentState;
    }
    FoldStatus state = FoldStatus::UNKNOWN;

    if (allowUserSensorForLargeFoldDevice == SMALLER_BOUNDARY_FLAG) {
        if (std::islessequal(angle, OPEN_GRL_HALF_FOLDED_MIN_THRESHOLD)) {
            state = FoldStatus::FOLDED;
        } else if (std::isgreaterequal(angle, OPEN_GRL_HALF_FOLDED_MIN_THRESHOLD + GRL_HALF_FOLDED_BUFFER) &&
            hall == HALL_FOLDED_THRESHOLD) {
            state = FoldStatus::HALF_FOLD;
        } else if (std::islessequal(angle, GRL_HALF_FOLDED_MAX_THRESHOLD - GRL_HALF_FOLDED_BUFFER) &&
            hall == HALL_THRESHOLD) {
            state = FoldStatus::HALF_FOLD;
        } else if (std::isgreaterequal(angle, GRL_HALF_FOLDED_MAX_THRESHOLD)) {
            state = FoldStatus::EXPAND;
        } else {
            state = CurrentState;
            if (state == FoldStatus::UNKNOWN) {
                state = FoldStatus::HALF_FOLD;
            }
        }
        return state;
    }

    if (hall == HALL_THRESHOLD && angle == OPEN_GRL_HALF_FOLDED_MIN_THRESHOLD) {
        state = CurrentState;
    } else if (std::islessequal(angle, CLOSE_GRL_HALF_FOLDED_MIN_THRESHOLD)) {
        state = FoldStatus::FOLDED;
    } else if (std::islessequal(angle, GRL_HALF_FOLDED_MAX_THRESHOLD - GRL_HALF_FOLDED_BUFFER) &&
        std::isgreater(angle, CLOSE_GRL_HALF_FOLDED_MIN_THRESHOLD + GRL_HALF_FOLDED_BUFFER)) {
        state = FoldStatus::HALF_FOLD;
    } else if (std::isgreaterequal(angle, GRL_HALF_FOLDED_MAX_THRESHOLD)) {
        state = FoldStatus::EXPAND;
    } else {
        state = CurrentState;
        if (state == FoldStatus::UNKNOWN) {
            state = FoldStatus::HALF_FOLD;
        }
    }
    return state;
}

FoldStatus SecondaryDisplaySensorFoldStateManager::GetGlobalFoldState(FoldStatus mPrimaryFoldState,
    FoldStatus mSecondaryFoldState)
{
    FoldStatus defaultState = FoldStatus::FOLDED;
    if (mSecondaryFoldState == FoldStatus::UNKNOWN || mSecondaryFoldState == defaultState) {
        return mPrimaryFoldState;
    }
    int32_t mPrimaryFoldStatus = static_cast<int32_t>(mPrimaryFoldState);
    int32_t mSecondaryFoldStatus = static_cast<int32_t>(mSecondaryFoldState);
    if (mSecondaryFoldStatus == HALF_FOLD_VALUE) {
        mSecondaryFoldStatus --;
    }
    int32_t globalFoldStatus = mPrimaryFoldStatus + mSecondaryFoldStatus * 10;
    FoldStatus globalFoldState = static_cast<FoldStatus>(globalFoldStatus);
    return globalFoldState;
}

FoldStatus SecondaryDisplaySensorFoldStateManager::GetFoldStateUnpower(const std::vector<uint16_t> &halls)
{
    TLOGW(WmsLogTag::DMS, "The screen is not currently lit");
    FoldStatus state = FoldStatus::UNKNOWN;
    int hall1 = halls[0];
    int hall2 = halls[1];
    if (hall2 == HALL_THRESHOLD && hall1 == HALL_THRESHOLD) {
        state = FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND;
    } else if (hall2 == HALL_FOLDED_THRESHOLD && hall1 == HALL_THRESHOLD) {
        state = FoldStatus::EXPAND;
    } else if (hall2 == HALL_FOLDED_THRESHOLD && hall1 == HALL_FOLDED_THRESHOLD) {
        state = FoldStatus::FOLDED;
    } else if (hall2 == HALL_THRESHOLD && hall1 == HALL_FOLDED_THRESHOLD) {
        state = FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED;
    }
    return state;
}

void SecondaryDisplaySensorFoldStateManager::ReportSecondaryReflexion(int32_t currentStatus, int32_t nextStatus,
    bool isSecondaryReflexion)
{
    TLOGW(WmsLogTag::DMS, "ReportSecondaryReflexion currentStatus: %{public}d, isSecondaryReflexion: %{public}d",
        currentStatus, isSecondaryReflexion);
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "NOTIFY_FOLD_STATE_CHANGE",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "CURRENT_FOLD_STATUS", currentStatus,
        "NEXT_FOLD_STATUS", nextStatus,
        "SENSOR_POSTURE", isSecondaryReflexion);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "Write HiSysEvent error, ret: %{public}d", ret);
    }
}

void SecondaryDisplaySensorFoldStateManager::SendReflexionResult(bool isSecondaryReflexion)
{
    auto screenSession = ScreenSessionManager::GetInstance().GetDefaultScreenSession();
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen session is null!");
        return;
    }
    ScreenId screenId = screenSession->GetScreenId();
    ScreenSessionManager::GetInstance().OnSecondaryReflexionChange(screenId, isSecondaryReflexion);
}
} // namespace OHOS::Rosen