/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_display_listener.h"

#include <hitrace_meter.h>

#include "dm_common.h"
#include "js_runtime_utils.h"
#include "window_manager_hilog.h"
#include "js_display.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

JsDisplayListener::JsDisplayListener(napi_env env) : env_(env), weakRef_(wptr<JsDisplayListener> (this))
{
    TLOGI(WmsLogTag::DMS, "Constructor");
    napi_add_env_cleanup_hook(env_, CleanEnv, this);
}

JsDisplayListener::~JsDisplayListener()
{
    TLOGI(WmsLogTag::DMS, "Destructor");
    napi_remove_env_cleanup_hook(env_, CleanEnv, this);
    env_ = nullptr;
}

void JsDisplayListener::CleanEnv(void* obj)
{
    JsDisplayListener* thisObj = reinterpret_cast<JsDisplayListener*>(obj);
    if (!thisObj) {
        TLOGE(WmsLogTag::DMS, "obj is nullptr");
        return;
    }
    TLOGI(WmsLogTag::DMS, "env_ is invalid, set to nullptr");
    thisObj->env_ = nullptr;
}

void JsDisplayListener::AddCallback(const std::string& type, napi_value jsListenerObject)
{
    TLOGD(WmsLogTag::DMS, "called");
    std::unique_ptr<NativeReference> callbackRef;
    if (env_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "env_ nullptr");
        return;
    }
    napi_ref result = nullptr;
    napi_create_reference(env_, jsListenerObject, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    std::lock_guard<std::mutex> lock(mtx_);
    jsCallBack_[type].emplace_back(std::move(callbackRef));
    TLOGD(WmsLogTag::DMS, "success, jsCallBack_ size: %{public}u!",
        static_cast<uint32_t>(jsCallBack_[type].size()));
}

void JsDisplayListener::RemoveAllCallback()
{
    std::lock_guard<std::mutex> lock(mtx_);
    jsCallBack_.clear();
}

void JsDisplayListener::RemoveCallback(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = jsCallBack_.find(type);
    if (it == jsCallBack_.end()) {
        TLOGE(WmsLogTag::DMS, "no callback to remove");
        return;
    }
    auto& listeners = it->second;
    for (auto iter = listeners.begin(); iter != listeners.end();) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, (*iter)->GetNapiValue(), &isEquals);
        if (isEquals) {
            iter = listeners.erase(iter);
        } else {
            iter++;
        }
    }
    TLOGI(WmsLogTag::DMS, "jsCallBack size: %{public}u!", static_cast<uint32_t>(listeners.size()));
}

void JsDisplayListener::CallJsMethod(const std::string& methodName, napi_value const * argv, size_t argc)
{
    if (methodName.empty()) {
        TLOGE(WmsLogTag::DMS, "empty method name str, call method failed");
        return;
    }
    TLOGD(WmsLogTag::DMS, "CallJsMethod methodName = %{public}s", methodName.c_str());
    if (env_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "env_ nullptr");
        return;
    }
    for (auto& callback : jsCallBack_[methodName]) {
        napi_value method = callback->GetNapiValue();
        if (method == nullptr) {
            TLOGE(WmsLogTag::DMS, "Failed to get method callback from object");
            continue;
        }
        napi_call_function(env_, NapiGetUndefined(env_), method, argc, argv, nullptr);
    }
}

void JsDisplayListener::OnCreate(DisplayId id)
{
    std::lock_guard<std::mutex> lock(mtx_);
    TLOGI(WmsLogTag::DMS, "called, displayId: %{public}d", static_cast<uint32_t>(id));
    if (jsCallBack_.empty()) {
        TLOGE(WmsLogTag::DMS, "not register!");
        return;
    }
    if (jsCallBack_.find(EVENT_ADD) == jsCallBack_.end()) {
        TLOGE(WmsLogTag::DMS, "not this event, return");
        return;
    }
    auto napiTask = [self = weakRef_, id, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayListener::OnCreate");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::DMS, "[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, static_cast<uint32_t>(id))};
        thisListener->CallJsMethod(EVENT_ADD, argv, ArraySize(argv));
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnCreate");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsDisplayListener::OnDestroy(DisplayId id)
{
    std::lock_guard<std::mutex> lock(mtx_);
    TLOGI(WmsLogTag::DMS, "displayId: %{public}d", static_cast<uint32_t>(id));
    if (jsCallBack_.empty()) {
        TLOGE(WmsLogTag::DMS, "not register!");
        return;
    }
    if (jsCallBack_.find(EVENT_REMOVE) == jsCallBack_.end()) {
        TLOGE(WmsLogTag::DMS, "not this event, return");
        return;
    }
    auto napiTask = [self = weakRef_, id, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayListener::OnDestroy");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::DMS, "[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, static_cast<uint32_t>(id))};
        thisListener->CallJsMethod(EVENT_REMOVE, argv, ArraySize(argv));
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnDestroy");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsDisplayListener::OnChange(DisplayId id)
{
    std::lock_guard<std::mutex> lock(mtx_);
    TLOGD(WmsLogTag::DMS, "called, displayId: %{public}d", static_cast<uint32_t>(id));
    if (jsCallBack_.empty()) {
        TLOGE(WmsLogTag::DMS, "not register!");
        return;
    }
    if (jsCallBack_.find(EVENT_CHANGE) == jsCallBack_.end()) {
        TLOGD(WmsLogTag::DMS, "not this event, return");
        return;
    }
    auto napiTask = [self = weakRef_, id, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayListener::OnChange");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::DMS, "[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, static_cast<uint32_t>(id))};
        thisListener->CallJsMethod(EVENT_CHANGE, argv, ArraySize(argv));
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE_LIMITN_HOUR(WmsLogTag::DMS, THREE_TIMES, "env is nullptr");
    }
}

void JsDisplayListener::OnAttributeChange(DisplayId displayId, const std::vector<std::string>& attributes)
{
    std::lock_guard<std::mutex> lock(mtx_);
    TLOGD(WmsLogTag::DMS, "called, displayId: %{public}d", static_cast<uint32_t>(displayId));
    if (jsCallBack_.empty()) {
        TLOGE(WmsLogTag::DMS, "not register!");
        return;
    }
    if (attributes.empty()) {
        TLOGE(WmsLogTag::DMS, "attributes is empty!");
        return;
    }
 
    auto napiTask = [self = weakRef_, displayId, attributes, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayListener::OnAttributeChange");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGE(WmsLogTag::DMS, "[NAPI]this listener or env is nullptr");
            return;
        }
        thisListener->ProcessAttributeCallbacks(env, attributes, displayId, thisListener);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnAttributeChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsDisplayListener::ProcessAttributeCallbacks(napi_env env, const std::vector<std::string>& attributes,
    DisplayId displayId, sptr<JsDisplayListener> thisListener)
{
    std::vector<napi_value> methods;
    for (auto attribute : attributes) {
        if (thisListener->jsCallBack_.find(attribute) == thisListener->jsCallBack_.end()) {
            TLOGD(WmsLogTag::DMS, "current listener dose not listen: %{public}s, continue", attribute.c_str());
            continue;
        }
        for (auto& callback : thisListener->jsCallBack_[attribute]) {
            napi_value method = callback->GetNapiValue();
            if (method == nullptr) {
                TLOGE(WmsLogTag::DMS, "Failed to get method callback from object");
                continue;
            }
            methods.push_back(method);
        }
    }
    thisListener->RemoveDuplicateMethods(methods);
    napi_value argv[] = {CreateJsValue(env, static_cast<uint32_t>(displayId))};
    for (auto method: methods) {
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    }
}
 
void JsDisplayListener::RemoveDuplicateMethods(std::vector<napi_value>& methods)
{
    std::vector<napi_value> uniqueMethods;
    
    for (auto& currentMethod : methods) {
        if (!currentMethod) continue;
        
        bool isDuplicate = false;
        for (const auto& existingMethod : uniqueMethods) {
            napi_strict_equals(env_, currentMethod, existingMethod, &isDuplicate);
            if (isDuplicate) {
                break;
            }
        }
        if (!isDuplicate) {
            uniqueMethods.push_back(std::move(currentMethod));
        }
    }
    methods = std::move(uniqueMethods);
}

void JsDisplayListener::OnPrivateWindow(bool hasPrivate)
{
    std::lock_guard<std::mutex> lock(mtx_);
    TLOGI(WmsLogTag::DMS, "called, private status: %{public}u", static_cast<uint32_t>(hasPrivate));
    if (jsCallBack_.empty()) {
        TLOGE(WmsLogTag::DMS, "not register!");
        return;
    }
    if (jsCallBack_.find(EVENT_PRIVATE_MODE_CHANGE) == jsCallBack_.end()) {
        TLOGE(WmsLogTag::DMS, "not this event, return");
        return;
    }
    auto napiTask = [self = weakRef_, hasPrivate, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayListener::OnPrivateWindow");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::DMS, "[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, hasPrivate)};
        thisListener->CallJsMethod(EVENT_PRIVATE_MODE_CHANGE, argv, ArraySize(argv));
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnPrivateWindow");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsDisplayListener::OnFoldStatusChanged(FoldStatus foldStatus)
{
    std::lock_guard<std::mutex> lock(mtx_);
    TLOGI(WmsLogTag::DMS, "called, foldStatus: %{public}u", static_cast<uint32_t>(foldStatus));
    if (jsCallBack_.empty()) {
        TLOGE(WmsLogTag::DMS, "not register!");
        return;
    }
    if (jsCallBack_.find(EVENT_FOLD_STATUS_CHANGED) == jsCallBack_.end()) {
        TLOGE(WmsLogTag::DMS, "not this event, return");
        return;
    }
    auto napiTask = [self = weakRef_, foldStatus, env = env_] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayListener::OnFoldStatusChanged");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::DMS, "[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, foldStatus)};
        thisListener->CallJsMethod(EVENT_FOLD_STATUS_CHANGED, argv, ArraySize(argv));
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnFoldStatusChanged");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsDisplayListener::OnFoldAngleChanged(std::vector<float> foldAngles)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCallBack_.empty()) {
        TLOGE(WmsLogTag::DMS, "not register!");
        return;
    }
    if (jsCallBack_.find(EVENT_FOLD_ANGLE_CHANGED) == jsCallBack_.end()) {
        TLOGE(WmsLogTag::DMS, "not this event, return");
        return;
    }
    auto napiTask = [self = weakRef_, foldAngles, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayListener::OnFoldAngleChanged");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::DMS, "[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateNativeArray(env, foldAngles)};
        thisListener->CallJsMethod(EVENT_FOLD_ANGLE_CHANGED, argv, ArraySize(argv));
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnFoldAngleChanged");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsDisplayListener::OnCaptureStatusChanged(bool isCapture)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCallBack_.empty()) {
        TLOGE(WmsLogTag::DMS, "not register!");
        return;
    }
    if (jsCallBack_.find(EVENT_CAPTURE_STATUS_CHANGED) == jsCallBack_.end()) {
        TLOGE(WmsLogTag::DMS, "not this event, return");
        return;
    }
    auto napiTask = [self = weakRef_, isCapture, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayListener::OnCaptureStatusChanged");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::DMS, "[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, isCapture)};
        thisListener->CallJsMethod(EVENT_CAPTURE_STATUS_CHANGED, argv, ArraySize(argv));
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnCaptureStatusChanged");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsDisplayListener::OnDisplayModeChanged(FoldDisplayMode displayMode)
{
    std::lock_guard<std::mutex> lock(mtx_);
    TLOGI(WmsLogTag::DMS, "called, displayMode: %{public}u", static_cast<uint32_t>(displayMode));
    if (jsCallBack_.empty()) {
        TLOGE(WmsLogTag::DMS, "not register!");
        return;
    }
    if (jsCallBack_.find(EVENT_DISPLAY_MODE_CHANGED) == jsCallBack_.end()) {
        TLOGE(WmsLogTag::DMS, "not this event, return");
        return;
    }
    auto napiTask = [self = weakRef_, displayMode, env = env_] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayListener::OnDisplayModeChanged");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::DMS, "[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, displayMode)};
        thisListener->CallJsMethod(EVENT_DISPLAY_MODE_CHANGED, argv, ArraySize(argv));
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnDisplayModeChanged");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsDisplayListener::OnAvailableAreaChanged(DMRect area)
{
    std::lock_guard<std::mutex> lock(mtx_);
    TLOGI(WmsLogTag::DMS, "called");
    if (jsCallBack_.empty()) {
        TLOGE(WmsLogTag::DMS, "not register!");
        return;
    }
    if (jsCallBack_.find(EVENT_AVAILABLE_AREA_CHANGED) == jsCallBack_.end()) {
        TLOGE(WmsLogTag::DMS, "not this event, return");
        return;
    }
    auto napiTask = [self = weakRef_, area, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayListener::OnAvailableAreaChanged");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::DMS, "[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsRectObject(env, area)};
        thisListener->CallJsMethod(EVENT_AVAILABLE_AREA_CHANGED, argv, ArraySize(argv));
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnAvailableAreaChanged");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsDisplayListener::OnBrightnessInfoChanged(DisplayId id, const ScreenBrightnessInfo& info)
{
    std::lock_guard<std::mutex> lock(mtx_);
    TLOGI(WmsLogTag::DMS, "called");
    if (jsCallBack_.empty()) {
        TLOGE(WmsLogTag::DMS, "not register!");
        return;
    }
    if (jsCallBack_.find(EVENT_BRIGHTNESS_INFO_CHANGED) == jsCallBack_.end()) {
        TLOGE(WmsLogTag::DMS, "not this event, return");
        return;
    }
    auto napiTask = [self = weakRef_, id, info, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayListener::OnBrightnessInfoChanged");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGE(WmsLogTag::DMS, "[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, static_cast<uint32_t>(id)), CreateJsBrightnessInfo(env, info)};
        thisListener->CallJsMethod(EVENT_BRIGHTNESS_INFO_CHANGED, argv, ArraySize(argv));
    };
    
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnBrightnessInfoChanged");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to sendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}
} // namespace Rosen
} // namespace OHOS
