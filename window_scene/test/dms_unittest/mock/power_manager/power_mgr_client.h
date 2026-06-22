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

#ifndef TEST_POWERMGR_POWER_MGR_CLIENT_H
#define TEST_POWERMGR_POWER_MGR_CLIENT_H

#include <string>
#include <singleton.h>

namespace OHOS {
namespace PowerMgr {

enum class PowerErrors : int32_t {
    ERR_OK = 0,
    ERR_FAILURE = 1,
    ERR_PERMISSION_DENIED = 201,
    ERR_SYSTEM_API_DENIED = 202,
    ERR_PARAM_INVALID = 401,
    ERR_CONNECTION_FAIL = 4900101,
    ERR_FREQUENT_FUNCTION_CALL = 4900201,
    ERR_POWER_MODE_TRANSIT_FAILED = 4900301,
};

enum class WakeupDeviceType : uint32_t {
    WAKEUP_DEVICE_UNKNOWN = 0,
    WAKEUP_DEVICE_POWER_BUTTON = 1,
    WAKEUP_DEVICE_APPLICATION = 2,
    WAKEUP_DEVICE_PLUGGED_IN = 3,
    WAKEUP_DEVICE_GESTURE = 4,
    WAKEUP_DEVICE_CAMERA_LAUNCH = 5,
    WAKEUP_DEVICE_WAKE_KEY = 6,
    WAKEUP_DEVICE_WAKE_MOTION = 7,
    WAKEUP_DEVICE_HDMI = 8,
    WAKEUP_DEVICE_LID = 9,
    WAKEUP_DEVICE_DOUBLE_CLICK = 10,
    WAKEUP_DEVICE_KEYBOARD = 11,
    WAKEUP_DEVICE_MOUSE = 12,
    WAKEUP_DEVICE_TOUCHPAD = 13,
    WAKEUP_DEVICE_PEN = 14,
    WAKEUP_DEVICE_TOUCH_SCREEN = 15,
    WAKEUP_DEVICE_SWITCH = 16,
    WAKEUP_DEVICE_SINGLE_CLICK = 17,
    WAKEUP_DEVICE_PRE_BRIGHT = 18,
    WAKEUP_DEVICE_PRE_BRIGHT_AUTH_SUCCESS = 19,
    WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON = 20,
    WAKEUP_DEVICE_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF = 21,
    WAKEUP_DEVICE_AOD_SLIDING = 22,
    WAKEUP_DEVICE_INCOMING_CALL = 23,
    WAKEUP_DEVICE_SHELL = 24,
    WAKEUP_DEVICE_PICKUP = 25,
    WAKEUP_DEVICE_EXIT_SYSTEM_STR = 26,  // STR: suspend to ram
    WAKEUP_DEVICE_SCREEN_CONNECT = 27,
    WAKEUP_DEVICE_TP_TOUCH = 28,
    WAKEUP_DEVICE_EX_SCREEN_INIT = 29,
    WAKEUP_DEVICE_ABNORMAL_SCREEN_CONNECT = 30,
    WAKEUP_DEVICE_PLUG_CHANGE = 31,
    WAKEUP_DEVICE_TENT_MODE_CHANGE = 32,
    WAKEUP_DEVICE_END_DREAM = 33,
    WAKEUP_DEVICE_BLUETOOTH_INCOMING_CALL = 34,
    WAKEUP_DEVICE_FROM_ULSR = 35,
    WAKEUP_DEVICE_MESSAGE_NOTIFICATION = 36,
    WAKEUP_DEVICE_MAX
};

class PowerMgrClient final {
public:
    static PowerMgrClient& GetInstance()
    {
        static PowerMgrClient instance_;
        return instance_;
    }
    virtual ~PowerMgrClient() = default;

    PowerErrors WakeupDevice(WakeupDeviceType reason = WakeupDeviceType::WAKEUP_DEVICE_APPLICATION,
                             const std::string& detail = std::string("app call"), const std::string& apiVersion = "-1")
    {
        return PowerErrors::ERR_OK;
    }
    void WakeupDeviceAsync(WakeupDeviceType reason = WakeupDeviceType::WAKEUP_DEVICE_APPLICATION,
                           const std::string& detail = std::string("app call"))
    {
    }
    bool RefreshActivity()
    {
        return true;
    }

    bool IsScreenOn(bool needPrintLog = true)
    {
        return true;
    }

    bool IsFoldScreenOn()
    {
        return true;
    }

    PowerErrors SetForceTimingOut(bool enabled)
    {
        return PowerErrors::ERR_OK;
    }

    PowerErrors LockScreenAfterTimingOut(bool enabledLockScreen, bool checkLock, bool sendScreenOffEvent = true)
    {
        return PowerErrors::ERR_OK;
    }

private:
    PowerMgrClient() = default;
};
}  // namespace PowerMgr
}  // namespace OHOS
#endif  // POWERMGR_POWER_MGR_CLIENT_H
