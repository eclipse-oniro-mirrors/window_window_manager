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
    constexpr int32_t MOTION_ACTION_TENT_MODE_OFF = 0;
    constexpr int32_t MOTION_ACTION_TENT_MODE_ON = 1;
#endif
}

#ifdef WM_SUBSCRIBE_MOTION_ENABLE
bool MotionSubscriber::isMotionSensorSubscribed_ = false;
sptr<RotationMotionEventCallback> MotionSubscriber::motionEventCallback_ = nullptr;
bool MotionTentSubscriber::isMotionSensorSubscribed_ = false;
sptr<TentMotionEventCallback> MotionTentSubscriber::motionEventCallback_ = nullptr;
#endif

void ScreenSensorConnector::SubscribeRotationSensor()
{
    TLOGD(WmsLogTag::DMS, "dms: subscribe rotation-related sensor");
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    MotionSubscriber::SubscribeMotionSensor();
    if (MotionSubscriber::isMotionSensorSubscribed_) {
        return;
    }
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
    TLOGI(WmsLogTag::DMS, "dms: Subscribe motion Sensor");
    if (isMotionSensorSubscribed_) {
        TLOGE(WmsLogTag::DMS, "dms: motion sensor's already subscribed");
        return;
    }
    sptr<RotationMotionEventCallback> callback = new (std::nothrow) RotationMotionEventCallback();
    if (callback == nullptr) {
        return;
    }
    int32_t ret = OHOS::Msdp::SubscribeCallback(OHOS::Msdp::MOTION_TYPE_ROTATION, callback);
    if (ret != 0) {
        return;
    }
    motionEventCallback_ = callback;
    isMotionSensorSubscribed_ = true;
}

void MotionSubscriber::UnsubscribeMotionSensor()
{
    if (!isMotionSensorSubscribed_) {
        TLOGI(WmsLogTag::DMS, "dms: Unsubscribe motion sensor");
        return;
    }
    int32_t ret = OHOS::Msdp::UnsubscribeCallback(OHOS::Msdp::MOTION_TYPE_ROTATION, motionEventCallback_);
    if (ret != static_cast<int32_t>(OHOS::Msdp::MotionErrorCode::MOTION_SUCCESS)
        && ret != static_cast<int32_t>(OHOS::Msdp::MotionErrorCode::MOTION_NO_SUBSCRIBE)) {
        return;
    }
    isMotionSensorSubscribed_ = false;
}

void RotationMotionEventCallback::OnMotionChanged(const MotionEvent& motionData)
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
    TLOGI(WmsLogTag::DMS, "dms: Subscribe tent motion Sensor");
    if (isMotionSensorSubscribed_) {
        TLOGE(WmsLogTag::DMS, "dms: tent motion sensor's already subscribed");
        return;
    }
    sptr<TentMotionEventCallback> callback = new (std::nothrow) TentMotionEventCallback();
    if (callback == nullptr) {
        TLOGE(WmsLogTag::DMS, "dms: malloc tent motion callback failed");
        return;
    }
    int32_t ret = OHOS::Msdp::SubscribeCallback(OHOS::Msdp::MOTION_TYPE_TENT, callback);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "dms: SubscribeCallback type:%{public}d failed", OHOS::Msdp::MOTION_TYPE_TENT);
        return;
    }
    motionEventCallback_ = callback;
    isMotionSensorSubscribed_ = true;
}

void MotionTentSubscriber::UnsubscribeMotionSensor()
{
    if (!isMotionSensorSubscribed_) {
        TLOGI(WmsLogTag::DMS, "dms: Unsubscribe tent motion sensor");
        return;
    }
    int32_t ret = OHOS::Msdp::UnsubscribeCallback(OHOS::Msdp::MOTION_TYPE_TENT, motionEventCallback_);
    if (ret != static_cast<int32_t>(OHOS::Msdp::MotionErrorCode::MOTION_SUCCESS)
        && ret != static_cast<int32_t>(OHOS::Msdp::MotionErrorCode::MOTION_NO_SUBSCRIBE)) {
        return;
    }
    isMotionSensorSubscribed_ = false;
}

void TentMotionEventCallback::OnMotionChanged(const MotionEvent& motionData)
{
    if (motionData.status == MOTION_ACTION_TENT_MODE_ON) {
        ScreenTentProperty::HandleSensorEventInput(true);
    } else if (motionData.status == MOTION_ACTION_TENT_MODE_OFF) {
        ScreenTentProperty::HandleSensorEventInput(false);
    } else {
        TLOGI(WmsLogTag::DMS, "dms: tent motion:%{public}d invalid", motionData.status);
    }
}
#endif
} // Rosen
} // OHOS