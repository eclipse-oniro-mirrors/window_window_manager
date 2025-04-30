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
 
#include <cmath>
#include <hisysevent.h>
#include <parameters.h>
#include <vector>
 
#include "dm_common.h"
 
#include "fold_screen_controller/super_fold_sensor_manager.h"
#include "fold_screen_controller/super_fold_state_manager.h"
#include "window_manager_hilog.h"
#include "screen_session_manager.h"
 
namespace OHOS {
 
namespace Rosen {
namespace {
constexpr float ANGLE_MIN_VAL = 30.0F;
constexpr float ANGLE_MAX_VAL = 180.0F;
constexpr float ANGLE_FLAT_THRESHOLD = 160.0F;
constexpr float ANGLE_SENSOR_THRESHOLD = 160.0F;
constexpr float ANGLE_HALF_FOLD_THRESHOLD = 135.0F;
constexpr uint16_t HALL_HAVE_KEYBOARD_THRESHOLD = 0B0100;
constexpr uint16_t HALL_REMOVE_KEYBOARD_THRESHOLD = 0B0000;
constexpr uint16_t HALL_ACTIVE = 1 << 2;
constexpr int32_t SENSOR_SUCCESS = 0;
constexpr int32_t POSTURE_INTERVAL = 100000000; // 100ms
constexpr int32_t POSTURE_INTERVAL_FOR_WIDE_ANGLE = 10000000; // 10ms
constexpr float UNFOLD_ANGLE = 170.0F;
constexpr uint16_t SENSOR_EVENT_FIRST_DATA = 0;
constexpr float ACCURACY_ERROR_FOR_PC = 0.0001F;
} // namespace

SuperFoldSensorManager &SuperFoldSensorManager::GetInstance()
{
    static SuperFoldSensorManager SuperFoldSensorManager;
    return SuperFoldSensorManager;
}

static void SensorPostureDataCallback(SensorEvent *event)
{
    OHOS::Rosen::SuperFoldSensorManager::GetInstance().HandlePostureData(event);
}

static void SensorHallDataCallback(SensorEvent *event)
{
    OHOS::Rosen::SuperFoldSensorManager::GetInstance().HandleHallData(event);
}

void SuperFoldSensorManager::RegisterPostureCallback()
{
    curInterval_ = POSTURE_INTERVAL;
    postureUser.callback = SensorPostureDataCallback;
    int32_t subscribeRet = SubscribeSensor(SENSOR_TYPE_ID_POSTURE, &postureUser);
    int32_t setBatchRet = SetBatch(SENSOR_TYPE_ID_POSTURE, &postureUser, POSTURE_INTERVAL, POSTURE_INTERVAL);
    int32_t activateRet = ActivateSensor(SENSOR_TYPE_ID_POSTURE, &postureUser);
    TLOGI(WmsLogTag::DMS_SSM,
        "subscribeRet: %{public}d, setBatchRet: %{public}d, activateRet: %{public}d",
        subscribeRet, setBatchRet, activateRet);
    if (subscribeRet != SENSOR_SUCCESS || setBatchRet != SENSOR_SUCCESS || activateRet != SENSOR_SUCCESS) {
        TLOGI(WmsLogTag::DMS_SSM, "RegisterPostureCallback failed.");
    } else {
        TLOGI(WmsLogTag::DMS_SSM, "RegisterPostureCallback success.");
    }
}

void SuperFoldSensorManager::UnregisterPostureCallback()
{
    int32_t deactivateRet = DeactivateSensor(SENSOR_TYPE_ID_POSTURE, &postureUser);
    int32_t unsubscribeRet = UnsubscribeSensor(SENSOR_TYPE_ID_POSTURE, &postureUser);
    TLOGI(WmsLogTag::DMS_SSM, "deactivateRet: %{public}d, unsubscribeRet: %{public}d",
        deactivateRet, unsubscribeRet);
    if (deactivateRet == SENSOR_SUCCESS && unsubscribeRet == SENSOR_SUCCESS) {
        TLOGI(WmsLogTag::DMS_SSM, "FoldScreenSensorManager.UnRegisterPostureCallback success.");
    }
}

void SuperFoldSensorManager::RegisterHallCallback()
{
    hallUser.callback = SensorHallDataCallback;
    int32_t subscribeRet = SubscribeSensor(SENSOR_TYPE_ID_HALL, &hallUser);
    TLOGI(WmsLogTag::DMS_SSM, "subscribeRet: %{public}d", subscribeRet);
    int32_t setBatchRet = SetBatch(SENSOR_TYPE_ID_HALL, &hallUser, POSTURE_INTERVAL, POSTURE_INTERVAL);
    TLOGI(WmsLogTag::DMS_SSM, "setBatchRet: %{public}d", setBatchRet);
    int32_t activateRet = ActivateSensor(SENSOR_TYPE_ID_HALL, &hallUser);
    TLOGI(WmsLogTag::DMS_SSM, "activateRet: %{public}d", activateRet);
    if (subscribeRet != SENSOR_SUCCESS || setBatchRet != SENSOR_SUCCESS || activateRet != SENSOR_SUCCESS) {
        TLOGI(WmsLogTag::DMS_SSM, "RegisterHallCallback failed.");
    }
}

void SuperFoldSensorManager::UnregisterHallCallback()
{
    int32_t deactivateRet = DeactivateSensor(SENSOR_TYPE_ID_HALL, &hallUser);
    int32_t unsubscribeRet = UnsubscribeSensor(SENSOR_TYPE_ID_HALL, &hallUser);
    if (deactivateRet == SENSOR_SUCCESS && unsubscribeRet == SENSOR_SUCCESS) {
        TLOGI(WmsLogTag::DMS_SSM, "UnRegisterHallCallback success.");
    }
}

void SuperFoldSensorManager::HandlePostureData(const SensorEvent * const event)
{
    if (event == nullptr) {
        TLOGI(WmsLogTag::DMS_SSM, "SensorEvent is nullptr.");
        return;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].data == nullptr) {
        TLOGI(WmsLogTag::DMS_SSM, "SensorEvent[0].data is nullptr.");
        return;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].dataLen < sizeof(PostureData)) {
        TLOGI(WmsLogTag::DMS_SSM, "SensorEvent dataLen less than posture data size.");
        return;
    }
    PostureData *postureData = reinterpret_cast<PostureData *>(event[SENSOR_EVENT_FIRST_DATA].data);
    curAngle_ = (*postureData).angle;
    if (curAngle_ > UNFOLD_ANGLE && curInterval_ != POSTURE_INTERVAL_FOR_WIDE_ANGLE) {
        int32_t setBatchRet = SetBatch(SENSOR_TYPE_ID_POSTURE, &postureUser,
            POSTURE_INTERVAL_FOR_WIDE_ANGLE, POSTURE_INTERVAL_FOR_WIDE_ANGLE);
        int32_t activateRet = ActivateSensor(SENSOR_TYPE_ID_POSTURE, &postureUser);
        if (setBatchRet == 0 && activateRet == 0) {
            curInterval_ = POSTURE_INTERVAL_FOR_WIDE_ANGLE;
        }
    } else if (curAngle_ < UNFOLD_ANGLE && curInterval_ != POSTURE_INTERVAL) {
        int32_t setBatchRet = SetBatch(SENSOR_TYPE_ID_POSTURE, &postureUser, POSTURE_INTERVAL, POSTURE_INTERVAL);
        int32_t activateRet = ActivateSensor(SENSOR_TYPE_ID_POSTURE, &postureUser);
        if (setBatchRet == 0 && activateRet == 0) {
            curInterval_ = POSTURE_INTERVAL;
        }
    }
    if (std::isgreater(curAngle_, ANGLE_MAX_VAL + ACCURACY_ERROR_FOR_PC)) {
        TLOGI(WmsLogTag::DMS_SSM, "Invalid value, angle value is: %{public}f.", curAngle_);
        return;
    }
    TLOGD(WmsLogTag::DMS_SSM, "angle value is: %{public}f.", curAngle_);
    NotifyFoldAngleChanged(curAngle_);
}

void SuperFoldSensorManager::NotifyFoldAngleChanged(float foldAngle)
{
    SuperFoldStatusChangeEvents events = SuperFoldStatusChangeEvents::UNDEFINED;
    if (std::isgreaterequal(foldAngle, ANGLE_FLAT_THRESHOLD)) {
        TLOGD(WmsLogTag::DMS_SSM, "NotifyFoldAngleChanged is Expanded");
        events = SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED;
    } else if (std::isless(foldAngle, ANGLE_HALF_FOLD_THRESHOLD) &&
        std::isgreater(foldAngle, ANGLE_MIN_VAL)) {
        TLOGD(WmsLogTag::DMS_SSM, "NotifyFoldAngleChanged is Half Folded");
        events = SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED;
    } else if (std::islessequal(foldAngle, ANGLE_MIN_VAL)) {
        TLOGD(WmsLogTag::DMS_SSM, "NotifyFoldAngleChanged is Folded");
        events = SuperFoldStatusChangeEvents::ANGLE_CHANGE_FOLDED;
    } else {
        if (SuperFoldStateManager::GetInstance().GetCurrentStatus() == SuperFoldStatus::UNKNOWN) {
            events = SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED;
        }
        TLOGD(WmsLogTag::DMS_SSM, "NotifyFoldAngleChanged is in BufferArea");
    }
    // notify
    std::vector<float> foldAngles;
    foldAngles.push_back(foldAngle);
    ScreenSessionManager::GetInstance().NotifyFoldAngleChanged(foldAngles);
    if (!ScreenRotationProperty::isDeviceHorizontal() ||
        events == SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED) {
        HandleSuperSensorChange(events);
    }
}

void SuperFoldSensorManager::HandleHallData(const SensorEvent * const event)
{
    if (event == nullptr) {
        TLOGI(WmsLogTag::DMS_SSM, "SensorEvent is nullptr.");
        return;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].data == nullptr) {
        TLOGI(WmsLogTag::DMS_SSM, "SensorEvent[0].data is nullptr.");
        return;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].dataLen < sizeof(HallData)) {
        TLOGI(WmsLogTag::DMS_SSM, "SensorEvent[0].dataLen is nullptr.");
        return;
    }
    auto data = reinterpret_cast<HallData*>(event->data);
    auto status = static_cast<uint32_t>(data->status);
    TLOGI(WmsLogTag::DMS_SSM, "HallData status is: %{public}u.", status);
    
    if (curHall_ == (status & HALL_ACTIVE)) {
        TLOGI(WmsLogTag::DMS_SSM, "Hall don't change, hall = %{public}u", curHall_);
        return;
    }
    curHall_ = (status & HALL_ACTIVE);
    TLOGI(WmsLogTag::DMS_SSM, "Hall change, hall = %{public}u", curHall_);
    NotifyHallChanged(curHall_);
}

void SuperFoldSensorManager::NotifyHallChanged(uint16_t Hall)
{
    SuperFoldStatusChangeEvents events;
    if (Hall == HALL_REMOVE_KEYBOARD_THRESHOLD) {
        TLOGI(WmsLogTag::DMS_SSM, "NotifyHallChanged: Keyboard off!");
        events = SuperFoldStatusChangeEvents::KEYBOARD_OFF;
    } else if (Hall == HALL_HAVE_KEYBOARD_THRESHOLD) {
        TLOGI(WmsLogTag::DMS_SSM, "NotifyHallChanged: Keyboard on!");
        events = SuperFoldStatusChangeEvents::KEYBOARD_ON;
        HandleSuperSensorChange(SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED);
    } else {
        TLOGI(WmsLogTag::DMS_SSM, "NotifyHallChanged: Invalid Hall Value!");
        return;
    }
    // notify
    HandleSuperSensorChange(events);
    if (std::isgreater(curAngle_, ANGLE_SENSOR_THRESHOLD)) {
        HandleSuperSensorChange(SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED);
    }
}

void SuperFoldSensorManager::HandleSuperSensorChange(SuperFoldStatusChangeEvents events)
{
    // trigger events
    if (ScreenSessionManager::GetInstance().GetIsExtendScreenConnected()) {
        return;
    }
    SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(events);
}

void SuperFoldSensorManager::HandleScreenConnectChange()
{
    TLOGI(WmsLogTag::DMS_SSM, "Screen connect to stop statemachine.");
    if (SuperFoldStateManager::GetInstance().GetCurrentStatus() == SuperFoldStatus::KEYBOARD) {
        SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(
            SuperFoldStatusChangeEvents::KEYBOARD_OFF);
        SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(
            SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED);
    } else {
        SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(
            SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED);
    }
}

void SuperFoldSensorManager::HandleScreenDisconnectChange()
{
    TLOGI(WmsLogTag::DMS_SSM, "Screen disconnect to stop statemachine.");
    NotifyHallChanged(curHall_);
    NotifyFoldAngleChanged(curAngle_);
}

float SuperFoldSensorManager::GetCurAngle()
{
    return curAngle_;
}

SuperFoldSensorManager::SuperFoldSensorManager() {}

SuperFoldSensorManager::~SuperFoldSensorManager() {}
} // Rosen
} // OHOS