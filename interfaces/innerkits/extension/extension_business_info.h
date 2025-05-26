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
constexpr const char* const GESTURE_BACK_ENABLED = "gestureBackEnabled";
constexpr const char* const IMMERSIVE_MODE_ENABLED = "immersiveModeEnabled";
constexpr const char* const HOST_WINDOW_DELAY_RAISE_STATE_FIELD = "hostWindowDelayRaiseState";
constexpr const char* const ATOMICSERVICE_KEY_FUNCTION = "ohos.atomicService.window.function";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_ENABLE = "ohos.atomicService.window.param.enable";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_STATUS = "ohos.atomicService.window.param.status";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_NAVIGATION = "ohos.atomicService.window.param.navigation";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_NAME = "ohos.atomicService.window.param.name";
constexpr const char* const ATOMICSERVICE_KEY_PARAM_ENABLEANIMATION = "ohos.atomicService.window.param.enableAnimation";

enum class Businesscode : uint32_t {
    SYNC_HOST_WINDOW_MODE = 0,  // Synchronize the host's window mode
    SYNC_CROSS_AXIS_STATE,
    SYNC_HOST_WATERFALL_MODE,
    SYNC_WANT_PARAMS,
    SYNC_HOST_IMMERSIVE_MODE_ENABLED,
    SYNC_HOST_GESTURE_BACK_ENABLED,
    SYNC_HOST_WINDOW_DELAY_RAISE_STATE,

    // IPC messages corresponding to the Businesscode between 10000 and 20000
    // will be forwarded to the host window by the UEC
    TRANSPARENT_TRANSMISSION_TO_HOST_WINDOW_CODE_BEGIN = 10000,
    NOTIFY_HOST_WINDOW_TO_RAISE = 10001,
    TRANSPARENT_TRANSMISSION_TO_HOST_WINDOW_CODE_END = 20000,
};
}  // namespace OHOS::Rosen::Extension

#endif  // OHOS_ROSEN_EXTENSION_BUSINESS_INFO_H