/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ani_pip_utils.h"

#include <string>

#include "ani.h"

#include "window.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
constexpr const char* WM_ERROR_CODE_MSG_OK = "ok";
constexpr const char* WM_ERROR_CODE_MSG_NO_PERMISSION = "Permission verification failed,"
    "The application does not have the permission required to call the API";
constexpr const char* WM_ERROR_CODE_MSG_NOT_SYSTEM_APP = "Permission verification failed,"
    "A non-system application calls a system API";
constexpr const char* WM_ERROR_CODE_MSG_DEVICE_NOT_SUPPORT = "Capability not supported,"
    "Failed to call the API due to limited device capabilities";
constexpr const char* WM_ERROR_CODE_MSG_REPEAT_OPERATION = "Repeated operation";
constexpr const char* WM_ERROR_CODE_MSG_STATE_ABNORMALLY = "This window state is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_SYSTEM_ABNORMALLY = "This window manager service works abnormally";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_CALLING = "Unauthorized operation";
constexpr const char* WM_ERROR_CODE_MSG_STAGE_ABNORMALLY = "This window stage is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_CONTEXT_ABNORMALLY = "This window context is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_START_ABILITY_FAILED = "Failed to start the ability";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_DISPLAY = "The display device is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_PARENT = "The parent window is invalid";
constexpr const char* WM_ERROR_CODE_MSG_PIP_DESTROY_FAILED = "Failed to destroy the PiP window";
constexpr const char* WM_ERROR_CODE_MSG_PIP_STATE_ABNORMALLY = "The PiP window state is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_PIP_CREATE_FAILED = "Failed to create the PiP window";
constexpr const char* WM_ERROR_CODE_MSG_PIP_INTERNAL_ERROR = "PiP internal error";
constexpr const char* WM_ERROR_CODE_MSG_PIP_REPEAT_OPERATION = "Repeated PiP operation";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_OP_IN_CUR_STATUS = "Invalid operation in current status";

static std::map<WmErrorCode, const char*> WM_ERROR_CODE_TO_ERROR_MSG_MAP {
    {WmErrorCode::WM_OK,                              WM_ERROR_CODE_MSG_OK                       },
    {WmErrorCode::WM_ERROR_NO_PERMISSION,             WM_ERROR_CODE_MSG_NO_PERMISSION            },
    {WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,            WM_ERROR_CODE_MSG_NOT_SYSTEM_APP           },
    {WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,        WM_ERROR_CODE_MSG_DEVICE_NOT_SUPPORT       },
    {WmErrorCode::WM_ERROR_REPEAT_OPERATION,          WM_ERROR_CODE_MSG_REPEAT_OPERATION         },
    {WmErrorCode::WM_ERROR_STATE_ABNORMALLY,          WM_ERROR_CODE_MSG_STATE_ABNORMALLY         },
    {WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,         WM_ERROR_CODE_MSG_SYSTEM_ABNORMALLY        },
    {WmErrorCode::WM_ERROR_INVALID_CALLING,           WM_ERROR_CODE_MSG_INVALID_CALLING          },
    {WmErrorCode::WM_ERROR_STAGE_ABNORMALLY,          WM_ERROR_CODE_MSG_STAGE_ABNORMALLY         },
    {WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY,        WM_ERROR_CODE_MSG_CONTEXT_ABNORMALLY       },
    {WmErrorCode::WM_ERROR_START_ABILITY_FAILED,      WM_ERROR_CODE_MSG_START_ABILITY_FAILED     },
    {WmErrorCode::WM_ERROR_INVALID_DISPLAY,           WM_ERROR_CODE_MSG_INVALID_DISPLAY          },
    {WmErrorCode::WM_ERROR_INVALID_PARENT,            WM_ERROR_CODE_MSG_INVALID_PARENT           },
    {WmErrorCode::WM_ERROR_PIP_DESTROY_FAILED,        WM_ERROR_CODE_MSG_PIP_DESTROY_FAILED       },
    {WmErrorCode::WM_ERROR_PIP_STATE_ABNORMALLY,      WM_ERROR_CODE_MSG_PIP_STATE_ABNORMALLY     },
    {WmErrorCode::WM_ERROR_PIP_CREATE_FAILED,         WM_ERROR_CODE_MSG_PIP_CREATE_FAILED        },
    {WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR,        WM_ERROR_CODE_MSG_PIP_INTERNAL_ERROR       },
    {WmErrorCode::WM_ERROR_PIP_REPEAT_OPERATION,      WM_ERROR_CODE_MSG_PIP_REPEAT_OPERATION     },
    {WmErrorCode::WM_ERROR_INVALID_OP_IN_CUR_STATUS,  WM_ERROR_CODE_MSG_INVALID_OP_IN_CUR_STATUS },
};

ani_ref AniGetUndefined(ani_env* env)
{
    ani_ref res;
    env->GetUndefined(&res);
    return static_cast<ani_object>(res);
}

ani_status CreateBusinessError(ani_env* env, int32_t error, std::string message, ani_object* err)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    ani_class aniClass;
    ani_status status = env->FindClass("@ohos.base.BusinessError", &aniClass);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "class not found, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    ani_method aniCtor;
    status = env->Class_FindMethod(aniClass, "<ctor>", "C{std.core.String}C{escompat.ErrorOptions}:", &aniCtor);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "ctor not found, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    ani_string aniMsg;
    env->String_NewUTF8(message.c_str(), static_cast<ani_size>(message.size()), &aniMsg);
    status = env->Object_New(aniClass, aniCtor, err, aniMsg, AniGetUndefined(env));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "fail to new err, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    status = env->Object_SetFieldByName_Int(*err, "code_", static_cast<ani_int>(error));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "fail to set code, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    TLOGI(WmsLogTag::WMS_PIP, "finish");
    return ANI_OK;
}

ani_ref AniThrowError(ani_env* env, WMError error, std::string msg)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
     // WMError → WmErrorCode
    WmErrorCode errorCode = WM_JS_TO_ERROR_CODE_MAP.at(error);
    std::string errorMessage = msg.empty() ? GetErrorMsg(errorCode) : msg;
    ani_object aniError;
    CreateBusinessError(env, static_cast<int32_t>(errorCode), errorMessage, &aniError);
    env->ThrowError(static_cast<ani_error>(aniError));
    TLOGI(WmsLogTag::WMS_PIP, "finish");
    return AniGetUndefined(env);
}

std::string GetErrorMsg(WmErrorCode errorCode)
{
    auto it = WM_ERROR_CODE_TO_ERROR_MSG_MAP.find(errorCode);
    if (it == WM_ERROR_CODE_TO_ERROR_MSG_MAP.end()) {
        return "[FB]unknown WMError";
    }
    return it->second;
}

ani_status GetStdString(ani_env *env, ani_string ani_str, std::string &result)
{
    TLOGI(WmsLogTag::WMS_PIP, "GetStdString");
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
    if ((status = env->FindClass("application.UIAbilityContext.UIAbilityContext", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP,  "find class fail, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP,  "find field fail, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP,  "get field fail, status : %{public}d", status);
        return nullptr;
    }
    return (void*)nativeContextLong;
}

ani_status CallAniFunctionVoid(ani_env *env, const char* ns, const char* fn, const char* signature, ...)
{
    ani_status ret = ANI_OK;
    ani_namespace aniNamespace{};
    if ((ret = env->FindNamespace(ns, &aniNamespace)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "cannot find namespace");
        return ret;
    }
    ani_function func{};
    if ((ret = env->Namespace_FindFunction(aniNamespace, fn, signature, &func)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "cannot find function, errorcode is: %{public}u", ret);
        return ret;
    }
    va_list args;
    va_start(args, signature);
    ret = env->Function_Call_Void_V(func, args);
    va_end(args);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "cannot call function");
        return ret;
    }
    return ret;
}

ani_status GetAniString(ani_env* env, const std::string& str, ani_string* result)
{
    return env->String_NewUTF8(str.c_str(), static_cast<ani_size>(str.size()), result);
}

} // Rosen
} // OHOS