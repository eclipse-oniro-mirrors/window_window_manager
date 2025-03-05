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
#include "display_ani_utils.h"
#include "display_ani.h"
#include "display_info.h"
#include "display.h"
#include "singleton_container.h"
#include "display_manager.h"
#include "window_manager_hilog.h"
#include "dm_common.h"
#include "refbase.h"


namespace OHOS {
namespace Rosen {

namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "DisplayAniUtils"};
}

enum class DisplayStateMode : uint32_t {
    STATE_UNKNOWN = 0,
    STATE_OFF,
    STATE_ON,
    STATE_DOZE,
    STATE_DOZE_SUSPEND,
    STATE_VR,
    STATE_ON_SUSPEND
};
 
static const std::map<DisplayState,      DisplayStateMode> NATIVE_TO_JS_DISPLAY_STATE_MAP {
    { DisplayState::UNKNOWN,      DisplayStateMode::STATE_UNKNOWN      },
    { DisplayState::OFF,          DisplayStateMode::STATE_OFF          },
    { DisplayState::ON,           DisplayStateMode::STATE_ON           },
    { DisplayState::DOZE,         DisplayStateMode::STATE_DOZE         },
    { DisplayState::DOZE_SUSPEND, DisplayStateMode::STATE_DOZE_SUSPEND },
    { DisplayState::VR,           DisplayStateMode::STATE_VR           },
    { DisplayState::ON_SUSPEND,   DisplayStateMode::STATE_ON_SUSPEND   },
};
 
 
ani_object DisplayAniUtils::convertRect(DMRect rect, ani_env* env)
{
    // find class
    ani_class cls;
    // return obj
    ani_object obj = nullptr;
    // find field
    ani_field leftFld;
    ani_field widthFld;
    ani_field topFld;
    ani_field heightFld;

    if (ANI_OK != env->FindClass("L@ohos/display/display/rectImpl", &cls)) {
        WLOGFE("[ANI] null class CutoutInfoImpl");
        return obj;
    }
    if (ANI_OK != DisplayAniUtils::NewAniObjectNoParams(env, cls , &obj)) {
        WLOGFE("[ANI] create rect obj fail");
        return obj;
    }
    if (ANI_OK != env->Class_FindField(cls, "left", &leftFld)) {
        WLOGFE("[ANI] null field left");
        return obj;
    }
    if (ANI_OK != env->Class_FindField(cls, "width", &widthFld)) {
        WLOGFE("[ANI] null field right");
        return obj;
    }
    if (ANI_OK != env->Class_FindField(cls, "top", &topFld)) {
        WLOGFE("[ANI] null field top");
        return obj;
    }
    if (ANI_OK != env->Class_FindField(cls, "height", &heightFld)) {
        WLOGFE("[ANI] null field bottom");
        return obj;
    }
    env->Object_SetField_Int(obj, leftFld, rect.posX_);
    env->Object_SetField_Int(obj, widthFld, rect.width_);
    env->Object_SetField_Int(obj, topFld, rect.posY_);
    env->Object_SetField_Int(obj, heightFld, rect.height_);

    return obj;
}
 
ani_array_ref DisplayAniUtils::convertRects(std::vector<DMRect> rects, ani_env* env)
{
    ani_array_ref arrayres = nullptr;
    int size = rects.size();
    ani_class cls = nullptr;
    if (ANI_OK != env->FindClass("L@ohos/display/display/RectImpl", &cls)) {
        WLOGFE("[ANI] null class CutoutInfoImpl");
    }
    ani_ref undefinedRef = nullptr;
    if (ANI_OK != env->GetUndefined(&undefinedRef)) {
        WLOGFE("[ANI] get undefined error");
    }
    if (ANI_OK != env->Array_New_Ref(cls, size, undefinedRef, &arrayres)) {
        WLOGFE("[ANI] create rect array error");
    }
    for (int i = 0; i < size; i++) {
        DMRect rect = rects[i];
        if (ANI_OK != env->Array_Set_Ref(arrayres, i, convertRect(rect, env))) {
            WLOGFE("[ANI] set rect array error");
        }
    }
    WLOGFE("arrayres size = %{public}d", size);
    return arrayres;
}
 
ani_status DisplayAniUtils::cvtDisplay(sptr<Display> display, ani_env* env, ani_object obj)
{
    sptr<DisplayInfo> info = display->GetDisplayInfoByJs();
    env->Object_SetFieldByName_Int(obj, "id",  static_cast<uint32_t>(info->GetDisplayId()));
    const ani_size stringLength = info->GetName().size();
    ani_string str = nullptr;
    env->String_NewUTF8(info->GetName().data(), stringLength, &str);
    env->Object_SetFieldByName_Ref(obj, "name", str);
    env->Object_SetFieldByName_Boolean(obj, "alive", info->GetAliveStatus());
    if (NATIVE_TO_JS_DISPLAY_STATE_MAP.count(info->GetDisplayState()) != 0) {
        env->Object_SetFieldByName_Int(obj, "state", static_cast<uint32_t>(info->GetDisplayState()));
    } else {
        env->Object_SetFieldByName_Int(obj, "state", 0);
    }
    env->Object_SetFieldByName_Int(obj, "refreshRate", info->GetRefreshRate());
    env->Object_SetFieldByName_Int(obj, "rotation", static_cast<uint32_t>(info->GetRotation()));
    env->Object_SetFieldByName_Int(obj, "width", info->GetWidth());
    env->Object_SetFieldByName_Int(obj, "height", display->GetHeight());
    env->Object_SetFieldByName_Int(obj, "availableWidth", info->GetAvailableWidth());
    env->Object_SetFieldByName_Int(obj, "availableHeight", info->GetAvailableHeight());
    env->Object_SetFieldByName_Float(obj, "densityDPI", info->GetVirtualPixelRatio() * DOT_PER_INCH);
    env->Object_SetFieldByName_Int(obj, "orientation", static_cast<uint32_t>(info->GetDisplayOrientation()));
    env->Object_SetFieldByName_Float(obj, "densityPixels", info->GetVirtualPixelRatio());
    env->Object_SetFieldByName_Float(obj, "scaledDensity", info->GetVirtualPixelRatio());
    env->Object_SetFieldByName_Float(obj, "xDPI", info->GetXDpi());
    env->Object_SetFieldByName_Float(obj, "yDPI", info->GetYDpi());
    auto colorSpaces = info->GetColorSpaces();
    auto hdrFormats = info->GetHdrFormats();
    if (colorSpaces.size() != 0) {
        ani_array_int colorSpacesAni;
        CreateAniArrayInt(env, colorSpaces.size(), &colorSpacesAni, colorSpaces);
        if (ANI_OK != env->Object_SetFieldByName_Ref(obj, "colorSpaces", static_cast<ani_ref>(colorSpacesAni))) {
            WLOGFE("[ANI] Array set colorSpaces field error");
        }
    }
    if (hdrFormats.size() != 0) {
        ani_array_int hdrFormatsAni;
        CreateAniArrayInt(env, hdrFormats.size(), &hdrFormatsAni, hdrFormats);
        if (ANI_OK != env->Object_SetFieldByName_Ref(obj, "hdrFormats", static_cast<ani_ref>(hdrFormatsAni))) {
            WLOGFE("[ANI] Array set hdrFormats field error");
        }
    }
    return ANI_OK;
}

void DisplayAniUtils::CreateAniArrayInt(ani_env* env, ani_size size, ani_array_int *aniArray, std::vector<uint32_t> vec)
{
    if (ANI_OK != env->Array_New_Int(size, aniArray)) {
        WLOGFE("[ANI] create colorSpace array error");
    }
    ani_int* aniArrayBuf = reinterpret_cast<ani_int *>(vec.data());
    if (ANI_OK != env->Array_SetRegion_Int(*aniArray, 0, size, aniArrayBuf)) {
        WLOGFE("[ANI] Array set region int error");
    }
}

void DisplayAniUtils::GetStdString(ani_env *env, ani_string str, std::string &result)
{
    ani_size sz {};
    env->String_GetUTF8Size(str, &sz);
    result.resize(sz + 1);
    env->String_GetUTF8SubString(str, 0, sz, result.data(), result.size(), &sz);
}

ani_status DisplayAniUtils::NewAniObject(ani_env* env, ani_class cls, const char *signature, ani_object* result, ...)
{
    ani_method aniCtor;
    auto ret = env->Class_FindMethod(cls, "<ctor>", signature, &aniCtor);
    if (ret != ANI_OK) {
        return ret;
    }
    va_list args;
    va_start(args, result);
    ani_status status = env->Object_New(cls, aniCtor, result, args);
    va_end(args);
    return status;
}

ani_status DisplayAniUtils::NewAniObjectNoParams(ani_env* env, ani_class cls, ani_object* object)
{
    ani_method aniCtor;
    auto ret = env->Class_FindMethod(cls, "<ctor>", ":V", &aniCtor);
    if (ret != ANI_OK) {
        WLOGFE("[ANI] find ctor method fail");
        return ret;
    }
    return env->Object_New(cls, aniCtor, object);
}

ani_object DisplayAniUtils::CreateAniUndefined(ani_env* env)
{
    ani_ref aniRef;
    env->GetUndefined(&aniRef);
    return static_cast<ani_object>(aniRef);
}
}
}