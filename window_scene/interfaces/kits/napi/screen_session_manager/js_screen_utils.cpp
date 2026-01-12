/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_screen_utils.h"

#include <js_runtime_utils.h>

#include "dm_common.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
using namespace AbilityRuntime;

napi_value NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

bool NapiIsCallable(napi_env env, napi_value value)
{
    bool result = false;
    napi_is_callable(env, value, &result);
    return result;
}

napi_valuetype GetType(napi_env env, napi_value value)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, value, &res);
    return res;
}

napi_value JsScreenUtils::CreateJsScreenProperty(napi_env env, const ScreenProperty& screenProperty)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "propertyChangeReason",
        CreateJsValue(env, static_cast<uint32_t>(screenProperty.GetPropertyChangeReason())));
    napi_set_named_property(env, objValue, "rotation", CreateJsValue(env, screenProperty.GetRotation()));
    napi_set_named_property(env, objValue, "bounds", CreateJsRRect(env, screenProperty.GetBounds()));
    napi_set_named_property(env, objValue, "fakeBounds", CreateJsRRect(env, screenProperty.GetFakeBounds()));
    napi_set_named_property(env, objValue, "isFakeInUse", CreateJsValue(env, screenProperty.GetIsFakeInUse()));
    napi_set_named_property(env, objValue, "accessTranslateX", CreateJsValue(env, screenProperty.GetTranslateX()));
    napi_set_named_property(env, objValue, "accessTranslateY", CreateJsValue(env, screenProperty.GetTranslateY()));
    napi_set_named_property(env, objValue, "scaleX", CreateJsValue(env, screenProperty.GetScaleX()));
    napi_set_named_property(env, objValue, "scaleY", CreateJsValue(env, screenProperty.GetScaleY()));
    napi_set_named_property(env, objValue, "rsId", CreateJsValue(env, static_cast<int64_t>(screenProperty.GetRsId())));
    return objValue;
}

napi_value JsScreenUtils::CreateJsRRect(napi_env env, const RRect& rrect)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "left", CreateJsValue(env, rrect.rect_.left_));
    napi_set_named_property(env, objValue, "top", CreateJsValue(env, rrect.rect_.top_));
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, rrect.rect_.width_));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, rrect.rect_.height_));
    napi_set_named_property(env, objValue, "radius", CreateJsValue(env, rrect.radius_[0].x_));
    return objValue;
}

napi_value JsScreenUtils::CreateJsScreenConnectChangeType(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "CONNECT", CreateJsValue(env, 0));
    napi_set_named_property(env, objValue, "DISCONNECT", CreateJsValue(env, 1));
    return objValue;
}

napi_value JsScreenUtils::CreateJsScreenPropertyChangeReason(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "UNDEFINED", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::UNDEFINED)));
    napi_set_named_property(env, objValue, "ROTATION", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::ROTATION)));
    napi_set_named_property(env, objValue, "CHANGE_MODE", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::CHANGE_MODE)));
    napi_set_named_property(env, objValue, "FOLD_SCREEN_EXPAND", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::FOLD_SCREEN_EXPAND)));
    napi_set_named_property(env, objValue, "SCREEN_CONNECT", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::SCREEN_CONNECT)));
    napi_set_named_property(env, objValue, "SCREEN_DISCONNECT", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::SCREEN_DISCONNECT)));
    napi_set_named_property(env, objValue, "FOLD_SCREEN_FOLDING", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING)));
    napi_set_named_property(env, objValue, "VIRTUAL_SCREEN_RESIZE", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::VIRTUAL_SCREEN_RESIZE)));
    napi_set_named_property(env, objValue, "RELATIVE_POSITION_CHANGE", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE)));
    napi_set_named_property(env, objValue, "SUPER_FOLD_STATUS_CHANGE", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::SUPER_FOLD_STATUS_CHANGE)));
    napi_set_named_property(env, objValue, "ACCESS_INFO_CHANGE", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::ACCESS_INFO_CHANGE)));
    napi_set_named_property(env, objValue, "VIRTUAL_PIXEL_RATIO_CHANGE", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::VIRTUAL_PIXEL_RATIO_CHANGE)));
    napi_set_named_property(env, objValue, "FOLD_SCREEN_EXPAND_SWITCH_USER", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::FOLD_SCREEN_EXPAND_SWITCH_USER)));
    napi_set_named_property(env, objValue, "FOLD_SCREEN_FOLDING_SWITCH_USER", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING_SWITCH_USER)));
    return objValue;
}

napi_value JsScreenUtils::CreateJsFoldStatus(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "FOLD_STATUS_UNKNOWN", CreateJsValue(env,
        static_cast<int32_t>(FoldStatus::UNKNOWN)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_EXPANDED", CreateJsValue(env,
        static_cast<int32_t>(FoldStatus::EXPAND)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_FOLDED", CreateJsValue(env,
        static_cast<int32_t>(FoldStatus::FOLDED)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_HALF_FOLDED", CreateJsValue(env,
        static_cast<int32_t>(FoldStatus::HALF_FOLD)));
    return objValue;
}

napi_value JsScreenUtils::CreateJsScreenPropertyChangeType(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "UNSPECIFIED", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeType::UNSPECIFIED)));
    napi_set_named_property(env, objValue, "ROTATION_BEGIN", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeType::ROTATION_BEGIN)));
    napi_set_named_property(env, objValue, "ROTATION_END", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeType::ROTATION_END)));
    napi_set_named_property(env, objValue, "ROTATION_UPDATE_PROPERTY_ONLY", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY)));
    napi_set_named_property(env, objValue, "ROTATION_UPDATE_PROPERTY_ONLY_NOT_NOTIFY", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY_NOT_NOTIFY)));
    napi_set_named_property(env, objValue, "SINGLE_HAND_SWITCH", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeType::SINGLE_HAND_SWITCH)));
    napi_set_named_property(env, objValue, "UNDEFINED", CreateJsValue(env,
        static_cast<int32_t>(ScreenPropertyChangeType::UNDEFINED)));
    return objValue;
}

napi_value JsScreenUtils::CreateJsSuperFoldStatus(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "SUPER_FOLD_STATUS_UNKNOWN", CreateJsValue(env,
        static_cast<int32_t>(SuperFoldStatus::UNKNOWN)));
    napi_set_named_property(env, objValue, "SUPER_FOLD_STATUS_EXPANDED", CreateJsValue(env,
        static_cast<int32_t>(SuperFoldStatus::EXPANDED)));
    napi_set_named_property(env, objValue, "SUPER_FOLD_STATUS_FOLDED", CreateJsValue(env,
        static_cast<int32_t>(SuperFoldStatus::FOLDED)));
    napi_set_named_property(env, objValue, "SUPER_FOLD_STATUS_HALF_FOLDED", CreateJsValue(env,
        static_cast<int32_t>(SuperFoldStatus::HALF_FOLDED)));
    napi_set_named_property(env, objValue, "SUPER_FOLD_STATUS_KEYBOARD", CreateJsValue(env,
        static_cast<int32_t>(SuperFoldStatus::KEYBOARD)));
    return objValue;
}

napi_value JsScreenUtils::CreateJsExtendScreenConnectStatus(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "UNKNOWN", CreateJsValue(env,
        static_cast<int32_t>(ExtendScreenConnectStatus::UNKNOWN)));
    napi_set_named_property(env, objValue, "CONNECT", CreateJsValue(env,
        static_cast<int32_t>(ExtendScreenConnectStatus::CONNECT)));
    napi_set_named_property(env, objValue, "DISCONNECT", CreateJsValue(env,
        static_cast<int32_t>(ExtendScreenConnectStatus::DISCONNECT)));
    return objValue;
}

napi_value JsScreenUtils::CreateJsScreenModeChangeEvent(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "SCREEN_MODE_CHANGE_EVENT_UNKNOWN", CreateJsValue(env,
        static_cast<int32_t>(ScreenModeChangeEvent::UNKNOWN)));
    napi_set_named_property(env, objValue, "SCREEN_MODE_CHANGE_EVENT_BEGIN", CreateJsValue(env,
        static_cast<int32_t>(ScreenModeChangeEvent::BEGIN)));
    napi_set_named_property(env, objValue, "SCREEN_MODE_CHANGE_EVENT_END", CreateJsValue(env,
        static_cast<int32_t>(ScreenModeChangeEvent::END)));
    return objValue;
}

napi_value JsScreenUtils::CreateJsFoldDisplayMode(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create object!");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "UNKNOWN", CreateJsValue(env,
        static_cast<int32_t>(FoldDisplayMode::UNKNOWN)));
    napi_set_named_property(env, objValue, "FULL", CreateJsValue(env,
        static_cast<int32_t>(FoldDisplayMode::FULL)));
    napi_set_named_property(env, objValue, "MAIN", CreateJsValue(env,
        static_cast<int32_t>(FoldDisplayMode::MAIN)));
    napi_set_named_property(env, objValue, "SUB", CreateJsValue(env,
        static_cast<int32_t>(FoldDisplayMode::SUB)));
    napi_set_named_property(env, objValue, "COORDINATION", CreateJsValue(env,
        static_cast<int32_t>(FoldDisplayMode::COORDINATION)));
    napi_set_named_property(env, objValue, "GLOBAL_FULL", CreateJsValue(env,
        static_cast<int32_t>(FoldDisplayMode::GLOBAL_FULL)));
    return objValue;
}

bool ConvertRRectFromJs(napi_env env, napi_value jsObject, RRect& bound)
{
    napi_value jsLeft = nullptr, jsTop = nullptr, jsWidth = nullptr, jsHeight = nullptr, jsRadius = nullptr;
    napi_get_named_property(env, jsObject, "left", &jsLeft);
    napi_get_named_property(env, jsObject, "top", &jsTop);
    napi_get_named_property(env, jsObject, "width", &jsWidth);
    napi_get_named_property(env, jsObject, "height", &jsHeight);
    napi_get_named_property(env, jsObject, "radius", &jsRadius);

    if (GetType(env, jsLeft) != napi_undefined) {
        int32_t left;
        if (!ConvertFromJsValue(env, jsLeft, left)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to left");
            return false;
        }
        bound.rect_.left_ = left;
    }
    if (GetType(env, jsTop) != napi_undefined) {
        int32_t top;
        if (!ConvertFromJsValue(env, jsTop, top)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to top");
            return false;
        }
        bound.rect_.top_ = top;
    }
    if (GetType(env, jsWidth) != napi_undefined) {
        int32_t width;
        if (!ConvertFromJsValue(env, jsWidth, width)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to width");
            return false;
        }
        bound.rect_.width_ = width;
    }
    if (GetType(env, jsHeight) != napi_undefined) {
        int32_t height;
        if (!ConvertFromJsValue(env, jsHeight, height)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to height");
            return false;
        }
        bound.rect_.height_ = height;
    }
    if (GetType(env, jsRadius) != napi_undefined) {
        int radius;
        if (!ConvertFromJsValue(env, jsRadius, radius)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to radius");
            return false;
        }
        bound.radius_[0].x_ = static_cast<float>(radius);
    }
    return true;
}

bool ConvertScreenDirectionInfoFromJs(napi_env env, napi_value jsObject, ScreenDirectionInfo& directionInfo)
{
    napi_value jsNotifyRotation = nullptr;
    napi_value jsScreenRotation = nullptr;
    napi_value jsRotation = nullptr;
    napi_value jsPhyRotation = nullptr;
    napi_get_named_property(env, jsObject, "notifyRotation", &jsNotifyRotation);
    napi_get_named_property(env, jsObject, "screenRotation", &jsScreenRotation);
    napi_get_named_property(env, jsObject, "rotation", &jsRotation);
    napi_get_named_property(env, jsObject, "phyRotation", &jsPhyRotation);

    if (GetType(env, jsNotifyRotation) != napi_undefined) {
        int32_t notifyRotation;
        if (!ConvertFromJsValue(env, jsNotifyRotation, notifyRotation)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to notifyRotation");
            return false;
        }
        directionInfo.notifyRotation_ = notifyRotation;
    }
    if (GetType(env, jsScreenRotation) != napi_undefined) {
        int32_t screenRotation;
        if (!ConvertFromJsValue(env, jsScreenRotation, screenRotation)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to screenRotation");
            return false;
        }
        directionInfo.screenRotation_ = screenRotation;
    }
    if (GetType(env, jsRotation) != napi_undefined) {
        int32_t rotation;
        if (!ConvertFromJsValue(env, jsRotation, rotation)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to rotation");
            return false;
        }
        directionInfo.rotation_ = rotation;
    }
    if (GetType(env, jsPhyRotation) != napi_undefined) {
        int32_t phyRotation;
        if (!ConvertFromJsValue(env, jsPhyRotation, phyRotation)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to phyRotation");
            return false;
        }
        directionInfo.phyRotation_ = phyRotation;
    }
    return true;
}

bool ConvertDMRectFromJs(napi_env env, napi_value jsObject, DMRect& rect)
{
    napi_value jsPosX = nullptr, jsPosY = nullptr, jsWidth = nullptr, jsHeight = nullptr;
    napi_get_named_property(env, jsObject, "posX", &jsPosX);
    napi_get_named_property(env, jsObject, "posY", &jsPosY);
    napi_get_named_property(env, jsObject, "width", &jsWidth);
    napi_get_named_property(env, jsObject, "height", &jsHeight);

    if (GetType(env, jsPosX) != napi_undefined) {
        int32_t posX;
        if (!ConvertFromJsValue(env, jsPosX, posX)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to posX");
            return false;
        }
        rect.posX_ = posX;
    }
    if (GetType(env, jsPosY) != napi_undefined) {
        int32_t top;
        if (!ConvertFromJsValue(env, jsPosY, top)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to posY");
            return false;
        }
        rect.posY_ = top;
    }
    if (GetType(env, jsWidth) != napi_undefined) {
        int32_t width;
        if (!ConvertFromJsValue(env, jsWidth, width)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to width");
            return false;
        }
        rect.width_ = static_cast<uint32_t>(width);
    }
    if (GetType(env, jsHeight) != napi_undefined) {
        int32_t height;
        if (!ConvertFromJsValue(env, jsHeight, height)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to height");
            return false;
        }
        rect.height_ = static_cast<uint32_t>(height);
    }
    return true;
}
} // namespace OHOS::Rosen
