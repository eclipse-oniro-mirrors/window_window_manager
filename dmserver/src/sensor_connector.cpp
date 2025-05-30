/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "sensor_connector.h"

#include <chrono>
#include <securec.h>

#include "display_manager_service_inner.h"

namespace OHOS {
namespace Rosen {
namespace {

#ifdef SENSOR_ENABLE
    constexpr int64_t ORIENTATION_SENSOR_SAMPLING_RATE = 200000000; // 200ms
    constexpr int64_t ORIENTATION_SENSOR_REPORTING_RATE = 0;
    constexpr long ORIENTATION_SENSOR_CALLBACK_TIME_INTERVAL = 200; // 200ms
    constexpr int VALID_INCLINATION_ANGLE_THRESHOLD_COEFFICIENT = 3;
#endif

#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    constexpr int32_t MOTION_ACTION_PORTRAIT = 0;
    constexpr int32_t MOTION_ACTION_LEFT_LANDSCAPE = 1;
    constexpr int32_t MOTION_ACTION_PORTRAIT_INVERTED = 2;
    constexpr int32_t MOTION_ACTION_RIGHT_LANDSCAPE = 3;
    const int32_t MOTION_TYPE_ROTATION = 700;
#endif
}

#ifdef SENSOR_ENABLE
bool GravitySensorSubscriber::isGravitySensorSubscribed_ = false;
SensorUser GravitySensorSubscriber::user_;
long GravitySensorSubscriber::lastCallbackTime_ = 0;
#endif

#ifdef WM_SUBSCRIBE_MOTION_ENABLE
bool MotionSubscriber::isMotionSensorSubscribed_ = false;
static void RotationMotionEventCallback(const MotionSensorEvent& motionData);
#endif

void SensorConnector::SubscribeRotationSensor()
{
    TLOGD(WmsLogTag::DMS, "dms: subscribe rotation-related sensor");
    ScreenRotationController::Init();
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    MotionSubscriber::SubscribeMotionSensor();
    if (MotionSubscriber::isMotionSensorSubscribed_) {
        return;
    }
#endif

#ifdef SENSOR_ENABLE
    GravitySensorSubscriber::SubscribeGravitySensor();
#endif
}

void SensorConnector::UnsubscribeRotationSensor()
{
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    MotionSubscriber::UnsubscribeMotionSensor();
#endif

#ifdef SENSOR_ENABLE
    GravitySensorSubscriber::UnsubscribeGravitySensor();
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
    if (SubscribeSensor(SENSOR_TYPE_ID_GRAVITY, &user_) != 0) {
        TLOGE(WmsLogTag::DMS, "dms: Subscribe gravity sensor failed");
        return;
    }
    SetBatch(SENSOR_TYPE_ID_GRAVITY, &user_, ORIENTATION_SENSOR_SAMPLING_RATE, ORIENTATION_SENSOR_REPORTING_RATE);
    SetMode(SENSOR_TYPE_ID_GRAVITY, &user_, SENSOR_ON_CHANGE);
    if (ActivateSensor(SENSOR_TYPE_ID_GRAVITY, &user_) != 0) {
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
    if (DeactivateSensor(SENSOR_TYPE_ID_GRAVITY, &user_) != 0) {
        TLOGE(WmsLogTag::DMS, "dms: Deactivate gravity sensor failed");
        return;
    }
    if (UnsubscribeSensor(SENSOR_TYPE_ID_GRAVITY, &user_) != 0) {
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
    if (event->sensorTypeId != SENSOR_TYPE_ID_GRAVITY) {
        TLOGE(WmsLogTag::DMS, "dms: Orientation Sensor Callback is not SENSOR_TYPE_ID_GRAVITY");
        return;
    }
    GravityData* gravityData = reinterpret_cast<GravityData*>(event->data);
    int sensorDegree = CalcRotationDegree(gravityData);
    DeviceRotation sensorRotationConverted = ScreenRotationController::ConvertSensorToDeviceRotation(
        CalcSensorRotation(sensorDegree));
    ScreenRotationController::HandleSensorEventInput(sensorRotationConverted);
}

SensorRotation GravitySensorSubscriber::CalcSensorRotation(int sensorDegree)
{
    // Use ROTATION_0 when degree range is [0, 30]∪[330, 359]
    if (sensorDegree >= 0 && (sensorDegree <= 30 || sensorDegree >= 330)) {
        return SensorRotation::ROTATION_0;
    } else if (sensorDegree >= 60 && sensorDegree <= 120) { // Use ROTATION_90 when degree range is [60, 120]
        return SensorRotation::ROTATION_90;
    } else if (sensorDegree >= 150 && sensorDegree <= 210) { // Use ROTATION_180 when degree range is [150, 210]
        return SensorRotation::ROTATION_180;
    } else if (sensorDegree >= 240 && sensorDegree <= 300) { // Use ROTATION_270 when degree range is [240, 300]
        return SensorRotation::ROTATION_270;
    } else {
        return SensorRotation::INVALID;
    }
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
    TLOGD(WmsLogTag::DMS, "dms: Subscribe motion Sensor");
    if (isMotionSensorSubscribed_) {
        TLOGE(WmsLogTag::DMS, "dms: motion sensor's already subscribed");
        return;
    }
    if (!SubscribeCallback(MOTION_TYPE_ROTATION, RotationMotionEventCallback)) {
        TLOGE(WmsLogTag::DMS, "dms: motion sensor subscribe failed");
        return;
    }
    isMotionSensorSubscribed_ = true;
}

void MotionSubscriber::UnsubscribeMotionSensor()
{
    if (!isMotionSensorSubscribed_) {
        TLOGI(WmsLogTag::DMS, "dms: Unsubscribe motion sensor");
        return;
    }
    if (!UnsubscribeCallback(MOTION_TYPE_ROTATION, RotationMotionEventCallback)) {
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
            motionRotation = ScreenRotationController::IsDefaultDisplayRotationPortrait() ?
                DeviceRotation::ROTATION_LANDSCAPE_INVERTED : DeviceRotation::ROTATION_LANDSCAPE;
            break;
        }
        case MOTION_ACTION_PORTRAIT_INVERTED: {
            motionRotation = DeviceRotation::ROTATION_PORTRAIT_INVERTED;
            break;
        }
        case MOTION_ACTION_RIGHT_LANDSCAPE: {
            motionRotation = ScreenRotationController::IsDefaultDisplayRotationPortrait() ?
                DeviceRotation::ROTATION_LANDSCAPE : DeviceRotation::ROTATION_LANDSCAPE_INVERTED;
            break;
        }
        default: {
            break;
        }
    }
    ScreenRotationController::HandleSensorEventInput(motionRotation);
}
#endif
} // Rosen
} // OHOS