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

#include "js_window_utils.h"
#include "js_extension_window.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace  AbilityRuntime;

napi_value GetRectAndConvertToJsValue(napi_env env, const Rect& rect)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert rect to jsObject");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "left", CreateJsValue(env, rect.posX_));
    napi_set_named_property(env, objValue, "top", CreateJsValue(env, rect.posY_));
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, rect.width_));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, rect.height_));
    return objValue;
}

napi_value CreateJsExtensionWindowPropertiesObject(napi_env env, sptr<Window>& window)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "CreateJsExtensionWindowPropertiesObject is called");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert windowProperties to jsObject");
        return nullptr;
    }

    Rect windowRect = window->GetRect();
    napi_value windowRectObj = GetRectAndConvertToJsValue(env, windowRect);
    if (windowRectObj == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "GetWindowRect Failed");
    }
    napi_set_named_property(env, objValue, "uiExtensionHostWindowProxyRect", windowRectObj);
    return objValue;
}

napi_value CreateJsExtensionWindowProperties(napi_env env, sptr<Window>& window)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "CreateJsWindowPropertiesObject is called");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert windowProperties to jsObject");
        return nullptr;
    }

    Rect windowRect = window->GetRect();
    napi_value windowRectObj = GetRectAndConvertToJsValue(env, windowRect);
    if (windowRectObj == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "GetWindowRect failed!");
    }
    napi_set_named_property(env, objValue, "windowRect", windowRectObj);
    
    WindowType type = window->GetType();
    if (NATIVE_JS_TO_WINDOW_TYPE_MAP.count(type) != 0) {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, NATIVE_JS_TO_WINDOW_TYPE_MAP.at(type)));
    } else {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, type));
    }
    napi_set_named_property(env, objValue, "isLayoutFullScreen", CreateJsValue(env, window->IsLayoutFullScreen()));
    napi_set_named_property(env, objValue, "isFullScreen", CreateJsValue(env, window->IsFullScreen()));
    napi_set_named_property(env, objValue, "touchable", CreateJsValue(env, window->GetTouchable()));
    napi_set_named_property(env, objValue, "focusable", CreateJsValue(env, window->GetFocusable()));
    napi_set_named_property(env, objValue, "name", CreateJsValue(env, window->GetWindowName()));
    napi_set_named_property(env, objValue, "isPrivacyMode", CreateJsValue(env, window->IsPrivacyMode()));
    napi_set_named_property(env, objValue, "isKeepScreenOn", CreateJsValue(env, window->IsKeepScreenOn()));
    napi_set_named_property(env, objValue, "brightness", CreateJsValue(env, window->GetBrightness()));
    napi_set_named_property(env, objValue, "isTransparent", CreateJsValue(env, window->IsTransparent()));
    napi_set_named_property(env, objValue, "isRoundCorner", CreateJsValue(env, false)); // empty method
    napi_set_named_property(env, objValue, "dimBehindValue", CreateJsValue(env, 0));
    napi_set_named_property(env, objValue, "id", CreateJsValue(env, window->GetWindowId()));
    return objValue;
}

static std::string GetHexColor(uint32_t color)
{
    std::stringstream ioss;
    std::string temp;
    ioss << std::setiosflags(std::ios::uppercase) << std::hex << color;
    ioss >> temp;
    int count = RGBA_LENGTH - static_cast<int>(temp.length());
    std::string tmpColor(count, '0');
    tmpColor += temp;
    std::string finalColor("#");
    finalColor += tmpColor;
    return finalColor;
}

napi_value GetHostStatusBarPropertyObject(napi_env env, sptr<Window>& window)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "in");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert statusbar property to jsObject");
        return nullptr;
    }
    uint32_t contentColor = window->GetHostStatusBarContentColor();
    napi_set_named_property(env, objValue, "contentColor", CreateJsValue(env, GetHexColor(contentColor)));
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