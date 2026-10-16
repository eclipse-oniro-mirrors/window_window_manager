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
#include <algorithm>
#include <memory>
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
    motionEventListener_ = listener;
    TLOGI(WmsLogTag::WMS_ROTATION, "Motion event listener set");
}

bool MotionManager::SubscribeMotionSensorInternal(MotionType motionType)
{
#ifdef WM_SUBSCRIBE_MOTION_ENABLE
    if (subscribedMotionTypes_[motionType]) {
        TLOGW(WmsLogTag::WMS_ROTATION, "Motion type %{public}d already subscribed", motionType);
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
    TLOGW(WmsLogTag::WMS_ROTATION, "Successfully subscribed motion type: %{public}d", motionType);
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
        TLOGW(WmsLogTag::WMS_ROTATION, "Motion type %{public}d not subscribed", motionType);
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
    TLOGW(WmsLogTag::WMS_ROTATION, "Successfully unsubscribed motion type: %{public}d", motionType);
    return true;
#else
    TLOGW(WmsLogTag::WMS_ROTATION, "WM_SUBSCRIBE_MOTION_ENABLE not defined");
    return false;
#endif
}

bool MotionManager::SubscribeMotionSensor(MotionType motionType)
{
    return SubscribeMotionSensorInternal(motionType);
}

bool MotionManager::UnsubscribeMotionSensor(MotionType motionType)
{
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

bool MotionManager::IsValidDigitString(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    for (char ch : str) {
        if ((ch >= '0' && ch <= '9')) {
            continue;
        }
        return false;
    }
    return true;
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
    // 智感旋转上报原因： 0-2（0：人脸；1：握姿； 2：重力），底层硬件通信数据为 int16_t
    int16_t reason = -1;
    int32_t dataLen = motionData.dataLen * 2; // 传入数据为int32_t,要转换为int16_t,因此对应的长度计算要*2

    std::unique_ptr<int16_t[]> patchData;
    if (dataLen < 1 || motionData.data == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Invalid motion data, dataLen: %{public}d", dataLen);
        patchData = std::make_unique<int16_t[]>(1);
        patchData[0] = -1;
    } else {
        patchData = std::make_unique<int16_t[]>(dataLen);
        errno_t ret = memcpy_s(patchData.get(), dataLen * sizeof(int16_t),
            motionData.data, dataLen * sizeof(int16_t));
        if (ret != EOK ) {
            TLOGE(WmsLogTag::WMS_ROTATION, "memcpy_s failed, ret: %{public}d", ret);
            std:fill(patchData.get(), patchData.get() + dataLen, -1);
        }
    }
    if (motionData.type == MotionType::SMART_MOTION_TYPE && 
        (motionData.status == 4 || motionData.status == 5)) {
        TLOGI(WmsLogTag::WMS_ROTATION, "Smart rotation motion callback, motionData: %{public}d", patchData[0]);
        motionRotation = ConvertMotionActionToDeviceRotation(patchData[0]);
        if (dataLen >= 2) {
            return = patchData[1];
        }
    } else {
        motionRotation = ConvertMotionActionToDeviceRotation(motionData.status);
        if (dataLen >= 4) {
            TLOGI(WmsLogTag::WMS_ROTATION, "Smart rotation motion callback enhance, face_direction: %{public}d,"
                "handhold_direction:%{public}d, sensor_direction: %{public}d, data_source: %{public}d",
                patchData[0], patchData[1], patchData[2], patchData[3]);
            reason = patchData[3];
        } else {
            TLOGE(WmsLogTag::WMS_ROTATION， "Invalid data with motion callback enhance, dataLen: %{public}d", dataLen)；
        }
    }
    float rotation = ConvertDeviceMotionToFloat(motionRotation);
    
    MotionManager::GetInstance().HandleMotionEvent(MotionType::SMART_MOTION_TYPE, rotation, reason);
}

void MotionManager::HandleMotionEvent(MotionType motionType, float rotation, int16_t reason)
{
    if (motionType == MotionType::DEVICE_MOTION_TYPE) {
        HandleDeviceSensorRotation(rotation);
    } else if (motionType == MotionType::SMART_MOTION_TYPE || motionType == MotionType::SMART_MOTION_ENHANCE_TYPE) {
        HandleSmartSensorRotation(rotation, reason);
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

void MotionManager::HandleSmartSensorRotation(float rotation, int16_t reason)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "rotation: %{public}f, reason: %{public}d", rotation, reason);
    lastSmartMotionRotation_ = rotation;
    if (motionEventListener_ != nullptr) {
        motionEventListener_->OnMotionSmartRotationChanged(rotation, reason);
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
    UnsubscribeAllMotionSensors();
    lastMotionRotation_ = -1.0f;
    lastSmartMotionRotation_ = -1.0f;
    motionEventListener_ = nullptr;
    isInitialized_ = false;
    TLOGI(WmsLogTag::WMS_ROTATION, "MotionManager reset");
}

void MotionManager::TestHandleMotionEvent(MotionType motionType, float rotation, int16_t reason)
{
    HandleMotionEvent(motionType, rotation, reason);
}

WSError MotionManager::SetMotionValueByDump(const std::vector<std::string>& args, std::string& dumpInfo)
{
    if (!this->IsValidDigitString(args[1]) || args[1].size() > 1) { // 1:string size
        TLOGE(WmsLogTag::WMS_ROTATION, "motion value must be digit and length can't above 1");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    int value = std::stoi(args[1]);
    if (value < MotionAction::MOTION_PORTRAIT || value > MotionAction::MOTION_LANDSCAPE_INVERTED) {
        TLOGE(WmsLogTag::WMS_ROTATION, "motion invalid value: %{public}d", value);
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    float rotation = static_cast<float>(value * 90);
    this->TestHandleMotionEvent(MotionType::DEVICE_MOTION_TYPE, rotation);
    dumpInfo.append("motion stub, rotation: ").append(args[1]).append("\n");
    return WSError::WS_OK;
}
 
WSError MotionManager::SetSmartMotionValueByDump(const std::vector<std::string>& args, std::string& dumpInfo)
{
    if (!this->IsValidDigitString(args[1]) || args[1].size() > 1) { // 1:string size
        TLOGE(WmsLogTag::WMS_ROTATION, "smartmotion value must be digit and length can't above 1");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    int value = std::stoi(args[1]);
    if (value < MotionAction::MOTION_PORTRAIT || value > MotionAction::MOTION_LANDSCAPE_INVERTED) {
        TLOGE(WmsLogTag::WMS_ROTATION, "smartmotion invalid value: %{public}d", value);
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    float rotation = static_cast<float>(value * 90);
    this->TestHandleMotionEvent(MotionType::SMART_MOTION_TYPE, rotation);
    dumpInfo.append("smartmotion stub, rotation: ").append(args[1]).append("\n");
    return WSError::WS_OK;
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