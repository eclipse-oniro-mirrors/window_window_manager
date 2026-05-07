/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "motion_manager.h"
#include "screen_session_manager.h"
#include <parameters.h>
#include <securec.h>

#ifdef WM_SUBSCRIBE_MOTION_ENABLE
#include "screen_sensor_plugin.h"
#endif

namespace OHOS {
namespace Rosen {

#ifdef WM_SUBSCRIBE_MOTION_ENABLE
std::map<MotionType, bool> MotionSubscriberWrapper::isMotionSubscribedMap_ = {
    {MotionType::DEVICE_MOTION_TYPE, false},
    {MotionType::SMART_MOTION_TYPE, false}
};
#endif

MotionManager& MotionManager::GetInstance()
{
    static MotionManager instance;
    return instance;
}

MotionManager::MotionManager()
{
    TLOGI(WmsLogTag::WMS_ROTATION, "MotionManager created");
}

MotionManager::~MotionManager()
{
    TLOGI(WmsLogTag::WMS_ROTATION, "MotionManager destroyed");
    UnsubscribeAllMotionSensors();
}

void MotionManager::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (isInitialized_) {
        TLOGI(WmsLogTag::WMS_ROTATION, "MotionManager already initialized");
        return;
    }
    
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    int32_t smartRotationEnabled = OHOS::system::GetIntParameter<int32_t>("const.window.device.default_rotation_sensor",
        DISABLE_SMART_ROTATION);
    isDefaultSmartMotionEnabled_ = (smartRotationEnabled == ENABLE_SMART_ROTATION);
    TLOGI(WmsLogTag::WMS_ROTATION, "default_rotation_sensor: %{public}d, smartMotionEnabled: %{public}d",
        smartRotationEnabled, isDefaultSmartMotionEnabled_);
    needSubscribedMotionTypes_[MotionType::DEVICE_MOTION_TYPE] = true;
    if (isDefaultSmartMotionEnabled_) {
        needSubscribedMotionTypes_[MotionType::SMART_MOTION_TYPE] = true;
    }
    if (isScreenOn_) {
        SubscribeDefaultMotionSensors();
    }
#endif
    
    isInitialized_ = true;
    TLOGI(WmsLogTag::WMS_ROTATION, "MotionManager initialized");
}

void MotionManager::SubscribeDefaultMotionSensors()
{
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    SubscribeMotionSensorInternal(MotionType::DEVICE_MOTION_TYPE);
    
    if (isDefaultSmartMotionEnabled_) {
        SubscribeMotionSensorInternal(MotionType::SMART_MOTION_TYPE);
        TLOGI(WmsLogTag::WMS_ROTATION, "Smart motion sensor enabled by default");
    }
#endif
}

void MotionManager::SetMotionEventListener(IMotionEventListener* listener)
{
    std::lock_guard<std::mutex> lock(mutex_);
    motionEventListener_ = listener;
    TLOGI(WmsLogTag::WMS_ROTATION, "Motion event listener set");
}

void MotionManager::SubscribeMotionSensorInternal(MotionType motionType)
{
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    if (!isScreenOn_) {
        TLOGI(WmsLogTag::WMS_ROTATION, "Screen is off, skip subscribe motion type: %{public}d", motionType);
        return;
    }
    
    if (subscribedMotionTypes_[motionType]) {
        TLOGI(WmsLogTag::WMS_ROTATION, "Motion type %{public}d already subscribed", motionType);
        return;
    }

    if (!needSubscribedMotionTypes_[motionType]) {
        TLOGI(WmsLogTag::WMS_ROTATION, "Motion type %{public}d no need subscribed", motionType);
        return;
    }
    
    OnMotionChangedPtr callback = nullptr;
    if (motionType == MotionType::DEVICE_MOTION_TYPE) {
        callback = RotationMotionEventCallback;
    } else if (motionType == MotionType::SMART_MOTION_TYPE) {
        callback = SmartRotationMotionEventCallback;
    } else {
        TLOGE(WmsLogTag::WMS_ROTATION, "Unknown motion type: %{public}d", motionType);
        return;
    }
    
    if (!SubscribeCallback(static_cast<int32_t>(motionType), callback)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to subscribe motion type: %{public}d", motionType);
        return;
    }
    
    subscribedMotionTypes_[motionType] = true;
    TLOGI(WmsLogTag::WMS_ROTATION, "Successfully subscribed motion type: %{public}d", motionType);
#else
    TLOGW(WmsLogTag::WMS_ROTATION, "WM_SUBSCRIBE_MOTION_ENABLE not defined");
#endif
}

void MotionManager::UnsubscribeMotionSensorInternal(MotionType motionType)
{
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    if (!subscribedMotionTypes_[motionType]) {
        TLOGI(WmsLogTag::WMS_ROTATION, "Motion type %{public}d not subscribed", motionType);
        return;
    }
    
    OnMotionChangedPtr callback = nullptr;
    if (motionType == MotionType::DEVICE_MOTION_TYPE) {
        callback = RotationMotionEventCallback;
    } else if (motionType == MotionType::SMART_MOTION_TYPE) {
        callback = SmartRotationMotionEventCallback;
    } else {
        TLOGE(WmsLogTag::WMS_ROTATION, "Unknown motion type: %{public}d", motionType);
        return;
    }
    
    if (!UnsubscribeCallback(static_cast<int32_t>(motionType), callback)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to unsubscribe motion type: %{public}d", motionType);
        return;
    }
    
    subscribedMotionTypes_[motionType] = false;
    TLOGI(WmsLogTag::WMS_ROTATION, "Successfully unsubscribed motion type: %{public}d", motionType);
#else
    TLOGW(WmsLogTag::WMS_ROTATION, "WM_SUBSCRIBE_MOTION_ENABLE not defined");
#endif
}

void MotionManager::SubscribeMotionSensor(MotionType motionType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    needSubscribedMotionTypes_[motionType] = true;
    SubscribeMotionSensorInternal(motionType);
}

void MotionManager::UnsubscribeMotionSensor(MotionType motionType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    needSubscribedMotionTypes_[motionType] = false;
    UnsubscribeMotionSensorInternal(motionType);
}

void MotionManager::OnScreenOn()
{
    std::lock_guard<std::mutex> lock(mutex_);
    isScreenOn_ = true;
    TLOGI(WmsLogTag::WMS_ROTATION, "Screen on, subscribing default motion sensors");
    
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    SubscribeDefaultMotionSensors();
#endif
}

void MotionManager::OnScreenOff()
{
    std::lock_guard<std::mutex> lock(mutex_);
    isScreenOn_ = false;
    TLOGI(WmsLogTag::WMS_ROTATION, "Screen off, unsubscribing all motion sensors");
    
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    UnsubscribeAllMotionSensors();
#endif
}

void MotionManager::UnsubscribeAllMotionSensors()
{
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    for (auto& pair : subscribedMotionTypes_) {
        if (pair.second) {
            UnsubscribeMotionSensorInternal(pair.first);
        }
    }
#endif
}

DeviceRotation MotionManager::ConvertMotionActionToDeviceRotation(int32_t motionAction)
{
    switch (motionAction) {
        case MotionAction::MOTION_ACTION_PORTRAIT:
            return DeviceRotation::ROTATION_PORTRAIT;
        case MotionAction::MOTION_ACTION_LEFT_LANDSCAPE:
            return DeviceRotation::ROTATION_LANDSCAPE_INVERTED;
        case MotionAction::MOTION_ACTION_PORTRAIT_INVERTED:
            return DeviceRotation::ROTATION_PORTRAIT_INVERTED;
        case MotionAction::MOTION_ACTION_RIGHT_LANDSCAPE:
            return DeviceRotation::ROTATION_LANDSCAPE;
        default:
            return DeviceRotation::INVALID;
    }
}

float MotionManager::ConvertDeviceMotionToFloat(DeviceRotation deviceRotation)
{
    switch (deviceRotation) {
        case DeviceRotation::ROTATION_PORTRAIT:
            return 0.0f;
        case DeviceRotation::ROTATION_LANDSCAPE:
            return 90.0f;
        case DeviceRotation::ROTATION_PORTRAIT_INVERTED:
            return 180.0f;
        case DeviceRotation::ROTATION_LANDSCAPE_INVERTED:
            return 270.0f;
        default:
            return -1.0f;
    }
}

void MotionManager::RotationMotionEventCallback(const MotionSensorEvent& motionData)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "Rotation motion callback, status: %{public}d", motionData.status);
    
    DeviceRotation motionRotation = ConvertMotionActionToDeviceRotation(motionData.status);
    float rotation = ScreenRotationProperty::ConvertDeviceToFloat(motionRotation);
    
    MotionManager::GetInstance().HandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, rotation);
}

void MotionManager::SmartRotationMotionEventCallback(const MotionSensorEvent& motionData)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "Smart rotation motion callback, status: %{public}d", motionData.status);
    
    DeviceRotation motionRotation = ConvertMotionActionToDeviceRotation(motionData.status);
    float rotation = ScreenRotationProperty::ConvertDeviceToFloat(motionRotation);
    
    MotionManager::GetInstance().HandleMotionEvent(MotionType::SMART_MOTION_TYPE, rotation);
}

void MotionManager::HandleMotionEvent(MotionType motionType, float rotation)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (motionType == MotionType::DEVICE_MOTION_TYPE) {
        lastMotionRotation_ = rotation;
    } else if (motionType == MotionType::SMART_MOTION_TYPE) {
        lastSmartMotionRotation_ = rotation;
    }
    
    NotifyMotionRotationChangedInternal();
}

float MotionManager::CalculateAverageRotation(float rotation1, float rotation2)
{
    if (rotation1 < 0.0f || rotation2 < 0.0f) {
        TLOGW(WmsLogTag::WMS_ROTATION, "Invalid rotation values, r1: %{public}f, r2: %{public}f", rotation1, rotation2);
        return -1.0f;
    }
    
    float avg = (rotation1 + rotation2) / 2.0f;
    TLOGI(WmsLogTag::WMS_ROTATION, "Calculate average rotation: (%{public}f + %{public}f) / 2 = %{public}f",
        rotation1, rotation2, avg);
    return avg;
}

void MotionManager::NotifyMotionRotationChangedInternal()
{
    float finalRotation = -1.0f;
    
    bool isMotionTypeSubscribed = subscribedMotionTypes_[MotionType::DEVICE_MOTION_TYPE];
    bool isSmartMotionTypeSubscribed = subscribedMotionTypes_[MotionType::SMART_MOTION_TYPE];
    
    if (isMotionTypeSubscribed && isSmartMotionTypeSubscribed) {
        float avgRotation = CalculateAverageRotation(lastMotionRotation_, lastSmartMotionRotation_);
        if (avgRotation >= 0.0f) {
            finalRotation = avgRotation;
            TLOGI(WmsLogTag::WMS_ROTATION, "Both motion types subscribed, using average: %{public}f", finalRotation);
        }
    } else if (isMotionTypeSubscribed) {
        finalRotation = lastMotionRotation_;
        TLOGI(WmsLogTag::WMS_ROTATION, "Only MOTION_TYPE_ROTATION subscribed, using: %{public}f", finalRotation);
    } else if (isSmartMotionTypeSubscribed) {
        finalRotation = lastSmartMotionRotation_;
        TLOGI(WmsLogTag::WMS_ROTATION, "Only SMART_MOTION_TYPE_ROTATION subscribed, using: %{public}f", finalRotation);
    } else {
        TLOGW(WmsLogTag::WMS_ROTATION, "No motion type subscribed");
        return;
    }
    
    if (motionEventListener_ && finalRotation >= 0.0f) {
        motionEventListener_->OnMotionRotationChanged(finalRotation);
    } else {
        TLOGW(WmsLogTag::WMS_ROTATION, "Motion event listener is null or invalid rotation");
    }
}

float MotionManager::GetLastMotionRotation() const
{
    return lastMotionRotation_;
}

float MotionManager::GetLastSmartMotionRotation() const
{
    return lastSmartMotionRotation_;
}

bool MotionManager::IsMotionSensorSubscribed(MotionType motionType) const
{
    auto it = subscribedMotionTypes_.find(motionType);
    if (it != subscribedMotionTypes_.end()) {
        return it->second;
    }
    return false;
}

bool MotionManager::IsScreenOn() const
{
    return isScreenOn_;
}

bool MotionManager::IsInitialized() const
{
    return isInitialized_;
}

bool MotionManager::IsDefaultSmartMotionEnabled() const
{
    return isDefaultSmartMotionEnabled_;
}

void MotionManager::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    UnsubscribeAllMotionSensors();
    lastMotionRotation_ = -1.0f;
    lastSmartMotionRotation_ = -1.0f;
    motionEventListener_ = nullptr;
    isInitialized_ = false;
    isScreenOn_ = true;
    TLOGI(WmsLogTag::WMS_ROTATION, "MotionManager reset");
}

void MotionManager::SetScreenOnState(bool isScreenOn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isScreenOn_ = isScreenOn;
    TLOGI(WmsLogTag::WMS_ROTATION, "Set screen on state: %{public}d", isScreenOn);
}

void MotionManager::SetDefaultSmartMotionEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isDefaultSmartMotionEnabled_ = enabled;
    TLOGI(WmsLogTag::WMS_ROTATION, "Set default smart motion enabled: %{public}d", enabled);
}

void MotionManager::TestHandleMotionEvent(MotionType motionType, float rotation)
{
    HandleMotionEvent(motionType, rotation);
}

#ifdef WM_SUBSCRIBE_MOTION_ENABLE
bool MotionSubscriberWrapper::SubscribeMotionSensor(MotionType motionType)
{
    MotionManager::GetInstance().SubscribeMotionSensor(motionType);
    return true;
}

bool MotionSubscriberWrapper::UnsubscribeMotionSensor(MotionType motionType)
{
    MotionManager::GetInstance().UnsubscribeMotionSensor(motionType);
    return true;
}
#endif

} // namespace Rosen
} // namespace OHOS