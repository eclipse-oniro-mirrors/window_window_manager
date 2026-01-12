/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef WINDOW_WINDOW_MANAGER_JS_ERROR_UTILS_H
#define WINDOW_WINDOW_MANAGER_JS_ERROR_UTILS_H

#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"

#include "dm_common.h"
#include "wm_common.h"

namespace OHOS::Rosen {
class JsErrUtils {
public:
    static napi_value CreateJsError(napi_env env, const WMError& errorCode, std::string msg = "");
    static napi_value CreateJsError(napi_env env, const WmErrorCode& errorCode, std::string msg = "");
    static napi_value CreateJsError(napi_env env, const DMError& errorCode, std::string msg = "");
    static napi_value CreateJsError(napi_env env, const DmErrorCode& errorCode, std::string msg = "");
private:
    static inline napi_value CreateJsNumber(napi_env env, int32_t value)
    {
        napi_value result = nullptr;
        napi_create_int32(env, value, &result);
        return result;
    }

    static inline napi_value CreateJsNumber(napi_env env, uint32_t value)
    {
        napi_value result = nullptr;
        napi_create_uint32(env, value, &result);
        return result;
    }

    static inline napi_value CreateJsNumber(napi_env env, int64_t value)
    {
        napi_value result = nullptr;
        napi_create_int64(env, value, &result);
        return result;
    }

    static inline napi_value CreateJsNumber(napi_env env, double value)
    {
        napi_value result = nullptr;
        napi_create_double(env, value, &result);
        return result;
    }

    template<class T>
    static napi_value CreateJsValue(napi_env env, const T& value);

    static std::string GetPreErrorMsg(const WmErrorCode& errorCode);
    static std::string GetPreErrorMsg(const DmErrorCode& errorCode);
    static std::string GetErrorMsg(const WMError& errorCode);
    static std::string GetErrorMsg(const WmErrorCode& errorCode);
    static std::string GetErrorMsg(const DMError& errorCode);
    static std::string GetErrorMsg(const DmErrorCode& errorCode);
};
} // namespace OHOS::Rosen

#endif //WINDOW_WINDOW_MANAGER_JS_ERROR_UTILS_H
