/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under * Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with * License.
 * You may obtain a copy of * License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under * License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See * License for * specific language governing permissions and
 * limitations under * License.
 */

#include "ani_fv_window_utils.h"

#include "ani_window_utils.h"

#include <string>
#include "ani.h"
#include "window.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
constexpr const char* WM_ERROR_MSG_OK = "ok";
constexpr const char* WM_ERROR_MSG_INVALID_PERMISSION = "invalid permission";
constexpr const char* WM_ERROR_MSG_REPEAT_OPERATION = "repeat operation";
constexpr const char* WM_ERROR_MSG_INVALID_CALLING = "Unauthorized operation";
constexpr const char* WM_ERROR_MSG_DEVICE_NOT_SUPPORT = "device not support";
constexpr const char* WM_ERROR_MSG_START_ABILITY_FAILED = "start ability failed";
constexpr const char* WM_ERROR_MSG_TIMEOUT = "API call timed out";
constexpr const char* WM_ERROR_MSG_FV_REPEAT_OPERATION = "repeated float view operation";
constexpr const char* WM_ERROR_MSG_FV_INVALID_STATE = "the float view state does not support this operation";
constexpr const char* WM_ERROR_MSG_FV_RESTORE_MAIN_WINDOW_FAILED = "failed to restore main window";

static std::map<WMError, const char*> WM_ERROR_TO_ERROR_MSG_MAP {
    {WMError::WM_OK,                                      WM_ERROR_MSG_OK                                },
    {WMError::WM_ERROR_INVALID_PERMISSION,                WM_ERROR_MSG_INVALID_PERMISSION                },
    {WMError::WM_ERROR_REPEAT_OPERATION,                  WM_ERROR_MSG_REPEAT_OPERATION                  },
    {WMError::WM_ERROR_INVALID_CALLING,                   WM_ERROR_MSG_INVALID_CALLING                   },
    {WMError::WM_ERROR_DEVICE_NOT_SUPPORT,                WM_ERROR_MSG_DEVICE_NOT_SUPPORT                },
    {WMError::WM_ERROR_START_ABILITY_FAILED,              WM_ERROR_MSG_START_ABILITY_FAILED              },
    {WMError::WM_ERROR_TIMEOUT,                           WM_ERROR_MSG_TIMEOUT                           },
    {WMError::WM_ERROR_FV_REPEAT_OPERATION,               WM_ERROR_MSG_FV_REPEAT_OPERATION               },
    {WMError::WM_ERROR_FV_INVALID_STATE,                  WM_ERROR_MSG_FV_INVALID_STATE                  },
    {WMError::WM_ERROR_FV_RESTORE_MAIN_WINDOW_FAILED,     WM_ERROR_MSG_FV_RESTORE_MAIN_WINDOW_FAILED     },
};

constexpr const char* WM_ERROR_CODE_MSG_OK = "ok";
constexpr const char* WM_ERROR_CODE_MSG_DEVICE_NOT_SUPPORT = "device not support";
constexpr const char* WM_ERROR_CODE_MSG_REPEAT_OPERATION = "Repeated operation.";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_CALLING = "Unauthorized operation.";
constexpr const char* WM_ERROR_CODE_MSG_START_ABILITY_FAILED = "Failed to start ability.";
constexpr const char* WM_ERROR_CODE_MSG_CONTEXT_ABNORMALLY = "this Window context is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_ILLEGAL_PARAM = "Parameter validation error";
constexpr const char* WM_ERROR_CODE_MSG_TIMEOUT = "API call timed out";
constexpr const char* WM_ERROR_CODE_MSG_FV_REPEAT_OPERATION = "repeated float view operation";
constexpr const char* WM_ERROR_CODE_MSG_FV_INVALID_STATE =
                                        "the float view state does not support this operation";
constexpr const char* WM_ERROR_CODE_MSG_FV_RESTORE_MAIN_WINDOW_FAILED =
                                        "failed to restore main window";

static std::map<WmErrorCode, const char*> WM_ERROR_CODE_TO_ERROR_MSG_MAP {
    {WmErrorCode::WM_OK,                                      WM_ERROR_CODE_MSG_OK                                },
    {WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,                WM_ERROR_CODE_MSG_DEVICE_NOT_SUPPORT                },
    {WmErrorCode::WM_ERROR_REPEAT_OPERATION,                  WM_ERROR_CODE_MSG_REPEAT_OPERATION                  },
    {WmErrorCode::WM_ERROR_INVALID_CALLING,                   WM_ERROR_CODE_MSG_INVALID_CALLING                   },
    {WmErrorCode::WM_ERROR_START_ABILITY_FAILED,              WM_ERROR_CODE_MSG_START_ABILITY_FAILED              },
    {WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY,                WM_ERROR_CODE_MSG_CONTEXT_ABNORMALLY                },
    {WmErrorCode::WM_ERROR_ILLEGAL_PARAM,                     WM_ERROR_CODE_MSG_ILLEGAL_PARAM                     },
    {WmErrorCode::WM_ERROR_TIMEOUT,                           WM_ERROR_CODE_MSG_TIMEOUT                           },
    {WmErrorCode::WM_ERROR_FV_REPEAT_OPERATION,               WM_ERROR_CODE_MSG_FV_REPEAT_OPERATION               },
    {WmErrorCode::WM_ERROR_FV_INVALID_STATE,                  WM_ERROR_CODE_MSG_FV_INVALID_STATE                  },
    {WmErrorCode::WM_ERROR_FV_RESTORE_MAIN_WINDOW_FAILED,     WM_ERROR_CODE_MSG_FV_RESTORE_MAIN_WINDOW_FAILED     },
};

ani_ref AniFvUtils::AniGetUndefined(ani_env* env)
{
    ani_ref res;
    env->GetUndefined(&res);
    return res;
}

std::string AniFvUtils::GetErrorMsg(WMError error)
{
    auto it = WM_ERROR_TO_ERROR_MSG_MAP.find(error);
    if (it != WM_ERROR_TO_ERROR_MSG_MAP.end()) {
        return it->second;
    }
    return "unknown error";
}

std::string AniFvUtils::GetErrorMsg(WmErrorCode error)
{
    auto it = WM_ERROR_CODE_TO_ERROR_MSG_MAP.find(error);
    if (it != WM_ERROR_CODE_TO_ERROR_MSG_MAP.end()) {
        return it->second;
    }
    return "unknown error";
}

ani_status AniFvUtils::CreateBusinessError(ani_env* env, int32_t error, std::string message, ani_object* err)
{
    TLOGI(WmsLogTag::DEFAULT, "start");
    ani_class aniClass = nullptr;
    ani_status status = env->FindClass("@ohos.base.BusinessError", &aniClass);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "class not found, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    ani_method aniCtor = nullptr;
    status = env->Class_FindMethod(aniClass, "<ctor>", "C{std.core.String}C{escompat.ErrorOptions}:", &aniCtor);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "ctor not found, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    ani_string aniMsg;
    GetAniString(env, message, &aniMsg);
    status = env->Object_New(aniClass, aniCtor, err, aniMsg, static_cast<ani_object>(AniFvUtils::AniGetUndefined(env)));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "fail to new err, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    status = env->Object_SetFieldByName_Int(*err, "code_", static_cast<ani_int>(error));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "fail to set code, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    return ANI_OK;
}

ani_object AniFvUtils::AniThrowError(ani_env* env, WMError wmError, const std::string& message)
{
    WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(wmError);
    return AniFvUtils::AniThrowError(env, wmErrorCode, message);
}

ani_object AniFvUtils::AniThrowError(ani_env* env, WmErrorCode wmErrorCode, const std::string& message)
{
    RETURN_IF_NULL(env, nullptr);
    ani_object err = nullptr;
    std::string errMsg = message.empty() ? GetErrorMsg(wmErrorCode) : message;
    ani_status ret = CreateBusinessError(env, static_cast<int32_t>(wmErrorCode), errMsg, &err);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "failed to new aniError, ret:%{public}d", static_cast<int32_t>(ret));
    }
    ret = env->ThrowError(static_cast<ani_error>(err));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "failed to throw error, ret:%{public}d", static_cast<int32_t>(ret));
    }
    return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
}

ani_status AniFvUtils::GetStdString(ani_env* env, ani_string ani_str, std::string &result)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "start");
    ani_size strSize;
    ani_status ret = env->String_GetUTF8Size(ani_str, &strSize);
    if (ret != ANI_OK) {
        return ret;
    }
    std::vector<char> buffer(strSize + 1);
    char* utf8Buffer = buffer.data();
    ani_size bytes_written = 0;
    ret = env->String_GetUTF8(ani_str, utf8Buffer, strSize + 1, &bytes_written);
    if (ret != ANI_OK) {
        return ret;
    }
    utf8Buffer[bytes_written] = '\0';
    result = std::string(utf8Buffer);
    return ret;
}

ani_status AniFvUtils::GetContextPtr(ani_env* env, ani_object floatViewConfigurations, void*& contextPtr)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[ANI]start");
    ani_ref ContextPtrValue;
    if ((env->Object_GetPropertyByName_Ref(floatViewConfigurations, "context", &ContextPtrValue)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]get context failed");
        return ANI_ERROR;
    }
    if (ContextPtrValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]get context failed");
        return ANI_ERROR;
    }
    ani_boolean isContextUndefined = false;
    env->Reference_IsUndefined(ContextPtrValue, &isContextUndefined);
    if (!isContextUndefined) {
        ani_object anicontextPtr = reinterpret_cast<ani_object>(ContextPtrValue);
        contextPtr = GetAbilityContext(env, anicontextPtr);
        auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
        if (context == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]context is nullptr");
            return ANI_ERROR;
        }
    }
    return ANI_OK;
}

void* AniFvUtils::GetAbilityContext(ani_env* env, ani_object aniObj)
{
    ani_long nativeContextLong;
    ani_class cls = nullptr;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass("application.UIAbilityContext.UIAbilityContext", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM,  "find class fail, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM,  "find field fail, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM,  "get field fail, status : %{public}d", status);
        return nullptr;
    }
    return (void*)nativeContextLong;
}

bool AniFvUtils::GetNativeAddress(ani_env* env,
    ani_object aniObj, const std::string& className, const std::string& field, ani_long& nativeAddress)
{
    ani_long address;
    ani_class cls = nullptr;
    ani_field addressField = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(className.c_str(), &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM,  "find class fail, status : %{public}d", status);
        return false;
    }
    if ((status = env->Class_FindField(cls, field.c_str(), &addressField)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM,  "find field fail, status : %{public}d", status);
        return false;
    }
    if ((status = env->Object_GetField_Long(aniObj, addressField, &address)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM,  "get field fail, status : %{public}d", status);
        return false;
    }
    nativeAddress = address;
    return true;
}

ani_status AniFvUtils::GetTemplateType(ani_env* env, ani_object floatViewConfigurations, uint32_t& templateType)
{
    ani_ref templateValue;
    env->Object_GetPropertyByName_Ref(floatViewConfigurations, "templateType", &templateValue);
    ani_boolean isTypeUndefined = false;
    env->Reference_IsUndefined(templateValue, &isTypeUndefined);
    if (!templateValue || isTypeUndefined) {
        return ANI_ERROR;
    }
    ani_int tempType = 0;
    auto ret = env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(templateValue), &tempType);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "EnumItem_GetValue_Int failed");
        return ANI_ERROR;
    }
    templateType = static_cast<uint32_t>(tempType);
    return ANI_OK;
}

ani_status AniFvUtils::CallAniFunctionVoid(ani_env* env, const char* ns, const char* fn, const char* signature, ...)
{
    RETURN_IF_NULL(env, ANI_ERROR);
    ani_namespace nsp = nullptr;
    ani_status ret = env->FindNamespace(ns, &nsp);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "FindNamespace failed");
        return ANI_ERROR;
    }
    ani_function func = nullptr;
    ret = env->Namespace_FindFunction(nsp, fn, signature, &func);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Namespace_FindFunction failed");
        return ANI_ERROR;
    }
    va_list args;
    va_start(args, signature);
    ret = env->Function_Call_Void_V(func, args);
    va_end(args);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Function_Call_Void_Va failed");
        return ANI_ERROR;
    }
    return ANI_OK;
}

ani_status AniFvUtils::GetAniString(ani_env* env, const std::string& str, ani_string* result)
{
    return env->String_NewUTF8(str.c_str(), static_cast<ani_size>(str.size()), result);
}

bool AniFvUtils::ParseWindowSize(ani_env* env, ani_object windowSize, Rect& rect)
{
    auto getAndAssign = [&, where = __func__](const char* name, uint32_t& field) -> ani_status {
        ani_int int_value;
        ani_status ret = env->Object_GetPropertyByName_Int(windowSize, name, &int_value);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM,
                "[ANI]Object_GetPropertyByName_Int failed,ret:%{public}d", static_cast<int32_t>(ret));
            return ret;
        }
        field = static_cast<uint32_t>(int_value);
        return ANI_OK;
    };
    if (getAndAssign("width", rect.width_) != ANI_OK ||
        getAndAssign("height", rect.height_) != ANI_OK) {
        return false;
    }
    return true;
}

ani_object AniFvUtils::CreateAniFloatViewStateChangeInfoObject(ani_env* env,
    const FloatViewState state, const std::string& reason)
{
    RETURN_IF_NULL(env, nullptr);
    ani_class infoClass = nullptr;
    ani_status ret = env->FindClass("@ohos.window.floatView.floatView.FloatViewStateChangeInfoInner", &infoClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]class not found");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    ani_method ctor = nullptr;
    ret = env->Class_FindMethod(infoClass, "<ctor>", "iC{std.core.String}:", &ctor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]find constructor failed");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    ani_string aniReason;
    if (GetAniString(env, reason, &aniReason) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]create ani string failed");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    ani_object infoObj = nullptr;
    ret = env->Object_New(infoClass, ctor, &infoObj, ani_int(static_cast<uint32_t>(state)), aniReason);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI] failed to create new obj");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    return infoObj;
}

ani_object AniFvUtils::CreateAniFloatViewRectChangeInfoObject(ani_env* env,
    const Rect& rect, double scale, const std::string& reason)
{
    RETURN_IF_NULL(env, nullptr);
    ani_class infoClass = nullptr;
    ani_status ret = env->FindClass("@ohos.window.floatView.floatView.FloatViewRectChangeInfoInner", &infoClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]class not found");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    ani_method ctor = nullptr;
    ret = env->Class_FindMethod(infoClass, "<ctor>", "C{@ohos.window.window.Rect}dC{std.core.String}:", &ctor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]find constructor failed");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    ani_object windowRect = AniWindowUtils::CreateAniRect(env, rect);
    ani_string aniReason;
    if (GetAniString(env, reason, &aniReason) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]create ani string failed");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    ani_object infoObj = nullptr;
    ret = env->Object_New(infoClass, ctor, &infoObj, windowRect, ani_double(scale), aniReason);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI] failed to create new obj");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    return infoObj;
}

ani_object AniFvUtils::CreateAniFloatViewPropertiesObject(ani_env* env, uint32_t templateType,
    const sptr<Window>& window, const FloatViewWindowInfo &windowInfo, const FvWindowState &state)
{
    RETURN_IF_NULL(env, nullptr);
    ani_class infoClass = nullptr;
    ani_status ret = env->FindClass("@ohos.window.floatView.floatView.FloatViewPropertiesInner", &infoClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]class not found");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    ani_method ctor = nullptr;
    ret = env->Class_FindMethod(infoClass, "<ctor>",
        "iiiC{@ohos.window.window.Rect}dC{@ohos.window.window.AvoidArea}z:", &ctor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]find constructor failed");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    ani_object windowRect = AniWindowUtils::CreateAniRect(env, windowInfo.windowRect_);
    ani_object avoidArea = AniWindowUtils::CreateAniAvoidArea(env,
        windowInfo.avoidArea_, AvoidAreaType::TYPE_CUTOUT, false);
    ani_boolean isSidebar = ani_boolean(state == FvWindowState::FV_STATE_IN_SIDEBAR);
    ani_object infoObj = nullptr;
    ret = env->Object_New(infoClass, ctor, &infoObj,
        ani_int(templateType), ani_int(window->GetWindowId()), ani_int(window->GetDisplayId()),
        windowRect, ani_double(windowInfo.scale_), avoidArea, isSidebar);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI] failed to create new obj");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    return infoObj;
}

ani_object AniFvUtils::CreateAniFloatViewLimitsObject(ani_env* env, const FloatViewLimits& limits)
{
    RETURN_IF_NULL(env, nullptr);
    ani_class limitsClass;
    ani_status ret = env->FindClass("@ohos.window.floatView.floatView.FloatViewLimitsInner", &limitsClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]class not found");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    ani_method ctor;
    ret = env->Class_FindMethod(limitsClass, "<ctor>",
        "C{@ohos.window.window.Size}C{@ohos.window.window.Size}C{std.core.Array}:", &ctor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]find constructor failed");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    ani_object minSize = AniWindowUtils::CreateAniSize(env, limits.minWidth_, limits.minHeight_);
    ani_object maxSize = AniWindowUtils::CreateAniSize(env, limits.maxWidth_, limits.maxHeight_);
    ani_array ratioLimits = nullptr;
    if (env->Array_New(limits.ratioLimits_.size(),
        static_cast<ani_object>(AniFvUtils::AniGetUndefined(env)), &ratioLimits) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI] create array failed");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    for (size_t i = 0; i < limits.ratioLimits_.size(); ++i) {
        if (env->Array_Set(ratioLimits, i,
            CreateAniRatioObject(env, limits.ratioLimits_[i].first, limits.ratioLimits_[i].second)) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI] create ratio limits failed");
            return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
        }
    }
    ani_object limitsObj;
    ret = env->Object_New(limitsClass, ctor, &limitsObj, minSize, maxSize, ratioLimits);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI] failed to create new obj");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    return limitsObj;
}

ani_object AniFvUtils::CreateAniRatioObject(ani_env* env, const double& min, const double& max)
{
    RETURN_IF_NULL(env, nullptr);
    ani_class ratioClass;
    ani_status ret = env->FindClass("@ohos.window.floatView.floatView.RatioLimitInner", &ratioClass);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]class not found");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    ani_method ctor;
    ret = env->Class_FindMethod(ratioClass, "<ctor>", "dd:", &ctor);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI]find constructor failed");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    ani_object ratioObj;
    ret = env->Object_New(ratioClass, ctor, &ratioObj, ani_double(min), ani_double(max));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[ANI] failed to create new obj");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    return ratioObj;
}
}  // namespace Rosen
}  // namespace OHOS