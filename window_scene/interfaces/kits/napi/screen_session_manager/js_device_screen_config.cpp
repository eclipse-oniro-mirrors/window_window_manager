/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_device_screen_config.h"
#include "js_screen_utils.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "JsDeviceScreenConfig" };
} // namespace

napi_value JsDeviceScreenConfig::CreateDeviceScreenConfig(napi_env env, const DeviceScreenConfig& config)
{
    WLOGI("[NAPI]CreateDeviceScreenConfig");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("[NAPI]Object is null!");
        return NapiGetUndefined(env);
    }
    WLOGI("DeviceScreenConfig rotationPolicy:%{public}s.", config.rotationPolicy_.c_str());
    napi_set_named_property(env, objValue, "rotationPolicy", CreateJsValue(env, config.rotationPolicy_));
    return objValue;
}

JsDeviceScreenConfig::JsDeviceScreenConfig()
{
    WLOGFD("Construct JsDeviceScreenConfig");
}

JsDeviceScreenConfig::~JsDeviceScreenConfig()
{
    WLOGFD("Destroy  JsDeviceScreenConfig");
}

} // namespace OHOS::Rosen
