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
}

JsExtensionWindowListener::~JsExtensionWindowListener()
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]~JsExtensionWindowListener");
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
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]CallJsMethod methodName = %{public}s", methodName);
    if (env_ == nullptr || jsCallBack_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]env_ nullptr or jsCallBack_ is nullptr");
        return;
    }
    napi_value method = jsCallBack_->GetNapiValue();
    if (method == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Failed to get method callback from object");
        return;
    }
    napi_value result = nullptr;
    napi_get_undefined(env_, &result);
    napi_call_function(env_, result, method, argc, argv, nullptr);
}

void JsExtensionWindowListener::OnSizeChange(Rect rect, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]OnSizeChange, [%{public}u, %{public}u], reason=%{public}u",
        rect.width_, rect.height_, reason);
    if (currRect_.width_ == rect.width_ && currRect_.height_ == rect.height_ &&
        reason != WindowSizeChangeReason::DRAG_END) {
        TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]no need to change size");
        return;
    }
    // js callback should run in js thread
    auto jsCallback = [self = weakRef_, rect, eng = env_] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsExtensionWindowListener::OnSizeChange");
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr) {
            TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]this listener or eng is nullptr");
            return;
        }
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(eng, &scope);
        if (scope == nullptr) {
            TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]open handle scope failed");
            return;
        }
        napi_value objValue = nullptr;
        napi_create_object(eng, &objValue);
        if (objValue == nullptr) {
            TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Failed to convert rect to jsObject");
            napi_close_handle_scope(eng, scope);
            return;
        }
        napi_set_named_property(eng, objValue, "width", CreateJsValue(eng, rect.width_));
        napi_set_named_property(eng, objValue, "height", CreateJsValue(eng, rect.height_));
        napi_value argv[] = {objValue};
        thisListener->CallJsMethod(WINDOW_SIZE_CHANGE_CB.c_str(), argv, ArraySize(argv));
        napi_close_handle_scope(eng, scope);
    };
    if (reason == WindowSizeChangeReason::ROTATION) {
        jsCallback();
    } else if (napi_status::napi_ok != napi_send_event(env_, jsCallback, napi_eprio_high)) {
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
            TLOGNE(WmsLogTag::WMS_UIEXT, "[NAPI] This listener or env is nullptr");
            return;
        }
        HandleScope handleScope(env);
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "[NAPI] Failed to create js object");
            return;
        }
        napi_value rectObjValue = GetRectAndConvertToJsValue(env, rect);
        if (rectObjValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "[NAPI] Failed to create rect js object");
            return;
        }
        napi_set_named_property(env, objValue, "rect", rectObjValue);
        napi_set_named_property(env, objValue, "reason", CreateJsValue(env,
            ComponentRectChangeReason::HOST_WINDOW_RECT_CHANGE));
        napi_value argv[] = { objValue };
        thisListener->CallJsMethod(WINDOW_RECT_CHANGE_CB.c_str(), argv, ArraySize(argv));
    };
    if (napi_status::napi_ok != napi_send_event(env_, jsCallback, napi_eprio_high)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "send event failed");
    }
    currRect_ = rect;
}

void JsExtensionWindowListener::OnModeChange(WindowMode mode, bool hasDeco)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]OnModeChange %{public}u", mode);
}

void JsExtensionWindowListener::OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]OnAvoidAreaChanged");
    // js callback should run in js thread
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback> (
        [self = weakRef_, avoidArea, type, eng = env_] (napi_env env, NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr || eng == nullptr) {
                TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]this listener or eng is nullptr");
                return;
            }
            napi_value avoidAreaValue = ConvertAvoidAreaToJsValue(env, avoidArea, type);
            if (avoidAreaValue == nullptr) {
                return;
            }
            if (thisListener->isDeprecatedInterface_) {
                napi_value argv[] = { avoidAreaValue };
                thisListener->CallJsMethod(SYSTEM_AVOID_AREA_CHANGE_CB.c_str(), argv, ArraySize(argv));
            } else {
                napi_value objValue = nullptr;
                napi_create_object(env, &objValue);
                if (objValue == nullptr) {
                    TLOGE(WmsLogTag::WMS_UIEXT, "Failed to get object");
                    return;
                }
                napi_set_named_property(env, objValue, "type",
                                        CreateJsValue(env, static_cast<uint32_t>(type)));
                napi_set_named_property(env, objValue, "area", avoidAreaValue);
                napi_value argv[] = { objValue };
                thisListener->CallJsMethod(AVOID_AREA_CHANGE_CB.c_str(), argv, ArraySize(argv));
            }
        }
    );

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsExtensionWindowListener::OnAvoidAreaChanged", env_,
                            std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsExtensionWindowListener::OnSizeChange(const sptr<OccupiedAreaChangeInfo>& info,
                                             const std::shared_ptr<RSTransaction>& rsTransaction)
{
    TLOGI(WmsLogTag::WMS_UIEXT,
        "[NAPI]OccupiedAreaChangeInfo, type: %{public}d, input rect:[%{public}d, %{public}d, %{public}d, %{public}d]",
        static_cast<uint32_t>(info->type_), info->rect_.posX_, info->rect_.posY_, info->rect_.width_,
        info->rect_.height_);
    // js callback should run in js thread
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback> (
        [self = weakRef_, info, eng = env_] (napi_env env, NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr || eng == nullptr) {
                TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]this listener or eng is nullptr");
                return;
            }
            napi_value argv[] = {CreateJsValue(eng, info->rect_.height_)};
            thisListener->CallJsMethod(KEYBOARD_HEIGHT_CHANGE_CB.c_str(), argv, ArraySize(argv));
        }
    );

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsExtensionWindowListener::OnSizeChange", env_,
                            std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

static void LifeCycleCallBack(LifeCycleEventType eventType, wptr<JsExtensionWindowListener> weakRef,
    napi_env env, std::shared_ptr<AppExecFwk::EventHandler> eventHandler)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "LifeCycleCallBack, envent type: %{public}u", eventType);
    auto task = [self = weakRef, eventType, eng = env] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsExtensionWindowListener::LifeCycleCallBack");
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr) {
            TLOGE(WmsLogTag::WMS_UIEXT, "this listener or eng is nullptr");
            return;
        }
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(eng, &scope);
        napi_value argv[] = {CreateJsValue(eng, static_cast<uint32_t>(eventType))};
        thisListener->CallJsMethod(LIFECYCLE_EVENT_CB.c_str(), argv, ArraySize(argv));
        napi_close_handle_scope(eng, scope);
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
