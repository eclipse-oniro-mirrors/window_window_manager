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

#include <map>

#include "display_ani_utils.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {

constexpr const char* DM_ERROR_MSG_OK = "ok";
constexpr const char* DM_ERROR_MSG_INIT_DMS_PROXY_LOCKED = "init dms proxy locked";
constexpr const char* DM_ERROR_MSG_IPC_FAILED = "ipc failed";
constexpr const char* DM_ERROR_MSG_REMOTE_CREATE_FAILED = "remote create failed";
constexpr const char* DM_ERROR_MSG_NULLPTR = "nullptr";
constexpr const char* DM_ERROR_MSG_INVALID_PARAM = "invalid param";
constexpr const char* DM_ERROR_MSG_WRITE_INTERFACE_TOKEN_FAILED = "write interface token failed";
constexpr const char* DM_ERROR_MSG_DEATH_RECIPIENT = "death recipient";
constexpr const char* DM_ERROR_MSG_INVALID_MODE_ID = "invalid mode id";
constexpr const char* DM_ERROR_MSG_WRITE_DATA_FAILED = "write data failed";
constexpr const char* DM_ERROR_MSG_RENDER_SERVICE_FAILED = "render service failed";
constexpr const char* DM_ERROR_MSG_UNREGISTER_AGENT_FAILED = "unregister agent failed";
constexpr const char* DM_ERROR_MSG_INVALID_CALLING = "invalid calling";
constexpr const char* DM_ERROR_MSG_INVALID_PERMISSION = "invalid permission";
constexpr const char* DM_ERROR_MSG_NOT_SYSTEM_APP = "not system app";
constexpr const char* DM_ERROR_MSG_DEVICE_NOT_SUPPORT = "device not support";
constexpr const char* DM_ERROR_MSG_UNKNOWN = "unknown";

static std::map<DMError, const char*> DM_ERROR_TO_ERROR_MSG_MAP {
    {DMError::DM_OK,                                    DM_ERROR_MSG_OK                           },
    {DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED,           DM_ERROR_MSG_INIT_DMS_PROXY_LOCKED        },
    {DMError::DM_ERROR_IPC_FAILED,                      DM_ERROR_MSG_IPC_FAILED                   },
    {DMError::DM_ERROR_REMOTE_CREATE_FAILED,            DM_ERROR_MSG_REMOTE_CREATE_FAILED         },
    {DMError::DM_ERROR_NULLPTR,                         DM_ERROR_MSG_NULLPTR                      },
    {DMError::DM_ERROR_INVALID_PARAM,                   DM_ERROR_MSG_INVALID_PARAM                },
    {DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED,    DM_ERROR_MSG_WRITE_INTERFACE_TOKEN_FAILED },
    {DMError::DM_ERROR_DEATH_RECIPIENT,                 DM_ERROR_MSG_DEATH_RECIPIENT              },
    {DMError::DM_ERROR_INVALID_MODE_ID,                 DM_ERROR_MSG_INVALID_MODE_ID              },
    {DMError::DM_ERROR_WRITE_DATA_FAILED,               DM_ERROR_MSG_WRITE_DATA_FAILED            },
    {DMError::DM_ERROR_RENDER_SERVICE_FAILED,           DM_ERROR_MSG_RENDER_SERVICE_FAILED        },
    {DMError::DM_ERROR_UNREGISTER_AGENT_FAILED,         DM_ERROR_MSG_UNREGISTER_AGENT_FAILED      },
    {DMError::DM_ERROR_INVALID_CALLING,                 DM_ERROR_MSG_INVALID_CALLING              },
    {DMError::DM_ERROR_INVALID_PERMISSION,              DM_ERROR_MSG_INVALID_PERMISSION           },
    {DMError::DM_ERROR_NOT_SYSTEM_APP,                  DM_ERROR_MSG_NOT_SYSTEM_APP               },
    {DMError::DM_ERROR_DEVICE_NOT_SUPPORT,              DM_ERROR_MSG_DEVICE_NOT_SUPPORT           },
    {DMError::DM_ERROR_UNKNOWN,                         DM_ERROR_MSG_UNKNOWN                      },
};

constexpr const char* DM_ERROR_CODE_MSG_OK = "ok";
constexpr const char* DM_ERROR_CODE_MSG_NO_PERMISSION = "no permission";
constexpr const char* DM_ERROR_CODE_MSG_NOT_SYSTEM_APP = "not system app";
constexpr const char* DM_ERROR_CODE_MSG_INVALID_PARAM = "invalid param";
constexpr const char* DM_ERROR_CODE_MSG_DEVICE_NOT_SUPPORT = "device not support";
constexpr const char* DM_ERROR_CODE_MSG_INVALID_SCREEN = "invalid screen";
constexpr const char* DM_ERROR_CODE_MSG_INVALID_CALLING = "invalid calling";
constexpr const char* DM_ERROR_CODE_MSG_SYSTEM_INNORMAL = "system innormal";

static std::map<DmErrorCode, const char*> DM_ERROR_CODE_TO_ERROR_MSG_MAP {
    {DmErrorCode::DM_OK,                              DM_ERROR_CODE_MSG_OK                 },
    {DmErrorCode::DM_ERROR_NO_PERMISSION,             DM_ERROR_CODE_MSG_NO_PERMISSION      },
    {DmErrorCode::DM_ERROR_NOT_SYSTEM_APP,            DM_ERROR_CODE_MSG_NOT_SYSTEM_APP     },
    {DmErrorCode::DM_ERROR_INVALID_PARAM,             DM_ERROR_CODE_MSG_INVALID_PARAM      },
    {DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT,        DM_ERROR_CODE_MSG_DEVICE_NOT_SUPPORT },
    {DmErrorCode::DM_ERROR_INVALID_SCREEN,            DM_ERROR_CODE_MSG_INVALID_SCREEN     },
    {DmErrorCode::DM_ERROR_INVALID_CALLING,           DM_ERROR_CODE_MSG_INVALID_CALLING    },
    {DmErrorCode::DM_ERROR_SYSTEM_INNORMAL,           DM_ERROR_CODE_MSG_SYSTEM_INNORMAL    },
};

std::string AniErrUtils::GetErrorMsg(const DMError& errorCode)
{
    return DM_ERROR_TO_ERROR_MSG_MAP.find(errorCode) != DM_ERROR_TO_ERROR_MSG_MAP.end() ?
        DM_ERROR_TO_ERROR_MSG_MAP.at(errorCode) : "";
}

std::string AniErrUtils::GetErrorMsg(const DmErrorCode& errorCode)
{
    return DM_ERROR_CODE_TO_ERROR_MSG_MAP.find(errorCode) != DM_ERROR_CODE_TO_ERROR_MSG_MAP.end() ?
        DM_ERROR_CODE_TO_ERROR_MSG_MAP.at(errorCode) : "";
}

ani_object AniErrUtils::CreateAniError(ani_env* env, const DMError& errorCode, const std::string& message)
{
    auto msg = message == "" ? GetErrorMsg(errorCode) : message;
    ani_string aniMsg;
    env->String_NewUTF8(msg.c_str(), msg.size(), &aniMsg);
    ani_object aniError = nullptr;
    ani_class cls;
    if (ANI_OK != env->FindClass("escompat.Error", &cls)) {
        TLOGE(WmsLogTag::DMS, "[ANI] null class FoldCreaseRegionImpl");
    }
    DisplayAniUtils::NewAniObject(env, cls, "C{std.core.String}C{escompat.ErrorOptions}:", &aniError, aniMsg);
    return aniError;
}

ani_object AniErrUtils::CreateAniError(ani_env* env, const DmErrorCode& errorCode, const std::string& message)
{
    auto msg = message == "" ? GetErrorMsg(errorCode) : message;
    ani_string aniMsg;
    env->String_NewUTF8(msg.c_str(), msg.size(), &aniMsg);
    ani_object aniError = nullptr;
    ani_class cls;
    if (ANI_OK != env->FindClass("escompat.Error", &cls)) {
        TLOGE(WmsLogTag::DMS, "[ANI] null class FoldCreaseRegionImpl");
    }
    DisplayAniUtils::NewAniObject(env, cls, "C{std.core.String}C{escompat.ErrorOptions}:", &aniError, aniMsg);
    return aniError;
}

ani_status AniErrUtils::ThrowBusinessError(ani_env* env, DMError error, std::string message)
{
    ani_object aniError;
    CreateBusinessError(env, static_cast<int32_t>(error), message == "" ? GetErrorMsg(error) : message, &aniError);
    ani_status status = env->ThrowError(static_cast<ani_error>(aniError));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] fail to throw err, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    return ANI_OK;
}

ani_status AniErrUtils::ThrowBusinessError(ani_env* env, DmErrorCode error, std::string message)
{
    ani_object aniError;
    CreateBusinessError(env, static_cast<int32_t>(error), message == "" ? GetErrorMsg(error) : message, &aniError);
    ani_status status = env->ThrowError(static_cast<ani_error>(aniError));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] fail to throw err, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    return ANI_OK;
}

ani_status AniErrUtils::CreateBusinessError(ani_env* env, int32_t error, std::string message, ani_object* err)
{
    TLOGI(WmsLogTag::DMS, "[ANI] in");
    ani_class aniClass;
    ani_status status = env->FindClass("@ohos.base.BusinessError", &aniClass);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] class not found, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    ani_method aniCtor;
    status = env->Class_FindMethod(aniClass, "<ctor>", "C{std.core.String}C{escompat.ErrorOptions}:", &aniCtor);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] ctor not found, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    ani_string aniMsg;
    DisplayAniUtils::GetAniString(env, message, &aniMsg);
    status = env->Object_New(aniClass, aniCtor, err, aniMsg, DisplayAniUtils::CreateAniUndefined(env));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] fail to new err, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    status = env->Object_SetFieldByName_Int(*err, "code_", static_cast<ani_int>(error));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] fail to set code, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    return ANI_OK;
}

} // namespace OHOS::Rosen
