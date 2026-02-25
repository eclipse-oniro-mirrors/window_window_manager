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
#include <chrono>
#include <securec.h>

namespace OHOS {
namespace Rosen {
namespace {
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    constexpr int32_t MOTION_ACTION_PORTRAIT = 0;
    constexpr int32_t MOTION_ACTION_LEFT_LANDSCAPE = 1;
    constexpr int32_t MOTION_ACTION_PORTRAIT_INVERTED = 2;
    constexpr int32_t MOTION_ACTION_RIGHT_LANDSCAPE = 3;
    const int32_t MOTION_TYPE_ROTATION = 700;
    const int32_t MOTION_TYPE_TENT = 2800;
#endif
}

#ifdef WM_SUBSCRIBE_MOTION_ENABLE
bool MotionSubscriber::isMotionSensorSubscribed_ = false;
bool MotionTentSubscriber::isMotionSensorSubscribed_ = false;
static void RotationMotionEventCallback(const MotionSensorEvent& motionData);
static void TentMotionEventCallback(const MotionSensorEvent& motionData);
#endif

void ScreenSensorConnector::SubscribeRotationSensor()
{
    TLOGD(WmsLogTag::DMS, "subscribe rotation-related sensor");
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    MotionSubscriber::SubscribeMotionSensor();
#endif
}

void ScreenSensorConnector::UnsubscribeRotationSensor()
{
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    MotionSubscriber::UnsubscribeMotionSensor();
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

// Motion
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
void MotionSubscriber::SubscribeMotionSensor()
{
    TLOGI(WmsLogTag::DMS, "start");
    if (isMotionSensorSubscribed_) {
        TLOGE(WmsLogTag::DMS, "motion sensor's already subscribed");
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
        TLOGI(WmsLogTag::DMS, "start");
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
        TLOGI(WmsLogTag::DMS, "start");
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

    TentMode motionStatus = static_cast<TentMode>(motionData.status);
    if (motionStatus == TentMode::UNKNOWN ||
        motionStatus == TentMode::TENT_MODE ||
        motionStatus == TentMode::HOVER) {
        ScreenTentProperty::HandleSensorEventInput(motionData.status, realHall);
        ScreenSessionManager::GetInstance().NotifyTentModeChange(motionStatus);
    } else {
        TLOGE(WmsLogTag::DMS, "tent motion:%{public}d invalid", motionData.status);
    }
}
#endif
} // Rosen
} // OHOS
