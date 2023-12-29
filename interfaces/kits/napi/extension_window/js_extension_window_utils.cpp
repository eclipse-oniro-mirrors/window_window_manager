/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "js_extension_window_utils.h"

#include "js_extension_window.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace  AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsExtensionWindowUtils"};
}

napi_value GetRectAndConvertToJsValue(napi_env env, const Rect& rect)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to convert rect to jsObject");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "left", CreateJsValue(env, rect.posX_));
    napi_set_named_property(env, objValue, "top", CreateJsValue(env, rect.posY_));
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, rect.width_));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, rect.height_));
    return objValue;
}

napi_value ConvertAvoidAreaToJsValue(napi_env env, const AvoidArea& avoidArea, AvoidAreaType type)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to convert avoidArea to jsObject");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "visible",
                            CreateJsValue(env, type == AvoidAreaType::TYPE_CUTOUT ? false : true));
    napi_set_named_property(env, objValue, "leftRect", GetRectAndConvertToJsValue(env, avoidArea.leftRect_));
    napi_set_named_property(env, objValue, "topRect", GetRectAndConvertToJsValue(env, avoidArea.topRect_));
    napi_set_named_property(env, objValue, "rightRect", GetRectAndConvertToJsValue(env, avoidArea.rightRect_));
    napi_set_named_property(env, objValue, "bottomRect", GetRectAndConvertToJsValue(env, avoidArea.bottomRect_));
    return objValue;
}

napi_value CreateJsExtensionWindowPropertiesObject(napi_env env, sptr<Window>& window)
{
    WLOGI("CreateJsExtensionWindowPropertiesObject is called");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to convert windowProperties to jsObject");
        return nullptr;
    }

    Rect windowRect = window->GetRect();
    napi_value windowRectObj = GetRectAndConvertToJsValue(env, windowRect);
    if (windowRectObj == nullptr) {
        WLOGFE("GetWindowRect Failed");
    }
    napi_set_named_property(env, objValue, "uiExtensionHostWindowProxyRect", windowRectObj);
    return objValue;
}

bool NapiIsCallable(napi_env env, napi_value value)
{
    bool result = false;
    napi_is_callable(env, value, &result);
    return result;
}

napi_value NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiThrowError(napi_env env, WmErrorCode errCode)
{
    napi_throw(env, CreateJsError(env, static_cast<int32_t>(errCode)));
    return NapiGetUndefined(env);
}
}  // namespace Rosen
}  // namespace OHOS