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
        ret = env->Object_CallMethodByName_Ref(ary, "$_get", "I:Lstd/core/Object;", &stringRef, ani_int(i));
        if (ret != ANI_OK) {
            return ret;
        }
        std::string str;
        AniWindowUtils::GetStdString(env, static_cast<ani_string>(stringRef), str);
        result.emplace_back(str);
    }
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
    ret = env->Object_CallMethodByName_Boolean(static_cast<ani_object>(bool_ref), "unboxed", ":Z", &bool_value);
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
    ret = env->Object_CallMethodByName_Long(static_cast<ani_object>(long_ref), "unboxed", ":J", &long_value);
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
    env->Array_GetRegion_Double(reinterpret_cast<ani_array_double>(ary), 0, size, native_buffer.get());

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
    ret = env->Class_FindMethod(aniClass, "<ctor>", ":V", &aniCtor);
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

    CallAniMethodVoid(env, aniLimits, aniClass, "<set>maxWidth", nullptr,
        CreateBaseTypeObject<int>(env, windowLimits.maxWidth_));
    CallAniMethodVoid(env, aniLimits, aniClass, "<set>maxHeight", nullptr,
        CreateBaseTypeObject<int>(env, windowLimits.maxHeight_));
    CallAniMethodVoid(env, aniLimits, aniClass, "<set>minWidth", nullptr,
        CreateBaseTypeObject<int>(env, windowLimits.minWidth_));
    CallAniMethodVoid(env, aniLimits, aniClass, "<set>minHeight", nullptr,
        CreateBaseTypeObject<int>(env, windowLimits.minHeight_));
    return aniLimits;
}

ani_object AniWindowUtils::CreateAniAvoidArea(ani_env* env, const AvoidArea& avoidArea,
    AvoidAreaType type, bool useActualVisibility)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("L@ohos/window/window/AvoidAreaInternal;", &aniClass);
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

ani_object AniWindowUtils::CreateAniSystemBarTintState(ani_env* env, DisplayId displayId,
    const SystemBarRegionTints& tints)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_class aniClass;
    if (env->FindClass("L@ohos/window/window/SystemBarTintStateInternal;", &aniClass) != ANI_OK) {
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
    ani_array_ref regionTintArray = nullptr;
    ani_class regionTintCls;
    if (env->FindClass("L@ohos/window/window/SystemBarRegionTintInternal;", &regionTintCls) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    if (env->Array_New_Ref(regionTintCls, tints.size(), CreateAniUndefined(env), &regionTintArray) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] create array failed");
        return AniWindowUtils::CreateAniUndefined(env);
    }
    for (size_t i = 0; i < tints.size(); i++) {
        if (env->Array_Set_Ref(regionTintArray, i, CreateAniSystemBarRegionTint(env, tints[i])) != ANI_OK) {
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
    if (env->FindClass("L@ohos/window/window/SystemBarRegionTintInternal;", &regionTintCls) != ANI_OK) {
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

ani_object AniWindowUtils::CreateAniRotationChangeInfo(ani_env* env, const RotationChangeInfo& info)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "[ANI]");
    ani_class aniClass;
    ani_status ret = env->FindClass("L@ohos/window/window/RotationChangeInfoInternal;", &aniClass);
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
    ret = env->FindEnum("L@ohos/window/window/RotationChangeType;", &rotationChangeType);
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
    ani_status ret = env->FindClass("L@ohos/window/window/KeyboardInfoInternal;", &aniClass);
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
    CallAniMethodVoid(env, keyboardInfo, aniClass, "<set>beginRect", nullptr,
        CreateAniRect(env, keyboardPanelInfo.beginRect_));
    CallAniMethodVoid(env, keyboardInfo, aniClass, "<set>endRect", nullptr,
        CreateAniRect(env, keyboardPanelInfo.endRect_));
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
    return InitAniObjectByCreator(env, "L@ohos/window/window/RectInternal;", "iiii:",
        ani_int(rect.posX_), ani_int(rect.posY_),
        ani_int(rect.width_), ani_int(rect.height_));
}

ani_object AniWindowUtils::CreateWindowsProperties(ani_env* env, const WindowPropertyInfo& windowPropertyInfo)
{
    ani_object aniRect = CreateAniRectObject(env, windowPropertyInfo.windowRect);
    ani_object aniDrawableRect = CreateAniRectObject(env, windowPropertyInfo.drawableRect);
    int windowType;
    if (NATIVE_JS_TO_WINDOW_TYPE_MAP.count(windowPropertyInfo.type) != 0) {
        windowType = static_cast<int>(NATIVE_JS_TO_WINDOW_TYPE_MAP.at(windowPropertyInfo.type));
    } else {
        windowType = static_cast<int>(windowPropertyInfo.type);
    }
    ani_string aniWindowName;
    GetAniString(env, windowPropertyInfo.name, &aniWindowName);
    ani_object aniWindowsProperties = InitAniObjectByCreator(env, "L@ohos/window/window/WindowPropertiesInternal;",
        "L@ohos/window/window/Rect;L@ohos/window/window/Rect;ZZZZDDZZZZIIJLstd/core/String;:V",
        aniRect, aniDrawableRect, ani_boolean(windowPropertyInfo.isFullScreen),
        ani_boolean(windowPropertyInfo.isLayoutFullScreen), ani_boolean(windowPropertyInfo.isFocusable),
        ani_boolean(windowPropertyInfo.isTouchable), ani_float(windowPropertyInfo.brightness), ani_float(0),
        ani_boolean(windowPropertyInfo.isKeepScreenOn), ani_boolean(windowPropertyInfo.isPrivacyMode),
        ani_boolean(false),  ani_boolean(windowPropertyInfo.isTransparent), ani_int(windowType),
        ani_int(windowPropertyInfo.id), ani_long(windowPropertyInfo.displayId), aniWindowName);
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
    if (env->FindClass("L@ohos/multimedia/image/image/PixelMap;", &cls) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] class not found");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    ani_array_ref pixelMapArray = nullptr;
    if (env->Array_New_Ref(cls, pixelMaps.size(), CreateAniUndefined(env), &pixelMapArray) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] create array failed");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    for (size_t i = 0; i < pixelMaps.size(); i++) {
        auto nativePixelMap = OHOS::Media::PixelMapTaiheAni::CreateEtsPixelMap(env, pixelMaps[i]);
        if (nativePixelMap == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "Create native pixelmap is nullptr!");
            return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
        }
        if (env->Array_Set_Ref(pixelMapArray, i, nativePixelMap) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_LIFE, "[ANI] create pixelMapArray failed");
            return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
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
    ani_array_ref mainWindowInfoArray = nullptr;
    if (env->Array_New_Ref(aniClass, infos.size(), CreateAniUndefined(env), &mainWindowInfoArray) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "[ANI] create array failed");
        return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    for (size_t i = 0; i < infos.size(); i++) {
        if (!infos[i]) {
            TLOGE(WmsLogTag::WMS_LIFE, "[ANI] infos[i] is nullptr at index %{public}d", static_cast<int32_t>(i));
            return AniWindowUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
        }
        if (env->Array_Set_Ref(mainWindowInfoArray, i, CreateAniMainWindowInfo(env, *infos[i])) != ANI_OK) {
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
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "[ANI] env is nullptr");
        return AniWindowUtils::CreateAniUndefined(env);
    }
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
    env->Object_CallMethodByName_Boolean(static_cast<ani_object>(nativeObj), "unboxed", ":z", &value);
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
        env->Object_CallMethodByName_Boolean(static_cast<ani_object>(aniStatusIcon), "unboxed", ":z", &isStatusIcon);
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
            "unboxed", ":z", &isNavigationIcon);
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
            "unboxed", "z", &isStatusAnimation);
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
            "unboxed", "z", &isNavigationAnimation);
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

bool AniWindowUtils::SetSpecificSystemBarEnabled(ani_env* env,
                                                 std::map<WindowType,
                                                 SystemBarProperty>& systemBarProperties,
                                                 ani_string aniName,
                                                 ani_boolean aniEnable,
                                                 ani_boolean aniEnableAnimation)
{
    std::string barName;
    GetStdString(env, aniName, barName);
    bool enable = static_cast<bool>(aniEnable);
    bool enableAnimation = static_cast<bool>(aniEnableAnimation);

    if (barName.compare("status") == 0) {
        systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = enable;
        systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_ = enableAnimation;
    } else if (barName.compare("navigation") == 0) {
        systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = enable;
        systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableAnimation_ = enableAnimation;
    } else if (barName.compare("navigationIndicator") == 0) {
        systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enable_ = enable;
        systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enableAnimation_ = enableAnimation;
    }
    return true;
}

void* AniWindowUtils::GetAbilityContext(ani_env *env, ani_object aniObj)
{
    ani_long nativeContextLong;
    ani_class cls = nullptr;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass("Lapplication/UIAbilityContext/UIAbilityContext;", &cls)) != ANI_OK) {
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

void AniWindowUtils::GetSpecificBarStatus(sptr<Window>& window, const std::string& name,
    std::map<WindowType, SystemBarProperty>& newSystemBarProperties,
    std::map<WindowType, SystemBarProperty>& systemBarProperties)
{
    auto type = (name.compare("status") == 0) ? WindowType::WINDOW_TYPE_STATUS_BAR :
                (name.compare("navigation") == 0) ? WindowType::WINDOW_TYPE_NAVIGATION_BAR :
                WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR;
    auto property = window->GetSystemBarPropertyByType(type);
    systemBarProperties[type] = property;
    systemBarProperties[type].enable_ = newSystemBarProperties[type].enable_;
    systemBarProperties[type].enableAnimation_ = newSystemBarProperties[type].enableAnimation_;
    systemBarProperties[type].settingFlag_ = systemBarProperties[type].settingFlag_ |
        SystemBarSettingFlag::ENABLE_SETTING;
}

WindowLimits AniWindowUtils::ParseWindowLimits(ani_env* env, ani_object aniWindowLimits)
{
    WindowLimits windowLimits;
    
    auto getAndAssign = [&, where = __func__](const char* name, uint32_t& field) {
        int value;
        ani_status ret = AniWindowUtils::GetPropertyIntObject(env, name, aniWindowLimits, value);
        if (ret == ANI_OK) {
            if (value >= 0) {
                field = static_cast<uint32_t>(value);
            } else {
                field = 0;
                TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: [ANI] Invalid %{public}s: %{public}d", where, name, value);
            }
        }
    };

    getAndAssign("maxWidth", windowLimits.maxWidth_);
    getAndAssign("maxHeight", windowLimits.maxHeight_);
    getAndAssign("minWidth", windowLimits.minWidth_);
    getAndAssign("minHeight", windowLimits.minHeight_);
    return windowLimits;
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
    ret = env->Reference_IsUndefined(modalityTypeRef, &isUndefined);
    ani_int modalityType;
    if (ret == ANI_OK && !isUndefined &&
        env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(modalityTypeRef), &modalityType)) {
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
} // namespace Rosen
} // namespace OHOS