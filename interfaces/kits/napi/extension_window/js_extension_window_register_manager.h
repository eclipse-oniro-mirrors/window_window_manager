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

#ifndef OHOS_JS_EXTENSION_WINDOW_REGISTER_MANAGER_H
#define OHOS_JS_EXTENSION_WINDOW_REGISTER_MANAGER_H

#include <map>
#include <mutex>
#include "js_extension_window_listener.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"
#include "refbase.h"
#include "window.h"

namespace OHOS {
namespace Rosen {
class JsExtensionWindowRegisterManager {
public:
    JsExtensionWindowRegisterManager();
    ~JsExtensionWindowRegisterManager();
    WmErrorCode RegisterListener(sptr<Window> window, std::string type,
        CaseType caseType, napi_env env, napi_value value);
    WmErrorCode UnregisterListener(sptr<Window> window, std::string type,
        CaseType caseType, napi_env env, napi_value value);
    WmErrorCode AtomicServiceRegisterListener(sptr<Window> window, std::string type,
        CaseType caseType, napi_env env, napi_value value);
    WmErrorCode AtomicServiceUnregisterListener(sptr<Window> window, std::string type,
        CaseType caseType, napi_env env, napi_value value);

private:
    enum class ListenerType : uint32_t {
        WINDOW_SIZE_CHANGE_CB,
        WINDOW_RECT_CHANGE_CB,
        AVOID_AREA_CHANGE_CB,
        WINDOW_EVENT_CB,
        WINDOW_STAGE_EVENT_CB,
        WINDOW_DISPLAYID_CHANGE_CB,
        SYSTEM_DENSITY_CHANGE_CB,
        SCREENSHOT_EVENT_CB,
        EXTENSION_SECURE_LIMIT_CHANGE_CB,
        KEYBOARD_DID_SHOW_CB,
        KEYBOARD_DID_HIDE_CB,
        KEYBOARD_HEIGHT_CHANGE_CB,
        WINDOW_STATUS_CHANGE_CB,
    };

    bool IsCallbackRegistered(napi_env env, std::string type, napi_value jsListenerObject);
    WmErrorCode RegisterListenerInner(sptr<Window> window, std::string type,
        CaseType caseType, napi_env env, napi_value value);
    WmErrorCode ProcessWindowChangeRegister(sptr<JsExtensionWindowListener> listener,
        sptr<Window> window, bool isRegister);
    WmErrorCode ProcessWindowRectChangeRegister(const sptr<JsExtensionWindowListener>& listener,
        const sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessAvoidAreaChangeRegister(sptr<JsExtensionWindowListener> listener,
        sptr<Window> window, bool isRegister);
    WmErrorCode ProcessLifeCycleEventRegister(sptr<JsExtensionWindowListener> listener,
        sptr<Window> window, bool isRegister);
    WmErrorCode ProcessDisplayIdChangeRegister(const sptr<JsExtensionWindowListener>& listener,
        const sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessSystemDensityChangeRegister(const sptr<JsExtensionWindowListener>& listener,
        const sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessScreenshotRegister(const sptr<JsExtensionWindowListener>& listener,
        const sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessExtensionSecureLimitChangeRegister(const sptr<JsExtensionWindowListener>& listener,
        const sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessKeyboardDidShowRegister(const sptr<JsExtensionWindowListener>& listener,
        const sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessKeyboardDidHideRegister(const sptr<JsExtensionWindowListener>& listener,
        const sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessOccupiedAreaChangeRegister(const sptr<JsExtensionWindowListener>& listener,
        const sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessWindowStatusChangeRegister(const sptr<JsExtensionWindowListener>& listener,
        const sptr<Window>& window, bool isRegister);
    WmErrorCode ProcessRegister(CaseType caseType, const sptr<JsExtensionWindowListener>& listener,
        const sptr<Window>& window, const std::string& type, bool isRegister);
    std::map<std::string, std::map<std::shared_ptr<NativeReference>, sptr<JsExtensionWindowListener>>> jsCbMap_;
    std::mutex mtx_;
    std::map<CaseType, std::map<std::string, ListenerType>> listenerCodeMap_;
};
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_JS_EXTENSION_WINDOW_REGISTER_MANAGER_H
