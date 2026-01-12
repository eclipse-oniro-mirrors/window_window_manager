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

#ifndef OHOS_ROSEN_EXTENSION_BUSINESS_INFO_H
#define OHOS_ROSEN_EXTENSION_BUSINESS_INFO_H

#include <cstdint>

namespace OHOS::Rosen::Extension {
constexpr const char* const WINDOW_MODE_FIELD = "windowMode";
constexpr const char* const CROSS_AXIS_FIELD = "crossAxis";
constexpr const char* const WATERFALL_MODE_FIELD = "waterfallMode";
constexpr const char* const UIEXTENSION_CONFIG_FIELD = "ohos.system.window.uiextension.params";
constexpr const char* const ROOT_HOST_WINDOW_TYPE_FIELD = "rootHostWindowType";
constexpr const char* const GESTURE_BACK_ENABLED = "gestureBackEnabled";
constexpr const char* const HOST_WINDOW_DELAY_RAISE_STATE_FIELD = "hostWindowDelayRaiseState";
constexpr const char* const ATOMICSERVICE_KEY_FUNCTION = "ohos.atomicService.window.function";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_ENABLE = "ohos.atomicService.window.param.enable";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_STATUS = "ohos.atomicService.window.param.status";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_NAVIGATION = "ohos.atomicService.window.param.navigation";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_NAME = "ohos.atomicService.window.param.name";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_ENABLEANIMATION = "ohos.atomicService.window.param.enableAnimation";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_ISKEEPSCREENON = "ohos.atomicService.window.param.isKeepScreenOn";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_BRIGHTNESS = "ohos.atomicService.window.param.brightness";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_COLOR_NUMERIC = "ohos.atomicService.window.param.color.numeric";
constexpr const char* const RECT_X = "rectX";
constexpr const char* const RECT_Y = "rectY";
constexpr const char* const RECT_WIDTH = "rectWidth";
constexpr const char* const RECT_HEIGHT = "rectHeight";
constexpr const char* const BEGIN_X = "beginX";
constexpr const char* const BEGIN_Y = "beginY";
constexpr const char* const BEGIN_WIDTH = "beginWidth";
constexpr const char* const BEGIN_HEIGHT = "beginHeight";
constexpr const char* const END_X = "endX";
constexpr const char* const END_Y = "endY";
constexpr const char* const END_WIDTH = "endWidth";
constexpr const char* const END_HEIGHT = "endHeight";
constexpr const char* const RECT_CHANGE_REASON = "rectChangeReason";
constexpr const char* const COMPAT_IS_SIMULATION_SCALE_FIELD = "compat.isSimulationScale";
constexpr const char* const COMPAT_IS_PROPORTION_SCALE_FIELD = "compat.isProportionScale";
constexpr const char* const COMPAT_SCALE_X_FIELD = "compat.scaleX";
constexpr const char* const COMPAT_SCALE_Y_FIELD = "compat.scaleY";
constexpr const char* const EXTENSION_SECURE_LIMIT_CHANGE = "extensionSecureLimitChange";
constexpr const char* const HOST_STATUS_BAR_CONTENT_COLOR = "hostStatusBarContentColor";
constexpr const char* const GRAVITY = "gravity";
constexpr const char* const ISSHOWING = "isShowing";

enum class Businesscode : uint32_t {
    SYNC_HOST_WINDOW_MODE = 0,  // Synchronize the host's window mode
    SYNC_CROSS_AXIS_STATE,
    SYNC_HOST_WATERFALL_MODE,
    SYNC_WANT_PARAMS,
    SYNC_HOST_GESTURE_BACK_ENABLED,
    SYNC_HOST_WINDOW_DELAY_RAISE_STATE,
    NOTIFY_HOST_WINDOW_RECT_CHANGE,
    SYNC_COMPAT_INFO,
    NOTIFY_SCREENSHOT,
    NOTIFY_EXTENSION_SECURE_LIMIT_CHANGE,
    NOTIFY_KEYBOARD_DID_HIDE,
    NOTIFY_KEYBOARD_DID_SHOW,
    SYNC_HOST_STATUS_BAR_CONTENT_COLOR,
    NOTIFY_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY,

    // send to host
    OCCUPY_EVENTS = 1001,
    // IPC messages corresponding to the Businesscode between 10000 and 20000
    // will be forwarded to the host window by the UEC
    TRANSPARENT_TRANSMISSION_TO_HOST_WINDOW_CODE_BEGIN = 10000,
    NOTIFY_HOST_WINDOW_TO_RAISE = 10001,
    REGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER,
    UNREGISTER_HOST_WINDOW_RECT_CHANGE_LISTENER,
    REGISTER_KEYBOARD_DID_SHOW_LISTENER,
    UNREGISTER_KEYBOARD_DID_SHOW_LISTENER,
    REGISTER_KEYBOARD_DID_HIDE_LISTENER,
    UNREGISTER_KEYBOARD_DID_HIDE_LISTENER,
    REGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER,
    UNREGISTER_HOST_RECT_CHANGE_IN_GLOBAL_DISPLAY_LISTENER,
    TRANSPARENT_TRANSMISSION_TO_HOST_WINDOW_CODE_END = 20000,
};
}  // namespace OHOS::Rosen::Extension

#endif  // OHOS_ROSEN_EXTENSION_BUSINESS_INFO_H