/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "js_window_register_manager.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsRegisterManager"};

const std::map<std::string, ListenerFunctionType> WindowListenerFunctionMap {
    {SYSTEM_AVOID_AREA_CHANGE_CB, ListenerFunctionType::SYSTEM_AVOID_AREA_CHANGE_CB},
    {AVOID_AREA_CHANGE_CB, ListenerFunctionType::AVOID_AREA_CHANGE_CB},
};

const std::map<CaseType, std::map<std::string, ListenerFunctionType>> ListenerFunctionMap {
    {CaseType::CASE_WINDOW, WindowListenerFunctionMap},
};
}

JsWindowRegisterManager::JsWindowRegisterManager()
{
}

JsWindowRegisterManager::~JsWindowRegisterManager()
{
}

WmErrorCode JsWindowRegisterManager::ProcessSystemAvoidAreaChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        WLOGFE("Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listener == nullptr) {
        WLOGFE("listener is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    listener->SetIsDeprecatedInterface(true);
    sptr<IAvoidAreaChangedListener> thisListener(listener);
    WmErrorCode ret;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterAvoidAreaChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterAvoidAreaChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessAvoidAreaChangeRegister(sptr<JsWindowListener> listener,
    sptr<Window> window, bool isRegister, napi_env env, napi_value parameter)
{
    if (window == nullptr) {
        WLOGFE("Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IAvoidAreaChangedListener> thisListener(listener);
    WmErrorCode ret;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterAvoidAreaChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterAvoidAreaChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::RegisterListener(sptr<Window> window, std::string type,
    CaseType caseType, napi_env env, napi_value callback, napi_value parameter)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (IsCallbackRegistered(env, type, callback)) {
        return WmErrorCode::WM_OK;
    }
    auto iterCaseType = ListenerFunctionMap.find(caseType);
    if (iterCaseType == ListenerFunctionMap.end()) {
        WLOGFE("CaseType %{public}u is not supported", static_cast<uint32_t>(caseType));
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    auto iterCallbackType = iterCaseType->second.find(type);
    if (iterCallbackType == iterCaseType->second.end()) {
        WLOGFE("Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    ListenerFunctionType listenerFunctionType = iterCallbackType->second;
    napi_ref result = nullptr;
    napi_create_reference(env, callback, 1, &result);
    std::shared_ptr<NativeReference> callbackRef(reinterpret_cast<NativeReference*>(result));
    sptr<JsWindowListener> windowManagerListener = new(std::nothrow) JsWindowListener(env, callbackRef);
    if (windowManagerListener == nullptr) {
        WLOGFE("New JsWindowListener failed");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    windowManagerListener->SetMainEventHandler();
    WmErrorCode ret = ProcessRegisterCallback(listenerFunctionType, caseType, windowManagerListener, window,
        true, env, parameter);
    if (ret != WmErrorCode::WM_OK) {
        WLOGFE("Register type %{public}s failed", type.c_str());
        return ret;
    }
    jsCbMap_[type][callbackRef] = windowManagerListener;
    WLOGI("Register type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    return WmErrorCode::WM_OK;
}

WmErrorCode JsWindowRegisterManager::ProcessRegisterCallback(ListenerFunctionType listenerFunctionType,
    CaseType caseType, const sptr<JsWindowListener>& listener, const sptr<Window>& window, bool isRegister,
    napi_env env, napi_value parameter)
{
    if (caseType == CaseType::CASE_WINDOW) {
        switch (listenerFunctionType) {
            case ListenerFunctionType::SYSTEM_AVOID_AREA_CHANGE_CB:
                return ProcessSystemAvoidAreaChangeRegister(listener, window, isRegister, env, parameter);
            case ListenerFunctionType::AVOID_AREA_CHANGE_CB:
                return ProcessAvoidAreaChangeRegister(listener, window, isRegister, env, parameter);
            default:
                return WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
    }
    return WmErrorCode::WM_ERROR_INVALID_PARAM;
}

WmErrorCode JsWindowRegisterManager::UnregisterListener(sptr<Window> window, std::string type,
    CaseType caseType, napi_env env, napi_value value)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGFE("Type %{public}s was not registerted", type.c_str());
        return WmErrorCode::WM_OK;
    }
    auto iterCaseType = ListenerFunctionMap.find(caseType);
    if (iterCaseType == ListenerFunctionMap.end()) {
        WLOGFE("CaseType %{public}u is not supported", static_cast<uint32_t>(caseType));
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    auto iterCallbackType = iterCaseType->second.find(type);
    if (iterCallbackType == iterCaseType->second.end()) {
        WLOGFE("Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    ListenerFunctionType listenerFunctionType = iterCallbackType->second;
    if (value == nullptr) {
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
            WmErrorCode ret = ProcessRegisterCallback(listenerFunctionType, caseType, it->second, window, false,
                env, nullptr);
            if (ret != WmErrorCode::WM_OK) {
                WLOGFE("Unregister type %{public}s failed, no value", type.c_str());
                return ret;
            }
            jsCbMap_[type].erase(it++);
        }
    } else {
        bool findFlag = false;
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end(); ++it) {
            bool isEquals = false;
            napi_strict_equals(env, value, it->first->GetNapiValue(), &isEquals);
            if (!isEquals) {
                continue;
            }
            findFlag = true;
            WmErrorCode ret = ProcessRegisterCallback(listenerFunctionType, caseType, it->second, window, false,
                env, nullptr);
            if (ret != WmErrorCode::WM_OK) {
                WLOGFE("Unregister type %{public}s failed", type.c_str());
                return ret;
            }
            jsCbMap_[type].erase(it);
            break;
        }
        if (!findFlag) {
            WLOGFE("Unregister type %{public}s failed because not found callback!", type.c_str());
            return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        }
    }
    WLOGI("Unregister type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    // erase type when there is no callback in one type
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    return WmErrorCode::WM_OK;
}

WmErrorCode JsWindowRegisterManager::RegisterWindowPostureModeListener(napi_env env, sptr<Window> window,
    napi_value callback, WindowPostureMode mode)
{
    return WmErrorCode::WM_OK;
}

WmErrorCode JsWindowRegisterManager::UnregisterWindowPostureModeListener(napi_env env, sptr<Window> window,
    napi_value callback, WindowPostureMode mode)
{
    return WmErrorCode::WM_OK;
}

bool JsWindowRegisterManager::IsCallbackRegistered(napi_env env, std::string& type, napi_value jsListenerObject)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGI("Method %{public}s has not been registerted", type.c_str());
        return false;
    }

    for (auto iter = jsCbMap_[type].begin(); iter != jsCbMap_[type].end(); ++iter) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, iter->first->GetNapiValue(), &isEquals);
        if (isEquals) {
            WLOGFE("Method %{public}s has already been registered", type.c_str());
            return true;
        }
    }
    return false;
}

} // namespace Rosen
} // namespace OHOS
