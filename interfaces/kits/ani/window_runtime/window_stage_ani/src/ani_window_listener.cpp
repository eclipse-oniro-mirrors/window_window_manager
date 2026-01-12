/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "ani_window_listener.h"

#include <hitrace_meter.h>

#include "ani_window_utils.h"
#include "event_handler.h"
#include "event_runner.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

AniWindowListener::~AniWindowListener()
{
    ani_status ret = ANI_OK;
    if (env_ != nullptr && aniCallback_ != nullptr) {
        ret = env_->GlobalReference_Delete(aniCallback_);
    }
    TLOGI(WmsLogTag::DEFAULT, "[ANI]~AniWindowListener ret: %{public}u", ret);
}

void AniWindowListener::OnLastStrongRef(const void *)
{
}

void AniWindowListener::SetMainEventHandler()
{
    auto mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainRunner == nullptr) {
        return;
    }
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainRunner);
}

void AniWindowListener::SetTimeout(int64_t timeout)
{
    noInteractionTimeout_ = timeout;
}

int64_t AniWindowListener::GetTimeout() const
{
    return noInteractionTimeout_;
}

void AniWindowListener::OnSizeChange(Rect rect, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI] wh[%{public}u, %{public}u], reason = %{public}u", rect.width_,
        rect.height_, reason);
    if (currRect_.width_ == rect.width_ && currRect_.height_ == rect.height_ &&
        reason != WindowSizeChangeReason::DRAG_END) {
        TLOGI(WmsLogTag::DEFAULT, "[ANI]no need to change size");
        return;
    }
    currRect_ = rect;

    auto task = [self = weakRef_, rect, vm = vm_] () {
        auto thisListener = self.promote();
        if (thisListener == nullptr || vm == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener, vm or callback is nullptr");
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]Get env failed, ret:%{public}u", ret);
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(env, "@ohos.window.window", "runWindowSizeCallback",
            nullptr, thisListener->aniCallback_, AniWindowUtils::CreateAniSize(env, rect.width_, rect.height_));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:AniWindowListener::SizeChangeCallback", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnModeChange(WindowMode mode, bool hasDeco)
{
}

void AniWindowListener::OnSystemBarPropertyChange(DisplayId displayId, const SystemBarRegionTints& tints)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto task = [self = weakRef_, eng = env_, displayId, tints] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener, eng or callback is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runSystemBarTintChangeCallback",
            nullptr, thisListener->aniCallback_, AniWindowUtils::CreateAniSystemBarTintState(eng, displayId, tints));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, __func__, 0, AppExecFwk::EventQueue::Priority::HIGH);
}

void AniWindowListener::OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type,
    const sptr<OccupiedAreaChangeInfo>& info)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto task = [self = weakRef_, eng = env_, avoidArea, type] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "AniWindowListener::OnAvoidAreaChanged");
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener, eng or callback is nullptr");
            return;
        }
        auto nativeAvoidArea = AniWindowUtils::CreateAniAvoidArea(eng, avoidArea, type);
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runAvoidAreaChangeCallback",
            nullptr, thisListener->aniCallback_, nativeAvoidArea, static_cast<ani_int>(type));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, __func__, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnSystemDensityChanged(float density)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto task = [self = weakRef_, density, eng = env_] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "AniWindowListener::OnSystemDensityChanged");
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener, eng or callback is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runSystemDensityChangeCallback",
            nullptr, thisListener->aniCallback_, static_cast<ani_float>(density));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, __func__, 0, AppExecFwk::EventQueue::Priority::HIGH);
}

void AniWindowListener::OnDisplayIdChanged(DisplayId displayId)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto task = [self = weakRef_, displayId, eng = env_] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "AniWindowListener::OnDisplayIdChanged");
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener, eng or callback is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runDisplayIdChangeCallback",
            nullptr, thisListener->aniCallback_, static_cast<ani_long>(displayId));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, __func__, 0, AppExecFwk::EventQueue::Priority::HIGH);
}

void AniWindowListener::LifeCycleCallback(LifeCycleEventType eventType)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]LifeCycleCallback, envent type: %{public}u", eventType);
    auto task = [self = weakRef_, eventType, caseType = caseType_, eng = env_] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "AniWindowListener::LifeCycleCallback");
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener or eng is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window",
            caseType == CaseType::CASE_STAGE ? "runWindowStageEventCallback" : "runWindowEventCallback",
            nullptr, thisListener->aniCallback_, static_cast<ani_int>(eventType));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:AniWindowListener::LifeCycleCallback", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::WindowStageLifecycleCallback(WindowStageLifeCycleEventType eventType)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]windowStageLifecycleCallback, envent type: %{public}u", eventType);
    auto task = [self = weakRef_, eventType, eng = env_] () {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "AniWindowListener::windowStageLifecycleCallback");
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener or eng is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runWindowStageLifecycleEventCallback",
            nullptr, thisListener->aniCallback_, static_cast<ani_int>(eventType));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:AniWindowListener::windowStageLifecycleCallback", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::AfterForeground()
{
    if (state_ == WindowState::STATE_INITIAL || state_ == WindowState::STATE_HIDDEN) {
        LifeCycleCallback(LifeCycleEventType::FOREGROUND);
        state_ = WindowState::STATE_SHOWN;
    } else {
        TLOGD(WmsLogTag::DEFAULT, "[ANI]window is already shown");
    }
}

void AniWindowListener::AfterBackground()
{
    if (state_ == WindowState::STATE_INITIAL || state_ == WindowState::STATE_SHOWN) {
        LifeCycleCallback(LifeCycleEventType::BACKGROUND);
        state_ = WindowState::STATE_HIDDEN;
    } else {
        TLOGD(WmsLogTag::DEFAULT, "[ANI]window is already hide");
    }
}

void AniWindowListener::AfterFocused()
{
    LifeCycleCallback(LifeCycleEventType::ACTIVE);
}

void AniWindowListener::AfterUnfocused()
{
    LifeCycleCallback(LifeCycleEventType::INACTIVE);
}

void AniWindowListener::AfterResumed()
{
    if (caseType_ == CaseType::CASE_STAGE) {
        LifeCycleCallback(LifeCycleEventType::RESUMED);
    }
}

void AniWindowListener::AfterPaused()
{
    if (caseType_ == CaseType::CASE_STAGE) {
        LifeCycleCallback(LifeCycleEventType::PAUSED);
    }
}

void AniWindowListener::AfterDestroyed()
{
    if (caseType_ == CaseType::CASE_WINDOW) {
        LifeCycleCallback(LifeCycleEventType::DESTROYED);
    }
}

void AniWindowListener::OnSizeChange(const sptr<OccupiedAreaChangeInfo>& info,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    if (info == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "OccupiedAreaChangeInfo is nullptr");
        return;
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD,
        "OccupiedAreaChangeInfo, type: %{public}u, input rect: [%{public}d, %{public}d, %{public}u, %{public}u]",
        static_cast<uint32_t>(info->type_),
        info->rect_.posX_, info->rect_.posY_, info->rect_.width_, info->rect_.height_);
    // callback should run in js thread
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr || env_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "this listener or env is nullptr.");
        return;
    }
    AniWindowUtils::CallAniFunctionVoid(env_, "@ohos.window.window", "runKeyboardHeightChangeCallback",
        nullptr, thisListener->aniCallback_, static_cast<ani_int>(info->rect_.height_));
}

void AniWindowListener::OnKeyboardDidShow(const KeyboardPanelInfo& keyboardPanelInfo)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD,
        "keyboardPanelInfo, beginRect: %{public}s, endRect: %{public}s",
        keyboardPanelInfo.beginRect_.ToString().c_str(), keyboardPanelInfo.endRect_.ToString().c_str());
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr || env_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "this listener or env is nullptr.");
        return;
    }
    AniWindowUtils::CallAniFunctionVoid(env_, "@ohos.window.window", "runKeyboardDidShowCallback",
        nullptr, thisListener->aniCallback_, AniWindowUtils::CreateAniKeyboardInfo(env_, keyboardPanelInfo));
}

void AniWindowListener::OnKeyboardDidHide(const KeyboardPanelInfo& keyboardPanelInfo)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD,
        "keyboardPanelInfo, beginRect: %{public}s, endRect: %{public}s",
        keyboardPanelInfo.beginRect_.ToString().c_str(), keyboardPanelInfo.endRect_.ToString().c_str());
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr || env_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "this listener or env is nullptr.");
        return;
    }
    AniWindowUtils::CallAniFunctionVoid(env_, "@ohos.window.window", "runKeyboardDidHideCallback",
        nullptr, thisListener->aniCallback_, AniWindowUtils::CreateAniKeyboardInfo(env_, keyboardPanelInfo));
}

void AniWindowListener::OnKeyboardWillShow(const KeyboardAnimationInfo& keyboardAnimationInfo,
    const KeyboardAnimationCurve& curve)
{
    KeyboardWillAnimateWithName(keyboardAnimationInfo, "runKeyboardWillShowCallback", curve);
}

void AniWindowListener::OnKeyboardWillHide(const KeyboardAnimationInfo& keyboardAnimationInfo,
    const KeyboardAnimationCurve& curve)
{
    KeyboardWillAnimateWithName(keyboardAnimationInfo, "runKeyboardWillHideCallback", curve);
}

void AniWindowListener::KeyboardWillAnimateWithName(const KeyboardAnimationInfo& keyboardAnimationInfo,
    const char* fn, const KeyboardAnimationCurve& curve)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD,
        "keyboardAnimationInfo, beginRect: %{public}s, endRect: %{public}s",
        keyboardAnimationInfo.beginRect.ToString().c_str(), keyboardAnimationInfo.endRect.ToString().c_str());
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr || env_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "this listener or env is nullptr.");
        return;
    }
    AniWindowUtils::CallAniFunctionVoid(env_, "@ohos.window.window", fn, nullptr,
        thisListener->aniCallback_, AniWindowUtils::CreateAniAnimationInfo(env_, keyboardAnimationInfo, curve));
}

void AniWindowListener::OnTouchOutside() const
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]in");
    auto task = [self = weakRef_, eng = env_] () {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_EVENT, "[ANI]thisListener, eng or callback is nullptr!");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runWindowTouchOutCallback",
            nullptr, thisListener->aniCallback_);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_EVENT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:AniWindowListener::TouchOutsideCallback", 0,
        AppExecFwk::EventQueue::Priority::HIGH);
}

void AniWindowListener::OnDialogTargetTouch() const
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]in");
    auto task = [self = weakRef_, eng = env_] () {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_EVENT, "[ANI]thisListener, eng or callback is nullptr!");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runWindowDialogTargetCallback",
            nullptr, thisListener->aniCallback_);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_EVENT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:AniWindowListener::DialogTargetTouchCallback", 0,
        AppExecFwk::EventQueue::Priority::HIGH);
}

void AniWindowListener::OnWindowNoInteractionCallback()
{
    TLOGI(WmsLogTag::WMS_EVENT, "[ANI]in");
    auto task = [self = weakRef_, vm = vm_] () {
        auto thisListener = self.promote();
        if (thisListener == nullptr || vm == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_EVENT, "[ANI]thisListener, vm or callback is nullptr!");
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::WMS_EVENT, "[ANI]Get env failed, ret: %{public}u", ret);
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(env, "@ohos.window.window", "runWindowNoInteractionCallback",
            "C{std.core.Object}:", thisListener->aniCallback_);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_EVENT, "Get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:AniWindowListener::WindowNoInteractionCallback", 0,
        AppExecFwk::EventQueue::Priority::HIGH);
}

void AniWindowListener::OnScreenshot()
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto task = [self = weakRef_, eng = env_] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener, eng or callback is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runWindowListenerVoidArgCallback",
            nullptr, thisListener->aniCallback_);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, __func__, 0, AppExecFwk::EventQueue::Priority::HIGH);
}

void AniWindowListener::OnDialogDeathRecipient() const
{
}

void AniWindowListener::OnGestureNavigationEnabledUpdate(bool enable)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto task = [self = weakRef_, eng = env_, enable] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener, eng or callback is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runWindowListenerBooleanArgCallback",
            nullptr, thisListener->aniCallback_, ani_boolean(enable));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, __func__, 0, AppExecFwk::EventQueue::Priority::HIGH);
}

void AniWindowListener::OnWaterMarkFlagUpdate(bool showWaterMark)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto task = [self = weakRef_, eng = env_, showWaterMark] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener, eng or callback is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runWindowListenerBooleanArgCallback",
            nullptr, thisListener->aniCallback_, ani_boolean(showWaterMark));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, __func__, 0, AppExecFwk::EventQueue::Priority::HIGH);
}

void AniWindowListener::OnWindowStatusChange(WindowStatus windowstatus)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI] windowstatus: %{public}u", windowstatus);
    auto task = [self = weakRef_, windowstatus, eng = env_] () {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener or eng is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runWindowStatusCallback",
            nullptr, thisListener->aniCallback_, static_cast<ani_int>(windowstatus));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:AniWindowListener::StatusChangeCallback", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnWindowVisibilityChangedCallback(const bool isVisible)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto task = [self = weakRef_, eng = env_, isVisible] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener, eng or callback is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runWindowListenerBooleanArgCallback",
            nullptr, thisListener->aniCallback_, ani_boolean(isVisible));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, __func__, 0, AppExecFwk::EventQueue::Priority::HIGH);
}

void AniWindowListener::OnOcclusionStateChanged(const WindowVisibilityState state)
{
    const char* const where = __func__;
    auto task = [self = weakRef_, state, where, env = env_] () {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] %{public}s: listener or env is null", where);
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(env, "@ohos.window.window", "runOcclusionStateChangeCallback",
            nullptr, thisListener->aniCallback_, static_cast<ani_int>(state));
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "[ANI] %{public}s: occlusionState=%{public}u",
            where, static_cast<uint32_t>(state));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] main event handler is null");
        return;
    }
    eventHandler_->PostTask(task, "[ANI] wms:AniWindowListener::OnOcclusionStateChanged", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnFrameMetricsChanged(const FrameMetrics& metrics)
{
    const char* const where = __func__;
    auto task = [self = weakRef_, metrics, where, env = env_]() {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] %{public}s: listener or env is null", where);
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(env, "@ohos.window.window", "runFrameMetricsChangeCallback",
            nullptr, thisListener->aniCallback_, AniWindowUtils::CreateAniFrameMetrics(env, metrics));
        TLOGND(WmsLogTag::WMS_ATTRIBUTE, "[ANI] %{public}s: firstDrawFrame=%{public}d"
            ", inputHandlingDuration=%{public}" PRIu64 ", layoutMeasureDuration=%{public}" PRIu64
            ", vsyncTimestamp=%{public}" PRIu64, where, metrics.firstDrawFrame_, metrics.inputHandlingDuration_,
            metrics.layoutMeasureDuration_, metrics.vsyncTimestamp_);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] main event handler is null");
        return;
    }
    eventHandler_->PostTask(task, "[ANI] wms:AniWindowListener::OnFrameMetricsChanged", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnWindowHighlightChange(bool isHighlight)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    auto task = [self = weakRef_, eng = env_, isHighlight] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "[ANI]this listener, eng or callback is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runWindowListenerBooleanArgCallback",
            nullptr, thisListener->aniCallback_, ani_boolean(isHighlight));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, __func__, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnWindowTitleButtonRectChanged(const TitleButtonRect& titleButtonRect)
{
    auto task = [self = weakRef_, titleButtonRect, eng = env_] () {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_DECOR, "[ANI]this listener, eng or callback is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runWindowTitleButtonRectChangedCallback",
            nullptr, thisListener->aniCallback_, AniWindowUtils::CreateAniTitleButtonRect(eng, titleButtonRect));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_DECOR, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:AniWindowListener::WindowTitleButtonRectChangedCallback", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnRectChange(Rect rect, WindowSizeChangeReason reason)
{
    if (currRect_ == rect && reason == WindowSizeChangeReason::UNDEFINED) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "skip redundant rect update");
        return;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI] rect: %{public}s, reason: %{public}d", rect.ToString().c_str(), reason);
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
    auto task = [self = weakRef_, rect, rectChangeReason, vm = vm_] () {
        auto thisListener = self.promote();
        if (thisListener == nullptr || vm == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI]this listener, vm or callback is nullptr");
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "[ANI]Get env failed, ret:%{public}u", ret);
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(env, "@ohos.window.window", "runWindowRectChangeCallback",
            nullptr, thisListener->aniCallback_, AniWindowUtils::CreateAniRect(env, rect),
            static_cast<ani_int>(rectChangeReason));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:AniWindowListener::RectChangeCallback", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
    currRect_ = rect;
    if (rectChangeReason == RectChangeReason::UNDEFINED) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "ignore undefined reason to change last reason");
    } else {
        currentReason_ = rectChangeReason;
    }
}

void AniWindowListener::OnSubWindowClose(bool& terminateCloseProcess)
{
    TLOGI(WmsLogTag::WMS_SUB, "[ANI]");
    auto task = [self = weakRef_, eng = env_, terminateCloseProcess] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_SUB, "[ANI]this listener, eng or callback is nullptr");
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(eng, "@ohos.window.window", "runWindowListenerBooleanArgCallback",
            nullptr, thisListener->aniCallback_, ani_boolean(terminateCloseProcess));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::DEFAULT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, __func__, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnMainWindowClose(bool& terminateCloseProcess)
{
    TLOGI(WmsLogTag::WMS_PC, "[ANI]");
    auto thisListener = weakRef_.promote();
    if (thisListener == nullptr || env_ == nullptr || thisListener->aniCallback_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]this listener, env_ or callback is nullptr");
        return;
    }
    ani_ref preClose;
    AniWindowUtils::CallAniFunctionRef(env_, preClose, thisListener->aniCallback_, 0);
    auto aniRet = AniWindowUtils::GetBooleanObject(env_, static_cast<ani_object>(preClose), terminateCloseProcess);
    if (aniRet != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PC, "[ANI]Get terminateCloseProcess failed, ret: %{public}u", aniRet);
    }
}

void AniWindowListener::AfterLifecycleForeground()
{
    if (caseType_ == CaseType::CASE_STAGE) {
        if (state_ == WindowState::STATE_INITIAL || state_ == WindowState::STATE_HIDDEN) {
            WindowStageLifecycleCallback(WindowStageLifeCycleEventType::FOREGROUND);
            state_ = WindowState::STATE_SHOWN;
        }
    }
}

void AniWindowListener::AfterLifecycleBackground()
{
    if (caseType_ == CaseType::CASE_STAGE) {
        if (state_ == WindowState::STATE_INITIAL || state_ == WindowState::STATE_SHOWN) {
            WindowStageLifecycleCallback(WindowStageLifeCycleEventType::BACKGROUND);
            state_ = WindowState::STATE_HIDDEN;
        }
    }
}

void AniWindowListener::AfterLifecycleResumed()
{
    if (caseType_ == CaseType::CASE_STAGE) {
        WindowStageLifecycleCallback(WindowStageLifeCycleEventType::RESUMED);
    }
}

void AniWindowListener::AfterLifecyclePaused()
{
    if (caseType_ == CaseType::CASE_STAGE) {
        WindowStageLifecycleCallback(WindowStageLifeCycleEventType::PAUSED);
    }
}

void AniWindowListener::OnRotationChange(const RotationChangeInfo& rotationChangeInfo,
    RotationChangeResult& rotationChangeResult)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "[ANI]");
    auto task = [self = weakRef_, eng = env_, rotationChangeInfo, &rotationChangeResult] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "AniWindowListener::OnRotationChange");
        auto thisListener = self.promote();
        if (thisListener == nullptr || eng == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGE(WmsLogTag::WMS_ROTATION, "[ANI]this listener, eng or callback is nullptr");
            return;
        }
        ani_object rotationInfoObj = AniWindowUtils::CreateAniRotationChangeInfo(eng, rotationChangeInfo);
        if (rotationInfoObj == nullptr) {
            TLOGE(WmsLogTag::WMS_ROTATION, "failed to create ani object");
            return;
        }
        ani_ref rotationChangeResultObj;
        AniWindowUtils::CallAniFunctionRef(eng, rotationChangeResultObj, thisListener->aniCallback_, 1,
            rotationInfoObj);
        if (rotationChangeResultObj != nullptr) {
            AniWindowUtils::ParseRotationChangeResult(eng, static_cast<ani_object>(rotationChangeResultObj),
                rotationChangeResult);
        }
    };
    if (!eventHandler_ ||
        (eventHandler_->GetEventRunner() && eventHandler_->GetEventRunner()->IsCurrentRunnerThread())) {
        TLOGW(WmsLogTag::WMS_ROTATION, "get main event handler failed or current is already main thread!");
        return task();
    }
    eventHandler_->PostSyncTask(task, __func__, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnRectChangeInGlobalDisplay(const Rect& rect, WindowSizeChangeReason reason)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI] rect:%{public}s, reason:%{public}d", rect.ToString().c_str(), reason);
    auto it = JS_SIZE_CHANGE_REASON.find(reason);
    RectChangeReason rectChangeReason = (it != JS_SIZE_CHANGE_REASON.end()) ? it->second : RectChangeReason::UNDEFINED;
    auto task = [self = weakRef_, rect, rectChangeReason, vm = vm_] () {
        auto thisListener = self.promote();
        if (thisListener == nullptr || vm == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "[ANI]this listener, vm or callback is nullptr");
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "[ANI]get env failed, ret:%{public}u", ret);
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(env, "@ohos.window.window", "runRectChangeInGlobalDisplayCallback",
            nullptr, thisListener->aniCallback_, AniWindowUtils::CreateAniRect(env, rect),
            static_cast<ani_int>(rectChangeReason));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:AniWindowListener::RectChangeInGlobalDisplayCallback", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnSecureLimitChange(bool isLimit)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "isLimit: %{public}d", isLimit);
    auto task = [self = weakRef_, isLimit, vm = vm_, where = __func__]() {
        auto thisListener = self.promote();
        if (thisListener == nullptr || vm == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s: this listener, vm or callback is nullptr", where);
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "[ANI]get env failed, ret:%{public}u", ret);
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(env, "@ohos.window.window", "runUiExtensionSecureLimitChangeCallback",
            nullptr, thisListener->aniCallback_, isLimit);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, __func__, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnWindowStatusDidChange(WindowStatus status)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "[ANI] status:%{public}u", status);
    auto task = [self = weakRef_, status, vm = vm_] () {
        auto thisListener = self.promote();
        if (thisListener == nullptr || vm == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "[ANI]this listener, vm or callback is nullptr");
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "[ANI]get env failed, ret:%{public}u", ret);
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(env, "@ohos.window.window", "runWindowStatusDidChangeCallback",
            nullptr, thisListener->aniCallback_, static_cast<ani_int>(status));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, "wms:AniWindowListener::WindowStatusDidChangeCallback", 0,
        AppExecFwk::EventQueue::Priority::HIGH);
}

void AniWindowListener::OnAcrossDisplaysChanged(bool isAcrossDisplays)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    const char* const where = __func__;
    auto task = [self = weakRef_, isAcrossDisplays, env = env_, where] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] %{public}s: listener or env is null", where);
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(env, "C{@ohos.window.window}",
            "runMainWindowFullScreenAcrossDisplaysChangedCallback",
            nullptr, thisListener->aniCallback_, static_cast<ani_boolean>(isAcrossDisplays));
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "[ANI] %{public}s: isAcrossDisplays=%{public}u",
            where, isAcrossDisplays);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] main event handler is null");
        return;
    }
    eventHandler_->PostTask(task, "[ANI] wms:AniWindowListener::OnAcrossDisplaysChanged", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnScreenshotAppEvent(ScreenshotEventType type)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[ANI]");
    const char* const where = __func__;
    auto task = [self = weakRef_, type, env = env_, where] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] %{public}s: listener or env is null", where);
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(env, "C{@ohos.window.window}",
            "runScreenshotAppEvent",
            nullptr, thisListener->aniCallback_, type);
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "[ANI] main event handler is null");
        return;
    }
    eventHandler_->PostTask(task, "[ANI] wms:AniWindowListener::OnScreenshotAppEvent", 0,
        AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void AniWindowListener::OnFreeWindowModeChange(bool isInFreeWindowMode)
{
    TLOGI(WmsLogTag::WMS_PC, "[ANI]");
    auto task = [self = weakRef_, vm = vm_, isInFreeWindowMode] {
        auto thisListener = self.promote();
        if (thisListener == nullptr || vm == nullptr || thisListener->aniCallback_ == nullptr) {
            TLOGNE(WmsLogTag::WMS_PC, "[ANI]this listener, vm or callback is nullptr");
            return;
        }
        ani_env* env = nullptr;
        ani_status ret = vm->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK || env == nullptr) {
            TLOGNE(WmsLogTag::WMS_PC, "[ANI]get env failed, ret: %{public}u", ret);
            return;
        }
        AniWindowUtils::CallAniFunctionVoid(env, "@ohos.window.window", "runWindowListenerBooleanArgCallback",
            nullptr, thisListener->aniCallback_, ani_boolean(isInFreeWindowMode));
    };
    if (!eventHandler_) {
        TLOGE(WmsLogTag::WMS_PC, "get main event handler failed!");
        return;
    }
    eventHandler_->PostTask(task, __func__, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}
} // namespace Rosen
} // namespace OHOS