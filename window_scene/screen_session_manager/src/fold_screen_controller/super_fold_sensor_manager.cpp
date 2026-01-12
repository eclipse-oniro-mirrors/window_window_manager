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
#include "fold_screen_controller/fold_screen_controller_config.h"
#include "window_manager_hilog.h"
#include "screen_session_manager.h"
#include "screen_sensor_mgr.h"

namespace OHOS {
 
namespace Rosen {
namespace {
constexpr float ANGLE_MAX_VAL = 180.0F;
constexpr float ANGLE_FLAT_THRESHOLD = 160.0F;
constexpr float ANGLE_SENSOR_THRESHOLD = 160.0F;
constexpr float ANGLE_HALF_FOLD_THRESHOLD = 150.0F;
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
    DMS::ScreenSensorMgr::GetInstance().SubscribeSensorCallback(
        SENSOR_TYPE_ID_POSTURE, POSTURE_INTERVAL, SensorPostureDataCallback);
}

void SuperFoldSensorManager::UnregisterPostureCallback()
{
    int ret = DMS::ScreenSensorMgr::GetInstance().UnSubscribeSensorCallback(
        SENSOR_TYPE_ID_POSTURE);
    if (ret == SENSOR_SUCCESS) {
        TLOGI(WmsLogTag::DMS, "success.");
    } else {
        TLOGE(WmsLogTag::DMS, "UnregisterPostureCallback failed with ret: %{public}d", ret);
    }
}

void SuperFoldSensorManager::RegisterHallCallback()
{
    DMS::ScreenSensorMgr::GetInstance().SubscribeSensorCallback(
        SENSOR_TYPE_ID_HALL, POSTURE_INTERVAL, SensorHallDataCallback);
}

void SuperFoldSensorManager::UnregisterHallCallback()
{
    int ret = DMS::ScreenSensorMgr::GetInstance().UnSubscribeSensorCallback(SENSOR_TYPE_ID_HALL_EXT);
    if (ret == SENSOR_SUCCESS) {
        TLOGI(WmsLogTag::DMS, "success.");
    } else {
        TLOGE(WmsLogTag::DMS, "UnRegisterHallCallback failed with ret: %{public}d", ret);
    }
}

void SuperFoldSensorManager::HandlePostureData(const SensorEvent * const event)
{
    if (event == nullptr) {
        TLOGI(WmsLogTag::DMS, "SensorEvent is nullptr.");
        return;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].data == nullptr) {
        TLOGI(WmsLogTag::DMS, "SensorEvent[0].data is nullptr.");
        return;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].dataLen < sizeof(PostureData)) {
        TLOGI(WmsLogTag::DMS, "SensorEvent dataLen less than posture data size.");
        return;
    }
    PostureData *postureData = reinterpret_cast<PostureData *>(event[SENSOR_EVENT_FIRST_DATA].data);
    curAngle_ = (*postureData).angle;
    if (curAngle_ > UNFOLD_ANGLE && curInterval_ != POSTURE_INTERVAL_FOR_WIDE_ANGLE) {
        int32_t ret = DMS::ScreenSensorMgr::GetInstance().UpdateSensorInterval(
            SENSOR_TYPE_ID_POSTURE, POSTURE_INTERVAL_FOR_WIDE_ANGLE);
        if (ret == SENSOR_SUCCESS) {
            curInterval_ = POSTURE_INTERVAL_FOR_WIDE_ANGLE;
        }
    } else if (curAngle_ < UNFOLD_ANGLE && curInterval_ != POSTURE_INTERVAL) {
        int32_t ret =
            DMS::ScreenSensorMgr::GetInstance().UpdateSensorInterval(SENSOR_TYPE_ID_POSTURE, POSTURE_INTERVAL);
        if (ret == SENSOR_SUCCESS) {
            curInterval_ = POSTURE_INTERVAL;
        }
    }
    if (std::isgreater(curAngle_, ANGLE_MAX_VAL + ACCURACY_ERROR_FOR_PC)) {
        TLOGI(WmsLogTag::DMS, "Invalid value, angle value is: %{public}f.", curAngle_);
        return;
    }
    TLOGD(WmsLogTag::DMS, "angle value is: %{public}f.", curAngle_);
    auto task = [this, curAngle = curAngle_] {
        NotifyFoldAngleChanged(curAngle);
    };
    if (taskScheduler_) {
        taskScheduler_->PostAsyncTask(task, "DMSHandlePosture");
    }
}

void SuperFoldSensorManager::NotifyFoldAngleChanged(float foldAngle)
{
    SuperFoldStatusChangeEvents events = SuperFoldStatusChangeEvents::UNDEFINED;
    if (std::isgreaterequal(foldAngle, ANGLE_FLAT_THRESHOLD)) {
        TLOGD(WmsLogTag::DMS, "NotifyFoldAngleChanged is Expanded");
        events = SuperFoldStatusChangeEvents::ANGLE_CHANGE_EXPANDED;
    } else if (std::isless(foldAngle, ANGLE_HALF_FOLD_THRESHOLD) &&
        std::isgreater(foldAngle, ANGLE_MIN_VAL)) {
        TLOGD(WmsLogTag::DMS, "NotifyFoldAngleChanged is Half Folded");
        events = SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED;
    } else if (std::islessequal(foldAngle, ANGLE_MIN_VAL)) {
        TLOGD(WmsLogTag::DMS, "NotifyFoldAngleChanged is Folded");
        events = SuperFoldStatusChangeEvents::ANGLE_CHANGE_FOLDED;
    } else {
        if (SuperFoldStateManager::GetInstance().GetCurrentStatus() == SuperFoldStatus::UNKNOWN ||
        SuperFoldStateManager::GetInstance().GetCurrentStatus() == SuperFoldStatus::FOLDED) {
            events = SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED;
        }
        TLOGD(WmsLogTag::DMS, "NotifyFoldAngleChanged is in BufferArea");
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
        TLOGI(WmsLogTag::DMS, "SensorEvent is nullptr.");
        return;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].data == nullptr) {
        TLOGI(WmsLogTag::DMS, "SensorEvent[0].data is nullptr.");
        return;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].dataLen < sizeof(HallData)) {
        TLOGI(WmsLogTag::DMS, "SensorEvent[0].dataLen is nullptr.");
        return;
    }
    auto data = reinterpret_cast<HallData*>(event->data);
    auto status = static_cast<uint32_t>(data->status);
    TLOGI(WmsLogTag::DMS, "HallData status is: %{public}u.", status);
    
    if (curHall_ == (status & HALL_ACTIVE)) {
        TLOGI(WmsLogTag::DMS, "Hall don't change, hall = %{public}u", curHall_);
        return;
    }
    curHall_ = (status & HALL_ACTIVE);
    TLOGI(WmsLogTag::DMS, "Hall change, hall = %{public}u", curHall_);
    auto task = [this, curHall = curHall_] {
        NotifyHallChanged(curHall);
    };
    if (taskScheduler_) {
        taskScheduler_->PostAsyncTask(task, "DMSHandleHall");
    }
}

void SuperFoldSensorManager::NotifyHallChanged(uint16_t Hall)
{
    SuperFoldStatusChangeEvents events;
    if (Hall == HALL_REMOVE_KEYBOARD_THRESHOLD) {
        TLOGD(WmsLogTag::DMS, "NotifyHallChanged: Keyboard off!");
        events = SuperFoldStatusChangeEvents::KEYBOARD_OFF;
    } else if (Hall == HALL_HAVE_KEYBOARD_THRESHOLD) {
        if (SuperFoldStateManager::GetInstance().GetCurrentStatus() == SuperFoldStatus::EXPANDED) {
            TLOGI(WmsLogTag::DMS, "is expanded");
            return;
        }
        TLOGI(WmsLogTag::DMS, "NotifyHallChanged: Keyboard on!");
        events = SuperFoldStatusChangeEvents::KEYBOARD_ON;
        HandleSuperSensorChange(SuperFoldStatusChangeEvents::ANGLE_CHANGE_HALF_FOLDED);
    } else {
        TLOGI(WmsLogTag::DMS, "NotifyHallChanged: Invalid Hall Value!");
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
    if (ScreenSessionManager::GetInstance().GetIsExtendModelocked() ||
        ScreenSessionManager::GetInstance().GetIsFoldStatusLocked() ||
        ScreenSessionManager::GetInstance().GetIsLandscapeLockStatus()) {
        return;
    }
    SuperFoldStateManager::GetInstance().HandleSuperFoldStatusChange(events);
}

void SuperFoldSensorManager::DriveStateMachineToExpand()
{
    TLOGW(WmsLogTag::DMS, "to expand.");
    SuperFoldStateManager::GetInstance().DriveStateMachineToExpand();
}

void SuperFoldSensorManager::SetStateMachineToActived()
{
    if (ScreenSessionManager::GetInstance().GetIsFoldStatusLocked()) {
        TLOGW(WmsLogTag::DMS, "Fold status is still locked.");
        return;
    }
    if (ScreenSessionManager::GetInstance().GetIsExtendModelocked()) {
        TLOGW(WmsLogTag::DMS, "Extend screen is still connected.");
        return;
    }
    if (ScreenSessionManager::GetInstance().GetIsLandscapeLockStatus()) {
        TLOGW(WmsLogTag::DMS, "Landscape status is still locked.");
        return;
    }
    TLOGW(WmsLogTag::DMS, "All locks have been unlocked to start statemachine.");
    NotifyHallChanged(curHall_);
    NotifyFoldAngleChanged(curAngle_);
}

void SuperFoldSensorManager::HandleFoldStatusLockedToExpand()
{
    TLOGI(WmsLogTag::DMS, "Fold status locked to expand and stop statemachine.");
    SuperFoldStateManager::GetInstance().DriveStateMachineToExpand();
}

void SuperFoldSensorManager::HandleFoldStatusUnlocked()
{
    if (ScreenSessionManager::GetInstance().GetIsLandscapeLockStatus()) {
        TLOGI(WmsLogTag::DMS, "Landscape status is still locked.");
        return;
    }
    TLOGI(WmsLogTag::DMS, "All locks have been unlocked to start statemachine.");
    NotifyHallChanged(curHall_);
    NotifyFoldAngleChanged(curAngle_);
}

float SuperFoldSensorManager::GetCurAngle()
{
    return curAngle_;
}

SuperFoldSensorManager::SuperFoldSensorManager() {}

SuperFoldSensorManager::~SuperFoldSensorManager() {}

void SuperFoldSensorManager::SetTaskScheduler(std::shared_ptr<TaskScheduler> scheduler)
{
    TLOGI(WmsLogTag::DMS, "Set task scheduler.");
    taskScheduler_ = scheduler;
}
} // Rosen
} // OHOS