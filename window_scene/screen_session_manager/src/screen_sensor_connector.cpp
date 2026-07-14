/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "screen_sensor_connector.h"
#include "screen_session_manager.h"
#include "dms_global_mutex.h"
#include <chrono>
#include <securec.h>
#include <parameters.h>
#include "motion_manager.h"

namespace OHOS {
namespace Rosen {
namespace {
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    constexpr int32_t MOTION_ACTION_PORTRAIT = 0;
    constexpr int32_t MOTION_ACTION_LEFT_LANDSCAPE = 1;
    constexpr int32_t MOTION_ACTION_PORTRAIT_INVERTED = 2;
    constexpr int32_t MOTION_ACTION_RIGHT_LANDSCAPE = 3;
    const int32_t MOTION_TYPE_ROTATION = 700;
    const int32_t SMART_MOTION_TYPE_ROTATION = 701;
    const int32_t DISABLE_SMART_ROTATION = 0;
    const int32_t ENABLE_SMART_ROTATION = 1;
    const int32_t MOTION_TYPE_TENT = 2800;
    const int32_t TENT_MODE_UNKNOWN = 0;
    const int32_t TENT_MODE_TENT = 1;
    const int32_t TENT_MODE_HOVER = 2;
    const int32_t TENT_MODE_SUPER_DEVICE_HOVER = 3;
    const int32_t TENT_MODE_SUPER_DEVICE_TENT_ONE = 4;
    const int32_t TENT_MODE_SUPER_DEVICE_TENT_TWO = 5;
#endif

#ifdef SENSOR_ENABLE
    constexpr int64_t ORIENTATION_SENSOR_SAMPLING_RATE = 200000000; // 200ms
    constexpr int64_t ORIENTATION_SENSOR_REPORTING_RATE = 0;
    constexpr long ORIENTATION_SENSOR_CALLBACK_TIME_INTERVAL = 1000; // 1000ms
    constexpr int VALID_INCLINATION_ANGLE_THRESHOLD_COEFFICIENT = 3;
    constexpr int DEGREE_0 = 0;
    constexpr int DEGREE_30 = 30;
    constexpr int DEGREE_60 = 60;
    constexpr int DEGREE_120 = 120;
    constexpr int DEGREE_150 = 150;
    constexpr int DEGREE_210 = 210;
    constexpr int DEGREE_240 = 240;
    constexpr int DEGREE_300 = 300;
    constexpr int DEGREE_330 = 330;
#endif
}

#ifdef WM_SUBSCRIBE_MOTION_ENABLE
bool MotionSubscriber::isMotionSensorSubscribed_ = false;
bool MotionTentSubscriber::isMotionSensorSubscribed_ = false;
static void RotationMotionEventCallback(const MotionSensorEvent& motionData);
static void TentMotionEventCallback(const MotionSensorEvent& motionData);
static bool GetMatchTentMode(int32_t motionStatus, TentMode& tentMode);
#endif

#ifdef SENSOR_ENABLE
bool GravitySensorSubscriber::isGravitySensorSubscribed_ = false;
SensorUser GravitySensorSubscriber::user_;
long GravitySensorSubscriber::lastCallbackTime_ = 0;
#endif

void ScreenSensorConnector::SubscribeRotationSensor()
{
    TLOGD(WmsLogTag::DMS, "subscribe rotation-related sensor");
#ifdef SENSOR_ENABLE
    GravitySensorSubscriber::SubscribeGravitySensor();
#endif
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    // MotionSubscriber::SubscribeMotionSensor();
    MotionManager::GetInstance().Init();
    MotionManager::GetInstance().OnScreenOn();
#endif
}

void ScreenSensorConnector::UnsubscribeRotationSensor()
{
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    // MotionSubscriber::UnsubscribeMotionSensor();
    MotionManager::GetInstance().OnScreenOff();
#endif
}

void ScreenSensorConnector::SubscribeTentSensor()
{
    TLOGD(WmsLogTag::DMS, "start");
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    MotionTentSubscriber::SubscribeMotionSensor();
#endif
}

void ScreenSensorConnector::UnsubscribeTentSensor()
{
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    MotionTentSubscriber::UnsubscribeMotionSensor();
#endif
}

// Gravity Sensor
#ifdef SENSOR_ENABLE
void GravitySensorSubscriber::SubscribeGravitySensor()
{
    TLOGI(WmsLogTag::DMS, "dms: Subscribe gravity Sensor");
    if (isGravitySensorSubscribed_) {
        TLOGE(WmsLogTag::DMS, "dms: gravity sensor's already subscribed");
        return;
    }
    if (strcpy_s(user_.name, sizeof(user_.name), "ScreenRotationController") != EOK) {
        TLOGE(WmsLogTag::DMS, "dms strcpy_s error");
        return;
    }
    user_.userData = nullptr;
    user_.callback = &HandleGravitySensorEventCallback;
    if (SubscribeSensor(SENSOR_TYPE_ID_ACCELEROMETER, &user_) != 0) {
        TLOGE(WmsLogTag::DMS, "dms: Subscribe gravity sensor failed");
        return;
    }
    SetBatch(SENSOR_TYPE_ID_ACCELEROMETER, &user_, ORIENTATION_SENSOR_SAMPLING_RATE, ORIENTATION_SENSOR_REPORTING_RATE);
    SetMode(SENSOR_TYPE_ID_ACCELEROMETER, &user_, SENSOR_ON_CHANGE);
    if (ActivateSensor(SENSOR_TYPE_ID_ACCELEROMETER, &user_) != 0) {
        TLOGE(WmsLogTag::DMS, "dms: Activate gravity sensor failed");
        return;
    }
    isGravitySensorSubscribed_ = true;
}

void GravitySensorSubscriber::UnsubscribeGravitySensor()
{
    TLOGI(WmsLogTag::DMS, "dms: Unsubscribe gravity Sensor");
    if (!isGravitySensorSubscribed_) {
        TLOGE(WmsLogTag::DMS, "dms: Orientation Sensor is not subscribed");
        return;
    }
    if (DeactivateSensor(SENSOR_TYPE_ID_ACCELEROMETER, &user_) != 0) {
        TLOGE(WmsLogTag::DMS, "dms: Deactivate gravity sensor failed");
        return;
    }
    if (UnsubscribeSensor(SENSOR_TYPE_ID_ACCELEROMETER, &user_) != 0) {
        TLOGE(WmsLogTag::DMS, "dms: Unsubscribe gravity sensor failed");
        return;
    }
    isGravitySensorSubscribed_ = false;
}

void GravitySensorSubscriber::HandleGravitySensorEventCallback(SensorEvent *event)
{
    if (!CheckCallbackTimeInterval() || event == nullptr) {
        return;
    }
    if (event->sensorTypeId != SENSOR_TYPE_ID_ACCELEROMETER) {
        TLOGE(WmsLogTag::DMS, "dms: Orientation Sensor Callback is not SENSOR_TYPE_ID_ACCELEROMETER");
        return;
    }

    GravityData* gravityData = reinterpret_cast<GravityData*>(event->data);
    int sensorDegree = CalcRotationDegree(gravityData);
    DeviceRotation motionRotation = DeviceRotation::INVALID;
    // 当前传感器返回数据有问题，临时修正
    if (sensorDegree >= DEGREE_0 && (sensorDegree <= DEGREE_30 || sensorDegree >= DEGREE_330)) {
        motionRotation = DeviceRotation::ROTATION_LANDSCAPE_INVERTED;
    } else if (sensorDegree >= DEGREE_60 && sensorDegree <= DEGREE_120) {
        motionRotation = DeviceRotation::ROTATION_PORTRAIT_INVERTED;
    } else if (sensorDegree >= DEGREE_150 && sensorDegree <= DEGREE_210) {
        motionRotation = DeviceRotation::ROTATION_LANDSCAPE;
    } else if (sensorDegree >= DEGREE_240 && sensorDegree <= DEGREE_300) {
        motionRotation = DeviceRotation::ROTATION_PORTRAIT;
    }
    ScreenRotationProperty::HandleSensorEventInput(motionRotation);
}

int GravitySensorSubscriber::CalcRotationDegree(GravityData* gravityData)
{
    if (gravityData == nullptr) {
        return -1;
    }
    float x = gravityData->x;
    float y = gravityData->y;
    float z = gravityData->z;

    int degree = -1;
    if ((x * x + y * y) * VALID_INCLINATION_ANGLE_THRESHOLD_COEFFICIENT < z * z) {
        return degree;
    }
    // arccotx = pi / 2 - arctanx, 90 is used to calculate acot(in degree); degree = rad / pi * 180
    degree = 90 - static_cast<int>(round(atan2(y, -x) / M_PI * 180));
    // Normalize the degree to the range of 0~360
    return degree >= 0 ? degree % 360 : degree % 360 + 360;
}

bool GravitySensorSubscriber::CheckCallbackTimeInterval()
{
    std::chrono::milliseconds ms = std::chrono::time_point_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now()).time_since_epoch();
    long currentTimeInMillitm = ms.count();
    if (currentTimeInMillitm - lastCallbackTime_ < ORIENTATION_SENSOR_CALLBACK_TIME_INTERVAL) {
        return false;
    }
    lastCallbackTime_ = currentTimeInMillitm;
    return true;
}
#endif

// Motion
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
void MotionSubscriber::SubscribeMotionSensor()
{
    return;
    TLOGI(WmsLogTag::DMS, "start");
    if (isMotionSensorSubscribed_) {
        TLOGE(WmsLogTag::DMS, "motion sensor's already subscribed");
        return;
    }

    int32_t sensorType = MOTION_TYPE_ROTATION;
    int32_t smartRotationEnabled = system::GetIntParameter<int32_t>("const.window.device.default_rotation_sensor",
        DISABLE_SMART_ROTATION);
    if (smartRotationEnabled == ENABLE_SMART_ROTATION) {
        TLOGNFI(WmsLogTag::DMS, "start subscribe smart rotation");
        sensorType = SMART_MOTION_TYPE_ROTATION;
    }
    if (!SubscribeCallback(sensorType, RotationMotionEventCallback)) {
        TLOGE(WmsLogTag::DMS, "dms: motion sensor subscribe failed");
        return;
    }
    isMotionSensorSubscribed_ = true;
}

void MotionSubscriber::UnsubscribeMotionSensor()
{
    return;
    if (!isMotionSensorSubscribed_) {
        TLOGI(WmsLogTag::DMS, "dms: Unsubscribe motion sensor");
        return;
    }

    int32_t sensorType = MOTION_TYPE_ROTATION;
    int32_t smartRotationEnabled = system::GetIntParameter<int32_t>("const.window.device.default_rotation_sensor",
        DISABLE_SMART_ROTATION);
    if (smartRotationEnabled == ENABLE_SMART_ROTATION) {
        TLOGNFI(WmsLogTag::DMS, "start unsubscribe smart rotation");
        sensorType = SMART_MOTION_TYPE_ROTATION;
    }
    if (!UnsubscribeCallback(sensorType, RotationMotionEventCallback)) {
        TLOGE(WmsLogTag::DMS, "dms: motion sensor unsubscribe failed");
        return;
    }
    isMotionSensorSubscribed_ = false;
}

void RotationMotionEventCallback(const MotionSensorEvent& motionData)
{
    DeviceRotation motionRotation = DeviceRotation::INVALID;
    switch (motionData.status) {
        case MOTION_ACTION_PORTRAIT: {
            motionRotation = DeviceRotation::ROTATION_PORTRAIT;
            break;
        }
        case MOTION_ACTION_LEFT_LANDSCAPE: {
            motionRotation = DeviceRotation::ROTATION_LANDSCAPE_INVERTED;
            break;
        }
        case MOTION_ACTION_PORTRAIT_INVERTED: {
            motionRotation = DeviceRotation::ROTATION_PORTRAIT_INVERTED;
            break;
        }
        case MOTION_ACTION_RIGHT_LANDSCAPE: {
            motionRotation = DeviceRotation::ROTATION_LANDSCAPE;
            break;
        }
        default: {
            break;
        }
    }
    DmUtils::HoldLock callbackLock;
    ScreenRotationProperty::HandleSensorEventInput(motionRotation);
}

void MotionTentSubscriber::SubscribeMotionSensor()
{
    TLOGI(WmsLogTag::DMS, "Subscribe tent motion Sensor");
    if (isMotionSensorSubscribed_) {
        TLOGE(WmsLogTag::DMS, "tent motion sensor's already subscribed");
        return;
    }
    
    if (!SubscribeCallback(MOTION_TYPE_TENT, TentMotionEventCallback)) {
        TLOGE(WmsLogTag::DMS, "dms: motion sensor subscribe failed");
        return;
    }
    isMotionSensorSubscribed_ = true;
}

void MotionTentSubscriber::UnsubscribeMotionSensor()
{
    if (!isMotionSensorSubscribed_) {
        TLOGI(WmsLogTag::DMS, "dms: Unsubscribe tent motion sensor");
        return;
    }
    
    if (!UnsubscribeCallback(MOTION_TYPE_TENT, TentMotionEventCallback)) {
        TLOGE(WmsLogTag::DMS, "dms: motion sensor unsubscribe failed");
        return;
    }
    isMotionSensorSubscribed_ = false;
}

void TentMotionEventCallback(const MotionSensorEvent& motionData)
{
    TLOGI(WmsLogTag::DMS, "tent mode status %{public}d", motionData.status);
    if (motionData.dataLen < 1 || motionData.data == nullptr) {
        TLOGE(WmsLogTag::DMS, "tent mode datalen %{public}d", motionData.dataLen);
        return;
    }
    uint32_t tentData = static_cast<uint32_t>(motionData.data[0]);
    int realHall = static_cast<int>(tentData & 0xFF);
    if (realHall != 0 && realHall != 1) {
        TLOGW(WmsLogTag::DMS, "tent mode invalid hall : %{public}d", realHall);
        realHall = -1;
    }

    TentMode motionStatus = TentMode::TENT_MODE_MAX;
    if (GetMatchTentMode(motionData.status, motionStatus)) {
        TLOGI(WmsLogTag::DMS, "tent motion:%{public}d", motionStatus);
        ScreenTentProperty::HandleSensorEventInput(static_cast<int>(motionStatus), realHall);
        ScreenSessionManager::GetInstance().NotifyTentModeChange(motionStatus);
    } else {
        TLOGE(WmsLogTag::DMS, "tent motion:%{public}d invalid", motionData.status);
    }
}

bool GetMatchTentMode(int32_t motionStatus, TentMode& tentMode)
{
    switch (motionStatus) {
        case TENT_MODE_UNKNOWN :
            tentMode = TentMode::UNKNOWN;
            break;
        case TENT_MODE_TENT :
        case TENT_MODE_SUPER_DEVICE_TENT_ONE :
        case TENT_MODE_SUPER_DEVICE_TENT_TWO :
            tentMode = TentMode::TENT_MODE;
            break;
        case TENT_MODE_HOVER :
        case TENT_MODE_SUPER_DEVICE_HOVER :
            tentMode = TentMode::HOVER;
            break;
        default:
            tentMode = TentMode::TENT_MODE_MAX;
            return false;
    }
    return true;
}
#endif
} // Rosen
} // OHOS