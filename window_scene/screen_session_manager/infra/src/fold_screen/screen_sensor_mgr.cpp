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

#include "screen_sensor_mgr.h"
#include <mutex>
#include <parameters.h>
#include "window_manager_hilog.h"
#include "dms_global_mutex.h"
#include "sensor_fold_state_mgr.h"
#include "fold_screen_controller/fold_screen_controller_config.h"
#include "fold_screen_controller/secondary_fold_sensor_manager.h"
#include "screen_session_manager.h"
#include "product_config.h"
#include "fold_screen_state_internel.h"

namespace OHOS::Rosen::DMS {
namespace {
constexpr float DUAL_INVALID_ANGLE_VALUE = -1.0F;
constexpr int32_t SENSOR_SUCCESS = 0;
constexpr int32_t SENSOR_FAILURE = 1;
constexpr int32_t POSTURE_INTERVAL = 10000000;
constexpr uint16_t FIRST_DATA = 0;
constexpr uint16_t SECOND_DATA = 1;
constexpr float MINI_NOTIFY_FOLD_ANGLE = 0.5F;
constexpr uint16_t SENSOR_EVENT_FIRST_DATA = 0;
constexpr float ANGLE_MAX_VAL = 180.0F;
constexpr float ACCURACY_ERROR_FOR_ALTA = 0.0001F;
}  // namespace

ScreenSensorMgr& ScreenSensorMgr::GetInstance()
{
    static std::mutex singletonMutex_;
    static ScreenSensorMgr* instance_ = nullptr;
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(singletonMutex_);
        if (instance_ == nullptr) {
            // 这里通过设备类型创建对象
            instance_ = new ScreenSensorMgr();
        }
    }
    return *instance_;
}

ScreenSensorMgr::ScreenSensorMgr()
{
    angle_ = {-1.0F};
    oldAngle_ = {0.0F};
    hall_ = {USHRT_MAX};
}

void ScreenSensorMgr::RegisterPostureCallback()
{
    if (!ProductConfig::GetInstance().IsSingleDisplaySuperFoldDevice()) {
        if (ProductConfig::GetInstance().IsSecondaryDisplayFoldDevice()) {
            SecondaryFoldSensorManager::GetInstance().RegisterPostureCallback();
        } else {
            FoldScreenSensorManager::GetInstance().RegisterPostureCallback();
        }
        return;
    }

    int ret = SubscribeSensorCallback(SENSOR_TYPE_ID_POSTURE, POSTURE_INTERVAL,
        [](SensorEvent* event) { ScreenSensorMgr::GetInstance().HandlePostureData(event); });
    if (ret == SENSOR_SUCCESS) {
        registerPosture_ = true;
        TLOGI(WmsLogTag::DMS, "register posture callback success.");
    } else {
        registerPosture_ = false;
        TLOGE(WmsLogTag::DMS, "register posture callback failed.");
    }
}

void ScreenSensorMgr::RegisterHallCallback()
{
    if (!ProductConfig::GetInstance().IsSingleDisplaySuperFoldDevice()) {
        if (ProductConfig::GetInstance().IsSecondaryDisplayFoldDevice()) {
            SecondaryFoldSensorManager::GetInstance().RegisterHallCallback();
        } else {
            FoldScreenSensorManager::GetInstance().RegisterHallCallback();
        }
        return;
    }
    int ret = SubscribeSensorCallback(SENSOR_TYPE_ID_HALL_EXT, POSTURE_INTERVAL,
        [](SensorEvent* event) { ScreenSensorMgr::GetInstance().HandleHallData(event); });
    if (ret == SENSOR_SUCCESS) {
        registerHall_ = true;
        TLOGI(WmsLogTag::DMS, "register hall callback success.");
    } else {
        registerHall_ = false;
        TLOGE(WmsLogTag::DMS, "register hall callback failed.");
    }
}

void ScreenSensorMgr::UnRegisterHallCallback()
{
    if (!ProductConfig::GetInstance().IsSingleDisplaySuperFoldDevice()) {
        if (ProductConfig::GetInstance().IsSecondaryDisplayFoldDevice()) {
            SecondaryFoldSensorManager::GetInstance().UnRegisterHallCallback();
        } else {
            FoldScreenSensorManager::GetInstance().UnRegisterHallCallback();
        }
        return;
    }

    int ret = UnSubscribeSensorCallback(SENSOR_TYPE_ID_HALL_EXT);
    if (ret == SENSOR_SUCCESS) {
        registerHall_ = false;
        TLOGI(WmsLogTag::DMS, "success.");
    } else {
        TLOGE(WmsLogTag::DMS, "unregister hall sensor failed with ret: %{public}d", ret);
    }
}

void ScreenSensorMgr::UnRegisterPostureCallback()
{
    if (!ProductConfig::GetInstance().IsSingleDisplaySuperFoldDevice()) {
        if (ProductConfig::GetInstance().IsSecondaryDisplayFoldDevice()) {
            SecondaryFoldSensorManager::GetInstance().UnRegisterPostureCallback();
        } else {
            FoldScreenSensorManager::GetInstance().UnRegisterPostureCallback();
        }
        return;
    }

    int ret = UnSubscribeSensorCallback(SENSOR_TYPE_ID_POSTURE);
    if (ret == SENSOR_SUCCESS) {
        registerPosture_ = false;
        TLOGI(WmsLogTag::DMS, "success.");
    } else {
        TLOGE(WmsLogTag::DMS, "UnRegisterPostureCallback failed with ret: %{public}d", ret);
    }
}

static void GlobalSensorCallback(SensorEvent* event)
{
    DmUtils::HoldLock callbackLock;
    ScreenSensorMgr::GetInstance().HandleSensorData(event);
}

int32_t ScreenSensorMgr::SubscribeSensorCallback(
    int32_t sensorTypeId, int64_t interval, const RecordSensorCallback taskCallback)
{
    sensorCallbacks_[sensorTypeId] = {taskCallback, interval};
    auto& user = users_[sensorTypeId];
    user.callback = GlobalSensorCallback;
    int32_t subscribeRet = SubscribeSensor(sensorTypeId, &user);
    int32_t setBatchRet = SetBatch(sensorTypeId, &user, interval, interval);
    int32_t activateRet = ActivateSensor(sensorTypeId, &user);
    TLOGI(WmsLogTag::DMS, "subscribeRet: %{public}d, setBatchRet: %{public}d, activateRet: %{public}d", subscribeRet,
        setBatchRet, activateRet);
    if (subscribeRet != SENSOR_SUCCESS || setBatchRet != SENSOR_SUCCESS || activateRet != SENSOR_SUCCESS) {
        TLOGE(WmsLogTag::DMS, "failed.");
        return SENSOR_FAILURE;
    }
    TLOGI(WmsLogTag::DMS, "success.");
    return SENSOR_SUCCESS;
}

void ScreenSensorMgr::CleanupCallback(int32_t sensorTypeId)
{
    auto callbackIt = sensorCallbacks_.find(sensorTypeId);
    if (callbackIt != sensorCallbacks_.end()) {
        sensorCallbacks_.erase(callbackIt);
        TLOGI(WmsLogTag::DMS, "Cleaned up callback for sensor type %{public}d", sensorTypeId);
    } else {
        TLOGE(WmsLogTag::DMS, "No callback to clean up for sensor type %{public}d", sensorTypeId);
    }
}

int32_t ScreenSensorMgr::UnSubscribeSensorCallback(int32_t sensorTypeId)
{
    auto userIt = users_.find(sensorTypeId);
    if (userIt == users_.end()) {
        TLOGI(WmsLogTag::DMS, "User data not found for sensor type %{public}d", sensorTypeId);
        return SENSOR_FAILURE;
    }

    auto& user = userIt->second;
    int32_t deactivateRet = DeactivateSensor(sensorTypeId, &user);
    int32_t unsubscribeRet = UnsubscribeSensor(sensorTypeId, &user);
    TLOGI(WmsLogTag::DMS,
        "Unsubscribe sensor type: sensorTypeId=%{public}d, deactivateRet=%{public}d, unsubscribeRet=%{public}d",
        sensorTypeId, deactivateRet, unsubscribeRet);

    if (deactivateRet == SENSOR_SUCCESS && unsubscribeRet == SENSOR_SUCCESS) {
        users_.erase(userIt);
        CleanupCallback(sensorTypeId);
        TLOGI(WmsLogTag::DMS, "success.");
        return SENSOR_SUCCESS;
    }
    TLOGE(WmsLogTag::DMS, "failed.");
    return SENSOR_FAILURE;
}

int32_t ScreenSensorMgr::UpdateSensorInterval(int32_t sensorTypeId, int64_t interval)
{
    auto userIt = users_.find(sensorTypeId);
    if (userIt == users_.end()) {
        TLOGW(WmsLogTag::DMS, "User data not found for sensor type %{public}d", sensorTypeId);
        return SENSOR_FAILURE;
    }
    auto& user = users_[sensorTypeId];
    int32_t setBatchRet = SetBatch(sensorTypeId, &user, interval, interval);
    int32_t activateRet = ActivateSensor(sensorTypeId, &user);
    if (setBatchRet == SENSOR_SUCCESS && activateRet == SENSOR_SUCCESS) {
        return SENSOR_SUCCESS;
    }
    TLOGE(WmsLogTag::DMS, "update interval failed.");
    return SENSOR_FAILURE;
}

bool ScreenSensorMgr::HasSubscribedSensor(int32_t sensorTypeId)
{
    return sensorCallbacks_.find(sensorTypeId) != sensorCallbacks_.end();
}

void ScreenSensorMgr::HandleSensorData(SensorEvent* event)
{
    if (event == nullptr || event->data == nullptr) {
        TLOGE(WmsLogTag::DMS, "Received invalid sensor event or data is null.");
        return;
    }

    if (event->dataLen == 0) {
        TLOGW(WmsLogTag::DMS, "Sensor event data length is zero, skipping processing.");
        return;
    }

    auto it = sensorCallbacks_.find(event->sensorTypeId);
    if (it == sensorCallbacks_.end()) {
        TLOGI(WmsLogTag::DMS, "No callback registered for sensorTypeId: %{public}d", event->sensorTypeId);
        return;
    }
    const auto& entry = it->second;
    entry.taskCallback(event);
}

void ScreenSensorMgr::HandleTentSensorData(int32_t tentType, int32_t hall)
{
    tentSensorInfo_.tentType_ = tentType;
    tentSensorInfo_.hall_ = hall;
    SensorFoldStateMgr::GetInstance().HandleSensorEvent(GetSensorStatus(DmsSensorType::SENSOR_TYPE_TENT));
}

template <typename T>
T* ScreenSensorMgr::GetSensorData(const SensorEvent* const event)
{
    if (event == nullptr) {
        TLOGE(WmsLogTag::DMS, "SensorEvent is nullptr.");
        return nullptr;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].data == nullptr) {
        TLOGE(WmsLogTag::DMS, "SensorEvent[0].data is nullptr.");
        return nullptr;
    }
    if (event[SENSOR_EVENT_FIRST_DATA].dataLen < sizeof(T)) {
        TLOGE(WmsLogTag::DMS, "SensorEvent dataLen less than data size.");
        return nullptr;
    }
    return reinterpret_cast<T*>(event[SENSOR_EVENT_FIRST_DATA].data);
}

void ScreenSensorMgr::HandlePostureData(const SensorEvent* const event)
{
    if (!ParsePostureData(event)) {
        TLOGE(WmsLogTag::DMS, "Parse posture data failed.");
        return;
    }

    if (!checkCurrentPostureAndHall()) {
        return;
    }

    SensorFoldStateMgr::GetInstance().HandleSensorEvent(GetSensorStatus(DmsSensorType::SENSOR_TYPE_POSTURE));
    NotifyFoldAngleChanged();
}

void ScreenSensorMgr::HandleHallData(const SensorEvent* const event)
{
    if (!ParseHallData(event)) {
        TLOGE(WmsLogTag::DMS, "Parse hall data failed.");
        return;
    }

    if (!checkCurrentPostureAndHall()) {
        HandleAbnormalAngle();
        return;
    }

    if (!registerPosture_) {
        for (auto& it : angle_) {
            it = ANGLE_MIN_VAL;
        }
    }

    SensorFoldStateMgr::GetInstance().HandleSensorEvent(GetSensorStatus(DmsSensorType::SENSOR_TYPE_HALL));
}

bool ScreenSensorMgr::ParsePostureData(const SensorEvent* const event)
{
    PostureData* postureData = GetSensorData<PostureData>(event);
    if (postureData == nullptr) {
        return false;
    }
    angle_[FIRST_DATA] = postureData->angle;
    TLOGD(WmsLogTag::DMS, "angle value in PostureData is: %{public}f.", angle_[FIRST_DATA]);
    return true;
}

bool ScreenSensorMgr::ParseHallData(const SensorEvent* const event)
{
    ExtHallData* extHallData = GetSensorData<ExtHallData>(event);
    if (extHallData == nullptr) {
        return false;
    }
    uint16_t flag = (uint16_t)(*extHallData).flag;
    if (!(flag & (1 << 1))) {
        TLOGI(WmsLogTag::DMS, "NOT Support Extend Hall.");
        return false;
    }
    if (hall_[FIRST_DATA] == (uint16_t)(*extHallData).hall) {
        TLOGI(WmsLogTag::DMS, "Hall don't change, hall = %{public}u", hall_[FIRST_DATA]);
        return false;
    }
    hall_[FIRST_DATA] = (uint16_t)(*extHallData).hall;
    TLOGI(
        WmsLogTag::DMS, "hall value is: %{public}u, angle value is: %{public}f", hall_[FIRST_DATA], angle_[FIRST_DATA]);
    return true;
}

bool ScreenSensorMgr::checkCurrentPostureAndHall()
{
    for (size_t i = 0; i < angle_.size(); ++i) {
        if (std::isless(angle_[i], ANGLE_MIN_VAL) ||
            std::isgreater(angle_[i], ANGLE_MAX_VAL + ACCURACY_ERROR_FOR_ALTA)) {
            TLOGE(WmsLogTag::DMS, "invalid angle, i = %{public}zu, angle = %{public}f", i, angle_[i]);
            return false;
        }
    }
    for (size_t i = 0; i < hall_.size(); ++i) {
        if (hall_[i] != 0 && hall_[i] != 1) {
            TLOGE(WmsLogTag::DMS, "invalid hall, i = %{public}zu, hall = %{public}u", i, hall_[i]);
            return false;
        }
    }
    return true;
}

void ScreenSensorMgr::NotifyFoldAngleChanged()
{
    for (size_t i = 0; i < angle_.size(); ++i) {
        if (fabs(angle_[i] - oldAngle_[i]) >= MINI_NOTIFY_FOLD_ANGLE) {
            oldAngle_ = angle_;
            ScreenSessionManager::GetInstance().NotifyFoldAngleChanged(angle_);
            return;
        }
    }
}

void ScreenSensorMgr::HandleAbnormalAngle()
{
    if (FoldScreenStateInternel::FloatEqualAbs(angle_[FIRST_DATA], DUAL_INVALID_ANGLE_VALUE) &&
        hall_[FIRST_DATA] == 0 && !registerPosture_) {
        angle_[FIRST_DATA] = ANGLE_MIN_VAL;
        TLOGI(WmsLogTag::DMS, "hall value is: %{public}u, let angle value is: %{public}f, continue", hall_[FIRST_DATA],
            angle_[FIRST_DATA]);

        SensorFoldStateMgr::GetInstance().HandleSensorEvent(GetSensorStatus(DmsSensorType::SENSOR_TYPE_HALL));
    }
}

SensorStatus ScreenSensorMgr::GetSensorStatus(DmsSensorType updateSensorType)
{
    SensorStatus sensorStatus = {.reflexionAngle_ = reflexionAngle_,
        .registerPosture_ = registerPosture_,
        .registerHall_ = registerHall_,
        .tentSensorInfo_ = tentSensorInfo_,
        .updateSensorType_ = updateSensorType};
    for (size_t i = 0; i < hall_.size(); ++i) {
        ScreenAxis axis = {.angle_ = angle_[i], .hall_ = hall_[i]};
        sensorStatus.axis_.emplace_back(axis);
    }
    return sensorStatus;
}

bool ScreenSensorMgr::GetSensorRegisterStatus()
{
    return registerHall_ || registerPosture_;
}

void ScreenSensorMgr::RegisterApplicationStateObserver()
{
    if (!ProductConfig::GetInstance().IsSingleDisplaySuperFoldDevice() &&
        !ProductConfig::GetInstance().IsSecondaryDisplayFoldDevice()) {
        FoldScreenSensorManager::GetInstance().RegisterApplicationStateObserver();
    }
}

}  // namespace OHOS::Rosen::DMS