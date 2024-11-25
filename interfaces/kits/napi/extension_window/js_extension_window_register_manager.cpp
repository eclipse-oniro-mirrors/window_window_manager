/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "js_extension_window_register_manager.h"
#include "singleton_container.h"
#include "window_manager.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
const std::string WINDOW_SIZE_CHANGE_CB = "windowSizeChange";
const std::string WINDOW_RECT_CHANGE_CB = "windowRectChange";
const std::string AVOID_AREA_CHANGE_CB = "avoidAreaChange";
const std::string WINDOW_STAGE_EVENT_CB = "windowStageEvent";
const std::string WINDOW_EVENT_CB = "windowEvent";
}

JsExtensionWindowRegisterManager::JsExtensionWindowRegisterManager()
{
    // white register list for window
    listenerCodeMap_[CaseType::CASE_WINDOW] = {
        {WINDOW_SIZE_CHANGE_CB, ListenerType::WINDOW_SIZE_CHANGE_CB},
        {WINDOW_RECT_CHANGE_CB, ListenerType::WINDOW_RECT_CHANGE_CB},
        {AVOID_AREA_CHANGE_CB, ListenerType::AVOID_AREA_CHANGE_CB},
        {WINDOW_EVENT_CB, ListenerType::WINDOW_EVENT_CB},
    };
    // white register list for window stage
    listenerCodeMap_[CaseType::CASE_STAGE] = {
        {WINDOW_STAGE_EVENT_CB, ListenerType::WINDOW_STAGE_EVENT_CB}
    };
}

JsExtensionWindowRegisterManager::~JsExtensionWindowRegisterManager()
{
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessWindowChangeRegister(sptr<JsExtensionWindowListener> listener,
    sptr<Window> window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessWindowRectChangeRegister(
    const sptr<JsExtensionWindowListener>& listener, const sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowRectChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowRectChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowRectChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessAvoidAreaChangeRegister(sptr<JsExtensionWindowListener> listener,
    sptr<Window> window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IAvoidAreaChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterAvoidAreaChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterAvoidAreaChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessLifeCycleEventRegister(sptr<JsExtensionWindowListener> listener,
    sptr<Window> window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowLifeCycle> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterLifeCycleListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterLifeCycleListener(thisListener));
    }
    return ret;
}

bool JsExtensionWindowRegisterManager::IsCallbackRegistered(napi_env env, std::string type, napi_value jsListenerObject)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]Method %{public}s has not been registered", type.c_str());
        return false;
    }

    for (auto iter = jsCbMap_[type].begin(); iter != jsCbMap_[type].end(); ++iter) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, iter->first->GetNapiValue(), &isEquals);
        if (isEquals) {
            TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Method %{public}s has already been registered", type.c_str());
            return true;
        }
    }
    return false;
}

WmErrorCode JsExtensionWindowRegisterManager::RegisterListener(sptr<Window> window, std::string type,
    CaseType caseType, napi_env env, napi_value value)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (IsCallbackRegistered(env, type, value)) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listenerCodeMap_[caseType].count(type) == 0) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    std::shared_ptr<NativeReference> callbackRef(reinterpret_cast<NativeReference*>(result));
    sptr<JsExtensionWindowListener> extensionWindowListener =
        new(std::nothrow) JsExtensionWindowListener(env, callbackRef);
    if (extensionWindowListener == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]New JsExtensionWindowListener failed");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    extensionWindowListener->SetMainEventHandler();
    WmErrorCode ret = ProcessRegister(caseType, extensionWindowListener, window, type, true);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Register type %{public}s failed", type.c_str());
        return ret;
    }
    jsCbMap_[type][callbackRef] = extensionWindowListener;
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]Register type %{public}s success! callback map size: %{public}zu", type.c_str(),
          jsCbMap_[type].size());
    return WmErrorCode::WM_OK;
}

WmErrorCode JsExtensionWindowRegisterManager::UnregisterListener(sptr<Window> window, std::string type,
    CaseType caseType, napi_env env, napi_value value)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Type %{public}s was not registered", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listenerCodeMap_[caseType].count(type) == 0) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (value == nullptr) {
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
            WmErrorCode ret = ProcessRegister(caseType, it->second, window, type, false);
            if (ret != WmErrorCode::WM_OK) {
                TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Unregister type %{public}s failed, no value", type.c_str());
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
            WmErrorCode ret = ProcessRegister(caseType, it->second, window, type, false);
            if (ret != WmErrorCode::WM_OK) {
                TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Unregister type %{public}s failed", type.c_str());
                return ret;
            }
            jsCbMap_[type].erase(it);
            break;
        }
        if (!findFlag) {
            TLOGE(WmsLogTag::WMS_UIEXT,
                "[NAPI]Unregister type %{public}s failed because not found callback!", type.c_str());
            return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        }
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]Unregister type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    // erase type when there is no callback in one type
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    return WmErrorCode::WM_OK;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessRegister(CaseType caseType,
    const sptr<JsExtensionWindowListener>& listener, const sptr<Window>& window, const std::string& type,
    bool isRegister)
{
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (caseType == CaseType::CASE_WINDOW) {
        switch (listenerCodeMap_[caseType][type]) {
            case ListenerType::WINDOW_SIZE_CHANGE_CB:
                ret = ProcessWindowChangeRegister(listener, window, isRegister);
                break;
            case ListenerType::WINDOW_RECT_CHANGE_CB:
                ret = ProcessWindowRectChangeRegister(listener, window, isRegister);
                break;
            case ListenerType::AVOID_AREA_CHANGE_CB:
                ret = ProcessAvoidAreaChangeRegister(listener, window, isRegister);
                break;
            case ListenerType::WINDOW_EVENT_CB:
                ret = ProcessLifeCycleEventRegister(listener, window, isRegister);
                break;
            default:
                break;
        }
    } else if (caseType == CaseType::CASE_STAGE) {
        if (type == WINDOW_STAGE_EVENT_CB) {
            ret = ProcessLifeCycleEventRegister(listener, window, isRegister);
        }
    }
    return ret;
}

} // namespace Rosen
} // namespace OHOS