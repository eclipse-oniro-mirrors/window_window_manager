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

#include "ani_err_utils.h"

#include "ani_scene_session_utils.h"
#include "window_manager_hilog.h"

#include <map>

namespace OHOS::Rosen {

constexpr const char* WM_ERROR_CODE_MSG_OK = "ok";
constexpr const char* WM_ERROR_CODE_MSG_NO_PERMISSION = "Permission verification failed. "
    "The application does not have the permission required to call the API.";
constexpr const char* WM_ERROR_CODE_MSG_NOT_SYSTEM_APP = "Permission verification failed. "
    "A non-system application calls a system API.";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_PARAM = "Parameter error. Possible causes: "
    "1. Mandatory parameters are left unspecified; "
    "2. Incorrect parameter types; "
    "3. Parameter verification failed.";
constexpr const char* WM_ERROR_CODE_MSG_DEVICE_NOT_SUPPORT = "Capability not supported. "
    "Failed to call the API due to limited device capabilities.";
constexpr const char* WM_ERROR_CODE_MSG_REPEAT_OPERATION = "Repeated operation.";
constexpr const char* WM_ERROR_CODE_MSG_STATE_ABNORMALLY = "This window state is abnormal.";
constexpr const char* WM_ERROR_CODE_MSG_SYSTEM_ABNORMALLY = "This window manager service works abnormally.";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_CALLING = "Unauthorized operation.";
constexpr const char* WM_ERROR_CODE_MSG_STAGE_ABNORMALLY = "This window stage is abnormal.";
constexpr const char* WM_ERROR_CODE_MSG_CONTEXT_ABNORMALLY = "This window context is abnormal.";
constexpr const char* WM_ERROR_CODE_MSG_START_ABILITY_FAILED = "Failed to start the ability.";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_DISPLAY = "The display device is abnormal.";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_PARENT = "The parent window is invalid.";
constexpr const char* WM_ERROR_CODE_MSG_PIP_DESTROY_FAILED = "Failed to destroy the PiP window.";
constexpr const char* WM_ERROR_CODE_MSG_PIP_STATE_ABNORMALLY = "The PiP window state is abnormal.";
constexpr const char* WM_ERROR_CODE_MSG_PIP_CREATE_FAILED = "Failed to create the PiP window.";
constexpr const char* WM_ERROR_CODE_MSG_PIP_INTERNAL_ERROR = "PiP internal error.";
constexpr const char* WM_ERROR_CODE_MSG_PIP_REPEAT_OPERATION = "Repeated PiP operation.";

static std::map<WmErrorCode, const char*> WM_ERROR_CODE_TO_ERROR_MSG_MAP {
    {WmErrorCode::WM_OK,                              WM_ERROR_CODE_MSG_OK                     },
    {WmErrorCode::WM_ERROR_NO_PERMISSION,             WM_ERROR_CODE_MSG_NO_PERMISSION          },
    {WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,            WM_ERROR_CODE_MSG_NOT_SYSTEM_APP         },
    {WmErrorCode::WM_ERROR_INVALID_PARAM,             WM_ERROR_CODE_MSG_INVALID_PARAM          },
    {WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,        WM_ERROR_CODE_MSG_DEVICE_NOT_SUPPORT     },
    {WmErrorCode::WM_ERROR_REPEAT_OPERATION,          WM_ERROR_CODE_MSG_REPEAT_OPERATION       },
    {WmErrorCode::WM_ERROR_STATE_ABNORMALLY,          WM_ERROR_CODE_MSG_STATE_ABNORMALLY       },
    {WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,         WM_ERROR_CODE_MSG_SYSTEM_ABNORMALLY      },
    {WmErrorCode::WM_ERROR_INVALID_CALLING,           WM_ERROR_CODE_MSG_INVALID_CALLING        },
    {WmErrorCode::WM_ERROR_STAGE_ABNORMALLY,          WM_ERROR_CODE_MSG_STAGE_ABNORMALLY       },
    {WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY,        WM_ERROR_CODE_MSG_CONTEXT_ABNORMALLY     },
    {WmErrorCode::WM_ERROR_START_ABILITY_FAILED,      WM_ERROR_CODE_MSG_START_ABILITY_FAILED   },
    {WmErrorCode::WM_ERROR_INVALID_DISPLAY,           WM_ERROR_CODE_MSG_INVALID_DISPLAY        },
    {WmErrorCode::WM_ERROR_INVALID_PARENT,            WM_ERROR_CODE_MSG_INVALID_PARENT         },
    {WmErrorCode::WM_ERROR_PIP_DESTROY_FAILED,        WM_ERROR_CODE_MSG_PIP_DESTROY_FAILED     },
    {WmErrorCode::WM_ERROR_PIP_STATE_ABNORMALLY,      WM_ERROR_CODE_MSG_PIP_STATE_ABNORMALLY   },
    {WmErrorCode::WM_ERROR_PIP_CREATE_FAILED,         WM_ERROR_CODE_MSG_PIP_CREATE_FAILED      },
    {WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR,        WM_ERROR_CODE_MSG_PIP_INTERNAL_ERROR     },
    {WmErrorCode::WM_ERROR_PIP_REPEAT_OPERATION,      WM_ERROR_CODE_MSG_PIP_REPEAT_OPERATION   },
};

ani_status AniErrUtils::ThrowBusinessError(ani_env* env, WmErrorCode error, std::string message)
{
    ani_object aniError;
    CreateBusinessError(env, static_cast<int32_t>(error), message == "" ? GetErrorMsg(error) : message, &aniError);
    ani_status status = env->ThrowError(static_cast<ani_error>(aniError));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::WMS_MAIN, "[ANI] fail to throw err, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    return ANI_OK;
}

std::string AniErrUtils::GetErrorMsg(const WmErrorCode& errorCode)
{
    return WM_ERROR_CODE_TO_ERROR_MSG_MAP.find(errorCode) != WM_ERROR_CODE_TO_ERROR_MSG_MAP.end() ?
        WM_ERROR_CODE_TO_ERROR_MSG_MAP.at(errorCode) : "";
}

ani_status AniErrUtils::CreateBusinessError(ani_env* env, int32_t error, std::string message, ani_object* err)
{
    TLOGI(WmsLogTag::DMS, "[ANI] in1");
    ani_class aniClass;
    ani_status status = env->FindClass("escompat.Error", &aniClass);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] class not found, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    ani_method aniCtor;
    status = env->Class_FindMethod(aniClass, "<ctor>", "C{std.core.String}C{escompat.ErrorOptions}:", &aniCtor);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] ctor not found, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    ani_string aniMsg;
    env->String_NewUTF8(message.c_str(), static_cast<ani_size>(message.size()), &aniMsg);
    status = env->Object_New(aniClass, aniCtor, err, aniMsg, AniSceneSessionUtils::CreateAniUndefined(env));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to new err, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    return ANI_OK;
}

} // namespace OHOS::Rosen