/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <hitrace_meter.h>

#include "event_handler.h"
#include "event_runner.h"
#include "js_window_listener.h"
#include "js_runtime_utils.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsListener"};
constexpr size_t INDEX_ZERO = 0;
constexpr size_t ARG_COUNT_ONE = 1;
}

JsWindowListener::~JsWindowListener()
{
    WLOGI("[NAPI]~JsWindowListener");
}

void JsWindowListener::SetMainEventHandler()
{
    auto mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainRunner == nullptr) {
        return;
    }
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainRunner);
}

napi_value JsWindowListener::CallJsMethod(const char* methodName, napi_value const * argv, size_t argc)
{
    WLOGFD("[NAPI]CallJsMethod methodName = %{public}s", methodName);
    if (env_ == nullptr || jsCallBack_ == nullptr) {
        WLOGFE("[NAPI]env_ nullptr or jsCallBack_ is nullptr");
        return nullptr;
    }
    napi_value method = jsCallBack_->GetNapiValue();
    if (method == nullptr) {
        WLOGFE("[NAPI]Failed to get method callback from object");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value callResult = nullptr;
    napi_get_undefined(env_, &result);
    napi_get_undefined(env_, &callResult);
    napi_call_function(env_, result, method, argc, argv, &callResult);

    return callResult;
}

void JsWindowListener::OnSizeChange(Rect rect, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    if (currRect_.width_ == rect.width_ && currRect_.height_ == rect.height_ &&
        reason != WindowSizeChangeReason::DRAG_END) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "no need to change size");
        return;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "wh[%{public}u, %{public}u], reason = %{public}u",
        rect.width_, rect.height_, reason);
    // js callback should run in js thread
    auto jsCallback = [self = weakRef_, rect, env = env_, funcName = __func__] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsWindowListener::OnSizeChange");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: this listener or env is nullptr", funcName);
            return;
        }
        HandleScope handleScope(env);
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: Failed to convert rect to jsObject", funcName);
            return;
        }
        napi_set_named_property(env, objValue, "width", CreateJsValue(env, rect.width_));
        napi_set_named_property(env, objValue, "height", CreateJsValue(env, rect.height_));
        napi_value argv[] = {objValue};
        thisListener->CallJsMethod(WINDOW_SIZE_CHANGE_CB.c_str(), argv, ArraySize(argv));
    };
    if (reason == WindowSizeChangeReason::ROTATION) {
        jsCallback();
    } else if (napi_status::napi_ok != napi_send_event(env_, jsCallback, napi_eprio_immediate)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "failed to send event");
        return;
    }
    currRect_ = rect;
}

void JsWindowListener::OnModeChange(WindowMode mode, bool hasDeco)
{
    WLOGI("[NAPI]OnModeChange %{public}u", mode);
}

void JsWindowListener::OnSystemBarPropertyChange(DisplayId displayId, const SystemBarRegionTints& tints)
{
    WLOGFD("[NAPI]OnSystemBarPropertyChange");
    // js callback should run in js thread
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback> (
        [self = weakRef_, displayId, tints, eng = env_] (napi_env env,
            NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr || eng == nullptr) {
                WLOGFE("[NAPI]this listener or eng is nullptr");
                return;
            }
            napi_value propertyValue = nullptr;
            napi_create_object(eng, &propertyValue);
            if (propertyValue == nullptr) {
                WLOGFE("[NAPI]Failed to convert prop to jsObject");
                return;
            }
            napi_set_named_property(env, propertyValue, "displayId",
                CreateJsValue(eng, static_cast<uint32_t>(displayId)));
            napi_set_named_property(env, propertyValue, "regionTint",
                CreateJsSystemBarRegionTintArrayObject(eng, tints));
            napi_value argv[] = {propertyValue};
            thisListener->CallJsMethod(SYSTEM_BAR_TINT_CHANGE_CB.c_str(), argv, ArraySize(argv));
        }
    );

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsWindowListener::OnSystemBarPropertyChange",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsWindowListener::OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type)
{
    WLOGFD("[NAPI]");
    // js callback should run in js thread
    auto jsCallback = [self = weakRef_, avoidArea, type, env = env_] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "[NAPI]this listener or env is nullptr");
            return;
        }
        HandleScope handleScope(env);
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
                TLOGNE(WmsLogTag::WMS_IMMS, "Failed to get object");
                return;
            }
            napi_set_named_property(env, objValue, "type", CreateJsValue(env, static_cast<uint32_t>(type)));
            napi_set_named_property(env, objValue, "area", avoidAreaValue);
            napi_value argv[] = { objValue };
            thisListener->CallJsMethod(AVOID_AREA_CHANGE_CB.c_str(), argv, ArraySize(argv));
        }
    };
    if (napi_status::napi_ok != napi_send_event(env_, jsCallback, napi_eprio_immediate)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to send event");
    }
}

void JsWindowListener::LifeCycleCallBack(LifeCycleEventType eventType)
{
    WLOGI("[NAPI]LifeCycleCallBack, envent type: %{public}u", eventType);
    auto task = [self = weakRef_, eventType, eng = env_] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsWindowListener::LifeCycleCallBack");
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr) {
            WLOGFE("[NAPI]this listener or eng is nullptr");
            return;
        }
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(eng, &scope);
        napi_value argv[] = {CreateJsValue(eng, static_cast<uint32_t>(eventType))};
        thisListener->CallJsMethod(LIFECYCLE_EVENT_CB.c_str(), argv, ArraySize(argv));
        napi_close_handle_scope(eng, scope);
    };
    if (!eventHandler_) {
        WLOGFE("get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:JsWindowListener::LifeCycleCallBack", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void JsWindowListener::AfterForeground()
{
    if (state_ == WindowState::STATE_INITIAL || state_ == WindowState::STATE_HIDDEN) {
        LifeCycleCallBack(LifeCycleEventType::FOREGROUND);
        state_ = WindowState::STATE_SHOWN;
    } else {
        WLOGFD("[NAPI]window is already shown");
    }
}

void JsWindowListener::AfterBackground()
{
    if (state_ == WindowState::STATE_INITIAL || state_ == WindowState::STATE_SHOWN) {
        LifeCycleCallBack(LifeCycleEventType::BACKGROUND);
        state_ = WindowState::STATE_HIDDEN;
    } else {
        WLOGFD("[NAPI]window is already hide");
    }
}

void JsWindowListener::AfterFocused()
{
    LifeCycleCallBack(LifeCycleEventType::ACTIVE);
}

void JsWindowListener::AfterUnfocused()
{
    LifeCycleCallBack(LifeCycleEventType::INACTIVE);
}

void JsWindowListener::AfterResumed()
{
    if (caseType_ == CaseType::CASE_STAGE) {
        LifeCycleCallBack(LifeCycleEventType::RESUMED);
    }
}

void JsWindowListener::AfterPaused()
{
    if (caseType_ == CaseType::CASE_STAGE) {
        LifeCycleCallBack(LifeCycleEventType::PAUSED);
    }
}

void JsWindowListener::AfterDestroyed()
{
    if (caseType_ == CaseType::CASE_WINDOW) {
        LifeCycleCallBack(LifeCycleEventType::DESTROYED);
    }
}

void JsWindowListener::OnSizeChange(const sptr<OccupiedAreaChangeInfo>& info,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    WLOGI("[NAPI]OccupiedAreaChangeInfo, type: %{public}u, " \
        "input rect: [%{public}d, %{public}d, %{public}u, %{public}u]", static_cast<uint32_t>(info->type_),
        info->rect_.posX_, info->rect_.posY_, info->rect_.width_, info->rect_.height_);
    // js callback should run in js thread
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback> (
        [self = weakRef_, info, eng = env_] (napi_env env,
            NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr || eng == nullptr) {
                WLOGFE("[NAPI]this listener or eng is nullptr");
                return;
            }
            napi_value argv[] = {CreateJsValue(eng, info->rect_.height_)};
            thisListener->CallJsMethod(KEYBOARD_HEIGHT_CHANGE_CB.c_str(), argv, ArraySize(argv));
        }
    );

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsWindowListener::OnSizeChange",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsWindowListener::OnTouchOutside() const
{
    WLOGI("CALLED");
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback> (
        [self = weakRef_] (napi_env env, NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr) {
                WLOGFE("[NAPI]this listener is nullptr");
                return;
            }
            thisListener->CallJsMethod(TOUCH_OUTSIDE_CB.c_str(), nullptr, 0);
        }
    );

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsWindowListener::OnOutsidePressed",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsWindowListener::OnScreenshot()
{
    WLOGI("CALLED");
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback> (
        [self = wptr<JsWindowListener>(this)] (napi_env env, NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr) {
                WLOGFE("[NAPI]this listener is nullptr");
                return;
            }
            thisListener->CallJsMethod(SCREENSHOT_EVENT_CB.c_str(), nullptr, 0);
        }
    );

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsWindowListener::OnScreenshot",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsWindowListener::OnDialogTargetTouch() const
{
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback> (
        [self = weakRef_] (napi_env env, NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr) {
                WLOGFE("[NAPI]this listener is nullptr");
                return;
            }
            thisListener->CallJsMethod(DIALOG_TARGET_TOUCH_CB.c_str(), nullptr, 0);
        }
    );

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsWindowListener::OnDialogTargetTouch",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsWindowListener::OnDialogDeathRecipient() const
{
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback> (
        [self = weakRef_] (napi_env env, NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr) {
                WLOGFE("[NAPI]this listener is nullptr");
                return;
            }
            thisListener->CallJsMethod(DIALOG_DEATH_RECIPIENT_CB.c_str(), nullptr, 0);
        }
    );

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsWindowListener::OnDialogDeathRecipient",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsWindowListener::OnGestureNavigationEnabledUpdate(bool enable)
{
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback> (
        [self = weakRef_, enable, eng = env_] (napi_env env, NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr || eng == nullptr) {
                WLOGFE("[NAPI]this listener or eng is nullptr");
                return;
            }
            napi_value argv[] = {CreateJsValue(eng, enable)};
            thisListener->CallJsMethod(GESTURE_NAVIGATION_ENABLED_CHANGE_CB.c_str(), argv, ArraySize(argv));
        }
    );

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsWindowListener::OnGestureNavigationEnabledUpdate",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsWindowListener::OnWaterMarkFlagUpdate(bool showWaterMark)
{
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback> (
        [self = weakRef_, showWaterMark, eng = env_] (napi_env env, NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr || eng == nullptr) {
                WLOGFE("[NAPI]this listener or eng is nullptr");
                return;
            }
            napi_value argv[] = {CreateJsValue(eng, showWaterMark)};
            thisListener->CallJsMethod(WATER_MARK_FLAG_CHANGE_CB.c_str(), argv, ArraySize(argv));
        }
    );

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsWindowListener::OnWaterMarkFlagUpdate",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsWindowListener::OnWindowVisibilityChangedCallback(const bool isVisible)
{
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [self = weakRef_, isVisible, eng = env_] (napi_env env, NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr || eng == nullptr) {
                WLOGFE("This listener or eng is nullptr");
                return;
            }
            napi_value argv[] = { CreateJsValue(eng, isVisible) };
            thisListener->CallJsMethod(WINDOW_VISIBILITY_CHANGE_CB.c_str(), argv, ArraySize(argv));
        }
    );
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsWindowListener::OnWindowVisibilityChangedCallback", env_,
        std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsWindowListener::SetTimeout(int64_t timeout)
{
    noInteractionTimeout_ = timeout;
}

int64_t JsWindowListener::GetTimeout() const
{
    return noInteractionTimeout_;
}

void JsWindowListener::OnWindowNoInteractionCallback()
{
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback> (
        [self = weakRef_] (napi_env env, NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr) {
                WLOGFE("[NAPI]this listener is nullptr");
                return;
            }
            thisListener->CallJsMethod(WINDOW_NO_INTERACTION_DETECT_CB.c_str(), nullptr, 0);
        }
    );

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsWindowListener::OnWindowNoInteractionCallback", env_,
        std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsWindowListener::OnWindowStatusChange(WindowStatus windowstatus)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    // js callback should run in js thread
    auto jsCallback = [self = weakRef_, windowstatus, env = env_, funcName = __func__] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: this listener or env is nullptr", funcName);
            return;
        }
        HandleScope handleScope(env);
        napi_value argv[] = {CreateJsValue(env, static_cast<uint32_t>(windowstatus))};
        thisListener->CallJsMethod(WINDOW_STATUS_CHANGE_CB.c_str(), argv, ArraySize(argv));
    };
    if (napi_status::napi_ok != napi_send_event(env_, jsCallback, napi_eprio_high)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "failed to send event");
    }
}

void JsWindowListener::OnDisplayIdChanged(DisplayId displayId)
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    auto jsCallback = [self = weakRef_, displayId, env = env_] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::DEFAULT, "[NAPI]this listener or env is nullptr");
            return;
        }
        HandleScope handleScope(env);
        napi_value argv[] = { CreateJsValue(env, static_cast<int64_t>(displayId)) };
        thisListener->CallJsMethod(WINDOW_DISPLAYID_CHANGE_CB.c_str(), argv, ArraySize(argv));
    };
    if (napi_status::napi_ok != napi_send_event(env_, jsCallback, napi_eprio_high)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to send event");
    }
}

void JsWindowListener::OnSystemDensityChanged(float density)
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
    if (napi_status::napi_ok != napi_send_event(env_, jsCallback, napi_eprio_high)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to send event");
    }
}

void JsWindowListener::OnWindowTitleButtonRectChanged(const TitleButtonRect& titleButtonRect)
{
    WLOGFD("[NAPI]OnWindowTitleButtonRectChanged");
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback> (
        [self = weakRef_, titleButtonRect, eng = env_] (napi_env env,
            NapiAsyncTask& task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr || eng == nullptr) {
                WLOGFE("[NAPI]this listener or eng is nullptr");
                return;
            }
            napi_value titleButtonRectValue = ConvertTitleButtonAreaToJsValue(env, titleButtonRect);
            if (titleButtonRectValue == nullptr) {
                return;
            }
            napi_value argv[] = { titleButtonRectValue };
            thisListener->CallJsMethod(WINDOW_TITLE_BUTTON_RECT_CHANGE_CB.c_str(), argv, ArraySize(argv));
        }
    );

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsWindowListener::OnWindowTitleButtonRectChanged",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsWindowListener::OnRectChange(Rect rect, WindowSizeChangeReason reason)
{
    if (currRect_ == rect && reason == WindowSizeChangeReason::UNDEFINED) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]skip redundant rect update");
        return;
    }
    RectChangeReason rectChangeReason = RectChangeReason::UNDEFINED;
    if (JS_SIZE_CHANGE_REASON.count(reason) != 0 &&
        !(reason == WindowSizeChangeReason::MAXIMIZE && rect.posX_ != 0)) {
        rectChangeReason = JS_SIZE_CHANGE_REASON.at(reason);
    }
    if (rectChangeReason == RectChangeReason::DRAG_END &&
        currentReason_ != RectChangeReason::DRAG_START && currentReason_ != RectChangeReason::DRAG) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "drag end change to move event");
        rectChangeReason = RectChangeReason::MOVE;
    }
    // js callback should run in js thread
    auto jsCallback = [self = weakRef_, rect, rectChangeReason, env = env_, funcName = __func__] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsWindowListener::OnRectChange");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: this listener or env is nullptr", funcName);
            return;
        }
        HandleScope handleScope(env);
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: Failed to create js object", funcName);
            return;
        }
        napi_value rectObjValue = GetRectAndConvertToJsValue(env, rect);
        if (rectObjValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: Failed to create rect js object", funcName);
            return;
        }
        napi_set_named_property(env, objValue, "rect", rectObjValue);
        napi_set_named_property(env, objValue, "reason", CreateJsValue(env, rectChangeReason));
        napi_value argv[] = {objValue};
        thisListener->CallJsMethod(WINDOW_RECT_CHANGE_CB.c_str(), argv, ArraySize(argv));
    };
    if (napi_status::napi_ok != napi_send_event(env_, jsCallback, napi_eprio_immediate)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "failed to send event");
        return;
    }
    currRect_ = rect;
    if (rectChangeReason == RectChangeReason::UNDEFINED) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "ignore undefined reason to change last reason");
    } else {
        currentReason_ = rectChangeReason;
    }
}

void JsWindowListener::OnSubWindowClose(bool& terminateCloseProcess)
{
    const char* const where = __func__;
    auto jsCallback = [self = weakRef_, &terminateCloseProcess, env = env_, where] () mutable {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsWindowListener::OnSubWindowClose");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s this listener or env is nullptr", where);
            return;
        }
        HandleScope handleScope(env);
        bool value = terminateCloseProcess;
        napi_value returnValue = thisListener->CallJsMethod(SUB_WINDOW_CLOSE_CB.c_str(), nullptr, 0);
        if (napi_get_value_bool(env, returnValue, &value) == napi_ok) {
            terminateCloseProcess = value;
        }
    };

    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_SUB, "get main event handler failed!");
        return;
    }
    eventHandler_->PostSyncTask(jsCallback, "wms:JsWindowRectListener::OnSubWindowClose",
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void JsWindowListener::OnMainWindowClose(bool& terminateCloseProcess)
{
    auto jsCallback = [self = weakRef_, &terminateCloseProcess, env = env_]() mutable {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsWindowListener::OnMainWindowClose");
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "this listener or env is nullptr");
            return;
        }
        HandleScope handleScope(env);
        bool value = terminateCloseProcess;
        napi_value returnValue = thisListener->CallJsMethod(WINDOW_STAGE_CLOSE_CB.c_str(), nullptr, 0);
        if (napi_get_value_bool(env, returnValue, &value) == napi_ok) {
            terminateCloseProcess = value;
        }
    };

    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_LIFE, "get main event handler failed!");
        return;
    }
    eventHandler_->PostSyncTask(jsCallback, "wms:JsWindowListener::OnMainWindowClose",
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

WmErrorCode JsWindowListener::CanCancelUnregister(const std::string& eventType)
{
    if (eventType == WINDOW_WILL_CLOSE_CB) {
        if (asyncCloseExecuteCount_.load() != 0) {
            TLOGE(WmsLogTag::WMS_PC, "async task in excuting, not unregister");
            return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        }
    }
    return WmErrorCode::WM_OK;
}

void JsWindowListener::InitAsyncCloseCallback(sptr<Window> window)
{
    if (closeAsyncCallback_ != nullptr) {
        return;
    }
    TLOGD(WmsLogTag::WMS_DECOR, "init");
    const char* const where = __func__;
    auto thenCallback = [self = weakRef_, weakWindow = wptr<Window>(window), where](napi_env env,
        size_t argc, napi_value* argv) {
        auto window = weakWindow.promote();
        auto thisListener = self.promote();
        if (window == nullptr || thisListener == nullptr) {
            TLOGNE(WmsLogTag::WMS_DECOR, "%{public}s window or thisListener is nullptr", where);
            return;
        }
        thisListener->asyncCloseExecuteCount_.fetch_sub(1);
        bool notNeedClose = false;
        if (argc != ARG_COUNT_ONE || !ConvertFromJsValue(env, argv[INDEX_ZERO], notNeedClose)) {
            TLOGNE(WmsLogTag::WMS_DECOR, "%{public}s Failed to convert parameter to notNeedClose", where);
        }
        TLOGD(WmsLogTag::WMS_DECOR, "%{public}s notNeedClose: %{public}d", where, notNeedClose);
        if (!notNeedClose) {
            window->CloseDirectly();
        }
    };

    auto catchCallback = [self = weakRef_, weakWindow = wptr<Window>(window), where](napi_env env,
        size_t argc, napi_value* argv) {
        auto window = weakWindow.promote();
        auto thisListener = self.promote();
        if (window == nullptr || thisListener == nullptr) {
            TLOGNE(WmsLogTag::WMS_DECOR, "%{public}s window or thisListener is nullptr", where);
            return;
        }
        thisListener->asyncCloseExecuteCount_.fetch_sub(1);
        window->CloseDirectly();
    };

    closeAsyncCallback_ = sptr<AsyncCallback>::MakeSptr(thenCallback, catchCallback);
}

void JsWindowListener::OnWindowWillClose(sptr<Window> window)
{
    InitAsyncCloseCallback(window);
    const char* const where = __func__;
    auto jsCallback = [self = weakRef_, weakWindow = wptr<Window>(window), env = env_,
        weakCloseAsyncCallback = wptr<AsyncCallback>(closeAsyncCallback_), where] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsWindowListener::OnWindowWillClose");
        auto thisListener = self.promote();
        auto window = weakWindow.promote();
        auto closeAsyncCallback = weakCloseAsyncCallback.promote();
        if (thisListener == nullptr || env == nullptr ||
            window == nullptr || closeAsyncCallback == nullptr) {
            TLOGNE(WmsLogTag::WMS_DECOR, "%{public}s this listener or env or window or "
                "closeAsyncCallback is nullptr", where);
            return;
        }
        HandleScope handleScope(env);
        napi_value returnValue = thisListener->CallJsMethod(WINDOW_WILL_CLOSE_CB.c_str(), nullptr, 0);
        thisListener->asyncCloseExecuteCount_.fetch_add(1);
        bool isPromiseCallback = CallPromise(env, returnValue, closeAsyncCallback);
        if (!isPromiseCallback) {
            window->CloseDirectly();
        }
    };

    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_DECOR, "get main event handler failed!");
        return;
    }
    eventHandler_->PostSyncTask(jsCallback, "wms:JsWindowListener::OnWindowWillClose",
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void JsWindowListener::OnWindowHighlightChange(bool isHighlight)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "isHighlight: %{public}d", isHighlight);
    auto jsCallback = [self = weakRef_, isHighlight, env = env_, where = __func__] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s: this listener or env is nullptr", where);
            return;
        }
        HandleScope handleScope(env);
        napi_value argv[] = { CreateJsValue(env, isHighlight) };
        thisListener->CallJsMethod(WINDOW_HIGHLIGHT_CHANGE_CB.c_str(), argv, ArraySize(argv));
    };
    if (napi_status::napi_ok != napi_send_event(env_, jsCallback, napi_eprio_immediate)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "failed to send event");
    }
}
} // namespace Rosen
} // namespace OHOS