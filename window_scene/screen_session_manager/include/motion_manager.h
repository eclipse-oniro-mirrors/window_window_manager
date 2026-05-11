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

#ifndef OHOS_ROSEN_MOTION_MANAGER_H
#define OHOS_ROSEN_MOTION_MANAGER_H

#include <mutex>
#include <map>
#include "refbase.h"
#include "dm_common.h"
#include "screen_rotation_property.h"
#include "screen_sensor_plugin.h"
#include "window_manager_hilog.h"

#ifdef WM_SUBSCRIBE_MOTION_ENABLE
constexpr int32_t DISABLED_SMART_ROTATION = 0;
constexpr int32_t ENABLED_SMART_ROTATION = 1;
#endif

namespace OHOS {
namespace Rosen {

enum MotionType : int32_t {
    DEVICE_MOTION_TYPE = 700,
    SMART_MOTION_TYPE = 701,
    SMART_MOTION_ENHANCE_TYPE = 702,
};

enum MotionAction : int32_t {
    MOTION_PORTRAIT = 0,
    MOTION_LANDSCAPE = 1,
    MOTION_PORTRAIT_INVERTED = 2,
    MOTION_LANDSCAPE_INVERTED = 3,
};

class IMotionEventListener {
public:
    virtual ~IMotionEventListener() = default;
    virtual void OnMotionRotationChanged(float sensorRotation) = 0;
};

class MotionManager {
public:
    static MotionManager& GetInstance();
    
    void Init();
    void SetMotionEventListener(IMotionEventListener* listener);
    
    void SubscribeMotionSensor(MotionType motionType);
    void UnsubscribeMotionSensor(MotionType motionType);
    
    void OnScreenOn();
    void OnScreenOff();
    
    float GetLastMotionRotation() const;
    float GetLastSmartMotionRotation() const;
    
    bool IsMotionSensorSubscribed(MotionType motionType) const;
    bool NeedMotionSensorSubscribe(MotionType motionType) const;
    bool IsScreenOn() const;
    bool IsInitialized() const;
    bool IsDefaultSmartMotionEnabled() const;
    void Reset();
    void SetScreenOnState(bool isScreenOn);
    void SetDefaultSmartMotionEnabled(bool enabled);
    
    void TestHandleMotionEvent(MotionType motionType, float rotation);
    
private:
    MotionManager();
    ~MotionManager();
    MotionManager(const MotionManager&) = delete;
    MotionManager& operator=(const MotionManager&) = delete;
    
    void SubscribeDefaultMotionSensors();
    void UnsubscribeAllMotionSensors();
    void SubscribeMotionSensorInternal(MotionType motionType);
    void UnsubscribeMotionSensorInternal(MotionType motionType);
    void HandleMotionEvent(MotionType motionType, float rotation);
    void HandleDeviceSensorRotation(float rotation);
    void HandleSmartSensorRotation(float rotation);
    
    static DeviceRotation ConvertMotionActionToDeviceRotation(int32_t motionAction);
    static float ConvertDeviceMotionToFloat(DeviceRotation deviceRotation);
    static void RotationMotionEventCallback(const MotionSensorEvent& motionData);
    static void SmartRotationMotionEventCallback(const MotionSensorEvent& motionData);
    
    std::mutex mutex_;
    IMotionEventListener* motionEventListener_ = nullptr;
    
    // 已注册类型
    std::map<MotionType, bool> subscribedMotionTypes_;
    // 亮屏时需要注册类型
    std::map<MotionType, bool> needSubscribedMotionTypes_;
    float lastMotionRotation_ = -1.0f;
    float lastSmartMotionRotation_ = -1.0f;
    
    bool isDefaultSmartMotionEnabled_ = false;
    bool isScreenOn_ = true;
    bool isInitialized_ = false;
};

#ifdef WM_SUBSCRIBE_MOTION_ENABLE
class MotionSubscriberWrapper {
friend MotionManager;
public:
    MotionSubscriberWrapper() = delete;
    ~MotionSubscriberWrapper() = default;
private:
    static bool SubscribeMotionSensor(MotionType motionType);
    static bool UnsubscribeMotionSensor(MotionType motionType);
    
    static std::map<MotionType, bool> isMotionSubscribedMap_;
};
#endif

} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_MOTION_MANAGER_H