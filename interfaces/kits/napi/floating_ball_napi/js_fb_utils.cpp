/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "js_fb_utils.h"

#include <string>
#include "window.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
napi_value NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiThrowInvalidParam(napi_env env, const std::string& msg)
{
    napi_throw(env, AbilityRuntime::CreateJsError(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_FB_PARAM_INVALID), msg));
    return NapiGetUndefined(env);
}

napi_valuetype GetType(napi_env env, napi_value value)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, value, &res);
    return res;
}

bool NapiIsCallable(napi_env env, napi_value value)
{
    bool result = false;
    napi_is_callable(env, value, &result);
    return result;
}

static napi_status SetEnumProperty(napi_env env, napi_value& obj, const std::string& enumName, int32_t enumValue)
{
    napi_value value = nullptr;
    napi_status ret = napi_create_int32(env, enumValue, &value);
    if (ret != napi_ok) {
        return ret;
    }
    ret = napi_set_named_property(env, obj, enumName.c_str(), value);
    if (ret != napi_ok) {
        return ret;
    }
    return ret;
}

static napi_value ExportFloatingBallState(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to get object");
        return nullptr;
    }
    (void)SetEnumProperty(env, result, "STARTED", static_cast<int32_t>(FloatingBallState::STARTED));
    (void)SetEnumProperty(env, result, "STOPPED", static_cast<int32_t>(FloatingBallState::STOPPED));
    napi_object_freeze(env, result);
    return result;
}

static napi_value ExportFloatingBallTemplate(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to get object");
        return nullptr;
    }
    (void)SetEnumProperty(env, result, "STATIC", static_cast<int32_t>(FloatingBallTemplate::STATIC));
    (void)SetEnumProperty(env, result, "NORMAL", static_cast<int32_t>(FloatingBallTemplate::NORMAL));
    (void)SetEnumProperty(env, result, "EMPHATIC", static_cast<int32_t>(FloatingBallTemplate::EMPHATIC));
    (void)SetEnumProperty(env, result, "SIMPLE", static_cast<int32_t>(FloatingBallTemplate::SIMPLE));
    napi_object_freeze(env, result);
    return result;
}

static napi_value ExportFbTextUpdateAnimationType(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to get object");
        return nullptr;
    }
    (void)SetEnumProperty(env, result, "ANIMATION_NONE",
        static_cast<int32_t>(FloatingBallTextUpdateAnimationType::ANIMATION_NONE));
    (void)SetEnumProperty(env, result, "ANIMATION_OPACITY",
        static_cast<int32_t>(FloatingBallTextUpdateAnimationType::ANIMATION_OPACITY));
    napi_object_freeze(env, result);
    return result;
}

napi_status InitFbEnums(napi_env env, napi_value exports)
{
    const napi_property_descriptor properties[] = {
        DECLARE_NAPI_GETTER("FloatingBallState", ExportFloatingBallState),
        DECLARE_NAPI_GETTER("FloatingBallTemplate", ExportFloatingBallTemplate),
        DECLARE_NAPI_GETTER("FloatingBallTextUpdateAnimationType", ExportFbTextUpdateAnimationType),
    };
    const size_t count = sizeof(properties) / sizeof(napi_property_descriptor);
    return napi_define_properties(env, exports, count, properties);
}

} // Rosen
} // OHOS