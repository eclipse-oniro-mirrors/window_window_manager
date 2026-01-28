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

#include "js_extension_window_listener.h"

#include <hitrace_meter.h>

#include "event_handler.h"
#include "event_runner.h"
#include "js_runtime_utils.h"
#include "window_manager_hilog.h"
#include "js_window_utils.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
const std::string WINDOW_SIZE_CHANGE_CB = "windowSizeChange";
const std::string WINDOW_RECT_CHANGE_CB = "rectChange";
const std::string SYSTEM_AVOID_AREA_CHANGE_CB = "systemAvoidAreaChange";
const std::string AVOID_AREA_CHANGE_CB = "avoidAreaChange";
const std::string LIFECYCLE_EVENT_CB = "lifeCycleEvent";
const std::string KEYBOARD_HEIGHT_CHANGE_CB = "keyboardHeightChange";
const std::string WINDOW_DISPLAYID_CHANGE_CB = "displayIdChange";
const std::string SYSTEM_DENSITY_CHANGE_CB = "systemDensityChange";
const std::string SCREENSHOT_EVENT_CB = "screenshot";
const std::string EXTENSION_SECURE_LIMIT_CHANGE_CB = "uiExtensionSecureLimitChange";
const std::string KEYBOARD_DID_SHOW_CB = "keyboardDidShow";
const std::string KEYBOARD_DID_HIDE_CB = "keyboardDidHide";
}

JsExtensionWindowListener::~JsExtensionWindowListener()
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
}

void JsExtensionWindowListener::SetMainEventHandler()
{
    auto mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainRunner == nullptr) {
        return;
    }
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainRunner);
}

void JsExtensionWindowListener::CallJsMethod(const char* methodName, napi_value const * argv, size_t argc)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "methodName=%{public}s", methodName);
    if (env_ == nullptr || jsCallBack_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "env_ nullptr or jsCallBack_ is nullptr");
        return;
    }
    napi_value method = jsCallBack_->GetNapiValue();
    if (method == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get method callback from object");
        return;
    }
    napi_value result = nullptr;
    napi_get_undefined(env_, &result);
    napi_call_function(env_, result, method, argc, argv, nullptr);
}

void JsExtensionWindowListener::OnSizeChange(Rect rect, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "rect [%{public}u, %{public}u], reason=%{public}u",
        rect.width_, rect.height_, reason);
    if (currRect_.width_ == rect.width_ && currRect_.height_ == rect.height_ &&
        reason != WindowSizeChangeReason::DRAG_END) {
        TLOGD(WmsLogTag::WMS_UIEXT, "no need to change size");
        return;
    }
    // js callback should run in js thread
    auto jsCallback = [self = weakRef_, rect, eng = env_] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsExtensionWindowListener::OnSizeChange");
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "this listener or eng is nullptr");
            return;
        }
        napi_handle_scope scope = nullptr;
        napi_status status = napi_open_handle_scope(eng, &scope);
        if (status != napi_ok || scope == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "open handle scope failed");
            return;
        }
        napi_value objValue = nullptr;
        napi_create_object(eng, &objValue);
        if (objValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "Failed to convert rect to jsObject");
            status = napi_close_handle_scope(eng, scope);
            if (status != napi_ok) {
                TLOGNW(WmsLogTag::WMS_UIEXT, "close handle scope failed");
            }
            return;
        }
        napi_set_named_property(eng, objValue, "width", CreateJsValue(eng, rect.width_));
        napi_set_named_property(eng, objValue, "height", CreateJsValue(eng, rect.height_));
        napi_value argv[] = {objValue};
        thisListener->CallJsMethod(WINDOW_SIZE_CHANGE_CB.c_str(), argv, ArraySize(argv));
        status = napi_close_handle_scope(eng, scope);
        if (status != napi_ok) {
            TLOGNW(WmsLogTag::WMS_UIEXT, "close handle scope failed");
        }
    };
    if (reason == WindowSizeChangeReason::ROTATION) {
        jsCallback();
    } else if (napi_send_event(env_, jsCallback, napi_eprio_high, "OnSizeChange") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_UIEXT, "send event failed");
    }
    currRect_ = rect;
}

void JsExtensionWindowListener::OnRectChange(Rect rect, WindowSizeChangeReason reason)
{
    if (currRect_ == rect) {
        TLOGD(WmsLogTag::WMS_UIEXT, "Skip redundant rect update");
        return;
    }
    // js callback should run in js thread
    const char* const where = __func__;
    auto jsCallback = [self = weakRef_, rect, env = env_, where] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsExtensionWindowListener::OnRectChange");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s This listener or env is nullptr", where);
            return;
        }
        HandleScope handleScope(env);
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s Failed to create js object", where);
            return;
        }
        napi_value rectObjValue = GetRectAndConvertToJsValue(env, rect);
        if (rectObjValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s Failed to create rect js object", where);
            return;
        }
        napi_set_named_property(env, objValue, "rect", rectObjValue);
        napi_set_named_property(env, objValue, "reason", CreateJsValue(env,
            ComponentRectChangeReason::HOST_WINDOW_RECT_CHANGE));
        napi_value argv[] = { objValue };
        thisListener->CallJsMethod(WINDOW_RECT_CHANGE_CB.c_str(), argv, ArraySize(argv));
    };
    if (napi_send_event(env_, jsCallback, napi_eprio_high, "OnRectChange") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_UIEXT, "send event failed");
    }
    currRect_ = rect;
}

void JsExtensionWindowListener::OnModeChange(WindowMode mode, bool hasDeco)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "%{public}u", mode);
}

void JsExtensionWindowListener::OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type,
    const sptr<OccupiedAreaChangeInfo>& info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    // js callback should run in js thread
    auto jsCallback = [self = weakRef_, avoidArea, type, eng = env_] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "this listener or eng is nullptr");
            return;
        }
        napi_value avoidAreaValue = ConvertAvoidAreaToJsValue(eng, avoidArea, type);
        if (avoidAreaValue == nullptr) {
            return;
        }
        if (thisListener->isDeprecatedInterface_) {
            napi_value argv[] = { avoidAreaValue };
            thisListener->CallJsMethod(SYSTEM_AVOID_AREA_CHANGE_CB.c_str(), argv, ArraySize(argv));
        } else {
            napi_value objValue = nullptr;
            napi_create_object(eng, &objValue);
            if (objValue == nullptr) {
                TLOGNE(WmsLogTag::WMS_UIEXT, "Failed to get object");
                return;
            }
            napi_set_named_property(eng, objValue, "type",
                                    CreateJsValue(eng, static_cast<uint32_t>(type)));
            napi_set_named_property(eng, objValue, "area", avoidAreaValue);
            napi_value argv[] = { objValue };
            thisListener->CallJsMethod(AVOID_AREA_CHANGE_CB.c_str(), argv, ArraySize(argv));
        }
    };
    if (napi_send_event(env_, jsCallback, napi_eprio_high, "OnAvoidAreaChanged") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_UIEXT, "send event failed");
    }
}

void JsExtensionWindowListener::OnSizeChange(const sptr<OccupiedAreaChangeInfo>& info,
                                             const std::shared_ptr<RSTransaction>& rsTransaction)
{
    TLOGI(WmsLogTag::WMS_UIEXT,
        "OccupiedAreaChangeInfo, type: %{public}d, input rect:[%{public}d, %{public}d, %{public}d, %{public}d]",
        static_cast<uint32_t>(info->type_), info->rect_.posX_, info->rect_.posY_, info->rect_.width_,
        info->rect_.height_);
    // js callback should run in js thread
    auto jsCallback = [self = weakRef_, info, eng = env_] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "this listener or eng is nullptr");
            return;
        }
        napi_value argv[] = { CreateJsValue(eng, info->rect_.height_) };
        thisListener->CallJsMethod(KEYBOARD_HEIGHT_CHANGE_CB.c_str(), argv, ArraySize(argv));
    };
    if (napi_send_event(env_, jsCallback, napi_eprio_high, "OnSizeChange") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_UIEXT, "send event failed");
    }
}

void JsExtensionWindowListener::OnDisplayIdChanged(DisplayId displayId)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "in");
    const char* const where = __func__;
    auto jsCallback = [self = weakRef_, displayId, env = env_, where] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s this listener or env is nullptr", where);
            return;
        }
        HandleScope handleScope(env);
        napi_value argv[] = { CreateJsValue(env, static_cast<int64_t>(displayId)) };
        thisListener->CallJsMethod(WINDOW_DISPLAYID_CHANGE_CB.c_str(), argv, ArraySize(argv));
    };
    if (napi_send_event(env_, jsCallback, napi_eprio_high, "OnDisplayIdChanged") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to send event");
    }
}

void JsExtensionWindowListener::OnSystemDensityChanged(float density)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    auto jsCallback = [self = weakRef_, density, env = env_] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "This listener or env is nullptr");
            return;
        }
        HandleScope handleScope(env);
        napi_value argv[] = { CreateJsValue(env, density) };
        thisListener->CallJsMethod(SYSTEM_DENSITY_CHANGE_CB.c_str(), argv, ArraySize(argv));
    };
    if (napi_send_event(env_, jsCallback, napi_eprio_high, "OnSystemDensityChanged") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to send event");
    }
}

void JsExtensionWindowListener::OnScreenshot()
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    auto jsCallback = [self = weakRef_] {
        auto thisListener = self.promote();
        if (thisListener == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "this listener is nullptr");
            return;
        }
        thisListener->CallJsMethod(SCREENSHOT_EVENT_CB.c_str(), nullptr, 0);
    };
    if (napi_send_event(env_, jsCallback, napi_eprio_high, "OnScreenshot") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to send event");
    }
}

void JsExtensionWindowListener::OnSecureLimitChange(bool isLimit)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "isLimit: %{public}d", isLimit);
    auto jsCallback = [self = weakRef_, isLimit, env = env_, where = __func__]() {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s: this listener or env is nullptr", where);
            return;
        }
        HandleScope handleScope(env);
        napi_value argv[] = { CreateJsValue(env, isLimit) };
        thisListener->CallJsMethod(EXTENSION_SECURE_LIMIT_CHANGE_CB.c_str(), argv, ArraySize(argv));
    };
    if (napi_send_event(env_, jsCallback, napi_eprio_high, "OnSecureLimitChange") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to send event");
    }
}

void JsExtensionWindowListener::OnKeyboardDidShow(const KeyboardPanelInfo& keyboardPanelInfo)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Called");
    auto jsCallback = [self = weakRef_, env = env_, keyboardPanelInfo, funcName = __func__] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s: this listener or env is nullptr", funcName);
            return;
        }
        HandleScope handleScope(env);
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s failed to create js object", funcName);
            return;
        }
        napi_value beginRectObjValue = GetRectAndConvertToJsValue(env, keyboardPanelInfo.beginRect_);
        if (beginRectObjValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s failed to convert begin rect to jsObject", funcName);
            return;
        }
        napi_value endRectObjValue = GetRectAndConvertToJsValue(env, keyboardPanelInfo.endRect_);
        if (endRectObjValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s failed to convert end rect to jsObject", funcName);
            return;
        }
        napi_set_named_property(env, objValue, "beginRect", beginRectObjValue);
        napi_set_named_property(env, objValue, "endRect", endRectObjValue);
        napi_value argv[] = { objValue };
        thisListener->CallJsMethod(KEYBOARD_DID_SHOW_CB.c_str(), argv, ArraySize(argv));
    };
    if (napi_send_event(env_, jsCallback, napi_eprio_high, "OnKeyboardDidShow") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to send event");
    }
}

void JsExtensionWindowListener::OnKeyboardDidHide(const KeyboardPanelInfo& keyboardPanelInfo)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Called");
    auto jsCallback = [self = weakRef_, env = env_, keyboardPanelInfo, funcName = __func__] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "%{public}s: this listener or env is nullptr", funcName);
            return;
        }
        HandleScope handleScope(env);
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s failed to create js object", funcName);
            return;
        }
        napi_value beginRectObjValue = GetRectAndConvertToJsValue(env, keyboardPanelInfo.beginRect_);
        if (beginRectObjValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s failed to convert begin rect to jsObject", funcName);
            return;
        }
        napi_value endRectObjValue = GetRectAndConvertToJsValue(env, keyboardPanelInfo.endRect_);
        if (endRectObjValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s failed to convert end rect to jsObject", funcName);
            return;
        }
        napi_set_named_property(env, objValue, "beginRect", beginRectObjValue);
        napi_set_named_property(env, objValue, "endRect", endRectObjValue);
        napi_value argv[] = { objValue };
        thisListener->CallJsMethod(KEYBOARD_DID_HIDE_CB.c_str(), argv, ArraySize(argv));
    };
    if (napi_send_event(env_, jsCallback, napi_eprio_high, "OnKeyboardDidHide") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to send event");
    }
}

static void LifeCycleCallBack(LifeCycleEventType eventType, wptr<JsExtensionWindowListener> weakRef,
    napi_env env, std::shared_ptr<AppExecFwk::EventHandler> eventHandler)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "event type: %{public}u", eventType);
    auto task = [self = weakRef, eventType, eng = env] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsExtensionWindowListener::LifeCycleCallBack");
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "this listener or eng is nullptr");
            return;
        }
        napi_handle_scope scope = nullptr;
        napi_status status = napi_open_handle_scope(eng, &scope);
        if (status != napi_ok || scope == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "open handle scope failed");
            return;
        }
        napi_value argv[] = {CreateJsValue(eng, static_cast<uint32_t>(eventType))};
        thisListener->CallJsMethod(LIFECYCLE_EVENT_CB.c_str(), argv, ArraySize(argv));
        status = napi_close_handle_scope(eng, scope);
        if (status != napi_ok) {
            TLOGNW(WmsLogTag::WMS_UIEXT, "close handle scope failed");
        }
    };
    if (!eventHandler) {
        TLOGE(WmsLogTag::WMS_UIEXT, "get main event handler failed!");
        return;
    }
    eventHandler->PostTask(task, "wms:JsExtensionWindowListener::LifeCycleCallBack", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void JsExtensionWindowListener::AfterForeground()
{
    if (state_ == WindowState::STATE_INITIAL || state_ == WindowState::STATE_HIDDEN) {
        LifeCycleCallBack(LifeCycleEventType::FOREGROUND, weakRef_, env_, eventHandler_);
        state_ = WindowState::STATE_SHOWN;
    } else {
        TLOGD(WmsLogTag::WMS_UIEXT, "window is already shown");
    }
}

void JsExtensionWindowListener::AfterBackground()
{
    if (state_ == WindowState::STATE_INITIAL || state_ == WindowState::STATE_SHOWN) {
        LifeCycleCallBack(LifeCycleEventType::BACKGROUND, weakRef_, env_, eventHandler_);
        state_ = WindowState::STATE_HIDDEN;
    } else {
        TLOGD(WmsLogTag::WMS_UIEXT, "window is already hide");
    }
}

void JsExtensionWindowListener::AfterFocused()
{
    LifeCycleCallBack(LifeCycleEventType::ACTIVE, weakRef_, env_, eventHandler_);
}

void JsExtensionWindowListener::AfterUnfocused()
{
    LifeCycleCallBack(LifeCycleEventType::INACTIVE, weakRef_, env_, eventHandler_);
}

void JsExtensionWindowListener::AfterResumed()
{
    LifeCycleCallBack(LifeCycleEventType::RESUMED, weakRef_, env_, eventHandler_);
}

void JsExtensionWindowListener::AfterPaused()
{
    LifeCycleCallBack(LifeCycleEventType::PAUSED, weakRef_, env_, eventHandler_);
}

void JsExtensionWindowListener::AfterDestroyed()
{
    LifeCycleCallBack(LifeCycleEventType::DESTROYED, weakRef_, env_, eventHandler_);
}
} // namespace Rosen
} // namespace OHOS
