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
#include <parameters.h>
#include <securec.h>
#include "session_sensor_plugin.h"

namespace OHOS {
namespace Rosen {

std::map<MotionType, bool> MotionSubscriberWrapper::isMotionSubscribedMap_ = {
    {MotionType::DEVICE_MOTION_TYPE, false},
    {MotionType::SMART_MOTION_TYPE, false},
    {MotionType::SMART_MOTION_ENHANCE_TYPE, false},
};

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
    SessionUnloadMotionSensor();
}

void MotionManager::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (isInitialized_) {
        TLOGI(WmsLogTag::WMS_ROTATION, "MotionManager already initialized");
        return;
    }
    
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    if (!SessionLoadMotionSensor()) {
        TLOGE(WmsLogTag::WMS_ROTATION, "load motion plugin failed");
    }
#endif
    
    isInitialized_ = true;
    TLOGI(WmsLogTag::WMS_ROTATION, "MotionManager initialized");
}

void MotionManager::SetMotionEventListener(IMotionEventListener* listener)
{
    std::lock_guard<std::mutex> lock(mutex_);
    motionEventListener_ = listener;
    TLOGI(WmsLogTag::WMS_ROTATION, "Motion event listener set");
}

bool MotionManager::SubscribeMotionSensorInternal(MotionType motionType)
{
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    if (subscribedMotionTypes_[motionType]) {
        TLOGI(WmsLogTag::WMS_ROTATION, "Motion type %{public}d already subscribed", motionType);
        return true;
    }

    OnMotionChangedPtr callback = nullptr;
    if (motionType == MotionType::DEVICE_MOTION_TYPE) {
        callback = RotationMotionEventCallback;
    } else if (motionType == MotionType::SMART_MOTION_TYPE || motionType == MotionType::SMART_MOTION_ENHANCE_TYPE) {
        callback = SmartRotationMotionEventCallback;
    } else {
        TLOGE(WmsLogTag::WMS_ROTATION, "Unknown motion type: %{public}d", motionType);
        return false;
    }
    
    if (!SessionSubscribeCallback(static_cast<int32_t>(motionType), callback)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to subscribe motion type: %{public}d", motionType);
        return false;
    }
    
    subscribedMotionTypes_[motionType] = true;
    TLOGI(WmsLogTag::WMS_ROTATION, "Successfully subscribed motion type: %{public}d", motionType);
    return true;
#else
    TLOGW(WmsLogTag::WMS_ROTATION, "WM_SUBSCRIBE_MOTION_ENABLE not defined");
    return false;
#endif
}

bool MotionManager::UnsubscribeMotionSensorInternal(MotionType motionType)
{
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    if (!subscribedMotionTypes_[motionType]) {
        TLOGI(WmsLogTag::WMS_ROTATION, "Motion type %{public}d not subscribed", motionType);
        return true;
    }
    
    OnMotionChangedPtr callback = nullptr;
    if (motionType == MotionType::DEVICE_MOTION_TYPE) {
        callback = RotationMotionEventCallback;
    } else if (motionType == MotionType::SMART_MOTION_TYPE || motionType == MotionType::SMART_MOTION_ENHANCE_TYPE) {
        callback = SmartRotationMotionEventCallback;
    } else {
        TLOGE(WmsLogTag::WMS_ROTATION, "Unknown motion type: %{public}d", motionType);
        return false;
    }
    
    if (!SessionUnsubscribeCallback(static_cast<int32_t>(motionType), callback)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to unsubscribe motion type: %{public}d", motionType);
        return false;
    }
    
    subscribedMotionTypes_[motionType] = false;
    TLOGI(WmsLogTag::WMS_ROTATION, "Successfully unsubscribed motion type: %{public}d", motionType);
    return true;
#else
    TLOGW(WmsLogTag::WMS_ROTATION, "WM_SUBSCRIBE_MOTION_ENABLE not defined");
    return false;
#endif
}

bool MotionManager::SubscribeMotionSensor(MotionType motionType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return SubscribeMotionSensorInternal(motionType);
}

bool MotionManager::UnsubscribeMotionSensor(MotionType motionType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return UnsubscribeMotionSensorInternal(motionType);
}

void MotionManager::UnsubscribeAllMotionSensors()
{
    for (auto& pair : subscribedMotionTypes_) {
        if (pair.second) {
            UnsubscribeMotionSensorInternal(pair.first);
        }
    }
}

DeviceRotation MotionManager::ConvertMotionActionToDeviceRotation(int32_t motionAction)
{
    switch (motionAction) {
        case MotionAction::MOTION_PORTRAIT:
            return DeviceRotation::ROTATION_PORTRAIT;
        case MotionAction::MOTION_LANDSCAPE:
            return DeviceRotation::ROTATION_LANDSCAPE_INVERTED;
        case MotionAction::MOTION_PORTRAIT_INVERTED:
            return DeviceRotation::ROTATION_PORTRAIT_INVERTED;
        case MotionAction::MOTION_LANDSCAPE_INVERTED:
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
    float rotation = ConvertDeviceMotionToFloat(motionRotation);
    
    MotionManager::GetInstance().HandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, rotation);
}

void MotionManager::SmartRotationMotionEventCallback(const MotionSensorEvent& motionData)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "Smart rotation motion callback, status: %{public}d", motionData.status);
    DeviceRotation motionRotation = DeviceRotation::INVALID;
    motionRotation = ConvertMotionActionToDeviceRotation(motionData.status);
    float rotation = ConvertDeviceMotionToFloat(motionRotation);
    
    MotionManager::GetInstance().HandleMotionEvent(MotionType::SMART_MOTION_TYPE, rotation);
}

void MotionManager::HandleMotionEvent(MotionType motionType, float rotation)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (motionType == MotionType::DEVICE_MOTION_TYPE) {
        HandleDeviceSensorRotation(rotation);
    } else if (motionType == MotionType::SMART_MOTION_TYPE || motionType == MotionType::SMART_MOTION_ENHANCE_TYPE) {
        HandleSmartSensorRotation(rotation);
    }
}

void MotionManager::HandleDeviceSensorRotation(float rotation)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "rotation: %{public}f", rotation);
    if (lastMotionRotation_ == rotation) {
        TLOGD(WmsLogTag::WMS_ROTATION, "rotation unchanged, skip notification");
        return;
    }
    lastMotionRotation_ = rotation;
    if (motionEventListener_ != nullptr) {
        motionEventListener_->OnMotionRotationChanged(rotation);
    }
}

void MotionManager::HandleSmartSensorRotation(float rotation)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "rotation: %{public}f", rotation);
    lastSmartMotionRotation_ = rotation;
    if (motionEventListener_ != nullptr) {
        motionEventListener_->OnMotionSmartRotationChanged(rotation);
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

bool MotionManager::IsInitialized() const
{
    return isInitialized_;
}

void MotionManager::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    UnsubscribeAllMotionSensors();
    lastMotionRotation_ = -1.0f;
    lastSmartMotionRotation_ = -1.0f;
    motionEventListener_ = nullptr;
    isInitialized_ = false;
    TLOGI(WmsLogTag::WMS_ROTATION, "MotionManager reset");
}

void MotionManager::TestHandleMotionEvent(MotionType motionType, float rotation)
{
    HandleMotionEvent(motionType, rotation);
}

bool MotionSubscriberWrapper::SubscribeMotionSensor(MotionType motionType)
{
    return MotionManager::GetInstance().SubscribeMotionSensor(motionType);
}

bool MotionSubscriberWrapper::UnsubscribeMotionSensor(MotionType motionType)
{
    return MotionManager::GetInstance().UnsubscribeMotionSensor(motionType);
}

} // namespace Rosen
} // namespace OHOS