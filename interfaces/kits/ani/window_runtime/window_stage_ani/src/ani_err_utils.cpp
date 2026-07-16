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

#include "ani_window_utils.h"
#include "window_error_msg.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
ani_status AniErrUtils::ThrowBusinessError(ani_env* env, WMError error, std::string message)
{
    ani_object aniError;
    CreateBusinessError(env, static_cast<int32_t>(error), message == "" ? GetErrorMsg(error) : message, &aniError);
    ani_status status = env->ThrowError(static_cast<ani_error>(aniError));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to throw err, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    return ANI_OK;
}

ani_status AniErrUtils::ThrowBusinessError(ani_env* env, WmErrorCode error, std::string message)
{
    ani_object aniError;
    CreateBusinessError(env, static_cast<int32_t>(error), message == "" ? GetErrorMsg(error) : message, &aniError);
    ani_status status = env->ThrowError(static_cast<ani_error>(aniError));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to throw err, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    return ANI_OK;
}

ani_status AniErrUtils::CreateBusinessError(ani_env* env, int32_t error, std::string message, ani_object* err)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI] in");
    ani_class aniClass;
    ani_status status = env->FindClass("@ohos.base.BusinessError", &aniClass);
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
    AniWindowUtils::GetAniString(env, message, &aniMsg);
    status = env->Object_New(aniClass, aniCtor, err, aniMsg, AniWindowUtils::CreateAniUndefined(env));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to new err, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    status = env->Object_SetFieldByName_Int(*err, "code_", static_cast<ani_int>(error));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] fail to set code, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    return ANI_OK;
}

std::string AniErrUtils::GetErrorMsg(WMError error)
{
    return WindowErrorMsg::GetErrorMsg(error);
}

std::string AniErrUtils::GetErrorMsg(WmErrorCode error)
{
    return WindowErrorMsg::GetFullErrorMsg(error);
}
} // namespace OHOS::Rosen
