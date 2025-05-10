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

#ifdef SENSOR_ENABLE
#include <cmath>
#include <hisysevent.h>
#include <parameters.h>

#include "fold_screen_controller/secondary_fold_sensor_manager.h"
#include "fold_screen_state_internel.h"
#include "window_manager_hilog.h"
#include "screen_session_manager.h"

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

namespace OHOS {
namespace Rosen {
namespace {
constexpr float ANGLE_MIN_VAL = 0.0F;
constexpr float ANGLE_MAX_VAL = 180.0F;
constexpr int32_t SENSOR_SUCCESS = 0;
constexpr int32_t POSTURE_INTERVAL = 100000000;
constexpr uint16_t SENSOR_EVENT_FIRST_DATA = 0;
constexpr uint16_t HALL_B_C_COLUMN_ORDER = 1;
constexpr uint16_t HALL_A_B_COLUMN_ORDER = 4;
constexpr float ACCURACY_ERROR_FOR_ALTA = 0.0001F;
constexpr float MINI_NOTIFY_FOLD_ANGLE = 0.5F;
std::vector<float> oldFoldAngle = {0.0F, 0.0F};
constexpr size_t SECONDARY_POSTURE_SIZE = 3;
constexpr size_t SECONDARY_HALL_SIZE = 2;
constexpr uint16_t FIRST_DATA = 0;
constexpr uint16_t SECOND_DATA = 1;
constexpr uint16_t THIRD_DATA = 2;
const int32_t MAIN_STATUS_WIDTH = 0;
const int32_t FULL_STATUS_WIDTH = 1;
const int32_t GLOBAL_FULL_STATUS_WIDTH = 2;
const int32_t SCREEN_HEIGHT = 3;
const int32_t FULL_STATUS_OFFSET_X = 4;
const int32_t PARAMS_VECTOR_SIZE = 5;
} // namespace
WM_IMPLEMENT_SINGLE_INSTANCE(SecondaryFoldSensorManager);

static void SecondarySensorPostureDataCallback(SensorEvent *event)
{
    SecondaryFoldSensorManager::GetInstance().HandlePostureData(event);
}

static void SecondarySensorHallDataCallbackExt(SensorEvent *event)
{
    SecondaryFoldSensorManager::GetInstance().HandleHallDataExt(event);
}

void SecondaryFoldSensorManager::SetFoldScreenPolicy(sptr<FoldScreenPolicy> foldScreenPolicy)
{
    foldScreenPolicy_ = foldScreenPolicy;
}

void SecondaryFoldSensorManager::SetSensorFoldStateManager(sptr<SensorFoldStateManager> sensorFoldStateManager)
{
    sensorFoldStateManager_ = sensorFoldStateManager;
}

void SecondaryFoldSensorManager::RegisterPostureCallback()
{
    postureUser.callback = SecondarySensorPostureDataCallback;
    int32_t subscribeRet = SubscribeSensor(SENSOR_TYPE_ID_POSTURE, &postureUser);
    int32_t setBatchRet = SetBatch(SENSOR_TYPE_ID_POSTURE, &postureUser, POSTURE_INTERVAL, POSTURE_INTERVAL);
    int32_t activateRet = ActivateSensor(SENSOR_TYPE_ID_POSTURE, &postureUser);
    TLOGI(WmsLogTag::DMS,
        "subscribeRet: %{public}d, setBatchRet: %{public}d, activateRet: %{public}d",
        subscribeRet, setBatchRet, activateRet);
    if (subscribeRet != SENSOR_SUCCESS || setBatchRet != SENSOR_SUCCESS || activateRet != SENSOR_SUCCESS) {
        TLOGE(WmsLogTag::DMS, "failed.");
    } else {
        registerPosture_ = true;
        TLOGI(WmsLogTag::DMS, "success.");
    }
}

void SecondaryFoldSensorManager::UnRegisterPostureCallback()
{
    int32_t deactivateRet = DeactivateSensor(SENSOR_TYPE_ID_POSTURE, &postureUser);
    int32_t unsubscribeRet = UnsubscribeSensor(SENSOR_TYPE_ID_POSTURE, &postureUser);
    TLOGI(WmsLogTag::DMS, "deactivateRet: %{public}d, unsubscribeRet: %{public}d",
        deactivateRet, unsubscribeRet);
    if (deactivateRet == SENSOR_SUCCESS && unsubscribeRet == SENSOR_SUCCESS) {
        registerPosture_ = false;
        TLOGI(WmsLogTag::DMS, "success.");
    }
}

void SecondaryFoldSensorManager::RegisterHallCallback()
{
    hallUser.callback = SecondarySensorHallDataCallbackExt;
    int32_t subscribeRet = SubscribeSensor(SENSOR_TYPE_ID_HALL_EXT, &hallUser);
    TLOGI(WmsLogTag::DMS, "subscribeRet: %{public}d", subscribeRet);
    int32_t setBatchRet = SetBatch(SENSOR_TYPE_ID_HALL_EXT, &hallUser, POSTURE_INTERVAL, POSTURE_INTERVAL);
    TLOGI(WmsLogTag::DMS, "setBatchRet: %{public}d", setBatchRet);
    int32_t activateRet = ActivateSensor(SENSOR_TYPE_ID_HALL_EXT, &hallUser);
    TLOGI(WmsLogTag::DMS, "activateRet: %{public}d", activateRet);
    if (subscribeRet != SENSOR_SUCCESS || setBatchRet != SENSOR_SUCCESS || activateRet != SENSOR_SUCCESS) {
        TLOGE(WmsLogTag::DMS, "failed.");
    } else {
        TLOGI(WmsLogTag::DMS, "success.");
    }
}

void SecondaryFoldSensorManager::UnRegisterHallCallback()
{
    int32_t deactivateRet = DeactivateSensor(SENSOR_TYPE_ID_HALL_EXT, &hallUser);
    int32_t unsubscribeRet = UnsubscribeSensor(SENSOR_TYPE_ID_HALL_EXT, &hallUser);
    if (deactivateRet == SENSOR_SUCCESS && unsubscribeRet == SENSOR_SUCCESS) {
        TLOGW(WmsLogTag::DMS, "success.");
    }
}

void SecondaryFoldSensorManager::HandlePostureData(const SensorEvent * const event)
{
    float postureBc = 0.0F;
    float postureAb = 0.0F;
    float postureAbAnti = 0.0F;
    if (!GetPostureInner(event, postureBc, postureAb, postureAbAnti)) {
        return;
    }
    globalAngle_[FIRST_DATA] = postureBc;
    globalAngle_[SECOND_DATA] = postureAb;
    globalAngle_[THIRD_DATA] = postureAbAnti;
    if (IsDataBeyondBoundary()) {
        return;
    }
    TLOGD(WmsLogTag::DMS, "%{public}s, %{public}s",
        FoldScreenStateInternel::TransVec2Str(globalAngle_, "angle").c_str(),
        FoldScreenStateInternel::TransVec2Str(globalHall_, "hall").c_str());
    if (sensorFoldStateManager_ == nullptr) {
        return;
    }
    sensorFoldStateManager_->HandleAngleOrHallChange(globalAngle_, globalHall_, foldScreenPolicy_);
    NotifyFoldAngleChanged(globalAngle_);
}

void SecondaryFoldSensorManager::NotifyFoldAngleChanged(const std::vector<float> &angles)
{
    if (angles.size() < SECONDARY_HALL_SIZE) {
        return;
    }
    bool bcFlag = fabs(angles[0] - oldFoldAngle[0]) < MINI_NOTIFY_FOLD_ANGLE;
    bool abFlag = fabs(angles[1] - oldFoldAngle[1]) < MINI_NOTIFY_FOLD_ANGLE;
    if (bcFlag && abFlag) {
        return;
    }
    oldFoldAngle[0] = angles[0];
    oldFoldAngle[1] = angles[1];
    std::vector<float> notifyAngles = {angles[0], angles[1]};
    ScreenSessionManager::GetInstance().NotifyFoldAngleChanged(notifyAngles);
}

void SecondaryFoldSensorManager::HandleHallDataExt(const SensorEvent * const event)
{
    uint16_t hallBc = 0;
    uint16_t hallAb = 0;
    if (!GetHallInner(event, hallBc, hallAb)) {
        return;
    }
    globalHall_[FIRST_DATA] = hallBc;
    globalHall_[SECOND_DATA] = hallAb;
    if (IsDataBeyondBoundary()) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "%{public}s, %{public}s",
        FoldScreenStateInternel::TransVec2Str(globalAngle_, "angle").c_str(),
        FoldScreenStateInternel::TransVec2Str(globalHall_, "hall").c_str());
    if (!registerPosture_) {
        globalAngle_[FIRST_DATA] = ANGLE_MIN_VAL;
        globalAngle_[SECOND_DATA] = ANGLE_MIN_VAL;
        globalAngle_[THIRD_DATA] = ANGLE_MIN_VAL;
    }
    if (sensorFoldStateManager_ == nullptr) {
        return;
    }
    sensorFoldStateManager_->HandleAngleOrHallChange(globalAngle_, globalHall_, foldScreenPolicy_);
    return;
}

bool SecondaryFoldSensorManager::IsDataBeyondBoundary()
{
    if (globalAngle_.size() < SECONDARY_POSTURE_SIZE || globalHall_.size() < SECONDARY_HALL_SIZE) {
        TLOGE(WmsLogTag::DMS, "oversize, global angles: %{public}zu, halls size: %{public}zu.",
            globalAngle_.size(), globalHall_.size());
        return true;
    }
    for (size_t i = 0; i < SECONDARY_POSTURE_SIZE - 1; i++) {
        float angle = globalAngle_[i];
        if (std::isless(angle, ANGLE_MIN_VAL) ||
            std::isgreater(angle, ANGLE_MAX_VAL + ACCURACY_ERROR_FOR_ALTA)) {
            TLOGE(WmsLogTag::DMS, "i = %{public}zu, angle = %{public}f", i, angle);
            return true;
        }
    }
    for (size_t i = 0; i < SECONDARY_HALL_SIZE; i++) {
        uint16_t hall = globalHall_[i];
        if (hall != 0 && hall != 1) {
            TLOGE(WmsLogTag::DMS, "i = %{public}zu, hall = %{public}u", i, hall);
            return true;
        }
    }
    return false;
}

bool SecondaryFoldSensorManager::GetPostureInner(const SensorEvent * const event, float &valueBc, float &valueAb,
    float &valueAbAnti)
{
    if (event == nullptr) {
        TLOGW(WmsLogTag::DMS, "SensorEvent is nullptr.");
        return false;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].data == nullptr) {
        TLOGW(WmsLogTag::DMS, "SensorEvent[0].data is nullptr.");
        return false;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].dataLen < sizeof(FoldScreenSensorManager::PostureDataSecondary)) {
        TLOGW(WmsLogTag::DMS, "SensorEvent dataLen less than posture data size.");
        return false;
    }
    FoldScreenSensorManager::PostureDataSecondary *postureData =
        reinterpret_cast<FoldScreenSensorManager::PostureDataSecondary *>(event[SENSOR_EVENT_FIRST_DATA].data);
    valueBc = (*postureData).postureBc;
    valueAb = (*postureData).postureAb;
    valueAbAnti = (*postureData).postureAbAnti;
    TLOGD(WmsLogTag::DMS, "postureBc: %{public}f, postureAb: %{public}f, postureAbAnti: %{public}f.",
        valueBc, valueAb, valueAbAnti);
    return true;
}

bool SecondaryFoldSensorManager::GetHallInner(const SensorEvent * const event, uint16_t &valueBc, uint16_t &valueAb)
{
    if (event == nullptr) {
        TLOGW(WmsLogTag::DMS, "SensorEvent is nullptr.");
        return false;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].data == nullptr) {
        TLOGW(WmsLogTag::DMS, "SensorEvent[0].data is nullptr.");
        return false;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].dataLen < sizeof(FoldScreenSensorManager::ExtHallData)) {
        TLOGW(WmsLogTag::DMS, "SensorEvent dataLen less than hall data size.");
        return false;
    }
    FoldScreenSensorManager::ExtHallData *extHallData =
        reinterpret_cast<FoldScreenSensorManager::ExtHallData *>(event[SENSOR_EVENT_FIRST_DATA].data);
    uint16_t flag = static_cast<uint16_t>((*extHallData).flag);
    if (!(flag & (1 << HALL_B_C_COLUMN_ORDER)) || !(flag & (1 << HALL_A_B_COLUMN_ORDER))) {
        TLOGW(WmsLogTag::DMS, "not support Extend Hall.");
        return false;
    }
    valueBc = static_cast<uint16_t>((*extHallData).hall); // axis of bc screen. 0: hall closed, 1: hall expaned
    valueAb = static_cast<uint16_t>((*extHallData).hallAb);
    TLOGI(WmsLogTag::DMS, "hallBc: %{public}u, hallAb: %{public}u.", valueBc, valueAb);
    return true;
}

void SecondaryFoldSensorManager::PowerKeySetScreenActiveRect()
{
    if (foldScreenPolicy_->GetScreenParams().size() != PARAMS_VECTOR_SIZE) {
        return;
    }
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = foldScreenPolicy_->GetScreenParams()[SCREEN_HEIGHT];
    uint32_t height = 0;
    if (foldScreenPolicy_->currentDisplayMode_ == FoldDisplayMode::FULL) {
        y = foldScreenPolicy_->GetScreenParams()[FULL_STATUS_OFFSET_X];
        height = foldScreenPolicy_->GetScreenParams()[FULL_STATUS_WIDTH];
    } else if (foldScreenPolicy_->currentDisplayMode_ == FoldDisplayMode::MAIN) {
        height = foldScreenPolicy_->GetScreenParams()[MAIN_STATUS_WIDTH];
    } else if (foldScreenPolicy_->currentDisplayMode_ == FoldDisplayMode::GLOBAL_FULL) {
        height = foldScreenPolicy_->GetScreenParams()[GLOBAL_FULL_STATUS_WIDTH];
    } else {
        TLOGW(WmsLogTag::DMS, "displayMode[%{public}u] unknown.", foldScreenPolicy_->currentDisplayMode_);
        return;
    }
    OHOS::Rect rectCur {
        .x = x,
        .y = y,
        .w = width,
        .h = height,
    };
    RSInterfaces::GetInstance().SetScreenActiveRect(0, rectCur);
    isPowerRectExe_ = true;
}

bool SecondaryFoldSensorManager::IsPostureUserCallbackInvalid() const
{
    return postureUser.callback == nullptr;
}

bool SecondaryFoldSensorManager::IsHallUserCallbackInvalid() const
{
    return hallUser.callback == nullptr;
}

std::vector<float> SecondaryFoldSensorManager::GetGlobalAngle() const
{
    return globalAngle_;
}

std::vector<uint16_t> SecondaryFoldSensorManager::GetGlobalHall() const
{
    return globalHall_;
}
} // Rosen
} // OHOS
#endif