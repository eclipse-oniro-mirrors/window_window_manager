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

#include "screen_ani_utils.h"

#include <hitrace_meter.h>

#include "ani.h"
#include <ani_signature_builder.h>
#include "dm_common.h"
#include "refbase.h"
#include "screen.h"
#include "screen_ani.h"
#include "screen_info.h"
#include "screen_manager.h"
#include "singleton_container.h"
#include "window_manager_hilog.h"
#include "surface_utils.h"

namespace OHOS {
namespace Rosen {
using namespace arkts::ani_signature;

ani_status ScreenAniUtils::GetStdString(ani_env *env, ani_string ani_str, std::string &result)
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

ani_object ScreenAniUtils::CreateAniUndefined(ani_env* env)
{
    ani_ref aniRef;
    env->GetUndefined(&aniRef);
    return static_cast<ani_object>(aniRef);
}

ani_status ScreenAniUtils::GetAniString(ani_env* env, const std::string& str, ani_string* result)
{
    return env->String_NewUTF8(str.c_str(), static_cast<ani_size>(str.size()), result);
}

ani_status ScreenAniUtils::ConvertScreen(ani_env *env, sptr<Screen> screen, ani_object obj)
{
    sptr<ScreenInfo> info = screen->GetScreenInfo();
    TLOGI(WmsLogTag::DMS, "[ANI] convert screen id %{public}u", static_cast<uint32_t>(info->GetScreenId()));
    env->Object_SetFieldByName_Long(obj, Builder::BuildPropertyName("id").c_str(),
        static_cast<ani_long>(info->GetScreenId()));
    env->Object_SetFieldByName_Long(obj, Builder::BuildPropertyName("parent").c_str(),
        static_cast<ani_long>(info->GetParentId()));
    env->Object_SetFieldByName_Long(obj, Builder::BuildPropertyName("activeModeIndex").c_str(),
        static_cast<ani_long>(info->GetModeId()));
    env->Object_SetFieldByName_Ref(obj, Builder::BuildPropertyName("orientation").c_str(),
        ScreenAniUtils::CreateAniEnum(env, "@ohos.screen.screen.Orientation",
        static_cast<ani_int>(info->GetOrientation())));
    env->Object_SetFieldByName_Ref(obj, Builder::BuildPropertyName("sourceMode").c_str(),
        ScreenAniUtils::CreateAniEnum(env, "@ohos.screen.screen.ScreenSourceMode",
        static_cast<ani_int>(info->GetSourceMode())));
    std::unique_ptr<ScreenAni> screenAni = std::make_unique<ScreenAni>(screen);
    if (ANI_OK != env->Object_SetFieldByName_Long(obj, "screenNativeObj",
        reinterpret_cast<ani_long>(screenAni.release()))) {
        TLOGE(WmsLogTag::DMS, "[ANI] set screenNativeObj fail");
        return ANI_ERROR;
    }
    std::vector<sptr<SupportedScreenModes>> modes = info->GetModes();
    ani_array screenModeInfos = NewNativeArray(env, "@ohos.screen.screen.ScreenModeInfoImpl",
        static_cast<uint32_t>(modes.size()));
    ani_size index = 0;
    for (auto mode : modes) {
        ani_object screenModeInfo = NewNativeObject(env, "@ohos.screen.screen.ScreenModeInfoImpl");
        ConvertScreenMode(env, mode, screenModeInfo);
        if (ANI_OK != env->Array_Set(screenModeInfos, index, screenModeInfo)) {
            TLOGE(WmsLogTag::DMS, "[ANI] Array_Set_Ref fail");
            return ANI_ERROR;
        }
        index++;
    }
    auto ret = env->Object_SetFieldByName_Ref(obj, Builder::BuildPropertyName("supportedModeInfo").c_str(),
        static_cast<ani_ref>(screenModeInfos));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get ScreenModeInfos fail, ret: %{public}d", ret);
        return ANI_ERROR;
    }
    return ANI_OK;
}

ani_array ScreenAniUtils::NewNativeArray(ani_env* env, const std::string& objName, uint32_t size)
{
    ani_array array = nullptr;

    if (env->Array_New(size, CreateAniUndefined(env), &array) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] create array failed");
    }
    return array;
}

ani_object ScreenAniUtils::NewNativeObject(ani_env* env, const std::string& objName)
{
    ani_class cls;
    if (env->FindClass(objName.c_str(), &cls) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] class not found");
        return CreateAniUndefined(env);
    }
    ani_method ctor;
    if (env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] ctor not found");
        return CreateAniUndefined(env);
    }
    ani_object obj;
    if (env->Object_New(cls, ctor, &obj) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] fail to new obj");
        return CreateAniUndefined(env);
    }
    return obj;
}

void ScreenAniUtils::ConvertScreenMode(ani_env* env, sptr<SupportedScreenModes> mode, ani_object obj)
{
    env->Object_SetFieldByName_Long(obj, Builder::BuildPropertyName("id").c_str(),
        static_cast<ani_long>(mode->id_));
    env->Object_SetFieldByName_Long(obj, Builder::BuildPropertyName("width").c_str(),
        static_cast<ani_long>(mode->width_));
    env->Object_SetFieldByName_Long(obj, Builder::BuildPropertyName("height").c_str(),
        static_cast<ani_long>(mode->height_));
    env->Object_SetFieldByName_Int(obj, Builder::BuildPropertyName("refreshRate").c_str(),
        static_cast<ani_int>(mode->refreshRate_));
}

ani_status ScreenAniUtils::ConvertScreens(ani_env *env, std::vector<sptr<Screen>> screens, ani_object& screensAni)
{
    TLOGI(WmsLogTag::DMS, "[ANI] screens size %{public}u", static_cast<uint32_t>(screens.size()));
    for (uint32_t i = 0; i < screens.size(); i++) {
        ani_ref currentScreenAni;
        if (ANI_OK != env->Object_CallMethodByName_Ref(screensAni, "$_get", "i:Y",
            &currentScreenAni, (ani_int)i)) {
            TLOGE(WmsLogTag::DMS, "[ANI] get ani_array index %{public}u fail", (ani_int)i);
            return ANI_ERROR;
        }
        if (ANI_OK != ConvertScreen(env, screens[i], static_cast<ani_object>(currentScreenAni))) {
            TLOGE(WmsLogTag::DMS, "[ANI] get convert screenAni index %{public}u fail", (ani_int)i);
            return ANI_ERROR;
        }
    }
    return ANI_OK;
}

ani_status ScreenAniUtils::CallAniFunctionVoid(ani_env *env, const char* ns,
    const char* fn, const char* signature, ...)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    ani_status ret = ANI_OK;
    ani_namespace aniNamespace{};
    if ((ret = env->FindNamespace(ns, &aniNamespace)) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI]canot find ns %{public}d", ret);
        return ret;
    }
    ani_function func{};
    if ((ret = env->Namespace_FindFunction(aniNamespace, fn, signature, &func)) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI]canot find callBack %{public}d", ret);
        return ret;
    }
    if (func == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] null func ani");
        return ret;
    }
    va_list args;
    va_start(args, signature);
    TLOGI(WmsLogTag::DMS, "[ANI]CallAniFunctionVoid begin %{public}s", signature);
    ret = env->Function_Call_Void_V(func, args);
    va_end(args);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI]canot run callBack %{public}d", ret);
        return ret;
    }
    return ret;
}

ani_enum_item ScreenAniUtils::CreateAniEnum(ani_env* env, const char* enum_descriptor, ani_size index)
{
    ani_enum enumType;
    ani_status ret = env->FindEnum(enum_descriptor, &enumType);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Failed to find enum, %{public}s", enum_descriptor);
        return nullptr;
    }
    ani_enum_item enumItem;
    env->Enum_GetEnumItemByIndex(enumType, index, &enumItem);
    return enumItem;
}

static DmErrorCode GetScreenFocusFromAni(ani_env* env, ani_object virtualScreenObj, VirtualScreenOption& option)
{
    ani_ref focus = nullptr;
    if (env->Object_GetPropertyByName_Ref(virtualScreenObj, "%%property-supportsFocus", &focus) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get supportsFocus.");
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }
    ani_boolean isUndefined;
    env->Reference_IsUndefined(focus, &isUndefined);
    if (isUndefined) {
        TLOGD(WmsLogTag::DMS, "supportsFocus is undefined.");
    } else {
        ani_boolean result;
        env->Object_CallMethodByName_Boolean(static_cast<ani_object>(focus), "toBoolean", ":z", &result);
        option.supportsFocus_ = static_cast<bool>(result);
        TLOGD(WmsLogTag::DMS, "Convert supportsFocus:%{public}d", option.supportsFocus_);
    }
    return DmErrorCode::DM_OK;
}

DmErrorCode ScreenAniUtils::GetVirtualScreenOption(ani_env* env, ani_object options, VirtualScreenOption& option)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    ani_ref nameRef = nullptr;
    auto ret = env->Object_GetFieldByName_Ref(options, Builder::BuildPropertyName("name").c_str(), &nameRef);
    if (ret != ANI_OK || nameRef == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get nameRef, ret:%{public}d", ret);
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }
    ret = GetStdString(env, static_cast<ani_string>(nameRef), option.name_);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get name, ret:%{public}d", ret);
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }

    ani_long widthAni = 0;
    ret = env->Object_GetFieldByName_Long(options, Builder::BuildPropertyName("width").c_str(), &widthAni);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get width, ret:%{public}d", ret);
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }
    option.width_ = static_cast<uint32_t>(widthAni);
    ani_long heightAni = 0;
    ret = env->Object_GetFieldByName_Long(options, Builder::BuildPropertyName("height").c_str(), &heightAni);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get height, ret:%{public}d", ret);
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }
    option.height_ = static_cast<uint32_t>(heightAni);

    ani_double densityAni = 0;
    ret = env->Object_GetFieldByName_Double(options, Builder::BuildPropertyName("density").c_str(), &densityAni);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get density, ret:%{public}d", ret);
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }
    option.density_ = static_cast<float>(densityAni);

    ani_ref surfaceIdRef = nullptr;
    ret = env->Object_GetFieldByName_Ref(options, Builder::BuildPropertyName("surfaceId").c_str(), &surfaceIdRef);
    if (ret != ANI_OK || surfaceIdRef == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get surfaceIdRef, ret:%{public}d", ret);
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }

    ret = GetSurfaceFromAni(env, static_cast<ani_string>(surfaceIdRef), option.surface_);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get surface, ret:%{public}d", ret);
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }
    return GetScreenFocusFromAni(env, options, option);
}

ani_status ScreenAniUtils::GetSurfaceFromAni(ani_env* env, ani_string surfaceIdAniStr, sptr<Surface>& surface)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    std::string surfaceIdStr;
    auto ret = GetStdString(env, surfaceIdAniStr, surfaceIdStr);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get surface, ret:%{public}d", ret);
        return ret;
    }
    uint64_t surfaceId = 0;
    std::istringstream inputStream(surfaceIdStr);
    inputStream >> surfaceId;
    surface = SurfaceUtils::GetInstance()->GetSurface(surfaceId);
    if (surface == nullptr) {
        TLOGI(WmsLogTag::DMS, "GetSurfaceFromAni failed, surfaceId:%{public}" PRIu64, surfaceId);
    }
    return ANI_OK;
}

ani_status ScreenAniUtils::GetMultiScreenPositionOptionsFromAni(ani_env* env, ani_object screenOptionsAni,
    MultiScreenPositionOptions& mainScreenOptions)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    ani_long screenIdAni = 0;
    auto ret = env->Object_GetFieldByName_Long(screenOptionsAni,
        Builder::BuildPropertyName("id").c_str(), &screenIdAni);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get screenId, ret:%{public}d", ret);
        return ret;
    }
    mainScreenOptions.screenId_ = static_cast<ScreenId>(screenIdAni);

    ani_long startXAni = 0;
    ret = env->Object_GetFieldByName_Long(screenOptionsAni,
        Builder::BuildPropertyName("startX").c_str(), &startXAni);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get startX, ret:%{public}d", ret);
        return ret;
    }
    mainScreenOptions.startX_ = static_cast<uint32_t>(startXAni);

    ani_long startYAni = 0;
    ret = env->Object_GetFieldByName_Long(screenOptionsAni,
        Builder::BuildPropertyName("startY").c_str(), &startYAni);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get startY, ret:%{public}d", ret);
        return ret;
    }
    mainScreenOptions.startY_ = static_cast<uint32_t>(startYAni);
    return ANI_OK;
}

ani_object ScreenAniUtils::CreateDisplayIdVectorAniObject(ani_env* env, std::vector<DisplayId>& displayIds)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    ani_object arrayObj = ScreenAniUtils::CreateAniArray(env, displayIds.size());
    ani_boolean isUndefined = false;
    env->Reference_IsUndefined(arrayObj, &isUndefined);
    if (isUndefined) {
        TLOGE(WmsLogTag::DMS, "Failed to create arrayObject");
        return arrayObj;
    }
    ani_size index = 0;
    for (const auto& displayId : displayIds) {
        TLOGI(WmsLogTag::DMS, "displayId: %{public}" PRIu64, displayId);
        ani_status ret = env->Object_CallMethodByName_Void(arrayObj, "$_set", "il:", index,
            static_cast<ani_long>(displayId));
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::DMS, "Failed to set displayId item, index: %{public}zu, ret: %{public}d", index, ret);
            continue;
        }
        index++;
    }
    return arrayObj;
}

ani_object ScreenAniUtils::CreateAniArray(ani_env* env, size_t size)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    ani_class arrayCls;
    if (env->FindClass("std.core.Array", &arrayCls) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to find class std.core.Array");
        return CreateAniUndefined(env);
    }
    ani_method arrayCtor;
    if (env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to find <ctor>");
        return CreateAniUndefined(env);
    }
    ani_object arrayObj = nullptr;
    if (env->Object_New(arrayCls, arrayCtor, &arrayObj, size) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to create object Array");
        return CreateAniUndefined(env);
    }
    return arrayObj;
}

ani_status ScreenAniUtils::GetRectFromAni(ani_env* env, ani_object mainScreenRegionAni, DMRect& mainScreenRegion)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    ani_long left = 0;
    auto ret = env->Object_GetFieldByName_Long(mainScreenRegionAni,
        Builder::BuildPropertyName("left").c_str(), &left);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get left, ret:%{public}d", ret);
        return ret;
    }
    mainScreenRegion.posX_ = static_cast<int32_t>(left);

    ani_long top = 0;
    ret = env->Object_GetFieldByName_Long(mainScreenRegionAni,
        Builder::BuildPropertyName("top").c_str(), &top);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get top, ret:%{public}d", ret);
        return ret;
    }
    mainScreenRegion.posY_ = static_cast<int32_t>(top);

    ani_long width = 0;
    ret = env->Object_GetFieldByName_Long(mainScreenRegionAni,
        Builder::BuildPropertyName("width").c_str(), &width);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get width, ret:%{public}d", ret);
        return ret;
    }
    mainScreenRegion.width_ = static_cast<uint32_t>(width);

    ani_long height = 0;
    ret = env->Object_GetFieldByName_Long(mainScreenRegionAni,
        Builder::BuildPropertyName("height").c_str(), &height);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get height, ret:%{public}d", ret);
        return ret;
    }
    mainScreenRegion.height_ = static_cast<uint32_t>(height);
    return ANI_OK;
}

ani_status ScreenAniUtils::GetScreenIdArrayFromAni(ani_env* env, ani_object mirrorScreen,
    std::vector<ScreenId>& mirrorScreenIds)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    ani_int length = 0;
    auto ret = env->Object_GetPropertyByName_Int(mirrorScreen, "length", &length);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get ani_array length failed, ret: %{public}u", ret);
        return ret;
    }
    TLOGI(WmsLogTag::DMS, "[ANI] length %{public}d", (ani_int)length);
    for (int32_t i = 0; i < length; i++) {
        ani_ref screenIdRef;
        ret = env->Object_CallMethodByName_Ref(mirrorScreen, "$_get", "i:Y",
            &screenIdRef, (ani_int)i);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::DMS, "[ANI] get ani_array index %{public}u failed, ret: %{public}u", (ani_int)i, ret);
            return ret;
        }
        ani_long screenId;
        ret = env->Object_CallMethodByName_Long(static_cast<ani_object>(screenIdRef), "toLong", ":l", &screenId);
        if (ret !=ANI_OK) {
            TLOGE(WmsLogTag::DMS, "[ANI] unbox screenId failed, ret: %{public}u", ret);
            return ret;
        }
        mirrorScreenIds.emplace_back(static_cast<ScreenId>(screenId));
    }
    return ANI_OK;
}

ani_status ScreenAniUtils::GetExpandOptionFromAni(ani_env* env, ani_object optionAniObj, ExpandOption& expandOption)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    ani_long screenId = 0;
    ani_status ret = env->Object_GetFieldByName_Long(optionAniObj,
        Builder::BuildPropertyName("screenId").c_str(), &screenId);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get screenId, ret:%{public}d", ret);
        return ret;
    }
    expandOption.screenId_ = static_cast<ScreenId>(screenId);

    ani_long startX = 0;
    ret = env->Object_GetFieldByName_Long(optionAniObj,
        Builder::BuildPropertyName("startX").c_str(), &startX);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get startX, ret:%{public}d", ret);
        return ret;
    }
    expandOption.startX_ = static_cast<uint32_t>(startX);

    ani_long startY = 0;
    ret = env->Object_GetFieldByName_Long(optionAniObj,
        Builder::BuildPropertyName("startY").c_str(), &startY);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get startY, ret:%{public}d", ret);
        return ret;
    }
    expandOption.startY_ = static_cast<uint32_t>(startY);
    return ANI_OK;
}
}
}
