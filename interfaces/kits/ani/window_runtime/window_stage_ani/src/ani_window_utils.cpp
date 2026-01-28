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

#include "ani_window_utils.h"

#include <iomanip>
#include <mutex>
#include <regex>
#include <sstream>

#include "ani.h"
#include "ani_err_utils.h"
#include "ani_window.h"
#include "bundle_constants.h"
#include "ipc_skeleton.h"
#include "ui_content.h"
#include "window_manager_hilog.h"
#include "pixel_map_taihe_ani.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr int32_t MAX_TOUCHABLE_AREAS = 10;
ani_ref g_booleanCls {};
ani_ref g_doubleCls {};
ani_ref g_intCls {};
ani_ref g_longCls {};

ani_method unboxBoolean {};
ani_method unboxDouble {};
ani_method unboxInt {};
ani_method unboxLong {};

constexpr uint32_t ANIMATION_FOUR_PARAMS_SIZE = 4;
const std::string INTERPOLATINGSPRING  = "interpolatingSpring";

std::mutex g_aniCreatorsMutex;
std::unordered_map<std::string, std::pair<ani_class, std::unordered_map<std::string, ani_method>>> globalAniCreators;

std::string GetHexColor(uint32_t color)
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
}

template<typename T>
ani_status unbox(ani_env* env, ani_object obj, T* result)
{
    return ANI_INVALID_TYPE;
}

template<>
ani_status unbox<ani_double>(ani_env* env, ani_object obj, ani_double* result)
{
    if (g_doubleCls == nullptr) {
        ani_class doubleCls {};
        auto status = env->FindClass("std.core.Double", &doubleCls);
        if (status != ANI_OK) {
            return status;
        }
        status = env->GlobalReference_Create(doubleCls, &g_doubleCls);
        if (status != ANI_OK) {
            return status;
        }
        status = env->Class_FindMethod(doubleCls, "toDouble", ":d", &unboxDouble);
        if (status != ANI_OK) {
            return status;
        }
    }
    return env->Object_CallMethod_Double(obj, unboxDouble, result);
}

template<>
ani_status unbox<ani_boolean>(ani_env* env, ani_object obj, ani_boolean* result)
{
    if (g_booleanCls == nullptr) {
        ani_class booleanCls {};
        auto status = env->FindClass("std.core.Boolean", &booleanCls);
        if (status != ANI_OK) {
            return status;
        }
        status = env->GlobalReference_Create(booleanCls, &g_booleanCls);
        if (status != ANI_OK) {
            return status;
        }
        status = env->Class_FindMethod(booleanCls, "toBoolean", ":z", &unboxBoolean);
        if (status != ANI_OK) {
            return status;
        }
    }
    return env->Object_CallMethod_Boolean(obj, unboxBoolean, result);
}

template<>
ani_status unbox<ani_int>(ani_env* env, ani_object obj, ani_int* result)
{
    if (g_intCls == nullptr) {
        ani_class intCls {};
        auto status = env->FindClass("std.core.Int", &intCls);
        if (status != ANI_OK) {
            return status;
        }
        status = env->GlobalReference_Create(intCls, &g_intCls);
        if (status != ANI_OK) {
            return status;
        }
        status = env->Class_FindMethod(intCls, "toInt", ":i", &unboxInt);
        if (status != ANI_OK) {
            return status;
        }
    }
    return env->Object_CallMethod_Int(obj, unboxInt, result);
}

template<>
ani_status unbox<ani_long>(ani_env* env, ani_object obj, ani_long* result)
{
    if (g_longCls == nullptr) {
        ani_class longCls {};
        auto status = env->FindClass("std.core.Long", &longCls);
        if (status != ANI_OK) {
            return status;
        }
        status = env->GlobalReference_Create(longCls, &g_longCls);
        if (status != ANI_OK) {
            return status;
        }
        status = env->Class_FindMethod(longCls, "toLong", ":l", &unboxLong);
        if (status != ANI_OK) {
            return status;
        }
    }
    return env->Object_CallMethod_Long(obj, unboxLong, result);
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

ani_status AniWindowUtils::GetStdString(ani_env *env, ani_string ani_str, std::string &result)
{
    ani_size strSize;
    ani_status ret = env->String_GetUTF8Size(ani_str, &strSize);
    if (ret != ANI_OK) {
        return ret;
    }
    std::vector<char> buffer(strSize + 1);
    char* utf8_buffer = buffer.data();
    ani_size bytes_written = 0;
    ret = env->String_GetUTF8(ani_str, utf8_buffer, strSize + 1, &bytes_written);
    if (ret != ANI_OK) {
        return ret;
    }
    utf8_buffer[bytes_written] = '\0';
    result = std::string(utf8_buffer);
    return ret;
}

ani_status AniWindowUtils::GetStdStringVector(ani_env* env, ani_object ary, std::vector<std::string>& result)
{
    ani_double length;
    ani_status ret = env->Object_GetPropertyByName_Double(ary, "length", &length);
    if (ret != ANI_OK) {
        return ret;
    }
    for (int32_t i = 0; i< static_cast<int32_t>(length); i++) {
        ani_ref stringRef;
        ret = env->Object_CallMethodByName_Ref(ary, "$_get", "i:Y", &stringRef, ani_int(i));
        if (ret != ANI_OK) {
            return ret;
        }
        std::string str;
        AniWindowUtils::GetStdString(env, static_cast<ani_string>(stringRef), str);
        result.emplace_back(str);
    }
    return ANI_OK;
}

ani_status AniWindowUtils::GetOptionalProperty(
    ani_env* env, ani_object object, const char* propertyName, ani_ref& outPropRef, bool& outIsUndefined)
{
    ani_status ret = env->Object_GetPropertyByName_Ref(object, propertyName, &outPropRef);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT,
              "[ANI] Failed to get property %{public}s, ret: %{public}d",
              propertyName, static_cast<int32_t>(ret));
        return ret;
    }

    ani_boolean isUndefined = ANI_FALSE;
    ret = env->Reference_IsUndefined(outPropRef, &isUndefined);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT,
              "[ANI] Failed to check if property %{public}s is undefined, ret: %{public}d",
              propertyName, static_cast<int32_t>(ret));
        return ret;
    }

    outIsUndefined = (isUndefined == ANI_TRUE);
    if (outIsUndefined) {
        TLOGD(WmsLogTag::DEFAULT, "[ANI] %{public}s is undefined", propertyName);
    }
    return ANI_OK;
}

ani_status AniWindowUtils::GetOptionalIntProperty(
    ani_env* env, const char* propertyName, ani_object object, std::optional<ani_int>& optIntProp)
{
    optIntProp.reset();

    ani_ref propRef;
    bool isUndefined;
    ani_status ret = AniWindowUtils::GetOptionalProperty(env, object, propertyName, propRef, isUndefined);
    if (ret != ANI_OK || isUndefined) {
        return ret;
    }

    ani_int intValue = 0;
    ret = env->Object_CallMethodByName_Int(static_cast<ani_object>(propRef), "intValue", nullptr, &intValue);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT,
              "[ANI] Failed to get intValue for %{public}s, ret: %{public}d",
              propertyName, static_cast<int32_t>(ret));
        return ret;
    }
    optIntProp = intValue;
    return ANI_OK;
}

ani_status AniWindowUtils::GetPropertyIntObject(ani_env* env, const char* propertyName,
    ani_object object, int32_t& result)
{
    ani_ref int_ref;
    ani_status ret = env->Object_GetPropertyByName_Ref(object, propertyName, &int_ref);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed, ret : %{public}u",
            propertyName, static_cast<int32_t>(ret));
        return ret;
    }

    ani_boolean isUndefined;
    ret = env->Reference_IsUndefined(int_ref, &isUndefined);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed", propertyName);
        return ret;
    }

    if (isUndefined) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] %{public}s is Undefined Now", propertyName);
        return ANI_ERROR;
    }

    ani_int int_value;
    ret = env->Object_CallMethodByName_Int(static_cast<ani_object>(int_ref), "intValue", nullptr, &int_value);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed", propertyName);
        return ret;
    }
    result = static_cast<int32_t>(int_value);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] %{public}s is: %{public}u", propertyName, result);
    return ret;
}

ani_status AniWindowUtils::GetPropertyDoubleObject(ani_env* env, const char* propertyName,
    ani_object object, double& result)
{
    ani_ref double_ref;
    ani_status ret = env->Object_GetPropertyByName_Ref(object, propertyName, &double_ref);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed, ret : %{public}u",
            propertyName, static_cast<int32_t>(ret));
        return ret;
    }
    ani_boolean isUndefined;
    ret = env->Reference_IsUndefined(double_ref, &isUndefined);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed", propertyName);
        return ret;
    }

    if (isUndefined) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] %{public}s is Undefined Now", propertyName);
        return ANI_ERROR;
    }

    ani_double double_value;
    ret = env->Object_CallMethodByName_Double(static_cast<ani_object>(double_ref),
        "doubleValue", nullptr, &double_value);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed", propertyName);
        return ret;
    }
    result = static_cast<double>(double_value);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] %{public}s is: %{public}f", propertyName, result);
    return ret;
}

ani_status AniWindowUtils::GetPropertyBoolObject(ani_env* env, const char* propertyName,
    ani_object object, bool& result)
{
    ani_ref bool_ref;
    ani_status ret = env->Object_GetPropertyByName_Ref(object, propertyName, &bool_ref);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed, ret : %{public}u",
            propertyName, static_cast<int32_t>(ret));
        return ret;
    }
    ani_boolean isUndefined;
    ret = env->Reference_IsUndefined(bool_ref, &isUndefined);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed", propertyName);
        return ret;
    }
    if (isUndefined) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] %{public}s is Undefined Now", propertyName);
        return ANI_ERROR;
    }

    ani_boolean bool_value;
    ret = env->Object_CallMethodByName_Boolean(static_cast<ani_object>(bool_ref), "toBoolean", ":z", &bool_value);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed", propertyName);
        return ret;
    }
    result = static_cast<bool>(bool_value);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] %{public}s is: %{public}d", propertyName, result);
    return ret;
}

ani_status AniWindowUtils::GetPropertyLongObject(ani_env* env, const char* propertyName, ani_object object,
                                                 int64_t& result)
{
    ani_ref long_ref;
    ani_status ret = env->Object_GetPropertyByName_Ref(object, propertyName, &long_ref);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s failed, ret : %{public}d",
            propertyName, static_cast<int32_t>(ret));
        return ret;
    }

    ani_boolean isUndefined;
    ret = env->Reference_IsUndefined(long_ref, &isUndefined);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Reference_IsUndefined %{public}s failed, ret : %{public}d",
            propertyName, static_cast<int32_t>(ret));
        return ret;
    }
    if (isUndefined) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] %{public}s is Undefined Now", propertyName);
        return ANI_ERROR;
    }
    ani_long long_value;
    ret = env->Object_CallMethodByName_Long(static_cast<ani_object>(long_ref), "toLong", ":l", &long_value);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_CallMethodByName_Long %{public}s failed, ret : %{public}d",
            propertyName, static_cast<int32_t>(ret));
        return ret;
    }
    result = static_cast<int64_t>(long_value);
    TLOGD(WmsLogTag::DEFAULT, "[ANI] property name is %{public}s, value is:%{public}" PRIu64, propertyName, result);
    return ret;
}

ani_status AniWindowUtils::GetEnumValue(ani_env* env, ani_enum_item enumPara, uint32_t& result)
{
    ani_int enumValue;
    ani_status ret = env->EnumItem_GetValue_Int(enumPara, &enumValue);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] EnumItem_GetValue_Int failed, ret : %{public}d", static_cast<int32_t>(ret));
        return ret;
    }
    result = static_cast<uint32_t>(enumValue);
    TLOGD(WmsLogTag::DEFAULT, "[ANI] enum value is:%{public}u", result);
    return ret;
}

bool AniWindowUtils::GetPropertyRectObject(ani_env* env, const char* propertyName,
    ani_object object, Rect& result)
{
    ani_ref windowRect;
    ani_status ret = env->Object_GetPropertyByName_Ref(object, propertyName, &windowRect);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed, ret : %{public}u",
            propertyName, static_cast<int32_t>(ret));
        return false;
    }

    int32_t posX = 0;
    int32_t posY = 0;
    int32_t width = 0;
    int32_t height = 0;
    bool ret_bool = GetIntObject(env, "left", static_cast<ani_object>(windowRect), posX);
    ret_bool &= GetIntObject(env, "top", static_cast<ani_object>(windowRect), posY);
    ret_bool &= GetIntObject(env, "width", static_cast<ani_object>(windowRect), width);
    ret_bool &= GetIntObject(env, "height", static_cast<ani_object>(windowRect), height);
    if (!ret_bool) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] GetIntObject Failed");
        return false;
    }
    result.posX_ = posX;
    result.posY_ = posY;
    result.width_ = width;
    result.height_ = height;
    TLOGI(WmsLogTag::DEFAULT, "[ANI] rect is [%{public}u, %{public}u, %{public}u, %{public}u]",
        result.posX_, result.posY_, result.width_, result.height_);
    return ret_bool;
}

bool AniWindowUtils::GetIntObject(ani_env* env, const char* propertyName,
    ani_object object, int32_t& result)
{
    ani_int int_value;
    ani_status ret = env->Object_GetPropertyByName_Int(object, propertyName, &int_value);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Int %{public}s Failed, ret : %{public}u",
            propertyName, static_cast<int32_t>(ret));
        return false;
    }
    result = static_cast<int32_t>(int_value);
    return true;
}

bool AniWindowUtils::GetPropertyUIntObject(ani_env* env, const char* propertyName,
    ani_object object, uint32_t& result)
{
    ani_ref uint_ref;
    ani_status ret = env->Object_GetPropertyByName_Ref(object, propertyName, &uint_ref);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed, ret : %{public}u",
            propertyName, static_cast<int32_t>(ret));
        return false;
    }
    ani_boolean isUndefined;
    if (ANI_OK != env->Reference_IsUndefined(uint_ref, &isUndefined)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Reference_IsUndefined %{public}s Failed", propertyName);
        return false;
    }

    if (isUndefined) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] %{public}s is Undefined Now", propertyName);
        return false;
    }

    ani_int int_value;
    if (ANI_OK != env->Object_CallMethodByName_Int(static_cast<ani_object>(uint_ref),
        "intValue", nullptr, &int_value)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed", propertyName);
        return false;
    }
    result = static_cast<uint32_t>(int_value);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] %{public}s is: %{public}u", propertyName, result);
    return true;
}

ani_status AniWindowUtils::GetBooleanObject(ani_env* env, ani_object boolean_object, bool& result)
{
    ani_boolean isUndefined;
    ani_status isUndefinedRet = env->Reference_IsUndefined(boolean_object, &isUndefined);
    if (ANI_OK != isUndefinedRet) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Check boolean_object isUndefined fail");
        return isUndefinedRet;
    }
    if (isUndefined) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] CallMeWithOptionalBoolean is undefined");
        return ANI_OK;
    }
    ani_boolean bool_value;
    ani_status ret = env->Object_CallMethodByName_Boolean(boolean_object, "toBoolean", ":z", &bool_value);
    if (ANI_OK != ret) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_CallMethodByName_Boolean Failed!");
        return ret;
    }
    result = static_cast<bool>(bool_value);
    return ret;
}

ani_status AniWindowUtils::GetDoubleObject(ani_env* env, ani_object double_object, double& result)
{
    ani_boolean isUndefined;
    ani_status isUndefinedRet = env->Reference_IsUndefined(double_object, &isUndefined);
    if (ANI_OK != isUndefinedRet) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Check double_object isUndefined fail");
        return isUndefinedRet;
    }

    if (isUndefined) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] CallMeWithOptionalDouble Not Pass Value");
        return ANI_INVALID_ARGS;
    }

    ani_double double_value;
    ani_status ret = env->Object_CallMethodByName_Double(double_object, "doubleValue", nullptr, &double_value);
    if (ANI_OK != ret) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_CallMethodByName_Double Failed!");
        return ret;
    }
    result = static_cast<double>(double_value);
    TLOGI(WmsLogTag::DEFAULT, "[ANI] double result is: %{public}f", result);
    return ret;
}

ani_status AniWindowUtils::GetIntVector(ani_env* env, ani_object ary, std::vector<int32_t>& result)
{
    ani_size size = 0;
    ani_status status = ANI_ERROR;

    if ((status = env->Array_GetLength(reinterpret_cast<ani_array>(ary), &size)) != ANI_OK) {
        TLOGI(WmsLogTag::WMS_LIFE, "getLength failed, status : %{public}d", status);
        return status;
    }

    TLOGI(WmsLogTag::WMS_LIFE, "vector size: %{public}d", static_cast<int32_t>(size));
    if (static_cast<int32_t>(size) <= 0) {
        return ANI_INVALID_ARGS;
    }
    std::unique_ptr<ani_double[]> native_buffer = std::make_unique<ani_double[]>(size);

    for (unsigned int i = 0; i < size; ++i) {
        ani_ref doubleRef {};
        ani_double doubleValue {};
        status = env->Array_Get(reinterpret_cast<ani_array>(ary), i, &doubleRef);
        if (status != ANI_OK) {
            TLOGI(WmsLogTag::WMS_LIFE, "Array_Get failed, status: %{public}d", status);
            return status;
        }
        status = unbox(env, static_cast<ani_object>(doubleRef), &doubleValue);
        if (status != ANI_OK) {
            TLOGI(WmsLogTag::WMS_LIFE, "Unbox failed, status: %{public}d", status);
            return status;
        }
        native_buffer[i] = doubleValue;
    }

    ani_size idx;
    for (idx = 0; idx < size; idx++) {
        result.push_back(static_cast<int32_t>(native_buffer[idx]));
    }
    return ANI_OK;
}

ani_status AniWindowUtils::NewAniObject(ani_env* env, const char* cls, const char* signature, ani_object* result, ...)
{
    ani_class aniClass;
    ani_status ret = env->FindClass(cls, &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found, ret:%{public}d", static_cast<int32_t>(ret));
        return ret;
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", signature, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] method not found, ret:%{public}d", static_cast<int32_t>(ret));
        return ret;
    }
    va_list args;
    va_start(args, result);
    ani_status status = env->Object_New(aniClass, aniCtor, result, args);
    va_end(args);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] new obj fail, ret:%{public}d", static_cast<int32_t>(ret));
        return ret;
    }
    return status;
}

ani_object AniWindowUtils::CreateAniUndefined(ani_env* env)
{
    ani_ref aniRef;
    env->GetUndefined(&aniRef);
    return static_cast<ani_object>(aniRef);
}

ani_object AniWindowUtils::AniThrowError(ani_env* env, WMError errorCode, std::string msg)
{
    AniErrUtils::ThrowBusinessError(env, errorCode, msg);
    return AniWindowUtils::CreateAniUndefined(env);
}

ani_object AniWindowUtils::AniThrowError(ani_env* env, WmErrorCode errorCode, std::string msg)
{
    AniErrUtils::ThrowBusinessError(env, errorCode, msg);
    return AniWindowUtils::CreateAniUndefined(env);
}

ani_object AniWindowUtils::CreateAniSize(ani_env* env, int32_t width, int32_t height)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.SizeInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object aniSize;
    ret = env->Object_New(aniClass, aniCtor, &aniSize);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, aniSize, aniClass, "<set>width", nullptr, ani_int(width));
    CallAniMethodVoid(env, aniSize, aniClass, "<set>height", nullptr, ani_int(height));
    return aniSize;
}

ani_object AniWindowUtils::CreateAniStatusBarProperty(ani_env* env, const SystemBarProperty& prop)
{
    ani_class cls;
    if (env->FindClass("@ohos.window.window.StatusBarPropertyInternal", &cls) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method ctor;
    if (env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object statusBarProperty;
    if (env->Object_New(cls, ctor, &statusBarProperty) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_string contentColor;
    if (GetAniString(env, GetHexColor(prop.contentColor_), &contentColor) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] create string failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, statusBarProperty, cls, "<set>contentColor", nullptr, contentColor);
    return statusBarProperty;
}

ani_object AniWindowUtils::CreateAniWindowDensityInfo(ani_env* env, const WindowDensityInfo& info)
{
    ani_class cls;
    if (env->FindClass("@ohos.window.window.WindowDensityInfoInternal", &cls) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method ctor;
    if (env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object windowDensityInfo;
    if (env->Object_New(cls, ctor, &windowDensityInfo) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, windowDensityInfo, cls, "<set>systemDensity", nullptr,
        static_cast<double>(info.systemDensity));
    CallAniMethodVoid(env, windowDensityInfo, cls, "<set>defaultDensity", nullptr,
        static_cast<double>(info.defaultDensity));
    CallAniMethodVoid(env, windowDensityInfo, cls, "<set>customDensity", nullptr,
        static_cast<double>(info.customDensity));
    return windowDensityInfo;
}

ani_object AniWindowUtils::CreateAniWindowSystemBarProperties(ani_env* env,
    const SystemBarProperty& status, const SystemBarProperty& navi)
{
    ani_class cls;
    if (env->FindClass("@ohos.window.window.SystemBarPropertiesInternal", &cls) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method ctor;
    if (env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object systemBarProperties;
    if (env->Object_New(cls, ctor, &systemBarProperties) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_string backgroundColor;
    if (GetAniString(env, GetHexColor(status.backgroundColor_), &backgroundColor) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] create string failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, systemBarProperties, cls, "<set>statusBarColor", nullptr, backgroundColor);
    ani_string statusBarContentColor;
    if (GetAniString(env, GetHexColor(status.contentColor_), &statusBarContentColor) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] create string failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, systemBarProperties, cls, "<set>statusBarContentColor", nullptr, statusBarContentColor);
    CallAniMethodVoid(env, systemBarProperties, cls, "<set>isStatusBarLightIcon", nullptr,
        status.contentColor_ == SYSTEM_COLOR_WHITE);
    ani_string navigationBarColor;
    if (GetAniString(env, GetHexColor(navi.backgroundColor_), &navigationBarColor) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] create string failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, systemBarProperties, cls, "<set>navigationBarColor", nullptr, navigationBarColor);
    ani_string navigationBarContentColor;
    if (GetAniString(env, GetHexColor(navi.contentColor_), &navigationBarContentColor) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] create string failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, systemBarProperties, cls, "<set>navigationBarContentColor",
        nullptr, navigationBarContentColor);
    CallAniMethodVoid(env, systemBarProperties, cls, "<set>isNavigationBarLightIcon", nullptr,
        navi.contentColor_ == SYSTEM_COLOR_WHITE);
    CallAniMethodVoid(env, systemBarProperties, cls, "<set>enableStatusBarAnimation", nullptr, status.enableAnimation_);
    CallAniMethodVoid(env, systemBarProperties, cls, "<set>enableNavigationBarAnimation", nullptr,
        navi.enableAnimation_);
    return systemBarProperties;
}

ani_object AniWindowUtils::CreateAniWindowLayoutInfo(ani_env* env, const WindowLayoutInfo& info)
{
    ani_class cls;
    if (env->FindClass("@ohos.window.window.WindowLayoutInfoInternal", &cls) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method ctor;
    if (env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object windowLayoutInfo;
    if (env->Object_New(cls, ctor, &windowLayoutInfo) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, windowLayoutInfo, cls, "<set>windowRect", nullptr, CreateAniRect(env, info.rect));
    return windowLayoutInfo;
}

ani_object AniWindowUtils::CreateAniWindowLayoutInfoArray(ani_env* env,
    const std::vector<sptr<WindowLayoutInfo>>& infos)
{
    ani_array windowLayoutInfoArray = nullptr;
    if (env->Array_New(infos.size(), CreateAniUndefined(env), &windowLayoutInfoArray) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] create array failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    for (size_t i = 0; i < infos.size(); i++) {
        if (env->Array_Set(windowLayoutInfoArray, i, CreateAniWindowLayoutInfo(env, *infos[i])) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] create windowLayoutInfoArray failed");
            return AniWindowUtils::CreateAniUndefined(env);
        }
    }
    return windowLayoutInfoArray;
}

ani_object AniWindowUtils::CreateAniWindowInfo(ani_env* env, const WindowVisibilityInfo& info)
{
    ani_class cls;
    if (env->FindClass("@ohos.window.window.WindowInfoInternal", &cls) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method ctor;
    if (env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object windowInfo;
    if (env->Object_New(cls, ctor, &windowInfo) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, windowInfo, cls, "<set>rect", nullptr, CreateAniRect(env, info.GetRect()));
    CallAniMethodVoid(env, windowInfo, cls, "<set>globalDisplayRect", nullptr,
        CreateAniRect(env, info.GetGlobalDisplayRect()));
    ani_string bundleName;
    if (GetAniString(env, info.GetBundleName(), &bundleName) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] create string failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, windowInfo, cls, "<set>bundleName", nullptr, bundleName);
    ani_string abilityName;
    if (GetAniString(env, info.GetAbilityName(), &abilityName) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] create string failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, windowInfo, cls, "<set>abilityName", nullptr, abilityName);
    CallAniMethodVoid(env, windowInfo, cls, "<set>windowId", nullptr, info.GetWindowId());
    env->Object_SetFieldByName_Int(windowInfo, "windowStatusTypeInternal", ani_int(info.GetWindowStatus()));
    env->Object_SetFieldByName_Boolean(windowInfo, "isFocusedInternal", ani_boolean(info.IsFocused()));
    return windowInfo;
}

ani_object AniWindowUtils::CreateAniWindowInfoArray(ani_env* env,
    const std::vector<sptr<WindowVisibilityInfo>>& infos)
{
    ani_array windowInfoArray = nullptr;
    if (env->Array_New(infos.size(), CreateAniUndefined(env), &windowInfoArray) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] create array failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    for (size_t i = 0; i < infos.size(); i++) {
        if (env->Array_Set(windowInfoArray, i, CreateAniWindowInfo(env, *infos[i])) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] CreateAniWindowInfo failed");
            return AniWindowUtils::CreateAniUndefined(env);
        }
    }
    return windowInfoArray;
}

ani_object AniWindowUtils::CreateAniWindowArray(ani_env* env, std::vector<ani_ref>& windows)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_array windowArray = nullptr;

    if (env->Array_New(windows.size(), CreateAniUndefined(env), &windowArray) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] create array fail");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    for (size_t i = 0; i < windows.size(); i++) {
        if (env->Array_Set(windowArray, i, windows[i]) != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] set window array failed");
            return AniWindowUtils::CreateAniUndefined(env);
        }
    }
    return windowArray;
}

ani_object AniWindowUtils::CreateAniRect(ani_env* env, const Rect& rect)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.RectInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", ":", &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object aniRect;
    ret = env->Object_New(aniClass, aniCtor, &aniRect);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to create new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, aniRect, aniClass, "<set>left", nullptr, ani_int(rect.posX_));
    CallAniMethodVoid(env, aniRect, aniClass, "<set>top", nullptr, ani_int(rect.posY_));
    CallAniMethodVoid(env, aniRect, aniClass, "<set>width", nullptr, ani_int(rect.width_));
    CallAniMethodVoid(env, aniRect, aniClass, "<set>height", nullptr, ani_int(rect.height_));
    return aniRect;
}

ani_object AniWindowUtils::CreateAniWindowLimits(ani_env* env, const WindowLimits& windowLimits)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.WindowLimitsInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    ani_object aniLimits;
    ret = env->Object_New(aniClass, aniCtor, &aniLimits);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to create new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_enum pixelUnit;
    ret = env->FindEnum("@ohos.window.window.PixelUnit", &pixelUnit);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to FindEnum");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    ani_enum_item pixelUnitItem;

    std::string itemName = GetPixelUnitString(windowLimits.pixelUnit_);
    ret = env->Enum_GetEnumItemByName(pixelUnit, itemName.c_str(), &pixelUnitItem);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Enum_GetEnumItemByName failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    CallAniMethodVoid(env, aniLimits, aniClass, "<set>maxWidth", nullptr,
        CreateBaseTypeObject<int>(env, windowLimits.maxWidth_));
    CallAniMethodVoid(env, aniLimits, aniClass, "<set>maxHeight", nullptr,
        CreateBaseTypeObject<int>(env, windowLimits.maxHeight_));
    CallAniMethodVoid(env, aniLimits, aniClass, "<set>minWidth", nullptr,
        CreateBaseTypeObject<int>(env, windowLimits.minWidth_));
    CallAniMethodVoid(env, aniLimits, aniClass, "<set>minHeight", nullptr,
        CreateBaseTypeObject<int>(env, windowLimits.minHeight_));
    CallAniMethodVoid(env, aniLimits, aniClass, "<set>pixelUnit", nullptr, pixelUnitItem);
    return aniLimits;
}

ani_object AniWindowUtils::CreateAniAvoidArea(ani_env* env, const AvoidArea& avoidArea,
    AvoidAreaType type, bool useActualVisibility)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.AvoidAreaInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object aniAvoidArea;
    ret = env->Object_New(aniClass, aniCtor, &aniAvoidArea);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    if (useActualVisibility) {
        CallAniMethodVoid(env, aniAvoidArea, aniClass, "<set>visible", nullptr,
            ani_boolean(!avoidArea.isEmptyAvoidArea()));
    } else {
        CallAniMethodVoid(env, aniAvoidArea, aniClass, "<set>visible", nullptr,
            ani_boolean(type != AvoidAreaType::TYPE_CUTOUT));
    }
    CallAniMethodVoid(env, aniAvoidArea, aniClass, "<set>leftRect", nullptr,
        CreateAniRect(env, avoidArea.leftRect_));
    CallAniMethodVoid(env, aniAvoidArea, aniClass, "<set>topRect", nullptr,
        CreateAniRect(env, avoidArea.topRect_));
    CallAniMethodVoid(env, aniAvoidArea, aniClass, "<set>rightRect", nullptr,
        CreateAniRect(env, avoidArea.rightRect_));
    CallAniMethodVoid(env, aniAvoidArea, aniClass, "<set>bottomRect", nullptr,
        CreateAniRect(env, avoidArea.bottomRect_));
    return aniAvoidArea;
}

ani_object AniWindowUtils::CreateAniTitleButtonRect(ani_env* env, const TitleButtonRect& rect)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.TitleButtonRectInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object aniRect;
    ret = env->Object_New(aniClass, aniCtor, &aniRect);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, aniRect, aniClass, "<set>right", nullptr, ani_int(rect.posX_));
    CallAniMethodVoid(env, aniRect, aniClass, "<set>top", nullptr, ani_int(rect.posY_));
    CallAniMethodVoid(env, aniRect, aniClass, "<set>width", nullptr, ani_int(rect.width_));
    CallAniMethodVoid(env, aniRect, aniClass, "<set>height", nullptr, ani_int(rect.height_));
    return aniRect;
}

ani_object AniWindowUtils::CreateAniDecorButtonStyle(ani_env* env, const DecorButtonStyle& style)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.DecorButtonStyleInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object aniStyle;
    ret = env->Object_New(aniClass, aniCtor, &aniStyle);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_enum aniColorModeType;
    ret = env->FindEnum("@ohos.app.ability.ConfigurationConstant.ConfigurationConstant.ColorMode", &aniColorModeType);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] failed to FindEnum");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    std::string itemName = "COLOR_MODE_NOT_SET";
    if (style.colorMode == LIGHT_COLOR_MODE) {
        itemName = "COLOR_MODE_DARK";
    } else if (style.colorMode == DARK_COLOR_MODE) {
        itemName = "COLOR_MODE_LIGHT";
    }
    ani_enum_item aniColorMode;
    ret = env->Enum_GetEnumItemByName(aniColorModeType, itemName.c_str(), &aniColorMode);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Enum_GetEnumItemByName failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, aniStyle, aniClass, "<set>colorMode", nullptr, aniColorMode);
    SetOptionalFieldInt(env, aniStyle, aniClass, "<set>buttonBackgroundSize", ani_int(style.buttonBackgroundSize));
    SetOptionalFieldInt(env, aniStyle, aniClass, "<set>spacingBetweenButtons", ani_int(style.spacingBetweenButtons));
    SetOptionalFieldInt(env, aniStyle, aniClass, "<set>closeButtonRightMargin", ani_int(style.closeButtonRightMargin));
    SetOptionalFieldInt(env, aniStyle, aniClass, "<set>buttonIconSize", ani_int(style.buttonIconSize));
    SetOptionalFieldInt(env, aniStyle, aniClass, "<set>buttonBackgroundCornerRadius",
        ani_int(style.buttonBackgroundCornerRadius));
    return aniStyle;
}

ani_status AniWindowUtils::SetOptionalFieldInt(ani_env* env, ani_object obj,
    ani_class cls, const char* method, ani_int aniInt)
{
    ani_class aniIntClass {};
    ani_status ret = env->FindClass("std.core.Int", &aniIntClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] class not found");
        return ret;
    }
    ani_method aniCtor {};
    ret = env->Class_FindMethod(aniIntClass, "<ctor>", "i:", &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] ctor not found");
        return ret;
    }
    ani_object intObj {};
    if (env->Object_New(aniIntClass, aniCtor, &intObj, aniInt) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "[ANI] fail to new obj: %{public}s", method);
        return ret;
    }
    return CallAniMethodVoid(env, obj, cls, method, nullptr, intObj);
}

bool AniWindowUtils::SetDecorButtonStyleFromAni(ani_env* env, DecorButtonStyle& decorButtonStyle,
                                                const ani_object& decorStyle)
{
    bool hasParam = false;
    hasParam |= GetColorMode(env, decorStyle, decorButtonStyle.colorMode);
    hasParam |= GetPropertyUIntObject(env, "buttonBackgroundSize", decorStyle, decorButtonStyle.buttonBackgroundSize);
    hasParam |= GetPropertyUIntObject(env, "spacingBetweenButtons",
        decorStyle, decorButtonStyle.spacingBetweenButtons);
    hasParam |= GetPropertyUIntObject(env, "closeButtonRightMargin",
        decorStyle, decorButtonStyle.closeButtonRightMargin);
    hasParam |= GetPropertyUIntObject(env, "buttonIconSize", decorStyle, decorButtonStyle.buttonIconSize);
    hasParam |= GetPropertyUIntObject(env, "buttonBackgroundCornerRadius",
        decorStyle, decorButtonStyle.buttonBackgroundCornerRadius);
    return hasParam;
}

bool AniWindowUtils::GetColorMode(ani_env* env, const ani_object& decorStyle, int32_t& colorMode)
{
    ani_ref colorModeRef;
    ani_status ret = env->Object_GetPropertyByName_Ref(decorStyle, "colorMode", &colorModeRef);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref colorMode Failed, ret : %{public}u",
            static_cast<int32_t>(ret));
        return false;
    }
    ani_boolean isUndefined;
    if (ANI_OK != env->Reference_IsUndefined(colorModeRef, &isUndefined)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Reference_IsUndefined colorMode Failed");
        return false;
    }
    if (isUndefined) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] colorMode is Undefined Now");
        return false;
    }

    ani_int colorModeValue;
    if (ANI_OK != env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(colorModeRef), &colorModeValue)) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref colorMode Failed");
        return false;
    }
    colorMode = static_cast<int32_t>(colorModeValue);
    return true;
}

ani_object AniWindowUtils::CreateAniWindowsArray(ani_env* env, std::vector<ani_ref>& windows)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_array windowArray = nullptr;

    if (env->Array_New(windows.size(), CreateAniUndefined(env), &windowArray) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] create array fail");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    for (size_t i = 0; i < windows.size(); i++) {
        if (env->Array_Set(windowArray, i, windows[i]) != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] set window array failed");
            return AniWindowUtils::CreateAniUndefined(env);
        }
    }
    return windowArray;
}

ani_object AniWindowUtils::CreateAniSystemBarTintState(ani_env* env, DisplayId displayId,
    const SystemBarRegionTints& tints)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    if (env->FindClass("@ohos.window.window.SystemBarTintStateInternal", &aniClass) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    if (env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object state;
    if (env->Object_New(aniClass, aniCtor, &state) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, state, aniClass, "<set>displayId", nullptr, static_cast<ani_long>(displayId));
    ani_array regionTintArray = nullptr;
    ani_class regionTintCls;
    if (env->FindClass("@ohos.window.window.SystemBarRegionTintInternal", &regionTintCls) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    if (env->Array_New(tints.size(), CreateAniUndefined(env), &regionTintArray) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] create array failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    for (size_t i = 0; i < tints.size(); i++) {
        if (env->Array_Set(regionTintArray, i, CreateAniSystemBarRegionTint(env, tints[i])) != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI] create region tint failed");
            return AniWindowUtils::CreateAniUndefined(env);
        }
    }
    CallAniMethodVoid(env, state, aniClass, "<set>regionTint", nullptr, regionTintArray);
    return state;
}

ani_object AniWindowUtils::CreateAniSystemBarRegionTint(ani_env* env, const SystemBarRegionTint& tint)
{
    ani_class regionTintCls;
    if (env->FindClass("@ohos.window.window.SystemBarRegionTintInternal", &regionTintCls) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method regionTintCtor;
    if (env->Class_FindMethod(regionTintCls, "<ctor>", nullptr, &regionTintCtor) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object regionTint;
    if (env->Object_New(regionTintCls, regionTintCtor, &regionTint) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    if (NATIVE_JS_TO_WINDOW_TYPE_MAP.count(tint.type_) != 0) {
        CallAniMethodVoid(env, regionTint, regionTintCls, "<set>type", nullptr,
            ani_long(NATIVE_JS_TO_WINDOW_TYPE_MAP.at(tint.type_)));
    } else {
        CallAniMethodVoid(env, regionTint, regionTintCls, "<set>type", nullptr, ani_long(tint.type_));
    }
    CallAniMethodVoid(env, regionTint, regionTintCls, "<set>isEnable", nullptr, ani_boolean(tint.prop_.enable_));
    ani_string backgroundColor;
    if (GetAniString(env, GetHexColor(tint.prop_.backgroundColor_), &backgroundColor) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] create string failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, regionTint, regionTintCls, "<set>backgroundColor", nullptr, backgroundColor);
    ani_string contentColor;
    if (GetAniString(env, GetHexColor(tint.prop_.contentColor_), &contentColor) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] create string failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, regionTint, regionTintCls, "<set>contentColor", nullptr, contentColor);
    CallAniMethodVoid(env, regionTint, regionTintCls, "<set>region", nullptr, CreateAniRect(env, tint.region_));
    return regionTint;
}

ani_object AniWindowUtils::CreateAniFrameMetrics(ani_env* env, const FrameMetrics& metrics)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.FrameMetricsInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object frameMetrics;
    ret = env->Object_New(aniClass, aniCtor, &frameMetrics);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] failed to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ret = CallAniMethodVoid(env, frameMetrics, aniClass, "<set>firstDrawFrame", nullptr,
        CreateOptionalBool(env, static_cast<ani_boolean>(metrics.firstDrawFrame_)));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] failed to set firstDrawFrame");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ret = CallAniMethodVoid(env, frameMetrics, aniClass, "<set>inputHandlingDuration", nullptr,
        ani_long(metrics.inputHandlingDuration_));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] failed to set inputHandlingDuration");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ret = CallAniMethodVoid(env, frameMetrics, aniClass, "<set>layoutMeasureDuration", nullptr,
        ani_long(metrics.layoutMeasureDuration_));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] failed to set layoutMeasureDuration");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ret = CallAniMethodVoid(env, frameMetrics, aniClass, "<set>vsyncTimestamp", nullptr,
        ani_long(metrics.vsyncTimestamp_));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] failed to set vsyncTimestamp");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    return frameMetrics;
}

ani_object AniWindowUtils::CreateAniRotationChangeInfo(ani_env* env, const RotationChangeInfo& info)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.RotationChangeInfoInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object RotationChangeInfo;
    ret = env->Object_New(aniClass, aniCtor, &RotationChangeInfo);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] failed to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_enum rotationChangeType;
    ret = env->FindEnum("@ohos.window.window.RotationChangeType", &rotationChangeType);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] failed to FindEnum");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_enum_item rotationChangeTypeItem;
    std::string itemName =
        info.type_ == RotationChangeType::WINDOW_WILL_ROTATE? "WINDOW_WILL_ROTATE" : "WINDOW_DID_ROTATE";
    ret = env->Enum_GetEnumItemByName(rotationChangeType, itemName.c_str(), &rotationChangeTypeItem);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] Enum_GetEnumItemByName failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, RotationChangeInfo, aniClass, "<set>type", nullptr,
        rotationChangeTypeItem);
    CallAniMethodVoid(env, RotationChangeInfo, aniClass, "<set>orientation", nullptr,
        ani_int(info.orientation_));
    CallAniMethodVoid(env, RotationChangeInfo, aniClass, "<set>displayId", nullptr,
        ani_long(info.displayId_));
    CallAniMethodVoid(env, RotationChangeInfo, aniClass, "<set>displayRect", nullptr,
        CreateAniRect(env, info.displayRect_));
    return RotationChangeInfo;
}

void AniWindowUtils::ParseRotationChangeResult(ani_env* env, ani_object obj, RotationChangeResult& rotationChangeResult)
{
    ani_boolean isUndefined;
    ani_status ret = env->Reference_IsUndefined(obj, &isUndefined);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] Check rotationChangeResultObj isUndefined failed, ret: %{public}d", ret);
        return;
    }
    if (isUndefined) {
        TLOGI(WmsLogTag::WMS_ROTATION, "[ANI] RotationChangeResult is undefined");
        return;
    }
    ani_ref rectTypeRef;
    ret = env->Object_GetPropertyByName_Ref(obj, "rectType", &rectTypeRef);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] Object_GetPropertyByName_Ref failed, ret: %{public}d", ret);
        return;
    }
    ani_int rectType;
    ret = env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(rectTypeRef), &rectType);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] EnumItem_GetValue_Int failed, ret: %{public}d", ret);
        return;
    }

    Rect windowRect;
    bool ret_bool = GetPropertyRectObject(env, "windowRect", obj, windowRect);
    if (!ret_bool) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] GetPropertyRectObject failed");
        return;
    }
    rotationChangeResult.rectType_ = static_cast<RectType>(rectType);
    rotationChangeResult.windowRect_ = windowRect;
}

ani_object AniWindowUtils::CreateAniKeyboardInfo(ani_env* env, const KeyboardPanelInfo& keyboardPanelInfo)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.KeyboardInfoInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object keyboardInfo;
    ret = env->Object_New(aniClass, aniCtor, &keyboardInfo);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] failed to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ret = CallAniMethodVoid(env, keyboardInfo, aniClass, "<set>beginRect", nullptr,
        CreateAniRect(env, keyboardPanelInfo.beginRect_));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] failed to set beginRect");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ret = CallAniMethodVoid(env, keyboardInfo, aniClass, "<set>endRect", nullptr,
        CreateAniRect(env, keyboardPanelInfo.endRect_));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] failed to set endRect");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    return keyboardInfo;
}

ani_object AniWindowUtils::CreateAniAnimationConfig(ani_env* env, const KeyboardAnimationCurve& curve)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.WindowAnimationConfigInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object aniConfig;
    ret = env->Object_New(aniClass, aniCtor, &aniConfig);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    if (curve.curveType_ == INTERPOLATINGSPRING) {
        ani_enum aniAnimationCurveType;
        ret = env->FindEnum("@ohos.window.window.WindowAnimationCurve", &aniAnimationCurveType);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] Find enum animationType failed");
            return AniWindowUtils::CreateAniUndefined(env);
        }
        ani_enum_item animationCurveTypeItem;
        std::string itemName = "INTERPOLATION_SPRING";
        ret = env->Enum_GetEnumItemByName(aniAnimationCurveType, itemName.c_str(), &animationCurveTypeItem);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] Get enum item %{public}s failed. ret: %{public}d",
                itemName.c_str(), ret);
            return AniWindowUtils::CreateAniUndefined(env);
        }
        ret = CallAniMethodVoid(env, aniConfig, aniClass, "<set>curve", nullptr, animationCurveTypeItem);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] failed to set curve");
            return AniWindowUtils::CreateAniUndefined(env);
        }
    }

    ret = CallAniMethodVoid(env, aniConfig, aniClass, "<set>duration", nullptr,
        CreateBaseTypeObject(env, curve.duration_));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] failed to set duration");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    auto paramSize = curve.curveParams_.size();
    if (paramSize == ANIMATION_FOUR_PARAMS_SIZE) {
        ani_array params = nullptr;
        if (env->Array_New(ANIMATION_PARAM_SIZE, static_cast<ani_ref>(CreateAniUndefined(env)),
            &params) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] create array failed");
            return AniWindowUtils::CreateAniUndefined(env);
        }
        for (uint32_t i = 0; i < ANIMATION_PARAM_SIZE; ++i) {
            if (env->Array_Set(params, i, CreateDouble(env, curve.curveParams_[i])) != ANI_OK) {
                TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] set params failed at %{public}d", i);
                return AniWindowUtils::CreateAniUndefined(env);
            }
        }
        ret = CallAniMethodVoid(env, aniConfig, aniClass, "<set>param", nullptr, params);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] failed to set param");
            return AniWindowUtils::CreateAniUndefined(env);
        }
    }
    return aniConfig;
}

ani_object AniWindowUtils::CreateAniAnimationInfo(ani_env* env, const KeyboardAnimationInfo& keyboardAnimationInfo,
    const KeyboardAnimationCurve& curve)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.KeyboardInfoInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object keyboardInfo;
    ret = env->Object_New(aniClass, aniCtor, &keyboardInfo);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] failed to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    ret = CallAniMethodVoid(env, keyboardInfo, aniClass, "<set>beginRect", nullptr,
        CreateAniRect(env, keyboardAnimationInfo.beginRect));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] failed to set beginRect");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    ret = CallAniMethodVoid(env, keyboardInfo, aniClass, "<set>endRect", nullptr,
        CreateAniRect(env, keyboardAnimationInfo.endRect));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] failed to set endRect");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    ret = CallAniMethodVoid(env, keyboardInfo, aniClass, "<set>animated", nullptr,
        CreateOptionalBool(env, static_cast<ani_boolean>(keyboardAnimationInfo.withAnimation)));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] failed to set animated");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    ret = CallAniMethodVoid(env, keyboardInfo, aniClass, "<set>config", nullptr,
        CreateAniAnimationConfig(env, curve));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[ANI] failed to set config");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    return keyboardInfo;
}

ani_status AniWindowUtils::CallAniFunctionVoid(ani_env *env, const char* ns,
    const char* fn, const char* signature, ...)
{
    ani_status ret = ANI_OK;
    ani_namespace aniNamespace{};
    if ((ret = env->FindNamespace(ns, &aniNamespace)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]canot find ns %{public}d", ret);
        return ret;
    }
    ani_function func{};
    if ((ret = env->Namespace_FindFunction(aniNamespace, fn, signature, &func)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]canot find callback %{public}d", ret);
        return ret;
    }
    va_list args;
    va_start(args, signature);
    ret = env->Function_Call_Void_V(func, args);
    va_end(args);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]canot run callback %{public}d", ret);
        return ret;
    }
    return ret;
}

ani_status AniWindowUtils::CallAniFunctionRef(ani_env *env, ani_ref& result,
    ani_ref ani_callback, const int32_t args_num, ...)
{
    va_list args;
    va_start(args, args_num);
    std::vector<ani_ref> vec;
    for (int i = 0; i < args_num; i++) {
        vec.push_back(va_arg(args, ani_object));
    }
    va_end(args);
    ani_status ret = env->FunctionalObject_Call(static_cast<ani_fn_object>(ani_callback), ani_size(args_num),
        vec.data(), &result);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]cannot run callback %{public}d", ret);
        return ret;
    }
    return ret;
}

ani_status AniWindowUtils::CallAniMethodVoid(ani_env *env, ani_object object, const char* cls,
    const char* method, const char* signature, ...)
{
    ani_class aniClass;
    ani_status ret = env->FindClass(cls, &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class:%{public}s not found", cls);
        return ret;
    }
    ani_method aniMethod;
    ret = env->Class_FindMethod(aniClass, method, nullptr, &aniMethod);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]2 class:%{public}s method:%{public}s not found, ret:%{public}d",
            cls, method, ret);
        return ret;
    }
    va_list args;
    va_start(args, signature);
    ret = env->Object_CallMethod_Void_V(object, aniMethod, args);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to call method:%{public}s", method);
    }
    va_end(args);
    return ret;
}

ani_status AniWindowUtils::CallAniMethodVoid(ani_env *env, ani_object object, ani_class cls,
    const char* method, const char* signature, ...)
{
    ani_method aniMethod;
    ani_status ret = env->Class_FindMethod(cls, method, signature, &aniMethod);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] method:%{public}s not found", method);
        return ret;
    }
    va_list args;
    va_start(args, signature);
    ret = env->Object_CallMethod_Void_V(object, aniMethod, args);
    va_end(args);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to call method:%{public}s", method);
    }
    return ret;
}

ani_status AniWindowUtils::GetAniString(ani_env* env, const std::string& str, ani_string* result)
{
    return env->String_NewUTF8(str.c_str(), static_cast<ani_size>(str.size()), result);
}

ani_object AniWindowUtils::CreateAniRectObject(ani_env* env, const Rect& rect)
{
    return InitAniObjectByCreator(env, "@ohos.window.window.RectInternal", "iiii:",
        ani_int(rect.posX_), ani_int(rect.posY_),
        ani_int(rect.width_), ani_int(rect.height_));
}

ani_object AniWindowUtils::CreateWindowsProperties(ani_env* env, const WindowPropertyInfo& windowPropertyInfo)
{
    ani_object aniRect = CreateAniRectObject(env, windowPropertyInfo.windowRect);
    ani_object aniDrawableRect = CreateAniRectObject(env, windowPropertyInfo.drawableRect);
    ani_object aniGlobalDisplayRect = CreateAniRectObject(env, windowPropertyInfo.globalDisplayRect);
    int windowType;
    if (NATIVE_JS_TO_WINDOW_TYPE_MAP.count(windowPropertyInfo.type) != 0) {
        windowType = static_cast<int>(NATIVE_JS_TO_WINDOW_TYPE_MAP.at(windowPropertyInfo.type));
    } else {
        windowType = static_cast<int>(windowPropertyInfo.type);
    }
    ani_string aniWindowName;
    GetAniString(env, windowPropertyInfo.name, &aniWindowName);
    ani_object aniWindowsProperties = InitAniObjectByCreator(env, "@ohos.window.window.WindowPropertiesInternal",
        "C{@ohos.window.window.Rect}C{@ohos.window.window.Rect}zzzzddzzzziilC{std.core.String}"
        "C{@ohos.window.window.Rect}:",
        aniRect, aniDrawableRect, ani_boolean(windowPropertyInfo.isFullScreen),
        ani_boolean(windowPropertyInfo.isLayoutFullScreen), ani_boolean(windowPropertyInfo.isFocusable),
        ani_boolean(windowPropertyInfo.isTouchable), ani_float(windowPropertyInfo.brightness), ani_float(0),
        ani_boolean(windowPropertyInfo.isKeepScreenOn), ani_boolean(windowPropertyInfo.isPrivacyMode),
        ani_boolean(false),  ani_boolean(windowPropertyInfo.isTransparent), ani_int(windowType),
        ani_int(windowPropertyInfo.id), ani_long(windowPropertyInfo.displayId), aniWindowName, aniGlobalDisplayRect);
    return aniWindowsProperties;
}

ani_status AniWindowUtils::InitAniCreator(ani_env* env,
    const std::string& aniClassDescriptor, const std::string& aniCtorSignature)
{
    ani_status status = ANI_OK;
    auto aniClassIter = globalAniCreators.find(aniClassDescriptor);
    if (aniClassIter != globalAniCreators.end()) {
        auto& aniCtorSignatureMap = aniClassIter->second.second;
        if (aniCtorSignatureMap.find(aniCtorSignature) != aniCtorSignatureMap.end()) {
            TLOGD(WmsLogTag::DEFAULT, "class %{public}s and its ctor already exist", aniClassDescriptor.c_str());
            return status;
        }
    }
    bool isNewClassEntry = false;
    ani_class aniClass = nullptr;
    if (aniClassIter == globalAniCreators.end()) {
        status = env->FindClass(aniClassDescriptor.c_str(), &aniClass);
        if (status != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "class %{public}s not found, ret %{public}d", aniClassDescriptor.c_str(), status);
            return status;
        }
        auto [iter, inserted] = globalAniCreators.emplace(
            aniClassDescriptor, std::make_pair(aniClass, std::unordered_map<std::string, ani_method>()));
        if (!inserted) {
            TLOGE(WmsLogTag::DEFAULT, "emplace class %{public}s failed", aniClassDescriptor.c_str());
            return ANI_ERROR;
        }
        aniClassIter = iter;
        isNewClassEntry = true;
        auto& newClassEntry = aniClassIter->second;
        status = env->GlobalReference_Create(static_cast<ani_ref>(newClassEntry.first),
            reinterpret_cast<ani_ref*>(&(newClassEntry.first)));
        if (status != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT, "GlobalReference_Create failed ret %{public}d", status);
            globalAniCreators.erase(aniClassIter);
            return status;
        }
    }
    aniClass = aniClassIter->second.first;
    ani_method aniCtorMethod;
    status = env->Class_FindMethod(aniClass, "<ctor>", aniCtorSignature.c_str(), &aniCtorMethod);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "find %{public}s ctor failed ret %{public}d", aniClassDescriptor.c_str(), status);
        if (isNewClassEntry) {
            env->GlobalReference_Delete(static_cast<ani_ref>(aniClass));
            globalAniCreators.erase(aniClassIter);
        }
        return status;
    }
    aniClassIter->second.second.emplace(aniCtorSignature, aniCtorMethod);
    return status;
}

ani_object AniWindowUtils::InitAniObjectByCreator(ani_env* env,
    const std::string& aniClassDescriptor, const std::string aniCtorSignature, ...)
{
    std::lock_guard<std::mutex> lock(g_aniCreatorsMutex);
    ani_status status = InitAniCreator(env, aniClassDescriptor, aniCtorSignature);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "InitAniCreator failed, ret %{public}d", status);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    va_list args;
    va_start(args, aniCtorSignature);
    auto& creatorEntry = globalAniCreators[aniClassDescriptor];
    ani_object aniObject;
    status = env->Object_New_V(
        creatorEntry.first,
        creatorEntry.second[aniCtorSignature],
        &aniObject,
        args);
    va_end(args);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "Object_New_V failed, ret %{public}d", status);
        return AniWindowUtils::CreateAniUndefined(env);
    }
    return aniObject;
}

ani_object AniWindowUtils::CreateAniPixelMapArray(ani_env* env,
    const std::vector<std::shared_ptr<Media::PixelMap>>& pixelMaps)
{
    ani_class cls;
    if (env->FindClass("@ohos.multimedia.image.image.PixelMap", &cls) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] class not found");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    ani_array pixelMapArray = nullptr;
    if (env->Array_New(pixelMaps.size(), CreateAniUndefined(env), &pixelMapArray) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] create array failed");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    for (size_t i = 0; i < pixelMaps.size(); i++) {
        if (pixelMaps[i] == nullptr) {
            auto status = env->Array_Set(pixelMapArray, i, AniWindowUtils::CreateAniUndefined(env));
            if (status != ANI_OK) {
                TLOGW(WmsLogTag::WMS_LIFE, "[ANI] pixelMap is nullptr, status: %{public}d", status);
            }
            continue;
        }
        auto nativePixelMap = OHOS::Media::PixelMapTaiheAni::CreateEtsPixelMap(env, pixelMaps[i]);
        if (nativePixelMap == nullptr) {
            auto status = env->Array_Set(pixelMapArray, i, AniWindowUtils::CreateAniUndefined(env));
            if (status != ANI_OK) {
                TLOGW(WmsLogTag::WMS_LIFE, "[ANI] nativePixelMap is nullptr, status: %{public}d", status);
            }
        } else {
            auto status = env->Array_Set(pixelMapArray, i, nativePixelMap);
            if (status != ANI_OK) {
                TLOGE(WmsLogTag::WMS_LIFE, "[ANI] set nativePixelMap failed: %{public}d", status);
            }
        }
    }
    return pixelMapArray;
}

ani_object AniWindowUtils::CreateAniMainWindowInfoArray(ani_env* env,
    const std::vector<sptr<MainWindowInfo>>& infos)
{
    ani_class aniClass;
    if (env->FindClass("@ohos.window.window.MainWindowInfoInternal", &aniClass) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] class not found");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    ani_array mainWindowInfoArray = nullptr;
    if (env->Array_New(infos.size(), CreateAniUndefined(env), &mainWindowInfoArray) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] create array failed");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    for (size_t i = 0; i < infos.size(); i++) {
        if (!infos[i]) {
            TLOGE(WmsLogTag::WMS_LIFE, "[ANI] infos[i] is nullptr at index %{public}d", static_cast<int32_t>(i));
            return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
        }
        if (env->Array_Set(mainWindowInfoArray, i, CreateAniMainWindowInfo(env, *infos[i])) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_LIFE, "[ANI] create mainWindowInfoArray failed");
            return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
        }
    }
    return mainWindowInfoArray;
}

ani_object AniWindowUtils::CreateAniMainWindowInfo(ani_env* env, const MainWindowInfo& info)
{
    ani_class aniClass;
    if (env->FindClass("@ohos.window.window.MainWindowInfoInternal", &aniClass) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] class not found");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    ani_method ctor;
    if (env->Class_FindMethod(aniClass, "<ctor>", nullptr, &ctor) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] ctor not found");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    ani_object mainWindowInfo;
    if (env->Object_New(aniClass, ctor, &mainWindowInfo) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] fail to new obj");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    ani_string label;
    if (GetAniString(env, info.label_, &label) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] create string failed");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    CallAniMethodVoid(env, mainWindowInfo, aniClass, "<set>displayId", nullptr, ani_double(info.displayId_));
    CallAniMethodVoid(env, mainWindowInfo, aniClass, "<set>showing", nullptr, ani_boolean(info.showing_));
    CallAniMethodVoid(env, mainWindowInfo, aniClass, "<set>windowId", nullptr, ani_double(info.persistentId_));
    CallAniMethodVoid(env, mainWindowInfo, aniClass, "<set>label", nullptr, label);
    return mainWindowInfo;
}

ani_status AniWindowUtils::CheckPropertyNameUndefined(ani_env* env, const char* propertyName,
    ani_object object, bool& result)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] env is nullptr");
        return ANI_ERROR;
    }
    result = false;
    ani_ref ref;
    ani_status ret = env->Object_GetPropertyByName_Ref(object, propertyName, &ref);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed, ret : %{public}u",
            propertyName, static_cast<int32_t>(ret));
        return ret;
    }
    ani_boolean isUndefined = false;
    ret = env->Reference_IsUndefined(ref, &isUndefined);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Object_GetPropertyByName_Ref %{public}s Failed", propertyName);
        return ret;
    }
    result = isUndefined;
    return ANI_OK;
}

bool AniWindowUtils::ParseKeyFramePolicy(ani_env* env, ani_object aniKeyFramePolicy, KeyFramePolicy& keyFramePolicy)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] env is nullptr");
        return false;
    }

    ani_boolean enable = false;
    if (env->Object_GetPropertyByName_Boolean(aniKeyFramePolicy, "enable", &enable) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Failed to convert parameter to enable");
        return false;
    }
    keyFramePolicy.dragResizeType_ = enable ? DragResizeType::RESIZE_KEY_FRAME :
        DragResizeType::RESIZE_TYPE_UNDEFINED;

    bool propertyUndefined = false;
    int32_t distance = 0;
    if (CheckPropertyNameUndefined(env, "distance", aniKeyFramePolicy, propertyUndefined) != ANI_OK ||
        !propertyUndefined) {
        if (GetPropertyIntObject(env, "distance", aniKeyFramePolicy, distance) != ANI_OK || distance < 0) {
            TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Failed to convert parameter to distance");
            return false;
        }
        keyFramePolicy.distance_ = static_cast<uint32_t>(distance);
    }

    int64_t longData = 0;
    if (CheckPropertyNameUndefined(env, "interval", aniKeyFramePolicy, propertyUndefined) != ANI_OK ||
        !propertyUndefined) {
        if (GetPropertyLongObject(env, "interval", aniKeyFramePolicy, longData) != ANI_OK || longData <= 0) {
            TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Failed to convert parameter to interval");
            return false;
        }
        keyFramePolicy.interval_ = static_cast<uint32_t>(longData);
    }
    if (CheckPropertyNameUndefined(env, "animationDelay", aniKeyFramePolicy, propertyUndefined) != ANI_OK ||
        !propertyUndefined) {
        if (GetPropertyLongObject(env, "animationDelay", aniKeyFramePolicy, longData) != ANI_OK || longData < 0) {
            TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Failed to convert parameter to animationDelay");
            return false;
        }
        keyFramePolicy.animationDelay_ = static_cast<uint32_t>(longData);
    }
    if (CheckPropertyNameUndefined(env, "animationDuration", aniKeyFramePolicy, propertyUndefined) != ANI_OK ||
        !propertyUndefined) {
        if (GetPropertyLongObject(env, "animationDuration", aniKeyFramePolicy, longData) != ANI_OK || longData < 0) {
            TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] Failed to convert parameter to animationDuration");
            return false;
        }
        keyFramePolicy.animationDuration_ = static_cast<uint32_t>(longData);
    }
    return true;
}

ani_object AniWindowUtils::CreateKeyFramePolicy(ani_env* env, const KeyFramePolicy& keyFramePolicy)
{
    ani_class aniClass;
    if (env->FindClass("@ohos.window.window.KeyFramePolicyInternal", &aniClass) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method ctor;
    if (env->Class_FindMethod(aniClass, "<ctor>", nullptr, &ctor) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object aniKeyFramePolicy;
    if (env->Object_New(aniClass, ctor, &aniKeyFramePolicy) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] fail to new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    CallAniMethodVoid(env, aniKeyFramePolicy, aniClass, "<set>enable", nullptr,
        ani_boolean(keyFramePolicy.enabled()));
    CallAniMethodVoid(env, aniKeyFramePolicy, aniClass, "<set>interval", nullptr,
        CreateBaseTypeObject<long>(env, keyFramePolicy.interval_));
    CallAniMethodVoid(env, aniKeyFramePolicy, aniClass, "<set>distance", nullptr,
        CreateBaseTypeObject<int>(env, keyFramePolicy.distance_));
    CallAniMethodVoid(env, aniKeyFramePolicy, aniClass, "<set>animationDuration", nullptr,
        CreateBaseTypeObject<long>(env, keyFramePolicy.animationDuration_));
    CallAniMethodVoid(env, aniKeyFramePolicy, aniClass, "<set>animationDelay", nullptr,
        CreateBaseTypeObject<long>(env, keyFramePolicy.animationDelay_));
    return aniKeyFramePolicy;
}

void AniWindowUtils::GetWindowSnapshotConfiguration(ani_env* env, ani_object config,
    WindowSnapshotConfiguration& windowSnapshotConfiguration)
{
    ani_status ret;
    ani_ref nativeObj;
    if ((ret = env->Object_GetPropertyByName_Ref(config, "useCache", &nativeObj)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] obj fetch long %{public}u", ret);
        return;
    }
    ani_boolean value = 0;
    env->Object_CallMethodByName_Boolean(static_cast<ani_object>(nativeObj), "toBoolean", ":z", &value);
    windowSnapshotConfiguration.useCache = static_cast<bool>(value);
}

ani_object AniWindowUtils::CreateProperties(ani_env* env, const sptr<Window>& window)
{
    return {};
}

uint32_t AniWindowUtils::GetColorFromAni(ani_env* env,
                                         const char* name,
                                         uint32_t defaultColor,
                                         bool& flag,
                                         const ani_object& aniObject)
{
    ani_ref result;
    env->Object_GetPropertyByName_Ref(aniObject, name, &result);
    ani_boolean isColorUndefined;
    env->Reference_IsUndefined(result, &isColorUndefined);
    if (isColorUndefined) {
        TLOGI(WmsLogTag::WMS_IMMS, "the color is undefined, return default");
        return defaultColor;
    }

    ani_string aniColor = reinterpret_cast<ani_string>(result);
    std::string colorStr;
    GetStdString(env, aniColor, colorStr);
    std::regex pattern("^#([A-Fa-f0-9]{6}|[A-Fa-f0-9]{8})$");
    if (!std::regex_match(colorStr, pattern)) {
        TLOGD(WmsLogTag::DEFAULT, "Invalid color input");
        return defaultColor;
    }
    std::string color = colorStr.substr(1);
    if (color.length() == RGB_LENGTH) {
        color = "FF" + color; // ARGB
    }
    flag = true;
    std::stringstream ss;
    uint32_t hexColor;
    ss << std::hex << color;
    ss >> hexColor;
    TLOGI(WmsLogTag::DEFAULT, "Origin %{public}s, process %{public}s, final %{public}x",
        colorStr.c_str(), color.c_str(), hexColor);
    return hexColor;
}

bool AniWindowUtils::SetWindowStatusBarContentColor(ani_env* env,
                                                    ani_object aniObject,
                                                    std::map<WindowType, SystemBarProperty>& properties,
                                                    std::map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    auto statusProperty = properties[WindowType::WINDOW_TYPE_STATUS_BAR];
    ani_ref aniStatusContentColor;
    env->Object_GetPropertyByName_Ref(aniObject, "statusBarContentColor", &aniStatusContentColor);
    ani_ref aniStatusIcon;
    env->Object_GetPropertyByName_Ref(aniObject, "isStatusBarLightIcon", &aniStatusIcon);

    ani_boolean isColorUndefined;
    env->Reference_IsUndefined(aniStatusContentColor, &isColorUndefined);
    ani_boolean isIconUndefined;
    env->Reference_IsUndefined(aniStatusIcon, &isIconUndefined);

    if (!isColorUndefined) {
        properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = GetColorFromAni(
            env,
            "statusBarContentColor",
            statusProperty.contentColor_,
            propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag,
            aniObject);
    } else if (!isIconUndefined) {
        ani_boolean isStatusIcon;
        env->Object_CallMethodByName_Boolean(static_cast<ani_object>(aniStatusIcon), "toBoolean", ":z", &isStatusIcon);
        if (static_cast<bool>(aniStatusIcon)) {
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = SYSTEM_COLOR_WHITE;
        } else {
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = SYSTEM_COLOR_BLACK;
        }
        propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag = true;
    }
    return true;
}

bool AniWindowUtils::SetWindowNavigationBarContentColor(ani_env* env,
                                                        ani_object aniObject,
                                                        std::map<WindowType,
                                                        SystemBarProperty>& properties,
                                                        std::map<WindowType,
                                                        SystemBarPropertyFlag>& propertyFlags)
{
    auto navProperty = properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR];
    ani_ref aniNaviGationBarColor;
    env->Object_GetPropertyByName_Ref(aniObject, "navigationBarContentColor", &aniNaviGationBarColor);
    ani_ref aniNavigationIcon;
    env->Object_GetPropertyByName_Ref(aniObject, "isNavigationBarLightIcon", &aniNavigationIcon);

    ani_boolean isColorUndefined;
    env->Reference_IsUndefined(aniNaviGationBarColor, &isColorUndefined);
    ani_boolean isIconUndefined;
    env->Reference_IsUndefined(aniNavigationIcon, &isIconUndefined);

    if (!isColorUndefined) {
        properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = GetColorFromAni(
            env,
            "navigationBarContentColor",
            navProperty.contentColor_,
            propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColorFlag,
            aniObject);
    } else if (!isIconUndefined) {
        ani_boolean isNavigationIcon;
        env->Object_CallMethodByName_Boolean(static_cast<ani_object>(aniNavigationIcon),
            "toBoolean", ":z", &isNavigationIcon);
        if (static_cast<bool>(isNavigationIcon)) {
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = SYSTEM_COLOR_WHITE;
        } else {
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = SYSTEM_COLOR_BLACK;
        }
        propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColorFlag = true;
    }
    return true;
}

bool AniWindowUtils::SetSystemBarPropertiesFromAni(ani_env* env,
                                                   std::map<WindowType, SystemBarProperty>& windowBarProperties,
                                                   std::map<WindowType, SystemBarPropertyFlag>& windowPropertyFlags,
                                                   const ani_object& aniProperties,
                                                   const sptr<Window>& window)
{
    auto statusProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    auto navProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    windowBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR] = statusProperty;
    windowBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR] = navProperty;
    windowPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarPropertyFlag();
    windowPropertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR] = SystemBarPropertyFlag();

    // statusBarColor
    uint32_t aniStatusBarColor = GetColorFromAni(
        env,
        "statusBarColor",
        statusProperty.backgroundColor_,
        windowPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColorFlag,
        aniProperties);
    windowBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColor_ = aniStatusBarColor;

    // navigationBarColor
    uint32_t aniNaviGationBarColor = GetColorFromAni(
        env,
        "navigationBarColor",
        navProperty.backgroundColor_,
        windowPropertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColorFlag,
        aniProperties);
    windowBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColor_ = aniNaviGationBarColor;

    // windowStatusBarContentColor
    if (!SetWindowStatusBarContentColor(env, aniProperties, windowBarProperties, windowPropertyFlags) ||
        !SetWindowNavigationBarContentColor(env, aniProperties, windowBarProperties, windowPropertyFlags)) {
        return false;
    }

    ani_ref aniEnableStatusBarAnimation;
    env->Object_GetPropertyByName_Ref(aniProperties, "enableStatusBarAnimation", &aniEnableStatusBarAnimation);
    ani_boolean isStatusAnimationUndefined;
    env->Reference_IsUndefined(aniEnableStatusBarAnimation, &isStatusAnimationUndefined);
    if (!isStatusAnimationUndefined) {
        ani_boolean isStatusAnimation;
        env->Object_CallMethodByName_Boolean(static_cast<ani_object>(aniEnableStatusBarAnimation),
            "toBoolean", "z", &isStatusAnimation);
        windowBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_ =
            static_cast<bool>(isStatusAnimation);
        windowPropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimationFlag = true;
    }

    ani_ref aniEnableNavigationBarAnimation;
    env->Object_GetPropertyByName_Ref(aniProperties, "enableNavigationBarAnimation",
        &aniEnableNavigationBarAnimation);
    ani_boolean isNavigationAnimationUndefined;
    env->Reference_IsUndefined(aniEnableNavigationBarAnimation, &isNavigationAnimationUndefined);
    if (!isNavigationAnimationUndefined) {
        ani_boolean isNavigationAnimation;
        env->Object_CallMethodByName_Boolean(static_cast<ani_object>(aniEnableNavigationBarAnimation),
            "toBoolean", "z", &isNavigationAnimation);
        windowBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableAnimation_ =
            static_cast<bool>(isNavigationAnimation);
        windowPropertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableAnimationFlag = true;
    }
    return true;
}

void AniWindowUtils::GetSystemBarPropertiesFromAni(sptr<Window>& window,
    std::map<WindowType, SystemBarProperty>& newProperties,
    std::map<WindowType, SystemBarPropertyFlag>& newPropertyFlags,
    std::map<WindowType, SystemBarProperty>& properties,
    std::map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    for (auto type : {WindowType::WINDOW_TYPE_STATUS_BAR, WindowType::WINDOW_TYPE_NAVIGATION_BAR}) {
        auto property = window->GetSystemBarPropertyByType(type);
        properties[type] = property;
        propertyFlags[type] = SystemBarPropertyFlag();

        properties[type].backgroundColor_ = newProperties[type].backgroundColor_;
        properties[type].contentColor_ = newProperties[type].contentColor_;
        properties[type].enableAnimation_ = newProperties[type].enableAnimation_;
        propertyFlags[type].backgroundColorFlag = newPropertyFlags[type].backgroundColorFlag;
        propertyFlags[type].contentColorFlag = newPropertyFlags[type].contentColorFlag;
        propertyFlags[type].enableAnimationFlag = newPropertyFlags[type].enableAnimationFlag;
    }
}

void AniWindowUtils::UpdateSystemBarProperties(std::map<WindowType, SystemBarProperty>& systemBarProperties,
    const std::map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags, sptr<Window> windowToken)
{
    for (auto it : systemBarPropertyFlags) {
        WindowType type = it.first;
        SystemBarPropertyFlag flag = it.second;
        auto property = windowToken->GetSystemBarPropertyByType(type);
        if (flag.enableFlag == false) {
            systemBarProperties[type].enable_ = property.enable_;
        }
        if (flag.backgroundColorFlag == false) {
            systemBarProperties[type].backgroundColor_ = property.backgroundColor_;
        }
        if (flag.contentColorFlag == false) {
            systemBarProperties[type].contentColor_ = property.contentColor_;
        }
        if (flag.enableAnimationFlag == false) {
            systemBarProperties[type].enableAnimation_ = property.enableAnimation_;
        }
        if (flag.enableFlag == true) {
            systemBarProperties[type].settingFlag_ =
                static_cast<SystemBarSettingFlag>(static_cast<uint32_t>(property.settingFlag_) |
                static_cast<uint32_t>(SystemBarSettingFlag::ENABLE_SETTING));
        }
        if (flag.backgroundColorFlag == true || flag.contentColorFlag == true) {
            systemBarProperties[type].settingFlag_ =
                static_cast<SystemBarSettingFlag>(static_cast<uint32_t>(property.settingFlag_) |
                static_cast<uint32_t>(SystemBarSettingFlag::COLOR_SETTING));
        }
    }

    return;
}

bool AniWindowUtils::GetSpecificBarStatus(ani_env* env,
    ani_string aniName, ani_boolean aniEnable, ani_object aniAnimation,
    WindowType& type, SystemBarProperty& systemBarProperty, SystemBarPropertyFlag& systemBarPropertyFlag)
{
    std::string barName;
    GetStdString(env, aniName, barName);
    if (barName.compare("status") == 0) {
        type = WindowType::WINDOW_TYPE_STATUS_BAR;
    } else if (barName.compare("navigation") == 0) {
        type = WindowType::WINDOW_TYPE_NAVIGATION_BAR;
    } else if (barName.compare("navigationIndicator") == 0) {
        type = WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR;
    } else {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] name is invalid");
        return false;
    }
    systemBarProperty.enable_ = static_cast<bool>(aniEnable);
    systemBarPropertyFlag.enableFlag = true;

    ani_boolean isUndefined;
    if (env->Reference_IsUndefined(aniAnimation, &isUndefined) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "[ANI] check boolean_object failed");
        return false;
    }
    if (!isUndefined) {
        ani_boolean bool_value;
        if (env->Object_CallMethodByName_Boolean(aniAnimation, "toBoolean", ":z", &bool_value) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_IMMS, "[ANI] Object_CallMethodByName_Boolean failed");
            return false;
        }
        systemBarProperty.enableAnimation_ = static_cast<bool>(bool_value);
        systemBarPropertyFlag.enableAnimationFlag = true;
    }
    return true;
}

void* AniWindowUtils::GetAbilityContext(ani_env *env, ani_object aniObj)
{
    ani_long nativeContextLong;
    ani_class cls = nullptr;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass("application.UIAbilityContext.UIAbilityContext", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT,  "[ANI] find class fail, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT,  "[ANI] find field fail, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT,  "[ANI] get field fail, status : %{public}d", status);
        return nullptr;
    }
    return (void*)nativeContextLong;
}

bool AniWindowUtils::ParseWindowLimits(ani_env* env, ani_object aniWindowLimits, WindowLimits& windowLimits)
{
    auto getAndAssign = [&, where = __func__](const char* name, uint32_t& field) -> ani_status {
        std::optional<ani_int> optValue;
        ani_status ret = AniWindowUtils::GetOptionalIntProperty(env, name, aniWindowLimits, optValue);
        if (ret != ANI_OK) {
            return ret;
        }
        if (!optValue || *optValue < 0) {
            field = 0;
            TLOGNW(WmsLogTag::WMS_LAYOUT, "%{public}s: [ANI] Use default parameters for %{public}s", where, name);
            return ANI_OK;
        }
        field = static_cast<uint32_t>(*optValue);
        return ANI_OK;
    };

    auto getAndAssignUnit = [&](const char* name, PixelUnit& field) -> ani_status {
        std::optional<PixelUnit> optValue;
        ani_status ret = AniWindowUtils::GetOptionalEnumProperty(env, name, aniWindowLimits, optValue);
        if (ret != ANI_OK) {
            return ret;
        }
        if (optValue) {
            field = *optValue;
        } else {
            field = PixelUnit::PX;
        }
        return ANI_OK;
    };
    if (getAndAssign("maxWidth", windowLimits.maxWidth_) != ANI_OK ||
        getAndAssign("maxHeight", windowLimits.maxHeight_) != ANI_OK ||
        getAndAssign("minWidth", windowLimits.minWidth_) != ANI_OK ||
        getAndAssign("minHeight", windowLimits.minHeight_) != ANI_OK ||
        getAndAssignUnit("pixelUnit", windowLimits.pixelUnit_) != ANI_OK) {
        return false;
    }
    return true;
}

bool AniWindowUtils::CheckParaIsUndefined(ani_env* env, ani_object para)
{
    ani_boolean isUndefined;
    ani_status aniRet = env->Reference_IsUndefined(para, &isUndefined);
    if (aniRet != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Reference_IsUndefined failed, ret : %{public}d",
            static_cast<int32_t>(aniRet));
        return true;
    }
    return static_cast<bool>(isUndefined);
}

ani_object AniWindowUtils::CreateAniPosition(ani_env* env, const Position& position)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("@ohos.window.window.PositionInternal", &aniClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    ani_method aniCtor;
    ret = env->Class_FindMethod(aniClass, "<ctor>", nullptr, &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    ani_object aniPosition;
    ret = env->Object_New(aniClass, aniCtor, &aniPosition);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to create new obj");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    CallAniMethodVoid(env, aniPosition, aniClass, "<set>x", nullptr, ani_int(position.x));
    CallAniMethodVoid(env, aniPosition, aniClass, "<set>y", nullptr, ani_int(position.y));
    return aniPosition;
}

std::string AniWindowUtils::GetPixelUnitString(const PixelUnit& pixelUnit)
{
    switch (pixelUnit) {
        case PixelUnit::PX:
            return "PX";
        case PixelUnit::VP:
            return "VP";
        default:
            TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI] GetPixelUnitString default");
            return "PX";
    }
}

WmErrorCode AniWindowUtils::ToErrorCode(WMError error, WmErrorCode defaultCode)
{
    auto it = WM_JS_TO_ERROR_CODE_MAP.find(error);
    if (it != WM_JS_TO_ERROR_CODE_MAP.end()) {
        return it->second;
    }
    TLOGW(WmsLogTag::DEFAULT,
        "[ANI] Unknown error: %{public}d, return defaultCode: %{public}d",
        static_cast<int32_t>(error), static_cast<int32_t>(defaultCode));
    return defaultCode;
}

bool AniWindowUtils::ParseWindowMask(ani_env* env, ani_array windowMaskArray,
    std::vector<std::vector<uint32_t>>& windowMask)
{
    ani_size size;
    ani_status aniRet = env->Array_GetLength(windowMaskArray, &size);
    if (aniRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]Get windowMask rows failed, ret: %{public}u", aniRet);
        return false;
    }
    TLOGI(WmsLogTag::WMS_PC, "[ANI]windowMask rows: %{public}zu", size);
    for (ani_size i = 0; i < size; i++) {
        ani_ref innerArrayRef;
        aniRet = env->Array_Get(windowMaskArray, i, &innerArrayRef);
        if (aniRet != ANI_OK) {
            TLOGE(WmsLogTag::WMS_PC, "[ANI]Get windowMask cols ref failed, ret: %{public}u", aniRet);
            return false;
        }
        std::vector<uint32_t> elementArray;
        if (!ParseWindowMaskInnerValue(env, static_cast<ani_array>(innerArrayRef), elementArray)) {
            TLOGE(WmsLogTag::WMS_PC, "[ANI]Failed to convert parameter to window mask!");
            return false;
        }
        windowMask.emplace_back(elementArray);
    }
    aniRet = env->GlobalReference_Delete(g_longCls);
    if (aniRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]Failed to delete g_longCls ref, ret: %{public}u", aniRet);
    }
    return true;
}

bool AniWindowUtils::ParseWindowMaskInnerValue(ani_env* env, ani_array innerArray,
    std::vector<uint32_t>& elementArray)
{
    ani_size size;
    ani_status aniRet = env->Array_GetLength(innerArray, &size);
    if (aniRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]Get windowMask cols failed, ret: %{public}u", aniRet);
        return false;
    }
    for (ani_size i = 0; i < size; i++) {
        ani_ref maskValueRef;
        aniRet = env->Array_Get(innerArray, i, &maskValueRef);
        if (aniRet != ANI_OK) {
            TLOGE(WmsLogTag::WMS_PC, "[ANI]Get maskValueRef failed, ret: %{public}u", aniRet);
            return false;
        }
        ani_long maskValue = 0;
        aniRet = unbox(env, static_cast<ani_object>(maskValueRef), &maskValue);
        if (aniRet != ANI_OK) {
            TLOGE(WmsLogTag::WMS_PC, "[ANI]Get maskValue failed, ret: %{public}u", aniRet);
            return false;
        }
        elementArray.emplace_back(static_cast<uint32_t>(maskValue));
    }
    return true;
}

WmErrorCode AniWindowUtils::ParseTouchableAreas(ani_env* env, ani_array rects, const Rect& windowRect,
    std::vector<Rect>& touchableAreas)
{
    WmErrorCode errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    ani_size size;
    ani_status aniRet = env->Array_GetLength(rects, &size);
    if (aniRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]Get rects size failed, ret: %{public}u", aniRet);
        return errCode;
    }
    if (size > static_cast<ani_size>(MAX_TOUCHABLE_AREAS)) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]Exceed maximum rects limit, rects size: %{public}zu", size);
        return errCode;
    }
    errCode = WmErrorCode::WM_OK;
    for (ani_size i = 0; i < size; i++) {
        ani_ref rectRef;
        aniRet = env->Array_Get(rects, i, &rectRef);
        if (aniRet != ANI_OK) {
            TLOGE(WmsLogTag::WMS_EVENT, "[ANI]Get rect ref failed, ret: %{public}u", aniRet);
            return WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
        Rect touchableArea;
        if (ParseAndCheckRect(env, static_cast<ani_object>(rectRef), windowRect, touchableArea)) {
            touchableAreas.emplace_back(touchableArea);
        } else {
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
            break;
        }
    }
    return errCode;
}

bool AniWindowUtils::ParseAndCheckRect(ani_env* env, ani_object rect, const Rect& windowRect, Rect& touchableRect)
{
    int32_t data = 0;
    if (AniWindowUtils::GetIntObject(env, "left", rect, data)) {
        touchableRect.posX_ = data;
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]Failed to parse rect:left");
        return false;
    }
    if (AniWindowUtils::GetIntObject(env, "top", rect, data)) {
        touchableRect.posY_ = data;
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]Failed to parse rect:top");
        return false;
    }
    if (AniWindowUtils::GetIntObject(env, "width", rect, data)) {
        touchableRect.width_ = static_cast<uint32_t>(data);
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]Failed to parse rect:width");
        return false;
    }
    if (AniWindowUtils::GetIntObject(env, "height", rect, data)) {
        touchableRect.height_ = static_cast<uint32_t>(data);
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]Failed to parse rect:height");
        return false;
    }
    if ((touchableRect.posX_ < 0) || (touchableRect.posY_ < 0) ||
        (touchableRect.posX_ > static_cast<int32_t>(windowRect.width_)) ||
        (touchableRect.posY_ > static_cast<int32_t>(windowRect.height_)) ||
        (touchableRect.width_ > (windowRect.width_ - static_cast<uint32_t>(touchableRect.posX_))) ||
        (touchableRect.height_ > (windowRect.height_ - static_cast<uint32_t>(touchableRect.posY_)))) {
        TLOGE(WmsLogTag::WMS_EVENT, "[ANI]Outside the window area, "
            "touchRect:[%{public}d %{public}d %{public}u %{public}u], "
            "windowRect:[%{public}d %{public}d %{public}u %{public}u]",
            touchableRect.posX_, touchableRect.posY_, touchableRect.width_, touchableRect.height_,
            windowRect.posX_, windowRect.posY_, windowRect.width_, windowRect.height_);
        return false;
    }
    return true;
}

bool AniWindowUtils::IsInstanceOf(ani_env* env, ani_object obj, const char* className)
{
    RETURN_IF_NULL(env, false);
    RETURN_IF_NULL(obj, false);
    RETURN_IF_NULL(className, false);

    ani_class cls;
    ani_status ret = env->FindClass(className, &cls);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT,
              "[ANI] Failed to find class %{public}s, ret: %{public}d",
              className, static_cast<int32_t>(ret));
        return false;
    }

    ani_boolean isInstance = ANI_FALSE;
    ret = env->Object_InstanceOf(obj, cls, &isInstance);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT,
              "[ANI] Failed to check instance of %{public}s, ret: %{public}d",
              className, static_cast<int32_t>(ret));
        return false;
    }
    return static_cast<bool>(isInstance);
}

std::vector<ani_ref> AniWindowUtils::ExtractArrayElements(ani_env* env, ani_object arrayObj)
{
    std::vector<ani_ref> result;
    RETURN_IF_NULL(env, result);
    RETURN_IF_NULL(arrayObj, result);

    if (!IsInstanceOf(env, arrayObj, "std.core.Array")) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] The arrayObj is not instance of std.core.Array");
        return result;
    }

    ani_array aniArray = static_cast<ani_array>(arrayObj);
    ani_int length = 0;
    ani_status ret = env->Object_GetPropertyByName_Int(aniArray, "length", &length);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Failed to get array length, ret: %{public}d", static_cast<int32_t>(ret));
        return result;
    }
    for (ani_int i = 0; i < length; ++i) {
        ani_ref element = nullptr;
        ret = env->Array_Get(aniArray, i, &element);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::DEFAULT,
                  "[ANI] Failed to get array element at index %{public}d, ret: %{public}d",
                  i, static_cast<int32_t>(ret));
            continue;
        }
        result.push_back(element);
    }
    return result;
}

ani_object AniWindowUtils::CreateOptionalBool(ani_env *env, ani_boolean value)
{
    ani_class intClass;
    ani_status ret = env->FindClass("std.core.Boolean", &intClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(intClass, "<ctor>", "z:", &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object obj {};
    if (env->Object_New(intClass, aniCtor, &obj, value) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Failed to allocate Boolean");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    return obj;
}

ani_object AniWindowUtils::CreateOptionalInt(ani_env *env, ani_int value)
{
    ani_class intClass;
    ani_status ret = env->FindClass("std.core.Int", &intClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_method aniCtor;
    ret = env->Class_FindMethod(intClass, "<ctor>", "i:", &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    ani_object obj {};
    if (env->Object_New(intClass, aniCtor, &obj, value) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] Failed to allocate Int");
        return AniWindowUtils::CreateAniUndefined(env);
    }

    return obj;
}

bool AniWindowUtils::ParseSubWindowOptions(ani_env *env, ani_object aniObject, const sptr<WindowOption>& windowOption)
{
    if (aniObject == nullptr || windowOption == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "aniObject or windowOption is null");
        return false;
    }
    ani_ref titleRef;
    env->Object_GetPropertyByName_Ref(aniObject, "title", &titleRef);
    std::string title;
    ani_status ret = GetStdString(env, static_cast<ani_string>(titleRef), title);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to title");
        return false;
    }
    ani_boolean decorEnabled = false;
    env->Object_GetPropertyByName_Boolean(aniObject, "decorEnabled", &decorEnabled);

    // optional
    bool maximizeSupported = false;
    GetPropertyBoolObject(env, "maximizeSupported", aniObject, maximizeSupported);
    bool outlineEnabled = false;
    GetPropertyBoolObject(env, "outlineEnabled", aniObject, outlineEnabled);
    windowOption->SetSubWindowTitle(title);
    windowOption->SetSubWindowDecorEnable(decorEnabled);
    windowOption->SetSubWindowMaximizeSupported(maximizeSupported);
    windowOption->SetSubWindowOutlineEnabled(outlineEnabled);
    if (!ParseRectParam(env, aniObject, windowOption)) {
        return false;
    }
    if (!ParseModalityParam(env, aniObject, windowOption)) {
        return false;
    }
    return ParseZLevelParam(env, aniObject, windowOption);
}

bool AniWindowUtils::ParseRectParam(ani_env *env, ani_object aniObject, const sptr<WindowOption>& windowOption)
{
    if (aniObject == nullptr || windowOption == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "aniObject or windowOption is null");
        return false;
    }
    ani_ref windowRectRef;
    ani_status ret = env->Object_GetPropertyByName_Ref(aniObject, "windowRect", &windowRectRef);
    ani_boolean isUndefined = false;
    env->Reference_IsUndefined(windowRectRef, &isUndefined);
    if (ret != ANI_OK || isUndefined) {
        return true;
    }
    Rect windowRect;
    if (!GetPropertyRectObject(env, "windowRect", aniObject, windowRect)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[ANI] GetPropertyRectObject failed");
        return false;
    }

    if (windowRect.width_ <= 0 || windowRect.height_ <= 0) {
        TLOGE(WmsLogTag::WMS_SUB, "width or height should greater than 0!");
        return false;
    }
    windowOption->SetWindowRect(windowRect);
    TLOGI(WmsLogTag::WMS_SUB, "windowRect: %{public}s", windowRect.ToString().c_str());
    return true;
}

bool AniWindowUtils::ParseModalityParam(ani_env *env, ani_object aniObject, const sptr<WindowOption>& windowOption)
{
    if (aniObject == nullptr || windowOption == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "aniObject or windowOption is null");
        return false;
    }
    bool isModal = false;
    GetPropertyBoolObject(env, "isModal", aniObject, isModal);
    if (isModal) {
        windowOption->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    }

    bool isTopmost = false;
    GetPropertyBoolObject(env, "isTopmost", aniObject, isTopmost);
    if (!isModal && isTopmost) {
        TLOGE(WmsLogTag::WMS_SUB, "Normal subwindow not support topmost");
        return false;
    }
    windowOption->SetWindowTopmost(isTopmost);

    ani_ref modalityTypeRef;
    ani_status ret = env->Object_GetPropertyByName_Ref(aniObject, "modalityType", &modalityTypeRef);
    ani_boolean isUndefined = false;
    env->Reference_IsUndefined(modalityTypeRef, &isUndefined);
    ani_int modalityType;
    if (ret == ANI_OK && !isUndefined &&
        env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(modalityTypeRef), &modalityType) == ANI_OK) {
        if (!isModal) {
            TLOGE(WmsLogTag::WMS_SUB, "Normal subwindow not support modalityType");
            return false;
        }
        using T = std::underlying_type_t<ApiModalityType>;
        T apiModalityType = static_cast<T>(modalityType);
        if (apiModalityType >= static_cast<T>(ApiModalityType::BEGIN) &&
            apiModalityType <= static_cast<T>(ApiModalityType::END)) {
            auto type = JS_TO_NATIVE_MODALITY_TYPE_MAP.at(static_cast<ApiModalityType>(apiModalityType));
            if (type == ModalityType::APPLICATION_MODALITY) {
                windowOption->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL);
            }
        } else {
            TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to modalityType");
            return false;
        }
    }
    TLOGI(WmsLogTag::WMS_SUB, "isModal: %{public}d, isTopmost: %{public}d, WindowFlag: %{public}d",
        isModal, isTopmost, windowOption->GetWindowFlags());
    return true;
}

bool AniWindowUtils::ParseZLevelParam(ani_env *env, ani_object aniObject, const sptr<WindowOption>& windowOption)
{
    if (aniObject == nullptr || windowOption == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "aniObject or windowOption is null");
        return false;
    }
    int zLevel = 0;
    bool isModal = false;
    if (GetPropertyIntObject(env, "zLevel", aniObject, zLevel) == ANI_OK) {
        if (zLevel < MINIMUM_Z_LEVEL || zLevel > MAXIMUM_Z_LEVEL) {
            TLOGE(WmsLogTag::WMS_SUB, "zLevel value %{public}d exceeds valid range [-10000, 10000]!", zLevel);
            return false;
        }
        if (GetPropertyBoolObject(env, "isModal", aniObject, isModal) == ANI_OK) {
            if (isModal) {
                TLOGE(WmsLogTag::WMS_SUB, "modal window not support custom zLevel");
                return false;
            }
        }
        windowOption->SetSubWindowZLevel(zLevel);
    }
    TLOGI(WmsLogTag::WMS_SUB, "zLevel: %{public}d", zLevel);
    return true;
}

bool AniWindowUtils::ParseSubWindowOption(ani_env* env, ani_object jsObject, const sptr<WindowOption>& windowOption)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] null env");
        return false;
    }
    if (jsObject == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "jsObject is null");
        return false;
    }
    if (windowOption == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "windowOption is null");
        return false;
    }
    std::string title;
    ani_ref result;
    ani_status titleResult = env->Object_GetPropertyByName_Ref(jsObject, "title", &result);
    if (titleResult != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to get title");
        return false;
    }
    ani_string aniResult = reinterpret_cast<ani_string>(result);
    ani_status optionFirst = AniWindowUtils::GetStdString(env, aniResult, title);
    if (optionFirst != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to title");
        return false;
    }
    ani_boolean decorEnabled;
    auto ret = env->Object_GetPropertyByName_Boolean(jsObject, "decorEnabled", &decorEnabled);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to decorEnabled");
        return false;
    }
    windowOption->SetSubWindowTitle(title);
    windowOption->SetSubWindowDecorEnable(decorEnabled);
    if (!ParseRectParams(env, jsObject, windowOption)) {
        return false;
    }
    if (!ParseModalityParams(env, jsObject, windowOption)) {
        return false;
    }
    return ParseZLevelParams(env, jsObject, windowOption);
}

bool AniWindowUtils::ParseModalityParams(ani_env* env, ani_object jsObject, const sptr<WindowOption>& windowOption)
{
    ani_boolean isModal { false };
    env->Object_GetPropertyByName_Boolean(jsObject, "isModal", &isModal);
    if (isModal) {
        windowOption->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
    }
    ani_boolean isTopmost { false };
    env->Object_GetPropertyByName_Boolean(jsObject, "isTopmost", &isTopmost);
    if (!isModal && isTopmost) {
        TLOGE(WmsLogTag::WMS_SUB, "Normal subwindow not support topmost");
        return false;
    }
    windowOption->SetWindowTopmost(isTopmost);
    ani_int aniModalityType { 0 };
    auto ret = env->Object_GetPropertyByName_Int(jsObject, "modalityType", &aniModalityType);
    ApiModalityType apiModalityType = static_cast<ApiModalityType>(static_cast<uint32_t>(aniModalityType));
    if (ret == ANI_OK) {
        if (!isModal) {
            TLOGE(WmsLogTag::WMS_SUB, "Normal subwindow not support modalityType");
            return false;
        }
        using T = std::underlying_type_t<ApiModalityType>;
        T modalityType = static_cast<T>(apiModalityType);
        if (modalityType >= static_cast<T>(ApiModalityType::BEGIN) &&
            modalityType <= static_cast<T>(ApiModalityType::END)) {
            auto type = JS_TO_NATIVE_MODALITY_TYPE_MAP.at(apiModalityType);
            if (type == ModalityType::APPLICATION_MODALITY) {
                windowOption->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL);
            }
        } else {
            TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to modalityType");
            return false;
        }
    }
    TLOGI(WmsLogTag::WMS_SUB, "isModal: %{public}d, isTopmost: %{public}d, WindowFlag: %{public}d",
        isModal, isTopmost, windowOption->GetWindowFlags());
    return true;
}

bool AniWindowUtils::ParseZLevelParams(ani_env* env, ani_object jsObject, const sptr<WindowOption>& windowOption)
{
    ani_int zLevel { 0 };
    ani_status ani_zLevel = env->Object_GetPropertyByName_Int(jsObject, "zLevel", &zLevel);
    ani_boolean isModal { 0 };
    ani_status ani_isModal = env->Object_GetPropertyByName_Boolean(jsObject, "isModal", &isModal);
    if (ani_zLevel == ANI_OK) {
        if (zLevel < MINIMUM_Z_LEVEL || zLevel > MAXIMUM_Z_LEVEL) {
            TLOGE(WmsLogTag::WMS_SUB, "zLevel value %{public}d exceeds valid range [-10000, 10000]!", zLevel);
            return false;
        }
        if (ani_isModal == ANI_OK) {
            if (isModal) {
                TLOGE(WmsLogTag::WMS_SUB, "modal window not support custom zLevel");
                return false;
            }
        }
        windowOption->SetSubWindowZLevel(zLevel);
    }
    TLOGI(WmsLogTag::WMS_SUB, "zLevel: %{public}d", zLevel);
    return true;
}

bool AniWindowUtils::ParseRectParams(ani_env* env, ani_object jsObject, const sptr<WindowOption>& windowOption)
{
    ani_ref rectRef;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(jsObject, "windowRect", &rectRef)) {
        TLOGE(WmsLogTag::WMS_SUB, "get windowRect fail");
        return false;
    }
    ani_boolean isUndefined;
    if (ANI_OK != env->Reference_IsUndefined(rectRef, &isUndefined) || isUndefined) {
        TLOGI(WmsLogTag::WMS_SUB, "windowRect is undefined");
        return true;
    }
    Rect windowRect;
    if (!GetPropertyRectObject(env, "windowRect", (ani_object)rectRef, windowRect)) {
        return false;
    }
    if (windowRect.width_ <= 0 || windowRect.height_ <= 0) {
        TLOGE(WmsLogTag::WMS_SUB, "width or height should greater than 0!");
        return false;
    }
    TLOGI(WmsLogTag::WMS_SUB, "windowRect: %{public}s", windowRect.ToString().c_str());
    windowOption->SetWindowRect(windowRect);
    return true;
}

} // namespace Rosen
} // namespace OHOS