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

#include "js_window_animation_utils.h"

#include <accesstoken_kit.h>
#include <ipc_skeleton.h>
#include <tokenid_kit.h>
#include "window_manager_hilog.h"
namespace {
    #define NAPI_CALL_NO_THROW(theCall, retVal)      \
    do {                                         \
        if ((theCall) != napi_ok) {              \
            return retVal;                       \
        }                                        \
    } while (0)

template <typename T>
napi_value CreateJsNumber(napi_env env, T value)
{
    napi_value result = nullptr;
    if constexpr (std::is_same_v<T, int32_t>) {
        napi_create_int32(env, value, &result);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        napi_create_uint32(env, value, &result);
    } else if constexpr (std::is_same_v<T, int64_t>) {
        napi_create_int64(env, value, &result);
    } else if constexpr (std::is_same_v<T, double>) {
        napi_create_double(env, value, &result);
    } else if constexpr (std::is_same_v<T, float>) {
        napi_create_double(env, static_cast<double>(value), &result);
    } else {
        static_assert(!std::is_same_v<T, T>, "not support type");
    }
    return result;
}

template<class T>
napi_value CreateJsValue(napi_env env, const T& value)
{
    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
    napi_value result = nullptr;
    if constexpr (std::is_same_v<ValueType, bool>) {
        napi_get_boolean(env, value, &result);
        return result;
    } else if constexpr (std::is_arithmetic_v<ValueType>) {
        return CreateJsNumber(env, value);
    } else if constexpr (std::is_same_v<ValueType, std::string>) {
        napi_create_string_utf8(env, value.c_str(), value.length(), &result);
        return result;
    } else if constexpr (std::is_enum_v<ValueType>) {
        return CreateJsNumber(env, static_cast<std::make_signed_t<std::underlying_type_t<ValueType>>>(value));
    } else if constexpr (std::is_same_v<ValueType, const char*>) {
        (value != nullptr) ? napi_create_string_utf8(env, value, strlen(value), &result) :
            napi_get_undefined(env, &result);
        return result;
    } else {
        static_assert(!std::is_same_v<T, T>, "not support type");
    }
}

template <typename T>
bool ConvertFromJsNumber(napi_env env, napi_value jsValue, T& outValue)
{
    if constexpr (std::is_same_v<T, int32_t>) {
        NAPI_CALL_NO_THROW(napi_get_value_int32(env, jsValue, &outValue), false);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        NAPI_CALL_NO_THROW(napi_get_value_uint32(env, jsValue, &outValue), false);
    } else if constexpr (std::is_same_v<T, int64_t>) {
        NAPI_CALL_NO_THROW(napi_get_value_int64(env, jsValue, &outValue), false);
    } else if constexpr (std::is_same_v<T, double>) {
        NAPI_CALL_NO_THROW(napi_get_value_double(env, jsValue, &outValue), false);
    } else {
        static_assert(!std::is_same_v<T, T>, "not support type");
    }
    return true;
}

template<class T>
bool ConvertFromJsValue(napi_env env, napi_value jsValue, T& value)
{
    if (jsValue == nullptr) {
        return false;
    }

    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_same_v<ValueType, bool>) {
        NAPI_CALL_NO_THROW(napi_get_value_bool(env, jsValue, &value), false);
        return true;
    } else if constexpr (std::is_arithmetic_v<ValueType>) {
        return ConvertFromJsNumber(env, jsValue, value);
    } else if constexpr (std::is_same_v<ValueType, std::string>) {
        size_t len = 0;
        NAPI_CALL_NO_THROW(napi_get_value_string_utf8(env, jsValue, nullptr, 0, &len), false);
        auto buffer = std::make_unique<char[]>(len + 1);
        size_t strLength = 0;
        NAPI_CALL_NO_THROW(napi_get_value_string_utf8(env, jsValue, buffer.get(), len + 1, &strLength), false);
        value = std::string(buffer.get(), strLength);
        return true;
    } else if constexpr (std::is_enum_v<ValueType>) {
        std::make_signed_t<ValueType> numberValue = 0;
        if (!ConvertFromJsNumber(env, jsValue, numberValue)) {
            return false;
        }
        value = static_cast<ValueType>(numberValue);
        return true;
    } else {
        static_assert(!std::is_same_v<T, T>, "not support type");
    }
}

template<class T>
bool ParseJsValue(napi_value jsObject, napi_env env, const std::string& name, T& data)
{
    napi_value value = nullptr;
    napi_status status = napi_get_named_property(env, jsObject, name.c_str(), &value);
    if (status != napi_ok) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    napi_typeof(env, value, &type);
    if (type != napi_undefined) {
        if (!ConvertFromJsValue(env, value, data)) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}
}
namespace OHOS {
namespace Rosen {
bool IsSystemCalling()
{
    uint64_t accessTokenID = IPCSkeleton::GetCallingFullTokenID();
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(accessTokenID);
}

napi_value ConvertTransitionAnimationToJsValue(napi_env env, std::shared_ptr<TransitionAnimation> transitionAnimation)
{
    napi_value objValue = nullptr;
    if (!transitionAnimation) {
        return objValue;
    }
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_value configJsValue = ConvertWindowAnimationOptionToJsValue(env, transitionAnimation->config);
    if (!configJsValue) {
        return nullptr;
    }
    napi_set_named_property(env, objValue, "config", configJsValue);
    napi_set_named_property(env, objValue, "opacity", CreateJsValue(env, transitionAnimation->opacity));

    return objValue;
}

napi_value ConvertStartAnimationOptionsToJsValue(napi_env env,
    std::shared_ptr<StartAnimationOptions> startAnimationOptions)
{
    napi_value objValue = nullptr;
    if (!startAnimationOptions) {
        return objValue;
    }
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "type",
        CreateJsValue(env, startAnimationOptions->animationType)),
        "ConvertStartAnimationOptionsToJsValue failed");
    return objValue;
}

napi_value ConvertStartAnimationSystemOptionsToJsValue(napi_env env,
    std::shared_ptr<StartAnimationSystemOptions> startAnimationSystemOptions)
{
    napi_value objValue = nullptr;
    if (!startAnimationSystemOptions) {
        return objValue;
    }
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "type",
        CreateJsValue(env, startAnimationSystemOptions->animationType)),
        "ConvertStartAnimationSystemOptionsToJsValue failed");
    if (startAnimationSystemOptions->animationConfig != nullptr) {
        napi_value configJsValue = ConvertWindowAnimationOptionToJsValue(env,
            *(startAnimationSystemOptions->animationConfig));
        if (configJsValue != nullptr) {
            NAPI_CHECK_RETURN_IF_NULL(napi_set_named_property(env, objValue, "animationConfig", configJsValue),
                "Set animationConfig failed");
        } else {
            TLOGE(WmsLogTag::WMS_ANIMATION, "ConvertWindowAnimationOptionToJsValue failed");
        }
    }
    return objValue;
}

napi_value ConvertWindowAnimationOptionToJsValue(napi_env env,
    const WindowAnimationOption& animationConfig)
{
    napi_value configJsValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, configJsValue);
    napi_set_named_property(env, configJsValue, "curve", CreateJsValue(env, animationConfig.curve));
    switch (animationConfig.curve) {
        case WindowAnimationCurve::LINEAR: {
            napi_set_named_property(env, configJsValue, "duration", CreateJsValue(env, animationConfig.duration));
            break;
        }
        case WindowAnimationCurve::CUBIC_BEZIER: {
            napi_set_named_property(env, configJsValue, "duration", CreateJsValue(env, animationConfig.duration));
            [[fallthrough]];
        }
        case WindowAnimationCurve::INTERPOLATION_SPRING: {
            napi_value params = nullptr;
            napi_create_array(env, &params);
            for (uint32_t i = 0; i < ANIMATION_PARAM_SIZE; ++i) {
                napi_value element;
                napi_create_double(env, static_cast<double>(animationConfig.param[i]), &element);
                napi_set_element(env, params, i, element);
            }
            napi_set_named_property(env, configJsValue, "param", params);
            break;
        }
        default:
            break;
    }
    return configJsValue;
}

bool ConvertTransitionAnimationFromJsValue(napi_env env, napi_value jsObject, TransitionAnimation& transitionAnimation,
    WmErrorCode& result)
{
    napi_value jsAnimationConfig = nullptr;
    napi_get_named_property(env, jsObject, "config", &jsAnimationConfig);
    if (!ConvertWindowAnimationOptionFromJsValue(env, jsAnimationConfig, transitionAnimation.config, result) ||
        !CheckWindowAnimationOption(env, transitionAnimation.config, result)) {
        return false;
    }
    double opacity = 1.0f;
    napi_value jsOpacityValue = nullptr;
    napi_get_named_property(env, jsObject, "opacity", &jsOpacityValue);
    napi_valuetype type = napi_undefined;
    napi_typeof(env, jsOpacityValue, &type);
    if (type != napi_undefined) {
        if (!ConvertFromJsValue(env, jsOpacityValue, opacity)) {
            result = WmErrorCode::WM_ERROR_INVALID_PARAM;
            return false;
        } else if (opacity < 0.0 || opacity > 1.0) {
            result = WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
            return false;
        }
    }
    transitionAnimation.opacity = static_cast<float>(opacity);
    return true;
}

bool ConvertStartAnimationOptionsFromJsValue(napi_env env, napi_value jsObject,
    StartAnimationOptions& startAnimationOptions)
{
    uint32_t animationType = 0;
    if (!ParseJsValue(jsObject, env, "type", animationType)) {
        return false;
    }
    if (animationType >= static_cast<uint32_t>(AnimationType::END)) {
        return false;
    }
    startAnimationOptions.animationType = static_cast<AnimationType>(animationType);
    return true;
}

bool ConvertStartAnimationSystemOptionsFromJsValue(napi_env env, napi_value jsObject,
    StartAnimationSystemOptions& startAnimationSystemOptions)
{
    WmErrorCode result = WmErrorCode::WM_OK;
    uint32_t animationType = 0;
    if (!ParseJsValue(jsObject, env, "type", animationType)) {
        return false;
    }
    if (animationType >= static_cast<uint32_t>(AnimationType::END)) {
        return false;
    }
    startAnimationSystemOptions.animationType = static_cast<AnimationType>(animationType);
    bool hasAnimationConfig = false;
    napi_has_named_property(env, jsObject, "animationConfig", &hasAnimationConfig);
    if (!hasAnimationConfig) {
        return true;
    }
    napi_value jsAnimationConfig = nullptr;
    napi_get_named_property(env, jsObject, "animationConfig", &jsAnimationConfig);
    startAnimationSystemOptions.animationConfig = std::make_shared<WindowAnimationOption>();
    if (!ConvertWindowAnimationOptionFromJsValue(env, jsAnimationConfig, *(startAnimationSystemOptions.animationConfig),
        result) || !CheckWindowAnimationOption(env, *(startAnimationSystemOptions.animationConfig), result)) {
        startAnimationSystemOptions.animationConfig = nullptr;
    }
    return true;
}

bool ConvertWindowCreateParamsFromJsValue(napi_env env, napi_value jsObject,
    WindowCreateParams& windowCreateParams)
{
    bool hasAnimationParams = false;
    napi_has_named_property(env, jsObject, "animationParams", &hasAnimationParams);
    if (hasAnimationParams) {
        napi_value jsAnimationParams = nullptr;
        napi_get_named_property(env, jsObject, "animationParams", &jsAnimationParams);
        windowCreateParams.animationParams = std::make_shared<StartAnimationOptions>();
        if (!ConvertStartAnimationOptionsFromJsValue(env, jsAnimationParams, *(windowCreateParams.animationParams))) {
            windowCreateParams.animationParams = nullptr;
        }
    }
    bool hasAnimationSystemParams = false;
    napi_has_named_property(env, jsObject, "systemAnimationParams", &hasAnimationSystemParams);
    if (hasAnimationSystemParams && IsSystemCalling()) {
        napi_value jsAnimationSystemParams = nullptr;
        napi_get_named_property(env, jsObject, "systemAnimationParams", &jsAnimationSystemParams);
        windowCreateParams.animationSystemParams = std::make_shared<StartAnimationSystemOptions>();
        if (!ConvertStartAnimationSystemOptionsFromJsValue(env, jsAnimationSystemParams,
            *(windowCreateParams.animationSystemParams))) {
            windowCreateParams.animationSystemParams = nullptr;
        }
    }
    bool hasNeedAnimation = false;
    napi_has_named_property(env, jsObject, "needAnimation", &hasNeedAnimation);
    if (hasNeedAnimation) {
        napi_value jsNeedAnimation = nullptr;
        napi_get_named_property(env, jsObject, "needAnimation", &jsNeedAnimation);
        napi_valuetype type = napi_undefined;
        napi_typeof(env, jsNeedAnimation, &type);
        if (type == napi_boolean) {
            bool tempVal = false;
            if (napi_get_value_bool(env, jsNeedAnimation, &tempVal) == napi_ok) {
                windowCreateParams.needAnimation = std::make_shared<bool>(tempVal);
            }
        }
    } else {
        windowCreateParams.needAnimation = nullptr;
    }
    return true;
}

bool CheckWindowAnimationOption(napi_env env, WindowAnimationOption& animationConfig, WmErrorCode& result)
{
    switch (animationConfig.curve) {
        case WindowAnimationCurve::LINEAR: {
            if (animationConfig.duration > ANIMATION_MAX_DURATION) {
                TLOGE(WmsLogTag::WMS_ANIMATION, "Duration is invalid: %{public}u", animationConfig.duration);
                result = WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
                return false;
            }
            break;
        }
        case WindowAnimationCurve::INTERPOLATION_SPRING: {
            for (uint32_t i = 1; i < ANIMATION_PARAM_SIZE; ++i) {
                if (!GreatNotEqual(animationConfig.param[i], 0.0f)) {
                    TLOGI(WmsLogTag::WMS_ANIMATION, "Interpolation spring param %{public}u is invalid: %{public}f",
                        i, animationConfig.param[i]);
                    animationConfig.param[i] = 1.0f;
                }
            }
            break;
        }
        case WindowAnimationCurve::CUBIC_BEZIER: {
            if (animationConfig.duration > ANIMATION_MAX_DURATION) {
                TLOGE(WmsLogTag::WMS_ANIMATION, "Duration is invalid: %{public}u", animationConfig.duration);
                result = WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
                return false;
            }
            break;
        }
        default:
            result = WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
            return false;
    }
    return true;
}

bool ConvertWindowAnimationOptionFromJsValue(napi_env env, napi_value jsAnimationConfig,
    WindowAnimationOption& animationConfig, WmErrorCode& result)
{
    if (jsAnimationConfig == nullptr) {
        result = WmErrorCode::WM_ERROR_INVALID_PARAM;
        return false;
    }
    uint32_t curve = 0;
    if (!ParseJsValue(jsAnimationConfig, env, "curve", curve)) {
        result = WmErrorCode::WM_ERROR_INVALID_PARAM;
        return false;
    }
    animationConfig.curve = static_cast<WindowAnimationCurve>(curve);
    uint32_t duration = 0;
    std::array<double, ANIMATION_PARAM_SIZE> params;
    switch (curve) {
        case static_cast<uint32_t>(WindowAnimationCurve::LINEAR): {
            if (!ParseJsValue(jsAnimationConfig, env, "duration", duration)) {
                result = WmErrorCode::WM_ERROR_INVALID_PARAM;
                return false;
            }
            animationConfig.duration = duration;
            break;
        }
        case static_cast<uint32_t>(WindowAnimationCurve::CUBIC_BEZIER): {
            if (!ParseJsValue(jsAnimationConfig, env, "duration", duration)) {
                result = WmErrorCode::WM_ERROR_INVALID_PARAM;
                return false;
            }
            animationConfig.duration = duration;
            [[fallthrough]];
        }
        case static_cast<uint32_t>(WindowAnimationCurve::INTERPOLATION_SPRING): {
            napi_value paramsValue = nullptr;
            napi_get_named_property(env, jsAnimationConfig, "param", &paramsValue);
            for (uint32_t i = 0; i < ANIMATION_PARAM_SIZE; ++i) {
                napi_value element;
                napi_get_element(env, paramsValue, i, &element);
                if (!ConvertFromJsValue(env, element, params[i])) {
                    result = WmErrorCode::WM_ERROR_INVALID_PARAM;
                    return false;
                }
                animationConfig.param[i] = static_cast<float>(params[i]);
            }
            break;
        }
        default:
            result = WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
            return false;
    }
    return true;
}
} // namespace Rosen
} // namespace OHOS
