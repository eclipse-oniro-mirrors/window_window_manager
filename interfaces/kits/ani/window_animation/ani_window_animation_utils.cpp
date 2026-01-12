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

#include "ani_window_animation_utils.h"

#include <accesstoken_kit.h>
#include <ipc_skeleton.h>
#include <tokenid_kit.h>
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
#define CHECK_RET_TO_RETURN_UNDEFINED(ret, returnValue, logStr) \
do {                                                            \
    if ((ret) != ANI_OK) {                                      \
        TLOGE(WmsLogTag::WMS_ANIMATION, logStr);                \
        return returnValue;                                     \
    }                                                           \
} while (0)

#define CHECK_RET_RETURN_NULLPTR(ret, logStr)                           \
do {                                                                    \
    if ((ret) != ANI_OK) {                                              \
        TLOGE(WmsLogTag::WMS_ANIMATION, logStr" ret: %{public}d", ret); \
        return nullptr;                                                 \
    }                                                                   \
} while (0)

#define CHECK_NULL_VALUE_RETURN_NULLPTR(aniObj, logStr) \
do {                                                    \
    if ((aniObj) == nullptr) {                          \
        TLOGE(WmsLogTag::WMS_ANIMATION, logStr);        \
        return nullptr;                                 \
    }                                                   \
} while (0)

#define CHECK_NULL_VALUE_RETURN_FALSE(aniObj, logStr)   \
do {                                                    \
    if ((aniObj) == nullptr) {                          \
        TLOGE(WmsLogTag::WMS_ANIMATION, logStr);        \
        return false;                                   \
    }                                                   \
} while (0)

#define CHECK_RET_RETURN_FALSE(ret, logStr)                             \
do {                                                                    \
    if ((ret) != ANI_OK) {                                              \
        TLOGE(WmsLogTag::WMS_ANIMATION, logStr" ret: %{public}d", ret); \
        return false;                                                   \
    }                                                                   \
} while (0)

ani_status CallAniMethodVoid(ani_env* env, ani_object object, ani_class cls,
    const char* method, const char* signature, ...)
{
    ani_method aniMethod;
    ani_status ret = env->Class_FindMethod(cls, method, signature, &aniMethod);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] method:%{public}s not found", method);
        return ret;
    }
    va_list args;
    va_start(args, signature);
    ret = env->Object_CallMethod_Void_V(object, aniMethod, args);
    va_end(args);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] fail to call method:%{public}s", method);
    }
    return ret;
}

ani_object CreateAniUndefined(ani_env* env)
{
    ani_ref aniRef;
    env->GetUndefined(&aniRef);
    return static_cast<ani_object>(aniRef);
}

std::string GetAnimationCurveItemName(WindowAnimationCurve curve)
{
    std::string name = "LINEAR";
    switch (curve) {
        case WindowAnimationCurve::LINEAR: {
            break;
        }
        case WindowAnimationCurve::INTERPOLATION_SPRING: {
            name = "INTERPOLATION_SPRING";
            break;
        }
        case WindowAnimationCurve::CUBIC_BEZIER: {
            name = "CUBIC_BEZIER";
            break;
        }
        default:
            break;
    }
    return name;
}

std::string GetAnimationTypeItemName(AnimationType type)
{
    std::string name = "END";
    switch (type) {
        case AnimationType::FADE_IN_OUT: {
            name = "FADE_IN_OUT";
            break;
        }
        case AnimationType::FADE_IN: {
            name = "FADE_IN";
            break;
        }
        case AnimationType::SEE_THE_WORLD: {
            name = "SEE_THE_WORLD";
            break;
        }
        default:
            break;
    }
    return name;
}

bool ParseDurationValue(ani_env* env, ani_object aniObject, ani_long& aniDuration)
{
    ani_ref durationRef = nullptr;
    ani_status ret = env->Object_GetPropertyByName_Ref(aniObject, "duration", &durationRef);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get duration value failed. %{public}d", ret);
        return false;
    }

    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(durationRef, &isUndefined);
    if (isUndefined) {
        TLOGW(WmsLogTag::WMS_ANIMATION, "[ANI] Duration is undefined.");
        return false;
    }
    ret = env->Object_CallMethodByName_Long(static_cast<ani_object>(durationRef), "toLong", ":l", &aniDuration);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] toLong duration value failed. %{public}d", ret);
        return false;
    }
    return true;
}

ani_ref CreateDouble(ani_env* env, double value)
{
    static constexpr const char* className = "std.core.Double";
    ani_class doubleCls {};
    ani_status ret = env->FindClass(className, &doubleCls);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Find class double failed. %{public}d", ret);
        ani_ref undefinedRef;
        env->GetUndefined(&undefinedRef);
        return undefinedRef;
    }
    ani_method ctor {};
    ret = env->Class_FindMethod(doubleCls, "<ctor>", "d:", &ctor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Find ctor method failed. %{public}d", ret);
        ani_ref undefinedRef;
        env->GetUndefined(&undefinedRef);
        return undefinedRef;
    }
    ani_object obj {};
    if (env->Object_New(doubleCls, ctor, &obj, static_cast<ani_double>(value)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] new double object failed");
        ani_ref undefinedRef;
        env->GetUndefined(&undefinedRef);
        return undefinedRef;
    }
    return obj;
}

bool CheckIsUndefinedAndGetProperty(ani_env* env, ani_object object, const std::string propertyName, ani_ref* result)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] env is null");
        return false;
    }
    ani_status ret = env->Object_GetPropertyByName_Ref(object, propertyName.c_str(), result);
    if (ret == ANI_OK) {
        ani_boolean isUndefined = true;
        env->Reference_IsUndefined(*result, &isUndefined);
        if (!isUndefined) {
            return false;
        }
    }
    TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] %{public}s is undefined", propertyName.c_str());
    return true;
}

template<typename T>
const char* GetClassName()
{
    if (std::is_same<T, int>::value) {
        return "std.core.Int";
    } else if (std::is_same<T, double>::value) {
        return "std.core.Double";
    } else if (std::is_same<T, long>::value) {
        return "std.core.Long";
    } else {
        return nullptr;
    }
}

template<typename T>
const char* GetCtorSignature()
{
    if (std::is_same<T, int>::value) {
        return "i:";
    } else if (std::is_same<T, double>::value) {
        return "d:";
    } else if (std::is_same<T, long>::value) {
        return "l:";
    } else {
        return nullptr;
    }
}

template<typename T>
ani_object CreateBaseTypeObject(ani_env* env, T value)
{
    static const char* className = GetClassName<T>();
    ani_class cls;
    ani_status ret =  env->FindClass(className, &cls);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] class not found for type");
        return CreateAniUndefined(env);
    }

    // Obtains the signature of a constructor based on the type
    const char* signature = GetCtorSignature<T>();
    ani_method ctor;
    ret = env->Class_FindMethod(cls, "<ctor>", signature, &ctor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] ctor not found for type");
        return CreateAniUndefined(env);
    }

    // Convert values based on types and create objects
    ani_object obj;
    if (std::is_same<T, int>::value) {
        ret = env->Object_New(cls, ctor, &obj, ani_int(value));
    } else if (std::is_same<T, double>::value) {
        ret = env->Object_New(cls, ctor, &obj, ani_double(value));
    } else if (std::is_same<T, long>::value) {
        ret = env->Object_New(cls, ctor, &obj, ani_long(value));
    } else {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] unsupported type");
        return CreateAniUndefined(env);
    }
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] failed to create new obj for type");
        return CreateAniUndefined(env);
    }
    return obj;
}
} // namespace

bool IsSystemCalling()
{
    uint64_t accessTokenID = IPCSkeleton::GetCallingFullTokenID();
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(accessTokenID);
}

ani_object ConvertTransitionAnimationToAniValue(ani_env* env, std::shared_ptr<TransitionAnimation> transitionAnimation)
{
    CHECK_NULL_VALUE_RETURN_NULLPTR(env, "env is null");
    if (!transitionAnimation) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] TransitionAnimation is null");
        return nullptr;
    }
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.TransitionAnimationInternal", &aniClass);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Class not found.");
    
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Ctor not found.");
    
    ani_object aniTransitionAnimation = nullptr;
    ret = env->Object_New(aniClass, aniCtor, &aniTransitionAnimation);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Failed to new obj.");

    ani_object windowAnimationOption = ConvertWindowAnimationOptionToAniValue(env, transitionAnimation->config);
    CHECK_NULL_VALUE_RETURN_NULLPTR(windowAnimationOption, "[ANI] Convert window animation config value failed.");

    ret = CallAniMethodVoid(env, aniTransitionAnimation, aniClass, "<set>config", nullptr,
        windowAnimationOption);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Set config failed.");

    ret = CallAniMethodVoid(env, aniTransitionAnimation, aniClass, "<set>opacity", nullptr,
        CreateBaseTypeObject<double>(env, transitionAnimation->opacity));
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Set opacity failed.");

    return aniTransitionAnimation;
}

ani_object ConvertStartAnimationOptionsToAniValue(ani_env* env,
    std::shared_ptr<StartAnimationOptions> startAnimationOptions)
{
    CHECK_NULL_VALUE_RETURN_NULLPTR(env, "[ANI] env is null");

    if (!startAnimationOptions) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] startAnimationOptions is null");
        return nullptr;
    }

    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.StartAnimationParamsInternal", &aniClass);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Class not found.");

    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Ctor not found.");

    ani_object aniStartAnimationOptions = nullptr;
    ret = env->Object_New(aniClass, aniCtor, &aniStartAnimationOptions);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Failed to new obj.");

    ani_enum animationType;
    ret = env->FindEnum("@ohos.window.window.AnimationType", &animationType);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Find enum AnimationType failed.");

    ani_enum_item animationTypeItem;
    std::string itemName = GetAnimationTypeItemName(startAnimationOptions->animationType);
    ret = env->Enum_GetEnumItemByName(animationType, itemName.c_str(), &animationTypeItem);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get enum item %{public}s failed. ret: %{public}d",
            itemName.c_str(), ret);
        return nullptr;
    }

    ret = CallAniMethodVoid(env, aniStartAnimationOptions, aniClass, "<set>type", nullptr, animationTypeItem);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Set type failed.");
    return aniStartAnimationOptions;
}

ani_object ConvertStartAnimationSystemOptionsToAniValue(ani_env* env,
    std::shared_ptr<StartAnimationSystemOptions> startAnimationSystemOptions)
{
    CHECK_NULL_VALUE_RETURN_NULLPTR(env, "[ANI] env is null");

    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.StartAnimationSystemParamsInternal", &aniClass);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Class not found.");

    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Ctor not found.");

    ani_object systemParamsObj = nullptr;
    ret = env->Object_New(aniClass, aniCtor, &systemParamsObj);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Failed to new obj.");

    ani_enum animationType;
    ret = env->FindEnum("@ohos.window.window.AnimationType", &animationType);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Find enum AnimationType failed.");

    ani_enum_item animationTypeItem;
    std::string itemName = GetAnimationTypeItemName(startAnimationSystemOptions->animationType);
    ret = env->Enum_GetEnumItemByName(animationType, itemName.c_str(), &animationTypeItem);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get enum item %{public}s failed. ret: %{public}d",
            itemName.c_str(), ret);
        return nullptr;
    }

    ret = CallAniMethodVoid(env, systemParamsObj, aniClass, "<set>type", nullptr, animationTypeItem);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Set type failed.");

    if (startAnimationSystemOptions->animationConfig != nullptr) {
        ani_object configAniValue = ConvertWindowAnimationOptionToAniValue(env,
            *(startAnimationSystemOptions->animationConfig));
        CHECK_NULL_VALUE_RETURN_NULLPTR(configAniValue, "[ANI] Convert window animation option failed.");

        ret = CallAniMethodVoid(env, systemParamsObj, aniClass, "<set>animationConfig", nullptr,
            configAniValue);
        CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Set animationConfig failed.");
    }
    return systemParamsObj;
}

ani_object ConvertWindowAnimationOptionToAniValue(ani_env* env,
    const WindowAnimationOption& animationConfig)
{
    CHECK_NULL_VALUE_RETURN_NULLPTR(env, "[ANI] env is null");

    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.WindowAnimationConfigInternal", &aniClass);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Class not found.");

    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Ctor not found.");

    ani_object animationConfigObj = nullptr;
    ret = env->Object_New(aniClass, aniCtor, &animationConfigObj);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Failed to new obj.");

    ani_enum aniAnimationCurveType;
    ret = env->FindEnum("@ohos.window.window.WindowAnimationCurve", &aniAnimationCurveType);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Find enum AnimationType failed.");

    ani_enum_item animationCurveTypeItem;
    std::string itemName = GetAnimationCurveItemName(animationConfig.curve);
    ret = env->Enum_GetEnumItemByName(aniAnimationCurveType, itemName.c_str(), &animationCurveTypeItem);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get enum item %{public}s failed. ret: %{public}d",
            itemName.c_str(), ret);
        return nullptr;
    }
    ret = CallAniMethodVoid(env, animationConfigObj, aniClass, "<set>curve", nullptr, animationCurveTypeItem);
    CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Set curve failed.");

    switch (animationConfig.curve) {
        case WindowAnimationCurve::LINEAR: {
            ret = CallAniMethodVoid(env, animationConfigObj, aniClass, "<set>duration", nullptr,
                CreateBaseTypeObject<long>(env, animationConfig.duration));
            CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Set duration failed with LINEAR curve.");
            break;
        }
        case WindowAnimationCurve::CUBIC_BEZIER: {
            ret = CallAniMethodVoid(env, animationConfigObj, aniClass, "<set>duration", nullptr,
                CreateBaseTypeObject<long>(env, animationConfig.duration));
            CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Set duration failed with CUBIC_BEZIER curve.");
            [[fallthrough]];
        }
        case WindowAnimationCurve::INTERPOLATION_SPRING: {
            ani_array params = nullptr;
            if (env->Array_New(ANIMATION_PARAM_SIZE, static_cast<ani_ref>(CreateAniUndefined(env)),
                &params) != ANI_OK) {
                TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] create array failed");
                return nullptr;
            }
            for (uint32_t i = 0; i < ANIMATION_PARAM_SIZE; ++i) {
                if (env->Array_Set(params, i, CreateDouble(env, animationConfig.param[i])) != ANI_OK) {
                    TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] set params failed at %{public}d", i);
                    return nullptr;
                }
            }
            ret = CallAniMethodVoid(env, animationConfigObj, aniClass, "<set>param", nullptr, params);
            CHECK_RET_RETURN_NULLPTR(ret, "[ANI] Set param failed with INTERPOLATION_SPRING curve.");
            break;
        }
        default:
            break;
    }
    return animationConfigObj;
}

bool ConvertTransitionAnimationFromAniValue(ani_env* env, ani_object aniObject,
    TransitionAnimation& transitionAnimation, WmErrorCode& result)
{
    CHECK_NULL_VALUE_RETURN_FALSE(env, "[ANI] env is null.");

    ani_ref aniAnimationConfig = nullptr;
    ani_status ret = env->Object_GetPropertyByName_Ref(aniObject, "config", &aniAnimationConfig);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get property config failed. ret: %{public}d", ret);
        result = WmErrorCode::WM_ERROR_INVALID_PARAM;
        return false;
    }
    if (!ConvertWindowAnimationOptionFromAniValue(env, static_cast<ani_object>(aniAnimationConfig),
        transitionAnimation.config, result) ||
        !CheckWindowAnimationOption(transitionAnimation.config, result)) {
        return false;
    }

    ani_ref aniOpacityObj = nullptr;
    ret = env->Object_GetPropertyByName_Ref(aniObject, "opacity", &aniOpacityObj);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get opacity failed. ret: %{public}d", ret);
        result = WmErrorCode::WM_ERROR_INVALID_PARAM;
        return false;
    }
    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(aniOpacityObj, &isUndefined);
    if (isUndefined) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] AnimationConfig is undefined.");
        return false;
    }

    ani_double aniOpacityValue = 0;
    ret = env->Object_CallMethodByName_Double(static_cast<ani_object>(aniOpacityObj), "toDouble", ":d",
        &aniOpacityValue);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Opacity toDouble failed. ret: %{public}d", ret);
        result = WmErrorCode::WM_ERROR_INVALID_PARAM;
        return false;
    }
    float opacity =  static_cast<float>(aniOpacityValue);
    if (opacity < 0.0 || opacity > 1.0) { // 1.0: max opacity
        result = WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Opacity value is wrong. value: %{public}f", opacity);
        return false;
    }

    transitionAnimation.opacity = opacity;
    return true;
}

bool ConvertStartAnimationOptionsFromAniValue(ani_env* env, ani_object aniObject,
    StartAnimationOptions& startAnimationOptions)
{
    CHECK_NULL_VALUE_RETURN_FALSE(env, "[ANI] env is null.");

    ani_ref aniAnimationEnumItem = nullptr;
    ani_status ret = env->Object_GetPropertyByName_Ref(aniObject, "type", &aniAnimationEnumItem);
    CHECK_RET_RETURN_FALSE(ret, "[ANI] Get property type failed.");

    ani_int enumValue = 0;
    ret = env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(aniAnimationEnumItem), &enumValue);
    CHECK_RET_RETURN_FALSE(ret, "[ANI] Get type value failed.");

    AnimationType animationType = static_cast<AnimationType>(enumValue);
    if (animationType >= AnimationType::END) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] AnimationType is out of range");
        return false;
    }
    startAnimationOptions.animationType = animationType;
    return true;
}

bool ConvertStartAnimationSystemOptionsFromAniValue(ani_env* env, ani_object aniObject,
    StartAnimationSystemOptions& startAnimationSystemOptions)
{
    CHECK_NULL_VALUE_RETURN_FALSE(env, "[ANI] env is null.");

    ani_ref aniAnimationEnumItem = nullptr;
    ani_status ret = env->Object_GetPropertyByName_Ref(aniObject, "type", &aniAnimationEnumItem);
    CHECK_RET_RETURN_FALSE(ret, "[ANI] Get property type failed.");

    ani_int enumValue = 0;
    ret = env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(aniAnimationEnumItem), &enumValue);
    CHECK_RET_RETURN_FALSE(ret, "[ANI] Get type value failed.");

    AnimationType animationType = static_cast<AnimationType>(enumValue);
    if (animationType >= AnimationType::END) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] type value is out of range. %{public}d",
            static_cast<uint32_t>(enumValue));
        return false;
    }
    startAnimationSystemOptions.animationType = animationType;
    ani_ref aniAnimationConfig = nullptr;
    ret = env->Object_GetPropertyByName_Ref(aniObject, "animationConfig", &aniAnimationConfig);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] There is no animationConfig. ret: %{public}d", ret);
        return false;
    }
    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(aniAnimationConfig, &isUndefined);
    if (isUndefined) {
        TLOGW(WmsLogTag::WMS_ANIMATION, "[ANI] AnimationConfig is undefined.");
        return true;
    }
    
    WmErrorCode result = WmErrorCode::WM_OK;
    startAnimationSystemOptions.animationConfig = std::make_shared<WindowAnimationOption>();
    if (!ConvertWindowAnimationOptionFromAniValue(env, static_cast<ani_object>(aniAnimationConfig),
        *(startAnimationSystemOptions.animationConfig), result) ||
        !CheckWindowAnimationOption(*(startAnimationSystemOptions.animationConfig), result)) {
        startAnimationSystemOptions.animationConfig = nullptr;
    }
    return true;
}

bool ConvertWindowCreateParamsFromAniValue(ani_env* env, ani_object aniObject,
    WindowCreateParams& windowCreateParams)
{
    CHECK_NULL_VALUE_RETURN_FALSE(env, "[ANI] env is null.");

    ani_ref aniAnimationParams = nullptr;
    if (!CheckIsUndefinedAndGetProperty(env, aniObject, "animationParams", &aniAnimationParams)) {
        windowCreateParams.animationParams = std::make_shared<StartAnimationOptions>();
        if (!ConvertStartAnimationOptionsFromAniValue(env,
            static_cast<ani_object>(aniAnimationParams), *(windowCreateParams.animationParams))) {
            windowCreateParams.animationParams = nullptr;
        }
    } else {
        TLOGW(WmsLogTag::WMS_ANIMATION, "[ANI] There is no animationParams.");
    }
    
    ani_ref aniAnimationSystemParams = nullptr;
    if (!CheckIsUndefinedAndGetProperty(env, aniObject, "systemAnimationParams", &aniAnimationSystemParams) &&
        IsSystemCalling()) {
        windowCreateParams.animationSystemParams = std::make_shared<StartAnimationSystemOptions>();
        if (!ConvertStartAnimationSystemOptionsFromAniValue(env,
            static_cast<ani_object>(aniAnimationSystemParams), *(windowCreateParams.animationSystemParams))) {
            windowCreateParams.animationSystemParams = nullptr;
        }
    } else {
        TLOGW(WmsLogTag::WMS_ANIMATION, "[ANI] There is no systemAnimationParams.");
    }
    return true;
}

bool CheckWindowAnimationOption(WindowAnimationOption& animationConfig, WmErrorCode& result)
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
                    TLOGW(WmsLogTag::WMS_ANIMATION, "Interpolation spring param %{public}u is invalid: %{public}f",
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

bool ConvertWindowAnimationOptionFromAniValue(ani_env* env, ani_object aniAnimationConfig,
    WindowAnimationOption& animationConfig, WmErrorCode& result)
{
    if (aniAnimationConfig == nullptr || env == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] aniAnimationConfig or env is null");
        result = WmErrorCode::WM_ERROR_INVALID_PARAM;
        return false;
    }

    ani_ref aniCurve = nullptr;
    ani_status ret = env->Object_GetPropertyByName_Ref(aniAnimationConfig, "curve", &aniCurve);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get property type failed. %{public}d", ret);
        result = WmErrorCode::WM_ERROR_INVALID_PARAM;
        return false;
    }

    ani_int enumValue = 0;
    ret = env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(aniCurve), &enumValue);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get type value failed. %{public}d", ret);
        result = WmErrorCode::WM_ERROR_INVALID_PARAM;
        return false;
    }
    animationConfig.curve = static_cast<WindowAnimationCurve>(enumValue);
    switch (animationConfig.curve) {
        case WindowAnimationCurve::LINEAR: {
            ani_long aniDuration = 0;
            if (!ParseDurationValue(env, aniAnimationConfig, aniDuration)) {
                result = WmErrorCode::WM_ERROR_INVALID_PARAM;
                return false;
            }
            animationConfig.duration = static_cast<uint32_t>(aniDuration);
            break;
        }
        case WindowAnimationCurve::CUBIC_BEZIER: {
            ani_long aniDuration = 0;
            if (!ParseDurationValue(env, aniAnimationConfig, aniDuration)) {
                result = WmErrorCode::WM_ERROR_INVALID_PARAM;
                return false;
            }
            animationConfig.duration = static_cast<uint32_t>(aniDuration);
            [[fallthrough]];
        }
        case WindowAnimationCurve::INTERPOLATION_SPRING: {
            ani_ref aniParam = nullptr;
            ani_status ret = env->Object_GetPropertyByName_Ref(aniAnimationConfig, "param", &aniParam);
            if (ret != ANI_OK) {
                result = WmErrorCode::WM_ERROR_INVALID_PARAM;
                TLOGE(WmsLogTag::WMS_ANIMATION, "[ANI] Get param value failed. %{public}d", ret);
                return false;
            }
            ani_boolean isUndefined = true;
            env->Reference_IsUndefined(aniParam, &isUndefined);
            if (isUndefined) {
                TLOGW(WmsLogTag::WMS_ANIMATION, "[ANI] Param is undefined.");
                result = WmErrorCode::WM_ERROR_INVALID_PARAM;
                return false;
            }
            for (uint32_t i = 0; i < ANIMATION_PARAM_SIZE; ++i) {
                ani_ref element;
                ret = env->Array_Get(static_cast<ani_array>(aniParam), i, &element);
                if (ret != ANI_OK) {
                    result = WmErrorCode::WM_ERROR_INVALID_PARAM;
                    return false;
                }
                ani_double value = 0;
                ret = env->Object_CallMethodByName_Double(static_cast<ani_object>(element), "toDouble", ":d", &value);
                if (ret != ANI_OK) {
                    result = WmErrorCode::WM_ERROR_INVALID_PARAM;
                    return false;
                }
                animationConfig.param[i] = static_cast<float>(value);
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
