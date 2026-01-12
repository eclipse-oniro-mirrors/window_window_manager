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
 
#include <hitrace_meter.h>
 
#include "ani.h"
#include <ani_signature_builder.h>
#include "screenshot_ani_utils.h"
#include "singleton_container.h"
#include "window_manager_hilog.h"
#include "dm_common.h"
#include "refbase.h"
 
namespace OHOS::Rosen {

static const uint32_t PIXMAP_VECTOR_ONLY_SDR_SIZE = 1;
static const uint32_t PIXMAP_VECTOR_SIZE = 2;
static const uint32_t HDR_PIXMAP = 1;

using namespace arkts::ani_signature;

ani_status ScreenshotAniUtils::GetStdString(ani_env* env, ani_string ani_str, std::string& result)
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
 
ani_object ScreenshotAniUtils::CreateAniUndefined(ani_env* env)
{
    ani_ref aniRef;
    env->GetUndefined(&aniRef);
    return static_cast<ani_object>(aniRef);
}

ani_object ScreenshotAniUtils::CreateRectObject(ani_env* env)
{
    ani_class aniClass{};
    ani_status status = env->FindClass("@ohos.screenshot.screenshot.RectImpl", &aniClass);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] class not found, status:%{public}d", static_cast<int32_t>(status));
        return nullptr;
    }
    ani_method aniCtor;
    auto ret = env->Class_FindMethod(aniClass, "<ctor>", ":", &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Class_FindMethod failed");
        return nullptr;
    }
    ani_object rectObj;
    status = env->Object_New(aniClass, aniCtor, &rectObj);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Object_New failed");
        return nullptr;
    }
    return rectObj;
}

void ScreenshotAniUtils::ConvertRect(ani_env* env, Media::Rect rect, ani_object rectObj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] rect area info: %{public}d, %{public}d, %{public}u, %{public}u",
        rect.left, rect.width, rect.top, rect.height);
    env->Object_SetFieldByName_Long(rectObj, Builder::BuildPropertyName("left").c_str(), rect.left);
    env->Object_SetFieldByName_Long(rectObj, Builder::BuildPropertyName("top").c_str(), rect.top);
    env->Object_SetFieldByName_Long(rectObj, Builder::BuildPropertyName("width").c_str(), rect.width);
    env->Object_SetFieldByName_Long(rectObj, Builder::BuildPropertyName("height").c_str(), rect.height);
}

ani_object ScreenshotAniUtils::CreateScreenshotPickInfo(ani_env* env, std::unique_ptr<Param>& param)
{
    ani_class aniClass{};
    ani_status status = env->FindClass("@ohos.screenshot.screenshot.PickInfoImpl", &aniClass);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] class not found, status:%{public}d", static_cast<int32_t>(status));
        return nullptr;
    }
    ani_method aniCtor;
    auto ret = env->Class_FindMethod(aniClass, "<ctor>", ":", &aniCtor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Class_FindMethod failed");
        return nullptr;
    }
    ani_object pickInfoObj;
    status = env->Object_New(aniClass, aniCtor, &pickInfoObj);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Object_New failed");
        return nullptr;
    }
    ani_object rectObj = ScreenshotAniUtils::CreateRectObject(env);
    ScreenshotAniUtils::ConvertRect(env, param->imageRect, rectObj);
    env->Object_SetFieldByName_Ref(
        pickInfoObj, Builder::BuildPropertyName("pickRect").c_str(), static_cast<ani_ref>(rectObj));

    auto nativePixelMap = Media::PixelMapTaiheAni::CreateEtsPixelMap(env, param->image);
    env->Object_SetFieldByName_Ref(
        pickInfoObj, Builder::BuildPropertyName("pixelMap").c_str(), static_cast<ani_ref>(nativePixelMap));
    return pickInfoObj;
}

ani_status ScreenshotAniUtils::GetAniString(ani_env* env, const std::string& str, ani_string* result)
{
    return env->String_NewUTF8(str.c_str(), static_cast<ani_size>(str.size()), result);
}
 
void ScreenshotAniUtils::ConvertScreenshot(ani_env* env, std::shared_ptr<Media::PixelMap> image, ani_object obj)
{
    return;
}
 
ani_status ScreenshotAniUtils::CallAniFunctionVoid(ani_env* env, const char* ns,
    const char* fn, const char* signature, ...)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI] begin");
    ani_status ret = ANI_OK;
    ani_namespace aniNamespace{};
    if ((ret = env->FindNamespace(ns, &aniNamespace)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]canot find ns %{public}d", ret);
        return ret;
    }
    ani_function func{};
    if ((ret = env->Namespace_FindFunction(aniNamespace, fn, signature, &func)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]canot find callBack %{public}d", ret);
        return ret;
    }
    if (func == nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "[ANI] null func ani");
        return ret;
    }
    va_list args;
    va_start(args, signature);
    TLOGI(WmsLogTag::DEFAULT, "[ANI]CallAniFunctionVoid begin %{public}s", signature);
    ret = env->Function_Call_Void_V(func, args);
    va_end(args);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]canot run callBack %{public}d", ret);
        return ret;
    }
    return ret;
}
 
ani_status ScreenshotAniUtils::GetScreenshotParam(ani_env* env, const std::unique_ptr<Param>& param,
    ani_object options)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    if (param == nullptr) {
        TLOGI(WmsLogTag::DMS, "[ANI] null param");
        return ANI_INVALID_ARGS;
    }
    ani_long displayId = 0;
    ani_status ret = ReadOptionalLongField(env, options, "displayId", displayId);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get displayId failed");
        return ret;
    }
    param->option.displayId = static_cast<DisplayId>(displayId);
    
    ret = ReadOptionalIntField(env, options, "rotation", param->option.rotation);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get rotation failed");
        return ret;
    }

    ret = ReadOptionalBoolField(env, options, "isNotificationNeeded", param->option.isNeedNotify);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get isNotificationNeeded failed");
        return ret;
    }

    ret = ReadOptionalBoolField(env, options, "isCaptureFullOfScreen", param->option.isCaptureFullOfScreen);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get isCaptureFullOfScreen failed");
        return ret;
    }

    ret = GetScreenshotSize(env, param, options);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get screenshot size failed");
        return ret;
    }
    ret = GetScreenshotRect(env, param, options);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get screenshot rect failed");
        return ret;
    }
    return ANI_OK;
}

ani_object ScreenshotAniUtils::CreateArrayPixelMap(
    ani_env* env, std::vector<std::shared_ptr<Media::PixelMap>> imageVec)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    ani_class cls;
    if (env->FindClass("@ohos.multimedia.image.image.PixelMap", &cls) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] class not found");
        return nullptr;
    }
    ani_size arrayLength = static_cast<ani_size>(PIXMAP_VECTOR_ONLY_SDR_SIZE);
    if (imageVec[HDR_PIXMAP] != nullptr) {
        arrayLength = static_cast<ani_size>(PIXMAP_VECTOR_SIZE);
    }
    ani_array pixelMapArray = nullptr;
    if (env->Array_New(arrayLength, ScreenshotAniUtils::CreateAniUndefined(env), &pixelMapArray) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] create array failed");
        return nullptr;
    }
    for (size_t i = 0; i < arrayLength; i++) {
        auto nativePixelMap = Media::PixelMapTaiheAni::CreateEtsPixelMap(env, imageVec[i]);
        if (nativePixelMap == nullptr) {
            TLOGE(WmsLogTag::DMS, "[ANI] Create native pixelmap is nullptr!");
            return nullptr;
        }
        if (env->Array_Set(pixelMapArray, i, nativePixelMap) != ANI_OK) {
            TLOGE(WmsLogTag::DMS, "[ANI] create pixelMapArray failed!");
            return nullptr;
        }
    }
    return pixelMapArray;
}
 
ani_status ScreenshotAniUtils::GetHdrScreenshotParam(ani_env* env, const std::unique_ptr<HdrParam>& param,
    ani_object options)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    if (param == nullptr) {
        TLOGI(WmsLogTag::DMS, "[ANI] null param");
        return ANI_INVALID_ARGS;
    }
    ani_long displayId = 0;
    ani_status ret = ReadOptionalLongField(env, options, "displayId", displayId);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get displayId failed");
        return ret;
    }
    param->option.displayId = static_cast<DisplayId>(displayId);
 
    ret = ReadOptionalBoolField(env, options, "isNotificationNeeded", param->option.isNeedNotify);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get isNotificationNeeded failed");
        return ret;
    }
 
    ret = ReadOptionalBoolField(env, options, "isCaptureFullOfScreen", param->option.isCaptureFullOfScreen);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get isCaptureFullOfScreen failed");
        return ret;
    }
 
    return ANI_OK;
}

ani_status ScreenshotAniUtils::GetScreenshotSize(ani_env* env, const std::unique_ptr<Param>& param, ani_object options)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    ani_ref obj;
    ani_status ret = env->Object_GetPropertyByName_Ref(options, "imageSize", &obj);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get imageSize failed");
        return ret;
    }
    ani_boolean isUndefRes = false;
    ret = env->Reference_IsUndefined(obj, &isUndefRes);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] check imageSize is undefined failed");
        return ret;
    }
    if (isUndefRes) {
        TLOGI(WmsLogTag::DMS, "[ANI] imageSize is undefined");
        return ANI_OK;
    }
    ani_long width;
    ani_long height;
    env->Object_GetPropertyByName_Long(static_cast<ani_object>(obj), "width", &width);
    env->Object_GetPropertyByName_Long(static_cast<ani_object>(obj), "height", &height);
    param->option.size.width = static_cast<int32_t>(width);
    param->option.size.height = static_cast<int32_t>(height);
    TLOGI(WmsLogTag::DMS, "[ANI] get imageSize width = %{public}d, height = %{public}d",
        param->option.size.width, param->option.size.height);
    return ANI_OK;
}

ani_status ScreenshotAniUtils::GetScreenshotRect(ani_env* env, const std::unique_ptr<Param>& param, ani_object options)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    ani_ref obj;
    ani_status ret = env->Object_GetPropertyByName_Ref(options, "screenRect", &obj);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get screenRect failed");
        return ret;
    }
    ani_boolean isUndefRes = false;
    ret = env->Reference_IsUndefined(obj, &isUndefRes);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] check screenRect is undefined failed");
        return ret;
    }
    if (isUndefRes) {
        TLOGI(WmsLogTag::DMS, "[ANI] screenRect is undefined");
        return ANI_OK;
    }
    ani_long left = 0;
    ani_long top = 0;
    ani_long width = 0;
    ani_long height = 0;
    env->Object_GetPropertyByName_Long(static_cast<ani_object>(obj), "left", &left);
    env->Object_GetPropertyByName_Long(static_cast<ani_object>(obj), "top", &top);
    env->Object_GetPropertyByName_Long(static_cast<ani_object>(obj), "width", &width);
    env->Object_GetPropertyByName_Long(static_cast<ani_object>(obj), "height", &height);
    param->option.rect.left = static_cast<int32_t>(left);
    param->option.rect.top = static_cast<int32_t>(top);
    param->option.rect.width = static_cast<int32_t>(width);
    param->option.rect.height = static_cast<int32_t>(height);
    TLOGI(WmsLogTag::DMS,
        "[ANI] get screenRect left = %{public}d, top = %{public}d, width = %{public}d, height = %{public}d",
        param->option.rect.left, param->option.rect.top, param->option.rect.width, param->option.rect.height);
    return ANI_OK;
}

ani_status ScreenshotAniUtils::ReadOptionalField(ani_env* env, ani_object obj, const char* fieldName, ani_ref& ref)
{
    ani_status ret = env->Object_GetPropertyByName_Ref(obj, fieldName, &ref);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get property %{public}s, ret %{public}d", fieldName, ret);
        return ret;
    }
    ani_boolean isUndefRes;
    ret = env->Reference_IsUndefined(ref, &isUndefRes);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to check ref is undefined, ret %{public}d", ret);
        ref = nullptr;
        return ret;
    }
    if (isUndefRes) {
        ref = nullptr;
    }
    return ret;
}

ani_status ScreenshotAniUtils::ReadOptionalLongField(ani_env* env, ani_object obj, const char* fieldName,
    ani_long& value)
{
    ani_ref ref = nullptr;
    ani_status result = ReadOptionalField(env, obj, fieldName, ref);
    if (result == ANI_OK && ref != nullptr) {
        result = env->Object_CallMethodByName_Long(reinterpret_cast<ani_object>(ref), "toLong", ":l", &value);
    }
    return result;
}


ani_status ScreenshotAniUtils::ReadOptionalIntField(ani_env* env, ani_object obj, const char* fieldName, int& value)
{
    ani_ref ref = nullptr;
    ani_status result = ReadOptionalField(env, obj, fieldName, ref);
    if (result == ANI_OK && ref != nullptr) {
        result = env->Object_CallMethodByName_Int(reinterpret_cast<ani_object>(ref), "toInt", ":i", &value);
    }
    return result;
}

ani_status ScreenshotAniUtils::ReadOptionalBoolField(ani_env* env, ani_object obj, const char* fieldName, bool& value)
{
    ani_ref ref = nullptr;
    ani_status result = ReadOptionalField(env, obj, fieldName, ref);
    if (result == ANI_OK && ref != nullptr) {
        ani_boolean aniBool;
        result = env->Object_CallMethodByName_Boolean(reinterpret_cast<ani_object>(ref), "toBoolean", ":z", &aniBool);
        if (result == ANI_OK) {
            value = static_cast<bool>(aniBool);
        }
    }
    return result;
}
}