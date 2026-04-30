/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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
#ifndef OHOS_JS_FLOAT_VIEW_CONTROLLER_H
#define OHOS_JS_FLOAT_VIEW_CONTROLLER_H

#include "float_view_controller.h"

#include "js_float_view_listener.h"
#include "js_runtime_utils.h"
#include "window_histogram_management.h"

namespace OHOS {
namespace Rosen {
napi_value CreateJsFloatViewControllerObject(napi_env env, const sptr<FloatViewController>& floatViewController);
class JsFloatViewController {
public:
    enum class CallbackType : uint32_t {
        STATE_CHANGE,
        RECT_CHANGE,
        LIMITS_CHANGE,
    };
    explicit JsFloatViewController(const sptr<FloatViewController>& floatViewController);
    virtual ~JsFloatViewController();
    sptr<FloatViewController> GetController() const;
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static napi_value SetUIContext(napi_env env, napi_callback_info info);
    static napi_value SetFloatViewVisibilityInApp(napi_env env, napi_callback_info info);
    static napi_value SetWindowSize(napi_env env, napi_callback_info info);
    static napi_value GetWindowProperties(napi_env env, napi_callback_info info);
    static napi_value RestoreMainWindow(napi_env env, napi_callback_info info);

    static napi_value OnStateChange(napi_env env, napi_callback_info info);
    static napi_value OffStateChange(napi_env env, napi_callback_info info);
    static napi_value OnRectChange(napi_env env, napi_callback_info info);
    static napi_value OffRectChange(napi_env env, napi_callback_info info);
    static napi_value OnLimitsChange(napi_env env, napi_callback_info info);
    static napi_value OffLimitsChange(napi_env env, napi_callback_info info);
private:
    bool IsCallbackRegistered(napi_env env, const CallbackType& type, napi_value jsCallback);
    napi_value OnStartFloatView(napi_env env, napi_callback_info info);
    napi_value OnStopFloatView(napi_env env, napi_callback_info info);
    napi_value OnSetUIContext(napi_env env, napi_callback_info info);
    napi_value SetUIContextTask(napi_env env, const std::string contextUrl,
        const std::shared_ptr<NativeReference>& contentStorage);
    napi_value OnSetFloatViewVisibilityInApp(napi_env env, napi_callback_info info);
    napi_value OnSetWindowSize(napi_env env, napi_callback_info info);
    napi_value OnSetWindowSizeTask(napi_env env, int32_t width, int32_t height);
    napi_value OnGetWindowProperties(napi_env env, napi_callback_info info);
    napi_value OnRestoreMainWindow(napi_env env, napi_callback_info info);

    napi_value RegisterCallbackWithType(const CallbackType& callbackType, napi_env env, napi_callback_info info);
    napi_value UnregisterCallbackWithType(const CallbackType& callbackType, napi_env env, napi_callback_info info);
    WMError DoRegisterCallbackWithType(const CallbackType& callbackType, const sptr<JsFloatViewListener>& listener);
    WMError DoUnregisterCallbackWithType(const CallbackType& callbackType, const sptr<JsFloatViewListener>& listener);

    sptr<FloatViewController> fvController_ = nullptr;
    std::unordered_map<CallbackType, std::set<sptr<JsFloatViewListener>>> jsCallbackMap_;
    std::mutex jsCallbackMutex_;
};
} // namespace Rosen
} // namespace OHOS
#endif