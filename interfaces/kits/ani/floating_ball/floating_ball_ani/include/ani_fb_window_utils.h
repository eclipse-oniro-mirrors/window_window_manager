/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef ANI_FB_WINDOW_UTILS_H
#define ANI_FB_WINDOW_UTILS_H

#include "ani.h"
#include "window.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace Rosen {
    ani_ref AniGetUndefined(ani_env* env);
    std::string GetErrorMsg(WMError error);
    std::string GetErrorMsg(WmErrorCode error);
    ani_status CreateBusinessError(ani_env* env, int32_t error, std::string message, ani_object* err);
    ani_object AniThrowError(ani_env* env, WMError errorCode, std::string msg = "");
    ani_object AniThrowError(ani_env* env, WmErrorCode errorCode, std::string msg = "");
    ani_status GetStdString(ani_env* env, ani_string ani_str, std::string &result);
    void* GetAbilityContext(ani_env* env, ani_object aniObj);
    ani_status GetContextPtr(ani_env* env, ani_object floatingBallConfigurations, void*& contextPtr);
    bool IsInstanceOf(ani_env* env, ani_object obj, const char* className);
    ani_status CallAniFunctionVoid(ani_env* env, const char* ns, const char* fn, const char* signature, ...);
    ani_status GetAniString(ani_env* env, const std::string& str, ani_string* result);
    ani_object CreateAniUndefined(ani_env* env);

    template <typename T>
    ani_status ThrowBusinessError(ani_env* env, T errorCode, const std::string& message)
    {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
        std::string errorMessage = message.empty() ? GetErrorMsg(errorCode) : message;
        ani_object aniError;
        ani_status status = CreateBusinessError(env, static_cast<int32_t>(errorCode), errorMessage, &aniError);
        if (status != ANI_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]fail to new aniError, status:%{public}d", static_cast<int32_t>(status));
            return status;
        }
        status = env->ThrowError(static_cast<ani_error>(aniError));
        if (status != ANI_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]fail to ThrowAniError, status:%{public}d", static_cast<int32_t>(status));
            return status;
        }
        return ANI_OK;
    }

    template <typename T>
    ani_object AniThrowError(ani_env* env, T errorCode, const std::string& msg)
    {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
        ThrowBusinessError<T>(env, errorCode, msg);
        return CreateAniUndefined(env);
    }

    }
}
#endif // ANI_FB_WINDOW_UTILS_H