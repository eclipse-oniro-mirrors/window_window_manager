/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_NAPI_GRAPHIC_COMMON_COMMON_H
#define INTERFACES_KITS_NAPI_GRAPHIC_COMMON_COMMON_H

#include <cstdint>
#include <node_api.h>
#include <node_api_types.h>
#include <memory>
#include <string>

#include "js_native_api.h"
#include "js_native_api_types.h"
#include "window_manager_hilog.h"
#include "dm_common.h"
#include "napi/native_api.h"

constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, OHOS::Rosen::HILOG_DOMAIN_WINDOW,
                                                "NapiWindowManagerCommonLayer" };

const int PARAM_NUMBER = 2; // 2: callback func input number, also reused by Promise

#define GNAPI_LOG(fmt, ...) HILOG_IMPL(LABEL.type, LOG_INFO, LABEL.domain, LABEL.tag, \
    "%{public}s: " fmt, __func__, ##__VA_ARGS__)

#define GNAPI_ASSERT(env, assertion, fmt, ...)  \
    do {                                        \
        if (assertion) {                        \
            GNAPI_LOG(fmt, ##__VA_ARGS__);      \
            return nullptr;                     \
        }                                       \
    } while (0)

namespace OHOS {
napi_status SetMemberInt32(napi_env env, napi_value result, const char *key, int32_t value);
napi_status SetMemberUint32(napi_env env, napi_value result, const char *key, uint32_t value);
napi_status SetMemberUndefined(napi_env env, napi_value result, const char *key);

bool CheckCallingPermission(const std::string& permission);
void SetErrorInfo(napi_env env, Rosen::DmErrorCode wret, const std::string& errMessage,
    napi_value result[], int count);
void ProcessPromise(napi_env env, Rosen::DmErrorCode wret, napi_deferred deferred,
    napi_value result[], int count);
void ProcessCallback(napi_env env, napi_ref ref, napi_value result[], int count);
bool NAPICall(napi_env env, napi_status status);

template<typename ParamT>
struct AsyncCallbackInfo {
    napi_async_work asyncWork;
    napi_deferred deferred;
    void (*async)(napi_env env, std::unique_ptr<ParamT>& param);
    napi_value (*resolve)(napi_env env, std::unique_ptr<ParamT>& param);
    std::unique_ptr<ParamT> param;
    napi_ref ref;
};

template<typename ParamT>
void AsyncFunc(napi_env env, void *data)
{
    AsyncCallbackInfo<ParamT> *info = reinterpret_cast<AsyncCallbackInfo<ParamT> *>(data);
    if (info->async) {
        info->async(env, info->param);
    }
}

template<typename ParamT>
void CompleteFunc(napi_env env, napi_status status, void *data)
{
    AsyncCallbackInfo<ParamT> *info = reinterpret_cast<AsyncCallbackInfo<ParamT> *>(data);
    napi_value result[PARAM_NUMBER] = {nullptr};
    if (info->param->wret == Rosen::DmErrorCode::DM_OK) {
        napi_get_undefined(env, &result[0]);
        result[1] = info->resolve(env, info->param);
    } else {
        SetErrorInfo(env, info->param->wret, info->param->errMessage, result, PARAM_NUMBER);
    }
    if (info->deferred) {
        ProcessPromise(env, info->param->wret, info->deferred, result, PARAM_NUMBER);
    } else {
        ProcessCallback(env, info->ref, result, PARAM_NUMBER);
    }
    napi_delete_async_work(env, info->asyncWork);
    delete info;
}

template<typename ParamT>
napi_value CreatePromise(napi_env env, napi_value resourceName,
    const std::string& funcname, AsyncCallbackInfo<ParamT>* info)
{
    napi_value result = nullptr;
    if (!NAPICall(env, napi_create_promise(env, &info->deferred, &result))) {
        return nullptr;
    }
    return result;
}

template<typename ParamT>
napi_value CreateUndefined(napi_env env, napi_value resourceName,
    const std::string& funcname, AsyncCallbackInfo<ParamT>* info)
{
    napi_value result = nullptr;
    if (!NAPICall(env, napi_get_undefined(env, &result))) {
        return nullptr;
    }
    return result;
}

template<typename ParamT>
napi_value AsyncProcess(napi_env env,
                        const std::string& funcname,
                        void(*async)(napi_env env, std::unique_ptr<ParamT>& param),
                        napi_value(*resolve)(napi_env env, std::unique_ptr<ParamT>& param),
                        napi_ref& callbackRef,
                        std::unique_ptr<ParamT>& param)
{
    AsyncCallbackInfo<ParamT> *info = new AsyncCallbackInfo<ParamT> {
        .async = async,
        .resolve = resolve,
        .param = std::move(param),
        .ref = callbackRef,
    };

    napi_value resourceName = nullptr;
    if (!NAPICall(env, napi_create_string_latin1(env, funcname.c_str(), NAPI_AUTO_LENGTH, &resourceName))) {
        delete info;
        if (callbackRef != nullptr) {
            static_cast<void>(napi_delete_reference(env, callbackRef));
        }
        return nullptr;
    }

    napi_value result = nullptr;
    if (callbackRef == nullptr) {
        result = CreatePromise(env, resourceName, funcname, info);
    } else {
        result = CreateUndefined(env, resourceName, funcname, info);
    }

    if (result == nullptr) {
        delete info;
        if (callbackRef != nullptr) {
            static_cast<void>(napi_delete_reference(env, callbackRef));
        }
        return nullptr;
    }

    if (!NAPICall(env, napi_create_async_work(env, nullptr, resourceName, AsyncFunc<ParamT>,
        CompleteFunc<ParamT>, reinterpret_cast<void *>(info), &info->asyncWork))) {
        delete info;
        if (callbackRef != nullptr) {
            static_cast<void>(napi_delete_reference(env, callbackRef));
        }
        return nullptr;
    }
    if (!NAPICall(env, napi_queue_async_work_with_qos(env, info->asyncWork, napi_qos_user_initiated))) {
        delete info;
        if (callbackRef != nullptr) {
            static_cast<void>(napi_delete_reference(env, callbackRef));
        }
        return nullptr;
    }

    return result;
};
} // namespace OHOS

#endif // INTERFACES_KITS_NAPI_GRAPHIC_COMMON_COMMON_H
