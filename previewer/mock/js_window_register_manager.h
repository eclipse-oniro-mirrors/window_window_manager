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

#ifndef OHOS_JS_WINDOW_REGISTER_MANAGER_H
#define OHOS_JS_WINDOW_REGISTER_MANAGER_H
#include <map>
#include <mutex>
#include "js_window_listener.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"
#include "refbase.h"
#include "window.h"
namespace OHOS {
namespace Rosen {
enum class CaseType {
    CASE_WINDOW_MANAGER = 0,
    CASE_WINDOW,
    CASE_STAGE
};

enum class ListenerFunctionType : uint32_t {
    SYSTEM_AVOID_AREA_CHANGE_CB,
    AVOID_AREA_CHANGE_CB,
};

class JsWindowRegisterManager {
public:
    JsWindowRegisterManager();
    ~JsWindowRegisterManager();
    WmErrorCode RegisterListener(sptr<Window> window, std::string type,
        CaseType caseType, napi_env env, napi_value callback, napi_value parameter = nullptr);
    WmErrorCode UnregisterListener(sptr<Window> window, std::string type,
        CaseType caseType, napi_env env, napi_value value);
    WmErrorCode RegisterWindowPostureModeListener(napi_env env, sptr<Window> window,
        napi_value callback, WindowPostureMode mode);
    WmErrorCode UnregisterWindowPostureModeListener(napi_env env, sptr<Window> window,
        napi_value callback, WindowPostureMode mode);

private:
    bool IsCallbackRegistered(napi_env env, std::string& type, napi_value jsListenerObject);
    WmErrorCode ProcessSystemAvoidAreaChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window,
        bool isRegister, napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessAvoidAreaChangeRegister(sptr<JsWindowListener> listener, sptr<Window> window, bool isRegister,
        napi_env env, napi_value parameter = nullptr);
    WmErrorCode ProcessRegisterCallback(ListenerFunctionType listenerFunctionType, CaseType caseType,
        const sptr<JsWindowListener>& listener, const sptr<Window>& window, bool isRegister, napi_env env,
        napi_value parameter);
    std::map<std::string, std::map<std::shared_ptr<NativeReference>, sptr<JsWindowListener>>> jsCbMap_;
    std::mutex mtx_;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_JS_WINDOW_REGISTER_MANAGER_H