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

#include <string>

#include "ani.h"
#include "window.h"

#include "wm_common.h"
#include "window_manager_hilog.h"
#include "ani_fb_window_utils.h"

#define RETURN_IF_NULL(param, ...)                                          \
    do {                                                                    \
        if (!param) {                                                       \
            TLOGE(WmsLogTag::WMS_LAYOUT, "The %{public}s is null", #param); \
            return __VA_ARGS__;                                             \
        }                                                                   \
    } while (false)

namespace OHOS {
namespace Rosen {
constexpr const char* WM_ERROR_MSG_OK = "ok";
constexpr const char* WM_ERROR_MSG_INVALID_PERMISSION = "invalid permission";
constexpr const char* WM_ERROR_MSG_REPEAT_OPERATION = "repeat operation";
constexpr const char* WM_ERROR_MSG_INVALID_CALLING = "Unauthorized operation";
constexpr const char* WM_ERROR_MSG_DEVICE_NOT_SUPPORT = "device not support";
constexpr const char* WM_ERROR_MSG_START_ABILITY_FAILED = "start ability failed";
constexpr const char* WM_ERROR_MSG_TIMEOUT = "API call timed out";
constexpr const char* WM_ERROR_MSG_FB_PARAM_INVALID = "wrong parameters for operating floating ball";
constexpr const char* WM_ERROR_MSG_FB_CREATE_FAILED = "failed to create the floating ball window";
constexpr const char* WM_ERROR_MSG_FB_REPEAT_CONTROLLER = "failed to start multiple floating ball windows";
constexpr const char* WM_ERROR_MSG_FB_REPEAT_OPERATION = "repeated floating ball operation";
constexpr const char* WM_ERROR_MSG_FB_INTERNAL_ERROR = "floating ball internal error";
constexpr const char* WM_ERROR_MSG_FB_STATE_ABNORMALLY = "the floating ball window state is abnormal";
constexpr const char* WM_ERROR_MSG_FB_INVALID_STATE = "the floating ball state does not support this operation";
constexpr const char* WM_ERROR_MSG_FB_RESTORE_MAIN_WINDOW_FAILED = "failed to restore the main window";
constexpr const char* WM_ERROR_MSG_FB_UPDATE_TEMPLATE_TYPE_DENIED =
                                    "when updating the floating ball, the template type cannot be changed";
constexpr const char* WM_ERROR_MSG_FB_UPDATE_STATIC_TEMPLATE_DENIED =
                                    "updating static template-based floating ball is not supported";

static std::map<WMError, const char*> WM_ERROR_TO_ERROR_MSG_MAP {
    {WMError::WM_OK,                                      WM_ERROR_MSG_OK                                },
    {WMError::WM_ERROR_INVALID_PERMISSION,                WM_ERROR_MSG_INVALID_PERMISSION                },
    {WMError::WM_ERROR_REPEAT_OPERATION,                  WM_ERROR_MSG_REPEAT_OPERATION                  },
    {WMError::WM_ERROR_INVALID_CALLING,                   WM_ERROR_MSG_INVALID_CALLING                   },
    {WMError::WM_ERROR_DEVICE_NOT_SUPPORT,                WM_ERROR_MSG_DEVICE_NOT_SUPPORT                },
    {WMError::WM_ERROR_START_ABILITY_FAILED,              WM_ERROR_MSG_START_ABILITY_FAILED              },
    {WMError::WM_ERROR_TIMEOUT,                           WM_ERROR_MSG_TIMEOUT                           },
    {WMError::WM_ERROR_FB_PARAM_INVALID,                  WM_ERROR_MSG_FB_PARAM_INVALID                  },
    {WMError::WM_ERROR_FB_CREATE_FAILED,                  WM_ERROR_MSG_FB_CREATE_FAILED                  },
    {WMError::WM_ERROR_FB_REPEAT_CONTROLLER,              WM_ERROR_MSG_FB_REPEAT_CONTROLLER              },
    {WMError::WM_ERROR_FB_REPEAT_OPERATION,               WM_ERROR_MSG_FB_REPEAT_OPERATION               },
    {WMError::WM_ERROR_FB_INTERNAL_ERROR,                 WM_ERROR_MSG_FB_INTERNAL_ERROR                 },
    {WMError::WM_ERROR_FB_STATE_ABNORMALLY,               WM_ERROR_MSG_FB_STATE_ABNORMALLY               },
    {WMError::WM_ERROR_FB_INVALID_STATE,                  WM_ERROR_MSG_FB_INVALID_STATE                  },
    {WMError::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED,     WM_ERROR_MSG_FB_RESTORE_MAIN_WINDOW_FAILED     },
    {WMError::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED,    WM_ERROR_MSG_FB_UPDATE_TEMPLATE_TYPE_DENIED    },
    {WMError::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED,  WM_ERROR_MSG_FB_UPDATE_STATIC_TEMPLATE_DENIED  },
};

constexpr const char* WM_ERROR_CODE_MSG_OK = "ok";
constexpr const char* WM_ERROR_CODE_MSG_DEVICE_NOT_SUPPORT = "device not support";
constexpr const char* WM_ERROR_CODE_MSG_REPEAT_OPERATION = "Repeated operation.";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_CALLING = "Unauthorized operation.";
constexpr const char* WM_ERROR_CODE_MSG_START_ABILITY_FAILED = "Failed to start the ability.";
constexpr const char* WM_ERROR_CODE_MSG_CONTEXT_ABNORMALLY = "this Window context is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_ILLEGAL_PARAM = "Parameter validation error";
constexpr const char* WM_ERROR_CODE_MSG_TIMEOUT = "API call timed out";
constexpr const char* WM_ERROR_CODE_MSG_FB_PARAM_INVALID = "wrong parameters for operating floating ball";
constexpr const char* WM_ERROR_CODE_MSG_FB_CREATE_FAILED = "failed to create the floating ball window";
constexpr const char* WM_ERROR_CODE_MSG_FB_REPEAT_CONTROLLER = "failed to start multiple floating ball windows";
constexpr const char* WM_ERROR_CODE_MSG_FB_REPEAT_OPERATION = "repeated floating ball operation";
constexpr const char* WM_ERROR_CODE_MSG_FB_INTERNAL_ERROR = "floating ball internal error";
constexpr const char* WM_ERROR_CODE_MSG_FB_STATE_ABNORMALLY = "the floating ball window state is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_FB_INVALID_STATE =
                                        "the floating ball state does not support this operation";
constexpr const char* WM_ERROR_CODE_MSG_FB_RESTORE_MAIN_WINDOW_FAILED =
                                        "failed to restore the main window";
constexpr const char* WM_ERROR_CODE_MSG_FB_UPDATE_TEMPLATE_TYPE_DENIED =
                                        "when updating the floating ball, the template type cannot be changed";
constexpr const char* WM_ERROR_CODE_MSG_FB_UPDATE_STATIC_TEMPLATE_DENIED =
                                        "updating static template-based floating ball is not supported";

static std::map<WmErrorCode, const char*> WM_ERROR_CODE_TO_ERROR_MSG_MAP {
    {WmErrorCode::WM_OK,                                      WM_ERROR_CODE_MSG_OK                                },
    {WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,                WM_ERROR_CODE_MSG_DEVICE_NOT_SUPPORT                },
    {WmErrorCode::WM_ERROR_REPEAT_OPERATION,                  WM_ERROR_CODE_MSG_REPEAT_OPERATION                  },
    {WmErrorCode::WM_ERROR_INVALID_CALLING,                   WM_ERROR_CODE_MSG_INVALID_CALLING                   },
    {WmErrorCode::WM_ERROR_START_ABILITY_FAILED,              WM_ERROR_CODE_MSG_START_ABILITY_FAILED              },
    {WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY,                WM_ERROR_CODE_MSG_CONTEXT_ABNORMALLY                },
    {WmErrorCode::WM_ERROR_ILLEGAL_PARAM,                     WM_ERROR_CODE_MSG_ILLEGAL_PARAM                     },
    {WmErrorCode::WM_ERROR_TIMEOUT,                           WM_ERROR_CODE_MSG_TIMEOUT                           },
    {WmErrorCode::WM_ERROR_FB_PARAM_INVALID,                  WM_ERROR_CODE_MSG_FB_PARAM_INVALID                  },
    {WmErrorCode::WM_ERROR_FB_CREATE_FAILED,                  WM_ERROR_CODE_MSG_FB_CREATE_FAILED                  },
    {WmErrorCode::WM_ERROR_FB_REPEAT_CONTROLLER,              WM_ERROR_CODE_MSG_FB_REPEAT_CONTROLLER              },
    {WmErrorCode::WM_ERROR_FB_REPEAT_OPERATION,               WM_ERROR_CODE_MSG_FB_REPEAT_OPERATION               },
    {WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,                 WM_ERROR_CODE_MSG_FB_INTERNAL_ERROR                 },
    {WmErrorCode::WM_ERROR_FB_STATE_ABNORMALLY,               WM_ERROR_CODE_MSG_FB_STATE_ABNORMALLY               },
    {WmErrorCode::WM_ERROR_FB_INVALID_STATE,                  WM_ERROR_CODE_MSG_FB_INVALID_STATE                  },
    {WmErrorCode::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED,     WM_ERROR_CODE_MSG_FB_RESTORE_MAIN_WINDOW_FAILED     },
    {WmErrorCode::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED,    WM_ERROR_CODE_MSG_FB_UPDATE_TEMPLATE_TYPE_DENIED    },
    {WmErrorCode::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED,  WM_ERROR_CODE_MSG_FB_UPDATE_STATIC_TEMPLATE_DENIED  },
};

ani_ref AniGetUndefined(ani_env* env)
{
    ani_ref res;
    env->GetUndefined(&res);
    return res;
}

ani_status CreateBusinessError(ani_env* env, int32_t error, std::string message, ani_object* err)
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]start");
    ani_class aniClass = nullptr;
    ani_status status = env->FindClass("@ohos.base.BusinessError", &aniClass);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]class not found, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    ani_method aniCtor = nullptr;
    status = env->Class_FindMethod(aniClass, "<ctor>", "C{std.core.String}C{escompat.ErrorOptions}:", &aniCtor);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]ctor not found, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    ani_string aniMsg;
    GetAniString(env, message, &aniMsg);
    status = env->Object_New(aniClass, aniCtor, err, aniMsg, CreateAniUndefined(env));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]fail to new err, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    status = env->Object_SetFieldByName_Int(*err, "code_", static_cast<ani_int>(error));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]fail to set code, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    return ANI_OK;
}

ani_status GetAniString(ani_env* env, const std::string& str, ani_string* result)
{
    return env->String_NewUTF8(str.c_str(), static_cast<ani_size>(str.size()), result);
}

std::string GetErrorMsg(WMError error)
{
    auto it = WM_ERROR_TO_ERROR_MSG_MAP.find(error);
    if (it == WM_ERROR_TO_ERROR_MSG_MAP.end()) {
        return "[FB]unknown WMError";
    }
    return it->second;
}

std::string GetErrorMsg(WmErrorCode error)
{
    auto it = WM_ERROR_CODE_TO_ERROR_MSG_MAP.find(error);
    if (it == WM_ERROR_CODE_TO_ERROR_MSG_MAP.end()) {
        return "[FB]unknown WmErrorCode";
    }
    return it->second;
}

ani_object CreateAniUndefined(ani_env* env)
{
    ani_ref aniRef;
    env->GetUndefined(&aniRef);
    return static_cast<ani_object>(aniRef);
}

ani_status GetStdString(ani_env *env, ani_string ani_str, std::string &result)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
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

void* GetAbilityContext(ani_env *env, ani_object aniObj)
{
    ani_long nativeContextLong;
    ani_class cls = nullptr;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    if ((status = env->FindClass("application.UIAbilityContext.UIAbilityContext", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM,  "[FB]find class fail, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM,  "[FB]find field fail, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM,  "[FB]get field fail, status : %{public}d", status);
        return nullptr;
    }
    return (void*)nativeContextLong;
}

ani_status GetContextPtr(ani_env* env, ani_object floatingBallConfigurations, void*& contextPtr)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    ani_ref ContextPtrValue;
    if ((env->Object_GetPropertyByName_Ref(floatingBallConfigurations, "context", &ContextPtrValue)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]get context failed");
        return ANI_ERROR;
    }
    if (ContextPtrValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]get context failed");
        return ANI_ERROR;
    }
    ani_boolean isContextUndefined = false;
    env->Reference_IsUndefined(ContextPtrValue, &isContextUndefined);
    if (!isContextUndefined) {
        ani_object anicontextPtr = reinterpret_cast<ani_object>(ContextPtrValue);
        contextPtr = GetAbilityContext(env, anicontextPtr);
        auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
        if (context == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]context is nullptr");
            return ANI_ERROR;
        }
    }
    return ANI_OK;
}

bool IsInstanceOf(ani_env* env, ani_object obj, const char* className)
{
    RETURN_IF_NULL(env, false);
    RETURN_IF_NULL(obj, false);
    RETURN_IF_NULL(className, false);

    ani_class cls;
    ani_status ret = env->FindClass(className, &cls);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT,
              "[FB]Failed to find class %{public}s, ret: %{public}d",
              className, static_cast<int32_t>(ret));
        return false;
    }

    ani_boolean isInstance = ANI_FALSE;
    ret = env->Object_InstanceOf(obj, cls, &isInstance);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT,
              "[FB]Failed to check instance of %{public}s, ret: %{public}d",
              className, static_cast<int32_t>(ret));
        return false;
    }
    return static_cast<bool>(isInstance);
}

ani_status CallAniFunctionVoid(ani_env *env, const char* ns, const char* fn, const char* signature, ...)
{
    ani_status ret = ANI_OK;
    ani_namespace aniNamespace{};
    if ((ret = env->FindNamespace(ns, &aniNamespace)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]cannot find namespace");
        return ret;
    }
    ani_function func{};
    if ((ret = env->Namespace_FindFunction(aniNamespace, fn, signature, &func)) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]cannot find callback");
        return ret;
    }
    va_list args;
    va_start(args, signature);
    ret = env->Function_Call_Void_V(func, args);
    va_end(args);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]cannot run callback");
        return ret;
    }
    return ret;
}
} // Rosen
} // OHOS