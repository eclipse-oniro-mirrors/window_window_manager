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

#include "js_err_utils.h"

#include "window_error_msg.h"

namespace OHOS::Rosen {
template<class T>
napi_value JsErrUtils::CreateJsValue(napi_env env, const T& value)
{
    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
    napi_value result = nullptr;
    if constexpr (std::is_same_v<ValueType, bool>) {
        napi_get_boolean(env, value, &result);
        return result;
    } else if constexpr (std::is_arithmetic_v<ValueType>) {
        return CreateJsNumber(env, value);
    } else if constexpr (std::is_same_v<ValueType, std::string>) {
        napi_create_string_utf8(env, value.c_str(), value.length(), &result);
        return result;
    } else if constexpr (std::is_enum_v<ValueType>) {
        return CreateJsNumber(env, static_cast<std::make_signed_t<ValueType>>(value));
    } else if constexpr (std::is_same_v<ValueType, const char*>) {
        (value != nullptr) ? napi_create_string_utf8(env, value, strlen(value), &result) :
            napi_get_undefined(env, &result);
        return result;
    }
}

std::string JsErrUtils::GetPreErrorMsg(const WmErrorCode& errorCode)
{
    return WindowErrorMsg::GetShortErrorMsg(errorCode);
}

std::string JsErrUtils::GetPreErrorMsg(const DmErrorCode& errorCode)
{
    return WindowErrorMsg::GetShortErrorMsg(errorCode);
}

std::string JsErrUtils::GetErrorMsg(const WMError& errorCode)
{
    return WindowErrorMsg::GetErrorMsg(errorCode);
}

std::string JsErrUtils::GetErrorMsg(const WmErrorCode& errorCode)
{
    return WindowErrorMsg::GetFullErrorMsg(errorCode);
}

std::string JsErrUtils::GetErrorMsg(const DMError& errorCode)
{
    return WindowErrorMsg::GetErrorMsg(errorCode);
}

std::string JsErrUtils::GetErrorMsg(const DmErrorCode& errorCode)
{
    return WindowErrorMsg::GetFullErrorMsg(errorCode);
}

napi_value JsErrUtils::CreateJsError(napi_env env, const WMError& errorCode, std::string msg)
{
    napi_value result = nullptr;
    napi_create_error(env, CreateJsValue(env, static_cast<int32_t>(errorCode)),
        CreateJsValue(env, msg == "" ? GetErrorMsg(errorCode) : msg), &result);
    return result;
}

napi_value JsErrUtils::CreateJsError(napi_env env, const WmErrorCode& errorCode, std::string msg)
{
    napi_value result = nullptr;
    napi_create_error(env, CreateJsValue(env, static_cast<int32_t>(errorCode)),
        CreateJsValue(env, WindowErrorMsg::BuildErrorMsg(errorCode, msg)), &result);
    return result;
}

napi_value JsErrUtils::CreateJsError(napi_env env, const DMError& errorCode, std::string msg)
{
    napi_value result = nullptr;
    napi_create_error(env, CreateJsValue(env, static_cast<int32_t>(errorCode)),
        CreateJsValue(env, msg == "" ? GetErrorMsg(errorCode) : msg), &result);
    return result;
}

napi_value JsErrUtils::CreateJsError(napi_env env, const DmErrorCode& errorCode, std::string msg)
{
    napi_value result = nullptr;
    napi_create_error(env, CreateJsValue(env, static_cast<int32_t>(errorCode)),
        CreateJsValue(env, WindowErrorMsg::BuildErrorMsg(errorCode, msg)), &result);
    return result;
}
} // namespace OHOS::Rosen
