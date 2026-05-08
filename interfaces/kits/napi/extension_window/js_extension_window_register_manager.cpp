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
#include "window_histogram_management.h"

namespace OHOS {
namespace Rosen {
namespace {
const std::string WINDOW_SIZE_CHANGE_CB = "windowSizeChange";
const std::string WINDOW_RECT_CHANGE_CB = "rectChange";
const std::string AVOID_AREA_CHANGE_CB = "avoidAreaChange";
const std::string WINDOW_STAGE_EVENT_CB = "windowStageEvent";
const std::string WINDOW_EVENT_CB = "windowEvent";
const std::string WINDOW_DISPLAYID_CHANGE_CB = "displayIdChange";
const std::string SYSTEM_DENSITY_CHANGE_CB = "systemDensityChange";
const std::string SCREENSHOT_EVENT_CB = "screenshot";
const std::string EXTENSION_SECURE_LIMIT_CHANGE_CB = "uiExtensionSecureLimitChange";
const std::string KEYBOARD_DID_SHOW_CB = "keyboardDidShow";
const std::string KEYBOARD_DID_HIDE_CB = "keyboardDidHide";
const std::string KEYBOARD_HEIGHT_CHANGE_CB = "keyboardHeightChange";
const std::string WINDOW_STATUS_CHANGE_CB = "windowStatusChange";
}

JsExtensionWindowRegisterManager::JsExtensionWindowRegisterManager()
{
    // white register list for window
    listenerCodeMap_[CaseType::CASE_WINDOW] = {
        {WINDOW_SIZE_CHANGE_CB, ListenerType::WINDOW_SIZE_CHANGE_CB},
        {WINDOW_RECT_CHANGE_CB, ListenerType::WINDOW_RECT_CHANGE_CB},
        {AVOID_AREA_CHANGE_CB, ListenerType::AVOID_AREA_CHANGE_CB},
        {WINDOW_EVENT_CB, ListenerType::WINDOW_EVENT_CB},
        {WINDOW_DISPLAYID_CHANGE_CB, ListenerType::WINDOW_DISPLAYID_CHANGE_CB},
        {SYSTEM_DENSITY_CHANGE_CB, ListenerType::SYSTEM_DENSITY_CHANGE_CB},
        {SCREENSHOT_EVENT_CB, ListenerType::SCREENSHOT_EVENT_CB},
        {EXTENSION_SECURE_LIMIT_CHANGE_CB, ListenerType::EXTENSION_SECURE_LIMIT_CHANGE_CB},
        {KEYBOARD_DID_SHOW_CB, ListenerType::KEYBOARD_DID_SHOW_CB},
        {KEYBOARD_DID_HIDE_CB, ListenerType::KEYBOARD_DID_HIDE_CB},
        {KEYBOARD_HEIGHT_CHANGE_CB, ListenerType::KEYBOARD_HEIGHT_CHANGE_CB},
        {WINDOW_STATUS_CHANGE_CB, ListenerType::WINDOW_STATUS_CHANGE_CB},
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
        TLOGE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
        HISTOGRAM_ENUMERATION_ERROR_CODE(
            isRegister ? "ArkUI.window.onWindowSizeChange" : "ArkUI.window.offWindowSizeChange",
            WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowChangeListener(thisListener));
    }
    HISTOGRAM_ENUMERATION_ERROR_CODE(
        isRegister ? "ArkUI.window.onWindowSizeChange" : "ArkUI.window.offWindowSizeChange", ret);
    return ret;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessWindowRectChangeRegister(
    const sptr<JsExtensionWindowListener>& listener, const sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
        HISTOGRAM_ENUMERATION_ERROR_CODE(
            isRegister ? "ArkUI.window.onRectChange" : "ArkUI.window.offRectChange",
            WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WmErrorCode ret = isRegister ?
        WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowRectChangeListener(listener)) :
        WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowRectChangeListener(listener));
    HISTOGRAM_ENUMERATION_ERROR_CODE(
        isRegister ? "ArkUI.window.onRectChange" : "ArkUI.window.offRectChange", ret);
    return ret;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessAvoidAreaChangeRegister(sptr<JsExtensionWindowListener> listener,
    sptr<Window> window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
        HISTOGRAM_ENUMERATION_ERROR_CODE(
            isRegister ? "ArkUI.window.onAvoidAreaChange" : "ArkUI.window.offAvoidAreaChange",
            WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IAvoidAreaChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterAvoidAreaChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterAvoidAreaChangeListener(thisListener));
    }
    HISTOGRAM_ENUMERATION_ERROR_CODE(
        isRegister ? "ArkUI.window.onAvoidAreaChange" : "ArkUI.window.offAvoidAreaChange", ret);
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

WmErrorCode JsExtensionWindowRegisterManager::ProcessDisplayIdChangeRegister(
    const sptr<JsExtensionWindowListener>& listener, const sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    IDisplayIdChangeListenerSptr thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterDisplayIdChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterDisplayIdChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessSystemDensityChangeRegister(
    const sptr<JsExtensionWindowListener>& listener, const sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    ISystemDensityChangeListenerSptr thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterSystemDensityChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterSystemDensityChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessScreenshotRegister(
    const sptr<JsExtensionWindowListener>& listener, const sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IScreenshotListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterScreenshotListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterScreenshotListener(thisListener));
    }
    return ret;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessExtensionSecureLimitChangeRegister(
    const sptr<JsExtensionWindowListener>& listener, const sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IExtensionSecureLimitChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterExtensionSecureLimitChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterExtensionSecureLimitChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessKeyboardDidShowRegister(
    const sptr<JsExtensionWindowListener>& listener, const sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IKeyboardDidShowListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterKeyboardDidShowListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterKeyboardDidShowListener(thisListener));
    }
    return ret;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessKeyboardDidHideRegister(
    const sptr<JsExtensionWindowListener>& listener, const sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IKeyboardDidHideListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterKeyboardDidHideListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterKeyboardDidHideListener(thisListener));
    }
    return ret;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessOccupiedAreaChangeRegister(
    const sptr<JsExtensionWindowListener>& listener, const sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IOccupiedAreaChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterOccupiedAreaChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterOccupiedAreaChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsExtensionWindowRegisterManager::ProcessWindowStatusChangeRegister(
    const sptr<JsExtensionWindowListener>& listener, const sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowStatusChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowStatusChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowStatusChangeListener(thisListener));
    }
    return ret;
}

bool JsExtensionWindowRegisterManager::IsCallbackRegistered(napi_env env, std::string type, napi_value jsListenerObject)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::WMS_UIEXT, "Method %{public}s has not been registered", type.c_str());
        return false;
    }

    for (auto iter = jsCbMap_[type].begin(); iter != jsCbMap_[type].end(); ++iter) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, iter->first->GetNapiValue(), &isEquals);
        if (isEquals) {
            TLOGE(WmsLogTag::WMS_UIEXT, "Method %{public}s has already been registered", type.c_str());
            return true;
        }
    }
    return false;
}

WmErrorCode JsExtensionWindowRegisterManager::AtomicServiceRegisterListener(sptr<Window> window, std::string type,
    CaseType caseType, napi_env env, napi_value value)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (IsCallbackRegistered(env, type, value)) {
        return WmErrorCode::WM_OK;
    }
    return RegisterListenerInner(window, type, caseType, env, value);
}

WmErrorCode JsExtensionWindowRegisterManager::RegisterListener(sptr<Window> window, std::string type,
    CaseType caseType, napi_env env, napi_value value)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (IsCallbackRegistered(env, type, value)) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    return RegisterListenerInner(window, type, caseType, env, value);
}

WmErrorCode JsExtensionWindowRegisterManager::RegisterListenerInner(sptr<Window> window, std::string type,
    CaseType caseType, napi_env env, napi_value value)
{
    if (listenerCodeMap_[caseType].count(type) == 0) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    std::shared_ptr<NativeReference> callbackRef(reinterpret_cast<NativeReference*>(result));
    sptr<JsExtensionWindowListener> extensionWindowListener =
        new(std::nothrow) JsExtensionWindowListener(env, callbackRef);
    if (extensionWindowListener == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "New JsExtensionWindowListener failed");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    extensionWindowListener->SetMainEventHandler();
    WmErrorCode ret = ProcessRegister(caseType, extensionWindowListener, window, type, true);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Register type %{public}s failed", type.c_str());
        return ret;
    }
    jsCbMap_[type][callbackRef] = extensionWindowListener;
    TLOGI(WmsLogTag::WMS_UIEXT, "Register type %{public}s success! callback map size: %{public}zu", type.c_str(),
          jsCbMap_[type].size());
    return WmErrorCode::WM_OK;
}


WmErrorCode JsExtensionWindowRegisterManager::AtomicServiceUnregisterListener(sptr<Window> window, std::string type,
    CaseType caseType, napi_env env, napi_value value)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Type %{public}s was not registered", type.c_str());
        return WmErrorCode::WM_OK;
    }
    if (listenerCodeMap_[caseType].count(type) == 0) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (value == nullptr) {
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
            WmErrorCode ret = ProcessRegister(caseType, it->second, window, type, false);
            if (ret != WmErrorCode::WM_OK) {
                TLOGE(WmsLogTag::WMS_UIEXT, "Atomic Unregister type %{public}s failed, no value", type.c_str());
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
                TLOGE(WmsLogTag::WMS_UIEXT, "Atomic Unregister type %{public}s failed", type.c_str());
                return ret;
            }
            jsCbMap_[type].erase(it);
            break;
        }
        if (!findFlag) {
            TLOGW(WmsLogTag::WMS_UIEXT,
                "Atomic Unregister type %{public}s failed because not found callback!", type.c_str());
            return WmErrorCode::WM_OK;
        }
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Atomic Unregister type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    return WmErrorCode::WM_OK;
}

WmErrorCode JsExtensionWindowRegisterManager::UnregisterListener(sptr<Window> window, std::string type,
    CaseType caseType, napi_env env, napi_value value)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Type %{public}s was not registered", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listenerCodeMap_[caseType].count(type) == 0) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (value == nullptr) {
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
            WmErrorCode ret = ProcessRegister(caseType, it->second, window, type, false);
            if (ret != WmErrorCode::WM_OK) {
                TLOGE(WmsLogTag::WMS_UIEXT, "Unregister type %{public}s failed, no value", type.c_str());
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
                TLOGE(WmsLogTag::WMS_UIEXT, "Unregister type %{public}s failed", type.c_str());
                return ret;
            }
            jsCbMap_[type].erase(it);
            break;
        }
        if (!findFlag) {
            TLOGE(WmsLogTag::WMS_UIEXT,
                "Unregister type %{public}s failed because not found callback!", type.c_str());
            return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        }
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Unregister type %{public}s success! callback map size: %{public}zu",
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
            case ListenerType::WINDOW_DISPLAYID_CHANGE_CB:
                ret = ProcessDisplayIdChangeRegister(listener, window, isRegister);
                break;
            case ListenerType::SYSTEM_DENSITY_CHANGE_CB:
                ret = ProcessSystemDensityChangeRegister(listener, window, isRegister);
                break;
            case ListenerType::SCREENSHOT_EVENT_CB:
                ret = ProcessScreenshotRegister(listener, window, isRegister);
                break;
            case ListenerType::EXTENSION_SECURE_LIMIT_CHANGE_CB:
                ret = ProcessExtensionSecureLimitChangeRegister(listener, window, isRegister);
                break;
            case ListenerType::KEYBOARD_DID_SHOW_CB:
                ret = ProcessKeyboardDidShowRegister(listener, window, isRegister);
                break;
            case ListenerType::KEYBOARD_DID_HIDE_CB:
                ret = ProcessKeyboardDidHideRegister(listener, window, isRegister);
                break;
            case ListenerType::KEYBOARD_HEIGHT_CHANGE_CB:
                ret = ProcessOccupiedAreaChangeRegister(listener, window, isRegister);
                break;
            case ListenerType::WINDOW_STATUS_CHANGE_CB:
                ret = ProcessWindowStatusChangeRegister(listener, window, isRegister);
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