/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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
#include "js_float_view_utils.h"

#include "js_err_utils.h"
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

napi_value NapiThrowError(napi_env env, WmErrorCode errCode, const std::string& msg)
{
    napi_throw(env, JsErrUtils::CreateJsError(env, errCode, msg));
    return NapiGetUndefined(env);
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

static napi_value ExportFloatViewState(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to get object");
        return nullptr;
    }
    (void)SetEnumProperty(env, result, "STARTED", static_cast<int32_t>(FloatViewState::FV_STARTED));
    (void)SetEnumProperty(env, result, "HIDDEN", static_cast<int32_t>(FloatViewState::FV_HIDDEN));
    (void)SetEnumProperty(env, result, "STOPPED", static_cast<int32_t>(FloatViewState::FV_STOPPED));
    (void)SetEnumProperty(env, result, "IN_SIDEBAR", static_cast<int32_t>(FloatViewState::FV_IN_SIDEBAR));
    (void)SetEnumProperty(env, result, "IN_FLOATING_BALL", static_cast<int32_t>(FloatViewState::FV_IN_FLOATING_BALL));
    (void)SetEnumProperty(env, result, "ERROR", static_cast<int32_t>(FloatViewState::FV_ERROR));
    napi_object_freeze(env, result);
    return result;
}

napi_value CallJsFunction(napi_env env, napi_value method, napi_value const* argv, size_t argc)
{
    if (env == nullptr || method == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "env nullptr or method is nullptr");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value callResult = nullptr;
    napi_get_undefined(env, &result);
    napi_get_undefined(env, &callResult);
    napi_call_function(env, result, method, argc, argv, &callResult);
    TLOGD(WmsLogTag::WMS_SYSTEM, "called.");
    return callResult;
}

bool NapiIsCallable(napi_env env, napi_value value)
{
    bool result = false;
    napi_is_callable(env, value, &result);
    return result;
}

static napi_value ExportFloatViewTemplate(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (result == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to get object");
        return nullptr;
    }
    (void)SetEnumProperty(env, result, "ROUNDED_RECTANGLE", static_cast<int32_t>(FloatViewTemplate::ROUNDED_RECTANGLE));
    napi_object_freeze(env, result);
    return result;
}

napi_status InitFvEnums(napi_env env, napi_value exports)
{
    const napi_property_descriptor properties[] = {
        DECLARE_NAPI_GETTER("FloatViewState", ExportFloatViewState),
        DECLARE_NAPI_GETTER("FloatViewTemplateType", ExportFloatViewTemplate),
    };
    const size_t count = sizeof(properties) / sizeof(napi_property_descriptor);
    return napi_define_properties(env, exports, count, properties);
}

napi_value GetRectAndConvertToJsValue(napi_env env, const Rect& rect)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to get object when get rect");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "left", CreateJsValue(env, rect.posX_));
    napi_set_named_property(env, objValue, "top", CreateJsValue(env, rect.posY_));
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, rect.width_));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, rect.height_));
    return objValue;
}

napi_value ConvertAvoidAreaToJsValue(napi_env env, const AvoidArea& avoidArea)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to create object when get avoidArea");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "visible", CreateJsValue(env, true));
    napi_set_named_property(env, objValue, "leftRect", GetRectAndConvertToJsValue(env, avoidArea.leftRect_));
    napi_set_named_property(env, objValue, "topRect", GetRectAndConvertToJsValue(env, avoidArea.topRect_));
    napi_set_named_property(env, objValue, "rightRect", GetRectAndConvertToJsValue(env, avoidArea.rightRect_));
    napi_set_named_property(env, objValue, "bottomRect", GetRectAndConvertToJsValue(env, avoidArea.bottomRect_));
    return objValue;
}

napi_value CreateJsFloatViewPropertiesObject(napi_env env, uint32_t templateType,
    const sptr<Window>& window, const FloatViewWindowInfo &windowInfo, const FvWindowState &state)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "templateType", CreateJsValue(env, templateType));
    auto windowId = window->GetWindowId();
    napi_set_named_property(env, objValue, "windowId", CreateJsValue(env, windowId));
    auto displayId = window->GetDisplayId();
    napi_set_named_property(env, objValue, "displayId", CreateJsValue(env, static_cast<int64_t>(displayId)));
    napi_value windowRectObj = GetRectAndConvertToJsValue(env, windowInfo.windowRect_);
    if (windowRectObj == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "GetWindowRect failed!");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "windowRect", windowRectObj);
    napi_set_named_property(env, objValue, "windowScale", CreateJsValue(env, windowInfo.scale_));
    napi_value avoidAreaObj = ConvertAvoidAreaToJsValue(env, windowInfo.avoidArea_);
    if (avoidAreaObj == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "GetWindowAvoidArea failed!");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "avoidArea", avoidAreaObj);
    bool isSidebar = state == FvWindowState::FV_STATE_IN_SIDEBAR;
    napi_set_named_property(env, objValue, "inSidebar", CreateJsValue(env, isSidebar));
    return objValue;
}

napi_value CreateJsFloatViewLimitsObject(napi_env env, const FloatViewLimits& limits)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to get object");
        return nullptr;
    }

    napi_value minSize = nullptr;
    napi_create_object(env, &minSize);
    if (minSize == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, minSize, "width", CreateJsValue(env, limits.minWidth_));
    napi_set_named_property(env, minSize, "height", CreateJsValue(env, limits.minHeight_));

    napi_value maxSize = nullptr;
    napi_create_object(env, &maxSize);
    if (maxSize == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, maxSize, "width", CreateJsValue(env, limits.maxWidth_));
    napi_set_named_property(env, maxSize, "height", CreateJsValue(env, limits.maxHeight_));

    napi_set_named_property(env, objValue, "minSize", minSize);
    napi_set_named_property(env, objValue, "maxSize", maxSize);

    napi_value ratioLimit = nullptr;
    napi_create_array_with_length(env, limits.ratioLimits_.size(), &ratioLimit);
    if (ratioLimit == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "ratioLimit is null");
        return nullptr;
    }
    for (size_t i = 0; i < limits.ratioLimits_.size(); i++) {
        napi_value ratioObj = nullptr;
        napi_create_object(env, &ratioObj);
        if (ratioObj == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to get ratioObj");
            return nullptr;
        }
        napi_set_named_property(env, ratioObj, "minRatio", CreateJsValue(env, limits.ratioLimits_[i].first));
        napi_set_named_property(env, ratioObj, "maxRatio", CreateJsValue(env, limits.ratioLimits_[i].second));
        napi_set_element(env, ratioLimit, i, ratioObj);
    }
    napi_set_named_property(env, objValue, "ratioLimits", ratioLimit);
    return objValue;
}
} // namespace Rosen
} // namespace OHOS
